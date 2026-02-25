/*=============================================================================$
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
* ============================================================================*/

#include <kapp/args.h> /* ArgsMakeAndHandle */
#include <kfg/config.h> /* KConfigDisableUserSettings */
#include <kfs/file.h> /* KFileRelease */
#include <klib/debug.h> /* KDbgSetString */
#include <kns/endpoint.h> /* KEndPoint */
#include <kns/http.h> /* KNSManagerMakeHttpFile */
#include <kns/manager.h> /* KNSManagerRelease */
#include <ktst/unit_test.hpp> // TEST_SUITE
#include <vfs/path.h> /* VPathRelease */
#include <vfs/services.h> /* KServiceRelease */
#include <vfs/services-priv.h> /* KServiceSetQuality */

#include <climits> /* PATH_MAX */
#ifndef PATH_MAX
    #define PATH_MAX 4096
#endif

static rc_t argsHandler(int argc, char * argv[]) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

TEST_SUITE_WITH_ARGS_HANDLER( AwsProxyTestSuite, argsHandler )

static bool Am_I_on_GCP(const KNSManager* mgr) {
    String hostname;
    CONST_STRING(&hostname, "metadata.google.internal");

    KEndPoint ep;
    rc_t rc(KNSManagerInitDNSEndpoint(mgr, &ep, &hostname, 80));
    return rc == 0;
}

TEST_CASE ( AwsProxyTest ) {
    KConfig* kfg(NULL);
    REQUIRE_RC(KConfigMake(&kfg, NULL));

    KNSManager * mgr = NULL;
    REQUIRE_RC ( KNSManagerMake ( & mgr ) );

    if (Am_I_on_GCP(mgr))
        // SDL refuses to return a URL if the request was sent from GCP:
        // stop sending location.
        REQUIRE_RC(KConfigWriteString(kfg, "/libs/cloud/provider", "n"));
    
    KService * service = NULL;
    REQUIRE_RC ( KServiceMakeWithMgr ( & service, NULL, NULL, kfg ) );
    
    REQUIRE_RC(KConfigRelease(kfg));
    
    REQUIRE_RC ( KServiceAddId ( service, "SRR1219902" ) );
    REQUIRE_RC ( KServiceSetNgcFile ( service, "data/prj_phs710EA_test.ngc" ) );
    REQUIRE_RC ( KServiceSetQuality ( service, "R" ) );

    const KSrvResponse * r = NULL;
    REQUIRE_RC ( KServiceNamesQuery ( service, 0, & r ) );

    KSrvRunIterator * it = NULL;
    REQUIRE_RC ( KSrvResponseMakeRunIterator ( r, & it ) );

    const KSrvRun * run = NULL;
    REQUIRE_RC ( KSrvRunIteratorNextRun ( it, & run ) );

    const VPath * remote = NULL;
    REQUIRE_RC ( KSrvRunQuery ( run, 0, & remote, 0, 0 ) );

    char url [ PATH_MAX ] = "";
    REQUIRE_RC ( VPathReadUri ( remote, url, sizeof url, 0 ) );

    const KFile * file = NULL;
    REQUIRE_RC ( KNSManagerMakeHttpFile ( mgr, & file, 0, 0x01010000, url ) );

    char buffer [ 9 ] = "";
    size_t num_read = 0;
    REQUIRE_RC ( KFileRead ( file, 0, buffer, sizeof buffer, & num_read ) );

    REQUIRE_RC ( KFileRelease ( file ) );

    REQUIRE_RC ( VPathRelease ( remote ) );

    REQUIRE_RC ( KSrvRunRelease ( run ) );

    REQUIRE_RC ( KSrvRunIteratorRelease ( it ) );

    REQUIRE_RC ( KSrvResponseRelease ( r ) );

    REQUIRE_RC ( KServiceRelease ( service ) );

    REQUIRE_RC ( KNSManagerRelease ( mgr ) );
}

int main( int argc, char * argv [] )
{
    KDbgInit();
    KWrtInit("test", 0);
    KDbgHandlerSetStdErr();
    if (0) {
        assert(!KDbgSetString("KNS"));
        assert(!KDbgSetString("VFS"));
    }

    KConfigDisableUserSettings ();
    
    return AwsProxyTestSuite(argc, argv);
}
