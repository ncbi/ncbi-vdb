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

#include <sysalloc.h>
#include <cstdlib>
#include <stdexcept>

#include <vdb/table.h>
#include <vdb/cursor.h>


using namespace std;

TEST_SUITE( ByteCodeTestSuite )

const string ScratchDir = "./db/";
static const char* TableName = "TABLE1";

class ByteCodeFixture : public WVDB_Fixture
{
public:

    void CreateDb ( const char * p_dbName, const char * p_schema, const string & p_colName1, const string & p_colName2 = string() )
    {
        m_databaseName = ScratchDir + p_dbName;
        MakeDatabase ( p_schema, "root_database" );

        THROW_ON_RC ( VDatabaseCreateTable ( m_db , & m_table, TableName, kcmInit + kcmMD5, TableName ) );
        THROW_ON_RC ( VTableCreateCursorWrite ( m_table, & m_cursor, kcmInsert ) );
        THROW_ON_RC ( VCursorAddColumn ( m_cursor, & m_column_idx1, p_colName1 . c_str () ) );
        if ( ! p_colName2 . empty () )
        {
            THROW_ON_RC ( VCursorAddColumn ( m_cursor, & m_column_idx2, p_colName2 . c_str () ) );
        }
        THROW_ON_RC ( VCursorOpen ( m_cursor ) );
    }

    void Append ( uint32_t p_columnIdx, const string & p_value )
    {
        THROW_ON_RC ( VCursorOpenRow ( m_cursor ) );
        THROW_ON_RC ( VCursorWrite ( m_cursor, p_columnIdx, 8, p_value . c_str (), 0, p_value . length () ) );
        THROW_ON_RC ( VCursorCommitRow ( m_cursor ) );
        THROW_ON_RC ( VCursorCloseRow ( m_cursor ) );
    }

    template < typename T > void Append ( uint32_t p_columnIdx, T p_value )
    {
        THROW_ON_RC ( VCursorOpenRow ( m_cursor ) );
        THROW_ON_RC ( VCursorWrite ( m_cursor, p_columnIdx, 8 * sizeof ( p_value ), & p_value, 0, 1 ) );
        THROW_ON_RC ( VCursorCommitRow ( m_cursor ) );
        THROW_ON_RC ( VCursorCloseRow ( m_cursor ) );
    }
    template < typename T, size_t Dim > void Append ( uint32_t p_columnIdx, T * p_value )
    {
        THROW_ON_RC ( VCursorOpenRow ( m_cursor ) );
        THROW_ON_RC ( VCursorWrite ( m_cursor, p_columnIdx, 8 * sizeof ( T ), p_value, 0, Dim ) );
        THROW_ON_RC ( VCursorCommitRow ( m_cursor ) );
        THROW_ON_RC ( VCursorCloseRow ( m_cursor ) );
    }

    void Append ( uint32_t p_columnIdx1, const string & p_value1, uint32_t p_columnIdx2, const string & p_value2 )
    {
        THROW_ON_RC ( VCursorOpenRow ( m_cursor ) );
        THROW_ON_RC ( VCursorWrite ( m_cursor, p_columnIdx1, 8, p_value1 . c_str (), 0, p_value1 . length () ) );
        THROW_ON_RC ( VCursorWrite ( m_cursor, p_columnIdx2, 8, p_value2 . c_str (), 0, p_value2 . length () ) );
        THROW_ON_RC ( VCursorCommitRow ( m_cursor ) );
        THROW_ON_RC ( VCursorCloseRow ( m_cursor ) );
    }

    void CommitDb()
    {
        THROW_ON_RC ( VCursorCommit ( m_cursor ) );
        CloseDb();
    }

