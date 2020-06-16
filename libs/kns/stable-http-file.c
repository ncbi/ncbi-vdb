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
* KStableHttpFile : retry wrapper on KHttpFile
*/

#include <kns/extern.h>

#include <kns/http-priv.h> /* HttpFileGetReadTimeouts */
#include <kns/stream.h> /* KStreamRelease */

#include <klib/debug.h> /* KStsLevel */
#include <klib/log.h> /* PLOGERR */
#include <klib/printf.h> /* KDataBufferVPrintf */
#include <klib/rc.h> /* RC */
#include <klib/status.h> /* KStsLevelGet */
#include <klib/time.h> /* KSleep */

#include <strtol.h> /* strtou64 */

#include "http-priv.h" /* SUPPORT_CHUNKED_READ */
#include "mgr-priv.h" /* KNSManager */

#include "stable-http-file-priv.h" /* KStableHttpFile */


/*********************************** RETRIER **********************************/

/* the last call to the file was successful */
static
void RetrierReset(const KStableHttpFile * cself, const char * func)
{
    KStableHttpFile * self = (KStableHttpFile *)cself;
    self->live = true;

    if (self->_failed) {

        self->_failed = false;

        self->_state = eRSJustRetry;
        self->_sleepTO = 0;

        if (KStsLevelGet() >= 1)
            PLOGERR(klogErr, (klogErr, 0, "$(f) success", "f=%s", func));
    }
}

/* reopen the underlying file */
static
rc_t RetrierReopenRemote(KStableHttpFile * self)
{
    rc_t rc = 0;

    uint32_t timeout = 1;

    KFileRelease(self->file);
    self->file = NULL;

    while (timeout < 10 * 60 /* 10 minutes */) {
        rc = KNSManagerVMakeHttpFileIntUnstableFromBuffer(self->mgr,
            &self->file, self->conn, self->vers, self->reliable,
            self->need_env_token, self->payRequired, self->url, &self->buf);
        if (rc == 0)
            break;

        KSleep(timeout);
        timeout *= 2;
    }

    return rc;
}

/* increase time before next call to the file */
static
bool RetrierIncSleepTO(KStableHttpFile * self)
{
    if (self->_sleepTO == 0)
        self->_sleepTO = 1;
    else
        self->_sleepTO *= 2;

    if (self->_sleepTO == 16)
        self->_sleepTO = 15;

    if (self->_sleepTO > 20 * 60 /* 20 minutes */)
        return false;
    else
        return true;
}


/* Get read timeout and Maximum TotalWait time for Read calls */
rc_t HttpFileGetReadTimeouts(const KFile * self, int32_t * millis,
    int32_t * totalMillis)
{
    const KStableHttpFile * obj = (const KStableHttpFile*)self;

    if (millis == NULL || totalMillis == NULL)
        return RC(rcNS, rcFile, rcAccessing, rcParam, rcNull);

    if (obj == NULL)
        return RC(rcNS, rcFile, rcAccessing, rcSelf, rcNull);

    *millis = obj->readWaitMillis;
    *totalMillis = obj->totalReadWaitMillis;

    return 0;
}

/* SET readWaitMillis and totalReadWaitMillis FOR HttpFileGetReadTimeouts() */
static
int32_t CalculateTotalReadWait(const KStableHttpFile * self,
    int32_t * ms)
{
    assert(self && self->mgr && ms);

    *ms = self->mgr->http_read_timeout;

    if (!self->reliable)
        /* There is no retry for not reliable files.
           Max wait timeout for regular HTTP file is http_read_timeout */
        return self->mgr->http_read_timeout;
    else
        /* Max wait timeout for reliable files is
           maxTotalWaitForReliableURLs_ms : see RetrierAgain() */
        if (self->mgr->http_read_timeout < 0)
            return self->mgr->http_read_timeout;
        else if (self->mgr->maxTotalWaitForReliableURLs_ms < 0)
            return self->mgr->maxTotalWaitForReliableURLs_ms;
        else if (self->mgr->http_read_timeout >
            self->mgr->maxTotalWaitForReliableURLs_ms)
        {
            return self->mgr->http_read_timeout;
        }
        else
            return self->mgr->maxTotalWaitForReliableURLs_ms;
}


