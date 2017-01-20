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

#include <vdb/manager.h>

#include "../../libs/vdb/schema-priv.h"
#include "../../libs/vdb/schema-expr.h"

// hide an unfortunately named C function
#define typename __typename
#include "../../libs/vdb/schema-parse.h"
#undef typename

#include "../../libs/schema/SchemaParser.hpp"
#include "../../libs/schema/ParseTree.hpp"
#include "../../libs/schema/ASTBuilder.hpp"

using namespace ncbi::SchemaParser;
#include "../../libs/schema/schema-tokens.h"

#define KW_TOKEN(v,k) SchemaToken v = { KW_##k, #k, strlen(#k), 0, 0 }

using namespace std;
using namespace ncbi::NK;

TEST_SUITE ( SchemaASTTestSuite );

// AST

static
string
ToCppString ( const String & p_str)
{
    return string ( p_str . addr, p_str . len );
}

static
bool
VerifyNextToken ( ParseTreeScanner& p_scan, int p_type)
{
    const Token* token;
    return p_scan . NextToken ( token ) == p_type;
}

TEST_CASE(Construct_Empty)
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

TEST_CASE(WalkParseTree)
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

// AST subclasses
static
AST_FQN *
MakeFqn ( const char* p_text ) // p_text = (ident:)+ident
{
    SchemaToken id = { PT_IDENT, 0, 0, 0, 0 };
    Token ident ( id );
    AST_FQN * ret = new AST_FQN ( & ident );

    std::string s ( p_text );

    while ( s . length () > 0 )
    {
        std::string token;
        size_t pos = s . find(':');
        if (pos != std::string::npos)
        {
            token = s . substr ( 0, pos );
            s . erase(0, pos + 1);
        }
        else
        {
            token = s;
            s . clear ();
        }
        SchemaToken name = { IDENTIFIER_1_0, token . c_str () , token . length (), 0, 0 };
        Token tname ( name );
        ret -> AddNode ( & tname );
    }

    return ret;
}

TEST_CASE ( AST_FQN_NakedIdent )
{
    AST_FQN* fqn = MakeFqn ( "a" );

    REQUIRE_EQ ( 0u, fqn -> NamespaceCount () );

    String str;
    fqn -> GetIdentifier ( str );
    REQUIRE_EQ ( string ("a"), ToCppString ( str ) );

    char buf [ 10 ];
    fqn -> GetFullName ( buf, sizeof buf );
    REQUIRE_EQ ( string ("a"), string ( buf ) );

    delete fqn;
}

TEST_CASE ( AST_FQN_Full )
{
    AST_FQN* fqn = MakeFqn ( "a:b:c" );

    REQUIRE_EQ ( 2u, fqn -> NamespaceCount () );

    String str;
    fqn -> GetIdentifier ( str );
    REQUIRE_EQ ( string ("c"), ToCppString ( str ) );

    char buf [ 10 ];
    fqn -> GetFullName ( buf, sizeof buf );
    REQUIRE_EQ ( string ("a:b:c"), string ( buf ) );

    fqn -> GetPartialName ( buf, sizeof buf, 1 );
    REQUIRE_EQ ( string ("a:b"), string ( buf ) );

    delete fqn;
}

TEST_CASE ( AST_FQN_WithVersionMaj )
{
    AST_FQN* fqn = MakeFqn ( "a" );
    fqn -> SetVersion ( "#1" );
    REQUIRE_EQ ( 1 << 24, ( int ) fqn -> GetVersion () );
    delete fqn;
}
TEST_CASE ( AST_FQN_WithVersionMajMin )
{
    AST_FQN* fqn = MakeFqn ( "a" );
    fqn -> SetVersion ( "#1.22" );
    REQUIRE_EQ ( ( 1 << 24 ) | ( 22 << 16 ), ( int ) fqn -> GetVersion () );
    delete fqn;
}
TEST_CASE ( AST_FQN_WithVersionMajMinRel )
{
    AST_FQN* fqn = MakeFqn ( "a" );
    fqn -> SetVersion ( "#1.22.33" );
    REQUIRE_EQ ( ( 1 << 24 ) | ( 22 << 16 ) | 33, ( int ) fqn -> GetVersion () );
    delete fqn;
}

