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

#include <ktst/unit_test.hpp> // TEST_CASE

#include <klib/symbol.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/view.h>
#include <vdb/blob.h>
#include <vdb/vdb-priv.h>

#include "../libs/vdb/database-priv.h"
#include "../libs/vdb/schema-priv.h"
#include "../libs/vdb/schema-parse.h"
#include "../libs/vdb/table-priv.h"
#include "../libs/vdb/cursor-priv.h"
#include "../libs/vdb/cursor-struct.h"
#include "../libs/vdb/prod-priv.h"
#include "../libs/vdb/column-priv.h"

#include "../libs/schema/SchemaParser.hpp"
#include "../libs/schema/ASTBuilder.hpp"

#include "WVDB_Fixture.hpp"

using namespace std;
using namespace ncbi :: SchemaParser;

TEST_SUITE( ViewCursorTestSuite )

static const char * SchemaText =
    "version 2.0;"
    "table T#1 { column ascii c1; };"
    "database DB#1 { table T t; };"

    "view ViewOnTable#1 < T p_tbl > "
    "{ "
    "   column ascii c = p_tbl . c1; "
    "   column ascii cc = c; "
    "};"

    "view ViewOnView#1 < ViewOnTable p_v > { column ascii c = p_v . c; };"

    "view ViewWithProduction#1 < T p_tbl > "
    "{ "
    "   ascii prod = p_tbl . c1; "
    "   column ascii c = prod; "
    "};"
    ;

static const char* TableName = "t";
static const char* TableColumnName = "c1";

static const char* ViewOnTableName = "ViewOnTable";
static const char* ViewOnViewName = "ViewOnView";
static const char* ViewWithProductionName = "ViewWithProduction";
static const char* TableParamName = "p_tbl";
static const char* ViewParamName = "p_v";
static const char* ViewColumnName = "c";

class ViewCursorFixture : public WVDB_v2_Fixture
{
public:
    ViewCursorFixture()
    :   m_schemaText ( SchemaText ),
        m_view ( 0 ),
        m_table ( 0 ),
        m_cur ( 0 ),
        m_blob ( 0 ),
        m_base ( 0 )
    {
    }
    ~ViewCursorFixture()
    {
        VBlobRelease ( (VBlob*) m_blob );
        VCursorRelease ( m_cur );
        VTableRelease ( m_table );
        VViewRelease ( m_view );
    }

    VCursor * CreateTableOpenWriteCursor( const string & p_tableName, const string & p_columnName )
    {
        VCursor * ret = CreateTable ( p_tableName . c_str () );
        THROW_ON_RC ( VCursorAddColumn ( ret, & m_columnIdx, p_columnName . c_str () ) );
        THROW_ON_RC ( VCursorOpen ( ret ) );
        return ret;
    }

    void CreateDb ( const string & p_testName )
    {
        MakeDatabase ( p_testName, m_schemaText, "DB" );
        VCursor * cursor = CreateTableOpenWriteCursor ( TableName, TableColumnName );
        // insert some rows
        WriteRow ( cursor, m_columnIdx, "blah" );
        WriteRow ( cursor, m_columnIdx, "eeee" );
        THROW_ON_RC ( VCursorCommit ( cursor ) );
        THROW_ON_RC ( VCursorRelease ( cursor ) );
    }

    uint32_t AddColumn ( const char * p_colName )
    {
        uint32_t ret;
        THROW_ON_RC ( VCursorAddColumn ( m_cur, & ret, "%s", p_colName ) );
        return ret;
    }

    string ReadAscii ( int64_t p_row, uint32_t p_colIdx )
    {
        THROW_ON_RC ( VCursorReadDirect ( m_cur, p_row, p_colIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
        return string ( m_buf, m_rowLen );
    }

    string          m_schemaText;
    const VView *   m_view;
    const VTable *  m_table;
    const VCursor * m_cur;
    uint32_t        m_columnIdx;
    const VBlob *   m_blob;
    char            m_buf [1024];
    uint32_t        m_rowLen;
    const void *    m_base;
    uint32_t        m_elemBits;
    uint32_t        m_boff;
};

class ViewOnTableCursorFixture : public ViewCursorFixture
{
public:
    void CreateCursor ( const string & p_testName, const char * p_viewName, bool p_bind = true )
    {
        CreateDb ( p_testName );

        THROW_ON_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, p_viewName ) );
        if ( p_bind )
        {
            THROW_ON_RC ( VDatabaseOpenTableRead ( m_db, & m_table, TableName ) );
            String t;
            StringInitCString ( & t, TableParamName );
            THROW_ON_RC ( VViewBindParameterTable ( m_view, & t, m_table ) );
        }
        THROW_ON_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    }

