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

#include "http-file-priv.h"

#include "http-priv.h"
#include "mgr-priv.h"
#include "stream-priv.h"

#include <kns/adapt.h>
#include <kns/endpoint.h>
#include <kns/impl.h>
#include <kns/kns-mgr-priv.h> /* KHttpRetrier */
#include <kns/socket.h>
#include <kns/stream.h>
#include <kns/http-priv.h>

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

#define SUPPORT_CHUNKED_READ 1

#if SUPPORT_CHUNKED_READ
#include <kfs/chunk-reader.h>
#endif

#if _DEBUGGING && 0
#include <stdio.h>
#define TRACE( x, ... ) \
    fprintf ( stderr, "@@ %s:%d: %s: " x, __FILE__, __LINE__, __func__, __VA_ARGS__ )
#else
#define TRACE( x, ... ) \
    ( ( void ) 0 )
#endif

#define USE_CACHE_CONTROL 1
#define NO_CACHE_LIMIT ( ( uint64_t ) ( 16 * 1024 * 1024 ) )

/* VDB-4146: Use Content-Length from HEAD in GET
   when GET doesn't return Content-Length and Range was not requested. */
#define USE_SIZE_FROM_HEAD

static
rc_t CC KHttpFileDestroy ( KHttpFile *self )
{
    KLockRelease ( self -> lock );
    KNSManagerRelease ( self -> kns );
    KClientHttpRelease ( self -> http );
    KDataBufferWhack ( & self -> url_buffer );
    KDataBufferWhack ( & self -> orig_url_buffer );
    free ( self );

    return 0;
}

static
struct KSysFile* CC KHttpFileGetSysFile ( const KHttpFile *self, uint64_t *offset )
{
    * offset = 0;
    return NULL;
}

static
rc_t CC KHttpFileRandomAccess ( const KHttpFile *self )
{
    /* we ensure during construction that the server accepts partial range requests */
    return 0;
}

/* KHttpFile must have a file size to be created
   impossible for this funciton to fail */
static
rc_t CC KHttpFileSize ( const KHttpFile *self, uint64_t *size )
{
    * size = self -> file_size;
    return 0;
}

static
rc_t CC KHttpFileSetSize ( KHttpFile *self, uint64_t size )
{
    return RC ( rcNS, rcFile, rcUpdating, rcFile, rcReadonly );
}

#if SUPPORT_CHUNKED_READ
static
rc_t KHttpFileMakeRequest ( const KHttpFile *cself, uint64_t pos, size_t req_size,
    struct timeout_t *tm, KClientHttpResult **rslt, uint32_t * http_status )
{
    rc_t rc;
    KClientHttpRequest *req;

    * rslt = NULL;
    * http_status = 0;

    if ( cself -> url_is_temporary )
    {   /* check for expiration of the URL */
        /* KTime_t is in seconds */
        const KTime_t now = KTimeStamp();
        const KTime_t expTime = KTimeMakeTime ( & cself -> url_expiration );
        const KTime_t advance = 60;
        if ( expTime < ( now + advance ) ) /* expires with 'advance' seconds */
        {
            // replace current URL with the original URL and expect to be redirected to a new temporary URL
            KHttpFile * self = (KHttpFile *) cself;
            KDataBufferWhack ( & self -> url_buffer );
            rc = KDataBufferSub ( & self -> orig_url_buffer,
                                  & self -> url_buffer,
                                  0,
                                  KDataBufferBytes( & self -> orig_url_buffer ) );
            if ( rc != 0 )
            {
                TRACE ( "KDataBufferSub failed: rc=%u\n", rc );
                return rc;
            }
            self -> url_is_temporary = false;
        }
    }

    rc = KClientHttpMakeRequest ( cself -> http, &req, "%s", cself -> url_buffer . base );
    if ( rc != 0 )
    {
        TRACE ( "KClientHttpMakeRequest ( http, & req, url=\"%s\" ); failed: rc=%u\n",
                ( const char* ) cself -> url_buffer . base, rc );
    }
    else
    {
#if USE_CACHE_CONTROL
            /* tell proxies not to cache if file is above limit */
        if ( cself -> no_cache )
            rc = KClientHttpRequestSetNoCache ( req );
        if ( rc == 0 )
#endif
        {
            /* request range unless whole file */
            if ( pos != 0 || ( uint64_t ) req_size < cself -> file_size )
            {
                rc = KClientHttpRequestByteRange ( req, pos, req_size );
                if ( rc != 0 )
                {
                    TRACE ( "KClientHttpRequestByteRange ( req, pos=%lu, bsize=%lu ); failed: rc=%u\n",
                            pos, req_size, rc );
                }
                else
                    req->rangeRequested = true;
            }

            if ( rc == 0 )
            {
                KClientHttpRequestSetCloudParams(req, cself -> need_env_token, cself -> payRequired);
                if ( cself -> need_env_token && ! cself -> url_is_temporary)
                {
                    KClientHttpRequestSetCloudParams(req, true, cself -> payRequired);
                    KClientHttpRequestAttachEnvironmentToken ( req );
                    /* TBD - there should be a version of POST that takes a timeout */
                    rc = KClientHttpRequestPOST ( req, rslt );
                }
                else
                {
                    /* TBD - there should be a version of GET that takes a timeout */
                    rc = KClientHttpRequestGET ( req, rslt );
                }

                if ( rc != 0 )
                    TRACE ( "KClientHttpRequestGET ( req, & rslt ); failed: rc=%u\n", rc );
                else
                {
                    /* dont need to know what the response message was */
                    rc = KClientHttpResultStatus ( * rslt, http_status, NULL, 0, NULL );
                    if ( rc != 0 )
                    {
                        TRACE ( "KClientHttpResultStatus ( rslt, & http_status, NULL, 0, NULL );"
                                " failed: rc=%u\n", rc );

                        KClientHttpResultRelease ( * rslt );
                        * rslt = NULL;
                    }
                    else
                    {
                        KHttpFile *self = (KHttpFile *)cself;
                        if ( (*rslt) -> expiration != NULL )
                        {   /* retrieve and save the URL expiration time */
                            self -> url_is_temporary = true;
                            KTimeFromIso8601 ( & self -> url_expiration, (*rslt) -> expiration, string_size ( (*rslt) -> expiration ) );
                        }

                        /* update url_buffer with the (possibly different and/or temporary) URL*/
                        KClientHttpRequestURL ( req, & self -> url_buffer );
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ),
                            ( "HttpFile.URL updated to '%.*s'\n",
                                ( int ) self -> url_buffer . elem_count, self -> url_buffer . base ) );
                    }
                }
            }
        }

        if (rc == 0 && rslt != NULL && *rslt != NULL)
            (*rslt)->rangeRequested = req->rangeRequested;
        KClientHttpRequestRelease ( req );
    }
    return rc;
}
#endif /* SUPPORT_CHUNKED_READ */

