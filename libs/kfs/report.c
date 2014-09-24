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
* ===========================================================================
*
*/

#include <kfs/extern.h>

#include <klib/klib-priv.h>
#include <klib/out.h> /* KOutHandlerSet */
#include <klib/rc.h>

#include <kfs/directory.h>
#include <kfs/file.h> /* KFileWrite */

#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h> /* calloc */
#include <string.h> /* memset */

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

/*
 * An unrecoverable error happened.
 * We can help to solve it
 * by reporting information about known application execution environment.
 */

#define report ( * f -> report )
#define reportData ( * f -> reportData )
#define reportData1 ( * f -> reportData1 )
#define reportOpen ( * f -> reportOpen )
#define reportOpen1 ( * f -> reportOpen1 )
#define reportClose ( * f -> reportClose )
#define reportClose1 ( * f -> reportClose1 )
#define reportError ( * f -> reportError )
#define reportErrorStr ( * f -> reportErrorStr )
#define reportErrorStrImpl ( * f -> reportErrorStrImpl )
#define reportErrorStrInt ( * f -> reportErrorStrInt )
#define reportError3Str ( * f -> reportError3Str )

#define MAGIC 0x6521

typedef struct SFile {
    uint64_t magic;
    KFile* f;
    uint64_t pos;
} SFile;

static rc_t CC fileWriter
(void* data, const char* buffer, size_t bytes, size_t* num_writ)
{
    rc_t rc = 0;

    SFile* self = (SFile*)data;
    size_t dummy;
    if (num_writ == NULL)
    {   num_writ = &dummy; }
    *num_writ = 0;

    assert(self);
    if (self->magic != MAGIC) {
        return rc;
    }

    rc = KFileWrite(self->f, self->pos, buffer, bytes, num_writ);
    self->pos += *num_writ;

    return rc;
}

rc_t CC ReportRedirect
( KWrtHandler* handler, const char* filename, bool* to_file, bool finalize )
{
    rc_t rc = 0;
    if (!finalize) {
        if (handler) {
            handler->writer = KOutWriterGet();
            handler->data = KOutDataGet();
        }
        if (filename) {
            KDirectory* dir = NULL;
            SFile* data = calloc(1, sizeof *data);
            if (data == NULL) {
                return RC(rcFS, rcMemory, rcAllocating, rcMemory, rcExhausted);
            }
            data->magic = MAGIC;
            rc = KDirectoryNativeDir(&dir);
            if (rc == 0) {
                rc = KDirectoryCreateFile
                    (dir, &data->f, false, 0664, kcmInit, "%s", filename);
            }
            if (rc == 0) {
                rc = KOutHandlerSet(fileWriter, data);
            }
            RELEASE(KDirectory, dir);
        }
        if (rc != 0 || filename == NULL) {
            KOutHandlerSetStdErr();
            if (to_file)
            {   *to_file = false; }
        }
        else if (to_file)
        {   *to_file = true; }

    }
    else {
        void* data = KOutDataGet();
        SFile* self = (SFile*)data;
        if (self) {
            if (self->magic == MAGIC) {
                RELEASE(KFile, self->f);;
            }
            memset(self, 0, sizeof *self);
            free(self);
        }
        if (handler) {
            KOutHandlerSet(handler->writer, handler->data);
        }
    }
    return rc;
}

rc_t CC ReportCWD ( const ReportFuncs *f, uint32_t indent )
{
    KDirectory *wd;
    rc_t rc = KDirectoryNativeDir ( & wd );
    if ( rc != 0 )
        reportError(indent, rc, "KDirectoryNativeDir");
    else
    {
        char cwd [ PATH_MAX + 1 ];
        rc = KDirectoryResolvePath ( wd, true, cwd, sizeof cwd, "." );
        KDirectoryRelease ( wd );

        if ( rc != 0 )
            reportError(indent, rc, "KDirectoryResolvePath");
        else
            reportData(indent, "Cwd", cwd, 0);
    }

    return rc;
}
