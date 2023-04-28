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

#include <klib/symbol.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/view.h>
#include <vdb/blob.h>

#include "../libs/vdb/database-priv.h"
#include "../libs/vdb/schema-priv.h"
#include "../libs/vdb/view-priv.h"
#include "../libs/vdb/table-priv.h"

#include <ktst/unit_test.hpp> // TEST_CASE

using namespace std;
using namespace ncbi :: SchemaParser;

TEST_SUITE( ViewTestSuite )

static string BaseSchemaText =
    "version 2.0;"
    "table T#1 { column ascii c1; };"
    "table P#1 { column ascii c1; };"
    "database DB#1 { table T t; table P p; };"
    "view V#1 < T tbl > {};"
    ;

static const char* TableName_1 = "t";
static const char* TableName_2 = "p";
static const char* TableColumnName = "c1";
static const char* TableParamName = "tbl";

class ViewFixture : public WVDB_v2_Fixture
{
public:
    ViewFixture()
    :   m_schemaText ( BaseSchemaText ),
        m_view ( 0 ),
        m_table ( 0 ),
        m_cur ( 0 ),
        m_blob ( 0 ),
        m_base ( 0 )
    {
    }
    ~ViewFixture()
    {
        VBlobRelease ( m_blob );
        VCursorRelease ( m_cur );
        VTableRelease ( m_table );
        VViewRelease ( m_view );
    }

    void CreateDb ( const string & p_testName )
    {
        MakeDatabase ( p_testName, m_schemaText, "DB" );

        {
            VCursor* cursor = CreateTable ( TableName_1 );

            uint32_t column_idx;
            THROW_ON_RC ( VCursorAddColumn ( cursor, & column_idx, TableColumnName ) );
            THROW_ON_RC ( VCursorOpen ( cursor ) );

            // insert some rows
            WriteRow ( cursor, column_idx, "blah" );
            WriteRow ( cursor, column_idx, "eeee" );

            THROW_ON_RC ( VCursorCommit ( cursor ) );
            THROW_ON_RC ( VCursorRelease ( cursor ) );
        }
        {
            VCursor* cursor = CreateTable ( TableName_2 );

            uint32_t column_idx;
            THROW_ON_RC ( VCursorAddColumn ( cursor, & column_idx, TableColumnName ) );
            THROW_ON_RC ( VCursorOpen ( cursor ) );

            // insert some rows
            WriteRow ( cursor, column_idx, "123" );
            WriteRow ( cursor, column_idx, "456789" );

            THROW_ON_RC ( VCursorCommit ( cursor ) );
            THROW_ON_RC ( VCursorRelease ( cursor ) );
        }
    }

    void CreateView ( const string & p_testName, const char * p_viewName )
    {
        CreateDb ( p_testName );
        THROW_ON_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, p_viewName ) );
    }

    void BindTable ( const char * p_paramName, const char * p_tableName )
    {
        String param;
        StringInitCString ( & param, p_paramName );
        const VTable * table;
        THROW_ON_RC ( VDatabaseOpenTableRead ( m_db, & table, p_tableName ) );
        THROW_ON_RC ( VViewBindParameterTable ( m_view, & param, table ) );
        VTableRelease ( table );
    }

    void CreateAndBindView ( const string & p_testName, const char * p_viewName )
    {
        CreateView ( p_testName, p_viewName );
        BindTable ( TableParamName, TableName_1 );
    }

    void CreateCursor ( const string & p_testName, const char * p_viewName )
    {
        CreateAndBindView ( p_testName, p_viewName );
        THROW_ON_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    }

    void CreateCursorAddColumn ( const string & p_testName,
                                 const char *   p_viewName,
                                 const char *   p_colName )
    {
        CreateCursor ( p_testName, p_viewName );
        THROW_ON_RC ( VCursorAddColumn ( m_cur, & m_colIdx, "%s", p_colName ) );
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

    void ValidateBlob ( int64_t p_first, uint64_t p_count, size_t p_bytes )
    {
        THROW_ON_FALSE ( m_blob != 0 );
        int64_t first;
        uint64_t count;
        THROW_ON_RC ( VBlobIdRange ( m_blob, & first, & count ) );
        THROW_ON_FALSE ( p_first == first );
        THROW_ON_FALSE ( p_count == count );
        size_t bytes;
        THROW_ON_RC ( VBlobSize ( m_blob, & bytes ) );
        THROW_ON_FALSE ( p_bytes == bytes );
    }

    string          m_schemaText;
    const VView *   m_view;
    const VTable *  m_table;
    const VCursor * m_cur;
    uint32_t        m_colIdx;
    const VBlob *   m_blob;
    char            m_buf [1024];
    uint32_t        m_rowLen;
    const void *    m_base;
    uint32_t        m_elemBits;
    uint32_t        m_boff;
};

