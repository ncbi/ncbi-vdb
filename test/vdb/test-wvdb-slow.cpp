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

#include "WVDB_Fixture.hpp"

using namespace std;

TEST_SUITE( WVdbSlowTestSuite );

const string ScratchDir = "./db/";

#if 0
FIXTURE_TEST_CASE ( VCursorCommit_BufferOverflow, WVDB_Fixture )
{   // VDB-4341
    m_databaseName = ScratchDir + GetName();
    RemoveDatabase();

    string schemaText =
        "table table1 #1.0.0 { column ascii column1; };"
        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName1 = "column1";

    MakeDatabase ( schemaText, "root_database" );
    {
        VCursor* cursor = CreateTable ( TableName );

        // make sure we have enough memory
        try
        {
           char * mem = (char*)new char[20][1000][1000][1000];
           delete [] mem;
        }
        catch (...)
        {
           cout << "not enough memory, skipping VCursorCommit_BufferOverflow" << endl;
           return;
        }

        uint32_t column_idx1;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx1, ColumnName1 ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        string s = string ( 1858195865, ' ' );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx1, 8, s.c_str(), 0, s.size() ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );
        // REQUIRE_RC ( VCursorFlushPage ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        s = string ( 1355058268, ' ' );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx1, 8, s.c_str(), 0, s.size() ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );
        // REQUIRE_RC ( VCursorFlushPage ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        s = string ( 2093616421, ' ' );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx1, 8, s.c_str(), 0, s.size() ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );
        // REQUIRE_RC ( VCursorFlushPage ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        s = string ( 1651157214, ' ' );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx1, 8, s.c_str(), 0, s.size() ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );
        // REQUIRE_RC ( VCursorFlushPage ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        s = string ( 2089204295, ' ' );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx1, 8, s.c_str(), 0, s.size() ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );
        // REQUIRE_RC ( VCursorFlushPage ( cursor ) );

        REQUIRE_RC ( VCursorCommit ( cursor ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
}
#endif

FIXTURE_TEST_CASE ( VCursor_PageMapOverflow, WVDB_Fixture )
{   // VDB-4897
    m_databaseName = ScratchDir + GetName();
    RemoveDatabase();

    string schemaText =
        "table table1 #1.0.0 { column ascii column1; };"
        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName1 = "column1";

    MakeDatabase ( schemaText, "root_database" );
    {
        VCursor* cursor = CreateTable ( TableName );

        uint32_t column_idx1;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx1, ColumnName1 ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        string s = string ();
        for ( uint64_t i = 0; i < numeric_limits<uint32_t>::max(); ++i)
        {
if ( i % 1000000 == 0 )
cout << i/1000000 <<endl;
            REQUIRE_RC ( VCursorOpenRow ( cursor ) );
            REQUIRE_RC ( VCursorWrite ( cursor, column_idx1, 8, s.c_str(), 0, s.size() ) );
            REQUIRE_RC ( VCursorCommitRow ( cursor ) );
            REQUIRE_RC ( VCursorCloseRow ( cursor ) );
        }

        REQUIRE_RC ( VCursorCommit ( cursor ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
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

const char UsageDefaultName[] = "test-wvdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=WVdbSlowTestSuite(argc, argv);
    return rc;
}

}
