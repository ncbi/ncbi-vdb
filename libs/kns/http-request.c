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

#include <klib/rc.h>
#include <klib/data-buffer.h>
#include <klib/printf.h>
#include <klib/log.h>
#include <klib/base64.h>
#include <klib/debug.h>

#include <cloud/manager.h>

#include <kns/manager.h>
#include <kns/stream.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/mmap.h>

#include "http-priv.h"
#include "mgr-priv.h"

#if _DEBUGGING && 0
#include <stdio.h>
#define TRACE( x, ... ) \
    fprintf ( stderr, "@@ %s:%d: %s: " x, __FILE__, __LINE__, __func__, __VA_ARGS__ )
#elif _DEBUGGING && defined _h_klib_status_ && 0
#define TRACE( x, ... ) \
    STATUS ( 0, "@@ %s:%d: %s: " x, __FILE__, __LINE__, __func__, __VA_ARGS__ )
#else
#define TRACE( x, ... ) \
    ( ( void ) 0 )
#endif

/*--------------------------------------------------------------------------
 * KClientHttpRequest
 *  hyper text transfer protocol
 */

LIB_EXPORT rc_t CC KClientHttpRequestSetCloudParams(
    KClientHttpRequest * self, bool ceRequired, bool payRequired)
{
    if (self != NULL)
    {
        self->ceRequired = ceRequired;
        self->payRequired = payRequired;
    }

    return 0;
}

rc_t
KClientHttpRequestAttachEnvironmentToken( KClientHttpRequest * self )
{
    CloudMgr * cloudMgr;
    rc_t rc = CloudMgrMake ( & cloudMgr, NULL, NULL );
    if ( rc == 0 )
    {
        Cloud * cloud;
        rc = CloudMgrGetCurrentCloud ( cloudMgr, & cloud );
        if ( rc == 0 )
        {
            rc = CloudAddComputeEnvironmentTokenForSigner ( cloud, self );
            CloudRelease ( cloud );
        }
        CloudMgrRelease ( cloudMgr );
    }
    return rc;
}

rc_t KClientHttpRequestURL(KClientHttpRequest const *self, KDataBuffer *rslt)
{
    KDataBufferWhack ( rslt );
    return KDataBufferSub(&self->url_buffer, rslt, 0, self->url_buffer.elem_count);
}

rc_t KClientHttpRequestClear ( KClientHttpRequest *self )
{
    KDataBufferWhack ( & self -> url_buffer );

    return 0;
}

const char *
KClientHttpRequestGetBody( struct KClientHttpRequest * self )
{
    assert ( self );
    return ( const char * ) ( self -> body . base );
}

rc_t KClientHttpRequestInit ( KClientHttpRequest * req,
    const URLBlock *b, const KDataBuffer *buf )
{
    rc_t rc = KDataBufferSub ( buf, & req -> url_buffer, 0, UINT64_MAX );
    if ( rc == 0 )
    {
        /* assign url_block */
        req -> url_block = * b;
    }
    return rc;
}

rc_t KClientHttpRequestGetQuery( struct KClientHttpRequest * self, const struct String ** query )
{
    if ( self == NULL )
        return RC ( rcNS, rcNoTarg, rcReading, rcSelf, rcNull );
    if ( query == NULL )
        return RC ( rcNS, rcNoTarg, rcReading, rcParam, rcNull );

    * query = & self -> url_block . query;
    return 0;
}


/* MakeRequestInt[ernal]
 */
rc_t KClientHttpMakeRequestInt ( const KClientHttp *self,
    KClientHttpRequest **_req, const URLBlock *block, const KDataBuffer *buf )
{
    rc_t rc;

    /* create the object with empty buffer */
    KClientHttpRequest * req
        = (KClientHttpRequest *) calloc ( 1, sizeof * req );
    if ( req == NULL )
        rc = RC ( rcNS, rcNoTarg, rcAllocating, rcMemory, rcNull );
    else
    {
        rc = KClientHttpAddRef ( self );
        if ( rc == 0 )
        {
            /* assign http */
            req -> http = ( KClientHttp* ) self;

            /* initialize body to zero size */
            rc = KDataBufferMakeBytes ( & req -> body, 0 );
            if ( rc == 0 )
            {
                KRefcountInit ( & req -> refcount, 1, "KClientHttpRequest",
                    "make", (char*) buf -> base );

                /* fill out url_buffer with URL */
                rc = KClientHttpRequestInit ( req, block, buf );
                if ( rc == 0 )
                {
                    * _req = req;
    /*              DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ),
                        ( " KClientHttpMakeRequestInt (path=%S) = (path:%S)\n",
                        & block -> path, & ( * _req ) -> url_block . path ) ); */
                    return 0;
                }
            }

            KClientHttpRelease ( self );
        }
    }

    free ( req );

    return rc;
}


/* MakeRequest
 *  create a request that can be used to contact HTTP server
 *
 *  "req" [ OUT ] - return parameter for HTTP request object
 *
 *  "vers" [ IN ] - http version
 *
 *  "conn" [ IN, NULL OKAY ] - previously opened stream for communications.
 *
 *  "url" [ IN ] - full resource identifier. if "conn" is NULL,
 *   the url is parsed for remote endpoint and is opened by mgr.
 */
LIB_EXPORT rc_t CC KClientHttpVMakeRequest ( const KClientHttp *self,
    KClientHttpRequest **_req, const char *url, va_list args )
{
    rc_t rc;

    if ( _req == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    else
    {
        * _req = NULL;

        if ( self == NULL )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
        else if ( url ==  NULL )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
        else if ( url [ 0 ] == 0 )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcInsufficient );
        else
        {
            KDataBuffer buf;

            /* make a KDataBuffer and copy in url with the va_lis */
            rc = KDataBufferMakeBytes ( & buf, 0 );
            if ( rc == 0 )
            {
                rc = KDataBufferVPrintf ( &buf, url, args );
                if ( rc == 0 )
                {
                    /* parse the URL */
                    URLBlock block;
                    rc = ParseUrl ( & block, (char*) buf . base,
                        buf . elem_count - 1 );
                    if ( rc == 0 )
                        rc = KClientHttpMakeRequestInt ( self, _req, & block, & buf );
                }
            }

            KDataBufferWhack ( & buf );
        }
    }

    return rc;
}

/* MakeRequest
 *  create a request that can be used to contact HTTP server
 *
 *  "req" [ OUT ] - return parameter for HTTP request object
 *
 *  "url" [ IN ] - full resource identifier. if "conn" is NULL,
 *   the url is parsed for remote endpoint and is opened by mgr.
 */
LIB_EXPORT rc_t CC KClientHttpMakeRequest ( const KClientHttp *self,
    KClientHttpRequest **_req, const char *url, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, url );
    rc = KClientHttpVMakeRequest ( self, _req, url, args );
    va_end ( args );

    return rc;
}

