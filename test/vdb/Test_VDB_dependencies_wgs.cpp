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
 * Resolving internal reference sequences that are not found first:
 * this test should not print any error message.
 * ========================================================================== */

#include <kapp/args.h> // ArgsWhack
#include <kfg/config.h> /* KConfigDisableUserSettings */
#include <ktst/unit_test.hpp> /* TEST_SUITE_WITH_ARGS_HANDLER */
#include <vdb/database.h> /* VDatabaseRelease */
#include <vdb/dependencies.h> /* VDBDependenciesRelease */
#include <vdb/manager.h> // VDBManagerRelease

#define ALL

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

static rc_t argsHandler(int argc, char* argv[]){
    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}
TEST_SUITE_WITH_ARGS_HANDLER(Test_VDB_dependencies_wgsSuite, argsHandler)

#ifdef ALL
TEST_CASE(Test_VDB_dependencies_wgs) {
    const VDBManager *m = NULL;
    REQUIRE_RC(VDBManagerMakeRead(&m, NULL));

    const VDatabase *b= NULL;
    REQUIRE_RC(VDBManagerOpenDBRead(m, &b, NULL, "ERR3091357"));

    const VDBDependencies *d = NULL;
    /* The following has to make a single SDL call for 
     multiple WGS refseqs that are resolved to the same URL */
    REQUIRE_RC(VDatabaseListDependencies(b, &d, false));
    REQUIRE_RC(VDBDependenciesRelease(d));

    REQUIRE_RC(VDatabaseRelease(b));

    REQUIRE_RC(VDBManagerRelease(m));
}
#endif

extern "C" {
    ver_t CC KAppVersion(void) { return 0x1000000; }
    rc_t CC Usage(const Args *args) { return 0; }
    const char UsageDefaultName[]("Test_VDB_dependencies_wgs");
    rc_t CC UsageSummary(const char *progname) { return 0; }

    rc_t CC KMain(int argc, char *argv[]) {
        KConfigDisableUserSettings();
        return Test_VDB_dependencies_wgsSuite(argc, argv);
    }
}
