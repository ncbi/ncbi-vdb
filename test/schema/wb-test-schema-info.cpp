/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

/**
* Unit tests for schema AST traversal
*/

#include "AST_Fixture.hpp"

#include <schema/SchemaInfo.hpp>

#include <ktst/unit_test.hpp>

using namespace std;
using namespace ncbi;
using namespace ncbi::NK;

static
ver_t
MakeVer( uint8_t maj, uint8_t min = 0, uint8_t rel = 0 )
{
    return VTRANSVERS( maj, min, rel );
}

TEST_SUITE ( SchemaInfoTestSuite );

// version resolution
TEST_CASE( VersionedNameMap_NotFound )
{
    SchemaInfo::VersionedNameMap<int> vr;
    vr.addUnique("name", MakeVer( 1, 2, 3 ), 1);
    auto it = vr.find( "bad_name", MakeVer( 1, 2, 3 ) );
    REQUIRE( vr.cend() == it );
}

TEST_CASE( VersionedNameMap_DefFull_RefFull )
{
    SchemaInfo::VersionedNameMap<int> vr;
    vr.addUnique("name", MakeVer( 1, 2, 3 ), 1);
    auto it = vr.find( "name", MakeVer( 1, 2, 3 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1.2.3"), it->first );
}
TEST_CASE( VersionedNameMap_DefNoRev_RefNoRel )
{
    SchemaInfo::VersionedNameMap<int> vr;
    vr.addUnique("name", MakeVer( 1, 2, 0 ), 1);
    auto it = vr.find( "name", MakeVer( 1, 2, 0 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1.2"), it->first );
}
TEST_CASE( VersionedNameMap_DefNoMin_RefNoMin )
{
    SchemaInfo::VersionedNameMap<int> vr;
    vr.addUnique("name", MakeVer( 1, 0, 0 ), 1);
    auto it = vr.find( "name", MakeVer( 1, 0, 0 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1"), it->first );
}

TEST_CASE( VersionedNameMap_DefFull_RefNoRel )
{
    SchemaInfo::VersionedNameMap<int> vr;
    vr.addUnique("name", MakeVer( 1, 2, 3 ), 1);
    auto it = vr.find( "name", MakeVer( 1, 2, 0 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1.2.3"), it->first );
}

TEST_CASE( VersionedNameMap_DefFull_RefRelHi )
{
    SchemaInfo::VersionedNameMap<int> vr;
    vr.addUnique("name", MakeVer( 1, 2, 3 ), 1);
    auto it = vr.find( "name", MakeVer( 1, 2, 4 ) );
    REQUIRE( vr.cend() == it );
}

TEST_CASE( VersionedNameMap_DefFull_RefRelMultiple_Exact )
{
    SchemaInfo::VersionedNameMap<int> vr;
    vr.addUnique("name", MakeVer( 1, 2, 1 ), 1);
    vr.addUnique("name", MakeVer( 1, 2, 2 ), 1);
    vr.addUnique("name", MakeVer( 1, 2, 3 ), 1);
    auto it = vr.find( "name", MakeVer( 1, 2, 2 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1.2.2"), it->first );
}

TEST_CASE( VersionedNameMap_DefFull_RefRelMultiple_Latest )
{   // no release specified, the highest release selected
    SchemaInfo::VersionedNameMap<int> vr;
    vr.addUnique("name", MakeVer( 1, 2, 1 ), 1);
    vr.addUnique("name", MakeVer( 1, 2, 2 ), 1);
    auto it = vr.find( "name", MakeVer( 1, 2, 0 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1.2.2"), it->first );
}

TEST_CASE( VersionedNameMap_DefFull_RefNoMinor )
{   // no release specified, the highest release selected
    SchemaInfo::VersionedNameMap<int> vr;
    vr.addUnique("name", MakeVer( 2, 1, 1 ), 1);
    auto it = vr.find( "name", MakeVer( 2, 0, 0 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#2.1.1"), it->first );
}

// SchemaInfo

class SchemaInfoFixture : public AST_Fixture
{
public:
    SchemaInfoFixture(){}
    ~SchemaInfoFixture(){}

    void Setup(const char* schema )
    {
        AST * root = MakeAst  ( schema );
        THROW_ON_FALSE( root );
        si = SchemaInfo( *root );
    }

    SchemaInfo si;
};

FIXTURE_TEST_CASE(Functions, SchemaInfoFixture)
{
    Setup( R"(
        function ascii fn1 #1.0( ascii a , ascii b );
        function ascii fn2 #1.0( ascii a , ascii b );
    )" );

    REQUIRE_EQ( 2, (int)si.functions.size() );
    auto it = si.functions.find("fn1", MakeVer( 1 ) );
    REQUIRE( si.functions.end() != it );
    {
        auto& l = it->second.getLocation();
        REQUIRE_EQ( string("<unknown>"), l.m_file );
        REQUIRE_EQ( 2, (int)l.m_line );
        REQUIRE_EQ( 24, (int)l.m_column );
    }
    it = si.functions.find("fn2", MakeVer( 1 ) );
    REQUIRE( si.functions.end() != it );
    {
        auto& l = it->second.getLocation();
        REQUIRE_EQ( string("<unknown>"), l.m_file );
        REQUIRE_EQ( 3, (int)l.m_line );
        REQUIRE_EQ( 24, (int)l.m_column );
    }
}

FIXTURE_TEST_CASE(Functions_Redefinition, SchemaInfoFixture)
{
    AST * root = MakeAst  ( R"(
        function ascii fn1 #1.0( ascii a , ascii b );
        function ascii fn1 #1.0( ascii a , ascii b );
    )" );
    REQUIRE_NOT_NULL( root );
    REQUIRE_THROW( SchemaInfo si( *root ); );
}

FIXTURE_TEST_CASE(Database, SchemaInfoFixture)
{
    Setup( R"(
        table T1 #1{}
        table T2 #1{}
        database DB1 #1.0.1
        {
            table T1 #1 t1_1;
            table T2 #1 t1_2;
        };
        database DB2 #1 = DB1#1
        {
            table T2 #1 t2;
        }
    )" );

    //si.databases.print(cout);

    REQUIRE_EQ( 2, (int)si.databases.size() );
    auto it = si.databases.find( "DB1", MakeVer( 1 ) );
    REQUIRE( si.databases.end() != it );
    {
        REQUIRE_EQ( string("DB1#1.0.1"), it->first );
        auto & d = it->second;

        auto& l = d.getLocation();
        REQUIRE_EQ( string("<unknown>"), l.m_file );
        REQUIRE_EQ( 4, (int)l.m_line );
        REQUIRE_EQ( 18, (int)l.m_column );

        REQUIRE( d.parent.empty() );
        REQUIRE_EQ( 2, (int)d.tables.size() );
    }

    it = si.databases.find( "DB2", MakeVer( 1 ) );
    REQUIRE( si.databases.end() != it );
    {
        auto & d = it->second;
        REQUIRE_EQ( string("DB1#1.0.1"), d.parent );    // best fit version
        REQUIRE_EQ( 1, (int)d.tables.size() );
    }
}

FIXTURE_TEST_CASE(Database_Redefinition, SchemaInfoFixture)
{
    AST * root = MakeAst  ( R"(
        table T1 #1{}
        database DB1 #1
        {
            table T1 #1 t1_1;
        };
        database DB1 #1
        {
            table T1 #1 t1_1;
        };
    )" );
    REQUIRE_NOT_NULL( root );
    REQUIRE_THROW( SchemaInfo si( *root ); );}

FIXTURE_TEST_CASE(Table, SchemaInfoFixture)
{
    Setup( R"(
        table T1#1.0.1 {}
        table T2#2 {}
        table T3 #3 = T1#1, T2#2{
            U8 p = 1;
            column U8 c = p;
        }
    )" );

    REQUIRE_EQ( 3, (int)si.tables.size() );
    REQUIRE( si.tables.end() != si.tables.find("T1", MakeVer(1)) );
    REQUIRE( si.tables.end() != si.tables.find("T2", MakeVer(2)) );
    auto it = si.tables.find("T3", MakeVer(3) );
    REQUIRE( si.tables.end() != it );
    {
        auto & t = it->second;
        REQUIRE_EQ( 2, (int)t.parents.size() );
        REQUIRE( t.parents.end() != t.parents.find("T1#1.0.1") ); // best fit version
        REQUIRE( t.parents.end() != t.parents.find("T2#2") );

        REQUIRE_EQ( 1, (int)t.columns.size() );
        auto & c = t.columns.begin()->second;
        REQUIRE_EQ( string("<unknown>"), c.location.m_file );
        REQUIRE_EQ( 6, (int)c.location.m_line );
        REQUIRE_EQ( 23, (int)c.location.m_column );

        REQUIRE_EQ( 1, (int)t.productions.size() );
    }
}

FIXTURE_TEST_CASE(Table_Redefinition, SchemaInfoFixture)
{
    AST * root = MakeAst  ( R"(
        table T1#1 {}
        table T1#1 {}
    )" );
    REQUIRE_NOT_NULL( root );
    REQUIRE_THROW( SchemaInfo si( *root ); );
}

FIXTURE_TEST_CASE(Table_ReferencesFromColumn, SchemaInfoFixture)
{
    Setup( R"(
        function U8 fn1 #1.0( U8 a );
        table T1#1{
            U8 p = 1;
            column U8 c = forward | fn1( p );
        }
    )" );

    auto it = si.tables.find("T1", MakeVer(1));
    REQUIRE( si.tables.end() != it );
    {
        auto & t = it->second;
        REQUIRE_EQ( 1, (int)t.columns.size() );
        auto & c = t.columns.at( "c" );

        // ids of called functions go into c.calls with a version, c.ids without
        REQUIRE_EQ( 1, (int)c.calls.size() );
        REQUIRE( c.calls.end() != c.calls.find("fn1#1(p)") );

        REQUIRE_EQ( 3, (int)c.ids.size() );
        REQUIRE( c.ids.end() != c.ids.find("fn1") );
        REQUIRE( c.ids.end() != c.ids.find("p") );
        REQUIRE( c.ids.end() != c.ids.find("forward") );
    }
}

FIXTURE_TEST_CASE(Table_ReferencesFromProduction, SchemaInfoFixture)
{
    Setup( R"(
        function U8 fn1 #1.0( U8 a );
        table T1#1{
            U8 p1 = 1;
            U8 p2 = forward | fn1( p1 );
        }
    )" );

    auto it = si.tables.find("T1", MakeVer(1));
    REQUIRE( si.tables.end() != it );
    {
        auto & t = it->second;
        REQUIRE_EQ( 2, (int)t.productions.size() );

        {
            auto & p = t.productions.at( "p1" );
            REQUIRE_EQ( 0, (int)p.calls.size() );
            REQUIRE_EQ( 0, (int)p.ids.size() );
        }

        {
            auto & p = t.productions.at( "p2" );

            // ids of called functions go into p.calls with a version, p.ids without
            REQUIRE_EQ( 1, (int)p.calls.size() );
            REQUIRE( p.calls.end() != p.calls.find("fn1#1(p1)") );

            REQUIRE_EQ( 3, (int)p.ids.size() );
            REQUIRE( p.ids.end() != p.ids.find("fn1") );
            REQUIRE( p.ids.end() != p.ids.find("p1") );
            REQUIRE( p.ids.end() != p.ids.find("forward") );
        }
    }
}

FIXTURE_TEST_CASE(Table_FunctionCalls, SchemaInfoFixture)
{   // parameters recorded at the call site
    Setup( R"(
        function U8 fn1 #1.0( U8 a, U16 b );
        table T1#1{
            column U8 c1;
            column U16 c2;
            column U8 c3 = fn1( c1, c2 );
        }
    )" );

    auto it = si.tables.find("T1", MakeVer(1));
    REQUIRE( si.tables.end() != it );
    {
        auto & t = it->second;
        REQUIRE_EQ( 3, (int)t.columns.size() );
        const auto & c = t.columns.find( "c3" );
        REQUIRE( t.columns.end() != c );
        REQUIRE_EQ( 1, (int)c->second.calls.size() );
        REQUIRE_EQ( string( "fn1#1(c1,c2)" ), *c->second.calls.begin() );
    }

}

FIXTURE_TEST_CASE(Id_Resolution_same_table, SchemaInfoFixture)
{
    Setup( R"(
        table T1#1{
            U8 p = 1;
            column U8 c;
        }
    )" );

    const string T1 = "T1#1";
    SchemaInfo::Definition def = si.resolve( T1, "p" );
    REQUIRE_EQ( T1, def.owner );
    REQUIRE( ! def.is_column );

    def = si.resolve( T1, "c" );
    REQUIRE_EQ( T1, def.owner );
    REQUIRE( def.is_column );
}

FIXTURE_TEST_CASE(Id_Resolution_ancestor, SchemaInfoFixture)
{
    Setup( R"(
        table T1#1{
            U8 p = 1;
            column U8 c;
        }
        table T2#1 = T1#1{
        }
    )" );

    const string T1 = "T1#1";
    const string T2 = "T2#1";

    auto it = si.tables.find(T1);
    REQUIRE( si.tables.end() != it );
    {
        SchemaInfo::Definition def = si.resolve( T1, "p" );
        REQUIRE( ! def.empty() );
        REQUIRE_EQ( T1, def.owner );
        REQUIRE( ! def.is_column );

        def = si.resolve( T1, "c" );
        REQUIRE( ! def.empty() );
        REQUIRE_EQ( T1, def.owner );
        REQUIRE( def.is_column );
    }
}

FIXTURE_TEST_CASE(Id_Resolution_undefined, SchemaInfoFixture)
{   // undefined or defined lower in the hiararchy
    Setup( R"(
        table T1#1{
            U8 p = 1;
            column U8 c = p2;
        }
        table T2#1 = T1#1{
            column U8 c2;
        }
    )" );

    const string T1 = "T1#1";
    REQUIRE( si.resolve( T1, "p2" ).empty() ); // never defined
    REQUIRE( si.resolve( T1, "c2" ).empty() ); // defined lower in the hierarchy than T1
}

FIXTURE_TEST_CASE(Database_Closure, SchemaInfoFixture)
{   // all databases in an inheritance hierarchy
    Setup( R"(
        database D1#1{}
        database D2#1 = D1#1{}
        database D3#1 = D2#1{}
        database D4#1 = D3#1{}
    )" );

    auto c = DatabaseClosure( si, "D3#1" );
    REQUIRE_EQ( 3, (int)c.size() );
    REQUIRE( c.end() != c.find("D1#1") );
    REQUIRE( c.end() != c.find("D2#1") );
    REQUIRE( c.end() != c.find("D3#1") );
}

FIXTURE_TEST_CASE(Tables_closure, SchemaInfoFixture)
{   // all tables in an inheritance hierarchy
    Setup( R"(
        table T1#1{
        }
        table T2#1 = T1#1{
        }
        table T3#1 = T1#1, T2#1{
        }
        table T4#1 = T1#1{
        }
    )" );

    auto c = TablesClosure( si, "T3#1" );
    REQUIRE_EQ( 3, (int)c.size() );
    REQUIRE( c.end() != c.find("T1#1") );
    REQUIRE( c.end() != c.find("T2#1") );
    REQUIRE( c.end() != c.find("T3#1") );
}

FIXTURE_TEST_CASE(FunctionCallClosure_empty, SchemaInfoFixture)
{
    Setup( R"(
        table T1#1{
            ascii j = i;
        }
        table T2#1 = T1#1{
        }
    )" );

    auto c = FunctionCallClosure( si, "T2#1", "T1#1", "j" );
    REQUIRE_EQ( 0, (int)c.size() );
}

