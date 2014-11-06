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

#include <ktst/unit_test.hpp>

#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/index.h>

using namespace std;

TEST_SUITE(KdbTestSuite);

TEST_CASE(MissingRows)
{   // VDB-177
    KDirectory* wd;
    REQUIRE_RC(KDirectoryNativeDir(&wd));
    KDirectoryRemove(wd, true, GetName().c_str());
    
    KDBManager* mgr;
    REQUIRE_RC(KDBManagerMakeUpdate(&mgr, wd));
    
    KDatabase* db;
    REQUIRE_RC(KDBManagerCreateDB(mgr, &db, kcmCreate, GetName().c_str()));
    
    KIndex *idx;
    REQUIRE_RC(KDatabaseCreateIndex(db, &idx, kitText, kcmCreate, "index"));
    
    REQUIRE_RC(KIndexInsertText(idx, true, "aaaa1", 1));
    REQUIRE_RC(KIndexInsertText(idx, true, "aaaa3", 3));
    
    int64_t start_id;
    uint64_t id_count;
    REQUIRE_RC(KIndexFindText (idx, "aaaa1", &start_id, &id_count, NULL, NULL));
    REQUIRE_EQ(start_id, (int64_t)1);
    REQUIRE_EQ(id_count, (uint64_t)1);
    
    REQUIRE_RC_FAIL(KIndexFindText (idx, "aaaa2", &start_id, &id_count, NULL, NULL));
    REQUIRE_RC_FAIL(KIndexFindText (idx, "", &start_id, &id_count, NULL, NULL));
    
    REQUIRE_RC(KIndexFindText (idx, "aaaa3", &start_id, &id_count, NULL, NULL));
    REQUIRE_EQ(start_id, (int64_t)3);
    REQUIRE_EQ(id_count, (uint64_t)1);
    
    REQUIRE_RC(KIndexRelease(idx));
    
    REQUIRE_RC(KDatabaseRelease(db));
    REQUIRE_RC(KDBManagerRelease(mgr));
    
    KDirectoryRemove(wd, true, GetName().c_str());
    REQUIRE_RC(KDirectoryRelease(wd));
    
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

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

const char UsageDefaultName[] = "test-wkdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KdbTestSuite(argc, argv);
    return rc;
}

}
