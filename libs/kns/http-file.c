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

#define KFILE_IMPL KHttpFile
typedef struct KHttpFile KHttpFile;
#include <kfs/impl.h>

#include "http-priv.h"
#include "mgr-priv.h"
#include "stream-priv.h"

#include <kns/adapt.h>
#include <kns/endpoint.h>
#include <kns/http.h>
#include <kns/impl.h>
#include <kns/kns-mgr-priv.h> /* KHttpRetrier */
#include <kns/manager.h>
#include <kns/socket.h>
#include <kns/stream.h>

#include <kfs/file.h>
#include <kfs/directory.h>

#ifdef ERR
#undef ERR
#endif

#include <klib/container.h>
#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h>
#include <klib/out.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/refcount.h>
#include <klib/text.h>
#include <klib/time.h> /* KSleep */
#include <klib/vector.h>

#include <kproc/timeout.h>

#include <os-native.h>
#include <strtol.h>
#include <va_copy.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KHttpFile
 */
struct KHttpFile
{
    KFile dad;
    
    uint64_t file_size;

    KClientHttp *http;

    String url;
    KDataBuffer url_buffer;

    uint32_t maxNumberOfRetriesOnFailure;
    uint32_t testFailuresNumber;
    uint32_t logFailures;
};

static
rc_t CC KHttpFileDestroy ( KHttpFile *self )
{
    KClientHttpRelease ( self -> http );
    KDataBufferWhack ( & self -> url_buffer );
    free ( self );

    return 0;
}

static
struct KSysFile* CC KHttpFileGetSysFile ( const KHttpFile *self, uint64_t *offset )
{
    *offset = 0;
    return NULL;
}

static
rc_t CC KHttpFileRandomAccess ( const KHttpFile *self )
{
    /* TBD - not all HTTP servers will support this
       detect if the server does not, and alter the vTable */
    return 0;
}

/* KHttpFile must have a file size to be created
   impossible for this funciton to fail */
static
rc_t CC KHttpFileSize ( const KHttpFile *self, uint64_t *size )
{
    *size = self -> file_size;
    return 0;
}

static
rc_t CC KHttpFileSetSize ( KHttpFile *self, uint64_t size )
{
    return RC ( rcNS, rcFile, rcUpdating, rcFile, rcReadonly );
}

