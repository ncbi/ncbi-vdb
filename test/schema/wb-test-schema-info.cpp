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

#include "SchemaInfo.hpp"

#include <ktst/unit_test.hpp>

// #include <kfc/defs.h>

// #include <klib/printf.h>

// #include <vdb/xform.h>

// #include <map>
// #include <set>
// #include <sstream>

using namespace std;
using namespace ncbi::NK;

TEST_SUITE ( SchemaInfoTestSuite );

FIXTURE_TEST_CASE(ConstructDestruct, AST_Fixture)
{
    SchemaInfo si;
    REQUIRE_EQ( 0, (int)si.functions.size() );
    REQUIRE_EQ( 0, (int)si.tables.size() );
    REQUIRE_EQ( 0, (int)si.databases.size() );
}

FIXTURE_TEST_CASE(Functions, AST_Fixture)
{
    AST * root = MakeAst  ( R"(
        function ascii fn1 #1.0( ascii a , ascii b );
        function ascii fn2 #1.0( ascii a , ascii b );
    )" );
    REQUIRE_NOT_NULL( root );

    SchemaInfo si;
    si.populate( *root );

    //si.functions.print(cout);

    REQUIRE_EQ( 2, (int)si.functions.size() );
    REQUIRE( si.functions.end() != si.functions.find("fn1#1") );
    {
        auto& l = si.functions.find("fn1#1")->second.getLocation();
        REQUIRE_EQ( string("<unknown>"), l.m_file );
        REQUIRE_EQ( 2, (int)l.m_line );
        REQUIRE_EQ( 24, (int)l.m_column );
    }
    REQUIRE( si.functions.end() != si.functions.find("fn2#1") );
    {
        auto& l = si.functions.find("fn2#1")->second.getLocation();
        REQUIRE_EQ( string("<unknown>"), l.m_file );
        REQUIRE_EQ( 3, (int)l.m_line );
        REQUIRE_EQ( 24, (int)l.m_column );
    }
}

FIXTURE_TEST_CASE(Functions_Redefinition, AST_Fixture)
{
    AST * root = MakeAst  ( R"(
        function ascii fn1 #1.0( ascii a , ascii b );
        function ascii fn1 #1.0( ascii a , ascii b );
    )" );
    REQUIRE_NOT_NULL( root );
    REQUIRE_THROW( SchemaInfo().populate( *root ) );
}

FIXTURE_TEST_CASE(Database, AST_Fixture)
{
    AST * root = MakeAst  ( R"(
        table T1 #1{}
        table T2 #1{}
        database DB1 #1
        {
            table T1 #1 t1_1;
            table T2 #1 t1_2;
        };
        database DB2 #1 = DB1#1
        {
            table T2 #1 t2;
        }
    )" );
    REQUIRE_NOT_NULL( root );

    SchemaInfo si;
    si.populate( *root );

    //si.databases.print(cout);

    REQUIRE_EQ( 2, (int)si.databases.size() );
    REQUIRE( si.databases.end() != si.databases.find("DB1#1") );
    {
        auto & d = si.databases.find("DB1#1")->second;

        auto& l = d.getLocation();
        REQUIRE_EQ( string("<unknown>"), l.m_file );
        REQUIRE_EQ( 4, (int)l.m_line );
        REQUIRE_EQ( 18, (int)l.m_column );

        REQUIRE( d.parent.empty() );
        REQUIRE_EQ( 2, (int)d.tables.size() );
    }

    REQUIRE( si.databases.end() != si.databases.find("DB2#1") );
    {
        auto & d = si.databases.find("DB2#1")->second;
        REQUIRE_EQ( string("DB1#1"), d.parent );
        REQUIRE_EQ( 1, (int)d.tables.size() );
    }
}

FIXTURE_TEST_CASE(Database_Redefinition, AST_Fixture)
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
    REQUIRE_THROW( SchemaInfo().populate( *root ) );
}

