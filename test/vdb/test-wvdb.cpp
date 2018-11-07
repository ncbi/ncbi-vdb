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

#include <sysalloc.h>

#include <sstream>
#include <cstdlib>

#include <klib/log.h>

#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>

#include <sra/sraschema.h> // VDBManagerMakeSRASchema

#include <kdb/meta.h>
#include <kdb/table.h>

#include <kfg/config.h>

#include <../../libs/vdb/schema-priv.h>
#include <../../libs/vdb/schema-parse.h>
#include <../../libs/vdb/column-priv.h>
#include <../../libs/vdb/cursor-priv.h>

#include "WVDB_Fixture.hpp"

using namespace std;

TEST_SUITE( WVdbTestSuite )

// this test case is not very useful but is here as a blueprint for other write-side tests
FIXTURE_TEST_CASE ( BlobCorruptOnCommit, WVDB_Fixture)
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
    MakeDatabase ( GetName(), schemaText, schemaSpec );

    // MakeCursor
    VCursor* cursor = CreateTable ( "REFERENCE" );

    // AddColumns
    uint32_t columnIdx;
    REQUIRE_RC ( VCursorAddColumn ( cursor, & columnIdx, "%s", "LABEL" ) );

    REQUIRE_RC ( VCursorOpen ( cursor  ) );

    for ( int i = 0; i < 4096; ++i)
    {
        ostringstream out;
        out << rand();
        WriteRow ( cursor, columnIdx, out.str() );

        //REQUIRE_RC ( VCursorFlushPage ( cursor ) ); // kaboom
    }

    REQUIRE_RC ( VCursorCommit ( cursor ) );    // this returns rcVDB,rcBlob,rcValidating,rcBlob,rcCorrupt if the schema does not support
                                                // writing to the LABEL column from the code
    REQUIRE_RC ( VCursorRelease ( cursor ) );
}

FIXTURE_TEST_CASE ( ColumnOpenMetadata, WVDB_Fixture )
{   // setting column metadata in a freshly created VDatabase
    string schemaText = "table table1 #1.0.0 { column ascii column1; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName = "column1";

    MakeDatabase ( GetName(), schemaText, "root_database" );

    {
        VCursor* cursor = CreateTable ( TableName );

        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        // need to insert 2 rows with different values to make the column physical
        WriteRow ( cursor, column_idx, "blah" );
        WriteRow ( cursor, column_idx, "eeee" );

        REQUIRE_RC ( VCursorCommit ( cursor ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
    // update the column's metadata without re-opening the database
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseOpenTableUpdate ( m_db , & table, TableName ) );
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
    }
}

FIXTURE_TEST_CASE ( VTableDropColumn_PhysicalColumn, WVDB_Fixture )
{
    string schemaText = "table table1 #1.0.0 { column ascii column1; column ascii column2; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName1 = "column1";
    const char* ColumnName2 = "column2";

    MakeDatabase ( GetName(), schemaText, "root_database" );
    {
        VCursor* cursor = CreateTable ( TableName );

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
    }
    // drop column1
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseOpenTableUpdate ( m_db , & table, TableName ) );
        REQUIRE_RC ( VTableDropColumn ( table, ColumnName1 ) );
        REQUIRE_RC ( VTableRelease ( table ) );
    }
    // finally, check resulted db
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseOpenTableUpdate ( m_db , & table, TableName ) );
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
}