/* The last call to the file was failed: prepare to retry.
 *
 * RetrierAgain returns:
 *  0:        to retry operation
 *  non-0 rc: to abandon - break retry loop
 */
static
rc_t RetrierAgain(const KStableHttpFile * cself,
    rc_t rc, const char * func)
{
    KStableHttpFile * self = (KStableHttpFile *)cself;

    bool retry = true;

    KTime_t total = ~0;
    bool retryFirst = false;

    assert(self && self->mgr);

    if (self->mgr->maxTotalWaitForReliableURLs_ms >= 0)
        total = self->mgr->maxTotalWaitForReliableURLs_ms / 1000;
    else
        total = ~0;

    retryFirst = self->mgr->retryFirstRead;

    if (total == 0)           /* don't retry when overall retry time == 0 */
        retry = false;
    else if (!self->live      /* don't retry when no READ calls succeed */
        && (!self->reliable   /* unless file is reliable */
            || !retryFirst))  /* and retry was not disabled by env.var. */
    {
        retry = false;
    }
    else
        switch (self->_state) {
        case eRSJustRetry:
            break;
        case eRSReopen:
            if (RetrierReopenRemote(self) != 0)
                retry = false;
            break;
        case eRSIncTO:
            if (!RetrierIncSleepTO(self))
                retry = false;
            else if (total != ~0 && KTimeStamp() - self->_tFailed > total)
                retry = false;
            break;
        default: assert(0); break;
        }

    if (!self->_failed) {
        self->_failed = true;
        self->_tFailed = KTimeStamp();
    }

    if (retry) {
        KStsLevel lvl = KStsLevelGet();
        if (lvl > 0) {
            switch (self->_state) {
            case eRSJustRetry:
                if (self->live)
                    PLOGERR(klogErr, (klogErr, rc,
                        "Cannot $(f): retrying...", "f=%s", func));
                else
                    PLOGERR(klogErr, (klogErr, rc,
                        "Cannot $(f): retrying [first]...", "f=%s", func));
                break;
            case eRSReopen:
                if (self->live)
                    PLOGERR(klogErr, (klogErr, rc,
                        "Cannot $(f): reopened, retrying...",
                        "f=%s", func));
                else
                    PLOGERR(klogErr, (klogErr, rc,
                        "Cannot $(f): reopened, retrying [first]...",
                        "f=%s", func));
                break;
            case eRSIncTO:
                if (self->live)
                    PLOGERR(klogErr, (klogErr, rc, "Cannot $(f): "
                        "sleep TO = $(to)s, retrying...",
                        "f=%s,to=%u", func, self->_sleepTO));
                else
                    PLOGERR(klogErr, (klogErr, rc, "Cannot $(f): "
                        "sleep TO = $(to)s, retrying [first]...",
                        "f=%s,to=%u", func, self->_sleepTO));
                break;
            default: assert(0); break;
            }
        }

        rc = 0;
    }
    else if (self->live)
        PLOGERR(klogErr, (klogErr, rc,
            "Cannot $(f): to=$(to)", "f=%s,to=%u", func, self->_sleepTO));
    else
        PLOGERR(klogErr, (klogErr, rc, "Cannot $(f)", "f=%s", func));

    if (rc == 0) {
        if (self->_sleepTO > 0) {
            STSMSG(2, ("Sleeping %us...", self->_sleepTO));
#ifndef TESTING_FAILURES
            KSleep(self->_sleepTO);
#endif
        }

        if (++self->_state == eRSMax)
            self->_state = eRSReopen;
    }

    return rc;
}


/************************ wrapper KFile implementation ************************/

static
rc_t CC KStblHttpFileDestroy(KStableHttpFile *self)
{
    rc_t rc = 0, r2 = 0;

    rc = KNSManagerRelease(self->mgr);

    r2 = KStreamRelease(self->conn);
    if (rc == 0 && r2 != 0)
        rc = r2;

    r2 = KDataBufferWhack(&self->buf);
    if (rc == 0 && r2 != 0)
        rc = r2;

    r2 = KFileRelease(self->file);
    if (rc == 0 && r2 != 0)
        rc = r2;

    free(self->url);

    memset(self, 0, sizeof *self);

    free(self);

    return rc;
}

static
struct KSysFile* CC KStblHttpFileGetSysFile(const KStableHttpFile *self,
    uint64_t *offset)
{
    *offset = 0;
    return NULL;
}

