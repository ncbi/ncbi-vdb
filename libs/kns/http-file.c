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

#include <kproc/lock.h>
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


/*--------------------------------------------------------------------------
 * KHttpFile
 */
struct KHttpFile
{
    KFile dad;
    
    uint64_t file_size;

    const KNSManager * kns;

    KLock * lock;
    KClientHttp *http;

    KDataBuffer url_buffer;

    bool no_cache;
};

static
rc_t CC KHttpFileDestroy ( KHttpFile *self )
{
    KLockRelease ( self -> lock );
    KNSManagerRelease ( self -> kns );
    KClientHttpRelease ( self -> http );
    KDataBufferWhack ( & self -> url_buffer );
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
                            switch ( * http_status )
                            {
                            case 206:
                            {
                                uint64_t start_pos;
                                size_t result_size;

                                /* don't need retries now */
                                proxy_retries = 0;

                                /* extract actual amount being returned by server */
                                rc = KClientHttpResultRange ( rslt, &start_pos, &result_size );
                                if ( rc != 0 || start_pos != pos || result_size != bsize )
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
                                }
                                else
                                {
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
                            }

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

static
rc_t KHttpFileTimedReadLocked ( const KHttpFile *cself,
    uint64_t aPos, void *aBuf, size_t aBsize,
    size_t * num_read, struct timeout_t * tm, uint32_t * http_status )
{
    rc_t rc = KLockAcquire ( cself -> lock );
    if ( rc == 0 )
    {
        rc = KHttpFileTimedReadInt ( cself, aPos, aBuf, aBsize, num_read, tm, http_status );
        KLockUnlock ( cself -> lock );
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
        
        {
            rc_t rc2 = KHttpRetrierDestroy ( & retrier );
            if ( rc == 0 ) rc = rc2;
        }
    }
    
    return rc;
}

static
rc_t CC KHttpFileRead ( const KHttpFile *self, uint64_t pos,
     void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = 0;
    struct timeout_t tm;
    TimeoutInit ( & tm, self -> kns -> http_read_timeout );
    rc = KHttpFileTimedRead ( self, pos, buffer, bsize, num_read, & tm );
    if ( rc != 0 && KNSManagerLogNcbiVdbNetError ( self -> kns ) ) {
        KEndPoint ep, local_ep;
        KClientHttpGetLocalEndpoint  ( self -> http, & local_ep );
        KClientHttpGetRemoteEndpoint ( self -> http, & ep );
        PLOGERR ( klogErr, ( klogErr, rc,
            "Failed to KHttpFileRead("
            "'$(path)' ($(ip)), $(bytes)) from '$(local)'",
               "path=%s,ip=%s,bytes=%zu,local=%s",
            self -> url_buffer . base,
            ep . ip_address, bsize, local_ep . ip_address ) );
    }
    return rc;
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
                    rc = KLockMake ( & f -> lock );
                    if ( rc == 0 )
                    {
                        KDataBuffer *buf = & f -> url_buffer;
                        buf -> elem_bits = 8;
                        rc = KDataBufferVPrintf ( buf, url, args );
                        if ( rc == 0 )
                        {
                            URLBlock block;
                            rc = ParseUrl ( &block, buf -> base, buf -> elem_count - 1 );
                            if ( rc == 0 ) 
                            {
                                KClientHttp *http;
                          
                                rc = KNSManagerMakeClientHttpInt ( self, & http, buf, conn, vers,
                                    self -> http_read_timeout, self -> http_write_timeout, &block . host, block . port, reliable, block . tls );
                                if ( rc == 0 )
                                {
                                    KClientHttpRequest *req;

                                    rc = KClientHttpMakeRequestInt ( http, &req, &block, buf );
                                    if ( rc == 0 )
                                    {
                                        KClientHttpResult *rslt;
                                        rc = KClientHttpRequestHEAD ( req, & rslt );
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

                                            /* get the file size from HEAD query */
                                            bool have_size = KClientHttpResultSize ( rslt, & size );

                                            /* see if the server accepts partial content range requests */
                                            bool accept_ranges = KClientHttpResultTestHeaderValue ( rslt, "Accept-Ranges", "bytes" );

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
                            }
                        }

                        KDataBufferWhack ( buf );
                        KLockRelease ( f -> lock );
                    }
                }
                free ( f );
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
    rc = KNSManagerVMakeHttpFileInt ( self, file, conn, vers, false, url, args);
    va_end(args);
    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMakeReliableHttpFile(const KNSManager *self,
    const KFile **file, struct KStream *conn, ver_t vers, const char *url, ...)
{
    rc_t rc = 0;
    va_list args;
    va_start(args, url);
    rc = KNSManagerVMakeHttpFileInt ( self, file, conn, vers, true, url, args);
    va_end(args);
    return rc;
}

LIB_EXPORT bool CC KFileIsKHttpFile ( const struct KFile * self )
{
    return self != NULL && &self->vt->v1 == &vtKHttpFile;
}
