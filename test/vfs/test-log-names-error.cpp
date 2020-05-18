/*==============================================================================
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
* =========================================================================== */

#include <kfg/kfg-priv.h> /* KConfigMakeLocal */

#include <ktst/unit_test.hpp> /* KMain */

#include <klib/debug.h> /* KDbgSetString */

#include <vdb/database.h> /* VDBManagerOpenDBRead */
#include <vdb/manager.h> /* VDBManagerRelease */
#include <vdb/table.h> /* VDBManagerOpenTableRead */
#include <vdb/vdb-priv.h> /* VDBManagerMakeReadWithVFSManager */

#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/manager-priv.h> /* VFSManagerMakeLocal */

TEST_SUITE ( TestLogNames );

#define ALL

#ifdef ALL
TEST_CASE(TestAPI) {
    putenv((char*)"NCBI_VDB_NO_ETC_NCBI_KFG=1");

    KConfig * kfg = NULL;
    REQUIRE_RC(KConfigMakeLocal(&kfg, NULL));
    REQUIRE_RC(KConfigWriteString(kfg,
        "/repository/remote/main/SDL.2/resolver-cgi",
        "https://locate.ncbi.nlm.nih.gov/sdl/2/retrieve"));

    VFSManager * vmgr = NULL;
    REQUIRE_RC(VFSManagerMakeLocal(&vmgr, kfg));
    const VDBManager * mgr = NULL;
    REQUIRE_RC(VDBManagerMakeReadWithVFSManager(&mgr, NULL, vmgr));

    const VDatabase * db = NULL;
    const VTable * tbl = NULL;

    std::cerr << "Expect 'name not found' error messages: vvvvvvvvvvvvvvvvvv\n";
    REQUIRE_RC_FAIL(VDBManagerOpenDBRead(mgr, &db, NULL, "NA000001672.1"));
    REQUIRE_RC_FAIL(VDBManagerOpenTableRead(mgr, &tbl, NULL, "SRR0000001"));
    std::cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^ No more error messages expected.\n";

    bool enabled = false;
    REQUIRE_RC(VFSManagerGetLogNamesServiceErrors(vmgr, &enabled));
    REQUIRE(enabled);

    REQUIRE_RC(VFSManagerLogNamesServiceErrors(vmgr, false));

    REQUIRE_RC_FAIL(VDBManagerOpenDBRead(mgr, &db, NULL, "SRR0000001"));
    REQUIRE_RC_FAIL(VDBManagerOpenTableRead(mgr, &tbl, NULL, "SRR0000001"));

    REQUIRE_RC(VFSManagerGetLogNamesServiceErrors(vmgr, &enabled));
    REQUIRE(!enabled);

    REQUIRE_RC(VDBManagerRelease(mgr));
    REQUIRE_RC(VFSManagerRelease(vmgr));

    REQUIRE_RC(KConfigRelease(kfg));
}
#endif

#ifdef ALL
TEST_CASE(TestDisableInConfig) {
    putenv((char*)"NCBI_VDB_NO_ETC_NCBI_KFG=1");

    KConfig * kfg = NULL;
    REQUIRE_RC(KConfigMakeLocal(&kfg, NULL));
    REQUIRE_RC(KConfigWriteString(kfg,
        "/name-resolver/log-names-service-errors", "false"));
    REQUIRE_RC(KConfigWriteString(kfg,
        "/repository/remote/main/SDL.2/resolver-cgi",
        "https://locate.ncbi.nlm.nih.gov/sdl/2/retrieve"));

    VFSManager * vmgr = NULL;
    REQUIRE_RC(VFSManagerMakeLocal(&vmgr, kfg));
    const VDBManager * mgr = NULL;
    REQUIRE_RC(VDBManagerMakeReadWithVFSManager(&mgr, NULL, vmgr));

    const VDatabase * db = NULL;
    const VTable * tbl = NULL;

    REQUIRE_RC_FAIL(VDBManagerOpenDBRead(mgr, &db, NULL, "NA000001672.1"));
    REQUIRE_RC_FAIL(VDBManagerOpenTableRead(mgr, &tbl, NULL, "SRR0000001"));

    bool enabled = true;
    REQUIRE_RC(VFSManagerGetLogNamesServiceErrors(vmgr, &enabled));
    REQUIRE(!enabled);

    REQUIRE_RC(VFSManagerLogNamesServiceErrors(vmgr, true));
    std::cerr << "Expect 'name not found' error messages: vvvvvvvvvvvvvvvvvv\n";
    REQUIRE_RC_FAIL(VDBManagerOpenDBRead(mgr, &db, NULL, "NA000001672.1"));
    REQUIRE_RC_FAIL(VDBManagerOpenTableRead(mgr, &tbl, NULL, "SRR0000001"));
    std::cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^ No more error messages expected.\n";

    REQUIRE_RC(VFSManagerGetLogNamesServiceErrors(vmgr, &enabled));
    REQUIRE(enabled);

    REQUIRE_RC(VFSManagerLogNamesServiceErrors(vmgr, false));

    REQUIRE_RC_FAIL(VDBManagerOpenDBRead(mgr, &db, NULL, "SRR0000001"));
    REQUIRE_RC_FAIL(VDBManagerOpenTableRead(mgr, &tbl, NULL, "SRR0000001"));

    REQUIRE_RC(VFSManagerGetLogNamesServiceErrors(vmgr, &enabled));
    REQUIRE(!enabled);

    REQUIRE_RC(VDBManagerRelease(mgr));
    REQUIRE_RC(VFSManagerRelease(vmgr));

    REQUIRE_RC(KConfigRelease(kfg));
}
#endif