#if SUPPORT_CHUNKED_READ
static
rc_t KHttpFileReadResponse ( KStream * response,
    void * buf, size_t bsize, size_t * num_read,
    struct timeout_t * tm )
{
    rc_t rc = KStreamTimedReadExactly ( response, buf, bsize, tm );

    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcNS, rcFile, rcReading );
    else
        * num_read = bsize;

    return rc;
}
#endif /* SUPPORT_CHUNKED_READ */

#if SUPPORT_CHUNKED_READ
static
rc_t KHttpFileTimedReadInt ( const KHttpFile * self,
    uint64_t pos, void * buf, size_t bsize, size_t *num_read,
    struct timeout_t *tm, uint32_t * http_status )
{
    rc_t rc = 0;
    uint32_t proxy_retries;

    size_t req_size = bsize;

    if ( tm != 0 )
    {
        rc = TimeoutPrepare ( tm );
         if ( rc != 0 )
         {
             return false;
         }
    }

    * http_status = 0;

    /* limit request size to EOF */
    assert ( pos < self -> file_size );
    if ( pos + bsize > self -> file_size )
        req_size = ( size_t ) ( self -> file_size - pos );

    /* try for a number of times to issue a request and get result */
    for ( proxy_retries = 5; rc == 0 && proxy_retries != 0; )
    {
        KClientHttpResult * rslt = NULL;

        rc = KHttpFileMakeRequest ( self, pos, req_size, tm, & rslt, http_status );
        if ( rc == 0 )
        {
            bool have_size;
            uint64_t start_pos;
            size_t result_size;
            uint64_t result_size64;

            switch ( * http_status )
            {

            case 200:
                proxy_retries = 0;

                /* extract stated bytes returned - must be whole file */
                have_size = KClientHttpResultSize ( rslt, & result_size64 );
#ifdef USE_SIZE_FROM_HEAD
                if (!have_size && !rslt->rangeRequested) {
                    result_size64 = self->file_size;
                    have_size = true;
                }
#endif
                result_size = ( size_t ) result_size64;
                if ( pos != 0 || ! have_size || result_size > bsize )
                {
                    rc = RC ( rcNS, rcFile, rcReading, rcData, rcUnexpected );
                    TRACE ( "KClientHttpResultSize ( rslt, & result_size ); unexpected status=%d\n",
                            * http_status );
                }
                else
                {
                    KStream * response;

                    /* assume we are reading the entire file */
                    assert ( ( uint64_t ) result_size == self -> file_size );
                    rc = KClientHttpResultGetInputStream ( rslt, & response );
                    if ( rc == 0 )
                    {
                        rc = KHttpFileReadResponse ( response, buf, req_size, num_read, tm );
                        KStreamRelease ( response );
                    }
                }
                break;

            case 206:
                proxy_retries = 0;

                /* extract actual amount being returned by server */
                rc = KClientHttpResultRange ( rslt, & start_pos, & result_size );
                if ( rc != 0 )
                {
                    TRACE ( "KClientHttpResultRange ( rslt, & start_pos, & result_size ); "
                            "failed: rc=%u\n", rc );
                }
                else if ( start_pos != pos )
                {
                    TRACE ( "KClientHttpResultRange ( rslt, & start_pos, & result_size ); "
                            "failed: start_pos=%lu != pos=%lu\n", start_pos, pos );
                    rc = RC ( rcNS, rcFile, rcReading, rcData, rcUnexpected );
                }
                else
                {
                    KStream * response;

                    if ( result_size != bsize )
                    {
                        TRACE ( "KClientHttpResultRange ( rslt, & start_pos, & result_size ); "
                                "short read: result_size=%lu != bsize=%lu\n", result_size, bsize );
                        if ( result_size > bsize )
                        {
                            result_size = bsize;
                        }
                    }

                    rc = KClientHttpResultGetInputStream ( rslt, & response );
                    if ( rc == 0 )
                    {
                        rc = KHttpFileReadResponse ( response, buf, result_size, num_read, tm );
                        KStreamRelease ( response );
                    }
                }
                break;

            case 403:
            case 404:
                if ( -- proxy_retries != 0 )
                {
                    TRACE ( "KClientHttpResultStatus ( rslt, & http_status, NULL, 0, NULL ); "
                            "unexpected status=%d - sleeping and retrying\n", * http_status );
                    KSleep ( 1 );
                    rc = 0;
                    break;
                }

                /* NO BREAK */

            default:
                rc = RC ( rcNS, rcFile, rcReading, rcData, rcUnexpected );
                TRACE ( "KClientHttpResultStatus ( rslt, & http_status, NULL, 0, NULL ); "
                        "unexpected status=%d\n", * http_status );
                break;
            }

            KClientHttpResultRelease ( rslt );
        }
    }

