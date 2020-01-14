/* TODO: move it to interfaces/klib/strings.h */
#define MAGIC_PAY_REQUIRED "NCBI_VDB_PAY_REQUIRED"

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
#define KSTREAM_IMPL KClientHttpStream

typedef struct KClientHttpStream KClientHttpStream;

#include <kns/manager.h>
#include <kns/socket.h>
#include <kns/stream.h>
#include <kns/impl.h>

#include <klib/debug.h>
#include <klib/text.h>
#include <klib/out.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <klib/printf.h>
#include <klib/status.h>
#include <klib/data-buffer.h>
#include <kproc/timeout.h>

#include <strtol.h>

#include "http-priv.h"
#include "mgr-priv.h"
#include "stream-priv.h"

#include <ctype.h>

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

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

#define KDataBufferClear(buf) KDataBufferMakeBytes(buf, 0)

#if _DEBUGGING
static
bool KDataBufferContainsString ( const KDataBuffer *buf, const String *str )
{
    return str -> addr >= ( const char* ) buf -> base &&
        & str -> addr [ str -> size ] <= & ( ( const char* ) buf -> base ) [ buf -> elem_count ];
}
#endif


/*--------------------------------------------------------------------------
 * KClientHttp
 *  hyper text transfer protocol
 *  structure that will act as the 'client' for networking tasks
 */

#define KClientHttpBlockBufferIsEmpty( self ) \
    ( ( self ) -> block_read == ( self ) -> block_valid )

#define KClientHttpBlockBufferReset( self ) \
    ( ( void ) ( ( self ) -> block_valid = ( self ) -> block_read = 0 ) )

#define KClientHttpLineBufferReset( self ) \
    ( ( void ) ( ( self ) -> line_valid = 0 ) )

void KClientHttpClose ( KClientHttp *self )
{
    KStreamRelease ( self -> sock );
    self -> sock = NULL;

    KClientHttpBlockBufferReset ( self );
    KClientHttpLineBufferReset ( self );
#if 0
    TRACE ( "closed connection%c", '\n' );
#endif
}


/* used to be in whack function, but we needed the ability
   to clear out the http object for redirection */
rc_t KClientHttpClear ( KClientHttp *self )
{
    KClientHttpClose ( self );

    KDataBufferWhack ( & self -> hostname_buffer );
    self -> tls = false;

    return 0;
}

static
rc_t KClientHttpWhack ( KClientHttp * self )
{
    if ( self -> close_connection )
    {
        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS ),
            ("*** closing connection ***\n"));
        KClientHttpClose ( self );
        self -> close_connection = false;
    }

    KClientHttpClear ( self );

    KStreamRelease ( self -> test_sock );
    self -> test_sock = NULL;

    KDataBufferWhack ( & self -> block_buffer );
    KDataBufferWhack ( & self -> line_buffer );
    KNSManagerRelease ( self -> mgr );
    KRefcountWhack ( & self -> refcount, "KClientHttp" );
    free ( self );

    return 0;
}


typedef struct KEndPointArgsIterator
{
    struct KNSProxies * proxies;
    size_t crnt;
    bool last;

    const String * directHostname;
    uint16_t directPort;

    uint16_t dflt_proxy_ports [ 3 ];
    size_t dflt_proxy_ports_idx;

    const String * hostname;
    uint16_t port;
    bool dontAskForNextProxy;

    bool done;
} KEndPointArgsIterator;

static
void KEndPointArgsIteratorMake ( KEndPointArgsIterator * self,
    const KNSManager * mgr, const String * hostname, uint16_t port,
    size_t * cnt  )
{
    assert ( self );
    memset ( self, 0, sizeof * self );

    self -> dflt_proxy_ports [ 0 ] = 3128;
    self -> dflt_proxy_ports [ 1 ] = 8080;
    self -> dflt_proxy_ports [ 2 ] = 80;

    if ( ! KNSManagerHttpProxyOnly ( mgr ) ) {
        self -> directHostname = hostname;
        self -> directPort = port;
    }

    if ( KNSManagerGetHTTPProxyEnabled ( mgr ) )
        self -> proxies = KNSManagerGetProxies ( mgr, cnt );

    if ( self -> directHostname == NULL && self -> proxies == NULL )
        self -> done = true;
}

/* proxy_default_port =
    true : no port in proxy spec, returning one of default ports
    false: port in proxy spec, returning it
   proxy_ep = true: proxy, false: direct connection */
static
bool KEndPointArgsIteratorNext ( KEndPointArgsIterator * self,
    const String ** hostname, uint16_t * port, bool * proxy_default_port,
    bool * proxy_ep, size_t * crnt_proxy_idx, bool * last_proxy )
{
    static const uint16_t dflt_proxy_ports_sz =
        sizeof self -> dflt_proxy_ports / sizeof self -> dflt_proxy_ports [ 0 ];

    bool found = false;
    bool ask = false;

    size_t dummy;
    bool dummy2;
    if ( crnt_proxy_idx == NULL )
        crnt_proxy_idx = & dummy;
    if ( last_proxy == NULL )
        last_proxy = & dummy2;

    assert ( self != NULL );

    if ( self -> done )
        return false;

    assert ( hostname && port && proxy_default_port && proxy_ep );

    /* KNSProxiesGet sets/returns hostname/port */
    /* DO NOT WHACK hostname !!! */
    if ( ! self -> dontAskForNextProxy ) {
        if ( ! self -> last ) {
            /* not asked the last proxy in the list already:
               ask for a next proxy
               ( if (self -> last) then next call to KNSProxiesGet will
                 reset list pointer and return the first proxy again ) */
            ask = true;
        }
        else if ( self -> directHostname == NULL ) {
            /* asked the last proxy in the list
               but we will not return direct connection so
               reset list pointer and return the first proxy again ) */
            ask = true;
        }
        if ( ask ) {
            KNSProxiesGet ( self -> proxies, & self -> hostname, & self -> port,
                            & self -> crnt, & self -> last );
            self -> dontAskForNextProxy = true;
        }
        else if ( self -> last ) {
            /* asked the last proxy in the list: return direct connection */
            self -> hostname = NULL;
        }
    }
    * hostname = self -> hostname;
    * port     = self -> port;

    * proxy_default_port = false;

    if ( * hostname != NULL ) /* KNSProxiesGet returned next proxy */
    {
        if ( * port != 0 ) {
            /* KNSProxiesGet returned proxy with port :
               don't iterate default ports */
            self -> dflt_proxy_ports_idx = 0;
        }
        else
        {   /* no port in proxy spec, return next default port */
            assert ( self -> dflt_proxy_ports_idx < dflt_proxy_ports_sz );
            * port = self -> dflt_proxy_ports [ self -> dflt_proxy_ports_idx ];

            if ( ++ self -> dflt_proxy_ports_idx >= dflt_proxy_ports_sz ) {
                /* done iterating over default ports :
                   ask for a next proxy next time */
                self -> dflt_proxy_ports_idx = 0;
            }

            * proxy_default_port = true;
        }

        if ( self -> dflt_proxy_ports_idx == 0 ) {
            /* KNSProxiesGet returned port or iterated over all default ports:
               ask for a next proxy next time */
            self -> dontAskForNextProxy = false; // ++ self -> idx;
        }

        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_PROXY ),
            ( "Connecting using proxy '%S:%d'\n", * hostname, * port ) );

        * proxy_ep = true;
        found = true;
    }
    else {
        /* no more proxies to iterate */
        self -> done = true;

        if ( self -> directHostname != NULL )
        {   /* allowed to make direct connection without proxies:
               return host:port of direct connection */
            * hostname = self -> directHostname;
            * port = self -> directPort;

            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_PROXY ),
                ( "Connecting directly\n" ) );

            * proxy_ep = false;
            found = true;
        }
        else /* no more proxies to iterate and cannot make direct connection */
            found = false;
    }

    * crnt_proxy_idx = self -> crnt;
    * last_proxy = self -> last;
    return found;
}

static
rc_t KClientHttpGetLine ( KClientHttp *self, struct timeout_t *tm );

