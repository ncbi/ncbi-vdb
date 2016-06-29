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

#include <klib/log.h>

#include <vdb/manager.h> // VDBManager
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/schema.h> /* VSchemaRelease */
#include <vdb/vdb-priv.h>

#include <sra/sraschema.h> // VDBManagerMakeSRASchema

#include <kdb/meta.h>
#include <kdb/table.h>

#include <ktst/unit_test.hpp> // TEST_CASE

#include <sysalloc.h>

#include <sstream>
#include <cstdlib>

using namespace std;

TEST_SUITE( WVdbTestSuite )

const string ScratchDir = "./db/";

class WVDB_Fixture
{
public:
    WVDB_Fixture()
    {
        THROW_ON_RC ( KDirectoryNativeDir ( & m_wd ) );
        THROW_ON_RC ( VDBManagerMakeUpdate ( & m_mgr, m_wd ) );
    }
    ~WVDB_Fixture()
    {
        VDBManagerRelease ( m_mgr );
        RemoveDatabase();
        KDirectoryRelease ( m_wd );
    }
    void RemoveDatabase()
    {
        if ( ! m_databaseName . empty () )
        {
            KDirectoryRemove ( m_wd, true, m_databaseName . c_str () );
        }
    }

    KDirectory* m_wd;
    VDBManager* m_mgr;
    string m_databaseName;
};


// this test case is not very useful but is here as a blueprint for other write-side tests
FIXTURE_TEST_CASE ( BlobCorruptOnCommit, WVDB_Fixture)
{
    m_databaseName = ScratchDir + GetName();
    RemoveDatabase();

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
                                          m_databaseName . c_str () ) );
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
}

FIXTURE_TEST_CASE ( ColumnOpenMetadata, WVDB_Fixture )
{   // setting column metadata in a freshly created VDatabase
    m_databaseName = ScratchDir + GetName();
    RemoveDatabase();

    string schemaText = "table table1 #1.0.0 { column ascii column1; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName = "column1";

    VDatabase* db;
    {
        VSchema* schema;
        REQUIRE_RC ( VDBManagerMakeSchema ( m_mgr, & schema ) );
        REQUIRE_RC ( VSchemaParseText ( schema, NULL, schemaText . c_str (), schemaText . size () ) );

        REQUIRE_RC ( VDBManagerCreateDB ( m_mgr,
                                          & db,
                                          schema,
                                          "root_database",
                                          kcmInit + kcmMD5,
                                          "%s",
                                          m_databaseName . c_str () ) );

        VTable* table;
        REQUIRE_RC ( VDatabaseCreateTable ( db , & table, TableName, kcmInit + kcmMD5, TableName ) );

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
        REQUIRE_RC ( VSchemaRelease ( schema ) );
    }
    // update the column's metadata without re-opening the database
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseOpenTableUpdate ( db , & table, TableName ) );
        KTable* ktbl;
        REQUIRE_RC ( VTableOpenKTableUpdate ( table, & ktbl ) );
        KColumn* col;
        REQUIRE_RC ( KTableOpenColumnUpdate ( ktbl, & col, ColumnName ) );

        // finally, open the metadata
        KMetadata *meta;
        REQUIRE_RC ( KColumnOpenMetadataUpdate ( col, &meta ) );

        KMDataNode* node;
        REQUIRE_RC ( KMetadataOpenNodeUpdate ( meta, & node, "key" ) );
        const char* MetadataValue = "metavalue";
        REQUIRE_RC ( KMDataNodeWrite ( node, MetadataValue, string_size ( MetadataValue ) ) );
        REQUIRE_RC ( KMDataNodeRelease ( node ) );

        REQUIRE_RC ( KMetadataRelease ( meta ) );
        REQUIRE_RC ( KColumnRelease ( col ) );
        REQUIRE_RC ( KTableRelease ( ktbl ) );
        REQUIRE_RC ( VTableRelease ( table ) );
        REQUIRE_RC ( VDatabaseRelease ( db ) );
    }
}