/* MakeRequest
 *  create a request that can be used to contact HTTP server
 *
 *  "req" [ OUT ] - return parameter for HTTP request object
 *
 *  "vers" [ IN ] - http version
 *
 *  "conn" [ IN, NULL OKAY ] - previously opened stream for communications.
 *
 *  "url" [ IN ] - full resource identifier. if "conn" is NULL,
 *   the url is parsed for remote endpoint and is opened by mgr.
 */
static
rc_t CC KNSManagerMakeClientRequestInt ( const KNSManager *self,
    KClientHttpRequest **req, ver_t vers, KStream *conn, bool reliable, const char *url, va_list args )
{
    rc_t rc;

    if ( req == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    else
    {
        * req = NULL;

        if ( self == NULL )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
        else if ( vers < 0x01000000 || vers > 0x01010000 )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcIncorrect );
        else if ( url == NULL )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcString, rcNull );
        else if ( url [ 0 ] == 0 )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcString, rcEmpty );
        else
        {
            KDataBuffer buf;
            rc = KDataBufferMakeBytes ( & buf, 0 );
            if ( rc == 0 )
            {   /* convert var-arg "url" to a full string */
                rc = KDataBufferVPrintf ( & buf, url, args );
                if ( rc == 0 )
                {
                    /* parse the URL */
                    URLBlock block;
                    rc = ParseUrl ( & block, (char*) buf . base,
                        buf . elem_count - 1 );
                    if ( rc == 0 )
                    {
                        KClientHttp * http;

                        rc = KNSManagerMakeClientHttpInt ( self, & http, & buf, conn, vers,
                            self -> http_read_timeout, self -> http_write_timeout, & block . host, block . port, reliable, block . tls );
                        if ( rc == 0 )
                        {
                            rc = KClientHttpMakeRequestInt ( http, req, & block, & buf );
                            KClientHttpRelease ( http );
                        }
                    }
                }
                KDataBufferWhack ( & buf );
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMakeClientRequest ( const KNSManager *self,
    KClientHttpRequest **req, ver_t vers, KStream *conn, const char *url, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, url );
    rc = KNSManagerMakeClientRequestInt ( self, req, vers, conn, false, url, args );
    va_end ( args );
    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMakeReliableClientRequest ( const KNSManager *self,
    KClientHttpRequest **req, ver_t vers, KStream *conn, const char *url, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, url );
    rc = KNSManagerMakeClientRequestInt ( self, req, vers, conn, true, url, args );
    va_end ( args );
    return rc;
}


/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KClientHttpRequestAddRef ( const KClientHttpRequest *self )
{
        if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KClientHttpRequest" ) )
        {
        case krefLimit:
            return RC ( rcNS, rcNoTarg, rcAttaching, rcRange, rcExcessive );
        case krefNegative:
            return RC ( rcNS, rcNoTarg, rcAttaching, rcSelf, rcInvalid );
        default:
            break;
        }
    }

    return 0;
}

static
rc_t KClientHttpRequestWhack ( KClientHttpRequest * self )
{
    KClientHttpRequestClear ( self );

    KClientHttpRelease ( self -> http );
    KDataBufferWhack ( & self -> body );

    BSTreeWhack  ( & self -> hdrs, KHttpHeaderWhack, NULL );
    KRefcountWhack ( & self -> refcount, "KClientHttpRequest" );
    free ( self );
    return 0;
}

LIB_EXPORT rc_t CC KClientHttpRequestRelease ( const KClientHttpRequest *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KClientHttpRequest" ) )
        {
        case krefWhack:
            return KClientHttpRequestWhack ( ( KClientHttpRequest* ) self );
        case krefNegative:
            return RC ( rcNS, rcNoTarg, rcReleasing, rcRange, rcExcessive );
        default:
            break;
        }
    }

    return 0;
}

/* Connection
 *  sets connection management headers
 *
 *  "close" [ IN ] - if "true", inform the server to close the connection
 *   after its response ( default for version 1.0 ). when "false" ( default
 *   for version 1.1 ), ask the server to keep the connection open.
 *
 * NB - the server is not required to honor the request
 */
LIB_EXPORT rc_t CC KClientHttpRequestConnection ( KClientHttpRequest *self, bool close )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
    else
    {
        String name, value;

        CONST_STRING ( & name, "Connection" );
        /* if version is 1.1 and close is true, add 'close' to Connection header value. */
        /* if version if 1.1 default is false - no action needed */
        if ( self -> http -> vers == 0x01010000 && close == true )
            CONST_STRING ( & value, "close" );
        else if ( self -> http -> vers == 0x01000000 && close == false )
            CONST_STRING ( & value, "keep-alive" );
        else
            return 0;

        rc = KClientHttpRequestAddHeader ( self,  name . addr, value . addr );

    }
    return rc;
}

/* SetNoCache
 *  guard against over-eager proxies that try to cache entire files
 *  and handle byte-ranges locally.
 */
LIB_EXPORT rc_t CC KClientHttpRequestSetNoCache ( KClientHttpRequest *self )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcNS, rcNoTarg, rcUpdating, rcSelf, rcNull );
    else
    {
        rc = KClientHttpRequestAddHeader ( self, "Cache-Control", "no-cache, no-store, max-age=0, no-transform, must-revalidate" );
        if ( rc == 0 )
            rc = KClientHttpRequestAddHeader ( self, "Pragma", "no-cache" );
        if ( rc == 0 )
            rc = KClientHttpRequestAddHeader ( self, "Expires", "0" );
    }

    return rc;
}

/* ByteRange
 *  set requested byte range of response
 *
 *  "pos" [ IN ] - beginning offset within remote entity
 *
 *  "bytes" [ IN ] - the number of bytes being requested
 */
LIB_EXPORT rc_t CC KClientHttpRequestByteRange ( KClientHttpRequest *self, uint64_t pos, size_t bytes )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull);
    else
    {
        char  range [ 256 ];
        size_t num_writ;
        String name, value;

        CONST_STRING ( & name, "Range" );
        rc = string_printf /* USE IS OKAY */ ( range, sizeof range, & num_writ, "bytes=%lu-%lu"
                             , pos
                             , pos + bytes - 1);
        if ( rc == 0 )
        {
            StringInitCString ( & value, range );

            rc = KClientHttpRequestAddHeader ( self, name . addr, value . addr );
        }
    }
    return rc;
}

/* AddHeader
 *  allow addition of an arbitrary HTTP header to message
 */