    if ( rc != 0 || * num_read == 0 )
        KClientHttpClose ( self -> http );

    return rc;
}
#endif /* SUPPORT_CHUNKED_READ */

#if ! SUPPORT_CHUNKED_READ

static
rc_t KHttpFileTimedReadInt ( const KHttpFile *cself,
    uint64_t aPos, void *aBuf, size_t aBsize,
    size_t *num_read, struct timeout_t *tm, uint32_t * http_status )
{
    uint64_t pos = aPos;
    rc_t rc = 0;
    KHttpFile *self = ( KHttpFile * ) cself;
    KClientHttp *http = self -> http;

    * http_status = 0;

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
        char buf [ 256 ] = "";
        void *bPtr = aBuf;
        size_t bsize = aBsize;
        uint32_t proxy_retries;

        rc = CloudRefresh( self, tm );
        if ( rc != 0 )
        {
            return rc;
        }

        /* extend buffer size to MIN_SZ */
        if ( bsize < sizeof buf )
        {
            bPtr = buf;
            bsize = sizeof buf;
        }

        /* limit request to file size */
        if ( pos + bsize > self -> file_size )
        {
            bsize = ( size_t ) ( self -> file_size - pos );
            if (bsize < sizeof buf)
            {
                size_t d = sizeof buf - bsize;
                if (pos >= d)
                {
                    bsize += d;
                    pos -= d;
                }
                else { /* TODO: Downloading file with size < 256:
need to reopen the connection now;
otherwise we are going to hit "Apache return HTTP headers twice" bug */
                    bsize += ( size_t ) pos;
                    pos = 0;
                }
            }
        }

        assert(bsize >= sizeof buf || (pos == 0 && bsize == self -> file_size));

        for ( proxy_retries = 5; rc == 0 && proxy_retries != 0; )
        {
            rc = KClientHttpMakeRequest ( http, &req, self -> url_buffer . base );
            if ( rc != 0 )
            {
                TRACE ( "KClientHttpMakeRequest ( http, & req, url=\"%s\" ); failed: rc=%u\n", ( const char* ) self -> url_buffer . base, rc );
                break;
            }

#if USE_CACHE_CONTROL
            /* tell proxies not to cache if file is above limit */
            if ( rc == 0 && self -> no_cache )
                rc = KClientHttpRequestSetNoCache ( req );
#endif
            if ( rc == 0 )
            {
                /* request min ( bsize, file_size ) bytes */
                rc = KClientHttpRequestByteRange ( req, pos, bsize );
                if ( rc != 0 )
                    TRACE ( "KClientHttpRequestByteRange ( req, pos=%lu, bsize=%lu ); failed: rc=%u\n", pos, bsize, rc );
                else
                {
                    KClientHttpResult *rslt;

                    rc = KClientHttpRequestGET ( req, &rslt );
                    if ( rc != 0 )
                        TRACE ( "KClientHttpRequestGET ( req, & rslt ); failed: rc=%u\n", rc );
                    else
                    {
                        /* dont need to know what the response message was */
                        rc = KClientHttpResultStatus ( rslt, http_status, NULL, 0, NULL );
                        if ( rc != 0 )
                            TRACE ( "KClientHttpResultStatus ( rslt, & http_status, NULL, 0, NULL ); failed: rc=%u\n", rc );
                        else
                        {
                            uint64_t start_pos = 0;
                            size_t result_size = 0;
                            bool have_size = false;
                            switch ( * http_status )
                            {
                            case 200:
                               /* We requested Bytes Range but got 200:
                                  accept it what the whole file was requested */
                                have_size = KClientHttpResultSize ( rslt,
                                                    &result_size );
                                if ( pos != 0 || ! have_size
                                              || result_size > bsize )
                                {
                                    rc = RC ( rcNS, rcFile, rcReading,
                                                    rcData, rcUnexpected );
                                    TRACE ( "KClientHttpResultStatus ( rslt, "
"& http_status, NULL, 0, NULL ); unexpected status=%d\n", * http_status );
                                    break;
                                }
                          /* no break here, now read the file */
                            case 206:
                                /* don't need retries now */
                                proxy_retries = 0;

                                /* extract actual amount being returned by server */
                                if ( * http_status == 206 ) {
                                    /* get result range when 206 was returned,
                                       we got it already when status == 200 */
                                    rc = KClientHttpResultRange ( rslt,
                                        &start_pos, &result_size );
                                    if ( rc != 0 || start_pos != pos
                                                 || result_size != bsize )
                                    {
                                        if ( rc != 0 )
                                            TRACE ( "KClientHttpResultRange ( rslt, & start_pos, & result_size ); failed: rc=%u\n", rc );
                                        else
                                        {
                                            if ( start_pos != pos )
                                                TRACE ( "KClientHttpResultRange ( rslt, & start_pos, & result_size ); failed: start_pos=%lu != pos=%lu\n", start_pos, pos );
                                            if ( result_size != bsize )
                                                TRACE ( "KClientHttpResultRange ( rslt, & start_pos, & result_size ); failed: result_size=%lu != bsize=%lu\n", result_size, bsize );
                                        }
                                        break;
                                    }
                                }
                                {
                                  /* read the response for partial file requests
                                     or when the whole file was returned */

                                    KStream *response;

                                    rc = KClientHttpResultGetInputStream ( rslt, &response );
                                    if ( rc == 0 )
                                    {
                                        size_t skip = 0;

                                        rc = KStreamTimedReadExactly( response, bPtr, result_size, tm );
                                        if ( rc != 0 )
                                        {
                                            KStreamRelease ( response );
                                            KClientHttpResultRelease ( rslt );
                                            KClientHttpRequestRelease ( req );
                                            KClientHttpClose ( http );
                                            return ResetRCContext ( rc, rcNS, rcFile, rcReading );
                                        }

                                        if (pos != aPos)
                                        {
                                            assert(pos < aPos);
                                            skip = ( size_t ) ( aPos - pos );
                                            assert(result_size >= skip);
                                            result_size -= skip;
                                        }

                                        if (result_size > aBsize)
                                            result_size = aBsize;

                                        if (bPtr == buf)
                                            memmove(aBuf, buf + skip, result_size);
                                        else if (skip > 0)
                                        {
                                            const void *src = ( const char * ) aBuf + skip;
                                            memmove(aBuf, src, result_size);
                                        }

                                        * num_read = result_size;

                                        KStreamRelease ( response );
                                    }
                                }
                                break;

                            case 403:
                            case 404:
                                if ( -- proxy_retries != 0 )
                                {
                                    TRACE ( "KClientHttpResultStatus ( rslt, & http_status, NULL, 0, NULL ); unexpected status=%d - sleeping and retrying\n", * http_status );
                                    KSleep ( 1 );
                                    rc = 0;
                                    break;
                                }

                                /* NO BREAK */

                            default:
                                rc = RC ( rcNS, rcFile, rcReading, rcData, rcUnexpected );
                                TRACE ( "KClientHttpResultStatus ( rslt, & http_status, NULL, 0, NULL ); unexpected status=%d\n", * http_status );
                                break;
                            }
                        }

                        KClientHttpResultRelease ( rslt );
                    }
                }

                KClientHttpRequestRelease ( req );
            }
        }
    }

    if ( rc != 0 || * num_read == 0 )
        KClientHttpClose ( http );

    return rc;
}
#endif /* ! SUPPORT_CHUNKED_READ */

