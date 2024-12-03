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
 * =============================================================================
 * Test of VDBManagerGetObjVersion and VDBManagerGetLoaderVersFromMeta
 * ========================================================================== */

#include <kapp/args.h> // ArgsWhack
#include <kfg/config.h> /* KConfigDisableUserSettings */
#include <klib/debug.h> /* KDbgSetString */
#include <ktst/unit_test.hpp> /* TEST_SUITE_WITH_ARGS_HANDLER */
#include <vdb/manager.h> // VDBManagerRelease

static rc_t argsHandler(int argc, char* argv[]) {
    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}
TEST_SUITE_WITH_ARGS_HANDLER(Test_VDBManagerGetObjVersionSuite, argsHandler)

TEST_CASE(Test_VDBManagerGetObjVersion) {
    // N.B. This test is required to run in ncbi-vdb/test/vfs to find local runs

#define LD "SRR599578"
    const ver_t LDV(0x02010016);
#define RD "SRR600096"
    const ver_t RDV(0x02010016);
#define LT "SRR045450"
    const ver_t LTV(0x02000000);
#define RT "SRR053325"
    const ver_t RTV(0x02090001);

    ver_t v(0);

    const VDBManager *m(NULL);
    REQUIRE_RC(VDBManagerMakeRead(&m, NULL));

    REQUIRE_RC_FAIL(VDBManagerGetObjVersion(NULL, &v, RT));
    REQUIRE_RC_FAIL(VDBManagerGetObjVersion(m, NULL, RT));
    REQUIRE_RC_FAIL(VDBManagerGetObjVersion(m, &v, NULL));

    // remote table
    REQUIRE_RC(VDBManagerGetObjVersion(m, &v, RT));
    REQUIRE_EQ(v, RTV);

    // remote DB
    REQUIRE_RC(VDBManagerGetObjVersion(m, &v, RD));
    REQUIRE_EQ(v, RDV);

    // local table AD
    REQUIRE_RC(VDBManagerGetObjVersion(m, &v, LT));
    REQUIRE_EQ(v, LTV);

    // local DB AD
    REQUIRE_RC(VDBManagerGetObjVersion(m, &v, LD));
    REQUIRE_EQ(v, LDV);

    // path to local table AD
    REQUIRE_RC(VDBManagerGetObjVersion(m, &v, LT "/"));
    REQUIRE_EQ(v, LTV);

    // path to local DB AD
    REQUIRE_RC(VDBManagerGetObjVersion(m, &v, LD "/"));
    REQUIRE_EQ(v, LDV);

    // path to local table run file
    REQUIRE_RC(VDBManagerGetObjVersion(m, &v, LT "/" LT ".sra"));
    REQUIRE_EQ(v, LTV);

    // path to local DB run file
    REQUIRE_RC(VDBManagerGetObjVersion(m, &v, LD "/" LD ".sralite"));
    REQUIRE_EQ(v, LDV);

    REQUIRE_RC(VDBManagerRelease(m));
}

extern "C" {
    ver_t CC KAppVersion(void) { return 0; }
    const char UsageDefaultName[]("Test_VDBManagerGetObjVersion");
    rc_t CC UsageSummary(const char *progname) { return 0; }
    rc_t CC Usage(const Args *args) { return 0; }

    rc_t CC KMain(int argc, char *argv[]) {
        if (0)
            KDbgSetString("VFS");

        KConfigDisableUserSettings();

        return Test_VDBManagerGetObjVersionSuite(argc, argv);
    }
}