LIB_EXPORT rc_t CC KClientHttpRequestAddHeader ( KClientHttpRequest *self,
    const char *name, const char *val, ... )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull);
    else
    {
        if ( name == NULL )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
        /* have to test for empty name, too */
        else if ( name [ 0 ] == 0 )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcInsufficient );
        else if ( val == NULL )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
        /* same for empty value fmt string */
        else if ( val [ 0 ] == 0 )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcInsufficient );
        else
        {
            size_t name_size;
            bool accept_not_modified;

            va_list args;
            va_start ( args, val );

            /* disallow setting of "Host" and other headers */
            name_size = string_size ( name );

#define CSTRLEN( str ) \
            sizeof ( str ) - 1
#define NAMEIS( str ) \
            strcase_cmp ( name, name_size, str, sizeof str, CSTRLEN ( str ) ) == 0

            rc = 0;
            accept_not_modified = false;

            switch ( name_size )
            {
            case CSTRLEN ( "Host" ):
                if ( NAMEIS ( "Host" ) )
                    rc = RC ( rcNS, rcNoTarg, rcComparing, rcParam, rcUnsupported );
                break;
            case CSTRLEN ( "Content-Length" ):
                if ( NAMEIS ( "Content-Length" ) )
                    rc = RC ( rcNS, rcNoTarg, rcComparing, rcParam, rcUnsupported );
                break;
            case CSTRLEN ( "If-None-Match" ):
                if ( NAMEIS ( "If-None-Match" ) )
                    accept_not_modified = true;
                break;
            case CSTRLEN ( "If-Modified-Since" ):
                if ( NAMEIS ( "If-Modified-Since" ) )
                    accept_not_modified = true;
                break;
            }

#undef CSTRLEN
#undef NAMEIS

            if ( rc == 0 )
            {
                rc = KClientHttpVAddHeader
                    ( & self -> hdrs, false, name, val, args );
                if ( rc == 0 && accept_not_modified )
                    self -> accept_not_modified = true;
            }

            va_end ( args );
        }
    }
    return rc;
}


/* GetHeader
 *  retrieve named header if present
 *  this can potentially return a comma separated value list
 */
LIB_EXPORT rc_t CC KClientHttpRequestGetHeader(const KClientHttpRequest *self,
    const char *name, char *buffer, size_t bsize, size_t *num_read)
{
    rc_t rc = 0;

    if (num_read == NULL)
        rc = RC(rcNS, rcNoTarg, rcValidating, rcParam, rcNull);
    else
    {
        *num_read = 0;

        if (self == NULL)
            rc = RC(rcNS, rcNoTarg, rcValidating, rcSelf, rcNull);
        else if (name == NULL)
            rc = RC(rcNS, rcNoTarg, rcValidating, rcParam, rcNull);
        else if (buffer == NULL && bsize != 0)
            rc = RC(rcNS, rcNoTarg, rcValidating, rcParam, rcNull);
        else
        {
            rc = KClientHttpFindHeader(&self->hdrs, name,
                buffer, bsize, num_read);
        }
    }

    return rc;
}


