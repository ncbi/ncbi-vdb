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

#include "AST_Fixture.hpp"

#include <cstdio>

#include <ktst/unit_test.hpp>

#include <klib/symbol.h>

#include <kfs/directory.h>

#include "../../libs/vdb/schema-expr.h"

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

TEST_CASE(Construct_Empty)
{
    SchemaParser p;
    REQUIRE ( p . ParseString ( "" ) );
    ParseTree * root = p . MoveParseTree ();
    REQUIRE_NOT_NULL ( root );
    ParseTreeScanner scan ( * root );
    REQUIRE ( VerifyNextToken ( scan, PT_PARSE ) );
    REQUIRE ( VerifyNextToken ( scan, '(' ) );
    REQUIRE ( VerifyNextToken ( scan, Token :: EndSource ) );
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

    REQUIRE ( VerifyNextToken ( scan, Token :: EndSource ) );
    REQUIRE ( VerifyNextToken ( scan, ')' ) );

    delete root;
}

// ErrorReport
TEST_CASE ( ErrorReport_Formatting_NullBuf )
{
    ErrorReport rep;
    rep . ReportError ( ErrorReport :: Location ( "", 0, 0 ), "" );
    const ErrorReport :: Error * err = rep . GetError ( 0 );
    REQUIRE_NOT_NULL ( err );
    REQUIRE ( ! err -> Format ( 0, 1024 ) );
}

TEST_CASE ( ErrorReport_Formatting_ShortBuf )
{
    ErrorReport rep;
    rep . ReportError ( ErrorReport :: Location ( "", 0, 0 ), "msg" );
    const ErrorReport :: Error * err = rep . GetError ( 0 );
    REQUIRE_NOT_NULL ( err );
    char buf [1];
    REQUIRE ( ! err -> Format ( buf, sizeof ( buf ) ) );
}

TEST_CASE ( ErrorReport_Formatting )
{
    ErrorReport rep;
    ErrorReport :: Location loc ( "dir/file", 1, 2 ); // file/line/col
    rep . ReportError ( loc, "msg: %s, num: %i", "error message", 42 );
    const ErrorReport :: Error * err = rep . GetError ( 0 );
    REQUIRE_NOT_NULL ( err );
    char buf [ 1024 ];
    REQUIRE ( err -> Format ( buf, sizeof buf ) );
    REQUIRE_EQ ( string ( buf ), string ( "dir/file:1:2 msg: error message, num: 42" ) );
}

// AST subclasses

TEST_CASE ( AST_FQN_NakedIdent )
{
    AST_FQN* fqn = AST_Fixture :: MakeFqn ( "a" );

    REQUIRE_EQ ( 0u, fqn -> NamespaceCount () );

    String str;
    fqn -> GetIdentifier ( str );
    REQUIRE_EQ ( string ("a"), AST_Fixture :: ToCppString ( str ) );

    char buf [ 10 ];
    fqn -> GetFullName ( buf, sizeof buf );
    REQUIRE_EQ ( string ("a"), string ( buf ) );

    delete fqn;
}

TEST_CASE ( AST_FQN_Full )
{
    AST_FQN* fqn = AST_Fixture :: MakeFqn ( "a:b:c" );

    REQUIRE_EQ ( 2u, fqn -> NamespaceCount () );

    String str;
    fqn -> GetIdentifier ( str );
    REQUIRE_EQ ( string ("c"), AST_Fixture :: ToCppString ( str ) );

    char buf [ 10 ];
    fqn -> GetFullName ( buf, sizeof buf );
    REQUIRE_EQ ( string ("a:b:c"), string ( buf ) );

    fqn -> GetPartialName ( buf, sizeof buf, 1 );
    REQUIRE_EQ ( string ("a:b"), string ( buf ) );

    delete fqn;
}

TEST_CASE ( AST_FQN_WithVersionMaj )
{
    AST_FQN* fqn = AST_Fixture :: MakeFqn ( "a" );
    fqn -> SetVersion ( "#1" );
    REQUIRE_EQ ( 1 << 24, ( int ) fqn -> GetVersion () );
    delete fqn;
}
TEST_CASE ( AST_FQN_WithVersionMajMin )
{
    AST_FQN* fqn = AST_Fixture :: MakeFqn ( "a" );
    fqn -> SetVersion ( "#1.22" );
    REQUIRE_EQ ( ( 1 << 24 ) | ( 22 << 16 ), ( int ) fqn -> GetVersion () );
    delete fqn;
}
TEST_CASE ( AST_FQN_WithVersionMajMinRel )
{
    AST_FQN* fqn = AST_Fixture :: MakeFqn ( "a" );
    fqn -> SetVersion ( "#1.22.33" );
    REQUIRE_EQ ( ( 1 << 24 ) | ( 22 << 16 ) | 33, ( int ) fqn -> GetVersion () );
    delete fqn;
}

