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

struct KTLSStream;
#define KSTREAM_IMPL struct KTLSStream

#include <kns/extern.h>
#include <kns/manager.h>
#include <kns/tls.h>
#include <kns/impl.h>
#include <kns/socket.h>
#include <klib/rc.h>
#include <klib/status.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/text.h>
#include <kproc/timeout.h>

#include <os-native.h>

#include <sysalloc.h>

#include "mgr-priv.h"
#include "stream-priv.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#if ! defined ( MBEDTLS_CONFIG_FILE )
#include <mbedtls/config.h>
#else
#include MBEDTLS_CONFIG_FILE 
#endif

#include <mbedtls/net.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/aes.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>


/*****************************
 *******     NOTES     *******

  6. update Windows syssock.c for RC

*****************************/


/*--------------------------------------------------------------------------
 * mbedtls_strerror
 */

static
const char * mbedtls_strerror2 ( int err )
{
    static char buffer [ 256 ];
    mbedtls_strerror ( err, buffer, sizeof buffer );
    return buffer;
}

/*--------------------------------------------------------------------------
 * KTLSGlobals
 */

static
rc_t tlsg_seed_rng ( KTLSGlobals *self )
{
    int ret;
    const char *pers = "vdb_tls_client";
    const size_t pers_size = sizeof "vdb_tls_client" - 1;

    STATUS ( STAT_QA, "Seeding the random number generator\n" );

    ret = mbedtls_ctr_drbg_seed ( &self -> ctr_drbg, mbedtls_entropy_func, 
                                  &self -> entropy, ( const unsigned char * ) pers, pers_size );

    if ( ret != 0 )
    {
        rc_t rc = RC ( rcKrypto, rcRng, rcInitializing, rcSeed, rcFailed );
        PLOGERR ( klogSys, ( klogSys, rc
                             , "mbedtls_ctr_drbg_seed returned $(ret) ( $(expl) )"
                             , "ret=%d,expl=%s"
                             , ret
                             , mbedtls_strerror2 ( ret )
                      ) );
        return rc;
    }
   
    return 0;
}

static 
rc_t tlsg_init_certs ( KTLSGlobals *self )
{
    int ret;

    STATUS ( STAT_QA, "Loading the CA root certificate\n" );

    ret = mbedtls_x509_crt_parse ( &self -> cacert, (const unsigned char *) mbedtls_test_cas_pem,
                                   mbedtls_test_cas_pem_len );
    if ( ret < 0 )
    {        
        rc_t rc = RC ( rcKrypto, rcToken, rcInitializing, rcEncryption, rcFailed );
        PLOGERR ( klogSys, ( klogSys, rc
                             , "mbedtls_x509_crt_parse returned $(ret) ( $(expl) )"
                             , "ret=%d,expl=%s"
                             , ret
                             , mbedtls_strerror2 ( ret )
                      ) );
        return rc;
    }
   
    return 0;
}

static
rc_t tlsg_setup ( KTLSGlobals * self )
{
    int ret;

    STATUS ( STAT_QA, "Configuring SSl defaults\n" );

    ret = mbedtls_ssl_config_defaults ( &self -> config,
                                        MBEDTLS_SSL_IS_CLIENT,
                                        MBEDTLS_SSL_TRANSPORT_STREAM,
                                        MBEDTLS_SSL_PRESET_DEFAULT );

    if ( ret != 0 )
    {
        rc_t rc = RC ( rcKrypto, rcNoTarg, rcFormatting, rcEncryption, rcFailed );
        PLOGERR ( klogSys, ( klogSys, rc
                             , "mbedtls_ssl_config_defaults returned $(ret) ( $(expl) )"
                             , "ret=%d,expl=%s"
                             , ret
                             , mbedtls_strerror2 ( ret )
                      ) );
        return rc;
    }

    mbedtls_ssl_conf_authmode( &self -> config, MBEDTLS_SSL_VERIFY_NONE );
    mbedtls_ssl_conf_ca_chain( &self -> config, &self -> cacert, NULL );
    mbedtls_ssl_conf_rng( &self -> config, mbedtls_ctr_drbg_random, &self -> ctr_drbg );

    return 0;
}

