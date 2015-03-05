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
#include <sra/sraschema.h> // VDBManagerMakeSRASchema
#include <vdb/schema.h> /* VSchemaRelease */

#include <ktst/unit_test.hpp> // TEST_CASE

#include <sysalloc.h>

using namespace std;

TEST_SUITE( WVdbTestSuite )

#if 0
TEST_CASE(BlobCorruptOnCommit) 
{
    // MakeDatabase
    VDatabase* db;
    {
        VDBManager* mgr;
        REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        VSchema* schema;
        REQUIRE_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
        const char * schemaFile = "align/align.vschema";
        REQUIRE_RC ( VSchemaParseFile(schema, "%s", schemaFile ) );
        const char * schemaSpec = "NCBI:align:db:alignment_sorted";
        const char * databaseName = GetName();
        
        REQUIRE_RC ( VDBManagerCreateDB ( mgr, 
                                          & db, 
                                          schema, 
                                          schemaSpec, 
                                          kcmInit + kcmMD5, 
                                          "%s", 
                                          databaseName ) );
        REQUIRE_RC ( VSchemaRelease ( schema ) );
        REQUIRE_RC ( VDBManagerRelease ( mgr ) );
    }
    
    // MakeCursor
    VCursor* cursor;
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseCreateTable ( db, & table, "REFERENCE", kcmCreate | kcmMD5, "%s", "REFERENCE" ) );
        REQUIRE_RC ( VTableCreateCursorWrite ( table, & cursor, kcmInsert ) );
        REQUIRE_RC ( VTableRelease ( table ) );
    }
    
    // AddColumns
    uint32_t columnIdx;
    REQUIRE_RC ( VCursorAddColumn ( cursor, & columnIdx, "%s", "LABEL" ) );
    // this column has no problem:
    //REQUIRE_RC ( VCursorAddColumn ( cursor, & columnIdx, "%s", "SPOT_ID" ) );
    
    REQUIRE_RC ( VCursorOpen ( cursor  ) );
    REQUIRE_RC ( VCursorOpenRow ( cursor ) );
    REQUIRE_RC ( VCursorWrite ( cursor, columnIdx, 8, "", 0, 0 ) );
    REQUIRE_RC ( VCursorCommitRow ( cursor ) );
    REQUIRE_RC ( VCursorCloseRow ( cursor ) );
    REQUIRE_RC ( VCursorCommit ( cursor ) ); // kaboom
    REQUIRE_RC ( VCursorRelease ( cursor ) );
    
    REQUIRE_RC ( VDatabaseRelease ( db ) );
}
#endif

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
    rc_t rc=WVdbTestSuite(argc, argv);
    return rc;
}

}
