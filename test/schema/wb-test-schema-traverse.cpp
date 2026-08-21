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

    cout << jsonStr.str();
    //REQUIRE_NE( string(), jsonStr.str() );
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

string activeProd;
NameMap ProdToFn;

string activeCol;
NameMap ColToFn;
NameMap ColToProd;

string activeTable;

string activeDatabase;

string GetFullName ( const AST* node )
{
    switch( node -> GetTokenType() )
    {
    case PT_IDENT:
        {
            auto fqn = ToFQN( node );
            char buf[1024];
            fqn -> GetFullName( buf, sizeof( buf ) );
            return buf;
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
        {
            //cout << "database " << GetFullName( ast_node.GetChild(0) ) << endl;
            break;
        }

    case PT_TYPEDCOL:
    case PT_TYPEDCOLEXPR:
        {
            string name = GetFullName( ast_node.GetChild(0) );
            activeCol = name;
            ColToFn.add( name );
            //cout << "column " << name << endl;
            break;
        }
    case PT_FUNCEXPR:
        {
            string name = GetFullName( ast_node.GetChild(1) );
            //cout << "function " << name << endl;
            if ( !activeCol.empty() )
            {
                ColToFn.add( activeCol, name );
            }
            else if ( !activeProd.empty() )
            {
                ProdToFn.add( activeProd, name );
            }
            break;
        }
    case PT_PRODSTMT:
        {
            string name = GetFullName( ast_node.GetChild(1) );
            activeProd = name;
            ProdToFn.add( name );
            //cout << "production " << name << endl;
            break;
        }

    case PT_IDENT:
        {
            string name = GetFullName( & ast_node );
            cout << "ident " << name << endl;
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
            break;
        }

    case PT_TYPEDCOL:
    case PT_TYPEDCOLEXPR:
        {
            //cout << "end column " << GetFullName( ast_node.GetChild(0) ) << endl;
            activeCol.clear();
            break;
        }
    case PT_PRODSTMT:
        {
            //cout << "end production " << GetFullName( ast_node.GetChild(1) ) << endl;
            activeProd.clear();
            break;
        }

    default:
        break;
    }
}

FIXTURE_TEST_CASE(VDB_6444, AST_Fixture)
{   // discover dependencies of columns on schema functions
    AST * root = MakeAst  ( "version 2; include 'align/align.vschema';" );

    root -> traverse( pre_columnToFunctions, post_columnToFunctions );

    REQUIRE_EQ( 216, (int)ProdToFn.size() );
    REQUIRE_EQ( 157, (int)ColToFn.size() );
//    REQUIRE_EQ( 157, (int)ColToProd.size() );

    // ProdToFn.print( "Productions to Functions" );
    // ColToFn.print( "Columns to Functions" );
}

//////////////////////////////////////////// Main
int main( int argc, char *argv [] )
{
    return SchemaASTTraversalTestSuite(argc, argv);
}
