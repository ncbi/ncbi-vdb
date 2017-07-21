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

#include "wvdb-fixture.hpp"

#include <sysalloc.h>
#include <cstdlib>
#include <stdexcept>

#include <vdb/table.h>
#include <vdb/cursor.h>


using namespace std;

TEST_SUITE( ByteCodeTestSuite )

const string ScratchDir = "./bytecode/";

FIXTURE_TEST_CASE ( SimpleColumn, WVDB_Fixture )
{
    m_databaseName = ScratchDir + GetName();
    RemoveDatabase();

    string schemaText = "table table1 #1.0.0 { column ascii column1; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName = "column1";

    MakeDatabase ( schemaText, "root_database" );

    {   // Create database
        VTable* table;
        REQUIRE_RC ( VDatabaseCreateTable ( m_db , & table, TableName, kcmInit + kcmMD5, TableName ) );

        VCursor* cursor;
        REQUIRE_RC ( VTableCreateCursorWrite ( table, & cursor, kcmInsert ) );
        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        // need to insert 2 rows with different values to make the column physical
        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx, 8, "blah", 0, 4 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx, 8, "eeee", 0, 4 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorCommit ( cursor ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
        REQUIRE_RC ( VTableRelease ( table ) );
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

const char UsageDefaultName[] = "test-bytecode";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=ByteCodeTestSuite(argc, argv);
    return rc;
}

}
