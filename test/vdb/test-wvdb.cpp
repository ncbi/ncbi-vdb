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

class WVDBFixture
{
public:
    WVDBFixture() 
    : m_databaseName(0), m_db(0)
    {
    }
    ~WVDBFixture() 
    {
        if ( m_db )
        {
            VDatabaseRelease ( m_db );
        }
        //RemoveDatabase();
    }
    
    void RemoveDatabase ()
    {
        if ( m_databaseName )
        {
            KDirectory* wd;
            KDirectoryNativeDir ( & wd );
            KDirectoryRemove ( wd, true, m_databaseName ); 
            KDirectoryRelease ( wd );
        }
    }

    void MakeDatabase ( const string& p_schemaText, const string& p_schemaSpec ) 
    {
        RemoveDatabase();
    
        VDBManager* mgr;
        THROW_ON_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        VSchema* schema;
        THROW_ON_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
        THROW_ON_RC ( VSchemaParseText(schema, NULL, p_schemaText . c_str(), p_schemaText . size () ) );
        
        THROW_ON_RC ( VDBManagerCreateDB ( mgr, 
                                          & m_db, 
                                          schema, 
                                          p_schemaSpec . c_str (), 
                                          kcmInit + kcmMD5, 
                                          "%s", 
                                          m_databaseName ) );
        THROW_ON_RC ( VSchemaRelease ( schema ) );
        THROW_ON_RC ( VDBManagerRelease ( mgr ) );
    }

    const char * m_databaseName;
    VDatabase* m_db;
};

// this test case is not very useful but is here as a blueprint for other write-side tests 
FIXTURE_TEST_CASE ( BlobCorruptOnCommit, WVDBFixture ) 
{
    m_databaseName = GetName();
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

    MakeDatabase ( schemaText, "db" );
    
    // MakeCursor
    VCursor* cursor;
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseCreateTable ( m_db, & table, "REFERENCE", kcmCreate | kcmMD5, "%s", "REFERENCE" ) );
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
}

FIXTURE_TEST_CASE ( CreateTableInNestedDatabase, WVDBFixture ) 
{   // VDB-1617: VDatabaseOpenTableRead inside a nested database segfaults
    m_databaseName = GetName();
    string schemaText = 
        "table table1 #1.0.0 { column ascii column1; };"
        "database database0 #1 { table table1 #1 TABLE1; } ;" 
        "database db #1 { database database0 #1 SUBDB; } ;" ; 
    
    const char * hallo = "hallo";

    // Create the database and the table
    MakeDatabase ( schemaText, "db" );
    {   // make nested database and a table in it
        VDatabase* subdb;
        REQUIRE_RC ( VDatabaseCreateDB ( m_db, & subdb, "SUBDB", kcmInit + kcmMD5, "SUBDB" ) );
                                      
        VTable *tbl;
        REQUIRE_RC ( VDatabaseCreateTable ( subdb, & tbl, "TABLE1", kcmInit + kcmMD5, "TABLE1" ) );

        VCursor *curs;
        REQUIRE_RC ( VTableCreateCursorWrite ( tbl, & curs, kcmInsert ) );
        
        uint32_t col_idx;
        REQUIRE_RC ( VCursorAddColumn ( curs, & col_idx, "column1" ) );
        REQUIRE_RC ( VCursorOpen ( curs ) );
        REQUIRE_RC ( VCursorOpenRow ( curs ) );
        REQUIRE_RC ( VCursorWrite ( curs, col_idx, 8, hallo, 0, 5 ) );

        REQUIRE_RC ( VCursorCommitRow ( curs ) );
        REQUIRE_RC ( VCursorCloseRow ( curs ) );
        REQUIRE_RC ( VCursorCommit ( curs ) );
        
        REQUIRE_RC ( VCursorRelease ( curs ) );
        
        REQUIRE_RC ( VTableRelease ( tbl ) );
        REQUIRE_RC ( VDatabaseRelease ( subdb ) );
    }
    REQUIRE_RC ( VDatabaseRelease ( m_db ) );
    
    // Re-open the database, try to open the table
    {
        VDBManager * mgr;
        VDatabase  * db;
        VDatabase  * subdb;
        const VTable * tbl;

        REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        REQUIRE_RC ( VDBManagerOpenDBUpdate ( mgr, 
                                              & db, 
                                              NULL, 
                                              "%s", 
                                              m_databaseName ) );
        REQUIRE_RC ( VDatabaseOpenDBUpdate ( db, & subdb, "SUBDB" ) );

        // open the nested database and a table in it
        //REQUIRE_RC ( VDatabaseOpenTableRead ( subdb, & tbl, NULL, "TABLE1" ) ); // segfault
        //REQUIRE_RC ( VTableRelease ( tbl ) );
        
        REQUIRE_RC ( VDatabaseRelease ( subdb ) );
        REQUIRE_RC ( VDBManagerRelease ( mgr ) );
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