#if SUPPORT_CHUNKED_READ
static
rc_t KHttpFileTimedReadShort ( const KHttpFile * self,
    uint64_t pos, void * buf, size_t bsize, size_t * num_read,
    struct timeout_t * tm, uint32_t * http_status )
{
    rc_t rc = 0;
    uint8_t min_read_buffer [ 256 ];

    /* if the whole file fits into 256 bytes */
    if ( self -> file_size <= sizeof min_read_buffer )
    {
        /* read into local buffer */
        rc = KHttpFileTimedReadInt ( self, 0, min_read_buffer,
            sizeof min_read_buffer, num_read, tm, http_status );

        /* transfer any bytes read up to bsize */
        if ( * num_read != 0 )
        {
            if ( * num_read > bsize )
                * num_read = bsize;

            /* move bytes out into supplied buffer.
               because we know that file size <= 256,
               and we know that pos < file_size, we
               and we know that pos + bsize <= file_size,
               the following assertions must hold. */
            assert ( pos < sizeof min_read_buffer );
            assert ( pos + * num_read <= sizeof min_read_buffer );
            memmove ( buf, & min_read_buffer [ pos ], * num_read );
        }
    }

    /* if the read is at the end of file, it could be short
       just because of buffer-size modulus. */
    else if ( pos + bsize == self -> file_size )
    {
        /* this is the pos offset to produce a full 256-byte read */
        size_t d = sizeof min_read_buffer - bsize;

        /* read last 256 bytes from file.
           because we know that bsize < 256,
           and we know that file_size > 256,
           we know that sizeof buffer - bsize <= file_size - pos
           and the following assertions must hold. */
        assert ( pos >= d );
        rc = KHttpFileTimedReadInt ( self, pos - d, min_read_buffer,
            sizeof min_read_buffer, num_read, tm, http_status );

        /* transfer any bytes read up to bsize */
        if ( * num_read != 0 )
        {
            if ( * num_read > bsize )
                * num_read = bsize;

            assert ( d + * num_read <= sizeof min_read_buffer );
            memmove ( buf, & min_read_buffer [ d ], * num_read );
        }

    }

    /* this appears to be a short read of a partial file,
       but not at the end. */
    else
    {
        /* try to read 256 bytes at stated position */
        rc = KHttpFileTimedReadInt ( self, pos, min_read_buffer,
            sizeof min_read_buffer, num_read, tm, http_status );

        /* transfer any bytes read up to bsize */
        if ( * num_read != 0 )
        {
            if ( * num_read > bsize )
                * num_read = bsize;

            memmove ( buf, min_read_buffer, * num_read );
        }
    }

    return rc;
}
#endif

static
rc_t KHttpFileTimedReadLocked ( const KHttpFile * self,
    uint64_t pos, void * buf, size_t bsize, size_t * num_read,
    struct timeout_t * tm, uint32_t * http_status )
{
    rc_t rc = KLockAcquire ( self -> lock );
    if ( rc == 0 )
    {
#if SUPPORT_CHUNKED_READ
        /* moved the request boundary processing here.
           first, check trivial case of read beyond EOF. */
        if ( pos >= self -> file_size )
            * num_read = 0;
        else
        {
            /* limit read request to amount available in file */
            if ( pos + bsize > self -> file_size )
                bsize = ( size_t ) ( self -> file_size - pos );

            /* if there are at least 256 bytes between pos and EOF,
               just go ahead and read it directly into this buffer. */
            if ( bsize >= 256 )
                rc = KHttpFileTimedReadInt ( self, pos, buf, bsize, num_read, tm, http_status );

            /* otherwise, perform a short read */
            else
                rc = KHttpFileTimedReadShort ( self, pos, buf, bsize, num_read, tm, http_status );
        }

#else
        rc = KHttpFileTimedReadInt ( self, pos, buf, bsize, num_read, tm, http_status );
#endif
        KLockUnlock ( self -> lock );
    }
    return rc;
}

