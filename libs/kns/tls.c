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
#include <kns/endpoint.h> /* KEndPoint */
#include <kns/manager.h>
#include <kns/tls.h>
#include <kns/impl.h>
#include <kns/socket.h>
#include <klib/rc.h>
#include <klib/status.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/text.h>
#include <klib/namelist.h>
#include <kproc/timeout.h>
#include <kfg/config.h>
#include <kfs/directory.h>

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

#if WINDOWS
#define IGNORE_ALL_CERTS_ALLOWED 1
#endif

static const char ca_crt_ncbi1 [] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\r\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\r\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n"
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\r\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\r\n"
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\r\n"
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\r\n"
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\r\n"
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\r\n"
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\r\n"
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\r\n"
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\r\n"
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\r\n"
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\r\n"
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\r\n"
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\r\n"
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\r\n"
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4wOTAeBggrBgEFBQcD\r\n"
    "BAYIKwYBBQUHAwEGCCsGAQUFBwMDDBdEaWdpQ2VydCBHbG9iYWwgUm9vdCBDQQ==\r\n"
    "-----END CERTIFICATE-----\r\n";

static const char ca_crt_ncbi2 [] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\r\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
    "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\r\n"
    "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\r\n"
    "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\r\n"
    "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\r\n"
    "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\r\n"
    "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\r\n"
    "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\r\n"
    "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\r\n"
    "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\r\n"
    "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\r\n"
    "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\r\n"
    "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\r\n"
    "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\r\n"
    "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\r\n"
    "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\r\n"
    "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\r\n"
    "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\r\n"
    "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\r\n"
    "+OkuE6N36B9KMEQwHgYIKwYBBQUHAwQGCCsGAQUFBwMBBggrBgEFBQcDAwwiRGln\r\n"
    "aUNlcnQgSGlnaCBBc3N1cmFuY2UgRVYgUm9vdCBDQQ==\r\n"
    "-----END CERTIFICATE-----\r\n";


/*--------------------------------------------------------------------------
 * mbedtls_strerror
 */

static
const char * mbedtls_strerror2 ( int err )
{
    static char buffer [ 256 ];
    vdb_mbedtls_strerror ( err, buffer, sizeof buffer );
    return buffer;
}

/*--------------------------------------------------------------------------
 * KTLSGlobals
 */

static
void ktls_ssl_dbg_print ( void * obj, int level, const char * file, int line, const char * msg )
{
    KLogLevel l = klogDebug;
    switch ( level ) {
        case 0: /* No debug */
            l = klogFatal;
            break;
        case 1: /* Error */
            l = klogErr;
            break;
        case 2: /* State change */
            l = klogWarn;
            break;
        case 3: /* Informational */
            l = klogInfo;
            break;
        case 4: /* Verbose */
        default:
            l = klogDebug;
            break;
    }

    if ( file == NULL )
        file = "mbedtls-file-unknown";
    if ( msg == NULL )
        msg = "<missing message>";

    PLOGMSG ( l, ( l, "[$(level)]:$(file):$(line) - $(msg)",
                         "level=%d,file=%s,line=%d,msg=%s",
                          level, file, line, msg ) );
}