// AST builder

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
                case_ ( PT_TYPESET );
                case_ ( PT_TYPESETDEF );
                case_ ( PT_IDENT );
                case_ ( PT_ASTLIST );
                case_ ( PT_ARRAY );
                case_ ( END_SOURCE );
                case_ ( PT_FUNCTION );
                case_ ( PT_FUNCDECL );
                case_ ( PT_EMPTY );
                case_ ( PT_SCHEMASIG );
                case_ ( PT_SCHEMAFORMAL );
                case_ ( PT_RETURNTYPE );
                case_ ( PT_FUNCSIG );
                case_ ( PT_FUNCPROLOGUE );
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

    AST * MakeAst ( const char* p_source, bool p_debugParse = false, bool p_printTree = false )
    {
        if ( ! m_parser . ParseString ( p_source, p_debugParse ) )
        {
            throw std :: logic_error ( "AST_Fixture::MakeAst : ParseString() failed" );
        }
        if ( m_parseTree != 0 )
        {
            delete m_parseTree;
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
        if ( m_ast != 0 )
        {
            delete m_ast;
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

    const KSymbol* VerifySymbol ( const char* p_name, uint32_t p_type )
    {
        AST_FQN * ast = MakeFqn ( p_name );
        const KSymbol* sym = m_builder . Resolve ( * ast );

        if ( sym == 0 )
        {
            throw std :: logic_error ( "AST_Fixture::VerifySymbol : symbol not found" );
        }
        else
        {
            if ( ToCppString ( sym -> name ) !=
                 ast -> GetChild ( ast -> ChildrenCount() - 1 ) -> GetTokenValue () )
            {
                throw std :: logic_error ( "AST_Fixture::VerifySymbol : object name mismatch" );
            }
            else if ( sym -> type != p_type )
            {
                throw std :: logic_error ( "AST_Fixture::VerifySymbol : wrong object type" );
            }
        }

        delete ast;

        return sym;
    }

    const SDatatype* VerifyDatatype ( const char* p_name, const char* p_baseName, uint32_t p_dim, uint32_t p_size )
    {
        const KSymbol* sym = VerifySymbol ( p_name, eDatatype );
        const SDatatype* ret = static_cast < const SDatatype* > ( sym -> u . obj );
        if ( ret -> super == 0 )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyDatatype : super == 0" );
        }
        if ( string ( p_baseName ) != ToCppString ( ret -> super -> name -> name ) )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyDatatype : wrong baseName" );
        }
        if ( p_dim != ret -> dim )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyDatatype : wrong dim" );
        }
        if ( p_size != ret -> size )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyDatatype : wrong size" );
        }
        if ( ret -> super -> domain != ret -> domain )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyDatatype : wrong domain" );
        }
        return ret;
    }

    const SFunction* VerifyFunction ( const char* p_name, uint32_t p_type = eFunction ) // not overloaded
    {
        const KSymbol* sym = VerifySymbol ( p_name, p_type );

        // for functions, sym points to an entry in the overloads table (schema->fname)
        const SNameOverload* name = static_cast < const SNameOverload* > ( sym -> u . obj );
        if ( 1u != VectorLength ( & name -> items ) )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyFunction : too many overloads" );
        }
        const SFunction* ret = static_cast < const SFunction* > ( VectorGet ( & name -> items, 0 ) );
        if ( string ( p_name ) != ToCppString ( ret -> name -> name ) )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyFunction : wrong name" );
        }

        return ret;
    }

    const SFunction * VerifyOverload ( const char* p_name, uint32_t p_idx )
    {
        const KSymbol* sym = VerifySymbol ( p_name, eFunction );

        // for functions, sym points to an entry in the overloads table (schema->fname)
        const SNameOverload* name = static_cast < const SNameOverload* > ( sym -> u . obj );
        if ( p_idx >= VectorLength ( & name -> items ) )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyOverload : index out of range" );
        }
        return static_cast < const SFunction * > ( VectorGet ( & name -> items, p_idx ) );
    }

    SchemaParser    m_parser;
    ParseTree *     m_parseTree;
    ASTBuilder      m_builder;
    AST*            m_ast;
};

