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

#include <klib/symbol.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/view.h>

#include "../libs/vdb/database-priv.h"
#include "../libs/vdb/schema-priv.h"
#include "../libs/schema/SchemaParser.hpp"
#include "../libs/schema/ASTBuilder.hpp"

#include <ktst/unit_test.hpp> // TEST_CASE

#include "WVDB_Fixture.hpp"

using namespace std;
using namespace ncbi :: SchemaParser;

TEST_SUITE( ViewTestSuite )

#include <ktst/unit_test.hpp> // THROW_ON_RC

const string ScratchDir = "./db/";

// WVDB_Fixture modified to use v2 schema parser
class WVDB_v2_Fixture : public WVDB_Fixture
{
public:
    virtual void ParseSchema ( VSchema * p_schema, const std :: string & p_schemaText )
    {
        SchemaParser parser;
        if ( ! parser . ParseString ( p_schemaText . c_str () ) )
        {
            throw std :: logic_error ( string ( "WVDB_Fixture::MakeDatabase : ParseString() failed: " ) + FormatErrorMessage ( * parser . GetErrors () . GetError ( 0 ) ) );
        }
        ParseTree * parseTree = parser . MoveParseTree ();
        if ( parseTree == 0 )
        {
            throw std :: logic_error ( "WVDB_Fixture::MakeDatabase : MoveParseTree() returned 0" );
        }
        ASTBuilder builder ( p_schema );
        AST * ast = builder . Build ( * parseTree, "", false );
        if ( builder . GetErrorCount() != 0)
        {
            throw std :: logic_error ( std :: string ( "AST_Fixture::MakeAst : ASTBuilder::Build() failed: " ) + FormatErrorMessage ( * parser . GetErrors () . GetError ( 0 ) ) );
        }
        else if ( ast == 0 )
        {
            throw std :: logic_error ( "AST_Fixture::MakeAst : ASTBuilder::Build() failed, no message!" );
        }
        delete ast;
        delete parseTree;
    }

    static string FormatErrorMessage( const ErrorReport :: Error & p_err )
    {
        char buf [1024];
        if ( p_err . Format ( buf, sizeof ( buf ) ) )
        {
            return string ( buf );
        }
        return "buffer to short for an error message";
    }

    static std :: string ToCppString ( const String & p_str)
    {
        return std :: string ( p_str . addr, p_str . len );
    }

};

static const char * SchemaText =
    "version 2.0;"
    "table T#1 { column ascii c1; };"
    "table P#1 { column ascii c1; };"
    "database DB#1 { table T t; table P p; };"
    "view V#1 < T tbl > {};"
    "view W#1 < V vw > {};"
    "view OneColumn#1 < T tbl > { column ascii c = tbl . c1; };"
    ;

static const char* TableName = "t";
static const char* TableColumnName = "c1";
static const char* EmptyViewName = "V";
static const char* TableParamName = "tbl";
static const char* ViewParamName = "vw";
static const char* OneColumnViewName = "OneColumn";
static const char* ViewColumnName = "c";

class ViewFixture : public WVDB_v2_Fixture
{
public:
    ViewFixture()
    :   m_view ( 0 ),
        m_cur ( 0 )
    {
    }
    ~ViewFixture()
    {
        VCursorRelease ( m_cur );
        VViewRelease ( m_view );
    }

    void CreateDb ( const string & p_testName )
    {
        m_databaseName = ScratchDir + p_testName;

        MakeDatabase ( SchemaText, "DB" );

        VCursor* cursor = CreateTable ( TableName );

        uint32_t column_idx;
        THROW_ON_RC ( VCursorAddColumn ( cursor, & column_idx, TableColumnName ) );
        THROW_ON_RC ( VCursorOpen ( cursor ) );

        // insert some rows
        WriteRow ( cursor, column_idx, "blah" );
        WriteRow ( cursor, column_idx, "eeee" );

        THROW_ON_RC ( VCursorCommit ( cursor ) );

        THROW_ON_RC ( VCursorRelease ( cursor ) );
    }

    void CreateView ( const string & p_testName, const char * p_viewName )
    {
        CreateDb ( p_testName );
        THROW_ON_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, p_viewName ) );
    }

    void CreateAndBindView ( const string & p_testName, const char * p_viewName )
    {
        CreateView ( p_testName, p_viewName );
        const VTable * tbl;
        THROW_ON_RC ( VDatabaseOpenTableRead ( m_db, & tbl, TableName ) );
        THROW_ON_RC ( VViewBindParameterTable ( m_view, TableParamName, tbl ) );
        THROW_ON_RC ( VTableRelease ( tbl ) );
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

    const VTable* CreateTableOpenRead ( const char * p_tableName )
    {
        VCursor* cursor = CreateTable ( p_tableName );
        uint32_t column_idx;
        THROW_ON_RC ( VCursorAddColumn ( cursor, & column_idx, TableColumnName ) );
        THROW_ON_RC ( VCursorOpen ( cursor ) );
        WriteRow ( cursor, column_idx, "blah" );
        WriteRow ( cursor, column_idx, "eeee" );
        THROW_ON_RC ( VCursorCommit ( cursor ) );
        THROW_ON_RC ( VCursorRelease ( cursor ) );
        const VTable * ret;
        THROW_ON_RC ( VDatabaseOpenTableRead ( m_db, & ret, p_tableName ) );
        return ret;
    }

    const VView *   m_view;
    const VCursor * m_cur;
    uint32_t        m_colIdx;
};