static
rc_t CC KStblHttpFileRandomAccess(const KStableHttpFile *self)
{
    return KFileRandomAccess(self->file);
}

static
rc_t CC KStblHttpFileSize(const KStableHttpFile *self, uint64_t *size)
{
    return KFileSize(self->file, size);
}

static
rc_t CC KStblHttpFileSetSize(KStableHttpFile *self, uint64_t size)
{
    return RC(rcNS, rcFile, rcUpdating, rcFile, rcReadonly);
}

static
rc_t CC KStblHttpFileTimedRead(const KStableHttpFile *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm)
{
    quitting_t quitting = self->quitting;

    while (true) {
        rc_t rc = KFileTimedRead(self->file, pos, buffer, bsize, num_read, tm);
        if (rc == 0) {
            RetrierReset(self, __func__);
            return rc;
        }
        else {
            if (quitting != NULL) {
                rc_t r2 = (*quitting)();
                if (r2 != 0)
                    return rc;
            }
            rc = RetrierAgain(self, rc, __func__);
            if (rc != 0)
                return rc;
        }
    }
}

static
rc_t CC KStblHttpFileRead(const KStableHttpFile *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read)
{
    quitting_t quitting = self->quitting;

    while (true) {
        rc_t rc = KFileRead(self->file, pos, buffer, bsize, num_read);
        if (rc == 0) {
            RetrierReset(self, __func__);
            return rc;
        }
        else {
            if (quitting != NULL) {
                rc_t r2 = (*quitting)();
                if (r2 != 0)
                    return rc;
            }
            rc = RetrierAgain(self, rc, __func__);
            if (rc != 0)
                return rc;
        }
    }
}

static
rc_t CC KStblHttpFileWrite(KStableHttpFile *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ)
{
    return RC(rcNS, rcFile, rcUpdating, rcInterface, rcUnsupported);
}

static
rc_t CC KStblHttpFileTimedWrite(KStableHttpFile *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ,
    struct timeout_t *tm)
{
    return RC(rcNS, rcFile, rcUpdating, rcInterface, rcUnsupported);
}

static
uint32_t CC KStblHttpFileGetType(const KStableHttpFile *self)
{
    return KFileType(self->file);
}

#if SUPPORT_CHUNKED_READ

static
rc_t CC KStblHttpFileTimedReadChunked(const KStableHttpFile * self,
    uint64_t pos, KChunkReader * chunks, size_t bytes, size_t * num_read,
    struct timeout_t * tm)
{
    quitting_t quitting = self->quitting;

    while (true) {
        rc_t rc =
            KFileTimedReadChunked(self->file, pos, chunks, bytes, num_read, tm);
        if (rc == 0) {
            RetrierReset(self, __func__);
            return rc;
        }
        else {
            if (quitting != NULL) {
                rc_t r2 = (*quitting)();
                if (r2 != 0)
                    return rc;
            }
            rc = RetrierAgain(self, rc, __func__);
            if (rc != 0)
                return rc;
        }
    }
}

static
rc_t CC KStblHttpFileReadChunked(const KStableHttpFile * self, uint64_t pos,
    KChunkReader * chunks, size_t bytes, size_t * num_read)
{
    quitting_t quitting = self->quitting;

    while (true) {
        rc_t rc = KFileReadChunked(self->file, pos, chunks, bytes, num_read);
        if (rc == 0) {
            RetrierReset(self, __func__);
            return rc;
        }
        else {
            if (quitting != NULL) {
                rc_t r2 = (*quitting)();
                if (r2 != 0)
                    return rc;
            }
            rc = RetrierAgain(self, rc, __func__);
            if (rc != 0)
                return rc;
        }
    }
}

#endif /* SUPPORT_CHUNKED_READ */

static KFile_vt_v1 vtKHttpFile =
{
    1,
#if SUPPORT_CHUNKED_READ
    3,
#else
    2,
#endif

    KStblHttpFileDestroy,
    KStblHttpFileGetSysFile,
    KStblHttpFileRandomAccess,
    KStblHttpFileSize,
    KStblHttpFileSetSize,
    KStblHttpFileRead,
    KStblHttpFileWrite,
    KStblHttpFileGetType,
    KStblHttpFileTimedRead,
    KStblHttpFileTimedWrite
#if SUPPORT_CHUNKED_READ
    , KStblHttpFileReadChunked
    , KStblHttpFileTimedReadChunked
#endif
};

