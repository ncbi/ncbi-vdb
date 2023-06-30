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
 * Unit tests for Kdb interface
 */

#include <kapp/args.h>

#include <ktst/unit_test.hpp>

#include <memory>
#include <string>
#include <sysalloc.h>
#include <vector>

#include <kdb/column.h>
#include <kdb/database.h>
#include <kdb/extern.h>
#include <kdb/index.h>
#include <kdb/kdb-priv.h>
#include <kdb/manager.h>
#include <kdb/meta.h>
#include <kdb/namelist.h>
#include <kdb/table.h>
#include <kfs/directory.h>
#include <klib/printf.h>
#include <vfs/manager.h>

using namespace std;

TEST_SUITE ( KdbTestSuite );

const char UsageDefaultName[] = "VDB-5323";
#define INSERT_2 0

class KDB_KIndexFixture {
public:
    void createTable(const char *tblname, unsigned keys, char const *const *key)
    {
        created.push_back ( tblname );

        KTable *Tbl = nullptr;
        THROW_ON_RC ( KDBManagerCreateTable ( mgr, &Tbl,
            kcmInit | kcmCreate | kcmParents, "%s/%s", tempPath (), tblname ) );

        KIndex *Ndx = nullptr;
        THROW_ON_RC (
            KTableCreateIndex ( Tbl, &Ndx, kitText | kitProj, kcmOpen, "%s", indexName ) );

        for (unsigned i = 0; i < keys; ++i) {
            THROW_ON_RC ( KIndexInsertText(Ndx, false, key[i], i + 1) );
        }
        THROW_ON_RC ( KIndexRelease ( Ndx ) );
        THROW_ON_RC ( KTableRelease ( Tbl ) );
    }

    void checkTable(const char *tblname, unsigned keys, char const *const *key) {
        KTable const *Tbl = nullptr;
        THROW_ON_RC(KDBManagerOpenTableRead(mgr, &Tbl, "%s/%s", tempPath(), tblname));

        KIndex const *Ndx = nullptr;
        THROW_ON_RC(KTableOpenIndexRead(Tbl, &Ndx, "%s", indexName));

        for (unsigned i = 0; i < keys; ++i) {
            int64_t start_id = 0;
            uint64_t count = 0;
            THROW_ON_RC(KIndexFindText(Ndx, key[i], &start_id, &count, NULL, NULL));
            THROW_ON_FALSE(start_id == i + 1);
            THROW_ON_FALSE(count == 1);
        }
        THROW_ON_RC ( KIndexRelease ( Ndx ) );
        THROW_ON_RC ( KTableRelease ( Tbl ) );
    }

    static char const *tempPath() {
        static char const *const env_vars[] = { "TMPDIR", "TEMPDIR", "TEMP",
            "TMP", "TMP_DIR", "TEMP_DIR", nullptr };
        for ( auto var = env_vars; *var; ++var ) {
            auto const val = getenv ( *var );
            if ( val ) return val;
        }
        return "/tmp";
    }

    ~KDB_KIndexFixture ()
    {
        for ( auto tblname : created ) {
            KDirectoryRemove (
                Dir, true, "%s/%s", tempPath (), tblname.c_str () );
        }
        KDBManagerRelease(mgr);
        KDirectoryRelease(Dir);
    }

    KDB_KIndexFixture() {
        THROW_ON_RC ( KDirectoryNativeDir ( &Dir ) );
        THROW_ON_RC ( KDBManagerMakeUpdate ( &mgr, Dir ) );
    }

private:
    static constexpr char const *indexName = "NDX_1";

    KDirectory *Dir;
    KDBManager *mgr;
    vector<string> created;
};

static constexpr char const *keyValues[] = {
    "K_1",
    "K_2"
};

FIXTURE_TEST_CASE ( CheckIndex2, KDB_KIndexFixture )
{
    static constexpr char const *tableName = "VDB-5323_2.ktable";
    createTable(tableName, 2, &keyValues[0]);
    checkTable(tableName, 2, &keyValues[0]);
}

FIXTURE_TEST_CASE ( CheckIndex1, KDB_KIndexFixture )
{
    static constexpr char const *tableName = "VDB-5323_1.ktable";
    createTable(tableName, 1, &keyValues[0]);
    checkTable(tableName, 1, &keyValues[0]);
}

extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/debug.h> // KDbgSetString

ver_t CC KAppVersion ( void ) { return 0x1000000; }
rc_t CC UsageSummary ( const char *progname ) { return 0; }

rc_t CC Usage ( const Args *args ) { return 0; }

rc_t CC KMain ( int argc, char *argv[] )
{
    KConfigDisableUserSettings ();
    return KdbTestSuite ( argc, argv );
}
}