// VDBManagerOpenView

FIXTURE_TEST_CASE ( OpenView_BadSelf, ViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( 0, & m_view, m_schema, "V" ) );
}

FIXTURE_TEST_CASE ( OpenView_BadView, ViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( m_mgr, 0, m_schema, "V" ) );
}

FIXTURE_TEST_CASE ( OpenView_BadSchema, ViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( m_mgr, & m_view, 0, "V" ) );
}

FIXTURE_TEST_CASE ( OpenView_BadName, ViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "zzz" ) );
}

FIXTURE_TEST_CASE ( OpenView, ViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V" ) );
}

//TODO: OpenView with the latest version
//TODO: OpenView with an old version
//TODO: OpenView with a non-existent version

// AddRef/Release

FIXTURE_TEST_CASE ( View_Refcount_AddRef_NullObject, ViewFixture )
{
    REQUIRE_RC ( VViewAddRef ( 0 ) );
}
FIXTURE_TEST_CASE ( View_Refcount_Release_NullObject, ViewFixture )
{
    REQUIRE_RC ( VViewRelease ( 0 ) );
}
FIXTURE_TEST_CASE ( View_Refcount_Add_Release, ViewFixture )
{
    CreateView ( GetName(), "V" );
    REQUIRE_RC ( VViewAddRef ( m_view ) );
    REQUIRE_RC ( VViewRelease ( m_view ) );
    REQUIRE_RC ( VViewRelease ( m_view ) );
    m_view = 0;
}

// BindParameterTable

FIXTURE_TEST_CASE ( View_BindParameterTable_NullObject, ViewFixture )
{
    CreateView ( GetName(), "V" );
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & m_table, TableName_1 ) );
    String t;
    CONST_STRING ( & t, "" );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( 0, & t, m_table ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable_NullName, ViewFixture )
{
    CreateView ( GetName(), "V" );
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & m_table, TableName_1 ) );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, 0, m_table ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable_NullTable, ViewFixture )
{
    CreateView ( GetName(), "V" );
    String t;
    StringInitCString ( & t, TableParamName );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, & t, 0 ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable_WrongName, ViewFixture )
{
    CreateView ( GetName(), "V" );
    String t;
    CONST_STRING ( & t, "notAtbl" );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, & t, 0 ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable_WrongTableType, ViewFixture )
{
    CreateView ( GetName(), "V" );

    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & m_table, TableName_2 ) ); // wrong type
    String t;
    StringInitCString ( & t, TableParamName );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, & t, m_table ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable, ViewFixture )
{
    CreateView ( GetName(), "V" );
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & m_table, TableName_1 ) );
    String t;
    StringInitCString ( & t, TableParamName );
    REQUIRE_RC ( VViewBindParameterTable ( m_view, & t, m_table ) ); // V<T tbl>
}

FIXTURE_TEST_CASE ( View_BindParameterTable_AlreadyBound, ViewFixture )
{
    CreateView ( GetName(), "V" );
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & m_table, TableName_1 ) );
    String t;
    StringInitCString ( & t, TableParamName );
    REQUIRE_RC ( VViewBindParameterTable ( m_view, & t, m_table ) ); // V<T tbl>
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, & t, m_table ) ); // already done
}

FIXTURE_TEST_CASE ( View_BindParameterTable_Derived, ViewFixture )
{
    m_schemaText =
        "version 2.0;"
        "table T#1 { column ascii c1; };"
        "table P#1 = T#1 { column ascii c2; };"
        "database DB#1 { table T t; table P p; };"
        "view V#1 < T tbl > {};";   // should accept P since it is derived from T

    CreateView ( GetName(), "V" );
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & m_table, "p" ) );
    String t;
    StringInitCString ( & t, TableParamName );
    REQUIRE_RC ( VViewBindParameterTable ( m_view, & t, m_table ) );
}

// BindParameterView

const string ViewOnView = BaseSchemaText + "view W#1 < V vw > {};";