static
rc_t KHttpFileTimedReadInt ( const KHttpFile *cself,
    uint64_t aPos, void *aBuf, size_t aBsize,
    size_t *num_read, struct timeout_t *tm )
{
    uint64_t pos = aPos;
    rc_t rc;
    KHttpFile *self = ( KHttpFile * ) cself;
    KClientHttp *http = self -> http;

    /* starting position was beyond EOF */
    if ( pos >= self -> file_size )
    {
        *num_read = 0;
        return 0;
    }
#if 0
    /* position is within http header buffer */
    else if ( KClientHttpBlockBufferContainsPos ( http, pos ) )
    {

    }
#endif
    /* starting position was within file but the range fell beyond EOF */
    else 
    {
        KClientHttpRequest *req;

/* When we call KFileRead(HttpFile, pos > 0, bsize < 256)
  several times on the same socket,
  the server returns HTTP headers twice and no content: See VDB-1256, SYS-185053
*/
#define MIN_SZ 256
        char buf[MIN_SZ] = "";
        void *bPtr = aBuf;
        size_t bsize = aBsize;

        /* extend buffer size to MIN_SZ */
        if (bsize < MIN_SZ) {
            bPtr = buf;
            bsize = MIN_SZ;
        }

        /* limit request to file size */
        if ( pos + bsize > self -> file_size ) {
            bsize = self -> file_size - pos;
            if (bsize < MIN_SZ) {
                size_t d = MIN_SZ - bsize;
                if (pos >= d) {
                    bsize += d;
                    pos -= d;
                }
                else { /* TODO: Downloading file with size < 256:
need to reopen the connection now;
otherwise we are going to hit "Apache return HTTP headers twice" bug */
                    bsize += pos;
                    pos = 0;
                }
            }
        }

        assert(bsize >= MIN_SZ || (pos == 0 && bsize == self -> file_size));

        rc = KClientHttpMakeRequest ( http, &req, self -> url_buffer . base );
        if ( rc == 0 )
        {
            /* request min ( bsize, file_size ) bytes */
            rc = KClientHttpRequestByteRange ( req, pos, bsize );
            if ( rc == 0 )
            {
                KClientHttpResult *rslt;
                
                rc = KClientHttpRequestGET ( req, &rslt );
                if ( rc == 0 )
                {
                    uint32_t code;
                    
                    /* dont need to know what the response message was */
                    rc = KClientHttpResultStatus ( rslt, &code, NULL, 0, NULL );
                    if ( rc == 0 )
                    {
                        switch ( code )
                        {
                        case 206:
                        {
                            uint64_t start_pos;
                            size_t result_size;

                            /* extract actual amount being returned by server */
                            rc = KClientHttpResultRange ( rslt, &start_pos, &result_size );
                            if ( rc == 0 && 
                                 start_pos == pos &&
                                 result_size == bsize )
                            {
                                KStream *response;
                                
                                rc = KClientHttpResultGetInputStream ( rslt, &response );
                                if ( rc == 0 )
                                {
                                    size_t skip = 0;

                                    rc = KStreamTimedReadExactly(
                                         response, bPtr, result_size, tm);
                                    if ( rc != 0 )
                                    {
                                        KClientHttpClose ( http );
                                        return ResetRCContext ( rc, rcNS, rcFile, rcReading );
                                    }

                                    if (pos != aPos) {
                                        assert(pos < aPos);
                                        skip = aPos - pos;
                                        assert(result_size >= skip);
                                        result_size -= skip;
                                    }

                                    if (result_size > aBsize) {
                                        result_size = aBsize;
                                    }

                                    if (bPtr == buf) {
                                        memcpy(aBuf, buf + skip, result_size);
                                    }
                                    else if (skip > 0) {
                                        const void *src
                                            = (const char *)aBuf + skip;
                                        memmove(aBuf, src, result_size);
                                    }

                                    * num_read = result_size;

                                    KStreamRelease ( response );
                                }
                            }
                            break;
                        }
                        case 416:
                        default:
                            rc = RC ( rcNS, rcFile, rcReading, rcFileDesc, rcInvalid );
                        }
                    }
                    KClientHttpResultRelease ( rslt );
                }
            }
            KClientHttpRequestRelease ( req );
        }
    }

    return rc;
}

void KHttpRetrierInit(KHttpRetrier *self, uint32_t maxRetryNumber,
    uint32_t testFailuresNumber, const char *url, uint32_t logNumber)
{
    assert(self);

    memset(self, 0, sizeof *self);

    self->maxRetriesNumber = maxRetryNumber;
    self->testFailuresNumber = testFailuresNumber;
    self->url = url;

    self->logNumber = logNumber;
}

static bool KHttpRetrierDone(const KHttpRetrier *self) {
    return self->triesNumber > self->maxRetriesNumber;
}

static rc_t KHttpRetrierReport(KHttpRetrier *self, rc_t rc) {
    const char *dummy = "";
    const char *url = dummy;
    assert(self);
    if (self->url != NULL) {
        url = self->url;
    }
    if (self->triesNumber < self->logNumber ||
        (self->logged && self->logNumber > 0))
    {
        return 0;
    }
    if (self->reportedTime == self->waitTime) {
/*      return 0; */
    }
    self->logged = true;
    self->reportedTime = self->waitTime;
    if (self->logNumber == 0) {
        return PLOGERR(klogSys, (klogSys, rc,
            "HTTP read failure: tried $(c)/$(m) times for $(t) seconds: $(u)",
            "c=%d,m=%d,t=%d,u=%s", self->triesNumber,
            self->maxRetriesNumber + 1, self->waitTime, url));
    }
    else {
        return
            LOGERR(klogSys, rc, "HTTP read failure: retrying...");
    }
}

