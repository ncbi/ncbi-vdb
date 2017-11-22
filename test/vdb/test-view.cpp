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

class ViewFixture : public WVDB_Fixture
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

const char* TableName = "t";
const char* ColumnName = "c1";
const char* ViewName = "V";

class EmptyViewFixture : public ViewFixture
{
public:
    EmptyViewFixture()
    : m_view ( 0 )
    {
    }
    ~EmptyViewFixture()
    {
        VViewRelease ( m_view );
    }

    void CreateDb ( const string & p_testName )
    {
        m_databaseName = ScratchDir + p_testName;

        const char * schema =
            "version 2.0;"
            "table T#1 { column ascii c1; };"
            "table P#1 { column ascii c1; };"
            "database DB#1 { table T t; table P p; };"
            "view V#1 < T t > {};"
            "view W#1 < V v > {};"
            ;
        MakeDatabase ( schema, "DB" );

        VCursor* cursor = CreateTable ( TableName );

        uint32_t column_idx;
        THROW_ON_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        THROW_ON_RC ( VCursorOpen ( cursor ) );

        // insert some rows
        WriteRow ( cursor, column_idx, "blah" );
        WriteRow ( cursor, column_idx, "eeee" );

        THROW_ON_RC ( VCursorCommit ( cursor ) );

        THROW_ON_RC ( VCursorRelease ( cursor ) );
    }

    void CreateView ( const string & p_testName )
    {
        CreateDb ( p_testName );
        THROW_ON_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, ViewName ) );
    }

    void CreateAndBindView ( const string & p_testName )
    {
        CreateView ( p_testName );
        const VTable * tbl;
        THROW_ON_RC ( VDatabaseOpenTableRead ( m_db, & tbl, TableName ) );
        THROW_ON_RC ( VViewBindParameterTable ( m_view, tbl, 0 ) );
        THROW_ON_RC ( VTableRelease ( tbl ) );
    }

    const VView * m_view;
};

// VDBManagerOpenView

FIXTURE_TEST_CASE ( OpenView_BadSelf, EmptyViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( 0, & m_view, m_schema, ViewName ) );
}

FIXTURE_TEST_CASE ( OpenView_BadView, EmptyViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( m_mgr, 0, m_schema, ViewName ) );
}

FIXTURE_TEST_CASE ( OpenView_BadSchema, EmptyViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( m_mgr, & m_view, 0, ViewName ) );
}

FIXTURE_TEST_CASE ( OpenView_BadName, EmptyViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC_FAIL ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "zzz" ) );
}

FIXTURE_TEST_CASE ( OpenView, EmptyViewFixture )
{
    CreateDb ( GetName() );
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, ViewName ) );
}

//TODO: OpenView with the latest version
//TODO: OpenView with an old version
//TODO: OpenView with a non-existent version

// AddRef/Release

FIXTURE_TEST_CASE ( View_Refcount_AddRef_NullObject, EmptyViewFixture )
{
    REQUIRE_RC ( VViewAddRef ( 0 ) );
}
FIXTURE_TEST_CASE ( View_Refcount_Release_NullObject, EmptyViewFixture )
{
    REQUIRE_RC ( VViewRelease ( 0 ) );
}
FIXTURE_TEST_CASE ( View_Refcount_Add_Release, EmptyViewFixture )
{
    CreateView ( GetName() );
    REQUIRE_RC ( VViewAddRef ( m_view ) );
    REQUIRE_RC ( VViewRelease ( m_view ) );
    REQUIRE_RC ( VViewRelease ( m_view ) );
    m_view = 0;
}

// BindParameterTable

FIXTURE_TEST_CASE ( View_BindParameterTable_NullObject, EmptyViewFixture )
{
    CreateView ( GetName() );
    const VTable * tbl;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & tbl, TableName ) );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( 0, tbl, 0 ) );
    REQUIRE_RC ( VTableRelease ( tbl ) );
}

FIXTURE_TEST_CASE ( View_BindParameterTable_NullParam, EmptyViewFixture )
{
    CreateView ( GetName() );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, 0, 0 ) );
}

FIXTURE_TEST_CASE ( View_BindParameterTable_IndexOutOfRange, EmptyViewFixture )
{
    CreateView ( GetName() );
    const VTable * tbl;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & tbl, TableName ) );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, tbl, 1 ) );
    REQUIRE_RC ( VTableRelease ( tbl ) );
}

FIXTURE_TEST_CASE ( View_BindParameterTable_WrongType, EmptyViewFixture )
{
    CreateView ( GetName() );
    VCursor* cursor = CreateTable ( "p" );
    uint32_t column_idx;
    REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
    REQUIRE_RC ( VCursorOpen ( cursor ) );
    WriteRow ( cursor, column_idx, "blah" );
    WriteRow ( cursor, column_idx, "eeee" );
    REQUIRE_RC ( VCursorCommit ( cursor ) );
    REQUIRE_RC ( VCursorRelease ( cursor ) );

    const VTable * tbl;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & tbl, "p" ) );
    REQUIRE_RC_FAIL ( VViewBindParameterTable ( m_view, tbl, 0 ) );
    REQUIRE_RC ( VTableRelease ( tbl ) );
}

FIXTURE_TEST_CASE ( View_BindParameterTable, EmptyViewFixture )
{
    CreateView ( GetName() );
    const VTable * tbl;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & tbl, TableName ) );
    REQUIRE_RC ( VViewBindParameterTable ( m_view, tbl, 0 ) ); // V<t>
    REQUIRE_RC ( VTableRelease ( tbl ) );
}

FIXTURE_TEST_CASE ( View_BindParameterView_NullParam, EmptyViewFixture )
{
    CreateView ( GetName() );
    REQUIRE_RC_FAIL ( VViewBindParameterView ( m_view, 0, 0 ) );
}
FIXTURE_TEST_CASE ( View_BindParameterView, EmptyViewFixture )
{
    CreateView ( GetName() );
    const VView * w;
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & w, m_schema, "W" ) );
    REQUIRE_RC ( VViewBindParameterView ( w, m_view, 0 ) ); // W<v>
    REQUIRE_RC ( VViewRelease ( w ) );
}


// VViewCreateCursor

FIXTURE_TEST_CASE ( View_CreateCursor_SelfNull, EmptyViewFixture )
{
    CreateAndBindView ( GetName() );
    const VCursor * curs;
    REQUIRE_RC_FAIL ( VViewCreateCursor ( 0, & curs, 0 ) );
}

FIXTURE_TEST_CASE ( View_CreateCursor_CursorNull, EmptyViewFixture )
{
    CreateAndBindView ( GetName() );
    REQUIRE_RC_FAIL ( VViewCreateCursor ( m_view, 0, 0 ) );
}

#if SHOW_UNIMPLEMENTED
FIXTURE_TEST_CASE ( View_CreateCursor, EmptyViewFixture )
{
    CreateAndBindView ( GetName() );
    const VCursor * curs;
    REQUIRE_RC ( VViewCreateCursor ( m_view, & curs, 0 ) );
    REQUIRE_NOT_NULL ( curs );
    REQUIRE_RC ( VCursorRelease ( curs ) );
}
#endif

//TODO: VViewCreateCursor with multiple table/view parameters

//TODO: View_CreateCursor_WithCapacity - how to test?

//TODO: VTableMakeDefaultView

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