/* Init
 */
rc_t KTLSGlobalsInit ( KTLSGlobals * tlsg )
{
    rc_t rc;

    mbedtls_x509_crt_init ( &tlsg -> cacert );
    mbedtls_ctr_drbg_init ( &tlsg -> ctr_drbg );
    mbedtls_entropy_init ( &tlsg -> entropy );
    mbedtls_ssl_config_init ( &tlsg -> config );

    rc = tlsg_seed_rng ( tlsg );
    if ( rc == 0 )
    {
        rc = tlsg_init_certs ( tlsg );
        if ( rc == 0 )
            rc = tlsg_setup ( tlsg );
    }

    return rc;
}

/* Whack
 */
void KTLSGlobalsWhack ( KTLSGlobals * self )
{
    mbedtls_ssl_config_free ( &self -> config );
    mbedtls_entropy_free ( &self -> entropy );
    mbedtls_ctr_drbg_free ( &self -> ctr_drbg );
    mbedtls_x509_crt_free ( &self -> cacert );

    memset ( self, 0, sizeof * self );
}

/*--------------------------------------------------------------------------
 * KTLSStream
 */

struct KTLSStream
{
    KStream dad;

    /* hold a reference to manager */
    const KNSManager * mgr;

    /* hold a reference to ciphertext object */
    KStream * ciphertext;

    /* optional timeout */
    struct timeout_t * tm;

    /* mbed library specific data */
    mbedtls_ssl_context ssl;

    /* error returned from ciphertext stream */
    rc_t rd_rc;
    rc_t wr_rc;
};

static
void KTLSStreamDestroy ( KTLSStream *self )
{
    /* tear down all of the stuff created during Make */
    mbedtls_ssl_close_notify( &self -> ssl ); /* close connection - this might need to be elsewhere */
    mbedtls_ssl_free ( &self -> ssl );

    /* release the ciphertext object */
    KStreamRelease ( self -> ciphertext );
    self -> ciphertext = NULL;
}

static
rc_t CC KTLSStreamWhack ( KTLSStream *self )
{
    KTLSStreamDestroy ( self );

    /* release the manager */
    KNSManagerRelease ( self -> mgr );
    self -> mgr = NULL;

    /* done */
    free ( self );
    return 0;
}

