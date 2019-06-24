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


#include <kapp/args.h>        /* Args */

#include <kfg/kfg-priv.h>     /* KConfigMakeEmpty */

#include <klib/out.h>         /* KOutMsg */

#include <kns/manager.h>      /* KNSManagerSetConnectionTimeouts */

#include <vfs/manager.h>      /* VFSManagerRelease */
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/path.h>         /* VFSManagerMakeAccPath */
#include <vfs/resolver.h>     /* VResolverQuery */

#include <ktst/unit_test.hpp> // TEST_SUITE

#define RELEASE( type, obj ) do { rc_t rc2 = type##Release ( obj ); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while ( false )

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(TestProxySchemeSuite, argsHandler);

static bool EXPECTED_FAILURE = true;
static char * PROXY = NULL;

TEST_CASE ( test ) {
    rc_t rc = 0;

    KConfig * kfg = NULL;
    REQUIRE_NULL ( kfg );
    REQUIRE_RC ( KConfigMakeEmpty ( & kfg ) );
    REQUIRE_RC ( KConfigWriteString ( kfg,
                    "/repository/remote/main/CGI/resolver-cgi",
                    "https://trace.ncbi.nlm.nih.gov/Traces/names/names.fcgi" ) );

    if ( PROXY != NULL )
        REQUIRE_RC ( KConfigWriteString ( kfg, "/http/proxy/path", PROXY ) );

    /* do not try direct http access, use proxy only: VDB-3015 */
    REQUIRE_RC ( KConfigWriteString ( kfg, "/http/proxy/only", "true" ) );

    REQUIRE_NOT_NULL ( kfg );

    VFSManager * vfs = NULL;
    REQUIRE_NULL ( vfs );
    REQUIRE_RC ( VFSManagerMakeFromKfg ( & vfs, kfg ) );
    REQUIRE_NOT_NULL ( vfs );

    KNSManager * kns = NULL;
    REQUIRE_RC ( VFSManagerGetKNSMgr ( vfs, & kns ) );
    // do not do long retries when calling proxy on bad port
    REQUIRE_RC ( KNSManagerSetConnectionTimeouts ( kns, 1, 0, 0 ) );
    RELEASE ( KNSManager, kns );

    VResolver * resolver = NULL;
    REQUIRE_RC ( VFSManagerMakeResolver ( vfs, & resolver, kfg ) );
    REQUIRE_NOT_NULL ( resolver );

    VPath * query = NULL;
    REQUIRE_NULL ( query );
    REQUIRE_RC ( VFSManagerMakeAccPath ( vfs, & query, "SRR000001" ) );
    REQUIRE_NOT_NULL ( query );

    const VPath * rmt = NULL;
    REQUIRE_NULL ( rmt );
    if ( EXPECTED_FAILURE )
        REQUIRE_RC_FAIL ( VResolverQuery ( resolver, 0, query, 0, & rmt, 0 ) );
    else
        REQUIRE_RC      ( VResolverQuery ( resolver, 0, query, 0, & rmt, 0 ) );
    RELEASE ( VPath     , rmt );

    RELEASE ( VPath     , query );

    RELEASE ( VResolver , resolver );

    RELEASE ( VFSManager, vfs );

    RELEASE ( KConfig   , kfg );

    REQUIRE_RC ( rc );
}

rc_t CC Usage ( const Args * args ) { return 0; }
const char UsageDefaultName [] = "test-proxy-with-scheme";

rc_t CC UsageSummary ( const char * prog_name ) {
    return KOutMsg (
"Usage:"
""
"      test-proxy-with-scheme"
"                             - expect failure"
""
"      test-proxy-with-scheme =<proxy-spec>"
"                             - set <proxy spec> in configuration;"
"                             - expect failure"
"           Example:"
"               test-proxy-with-scheme =bad.proxy"
                   );
}

static rc_t argsHandler ( int argc, char * argv [] ) {
    rc_t rc = ArgsMakeAndHandle ( NULL, argc, argv, 0, NULL, 0 );
    if ( rc != 0 )
        return rc;

    if ( argc > 1 )
        if ( argv [ 1 ] [ 0 ] == '=' )
            PROXY = strdup ( & argv [ 1 ] [ 1 ] );

    switch ( argc ) {
        case 1:
            EXPECTED_FAILURE = true;
            break;
        case 2:
            EXPECTED_FAILURE = PROXY != NULL;
            break;
        default:
            EXPECTED_FAILURE = false;
            break;
    }

    return rc;
}

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t CC KMain ( int argc, char * argv [] ) {
        rc_t rc = TestProxySchemeSuite ( argc, argv );

        free ( PROXY );
        PROXY = NULL;

        return rc;
    }
}
