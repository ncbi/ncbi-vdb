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

/**
* Integration tests for mbed_tls
*/

#if WINDOWS
#pragma comment(lib, "crypt32.lib")

#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>
#endif

#include <ktst/unit_test.hpp>

#include <mbedtls/x509_crt.h>
#include <mbedtls/ssl.h>
#include <mbedtls/error.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

#include <kns/stream.h>
#include <kns/manager.h>
#include <kns/endpoint.h>
#include <kns/socket.h>

#include <klib/rc.h>
#include <klib/text.h>

using namespace std;

TEST_SUITE(MbedTlsTestSuite);

struct BasicIO
{
    KStream * ciphertext;
    KSocket * conn;

    BasicIO( const char * hostname ) : ciphertext ( nullptr )
    {
        KNSManager * kns;
        THROW_ON_RC( KNSManagerMake ( & kns ) );
        KEndPoint ep;
        String strEp;
        CONST_STRING( & strEp, "www.nlm.nih.gov" );
        THROW_ON_RC( KNSManagerInitDNSEndpoint ( kns, & ep, & strEp, 443 ) );
        THROW_ON_RC( KNSManagerMakeConnection ( kns, & conn, NULL, & ep ) );
        THROW_ON_RC( KSocketGetStream ( conn, & ciphertext ) );

        THROW_ON_RC( KNSManagerRelease ( kns ) );
    }
    ~BasicIO()
    {
        KStreamRelease ( ciphertext );
        KSocketRelease ( conn );
    }
    int Read(unsigned char * buf, size_t len)
    {
        size_t num_read;
        rc_t rc = KStreamRead ( ciphertext, buf, len, & num_read );
        if ( rc != 0 )
        {
            switch ( ( int ) GetRCObject ( rc ) )
            {
            case rcData:
                return MBEDTLS_ERR_SSL_TIMEOUT;
            case rcConnection: /* EPIPE && ECONNRESET */
                if ( GetRCState ( rc ) == rcCanceled ||
                    GetRCState ( rc ) == rcInterrupted )
                    return MBEDTLS_ERR_NET_CONN_RESET;
            case rcTransfer: /* EINTR */
                return MBEDTLS_ERR_SSL_WANT_READ;
            default:
                return MBEDTLS_ERR_NET_RECV_FAILED;
            }
        }

        return ( int ) num_read;
    }

    int Write(const unsigned char * buf, size_t len)
    {
        size_t num_writ;
        rc_t rc = KStreamWriteAll ( ciphertext, buf, len, & num_writ );
        if ( rc != 0 )
        {
            switch ( GetRCObject ( rc ) )
            {
            case rcConnection: /* EPIPE && ECONNRESET */
                if ( GetRCState ( rc ) == rcCanceled ||
                    GetRCState ( rc ) == rcInterrupted )
                    return MBEDTLS_ERR_NET_CONN_RESET;
            case rcTransfer: /* EINTR */
                return MBEDTLS_ERR_SSL_WANT_WRITE;
            default:
                return MBEDTLS_ERR_NET_SEND_FAILED;
            }
        }

        return ( int ) num_writ;
    }
};

int KTLSStreamRead ( void * cself, unsigned char * buffer, size_t len )
{
    BasicIO * self = static_cast<BasicIO *>(cself);
    return self -> Read(buffer, len);
}

int KTLSStreamWrite ( void * cself, const unsigned char * buffer, size_t len )
{
    BasicIO * self = static_cast<BasicIO *>(cself);
    return self -> Write(buffer, len);
}

class MbedTlsFixture
{
public:
    MbedTlsFixture()
    {
        vdb_mbedtls_ssl_init( & m_ssl );

        vdb_mbedtls_x509_crt_init( & m_chain);
        vdb_mbedtls_entropy_init ( & m_entropy );
        vdb_mbedtls_ssl_config_init ( & m_config );

        vdb_mbedtls_ctr_drbg_init ( & m_ctr_drbg );
        string pers = "test-mbedtls";
        vdb_mbedtls_ctr_drbg_seed ( &m_ctr_drbg, vdb_mbedtls_entropy_func,
                                  &m_entropy, (const unsigned char*)pers.c_str(), pers.size() );
    }
    ~MbedTlsFixture()
    {
        vdb_mbedtls_ssl_config_free ( & m_config );
        vdb_mbedtls_entropy_free ( & m_entropy );
        vdb_mbedtls_ctr_drbg_free ( & m_ctr_drbg );
        vdb_mbedtls_x509_crt_free( & m_chain );

        vdb_mbedtls_ssl_free( & m_ssl );
    }