FIXTURE_TEST_CASE ( View_BindParameterView_NullObject, ViewFixture )
{
    m_schemaText = ViewOnView;
    CreateView ( GetName(), "W" );
    const VView * v;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & v, m_schema, "V" ) );
    String t;
    CONST_STRING ( & t, "vw" );
    REQUIRE_RC_FAIL ( VViewBindParameterView ( 0, & t, v ) );
    REQUIRE_RC ( VViewRelease ( v ) );
}
FIXTURE_TEST_CASE ( View_BindParameterView_NullName, ViewFixture )
{
    m_schemaText = ViewOnView;
    CreateView ( GetName(), "W" );
    const VView * v;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & v, m_schema, "V" ) );
    REQUIRE_RC_FAIL ( VViewBindParameterView ( m_view, 0, v ) );
    REQUIRE_RC ( VViewRelease ( v ) );
}
FIXTURE_TEST_CASE ( View_BindParameterView_NullView, ViewFixture )
{
    m_schemaText = ViewOnView;
    CreateView ( GetName(), "W" );
    String t;
    CONST_STRING ( & t, "vw" );
    REQUIRE_RC_FAIL ( VViewBindParameterView ( m_view, & t, 0 ) );
}
FIXTURE_TEST_CASE ( View_BindParameterView_WrongName, ViewFixture )
{
    m_schemaText = ViewOnView;
    CreateView ( GetName(), "W" );
    const VView * v;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & v, m_schema, "V" ) );
    String t;
    CONST_STRING ( & t, "notAview" );
    REQUIRE_RC_FAIL ( VViewBindParameterView ( m_view, & t, v ) );
    REQUIRE_RC ( VViewRelease ( v ) );
}
FIXTURE_TEST_CASE ( View_BindParameterView_WrongViewType, ViewFixture )
{
    m_schemaText = ViewOnView;
    CreateView ( GetName(), "W" );
    const VView * v;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & v, m_schema, "W" ) ); // W<W> - wrong
    String t;
    CONST_STRING ( & t, "vw" );
    REQUIRE_RC_FAIL ( VViewBindParameterView ( m_view, & t, v ) );
    REQUIRE_RC ( VViewRelease ( v ) );
}
FIXTURE_TEST_CASE ( View_BindParameterView, ViewFixture )
{
    m_schemaText = ViewOnView;
    CreateView ( GetName(), "W" );
    const VView * v;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & v, m_schema, "V" ) );
    String t;
    CONST_STRING ( & t, "vw" );
    REQUIRE_RC ( VViewBindParameterView ( m_view, & t, v ) ); // W<V>
    REQUIRE_RC ( VViewRelease ( v ) );
}
FIXTURE_TEST_CASE ( View_BindParameterView_AlreadyBound, ViewFixture )
{
    m_schemaText = ViewOnView;
    CreateView ( GetName(), "W" );
    const VView * v;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & v, m_schema, "V" ) );
    String t;
    CONST_STRING ( & t, "vw" );
    REQUIRE_RC ( VViewBindParameterView ( m_view, & t, v ) ); // W<V>
    REQUIRE_RC_FAIL ( VViewBindParameterView ( m_view, & t, v ) ); // already done
    REQUIRE_RC ( VViewRelease ( v ) );
}

FIXTURE_TEST_CASE ( View_BindParameterView_Derived, ViewFixture )
{
    m_schemaText =
        "version 2.0;"
        "table T#1 { column ascii c1; };"
        "table P#1 = T#1 { column ascii c2; };"
        "database DB#1 { table T t; table P p; };"
        "view W#1 < T tbl > {};"
        "view WW#1 < T tbl > = W < tbl > {};"
        "view V#1 < W vw > {};";   // should accept WW since it is derived from W

    CreateView ( GetName(), "V" );
    const VView * v;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & v, m_schema, "WW" ) );
    String t;
    CONST_STRING ( & t, "vw" );
    REQUIRE_RC ( VViewBindParameterView ( m_view, & t, v ) ); // V<WW>
    REQUIRE_RC ( VViewRelease ( v ) );
}

// VViewCreateCursor

FIXTURE_TEST_CASE ( View_CreateCursor_SelfNull, ViewFixture )
{
    CreateAndBindView ( GetName(), "V" );
    const VCursor * curs;
    REQUIRE_RC_FAIL ( VViewCreateCursor ( 0, & curs ) );
}

FIXTURE_TEST_CASE ( View_CreateCursor_CursorNull, ViewFixture )
{
    CreateAndBindView ( GetName(), "V" );
    REQUIRE_RC_FAIL ( VViewCreateCursor ( m_view, 0 ) );
}

FIXTURE_TEST_CASE ( View_CreateCursor_Release, ViewFixture )
{
    CreateAndBindView ( GetName(), "V" );
    const VCursor * curs;
    REQUIRE_RC ( VViewCreateCursor ( m_view, & curs ) );
    REQUIRE_NOT_NULL ( curs );
    REQUIRE_RC ( VCursorRelease ( curs ) );
}