static
rc_t CC KTLSStreamRead ( const KTLSStream * cself,
    void * buffer, size_t bsize, size_t * num_read )
{
    int ret;
    rc_t rc = 0;
    KTLSStream * self = ( KTLSStream * ) cself;

    if ( self -> ciphertext == NULL )
    {
        * num_read = 0;
        return RC ( rcNS, rcSocket, rcReading, rcSocket, rcInvalid );
    }

    STATUS ( STAT_QA, "Reading from server..." );

    self -> rd_rc = 0;

    while ( 1 )
    {
        /* read through TLS library */
        ret = mbedtls_ssl_read( &self -> ssl, buffer, bsize );

        /* no error */
        if ( ret >= 0 )
        {
            STATUS ( STAT_PRG, "%u bytes read", ret );
            break;
        }

        /* detect error at socket level */
        if ( self -> rd_rc != 0 )
        {
            rc = self -> rd_rc;
            PLOGERR ( klogSys, ( klogSys, rc
                                 , "mbedtls_ssl_read returned $(ret) ( $(expl) )"
                                 , "ret=%d,expl=%s"
                                 , ret
                                 , mbedtls_strerror2 ( ret )
                          ) );

            ret = 0;
            self -> rd_rc = 0;
            break;
        }
    
        /* this is a TLS error */
        switch ( ret )
        {
            /* The ret is anything other than the following 3, then the ssl context becomes
             * becomes unusable and should either be freed or call
             * mbedtls_ssl_session_reset () before a new connection; current connection
             * must be closed
             */
        case MBEDTLS_ERR_SSL_WANT_READ: 
        case MBEDTLS_ERR_SSL_WANT_WRITE:
            continue; /* TBD - allow the app to check signals */
        case MBEDTLS_ERR_SSL_CLIENT_RECONNECT: 
            /* can only happen server-side:   When this function return MBEDTLS_ERR_SSL_CLIENT_RECONNECT
             * (which can only happen server-side), it means that a client
             * is initiating a new connection using the same source port.
             * You can either treat that as a connection close and wait
             * for the client to resend a ClientHello, or directly
             * continue with \c mbedtls_ssl_handshake() with the same
             * context (as it has beeen reset internally). Either way, you
             * should make sure this is seen by the application as a new
             * connection: application state, if any, should be reset, and
             * most importantly the identity of the client must be checked
             * again. WARNING: not validating the identity of the client
             * again, or not transmitting the new identity to the
             * application layer, would allow authentication bypass!*/
            rc = RC ( rcNS, rcSocket, rcReading, rcMode, rcUnsupported );
            break;

        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY: /* check - only place their advanced sample client checks for this */
            break;
        case MBEDTLS_ERR_NET_CONN_RESET:
            rc = RC ( rcNS, rcSocket, rcWriting, rcConnection, rcCanceled );
            break;
        case MBEDTLS_ERR_SSL_BAD_INPUT_DATA:
            rc = RC ( rcNS, rcSocket, rcReading, rcParam, rcInvalid );
            break;
        case MBEDTLS_ERR_SSL_INTERNAL_ERROR: /* should never happen */
        case MBEDTLS_ERR_SSL_UNEXPECTED_MESSAGE:
        default:
            rc = RC ( rcNS, rcSocket, rcReading, rcError, rcUnexpected );
        }

        /* invalidate the stream for any further use */
        KTLSStreamDestroy ( self );
        ret = 0;
        break;
    }

    *num_read = ret;
    return rc;
}

static
rc_t CC KTLSStreamWrite ( KTLSStream * self,
    const void * buffer, size_t size, size_t * num_writ )
{
    int ret;
    rc_t rc = 0;

    if ( self -> ciphertext == NULL )
    {
        * num_writ = 0;
        return RC ( rcNS, rcSocket, rcWriting, rcSocket, rcInvalid );
    }

    STATUS ( STAT_PRG, "Writing %zu bytes to to server\n", size );
    
    self -> wr_rc = 0;

    while ( 1 )
    {
        /* write through TLS library 
        *  This function will do partial writes in some cases. If the
        *  return value is non-negative but less than length, the
        *  function must be called again with updated arguments:
        *  buf + ret, len - ret (if ret is the return value) until
        *  it returns a value equal to the last 'len' argument.
        *
        *  We expect to be called through KStreamWriteAll that will
        *  avoid the issue above. 
        */
        ret = mbedtls_ssl_write ( &self -> ssl, buffer, size );

        /* no error */
        if ( ret >= 0 )
        {
            STATUS ( STAT_PRG, "%u bytes written", ret );
            break;
        }

        /* detect error at socket level */
        if ( self -> wr_rc != 0 )
        {
            rc = self -> wr_rc;
            PLOGERR ( klogSys, ( klogSys, rc
                                 , "mbedtls_ssl_write returned $(ret) ( $(expl) )"
                                 , "ret=%d,expl=%s"
                                 , ret
                                 , mbedtls_strerror2 ( ret )
                          ) );
            ret = 0;
            self -> wr_rc = 0;
            break;
        }

        /* this is a TLS error */
        switch ( ret )
        {
        case MBEDTLS_ERR_SSL_WANT_READ:
        case MBEDTLS_ERR_SSL_WANT_WRITE:
            continue;
        case MBEDTLS_ERR_SSL_BAD_INPUT_DATA:
            rc = RC ( rcNS, rcSocket, rcReading, rcParam, rcInvalid );
            break;
        default:
            rc = RC ( rcNS, rcSocket, rcWriting, rcError, rcUnknown );
            break;
        }

        ret = 0;
        break;
    }

    * num_writ = ret;
    return rc;
}