    bool Config( BasicIO & bio, bool allow_all_certs = true )
    {
        if ( 0 != vdb_mbedtls_ssl_config_defaults ( & m_config,
                                            MBEDTLS_SSL_IS_CLIENT,
                                            MBEDTLS_SSL_TRANSPORT_STREAM,
                                            MBEDTLS_SSL_PRESET_DEFAULT ) )
        {
            return false;
        }
        vdb_mbedtls_ssl_conf_dbg( & m_config, NULL, NULL ); // if not called, valgrind complains

        vdb_mbedtls_ssl_conf_authmode( & m_config, allow_all_certs ? MBEDTLS_SSL_VERIFY_OPTIONAL : MBEDTLS_SSL_VERIFY_REQUIRED );

        vdb_mbedtls_ssl_conf_ca_chain( & m_config, & m_chain, NULL );
        vdb_mbedtls_ssl_conf_rng( & m_config, vdb_mbedtls_ctr_drbg_random, & m_ctr_drbg );
        vdb_mbedtls_ssl_set_bio( & m_ssl, & bio, KTLSStreamWrite, KTLSStreamRead, NULL );
        if ( 0 != vdb_mbedtls_ssl_setup( & m_ssl, & m_config ) )
        {
            return false;
        }

        return true;
    }

    mbedtls_x509_crt m_chain;
    mbedtls_ssl_context m_ssl;
    mbedtls_ssl_config m_config;
    mbedtls_ctr_drbg_context m_ctr_drbg;
    mbedtls_entropy_context m_entropy;
};

FIXTURE_TEST_CASE(ParseCert, MbedTlsFixture)
{
    REQUIRE_EQ ( 0, vdb_mbedtls_x509_crt_parse_file( & m_chain, "./ca-certificates.crt" ) );
}

FIXTURE_TEST_CASE(Config, MbedTlsFixture)
{
    REQUIRE_EQ ( 0, vdb_mbedtls_x509_crt_parse_file( & m_chain, "./ca-certificates.crt" ) );
    BasicIO bio( "www.nlm.nih.gov" );
    REQUIRE( Config( bio ) );
}
FIXTURE_TEST_CASE(Handshake_AllowAll, MbedTlsFixture)
{
    REQUIRE_EQ ( 0, vdb_mbedtls_x509_crt_parse_file( & m_chain, "./ca-certificates.crt" ) );
    BasicIO bio( "www.nlm.nih.gov" );
    REQUIRE( Config( bio, true )  );
    REQUIRE_EQ ( 0, vdb_mbedtls_ssl_set_hostname( & m_ssl, "www.nlm.nih.gov" ) );
    int ret = vdb_mbedtls_ssl_handshake( & m_ssl );
    if ( ret != 0 )
    {
        char buf[1024];
        vdb_mbedtls_strerror( ret, buf, sizeof ( buf ) );
        cout << buf << endl;
    }
    REQUIRE_EQ ( 0, ret );
}

FIXTURE_TEST_CASE(Handshake_Success, MbedTlsFixture)
{
    REQUIRE_EQ ( 0, vdb_mbedtls_x509_crt_parse_file( & m_chain, "./ca-certificates.crt" ) );
    BasicIO bio("www.nlm.nih.gov");
    REQUIRE(Config(bio, false));
    REQUIRE_EQ(0, vdb_mbedtls_ssl_set_hostname(&m_ssl, "www.nlm.nih.gov"));
    REQUIRE_EQ(0, vdb_mbedtls_ssl_handshake(&m_ssl));
}
FIXTURE_TEST_CASE(Handshake_NoCertFail, MbedTlsFixture)
{
    //not doing this: REQUIRE_EQ ( 0, vdb_mbedtls_x509_crt_parse_file( & m_chain, "./ca-certificates.crt" ) );
    BasicIO bio("www.nlm.nih.gov");
    REQUIRE(Config(bio, false));
    REQUIRE_EQ(0, vdb_mbedtls_ssl_set_hostname(&m_ssl, "www.nlm.nih.gov"));
    REQUIRE_EQ(MBEDTLS_ERR_X509_CERT_VERIFY_FAILED, vdb_mbedtls_ssl_handshake(&m_ssl));
}

#if WINDOWS

FIXTURE_TEST_CASE(WindowsRootStore, MbedTlsFixture)
{
    //-------------------------------------------------------------------
    // Pull all the certificates from the ROOT store. 

    HCERTSTORE hSystemStore = CertOpenSystemStoreA(0, "ROOT");
    REQUIRE_NOT_NULL(hSystemStore);

    PCCERT_CONTEXT pCertContext = NULL;
    while (true)
    {
        pCertContext = CertEnumCertificatesInStore(hSystemStore, pCertContext);
        if (pCertContext == NULL)
        {
            break;
        }

        // ignore errors
        vdb_mbedtls_x509_crt_parse(&m_chain, (const unsigned char*)pCertContext->pbCertEncoded, pCertContext->cbCertEncoded); 
    }
    REQUIRE(CertCloseStore(hSystemStore, 0));

    BasicIO bio("www.nlm.nih.gov");
    REQUIRE(Config(bio, false));
    REQUIRE_EQ(0, vdb_mbedtls_ssl_set_hostname(&m_ssl, "www.nlm.nih.gov"));
    REQUIRE_EQ(0, vdb_mbedtls_ssl_handshake(&m_ssl));
}

#endif

extern "C"
{

#include <kapp/args.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "test-mmbedtls";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=MbedTlsTestSuite(argc, argv);
    return rc;
}

}