// PrimaryTable
FIXTURE_TEST_CASE ( View_PrimaryTable, ViewFixture )
{
    CreateAndBindView ( GetName(), "V" );
    const VTable * t = VViewPrimaryTable ( m_view );
    REQUIRE_NOT_NULL ( t );
    REQUIRE_EQ ( string ( "T" ), ToCppString ( t -> stbl -> name -> name ) );
}

FIXTURE_TEST_CASE ( View_PrimaryTable_ThroughView, ViewFixture )
{
    m_schemaText = ViewOnView;

    CreateAndBindView ( GetName(), "V" );

    const VView * viewOnView;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & viewOnView, m_schema, "W" ) );
    String str;
    CONST_STRING ( & str, "vw" );
    REQUIRE_RC ( VViewBindParameterView ( viewOnView, & str, m_view ) );

    // V<T>; W<V>; the primary table of both views is a T
    const VTable * t = VViewPrimaryTable ( viewOnView );
    REQUIRE_NOT_NULL ( t );
    REQUIRE_EQ ( string ( "T" ), ToCppString ( t -> stbl -> name -> name ) );

    REQUIRE_RC ( VViewRelease ( viewOnView ) );
}

// GetBoundObject

FIXTURE_TEST_CASE ( View_GetBoundObject_TopView, ViewFixture )
{
    CreateAndBindView ( GetName(), "V" );
    const VTable * t = static_cast < const VTable * > ( VViewGetBoundObject ( m_view, m_view -> sview, 0 ) );
    REQUIRE_NOT_NULL ( t );
    REQUIRE_EQ ( string ( "T" ), ToCppString ( t -> stbl -> name -> name ) );
}

FIXTURE_TEST_CASE ( View_GetBoundObject_ParentView, ViewFixture )
{
    m_schemaText = BaseSchemaText + "view X#1 < T tbl > = V < tbl > {};";
    CreateView ( GetName(), "X" );
    BindTable ( "tbl", "t" );

    // first argument of X is bound to T
    const VTable * t = static_cast < const VTable * > ( VViewGetBoundObject ( m_view, m_view -> sview, 0 ) );
    REQUIRE_NOT_NULL ( t );
    REQUIRE_EQ ( string ( "T" ), ToCppString ( t -> stbl -> name -> name ) );
}

FIXTURE_TEST_CASE ( View_GetBoundObject_MultipleParameters, ViewFixture )
{
    m_schemaText = BaseSchemaText + // defines table T and view V
        "view X#1 < V p_v, T p_t > {};"
        "view Y#1 < T tab, V v >    = X < v, tab > {};"
    ;

    CreateAndBindView ( GetName(), "V" );
    // m_view is a V, m_table is a T
    const VView * Y;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & Y, m_schema, "Y" ) );
    String t;
    CONST_STRING ( & t, "tab" );
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & m_table, TableName_1 ) );
    REQUIRE_RC ( VViewBindParameterTable ( Y, & t, m_table ) );
    CONST_STRING ( & t, "v" );
    REQUIRE_RC ( VViewBindParameterView  ( Y, & t, m_view ) );

    // expect: X.p_v refers to m_view; X.p_t refers to m_table
    const SView * sX = static_cast < const SView * > ( VectorGet ( & m_schema -> view, 1 ) ); //X

    // first argument of X refers to V
    REQUIRE_EQ ( (const void *)m_view, VViewGetBoundObject ( Y, sX, 0 ) );

    // second argument of X refers to T
    REQUIRE_EQ ( (const void *)m_table, VViewGetBoundObject ( Y, sX, 1 ) );

    REQUIRE_RC ( VViewRelease ( Y ) );
}

FIXTURE_TEST_CASE ( View_GetBoundObject_IndirectParentView, ViewFixture )
{
    m_schemaText = BaseSchemaText + // defines T and V<T tbl>
        "view X#1 < T p_t > = V < p_t > {};"
        "view Y#1 < T tb > = X < tb > {};"
    ;
    CreateView ( GetName(), "Y" );
    // m_view is a Y, m_table is a T

    // first argument of grandparent (V) is bound to T
    // expect: V.tbl refers to m_table
    const SView * sV = static_cast < const SView * > ( VectorGet ( & m_schema -> view, 0 ) ); //V
    REQUIRE_EQ ( (const void *)m_table, VViewGetBoundObject ( m_view, sV, 0 ) );
}

FIXTURE_TEST_CASE ( View_ParameterCount_SelfNull, ViewFixture )
{
    REQUIRE_EQ ( 0u, VViewParameterCount ( NULL ) );
}