static
rc_t KHttpFileMake(KStableHttpFile ** self,
    const char * url, va_list args)
{
    rc_t rc;
    KStableHttpFile * f = calloc(1, sizeof *f);
    if (f == NULL)
    {
        rc = RC(rcNS, rcFile, rcConstructing, rcMemory, rcExhausted);
    }
    else
    {
        rc = KFileInit(&f->dad, (const KFile_vt *)&vtKHttpFile, "KHttpFile", url, true, false);
        if (rc == 0)
        {
            rc = KDataBufferMakeBytes(&f->buf, 0);
            if (rc == 0)
            {
                rc = KDataBufferVPrintf(&f->buf, url, args);
                if (rc == 0)
                {
                    *self = f;
                    return 0;
                }
                KDataBufferWhack(&f->buf);
            }
        }
        free(f);
    }
    return rc;
}


/********************************* constructor ********************************/

static
rc_t KNSManagerVMakeHttpFileInt(const KNSManager *self,
    const KFile **file, struct KStream *conn, ver_t vers, bool reliable,
    bool need_env_token, bool payRequired, const char *url,
    va_list args)
{
    rc_t rc = 0;

    if (self != NULL && !self->retryFile)
        return KNSManagerVMakeHttpFileIntUnstable(self, file,
            conn, vers, reliable, need_env_token, payRequired, url, args);

    if (file == NULL)
        rc = RC(rcNS, rcFile, rcConstructing, rcParam, rcNull);
    else {
        if (self == NULL)
            rc = RC(rcNS, rcFile, rcConstructing, rcParam, rcNull);
        else if (url == NULL)
            rc = RC(rcNS, rcFile, rcConstructing, rcPath, rcNull);
        else if (url[0] == 0)
            rc = RC(rcNS, rcFile, rcConstructing, rcPath, rcInvalid);
        else {
            KStableHttpFile * f = NULL;
            rc = KHttpFileMake(&f, url, args);

            if (rc == 0) {
                rc = KNSManagerVMakeHttpFileIntUnstableFromBuffer(self,
                    &f->file, conn, vers, reliable, need_env_token, payRequired,
                    url, &f->buf);

                if (rc == 0) {
                    rc = KNSManagerAddRef(self);

                    if (rc == 0) {
                        f->mgr = self;

                        rc = KStreamAddRef(conn);
                        if (rc == 0) {
                            f->conn = conn;

                            f->vers = vers;
                            f->reliable = reliable;
                            f->need_env_token = need_env_token;
                            f->payRequired = payRequired;
                            f->url = string_dup_measure(url, NULL);

                            f->quitting = KNSManagerGetQuitting(self);

                            /* readWaitMillis and totalReadWaitMillis
                               ARE NEEDED BY HttpFileGetReadTimeouts() */
                            f->totalReadWaitMillis
                                = CalculateTotalReadWait(f, &f->readWaitMillis);

                            *file = &f->dad;
                        }
                    }
                }
            }
        }
    }

    return rc;
}


/******************************************************************************/


LIB_EXPORT rc_t CC KNSManagerMakeHttpFile(const KNSManager *self,
    const KFile **file, struct KStream *conn, ver_t vers,
    const char *url, ...)
{
    rc_t rc = 0;
    va_list args;
    va_start(args, url);
    rc = KNSManagerVMakeHttpFileInt(self, file, conn,
        vers, false, false, false, url, args);
    va_end(args);
    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMakeReliableHttpFile(const KNSManager *self,
    const KFile **file, struct KStream *conn, ver_t vers, bool reliable,
    bool need_env_token, bool payRequired, const char *url, ...)
{
    rc_t rc = 0;
    va_list args;
    va_start(args, url);
    rc = KNSManagerVMakeHttpFileInt(self, file, conn,
        vers, reliable, need_env_token, payRequired, url, args);
    va_end(args);
    return rc;
}

LIB_EXPORT bool CC KFileIsKHttpFile(const KFile * self)
{
    if (self != NULL && &self->vt->v1 == &vtKHttpFile)
        return true;
    else
        return KUnstableFileIsKHttpFile(self);
}
