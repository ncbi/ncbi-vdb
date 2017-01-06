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
* Unit tests for schema AST
*/

#include <ktst/unit_test.hpp>

#include <sstream>
#include <cstring>

#include <klib/text.h>
#include <klib/symbol.h>
#include <klib/symtab.h>

#include "../../libs/vdb/schema-priv.h"

// hide an unfortunately named C function
#define typename __typename
#include "../../libs/vdb/schema-parse.h"
#undef typename

#include "../../libs/schema/SchemaParser.hpp"
#include "../../libs/schema/ParseTree.hpp"
#include "../../libs/schema/AST.hpp"

using namespace ncbi::SchemaParser;
#include "../../libs/schema/schema-tokens.h"

#define KW_TOKEN(v,k) SchemaToken v = { KW_##k, #k, strlen(#k), 0, 0 }

using namespace std;
using namespace ncbi::NK;

TEST_SUITE ( SchemaASTTestSuite );

// AST

static
bool
VerifyNextToken ( ParseTreeScanner& p_scan, int p_type)
{
    const Token* token;
    return p_scan . NextToken ( token ) == p_type;
}

TEST_CASE(SchemaAST_Construct_Empty)
{
    SchemaParser p;
    REQUIRE ( p . ParseString ( "" ) );
    ParseTree * root = p . MoveParseTree ();
    REQUIRE_NOT_NULL ( root );
    ParseTreeScanner scan ( * root );
    REQUIRE ( VerifyNextToken ( scan, PT_PARSE ) );
    REQUIRE ( VerifyNextToken ( scan, '(' ) );
    REQUIRE ( VerifyNextToken ( scan, SchemaScanner::EndSource ) );
    REQUIRE ( VerifyNextToken ( scan, ')' ) );

    delete root;
}

TEST_CASE(SchemaAST_WalkParseTree)
{
    SchemaParser p;
    REQUIRE ( p . ParseString ( "version 1; include \"qq\";" ) );
    ParseTree * root = p . MoveParseTree ();
    REQUIRE_NOT_NULL ( root );
    ParseTreeScanner scan ( * root );
    REQUIRE ( VerifyNextToken ( scan, PT_PARSE ) );
    REQUIRE ( VerifyNextToken ( scan, '(' ) );

        REQUIRE ( VerifyNextToken ( scan, PT_SOURCE ) );
        REQUIRE ( VerifyNextToken ( scan, '(' ) );

            REQUIRE ( VerifyNextToken ( scan, PT_VERSION_1_0 ) );
            REQUIRE ( VerifyNextToken ( scan, '(' ) );
            REQUIRE ( VerifyNextToken ( scan, KW_version ) );
            REQUIRE ( VerifyNextToken ( scan, VERS_1_0 ) );
            REQUIRE ( VerifyNextToken ( scan, ';' ) );
            REQUIRE ( VerifyNextToken ( scan, ')' ) );

            REQUIRE ( VerifyNextToken ( scan, PT_SCHEMA_1_0 ) );
            REQUIRE ( VerifyNextToken ( scan, '(' ) );

                REQUIRE ( VerifyNextToken ( scan, PT_INCLUDE ) );
                REQUIRE ( VerifyNextToken ( scan, '(' ) );
                REQUIRE ( VerifyNextToken ( scan, KW_include ) );
                REQUIRE ( VerifyNextToken ( scan, STRING ) );
                REQUIRE ( VerifyNextToken ( scan, ')' ) );

                REQUIRE ( VerifyNextToken ( scan, ';' ) );

            REQUIRE ( VerifyNextToken ( scan, ')' ) );

        REQUIRE ( VerifyNextToken ( scan, ')' ) );

    REQUIRE ( VerifyNextToken ( scan, SchemaScanner::EndSource ) );
    REQUIRE ( VerifyNextToken ( scan, ')' ) );

    delete root;
}

class AST_Fixture
{
public:
    AST_Fixture()
    :   m_parseTree ( 0 ),
        m_ast ( 0 )
    {
    }
    ~AST_Fixture()
    {
        delete m_ast;
        delete m_parseTree;
    }

    void PrintTree ( const ParseTree& p_tree )
    {
        ParseTreeScanner sc ( p_tree );
        const Token* tok;
        SchemaScanner :: TokenType tt;
        unsigned int indent = 0;
        do
        {
            tt = sc . NextToken ( tok );
            string v = tok -> GetValue ();
            if ( v . empty () )
            {
                switch (tt)
                {
                #define case_(t) case t:  v = #t; break
                case_ ( PT_PARSE );
                case_ ( PT_SOURCE );
                case_ ( PT_SCHEMA_1_0 );
                case_ ( PT_TYPEDEF );
                case_ ( PT_IDENT );
                case_ ( PT_ASTLIST );
                case_ ( PT_DIM );
                case_ ( PT_ARRAY );
                case_ ( END_SOURCE );

                default:
                    if ( tt < 256 )
                    {
                        v = string ( 1, ( char ) tt );
                    }
                    break;
                }
            }
            if ( tt == ')' && indent != 0 )
            {
                --indent;
            }
            cout << string ( indent * 2, ' ' ) << v << " (" << tt << ")" << endl;
            if ( tt == '(' )
            {
                ++indent;
            }
        }
        while ( tt != END_SOURCE );
        cout << string ( indent * 2, ' ' ) << ") (41)" << endl;
    }