    void OpenDb()
    {
        VDBManager * mgr;
        THROW_ON_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        THROW_ON_RC ( VDBManagerOpenDBRead ( mgr, (const VDatabase**)& m_db, NULL, m_databaseName . c_str () ) );
        THROW_ON_RC ( VDatabaseOpenTableRead ( m_db , (const VTable**)& m_table, TableName ) );
        THROW_ON_RC ( VTableCreateCursorRead ( m_table, (const VCursor**)& m_cursor ) );
        THROW_ON_RC ( VDBManagerRelease ( mgr ) );
    }
    void CloseDb()
    {
        THROW_ON_RC ( VCursorRelease ( m_cursor ) );
        THROW_ON_RC ( VTableRelease ( m_table ) );
        THROW_ON_RC ( VDatabaseRelease ( m_db ) );
        m_db = 0;
    }

    uint32_t AddColumn ( const char * p_name, bool p_openCursor = true )
    {
        uint32_t ret;
        THROW_ON_RC ( VCursorAddColumn ( m_cursor, & ret, p_name ) );
        if ( p_openCursor )
        {
            THROW_ON_RC ( VCursorOpen ( m_cursor ) );
        }
        return ret;
    }

    void VerifyDb ( const char * p_colName, const string & p_value1, const string & p_value2 = string() )
    {
        OpenDb();
        uint32_t columnIdx = AddColumn ( p_colName );
        char buffer[1024];
        uint32_t row_len;
        THROW_ON_RC ( VCursorReadDirect ( m_cursor, 1, columnIdx, 8, buffer, sizeof ( buffer ), & row_len ) );
        if ( p_value1 . length () != row_len )
        {
            throw logic_error ( "ByteCodeFixture::VerifyDb: incorrect row_len[1]");
        }
        if ( p_value1 != string ( buffer, row_len ) )
        {
            throw logic_error ( string ( "ByteCodeFixture::VerifyDb: incorrect value[1]='" ) + string ( buffer, row_len ) + "'");
        }

        if ( ! p_value2 . empty () )
        {
            THROW_ON_RC ( VCursorReadDirect ( m_cursor, 2, columnIdx, 8, buffer, sizeof ( buffer ), & row_len ) );
            if ( p_value2 . length () != row_len )
            {
                throw logic_error ( "ByteCodeFixture::VerifyDb: incorrect row_len[2]");
            }
            if ( p_value2 != string ( buffer, row_len ) )
            {
                throw logic_error ( string ( "ByteCodeFixture::VerifyDb: incorrect value[2]='" ) + string ( buffer, row_len ) + "'");
            }
        }

        CloseDb();
    }

    template < typename T > void VerifyDb ( const char * p_colName, T p_value1, T p_value2 )
    {
        OpenDb();
        uint32_t columnIdx = AddColumn ( p_colName );

        T value;
        uint32_t row_len;
        THROW_ON_RC ( VCursorReadDirect ( m_cursor, 1, columnIdx, 8 * sizeof ( T ), & value, 1, & row_len ) );
        if ( 1 != row_len )
        {
            throw logic_error ( "ByteCodeFixture::VerifyDb: incorrect row_len[1]");
        }
        if ( p_value1 != value )
        {
            throw logic_error ( string ( "ByteCodeFixture::VerifyDb: incorrect value[1]" ) );
        }

        THROW_ON_RC ( VCursorReadDirect ( m_cursor, 2, columnIdx, 8 * sizeof ( T ), & value, 1, & row_len ) );
        if ( 1 != row_len )
        {
            throw logic_error ( "ByteCodeFixture::VerifyDb: incorrect row_len[2]");
        }
        if ( p_value2 != value )
        {
            throw logic_error ( string ( "ByteCodeFixture::VerifyDb: incorrect value[2]" ) );
        }

        CloseDb();
    }

    template < typename T > void VerifyValue ( uint32_t p_columnIdx, int64_t p_rowId, T p_value )
    {
        T value;
        uint32_t row_len;
        THROW_ON_RC ( VCursorReadDirect ( m_cursor, p_rowId, p_columnIdx, 8 * sizeof ( T ), & value, 1, & row_len ) );
        if ( 1 != row_len )
        {
            throw logic_error ( "ByteCodeFixture::VerifyValue: incorrect row_len");
        }
        if ( p_value != value )
        {
            throw logic_error ( string ( "ByteCodeFixture::VerifyValue: incorrect value" ) );
        }
    }