FIXTURE_TEST_CASE ( CreateTableInNestedDatabase, WVDB_Fixture )
{   // VDB-1617: VDatabaseOpenTableRead inside a nested database segfaults
    string schemaText =
        "table table1 #1.0.0 { column ascii column1; };"
        "database database0 #1 { table table1 #1 TABLE1; } ;"
        "database db #1 { database database0 #1 SUBDB; } ;" ;

    // Create the database and the table
    MakeDatabase ( GetName(), schemaText, "db" );
    {   // make nested database and a table in it
        VDatabase* subdb;
        REQUIRE_RC ( VDatabaseCreateDB ( m_db, & subdb, "SUBDB", kcmInit + kcmMD5, "SUBDB" ) );

        VTable *tbl;
        REQUIRE_RC ( VDatabaseCreateTable ( subdb, & tbl, "TABLE1", kcmInit + kcmMD5, "TABLE1" ) );

        REQUIRE_RC ( VTableRelease ( tbl ) );
        REQUIRE_RC ( VDatabaseRelease ( subdb ) );
    }
    REQUIRE_RC ( VDatabaseRelease ( m_db ) );

    // Re-open the database, try to open the table
    {
        VDBManager * mgr;
        VDatabase  * subdb;
        const VTable * tbl;

        REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        REQUIRE_RC ( VDBManagerOpenDBUpdate ( mgr,
                                              & m_db,
                                              NULL,
                                              "%s",
                                              m_databaseName . c_str () ) );
        REQUIRE_RC ( VDatabaseOpenDBUpdate ( m_db, & subdb, "SUBDB" ) );

        // open the nested database and a table in it
        REQUIRE_RC ( VDatabaseOpenTableRead ( subdb, & tbl, "%s", "TABLE1" ) ); // segfault no more...
        REQUIRE_RC ( VTableRelease ( tbl ) );

        REQUIRE_RC ( VDatabaseRelease ( subdb ) );
        REQUIRE_RC ( VDBManagerRelease ( mgr ) );
    }
}

