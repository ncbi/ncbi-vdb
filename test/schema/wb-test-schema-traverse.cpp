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

#include <ktst/unit_test.hpp>

#include <kfc/defs.h>

#include <klib/printf.h>

#include <vdb/xform.h>

#include <map>
#include <set>
#include <sstream>


using namespace std;
using namespace ncbi::NK;

TEST_SUITE ( SchemaASTTraversalTestSuite );

// AST

FIXTURE_TEST_CASE(Traverse_noop, AST_Fixture)
{   // does not crash w/o callbacks
    AST * root = MakeAst  ( "table t#1 { column U8 a = 1|2|3; } " );
    root -> traverse( nullptr, nullptr );
}

size_t decimal_counter = 0;
void countDecimals( const ParseTree& node )
{
    auto& ast_node = dynamic_cast< const AST& >( node );
    if ( ast_node . GetTokenType() == DECIMAL  )
    {
        ++ decimal_counter;
    }
}

FIXTURE_TEST_CASE(CondExpr, AST_Fixture)
{
    AST * root = MakeAst  ( "table t#1 { column U8 a = 1|2|3; } " );

    decimal_counter = 0;
    root -> traverse( countDecimals );

    REQUIRE_EQ( 3, (int)decimal_counter );
}

ostringstream jsonStr;
size_t indent = 0;
const size_t IndentUnit = 2;
string prefix()
{
    return string( IndentUnit * indent, ' ' );
}

void pre_Json( const ParseTree& node )
{
    jsonStr << prefix() << "{" << endl;
    ++indent;
    jsonStr << prefix() << "'type' : '" << AST::TokenTypeToString( node.GetToken().GetType() ) << "(" << node.GetToken().GetType() << ")'," << endl;
    if ( !string(node.GetToken().GetValue()).empty() )
    {
        jsonStr << prefix() << "'value' : '" << node.GetToken().GetValue() << "'," << endl;
    }

    const AST_FQN * fqn = dynamic_cast<const AST_FQN*>(&node);
    if ( fqn != nullptr )
    {
        ver_t v = fqn->GetVersion();
        if ( v != 0 )
        {
            jsonStr << prefix() << "'version' : '"
                << VersionGetMajor( v ) << ":"
                << VersionGetMinor( v ) << ":"
                << VersionGetRelease( v )
                << "'," << endl;
        }
    }

    if ( node.ChildrenCount() > 0 )
    {
        jsonStr << prefix() << "'childrenCount' : '" << node.ChildrenCount() << "'," << endl;

        jsonStr << prefix() << "'children' : [" << endl;
        ++indent;
    }
}
void post_Json( const ParseTree& node )
{
    if ( node.ChildrenCount() > 0 )
    {
        --indent;
        jsonStr << prefix() << "]" << endl;
    }
    --indent;
    jsonStr << prefix() << "}" << endl;
}

FIXTURE_TEST_CASE(ToJson, AST_Fixture)
{
    AST * root = MakeAst  ( "table t#1 { column U8 a = 1|2|3; } " );

    root -> traverse( pre_Json, post_Json );

    //cout << jsonStr.str();
    REQUIRE_NE( string(), jsonStr.str() );
}

FIXTURE_TEST_CASE(ToJson_debug, AST_Fixture)
{
    AST * root = MakeAst  ( R"(function < type T, U32 dim >
T [ dim ] vclip #1.0 < T lower, T upper > ( T [ dim ] in )
    = vdb:clip;
    )");

    root -> traverse( pre_Json, post_Json );

    //cout << jsonStr.str();
    REQUIRE_NE( string(), jsonStr.str() );
}

class NameMap : public map<string, set<string> >
{
public:
    void add( const string& key )
    {
        this->insert( make_pair( key, set<string>() ) );
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
                cout << endl;
                cout << i->first << ": ";
                for( auto j = i->second.begin(); j != i->second.end(); ++j )
                {
                    cout << endl << "   " << *j;
                }
            }
        }
        cout << endl;
    }
};