    template < typename T, size_t Dim > void ReadArray ( uint32_t p_columnIdx, int64_t p_rowId, T p_value[] )
    {
        uint32_t row_len;
        THROW_ON_RC ( VCursorReadDirect ( m_cursor, p_rowId, p_columnIdx, 8 * sizeof ( T ), p_value, Dim, & row_len ) );
        if ( Dim != row_len )
        {
            throw logic_error ( "ByteCodeFixture::ReadArray: incorrect row_len");
        }
    }

    VTable *    m_table;
    VCursor *   m_cursor;
    uint32_t    m_column_idx1;
    uint32_t    m_column_idx2;
    uint32_t    m_row_len;
};

/////////////////////////////////////////////////////// Test cases

FIXTURE_TEST_CASE ( SimpleColumn, ByteCodeFixture )
{   // operations covered:
    // VProductionReadBlob
    // bcPre_ProductionReadBlob
    // bcPost_ProductionReadBlob
    // VSimpleProdRead
    // Post_SimpleProdRead
    // VFunctionProdRead
    // VFunctionProdReadNormal
    // bcPre_FunctionProdReadNormal
    // bcPost_FunctionProdReadNormal
    // VPhysicalProdRead
    // bcColumnProdRead
    // bcSimpleProdPage2Blob
    // bcSimpleProdSerial2Blob
    // bcSimpleProdBlob2Serial
    // bcFunctionProdCallCompare
    // bcFunctionProdCallByteswap
    // bcWPhysicalRead
    // bcPhysicalReadKColumn
    // bcWColumnReadBlob
    // bcPhysicalReadBlob
    // bcPhysicalWrite
    // bcPre_PhysicalWrite
    // bcPost_PhysicalWrite
    // bcPhysicalWriteKColumn
    CreateDb ( GetName (),
                "table table1 #1.0.0 { column ascii column1; };"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
              "column1" );

    // need to insert 2 rows with different values to make the column physical
    const string v1 = "baaa";
    const string v2 = "eee";
    Append ( m_column_idx1, v1 );
    Append ( m_column_idx1, v2 );
    CommitDb();

    VerifyDb ( "column1", v1, v2 );
}

FIXTURE_TEST_CASE ( FunctionProdCallRowFunc, ByteCodeFixture )
{   // bcFunctionProdCallRowFunc
    CreateDb ( GetName (),
                "function < type T > T echo #1.0 < T val > ( * any row_len ) = vdb:echo; "
                "table table1 #1.0.0 { "
                "   column ascii column1 = < ascii > echo < \"baabaa\" > (); "
                "}"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
                "column1"
                );
    CommitDb();

    VerifyDb ( "column1", "baabaa" );
}

FIXTURE_TEST_CASE ( FunctionProdSelect, ByteCodeFixture )
{   // bcFunctionProdSelect
    CreateDb ( GetName (),
                "function < type T > T select #1.0 ( T first, T second, ... ) = vdb:select; "
                "table table1 #1.0.0 { "
                "   column ascii column1; "
                "   column ascii column2; "
                "   column ascii column3 = < ascii > select ( column1, column2 ); "
                "}"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
                "column1",
                "column2"
                );

    const string v1_1 = "baaa";
    const string v1_2 = "eee";
    Append ( m_column_idx1, v1_1,  m_column_idx2, "a" );
    Append ( m_column_idx1, v1_2,  m_column_idx2, "bb" );
    CommitDb();

    VerifyDb ( "column3", v1_1, v1_2 );
}

FIXTURE_TEST_CASE ( ScriptProdRead, ByteCodeFixture )
{   // bcScriptProdRead
    CreateDb ( GetName (),
                "function ascii Script #1 ( ascii p ) { return p; } "
                "table table1 #1.0.0 { "
                "   column ascii column1; "
                "   column ascii column2 = Script ( column1 ); "
                "}"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
                "column1"
                );

    const string v1_1 = "baaa";
    const string v1_2 = "eee";
    Append ( m_column_idx1, v1_1 );
    Append ( m_column_idx1, v1_2 );
    CommitDb();

    VerifyDb ( "column2", v1_1, v1_2 );
}