static
rc_t CC KHttpFileTimedRead ( const KHttpFile *self,
    uint64_t pos, void *buffer, size_t bsize,
    size_t *num_read, struct timeout_t *tm )
{
    KHttpRetrier retrier;
    rc_t rc = KHttpRetrierInit ( & retrier, self -> url_buffer . base, self -> kns );
    if ( rc == 0 )
    {
        rc_t rc2;

        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ),
            ( "KHttpFileTimedRead(pos=%lu,size=%zu)...\n", pos, bsize ) );

        /* loop using existing KClientHttp object */
        while ( rc == 0 )
        {
            uint32_t http_status;
            rc = KHttpFileTimedReadLocked ( self, pos, buffer, bsize, num_read, tm, & http_status );
            if ( rc != 0 )
            {
                rc_t rc2=KClientHttpReopen ( self -> http );
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ), ( "KHttpFileTimedRead: KHttpFileTimedReadLocked failed, reopening\n" ) );
                if ( rc2 == 0 )
                {
                    rc2 = KHttpFileTimedReadLocked ( self, pos, buffer, bsize, num_read, tm, & http_status );
                    if ( rc2 == 0 )
                    {
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ), ( "KHttpFileTimedRead: reopened successfully\n" ) );
                        rc= 0;
                    }
                    else
                    {
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ), ( "KHttpFileTimedRead: reopen failed\n" ) );
                        break;
                    }
                }
            }
            if ( ! KHttpRetrierWait ( & retrier, http_status ) )
            {
                assert ( num_read );
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ),
                    ( "...KHttpFileTimedRead(pos=%lu,size=%zu)=%zu\n\n",
                      pos, bsize, * num_read ) );
                break;
            }
            rc = KClientHttpReopen ( self -> http );
        }

        rc2 = KHttpRetrierDestroy ( & retrier );
        if ( rc == 0 )
            rc = rc2;
    }

    if ( rc != 0 && KNSManagerLogNcbiVdbNetError ( self -> kns ) )
    {
        KEndPoint ep, local_ep;
        KClientHttpGetLocalEndpoint  ( self -> http, & local_ep );
        KClientHttpGetRemoteEndpoint ( self -> http, & ep );

        PLOGERR ( klogErr, ( klogErr, rc,
            "Failed to KHttpFileTimedRead("
            "'$(path)' ($(ip)), $(bytes)) from '$(local)'",
               "path=%s,ip=%s,bytes=%zu,local=%s",
            self -> url_buffer . base,
            ep . ip_address, bsize, local_ep . ip_address ) );
    }

    return rc;
}