FIXTURE_TEST_CASE(Table, AST_Fixture)
{
    AST * root = MakeAst  ( R"(
        table T1#1 {}
        table T2#2 {}
        table T3 #3 = T1#1, T2#2{
            U8 p = 1;
            column U8 c = p;
        }
    )" );
    REQUIRE_NOT_NULL( root );

    SchemaInfo si;
    si.populate( *root );

    REQUIRE_EQ( 3, (int)si.tables.size() );
    REQUIRE( si.tables.end() != si.tables.find("T1#1") );
    REQUIRE( si.tables.end() != si.tables.find("T2#2") );
    REQUIRE( si.tables.end() != si.tables.find("T3#3") );
    {
        auto & t = si.tables.find("T3#3")->second;
        REQUIRE_EQ( 2, (int)t.parents.size() );
        REQUIRE( t.parents.end() != t.parents.find("T1#1") );
        REQUIRE( t.parents.end() != t.parents.find("T2#2") );
        REQUIRE_EQ( 1, (int)t.columns.size() );
        REQUIRE_EQ( 1, (int)t.productions.size() );
    }
}

FIXTURE_TEST_CASE(Table_Redefinition, AST_Fixture)
{
    AST * root = MakeAst  ( R"(
        table T1#1 {}
        table T1#1 {}
    )" );
    REQUIRE_NOT_NULL( root );
    REQUIRE_THROW( SchemaInfo().populate( *root ) );
}

FIXTURE_TEST_CASE(Table_ReferencesFromColumn, AST_Fixture)
{
    AST * root = MakeAst  ( R"(
        function U8 fn1 #1.0( U8 a );
        table T1#1{
            U8 p = 1;
            column U8 c = p | fn1( p );
        }
    )" );
    REQUIRE_NOT_NULL( root );

    SchemaInfo si;
    si.populate( *root );

    REQUIRE( si.tables.end() != si.tables.find("T1#1") );
    {
        auto & t = si.tables.find("T1#1")->second;
        REQUIRE_EQ( 1, (int)t.columns.size() );
        auto & c = t.columns.at( "c" );
        REQUIRE_EQ( 1, (int)c.calls.size() );
        REQUIRE_EQ( 1, (int)c.ids.size() );
    }

}

#if 0
class NameMap : public map<string, set<string> >
{
public:
    void add( const string& key, const Token::Location& loc = {"", 0, 0} )
    {
        this->insert( make_pair( key, set<string>() ) );
        if ( loc.m_line != 0 )
        {
            locations[ key ] = LocationToString( loc );
        }
    }
    void add( const string& key, const string& value  )
    {
        at( key ) . insert( value );
    }

    void print( const string& header = string(), bool skip_empty = false ) const
    {
        cout << endl << header << ":" << endl;
        for( auto i = begin(); i != end(); ++i )
        {
            if ( ! skip_empty || i->second.size() > 0 )
            {
                if ( i != begin() ) cout << endl;
                cout << "  " << i->first;
                if ( locations.find(i->first) != locations.end() )
                {
                    cout << "(" << locations.at( i->first ) << ")";
                }
                cout << ": ";
                for( auto j = i->second.begin(); j != i->second.end(); ++j )
                {
                    cout << endl << "   " << *j;
                }
            }
        }
        cout << endl;
    }

    map<string, string> locations;
};

class VersionedNameMap : public NameMap
{
    public:
        // only allow adding versioned names
        void add( const string& name, ver_t version, const Token::Location& loc = {"", 0, 0 } )
        {
            char buf[1024];
            string_printf ( buf, sizeof( buf ), nullptr, "%s#%V", name.c_str(), version );
            NameMap::add( buf, loc );
            nameToVersions[name].insert(version);
        }

        NameMap::const_iterator find(const string & name) const
        {   // name#version
            return NameMap::find( name );
        }

        NameMap::const_iterator find( const string & name, ver_t version ) const
        {
            const auto n = nameToVersions.find( name );
            if( n == nameToVersions.end() )
            {
                return end();
            }

            auto major = VersionGetMajor( version );
            auto minor = VersionGetMinor( version );
            auto release = VersionGetRelease( version );
            ver_t best_fit = 0;
            for ( auto i : n->second )
            {
                if ( major == VersionGetMajor( i ) )
                {   // check minor & release
                    auto i_minor = VersionGetMinor( i );
                    auto i_release = VersionGetRelease( i );
                    if ( minor == 0 && release == 0 ) // minor is unspecified
                    {
                        best_fit = i;
                        break;
                    }
                    else if ( minor == i_minor )
                    {   // check release
                        if ( release == i_release )
                        {   // exact match
                            best_fit = i;
                            break;
                        }
                        if ( release == 0 )
                        {   // select the highest release
                            if ( best_fit == 0 )
                            {
                                best_fit = i;
                            }
                            else if ( i_release > VersionGetRelease( best_fit ) )
                            {
                                best_fit = i;
                            }
                        }
                    }
                }
            }
            if( best_fit == 0 )
            {
                return end();
            }

            char buf[1024];
            string_printf ( buf, sizeof( buf ), nullptr, "%s#%V", name.c_str(), best_fit );
            return find( string( buf ) );
        }

