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

#include <kns/manager.h>
#include <kns/extern.h>
#include <kns/manager.h>
#include <kns/tls.h>
#include <kns/impl.h>
#include <kns/socket.h>
#include <klib/rc.h>
#include <klib/status.h>
#include <klib/debug.h>
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
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE 
#endif

#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/aes.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"


#define TLS_PORT "443"


/*--------------------------------------------------------------------------
 * KTLSGlobals
 */

static
rc_t tlsg_seed_rng ( KTLSGlobals *self )
{
    int ret;
    const char *pers = "vdb_tls_client";
    size_t pers_size = string_size ( pers );
    assert ( pers_size == sizeof "vdb_tls_client" - 1 );

    STATUS ( 0, "Seeding the random number generator\n" );

    ret = mbedtls_ctr_drbg_seed ( &self -> ctr_drbg, mbedtls_entropy_func, 
                                  &self -> entropy, ( const unsigned char * ) pers, pers_size );

    if ( ret != 0 )
    {
        STATUS ( 0, "failed...seed returned %d\n", ret );
        return RC ( rcKrypto, rcRng, rcInitializing, rcSeed, rcFailed );
    }
   
    return 0;
}

static 
rc_t tlsg_init_certs ( KTLSGlobals *self )
{
    int ret;

    STATUS ( 0, "Loading the CA root certificate\n" );

    ret = mbedtls_x509_crt_parse ( &self -> cacert, (const unsigned char *) mbedtls_test_cas_pem,
                                   mbedtls_test_cas_pem_len );
    if ( ret < 0 )
    {        
        STATUS ( 0, "failed...crt_parse returned %d\n", -ret );
        return RC ( rcKrypto, rcToken, rcInitializing, rcEncryption, rcFailed );
    }
   
    return 0;
}