// AST builder

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
    AST_FQN * id = AST_Fixture :: MakeFqn ( "foo" );
    REQUIRE_NULL ( m_builder -> Resolve ( * id ) ); delete id;
    id = AST_Fixture :: MakeFqn ( "bar" );
    REQUIRE_NULL ( m_builder -> Resolve ( * id ) ); delete id;
    REQUIRE_EQ ( 2u, m_builder -> GetErrorCount () );
    REQUIRE_EQ ( string ( "Undeclared identifier: 'foo'" ), string ( m_builder -> GetErrorMessage ( 0 ) ) );
    REQUIRE_EQ ( string ( "Undeclared identifier: 'bar'" ), string ( m_builder -> GetErrorMessage ( 1 ) ) );
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

    REQUIRE_EQ ( PT_SCHEMA_1_0,     TokenType ( m_ast ) );
    REQUIRE_EQ ( 2u,                m_ast -> ChildrenCount () );
    REQUIRE_EQ ( PT_EMPTY,          TokenType ( m_ast -> GetChild ( 0 ) ) );
    REQUIRE_EQ ( PT_VERSION_1_0,    TokenType ( m_ast -> GetChild ( 1 ) ) );
}

FIXTURE_TEST_CASE(MultipleCallsToBuilder, AST_Fixture)
{
    MakeAst ( "version 1; ;" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 2u,                m_ast -> ChildrenCount () );

    MakeAst ( "version 1; ;;" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 2u,            m_ast -> ChildrenCount () );
    // use valgrind for leaks
}