FIXTURE_TEST_CASE(Empty_Source, AST_Fixture)
{
    REQUIRE_EQ ( END_SOURCE, TokenType ( MakeAst ( "" ) ) );
}

FIXTURE_TEST_CASE(Intrinsic, AST_Fixture)
{
    MakeAst ( "" );
    // find a built-in type in the symtab
    VerifySymbol ( "U8", eDatatype );
}

FIXTURE_TEST_CASE(Builder_ErrorReporting, AST_Fixture)
{
    AST_FQN * id = MakeFqn ( "foo" );
    REQUIRE_NULL ( m_builder . Resolve ( * id ) ); delete id;
    id = MakeFqn ( "bar" );
    REQUIRE_NULL ( m_builder . Resolve ( * id ) ); delete id;
    REQUIRE_EQ ( 2u, m_builder . GetErrorCount () );
    REQUIRE_EQ ( string ( "Undeclared identifier: 'foo'" ), string ( m_builder . GetErrorMessage ( 0 ) ) );
    REQUIRE_EQ ( string ( "Undeclared identifier: 'bar'" ), string ( m_builder . GetErrorMessage ( 1 ) ) );
}

FIXTURE_TEST_CASE(NoVersion, AST_Fixture)
{
    MakeAst ( ";" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 1u,            m_ast -> ChildrenCount () );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 0 ) ) );
}

FIXTURE_TEST_CASE(Version1, AST_Fixture)
{
    MakeAst ( "version 1; ;" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 1u,            m_ast -> ChildrenCount () );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 0 ) ) );
}

FIXTURE_TEST_CASE(MultipleCallsToBuilder, AST_Fixture)
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

FIXTURE_TEST_CASE(MultipleDecls, AST_Fixture)
{
    MakeAst ( "version 1; ; ;;" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 3u,            m_ast -> ChildrenCount () );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 0 ) ) );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 1 ) ) );
    REQUIRE_EQ ( PT_EMPTY,      TokenType ( m_ast -> GetChild ( 2 ) ) );
}

///////// typedef

FIXTURE_TEST_CASE(Typedef_SimpleNames_OneScalar, AST_Fixture)
{
    MakeAst ( "typedef U8 t;" );
    VerifyDatatype ( "t", "U8", 1, 8 );
}

FIXTURE_TEST_CASE(Typedef_SimpleNames_MultipleScalars, AST_Fixture)
{
    MakeAst ( "typedef U8 t1, t2;" );
    VerifyDatatype ( "t1", "U8", 1, 8 );
    VerifyDatatype ( "t2", "U8", 1, 8 );
}

FIXTURE_TEST_CASE(Typedef_FQN_OneScalar, AST_Fixture)
{
    MakeAst ( "typedef U8 a:b:t;" );
    VerifySymbol ( "a", eNamespace );
    VerifySymbol ( "a:b", eNamespace );
    VerifyDatatype ( "a:b:t", "U8", 1, 8 );
}

FIXTURE_TEST_CASE(Resolve_UndefinedNamespace, AST_Fixture)
{
    VerifyErrorMessage ( "typedef a:zz t;", "Namespace not found: a" );
}
FIXTURE_TEST_CASE(Resolve_UndefinedNameInNamespace, AST_Fixture)
{
    VerifyErrorMessage ( "typedef U8 a:t; typedef a:zz t;", "Undeclared identifier: 'a:zz'" );
}

