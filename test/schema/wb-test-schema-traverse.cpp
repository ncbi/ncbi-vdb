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

using namespace std;
using namespace ncbi::NK;

TEST_SUITE ( SchemaASTTraversalTestSuite );

// AST

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

//////////////////////////////////////////// Main
int main( int argc, char *argv [] )
{
    return SchemaASTTraversalTestSuite(argc, argv);
}