LIB_EXPORT rc_t CC KClientHttpRequestGetHost(const KClientHttpRequest *self,
    char *buffer, size_t bsize, size_t *num_read)
{
    rc_t rc = 0;

    if (num_read == NULL)
        rc = RC(rcNS, rcNoTarg, rcValidating, rcParam, rcNull);
    else
    {
        *num_read = 0;

        if (self == NULL)
            rc = RC(rcNS, rcNoTarg, rcValidating, rcSelf, rcNull);
        else if (buffer == NULL && bsize != 0)
            rc = RC(rcNS, rcNoTarg, rcValidating, rcParam, rcNull);
        else if (bsize < self->url_block.host.size + 1)
            rc = RC(rcNS, rcNoTarg, rcValidating, rcBuffer, rcInsufficient);
        else {
            string_copy(buffer, bsize,
                self->url_block.host.addr, self->url_block.host.size);
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC KClientHttpRequestGetPath(const KClientHttpRequest *self,
    char *buffer, size_t bsize, size_t *num_read)
{
    rc_t rc = 0;

    if (num_read == NULL)
        rc = RC(rcNS, rcNoTarg, rcValidating, rcParam, rcNull);
    else
    {
        *num_read = 0;

        if (self == NULL)
            rc = RC(rcNS, rcNoTarg, rcValidating, rcSelf, rcNull);
        else if (buffer == NULL && bsize != 0)
            rc = RC(rcNS, rcNoTarg, rcValidating, rcParam, rcNull);
        else if (bsize < self->url_block.path.size + 1)
            rc = RC(rcNS, rcNoTarg, rcValidating, rcBuffer, rcInsufficient);
        else {
            string_copy(buffer, bsize,
                self->url_block.path.addr, self->url_block.path.size);
        }
    }

    return rc;
}


/* AddPostParam
 *  adds a parameter for POST
 */
LIB_EXPORT rc_t CC KClientHttpRequestVAddPostParam ( KClientHttpRequest *self, const char *fmt, va_list args )
{
    rc_t rc;

    if ( self == NULL )
    {
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
    }
    else if ( fmt == NULL )
    {
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    }
    else if ( fmt [ 0 ] == 0 )
    {
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    }
    else
    {

        /* TBD - reject embedded newlines */
        /* TBD - URL-encoding or at least detect need for it */

        /* first param */
        if ( self -> body . elem_count == 0 )
            rc = KDataBufferVPrintf ( & self -> body, fmt, args );
        else
        {
            /* additional param - add separator */
            rc = KDataBufferPrintf ( & self -> body, "&" );
            if ( rc == 0 )
                rc = KDataBufferVPrintf ( & self -> body, fmt, args );
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC KClientHttpRequestAddPostParam ( KClientHttpRequest *self, const char *fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );
    rc = KClientHttpRequestVAddPostParam ( self, fmt, args );
    va_end ( args );

    return rc;
}

static
unsigned char ToHex ( char ch )
{
    assert ( ch <= 16 );
    if ( ch < 10 )
    {
        return '0' + ch;
    }
    return 'a' + ch - 10;
}

static
void UrlEncodeChar ( unsigned char ch, char * target )
{
    target [ 0 ] = '%';
    target [ 1 ] = ToHex ( ch >> 4 );
    target [ 2 ] = ToHex ( ch & 0x0f );
}

static
rc_t
UrlEncode( const char * source, size_t size, char ** res )
{   /* source: https://www.tutorialspoint.com/html/html_url_encoding.htm */
    char * cur;
    size_t i = 0;
    assert ( source != NULL );
    assert ( res != NULL );

    * res = ( char * ) malloc ( size * 3 + 1 );
    if ( * res == NULL )
    {
        return RC ( rcNS, rcString, rcAllocating, rcMemory, rcExhausted );
    }
    cur = * res;
    for ( i = 0; i < size; ++i )
    {
        unsigned char ch = source [ i ];
        if ( ch < 32 || ch >= 127 )
        {   /* control and non-ASCII characters */
            UrlEncodeChar ( ch, cur );
            cur += 3;
        }
        else
        {
            switch ( ch )
            {
                /* reserved characters */
            case '$':
            case '&':
            case '+':
            case ',':
            case '/':
            case ':':
            case ';':
            case '=':
            case '?':
            case '@':
                /* unsafe characters */
            case ' ':
            case '"':
            case '<':
            case '>':
            case '#':
            case '%':
            case '{':
            case '}':
            case '|':
            case '\\':
            case '^':
            case '~':
            case '[':
            case ']':
            case '`':
                UrlEncodeChar ( ch, cur );
                cur += 3;
                break;
            default:
                * cur = ch;
                ++ cur;
                break;
            }
        }
    }

    * cur = 0;
    return 0;
}

LIB_EXPORT rc_t CC KClientHttpRequestVAddQueryParam ( KClientHttpRequest *self,
                                const char * name, const char *fmt, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
    else if ( fmt == NULL || fmt [ 0 ] == 0 )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    else
    {
        KDataBuffer valueBuf;
        bool first = self -> url_block.query.size == 0;
        bool hasName = ( name != NULL && name [ 0 ] != 0 );
        rc = KDataBufferMakeBytes( & valueBuf, 0 );
        if ( rc == 0 )
        {
            rc = KDataBufferVPrintf ( & valueBuf, fmt, args );
            if ( rc == 0 )
            {
                char * encValue;
                rc = UrlEncode( (const char *) valueBuf.base,
                                valueBuf.elem_count - 1,
                                & encValue );
                if ( rc == 0 )
                {
                    KDataBuffer newBuf;
                    rc = KDataBufferMakeBytes( & newBuf, 0 );
                    if ( rc == 0 )
                    {
                        rc = KDataBufferPrintf ( & newBuf, "%.*s%c%s%s%s",
                                                (int) self -> url_buffer . elem_count,
                                                (const char*) self -> url_buffer . base,
                                                first ? '?' : '&',
                                                hasName ? name : "",
                                                hasName ? "=" : "",
                                                encValue);
                        if ( rc == 0 )
                        {
                            KDataBufferWhack( & self -> url_buffer );
                            self -> url_buffer = newBuf;
                            /* re-parse the new URL */
                            rc = ParseUrl ( & self -> url_block,
                                (char*) self -> url_buffer . base,
                                self -> url_buffer . elem_count - 1 );
                        }
                    }
                    free ( encValue );
                }
            }
            KDataBufferWhack( & valueBuf );
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC KClientHttpRequestAddQueryParam ( KClientHttpRequest *self, const char * name, const char *fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );
    rc = KClientHttpRequestVAddQueryParam ( self, name, fmt, args );
    va_end ( args );

    return rc;
}

rc_t KClientHttpRequestUrlEncodeBase64(const String ** encoding) {
    int n = 0;
    size_t i = 0;

    if (encoding == NULL || *encoding == NULL || (*encoding)->addr == NULL)
        return 0;

    for (i = 0; i < (*encoding)->size; ++i) {
        if (((*encoding)->addr)[i] == '+' ||
            ((*encoding)->addr)[i] == '/')
        {
            ++n;
        }
    }

    if (n > 0) {
        size_t iFrom = 0, iTo = 0;
        const char *from = (*encoding)->addr;
        char *to = NULL;
        uint32_t len = (*encoding)->size + n + n;

        String * encoded = (String *) calloc(1, sizeof * encoded + len + 1);
        if (encoded == NULL)
            return RC(rcNS, rcString, rcAllocating, rcMemory, rcExhausted);

        to = (char*)(encoded + 1);
        StringInit(encoded, to, len, len);

        for (iFrom = 0; iFrom < (*encoding)->size; ++iFrom) {
            if (from[iFrom] == '+') {
                to[iTo++] = '%';
                to[iTo++] = '2';
                to[iTo++] = 'b';
            }
            else if (from[iFrom] == '/') {
                to[iTo++] = '%';
                to[iTo++] = '2';
                to[iTo++] = 'f';
            }
            else
                to[iTo++] = from[iFrom];
        }
        to[iTo] = '\0';
        assert(iTo == len);

        StringWhack(*encoding);
        *encoding = encoded;
    }

    return 0;
}

LIB_EXPORT rc_t CC KClientHttpRequestAddPostFileParam ( KClientHttpRequest * self, const char * name, const char * path )
{
    rc_t rc = 0;
    if ( self == NULL )
    {
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
    }
    else if ( name == NULL )
    {
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    }
    else if ( path == NULL || path [ 0 ] == 0 )
    {
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    }
    else
    {
        KDirectory *wd;
        rc = KDirectoryNativeDir ( & wd );
        if ( rc == 0 )
        {
            rc_t rc2;
            const KFile * file = NULL;
            rc = KDirectoryOpenFileRead( wd, & file, path);
            if ( rc == 0 )
            {
                uint64_t fileSize;
                rc = KFileSize( file, & fileSize );
                if ( rc == 0 )
                {   /* encode file contents */
                    if ( fileSize > 0 )
                    {
                        const void * fileStart;
                        const KMMap * mm;
                        rc = KMMapMakeRead( & mm, file );
                        if ( rc == 0 )
                        {
                            rc = KMMapAddrRead( mm, & fileStart );
                            if ( rc == 0 )
                            {
                                const String * encoded = NULL;
                                rc = encodeBase64( & encoded, fileStart, fileSize );
                                if ( rc == 0 )
                                    rc = KClientHttpRequestUrlEncodeBase64( & encoded );
                                if ( rc == 0 )
                                {
                                    rc = KClientHttpRequestAddPostParam( self, "%s=%S", name, encoded );
                                    StringWhack ( encoded );
                                }
                            }

                            rc2 = KMMapRelease( mm );
                            if ( rc == 0 )
                            {
                                rc = rc2;
                            }
                        }
                    }
                    else
                    {
                        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcEmpty );
                    }
                }

                rc2 = KFileRelease( file );
                if ( rc == 0 )
                {
                    rc = rc2;
                }
            }
            rc2 = KDirectoryRelease( wd );
            if ( rc == 0 )
            {
                rc = rc2;
            }
        }
    }

    return rc;
}

static EUriForm EUriFormGuess ( const String * hostname,
                                uint32_t uriForm,
                                EUriForm uf /* cached Request-URI */ )
{
    assert ( hostname );

    switch ( uriForm ) {
        case 0: /* used in POST requests: use absoluteURI form */
            return eUFAbsolute;

        default:
/*          no break here ; */

        case 2: /* retry after receiving 400 */
            if ( uf != eUFUndefined ) /* switch the previous URI form */
                return uf == eUFAbsolute ? eUFOrigin : eUFAbsolute;
/*          else no break here ; */

        case 1:
            if ( uf != eUFUndefined )
                return uf; /* reuse cached uriForm */
            else { /* first call; guess uriForm by hostnae */
                String googleapis;
                CONST_STRING ( & googleapis, "storage.googleapis.com" );
                if ( StringEqual ( & googleapis, hostname ) )
                    return eUFOrigin;
                else
                    return eUFAbsolute;
            }
    }
}

static rc_t KClientHttpRequestFormatMsgBegin (
    const KClientHttpRequest * self, struct KDataBuffer * buffer,
    const char * method, uint32_t uriForm )
{
    rc_t rc = 0;
    const char * has_query = NULL;
    String hostname;
    KClientHttp * http = NULL;
    assert ( self && self -> http );
    http = self -> http;

    /* determine if there is a query */
    has_query = ( self -> url_block . query . size == 0 ) ? "" : "?";

    /* there are 2 places where the can be a host name stored
       we give preference to the one attached to the url_block, because
       it is the most recently determined.
       If that one is empty, we look at the http object for its
       host name.
       Error if both are empty */
    hostname = self -> url_block . host;
    if ( hostname . size == 0 )
    {
        hostname = http -> hostname;
        if ( hostname . size == 0 )
            return RC ( rcNS, rcNoTarg, rcValidating, rcName, rcEmpty );
    }
    if ( ! http -> proxy_ep )
    {   /* direct connection */
        rc = KDataBufferPrintf ( buffer,
                             "%s %S%s%S HTTP/%.2V\r\nHost: %S\r\n"
                             , method
                             , & self -> url_block . path
                             , has_query
                             , & self -> url_block . query
                             , http -> vers
                             , & hostname
            );
    }
    else { /* using proxy */
        http -> uf = EUriFormGuess ( & hostname, uriForm, http -> uf );
        if ( http -> uf == eUFOrigin ) {
        /* the host does not like absoluteURI: use abs_path ( origin-form ) */
            rc = KDataBufferPrintf ( buffer,
                             "%s %S%s%S HTTP/%.2V\r\nHost: %S:%u\r\n"
                             , method
                             , & self -> url_block . path
                             , has_query
                             , & self -> url_block . query
                             , http -> vers
                             , & hostname
                             , http -> port
                );
        }
        else if ( http -> port != 80 ) { /* absoluteURI: non-default port */
            rc = KDataBufferPrintf ( buffer,
                             "%s %S://%S:%u%S%s%S HTTP/%.2V\r\nHost: %S\r\n"
                             , method
                             , & self -> url_block . scheme
                             , & hostname
                             , http -> port
                             , & self -> url_block . path
                             , has_query
                             , & self -> url_block . query
                             , http -> vers
                             , & hostname
                );
        }
        else {                           /* absoluteURI: default port */
            rc = KDataBufferPrintf ( buffer,
                             "%s %S://%S%S%s%S HTTP/%.2V\r\nHost: %S\r\n"
                             , method
                             , & self -> url_block . scheme
                             , & hostname
                             , & self -> url_block . path
                             , has_query
                             , & self -> url_block . query
                             , http -> vers
                             , & hostname
                );
        }
    }

    return rc;
}

static
rc_t
FormatForCloud( const KClientHttpRequest *cself, const char *method )
{   /* Discover whether the target URL is in a cloud and
        if it is, apply the corresponding cloud's formatting */
    rc_t rc = 0;

    size_t skip = 0;
    const String * hostname = & cself->url_block.host;
    CloudProviderId cpId = cloud_provider_none;

    CloudMgr * cloudMgr = cself->http->mgr->cloud;

    String stor31;

    String aws;
    String gcp;
    CONST_STRING(&aws, "169.254.169.254");
    CONST_STRING(&gcp, "metadata.google.internal");

    if ( StringEqual(hostname, &aws) || StringEqual(hostname, &gcp))
        return 0;

    if ( cloudMgr == NULL )
        rc = CloudMgrMake ( & cloudMgr, NULL, cself->http->mgr );

    if ( rc != 0 )
        return rc;

    CONST_STRING(&stor31, "s3-stor31.st-va.ncbi.nlm.nih.gov");
    skip = hostname->size - stor31.size;
    if (hostname->size >= stor31.size &&
        string_cmp(stor31.addr, stor31.size, hostname->addr + skip,
            hostname->size - skip, stor31.size) == 0)
    {
        cpId = cloud_provider_aws;
    }
    else
    {
        String amazonaws;
        CONST_STRING(&amazonaws, "amazonaws.com");
        skip = hostname->size - amazonaws.size;
        if (hostname->size >= amazonaws.size &&
            string_cmp(amazonaws.addr, amazonaws.size,
                hostname->addr + skip, hostname->size - skip,
                amazonaws.size) == 0)
        {
            cpId = cloud_provider_aws;
        }
        else {
            String google;
            CONST_STRING(&google, "storage.googleapis.com");
            skip = 0;
            if (hostname->size >= google.size &&
                string_cmp(google.addr, google.size,
                    hostname->addr + skip, hostname->size - skip,
                    google.size) == 0)
            {
                cpId = cloud_provider_gcp;
            }
        }
    }

    if ( cpId == cloud_provider_none )
        CloudMgrCurrentProvider ( cloudMgr, & cpId );

    {
        const char * e = getenv("NCBI_VDB_PROVIDER");
        if (e != NULL && e[0] != '\0') {
            CloudProviderId i = cloud_provider_none;
            i = atoi(e);
            if (i != cloud_provider_none)
                cpId = i;
        }
    }

    if ( cpId != cloud_provider_none && ( cself->ceRequired || cself->payRequired ) )
    {   /* add cloud authentication informantion if required */
        if ( rc == 0 )
        {
            /* create a cloud object based on the target URL */
            Cloud * cloud ;
            KClientHttpRequest * self = (KClientHttpRequest *)cself;
            rc = CloudMgrMakeCloud ( cloudMgr, & cloud, cpId );
            if (rc == 0) {
                if (cself->payRequired)
                    rc = CloudAddUserPaysCredentials(cloud, self, method);
                else if (cself->ceRequired)
                    rc = CloudAddComputeEnvironmentTokenForSigner(
                        cloud, self);
                CloudRelease ( cloud );
            }
        }
    }

    if ( cloudMgr != cself->http->mgr->cloud )
        CloudMgrRelease ( cloudMgr );

    return rc;
}

static
rc_t CC KClientHttpRequestFormatMsgInt( const KClientHttpRequest *self,
    struct KDataBuffer * buffer, const char *method,
    uint32_t uriForm )
{
    rc_t rc;
    rc_t r2 = 0;
    bool have_user_agent = false;
    bool have_accept = false;
    String user_agent_string;
    String accept_string;
    const KHttpHeader *node;

    if ( self == NULL ) {
         return RC ( rcNS, rcNoTarg, rcReading, rcSelf, rcNull );
    }
    if ( buffer == NULL ) {
         return RC ( rcNS, rcNoTarg, rcReading, rcParam, rcNull );
    }

    CONST_STRING ( &user_agent_string, "User-Agent" );
    CONST_STRING ( &accept_string, "Accept" );

    assert(method);
    if (method[0] != 'P') {
        /* POST is formatted for cloud inside of KClientHttpRequestPOST_Int */
        rc = FormatForCloud(self, method);
        if (rc != 0)
            return rc;
    }

    /* start building the buffer that will be sent
       We are inlining the host:port, instead of
       sending it in its own header */

    rc = KClientHttpRequestFormatMsgBegin( self, buffer, method, uriForm );

    /* print all headers remaining into buffer */
    for ( node = ( const KHttpHeader* ) BSTreeFirst ( & self -> hdrs );
          rc == 0 && node != NULL;
          node = ( const KHttpHeader* ) BSTNodeNext ( & node -> dad ) )
    {
        /* look for "User-Agent" */
        if ( !have_user_agent && node -> name . len == 10 )
        {
            if ( StringCaseCompare ( & node -> name, & user_agent_string ) == 0 )
                have_user_agent = true;
        }
        /* look for "Accept" */
        else if (!have_accept && node->name.len == 6) {
            if (StringCaseCompare(&node->name, &accept_string) == 0)
                have_accept = true;
        }

        /* add header line */
        rc = KDataBufferPrintf ( buffer, "%S: %S\r\n"
                                , & node -> name
                                , & node -> value );
    }

    /* add an User-Agent header from the kns-manager if we did not find one already in the header tree */
    if ( !have_user_agent )
    {
        const char * ua = NULL;
        rc_t r3 = KNSManagerGetUserAgent ( &ua );
        if ( r3 == 0 )
        {
            r2 = KDataBufferPrintf ( buffer, "User-Agent: %s\r\n", ua );
            if ( rc == 0 )
            {
                rc = r2;
            }
        }
    }

    if (!have_accept) {
        r2 = KDataBufferPrintf(buffer, "Accept: */*\r\n");
        if (rc == 0 && r2 != 0)
            rc = r2;
    }

    /* add terminating empty header line */
    if ( rc == 0 )
    {
        rc = KDataBufferPrintf ( buffer, "\r\n" );
    }

    return rc;
}

LIB_EXPORT
rc_t CC KClientHttpRequestFormatMsg(const KClientHttpRequest *self,
    struct KDataBuffer * buffer, const char *method)
{
    return KClientHttpRequestFormatMsgInt(self,
        buffer, method, 1);
}

LIB_EXPORT
rc_t CC KClientHttpRequestFormatPostMsg(const KClientHttpRequest *self,
    struct KDataBuffer * buffer)
{
    return KClientHttpRequestFormatMsgInt(self,
        buffer, "POST", 0);
}

static
rc_t KClientHttpRequestHandleRedirection ( KClientHttpRequest *self, const char *method, KClientHttpResult const *const rslt, char ** expiration )
{
    rc_t rc = 0;
    KHttpHeader *loc;
    KHttpHeader *exp;

    { /* find relocation URI */
        String Location;
        CONST_STRING ( & Location, "Location" );
        loc = ( KHttpHeader* ) BSTreeFind ( & rslt -> hdrs, & Location, KHttpHeaderCmp );
        if ( loc == NULL )
        {
            LOGERR ( klogSys, rc, "Location header not found on relocate msg" );
            return RC ( rcNS, rcNoTarg, rcValidating, rcNode, rcNull );
        }
    }

    {
        /* find expiration time if any */
        String Expiration;
        CONST_STRING ( & Expiration, "Expires" );
        exp = ( KHttpHeader* ) BSTreeFind ( & rslt -> hdrs, & Expiration, KHttpHeaderCmp );
    }

    /* capture the new URI in loc -> value_storage */
    if ( loc -> value . size == 0 )
    {
        LOGERR ( klogSys, rc, "Location does not provide a value" );
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcNode, rcIncorrect );
    }
    else
    {
        URLBlock b;
        KDataBuffer uri;

        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_HTTP), ("Redirected from '%s'\n", (const char*) self -> url_buffer . base ) );
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_HTTP), ("Redirected to '%S'\n", & loc -> value ) );
        if ( exp != NULL )
        {
            DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_HTTP), ("'To' URL expires at '%S'\n", & exp -> value ) );
            * expiration = string_dup( exp -> value . addr, exp -> value . size );
        }

        /* pull out uri */
        rc = KDataBufferSub ( &loc -> value_storage, &uri, loc -> name . size, loc -> value . size + 1 );
        if ( rc == 0 )
        {
            /* parse the URI into local url_block */
            rc = ParseUrl ( &b, (char *) uri . base, uri . elem_count - 1 );
            if ( rc == 0 )
            {
                KClientHttp *http = self -> http;

                /* close the open http connection and clear out all data except for the manager */
                KClientHttpClear ( http );

                /* clear the previous endpoint */
                http -> ep_valid = false;

                /* reinitialize the http from uri */
                rc = KClientHttpInit ( http, &uri, http -> vers , &b . host, b . port, b . tls );
                if ( rc == 0 )
                {
                    KClientHttpRequestClear ( self );
                    rc = KClientHttpRequestInit ( self, &b, &uri );
                    if ( rc == 0 )
                    {
                        self -> ceRequired = false; /* once redirected, CE token is not needed */
                        rc = FormatForCloud ( self, method );
                        KClientHttpResultRelease ( rslt );
                    }
                }
            }

            KDataBufferWhack ( & uri );
        }

    }

    return rc;
}