FIXTURE_TEST_CASE(Typedef_Array, AST_Fixture)
{
    MakeAst ( "typedef U8 t [11];" );
    VerifyDatatype ( "t", "U8", 11, 88 );
}

FIXTURE_TEST_CASE(Typedef_WultipleWithArray, AST_Fixture)
{
    MakeAst ( "typedef U8 t1,t2[2],t3;" );
    VerifyDatatype ( "t1", "U8", 1, 8 );
    VerifyDatatype ( "t2", "U8", 2, 16 );
    VerifyDatatype ( "t3", "U8", 1, 8 );
}

FIXTURE_TEST_CASE(Typedef_UndefinedBase, AST_Fixture)
{
    VerifyErrorMessage ( "typedef zz t;", "Undeclared identifier: 'zz'" );
}

FIXTURE_TEST_CASE(Typedef_DuplicateDefinition_1, AST_Fixture)
{
    VerifyErrorMessage ( "typedef U8 t; typedef U8 t;", "Object already declared: 't'" );
}

FIXTURE_TEST_CASE(Typedef_DuplicateDefinition_2, AST_Fixture)
{
    VerifyErrorMessage ( "typedef U8 t, t;", "Object already declared: 't'" );
}

#if SHOW_UNIMPLEMENTED
FIXTURE_TEST_CASE(Typedef_BaseNotAType, AST_Fixture)
{
    VerifyErrorMessage ( "table qq; typedef qq t;", "Not a datatype: 'qq'" );
}
#endif
//TODO: typedef U8 t[-1]; - error
//TODO: typedef U8 t[1.1]; - error
//TODO: typedef U8 t[non-const expr]; - error

///////// typeset

FIXTURE_TEST_CASE(Typeset_OneScalar, AST_Fixture)
{
    MakeAst ( "typeset t { U8 };" );
    const KSymbol* sym = VerifySymbol ( "t", eTypeset );
    const STypeset* ts = static_cast < const STypeset* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( ts );
    REQUIRE_EQ ( string ( "t" ), ToCppString ( ts -> name -> name ) );
    REQUIRE_EQ ( (uint16_t)1, ts -> count );
    REQUIRE_EQ ( (uint32_t)9, ts -> td [ 0 ] . type_id );
    REQUIRE_EQ ( (uint32_t)1, ts -> td [ 0 ] . dim );
}

FIXTURE_TEST_CASE(Typeset_MultipleScalars, AST_Fixture)
{
    MakeAst ( "typeset t { U8, U32 };" );
    const KSymbol* sym = VerifySymbol ( "t", eTypeset );
    const STypeset* ts = static_cast < const STypeset* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( ts );
    REQUIRE_EQ ( string ( "t" ), ToCppString ( ts -> name -> name ) );
    REQUIRE_EQ ( (uint16_t)2, ts -> count );
    REQUIRE_EQ ( (uint32_t)9, ts -> td [ 0 ] . type_id );
    REQUIRE_EQ ( (uint32_t)1, ts -> td [ 0 ] . dim );
    REQUIRE_EQ ( (uint32_t)11, ts -> td [ 1 ] . type_id );
    REQUIRE_EQ ( (uint32_t)1, ts -> td [ 1 ] . dim );
}

FIXTURE_TEST_CASE(Typeset_OneArray, AST_Fixture)
{
    MakeAst ( "typeset t { U8[2] };" );
    const KSymbol* sym = VerifySymbol ( "t", eTypeset );
    const STypeset* ts = static_cast < const STypeset* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( ts );
    REQUIRE_EQ ( string ( "t" ), ToCppString ( ts -> name -> name ) );
    REQUIRE_EQ ( (uint16_t)1, ts -> count );
    REQUIRE_EQ ( (uint32_t)9, ts -> td [ 0 ] . type_id );
    REQUIRE_EQ ( (uint32_t)2, ts -> td [ 0 ] . dim );
}