    void CreateCursorAddColumn ( const string & p_testName,
                                 const char *   p_viewName,
                                 const char *   p_colName )
    {
        CreateCursor ( p_testName, p_viewName );
        m_columnIdx = AddColumn ( p_colName );
    }

    void CreateCursorOpen ( const string & p_testName,
                            const char *   p_viewName,
                            const char *   p_colName )
    {
        CreateCursorAddColumn ( p_testName, p_viewName, p_colName );
        THROW_ON_RC ( VCursorOpen ( m_cur ) );
    }

    void CreateCursorOpenRow ( const string & p_testName,
                               const char *   p_viewName,
                               const char *   p_colName )
    {
        CreateCursorOpen ( p_testName, p_viewName, p_colName );
        THROW_ON_RC ( VCursorOpenRow ( m_cur ) );
    }

    void CreateTablePopulate ( const char *     p_tableName,
                               const char *     p_colName,
                               const string &   p_val1,
                               const string &   p_val2  )
    {
        VCursor * cur = CreateTableOpenWriteCursor( p_tableName, p_colName );
        WriteRow ( cur, m_columnIdx, p_val1 );
        WriteRow ( cur, m_columnIdx, p_val2 );
        THROW_ON_RC ( VCursorCommit ( cur ) );
        THROW_ON_RC ( VCursorRelease ( cur ) );
    }
    void CreateTablePopulate ( const char *     p_tableName,
                               const char *     p_colName,
                               int64_t   p_val1,
                               int64_t   p_val2  )
    {
        VCursor * cur = CreateTableOpenWriteCursor( p_tableName, p_colName );
        WriteRow ( cur, m_columnIdx, p_val1 );
        WriteRow ( cur, m_columnIdx, p_val2 );
        THROW_ON_RC ( VCursorCommit ( cur ) );
        THROW_ON_RC ( VCursorRelease ( cur ) );
    }

    void OpenTableBind ( const char * p_tableName, const char * p_paramName )
    {
        const VTable * t;
        THROW_ON_RC ( VDatabaseOpenTableRead ( m_db, & t, p_tableName ) );
        String n;
        StringInitCString ( & n, p_paramName );
        THROW_ON_RC ( VViewBindParameterTable ( m_view, & n, t ) );
        THROW_ON_RC ( VTableRelease ( t ) );
    }

    void ValidateBlob ( int64_t p_first, uint64_t p_count, size_t p_bytes )
    {
        THROW_ON_FALSE ( m_blob != 0 );
        int64_t first;
        uint64_t count;
        THROW_ON_RC ( VBlobIdRange ( m_blob, & first, & count ) );
        THROW_ON_FALSE ( p_first == first );
        THROW_ON_FALSE ( p_count == count );
        // blob size depends on release/debug build amd 32/64 bits, so will only check it in DEBUG/64
#if _DEBUGGING && _ARCH_BITS == 64
        size_t bytes;
        THROW_ON_RC ( VBlobSize ( m_blob, & bytes ) );
        THROW_ON_FALSE ( p_bytes == bytes );
#endif
    }
};

///////////////////////////// View-attached VCursor

// virtual function table
#include "test-view-vt.cpp"

