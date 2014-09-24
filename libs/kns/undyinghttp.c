/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ==============================================================================
*
*/

#include <kns/extern.h>

#define KSTREAM_IMPL KHttpStream
typedef struct KHttpStream KHttpStream;

#define KFILE_IMPL KHttpUndyingFile
typedef struct KHttpUndyingFile KHttpUndyingFile;

#include <kfs/impl.h>

#include "http-priv.h"

#include <kns/http.h> /* KNSManagerMakeHttpFile */
#include <kns/manager.h> /* KNSManager */

#ifdef ERR
#undef ERR
#endif

#include <klib/debug.h> /* DBGMSG */
#include <klib/printf.h> /* string_vprintf */
#include <klib/rc.h>

#include <sysalloc.h> /* for malloc/calloc */

#include <assert.h>
#include <string.h> /* memset */

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/*******************************************************************************
 * KHttpUndyingFile
 */

struct KHttpUndyingFile
{
    KFile dad;

    uint64_t size; /* file's size */

    const KFile *file; /* this struct is a wrapper around this file */

    const KNSManager *mgr; /* mgr to construct the file */
    const char *url; /* url to construct the file */
    ver_t vers; /* vers to construct the file */
};

static rc_t CC KHttpUndyingFileDestroy(KHttpUndyingFile *self) {
    rc_t rc = 0;

    if (self == NULL) {
        return 0;
    }

    free((void*)self->url);

    rc = KFileRelease(self->file);

    {
        rc_t rc2 = KNSManagerRelease(self->mgr);
        if (rc == 0) {
            rc = rc2;
        }
    }

    memset(self, 0, sizeof *self);

    free(self);

    return rc;
}

static KFile* GetUnderlyingFile(const KHttpUndyingFile *self) {
    if (self == NULL) {
        return NULL;
    }
    else {
        return (KFile*)self->file;
    }
}

static struct KSysFile* CC KHttpUndyingFileGetSysFile(
    const KHttpUndyingFile *self, uint64_t *offset)
{
    return KFileGetSysFile(GetUnderlyingFile(self), offset);
}

static
rc_t CC KHttpUndyingFileRandomAccess(const KHttpUndyingFile *self)
{
    return KFileRandomAccess(GetUnderlyingFile(self));
}

static rc_t CC KHttpUndyingFileSize(const KHttpUndyingFile *self,
    uint64_t *size)
{
    return KFileSize(GetUnderlyingFile(self), size);
}

static
rc_t CC KHttpUndyingFileSetSize(KHttpUndyingFile *self, uint64_t size)
{
    return KFileSetSize(GetUnderlyingFile(self), size);
}

static rc_t Revive(const KHttpUndyingFile *cself) {
    KHttpUndyingFile *self = (KHttpUndyingFile*)cself;

    rc_t rc = 0;

    assert(self);

    rc = KFileRelease(self->file);
    self->file = NULL;

    if (rc == 0) {
        rc = SecretKNSManagerMakeHttpFile(self->mgr,
            &self->file, NULL, self->vers, self->url);
    }

    return rc;
}

#define RETRY_REVIVE 2
#define RETRY_READ 2
static rc_t CC KHttpUndyingFileRead(const KHttpUndyingFile *self,
    uint64_t pos, void *buffer, size_t bsize, size_t *num_read)
{
    if (self == NULL || self->file == NULL) {
        return KFileRead(NULL, pos, buffer, bsize, num_read);
    }
    else {
        rc_t rc = 0, prev_rc = 0;
        int i = 0;
        for (i = 0; i < RETRY_REVIVE; ++i) {
            int r = 0;
            for (r = 0; r < RETRY_READ; ++r) {
                rc = KFileRead(self->file, pos, buffer, bsize, num_read);
                if (rc == 0) {
                    return rc;
                }
                else {
                    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@3 %s KFileRead(%s, %d)(%d/%d) = %R @@@@@@@"
                        "\n", __FUNCTION__,
                        self->url, pos, r + 1, RETRY_READ, rc));
                    if (prev_rc == 0) {
                        prev_rc = rc;
                    }
                    else if (rc == prev_rc) {
                        break;
                    }
                }
            }

            if (i < RETRY_REVIVE - 1) {
                DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@5 %s %d/%d KFileRead: Reviving... @@@@@@@@"
                        "\n", __FUNCTION__, i + 1, RETRY_REVIVE));
                rc = Revive(self);
                if (rc != 0) {
                    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@5 %s KFileRead(%s, %d): Revive = %R @@@@@@"
                        "\n", __FUNCTION__, self->url, pos, rc));
                    return rc;
                }
            }
            else {
                DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@5 %s %d/%d KFileRead: Not Reviving @@@@@@@@"
                        "\n", __FUNCTION__, i + 1, RETRY_REVIVE));
            }
        }

        return rc;
    }
}

static rc_t CC KHttpUndyingFileWrite ( KHttpUndyingFile *self, uint64_t pos, 
    const void *buffer, size_t size, size_t *num_writ )
{
    return KFileWrite(GetUnderlyingFile(self), pos, buffer, size, num_writ);
}

static
uint32_t CC KHttpUndyingFileGetType(const KHttpUndyingFile *self)
{
    return KFileType(GetUnderlyingFile(self));
}