static
rc_t KClientHttpProxyConnect ( KClientHttp * self, const String * hostname, uint32_t port, KSocket * sock,
    const String * phostname, uint32_t pport )
{
    rc_t rc;

    STATUS ( STAT_GEEK, "%s - extracting stream from socket\n", __func__ );
    assert ( sock != NULL );
    rc = KSocketGetStream ( sock, & self -> sock );
    if ( rc == 0 )
    {
        uint32_t port_save;
        String hostname_save, hostname_copy;

        KDataBuffer buffer;

        STATUS ( STAT_GEEK, "%s - saving hostname and port\n", __func__ );
        hostname_save = self -> hostname;
        port_save = self -> port;

        STATUS ( STAT_GEEK, "%s - saving hostname param\n", __func__ );
        hostname_copy = * hostname;

        assert ( hostname != NULL );
        STATUS ( STAT_GEEK, "%s - setting hostname and port to '%S:%u'\n", __func__, phostname, pport );
        self -> hostname = * phostname;
        self -> port = pport;

        /* format CONNECT request */
		rc = KDataBufferClear( & buffer );
		if (rc == 0)
		{
			rc = KDataBufferPrintf( & buffer,
				"CONNECT %S:%u HTTP/1.1\r\n"
				"Host: %S:%u\r\n\r\n"
				, &hostname_copy
				, port
				, &hostname_copy
				, port
			);

			if (rc != 0)
				DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS), ("Failed to create proxy request: %R\n", rc));
			else
			{
				size_t sent;
				timeout_t tm;
                                size_t size = buffer.elem_count - 1;

				STATUS(STAT_QA, "%s - created proxy request '%.*s'\n", __func__,
                                    (uint32_t)size, (char*)buffer.base);

				/* send request and receive a response */
				STATUS(STAT_PRG, "%s - sending proxy request\n", __func__);
				TimeoutInit(&tm, self->write_timeout);
				rc = KStreamTimedWriteAll(self->sock, buffer.base, size, &sent, &tm);
				if (rc != 0)
					DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS), ("Failed to send proxy request: %R\n", rc));
				else
				{
					String msg;
					ver_t version;
					uint32_t status;

					assert(sent == size);

					STATUS(STAT_PRG, "%s - reading proxy response status line\n", __func__);
					TimeoutInit(&tm, self->read_timeout);
					rc = KClientHttpGetStatusLine(self, &tm, &msg, &status, &version);
					if (rc != 0)
						DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS), ("Failed to read proxy response: %R\n", rc));
					else
					{
						if ((status / 100) != 2)
						{
							rc = RC(rcNS, rcNoTarg, rcOpening, rcConnection, rcFailed);
							DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS), ("Failed to create proxy tunnel: %03u '%S'\n", status, &msg));
							KClientHttpBlockBufferReset(self);
							KClientHttpLineBufferReset(self);
						}
						else
						{
							STATUS(STAT_QA, "%s - read proxy response status line: %03u '%S'\n", __func__, status, &msg);
							do
								rc = KClientHttpGetLine(self, &tm);
							while (self->line_valid != 0);
						}
					}
				}
			}
		}

        STATUS ( STAT_GEEK, "%s - restoring hostname and port\n", __func__ );
        self -> hostname = hostname_save;
        self -> port = port_save;

        STATUS ( STAT_GEEK, "%s - releasing socket stream\n", __func__ );
        KStreamRelease ( self -> sock );
        self -> sock = NULL;
    }

    return rc;
}


static
rc_t KClientHttpOpen ( KClientHttp * self, const String * aHostname, uint32_t aPort )
{
    rc_t rc = 0;
    KSocket * sock = NULL;
    const String * hostname = NULL;
    uint16_t port = 0;
    bool proxy_default_port = false;
    bool proxy_ep = false;
    KEndPointArgsIterator it;
    const KNSManager * mgr = NULL;

    STATUS ( STAT_QA, "%s - opening socket to %S:%u\n", __func__, aHostname, aPort );

    assert ( self );
    mgr = self -> mgr;
    assert ( mgr );

    KEndPointArgsIteratorMake ( & it, mgr, aHostname, aPort, NULL );
    while ( KEndPointArgsIteratorNext ( & it, & hostname, & port,
        & proxy_default_port, & proxy_ep, NULL, NULL ) )
    {
		/* for an externally provied (likely mocked) stream,
		   protect the stream from overwriting and pretend
		   the first endpoint was connected to successfully */
		if (self->sock == NULL && self->test_sock != NULL)
		{
			self->sock = self->test_sock;
			KStreamAddRef(self->test_sock);
			self->proxy_default_port = proxy_default_port;
			self->proxy_ep = proxy_ep;
			self->ep_valid = true;
			return 0;
		}

        rc = KNSManagerInitDNSEndpoint ( mgr, & self -> ep, hostname, port );
        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_DNS ),
            ( "KNSManagerInitDNSEndpoint(%S:%d)=%R\n", hostname, port, rc ) );
        if ( rc == 0 )
        {
            self -> proxy_default_port = proxy_default_port;
            self -> proxy_ep = proxy_ep;
        }
        if ( rc == 0 )
        {
			/* try to establish a connection */
			rc = KNSManagerMakeTimedConnection(mgr, &sock,
				self->read_timeout, self->write_timeout, NULL, &self->ep);

			/* if we connected to a proxy, try to follow-through to server */
			if (proxy_ep && self->tls && rc == 0)
				rc = KClientHttpProxyConnect(self, aHostname, aPort, sock, hostname, port);

            if ( rc == 0 )
            {
                /* this is a good endpoint */
                self -> ep_valid = true;
                break;
            }

            /* here rc is not 0, so we release the socket */
            KSocketRelease ( sock );
            sock = NULL;
        }
    }

    if ( rc != 0 ) {
        if ( KNSManagerLogNcbiVdbNetError ( mgr ) )
            PLOGERR ( klogSys, ( klogSys, rc, "Failed to Make Connection "
                "in KClientHttpOpen to '$(host):$(port)'",
                "host=%S,port=%hd", aHostname, aPort ) );
    }
    /* if the connection is open */
    else
    {
        rc_t r = KSocketGetLocalEndpoint ( sock, & self -> local_ep );
        if ( r == 0 )
            STATUS ( STAT_USR, "%s - connected from '%s' to %S (%s)\n",
                               __func__, self -> local_ep . ip_address,
                               hostname, self -> ep . ip_address );
        else
            STATUS ( STAT_USR, "%s - connected to %S (%s)\n",
                               __func__, hostname, self -> ep . ip_address );

        if ( self -> tls )
        {
            KTLSStream * tls_stream;

            STATUS ( STAT_PRG, "%s - creating TLS wrapper on socket\n", __func__ );
            rc = KNSManagerMakeTLSStream ( mgr, & tls_stream, sock, aHostname );

            if ( rc != 0 )
            {
                if ( ! proxy_ep ) {
                    if ( KNSManagerLogNcbiVdbNetError ( mgr ) )
                        PLOGERR ( klogSys, ( klogSys, rc,
                            "Failed to create TLS stream for '$(host)' ($(ip)) "
                            "from '$(local)'", "host=%S,ip=%s,local=%s",
                            aHostname, self -> ep . ip_address,
                            self -> local_ep . ip_address
                        ) );
                    else
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_TLS ),
                            ( "Failed to create TLS stream for '%S' (%s) from '%s'\n",
                              aHostname, self -> ep . ip_address,
                              self -> local_ep . ip_address ) );
                }
                else
                {
                    STATUS ( STAT_PRG, "%s - retrying TLS wrapper on socket with proxy hostname\n", __func__ );
                    rc = KNSManagerMakeTLSStream ( mgr, & tls_stream, sock, hostname );
                    if ( rc != 0 )
                        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_TLS ), ( "Failed to create TLS stream for '%S'\n", hostname ) );
                }
            }

            KSocketRelease ( sock );

            if ( rc == 0 )
            {
                STATUS ( STAT_USR, "%s - verifying CA cert\n", __func__ );
                rc = KTLSStreamVerifyCACert ( tls_stream );
                if ( rc != 0 )
                {
                    LOGERR ( klogErr, rc, "failed to verify CA cert" );
                    STATUS ( STAT_QA, "%s - WARNING: failed to verify CA cert - %R\n", __func__, rc );
                }

                if ( rc == 0 )
                {
                    STATUS ( STAT_PRG, "%s - extracting TLS wrapper as stream\n", __func__ );
                    rc = KTLSStreamGetStream ( tls_stream, & self -> sock );
                }

                KTLSStreamRelease ( tls_stream );
            }
        }
        else
        {
            STATUS ( STAT_PRG, "%s - extracting stream from socket\n", __func__ );
            rc = KSocketGetStream ( sock, & self -> sock );
            KSocketRelease ( sock );
        }

        if ( rc == 0 )
        {
            STATUS ( STAT_PRG, "%s - setting port number - %d\n", __func__, aPort );
            self -> port = aPort;
            return 0;
        }
    }

    self -> sock = NULL;
    return rc;
}


#if _DEBUGGING
/* we need this hook to be able to test the re-connection logic */
static struct KStream * (*ClientHttpReopenCallback) ( void ) = NULL;

void SetClientHttpReopenCallback ( struct KStream * (*fn) ( void ) )
{
    ClientHttpReopenCallback = fn;
}
#endif

rc_t KClientHttpReopen ( KClientHttp * self )
{
#if _DEBUGGING
    if ( ClientHttpReopenCallback != NULL )
    {
        self -> sock = ClientHttpReopenCallback ();
        return 0;
    }
#endif

    KClientHttpClose ( self );
    return KClientHttpOpen ( self, & self -> hostname, self -> port );
}

/* Initialize KClientHttp object */
rc_t KClientHttpInit ( KClientHttp * http, const KDataBuffer *hostname_buffer, ver_t _vers, const String * _host, uint32_t port, bool tls )
{
    rc_t rc = 0;

    if ( port == 0 )
        rc = RC ( rcNS, rcNoTarg, rcInitializing, rcParam, rcInvalid );

    /* early setting of TLS property */
    http -> tls = tls;

    rc = KClientHttpOpen ( http, _host, port );
    if ( rc == 0 )
    {
        http -> port = port;
        http -> vers = _vers & 0xFFFF0000; /* safety measure - limit to major.minor */


        /* YOU NEED AN assert HERE TO ENSURE _host IS WITHIN hostname_buffer */
        assert ( KDataBufferContainsString ( hostname_buffer, _host ) );

        /* initialize hostname buffer from external buffer */
        rc = KDataBufferSub ( hostname_buffer, &http -> hostname_buffer,
                              ( _host -> addr - ( const char* ) hostname_buffer -> base ),
                              _host -> size );
        if ( rc == 0 )
        {
            /* Its safe to assign pointer because we know
               that the pointer is within the buffer */
            http -> hostname = * _host;
        }
    }

    return rc;
}


