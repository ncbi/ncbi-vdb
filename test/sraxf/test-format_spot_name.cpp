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
 * to add debug arguments to to command line use: "-app_args='-+VFS -s'"
 * ========================================================================== */

#include <kapp/args.h> /* Args */
#include <kfg/kfg-priv.h> /* KConfigMakeEmpty */
#include <ktst/unit_test.hpp> /* TEST_SUITE_WITH_ARGS_HANDLER */

#include <vdb/cursor.h> /* VCursorRelease */
#include <vdb/database.h> /* VDatabaseRelease */
#include <vdb/manager.h> /* VDBManagerRelease */
#include <vdb/table.h> /* VTableRelease */

static rc_t argsHandler(int argc, char* argv[]) {

    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

TEST_SUITE_WITH_ARGS_HANDLER(FormatSpotNameSuite, argsHandler)

TEST_CASE(TestFormatSpotName) { // VDB-4097
    const VDBManager *mgr = NULL;
    REQUIRE_RC(VDBManagerMakeRead(&mgr, NULL));

    const VDatabase *db = NULL;
    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, "data/SRR9203980.1"));

    const VTable *tbl = NULL;
    REQUIRE_RC(VDatabaseOpenTableRead(db, &tbl, "SEQUENCE"));

    const VCursor *curs;
    REQUIRE_RC(VTableCreateCachedCursorRead(tbl, &curs, 256 * 1024 * 1024));
    uint32_t idx = 0;
    REQUIRE_RC(VCursorAddColumn(curs, &idx, "NAME"));
    REQUIRE_RC(VCursorOpen(curs));
    REQUIRE_RC(VCursorOpenRow(curs));

    const void *base = NULL;
    uint32_t boff = 0;
    uint32_t row_len = 0;
    REQUIRE_RC(VCursorCellData(curs, idx, NULL, &base, &boff, &row_len));

    REQUIRE_RC(VCursorRelease(curs));

    REQUIRE_RC(VTableRelease(tbl));

    REQUIRE_RC(VDatabaseRelease(db));

    REQUIRE_RC(VDBManagerRelease(mgr));
}

const char UsageDefaultName[] = "test-format-spot-name";
rc_t CC UsageSummary(const char * progname) { return 0; }
rc_t CC Usage(const Args * args) { return 0; }

extern "C" {

    ver_t CC KAppVersion(void) { return 0; }

    int KMain(int argc, char *argv[]) {
        KConfigDisableUserSettings(); // ignore ~/.ncbi/user-settings.mkfg

        rc_t rc = FormatSpotNameSuite(argc, argv);
        return rc;
    }

}

/******************************************************************************/