FIXTURE_TEST_CASE(Typeset_AlreadyNotTypeset, AST_Fixture)
{
    VerifyErrorMessage ( "typedef U8 t; typeset t { U8 };", "Already declared and is not a typeset: 't'" );
}

FIXTURE_TEST_CASE(Typeset_DirectDuplicatesAllowed, AST_Fixture)
{
    MakeAst ( "typeset t { U8, U8 };" );
    const KSymbol* sym = VerifySymbol ( "t", eTypeset );
    const STypeset* ts = static_cast < const STypeset* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( ts );
    REQUIRE_EQ ( string ( "t" ), ToCppString ( ts -> name -> name ) );
    REQUIRE_EQ ( (uint16_t)1, ts -> count );
}

FIXTURE_TEST_CASE(Typeset_BenignRedefinesAllowed, AST_Fixture)
{
    MakeAst ( "typeset t { U8 }; typeset t { U8 };" );
    VerifySymbol ( "t", eTypeset );
}

FIXTURE_TEST_CASE(Typeset_BadRedefine_1, AST_Fixture)
{
    VerifyErrorMessage ( "typedef U8 t; typeset t { U8 };", "Already declared and is not a typeset: 't'" );
}
FIXTURE_TEST_CASE(Typeset_BadRedefine_2, AST_Fixture)
{
    VerifyErrorMessage ( "typeset t { U8 }; typeset t { U8, U16 };", "Typeset already declared differently: 't'" );
}
FIXTURE_TEST_CASE(Typeset_BadRedefine_3, AST_Fixture)
{
    VerifyErrorMessage ( "typeset t { U8 }; typeset t { U16 };", "Typeset already declared differently: 't'" );
}

FIXTURE_TEST_CASE(Typeset_IncludesAnotherTypeset, AST_Fixture)
{
    MakeAst ( "typeset t1 { U8, U16 }; typeset t2 { t1, U32 };" );
    const KSymbol* sym = VerifySymbol ( "t2", eTypeset );
    const STypeset* ts = static_cast < const STypeset* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( ts );
    REQUIRE_EQ ( (uint16_t)3, ts -> count );
}

FIXTURE_TEST_CASE(Typeset_InirectDuplicatesAllowed, AST_Fixture)
{
    MakeAst ( "typeset t1 { U8, U16 }; typeset t2 { t1, U8 };" );
    const KSymbol* sym = VerifySymbol ( "t2", eTypeset );
    const STypeset* ts = static_cast < const STypeset* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( ts );
    REQUIRE_EQ ( (uint16_t)2, ts -> count );
}

///////// fmtdef

FIXTURE_TEST_CASE(Format_Simple, AST_Fixture)
{
    MakeAst ( "fmtdef f;" );
    const KSymbol* sym = VerifySymbol ( "f", eFormat );
    const SFormat* f = static_cast < const SFormat* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( f );
    REQUIRE_EQ ( string ( "f" ), ToCppString ( f -> name -> name ) );
    REQUIRE_NULL ( f -> super );
}

FIXTURE_TEST_CASE(Format_Derived, AST_Fixture)
{
    MakeAst ( "fmtdef s; fmtdef s f;" );
    const KSymbol* sym = VerifySymbol ( "f", eFormat );
    const SFormat* f = static_cast < const SFormat* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( f );
    REQUIRE_EQ ( string ( "f" ), ToCppString ( f -> name -> name ) );
    REQUIRE_NOT_NULL ( f -> super );
    REQUIRE_EQ ( string ( "s" ), ToCppString ( f -> super -> name -> name ) );
}

FIXTURE_TEST_CASE(Format_SuperUndefined, AST_Fixture)
{
    VerifyErrorMessage ( "fmtdef s f;", "Undeclared identifier: 's'" );
}

FIXTURE_TEST_CASE(Format_SuperWrong, AST_Fixture)
{
    VerifyErrorMessage ( "typedef U8 s; fmtdef s f;", "Not a format: 's'" );
}

///////// const