/* MakeHttp
 *  create an HTTP protocol
 *
 *  "http" [ OUT ] - return parameter for HTTP object
 *
 *  "opt_conn" [ IN ] - previously opened stream for communications.
 *
 *  "vers" [ IN ] - http version
 *   the only legal types are 1.0 ( 0x01000000 ) and 1.1 ( 0x01010000 )
 *
 *  "host" [ IN ] - parameter to give the host dns name for the connection
 *
 *  "port" [ IN, DEFAULT ZERO ] - if zero, defaults to standard for scheme
 *   if non-zero, is taken as explicit port specification
 *
 *  "reliable" [ IN ] - if true, then trust that the settings are good
 *   and apply extra stubbornness to fail
 *
 *  "tls" [ IN ] - if true, wrap socket in a TLS encryption protocol
 */
rc_t KNSManagerMakeClientHttpInt ( const KNSManager *self, KClientHttp **_http,
    const KDataBuffer *hostname_buffer,  KStream *opt_conn,
    ver_t vers, int32_t readMillis, int32_t writeMillis,
    const String *host, uint32_t port, bool reliable, bool tls )
{
    rc_t rc;

    KClientHttp * http = ( KClientHttp * ) calloc ( 1, sizeof * http );
    if ( http == NULL )
        rc = RC ( rcNS, rcNoTarg, rcAllocating, rcMemory, rcNull );
    else
    {
        rc = KNSManagerAddRef ( self );
        if ( rc == 0 )
        {
            char save, *text;

            http -> mgr = self;
            http -> read_timeout = readMillis;
            http -> write_timeout = writeMillis;

            /* Dont use MakeBytes because we dont need to allocate memory
               and we only need to know that the elem size is 8 bits */
            KDataBufferClear ( & http -> block_buffer );
            KDataBufferClear ( & http -> line_buffer );

            /* make sure address of bost is within hostname_buffer */
            assert ( KDataBufferContainsString ( hostname_buffer, host ) );

            /* SET TEXT TO POINT TO THE HOST NAME AND NUL TERMINATE IT FOR DEBUGGING
             Its safe to modify the const char array because we allocated the buffer*/
            text = ( char* ) ( host -> addr );
            save = text [ host -> size ];
            text [ host -> size ] = 0;

            /* initialize reference counter on object to 1 - text is now nul-terminated */
            KRefcountInit ( & http -> refcount, 1, "KClientHttp", "make", text );

            text [ host -> size ] = save;

            /* init the KClientHttp object */
            if ( opt_conn != NULL )
            {
                http -> test_sock = opt_conn;
                rc = KStreamAddRef ( http -> test_sock );
            }

            if ( rc == 0 )
            {
                rc = KClientHttpInit ( http, hostname_buffer, vers, host, port, tls );
                if ( rc == 0 )
                {
                    http -> reliable = reliable;

                    /* assign to OUT http param */
                    * _http = http;
                    return 0;
                }
                KStreamRelease ( http->test_sock );
            }

            KNSManagerRelease ( self );
        }

        free ( http );
    }

    return rc;
}

