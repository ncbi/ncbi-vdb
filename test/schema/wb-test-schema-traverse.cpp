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

    void print( const string& header ) const
    {
        cout << endl << header << ":" << endl;
        for( auto i = begin(); i != end(); ++i )
        {
            cout << i->first << ": ";
            for( auto j = i->second.begin(); j != i->second.end(); ++j )
            {
                cout << *j << " ";
            }
            cout << endl;
        }
        cout << endl;
    }
};

struct AstMap
{
    string activeProd;
    NameMap ProdToFn;

    string activeCol;
    NameMap ColToFn;
    NameMap ColToProd;
    NameMap TblToCol;
    NameMap DbToTbl;

    string activeTable;

    string activeDatabase;
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
                fqn -> GetVersionedName( buf, sizeof( buf ) );
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

void pre_columnToFunctions( const ParseTree& node )
{
    auto& ast_node = dynamic_cast< const AST& >( node );
    switch( ast_node . GetTokenType() )
    {
    case PT_DATABASE:
        {   // database definition; nested databases are not yet handled
            string name = GetFullName( ast_node.GetChild(0) );
            //cout << "database " << name << endl;
            astMap.activeDatabase = name;
            astMap.DbToTbl.add(name);
            if ( ast_node.GetChild(1)->GetTokenType() != PT_EMPTY )
            { // add parent
                string dad = GetFullName( ast_node.GetChild(1) );
                auto d = astMap.DbToTbl[dad];
                astMap.DbToTbl[name].insert( d.begin(), d.end() );
            }
            break;
        }
    case PT_TBLMEMBER:
        {   // database member table. record the type of the table, not its name in the DB
            string name = GetFullName( ast_node.GetChild(1) );
            //cout << "table " << name << endl;
            astMap.DbToTbl.add( astMap.activeDatabase, name );
            break;
        }
    case PT_TABLE:
        {   // table definition.
            string name = GetFullName( ast_node.GetChild(0) );
            //cout << "table " << name << endl;
            astMap.activeTable = name;
            astMap.TblToCol.add( name );
            auto plist = ast_node.GetChild(1);
            if ( plist->GetTokenType() == PT_TABLEPARENTS )
            { // add parents
                for ( uint32_t i = 0; i < plist->ChildrenCount(); ++i )
                {   //TODO: look for a definition with the correct version
                    string dad = GetFullName( plist->GetChild(i) );
if( astMap.TblToCol.find(dad) == astMap.TblToCol.end() )
{
cout<<"table "<<name<<" parent not found: "<<dad<<endl;
}
else{
                    auto d = astMap.TblToCol.find(dad)->second;
                    astMap.TblToCol[name].insert( d.begin(), d.end() );
}
                }
            }
            else
            {
                assert( plist->GetTokenType() == PT_EMPTY );
            }
            break;
        }

    case PT_TYPEDCOL:
        {   // column definition
            assert( ast_node.ChildrenCount() >= 1 );
            assert( ast_node.GetChild(0)->GetTokenType() == PT_IDENT );
            string name = GetFullName( ast_node.GetChild(0) );
            astMap.activeCol = name;
            astMap.ColToFn.add( name );
            astMap.TblToCol.add( astMap.activeTable, name );
            //cout << "column " << astMap.activeTable << "." << name << endl;
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
            string name = GetFullName( ast_node.GetChild(1) );
            astMap.activeProd = name;
            astMap.ProdToFn.add( name );
            //cout << "production " << name << endl;
            break;
        }

    case PT_IDENT:
        {   // use of an identifier
            string name = ast_node.GetChild(0)->GetTokenValue();
            //cout << "ident " << name << endl;
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
            //cout << "end production " << GetFullName( ast_node.GetChild(1) ) << endl;
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

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

    REQUIRE_EQ( 2, (int)astMap.DbToTbl.size());

    {   // DB1: T1, T2
        auto d1 = astMap.DbToTbl.find("DB1#2");
        REQUIRE_EQ( 2, (int)d1->second.size());
        auto m = d1->second;
        REQUIRE( m.end() != m.find( string("T1#3") ) );
        REQUIRE( m.end() != m.find( string("T2#4") ) );
    }

    {   // DB2: T1, T2, T3
        auto d2 = astMap.DbToTbl.find("DB2#1");
        REQUIRE_EQ( 3, (int)d2->second.size());
        auto m = d2->second;
        REQUIRE( m.end() != m.find( string("T1#3") ) );
        REQUIRE( m.end() != m.find( string("T2#4") ) );
        REQUIRE( m.end() != m.find( string("T3#1") ) );
    }
}

FIXTURE_TEST_CASE(TableToColumns, AST_Fixture)
{   // discover dependencies of tables on columns (closure through inheritance)
    AST * root = MakeAst  ( R"(
        table T3#1.0.1 { column ascii t3_1; column ascii t3_2; }
        table T4#1 { column ascii t4_1; column ascii t4_2;}
        table T1 #1 =
            T3 #1,
            T4 #1
            { column ascii t1_1; }
    )" );

    // root -> traverse( pre_Json, post_Json );
    // cout << jsonStr.str() << endl;

    astMap = AstMap();
    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

    REQUIRE_EQ( 3, (int)astMap.TblToCol.size());

    {   // T1: t3_1, t3_2, t4_1, t4_2, t1_1
        //astMap.TblToCol.print("TblToCol");
        auto t = astMap.TblToCol.find("T1#1");
        REQUIRE( astMap.TblToCol.end() != t );
        REQUIRE_EQ( 5, (int)t->second.size());
        auto m = t->second;
        REQUIRE( m.end() != m.find( string("t3_1") ) );
        REQUIRE( m.end() != m.find( string("t3_2") ) );
        REQUIRE( m.end() != m.find( string("t4_1") ) );
        REQUIRE( m.end() != m.find( string("t4_2") ) );
        REQUIRE( m.end() != m.find( string("t1_1") ) );
    }
}

FIXTURE_TEST_CASE(VDB_6444, AST_Fixture)
{   // discover dependencies of columns on schema functions
    AST * root = MakeAst  ( "version 2; include 'align/align.vschema';" );

    // astMap = AstMap();
    // root -> traverse( pre_columnToFunctions, post_columnToFunctions );

//    REQUIRE_EQ( 216, (int)ProdToFn.size() );
//    REQUIRE_EQ( 157, (int)ColToFn.size() );
//    REQUIRE_EQ( 157, (int)ColToProd.size() );

//    ProdToFn.print( "Productions to Functions" );
//    ColToFn.print( "Columns to Functions" );
//    ColToProd.print( "Columns to Productions" );
//    astMap.TblToCol.print( "Columns to Productions" );
//    astMap.DbToTbl.print("Db to Tables");

}

//////////////////////////////////////////// Main
int main( int argc, char *argv [] )
{
    return SchemaASTTraversalTestSuite(argc, argv);
}
