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

#include <ktst/unit_test.hpp>

#include <klib/symbol.h>

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

TEST_CASE ( AST_ParamSig_Empty )
{
    SchemaToken id = { PT_FUNCPARAMS, 0, 0, 0, 0 };
    Token tok ( id );
    AST_ParamSig* sig = new AST_ParamSig ( & tok, 0, 0, false );
    delete sig;
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
    REQUIRE_NULL ( m_builder . Resolve ( * id ) ); delete id;
    id = AST_Fixture :: MakeFqn ( "bar" );
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

FIXTURE_TEST_CASE(Const_Simple, AST_Fixture)
{
    MakeAst ( "const U8 c = 1;" );
    const KSymbol* sym = VerifySymbol ( "c", eConstant );
    const SConstant* c = static_cast < const SConstant* > ( sym -> u . obj );
    REQUIRE_NOT_NULL ( c );
    REQUIRE_EQ ( string ( "c" ), ToCppString ( c -> name -> name ) );
    REQUIRE_EQ ( U8_id, c -> td . type_id );
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

//TODO: array of arrays in typedef, typeset, constdef, return type

//TODO: function call: no arguments
//TODO: function call: optional parameters
//TODO: function call: vararg

//TODO: formatted type in production in a table (not allowed?)

#include "wb-test-schema-func.cpp"

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

