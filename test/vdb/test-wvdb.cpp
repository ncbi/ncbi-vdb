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

#include <sstream>
#include <cstdlib>

using namespace std;

TEST_SUITE( WVdbTestSuite )

// this test case is not very useful but is here as a blueprint for other write-side tests 
TEST_CASE(BlobCorruptOnCommit) 
{
    const string schemaText = 
"function < type T > T echo #1.0 < T val > ( * any row_len ) = vdb:echo;\n"
"table spotdesc #1\n"
"{\n"
"    extern column ascii LABEL = out_label;\n"
"};\n"
"table reference #1 = spotdesc #1\n"
"{\n"

//  this will break the test
//"    ascii out_label = < ascii > echo < \"reference\" > ();\n"

// this will work
"    physical column ascii .LABEL = in_label;\n"
"    ascii in_label = LABEL;\n"
"    ascii out_label = .LABEL | < ascii > echo < 'label' > ();\n"

"};\n"
"database db #1\n"
"{\n"
"    table reference #1 REFERENCE;\n"
"};\n"
;
    const char * schemaSpec = "db";
    const char * databaseName = GetName();

    // MakeDatabase
    VDatabase* db;
    {
        VDBManager* mgr;
        REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        VSchema* schema;
        REQUIRE_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
        REQUIRE_RC ( VSchemaParseText(schema, NULL, schemaText . c_str(), schemaText . size () ) );
        
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
    
    REQUIRE_RC ( VCursorOpen ( cursor  ) );
    
    for ( int i = 0; i < 4096; ++i)
    {
        ostringstream out;
        out << rand();

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, columnIdx, 8, out.str().c_str(), 0, out.str().size() ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );
        
        //REQUIRE_RC ( VCursorFlushPage ( cursor ) ); // kaboom
    }
    
    REQUIRE_RC ( VCursorCommit ( cursor ) );    // this returns rcVDB,rcBlob,rcValidating,rcBlob,rcCorrupt if the schema does not support 
                                                // writing to the LABEL column from the code
    REQUIRE_RC ( VCursorRelease ( cursor ) );
    
    REQUIRE_RC ( VDatabaseRelease ( db ) );
    
    {
        KDirectory* wd;
        REQUIRE_RC ( KDirectoryNativeDir ( & wd ) );
        REQUIRE_RC ( KDirectoryRemove ( wd, true, databaseName ) );
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
    rc_t rc=WVdbTestSuite(argc, argv);
    return rc;
}

}