    private:
        map< string, set<ver_t> > nameToVersions;
};

// version resolution
TEST_CASE( VersionedNameMap_NotFound )
{
    VersionedNameMap vr;
    vr.add("name", VTRANSVERS( 1, 2, 3 ));
    auto it = vr.find( "bad_name", VTRANSVERS( 1, 2, 3 ) );
    REQUIRE( vr.cend() == it );
}

TEST_CASE( VersionedNameMap_DefFull_RefFull )
{
    VersionedNameMap vr;
    vr.add("name", VTRANSVERS( 1, 2, 3 ));
    auto it = vr.find( "name", VTRANSVERS( 1, 2, 3 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1.2.3"), it->first );
}
TEST_CASE( VersionedNameMap_DefNoRev_RefNoRel )
{
    VersionedNameMap vr;
    vr.add("name", VTRANSVERS( 1, 2, 0 ));
    auto it = vr.find( "name", VTRANSVERS( 1, 2, 0 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1.2"), it->first );
}
TEST_CASE( VersionedNameMap_DefNoMin_RefNoMin )
{
    VersionedNameMap vr;
    vr.add("name", VTRANSVERS( 1, 0, 0 ));
    auto it = vr.find( "name", VTRANSVERS( 1, 0, 0 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1"), it->first );
}

TEST_CASE( VersionedNameMap_DefFull_RefNoRel )
{
    VersionedNameMap vr;
    vr.add("name", VTRANSVERS( 1, 2, 3 ));
    auto it = vr.find( "name", VTRANSVERS( 1, 2, 0 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1.2.3"), it->first );
}

TEST_CASE( VersionedNameMap_DefFull_RefRelHi )
{
    VersionedNameMap vr;
    vr.add("name", VTRANSVERS( 1, 2, 3 ));
    auto it = vr.find( "name", VTRANSVERS( 1, 2, 4 ) );
    REQUIRE( vr.cend() == it );
}

TEST_CASE( VersionedNameMap_DefFull_RefRelMultiple_Exact )
{
    VersionedNameMap vr;
    vr.add("name", VTRANSVERS( 1, 2, 1 ));
    vr.add("name", VTRANSVERS( 1, 2, 2 ));
    vr.add("name", VTRANSVERS( 1, 2, 3 ));
    auto it = vr.find( "name", VTRANSVERS( 1, 2, 2 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1.2.2"), it->first );
}

TEST_CASE( VersionedNameMap_DefFull_RefRelMultiple_Latest )
{   // no release specified, the highest release selected
    VersionedNameMap vr;
    vr.add("name", VTRANSVERS( 1, 2, 1 ));
    vr.add("name", VTRANSVERS( 1, 2, 2 ));
    auto it = vr.find( "name", VTRANSVERS( 1, 2, 0 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#1.2.2"), it->first );
}

TEST_CASE( VersionedNameMap_DefFull_RefNoMinor )
{   // no release specified, the highest release selected
    VersionedNameMap vr;
    vr.add("name", VTRANSVERS( 2, 1, 1 ));
    auto it = vr.find( "name", VTRANSVERS( 2, 0, 0 ) );
    REQUIRE( vr.cend() != it );
    REQUIRE_EQ( string("name#2.1.1"), it->first );
}

struct AstMap
{
    string activeProd;
    NameMap ProdToFn;
    NameMap ProdToProd;
    set<string> ProdDefs;

    string activeCol;
    NameMap ColToFn;
    NameMap ColToProd;

    string activeTable;
    VersionedNameMap TblToProd;
    VersionedNameMap TblToCol;

    string activeDatabase;
    VersionedNameMap DbToTbl;

    map<string, string> FnLocations;
    set<string> FnWhiteList; // can be empty (=no filtering)


    void PrintFnLocations( ostream& out ) const
    {
        out << "FnLocatiouns:" << endl;
        for(auto i : FnLocations )
        {
            cout << "   " << i.first << ": " << i.second << endl;
        }
    }

    void PrintProdDefs( ostream& out ) const
    {
        out << "ProdDefs:" << endl;
        for ( auto p : ProdDefs )
        {
            out << "   " << p << endl;
        }
    }

    string
    FindProdDef( const string& p_table, const string& p_name ) const
    {   // given A:B:C:name, find X:Y:Z:name that is both in TblToProd[p_table] and a key in ProdDefs
        size_t lastPos = p_name.find_last_of('.');
        if (lastPos != std::string::npos)
        {
            auto t_p = TblToProd.find(p_table);
            if ( t_p != TblToProd.end() )
            {
                string name = p_name.substr( lastPos );
                for ( auto p : t_p->second )
                {
                    if (name.size() <= p.size() )
                    {
                        if ( std::equal(name.rbegin(), name.rend(), p.rbegin()) )
                        {
                            if ( ProdDefs.find( p ) != ProdDefs.end() )
                            {
                                return p;
                            }
                        }
                    }
                }
            }
        }
        return string();
    }

    void
    PrintProductionCallTree( size_t prefix, const string& p_table, const string& p_prodDef  ) const
    {
        auto p_f = ProdToFn.find( p_prodDef );
        if ( p_f != ProdToFn.end() )
        {
            for ( auto f : p_f->second )
            {
                size_t last = f.find('(');
                size_t first = f.substr( 0, last ) . rfind('.') + 1;
                string vers_name = f.substr(first, last-first);
                cout << string( prefix, ' ' ) << f << "(" << FnLocations.at( vers_name ) << ")" << endl;
            }
        }

        auto p_p = ProdToProd.find( p_prodDef );
        if ( p_p != ProdToProd.end() )
        {
            for ( auto p : p_p->second )
            {
                string prodDef = FindProdDef( p_table, p );
                if ( ! prodDef.empty() )
                {
                    //cout << string( prefix, ' ' )  << prodDef << "(" << ProdToFn.locations.at( prodDef ) << "):" << endl;
                    PrintProductionCallTree( prefix + 3, p_table, prodDef );
                }
            }
        }
    }

    void
    PrintColumnCallTree( size_t prefix, const string& p_table, const string& p_column  ) const
    {
        auto c_f = ColToFn.find( p_column );
        if ( c_f != ColToFn.end() )
        {
            for ( auto f : c_f->second )
            {
                cout << string( prefix, ' ' ) << f << endl;
            }
        }

        auto c_p = ColToProd.find( p_column );
        if ( c_p != ColToProd.end() )
        {
            for ( auto p : c_p->second )
            {
                string prodDef = FindProdDef( p_table, p );
                if ( ! prodDef.empty() )
                {
                    //cout << string( prefix, ' ' )  << prodDef << "(" << ProdToFn.locations.at( prodDef ) << "):" << endl;
                    PrintProductionCallTree( prefix + 3, p_table, prodDef );
                }
            }
        }
    }

    set<string> CollectProductionCalls( const string & prod ) const
    {
        set<string> ret;
        auto p = ProdToFn.find( prod );
        if ( p != ProdToFn.end() )
        {
            cout<< prod << ": " << p->second.size() << " calls" <<endl;

            for ( auto i : p->second )
            {
                ret.insert(i);
            }
        }
        else
        {
            cout << "prod not found 1: " << prod << endl;
        }

        p = ProdToProd.find( prod );
        if ( p != ProdToProd.end() )
        {
            cout<< prod << ": " << p->second.size() << " prods" <<endl;

            for ( auto i : p->second )
            {
                auto r = CollectProductionCalls( i );
                cout<< prod << "(prod): collecting " << r.size() << " prods" <<endl;
                ret.insert( r.begin(), r.end() );
            }
        }
        else
        {
            cout << "prod not found 2: " << prod << endl;
        }
        return ret;
    }

    set<string> CollectColumnCalls( const string & col ) const
    {
        set<string> ret;
        auto c = ColToFn.find( col );
        if ( c != ColToFn.end() )
        {
            cout<< col << ": " << c->second.size() << " calls" <<endl;

            for ( auto i : c->second )
            {
                ret.insert(i);
            }
        }
        else
        {
            cout << "column not found 1: " << col << endl;
        }
        c = ColToProd.find( col );
        if ( c != ColToProd.end() )
        {
            cout<< col << ": " << c->second.size() << " prods" <<endl;
            for ( auto p : c->second )
            {
                auto r = CollectProductionCalls( p );
                cout<< col << "(col): collecting " << r.size() << " prods" <<endl;
                ret.insert( r.begin(), r.end() );
            }
        }
        else
        {
            cout << "column not found 2: " << col << endl;
        }
        return ret;
    }
};

AstMap astMap;




string FunctionCallSignature( const AST& node )
{
    assert( node.GetTokenType() == PT_FUNCEXPR );
    assert( node.ChildrenCount() == 4 );
    // 0:schema_parms_opt 1:fqn_opt_vers 2:factory_parms_opt 3:func_parms_opt
    auto fqn = ToFQN( node.GetChild(1) );
    assert( fqn );
    string ret = GetVersionedName( *fqn );

    if ( !astMap.FnWhiteList.empty() &&
         astMap.FnWhiteList.find( ret ) == astMap.FnWhiteList.end() )
    {   // ignore
        //cout << "ignoring " << ret << endl;
        return string();
    }
    else
    {
        //cout << "processing " << ret << endl;
    }

    ret += "(";

    auto func_parms = node.GetChild(3);
    size_t fp_count = func_parms->ChildrenCount();
    for ( size_t i = 0; i < fp_count; ++i )
    {
        if ( i > 0 )
        {
            ret += ",";
        }
        // allowed tags: PT_AT, PHYSICAL_IDENTIFIER_1_0, PT_CAST, PT_IDENT, PT_MEMBEREXPR
        auto param = func_parms->GetChild( i );
        switch ( param->GetTokenType() )
        {
        case PT_IDENT:
            ret += GetFullName( param->GetChild(0) );
            break;
        case '@':
            ret += "@";
            break;
        case PHYSICAL_IDENTIFIER_1_0:
            ret += param->GetTokenValue();
            break;
        case PT_CASTEXPR:
        case PT_MEMBEREXPR:
        default:
            assert(false);
        }
    }
    return ret + ")";
}


FIXTURE_TEST_CASE(DatabaseToTable, AST_Fixture)
{   // discover dependencies of databases on tables (closure through inheritance)
    AST * root = MakeAst  ( R"(
        table T3#1 {}
        table T4#1 {}
        table T5#1 {}
        table T6#1 {}

        table T1 #2 =
            T3 #1,
            T4 #1
            {}
        table T1 #3 =
            T5 #1,
            T6 #1
            {}
        table T2 #4 =
            T3 #1,
            T5 #1
            {}
        database DB1 #2
        {
            table T1 #3 t1;
            table T2 #4 t2;
        };
        database DB2 #1 = DB1#2
        {   // T1, T2, T3
            table T3 #1 t3;
        }
    )" );

//root -> traverse( pre_Json, post_Json );
//cout << jsonStr.str() << endl;

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

    REQUIRE_EQ( 2, (int)astMap.DbToTbl.size());
//astMap.DbToTbl.print("DbToTbl");
    // {   // DB1: T1, T2
    //     auto d1 = astMap.DbToTbl.find("DB1", VTRANSVERS( 2, 0, 0 ) );
    //     REQUIRE_EQ( 2, (int)d1->second.size());
    //     auto m = d1->second;
    //     REQUIRE( m.end() != m.find( string("T1#3") ) );
    //     REQUIRE( m.end() != m.find( string("T2#4") ) );
    // }

    // {   // DB2: T1, T2, T3
    //     auto d2 = astMap.DbToTbl.find("DB2", VTRANSVERS( 1, 0, 0 ) );
    //     REQUIRE_EQ( 3, (int)d2->second.size());
    //     auto m = d2->second;
    //     REQUIRE( m.end() != m.find( string("T1#3") ) );
    //     REQUIRE( m.end() != m.find( string("T2#4") ) );
    //     REQUIRE( m.end() != m.find( string("T3#1") ) );
    // }
}

FIXTURE_TEST_CASE(TableToColumns, AST_Fixture)
{   // discover dependencies of tables on columns (closure through inheritance; proper version resolution)
    AST * root = MakeAst  ( R"(
        table T3#1.0.1 { column ascii t3_1_1; column ascii t3_1_2; }
        table T3#2.1.1 { column ascii t3_2_1; column ascii t3_2_2; }
        table T4#1 { column ascii t4_1; column ascii t4_2;}
        table T1 #1 =
            T3 #2,
            T4 #1
            { column ascii t1_1; }
    )" );

// root -> traverse( pre_Json, post_Json );
// cout << jsonStr.str() << endl;

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

    REQUIRE_EQ( 4, (int)astMap.TblToCol.size());

    {   // T1: t3_1, t3_2, t4_1, t4_2, t1_1
        //astMap.TblToCol.print("TblToCol");
        auto t = astMap.TblToCol.find("T1", VTRANSVERS(1, 0, 0) );
        REQUIRE( astMap.TblToCol.end() != t );
        REQUIRE_EQ( 5, (int)t->second.size());
        auto m = t->second;

        // these 2 columns come from T3#1.0.1 referred to as T3#1 in the parent list
        REQUIRE( m.end() != m.find( string("T3#2.1.1.t3_2_1") ) );
        REQUIRE( m.end() != m.find( string("T3#2.1.1.t3_2_2") ) );

        REQUIRE( m.end() != m.find( string("T4#1.t4_1") ) );
        REQUIRE( m.end() != m.find( string("T4#1.t4_2") ) );

        REQUIRE( m.end() != m.find( string("T1#1.t1_1") ) );
    }
}

FIXTURE_TEST_CASE(ProdDefs, AST_Fixture)
{   // keep track of production definition points
    AST * root = MakeAst  ( R"(
        table T3#1.0.1 { ascii p3_1 = 1; }
        table T3#2.1.1 { ascii p3_2 = 2; }
        table T4#1 { ascii p4 = 3; }
        table T1 #1 =
            T3 #2,
            T4 #1
            { ascii p1 = 4; }
    )" );
    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

    REQUIRE_EQ( 4, (int)astMap.ProdDefs.size());
    const auto & prods = astMap.ProdDefs;
    REQUIRE( prods.end() != prods.find( string("T3#1.0.1.p3_1") ) );
    REQUIRE( prods.end() != prods.find( string("T3#2.1.1.p3_2") ) );
    REQUIRE( prods.end() != prods.find( string("T4#1.p4") ) );
    REQUIRE( prods.end() != prods.find( string("T1#1.p1") ) );
}

FIXTURE_TEST_CASE(TablesToProductions, AST_Fixture)
{   // discover dependencies of tables on productions (closure through inheritance)
    AST * root = MakeAst  ( R"(
        table T3#1.0.1 { ascii p3_1 = 1; }
        table T3#2.1.1 { ascii p3_2 = 2; }
        table T4#1 { ascii p4 = 3; }
        table T1 #1 =
            T3 #2,
            T4 #1
            { ascii p1 = 4; }
    )" );

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

    REQUIRE_EQ( 4, (int)astMap.TblToProd.size());
    auto prods = astMap.TblToProd.find( "T1", VTRANSVERS(1, 0, 0) )->second;
    REQUIRE_EQ( 3, (int)prods.size());
    REQUIRE( prods.end() != prods.find( string("T3#2.1.1.p3_2") ) );
    REQUIRE( prods.end() != prods.find( string("T4#1.p4") ) );
    REQUIRE( prods.end() != prods.find( string("T1#1.p1") ) );
}

FIXTURE_TEST_CASE(ProductionsToProductions, AST_Fixture)
{   // for a production, a closure of all productions it depends on
    AST * root = MakeAst  ( R"(
        table T1 #1 {
            ascii p1 = 1;
            ascii p2 = 2;
            ascii p3 = p1 | p2;
            ascii p4 = p3 | p2;
            column ascii t1_1 = p4;
        }
    )" );

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

// root -> traverse( pre_Json, post_Json );
// cout << jsonStr.str() << endl;
// astMap.ProdToProd.print("ProdToProd");

    REQUIRE_EQ( 4, (int)astMap.ProdToProd.size());

    auto prods = astMap.ProdToProd.find( "T1#1.p1" )->second;
    REQUIRE_EQ( 1, (int)prods.size());

    prods = astMap.ProdToProd.find( "T1#1.p4" )->second;
    REQUIRE( prods.end() != prods.find( string("T1#1.p1") ) );
    REQUIRE( prods.end() != prods.find( string("T1#1.p2") ) );
    REQUIRE( prods.end() != prods.find( string("T1#1.p3") ) );
}

FIXTURE_TEST_CASE(ColumnsToProductions, AST_Fixture)
{   // for a column, a closure of all productions it depends on
    AST * root = MakeAst  ( R"(
        table T1 #1 {
            ascii p1 = 1;
            ascii p2 = 2;
            ascii p3 = p1 | p2;
            column ascii t1_1 = p3;
        }
    )" );

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

// root -> traverse( pre_Json, post_Json );
// cout << jsonStr.str() << endl;
// astMap.ColToProd.print("ColToProd");

    REQUIRE_EQ( 1, (int)astMap.ColToProd.size());
    auto prods = astMap.ColToProd.begin()->second;
    REQUIRE( prods.end() != prods.find( string("T1#1.p1") ) );
    REQUIRE( prods.end() != prods.find( string("T1#1.p2") ) );
    REQUIRE( prods.end() != prods.find( string("T1#1.p3") ) );
}

FIXTURE_TEST_CASE(ColumnsToInheritedProductions, AST_Fixture)
{   // for a column, a closure of all productions it depends on
    AST * root = MakeAst  ( R"(
        table T1 #1 {
            ascii p1 = 1;
        }
        table T2 #1 = T1 #1 {
            column ascii t2_1 = p1;
        }
    )" );

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

// root -> traverse( pre_Json, post_Json );
// cout << jsonStr.str() << endl;
 astMap.ColToProd.print("ColToProd");

    REQUIRE_EQ( 1, (int)astMap.ColToProd.size());
    auto prods = astMap.ColToProd.begin()->second;
    REQUIRE( prods.end() != prods.find( string("T1#1.p1") ) );
}

FIXTURE_TEST_CASE(ColumnsToProductionsToFunctionCalls, AST_Fixture)
{   // for a column, a set of all function calls it can invoke
    AST * root = MakeAst  ( R"(
        function ascii fn1 #1.0( ascii a , ascii b );
        function ascii fn2 #1.0( ascii a , ascii b );

        table T1 #1 {
            ascii p1 = 1;
            ascii p2 = fn1( p1 );
            ascii p3 = fn2( p1, p2 );
            column ascii t1_1 = p3;
            column ascii t1_2 = fn1( .t1_1 );
        }
    )" );

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

// root -> traverse( pre_Json, post_Json );
// cout << jsonStr.str() << endl;
//  astMap.ProdToFn.print("ProdToFn");
//  astMap.ColToProd.print("ColToProd");
//  astMap.ColToFn.print("ColToFn");

    REQUIRE_EQ( 2, (int)astMap.ColToFn.size());
    REQUIRE_EQ( string("T1#1.t1_1"), astMap.ColToFn.begin()->first );
    auto fns = astMap.ColToFn.begin()->second;
    REQUIRE( fns.end() != fns.find( string("T1#1.fn1#1(p1)") ) );
    REQUIRE( fns.end() != fns.find( string("T1#1.fn2#1(p1,p2)") ) );
}

FIXTURE_TEST_CASE(CollectColumnsCalls, AST_Fixture)
{   // for a column, a set of all function calls it can invoke
    AST * root = MakeAst  ( R"(
        function ascii fn1 #1.0( ascii a , ascii b );
        function ascii fn2 #1.0( ascii a , ascii b );

        table T1 #1 {
            ascii p1 = 1;
            ascii p2 = fn1( p1 );
            ascii p3 = fn2( p1, p2 );
        };

        table T2 #1 = T1#1 {
            column ascii t1_1 = p3;
            column ascii t1_2 = fn1( .t1_1 );
        }
    )" );

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

// root -> traverse( pre_Json, post_Json );
// cout << jsonStr.str() << endl;
 astMap.ProdToFn.print("ProdToFn");
 astMap.ColToProd.print("ColToProd");
 astMap.ColToFn.print("ColToFn");

    REQUIRE_EQ( 2, (int)astMap.ColToFn.size());
    REQUIRE_EQ( string("T2#1.t1_1"), astMap.ColToFn.begin()->first );
    auto fns = astMap.ColToFn.begin()->second;
    REQUIRE( fns.end() != fns.find( string("fn1#1(p1)") ) );
    REQUIRE( fns.end() != fns.find( string("fn2#1(p1,p2)") ) );

    auto calls = astMap.CollectColumnCalls(string("T2#1.t1_1") );
    for ( auto i : calls )
    {
        cout << "         " << i << endl;
    }
}

FIXTURE_TEST_CASE(ProductionsToColumns, AST_Fixture)
{   // for a production, a set of all columns it depends on
    AST * root = MakeAst  ( R"(
        table T1 #1 {
            ascii p1 = t1_1;
            column ascii t1_1;
        }
    )" );

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

// root -> traverse( pre_Json, post_Json );
// cout << jsonStr.str() << endl;
// astMap.ProdToFn.print("ProdToFn");
// astMap.ColToProd.print("ColToProd");
// astMap.ColToFn.print("ColToFn");
    FAIL( "not implemented" );
}

FIXTURE_TEST_CASE(VDB_6444, AST_Fixture)
{   // discover dependencies of columns in a given database/table on schema function calls
    AST * root = MakeAst  ( "version 2; include 'align/align.vschema';" );

    astMap = AstMap();
    astMap.FnWhiteList = {
        "NCBI:align:get_mate_align_id#1",
        "NCBI:align:ref_restore_read#1",
        "NCBI:align:cigar#1",
        "NCBI:align:cigar#2",
        "NCBI:align:edit_distance#1",
        "NCBI:align:edit_distance#2",
        "NCBI:align:generate_has_mismatch#1",
        "NCBI:align:generate_mismatch#1",
        "NCBI:align:get_left_soft_clip#2",
        "NCBI:align:get_clipped_cigar#2",
        "NCBI:align:get_ref_len#1",
        "NCBI:align:clip#2",
        "NCBI:align:ref_sub_select#1",
        "NCBI:align:seq_restore_read#1",
        "NCBI:align:align_restore_read#1",
        "NCBI:align:project_from_sequence#1",
        "NCBI:align:seq_construct_read#1",
        "NCBI:align:ref_name#1",
        "NCBI:fp_extend#1",
        "NCBI:SRA:extract_name_fmt#1",
        "NCBI:dna_from_color#1",
        "NCBI:SRA:bio_end#1",
        "outlier_encode#1",
        "strtonum#1",
        "sprintf#1",
        "rldecode#1",
        "checksum#1",
        "map#1",
        "rlencode#1",
    };

    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

   // REQUIRE_EQ( 216, (int)astMap.ProdToFn.size() );
//   REQUIRE_EQ( 66, (int)astMap.ColToFn.size() );
//   REQUIRE_EQ( 157, (int)astMap.ColToProd.size() );

    // astMap.DbToTbl.print("Db to Tables");
    // astMap.TblToCol.print( "Tables to Columns" );

    // astMap.PrintProdDefs( cout );

    // astMap.ProdToFn.print( "Productions to Functions", true );
    // astMap.ProdToProd.print( "Productions to Production", true );
    // astMap.ColToProd.print( "Columns to Productions", true );
    // astMap.ColToFn.print( "Columns to Functions", true );

    const auto DB = "NCBI:align:db:alignment_unsorted#2";
    const auto Tbl = "NCBI:align:tbl:seq#2";
    const auto Col = "INSDC:tbl:sequence#1.0.1.READ";
    //const auto Col = "INSDC:SRA:tbl:spotcoord#1.X";

    const auto d = astMap.DbToTbl.find( DB );
    if ( d != astMap.DbToTbl.end() )
    {
        cout << "Database " << DB << "(" << astMap.DbToTbl.locations.at( d->first ) << "):" << endl;
        auto t = astMap.TblToCol.find( Tbl );
        if ( t != astMap.TblToCol.end() )
        {
            cout << "   Table " << t->first << "(" << astMap.TblToCol.locations.at( t->first ) << "):" << endl;

            //for ( auto c : t->second )
            auto c_it = t->second.find( Col );
            auto c = *c_it;
            if ( c_it != t->second.end() )

            {
                cout << "      Column " << c << "(" << astMap.ColToProd.locations.at( c ) << "):" << endl;

                auto calls = astMap.CollectColumnCalls( c );
                for ( auto i : calls )
                {
                    cout << "         " << i << endl;
                }
                cout << endl;
                //astMap.PrintColumnCallTree( 9, t->first, c );
            }
        }
        else
        {
            cout << Tbl << " is not found in TblToCol" << endl;
        }
    }
    else
    {
        cout << DB << " is not found in DbToTbl" << endl;
    }
}
#endif

//////////////////////////////////////////// Main
int main( int argc, char *argv [] )
{
    return SchemaInfoTestSuite(argc, argv);
}
