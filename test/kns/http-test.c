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

#include <kapp/args.h>
#include <kapp/main.h>

#include <kns/manager.h>
#include <kns/endpoint.h>
#include <kns/socket.h>
#include <kns/stream.h>
#include <kns/http.h>
#include <kns/impl.h>
#include <kns/adapt.h>
#include <kfg/config.h>
#include <kfs/file.h>
#include <kfs/directory.h>
#include <klib/text.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <klib/refcount.h>
#include <klib/container.h>

#include "stream-priv.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sysalloc.h>

#include "http-priv.h"


void URLBlockInitTest ( void )
{
    URLBlock b;
    URLBlockInit ( & b );

    OUTMSG ( ( "%s - URLBlockInit succeeded.\n"
               "scheme  : '%S'\n"
               "host    : '%S'\n"
               "path    : '%S'\n"
               "query   : '%S'\n"
               "fragment: '%S'\n"
               "port    : '%u'\n\n"
               , __func__,  & b . scheme, & b . host
               , & b . path, & b . query, & b . fragment, b . port ) );

}


rc_t ParseUrlTest ( void )
{
    rc_t rc;
    size_t i;

    static const char *test_urls [] =
        {
            /* <scheme>://<host>[:<port>]/<path>[?<query>][#<fragment>]*/
            /*0*/ "http://www.abc.com/library/index.html?x&y=123&z=test#ignore-me",
            /*1*/ "http://www.abc.com:123/library/index.html?x&y=123&z=test#ignore-me",
            /*2*/ "HTTP://www.abc.com:123/library/index.html?x&y=123&z=test#ignore-me",
            /* <scheme>://<host>[:<port>]/<path>[#<fragment>]*/
            /*3*/ "http://www.abc.com/library/index.html#ignore-me",
            /*4*/ "http://www.abc.com:123/library/index.html#ignore-me",
            /* <scheme>://<host>[:<port>]/<path>[?<query>]*/
            /*5*/ "http://www.abc.com/library/index.html?x&y=123&z=test",
            /*6*/ "http://www.abc.com:123/library/index.html?x&y=123&z=test",
            /* <scheme>://<host>[:<port>]/<path>*/
            /*7*/ "http://www.abc.com/library/index.html",
            /*8*/ "http://www.abc.com:80/library/index.html",
            /* <scheme>://<host>[:<port>]*/
            /*9*/ "http://www.abc.com",
            /*10*/ "http://www.abc.com:80",
            /* /<path>[#fragment]*/
            /*11*/ "/library/index.html#ignore-me",
            /* /<path>[?query]*/
            /*12*/ "/library/index.html?x&y=123&z=test",
            /*13*/ "///library"


        };
    const size_t num_urls = sizeof test_urls / sizeof test_urls [ 0 ];

    static const char *fail_url [] =
        {
            /*<scheme>:/<path>*/
            /*0*/ "http:/library/index.html",
            /*1*/ "http//library/index.html",
            /*<scheme>:/[#fragment]*/
            /*2*/ "http:#ignore-me",
            /*<scheme>:/[#query]*/
            /*3*/ "http:?x&y=123&z=test",
            /*<scheme>*/
            /*4*/ "http",
            /*5*/ "ftp",
            /* <host>[:<port>]*/
            /*6*/ "www.abc.com",
            /*7*/ "www.abc.com:80",
            /* <scheme>://<host>[:<port>]*/
            /*8*/ "ftp://www.abc.com"


        };
    const size_t num_fail_urls = sizeof fail_url / sizeof fail_url [ 0 ];

    for ( i = 0; i < num_urls; ++ i )
    {
        URLBlock b;

        String url;
        StringInitCString ( & url, test_urls [ i ] );

        rc  = ParseUrl ( & b, url . addr, url . size );
        if ( rc == 0 )
        {
            /* same as above */
            OUTMSG ( ( "%s - ParseUrl succeeded -- %d: %s\n"
                       "scheme  : '%S'\n"
                       "host    : '%S'\n"
                       "path    : '%S'\n"
                       "query   : '%S'\n"
                       "fragment: '%S'\n"
                       "port    : '%u'\n\n"
                       , __func__, i, test_urls [ i ], & b . scheme, & b . host
                       , & b . path, & b . query, & b . fragment, b . port ) );
        }
        else
        {
            /* same as above. analyze your code more */
            OUTMSG (( "%s: ParseUrl failed on iteration: '%d' url: %s\n"
                      "with rc=%R\n\n", __func__, i, test_urls [ i ], rc ));
            return rc;
        }
    }


    for ( i = 0; i < num_fail_urls; ++ i )
    {
        URLBlock b;

        String url;
        StringInitCString ( & url, fail_url [ i ] );

        rc  = ParseUrl ( & b, url . addr, url . size );
        if ( rc != 0 )
        {
            /* same as above */
            OUTMSG ( ( "%s - ParseUrl succeeded in catching error-- %d: %s\n"
                       "scheme  : '%S'\n"
                       "host    : '%S'\n"
                       "path    : '%S'\n"
                       "query   : '%S'\n"
                       "fragment: '%S'\n"
                       "port    : '%u'\n\n"
                       , __func__, i, fail_url [ i ], & b . scheme, & b . host
                       , & b . path, & b . query, & b . fragment, b . port ) );
        }
        else
        {
            /* same as above. analyze your code more */
            OUTMSG (( "%s: ParseUrl failed catching error on iteration: '%d' url: %s\n"
                      "with rc=%R\n\n", __func__, i, fail_url [ i ], rc ));
            return rc;
        }
    }

    return rc = 0;
}