// VDBManagerOpenView

FIXTURE_TEST_CASE ( OpenView_BadSelf, ViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( 0, & m_view, m_schema, EmptyViewName ) );
}

FIXTURE_TEST_CASE ( OpenView_BadView, ViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( m_mgr, 0, m_schema, EmptyViewName ) );
}

FIXTURE_TEST_CASE ( OpenView_BadSchema, ViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( m_mgr, & m_view, 0, EmptyViewName ) );
}

FIXTURE_TEST_CASE ( OpenView_BadName, ViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "zzz" ) );
}

FIXTURE_TEST_CASE ( OpenView, ViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, EmptyViewName ) );
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
    CreateView ( GetName(), EmptyViewName );
    REQUIRE_RC ( VViewAddRef ( m_view ) );
    REQUIRE_RC ( VViewRelease ( m_view ) );
    REQUIRE_RC ( VViewRelease ( m_view ) );
    m_view = 0;
}

// BindParameterTable

FIXTURE_TEST_CASE ( View_BindParameterTable_NullObject, ViewFixture )
{
    CreateView ( GetName(), EmptyViewName );
    const VTable * tbl;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & tbl, TableName ) );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( 0, "", tbl ) );
    REQUIRE_RC ( VTableRelease ( tbl ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable_NullName, ViewFixture )
{
    CreateView ( GetName(), EmptyViewName );
    const VTable * tbl;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & tbl, TableName ) );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, 0, tbl ) );
    REQUIRE_RC ( VTableRelease ( tbl ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable_NullTable, ViewFixture )
{
    CreateView ( GetName(), EmptyViewName );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, "tbl", 0 ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable_WrongName, ViewFixture )
{
    CreateView ( GetName(), EmptyViewName );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, "notAtbl", 0 ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable_NotATable, ViewFixture )
{
    CreateView ( GetName(), EmptyViewName );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, "notAtbl", 0 ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable_WrongTableType, ViewFixture )
{
    CreateView ( GetName(), EmptyViewName );
    const VTable * tbl = CreateTableOpenRead ( "p" );

    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, "tbl", tbl ) );

    REQUIRE_RC ( VTableRelease ( tbl ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable, ViewFixture )
{
    CreateView ( GetName(), EmptyViewName );
    const VTable * tbl;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & tbl, TableName ) );
    REQUIRE_RC ( VViewBindParameterTable ( m_view, "tbl", tbl ) ); // V<T tbl>
    REQUIRE_RC ( VTableRelease ( tbl ) );
}
FIXTURE_TEST_CASE ( View_BindParameterTable_TooManyParams, ViewFixture )
{
    CreateView ( GetName(), EmptyViewName );
    const VTable * tbl;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & tbl, TableName ) );
    REQUIRE_RC ( VViewBindParameterTable ( m_view, "tbl", tbl ) ); // V<T tbl>
//    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, "tbl", tbl ) );
    REQUIRE_RC ( VTableRelease ( tbl ) );
}

// BindParameterView
FIXTURE_TEST_CASE ( View_BindParameterView_NullParam, ViewFixture )
{
    CreateView ( GetName(), EmptyViewName );
    REQUIRE_RC_FAIL ( VViewBindParameterView ( m_view, 0, 0 ) );
}
//TODO: BindParameterView_TooManyParameters
//TODO: BindParameterView_NotAViewAtTheIndex
#if 0
FIXTURE_TEST_CASE ( View_BindParameterView, ViewFixture )
{
    CreateView ( GetName(), EmptyViewName );
    const VView * w;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & w, m_schema, "W" ) );
    REQUIRE_RC ( VViewBindParameterView ( w, m_view, 0 ) ); // W<v>
    REQUIRE_RC ( VViewRelease ( w ) );
}
#endif

// VViewCreateCursor
FIXTURE_TEST_CASE ( View_CreateCursor_SelfNull, ViewFixture )
{
    CreateAndBindView ( GetName(), EmptyViewName );
    const VCursor * curs;
    REQUIRE_RC_FAIL ( VViewCreateCursor ( 0, & curs ) );
}

FIXTURE_TEST_CASE ( View_CreateCursor_CursorNull, ViewFixture )
{
    CreateAndBindView ( GetName(), EmptyViewName );
    REQUIRE_RC_FAIL ( VViewCreateCursor ( m_view, 0 ) );
}

FIXTURE_TEST_CASE ( View_CreateCursor_Release, ViewFixture )
{
    CreateAndBindView ( GetName(), EmptyViewName );
    const VCursor * curs;
    REQUIRE_RC ( VViewCreateCursor ( m_view, & curs ) );
    REQUIRE_NOT_NULL ( curs );
    REQUIRE_RC ( VCursorRelease ( curs ) );
}

///////////////////////////// View-attached VCursor

