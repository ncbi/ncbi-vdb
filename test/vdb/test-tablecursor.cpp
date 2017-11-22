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

#include <vdb/cursor.h>

#include <vdb/table.h>

#include "WVDB_Fixture.hpp"

using namespace std;

TEST_SUITE( VdbTableCursorTestSuite )
const string ScratchDir = "./db/";

FIXTURE_TEST_CASE( VTableCursor_Make, WVDB_Fixture )
{
    m_databaseName = ScratchDir + GetName();

    const string schemaText =
        "table T#1 { column ascii c; };"
        "database db #1 { table T#1 t; };"
    ;
    const char * schemaSpec = "db";
    MakeDatabase ( schemaText, schemaSpec );
    VTable* table;
    REQUIRE_RC ( VDatabaseCreateTable ( m_db, & table, "t", kcmCreate | kcmMD5, "%s", "t" ) );

    VCursor* cursor;
    REQUIRE_RC ( VTableCreateCursorWrite ( table, & cursor, kcmInsert ) );

    REQUIRE_RC ( VCursorRelease ( cursor ) );

    REQUIRE_RC ( VTableRelease ( table ) );
}

//////////////////////////////////////////// Main
#include <kfg/config.h>

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

const char UsageDefaultName[] = "test-tablecursor";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=VdbTableCursorTestSuite(argc, argv);
    return rc;
}

}