    AST * MakeAst ( const char* p_source, bool p_printTree = false )
    {
        if ( ! m_parser . ParseString ( p_source ) )
        {
            throw std :: logic_error ( "AST_Fixture::MakeAst : ParseString() failed" );
        }
        m_parseTree = m_parser . MoveParseTree ();
        if ( m_parseTree == 0 )
        {
            throw std :: logic_error ( "AST_Fixture::MakeAst : MoveParseTree() returned 0" );
        }
        if ( p_printTree )
        {
            PrintTree ( * m_parseTree );
        }
        m_ast = m_builder . Build ( * m_parseTree );
        if ( m_builder . GetErrorCount() != 0)
        {
            throw std :: logic_error ( string ( "AST_Fixture::MakeAst : ASTBuilder::Build() failed: " ) + string ( m_builder . GetErrorMessage ( 0 ) ) );
        }
        else if ( m_ast == 0 )
        {
            throw std :: logic_error ( "AST_Fixture::MakeAst : ASTBuilder::Build() failed, no message!" );
        }
        return m_ast;
    }

    void VerifyErrorMessage ( const char* p_source, const char* p_expectedError )
    {
        if ( ! m_parser . ParseString ( p_source ) )
        {
            throw std :: logic_error ( "AST_Fixture::MakeAst : ParseString() failed" );
        }
        m_parseTree = m_parser . MoveParseTree ();
        if ( m_parseTree == 0 )
        {
            throw std :: logic_error ( "AST_Fixture::MakeAst : MoveParseTree() returned 0" );
        }
        delete m_builder . Build ( * m_parseTree );
        if ( m_builder . GetErrorCount() == 0 )
        {
            throw std :: logic_error ( "AST_Fixture::MakeAstExpectError : no error" );
        }
        if ( string ( m_builder . GetErrorMessage ( 0 ) ) != string ( p_expectedError ) )
        {
            throw std :: logic_error ( "AST_Fixture::MakeAstExpectError : expected '" + string ( p_expectedError ) +
                                                                      "', received '" + string ( m_builder . GetErrorMessage ( 0 ) ) + "'" );
        }
    }

    enum yytokentype TokenType ( const ParseTree * p_node ) const
    {
        return ( enum yytokentype ) p_node -> GetToken () . GetType ();
    }

    Token
    MakeIdent ( const char* p_text )
    {
        SchemaToken t = { IDENTIFIER_1_0, p_text, strlen ( p_text ), 0, 0 };
        return Token ( t );
    }

    void VerifySymbol ( const char* p_name, uint32_t p_type )
    {
        SchemaToken v = { IDENTIFIER_1_0, p_name, strlen ( p_name ), 0, 0 };
        KSymbol* sym = m_builder . Resolve ( v );

        if ( sym == 0 )
        {
            throw std :: logic_error ( "AST_Fixture::VerifySymbol : symbol not found" );
        }
        else
        {
            String expName;
            StringInitCString ( & expName, p_name );
            if ( StringCompare ( & expName, & sym -> name )  != 0 )
            {
                throw std :: logic_error ( "AST_Fixture::VerifySymbol : object name mismatch" );
            }
            else if ( sym -> type != p_type )
            {
                throw std :: logic_error ( "AST_Fixture::VerifySymbol : wrong object type" );
            }
        }
    }

    SchemaParser    m_parser;
    ParseTree *     m_parseTree;
    ASTBuilder      m_builder;
    AST*            m_ast;
};

FIXTURE_TEST_CASE(SchemaAST_Empty, AST_Fixture)
{
    REQUIRE_EQ ( END_SOURCE, TokenType ( MakeAst ( "" ) ) );
}

FIXTURE_TEST_CASE(SchemaAST_Intrinsic, AST_Fixture)
{
    MakeAst ( "" );
    // find a built-in type in the symtab
    VerifySymbol ( "U8", eDatatype );
}

FIXTURE_TEST_CASE(Builder_ErrorReporting, AST_Fixture)
{
    REQUIRE_NULL ( m_builder . Resolve ( MakeIdent ( "foo" ) ) );
    REQUIRE_NULL ( m_builder . Resolve ( MakeIdent ( "bar" ) ) );
    REQUIRE_EQ ( 2u, m_builder . GetErrorCount () );
    REQUIRE_EQ ( string ( "Undeclared identifier: 'foo'" ), string ( m_builder . GetErrorMessage ( 0 ) ) );
    REQUIRE_EQ ( string ( "Undeclared identifier: 'bar'" ), string ( m_builder . GetErrorMessage ( 1 ) ) );
}