bool KHttpRetrierWait(KHttpRetrier *self, rc_t rc) {
    uint32_t seconds = 0;
    assert(self);
    ++self->triesNumber;
    if (KHttpRetrierDone(self)) {
        KHttpRetrierReport(self, rc);
        return false;
    }
    switch (self->triesNumber) {
        case 1:
            seconds = 0;
            break;
        case 2:
            seconds = 5;
            break;
        case 3:
            seconds = 10;
            break;
        case 4:
            seconds = 15;
            break;
        case 5:
            seconds = 30;
            break;
        default:
            seconds = 60;
            break;
    }
    KHttpRetrierReport(self, rc);
    if (seconds > 0) {
        if (seconds > 0) {
        }
        KSleep(seconds);
        self->waitTime += seconds;
    }
    return true;
}

rc_t KHttpRetrierForceFailure(const KHttpRetrier *self, const KHttpFile *socket)
{
    assert(self);
    if (self->triesNumber + 1 <= self->testFailuresNumber) {
        if (socket != NULL && socket->http != NULL) {
            KClientHttpForceSocketClose(socket->http);
        }
        else {
            return RC(rcNS, rcFile, rcAccessing, rcError, rcUnknown);
        }
    }
    return 0;
}

static
rc_t CC KHttpFileTimedRead ( const KHttpFile *self,
    uint64_t pos, void *buffer, size_t bsize,
    size_t *num_read, struct timeout_t *tm )
{
    /* THIS COULD BE A PLACE TO INSERT UNDYING READ */
    URLBlock block;
    KHttpRetrier retrier;
    KHttpRetrierInit(&retrier, self->maxNumberOfRetriesOnFailure,
        self->testFailuresNumber, NULL, self->logFailures);
    memset(&block, 0, sizeof block);

    /* loop using existing KClientHttp object */
    while (true) {
        rc_t rc = KHttpRetrierForceFailure(&retrier, self);
        if (rc == 0) {
            rc = KHttpFileTimedReadInt(self,
                pos, buffer, bsize, num_read, tm);
        }
        if (rc == 0) {
            return rc;
        }
        if (!KHttpRetrierWait(&retrier, rc)) {
            return rc;
        }
        /* release KClientHttp object, and recreate */
        KClientHttpClose(self->http);
        if (block.host.addr == NULL) {
            rc = ParseUrl(&block,
                self->url_buffer.base, self->url_buffer.elem_count - 1);
            if (rc != 0) {
                return rc;
            }
        }
        while (true) {
            rc = KClientHttpOpen(self->http, &block.host, block.port);
            if (rc == 0) {
                break;
            }
            if (!KHttpRetrierWait(&retrier, rc)) {
                return rc;
            }
        }
    }
}

static
rc_t CC KHttpFileRead ( const KHttpFile *self, uint64_t pos,
     void *buffer, size_t bsize, size_t *num_read )
{
    return KHttpFileTimedRead ( self, pos, buffer, bsize, num_read, NULL );
}

static
rc_t CC KHttpFileWrite ( KHttpFile *self, uint64_t pos, 
    const void *buffer, size_t size, size_t *num_writ )
{
    return RC ( rcNS, rcFile, rcUpdating, rcInterface, rcUnsupported );
}

static
rc_t CC KHttpFileTimedWrite ( KHttpFile *self, uint64_t pos, 
    const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm )
{
    return RC ( rcNS, rcFile, rcUpdating, rcInterface, rcUnsupported );
}

static
uint32_t CC KHttpFileGetType ( const KHttpFile *self )
{
    assert ( self != NULL );

    /* the HTTP file behaves like a read-only file
       returning kfdSocket would be imply absence of
       random access: the HTTP protocol adds that. */

    return kfdFile;
}

static KFile_vt_v1 vtKHttpFile = 
{
    1, 2,

    KHttpFileDestroy,
    KHttpFileGetSysFile,
    KHttpFileRandomAccess,
    KHttpFileSize,
    KHttpFileSetSize,
    KHttpFileRead,
    KHttpFileWrite,
    KHttpFileGetType,
    KHttpFileTimedRead,
    KHttpFileTimedWrite
};