static
rc_t tlsg_seed_rng ( KTLSGlobals *self )
{
    int ret;
    const char *pers = "vdb_tls_client";
    const size_t pers_size = sizeof "vdb_tls_client" - 1;

    STATUS ( STAT_QA, "Seeding the random number generator\n" );

    ret = vdb_mbedtls_ctr_drbg_seed ( &self -> ctr_drbg, vdb_mbedtls_entropy_func,
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
rc_t tlsg_init_ca ( KTLSGlobals *self, const KConfig * kfg )
{
#if IGNORE_ALL_CERTS_ALLOWED
    const KConfigNode * allow_all_certs;
    rc_t rc = KConfigOpenNodeRead ( kfg, & allow_all_certs, "/tls/allow-all-certs" );
    if ( rc == 0 )
    {
        rc = KConfigNodeReadBool ( allow_all_certs, & self -> allow_all_certs );
        KConfigNodeRelease ( allow_all_certs );
        return rc;
    }
#endif
    /* if the node does not exist, it means false */
    self -> allow_all_certs = false;
    return 0;
}

static
rc_t tlsg_init_certs ( KTLSGlobals *self, const KConfig * kfg )
{
    int ret = 0;
    bool cert_file_loaded = false;

    rc_t rc = 0;
    uint32_t nidx, num_certs = 0;

    static char * node_names [] =
    {
        "/tls/ca.crt",
        "/tls/self-signed"
    };

    STATUS ( STAT_QA, "Loading CA root certificates\n" );

    for ( nidx = 0; rc == 0 && nidx < sizeof node_names / sizeof node_names [ 0 ]; ++ nidx )
    {
        const KConfigNode * ca_crt;
        const char * node_name = node_names [ nidx ];

        rc = KConfigOpenNodeRead ( kfg, & ca_crt, "%s", node_name );
        if ( rc != 0 )
            rc = 0;
        else
        {
            KNamelist * cert_names;

            STATUS ( STAT_PRG, "Listing CA root certificates\n" );
            rc = KConfigNodeListChildren ( ca_crt, & cert_names );
            if ( rc != 0 )
            {
                rc = ResetRCContext ( rc, rcKrypto, rcToken, rcInitializing );
                PLOGERR ( klogInt, ( klogInt, rc
                                     , "failed to list config node '$(node)'"
                                     , "node=%s"
                                     , node_name
                              ) );
            }
            else
            {
                uint32_t count;

                STATUS ( STAT_GEEK, "Counting CA root certificates\n" );
                rc = KNamelistCount ( cert_names, & count );
                if ( rc != 0 )
                {
                    rc = ResetRCContext ( rc, rcKrypto, rcToken, rcInitializing );
                    PLOGERR ( klogInt, ( klogInt, rc
                                         , "failed to count names in config node '$(node)'"
                                         , "node=%s"
                                         , node_name
                                  ) );
                }
                else
                {
                    uint32_t i;
                    String * cert_string;
                    const KConfigNode * root_cert;

                    STATUS ( STAT_GEEK, "Found %u names in CA root certificates\n", count );
                    STATUS ( STAT_PRG, "Retrieving names in CA root certificates\n" );
                    for ( i = 0; i < count; ++ i )
                    {
                        const char * cert_name;

                        STATUS ( STAT_GEEK, "Retrieving name %u in CA root certificates\n", i );
                        rc = KNamelistGet ( cert_names, i, & cert_name );
                        if ( rc != 0 )
                        {
                            rc = ResetRCContext ( rc, rcKrypto, rcToken, rcInitializing );
                            PLOGERR ( klogInt, ( klogInt, rc
                                                 , "failed to read cert $(idx) in config node '$(node)'"
                                                 , "node=%s,idx=%u"
                                                 , node_name
                                                 , i
                                          ) );
                            break;
                        }

                        STATUS ( STAT_GEEK, "Retrieving node '%s' from CA root certificates\n", cert_name );
                        rc = KConfigNodeOpenNodeRead ( ca_crt, & root_cert, "%s", cert_name );
                        if ( rc != 0 )
                        {
                            rc = ResetRCContext ( rc, rcKrypto, rcToken, rcInitializing );
                            PLOGERR ( klogInt, ( klogInt, rc
                                                 , "failed to read node for cert '$(name)' in config node '$(node)'"
                                                 , "node=%s,name=%s"
                                                 , node_name
                                                 , cert_name
                                          ) );
                            break;
                        }

                        STATUS ( STAT_GEEK, "Retrieving text for node '%s' from CA root certificates\n", cert_name );
                        rc = KConfigNodeReadString ( root_cert, & cert_string );
                        KConfigNodeRelease ( root_cert );

                        if ( rc != 0 )
                        {
                            rc = ResetRCContext ( rc, rcKrypto, rcToken, rcInitializing );
                            PLOGERR ( klogInt, ( klogInt, rc
                                                 , "failed to read node text for cert '$(name)' in config node '$(node)'"
                                                 , "node=%s,name=%s"
                                                 , node_name
                                                 , cert_name
                                          ) );
                            break;
                        }


                        /* these guys take a length, so presumably the string is not NUL terminated.
                           yet, the first thing they do is see if the NUL is included in the length! */
                        STATUS ( STAT_GEEK, "Parsing text for node '%s' from CA root certificates\n", cert_name );
                        ret = vdb_mbedtls_x509_crt_parse ( &self -> cacert,
                            ( const unsigned char * ) cert_string -> addr, cert_string -> size + 1 );

                        StringWhack ( cert_string );

                        if ( ret < 0 )
                        {
                            rc = RC ( rcKrypto, rcToken, rcInitializing, rcEncryption, rcFailed );
                            PLOGERR ( klogSys, ( klogSys, rc
                                                 , "mbedtls_x509_crt_parse returned $(ret) ( $(expl) )"
                                                 , "ret=%d,expl=%s"
                                                 , ret
                                                 , mbedtls_strerror2 ( ret )
                                          ) );
                            break;
                        }

                        ++ num_certs;
                    }
                }

                KNamelistRelease ( cert_names );
            }

            KConfigNodeRelease ( ca_crt );
        }
    }

    if ( rc == 0 )
    {
        rc = tlsg_init_ca ( self, kfg );
        if ( rc != 0 )
            return rc;
    }

#if _DEBUGGING
    {
        const KConfigNode * ca_crt;
        rc_t rc2 = KConfigOpenNodeRead ( kfg, & ca_crt, "/tls/path/ca.crt" );
        if ( rc2 == 0 )
        {
            String * ca_crt_path;
            STATUS ( STAT_GEEK, "Retrieving path to CA root certificate file\n" );
            rc2 = KConfigNodeReadString ( ca_crt, & ca_crt_path );
            if ( rc2 == 0 )
            {
                STATUS ( STAT_GEEK, "Parsing text from CA root certificate file '%S'\n", ca_crt_path );
                ret = vdb_mbedtls_x509_crt_parse_file ( &self -> cacert, ca_crt_path -> addr );
                if ( ret >= 0 )
                    cert_file_loaded = true;
                else
                {
                    PLOGMSG ( klogWarn, ( klogWarn
                                          , "mbedtls_x509_crt_parse_file ( '$(path)' ) returned $(ret) ( $(expl) )"
                                          , "ret=%d,expl=%s,path=%S"
                                          , ret
                                          , mbedtls_strerror2 ( ret )
                                          , ca_crt_path
                                  ) );
                }
                StringWhack ( ca_crt_path );
            }
            KConfigNodeRelease ( ca_crt );
        }
    }
#endif

    if ( ! cert_file_loaded )
    {
#if WINDOWS
        //-------------------------------------------------------------------
        // Pull all the certificates from the ROOT store.

        HCERTSTORE hSystemStore = CertOpenSystemStoreA(0, "ROOT");
        if ( hSystemStore != NULL )
        {
            PCCERT_CONTEXT pCertContext = NULL;
            while (true)
            {
                pCertContext = CertEnumCertificatesInStore(hSystemStore, pCertContext);
                if (pCertContext == NULL)
                {
                    break;
                }

                // ignore errors
                if ( vdb_mbedtls_x509_crt_parse(&self->cacert, (const unsigned char*)pCertContext->pbCertEncoded, pCertContext->cbCertEncoded ) == 0 )
                {
                    cert_file_loaded = true;
                }
            }
            CertCloseStore(hSystemStore, 0);
        }
#else
        const char * root_ca_paths [] =
        {
            "/etc/ssl/certs/ca-certificates.crt",                /* Debian/Ubuntu/Gentoo etc */
            "/etc/pki/tls/certs/ca-bundle.crt",                  /* Fedora/RHEL */
            "/etc/ssl/ca-bundle.pem",                            /* OpenSUSE */
            "/etc/pki/tls/cacert.pem",                           /* OpenELEC */
            "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", /* CentOS/RHEL 7 */
            "/etc/ssl/certs",                                    /* SLES10/SLES11, https://golang.org/issue/12139 */
            "/system/etc/security/cacerts",                      /* Android */
            "/usr/local/share/certs",                            /* FreeBSD */
            "/etc/pki/tls/certs",                                /* Fedora/RHEL */
            "/etc/openssl/certs",                                /* NetBSD */
            "/etc/ssl/cert.pem"                                  /* MacOSX */
        };

        KDirectory *n_dir;
        rc_t rc2 = KDirectoryNativeDir ( & n_dir );
        if ( rc2 == 0 )
        {
            size_t i;
            for ( ret = -1, i = 0; i < sizeof root_ca_paths / sizeof root_ca_paths [ 0 ]; ++ i )
            {
                const char * path = root_ca_paths [ i ];

                /* no fail, exit immediately if succesful or cycle through all files */
                switch ( KDirectoryPathType ( n_dir, path ) & ~ kptAlias )
                {
                case kptFile:
                    STATUS ( STAT_GEEK, "Parsing text from CA root certificate file '%s'\n", path);
                    ret = vdb_mbedtls_x509_crt_parse_file ( &self -> cacert, path );
                    break;
                case kptDir:
                    STATUS ( STAT_GEEK, "Parsing text from CA root certificate directory '%s'\n", path );
                    ret = vdb_mbedtls_x509_crt_parse_path ( &self -> cacert, path );
                    break;
                }

                if ( ret >= 0 )
                    cert_file_loaded = true;
            }
            KDirectoryRelease ( n_dir );
        }
#endif
    }

    if ( num_certs == 0 )
    {
        STATUS ( STAT_QA, "Parsing text for default CA root certificates\n" );
        ret = vdb_mbedtls_x509_crt_parse ( &self -> cacert,
            ( const unsigned char * ) ca_crt_ncbi1, sizeof ca_crt_ncbi1 );

        if ( ret < 0 )
        {
            rc = RC ( rcKrypto, rcToken, rcInitializing, rcEncryption, rcFailed );
            PLOGERR ( klogSys, ( klogSys, rc
                                 , "mbedtls_x509_crt_parse returned $(ret) ( $(expl) )"
                                 , "ret=%d,expl=%s"
                                 , ret
                                 , mbedtls_strerror2 ( ret )
                          ) );
        }
        else
        {
            num_certs = 1;

            ret = vdb_mbedtls_x509_crt_parse ( &self -> cacert,
                ( const unsigned char * ) ca_crt_ncbi2, sizeof ca_crt_ncbi2 );

            if ( ret >= 0 )
                ++ num_certs;
            else
            {
                rc = RC ( rcKrypto, rcToken, rcInitializing, rcEncryption, rcFailed );
                PLOGERR ( klogSys, ( klogSys, rc
                                     , "mbedtls_x509_crt_parse returned $(ret) ( $(expl) )"
                                     , "ret=%d,expl=%s"
                                     , ret
                                     , mbedtls_strerror2 ( ret )
                              ) );
            }
        }
    }

    return num_certs == 0 ? rc : 0;
}

static
rc_t tlsg_setup ( KTLSGlobals * self )
{
    int ret;

    STATUS ( STAT_QA, "Configuring SSl defaults\n" );

    ret = vdb_mbedtls_ssl_config_defaults ( &self -> config,
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

    /* turn off certificate validation when self -> allow_all_certs == true */
    vdb_mbedtls_ssl_conf_authmode( &self -> config,
        self -> allow_all_certs ? MBEDTLS_SSL_VERIFY_OPTIONAL
                                : MBEDTLS_SSL_VERIFY_REQUIRED );

    vdb_mbedtls_ssl_conf_ca_chain( &self -> config, &self -> cacert, NULL );
    vdb_mbedtls_ssl_conf_rng( &self -> config, vdb_mbedtls_ctr_drbg_random, &self -> ctr_drbg );

        /*  We need that to be sure that we are free to call
         *  vdb_mbedtls_ssl_conf_authmode () next time when
         *  KNSManagerSetAllowAllCerts () will be called
         *
         *  Because smart special design we do not need to add
         *  special code to deinitialize that variable.
         */
    self -> safe_to_modify_ssl_config = true;

    return 0;
}

/* threshold
   theshold level of messages to filter on.
   Messages at a higher level will be discarded.
  Debug levels
	0 No debug
	1 Error
	2 State change
	3 Informational
	4 Verbose */
static int set_threshold ( const KConfig * kfg ) {
    bool set = false;

    int64_t threshold = 0;

    const char * env = NULL;

    rc_t rc = KConfigReadI64 ( kfg, "/tls/NCBI_VDB_TLS", & threshold );
    if ( rc == 0 )
        set = true;

    env = getenv ( "NCBI_VDB_TLS" );

    if ( env != NULL ) {
        int NCBI_VDB_TLS = 0;

        for  ( ; * env != '\0'; ++ env ) {
            char c = * env;
            if ( c < '0' || c > '9' )
                break;

            NCBI_VDB_TLS = NCBI_VDB_TLS * 10 + c - '0';
            set = true;
        }

        if ( NCBI_VDB_TLS > threshold )
            threshold = NCBI_VDB_TLS;
    }

    if ( set )
        vdb_mbedtls_debug_set_threshold ( ( int ) threshold );

    return ( int ) threshold;
}

/* Init
 */
rc_t KTLSGlobalsInit ( KTLSGlobals * tlsg, const KConfig * kfg )
{
    rc_t rc;

    assert ( tlsg != NULL );
    assert ( kfg != NULL );

    vdb_mbedtls_x509_crt_init ( &tlsg -> cacert );
    vdb_mbedtls_ctr_drbg_init ( &tlsg -> ctr_drbg );
    vdb_mbedtls_entropy_init ( &tlsg -> entropy );
    vdb_mbedtls_ssl_config_init ( &tlsg -> config );

    if ( set_threshold ( kfg ) > 0 )
        vdb_mbedtls_ssl_conf_dbg ( &tlsg -> config, ktls_ssl_dbg_print, tlsg );

    rc = tlsg_seed_rng ( tlsg );
    if ( rc == 0 )
    {
        rc = tlsg_init_certs ( tlsg, kfg );
        if ( rc == 0 )
            rc = tlsg_setup ( tlsg );
    }

    return rc;
}

/* Whack
 */
void KTLSGlobalsWhack ( KTLSGlobals * self )
{
    assert ( self != NULL );

    vdb_mbedtls_ssl_config_free ( &self -> config );
    vdb_mbedtls_entropy_free ( &self -> entropy );
    vdb_mbedtls_ctr_drbg_free ( &self -> ctr_drbg );
    vdb_mbedtls_x509_crt_free ( &self -> cacert );

    memset ( self, 0, sizeof * self );
}

/* Set/Get AllowAllCerts
 *  modify behavior of TLS certificate validation
 */
LIB_EXPORT rc_t CC KNSManagerSetAllowAllCerts ( KNSManager *self, bool allow_all_certs )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC ( rcNS, rcMgr, rcAccessing, rcSelf, rcNull );
    else
    {
#if IGNORE_ALL_CERTS_ALLOWED
        self -> tlsg . allow_all_certs = allow_all_certs;
            /*
             *  We are acting from supposition that at some particular
             *  moments there should be called initlialisation of
             *  TLS configurations, which will be reflected at next
             *  handshake
             */
        if ( self -> tlsg . safe_to_modify_ssl_config ) {
            vdb_mbedtls_ssl_conf_authmode(
                            &self -> tlsg . config,
                                ( self -> tlsg . allow_all_certs
                                        ? MBEDTLS_SSL_VERIFY_OPTIONAL
                                        : MBEDTLS_SSL_VERIFY_REQUIRED
                                )
                            );
        }
#endif
    }
    return rc;
}

LIB_EXPORT rc_t CC KNSManagerGetAllowAllCerts ( const KNSManager *self, bool * allow_all_certs )
{
    rc_t rc;

    if ( allow_all_certs == NULL )
        rc = RC ( rcNS, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            * allow_all_certs = self -> tlsg . allow_all_certs;
            return 0;
        }

        * allow_all_certs = false;
    }

    return rc;
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
    vdb_mbedtls_ssl_close_notify( &self -> ssl ); /* close connection - this might need to be elsewhere */
    vdb_mbedtls_ssl_free ( &self -> ssl );

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
    KStreamWhack ( & self -> dad, "KTLSStream" );
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
        static bool inited = false;
        static int m = 0;
        static int e = 0;

        /* read through TLS library */
        ret = vdb_mbedtls_ssl_read( &self -> ssl, buffer, bsize );

        if (!inited) { /* simulate mbedtls read timeout */
            const char * v = getenv("NCBI_VDB_ERR_MBEDTLS_READ");
            if (v != NULL) {
                m = atoi(v);
                if (m < 0)
                    m = 0;
            }
            e = m;
            inited = true;
        }
        if (m > 0) {
            if (!e) {
                e = m;
                if (ret >= 0) {
                    ret = -76;
                    self->rd_rc
                        = RC(rcNS, rcStream, rcReading, rcTimeout, rcExhausted);
                }
            }
            --e;
        }

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
            if (self->mgr->logTlsErrors)
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
             * vdb_mbedtls_ssl_session_reset () before a new connection; current connection
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
             * continue with \c vdb_mbedtls_ssl_handshake() with the same
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
        ret = vdb_mbedtls_ssl_write ( &self -> ssl, buffer, size );

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

/* called by vdb_mbedtls_ssl_fetch_input */
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
        switch ( ( int ) GetRCObject ( self -> rd_rc ) )
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

    ret = vdb_mbedtls_ssl_setup( &self -> ssl, &tlsg -> config );
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

    if ( ( ( size_t ) ( & host -> addr [ host -> size ] ) & ( 4096 - 1 ) ) != 0 &&
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

    ret = vdb_mbedtls_ssl_set_hostname( &self -> ssl, hostz -> addr );

    if ( hostz != host )
        StringWhack ( hostz );

    if ( ret != 0 )
    {
        rc_t rc = RC ( rcKrypto, rcSocket, rcFormatting, rcEncryption, rcFailed );
        PLOGERR ( klogSys, ( klogSys, rc
                             , "mbedtls_ssl_set_hostname returned $(ret) ( $(expl) )"
                             , "ret=%d,expl=%s"
                             , ret
                             , mbedtls_strerror2 ( ret )
                      ) );
        return rc;
    }


    vdb_mbedtls_ssl_set_bio( &self -> ssl, ( void * ) self, ktls_net_send, ktls_net_recv, NULL );

    return 0;
}

static
rc_t ktls_handshake ( KTLSStream *self )
{
    static bool inited = false;
    static int m = 0;
    static int e = 0;

    int ret;

    STATUS ( STAT_QA, "Performing SSL/TLS handshake...\n" );

    assert(self && self->mgr);

    ret = vdb_mbedtls_ssl_handshake( &self -> ssl );

    if (!inited) { /* simulate mbedtls handshake timeout */
        const char * v = getenv("NCBI_VDB_ERR_MBEDTLS_HANDSHAKE");
        if (v != NULL) {
            m = atoi(v);
            if (m < 0)
                m = 0;
        }
        e = m;
        inited = true;
    }
    if (m > 0) {
        if (!e) {
            e = m;
            if (ret >= 0) {
                ret = -76;
                self->rd_rc
                    = RC(rcKrypto, rcFile, rcOpening, rcConnection, rcFailed);
            }
        }
        --e;
    }

    while ( ret != 0 )
    {
        if ( ret != MBEDTLS_ERR_SSL_WANT_READ &&
             ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            rc_t rc;

            /* check configuration to see if we are ignoring
               unrecognized certificates (ones that use a CA
               for signing that we don't recognize) */
            if ( self -> mgr -> tlsg . allow_all_certs &&
                 ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED )
            {
                /* ignore this case */
                rc = 0;
            }
            else
            {
                /* either we're forcing all certificates to be validated,
                   or the error is something other than a validation error */
                rc = RC ( rcKrypto, rcSocket, rcOpening, rcConnection, rcFailed );

                if (self->mgr->logTlsErrors)
                  PLOGERR ( klogSys, ( klogSys, rc
                                     , "mbedtls_ssl_handshake returned $(ret) ( $(expl) )"
                                     , "ret=%d,expl=%s"
                                     , ret
                                     , mbedtls_strerror2 ( ret )
                              ) );

                if ( ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED )
                {
                    uint32_t flags = vdb_mbedtls_ssl_get_verify_result( &self -> ssl );
                    if ( flags != 0 )
                    {
                        char buf [ 4096 ];
                        vdb_mbedtls_x509_crt_verify_info ( buf, sizeof( buf ), " !! ", flags );

                        PLOGMSG ( klogSys, ( klogSys
                                             , "mbedtls_ssl_get_verify_result returned $(flags) ( $(info) )"
                                             , "flags=0x%X,info=%s"
                                             , flags
                                             , buf
                                      ) );
                    }
                }
            }

            return rc;
        }
        ret = vdb_mbedtls_ssl_handshake( &self -> ssl );
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
                    vdb_mbedtls_ssl_init ( &obj -> ssl );

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
                    else {
                        if ( KNSManagerLogNcbiVdbNetError ( self ) ) {
                            KEndPoint ep, local_ep;
                            rc_t rr = KSocketGetRemoteEndpoint ( ciphertext,
                                                                 & ep );
                            rc_t rl = KSocketGetLocalEndpoint ( ciphertext,
                                                                 & local_ep );
                            if ( rr != 0 )
                                LOGERR ( klogInt, rr
                                    , "cannot KSocketGetRemoteEndpoint"
                                );
                            if ( rl != 0 )
                                LOGERR ( klogInt, rl
                                    , "cannot KSocketGetLocalEndpoint"
                                );
                            if ( rr == 0 || rl == 0 ) {
                                if ( rr == 0 )
                                    if ( rl == 0 )
                                        PLOGERR ( klogSys, ( klogSys, rc,
                                            "ktls_handshake failed while accessing '$(ip)' from '$(local)'"
                                            , "ip=%s,local=%s", ep . ip_address, local_ep . ip_address
                                        ) );
                                    else
                                        PLOGERR ( klogSys, ( klogSys, rc,
                                            "ktls_handshake failed while accessing '$(ip)'"
                                            , "ip=%s", ep . ip_address
                                        ) );
                                else
                                    PLOGERR ( klogSys, ( klogSys, rc,
                                        "ktls_handshake failed while accessing unknown IP from '$(local)'"
                                        , "local=%s", local_ep . ip_address
                                    ) );
                            }
                        }
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
    rc_t rc = 0;

   STATUS ( STAT_QA, "Verifying peer X.509 certificate..." );

   if ( self == NULL )
       rc = RC ( rcKrypto, rcToken, rcValidating, rcSelf, rcNull );
   else if ( ! self -> mgr -> tlsg . allow_all_certs )
   {
       uint32_t flags = vdb_mbedtls_ssl_get_verify_result( &self -> ssl );
       if ( flags != 0 )
       {
           char buf [ 4096 ];
           rc_t rc = RC ( rcKrypto, rcToken, rcValidating, rcEncryption, rcFailed );

           vdb_mbedtls_x509_crt_verify_info ( buf, sizeof( buf ), "  ! ", flags );

           PLOGERR ( klogSys, ( klogSys, rc
                                , "mbedtls_ssl_get_verify_result returned $(flags) ( $(info) )"
                                , "flags=0x%X,info=%s"
                                , flags
                                , buf
                         ) );
           return rc;
       }
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