FIXTURE_TEST_CASE(Const_Simple, AST_Fixture)
{
    MakeAst ( "const U8 c = 1;" );
    const KSymbol* sym = VerifySymbol ( "c", eConstant );
    const SConstant* c = static_cast < const SConstant* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( c );
    REQUIRE_EQ ( string ( "c" ), ToCppString ( c -> name -> name ) );
    REQUIRE_EQ ( 9u, c -> td . type_id );
    REQUIRE_NOT_NULL ( c -> expr );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, c -> expr -> var );
    const SConstExpr & expr =  * reinterpret_cast < const SConstExpr * > ( c -> expr );
    REQUIRE_EQ ( ( uint64_t ) 1, expr . u . u64 [ 0 ] );
    REQUIRE_EQ ( 12u, expr . td . type_id );
    REQUIRE_EQ ( 1u, expr . td . dim );
}

//TODO: const U8 [2] c = [1,2];
//TODO: const U8 c = [1,2]; - error ( not an array )
//TODO: const U8 c[2] = 1; - error ( not a scalar )
//TODO: const U8 c = 1.0; - error ( wrong type ) ?
//TODO: const U8 c1; const U8 c2 = c1 + 1;
//TODO: typedef U8 t; const U8 c2 = t; - error ( not const )

///////// alias

FIXTURE_TEST_CASE(Alias_Simple, AST_Fixture)
{
    MakeAst ( "alias U8 a;" );
    const KSymbol* sym = VerifySymbol ( "a", eDatatype );
    REQUIRE_NOT_NULL ( sym );
    const SDatatype* dt = static_cast < const SDatatype* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( dt );
    REQUIRE_EQ ( string ( "U8" ), ToCppString ( dt -> name -> name ) );
    REQUIRE_EQ ( 1u, dt -> dim );
    REQUIRE_EQ ( 8u, dt -> size );
    //TODO: verify schema->alias
}

FIXTURE_TEST_CASE(AliasedTypeInTypeset, AST_Fixture)
{
    MakeAst ( "alias U8 a; typeset ts { a };" );
    VerifySymbol ( "ts", eTypeset );
}

FIXTURE_TEST_CASE(AliasedTypesetInTypeset, AST_Fixture)
{
    MakeAst ( "typeset ts1 { U8 }; alias ts1 ats; typeset ts2 { ats };" );
    VerifySymbol ( "ts2", eTypeset );
}

///////// function

FIXTURE_TEST_CASE(Func_Untyped, AST_Fixture)
{
    MakeAst ( "function __untyped f();" );
    const SFunction* fn = VerifyFunction ( "f", eUntypedFunc );
    REQUIRE_NULL ( fn -> rt );
}

FIXTURE_TEST_CASE(Func_UntypedRedeclared, AST_Fixture)
{
    MakeAst ( "function __untyped f(); function __untyped f();" );
    VerifyFunction ( "f", eUntypedFunc );
}

FIXTURE_TEST_CASE(Func_Untyped_Exists, AST_Fixture)
{
    VerifyErrorMessage ( "typedef U8 f; function __untyped f();", "Declared earlier with a different type: 'f'" );
}

FIXTURE_TEST_CASE(Func_Rowlength, AST_Fixture)
{   // implemented by the same code as untyped, the basic test case will suffice
    MakeAst ( "function __row_length f();" );
    VerifyFunction ( "f", eRowLengthFunc );
}