class VersionedNameMap : public NameMap
{
    public:
        // only allow adding versioned names
        void add( const string& name, ver_t version )
        {
            char buf[1024];
            string_printf ( buf, sizeof( buf ), nullptr, "%s#%V", name.c_str(), version );
            NameMap::add( buf );
            nameToVersions[name].insert(version);
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
            return NameMap::find( string( buf ) );
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

    string activeCol;
    NameMap ColToFn;
    NameMap ColToProd;

    string activeTable;
    VersionedNameMap TblToProd;
    VersionedNameMap TblToCol;

    string activeDatabase;
    VersionedNameMap DbToTbl;
};

AstMap astMap;

string GetFullName ( const AST* node )
{
    switch( node -> GetTokenType() )
    {
    case PT_IDENT:
        {
            auto fqn = ToFQN( node );
            if ( fqn )
            {
                char buf[1024];
                fqn -> GetFullName( buf, sizeof( buf ) );
                return buf;
            }
            else
            {
                assert( node->ChildrenCount() == 1 );
                auto id = node->GetChild(0);
                assert( id->GetTokenType() == IDENTIFIER_1_0 );
                return id->GetTokenValue();
            }
        }
    case IDENTIFIER_1_0:
        {
            return node->GetTokenValue();
        }
    default:
        {
            ostringstream s;
            s << "GetFullName(): unexpected tag " << node -> GetTokenType();
            FAIL( s.str() );
        }
    }
    return "";
}

string GetVersionedName ( const AST_FQN& node )
{
    char buf[1024];
    node . GetVersionedName( buf, sizeof( buf ) );
    return buf;
}

void pre_columnToFunctions( const ParseTree& node )
{
    auto& ast_node = dynamic_cast< const AST& >( node );
    switch( ast_node . GetTokenType() )
    {
    case PT_DATABASE:
        {   // database definition; TODO: support nested databases
            string name = GetFullName( ast_node.GetChild(0) );
            auto fqn = ToFQN( ast_node.GetChild(0) );
            assert( fqn );
            auto vers_name = GetVersionedName( *fqn );
            //cout << "database " << name << endl;
            astMap.activeDatabase = vers_name;

            astMap.DbToTbl.add(name, fqn->GetVersion());
            auto dad_node = ast_node.GetChild(1);
            if ( dad_node->GetTokenType() != PT_EMPTY )
            { // add parent
                string dad = GetFullName( dad_node );
                auto dad_ver = ToFQN( dad_node )->GetVersion();
                if( astMap.DbToTbl.find(dad, dad_ver ) == astMap.DbToTbl.end() )
                {
                    throw logic_error( string("database ") + name + " parent not found: " + dad );
                }
                auto d = astMap.DbToTbl.find( dad, dad_ver );
                assert( d != astMap.DbToTbl.end() );
                astMap.DbToTbl[vers_name].insert( d->second.begin(), d->second.end() );
            }
            break;
        }
    case PT_TBLMEMBER:
        {   // database member table. record the type of the table, not its name in the DB
            auto fqn = ToFQN( ast_node.GetChild(1) ); // type
            assert( fqn );
            astMap.DbToTbl.NameMap::add( astMap.activeDatabase, GetVersionedName( *fqn ) );
            break;
        }
    case PT_TABLE:
        {   // table definition.
            string name = GetFullName( ast_node.GetChild(0) );
            auto fqn = ToFQN( ast_node.GetChild(0) );
            assert( fqn );
            auto vers_name = GetVersionedName( *fqn );
            //cout << "table " << vers_name << endl;
            astMap.activeTable = vers_name;

            assert( fqn );
            astMap.TblToCol.add( name, fqn->GetVersion() );
            astMap.TblToProd.add( name, fqn->GetVersion() );
            auto plist = ast_node.GetChild(1);
            if ( plist->GetTokenType() == PT_TABLEPARENTS )
            { // add parents
                for ( uint32_t i = 0; i < plist->ChildrenCount(); ++i )
                {   //TODO: look for a definition with the correct version
                    string dad = GetFullName( plist->GetChild(i) );
                    auto dad_ver = ToFQN( plist->GetChild(i) )->GetVersion();
                    if( astMap.TblToCol.find(dad, dad_ver ) == astMap.TblToCol.end() )
                    {
                        throw logic_error( string("table ") + name + " parent not found: " + dad );
                    }
                    auto d = astMap.TblToCol.find( dad, dad_ver )->second;
                    astMap.TblToCol[vers_name].insert( d.begin(), d.end() );
                    auto p = astMap.TblToProd.find( dad, dad_ver )->second;
                    astMap.TblToProd[vers_name].insert( p.begin(), p.end() );
                }
            }
            else
            {
                assert( plist->GetTokenType() == PT_EMPTY );
            }
            break;
        }

    case PT_TYPEDCOL:
    case PT_TYPEDCOLEXPR:
        {   // column definition
            assert( ast_node.ChildrenCount() >= 1 );
            assert( ast_node.GetChild(0)->GetTokenType() == PT_IDENT );
            string col_name = astMap.activeTable + "." + GetFullName( ast_node.GetChild(0) );
            // cout << "column " << col_name << endl;
            astMap.activeCol = col_name;
            astMap.ColToFn.add( col_name );
            astMap.ColToProd.add( col_name );
            astMap.TblToCol.NameMap::add( astMap.activeTable, col_name );
            break;
        }
    case PT_FUNCEXPR:
        {   // function call
            string name = GetFullName( ast_node.GetChild(1) );
            //cout << "function " << name << endl;
            if ( !astMap.activeCol.empty() )
            {
                astMap.ColToFn.add( astMap.activeCol, name );
            }
            else if ( !astMap.activeProd.empty() )
            {
                astMap.ProdToFn.add( astMap.activeProd, name );
            }
            break;
        }
    case PT_PRODSTMT:
        {   // production
            if ( ! astMap.activeTable.empty() )
            {
                string name = astMap.activeTable + "." + GetFullName( ast_node.GetChild(1) );
                astMap.activeProd = name;
                astMap.ProdToFn.add( name );
                astMap.ProdToProd.add( name );
                //cout << "adding production " << name << " to " << astMap.activeTable << endl;
                astMap.TblToProd.NameMap::add( astMap.activeTable, name );
            }
            break;
        }

    case PT_IDENT:
        {   // use of an identifier
            //string name = ast_node.GetChild(0)->GetTokenValue();
            string name = astMap.activeTable + "." + GetFullName( ast_node.GetChild(0) );

            if ( ! astMap.activeProd.empty() )
            {   // in a production
                //cout << astMap.activeProd << " " << name << endl;
                if ( name != astMap.activeProd )
                {
                    astMap.ProdToProd.add( astMap.activeProd, name );

                    auto p = astMap.ProdToProd.find( name );
                    if ( p != astMap.ProdToProd.end() )
                    {
                        astMap.ProdToProd[astMap.activeProd].insert( p->second.begin(), p->second.end() );
                    }
                    p = astMap.ProdToFn.find( name );
                    if ( p != astMap.ProdToFn.end() )
                    {
                        astMap.ProdToFn[astMap.activeProd].insert( p->second.begin(), p->second.end() );
                    }
                }
            }
            else if ( ! astMap.activeCol.empty() )
            {   // in a column definition
                //cout << astMap.activeProd << " " << name << endl;
                if ( name != astMap.activeCol )
                {
                    astMap.ColToProd.add( astMap.activeCol, name );

                    auto p = astMap.ProdToProd.find( name );
                    if ( p != astMap.ProdToProd.end() )
                    {
                        astMap.ColToProd[astMap.activeCol].insert( p->second.begin(), p->second.end() );
                    }
                    p = astMap.ProdToFn.find( name );
                    if ( p != astMap.ProdToFn.end() )
                    {
                        astMap.ColToFn[astMap.activeCol].insert( p->second.begin(), p->second.end() );
                    }
                }
            }
            else
            {
                //cout << "ident " << name << endl;
            }

            break;
        }

    default:
        break;
    }
}

void post_columnToFunctions( const ParseTree& node )
{
    auto& ast_node = dynamic_cast< const AST& >( node );
    switch( ast_node . GetTokenType() )
    {
    case PT_DATABASE:
        {
            //cout << "end database " << GetFullName( ast_node.GetChild(0) ) << endl;
            astMap.activeDatabase.clear();
            break;
        }

    case PT_TABLE:
        {
            astMap.activeTable.clear();
            break;
        }

    case PT_TYPEDCOL:
    case PT_TYPEDCOLEXPR:
        {
            //cout << "end column " << GetFullName( ast_node.GetChild(0) ) << endl;
            astMap.activeCol.clear();
            break;
        }
    case PT_PRODSTMT:
        {
            // cout << "end production " << GetFullName( ast_node.GetChild(1) ) << endl;
            astMap.activeProd.clear();
            break;
        }

    default:
        break;
    }
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

    {   // DB1: T1, T2
        auto d1 = astMap.DbToTbl.find("DB1", VTRANSVERS( 2, 0, 0 ) );
        REQUIRE_EQ( 2, (int)d1->second.size());
        auto m = d1->second;
        REQUIRE( m.end() != m.find( string("T1#3") ) );
        REQUIRE( m.end() != m.find( string("T2#4") ) );
    }

    {   // DB2: T1, T2, T3
        auto d2 = astMap.DbToTbl.find("DB2", VTRANSVERS( 1, 0, 0 ) );
        REQUIRE_EQ( 3, (int)d2->second.size());
        auto m = d2->second;
        REQUIRE( m.end() != m.find( string("T1#3") ) );
        REQUIRE( m.end() != m.find( string("T2#4") ) );
        REQUIRE( m.end() != m.find( string("T3#1") ) );
    }
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

FIXTURE_TEST_CASE(ColumnsToProductionsToFunctions, AST_Fixture)
{   // for a column, a closure of all productions it depends on
    AST * root = MakeAst  ( R"(
        function ascii vclip #1.0() = vdb:clip;

        table T1 #1 {
            ascii p1 = vclip();
            ascii p2 = 2;
            ascii p3 = p1 | p2;
            column ascii t1_1 = p3;
        }
    )" );

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

// root -> traverse( pre_Json, post_Json );
// cout << jsonStr.str() << endl;
//  astMap.ProdToFn.print("ProdToFn");
//  astMap.ColToProd.print("ColToProd");
//  astMap.ColToFn.print("ColToFn");

    REQUIRE_EQ( 1, (int)astMap.ColToFn.size());
    auto fns = astMap.ColToFn.begin()->second;
    REQUIRE( fns.end() != fns.find( string("vclip") ) );
}

FIXTURE_TEST_CASE(VDB_6444, AST_Fixture)
{   // discover dependencies of columns on schema functions
    AST * root = MakeAst  ( "version 2; include 'align/align.vschema';" );

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

   // REQUIRE_EQ( 216, (int)astMap.ProdToFn.size() );
//   REQUIRE_EQ( 66, (int)astMap.ColToFn.size() );
//   REQUIRE_EQ( 157, (int)astMap.ColToProd.size() );

    astMap.ProdToFn.print( "Productions to Functions", true );
    astMap.ColToProd.print( "Columns to Productions", true );
    astMap.ColToFn.print( "Columns to Functions", true );
//    astMap.TblToCol.print( "Tables to Columns" );
//    astMap.DbToTbl.print("Db to Tables");
//   astMap.ProdToProd.print( "Productions to Productions" );

}

//////////////////////////////////////////// Main
int main( int argc, char *argv [] )
{
    return SchemaASTTraversalTestSuite(argc, argv);
}