static rc_t CC KHttpUndyingFileTimedRead(const KHttpUndyingFile *self,
    uint64_t pos, void *buffer, size_t bsize, size_t *num_read,
    struct timeout_t *tm)
{
    if (self == NULL || self->file == NULL) {
        return KFileTimedRead(NULL, pos, buffer, bsize, num_read, tm);
    }
    else {
        rc_t rc = 0, prev_rc = 0;
        int i = 0;
        for (i = 0; i < RETRY_REVIVE; ++i) {
            int r = 0;
            for (r = 0; r < RETRY_READ; ++r) {
                rc = KFileTimedRead(self->file, pos, buffer, bsize, num_read,
                    tm);
                if (rc == 0) {
                    return rc;
                }
                else {
                    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@3 %s KFileTimedRead(%s, %d)(%d/%d) = %R @@"
                        "\n", __FUNCTION__,
                        self->url, pos, r + 1, RETRY_READ, rc));
                    if (prev_rc == 0) {
                        prev_rc = rc;
                    }
                    else if (rc == prev_rc) {
                        break;
                    }
                }
            }

            if (i < RETRY_REVIVE - 1) {
                DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@5 %s %d/%d KFileTimedRead: Reviving... @@@"
                        "\n", __FUNCTION__, i + 1, RETRY_REVIVE));
                rc = Revive(self);
                if (rc != 0) {
                    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@5 %s KFileTimedRead(%s, %d): Revive = %R @"
                        "\n", __FUNCTION__, self->url, pos, rc));
                    return rc;
                }
            }
            else {
                DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@5 %s %d/%d KFileTimedRead: Not Reviving @@"
                        "\n", __FUNCTION__, i + 1, RETRY_REVIVE));
            }
        }
        return rc;
    }
}

static rc_t CC KHttpUndyingFileTimedWrite(KHttpUndyingFile *self, uint64_t pos, 
    const void *buffer, size_t size, size_t *num_writ,
    struct timeout_t *tm)
{
    return KFileTimedWrite(GetUnderlyingFile(self),
        pos, buffer, size, num_writ, tm);
}

static KFile_vt_v1 vtKHttpFile =  {
    1, 2,

    KHttpUndyingFileDestroy,
    KHttpUndyingFileGetSysFile,
    KHttpUndyingFileRandomAccess,
    KHttpUndyingFileSize,
    KHttpUndyingFileSetSize,
    KHttpUndyingFileRead,
    KHttpUndyingFileWrite,
    KHttpUndyingFileGetType,
    KHttpUndyingFileTimedRead,
    KHttpUndyingFileTimedWrite
};

static rc_t KNSManagerVMakeUndyingHttpFile(const KNSManager *self, const KFile **file,
    struct KStream *conn, ver_t vers, const char *url, va_list args)
{
    char buffer[PATH_MAX] = "";
    size_t num_writ = 0;
    rc_t rc = 0;
    KHttpUndyingFile *f = NULL;
    if (file == NULL) {
        return RC(rcNS, rcFile, rcConstructing, rcParam, rcNull);
    }
    *file = NULL;
    f = calloc(1, sizeof *f);
    if (f == NULL) {
        return RC(rcNS, rcFile, rcConstructing, rcMemory, rcExhausted);
    }
    rc = args ?
        string_vprintf(buffer, sizeof buffer, &num_writ, url, args) :
        string_printf (buffer, sizeof buffer, &num_writ, "%s", url);
    if (rc == 0) {
        f->url = string_dup_measure(buffer, NULL);
        if (f->url == NULL) {
            rc = RC(rcNS, rcFile, rcConstructing, rcMemory, rcExhausted);
        }
    }
    f->vers = vers;
    if (rc == 0) {
        rc = KNSManagerAddRef(self);
        if (rc == 0) {
            f->mgr = self;
        }
    }
    if (rc == 0) {
        assert(conn == NULL);
        rc = Revive(f);
        if (rc != 0) {
            DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@5 %s Revive = %R @@"
                "\n", __FUNCTION__, rc));
        }
    }
    if (rc == 0) {
        KHttpUndyingFileSize(f, &f->size);
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_MGR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@4 %s KNSManagerMakeUndyingHttpFile: size = %ld"
            "\n", __FUNCTION__, f->size));
    }
    if (rc == 0) {
        rc = KFileInit(&f->dad, (const KFile_vt*)&vtKHttpFile,
            "KHttpUndyingFile", buffer /* or should we better use f->url here? */, true, false);
    }
    if (rc == 0) {
        *file = &f->dad;
    }
    else {
        KHttpUndyingFileDestroy(f);
    }

    return rc;
}

/*LIB_EXPORT rc_t CC KNSManagerMakeUndyingHttpFile(const KNSManager *self,
    const KFile **file, struct KStream *conn, ver_t vers, const char *url, ...)
{
    rc_t rc = 0;
    va_list args;
    va_start(args, url);
    rc = KNSManagerVMakeHttpFile(self, file, conn, vers, url, args);
    va_end(args);
    return rc;
}*/

LIB_EXPORT rc_t CC KNSManagerVMakeHttpFile(const KNSManager *self,
    const KFile **file, struct KStream *conn, ver_t vers, const char *url,
    va_list args)
{
    return KNSManagerVMakeUndyingHttpFile(self, file, conn, vers, url, args);
}