static rc_t KNSManagerVMakeHttpFileInt ( const KNSManager *self,
    const KFile **file, KStream *conn, ver_t vers, bool reliable,
    const char *url, va_list args )
{
    rc_t rc;

    if ( file == NULL )
        rc = RC ( rcNS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC( rcNS, rcNoTarg, rcConstructing, rcParam, rcNull );
        else if ( url == NULL )
            rc = RC ( rcNS, rcFile, rcConstructing, rcPath, rcNull );
        else if ( url [ 0 ] == 0 )
            rc = RC ( rcNS, rcFile, rcConstructing, rcPath, rcInvalid );
        else
        {
            KHttpFile *f;

            f = calloc ( 1, sizeof *f );
            if ( f == NULL )
                rc = RC ( rcNS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KFileInit ( &f -> dad, ( const KFile_vt * ) &vtKHttpFile, "KHttpFile", url, true, false );
                if ( rc == 0 )
                {
                  KHttpRetrier retrier;
                  uint32_t retries = reliable
                    ? self->maxNumberOfRetriesOnFailureForReliableURLs
                    : self->maxNumberOfRetriesOnFailure;
                  f->maxNumberOfRetriesOnFailure = retries;
                  f->testFailuresNumber = self->testFailuresNumber;
                  f->logFailures = self->logFailures;
                  KHttpRetrierInit(&retrier, f->maxNumberOfRetriesOnFailure,
                      f->testFailuresNumber, url, f->logFailures);
                  {

                    KDataBuffer *buf = & f -> url_buffer;
                    buf -> elem_bits = 8;
                    rc = KDataBufferVPrintf ( buf, url, args );
                    if ( rc == 0 )
                    {
                        URLBlock block;
                        rc = ParseUrl ( &block, buf -> base, buf -> elem_count - 1 );
                        if ( rc == 0 ) {
                          while (true) {
                            KClientHttp *http;
                          
                            rc = KNSManagerMakeClientHttpInt ( self, & http, buf, conn, vers,
                                self -> http_read_timeout, self -> http_write_timeout, &block . host, block . port );
                            if ( rc == 0 )
                            {
                                KClientHttpRequest *req;

                                rc = KClientHttpMakeRequestInt ( http, &req, &block, buf );
                                if ( rc == 0 )
                                {
                                    KClientHttpResult *rslt;
                                  
                                    rc = KHttpRetrierForceFailure(&retrier, f);
                                    if (rc == 0) {
                                        rc = KClientHttpRequestHEAD(req, &rslt);
                                    }
                                    KClientHttpRequestRelease ( req );

                                    if ( rc == 0 )
                                    {
                                        uint64_t size;

                                        bool have_size = false;
                                        have_size = KClientHttpResultSize
                                            ( rslt, &size );
                                        KClientHttpResultRelease ( rslt );

                                        if ( ! have_size )
                                            rc = RC ( rcNS, rcFile, rcValidating, rcNoObj, rcError );
                                        else
                                        {
                                            f -> file_size = size;
                                            f -> http = http;

                                            * file = & f -> dad;

                                            return 0;
                                        }
                                    }
                                }

                                KClientHttpRelease ( http );
                            }
                            if (!KHttpRetrierWait(&retrier, rc)) {
                                break;
                            }
                          }
                        }
                    }
                    KDataBufferWhack ( buf );
                  }
                }
                free ( f );
            }
        }

        * file = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMakeHttpFile(const KNSManager *self,
    const KFile **file, struct KStream *conn, ver_t vers, const char *url, ...)
{
    rc_t rc = 0;
    va_list args;
    va_start(args, url);
    rc = KNSManagerVMakeHttpFileInt ( self,
        file, conn, vers, false, url, args);
    va_end(args);
    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMakeReliableHttpFile(const KNSManager *self,
    const KFile **file, struct KStream *conn, ver_t vers, const char *url, ...)
{
    rc_t rc = 0;
    va_list args;
    va_start(args, url);
    rc = KNSManagerVMakeHttpFileInt ( self,
        file, conn, vers, true, url, args);
    va_end(args);
    return rc;
}