static
rc_t CC KTLSStreamTimedRead ( const KTLSStream *cself,
    void * buffer, size_t bsize, size_t * num_read, struct timeout_t * tm )
{
    rc_t rc;
    KTLSStream * self = ( KTLSStream * ) cself;

    self -> tm = tm;
    rc = KTLSStreamRead ( self, buffer, bsize, num_read );
    self -> tm = NULL;

    return rc;
}

static
rc_t CC KTLSStreamTimedWrite ( KTLSStream *self,
    const void * buffer, size_t size, size_t * num_writ, struct timeout_t * tm )
{
    rc_t rc;

    self -> tm = tm;
    rc = KTLSStreamWrite ( self, buffer, size, num_writ );
    self -> tm = NULL;

    return rc;
}


static KStream_vt_v1 vtKTLSStream =
{
    1, 1,

    KTLSStreamWhack,
    KTLSStreamRead,
    KTLSStreamWrite,
    KTLSStreamTimedRead,
    KTLSStreamTimedWrite
};

static
int CC ktls_net_send ( void *ctx, const unsigned char *buf, size_t len )
{
    KTLSStream * self = ctx;

    size_t num_writ;

    if ( self -> tm != NULL )
        self -> wr_rc = KStreamTimedWriteAll ( self -> ciphertext, buf, len, & num_writ, self -> tm );
    else
        self -> wr_rc = KStreamWriteAll ( self -> ciphertext, buf, len, & num_writ );

    if ( self -> wr_rc != 0 )
    {
        switch ( GetRCObject ( self -> wr_rc ) )
        {
        case rcConnection: /* EPIPE && ECONNRESET */
            if ( GetRCState ( self -> wr_rc ) == rcCanceled ||
                 GetRCState ( self -> wr_rc ) == rcInterrupted )
                return MBEDTLS_ERR_NET_CONN_RESET;
        case rcTransfer: /* EINTR */
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        default:
            return MBEDTLS_ERR_NET_SEND_FAILED;
        }
    }

    return ( int ) num_writ;
}

/* called by mbedtls_ssl_fetch_input */
static
int CC ktls_net_recv ( void *ctx, unsigned char *buf, size_t len )
{
    size_t num_read;
    KTLSStream * self = ctx;

    if ( self -> tm != NULL )
        self -> rd_rc = KStreamTimedRead ( self -> ciphertext, buf, len, & num_read, self -> tm );
    else
        self -> rd_rc = KStreamRead ( self -> ciphertext, buf, len, & num_read );

    if ( self -> rd_rc != 0 )
    {
        /* TBD - discover if the read timed out - possibly return MBEDTLS_ERR_SSL_WANT_READ */
        switch ( GetRCObject ( self -> rd_rc ) )
        {
        case rcData:
            return MBEDTLS_ERR_SSL_TIMEOUT;
        case rcConnection: /* EPIPE && ECONNRESET */
            if ( GetRCState ( self -> rd_rc ) == rcCanceled ||
                 GetRCState ( self -> rd_rc ) == rcInterrupted )
                return MBEDTLS_ERR_NET_CONN_RESET;
        case rcTransfer: /* EINTR */
            return MBEDTLS_ERR_SSL_WANT_READ;
        default:
            return MBEDTLS_ERR_NET_RECV_FAILED;
        }
    }

    return ( int ) num_read;
}