#ifdef ALL
TEST_CASE(TestEnableInConfig) {
    putenv((char*)"NCBI_VDB_NO_ETC_NCBI_KFG=1");

    KConfig * kfg = NULL;
    REQUIRE_RC(KConfigMakeLocal(&kfg, NULL));
    REQUIRE_RC(KConfigWriteString(kfg,
        "/name-resolver/log-names-service-errors", "true"));
    REQUIRE_RC(KConfigWriteString(kfg,
        "/repository/remote/main/SDL.2/resolver-cgi",
        "https://locate.ncbi.nlm.nih.gov/sdl/2/retrieve"));

    VFSManager * vmgr = NULL;
    REQUIRE_RC(VFSManagerMakeLocal(&vmgr, kfg));
    const VDBManager * mgr = NULL;
    REQUIRE_RC(VDBManagerMakeReadWithVFSManager(&mgr, NULL, vmgr));

    const VDatabase * db = NULL;
    const VTable * tbl = NULL;

    std::cerr << "Expect 'name not found' error messages: vvvvvvvvvvvvvvvvvv\n";
    REQUIRE_RC_FAIL(VDBManagerOpenDBRead(mgr, &db, NULL, "NA000001672.1"));
    REQUIRE_RC_FAIL(VDBManagerOpenTableRead(mgr, &tbl, NULL, "SRR0000001"));
    std::cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^ No more error messages expected.\n";

    bool enabled = false;
    REQUIRE_RC(VFSManagerGetLogNamesServiceErrors(vmgr, &enabled));
    REQUIRE(enabled);

    REQUIRE_RC(VFSManagerLogNamesServiceErrors(vmgr, false));
    REQUIRE_RC_FAIL(VDBManagerOpenDBRead(mgr, &db, NULL, "NA000001672.1"));
    REQUIRE_RC_FAIL(VDBManagerOpenTableRead(mgr, &tbl, NULL, "SRR0000001"));

    REQUIRE_RC(VFSManagerGetLogNamesServiceErrors(vmgr, &enabled));
    REQUIRE(!enabled);

    REQUIRE_RC(VFSManagerLogNamesServiceErrors(vmgr, false));

    REQUIRE_RC_FAIL(VDBManagerOpenDBRead(mgr, &db, NULL, "SRR0000001"));
    REQUIRE_RC_FAIL(VDBManagerOpenTableRead(mgr, &tbl, NULL, "SRR0000001"));

    REQUIRE_RC(VFSManagerGetLogNamesServiceErrors(vmgr, &enabled));
    REQUIRE(!enabled);

    REQUIRE_RC(VDBManagerRelease(mgr));
    REQUIRE_RC(VFSManagerRelease(vmgr));

    REQUIRE_RC(KConfigRelease(kfg));
}
#endif

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t CC KMain ( int argc, char * argv [] ) {
        KConfigDisableUserSettings(); // ignore ~/.ncbi/user-settings.mkfg

        if (
0) assert(!KDbgSetString("VFS"));

        return TestLogNames( argc, argv );
    }
}
