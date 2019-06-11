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
* index write test
*/

#include <ktst/unit_test.hpp>

#include <sysalloc.h>

#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/index.h>
#include <kdb/table.h>
#include <kdb/column.h>
#include <kdb/meta.h>
#include <kapp/main.h>

using namespace std;

TEST_SUITE(KdbTestSuite);

class WKDB_Fixture
{
public:
    WKDB_Fixture()
    {
        THROW_ON_RC ( KDirectoryNativeDir ( & m_wd ) );
        THROW_ON_RC ( KDBManagerMakeUpdate ( & m_mgr, m_wd ) );
    }
    ~WKDB_Fixture()
    {
        KDBManagerRelease ( m_mgr );
        KDirectoryRelease ( m_wd );
    }
    
    KDirectory* m_wd;
    KDBManager* m_mgr;
};

#if 0
FIXTURE_TEST_CASE ( IndexPersist, WKDB_Fixture )
{   // VDB-3577
    KDirectoryRemove(m_wd, true, GetName());
    {
        KDatabase* db;
        REQUIRE_RC(KDBManagerCreateDB(m_mgr, &db, kcmCreate, GetName()));
        
        KIndex *idx;
        REQUIRE_RC(KDatabaseCreateIndex(db, &idx, kitText, kcmCreate, "index"));
        
        REQUIRE_RC(KIndexInsertText(idx, true, "aaaa1", 1));
        REQUIRE_RC(KIndexInsertText(idx, true, "aaaa2", 2));
        REQUIRE_RC(KIndexInsertText(idx, true, "aaaa3", 3));
        
        REQUIRE_RC(KIndexCommit(idx));
        REQUIRE_RC(KIndexRelease(idx));
        
        REQUIRE_RC(KDatabaseRelease(db));
    }
    {
        KDatabase const *db;
        REQUIRE_RC(KDBManagerOpenDBRead(m_mgr, &db, GetName()));
        
        KIndex const *idx;
        REQUIRE_RC(KDatabaseOpenIndexRead(db, &idx, "index"));

        int64_t start_id;
        uint64_t id_count;

        REQUIRE_RC_FAIL(KIndexFindText (idx, "aaaa0", &start_id, &id_count, NULL, NULL));

        REQUIRE_RC(KIndexFindText (idx, "aaaa1", &start_id, &id_count, NULL, NULL));
        REQUIRE_EQ(start_id, (int64_t)1);
        REQUIRE_EQ(id_count, (uint64_t)1);
        
        REQUIRE_RC(KIndexFindText (idx, "aaaa2", &start_id, &id_count, NULL, NULL));
        REQUIRE_EQ(start_id, (int64_t)2);
        REQUIRE_EQ(id_count, (uint64_t)1);

        REQUIRE_RC(KIndexFindText (idx, "aaaa3", &start_id, &id_count, NULL, NULL));
        REQUIRE_EQ(start_id, (int64_t)3);
        REQUIRE_EQ(id_count, (uint64_t)1);
        
        REQUIRE_RC(KIndexRelease(idx));
        
        REQUIRE_RC(KDatabaseRelease(db));
    }
    KDirectoryRemove(m_wd, true, GetName());
}
#endif

#include <fstream>
#include <time.h>

FIXTURE_TEST_CASE ( IndexBigInsert, WKDB_Fixture )
{   // VDB-3577
    KDirectoryRemove(m_wd, true, GetName());
    {
        KDatabase* db;
        REQUIRE_RC(KDBManagerCreateDB(m_mgr, &db, kcmCreate, GetName()));
        
        KIndex *idx;
        REQUIRE_RC(KDatabaseCreateIndex(db, &idx, kitText, kcmCreate, "index"));

        {
            auto strm = std::ifstream("/panfs/pan1/sra-test/index-test/index.log");
            size_t n = 0;
            auto start = clock();
            auto report = start + 10 * CLOCKS_PER_SEC;
            auto prv = n;

            REQUIRE(strm.good());
            for ( ; ; ) {
                int64_t row;
                std::string key;
                if (strm >> row >> key) {
                    rc_t rc = KIndexInsertText(idx, true, key.c_str(), row);
                    if (rc != 0) {
                        REQUIRE_RC(rc);
                    }
                    ++n;
                    if ((n & 0x3FF) == 0) {
                        if (Quitting()) break;
                        auto const now = clock();
                        if (now >= report) {
                            auto const elapsed = (now - start) / double(CLOCKS_PER_SEC);
                            auto const inserts = n - prv;
                            std::cout << "read " << n << " rows; inserted " << inserts << " in last " << elapsed << " seconds; " << inserts / elapsed << "/sec" << std::endl;
                            report = now + (rand() / double(RAND_MAX) + 10) * CLOCKS_PER_SEC;
                            prv = n;
                            start = now;
                            // break;
                        }
                    }
                }
                else {
                    break;
                }
            }
#if 1
            std::cout << "read " << n << " rows" << std::endl;
            REQUIRE_EQ(n, (size_t)590284424);
#endif
        }
        
        REQUIRE_RC(KIndexCommit(idx));
        REQUIRE_RC(KIndexRelease(idx));
        
        REQUIRE_RC(KDatabaseRelease(db));
    }
    KDirectoryRemove(m_wd, true, GetName());
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/debug.h> // KDbgSetString

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

const char UsageDefaultName[] = "test-wkdb-index";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KdbTestSuite(argc, argv);
    return rc;
}

}