static
rc_t KClientHttpRequestSendReceiveNoBodyInt ( KClientHttpRequest *self, KClientHttpResult **_rslt, const char *method )
{
    rc_t rc = 0;
    uint32_t i;
    const uint32_t max_redirect = 5;
    char * expiration = NULL;

    /* TBD - may want to prevent a Content-Type or other headers here */

    if ( self -> body . elem_count != 0 )
        return RC ( rcNS, rcNoTarg, rcValidating, rcNoObj, rcIncorrect );

    for ( i = 0; i < max_redirect; ++ i )
    {
        uint32_t uriForm = 1;

        KClientHttpResult *rslt;

        KDataBuffer buffer;
        rc = KDataBufferMake( & buffer, 8, 0 );
        if ( rc != 0 )
            break;

        /* create message */
        rc = KClientHttpRequestFormatMsgInt ( self, & buffer, method, uriForm );
        if ( rc != 0 )
            break;

        /* send the message and create a response */
        rc = KClientHttpSendReceiveMsg ( self -> http, _rslt,
            (char *) buffer.base,
            buffer.elem_count - 1, NULL, (char *) self -> url_buffer . base );
        if ( rc != 0 )
        {
            KClientHttpClose ( self -> http );
            rc = KClientHttpSendReceiveMsg ( self -> http, _rslt,
                (char *) buffer.base, buffer.elem_count - 1, NULL,
                (char *) self -> url_buffer . base );
            if ( rc != 0 )
                break;
        }

        rslt = * _rslt;
        rslt -> expiration = expiration; /* expiration has to reach the caller */
        expiration = NULL;

        /* look at status code */
        switch ( rslt -> status )
        {
        case 200:
        case 206:
            return 0;
        case 304:
            /* check for "If-Modified-Since" or "If-None-Match" header in request and allow if present */
            if ( self -> accept_not_modified )
                return 0;
            break;
        }

        TRACE ( "unusual status code: %d\n", ( int ) rslt -> status );

        switch ( rslt -> status )
        {
            /* TBD - need to include RFC rule for handling codes for HEAD and GET */
        case 301: /* "moved permanently" */
        case 302: /* "found" - okay to reissue for HEAD and GET, but not for POST */
        case 303: /* "see other" - the response to the request can be found under another URI using a GET method */
        case 308: /* "permanent redirect" */
        case 307: /* "moved temporarily" */
            break;

        case 505: /* HTTP Version Not Supported */
            if ( self -> http -> vers > 0x01000000 )
            {
                /* downgrade version requested */
                self -> http -> vers -= 0x00010000;
                /* TBD - remove any HTTP/1.1 specific headers */
                continue;
            }

            /* NO BREAK */

        case 400:
            if ( uriForm == 1 ) {
                ++ uriForm; /* got 400; try to use different Request-URI form */
                continue;
            }
/*          else no break here: tried both Request-URI forms */

        default:

            if ( ! rslt -> len_zero || self -> http -> close_connection )
            {
                /* the connection is no good */
                KClientHttpClose ( self -> http );
            }

            /* rslt -> status may be looked at by the caller to determine actual success */
            return 0;
        }

        rc = KClientHttpRequestHandleRedirection ( self, method, rslt, & expiration );
        if ( rc != 0 )
            break;
    }

    if ( rc != 0 )
        KClientHttpClose ( self -> http );

    return rc;
}