FIXTURE_TEST_CASE(SchemaAST_NoVersion, AST_Fixture)
{
    MakeAst ( ";" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 1u,            m_ast -> ChildrenCount () );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 0 ) ) );
}

FIXTURE_TEST_CASE(SchemaAST_Version1, AST_Fixture)
{
    MakeAst ( "version 1; ;" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 1u,            m_ast -> ChildrenCount () );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 0 ) ) );
}

FIXTURE_TEST_CASE(SchemaAST_MultipleCallsToBuilder, AST_Fixture)
{
    MakeAst ( "version 1; ;" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 1u,            m_ast -> ChildrenCount () );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 0 ) ) );

    MakeAst ( "version 1; ;;" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 2u,            m_ast -> ChildrenCount () );
    // use valgrind for leaks
}

FIXTURE_TEST_CASE(SchemaAST_MultipleDecls, AST_Fixture)
{
    MakeAst ( "version 1; ; ;;" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 3u,            m_ast -> ChildrenCount () );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 0 ) ) );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 1 ) ) );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 2 ) ) );
}

FIXTURE_TEST_CASE(SchemaAST_Typedef_SimpleNames_OneScalar, AST_Fixture)
{
    MakeAst ( "typedef U8 t;" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 1u,            m_ast -> ChildrenCount () );

    const ParseTree* typeDef = m_ast -> GetChild ( 0 );

    REQUIRE_EQ ( PT_TYPEDEF,    TokenType ( typeDef ) );
    REQUIRE_EQ ( 2u,            typeDef -> ChildrenCount () );

    const ParseTree* baseType = typeDef -> GetChild ( 0 );
    REQUIRE_EQ ( 1u,                baseType -> ChildrenCount () );
    REQUIRE_EQ ( IDENTIFIER_1_0,    TokenType ( baseType -> GetChild ( 0 ) ) );
    REQUIRE_EQ ( string ( "U8" ),   string ( baseType -> GetChild ( 0 )-> GetToken () . GetValue () ) );

    const ParseTree* newTypes = typeDef -> GetChild ( 1 );
    REQUIRE_EQ ( 1u, newTypes -> ChildrenCount () );

    const ParseTree* newType = newTypes -> GetChild ( 0 );
    REQUIRE_EQ ( 1u,                newType -> ChildrenCount () );
    REQUIRE_EQ ( IDENTIFIER_1_0,    TokenType ( newType -> GetChild ( 0 ) ) );
    REQUIRE_EQ ( string ( "t" ),    string ( newType -> GetChild ( 0 ) -> GetToken () . GetValue () ) );

    // find "t" in the global scope
    VerifySymbol ( "t", eDatatype );
}

FIXTURE_TEST_CASE(SchemaAST_Typedef_SimpleNames_MultipleScalars, AST_Fixture)
{
    MakeAst ( "typedef U8 t1, t2;" );
    VerifySymbol ( "t1", eDatatype );
    VerifySymbol ( "t2", eDatatype );
}

#if SHOW_UNIMPLEMENTED
FIXTURE_TEST_CASE(SchemaAST_Typedef_Array, AST_Fixture)
{
    MakeAst ( "typedef U8 t [1];" );
    VerifySymbol ( "t", eDatatype );
    //TODO: verify dimension
}
#endif

FIXTURE_TEST_CASE(SchemaAST_Typedef_UndefinedBase, AST_Fixture)
{
    VerifyErrorMessage ( "typedef zz t;", "Undeclared identifier: 'zz'" );
}

FIXTURE_TEST_CASE(SchemaAST_Typedef_DuplicateDefinition_1, AST_Fixture)
{
    VerifyErrorMessage ( "typedef U8 t; typedef U8 t;", "Object already declared: 't'" );
}

FIXTURE_TEST_CASE(SchemaAST_Typedef_DuplicateDefinition_2, AST_Fixture)
{
    VerifyErrorMessage ( "typedef U8 t, t;", "Object already declared: 't'" );
}

#if SHOW_UNIMPLEMENTED
FIXTURE_TEST_CASE(SchemaAST_Typedef_BaseNotAType, AST_Fixture)
{
    VerifyErrorMessage ( "table qq; typedef qq t;", "Not a datatype: 'qq'" );
}
#endif
//TODO: typedef U8 t[2];
//TODO: typedef U8 t[-1]; - error
//TODO: typedef U8 t[1.1]; - error
//TODO: typedef U8 t[non-const expr]; - error

//TODO: typedef U8 t1,t2[2],t3;

//////////////////////////////////////////// Main
#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/out.h>

extern "C"
{

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

const char UsageDefaultName[] = "wb-test-schema-ast";

rc_t CC UsageSummary (const char * progname)
{
    return KOutMsg ( "Usage:\n" "\t%s [options] -o path\n\n", progname );
}

rc_t CC Usage( const Args* args )
{
    return 0;
}

rc_t CC KMain ( int argc, char *argv [] )
{
    return SchemaASTTestSuite(argc, argv);
}

}