FIXTURE_TEST_CASE( ViewCursor_Column_From_ViewVsTableWithSameId, ViewOnTableCursorFixture )
{   // when looking for columns in a view cursor, we need to distinguish between columns belonging to views (main view and paramteter views) from those belonging to (parameter) tables
    m_schemaText =
        "version 2.0;"
        "table T0#1 { column ascii c1; };" // T0 has the same id as V
        "database DB#1 { table T0 t; };"

        "view V#1 < T0 p_tbl > { column ascii col = p_tbl . c1; };"
        // we search for c1 in p_tbl (bound to t of type T0 <table> id 0, same as <view> id of V)
        // the bug this test case is exposing is in retrieving T0.c1 from column cache by its id {0, 0} which is the same as id of V.col. ctx==0 refers to different contexts, table vs view.
        // the fix is in introducing a separate column cache for view's columns
    ;

    CreateCursorOpen ( GetName(), "V", "col" );

    // read c1
    REQUIRE_EQ ( string ("blah"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("eeee"), ReadAscii ( 2, m_columnIdx ) );
}

FIXTURE_TEST_CASE( ViewCursor_ViewVsTableWithSameId_Virtual, ViewOnTableCursorFixture )
{   // when looking for overrides, we need to distinguish between the view, its's primary table and the table/view we are currently pivoted into
    m_schemaText =
        "version 2.0;"
        "table T0#1 { column ascii c2 = v1; };" // v1 is virtual; T0 has the same id as V
        "table T1#1 = T0#1{ column ascii c1; ascii v1 = c1; };" // inherits c2 from T0, resolves v1
        "database DB#1 { table T1 t; };"

        "view V#1 < T1 p_tbl > { column ascii c3 = p_tbl . c2; };"
        // we search for c2 in p_tbl (bound to t1 of type T1 which inherits c2 from T0 with <table> id 0, same as <view> id of V)
        // the bug this test case is exposing is in looking up c2 in V instead of T1, since c2's context id is 0.
    ;

    CreateCursorOpen ( GetName(), "V", "c3" );

    // read c1
    REQUIRE_EQ ( string ("blah"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("eeee"), ReadAscii ( 2, m_columnIdx ) );
}
//TODO: same with a parameter-view

//TODO: intermediate producitons ("owned", cursor-struct.h:157) - same deal or not?

FIXTURE_TEST_CASE( ViewCursor_ParametersColumnAddedToRow, ViewOnTableCursorFixture )
{   // columns referred to via view parameter are added to view cursor's row
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    Vector * row = VCursorGetRow ( ( VCursor * ) m_cur );
    REQUIRE_NOT_NULL ( row );
    REQUIRE_EQ ( 2u, VectorLength ( row ) ); // ViewOnTable.c, T.c1
    REQUIRE_EQ( string(ViewColumnName), ToCppString (((VColumn*)VectorGet(row, 1))->scol->name->name) );
    REQUIRE_EQ( string(TableColumnName), ToCppString (((VColumn*)VectorGet(row, 2))->scol->name->name) );
}

// View inheritance

FIXTURE_TEST_CASE( ViewCursor_SingleInheritance, ViewOnTableCursorFixture )
{
    m_schemaText =
        "version 2.0;"
        "table T1#1 { column ascii c1; };"
        "database DB#1 { table T1 t; };"

        "view V1#1 < T1 p_t > { column ascii c1 = p_t . c1; };"
        "view V#1 < T1 p_tbl > = V1 < p_tbl > {};" // inherits c1, p_tbl is bound to T1 via p_tbl
    ;

    CreateCursorOpen ( GetName(), "V", "c1" );

    // read c1
    REQUIRE_EQ ( string ("blah"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("eeee"), ReadAscii ( 2, m_columnIdx ) );
}

FIXTURE_TEST_CASE( ViewCursor_MultipleInheritance, ViewOnTableCursorFixture )
{
    m_schemaText =
        "version 2.0;"
        "table T1#1 { column ascii c01; };"
        "table T2#1 { column ascii c02; };"
        "database DB#1 { table T1 t1; table T2 t2; };"

        "view V1#1 < T1 p_tbl > { column ascii c1 = p_tbl . c01; };"
        "view V2#1 < T2 p_tbl > { column ascii c2 = p_tbl . c02; };"
        "view V#1 < T1 p_t1, T2 p_t2 > = V1<p_t1>, V2<p_t2>"
        "{"   // inherits c1, c2
        "};"
    ;

    {
        // create DB
        MakeDatabase ( GetName(), m_schemaText, "DB" );
        CreateTablePopulate( "t1", "c01", "t1_c_r1", "t1_c_r2" );
        CreateTablePopulate( "t2", "c02", "t2_c_r1", "t2_c_r2" );
    }

    // create V
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V" ) );
    REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    OpenTableBind ( "t1", "p_t1" );
    OpenTableBind ( "t2", "p_t2" );

    // read c1 and c2 joined on rowId
    uint32_t colIdx1 = AddColumn ( "c1" );
    uint32_t colIdx2 = AddColumn ( "c2" );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );

    REQUIRE_EQ ( string ("t1_c_r1"), ReadAscii ( 1, colIdx1 ) );
    REQUIRE_EQ ( string ("t2_c_r1"), ReadAscii ( 1, colIdx2 ) );

    REQUIRE_EQ ( string ("t1_c_r2"), ReadAscii ( 2, colIdx1 ) );
    REQUIRE_EQ ( string ("t2_c_r2"), ReadAscii ( 2, colIdx2 ) );
}

//TODO: deep inheritance

FIXTURE_TEST_CASE ( ViewCursor_FunctionCall, ViewOnTableCursorFixture )
{
    m_schemaText =
        "version 2.0;"
        "table T#1 { column ascii c1; };"
        "database DB#1 { table T t; };"
        "function ascii f(ascii v) { return v; }"
        "view V1#1 < T p_tbl > { column ascii c1 = f(p_tbl.c1); };"
    ;

    CreateCursorOpen ( GetName(), "V1", "c1" );
    REQUIRE_EQ ( string ("blah"), ReadAscii ( 1, m_columnIdx ) ); //TODO: modify the value inside f()
    REQUIRE_EQ ( string ("eeee"), ReadAscii ( 2, m_columnIdx ) );
}

FIXTURE_TEST_CASE ( ViewCursor_FunctionCall_Builtin, ViewOnTableCursorFixture )
{
    m_schemaText =
        "version 2.0;"
        "table T#1 { column ascii c1; };"
        "database DB#1 { table T t; };"
        "function < type T > T echo #1.0 < T val > ( * any row_len ) = vdb:echo;"
        "view V1#1 < T p_tbl > { column ascii c1 = <ascii>echo<\"qq\">(); };"
    ;

    CreateCursorOpen ( GetName(), "V1", "c1" );
    REQUIRE_EQ ( string ("qq"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("qq"), ReadAscii ( 2, m_columnIdx ) );
}

FIXTURE_TEST_CASE ( ViewCursor_FunctionCall_External, ViewOnTableCursorFixture )
{
    m_schemaText =
        "version 2.0;"
        "table T#1 { column U8[4] c1; column U8 one; };"
        "database DB#1 { table T t; };"
        "extern function < type T > T NCBI:SRA:swap #1 ( T in, U8 called );"
        "view V1#1 < T p_tbl > {"
        "   column U8[4] c1 = < U8 [ 4 ] > NCBI:SRA:swap ( p_tbl . c1, p_tbl . one );"
        "};"
    ;

    {
        MakeDatabase ( GetName(), m_schemaText, "DB" );
        m_cur = CreateTable ( TableName );
        uint32_t colIdx1 = AddColumn ( "c1" );
        uint32_t colIdx2 = AddColumn ( "one" );
        VCursor * cursor = (VCursor*) m_cur;
        REQUIRE_RC ( VCursorOpen ( cursor ) );

       // insert some rows
        const uint8_t c1_1[4] = {1, 2, 3, 4};
        const uint8_t c1_2[4] = {11, 22, 33, 44};
        const uint8_t One = 1;

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, colIdx1, 8, &c1_1, 0, 4 ) );
        REQUIRE_RC ( VCursorWrite ( cursor, colIdx2, 8, &One, 0, 1 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, colIdx1, 8, &c1_2, 0, 4 ) );
        REQUIRE_RC ( VCursorWrite ( cursor, colIdx2, 8, &One, 0, 1 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorCommit ( cursor ) );
        REQUIRE_RC ( VCursorRelease ( cursor ) );
        m_cur = 0;
    }

    {   // read and verify: swap(U8[2], 1) swaps the first 2 elements
        REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V1" ) );
        OpenTableBind ( TableName, TableParamName );
        REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );
        m_columnIdx = AddColumn ( "c1" );
        REQUIRE_RC ( VCursorOpen ( m_cur ) );

        uint8_t val[4] = {0, 0, 0, 0};
        REQUIRE_RC ( VCursorReadDirect ( m_cur, 1, m_columnIdx, 8, &val, 4, & m_rowLen ) );
        REQUIRE_EQ ( 2u, (uint32_t)val[0] ); // array elements 0 and 1 swapped
        REQUIRE_EQ ( 1u, (uint32_t)val[1] );
        REQUIRE_EQ ( 3u, (uint32_t)val[2] );
        REQUIRE_EQ ( 4u, (uint32_t)val[3] );

        REQUIRE_RC ( VCursorReadDirect ( m_cur, 2, m_columnIdx, 8, &val, 4, & m_rowLen ) );
        REQUIRE_EQ ( 22u, (uint32_t)val[0] ); // array elements 0 and swapped
        REQUIRE_EQ ( 11u, (uint32_t)val[1] );
        REQUIRE_EQ ( 33u, (uint32_t)val[2] );
        REQUIRE_EQ ( 44u, (uint32_t)val[3] );
    }
}

FIXTURE_TEST_CASE( ViewCursor_MemberExpr_TypePropagation, ViewOnTableCursorFixture )
{
    m_schemaText =
        "version 2.0;"
        "table T#1 { column I64[2] c; };"
        "database DB#1 { table T t; };"
        "view V#1 < T p_t > { column any col = p_t . c; };"
    ;

    {
        // create DB
        MakeDatabase ( GetName(), m_schemaText, "DB" );
        VCursor * cur = CreateTableOpenWriteCursor( "t", "c" );
        int64_t val[2] = { 1, 2 };
        WriteRow ( cur, m_columnIdx, val, 2 );
        THROW_ON_RC ( VCursorCommit ( cur ) );
        THROW_ON_RC ( VCursorRelease ( cur ) );
    }

    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V" ) );
    OpenTableBind ( "t", "p_t" );
    REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    m_columnIdx = AddColumn ( "col" );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );

    // verify: p_t.c has the type of T.c
    VCtxId id = { 0, 0, eView }; // 0th column in 0th view
    const VColumn * vcol = VCursorGetColumn ( (VCursor*)m_cur, & id );
    REQUIRE_NOT_NULL ( vcol );
    REQUIRE_NOT_NULL ( vcol -> in );
    const uint32_t I64_Id = 16;
    REQUIRE_EQ ( I64_Id, vcol -> in -> fd . td . type_id );
    REQUIRE_EQ ( 2u, vcol -> in -> fd . td . dim );
}