FIXTURE_TEST_CASE(Func_Scalar_NoParams, AST_Fixture)
{
    MakeAst ( "function U8 f();" );
    const SFunction* fn = VerifyFunction ( "f" );
    REQUIRE_EQ ( 9u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . type_id );
    REQUIRE_EQ ( 1u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_Array_NoParams, AST_Fixture)
{
    MakeAst ( "function U8[2] f();" );
    const SFunction* fn = VerifyFunction ( "f" );
    REQUIRE_EQ ( 9u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . type_id );
    REQUIRE_EQ ( 2u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_VarArray_NoParams, AST_Fixture)
{
    MakeAst ( "function U8[*] f();" );
    const SFunction* fn = VerifyFunction ( "f" );
    REQUIRE_EQ ( 9u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . type_id );
    REQUIRE_EQ ( 0u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_WithVersion, AST_Fixture)
{
    MakeAst ( "function U8 f#1.2();" );
    const SFunction* fn = VerifyFunction ( "f" );
    REQUIRE_EQ ( ( 1 << 24 ) | ( 2 << 16 ), ( int ) fn -> version );
}

FIXTURE_TEST_CASE(Func_Redeclared_NoVersion, AST_Fixture)
{
    MakeAst ( "function U8 f();function U16 f();" );
    const SFunction* fn = VerifyOverload ( "f", 0 );
    REQUIRE_EQ ( 9u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . type_id ); // 2nd decl with the same version ignored
    REQUIRE_EQ ( 1u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_Redeclared_SameVersion, AST_Fixture)
{
    MakeAst ( "function U8 f#1.2.3();function U16 f#1.2.3();" );
    const SFunction* fn = VerifyOverload ( "f", 0 );
    REQUIRE_EQ ( 9u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . type_id ); // 2nd decl with the same version ignored
}

FIXTURE_TEST_CASE(Func_Redeclared_DiffMajor, AST_Fixture)
{   // 2 overloads created
    MakeAst ( "function U8 f#2();function U16 f#1.2.3();" ); // release does not break
    {
        const SFunction* fn = VerifyOverload ( "f", 0 );    // lower version
        REQUIRE_EQ ( 10u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . type_id ); // U16
    }
    {
        const SFunction* fn = VerifyOverload ( "f", 1 );    // higher version
        REQUIRE_EQ ( 9u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . type_id ); // U8
    }
}

FIXTURE_TEST_CASE(Old_Func_Scalar_NoParams, AST_Fixture)
{
    const VDBManager *mgr;
    REQUIRE_RC ( VDBManagerMakeRead ( & mgr, 0 ) );
    VSchema *schema;
    REQUIRE_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
    string input = "function U8 f#2();function U16 f#1.2();"; // release breaks (why?)
    REQUIRE_RC ( VSchemaParseText ( schema, 0, input . c_str (), input . length () ) );

    REQUIRE_EQ ( 2u, VectorLength ( & schema -> func ) );
    const SFunction* fn = static_cast < const SFunction* > ( VectorGet ( & schema -> func, 0 ) );
    REQUIRE_EQ ( string ( "f" ), ToCppString ( fn -> name -> name ) );
    REQUIRE_EQ ( 9u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . type_id );
    fn = static_cast < const SFunction* > ( VectorGet ( & schema -> func, 1 ) );
    REQUIRE_EQ ( string ( "f" ), ToCppString ( fn -> name -> name ) );
    REQUIRE_EQ ( 10u, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . type_id );

    REQUIRE_EQ ( 1u, VectorLength ( & schema -> fname ) );
    const SNameOverload* name = static_cast < const SNameOverload* > ( VectorGet ( & schema -> fname, 0 ) );
    REQUIRE_EQ ( 0u, name -> cid . id );
    REQUIRE_EQ ( 2u, VectorLength ( & name -> items ) );

    VSchemaRelease ( schema );
    VDBManagerRelease ( mgr );
}

//TODO: function redeclared with diff major version (overload)
//TODO: function redeclared with same major and lower minor version (no effect)
//TODO: function redeclared with same major and higher minor version (replaces)

//TODO: script + untyped - error
//TODO: validate + untyped - error
//TODO: script fn without a body - error
//TODO: non-script fn with a body - error
//TODO: validate + non-void return - error
//TODO: non-validate + void return - error
//TODO: function returning a typeset
//TODO: function returning a schema type
//TODO: function returning a format (?)
//TODO: function returning something else - error

//TODO:array of arrays in typedef, typeset, constdef, return type

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