static
rc_t KClientHttpRequestSendReceiveNoBody ( KClientHttpRequest *self, KClientHttpResult **_rslt, const char *method )
{
    KHttpRetrier retrier;
    rc_t rc = KHttpRetrierInit ( & retrier,
        (char *) self -> url_buffer . base, self -> http -> mgr );

    if ( rc == 0 )
    {
        while ( rc == 0 )
        {
            rc = KClientHttpRequestSendReceiveNoBodyInt ( self, _rslt, method );
            if ( rc != 0 )
            {   /* a non-HTTP problem */
                break;
            }
            if ( ! self -> http -> reliable || ! KHttpRetrierWait ( & retrier, ( * _rslt ) -> status ) )
            {   /* We are either not configured to retry, or HTTP status is not retriable, or we exhausted
                    the max number of retries or the total wait time.
                    rc is 0, but the caller will have to look at _rslt->status to determine success */
                break;
            }
            KClientHttpResultRelease ( * _rslt );
        }

        {
            rc_t rc2 = KHttpRetrierDestroy ( & retrier );
            if ( rc == 0 )
                rc = rc2;
        }
    }

    return rc;
}

/* HEAD
 *  send HEAD message
 */
LIB_EXPORT rc_t CC KClientHttpRequestHEAD ( KClientHttpRequest *self, KClientHttpResult **rslt )
{
    rc_t rc=0;

    if ( self -> ceRequired || self -> payRequired )
    {   /* use POST or GET for 256 bytes */
        /* update UserAgent with -head */
        KNSManagerSetUserAgentSuffix("-head");

        char buf [ 256 ];

        /* add header "Range bytes = 0,HeadSize" */
        rc = KClientHttpRequestByteRange ( self, 0, sizeof buf );
        if ( rc == 0 )
        {
            rc = self -> ceRequired ? KClientHttpRequestPOST ( self, rslt ) : KClientHttpRequestGET ( self, rslt );
            if ( rc == 0 )
            {
                uint64_t result_size64 = sizeof buf;
                KStream * response;

                /* extractSize */
                KClientHttpResultSize ( *rslt, & result_size64 );

                if ( result_size64 > sizeof buf ) /* unlikely but would be very unpleasant */
                {
                    result_size64 = sizeof buf;
                }

                /* consume and discard result_size64 bytes */
                rc = KClientHttpResultGetInputStream ( *rslt, & response );
                if ( rc == 0 )
                {
                    rc = KStreamTimedReadExactly ( response, buf, result_size64, NULL );
                    KStreamRelease ( response );
                }
            }
        }
        else
        {
            rc = RC ( rcNS, rcString, rcAllocating, rcMemory, rcExhausted );
        }

        /* Restore UserAgent */
        KNSManagerSetUserAgentSuffix("");
        return rc;
    }
    else
    {
        return KClientHttpRequestSendReceiveNoBody ( self, rslt, "HEAD" );
    }
}