static
rc_t KNSManagerMakeTimedClientHttpInt ( const KNSManager *self,
    KClientHttp **_http, KStream *opt_conn, ver_t vers, int32_t readMillis, int32_t writeMillis,
    const String *host, uint32_t port, uint32_t dflt_port, bool tls )
{
    rc_t rc;

    /* check return parameters */
    if ( _http == NULL )
        rc = RC ( rcNS, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        /* check input parameters */
        if ( self == NULL )
            rc = RC ( rcNS, rcMgr, rcConstructing, rcSelf, rcNull );
        /* make sure we have one of the two versions supported - 1.0, 1.1 */
        else if ( vers < 0x01000000 || vers > 0x01010000 )
            rc = RC ( rcNS, rcMgr, rcConstructing, rcParam, rcBadVersion );
        else if ( host == NULL )
            rc = RC ( rcNS, rcMgr, rcConstructing, rcPath, rcNull );
        /* make sure there is data in the host name */
        else if ( host -> size == 0 )
            rc = RC ( rcNS, rcMgr, rcConstructing, rcPath, rcEmpty );
        else
        {
            KDataBuffer hostname_buffer;

            /* create storage buffer for hostname */
            rc = KDataBufferMakeBytes ( & hostname_buffer, host -> size + 1 );
            if ( rc == 0 )
            {
                String _host;

                /* copy hostname with nul termination */
                string_copy ( ( char * ) hostname_buffer . base,
                              ( size_t ) hostname_buffer . elem_count,
                              host -> addr, host -> size );

                /* create copy of host that points into new buffer */
                StringInit ( & _host, ( const char * ) hostname_buffer . base,
                             host -> size, host -> len );

                /* limit timeouts */
                if ( readMillis < 0 )
                    readMillis = -1;
                else if ( readMillis > MAX_HTTP_READ_LIMIT )
                    readMillis = MAX_HTTP_READ_LIMIT;
                if ( writeMillis < 0 )
                    writeMillis = -1;
                else if ( writeMillis > MAX_HTTP_WRITE_LIMIT )
                    writeMillis = MAX_HTTP_WRITE_LIMIT;

                /* default port */
                if ( port == 0 )
                    port = dflt_port;

                /* initialize http object - will create a new reference to hostname buffer */
                rc = KNSManagerMakeClientHttpInt ( self, _http, & hostname_buffer,
                    opt_conn, vers, readMillis, writeMillis, &_host, port, false, tls );

                /* release our reference to buffer */
                KDataBufferWhack ( & hostname_buffer );

                if ( rc == 0 )
                    return 0;
            }
        }

        * _http = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC KNSManagerMakeTimedClientHttp ( const KNSManager *self,
    KClientHttp ** http, KStream *opt_conn, ver_t vers, int32_t readMillis, int32_t writeMillis,
    const String *host, uint32_t port )
{
    return KNSManagerMakeTimedClientHttpInt ( self, http, opt_conn, vers, readMillis, writeMillis, host, port, 80, false );
}

LIB_EXPORT rc_t CC KNSManagerMakeClientHttp ( const KNSManager *self,
    KClientHttp **http, KStream *opt_conn, ver_t vers, const String *host, uint32_t port )
{
    if ( self == NULL )
    {
        if ( http == NULL )
            return RC ( rcNS, rcMgr, rcValidating, rcParam, rcNull );

        * http = NULL;

        return RC ( rcNS, rcMgr, rcValidating, rcSelf, rcNull );
    }

    return KNSManagerMakeTimedClientHttp ( self, http, opt_conn, vers,
        self -> http_read_timeout, self -> http_write_timeout, host, port );
}

LIB_EXPORT rc_t CC KNSManagerMakeTimedClientHttps ( const KNSManager *self,
    KClientHttp ** https, KStream *opt_conn, ver_t vers, int32_t readMillis, int32_t writeMillis,
    const String *host, uint32_t port )
{
    return KNSManagerMakeTimedClientHttpInt ( self, https, opt_conn, vers, readMillis, writeMillis, host, port, 443, true );
}

LIB_EXPORT rc_t CC KNSManagerMakeClientHttps ( const KNSManager *self,
    KClientHttp **https, KStream *opt_conn, ver_t vers, const String *host, uint32_t port )
{
    if ( self == NULL )
    {
        if ( https == NULL )
            return RC ( rcNS, rcMgr, rcValidating, rcParam, rcNull );

        * https = NULL;

        return RC ( rcNS, rcMgr, rcValidating, rcSelf, rcNull );
    }

    return KNSManagerMakeTimedClientHttps ( self, https, opt_conn, vers,
        self -> http_read_timeout, self -> http_write_timeout, host, port );
}


/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KClientHttpAddRef ( const KClientHttp *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KClientHttp" ) )
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

LIB_EXPORT rc_t CC KClientHttpRelease ( const KClientHttp *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KClientHttp" ) )
        {
        case krefWhack:
            return KClientHttpWhack ( ( KClientHttp* ) self );
        case krefNegative:
            return RC ( rcNS, rcNoTarg, rcReleasing, rcRange, rcExcessive );
        default:
            break;
        }
    }

    return 0;
}


LIB_EXPORT rc_t CC KClientHttpRead ( const KClientHttp *self,
    void *buffer, size_t bsize, size_t *num_read )
{
    if ( self == NULL )
        return RC ( rcNS, rcNoTarg, rcReading, rcSelf, rcNull );

    return KStreamRead ( self -> sock, buffer, bsize, num_read );
}

LIB_EXPORT rc_t CC KClientHttpWriteAll ( const KClientHttp *self,
    const void *buffer, size_t size, size_t *num_writ )
{
    if ( self == NULL )
        return RC ( rcNS, rcNoTarg, rcReading, rcSelf, rcNull );

     return KStreamWriteAll ( self -> sock, buffer, size, num_writ );
}


/* Communication Methods
 *  Read in the http response and return 1 char at a time
 */
static
rc_t KClientHttpGetCharFromResponse ( KClientHttp *self, char *ch, struct timeout_t *tm )
{
    rc_t rc;
    char * buffer = ( char * ) self -> block_buffer . base;

    /* check for data in buffer */
    if ( KClientHttpBlockBufferIsEmpty ( self ) )
    {
        /* check to see ho many bytes are in the buffer */
        size_t bsize = KDataBufferBytes ( & self -> block_buffer );

        /* First time around, bsize will be 0 */
        if ( bsize == 0 )
        {
            bsize = 64 * 1024;
            rc = KDataBufferResize ( & self -> block_buffer, bsize );
            if ( rc != 0 )
                return rc;

            /* re-assign new base pointer */
            buffer = ( char * ) self -> block_buffer . base;
        }

        /* zero out offsets */
        KClientHttpBlockBufferReset ( self );

        /* read from the stream into the buffer, and record the bytes read
           into block_valid */
        /* NB - do NOT use KStreamReadAll or it will block with http 1.1
           because http/1.1 uses keep alive and the read will block until the server
           drops the connection */
        rc = KStreamTimedRead ( self -> sock, buffer, bsize, & self -> block_valid, tm );
        if ( rc != 0 )
        {
            KClientHttpClose ( self );
            return rc;
        }

        /* if nothing was read, we have reached the end of the stream */
        if ( self -> block_valid == 0 )
        {
            KClientHttpClose ( self );

            /* return nul char */
            * ch = 0;
            return 0;
        }
    }

    /* return the next char in the buffer */
    * ch = buffer [ self -> block_read ++ ];
    return 0;
}

/* Read and return entire lines ( until \r\n ) */
static
rc_t KClientHttpGetLine ( KClientHttp *self, struct timeout_t *tm )
{
    rc_t rc;

    char * buffer = ( char * ) self -> line_buffer . base;
    size_t bsize = KDataBufferBytes ( & self -> line_buffer );

    /* num_valid bytes read starts at 0 */
    self -> line_valid = 0;
    while ( 1 )
    {
        char ch;

        /* get char */
        rc = KClientHttpGetCharFromResponse ( self, &ch, tm );
        if ( rc != 0 )
            break;

        if ( ch == '\n' )
        {
            /* check that there are valid bytes read and the previous char is '\r' */
            if ( self -> line_valid > 0 && buffer [ self -> line_valid - 1 ] == '\r' )
            {
                /* decrement number of valid bytes to remove '\r' */
                -- self -> line_valid;
            }
            /* record end of line */
            ch = 0;
        }

        /* check if the buffer is full */
        if ( self -> line_valid == bsize )
        {
            /* I assume that the header lines will not be too large
               so only need to increment  by small chunks */
            bsize += 256;

            /* TBD - place an upper limit on resize */

            /* resize */
            rc = KDataBufferResize ( & self -> line_buffer, bsize );
            if ( rc != 0 )
                return rc;

            /* assign new base pointer */
            buffer = ( char * ) self -> line_buffer . base;
        }

        /* buffer is not full, insert char into the buffer */
        buffer [ self -> line_valid ] = ch;

        /* get out of loop if end of line */
        if ( ch == 0 )
        {
#if _DEBUGGING
            if ( KNSManagerIsVerbose ( self -> mgr ) ) {
                size_t i = 0;
                KOutMsg ( "KClientHttpGetLine: '" );
                for (i = 0; i <= self->line_valid; ++i) {
                    if (isprint(buffer[i])) {
                        KOutMsg("%c", buffer[i]);
                    }
                    else {
                        KOutMsg("\\%02X", buffer[i]);
                    }
                }
                KOutMsg ( "'\n" );
            }
#endif
            break;
        }
        /* not end of line - increase num of valid bytes read */
        ++ self -> line_valid;
    }

    return rc;
}

/* AddHeaderString
 *  performs task of entering a header into BSTree
 *  or updating an existing node
 *
 * If header exists:
 *  if (add) then add "value" to "header value"
 *  else     replace "header value" with "value"
 *
 *  Headers are always made up of a name: value pair
 */
static
rc_t KClientHttpAddHeaderString
( BSTree *hdrs, bool add, const String *name, const String *value )
{
    rc_t rc = 0;

    /* if there is no name - error */
    if ( name -> size == 0 )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcInsufficient );
    else
    {
        /* test for previous existence of node by name */
        KHttpHeader * node = ( KHttpHeader * ) BSTreeFind ( hdrs, name, KHttpHeaderCmp );
        if ( node == NULL )
        {
            /* node doesnt exist - allocate memory for a new one */
            node = ( KHttpHeader * ) calloc ( 1, sizeof * node );
            if ( node == NULL )
                rc = RC ( rcNS, rcNoTarg, rcAllocating, rcMemory, rcNull );
            else
            {
                rc = KDataBufferMakeBytes ( & node -> value_storage, 0 );
                if ( rc == 0 )
                {
                    /* copy the string data into storage */
                    rc = KDataBufferPrintf ( & node -> value_storage,
                                             "%S%S", name, value );
                    if ( rc == 0 )
                    {
                        /* initialize the Strings to point into KHttpHeader node */
                        StringInit ( & node -> name,
                            ( const char * ) node -> value_storage . base,
                            name -> size, name -> len );
                        StringInit ( & node -> value, node -> name . addr + name -> size, value -> size, value -> len );

                        /* insert into tree, sorted by alphabetical order */
                        BSTreeInsert ( hdrs, & node -> dad, KHttpHeaderSort );

                        return 0;
                    }

                    KDataBufferWhack ( & node -> value_storage );
                }

                free ( node );
            }
        }

        /* node exists
           check that value param has data */
        else if ( value -> size != 0 )
        {
          if ( add ) { /* add value to node -> value
                          do not add value if node -> value == value */
            if ( ! StringEqual ( & node -> value, value ) )
            {
                rc = KDataBufferPrintf( & node -> value_storage, ",%S", value );
                if ( rc == 0 )
                {
                    /* update size and len of value in the node */
                    node -> value . size += value -> size + 1;
                    node -> value . len += value -> len + 1;
                    return 0;
                }
            }
          } else { /* replace value with node -> value */
            if ( ! StringEqual ( & node -> value, value ) )
            /* values are not equal - need to replace */
            {
                rc = KDataBufferWhack ( & node -> value_storage );
                if ( rc == 0 )
                {
                    rc = KDataBufferPrintf( & node -> value_storage, "%S%S", name, value );
                    if ( rc == 0 )
                    {
                        /* initialize the Strings to point into KHttpHeader node */
                        StringInit ( & node -> name,
                            ( const char * ) node -> value_storage . base,
                            name -> size, name -> len );
                        StringInit ( & node -> value,
                            node -> name . addr + name -> size,
                            value -> size, value -> len );
                    }
                }
            }
          }
        }
    }

    return rc;
}

rc_t KClientHttpVAddHeader ( BSTree *hdrs, bool add,
    const char *_name, const char *_val, va_list args )
{
    rc_t rc;

    KDataBuffer buf;
    String name, value;

    /* initialize data buffer */
    KDataBufferMakeBytes ( & buf, 0 );

    /* initialize name string from param */
    StringInitCString ( & name, _name );

    /* copy data into buf, using va_list for value format */
    rc = KDataBufferVPrintf ( & buf, _val, args );
    if ( rc == 0 && buf . elem_count != 0 )
    {
        size_t bsize = ( size_t ) buf . elem_count - 1;
      
        /* get length of buf */
        size_t blen = string_len ( buf . base, bsize );

        /* init value */
        StringInit ( & value, buf . base, bsize, ( uint32_t ) blen );

        rc = KClientHttpAddHeaderString ( hdrs, add, & name, & value );
    }

    KDataBufferWhack ( & buf );

    return rc;
}

rc_t KClientHttpAddHeader
( BSTree *hdrs, const char *name, const char *val, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, val );
    rc = KClientHttpVAddHeader ( hdrs, true, name, val, args );
    va_end ( args );
    return rc;
}

rc_t KClientHttpReplaceHeader
( BSTree *hdrs, const char *name, const char *val, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, val );
    rc = KClientHttpVAddHeader ( hdrs, false, name, val, args );
    va_end ( args );
    return rc;
}

/* Capture each header line to add to BSTree */
rc_t KClientHttpGetHeaderLine ( KClientHttp *self, timeout_t *tm, BSTree *hdrs,
    bool * blank, bool * len_zero, bool * close_connection )
{
    /* Starting from the second line of the response */
    rc_t rc = KClientHttpGetLine ( self, tm );
    assert ( len_zero );
    if ( rc == 0 )
    {
        /* blank = empty line_buffer = separation between headers and body of response */
        if ( self -> line_valid == 0 ) {
            * blank = true;

            /* print an empty string to separate response headers */
            DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ), ( "\n"  ) );
        }
        else
        {
            char * sep;
            const char * buffer = ( char * ) self -> line_buffer . base;
            const char * end = buffer + self -> line_valid;

            DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_HTTP), ("HTTP receive '%s'\n", buffer));

            /* find the separation between name: value */
            sep = string_chr ( buffer, end - buffer, ':' );
            if ( sep == NULL )
            {
                rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
                TRACE ( "badly formed header: '%.*s'\n", ( int ) ( end - buffer ), buffer );
            }
            else
            {
                String name, value;
                const char * last = sep;

                /* trim white space around name */
                while ( buffer < last && isspace ( buffer [ 0 ] ) )
                    ++ buffer;
                while ( buffer < last && isspace ( last [ -1 ] ) )
                    -- last;

                /* assign the name data into the name string */
                StringInit ( & name, buffer, last - buffer, ( uint32_t ) ( last - buffer ) );

                /* move the buffer forward to value */
                buffer = sep + 1;
                last = end;

                /* trim white space around value */
                while ( buffer < last && isspace ( buffer [ 0 ] ) )
                    ++ buffer;
                while ( buffer < last && isspace ( last [ -1 ] ) )
                    -- last;

                /* assign the value data into the value string */
                StringInit ( & value, buffer, last - buffer, ( uint32_t ) ( last - buffer ) );

                switch ( name . size )
                {
                case sizeof "Connection" - 1:
                    if ( value . size == sizeof "close" - 1 )
                    {
                        if ( tolower ( name . addr [ 0 ] ) == 'c' && tolower ( value . addr [ 0 ] ) == 'c' )
                        {
                            if ( strcase_cmp ( name . addr, name . size, "Connection", name . size, ( uint32_t ) name . size ) == 0 &&
                                 strcase_cmp ( value . addr, value . size, "close", value . size, ( uint32_t ) value . size ) == 0 )
                            {
                                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS ),
                                       ("*** seen connection close ***\n"));
                                * close_connection = true;
                            }
                        }
                    }
                    break;
                case sizeof "Content-Length" - 1:
                    if ( value . size == sizeof "0" - 1 )
                    {
                        if ( tolower ( name . addr [ 0 ] ) == 'c' && value . addr [ 0 ] == '0' )
                        {
                            if ( strcase_cmp ( name . addr, name . size, "Content-Length", name . size, ( uint32_t ) name . size ) == 0 )
                            {
                                * len_zero = true;
                            }
                        }
                    }
                    break;
                }

                rc = KClientHttpAddHeaderString
                    ( hdrs, true, & name, & value );
            }
        }
    }

    return rc;
}