static
rc_t CC KHttpFileRead ( const KHttpFile *self, uint64_t pos,
     void *buffer, size_t bsize, size_t *num_read )
{
    struct timeout_t tm;
    TimeoutInit ( & tm, self -> kns -> http_read_timeout );

    return KHttpFileTimedRead ( self, pos, buffer, bsize, num_read, & tm );
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

#if SUPPORT_CHUNKED_READ
static
rc_t KHttpFileReadResponseInChunks ( KStream * response,
    uint64_t pos, KChunkReader * chunks, size_t bsize, size_t * num_read,
    struct timeout_t * tm )
{
    rc_t rc = 0;
    size_t total, chunk_size;

    for ( total = 0; total < bsize && rc == 0; total += chunk_size )
    {
        void * chbuf;
        size_t chsize;

        /* retrieve buffer */
        rc = KChunkReaderNextBuffer ( chunks, & chbuf, & chsize );
        if ( rc != 0 )
        {
            TRACE ( "KHttpFileReadResponseInChunks (); failed: rc=%u\n", rc );
            break;
        }

        /* adjust number to read */
        chunk_size = chsize;
        if ( total + chsize > bsize )
            chunk_size = bsize - total;

        /* read bytes */
        rc = KStreamTimedReadExactly ( response, chbuf, chunk_size, tm );
        if ( rc != 0 )
        {
            TRACE ( "KStreamTimedReadExactly ( response, chbuf, chunk_size=%zu ); failed: rc=%u\n",
                    chunk_size, rc );
            chunk_size = 0;
        }
        else
        {
            /* consume them */
            rc = KChunkReaderConsumeChunk ( chunks, pos + total, chbuf, chunk_size );
        }

        /* return the buffer */
        KChunkReaderReturnBuffer ( chunks, chbuf, chsize );

        /* for multiple chunks, prepare the timeout */
        if ( tm != NULL && ! tm -> prepared && total + chunk_size < bsize )
            TimeoutPrepare ( tm );
    }

    * num_read = total;

    return ( total == 0 ) ? rc : 0;
}

static
rc_t KHttpFileTimedReadChunkedInt ( const KHttpFile * self,
    uint64_t pos, KChunkReader * chunks, size_t bytes, size_t * num_read,
    struct timeout_t * tm, uint32_t * http_status )
{
    rc_t rc = 0;
    uint32_t proxy_retries;

    size_t req_size = bytes;

    * http_status = 0;

    /* limit request size to EOF */
    assert ( pos < self -> file_size );
    if ( pos + bytes > self -> file_size )
        req_size = ( size_t ) ( self -> file_size - pos );

    /* try for a number of times to issue a request and get result */
    for ( proxy_retries = 5; rc == 0 && proxy_retries != 0; )
    {
        KClientHttpResult * rslt = NULL;
        rc = KHttpFileMakeRequest ( self, pos, req_size, tm, & rslt, http_status );
        if ( rc == 0 )
        {
            bool have_size;
            uint64_t start_pos;
            size_t result_size;
            uint64_t result_size64;

            switch ( * http_status )
            {

            case 200:
                proxy_retries = 0;

                /* extract stated bytes returned - must be whole file */
                have_size = KClientHttpResultSize ( rslt, & result_size64 );
#ifdef USE_SIZE_FROM_HEAD
                if (!have_size && !rslt->rangeRequested) {
                    result_size64 = self->file_size;
                    have_size = true;
                }
#endif
                result_size = ( size_t ) result_size64;
                if ( pos != 0 || ! have_size || result_size > bytes )
                {
                    rc = RC ( rcNS, rcFile, rcReading, rcData, rcUnexpected );
                    TRACE ( "KClientHttpResultSize ( rslt, & result_size ); unexpected status=%d\n",
                            * http_status );
                }
                else
                {
                    KStream * response;

                    /* assume we are reading the entire file */
                    assert ( ( uint64_t ) result_size == self -> file_size );
                    rc = KClientHttpResultGetInputStream ( rslt, & response );
                    if ( rc == 0 )
                    {
                        rc = KHttpFileReadResponseInChunks ( response, pos, chunks, req_size, num_read, tm );
                        KStreamRelease ( response );
                    }
                }
                break;

            case 206:
                proxy_retries = 0;

                /* extract actual amount being returned by server */
                rc = KClientHttpResultRange ( rslt, & start_pos, & result_size );
                if ( rc != 0 )
                {
                    TRACE ( "KClientHttpResultRange ( rslt, & start_pos, & result_size ); "
                            "failed: rc=%u\n", rc );
                }
                else if ( start_pos != pos )
                {
                    TRACE ( "KClientHttpResultRange ( rslt, & start_pos, & result_size ); "
                            "failed: start_pos=%lu != pos=%lu\n", start_pos, pos );
                    rc = RC ( rcNS, rcFile, rcReading, rcData, rcUnexpected );
                }
                else
                {
                    KStream * response;

                    if ( result_size != bytes )
                    {
                        TRACE ( "KClientHttpResultRange ( rslt, & start_pos, & result_size ); "
                                "short read: result_size=%lu != bytes=%lu\n", result_size, bytes );
                    }

                    rc = KClientHttpResultGetInputStream ( rslt, & response );
                    if ( rc == 0 )
                    {
                        rc = KHttpFileReadResponseInChunks ( response, pos, chunks, req_size, num_read, tm );
                        KStreamRelease ( response );
                    }
                }
                break;

            case 403:
            case 404:
                if ( -- proxy_retries != 0 )
                {
                    TRACE ( "KClientHttpResultStatus ( rslt, & http_status, NULL, 0, NULL ); "
                            "unexpected status=%d - sleeping and retrying\n", * http_status );
                    KSleep ( 1 );
                    rc = 0;
                    break;
                }

                /* NO BREAK */

            default:
                rc = RC ( rcNS, rcFile, rcReading, rcData, rcUnexpected );
                TRACE ( "KClientHttpResultStatus ( rslt, & http_status, NULL, 0, NULL ); "
                        "unexpected status=%d\n", * http_status );
                break;
            }

            KClientHttpResultRelease ( rslt );
        }
    }

    if ( rc != 0 || * num_read == 0 )
        KClientHttpClose ( self -> http );

    return rc;
}

static
rc_t KHttpFileTimedReadChunkedLocked ( const KHttpFile * self,
    uint64_t pos, KChunkReader * chunks, size_t bytes, size_t * num_read,
    struct timeout_t * tm, uint32_t * http_status )
{
    rc_t rc = KLockAcquire ( self -> lock );
    if ( rc == 0 )
    {
        /* moved the request boundary processing here.
           first, check trivial case of read beyond EOF. */
        if ( pos >= self -> file_size )
            * num_read = 0;
        else
        {
            /* limit read request to amount available in file */
            if ( pos + bytes > self -> file_size )
                bytes = ( size_t ) ( self -> file_size - pos );

            /* if request is for 256 bytes or more, go ahead */
            if ( bytes >= 256 )
                rc = KHttpFileTimedReadChunkedInt ( self, pos, chunks, bytes, num_read, tm, http_status );

            else
            {
                /* a single chunk will do */
                void * chbuf;
                size_t chsize;
                rc = KChunkReaderNextBuffer ( chunks, & chbuf, & chsize );
                if ( rc == 0 )
                {
                    /* issue request through normal, non-chunked read */
                    assert ( chsize >= 256 );
                    rc = KHttpFileTimedReadShort ( self, pos, chbuf, bytes, num_read, tm, http_status );
                    if ( rc == 0 )
                    {
                        KChunkReaderConsumeChunk ( chunks, pos, chbuf, * num_read );
                    }

                    KChunkReaderReturnBuffer ( chunks, chbuf, chsize );
                }
            }
        }

        KLockUnlock ( self -> lock );
    }
    return rc;
}

static
rc_t CC KHttpFileTimedReadChunked ( const KHttpFile * self, uint64_t pos,
     KChunkReader * chunks, size_t bytes, size_t * num_read, struct timeout_t * tm )
{
    rc_t rc;
    KHttpRetrier retrier;

    /* this shoud have been checked in the interface dispatch.
       it addresses the concern over attempts to read small amounts
       over HTTP with the Apache short-read bug. */
    assert ( KChunkReaderBufferSize ( chunks ) == 0 || KChunkReaderBufferSize ( chunks ) >= 256 );

    rc = KHttpRetrierInit ( & retrier, self -> url_buffer . base, self -> kns );
    if ( rc == 0 )
    {
        rc_t rc2;

        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ),
            ( "KHttpFileTimedReadChunked(pos=%lu,size=%zu)...\n", pos, bytes ) );

        /* loop using existing KClientHttp object */
        while ( rc == 0 )
        {
            uint32_t http_status;
            rc = KHttpFileTimedReadChunkedLocked ( self, pos, chunks, bytes, num_read, tm, & http_status );

            /* ALWAYS account for chunks already read */
            pos += * num_read;
            bytes -= * num_read;
            if ( bytes == 0 )
            {
                break;
            }

            if ( rc != 0 )
            {
                rc2 = KClientHttpReopen ( self -> http );
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ), ( "KHttpFileTimedReadChunked: "
                    "KHttpFileTimedReadChunkedLocked failed, reopening\n" ) );
                if ( rc2 == 0 )
                {
                    rc2 = KHttpFileTimedReadChunkedLocked ( self, pos, chunks, bytes, num_read, tm, & http_status );

                    /* ALWAYS account for chunks already read */
                    pos += * num_read;
                    bytes -= * num_read;
                    if ( bytes == 0 )
                    {
                        break;
                    }

                    if ( rc2 == 0 )
                    {
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ), ( "KHttpFileTimedReadChunked: "
                                                                       "reopened successfully\n" ) );
                        rc = 0;
                    }
                    else
                    {
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ), ( "KHttpFileTimedReadChunked: "
                                                                       "reopen failed\n" ) );
                        break;
                    }
                }
            }

            if ( ! KHttpRetrierWait ( & retrier, http_status ) )
            {
                assert ( num_read != NULL );
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ),
                    ( "...KHttpFileTimedReadChunked(pos=%lu,size=%zu)=%zu\n\n",
                      pos, bytes, * num_read ) );
                break;
            }
            rc = KClientHttpReopen ( self -> http );
        }

        rc2 = KHttpRetrierDestroy ( & retrier );
        if ( rc == 0 )
            rc = rc2;
    }

    if ( rc != 0 && KNSManagerLogNcbiVdbNetError ( self -> kns ) )
    {
        KEndPoint ep, local_ep;
        KClientHttpGetLocalEndpoint  ( self -> http, & local_ep );
        KClientHttpGetRemoteEndpoint ( self -> http, & ep );

        PLOGERR ( klogErr, ( klogErr, rc,
            "Failed to KHttpFileTimedReadChunked("
            "'$(path)' ($(ip)), $(bytes)) from '$(local)'",
               "path=%s,ip=%s,bytes=%zu,local=%s",
            self -> url_buffer . base,
            ep . ip_address, bytes, local_ep . ip_address ) );
    }

    return rc;
}