FIXTURE_TEST_CASE ( FunctionProdCallNDRowFunc, ByteCodeFixture )
{   // bcFunctionProdCallNDRowFunc
    CreateDb ( GetName (),
                "function I64 rowId #1 () = vdb:row_id;"
                "table table1 #1.0.0 { "
                "   column ascii column1; "
                "   column I64 column2 = rowId(); "
                "}"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
                "column1"
                );

    const string v1_1 = "baaa";
    const string v1_2 = "eee";
    Append ( m_column_idx1, v1_1 );
    Append ( m_column_idx1, v1_2 );
    CommitDb();

    VerifyDb <int64_t> ( "column2", 1, 2 );
}

FIXTURE_TEST_CASE ( FunctionProdCallBlobNFunc, ByteCodeFixture )
{   // bcFunctionProdCallBlobNFunc
    CreateDb ( GetName (),
                "function any cast #1.0 (any in) = vdb:cast;"
                "typedef U8 Array[2];"
                "table table1 #1.0.0 { "
                "   column U8[2] column1; "
                "   column Array column2 = cast(column1);"
                "}"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
                "column1"
                );

    uint8_t arr1 [ 2 ] = { 2, 3 };
    uint8_t arr2 [ 2 ] = { 4, 5 };
    Append < uint8_t, 2 > ( m_column_idx1, arr1 );
    Append < uint8_t, 2 > ( m_column_idx1, arr2 );
    CommitDb();

    OpenDb();
    m_column_idx1 = AddColumn ( "column2" );

    {
        uint8_t value[2]={0,0};
        ReadArray < uint8_t, 2 > ( m_column_idx1, 1, value );
        REQUIRE_EQ ( arr1[0], value[0] );
        REQUIRE_EQ ( arr1[1], value[1] );

        ReadArray < uint8_t, 2 > ( m_column_idx1, 2, value );
        REQUIRE_EQ ( arr2[0], value[0] );
        REQUIRE_EQ ( arr2[1], value[1] );
    }

    CloseDb();
}

FIXTURE_TEST_CASE ( FunctionProdCallArrayFunc, ByteCodeFixture )
{   // bcFunctionProdCallArrayFunc
    CreateDb ( GetName (),
                "function any cast #1.0 (any in) = vdb:cast;"
                "typedef U8 Array[2];"
                "table table1 #1.0.0 { "
                "   column U16[2] column1; "
                "   column Array column2 = cast(column1);"
                "}"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
                "column1"
                );

    uint16_t arr1 [ 2 ] = { 2, 3 };
    uint16_t arr2 [ 2 ] = { 4, 5 };
    Append < uint16_t, 2 > ( m_column_idx1, arr1 );
    Append < uint16_t, 2 > ( m_column_idx1, arr2 );
    CommitDb();

    OpenDb();
    m_column_idx1 = AddColumn ( "column2" );

    {
        uint8_t value[2]={0,0};
        ReadArray < uint8_t, 2 > ( m_column_idx1, 1, value );
        REQUIRE_EQ ( (uint8_t)arr1[0], value[0] );
        REQUIRE_EQ ( (uint8_t)arr1[1], value[1] );

        ReadArray < uint8_t, 2 > ( m_column_idx1, 2, value );
        REQUIRE_EQ ( (uint8_t)arr2[0], value[0] );
        REQUIRE_EQ ( (uint8_t)arr2[1], value[1] );
    }

    CloseDb();
}