/* Locate a KhttpHeader obj in BSTree */
rc_t KClientHttpFindHeader ( const BSTree *hdrs, const char *_name, char *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = 0;
    String name;
    KHttpHeader * node;

    StringInitCString ( &name, _name );

    /* find the header */
    node = ( KHttpHeader * ) BSTreeFind ( hdrs, &name, KHttpHeaderCmp );
    if ( node == NULL )
    {
        rc = SILENT_RC ( rcNS, rcTree, rcSearching, rcName, rcNotFound );
    }
    else
    {
        /* make sure buffer is large enough */
        if ( bsize < node -> value . size )
        {
            /* return the amount needed */
            * num_read = node -> value . size;

            return RC ( rcNS, rcNoTarg, rcParsing, rcBuffer, rcInsufficient );
        }

        /* copy data and return the num_read */
        * num_read = string_copy ( buffer, bsize, node -> value . addr, node -> value . size );
    }
    return rc;
}

rc_t KClientHttpGetStatusLine ( KClientHttp *self, timeout_t *tm, String *msg, uint32_t *status, ver_t *version )
{
    /* First time reading the response */
    rc_t rc = KClientHttpGetLine ( self, tm );

    if (rc == 0 && self->line_valid == 0) {
        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_ERR), (
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@2 %s empty HttpStatusLine @@@@@@@@@@@@@@@@"
            "\n", __FILE__));
        rc = RC(rcNS, rcNoTarg, rcReading, rcNoObj, rcIncomplete);
    }

    if ( rc == 0 )
    {
        char * sep;
        const char * buffer = ( char * ) self -> line_buffer . base;
        const char * end = buffer + self -> line_valid;

        DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_HTTP), ("HTTP receive '%s'\n", buffer));

        /* Detect protocol
           expect HTTP/1.[01]<sp><digit>+<sp><msg>\r\n */
        sep = string_chr ( buffer, end - buffer, '/' );
        if ( sep == NULL )
        {
            rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
            TRACE ( "badly formed status line: '%.*s'\n", ( int ) ( end - buffer ), buffer );
        }
        else
        {
            /* make sure it is http */
            if ( strcase_cmp ( "http", 4, buffer, sep - buffer, 4 ) != 0 )
            {
                rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcUnsupported );
                DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_HTTP), ("%s: protocol given as '%.*s'\n", __func__, ( uint32_t ) ( sep - buffer ), buffer ));
            }
            else
            {
                /* move buffer up to version */
                buffer = sep + 1;

                /* find end of version */
                sep = string_chr ( buffer, end - buffer, ' ' );
                if ( sep == NULL )
                {
                    rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
                    TRACE ( "badly formed HTTP version: '%.*s'\n", ( int ) ( end - buffer ), buffer );
                }
                else
                {
                    /* must be 1.0 or 1.1 */
                    if ( ( string_cmp ( "1.0", 3, buffer, sep - buffer, 3 ) != 0 ) &&
                         ( string_cmp ( "1.1", 3, buffer, sep - buffer, 3 ) != 0 ) )
                        rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcUnsupported );
                    else
                    {
                        /* which version was returned? */
                        * version = string_cmp ( "1.0", 3, buffer, sep - buffer, -1 ) == 0 ? 0x01000000 : 0x01010000;

                        /* move up to status code */
                        buffer = sep + 1;

                        /* record status as uint32
                         sep should point to 1 byte after end of status text */
                        * status = strtou32 ( buffer, & sep, 10 );

                        /* if at the end of buffer or sep didnt land on a space - error */
                        if ( sep == buffer || * sep != ' ' )
                        {
                            rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
                            TRACE ( "badly formed HTTP version: '%.*s': numeral ends on '%c'\n", ( int ) ( end - buffer ), buffer, ( sep == buffer ) ? 0 : * sep );
                        }
                        else
                        {
                            /* move up to status msg */
                            buffer = sep + 1;

                            /* initialize the msg String with the proper size and length */
                            StringInit ( msg, buffer, end - buffer, ( uint32_t ) ( end - buffer ) );
                        }
                    }
                }
            }
        }
    }
    return rc;
}

/*--------------------------------------------------------------------------
 * ClientHttpStream
 *  structure that represents the body of the response
 */
struct KClientHttpStream
{
    KStream dad;

    /* content_length is the size of the chunk
       total_read is the number of read from the chunk */
    uint64_t content_length;
    uint64_t total_read;

    KClientHttp * http;

    uint8_t state; /* keeps track of state for chunked reader */
    bool size_unknown; /* for HTTP/1.0 dynamic */
};

enum
{
    end_chunk,
    new_chunk,
    within_chunk,
    end_stream,
    error_state
};

static
rc_t CC KClientHttpStreamWhack ( KClientHttpStream *self )
{
    KClientHttpRelease ( self -> http );
    KStreamWhack ( & self -> dad, "KClientHttpStream" );
    free ( self );
    return 0;
}

/* Read from stream - either not chunked or within a chunk */
static
rc_t CC KClientHttpStreamTimedRead ( const KClientHttpStream *cself,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
    rc_t rc;
    KClientHttpStream *self = ( KClientHttpStream * ) cself;
    KClientHttp *http = self -> http;

    /* minimum of bytes requested and bytes available in stream */
    uint64_t num_to_read = self -> content_length - self -> total_read;

    /* take the minimum of bytes avail or bytes requested */
    if ( self -> size_unknown || bsize < num_to_read )
        num_to_read = bsize;

    /* Should be 0 because nothing has been read. Caller
       sets its to 0 */
    assert ( * num_read == 0 );
    /* exit if there is nothing to read */
    if ( num_to_read == 0 )
        return 0;

    /* read directly from stream
       check if the buffer is empty */
    if ( KClientHttpBlockBufferIsEmpty ( http ) )
    {
        /* ReadAll blocks for 1.1. Server will drop the connection */
        rc =  KStreamTimedRead ( http -> sock, buffer, num_to_read, num_read, tm );
        if ( rc != 0 )
        {
            /* handle dropped connection - may want to reestablish */
            KClientHttpClose ( http );

            /* LOOK FOR DROPPED CONNECTION && SIZE UNKNOWN - HTTP/1.0 DYNAMIC CASE */
            if ( self -> size_unknown )
                rc = 0;
        }

        /* if nothing was read - end of stream */
        else if ( * num_read == 0 )
        {
            KClientHttpClose ( http );

            /* if the size was known, it is an incomplete transfer */
            if ( ! self -> size_unknown )
                rc = RC ( rcNS, rcStream, rcReading, rcTransfer, rcIncomplete);
        }
    }
    else
    {
        const char * buf = NULL;

        /* bytes available in buffer */
        uint64_t bytes_in_buffer = http -> block_valid - http -> block_read;

        /* take the minimum of bytes avail or bytes requested */
        if ( num_to_read > bytes_in_buffer )
            num_to_read = bytes_in_buffer;

        /* capture base pointer */
        buf = ( char * ) http -> block_buffer . base;

        /* copy data into the user buffer from the offset of bytes not yet read */
        memmove ( buffer, & buf [ http -> block_read ], num_to_read );

        /* update the amount read */
        http -> block_read += num_to_read;

        /* return how much was read */
        * num_read = num_to_read;

        rc = 0;
    }

    /* update the total from the stream
       keep track of total bytes read within the chunk */
    self -> total_read += * num_read;

    return rc;
}

static
rc_t CC KClientHttpStreamRead ( const KClientHttpStream *self,
    void *buffer, size_t bsize, size_t *num_read )
{
    return KClientHttpStreamTimedRead ( self, buffer, bsize, num_read, NULL );
}