static
rc_t CC KHttpFileReadChunked ( const KHttpFile * self, uint64_t pos,
    KChunkReader * chunks, size_t bytes, size_t * num_read )
{
    struct timeout_t tm;
    TimeoutInit ( & tm, self -> kns -> http_read_timeout );

    return KHttpFileTimedReadChunked ( self, pos, chunks, bytes, num_read, & tm );
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
#if SUPPORT_CHUNKED_READ
    , KHttpFileReadChunked
    , KHttpFileTimedReadChunked
#endif
};

static
rc_t KHttpFileMake( KHttpFile ** self, const char *url, va_list args )
{
    rc_t rc;
    KHttpFile * f = calloc ( 1, sizeof *f );
    if ( f == NULL )
    {
        rc = RC ( rcNS, rcFile, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        rc = KFileInit ( &f -> dad, ( const KFile_vt * ) &vtKHttpFile, "KHttpFile", url, true, false );
        if ( rc == 0 )
        {
            rc = KLockMake ( & f -> lock );
            if ( rc == 0 )
            {
                KDataBuffer * buf = & f -> orig_url_buffer;
                rc = KDataBufferMake( buf, 8, 0 );
                if ( rc == 0 )
                {
                    rc = KDataBufferVPrintf ( buf, url, args );
                    if ( rc == 0 )
                    {
                        rc = ParseUrl ( & f -> block, buf -> base, buf -> elem_count - 1 );
                        if ( rc == 0 )
                        {
                            *self = f;
                            return 0;
                        }
                    }
                    KDataBufferWhack( & f -> orig_url_buffer );
                }
                KLockRelease ( f -> lock );
            }
        }
        free ( f );
    }
    return rc;
}

static rc_t KNSManagerVMakeHttpFileInt ( const KNSManager *self,
    const KFile **file, KStream *conn, ver_t vers, bool reliable, bool need_env_token, bool payRequired,
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
            KHttpFile * f;
            rc = KHttpFileMake ( &f, url, args );
            if ( rc == 0 )
            {
                KDataBuffer * buf = & f -> orig_url_buffer;
                KClientHttp *http;
                rc = KNSManagerMakeClientHttpInt ( self, & http, buf, conn, vers,
                    self -> http_read_timeout, self -> http_write_timeout, & f -> block . host, f -> block . port, reliable, f -> block . tls );
                if ( rc == 0 )
                {
                    KClientHttpRequest *req;

                    rc = KClientHttpMakeRequestInt ( http, & req, & f -> block, buf );
                    if ( rc == 0 )
                    {
                        KClientHttpResult *rslt;

                        if ( need_env_token )
                        {
                            KClientHttpRequestAttachEnvironmentToken ( req );
                        }
                        KClientHttpRequestSetCloudParams ( req, need_env_token, payRequired );

                        rc = KClientHttpRequestHEAD ( req, & rslt );
                        if ( rc == 0 && rslt -> expiration != NULL )
                        {   /* retrieve and save the URL expiration time */
                            f -> url_is_temporary = true;
                            KTimeFromIso8601 ( & f -> url_expiration, rslt -> expiration, string_size ( rslt -> expiration ) );
//TODO: still, handle the expiration in read methods (find out how AWS/GCP signal expiration)
                        }

                        /* update url_buffer with the (possibly different and/or temporary) URL*/
                        KClientHttpRequestURL ( req, & f -> url_buffer ); /* NB. f -> url_buffer is not valid until this point */
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ),
                            ( "HttpFile.URL updated to '%.*s'\n",
                                ( int ) f -> url_buffer . elem_count, f -> url_buffer . base ) );
                        KClientHttpRequestRelease ( req );

                        if ( rc != 0 ) {
                            if ( KNSManagerLogNcbiVdbNetError ( self ) )
                            {
                                KEndPoint ep, local_ep;
                                KClientHttpGetLocalEndpoint  ( http, & local_ep );
                                KClientHttpGetRemoteEndpoint ( http, & ep );
                                PLOGERR ( klogErr, ( klogErr, rc,
                                    "Failed to KClientHttpRequestHEAD("
                                    "'$(path)' ($(ip))) from '$(local)'",
                                    "path=%.*s,ip=%s,local=%s",
                                    buf -> elem_count - 1, buf -> base,
                                    ep . ip_address, local_ep . ip_address ) );
                            }
                        }
                        else
                        {
                            uint64_t size;
                            uint32_t status;

                            /* get the file size */
                            bool have_size = KClientHttpResultSize ( rslt, & size );

                            /* see if the server accepts partial content range requests */
                            char buffer[1024];
                            size_t num_read;
                            bool accept_ranges = KClientHttpResultGetHeader ( rslt, "Content-Range", buffer, sizeof buffer, &num_read ) == 0 ||
                                                KClientHttpResultTestHeaderValue ( rslt, "Accept-Ranges", "bytes" );

                            /* check the result status */
                            rc = KClientHttpResultStatus ( rslt, & status, NULL, 0, NULL );

                            /* done with result */
                            KClientHttpResultRelease ( rslt );

                            /* check for error status */
                            if ( rc == 0 )
                            {
                                switch ( status )
                                {
                                case 200:
                                case 206: /* can happen on the cloud if HEAD is simulated with a short GET */
                                    if ( ! have_size )
                                        rc = RC ( rcNS, rcFile, rcOpening, rcSize, rcUnknown );
                                    else if ( ! accept_ranges )
                                        rc = RC ( rcNS, rcFile, rcOpening, rcFunction, rcUnsupported );
                                    break;
                                case 403:
                                    rc = RC ( rcNS, rcFile, rcOpening, rcFile, rcUnauthorized );
                                    break;
                                case 404:
                                    rc = RC ( rcNS, rcFile, rcOpening, rcFile, rcNotFound );
                                    break;
                                default:
                                    rc = RC ( rcNS, rcFile, rcValidating, rcNoObj, rcEmpty );
                                }

                                if ( rc == 0 )
                                {
                                    rc = KNSManagerAddRef ( self );
                                    if ( rc == 0 )
                                    {
                                        f -> kns = self;
                                        f -> file_size = size;
                                        f -> http = http;
                                        f -> no_cache = size >= NO_CACHE_LIMIT;
                                        f -> need_env_token = need_env_token;
                                        f -> payRequired = payRequired;

                                        * file = & f -> dad;
                                        return 0;
                                    }
                                }
                                else {
                                    KEndPoint ep, local_ep;
                                    KClientHttpGetLocalEndpoint  ( http, & local_ep );
                                    KClientHttpGetRemoteEndpoint ( http, & ep );
                                    if ( KNSManagerLogNcbiVdbNetError ( self ) ) {
                                        char * base = buf -> base;
                                        bool print = true;
                                        char * query = string_chr ( base, buf -> elem_count, '?' );
                                        String vdbcache;
                                        CONST_STRING ( & vdbcache, ".vdbcache" );
                                        if ( buf -> elem_count > vdbcache . size ) {
                                            String ext;
                                            StringInit ( & ext,
                                                    base + buf -> elem_count - vdbcache . size - 1,
                                                    vdbcache . size, vdbcache . size );
                                            if ( ext . addr [ ext . size ] == '\0' &&
                                                StringEqual ( & vdbcache, & ext ) )
                                            {
                                                print = false;
                                            }
                                            else if ( query != NULL ) {
                                                size_t size = query - base;
                                                StringInit ( & ext,
                                                    base + size - vdbcache . size,
                                                    vdbcache . size, vdbcache . size );
                                                if ( ext . addr [ ext . size ] == '?' &&
                                                    StringEqual ( & vdbcache, & ext ) )
                                                {
                                                    print = false;
                                                }
                                            }
                                        }
                                        if ( ! reliable )
                                            print = false;
                                        if ( print ) {
                                        assert ( buf );
                                        PLOGERR ( klogErr,
                                            ( klogErr, rc,
                                            "Failed to KNSManagerVMakeHttpFileInt('$(path)' ($(ip)))"
                                            " from '$(local)'", "path=%.*s,ip=%s,local=%s",
                                            ( int ) buf -> elem_count, buf -> base,
                                            ep . ip_address, local_ep . ip_address
                                            ) );
                                        }
                                    }
                                    else
                                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ),
                                            ( "Failed to KNSManagerVMakeHttpFileInt('%.*s' (%s))\n",
                                            ( int ) buf -> elem_count, buf -> base,
                                            ep . ip_address ) );
                                }
                            }
                        }
                    }
                    KClientHttpRelease ( http );
                }
                KHttpFileDestroy(f);
            }
        }

        * file = NULL;
    }

    return rc;
}

/******************************************************************************/

LIB_EXPORT rc_t CC KNSManagerMakeHttpFile(const KNSManager *self,
    const KFile **file, struct KStream *conn, ver_t vers, const char *url, ...)
{
    rc_t rc = 0;
    va_list args;
    va_start(args, url);
    rc = KNSManagerVMakeHttpFileInt ( self, file, conn, vers, false, false, false, url, args);
    va_end(args);
    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMakeReliableHttpFile(const KNSManager *self,
    const KFile **file, struct KStream *conn, ver_t vers, bool reliable, bool need_env_token, bool payRequired, const char *url, ...)
{
    rc_t rc = 0;
    va_list args;
    va_start(args, url);
    rc = KNSManagerVMakeHttpFileInt ( self, file, conn, vers, true, need_env_token, payRequired, url, args);
    va_end(args);
    return rc;
}

LIB_EXPORT bool CC KFileIsKHttpFile ( const struct KFile * self )
{
    return self != NULL && &self->vt->v1 == &vtKHttpFile;
}