rc_t HttpTest ( const KFile *input )
{

    KNSManager *mgr;

    rc_t rc = KNSManagerMake ( & mgr );
    if ( rc == 0 )
    {
        KStream *sock;
        rc = KStreamFromKFilePair ( & sock, input, NULL );
        if ( rc == 0 )
        {
            struct KClientHttp *http;

            String host;
            CONST_STRING ( & host, "www.ncbi.nlm.nih.gov" );

            rc = KNSManagerMakeHttp ( mgr, & http, sock, 0x01010000, & host, 80 );
            if ( rc == 0 )
            {
                String msg;
                ver_t version;
                uint32_t status;

                rc = KHttpGetStatusLine ( http, NULL, &msg, &status, &version );
                if ( rc != 0 )
                    OUTMSG (( "%s: KHttpGetStatusLine failed with rc=%R\n", __func__, rc ));
                else
                {
                    bool blank, close_connection;
                    BSTree hdrs;
                    OUTMSG (( "%s: KHttpGetStatusLine returned msg='%S', status=%u, version=%V\n",
                              __func__, & msg, status, version ));

                    BSTreeInit ( & hdrs );

                    for ( blank = close_connection = false; ! blank && rc == 0; )
                        rc = KHttpGetHeaderLine ( http, NULL, & hdrs, & blank, & close_connection );

                    if ( rc != 0 )
                        OUTMSG (( "%s: KHttpGetHeaderLine failed with rc=%R\n", __func__, rc ));
                    else
                    {
                        const KHttpHeader * hdr;

                        OUTMSG (( "%s: KHttpGetStatusLine listing:\n", __func__ ));
                        for ( hdr = ( const KHttpHeader * ) BSTreeFirst ( & hdrs );
                              hdr != NULL;
                              hdr = ( const KHttpHeader * ) BSTNodeNext ( & hdr -> dad ) )
                        {
                            OUTMSG (( "    name='%S', value='%S'\n", & hdr -> name, & hdr -> value ));
                        }
                    }

                    BSTreeWhack ( & hdrs, KHttpHeaderWhack, NULL );

                }

                KHttpRelease ( http );
            }

            KStreamRelease ( sock );
        }

        KNSManagerRelease ( mgr );
    }

    return rc;
}

rc_t PreHttpTest ( void )
{
    KDirectory *dir;
    rc_t rc = KDirectoryNativeDir ( &dir );
    if ( rc == 0 )
    {
        const KFile *f;
        rc = KDirectoryOpenFileRead ( dir, &f, "data/nih_1_out.txt" );
        if ( rc == 0 )
        {
            rc = HttpTest ( f );
            KFileRelease ( f );
        }

        KDirectoryRelease ( dir );
    }
    return rc;
}