/* Uses a state machine*/
static
rc_t CC KClientHttpStreamTimedReadChunked ( const KClientHttpStream *cself,
    void *buffer, size_t bsize, size_t *num_read, timeout_t *tm )
{
    rc_t rc;
    char * sep;
    KClientHttpStream *self = ( KClientHttpStream * ) cself;
    KClientHttp * http = self -> http;

    assert ( * num_read == 0 );

    switch ( self -> state )
    {
    case end_chunk:
        rc = KClientHttpGetLine ( http, tm );
        if ( rc != 0 )
        {
            self -> state = error_state;
            break;
        }

        /* this should be the CRLF following chunk */
        if ( http -> line_valid != 0 )
        {
            KClientHttpClose ( http );
            rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcIncorrect);
            self -> state = error_state;
            break;
        }

        self -> state = new_chunk;

        /* NO BREAK */

        /* start */
    case new_chunk:

        /* Get chunk size */
        rc = KClientHttpGetLine ( http, tm );
        if ( rc != 0 )
        {
            self -> state = error_state;
            break;
        }

        /* convert the hex number containing chunk size to uint64
           sep should be pointing at nul byte */
        self -> content_length
          = strtou64 ( ( const char * ) http -> line_buffer . base, & sep, 16 );

        /* TBD - eat spaces here? */
        /* check if there was no hex number, or sep isn't pointing to nul byte */
        if ( sep == http -> line_buffer . base || ( * sep != 0 && * sep != ';' ) )
        {
            KClientHttpClose ( http );
            rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcIncorrect);
            self -> state = error_state;
            break;
        }

        /* check for end of stream */
        if ( self -> content_length == 0 )
        {
            self -> state = end_stream;
            return 0;
        }

        /* havent read anything - start at 0 */
        self -> total_read = 0;

        /* now within a chunk */
        self -> state = within_chunk;

        /* NO BREAK */

    case within_chunk:
        /* start reading */
        rc = KClientHttpStreamRead ( self, buffer, bsize, num_read );
        if ( rc != 0 )
            self -> state = error_state;
        /* incomplete if nothing to read */
        else if ( * num_read == 0 )
        {
            KClientHttpClose ( http );
            rc = RC ( rcNS, rcStream, rcReading, rcTransfer, rcIncomplete);
            self -> state = error_state;
        }
        /* check for end of chunk */
        else if ( self -> total_read == self -> content_length )
            self -> state = end_chunk;
        break;

    case end_stream:
        return 0;

    case error_state:
        rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcIncorrect );
        break;

    default:
        /* internal error */
        rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcError );
    }

    return rc;
}

static
rc_t CC KClientHttpStreamReadChunked ( const KClientHttpStream *self,
    void *buffer, size_t bsize, size_t *num_read )
{
    return KClientHttpStreamTimedReadChunked ( self, buffer, bsize, num_read, NULL );
}

/* cannot write - for now */
static
rc_t CC KClientHttpStreamTimedWrite ( KClientHttpStream *self,
    const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm )
{
    return RC ( rcNS, rcNoTarg, rcWriting, rcFunction, rcUnsupported );
}

static
rc_t CC KClientHttpStreamWrite ( KClientHttpStream *self,
    const void *buffer, size_t size, size_t *num_writ )
{
    return RC ( rcNS, rcNoTarg, rcWriting, rcFunction, rcUnsupported );
}

static KStream_vt_v1 vtKClientHttpStream =
{
    1, 1,
    KClientHttpStreamWhack,
    KClientHttpStreamRead,
    KClientHttpStreamWrite,
    KClientHttpStreamTimedRead,
    KClientHttpStreamTimedWrite
};

static KStream_vt_v1 vtKClientHttpStreamChunked =
{
    1, 1,
    KClientHttpStreamWhack,
    KClientHttpStreamReadChunked,
    KClientHttpStreamWrite,
    KClientHttpStreamTimedReadChunked,
    KClientHttpStreamTimedWrite
};

/* Make a KClientHttpStream object */
static
rc_t KClientHttpStreamMake ( KClientHttp *self, KStream **sp, const char *strname, size_t content_length, bool size_unknown )
{
    rc_t rc;
    KClientHttpStream * s = ( KClientHttpStream * ) calloc ( 1, sizeof * s );
    if ( s == NULL )
        rc = RC ( rcNS, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = KStreamInit ( & s -> dad, ( const KStream_vt * ) & vtKClientHttpStream,
                           "KClientHttpStream", strname, true, false );
        if ( rc == 0 )
        {
            rc = KClientHttpAddRef ( self );
            if ( rc == 0 )
            {
                s -> http = self;
                s -> content_length = content_length;
                s -> size_unknown = size_unknown;
                *sp = & s -> dad;
                return 0;
            }
        }
        free ( s );
    }
    *sp = NULL;

    return rc;
}

static
rc_t KClientHttpStreamMakeChunked ( KClientHttp *self, KStream **sp, const char *strname )
{
    rc_t rc;
    KClientHttpStream * s = ( KClientHttpStream * ) calloc ( 1, sizeof * s );
    if ( s == NULL )
        rc = RC ( rcNS, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = KStreamInit ( & s -> dad, ( const KStream_vt * ) & vtKClientHttpStreamChunked,
                           "KClientHttpStreamChunked", strname, true, false );
        if ( rc == 0 )
        {
            rc = KClientHttpAddRef ( self );
            if ( rc == 0 )
            {
                s -> http = self;

                /* state should be new_chunk */
                s -> state = new_chunk;

                *sp = & s -> dad;
                return 0;
            }
        }
        free ( s );
    }
    *sp = NULL;

    return rc;
}

/*--------------------------------------------------------------------------
 * KClientHttpResult
 *  hyper text transfer protocol
 *  Holds all the headers in a BSTree
 *  Records the status msg, status code and version of the response
 */

static
rc_t KClientHttpResultWhack ( KClientHttpResult * self )
{
    BSTreeWhack ( & self -> hdrs, KHttpHeaderWhack, NULL );

    KClientHttpRelease ( self -> http );

    KRefcountWhack ( & self -> refcount, "KClientHttpResult" );

    free ( self -> expiration );

    free ( self );

    return 0;
}


/* Sends the request and receives the response into a KClientHttpResult obj */
rc_t KClientHttpSendReceiveMsg ( KClientHttp *self, KClientHttpResult **rslt,
    const char *buffer, size_t len, const KDataBuffer *body, const char *url )
{
    rc_t rc = 0;
    size_t sent;
    timeout_t tm;
    uint32_t status;

    /* TBD - may want to assert that there is an empty line in "buffer" */
#if _DEBUGGING
    if ( KNSManagerIsVerbose ( self -> mgr ) )
        KOutMsg ( "KClientHttpSendReceiveMsg: '%.*s'\n", len, buffer );
#endif
    DBGMSG(DBG_KNS, DBG_FLAG(DBG_KNS_HTTP),
        ("HTTP send '%S' '%.*s'\n\n", &self->hostname, len, buffer));

    /* reopen connection if NULL */
    if ( self -> sock == NULL )
        rc = KClientHttpOpen ( self, & self -> hostname, self -> port );

    /* ALWAYS want to use write all when sending */
    if ( rc == 0 )
    {
        TimeoutInit ( & tm, self -> write_timeout );
        rc = KStreamTimedWriteAll ( self -> sock, buffer, len, & sent, & tm );
        if ( rc != 0 )
        {
            rc_t rc2 = KClientHttpReopen ( self );
            if ( rc2 == 0 )
            {
                TimeoutInit ( & tm, self -> write_timeout );
                rc2 = KStreamTimedWriteAll ( self -> sock, buffer, len, & sent, & tm );
                if ( rc2 == 0 )
                    rc = 0;
            }
        }
    }

    /* check the data was completely sent */
    if ( rc == 0 && sent != len )
    {
        rc = RC ( rcNS, rcNoTarg, rcWriting, rcTransfer, rcIncomplete );
        KClientHttpClose ( self );
    }
    if ( rc == 0 && body != NULL  && body -> elem_count > 0 )
    {
        /* "body" contains bytes plus trailing NUL */
        size_t to_send = ( size_t ) body -> elem_count - 1;
        rc = KStreamTimedWriteAll ( self -> sock, body -> base, to_send, & sent, & tm );
        if ( rc == 0 && sent != to_send )
        {
            rc = RC ( rcNS, rcNoTarg, rcWriting, rcTransfer, rcIncomplete );
            KClientHttpClose ( self );
        }
    }

    for ( status = 100; rc == 0 && status == 100; )
    {
        String msg;
        ver_t version;

        /* reinitialize the timeout for reading */
        TimeoutInit ( & tm, self -> read_timeout );

        /* we have now received a response
           start reading the header lines */
        rc = KClientHttpGetStatusLine ( self, & tm, & msg, & status, & version );
        if ( rc == 0 )
        {
            /* create a result object with enough space for msg string + nul */
            KClientHttpResult * result = ( KClientHttpResult * )
                                    malloc ( sizeof * result + msg . size + 1 );
            if ( result == NULL )
                rc = RC ( rcNS, rcNoTarg, rcAllocating, rcMemory, rcExhausted );
            else
            {
                /* zero out */
                memset ( result, 0, sizeof * result );

                rc = KClientHttpAddRef ( self );
                if ( rc == 0 )
                {
                    bool blank;

                    /* treat excess allocation memory as text space */
                    char *text = ( char* ) ( result + 1 );

                    /* copy in the data to the text space */
                    string_copy ( text, msg . size + 1, msg . addr, msg . size );

                    /* initialize the result members
                       "hdrs" is initialized via "memset" above
                     */
                    result -> http = self;
                    result -> status = status;
                    result -> version = version;
                    result -> expiration = NULL;

                    /* correlate msg string in result to the text space */
                    StringInit ( & result -> msg, text, msg . size, msg . len );

                    /* TBD - pass in URL as instance identifier */
                    KRefcountInit ( & result -> refcount, 1, "KClientHttpResult", "sending-msg", url );

                    /* receive and parse all header lines
                       blank = end of headers */
                    for ( blank = false; ! blank && rc == 0; )
                    {
                        rc = KClientHttpGetHeaderLine ( self, & tm, & result -> hdrs,
                            & blank, & result -> len_zero, & self -> close_connection );
                    }

                    if ( rc == 0 && status != 100 )
                    {
                        /* assign to OUT result obj */
                        * rslt = result;
                        return 0;
                    }

                    BSTreeWhack ( & result -> hdrs, KHttpHeaderWhack, NULL );
                }

                KClientHttpRelease ( self );
            }

            free ( result );
        }
    }

    return rc;
}

/* test */
/*static
void KClientHttpForceSocketClose(const KClientHttp *self)
{
    KStreamForceSocketClose(self->sock);
}
*/