/* GET
 *  send GET message
 *  all query AND post parameters are combined in URL
 */
LIB_EXPORT rc_t CC KClientHttpRequestGET ( KClientHttpRequest *self, KClientHttpResult **rslt )
{
    return KClientHttpRequestSendReceiveNoBody ( self, rslt, "GET" );
}

rc_t CC KClientHttpRequestPOST_Int ( KClientHttpRequest *self, KClientHttpResult **_rslt )
{
    rc_t rc = 0;

    KClientHttpResult *rslt;

    uint32_t i;
    const uint32_t max_redirect = 5;

    char * expiration = NULL;
    const char * method = "POST";

    rc = FormatForCloud(self, method);
    if (rc != 0)
        return rc;

    /* fix headers for POST params */
    if ( self -> body . elem_count > 1 )
    {
        /* "body" contains data plus NUL byte */
        rc = KClientHttpReplaceHeader ( & self -> hdrs,
            "Content-Length", "%lu", self -> body . elem_count - 1 );
        if ( rc == 0 )
        {
            String Content_Type;
            const KHttpHeader *node;

            CONST_STRING ( & Content_Type, "Content-Type" );

            node = ( const KHttpHeader* ) BSTreeFind ( & self -> hdrs, & Content_Type, KHttpHeaderCmp );
            if ( node == NULL )
            {
                /* add content type for form parameters */
                /* TBD - before general application, need to perform URL-encoding! */
                rc = KClientHttpAddHeader ( & self -> hdrs, "Content-Type", "application/x-www-form-urlencoded" );
            }
        }

        if ( rc != 0 )
        {
            KClientHttpClose ( self -> http );
            return rc;
        }
    }

    for ( i = 0; i < max_redirect; ++ i )
    {
        const KDataBuffer *body = & self -> body;
        KDataBuffer buffer;
        rc = KDataBufferMake( & buffer, 8, 0 );
        if ( rc != 0 )
            break;

        /* create message */
        rc = KClientHttpRequestFormatMsgInt ( self, & buffer, method, 0 );
        if ( rc != 0 )
        {
            KDataBufferWhack( & buffer );
            break;
        }

        if (body != NULL && body -> base != NULL && body -> elem_count > 0 )
        {   /* Try to add body to buffer to avoid double socket write */
            uint64_t len = buffer.elem_count;
            rc = KDataBufferResize( & buffer, buffer.elem_count + body -> elem_count - 1 );
            if ( rc == 0 )
            {   // insert before 0-terminator
                memmove( (char*)buffer.base + len - 1, body -> base, body -> elem_count );
                body = NULL;
            }
        }

        /* send the message and create a response */
        rc = KClientHttpSendReceiveMsg ( self -> http, _rslt,
            (char *) buffer.base, buffer.elem_count, body,
            (char *) self -> url_buffer . base );
        if ( rc != 0 )
        {
            KClientHttpClose ( self -> http );
            rc = KClientHttpSendReceiveMsg ( self -> http, _rslt,
                (char *) buffer.base, buffer.elem_count, NULL,
                (char *) self -> url_buffer . base );
        }
        KDataBufferWhack( & buffer );
        if ( rc != 0 )
        {
            break;
        }

        rslt = * _rslt;
        rslt -> expiration = expiration; /* expiration has to reach the caller */
        expiration = NULL;

        /* look at status code */
        switch ( rslt -> status )
        {
        case 200:
        case 206:
            return 0;
        case 304:
            /* check for "If-Modified-Since" or "If-None-Match" header in request and allow if present */
            if ( self -> accept_not_modified )
                return 0;
            break;
        }

        TRACE ( "unusual status code: %d\n", ( int ) rslt -> status );

        switch ( rslt -> status )
        {
            /* TBD - Add RFC rules about POST */
        case 301: /* "moved permanently" */
        case 308: /* "permanent redirect" */
            break;
        case 307: /* "moved temporarily" */
            method = "GET";
            rc = KDataBufferResize ( & self -> body , 0 ); /* drop POST parameters */
            if (rc == 0 )
            {
                rc = KClientHttpReplaceHeader ( & self -> hdrs, "Content-Length", "0" );
                if ( rc == 0 )
                {
                   String Content_Type;
                   BSTNode *node;

                   CONST_STRING ( & Content_Type, "Content-Type" );

                   node = BSTreeFind ( & self -> hdrs, & Content_Type, KHttpHeaderCmp );
                   if ( node != NULL )
                   {
                      BSTreeUnlink(&self->hdrs, node);
                      /*TODO: free (node) ? */
                   }
                }
             }
             break;

        case 505: /* HTTP Version Not Supported */
            if ( self -> http -> vers > 0x01000000 )
            {
                /* downgrade version requested */
                self -> http -> vers -= 0x00010000;
                /* TBD - remove any HTTP/1.1 specific headers */
                continue;
            }

            /* NO BREAK */

        default:

      /*    if ( ! rslt -> len_zero || self -> http -> close_connection ) */
            if ( false )
            {
                /* the connection is no good */
                KClientHttpClose ( self -> http );
            }

            /* rslt -> status may be looked at by the caller to determine actual success */
            return 0;
        }

        /* reset connection, reset request */
        rc = KClientHttpRequestHandleRedirection ( self, method, rslt, & expiration );
        if ( rc != 0 )
            break;
    }

    if ( rc != 0 )
        KClientHttpClose ( self -> http );

    return rc;
}