static
rc_t tlsg_setup ( KTLSGlobals * self )
{
    int ret = mbedtls_ssl_config_defaults ( &self -> config,
                                        MBEDTLS_SSL_IS_CLIENT,
                                        MBEDTLS_SSL_TRANSPORT_STREAM,
                                        MBEDTLS_SSL_PRESET_DEFAULT );

    if ( ret != 0 )
    {
        STATUS ( 0, "failed...config_defaults returned %d\n", ret );
        return RC ( rcKrypto, rcNoTarg, rcFormatting, rcEncryption, rcFailed );
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
    rc_t rc;
};

static
rc_t CC KTLSStreamWhack ( KTLSStream *self )
{
    /* tear down all of the stuff created during Make */
    mbedtls_ssl_close_notify( &self -> ssl ); /* close connection - this might need to be elsewhere */
    mbedtls_ssl_free ( &self -> ssl );

    /* release the ciphertext object */
    KStreamRelease ( self -> ciphertext );
    self -> ciphertext = NULL;

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
    rc_t rc;
    KTLSStream * self = ( KTLSStream * ) cself;

    STATUS ( 0, "Reading from server..." );

    
    self -> rc = 0;
    ret = mbedtls_ssl_read( &self -> ssl, buffer, bsize );
    STATUS ( 0, "%zu byte read as '%s'", bsize, buffer );
    
    if ( ret < 0 )
    {
        switch ( ret )
        {
     /* case MBEDTLS_ERR_SSL_WANT_READ: ERRNO ( EINTR ) - we deal with this ourselves */
     /* case MBEDTLS_ERR_SSL_WANT_WRITE: ERRNO ( EINTR ) - we deal with this ourselves */
     /* case MBEDTLS_ERR_NET_CONN_RESET: ERRNO ( EPIPE || ECONNRESET ) - we deal with this ourselves */
     /* case MBEDTLS_ERR_SSL_CLOSE_NOTIFY: - we deal with this ourselves */
        case MBEDTLS_ERR_SSL_UNEXPECTED_MESSAGE:
            self -> rc = RC ( rcNS, rcNoTarg, rcReading, rcTransfer, rcUnexpected );
            break;
        default:
            self -> rc = RC ( rcNS, rcNoTarg, rcReading, rcTransfer, rcUnknown );
            break;
        }
    }

    *num_read = ret;

    rc = self -> rc;
    self -> rc = 0;
   
    return rc;
}

static
rc_t CC KTLSStreamWrite ( KTLSStream * self,
    const void * buffer, size_t size, size_t * num_writ )
{
    int ret;

    STATUS ( 0, "Writing to server\n" );
    
    self -> rc = 0;

    ret = mbedtls_ssl_write ( &self -> ssl, buffer, size );
    if ( ret != MBEDTLS_ERR_SSL_WANT_READ && 
         ret != MBEDTLS_ERR_SSL_WANT_WRITE )
    {
        STATUS ( 0, "failed...ssl write returned %d\n", ret );
        return self -> rc;
    }

    assert ( ret > 0 );
    * num_writ = ret;

    return 0;
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
        self -> rc = KStreamTimedWriteAll ( self -> ciphertext, buf, len, & num_writ, self -> tm );
    else
        self -> rc = KStreamWriteAll ( self -> ciphertext, buf, len, & num_writ );

    if ( self -> rc != 0 )
    {
        switch ( GetRCState ( self -> rc ) )
        {
        /* EPIPE - MBEDTLS_ERR_NET_CONN_RESET: broken pipe */
        /* ECONNRESET - MBEDTLS_ERR_NET_CONN_RESET: connection reset */
        /* EINTR - MBEDTLS_ERR_SSL_WANT_READ: we just continue ( shown in unix/sysstream.c ) */    
        default:
            /* anything else is just failed */
            break;
        }
        return MBEDTLS_ERR_NET_SEND_FAILED;
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
        self -> rc = KStreamTimedRead ( self -> ciphertext, buf, len, & num_read, self -> tm );
    else
        self -> rc = KStreamRead ( self -> ciphertext, buf, len, & num_read );

    if ( self -> rc != 0 )
    {
        /* TBD - discover if the read timed out - possibly return MBEDTLS_ERR_SSL_WANT_READ */
        switch ( GetRCState ( self -> rc ) )
        {
        /* EPIPE - MBEDTLS_ERR_NET_CONN_RESET: broken pipe */
        /* ECONNRESET - MBEDTLS_ERR_NET_CONN_RESET:connection reset */
        /* EINTR - MBEDTLS_ERR_SSL_WANT_READ: we just continue ( shown in unix/sysstream.c ) */    
        /* MBEDTLS_ERR_SSL_TIMEOUT */
        default:
            /* anything else is just failed */
            break;
        }

        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    return ( int ) num_read;
}

static 
rc_t ktls_ssl_setup ( KTLSStream *self, const String *host )
{
    int ret;
    const String * hostz;
    const KTLSGlobals * tlsg;

    STATUS ( 0, "Setting up SSL/TLS structure" );

    assert ( self -> mgr != NULL );
    tlsg = & self -> mgr -> tlsg;

    ret = mbedtls_ssl_setup( &self -> ssl, &tlsg -> config );
    if ( ret != 0 )
    {
        STATUS ( 0, "failed...ssl_setup returned %d\n", ret );
        return RC ( rcKrypto, rcNoTarg, rcFormatting, rcEncryption, rcFailed );
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
        STATUS ( 0, "failed...ssl_set_hostname returned %d\n", ret );
        return RC ( rcKrypto, rcNoTarg, rcFormatting, rcEncryption, rcFailed );
    }


    mbedtls_ssl_set_bio( &self -> ssl, ( void * ) self, ktls_net_send, ktls_net_recv, NULL );

    return 0;
}

static 
rc_t ktls_handshake ( KTLSStream *self )
{
    int ret;

    STATUS ( 0, "Performing SSL/TLS handshake...\n" );

    ret = mbedtls_ssl_handshake( &self -> ssl );
    while ( ret != 0 )
    {
        if ( ret != MBEDTLS_ERR_SSL_WANT_READ && 
             ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            STATUS ( 0, "failed...handshake returned -0x%x\n", -ret );
            return RC ( rcKrypto, rcNoTarg, rcOpening, rcConnection, rcFailed );
        }
    }

    return 0;
}


static
rc_t KTLSStreamMake ( KTLSStream ** objp, const KNSManager * mgr, const KSocket *ciphertext )
{
    rc_t rc;
    KTLSStream * obj = calloc ( 1, sizeof * obj );
    if ( obj == NULL )
        rc = RC ( rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted );
    else
    {
        /* initialize the stream parent */
        rc = KStreamInit ( & obj -> dad, ( const KStream_vt* ) & vtKTLSStream, "KTLSStream", "", true, true );
        if ( rc == 0 )
        {
            rc = KNSManagerAddRef ( mgr );
            if ( rc == 0 )
            {
                rc = KSocketGetStream ( ciphertext, & obj -> ciphertext );
                if ( rc == 0 )
                {
                    obj -> mgr = mgr;

                    mbedtls_ssl_init ( &obj -> ssl );

                    * objp = obj;
                    return 0;
                }

                KNSManagerRelease ( mgr );
            }
        }

        free ( obj );
    }

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

   STATUS ( 0, "Verifying peer X.509 certificate..." );
   
   if ( self == NULL )
       rc = RC ( rcKrypto, rcNoTarg, rcValidating, rcSelf, rcNull );
   else
   {
       uint32_t flags = mbedtls_ssl_get_verify_result( &self -> ssl );
       if ( flags != 0 )
       {
           char buf [ 512 ];
           mbedtls_x509_crt_verify_info ( buf, sizeof( buf ), "  ! ", flags );        

           STATUS ( 0, "failed... %s\n", buf );
           return RC ( rcKrypto, rcToken, rcValidating, rcEncryption, rcFailed );
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

