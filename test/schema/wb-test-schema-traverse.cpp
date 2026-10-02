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
    jsonStr << prefix() << "'location' : " << LocationToString( node.GetToken().GetLocation() ) << "," << endl;

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
    // drop your schema here:
    AST * root = MakeAst  ( R"(
        table T1#1 {}
        table T2#2 {}
        table T3 #3 = T1#1, T2#2{
            U8 p = 1;
            column U8 c = p;
        }
    )" );

    root -> traverse( pre_Json, post_Json );

    REQUIRE_NE( string(), jsonStr.str() );

    // uncomment the following line to see the Json
    // cout << jsonStr.str();
}

//////////////////////////////////////////// Main
int main( int argc, char *argv [] )
{
    return SchemaASTTraversalTestSuite(argc, argv);
}