static 
rc_t ktls_ssl_setup ( KTLSStream *self, const String *host )
{
    int ret;
    const String * hostz;
    const KTLSGlobals * tlsg;

    STATUS ( STAT_QA, "Setting up SSL/TLS structure" );

    assert ( self -> mgr != NULL );
    tlsg = & self -> mgr -> tlsg;

    ret = mbedtls_ssl_setup( &self -> ssl, &tlsg -> config );
    if ( ret != 0 )
    {
        rc_t rc = RC ( rcKrypto, rcSocket, rcFormatting, rcEncryption, rcFailed );
        PLOGERR ( klogSys, ( klogSys, rc
                             , "mbedtls_ssl_setup returned $(ret) ( $(expl) )"
                             , "ret=%d,expl=%s"
                             , ret
                             , mbedtls_strerror2 ( ret )
                      ) );
        return rc;
    }

    /* HEURISTIC - no memory pages will be less than 4K in size.
       we can safely test for a NUL byte at the end of the address
       only if we know that this address is not exactly at a potential
       page boundary. If the NUL byte would be on a potential page boundary,
       it is not safe to test and we must reallocate.

       Also, if not NUL terminated.

       Finally, this will make Valgrind lose its mind...
    */

#if ! _DEBUGGING
    assert ( host != NULL );
    assert ( host -> addr != NULL );

    if ( ( ( & host -> addr [ host -> size ] ) & ( 4096 - 1 ) ) != 0 &&
         host -> addr [ host -> size ] == 0 )
    {
        hostz = host;
    }
    else
#endif
    {
        rc_t rc = StringCopy ( & hostz, host );
        if ( rc != 0 )
            return rc;
    }

    ret = mbedtls_ssl_set_hostname( &self -> ssl, hostz -> addr );

    if ( hostz != host )
        StringWhack ( hostz );

    if ( ret != 0 )
    {
        rc_t rc = RC ( rcKrypto, rcSocket, rcFormatting, rcEncryption, rcFailed );
        PLOGERR ( klogSys, ( klogSys, rc
                             , "mbedtls_ctr_drbg_seed returned $(ret) ( $(expl) )"
                             , "ret=%d,expl=%s"
                             , ret
                             , mbedtls_strerror2 ( ret )
                      ) );
        return rc;
    }


    mbedtls_ssl_set_bio( &self -> ssl, ( void * ) self, ktls_net_send, ktls_net_recv, NULL );

    return 0;
}

static 
rc_t ktls_handshake ( KTLSStream *self )
{
    int ret;

    STATUS ( STAT_QA, "Performing SSL/TLS handshake...\n" );

    ret = mbedtls_ssl_handshake( &self -> ssl );
    while ( ret != 0 )
    {
        if ( ret != MBEDTLS_ERR_SSL_WANT_READ && 
             ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            rc_t rc = RC ( rcKrypto, rcSocket, rcOpening, rcConnection, rcFailed );
            PLOGERR ( klogSys, ( klogSys, rc
                                 , "mbedtls_ssl_handshake returned $(ret) ( $(expl) )"
                                 , "ret=%d,expl=%s"
                                 , ret
                                 , mbedtls_strerror2 ( ret )
                          ) );
            return rc;
        }
    }

    return 0;
}


static
rc_t KTLSStreamMake ( KTLSStream ** objp, const KNSManager * mgr, const KSocket *ciphertext )
{
    rc_t rc;
    KTLSStream * obj;

    STATUS ( STAT_PRG, "%s\n", __func__ );

    obj = calloc ( 1, sizeof * obj );
    if ( obj == NULL )
        rc = RC ( rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted );
    else
    {
        /* initialize the stream parent */
        STATUS ( STAT_PRG, "%s - initializing KStream\n", __func__ );
        rc = KStreamInit ( & obj -> dad, ( const KStream_vt* ) & vtKTLSStream, "KTLSStream", "", true, true );
        if ( rc == 0 )
        {
            STATUS ( STAT_GEEK, "%s - attaching to KNSManager\n", __func__ );
            rc = KNSManagerAddRef ( mgr );
            if ( rc == 0 )
            {
                STATUS ( STAT_GEEK, "%s - accessing KStream from socket\n", __func__ );
                rc = KSocketGetStream ( ciphertext, & obj -> ciphertext );
                if ( rc == 0 )
                {
                    obj -> mgr = mgr;

                    STATUS ( STAT_PRG, "%s - initializing tls wrapper\n", __func__ );
                    mbedtls_ssl_init ( &obj -> ssl );

                    * objp = obj;
                    return 0;
                }

                KNSManagerRelease ( mgr );
            }
        }

        free ( obj );
    }

    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS ), ( "Failed to create TLS stream: %R\n", rc ) );

    * objp = NULL;
    return rc;
}