FIXTURE_TEST_CASE( ViewCursor_Join_Shorthand, ViewOnTableCursorFixture )
{
    m_schemaText =
        "version 2.0;"
        "table T1#1 { column I64 c01; };"
        "table T2#1 { column ascii c02; };"
        "database DB#1 { table T1 t1; table T2 t2; };"
        "view V#1 < T1 p_t1, T2 p_t2 > { column ascii c1 = p_t2 [ p_t1 . c01 ] . c02; };"
    ;

    {   // create DB
        MakeDatabase ( GetName(), m_schemaText, "DB" );
        CreateTablePopulate( "t1", "c01", 2, 1 );
        CreateTablePopulate( "t2", "c02", "t2_c_r1", "t2_c_r2" );
    }
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V" ) );
    OpenTableBind ( "t1", "p_t1" );
    OpenTableBind ( "t2", "p_t2" );
    REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    m_columnIdx = AddColumn ( "c1" );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
    // T2 rows seen through V in reverse order
    REQUIRE_EQ ( string ("t2_c_r2"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("t2_c_r1"), ReadAscii ( 2, m_columnIdx ) );
}

FIXTURE_TEST_CASE( ViewCursor_Join_Shorthand_Production, ViewOnTableCursorFixture )
{
    m_schemaText =
        "version 2.0;"
        "table T1#1 { column I64 c01; };"
        "table T2#1 { column ascii c02; ascii p = c02; };" // p is a production ...
        "database DB#1 { table T1 t1; table T2 t2; };"
        "view V#1 < T1 p_t1, T2 p_t2 > { column ascii c1 = p_t2 [ p_t1 . c01 ] . p; };" // ... used here
    ;

    {   // create DB
        MakeDatabase ( GetName(), m_schemaText, "DB" );
        CreateTablePopulate( "t1", "c01", 2, 1 );
        CreateTablePopulate( "t2", "c02", "t2_c_r1", "t2_c_r2" );
    }
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V" ) );
    OpenTableBind ( "t1", "p_t1" );
    OpenTableBind ( "t2", "p_t2" );
    REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    m_columnIdx = AddColumn ( "c1" );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
    // T2 rows seen through V in reverse order
    REQUIRE_EQ ( string ("t2_c_r2"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("t2_c_r1"), ReadAscii ( 2, m_columnIdx ) );
}

//TODO: join with a non-int key (error)
//TODO: join with a non-I64 integer key (cast required?)

//TODO: external function call in a view parameter's productions

#include "test-view-on-view-cursor.cpp"

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

const char UsageDefaultName[] = "test-view-cursor";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=ViewCursorTestSuite(argc, argv);
    return rc;
}

}