FIXTURE_TEST_CASE(FunctionCallClosure_forward_reference, SchemaInfoFixture)
{   // forward references may only be defined as productions
    Setup( R"(
        function U8 fn1 #1.0( U8 a, U16 b );
        table T1#1{
            ascii j = i;
        }
        table T2#1 = T1#1{
            column U8 c;
            U8 i = fn1( c );
        }
    )" );

    auto c = FunctionCallClosure( si, "T2#1", "T1#1", "j" );
    REQUIRE_EQ( 1, (int)c.size() );
    REQUIRE_EQ( string("fn1#1(c)"), *c.begin() );
}

FIXTURE_TEST_CASE(FunctionCallClosure_search, SchemaInfoFixture)
{
    Setup( R"(
        function U8 fn1 #1.0( U8 a, U16 b );
        table T1#1{
            ascii j = i;
        }
        table T2#1 = T1#1{
            column U8 c;
            U8 i = fn1( c );
        }
        table T3#1 = T2#1{
        }
    )" );

    auto c = FunctionCallClosure( si, "T3#1", "T1#1", "j" );
    REQUIRE_EQ( 1, (int)c.size() );
    REQUIRE_EQ( string("fn1#1(c)"), *c.begin() );
}

FIXTURE_TEST_CASE(FunctionCallClosure_column, SchemaInfoFixture)
{
    Setup( R"(
        function U8 fn1 #1.0( U8 a, U16 b );
        table T1#1{
            U8 p1 = 1;
            U8 p2 = fn1( p1 );
        }
        table T2#1 = T1#1{
            column U8 c = p2 | fn1( p2 );
        }
    )" );

    auto c = FunctionCallClosure( si, "T2#1", "T1#1", "c" );
    REQUIRE_EQ( 2, (int)c.size() );
    REQUIRE( c.end() != c.find("fn1#1(p1)") );
    REQUIRE( c.end() != c.find("fn1#1(p2)") );
}

//////////////////////////////////////////// Main
int main( int argc, char *argv [] )
{
    return SchemaInfoTestSuite(argc, argv);
}