rc_t HttpsTest ( const KFile *input )
{

    KNSManager *mgr;

    rc_t rc = KNSManagerMake ( & mgr );
    if ( rc == 0 )
    {
        KStream *sock;
        rc = KStreamFromKFilePair ( & sock, input, NULL );
        if ( rc == 0 )
        {
            struct KClientHttp *https;

            String host;
            CONST_STRING ( & host, "www.ncbi.nlm.nih.gov" );

            rc = KNSManagerMakeClientHttps ( mgr, & https, sock, 0x01010000, & host, 443 );
            if ( rc == 0 )
            {
                String msg;
                ver_t version;
                uint32_t status;

                rc = KHttpGetStatusLine ( https, NULL, &msg, &status, &version );
                if ( rc != 0 )
                    OUTMSG (( "%s: KHttpGetStatusLine failed with rc=%R\n", __func__, rc ));
                else
                {
                    bool blank, close_connection;
                    BSTree hdrs;
                    OUTMSG (( "%s: KHttpGetStatusLine returned msg='%S', status=%u, version=%V\n",
                              __func__, & msg, status, version ));

                    BSTreeInit ( & hdrs );

                    for ( blank = close_connection = false; ! blank && rc == 0; )
                        rc = KHttpGetHeaderLine ( http, NULL, & hdrs, & blank, & close_connection );

                    if ( rc != 0 )
                        OUTMSG (( "%s: KHttpGetHeaderLine failed with rc=%R\n", __func__, rc ));
                    else
                    {
                        const KHttpHeader * hdr;

                        OUTMSG (( "%s: KHttpGetStatusLine listing:\n", __func__ ));
                        for ( hdr = ( const KHttpHeader * ) BSTreeFirst ( & hdrs );
                              hdr != NULL;
                              hdr = ( const KHttpHeader * ) BSTNodeNext ( & hdr -> dad ) )
                        {
                            OUTMSG (( "    name='%S', value='%S'\n", & hdr -> name, & hdr -> value ));
                        }
                    }

                    BSTreeWhack ( & hdrs, KHttpHeaderWhack, NULL );

                }

                KHttpRelease ( http );
            }

            KStreamRelease ( sock );
        }

        KNSManagerRelease ( mgr );
    }

    return rc;
}

rc_t PreHttpsTest ( void )
{
    KDirectory *dir;
    rc_t rc = KDirectoryNativeDir ( &dir );
    if ( rc == 0 )
    {
        const KFile *f;
        rc = KDirectoryOpenFileRead ( dir, &f, "data/nih_1_out_https.txt" );
        if ( rc == 0 )
        {
            rc = HttpsTest ( f );
            KFileRelease ( f );
        }

        KDirectoryRelease ( dir );
    }
    return rc;
}

/* Version  EXTERN
 *  return 4-part version code: 0xMMmmrrrr, where
 *      MM = major release
 *      mm = minor release
 *    rrrr = bug-fix release
 */
ver_t CC KAppVersion ( void )
{
    return 0;
}


/* Usage
 *  This function is called when the command line argument
 *  handling sees -? -h or --help
 */
rc_t CC UsageSummary ( const char *progname )
{
    /*    return KOutMsg (
        "\n"
        "Usage:\n"
        "  %s [Options]\n"
        "\n"
        "Summary:\n"
        "  Simple test of printf.\n"
        , progname );
    */
    return 0;
}

const char UsageDefaultName[] = "time-test";

rc_t CC Usage ( const Args *args )
{
    /*
    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;
    rc_t rc;

    if (args == NULL)
        rc = RC (rcApp, rcArgv, rcAccessing, rcSelf, rcNull);
    else
        rc = ArgsProgram (args, &fullpath, &progname);

    UsageSummary (progname);

    KOutMsg ("Options:\n");

    HelpOptionsStandard();

    HelpVersion (fullpath, KAppVersion());

    return rc;
    */
    return 0;
}


static
rc_t run ( const char *progname )
{
    rc_t rc = 0;

    URLBlockInitTest ();
    rc = ParseUrlTest ();
    rc = PreHttpTest ();
    rc = PreHttpsTest ();
    return 1;
}

/* KMain
 */
rc_t CC KMain ( int argc, char *argv [] )
{
    Args *args;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0 );
    if ( rc == 0 )
    {
        KConfigDisableUserSettings();
        rc = run ( argv [ 0 ] );
        ArgsWhack ( args );
    }

    return rc;
}