typedef enum {
    eUPSBegin,
    eUPSHost,
    eUPSDone,
} EUrlParseState;
static bool GovSiteByHttp ( const char * path ) {
    if ( path != NULL ) {
        size_t path_size = string_measure ( path, NULL );
        size_t size = 0;
        String http;
        CONST_STRING ( & http, "http://" );
        size = http . size;

        /* resolver-cgi is called over http */
        if ( path_size > size &&
             strcase_cmp ( path, size, http . addr, size, size ) == 0 )
        {
            EUrlParseState state = eUPSBegin;
            unsigned i = 0;
            for ( i = 7; i < path_size && state != eUPSDone; ++i ) {
                switch ( state ) {
                    case eUPSBegin:
                        if ( path [ i ] != '/' ) {
                            state = eUPSHost;
                        }
                        break;
                    case eUPSHost:
                        if ( path [ i ] == '/' ) {
                            state = eUPSDone;
                        }
                        break;
                    default:
                        break;
                }
            }

            if ( state == eUPSBegin ) {
                return false;
            }
            else {
                size_t size = 0;
                String gov;
                CONST_STRING ( & gov, ".gov" );
                size = gov . size;
                if ( strcase_cmp
                    ( path + i - 5, size, gov . addr, size, size ) == 0 )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

/* POST
 *  send POST message
 *  query parameters are sent in URL
 *  post parameters are sent in body
 */
LIB_EXPORT rc_t CC KClientHttpRequestPOST ( KClientHttpRequest *self, KClientHttpResult **_rslt )
{
    KHttpRetrier retrier;
    rc_t rc = 0;

    if ( self == NULL ) {
        return RC ( rcNS, rcNoTarg, rcUpdating, rcSelf, rcNull );
    }
    if ( _rslt == NULL ) {
        return RC ( rcNS, rcNoTarg, rcUpdating, rcParam, rcNull );
    }

    rc = KHttpRetrierInit ( & retrier, (char *) self -> url_buffer . base,
        self -> http -> mgr );

    if ( rc == 0 )
    {
        while ( rc == 0 )
        {
            rc = KClientHttpRequestPOST_Int ( self, _rslt );
            if ( rc != 0 )
            {   /* a non-HTTP problem */
                break;
            }

            assert ( * _rslt );

            if ( ( * _rslt ) -> status  == 403 &&
                 GovSiteByHttp ((char *) self -> url_buffer . base ) )
            {
                break;
            }

            if ( ! self -> http -> reliable || ! KHttpRetrierWait ( & retrier, ( * _rslt ) -> status ) )
            {   /* We are either not configured to retry, or HTTP status is not retriable, or we exhausted
                    the max number of retries or the total wait time.
                    rc is 0, but the caller will have to look at _rslt->status to determine success */
                break;
            }

            KClientHttpResultRelease ( * _rslt );
        }

        {
            rc_t rc2 = KHttpRetrierDestroy ( & retrier );
            if ( rc == 0 ) rc = rc2;
        }
    }

    return rc;
}