FIXTURE_TEST_CASE(MultipleDecls, AST_Fixture)
{
    MakeAst ( "version 1; ; ;;" );

    REQUIRE_EQ ( PT_SCHEMA_1_0, TokenType ( m_ast ) );
    REQUIRE_EQ ( 2u, m_ast -> ChildrenCount () );
    // next level of decls
    const AST * child = m_ast -> GetChild ( 0 );
    REQUIRE_EQ ( PT_EMPTY,  TokenType ( child ) );
    REQUIRE_EQ ( 3u,        child -> ChildrenCount () );
    REQUIRE_EQ ( PT_EMPTY,  TokenType ( child -> GetChild ( 0 ) ) );
    REQUIRE_EQ ( PT_EMPTY,  TokenType ( child -> GetChild ( 1 ) ) );
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

FIXTURE_TEST_CASE(LocationInErrorMessages, AST_Fixture)
{
    if ( m_newParse )
    {
        REQUIRE ( m_parser . ParseString ( "\n\ntypedef a:zz t;" ) );
        m_parseTree = m_parser . MoveParseTree ();
        REQUIRE_NOT_NULL ( m_parseTree );
        delete m_builder -> Build ( * m_parseTree );
        const ErrorReport & errors = m_builder -> GetErrors ();
        REQUIRE_EQ ( 1u, errors . GetCount () );
        const ErrorReport :: Error * err = errors . GetError ( 0 );
        REQUIRE_EQ ( string ( "Undeclared identifier: 'zz'" ), string ( err -> m_message ) );
        REQUIRE_EQ ( 3u, err -> m_line );
        REQUIRE_EQ ( 11u, err -> m_column );
        REQUIRE_EQ ( string ( "<unknown>" ), string ( err -> m_file ) );
    }
}

FIXTURE_TEST_CASE(Resolve_UndefinedNameInNamespace, AST_Fixture)
{
    VerifyErrorMessage ( "typedef U8 a:t; typedef a:zz t;", "Undeclared identifier: 'zz'" );
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

FIXTURE_TEST_CASE(Typedef_BaseNotAType, AST_Fixture)
{
    VerifyErrorMessage ( "const U8 qq = 0; typedef qq t;", "Not a datatype: 'qq'", 1, 26 );
}

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
    REQUIRE_EQ ( U8_id, ts -> td [ 0 ] . type_id );
    REQUIRE_EQ ( (uint32_t)1, ts -> td [ 0 ] . dim );
}

FIXTURE_TEST_CASE(Typeset_MultipleScalars, AST_Fixture)
{
    MakeAst ( "typeset t { U8, U32 };" );
    const KSymbol* sym = VerifySymbol ( "t", eTypeset );
    const STypeset* ts = static_cast < const STypeset* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( ts );
    REQUIRE_EQ ( string ( "t" ), ToCppString ( ts -> name -> name ) );
    REQUIRE_EQ ( (uint16_t)2u, ts -> count );
    REQUIRE_EQ ( U8_id, ts -> td [ 0 ] . type_id );
    REQUIRE_EQ ( (uint32_t)1, ts -> td [ 0 ] . dim );
    REQUIRE_EQ ( U32_id, ts -> td [ 1 ] . type_id );
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
    REQUIRE_EQ ( U8_id, ts -> td [ 0 ] . type_id );
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
    REQUIRE_EQ ( (uint32_t)1, f -> id );
}

FIXTURE_TEST_CASE(Format_Derived, AST_Fixture)
{
    MakeAst ( "fmtdef s; fmtdef s f;" );
    const KSymbol* sym = VerifySymbol ( "f", eFormat );
    const SFormat* f = static_cast < const SFormat* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( f );
    REQUIRE_EQ ( string ( "f" ), ToCppString ( f -> name -> name ) );
    REQUIRE_NOT_NULL ( f -> super );
    REQUIRE_EQ ( (uint32_t)2, f -> id );
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

class ConstFixture : public AST_Fixture
{
public:
    ConstFixture ()
    {
        // uncomment to run all const tests through the old parser
        //m_newParse = false;
    }

    const SExpression * VerifyConst ( const char * p_input, const char * p_id, uint32_t p_type )
    {
        MakeAst ( p_input );
        const KSymbol* sym = VerifySymbol ( p_id, eConstant );
        const SConstant* c = static_cast < const SConstant* > ( sym -> u . obj );
        THROW_ON_FALSE ( c != 0 );
        THROW_ON_FALSE ( string ( p_id ) == ToCppString ( c -> name -> name ) );
        THROW_ON_FALSE ( p_type == c -> td . type_id );
        THROW_ON_FALSE ( c -> expr != 0 );
        return c -> expr;
    }
    void VerifyIntConst ( const char * p_input, const char * p_id, uint32_t p_type, uint64_t p_expectedValue )
    {
        const SExpression * c = VerifyConst ( p_input, p_id, p_type );
        THROW_ON_FALSE ( ( uint32_t ) eConstExpr == c -> var );
        const SConstExpr * expr = reinterpret_cast < const SConstExpr * > ( c );
        THROW_ON_FALSE ( 1u == expr -> td . dim );
        THROW_ON_FALSE ( p_expectedValue == expr -> u . u64 [ 0 ] );
    }

    void VerifyFloatConst ( const char * p_input, const char * p_id, uint32_t p_type, double p_expectedValue )
    {
        const SExpression * c = VerifyConst ( p_input, p_id, p_type );
        THROW_ON_FALSE ( ( uint32_t ) eConstExpr == c -> var );
        const SConstExpr * expr = reinterpret_cast < const SConstExpr * > ( c );
        THROW_ON_FALSE ( 1u == expr -> td . dim );
        THROW_ON_FALSE ( p_expectedValue == expr -> u . f64 [ 0 ] );
    }

    void VerifyStringConst ( const char * p_input, const char * p_id, const string & p_expectedValue, size_t p_expectedLength )
    {
        const SExpression * c = VerifyConst ( p_input, p_id, ASCII_id );
        THROW_ON_FALSE ( ( uint32_t ) eConstExpr == c -> var );
        const SConstExpr * expr = reinterpret_cast < const SConstExpr * > ( c );
        THROW_ON_FALSE ( p_expectedLength == expr -> td . dim );
        THROW_ON_FALSE ( p_expectedValue == expr -> u . ascii );
    }
};

FIXTURE_TEST_CASE(Const_Decimal, ConstFixture)
{
    VerifyIntConst ( "const U8 c = 1;", "c", U8_id, 1 );
}

FIXTURE_TEST_CASE(Const_Hex, ConstFixture)
{
    VerifyIntConst ( "const U16 c = 0xF1;", "c", U16_id, 0xF1 );
}

FIXTURE_TEST_CASE(Const_Octal, ConstFixture)
{
    VerifyIntConst ( "const U32 c = 07070;", "c", U32_id, 07070 );
}

FIXTURE_TEST_CASE(Const_Float, ConstFixture)
{
    VerifyFloatConst ( "const F32 c = 1.0;", "c", F32_id, 1.0 );
}

FIXTURE_TEST_CASE(Const_ExpFloat, ConstFixture)
{
    VerifyFloatConst ( "const F64 c = 1.0e1;", "c", F64_id, 1.0e1 );
}

FIXTURE_TEST_CASE(Const_String, ConstFixture)
{
    VerifyStringConst ( "const ascii c = \"qq\";", "c", "qq", 2 );
}

FIXTURE_TEST_CASE(Const_EscapedString, ConstFixture)
{
    VerifyStringConst ( "const ascii c = \"qq\\n\\t\\r\\a\\b\\v\\f\\xFE\\X01\\0\";", "c", "qq\n\t\r\a\b\v\f\xfe\x01\0", 12 );
}

FIXTURE_TEST_CASE(Const_UnterminatedString, ConstFixture)
{
    // valgrind will show a leak of 9 bytes in 2 blocks, this is a side effect of bison's error recovery
    REQUIRE_THROW ( MakeAst ( "const ascii  c   =    \"qq\\n\\t\\r\\a\\b\\v\\f\\xFE\\X01\\0;" ) );
}

FIXTURE_TEST_CASE(Const_Vector, ConstFixture)
{
    const SExpression * c = VerifyConst ( "const U8 [2] c = [1,2];", "c", U8_id );
    THROW_ON_FALSE ( ( uint32_t ) eVectorExpr == c -> var );
    const SVectExpr * expr = reinterpret_cast < const SVectExpr * > ( c );
    REQUIRE_EQ ( (uint32_t)2, VectorLength ( & expr -> expr ) );
}

FIXTURE_TEST_CASE(Const_Vector_NonConstElement, ConstFixture)
{
    VerifyErrorMessage ( "function U8 f(); const U8 [2] c = [1,f()];", "Not a constant expression", 1, 38 );
}

FIXTURE_TEST_CASE(Const_Bool_True, ConstFixture)
{
    const SExpression * c = VerifyConst ( "const bool c = true;", "c", Bool_id );
    THROW_ON_FALSE ( ( uint32_t ) eConstExpr == c -> var );
    const SConstExpr * expr = reinterpret_cast < const SConstExpr * > ( c );
    REQUIRE ( expr -> u . b [ 0 ] );
}

FIXTURE_TEST_CASE(Const_Bool_False, ConstFixture)
{
    const SExpression * c = VerifyConst ( "const bool c = false;", "c", Bool_id );
    THROW_ON_FALSE ( ( uint32_t ) eConstExpr == c -> var );
    const SConstExpr * expr = reinterpret_cast < const SConstExpr * > ( c );
    REQUIRE ( ! expr -> u . b [ 0 ] );
}

FIXTURE_TEST_CASE(Const_Negative, ConstFixture)
{
    const SExpression * c = VerifyConst ( "const I8 c = - 1;", "c", I8_id );
    THROW_ON_FALSE ( ( uint32_t ) eConstExpr == c -> var );
    const SConstExpr * expr = reinterpret_cast < const SConstExpr * > ( c );
    REQUIRE_EQ ( ( int8_t ) -1, expr -> u . i8 [ 0 ] );
}

FIXTURE_TEST_CASE(Const_DoubleNegativeLiteral, ConstFixture)
{
    const SExpression * c = VerifyConst ( "const I8 c = - - 1;", "c", I8_id );
    THROW_ON_FALSE ( ( uint32_t ) eConstExpr == c -> var );
    const SConstExpr * expr = reinterpret_cast < const SConstExpr * > ( c );
    REQUIRE_EQ ( ( int8_t ) 1, expr -> u . i8 [ 0 ] );
}

FIXTURE_TEST_CASE(Const_DoubleNegative, ConstFixture)
{
    const SExpression * c = VerifyConst ( "const I8 c1 = 1; const I8 c2 = - - c1;", "c2", I8_id );
    THROW_ON_FALSE ( ( uint32_t ) eConstExpr == c -> var );
    const SConstExpr * expr = reinterpret_cast < const SConstExpr * > ( c );
    REQUIRE_EQ ( ( int8_t ) 1, expr -> u . i8 [ 0 ] );
}

//TODO: const negate non-const expr - error

//TODO: const I8 c = + 1;
//TODO: const I8 c = (I8) 1;
//TODO: const U8 c1; const U8 c2 = c1 + 1;

// error cases:
//TODO: vector const with a vector const subvalue - error
//TODO: vector const with a non-const subvalue - error
//TODO: not a type: const U8 c = 1; const c cc = 1; - error
//TODO: const U8 c = [1,2]; - error ( not an array )
//TODO: const U8 c[2] = 1; - error ( not a scalar )
//TODO: const U8 c = 1.0; - error ( wrong type ) ?
//TODO: extern function U8 f(); const U8 c2 = f(); - error ( not const )

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

// include
FIXTURE_TEST_CASE(Include, AST_Fixture)
{
    CreateFile ( "inc", "typedef U8 t;" );
    MakeAst ( "include \"inc\";" );
    VerifySymbol ( "t", eDatatype );
    remove ( "inc" );
}

FIXTURE_TEST_CASE(Include_SearchPath, AST_Fixture)
{
    CreateFile ( "./actual/inc", "typedef U8 t;" );
    m_builder -> AddIncludePath ( "./actual" );
    MakeAst ( "include \"inc\";" );
    VerifySymbol ( "t", eDatatype );
    remove ( "./actual/inc" );
}

FIXTURE_TEST_CASE(Include_MoreThanOnce, AST_Fixture)
{
    CreateFile ( "inc", "typedef U8 t;" );
    MakeAst ( "include \"inc\"; include \"inc\";" );
    VerifySymbol ( "t", eDatatype );
    remove ( "inc" );
}

FIXTURE_TEST_CASE(Include_NotFound, AST_Fixture)
{
    KDirectory *wd;
    REQUIRE_RC ( KDirectoryNativeDir ( & wd ) );
    char path[1024];
    REQUIRE_RC ( KDirectoryResolvePath_v1 ( wd, true, path, sizeof path, "./notinc" ) );
    VerifyErrorMessage ( "include \"notinc\";", ( string ( "Could not open include file: '" ) + path + "'" ) . c_str ()  );
    KDirectoryRelease ( wd );
}

//TODO: eror message from within an include

//TODO: array of arrays in typedef, typeset, constdef, return type
//TODO: array of typeset: typeset ts { U8 }; typedef ts t[2];

// Expressions

FIXTURE_TEST_CASE(FuncCall_PlainNoArgs, AST_Fixture)
{
    MakeAst  ( "function U8 f (); table t#1 { column U8 i = f (); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    REQUIRE_NOT_NULL ( t );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 0 ) );
    REQUIRE_NOT_NULL ( c );
    REQUIRE_NOT_NULL ( c -> read );

    // verify the function call expression
    REQUIRE_EQ ( ( uint32_t ) eFuncExpr, c -> read -> var );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( c -> read );
    struct SFunction const * func = expr -> func;
    REQUIRE_NOT_NULL ( func );

    // verrify the expression's fields
    REQUIRE_EQ ( 0u, VectorLength ( & expr -> schem ) );
    REQUIRE_EQ ( string ("f"), ToCppString ( func -> name -> name ) );
    REQUIRE_EQ ( 0u, VectorLength ( & expr -> pfact ) );
    REQUIRE_EQ ( 0u, VectorLength ( & expr -> pfunc ) );
    REQUIRE_EQ ( 0u, expr -> version );
    REQUIRE ( ! expr -> version_requested );
}

FIXTURE_TEST_CASE(FuncCall_SchemaParams, AST_Fixture)
{
    MakeAst  ( "function <type T, U8 i> T f (); table t#1 { column U8 i = <U8, 1> f (); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 0 ) );

    // verify the function call expression
    REQUIRE_EQ ( ( uint32_t ) eFuncExpr, c -> read -> var );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( c -> read );

    // verify the schema parameters
    REQUIRE_EQ ( 2u, VectorLength ( & expr -> schem ) );
    const SExpression * p1 = static_cast < const SExpression * > ( VectorGet ( & expr -> schem, 0 ) );
    REQUIRE_NOT_NULL ( p1 );
    REQUIRE_EQ ( ( uint32_t ) eTypeExpr, p1 -> var );
    const STypeExpr * te1 =  reinterpret_cast < const STypeExpr * > ( p1 );
    REQUIRE_NOT_NULL ( te1 );
    REQUIRE_NOT_NULL ( te1 -> dt );
    REQUIRE_EQ ( string ( "U8" ), ToCppString ( te1 -> dt -> name -> name ) );
    REQUIRE_NULL ( te1 -> dim );
    REQUIRE_EQ ( 1u, te1 -> fd . td . dim );

    const SExpression * p2 = static_cast < const SExpression * > ( VectorGet ( & expr -> schem, 1 ) );
    REQUIRE_NOT_NULL ( p2 );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, p2 -> var );
    const SConstExpr * te2 =  reinterpret_cast < const SConstExpr * > ( p2 );
    REQUIRE_NOT_NULL ( te2 );
    REQUIRE_EQ ( U64_id, te2 -> td . type_id );
    REQUIRE_EQ ( ( uint8_t )1u, te2 -> u . u8[0] );
}

FIXTURE_TEST_CASE(FuncCall_SchemaParams_Array, AST_Fixture)
{
    MakeAst  ( "function <type T> T f (); table t#1 { column U8 i = <U8[2]> f (); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 0 ) );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( c -> read );

    // verify the schema parameter
    REQUIRE_EQ ( 1u, VectorLength ( & expr -> schem ) );
    const SExpression * p1 = static_cast < const SExpression * > ( VectorGet ( & expr -> schem, 0 ) );
    REQUIRE_NOT_NULL ( p1 );
    REQUIRE_EQ ( ( uint32_t ) eTypeExpr, p1 -> var );
    const STypeExpr * te1 =  reinterpret_cast < const STypeExpr * > ( p1 );
    REQUIRE_NOT_NULL ( te1 );
    REQUIRE_NOT_NULL ( te1 -> dt );
    REQUIRE_EQ ( string ( "U8" ), ToCppString ( te1 -> dt -> name -> name ) );
    REQUIRE_EQ ( 1u, te1 -> dt -> dim );
    REQUIRE_NOT_NULL ( te1 -> dim );
    const SExpression * p2 = static_cast < const SExpression * > ( te1 -> dim );
    REQUIRE_NOT_NULL ( p2 );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, p2 -> var );
    const SConstExpr * te2 =  reinterpret_cast < const SConstExpr * > ( p2 );
    REQUIRE_NOT_NULL ( te2 );
    REQUIRE_EQ ( U64_id, te2 -> td . type_id );
    REQUIRE_EQ ( ( uint8_t )2u, te2 -> u . u8[0] );
}

FIXTURE_TEST_CASE(FuncCall_BadSchemaParam, AST_Fixture)
{
    VerifyErrorMessage  ( "function <type T> T f (); table t0#1 {}; table t#1 { column U8 i = <t0> f (); } ",
                          "Cannot be used as a schema parameter: 't0'", 1, 69 );
}

FIXTURE_TEST_CASE(FuncCall_NotAFunction, AST_Fixture)
{
    VerifyErrorMessage  ( "table t0#1 {}; table t#1 { column U8 i = t0 (); } ", "Not a function: 't0'" );
}

FIXTURE_TEST_CASE(FuncCall_Script, AST_Fixture)
{
    MakeAst  ( "function U8 f () { return 1; }; table t#1 { column U8 i = f (); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    REQUIRE_NOT_NULL ( t );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 0 ) );
    REQUIRE_NOT_NULL ( c );
    REQUIRE_NOT_NULL ( c -> read );

    REQUIRE_EQ ( ( uint32_t ) eScriptExpr, c -> read -> var );
}

FIXTURE_TEST_CASE(FuncCall_VersionNotSpecified, AST_Fixture)
{
    MakeAst  ( "function U8 f#1 (); function U8 f#2 (); table t#1 { column U8 i = f (); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 0 ) );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( c -> read );

    // highest version selected
    REQUIRE_EQ ( 0u, expr -> version );
    REQUIRE_EQ ( Version ( 2, 0, 0 ), expr -> func -> version );
}

FIXTURE_TEST_CASE(FuncCall_VersionSpecified, AST_Fixture)
{
    MakeAst  ( "function U8 f#1 (); function U8 f#2 (); table t#1 { column U8 i = f#1 (); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 0 ) );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( c -> read );

    // correct version selected
    REQUIRE ( expr -> version_requested );
    REQUIRE_EQ ( Version ( 1, 0, 0 ), expr -> version );
    REQUIRE_EQ ( expr -> version, expr -> func -> version );
}

FIXTURE_TEST_CASE(FuncCall_FactoryParams, AST_Fixture)
{
    MakeAst  ( "function U8 f <U16 i, U8 j> (); table t#1 { column U8 i =  f <1, 2> (); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 0 ) );

    // verify the function call expression
    REQUIRE_EQ ( ( uint32_t ) eFuncExpr, c -> read -> var );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( c -> read );

    // verify the factory parameters
    REQUIRE_EQ ( 2u, VectorLength ( & expr -> pfact ) );
    const SExpression * p1 = static_cast < const SExpression * > ( VectorGet ( & expr -> pfact, 0 ) );
    REQUIRE_NOT_NULL ( p1 );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, p1 -> var );
    const SConstExpr * te1 =  reinterpret_cast < const SConstExpr * > ( p1 );
    REQUIRE_NOT_NULL ( te1 );
    REQUIRE_EQ ( U64_id, te1 -> td . type_id );
    REQUIRE_EQ ( ( uint8_t )1u, te1 -> u . u8[0] );

    const SExpression * p2 = static_cast < const SExpression * > ( VectorGet ( & expr -> pfact, 1 ) );
    REQUIRE_NOT_NULL ( p2 );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, p2 -> var );
    const SConstExpr * te2 =  reinterpret_cast < const SConstExpr * > ( p2 );
    REQUIRE_NOT_NULL ( te2 );
    REQUIRE_EQ ( U64_id, te2 -> td . type_id );
    REQUIRE_EQ ( ( uint8_t )2u, te2 -> u . u8[0] );
}

FIXTURE_TEST_CASE(FuncCall_FunctionAsFactoryParam, AST_Fixture)
{
    if ( m_newParse )
    {
        VerifyErrorMessage ( "function U8 g(); function U8 f <U16 i> (); table t#1 { column U8 i = f <g> (); } ",
                             "Function expressions are not yet implemented", 1, 73 );
    }
    // the old parser accepts but errors out at run time
}

FIXTURE_TEST_CASE(FuncCall_BadFactoryParam, AST_Fixture)
{
    VerifyErrorMessage ( "table g#1{} function U8 f <U16 i> (); table t#1 { column U8 i =  f <g> (); } ",
                         "Object cannot be used in this context: 'g'", 1, 69 );
}

//TODO: no tests on optional/variable factory parameters: the parser allows any number/type and (hopefully)
// does some checking in the run time. Need to revisit when testing run time logic.

FIXTURE_TEST_CASE(FuncCall_Params, AST_Fixture)
{
    MakeAst  ( "function U8 f (U16 i, U8 j); table t#1 { column U8 a; column U8 b; column U8 c = f (a, b); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 2 ) );

    // verify the function call expression
    REQUIRE_EQ ( ( uint32_t ) eFuncExpr, c -> read -> var );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( c -> read );

    // verify the function parameters
    REQUIRE_EQ ( 2u, VectorLength ( & expr -> pfunc ) );
    const SExpression * p1 = static_cast < const SExpression * > ( VectorGet ( & expr -> pfunc, 0 ) );
    REQUIRE_NOT_NULL ( p1 );
    REQUIRE_EQ ( ( uint32_t ) eColExpr, p1 -> var );
    const SSymExpr * te1 =  reinterpret_cast < const SSymExpr * > ( p1 );
    REQUIRE_NOT_NULL ( te1 );
    REQUIRE_EQ ( string ( "a" ), ToCppString ( te1 -> _sym -> name ) );

    const SExpression * p2 = static_cast < const SExpression * > ( VectorGet ( & expr -> pfunc, 1 ) );
    REQUIRE_NOT_NULL ( p2 );
    REQUIRE_EQ ( ( uint32_t ) eColExpr, p2 -> var );
    const SSymExpr * te2 =  reinterpret_cast < const SSymExpr * > ( p2 );
    REQUIRE_NOT_NULL ( te2 );
    REQUIRE_EQ ( string ( "b" ), ToCppString ( te2 -> _sym -> name ) );
}

FIXTURE_TEST_CASE(FuncCall_Param_At, AST_Fixture)
{
    MakeAst  (
        "extern function U8 f ( U8 in );"
        "physical U8 ns:pf #1 = { return f ( @ ); }" // the old parser requires the namespace
    );
    const SPhysical * phys = static_cast < const SPhysical* > ( VectorGet ( & m_schema -> phys, 0 ) );
    REQUIRE_NOT_NULL ( phys );
    const SExpression * ret = phys -> decode . u . script . rtn;
    REQUIRE_NOT_NULL ( ret );
    REQUIRE_EQ ( (uint32_t) eFuncExpr, ret -> var );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( ret );
    REQUIRE_EQ ( 1u, VectorLength ( & expr -> pfunc ) );
    const SExpression * p1 = static_cast < const SExpression * > ( VectorGet ( & expr -> pfunc, 0 ) );
    REQUIRE_NOT_NULL ( p1 );
    REQUIRE_EQ ( (uint32_t) eParamExpr, p1 -> var );
    const SSymExpr * sym = reinterpret_cast < const SSymExpr * > ( p1 );
    REQUIRE_EQ ( string ( "@" ), ToCppString ( sym -> _sym -> name ) );
}

FIXTURE_TEST_CASE(FuncCall_Param_Physical, AST_Fixture)
{
    MakeAst  ( "function U8 f (U8 i); table t#1 { physical column U8 .a; column U8 b = f (.a); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 0 ) );

    // verify the function call expression
    REQUIRE_EQ ( ( uint32_t ) eFuncExpr, c -> read -> var );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( c -> read );

    // verify the function parameters
    REQUIRE_EQ ( 1u, VectorLength ( & expr -> pfunc ) );
    const SExpression * p1 = static_cast < const SExpression * > ( VectorGet ( & expr -> pfunc, 0 ) );
    REQUIRE_NOT_NULL ( p1 );
    REQUIRE_EQ ( ( uint32_t ) ePhysExpr, p1 -> var );
    const SSymExpr * sym =  reinterpret_cast < const SSymExpr * > ( p1 );
    REQUIRE_EQ ( string ( ".a" ), ToCppString ( sym -> _sym -> name ) );
}

FIXTURE_TEST_CASE(FuncCall_Param_Member, AST_Fixture)
{
    MakeAst  ( "version 2; function U8 f (U8 i); table t#1 { column U8 a; } view v#1 < t p_t> { column U8 c = f(p_t.a); }" );
}

FIXTURE_TEST_CASE(FuncCall_OptionalParams, AST_Fixture)
{
    MakeAst  ( "function U8 f (U16 i, * U8 j); table t#1 { column U8 a; column U8 b; column U8 c = f (a); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 2 ) );
    REQUIRE_EQ ( ( uint32_t ) eFuncExpr, c -> read -> var );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( c -> read );
    REQUIRE_EQ ( 1u, VectorLength ( & expr -> pfunc ) );
}

FIXTURE_TEST_CASE(FuncCall_Vararg, AST_Fixture)
{
    MakeAst  ( "function U8 f (U16 i, ...); table t#1 { column U8 a; column U8 b; column U8 c = f (a, a, a); } " );
    const STable * t = static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, 0 ) );
    const SColumn * c = static_cast < const SColumn * > ( VectorGet ( & t -> col, 2 ) );
    REQUIRE_EQ ( ( uint32_t ) eFuncExpr, c -> read -> var );
    const SFuncExpr * expr = reinterpret_cast < const SFuncExpr * > ( c -> read );
    REQUIRE_EQ ( 3u, VectorLength ( & expr -> pfunc ) );
}

FIXTURE_TEST_CASE(ChildSchema_AccessToParentsSchemaNames, AST_Fixture)
{
    MakeAst  ( "typedef U8 NS:T;"  );
    VTypedecl resolved_dad;
    REQUIRE_RC ( VSchemaResolveTypedecl ( m_schema, & resolved_dad, "NS:T" ) );

    VSchema * child;
    REQUIRE_RC ( VSchemaMake ( &child, m_schema ) );
    string source = "typedef U8 NS:T;";
    REQUIRE_RC ( VSchemaParseText ( child, 0, source . c_str (), source . size () ) );
    VTypedecl resolved_child;
    REQUIRE_RC ( VSchemaResolveTypedecl ( child, & resolved_child, "NS:T" ) );

    REQUIRE_EQ ( resolved_dad . type_id, resolved_child . type_id );
    REQUIRE_RC ( VSchemaRelease ( child ) );
}

FIXTURE_TEST_CASE(ChildSchema_AccessToParentsNames, AST_Fixture)
{   // VDB-3635: partial copy of a namespace to the child schema obscures members left behind
    MakeAst  ( "typedef U8 NS:b:T; extern function U8 NS:a:fn();"  );
    VTypedecl resolved_dad;
    REQUIRE_RC ( VSchemaResolveTypedecl ( m_schema, & resolved_dad, "NS:b:T" ) );

    VSchema * child;
    REQUIRE_RC ( VSchemaMake ( &child, m_schema ) );
    string source = "typedef U8 NS:b:T;";
    REQUIRE_RC ( VSchemaParseText ( child, 0, source . c_str (), source . size () ) );
    VTypedecl resolved_child;
    REQUIRE_RC ( VSchemaResolveTypedecl ( child, & resolved_child, "NS:b:T" ) );

    REQUIRE_EQ ( resolved_dad . type_id, resolved_child . type_id );
    REQUIRE_RC ( VSchemaRelease ( child ) );
}

//TODO: invalid float
//TODO: nested vector constants - error
//TODO: negation applied to non-scalar - error
//TODO: eFwdExpr
//TODO: eCastExpr
//TODO: eVectorExpr
//TODO: eCondExpr

#include "wb-test-schema-func.cpp"
#include "wb-test-schema-table.cpp"
#include "wb-test-schema-db.cpp"
#include "wb-test-schema-view.cpp"

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

