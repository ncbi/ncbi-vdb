// ===========================================================================
//
//                            PUBLIC DOMAIN NOTICE
//               National Center for Biotechnology Information
//
//  This software/database is a "United States Government Work" under the
//  terms of the United States Copyright Act.  It was written as part of
//  the author's official duties as a United States Government employee and
//  thus cannot be copyrighted.  This software/database is freely available
//  to the public for use. The National Library of Medicine and the U.S.
//  Government have not placed any restriction on its use or reproduction.
//
//  Although all reasonable efforts have been taken to ensure the accuracy
//  and reliability of the software and data, the NLM and the U.S.
//  Government do not and cannot warrant the performance or results that
//  may be obtained by using this software or data. The NLM and the U.S.
//  Government disclaim all warranties, express or implied, including
//  warranties of performance, merchantability or fitness for any particular
//  purpose.
//
//  Please cite the author in any work or product based on this material.
//
// ===========================================================================

#include <vdb/manager.h> // VDBManager
#include <vdb/database.h> 
#include <vdb/table.h> 
#include <vdb/cursor.h> 
#include <vdb/vdb-priv.h>
//#include <sra/sradb-priv.h>

#include <ktst/unit_test.hpp> // TEST_CASE
#include <kfg/config.h> 

#include <sysalloc.h>
#include <cstdlib>

TEST_SUITE( VdbTestSuite )

extern "C" rc_t VDBManagerRunPeriodicTasks(const VDBManager*);

TEST_CASE( VdbMgr ) {
    const VDBManager *mgr = NULL;
    rc_t rc = VDBManagerMakeRead(NULL, NULL);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerMakeRead(NULL) succeed" );

    rc = VDBManagerMakeRead(&mgr, NULL);
    if (rc != 0)
        FAIL( "failed to make VDB manager" );

    rc = VDBManagerAddRef(mgr);
    if (rc != 0)
        FAIL( "failed to AddRef VDB manager" );

    rc = VDBManagerRelease(mgr);
    if (rc != 0)
        FAIL( "failed to release VDB manager" );

    rc = VDBManagerVersion(mgr, NULL);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerVersion(mgr, NULL) succeed" );

    uint32_t version = 0;
    rc = VDBManagerVersion(NULL, &version);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerVersion(NULL) succeed" );

    rc = VDBManagerVersion(mgr, &version);
    if (rc == 0)
        CHECK_GE( version, (uint32_t)0x2000000 );
    else
        FAIL( "failed to get VDB manager Version" );

    rc = VDBManagerRunPeriodicTasks(NULL);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerRunPeriodicTasks(NULL) succeed" );

    rc = VDBManagerRunPeriodicTasks(mgr);
    if (rc != 0)
        FAIL( "failed VDBManagerRunPeriodicTasks" );

    // In the following, NULL and "" are interpreted as "."

    // VDBManagerAddSchemaIncludePath
    REQUIRE_RC(VDBManagerAddSchemaIncludePath(mgr, NULL));
    REQUIRE_RC(VDBManagerAddSchemaIncludePath(mgr, ""));
#ifndef WINDOWS    
    REQUIRE_RC(VDBManagerAddSchemaIncludePath(mgr, "/"));
    REQUIRE_RC(VDBManagerAddSchemaIncludePath(mgr, "/usr"));
#else    
    REQUIRE_RC_FAIL(VDBManagerAddSchemaIncludePath(mgr, "/"));
    REQUIRE_RC_FAIL(VDBManagerAddSchemaIncludePath(mgr, "/usr"));
#endif

    // VDBManagerAddLoadLibraryPath
    REQUIRE_RC(VDBManagerAddLoadLibraryPath(mgr, NULL));
    REQUIRE_RC(VDBManagerAddLoadLibraryPath(mgr, ""));
#ifndef WINDOWS    
    REQUIRE_RC(VDBManagerAddLoadLibraryPath(mgr, "/"));
    REQUIRE_RC(VDBManagerAddLoadLibraryPath(mgr, "/usr"));
#else
    REQUIRE_RC_FAIL(VDBManagerAddLoadLibraryPath(mgr, "/"));
    REQUIRE_RC_FAIL(VDBManagerAddLoadLibraryPath(mgr, "/usr"));
#endif
    //

    struct Test { int i; };
    Test t;
    t.i = 12345;
    rc = VDBManagerGetUserData(mgr, NULL);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerGetUserData(mgr, NULL) succeed" );
    void *data = NULL;
    rc = VDBManagerGetUserData(NULL, &data);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerGetUserData(NULL) succeed" );
    rc = VDBManagerGetUserData(mgr, &data);
    if (rc != 0)
        FAIL( "failed VDBManagerGetUserData" );
    if (data != NULL)
        FAIL( "VDBManagerGetUserData != NULL" );

    rc = VDBManagerSetUserData(NULL, &t, free);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerSetUserData(NULL) succeed" );
    rc = VDBManagerSetUserData(mgr, &t, NULL);
    if (rc != 0)
        FAIL( "failed VDBManagerSetUserData" );
    rc = VDBManagerGetUserData(mgr, &data);
    if (rc != 0)
        FAIL( "failed VDBManagerGetUserData after Set" );
    if (data == NULL)
        FAIL( "VDBManagerGetUserData == NULL after Get" );
    else {
        Test *r = static_cast<Test*>(data);
        CHECK_EQUAL( r->i, 12345 );
    }

    Test *pt = static_cast<Test*>(malloc(sizeof *pt));
    rc = VDBManagerSetUserData(mgr, pt, free);
    if (rc != 0)
        FAIL( "failed VDBManagerSetUserData(free)" );

    //

    rc = VDBManagerRelease(mgr);
    if (rc != 0)
        FAIL( "failed to release VDB manager" );
    mgr = NULL;

}

#if _ARCH_BITS != 32
TEST_CASE(SimultaneousCursors)
{   // SRA-1669 WGS ALWZ01 cannot open multiple cursors with SEQUENCE.CONTIG_NAME column simultaneously (Win32)

    for ( int i = 0; i < 1; ++i ) {
        const VDBManager* mgr = 0;
        REQUIRE_RC(VDBManagerMakeRead(&mgr, 0));
        const VDatabase* db = 0;
        REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, 0, "ALWZ01"));
        const VTable* table = 0;
        REQUIRE_RC(VDatabaseOpenTableRead(db, &table, "SEQUENCE"));
        const size_t CURSOR_CNT = 10; 
        const VCursor* cursors[CURSOR_CNT] = {};
        for ( size_t i = 0; i < CURSOR_CNT; ++i ) {
            const VCursor* cursor = 0;
            REQUIRE_RC(VTableCreateCursorRead(table, &cursor));
            cursors[i] = cursor;
            REQUIRE_RC(VCursorPermitPostOpenAdd(cursor));
            REQUIRE_RC(VCursorOpen(cursor));
            uint32_t col_index;
            REQUIRE_RC(VCursorAddColumn(cursor, &col_index, "CONTIG_NAME"));
        }
        for ( size_t i = 0; i < CURSOR_CNT; ++i ) {
            REQUIRE_RC(VCursorRelease(cursors[i]));
        }
        REQUIRE_RC(VTableRelease(table));
        REQUIRE_RC(VDatabaseRelease(db));
        REQUIRE_RC(VDBManagerRelease(mgr));
    }
}
#endif

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

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

const char UsageDefaultName[] = "test-vdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=VdbTestSuite(argc, argv);
    return rc;
}

}