FIXTURE_TEST_CASE ( VTableDropColumn_MetadataColumn_VDB_2735, WVDB_Fixture )
{
    string schemaText = "table table1 #1.0.0 { column ascii column1; column ascii column2; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName1 = "column1";
    const char* ColumnName2 = "column2";

    MakeDatabase ( GetName(), schemaText, "root_database" );
    {
        VCursor* cursor = CreateTable ( TableName );

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
    }
    // drop column1
    {
        VTable* table;
        REQUIRE_RC ( VDatabaseOpenTableUpdate (m_db , & table, TableName ) );
        REQUIRE_RC ( VTableDropColumn ( table, ColumnName1 ) );
        REQUIRE_RC ( VTableRelease ( table ) );
    }
    // finally, check resulted db
    {
        const VCursor* cursor = OpenTable ( TableName );
        uint32_t column_idx1;
        uint32_t column_idx2;
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        KLogLevel logLevel = KLogLevelGet();
        REQUIRE_RC ( KLogLevelSet ( klogFatal ) );
        REQUIRE_RC_FAIL ( VCursorAddColumn ( cursor, & column_idx1, ColumnName1 ) );
        REQUIRE_RC ( KLogLevelSet ( logLevel ) );
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx2, ColumnName2 ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
}

FIXTURE_TEST_CASE ( VCursor_FindNextRowIdDirect, WVDB_Fixture )
{
    string schemaText = "table table1 #1.0.0 { column ascii column1; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName = "column1";

    MakeDatabase ( GetName(), schemaText, "root_database" );
    {
        VCursor* cursor = CreateTable ( TableName );

        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        // insert some rows
        WriteRow ( cursor, column_idx, "blah" );
        WriteRow ( cursor, column_idx, "eeee" );

        REQUIRE_RC ( VCursorCommit ( cursor ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
    REQUIRE_RC ( VDatabaseRelease ( m_db ) );

    {   // reopen
        VDBManager * mgr;
        REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        VDBManagerOpenDBRead ( mgr, (const VDatabase**)& m_db, NULL, m_databaseName . c_str () );

        const VCursor* cursor = OpenTable ( TableName );

        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        int64_t next;
        REQUIRE_RC ( VCursorFindNextRowIdDirect ( cursor, column_idx, 1, & next ) );
        REQUIRE_EQ ( (int64_t)1, next ) ;
        REQUIRE_RC ( VCursorFindNextRowIdDirect ( cursor, column_idx, 2, & next ) );
        REQUIRE_EQ ( (int64_t)2, next ) ; // VDB-3075: next == 1

        REQUIRE_RC ( VCursorRelease ( cursor ) );

        VDBManagerRelease ( mgr );
    }

}

FIXTURE_TEST_CASE ( VSchema_Version2, WVDB_Fixture )
{   // parses as v2 by default
    string schemaText = "version 2; table table1 #1.0.0 { column ascii column1; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ; "
                        "view V#1 < table1 t > {};";
    MakeDatabase ( GetName(), schemaText, "root_database" );
}

FIXTURE_TEST_CASE ( VSchema_Version2_Fail, WVDB_Fixture )
{   // tries to parse as v1
    KConfig * kfg;
    REQUIRE_RC ( KConfigMake ( & kfg, NULL ) );

    String * saved;
    const char * versionPath = "vdb/schema/version";
    REQUIRE_RC ( KConfigReadString ( kfg, versionPath, & saved ) );
    REQUIRE_RC ( KConfigWriteString( kfg, versionPath, "1" ) );

    string schemaText = "version 2; table table1 #1.0.0 { column ascii column1; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ; "
                        "view V#1 < table1 t > {};";
    REQUIRE_RC ( VDBManagerMakeUpdate ( & m_mgr, NULL ) );
    REQUIRE_RC ( VDBManagerMakeSchema ( m_mgr, & m_schema ) );

    // this is expected to log "expected 'include, typedef, typeset, fmtdef, function, schema, database or table' but found 'view'"
    REQUIRE_RC_FAIL ( VSchemaParseText ( m_schema, NULL, schemaText . c_str(), schemaText . size () ) );

    // restore version setting
    KConfigWriteSString( kfg, versionPath, saved );
    free ( saved );
}

FIXTURE_TEST_CASE ( VCursor_TypePropagation, WVDB_Fixture )
{
    string schemaText = "table table1 #1.0.0 { column ascii column1; column any col = column1; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName = "column1";

    MakeDatabase ( GetName(), schemaText, "root_database" );
    {
        VCursor* cursor = CreateTable ( TableName );

        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        // insert some rows
        WriteRow ( cursor, column_idx, "blah" );
        WriteRow ( cursor, column_idx, "eeee" );

        REQUIRE_RC ( VCursorCommit ( cursor ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
    REQUIRE_RC ( VDatabaseRelease ( m_db ) );

    {   // reopen
        VDBManager * mgr;
        REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        VDBManagerOpenDBRead ( mgr, (const VDatabase**)& m_db, NULL, m_databaseName . c_str () );

        const VCursor* cursor = OpenTable ( TableName );

        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, "col" ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        // col has the type of column1
        VCtxId id = { 0, 0, eTable };
        id . id = column_idx;
        const VColumn * vcol = VCursorGetColumn ( (VCursor*)cursor, & id );
        REQUIRE_NOT_NULL ( vcol );
        const uint32_t AsciiId = 23;
        REQUIRE_EQ ( AsciiId, vcol -> td . type_id );
        REQUIRE_EQ ( 1u, vcol -> td . dim );

        REQUIRE_RC ( VCursorRelease ( cursor ) );

        VDBManagerRelease ( mgr );
    }
}

FIXTURE_TEST_CASE ( VCursor_TypePropagation_Array, WVDB_Fixture )
{
    string schemaText = "table table1 #1.0.0 { column I64[2] column1; column any col = column1; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName = "column1";

    MakeDatabase ( GetName(), schemaText, "root_database" );
    {
        VCursor* cursor = CreateTable ( TableName );
        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );
        // insert some rows
        int64_t v[2] = {1, 2};
        WriteRow ( cursor, column_idx, v, 2 );
        REQUIRE_RC ( VCursorCommit ( cursor ) );
        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
    REQUIRE_RC ( VDatabaseRelease ( m_db ) );

    {   // reopen
        VDBManager * mgr;
        REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        VDBManagerOpenDBRead ( mgr, (const VDatabase**)& m_db, NULL, m_databaseName . c_str () );

        const VCursor* cursor = OpenTable ( TableName );

        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, "col" ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        // col has the type of column1
        VCtxId id = { 0, 0, eTable };
        id . id = column_idx;
        const VColumn * vcol = VCursorGetColumn ( (VCursor*)cursor, & id );
        REQUIRE_NOT_NULL ( vcol );
        const uint32_t I64Id = 16;
        REQUIRE_EQ ( I64Id, vcol -> td . type_id );
        REQUIRE_EQ ( 2u, vcol -> td . dim );

        REQUIRE_RC ( VCursorRelease ( cursor ) );

        VDBManagerRelease ( mgr );
    }
}

FIXTURE_TEST_CASE ( VCursor_Use_cut_ToAccessArrayElement, WVDB_Fixture )
{
    string schemaText = "function any NCBI:cut #1.0 < U32 idx, ... > ( any in ) = vdb:cut;\n"
                        "table table1 #1.0.0\n"
                        "{\n"
                        " column I64 [ 2 ] column1;\n"
                        " column I64 col = NCBI:cut < 1 > ( column1 );\n"
                        "};\n"
                        "database root_database #1 { table table1 #1 TABLE1; } ;\n";
    const char* TableName = "TABLE1";
    const char* ColumnName = "column1";

    MakeDatabase ( GetName(), schemaText, "root_database" );
    {
        VCursor* cursor = CreateTable ( TableName );
        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );
        // insert some rows
        int64_t v[2] = {1, 2};
        WriteRow ( cursor, column_idx, v, 2 );
        REQUIRE_RC ( VCursorCommit ( cursor ) );
        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
    REQUIRE_RC ( VDatabaseRelease ( m_db ) );

    {   // reopen
        VDBManager * mgr;
        REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        VDBManagerOpenDBRead ( mgr, (const VDatabase**)& m_db, NULL, m_databaseName . c_str () );

        const VCursor* cursor = OpenTable ( TableName );

        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, "col" ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        int64_t buf[2] = {0, 0};
        uint32_t row_len = 0;
        REQUIRE_RC ( VCursorReadDirect(cursor, 1, column_idx, 64, buf, 2, & row_len ) );
        REQUIRE_EQ ( 1u, row_len );
        REQUIRE_EQ((int64_t)2, buf[0]);
        REQUIRE_EQ((int64_t)0, buf[1]);

        REQUIRE_RC ( VCursorRelease ( cursor ) );

        VDBManagerRelease ( mgr );
    }
}

FIXTURE_TEST_CASE ( KDBManager_Leak, WVDB_Fixture )
{   // use valgrind to detect the leak
    string schemaText = "table table1 #1.0.0 { column ascii column1; };"
                        "database root_database #1 { table table1 #1 TABLE1; } ;";

    const char* TableName = "TABLE1";
    const char* ColumnName = "column1";

    MakeDatabase ( GetName(), schemaText, "root_database" );
    {
        VCursor* cursor = CreateTable ( TableName );
        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
    REQUIRE_RC ( VDatabaseRelease ( m_db ) );
    m_db = 0;

    {   // reopen
        VDBManager * mgr;
        REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        const VDatabase * db;
        REQUIRE_RC ( VDBManagerOpenDBRead ( mgr, & db, NULL, m_databaseName . c_str () ) );

        // opening the same table twice leaks a reference to KDBManager
        const VTable* table1;
        REQUIRE_RC ( VDatabaseOpenTableRead ( db, & table1, "%s", TableName ) );
        const VTable* table2;
        REQUIRE_RC ( VDatabaseOpenTableRead ( db, & table2, "%s", TableName ) );

        REQUIRE_RC ( VTableRelease ( table2 ) );
        REQUIRE_RC ( VTableRelease ( table1 ) );

        REQUIRE_RC ( VDatabaseRelease ( db ) );
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