FIXTURE_TEST_CASE ( VTableDropColumn_PhysicalColumn, WVDB_Fixture )
{
    m_databaseName = ScratchDir + GetName();
    RemoveDatabase();

    string schemaText = "table table1 #1.0.0 { column ascii column1; column ascii column2; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName1 = "column1";
    const char* ColumnName2 = "column2";

    VDatabase* db;
    {
        VSchema* schema;
        REQUIRE_RC ( VDBManagerMakeSchema ( m_mgr, & schema ) );
        REQUIRE_RC ( VSchemaParseText ( schema, NULL, schemaText . c_str (), schemaText . size () ) );

        REQUIRE_RC ( VDBManagerCreateDB ( m_mgr,
                                          & db,
                                          schema,
                                          "root_database",
                                          kcmInit + kcmMD5,
                                          "%s",
                                          m_databaseName . c_str () ) );

        VTable* table;
        REQUIRE_RC ( VDatabaseCreateTable ( db , & table, TableName, kcmInit + kcmMD5, TableName ) );

        VCursor* cursor;
        REQUIRE_RC ( VTableCreateCursorWrite ( table, & cursor, kcmInsert ) );
        uint32_t column_idx1;
        uint32_t column_idx2;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx1, ColumnName1 ) );
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx2, ColumnName2 ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        // need to insert 2 rows with different values to make the column physical
        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx1, 8, "blah", 0, 4 ) );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx2, 8, "blah", 0, 4 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx1, 8, "eeee", 0, 4 ) );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx2, 8, "eeee", 0, 4 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorCommit ( cursor ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
        REQUIRE_RC ( VTableRelease ( table ) );
        REQUIRE_RC ( VSchemaRelease ( schema ) );
    }
    // drop column1
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseOpenTableUpdate ( db , & table, TableName ) );
        REQUIRE_RC ( VTableDropColumn ( table, ColumnName1 ) );
        REQUIRE_RC ( VTableRelease ( table ) );
    }
    // finally, check resulted db
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseOpenTableUpdate ( db , & table, TableName ) );
        const VCursor* cursor;
        REQUIRE_RC ( VTableCreateCursorRead ( (const VTable*) table, & cursor ) );
        uint32_t column_idx1;
        uint32_t column_idx2;
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        KLogLevel logLevel = KLogLevelGet();
        REQUIRE_RC ( KLogLevelSet ( klogFatal ) );
        REQUIRE_RC_FAIL ( VCursorAddColumn ( cursor, & column_idx1, ColumnName1 ) );
        REQUIRE_RC ( KLogLevelSet ( logLevel ) );
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx2, ColumnName2 ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
        REQUIRE_RC ( VTableRelease ( table ) );
    }
    REQUIRE_RC ( VDatabaseRelease ( db ) );
}