FIXTURE_TEST_CASE ( ViewCursor_AddRef, ViewFixture )
{
    CreateCursor ( GetName(), EmptyViewName );
    REQUIRE_RC ( VCursorAddRef ( m_cur ) );
    REQUIRE_RC ( VCursorRelease ( m_cur ) );
    REQUIRE_RC ( VCursorRelease ( m_cur ) ); // use valgrind to find any leaks
    m_cur = 0;
}

// ViewCursor_AddColumn
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NullIdx, ViewFixture )
{
    CreateCursor ( GetName(), OneColumnViewName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, 0, "%s", ViewColumnName ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NullName, ViewFixture )
{
    CreateCursor ( GetName(), EmptyViewName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_colIdx, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_EmptyName, ViewFixture )
{
    CreateCursor ( GetName(), EmptyViewName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_colIdx, "" ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NameTooLong, ViewFixture )
{
    CreateCursor ( GetName(), EmptyViewName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_colIdx, "%s", string ( 2048, 'z' ) . c_str () ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NameNotInView, ViewFixture )
{
    CreateCursor ( GetName(), OneColumnViewName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_colIdx, "%s", "zz" ) );
}

FIXTURE_TEST_CASE ( ViewCursor_AddColumn, ViewFixture )
{
    CreateCursor ( GetName(), OneColumnViewName );
    REQUIRE_RC ( VCursorAddColumn ( m_cur, & m_colIdx, "%s", ViewColumnName ) );
    REQUIRE_EQ ( 1u, m_colIdx );
    //TODO: verify insertion into m_cur->row(at idx 1) and m_cur->col(at idx 0)
}

//TODO: ViewCursor_AddColumn_IncompleteType (is that possible?)
//TODO: ViewCursor_AddColumn_IncompatibleType (is that possible?)
//TODO: ViewCursor_AddColumn_PostOpen (error)
//TODO: ViewCursor_AddColumn_AlreadyAdded (error)

// VCursorGetColumnIdx

FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NullIdx, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, 0, "%s", ViewColumnName ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NullName, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_colIdx, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_EmptyName, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_colIdx, "" ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NameTooLong, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_colIdx, "%s", string ( 2048, 'z' ) . c_str () ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NameNotInView, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_colIdx, "%s", "zz" ) );
}
//TODO: ViewCursor_GetColumnIdx_StateFailed

FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );

    uint32_t idx = 0;
    REQUIRE_RC ( VCursorGetColumnIdx ( m_cur, & idx, "%s", ViewColumnName ) );
    REQUIRE_EQ ( 1u, idx );
}

//TODO: ViewCursor_GetColumnIdx_Ambiguous
//TODO: ViewCursor_GetColumnIdx_Overloaded

// VCursorDataDype
FIXTURE_TEST_CASE ( ViewCursor_DataType_NullParams, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );

    REQUIRE_RC_FAIL ( VCursorDatatype ( m_cur, m_colIdx, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_DataType_BadIndex, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    struct VTypedecl type = { 1, 1};
    struct VTypedesc desc = { 1, 1, 1};
    REQUIRE_RC_FAIL ( VCursorDatatype ( m_cur, 0, & type, & desc ) );
    // on error, outputs are 0-filled
    REQUIRE_EQ ( 0u, type . type_id );
    REQUIRE_EQ ( 0u, type . dim );
    REQUIRE_EQ ( 0u, desc . intrinsic_bits );
    REQUIRE_EQ ( 0u, desc . intrinsic_dim );
    REQUIRE_EQ ( 0u, desc . domain );
}
FIXTURE_TEST_CASE ( ViewCursor_DataType, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    struct VTypedecl type = { 0, 0};
    struct VTypedesc desc = { 0, 0, 0};
    REQUIRE_RC ( VCursorDatatype ( m_cur, m_colIdx, & type, & desc ) );
}

// VCursorOpen
FIXTURE_TEST_CASE ( ViewCursor_Open, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
}
#if 0
// VCursorIdRange
FIXTURE_TEST_CASE ( ViewCursor_IdRange_NullParams, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorIdRange ( m_cur, m_colIdx, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_IdRange_NotOpen, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    int64_t first;
    uint64_t count;
    REQUIRE_RC_FAIL ( VCursorIdRange ( m_cur, m_colIdx, & first, & count ) );
}
FIXTURE_TEST_CASE ( ViewCursor_IdRange, ViewFixture )
{
    CreateCursorAddColumn ( GetName(), OneColumnViewName, ViewColumnName );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
    int64_t first;
    uint64_t count;
    REQUIRE_RC ( VCursorIdRange ( m_cur, m_colIdx, & first, & count ) );
    REQUIRE_EQ ( 1l, first );
    REQUIRE_EQ ( 2lu, count );
}
//TODO: VTableMakeDefaultView

//TODO: VViewCreateCursor with multiple table/view parameters
//TODO: external function call in view's productions
//TODO: external function call in a view parameter's productions
//TODO: accessing a parameter view's columns

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

const char UsageDefaultName[] = "test-view";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=ViewTestSuite(argc, argv);
    return rc;
}

}