/* MakeTLSStream
 *  create a TLS wrapper upon an existing KStream
 *
 *  data read from or written to TLS stream is in plaintext
 *  data from to or wrapped stream is in ciphertext
 *
 *  "plaintext" [ OUT ] - return parameter for TLS stream
 *
 *  "ciphertext" [ IN ] - existing stream of ciphertext
 *
 *  "host" [ IN ] - host dns name
 */
LIB_EXPORT rc_t CC KNSManagerMakeTLSStream ( const KNSManager * self,
    KTLSStream ** plaintext, const KSocket * ciphertext, const String * host )
{
    rc_t rc;

    if ( plaintext == NULL )
        rc = RC ( rcNS, rcMgr, rcAllocating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcMgr, rcAllocating, rcSelf, rcNull );
        else if ( ciphertext == NULL )
            rc = RC ( rcNS, rcMgr, rcAllocating, rcParam, rcNull );
        else
        {
            KTLSStream * ktls;

            /* create KTLSStream object */
            rc = KTLSStreamMake ( & ktls, self, ciphertext );
            if ( rc == 0 )
            {
                /* perform all initialization possible with information given */
                rc = ktls_ssl_setup ( ktls, host );
                if ( rc == 0 )
                {
                    rc = ktls_handshake ( ktls );
                    if ( rc == 0 )
                    {
                        ktls -> mgr = self;
                        *plaintext = ktls;
                        return 0;
                    }
                }

                KTLSStreamWhack ( ktls );
            }
        }

        * plaintext = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KTLSStreamAddRef ( const KTLSStream * self )
{
    /* works because address of self == address of self -> dad,
       and if self == NULL, & self -> dad == NULL */
    return KStreamAddRef ( & self -> dad );
}

LIB_EXPORT rc_t CC KTLSStreamRelease ( const KTLSStream * self )
{
    /* works because address of self == address of self -> dad,
       and if self == NULL, & self -> dad == NULL */
    return KStreamRelease ( & self -> dad );
}

/* VerifyCACert
 */
LIB_EXPORT rc_t CC KTLSStreamVerifyCACert ( const KTLSStream * self )
{
    rc_t rc;

   STATUS ( STAT_QA, "Verifying peer X.509 certificate..." );
   
   if ( self == NULL )
       rc = RC ( rcKrypto, rcToken, rcValidating, rcSelf, rcNull );
   else
   {
       uint32_t flags = mbedtls_ssl_get_verify_result( &self -> ssl );
       if ( flags != 0 )
       {
           char buf [ 512 ];
           rc_t rc = RC ( rcKrypto, rcToken, rcValidating, rcEncryption, rcFailed );

           mbedtls_x509_crt_verify_info ( buf, sizeof( buf ), "  ! ", flags );        

           PLOGERR ( klogSys, ( klogSys, rc
                                , "mbedtls_ctr_drbg_seed returned $(ret) ( $(expl) )"
                                , "buffer=%s"
                                , &buf
                         ) );
           return rc;
       }
       
       rc = 0;
   }
   
   return rc;
}


/* GetStream
 *  reference-counted cast operation
 *  creates a reference to a KStream from a KTLSStream
 */
LIB_EXPORT rc_t CC KTLSStreamGetStream ( const KTLSStream * self,
    struct KStream ** strm )
{
    rc_t rc;

    if ( strm == NULL )
        rc = RC ( rcNS, rcStream, rcValidating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcStream, rcValidating, rcSelf, rcNull );
        else if ( self -> dad . vt != ( const KStream_vt* ) & vtKTLSStream )
            rc = RC (rcNS, rcStream, rcValidating, rcInterface, rcNull );
        else
        {
            /* at this point, the object must be in a fully
               constructed and validated state. If not, it
               is an error to access it as a stream. */

            rc = KStreamAddRef ( & self -> dad );
            if ( rc == 0 )
            {
                * strm = & ( ( KTLSStream * ) self ) -> dad;
                return 0;
            }
        }

        * strm = NULL;
    }

    return rc;
}