FIXTURE_TEST_CASE ( View_ParameterCount, ViewFixture )
{
    m_schemaText = BaseSchemaText + // defines T and V
        "view X#1 < V p_v, T p_t > {};"
    ;

    CreateView ( GetName(), "X" );
    REQUIRE_EQ ( 2u, VViewParameterCount ( m_view ) );
}

FIXTURE_TEST_CASE ( View_GetParameter_SelfNull, ViewFixture )
{
    const String * name;
    bool isTable = true;
    REQUIRE_RC_FAIL ( VViewGetParameter ( NULL, 0, & name, & isTable ) );
}

FIXTURE_TEST_CASE ( View_GetParameter_ParamsNull, ViewFixture )
{
    m_schemaText = BaseSchemaText + // defines T and V
        "view X#1 < V p_v, T p_t > {};"
    ;
    CreateView ( GetName(), "X" );
    REQUIRE_RC_FAIL ( VViewGetParameter ( m_view, 0, NULL, NULL ) );
}

FIXTURE_TEST_CASE ( View_GetParameter_OutOfRange, ViewFixture )
{
    m_schemaText = BaseSchemaText + // defines T and V
        "view X#1 < V p_v, T p_t > {};"
    ;
    CreateView ( GetName(), "X" );
    const String * name;
    bool isTable = true;
    REQUIRE_RC_FAIL ( VViewGetParameter ( m_view, 2, & name, & isTable ) );
}

FIXTURE_TEST_CASE ( View_GetParameter_Name, ViewFixture )
{
    m_schemaText = BaseSchemaText + // defines T and V
        "view X#1 < V p_v, T p_t > {};"
    ;

    CreateView ( GetName(), "X" );
    const String * name;
    REQUIRE_RC ( VViewGetParameter ( m_view, 0, & name, NULL ) );
    REQUIRE_EQ ( ToCppString ( * name ), string ( "p_v" ) );
    REQUIRE_RC ( VViewGetParameter ( m_view, 1, & name, NULL ) );
    REQUIRE_EQ ( ToCppString ( * name ), string ( "p_t" ) );
}

FIXTURE_TEST_CASE ( View_GetParameter_IsTable, ViewFixture )
{
    m_schemaText = BaseSchemaText + // defines T and V
        "view X#1 < V p_v, T p_t > {};"
    ;

    CreateView ( GetName(), "X" );
    bool isTable = true;
    REQUIRE_RC ( VViewGetParameter ( m_view, 0, NULL, & isTable ) );
    REQUIRE ( ! isTable );
    REQUIRE_RC ( VViewGetParameter ( m_view, 1, NULL, & isTable ) );
    REQUIRE ( isTable );
}

FIXTURE_TEST_CASE ( View_ListCol_SelfNull, ViewFixture )
{
    struct KNamelist * names = NULL;
    REQUIRE_RC_FAIL ( VViewListCol ( NULL, & names ) );
}

FIXTURE_TEST_CASE ( View_ListCol_PAramNull, ViewFixture )
{
    CreateView ( GetName(), "V" );

    REQUIRE_RC_FAIL ( VViewListCol ( m_view, NULL ) );
}

FIXTURE_TEST_CASE ( View_ListCol_NotBound, ViewFixture )
{
    m_schemaText = BaseSchemaText + // defines T and V
        "view X#1 < P p_v, T p_t > { column ascii v = p_v . c1; column ascii t = p_t . c1; }"
    ;
    CreateView ( GetName(), "X" );

    struct KNamelist * names = NULL;
    REQUIRE_RC_FAIL ( VViewListCol ( m_view, & names ) );
}

FIXTURE_TEST_CASE ( View_ListCol, ViewFixture )
{
    m_schemaText = BaseSchemaText + // defines T and V
        "view X#1 < P pp, T tt >"
        "{ column ascii p_c = pp . c1; column ascii t_c = tt . c1; }"
    ;
    CreateView ( GetName(), "X" );
    BindTable( "pp", "p" );
    BindTable( "tt", "t" );

    struct KNamelist * names = NULL;
    REQUIRE_RC ( VViewListCol ( m_view, & names ) );
    REQUIRE_NOT_NULL ( names );
    uint32_t count = 0;
    REQUIRE_RC ( KNamelistCount ( names, & count ) );
    REQUIRE_EQ ( 2u, count );
    // TODO: verify columns
    REQUIRE_RC ( KNamelistRelease ( names ) );
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

const char UsageDefaultName[] = "test-view";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=ViewTestSuite(argc, argv);
    return rc;
}

}