/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KClientHttpResultAddRef ( const KClientHttpResult *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KClientHttpResult" ) )
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

LIB_EXPORT rc_t CC KClientHttpResultRelease ( const KClientHttpResult *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KClientHttpResult" ) )
        {
        case krefWhack:
            return KClientHttpResultWhack ( ( KClientHttpResult* ) self );
        case krefNegative:
            return RC ( rcNS, rcNoTarg, rcReleasing, rcRange, rcExcessive );
        default:
            break;
        }
    }

    return 0;
}


/* Status
 *  access the response status code
 *  and optionally the message
 *
 *  "code" [ OUT ] - return parameter for status code
 *
 *  "msg_buff" [ IN, NULL OKAY ] and "buff_size" [ IN, ZERO OKAY ] -
 *   buffer for capturing returned message. if "msg_buff" is not
 *   NULL and "buff_size" is insufficient for copying status message,
 *   the message returns rcBuffer, rcInsufficient.
 *
 *  "msg_size" [ OUT, NULL OKAY ] - size of returned message in bytes.
 *   if not NULL, returns the size of status message. if "msg_buff" is
 *   NULL, returns rcBuffer, rcNull.
 */
LIB_EXPORT rc_t CC KClientHttpResultStatus ( const KClientHttpResult *self, uint32_t *code,
    char *msg_buff, size_t buff_size, size_t *msg_size )
{
    rc_t rc;

    /* check OUT parameters */
    if ( code == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    else
    {
        /* IN parameters */
        if ( self == NULL )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
        else
        {
            /* capture the status code to OUT param */
            * code = self -> status;
            rc = 0;

            /* if asking about msg size */
            if ( msg_size != NULL )
            {
                /* capture the msg size */
                * msg_size = self -> msg . size;

                /* catch NULL buffer pointer */
                if ( msg_buff == NULL )
                    rc = RC ( rcNS, rcNoTarg, rcValidating, rcBuffer, rcNull );
            }

            /* if they apparently want the message */
            if ( msg_buff != NULL )
            {
                /* check for an insufficient buffer size */
                if ( buff_size < self -> msg . size )
                    rc = RC ( rcNS, rcNoTarg, rcValidating, rcBuffer, rcInsufficient );
                else
                    /* copy out the message */
                    string_copy ( msg_buff, buff_size, self -> msg . addr, self -> msg . size );
            }

            return rc;
        }

        * code = 0;
    }

    return rc;
}


/* KeepAlive
 *  retrieves keep-alive property of response
 */
LIB_EXPORT bool CC KClientHttpResultKeepAlive ( const KClientHttpResult *self )
{
    rc_t rc;

    if ( self != NULL )
    {
        /* we're requiring version 1.1 -
           some 1.0 servers also supported it... */
        if ( self -> version == 0x01010000 )
        {
            size_t num_writ;
            char buffer [ 1024 ];
            size_t bsize = sizeof buffer;

            /* retreive the node that has the keep-alive property */
            rc = KClientHttpResultGetHeader ( self, "Connection", buffer, bsize, & num_writ );
            if ( rc == 0 )
            {
                String keep_alive, compare;

                /* init strings */
                StringInitCString ( & keep_alive, buffer );
                CONST_STRING ( & compare, "keep-alive" );

                /* compare strings for property value */
                if ( StringCaseCompare ( & keep_alive, & compare ) == 0 )
                    return true;
            }
        }
    }
    return false;
}


/* Range
 *  retrieves position, partial size and total size for partial requests
 *
 *  "pos" [ OUT ] - offset to beginning portion of response
 *
 *  "bytes" [ OUT ] - size of range
 *
 *  "_total" [ OUT, NULL OK ] - total size of file
 *
 *  HERE WE NEED TO HAVE PASSED THE RANGE REQUEST TO THE RESULT ON CREATION,
 *  AND WE WILL RESPOND TO THE HTTP "PARTIAL RESULT" OR WHATEVER RETURN CODE,
 *  AND BASICALLY UPDATE WHAT THE RANGE WAS.
 */
static
rc_t KClientHttpResultHandleContentRange ( const KClientHttpResult *self, uint64_t *pos, size_t *bytes, size_t *_total )
{
    rc_t rc;
    size_t num_read;
    char buffer [ 1024 ];
    const size_t bsize = sizeof buffer;

    /* get Content-Range
     *  expect: "bytes <first-position>-<last-position>/<total-size>"
     */
    rc = KClientHttpResultGetHeader ( self, "Content-Range", buffer, bsize, & num_read );
    if ( rc == 0 )
    {
        char * sep;
        char *buf = buffer;
        const char *end = & buffer [ num_read ];

        /* look for separation of 'bytes' and first position */
        sep = string_chr ( buf, end - buf, ' ' );
        if ( sep == NULL )
        {
            rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
            TRACE ( "badly formed Content-Range header: '%.*s': lacks a space separator\n", ( int ) ( end - buffer ), buffer );
        }
        else
        {
            uint64_t start_pos;

            /* buf now points to value */
            buf = sep + 1;

            /* capture starting position
               sep should land on '-' */
            start_pos = strtou64 ( buf, & sep, 10 );

            /* check if we didnt read anything or sep didnt land on '-' */
            if ( sep == buf || * sep != '-' )
            {
                rc =  RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
                TRACE ( "badly formed Content-Range header: '%.*s': numeral ends on '%c'\n", ( int ) ( end - buffer ), buffer, ( sep == buffer ) ? 0 : * sep );
            }
            else
            {
                uint64_t end_pos;

                buf = sep + 1;
                end_pos = strtou64 ( buf, & sep, 10 );
                if ( sep == buf || * sep != '/' )
                {
                    rc =  RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
                    TRACE ( "badly formed Content-Range header: '%.*s': numeral ends on '%c'\n", ( int ) ( end - buffer ), buffer, ( sep == buffer ) ? 0 : * sep );
                }
                else
                {
                    uint64_t total;

                    buf = sep +1;
                    total = strtou64 ( buf, &sep, 10 );
                    if ( sep == buf || * sep != 0 )
                    {
                        rc =  RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
                        TRACE ( "badly formed Content-Range header: '%.*s': numeral ends on '%c'\n", ( int ) ( end - buffer ), buffer, ( sep == buffer ) ? 0 : * sep );
                    }
                    else
                    {
                        /* check variables */
                        if ( total == 0 ||
                             start_pos > total ||
                             end_pos < start_pos ||
                             end_pos > total )
                        {
                            rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
                            TRACE ( "badly formed Content-Range header: total=%lu, start_pos=%lu, end_pos=%lu\n", total, start_pos, end_pos );
                            if ( total == 0 )
                                TRACE ( "badly formed Content-Range header: total==0 : ERROR%c", '\n' );
                            if ( start_pos > total )
                                TRACE ( "badly formed Content-Range header: start_pos=%lu > total=%lu : ERROR\n", start_pos, total );
                            if ( end_pos < start_pos )
                                TRACE ( "badly formed Content-Range header: end_pos=%lu < start_pos=%lu : ERROR\n", end_pos, start_pos );
                            if ( end_pos > total )
                                TRACE ( "badly formed Content-Range header: end_pos=%lu > total=%lu : ERROR\n", end_pos, total );
                        }
                        else
                        {
                            uint64_t length;

                            /* get content-length to confirm bytes sent */
                            rc = KClientHttpResultGetHeader ( self, "Content-Length", buffer, bsize, & num_read );
                            if ( rc != 0 )
                            {

                                /* remember that we can have chunked encoding,
                                   so "Content-Length" may not exist. */
                                * pos = start_pos;
                                * bytes = end_pos - start_pos + 1;
                                if ( _total != NULL )
                                {
                                    * _total = total;
                                }

                                return 0;
                            }

                            buf = buffer;
                            end = & buffer [ num_read ];

                            /* capture the length */
                            length  = strtou64 ( buf, & sep, 10 );
                            if ( sep == buf || * sep != 0 )
                            {
                                rc =  RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
                                TRACE ( "badly formed Content-Length header: '%.*s': numeral ends on '%c'\n", ( int ) ( end - buffer ), buffer, ( sep == buffer ) ? 0 : * sep );
                            }
                            else
                            {
                                /* finally check all the acquired information */
                                if ( ( length != ( ( end_pos - start_pos ) + 1 ) ) ||
                                     ( length > total ) )
                                {
                                    rc = RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
                                    TRACE ( "badly formed Content-Length header: length=%lu, range_len=%lu, total=%lu\n", length, ( end_pos - start_pos ) + 1, total );
                                    if ( length != ( end_pos - start_pos ) + 1 )
                                        TRACE ( "badly formed Content-Length header: length=%lu != 0 : ERROR\n", length );
                                    if ( start_pos > total )
                                        TRACE ( "badly formed Content-Length header: start_pos=%lu > range_len=%lu : ERROR\n", length, ( end_pos - start_pos ) + 1 );
                                    if ( length > total )
                                        TRACE ( "badly formed Content-Length header: length=%lu > total=%lu : ERROR\n", length, total );
                                }
                                else
                                {
                                    /* assign to OUT params */
                                    * pos = start_pos;
                                    * bytes = length;
                                    if ( _total != NULL )
                                    {
                                        * _total = total;
                                    }

                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC KClientHttpResultRange ( const KClientHttpResult *self, uint64_t *pos, size_t *bytes )
{
    rc_t rc;

    if ( pos ==  NULL || bytes == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    else if ( self == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
    else
    {
        switch ( self -> status )
        {
        case 206:
            /* partial content */
            rc = KClientHttpResultHandleContentRange ( self, pos, bytes, NULL );
            if ( rc == 0 )
                return 0;

        case 416:
            /* unsatisfiable range */
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcError, rcIncorrect );
            break;

        default:
            /* codes not handling right now */
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcError, rcUnsupported );
        }
    }

    if ( pos != NULL )
        * pos = 0;
    if ( bytes != NULL )
        * bytes = 0;

    return rc;
}


/* Size
 *  retrieves overall size of entity, if known
 *
 *  "size" [ OUT ] - size in bytes of response
 *   this is the number of bytes that may be expected from the input stream
 *
 */
LIB_EXPORT bool CC KClientHttpResultSize ( const KClientHttpResult *self, uint64_t *size )
{
    if ( size != NULL && self != NULL )
    {
        rc_t rc;
        size_t num_read;
        char buffer [ 1024 ];
        const size_t bsize = sizeof buffer;

        /* check for content-range */
        {
            uint64_t pos;
            size_t bytes;
            size_t total;
            rc = KClientHttpResultHandleContentRange ( self, & pos, & bytes, & total );
            if ( rc == 0 )
            {
                * size = total;
                return true;
            }
        }

        /* check for content-length */
        rc = KClientHttpResultGetHeader ( self, "Content-Length", buffer, bsize, & num_read );
        if ( rc == 0 )
        {
            char * sep;

            /* capture length as uint64 */
            uint64_t length = strtou64 ( buffer, & sep, 10 );
            if ( sep == buffer || * sep != 0 )
            {
                rc =  RC ( rcNS, rcNoTarg, rcParsing, rcNoObj, rcNotFound );
                TRACE ( "badly formed Content-Length header: '%.*s': numeral ends on '%c'\n", ( int ) num_read, buffer, ( sep == buffer ) ? 0 : * sep );
            }
            else
            {
                /* assign to OUT param */
                * size = length;
                return true;
            }
        }
    }
    return false;
}

/* AddHeader
 *  allow addition of an arbitrary HTTP header to RESPONSE
 *  this can be used to repair or normalize odd server behavior
 *
 */
LIB_EXPORT rc_t CC KClientHttpResultAddHeader ( KClientHttpResult *self,
    const char *name, const char *val, ... )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
    else if ( name == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    /* have to test for empty name */
    else if ( name [ 0 ] == 0 )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcInsufficient );
    else if ( val == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    /* same for empty value fmt string */
    else if ( val [ 0 ] == 0 )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcInsufficient );
    else
    {
        va_list args;
        va_start ( args, val );

        rc = KClientHttpVAddHeader ( & self -> hdrs, false, name, val, args );

        va_end ( args );
    }
    return rc;
}


/* GetHeader
 *  retrieve named header if present
 *  this cand potentially return a comma separated value list
 */
LIB_EXPORT rc_t CC KClientHttpResultGetHeader ( const KClientHttpResult *self, const char *name,
    char *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = 0;

    if ( num_read == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    else
    {
        * num_read = 0;

        if ( self == NULL )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
        else if ( name == NULL )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
        else if ( buffer == NULL && bsize != 0 )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
        else
        {
            rc = KClientHttpFindHeader ( & self -> hdrs, name, buffer, bsize, num_read );
        }
    }

    return rc;
}

#if _DEBUGGING
static
void PrintHeaders ( BSTNode *n, void *ignore )
{
    KHttpHeader *node = ( KHttpHeader * ) n;

    KOutMsg ( "%S: %S\n",
              & node -> name,
              & node -> value );
}
#endif

/* TestHeaderValue
 *  test for existence of header and a particular value
 *  if the header exists and has a comma-separated list of values,
 *  test each value individually, i.e. splits on comma before comparison
 */
LIB_EXPORT bool CC KClientHttpResultTestHeaderValue ( const KClientHttpResult *self,
    const char *name, const char *value )
{
    if ( value != NULL && value [ 0 ] != '\0' )
    {
        size_t num_read;
        char buffer [ 4096 ], * p = buffer;
        rc_t rc = KClientHttpResultGetHeader ( self, name, buffer, sizeof buffer, & num_read );
        if ( rc != 0 )
        {
            /* we're only interested in a case where the 4K buffer was too small */
            if ( GetRCObject ( rc ) == ( enum RCObject ) rcBuffer && GetRCState ( rc ) == rcInsufficient )
            {
                /* allocate dynamic memory and try again */
                p = malloc ( num_read + 1 );
                if ( p != 0 )
                    rc = KClientHttpResultGetHeader ( self, name, p, num_read + 1, & num_read );
            }
        }

        if ( rc == 0 )
        {
            size_t val_size;
            const char * start, * end;

            /* must either be stack buffer or malloc'd memory */
            assert ( p != NULL );

            /* the header value start and end */
            start = p;
            end = p + num_read;
            assert ( end [ 0 ] == '\0' );

            /* the size in bytes of value string */
            val_size = string_size ( value );

            do
            {
                /* look for a comma */
                const char * sep = string_chr ( start, end - start, ',' );

                /* if not found, use the end of the header value */
                if ( sep == NULL )
                    sep = end;

                /* test for case-insensitive match of value */
                if ( strcase_cmp ( start, sep - start, value, val_size, -1 ) == 0 )
                {
                    /* found it - delete p if malloc'd */
                    if ( p != buffer && p != NULL )
                        free ( p );
                    return true;
                }

                /* move to next value in list or beyond end */
                start = sep + 1;
            }
            while ( start < end );
        }

        /* if we malloc'd memory */
        if ( p != buffer && p != NULL )
            free ( p );
    }

    /* not found */
    return false;
}


/* GetInputStream
 *  access the body of response as a stream
 *  only reads are supported
 *
 *  "s" [ OUT ] - return parameter for input stream reference
 *   must be released via KStreamRelease
 */
LIB_EXPORT rc_t CC KClientHttpResultGetInputStream ( KClientHttpResult *self, KStream ** s )
{
    rc_t rc;

    if ( s == NULL )
        rc = RC ( rcNS, rcNoTarg, rcValidating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcNoTarg, rcValidating, rcSelf, rcNull );
        else
        {
            char buffer [ 512 ];
            size_t num_read = 0;
            uint64_t content_length = 0;

            /* find header to check for type of data being received
               assign bytes read from value to num_read */
            rc = KClientHttpResultGetHeader ( self, "Transfer-Encoding", buffer, sizeof buffer, & num_read );
            if ( rc == 0 && num_read > 0 )
            {
                /* check if chunked encoding */
                if ( strcase_cmp ( "chunked", sizeof "chunked" - 1,
                    buffer, num_read, sizeof "chunked" - 1 ) == 0 )
                {
                    return KClientHttpStreamMakeChunked ( self -> http, s, "KClientHttpStreamChunked" );
                }
                /* TBD - print actual value */
                LOGERR ( klogSys, rc, "Transfer-Encoding does not provide a value" );

            }
            /* get the content length of the entire stream if known */
            if ( KClientHttpResultSize ( self, & content_length ) )
                return KClientHttpStreamMake ( self -> http, s, "KClientHttpStream", content_length, false );

            /* detect connection: close or pre-HTTP/1.1 dynamic content */
            if ( self -> http -> close_connection ||
                 self -> version < 0x01010000 )
            {
                return KClientHttpStreamMake ( self -> http, s, "KClientHttpStream", 0, true );
            }

#if _DEBUGGING
            KOutMsg ( "HTTP/%.2V %03u %S\n", self -> version, self -> status, & self -> msg );
            BSTreeForEach ( & self -> hdrs, false, PrintHeaders, NULL );
#endif

            rc = RC ( rcNS, rcNoTarg, rcValidating, rcMessage, rcUnsupported );
            LOGERR ( klogInt, rc, "HTTP response does not give content length" );

        }
    }

    * s = NULL;

    return rc;
}

static void KClientHttpGetEndpoint ( const KClientHttp * self, KEndPoint * ep, bool remote ) {
    assert ( ep );
    memset ( ep, 0, sizeof * ep );
    if ( self != NULL )
        * ep = remote ? self -> ep : self -> local_ep;
}

void KClientHttpGetRemoteEndpoint ( const KClientHttp * self, KEndPoint * ep )
{   KClientHttpGetEndpoint ( self, ep, true ); }

void KClientHttpGetLocalEndpoint ( const KClientHttp * self, KEndPoint * ep )
{   KClientHttpGetEndpoint ( self, ep, false ); }

LIB_EXPORT rc_t CC KClientHttpResultFormatMsg (
    const struct KClientHttpResult * self, struct KDataBuffer * buffer, const char * bol, const char * eol )
{
    rc_t rc = 0;

    if ( self == NULL ) {
        return RC ( rcNS, rcNoTarg, rcReading, rcSelf, rcNull );
    }

    if ( bol == NULL || eol == NULL ) {
        return RC ( rcNS, rcNoTarg, rcReading, rcParam, rcNull );
    }

    rc = KDataBufferPrintf ( buffer, "%sHTTP/%.2V %d %S%s"
            , bol
            , self -> version
            , self -> status
            , & self -> msg, eol
        );

    if ( rc == 0 )
    {
        const KHttpHeader * node = NULL;
        for ( node = ( const KHttpHeader* ) BSTreeFirst ( & self -> hdrs );
              node != NULL;
              node = ( const KHttpHeader* ) BSTNodeNext ( & node -> dad ) )
        {
            /* add header line */
            rc = KDataBufferPrintf ( buffer, "%s%S: %S\r%s"
                                            , bol
                                            , & node -> name
                                            , & node -> value
                                            , eol );
        }
    }

    return rc;
}