FIXTURE_TEST_CASE ( FunctionProdCallPageFunc, ByteCodeFixture )
{   // bcFunctionProdCallPageFunc
    CreateDb ( GetName (),
                "function < type T > T min #1.0 ( T a, T b ) = vdb:min;"
                "table table1 #1.0.0 { "
                "   column U8 column1; "
                "   column U8 column2 = < U8 > min ( column1, column1 );"
                "}"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
                "column1"
                );

    uint8_t v1 = 1;
    uint8_t v2 = 2;
    Append ( m_column_idx1, v1 );
    Append ( m_column_idx1, v2 );
    CommitDb();

    VerifyDb ( "column2", v1, v2 );
}

FIXTURE_TEST_CASE ( FunctionProdCallBlobFunc, ByteCodeFixture )
{
    // bcPre_FunctionProdCallBlobFunc
    // bcFunctionProdCallBlobFunc
    // bcPost_FunctionProdCallBlobFunc
    // bcFunctionProdCallBlobFuncEncoding
    // bcFunctionProdCallBlobFuncDecoding
    CreateDb ( GetName (),
                "typeset integer_set { I8, U8, I16, U16, I32, U32, I64, U64 };"
                "typeset izip_set { integer_set };"
                "fmtdef izip_fmt;"
                "function izip_fmt izip #2.1 ( izip_set in ) = vdb:izip;"
                "function izip_set iunzip #2.1 ( izip_fmt in ) = vdb:iunzip;"
                "physical < type T > T izip_encoding #1.0 { decode { return ( T ) iunzip ( @ ); } encode { return izip ( @ ); } };"
                "table table1 #1.0.0 { "
                "   column < U8 > izip_encoding column1; "
                "}"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
                "column1"
                );

    uint8_t v1 = 1;
    uint8_t v2 = 2;
    Append ( m_column_idx1, v1 );
    Append ( m_column_idx1, v2 );
    CommitDb();

    VerifyDb ( "column1", v1, v2 );
}

FIXTURE_TEST_CASE ( PhysicalConvertStatic, ByteCodeFixture )
{
    // bcPre_PhysicalConvertStatic
    // bcPhysicalConvertStatic
    // bcPost_PhysicalConvertStatic
    // bcPhysicalReadStatic
    CreateDb ( GetName (),
                "table table1 #1.0.0 { column ascii column1; };"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
              "column1" );

    // need to insert 2 rows with different values to make the column physical
    const string v1 = "baaa";
    const string v2 = "eee";
    Append ( m_column_idx1, v1 );
    REQUIRE_RC ( VCursorCommit ( m_cursor ) ); // creates a static row

    Append ( m_column_idx1, v2 );
    CommitDb(); // converts static into non-static

    VerifyDb ( "column1", v1, v2 );
}

#ifdef SHOW_UNIMPLEMENTED
FIXTURE_TEST_CASE ( FunctionProdCallLegacyBlobFunc, ByteCodeFixture )
{   // bcFunctionProdCallLegacyBlobFunc - the support for this is likely to go away
    CreateDb ( GetName (),
                "fmtdef NCBI:zlib_encoded_t;"
                "extern function any NCBI:unzip #1.0 ( NCBI:zlib_encoded_t in );"
                "extern function NCBI:zlib_encoded_t NCBI:zlib_compress #1.0 ( any in );"
                "table table1 #1.0.0 { "
                "   column U32 column1; "
                "   column U64 column2 = NCBI:zlib_compress ( column1 );"
                "}"
                "database root_database #1 { table table1 #1 TABLE1; } ;",
                "column1"
                );
    uint32_t v1 = 1;
    uint32_t v2 = 2;
    Append ( m_column_idx1, v1 );
    Append ( m_column_idx1, v2 );
    CommitDb();

    OpenDb();
    m_column_idx1 = AddColumn ( "column2" );

    VerifyValue ( m_column_idx1, 1, v1 );
    VerifyValue ( m_column_idx1, 2, v2 );

    CloseDb();
}
#endif

//TODO:
// bcFunctionProdCallCompare1 - suggest combining with VFunctionProdCallCompare

// bcFunctionProdCallLegacyBlobFunc -- to be removed (see above)

// bcPhysicalRead = bcPhysicalReadBlob (read side only)

// bcReturn - remove ?

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