FIXTURE_TEST_CASE ( VTableDropColumn_MetadataColumn_VDB_2735, WVDB_Fixture )
{
    m_databaseName = ScratchDir + GetName();
    RemoveDatabase();

    string schemaText = "table table1 #1.0.0 { column ascii column1; column ascii column2; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName1 = "column1";
    const char* ColumnName2 = "column2";

    VDatabase* db;
    {
        VSchema* schema;
        REQUIRE_RC ( VDBManagerMakeSchema ( m_mgr, & schema ) );
        REQUIRE_RC ( VSchemaParseText ( schema, NULL, schemaText . c_str (), schemaText . size () ) );

        REQUIRE_RC ( VDBManagerCreateDB ( m_mgr,
                                          & db,
                                          schema,
                                          "root_database",
                                          kcmInit + kcmMD5,
                                          "%s",
                                          m_databaseName . c_str () ) );

        VTable* table;
        REQUIRE_RC ( VDatabaseCreateTable ( db , & table, TableName, kcmInit + kcmMD5, TableName ) );

        VCursor* cursor;
        REQUIRE_RC ( VTableCreateCursorWrite ( table, & cursor, kcmInsert ) );
        uint32_t column_idx1;
        uint32_t column_idx2;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx1, ColumnName1 ) );
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx2, ColumnName2 ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        // need to insert 2 rows with same values to keep the column in metadata
        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx1, 8, "blah", 0, 4 ) );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx2, 8, "blah", 0, 4 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx1, 8, "blah", 0, 4 ) );
        REQUIRE_RC ( VCursorWrite ( cursor, column_idx2, 8, "blah", 0, 4 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorCommit ( cursor ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
        REQUIRE_RC ( VTableRelease ( table ) );
        REQUIRE_RC ( VSchemaRelease ( schema ) );
    }
    // drop column1
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseOpenTableUpdate ( db , & table, TableName ) );
        REQUIRE_RC ( VTableDropColumn ( table, ColumnName1 ) );
        REQUIRE_RC ( VTableRelease ( table ) );
    }
    // finally, check resulted db
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseOpenTableUpdate ( db , & table, TableName ) );
        const VCursor* cursor;
        REQUIRE_RC ( VTableCreateCursorRead ( (const VTable*) table, & cursor ) );
        uint32_t column_idx1;
        uint32_t column_idx2;
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        KLogLevel logLevel = KLogLevelGet();
        REQUIRE_RC ( KLogLevelSet ( klogFatal ) );
        REQUIRE_RC_FAIL ( VCursorAddColumn ( cursor, & column_idx1, ColumnName1 ) );
        REQUIRE_RC ( KLogLevelSet ( logLevel ) );
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx2, ColumnName2 ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
        REQUIRE_RC ( VTableRelease ( table ) );
    }
    REQUIRE_RC ( VDatabaseRelease ( db ) );
}

FIXTURE_TEST_CASE ( VCursor_FindNextRowIdDirect, WVDB_Fixture )
{
    m_databaseName = ScratchDir + GetName();
    RemoveDatabase();

    string schemaText = "table table1 #1.0.0 { column ascii column1; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName = "column1";

    {
        VDatabase* db;
        VSchema* schema;
        REQUIRE_RC ( VDBManagerMakeSchema ( m_mgr, & schema ) );
        REQUIRE_RC ( VSchemaParseText ( schema, NULL, schemaText . c_str (), schemaText . size () ) );

        REQUIRE_RC ( VDBManagerCreateDB ( m_mgr,
                                          & db,
                                          schema,
                                          "root_database",
                                          kcmInit + kcmMD5,
                                          "%s",
                                          m_databaseName . c_str () ) );

        VTable* table;
        REQUIRE_RC ( VDatabaseCreateTable ( db , & table, TableName, kcmInit + kcmMD5, TableName ) );

        VCursor* cursor;
        REQUIRE_RC ( VTableCreateCursorWrite ( table, & cursor, kcmInsert ) );
        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        // insert some rows
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
        REQUIRE_RC ( VTableReindex ( table ) );
        REQUIRE_RC ( VTableRelease ( table ) );
        REQUIRE_RC ( VSchemaRelease ( schema ) );
        REQUIRE_RC ( VDatabaseRelease ( db ) );
    }
    {   // reopen
        const VDatabase* db;
        VDBManagerOpenDBRead ( m_mgr, & db, NULL, m_databaseName . c_str () );

        const VTable* table;
        REQUIRE_RC ( VDatabaseOpenTableRead ( db , & table, TableName ) );

        const VCursor* cursor;
        REQUIRE_RC ( VTableCreateCursorRead ( table, & cursor ) );

        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        int64_t next;
        REQUIRE_RC ( VCursorFindNextRowIdDirect ( cursor, column_idx, 1, & next ) );
        REQUIRE_EQ ( (int64_t)1, next ) ;
        REQUIRE_RC ( VCursorFindNextRowIdDirect ( cursor, column_idx, 2, & next ) );
        REQUIRE_EQ ( (int64_t)2, next ) ; // VDB-3075: next == 1

        REQUIRE_RC ( VCursorRelease ( cursor ) );
        REQUIRE_RC ( VTableRelease ( table ) );
        REQUIRE_RC ( VDatabaseRelease ( db ) );
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
