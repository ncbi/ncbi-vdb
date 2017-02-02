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
* Unit tests for schema AST, this file is #included into a bigger test suite
*/

#include <ktst/unit_test.hpp>

#include <klib/symbol.h>

#include <vdb/manager.h>

#include "../../libs/vdb/schema-expr.h"

#include "AST_Fixture.hpp"

using namespace std;
using namespace ncbi::NK;

TEST_SUITE ( ASTFuctionTestSuite );

class AST_Function_Fixture : public AST_Fixture
{
public:

    class FunctionAccess // encapsulates access to an SFunction in a VSchema
    {
    public:
        FunctionAccess ( const SFunction* p_fn )
        : m_fn ( p_fn )
        {
        }

        const STypeExpr *   ReturnType () const { return reinterpret_cast < const STypeExpr * > ( m_fn -> rt ); }
        uint32_t            Version () const { return m_fn -> version; }
        uint16_t            MandatoryParamCount () const { return m_fn -> func . mand; }
        bool                IsVariadic () const { return m_fn -> func . vararg != 0 ; }
        uint32_t            ParamCount () const { return VectorLength ( & m_fn -> func . parms ); }
        const SProduction * GetParam ( uint32_t p_idx ) const
        {
            return static_cast < const SProduction * > ( VectorGet ( & m_fn -> func . parms, p_idx ) );
        }
        uint32_t SchemaTypeParamCount () const { return VectorLength ( & m_fn -> type ); }
        uint32_t SchemaConstParamCount () const { return VectorLength ( & m_fn -> schem ); }
        const SIndirectType* GetSchemaTypeParam ( uint32_t p_idx ) const
        {
            return static_cast < const SIndirectType* > ( VectorGet ( & m_fn -> type, p_idx ) );
        }
        const SIndirectConst* GetSchemaConstParam ( uint32_t p_idx ) const
        {
            return static_cast < const SIndirectConst* > ( VectorGet ( & m_fn -> schem, p_idx ) );
        }

        const SFunction* m_fn;
    };

    FunctionAccess GetFunction ( const char* p_name, uint32_t p_type = eFunction )
    {
        const KSymbol* sym = VerifySymbol ( p_name, p_type );

        // for functions, sym points to an entry in the overloads table (schema->fname)
        const SNameOverload* name = static_cast < const SNameOverload* > ( sym -> u . obj );
        if ( 1u != VectorLength ( & name -> items ) )
        {
            throw std :: logic_error ( "AST_Function_Fixture::GetFunction : too many overloads" );
        }
        const SFunction* ret = static_cast < const SFunction* > ( VectorGet ( & name -> items, 0 ) );
        if ( string ( p_name ) != ToCppString ( ret -> name -> name ) )
        {
            throw std :: logic_error ( "AST_Function_Fixture::GetFunction : wrong name" );
        }

        return FunctionAccess ( ret );
    }
    FunctionAccess GetOverload ( const char* p_name, uint32_t p_idx = 0 )
    {
        const KSymbol* sym = VerifySymbol ( p_name, eFunction );

        // for functions, sym points to an entry in the overloads table (schema->fname)
        const SNameOverload* name = static_cast < const SNameOverload* > ( sym -> u . obj );
        if ( p_idx >= VectorLength ( & name -> items ) )
        {
            throw std :: logic_error ( "AST_Function_Fixture::GetOverload : index out of range" );
        }

        return FunctionAccess ( static_cast < const SFunction * > ( VectorGet ( & name -> items, p_idx ) ) );
    }
};

// untyped

FIXTURE_TEST_CASE(Func_Untyped, AST_Function_Fixture)
{
    MakeAst ( "function __untyped f();" );
    FunctionAccess fn = GetFunction ( "f", eUntypedFunc );
    REQUIRE_NULL ( fn . ReturnType () );
}

FIXTURE_TEST_CASE(Func_UntypedRedeclared, AST_Function_Fixture)
{
    MakeAst ( "function __untyped f(); function __untyped f();" );
    FunctionAccess fn = GetFunction ( "f", eUntypedFunc );
    REQUIRE_NULL ( fn . ReturnType () );
}

// row_length

FIXTURE_TEST_CASE(Func_Untyped_Exists, AST_Function_Fixture)
{
    VerifyErrorMessage ( "typedef U8 f; function __untyped f();", "Declared earlier with a different type: 'f'" );
}

FIXTURE_TEST_CASE(Func_Rowlength, AST_Function_Fixture)
{   // implemented by the same code as untyped, the basic test case will suffice
    MakeAst ( "function __row_length f();" );
    FunctionAccess fn = GetFunction ( "f", eRowLengthFunc );
    REQUIRE_NULL ( fn . ReturnType () );
}

// return type

FIXTURE_TEST_CASE(Func_Scalar_NoParams, AST_Function_Fixture)
{
    MakeAst ( "function U8 f();" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( 1u,    fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_Array_NoParams, AST_Function_Fixture)
{
    MakeAst ( "function U8[2] f();" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( 2u,    fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_VarArray_NoParams, AST_Function_Fixture)
{
    MakeAst ( "function U8[*] f();" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( 0u,    fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_ReturnArray, AST_Function_Fixture)
{
    MakeAst ( "function U8[10] f();" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( U8_id,         fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( (uint32_t)10,  fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_ReturnTypeset, AST_Function_Fixture)
{
    MakeAst ( "typeset ts { U8 }; function ts f();" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( (uint32_t)0x40000000,  fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( (uint32_t)1,           fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_ReturnFormat, AST_Function_Fixture)
{
    MakeAst ( "fmtdef fmt; function fmt f();" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( (uint32_t)1, fn . ReturnType () -> fmt -> id );
    REQUIRE_EQ ( (uint32_t)1, fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( (uint32_t)1, fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_ReturnBad, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U8 f1(); function f1 f2();", "Not a datatype: 'f1'" );
}

// version, overload

FIXTURE_TEST_CASE(Func_WithVersion, AST_Function_Fixture)
{
    MakeAst ( "function U8 f#1.2();" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( (uint32_t) ( 1 << 24 ) | ( 2 << 16 ), fn . Version () );
}

FIXTURE_TEST_CASE(Func_Redeclared_NoVersion, AST_Function_Fixture)
{
    MakeAst ( "function U8 f();function U16 f();" );
    FunctionAccess fn = GetOverload ( "f", 0 );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id ); // 2nd decl with the same version ignored
    REQUIRE_EQ ( 1u,    fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_Redeclared_SameVersion, AST_Function_Fixture)
{
    MakeAst ( "function U8 f#1.2();function U16 f#1.2();" );
    FunctionAccess fn = GetOverload ( "f", 0 );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id ); // 2nd decl with the same version ignored
}

FIXTURE_TEST_CASE(Func_Redeclared_DiffMajor, AST_Function_Fixture)
{   // 2 overloads created
    MakeAst ( "function U8 f#2();function U16 f#1.2();" );
    {
        FunctionAccess fn = GetOverload ( "f", 0 ); // lower version
        REQUIRE_EQ ( U16_id, fn . ReturnType () -> fd . td . type_id ); // U16
    }
    {
        FunctionAccess fn = GetOverload ( "f", 1 );    // higher version
        REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id ); // U8
    }
}

FIXTURE_TEST_CASE(Func_Redeclared_HigherMinor, AST_Function_Fixture)
{   // higner minor is used
    MakeAst ( "function U8 f#1();function U16 f#1.2();" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( U16_id, fn . ReturnType () -> fd . td . type_id ); // 2nd decl used
}

FIXTURE_TEST_CASE(Func_NoReleaseNumberForSimpleFunctions, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U16 f#1.2.3();", "Release number is not allowed for simple function: 'f'" );
}

// formal parameters

FIXTURE_TEST_CASE(Func_BadParamType, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U8 f1(); function U8 f2( f1 p );", "Not a datatype: 'f1'" );
}
FIXTURE_TEST_CASE(Func_BadParamName, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U8 f(U8 U16);", "Cannot be used as a formal parameter name: 'U16'" );
}

FIXTURE_TEST_CASE(Func_OneParamScalar, AST_Function_Fixture)
{
    MakeAst ( "function U8 f(U8 p);" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( (uint16_t)1, fn . MandatoryParamCount () );
    REQUIRE ( ! fn .  IsVariadic () );
    REQUIRE_EQ ( (uint32_t)1, fn . ParamCount () );
    const SProduction * param = fn . GetParam ( 0 );
    REQUIRE_NOT_NULL ( param );
    REQUIRE_EQ ( string ( "p" ), string ( param -> name -> name . addr ) );
    REQUIRE_NULL ( param -> expr );
    REQUIRE_NOT_NULL ( param -> fd );
    const STypeExpr * type = reinterpret_cast < const STypeExpr * > ( param -> fd );
    REQUIRE_EQ ( (uint32_t)1, type -> fd . td. dim );
    REQUIRE_EQ ( (uint32_t)0, param -> cid . id );
    REQUIRE ( ! param -> trigger );
    REQUIRE ( ! param -> control );
}

FIXTURE_TEST_CASE(Func_OneParamArray, AST_Function_Fixture)
{
    MakeAst ( "function U8 f(U8[2] p);" );
    FunctionAccess fn = GetFunction ( "f" );
    const SProduction * param = fn . GetParam ( 0 );
    const STypeExpr * type = reinterpret_cast < const STypeExpr * > ( param -> fd );
    REQUIRE_EQ ( (uint32_t)2, type -> fd . td. dim );
}

FIXTURE_TEST_CASE(Func_OneParamVarArray, AST_Function_Fixture)
{
    MakeAst ( "function U8 f(U8[*] p);" );
    FunctionAccess fn = GetFunction ( "f" );
    const SProduction * param = fn . GetParam ( 0 );
    const STypeExpr * type = reinterpret_cast < const STypeExpr * > ( param -> fd );
    REQUIRE_EQ ( (uint32_t)0, type -> fd . td. dim );
}

FIXTURE_TEST_CASE(Func_ControlParam, AST_Function_Fixture)
{
    MakeAst ( "function U8 f( control U8 p);" );
    FunctionAccess fn = GetFunction ( "f" );
    const SProduction * param = fn . GetParam ( 0 );
    REQUIRE ( param -> control );
}

FIXTURE_TEST_CASE(Func_TwoParamsSameName, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U8 f(U8 p, U16 p);", "Cannot be used as a formal parameter name: 'p'" );
}

FIXTURE_TEST_CASE(Func_TwoParams, AST_Function_Fixture)
{
    MakeAst ( "function U8 f(U8 p1, U16 p2);" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( (uint16_t)2, fn . MandatoryParamCount () );
    REQUIRE ( ! fn .  IsVariadic () );
    REQUIRE_EQ ( (uint32_t)2, fn . ParamCount () );
    const SProduction * param = fn . GetParam ( 0 );
    REQUIRE_NOT_NULL ( param );
    REQUIRE_EQ ( string ( "p1" ), string ( param -> name -> name . addr ) );
    param = fn . GetParam ( 1 );
    REQUIRE_NOT_NULL ( param );
    REQUIRE_EQ ( string ( "p2" ), string ( param -> name -> name . addr ) );
}

// schema signature

FIXTURE_TEST_CASE(Func_SchemaParam_Type, AST_Function_Fixture)
{
    MakeAst ( "function < type T > T f ( T p1 );" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( (uint32_t)1, fn . SchemaTypeParamCount () );
    REQUIRE_EQ ( (uint32_t)0, fn . SchemaConstParamCount () );

    const SIndirectType* param = fn . GetSchemaTypeParam ( 0 );
    REQUIRE_NOT_NULL ( param );
    REQUIRE_EQ ( string ( "T" ), string ( param -> name -> name . addr ) );
    REQUIRE_EQ ( (uint32_t)1, param -> type_id );
    REQUIRE_EQ ( (uint32_t)0x80000001, param -> id );
    REQUIRE_EQ ( (uint32_t)0, param -> pos );
}

FIXTURE_TEST_CASE(Func_SchemaParam_Value, AST_Function_Fixture)
{
    MakeAst ( "function < U32 T > U8 f ( U8 p );" );
    FunctionAccess fn = GetFunction ( "f" );
    REQUIRE_EQ ( (uint32_t)0, fn . SchemaTypeParamCount () );
    REQUIRE_EQ ( (uint32_t)1, fn . SchemaConstParamCount () );

    const SIndirectConst* param = fn . GetSchemaConstParam ( 0 );
    REQUIRE_NOT_NULL ( param );
    REQUIRE_EQ ( string ( "T" ), string ( param -> name -> name . addr ) );
    REQUIRE_NOT_NULL ( param -> td );
    REQUIRE_EQ ( (uint32_t)1, param -> expr_id );
    REQUIRE_EQ ( (uint32_t)0, param -> pos );
}

FIXTURE_TEST_CASE(Func_SchemaParam_NotType, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U8 notatype(); function < notatype X > U8 f ();", "Not a datatype: 'notatype'" );
}
FIXTURE_TEST_CASE(Func_SchemaParam_NotInt, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function < ascii X > U8 f ();", "Not a scalar unsigned integer: 'X'" );
}
FIXTURE_TEST_CASE(Func_SchemaParam_IntArray, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function < U8[2] X > U8 f ();", "Not a scalar unsigned integer: 'X'" );
}
FIXTURE_TEST_CASE(Func_SchemaParam_IntPseudoArray, AST_Function_Fixture)
{
    MakeAst ( "function < U8[1] X > U8 f ();" );
    FunctionAccess fn = GetFunction ( "f" );
    const SIndirectConst* param = fn . GetSchemaConstParam ( 0 );
    const STypeExpr * type = reinterpret_cast < const STypeExpr* > ( param -> td );
    REQUIRE_EQ ( U8_id, type -> fd . td . type_id );
    REQUIRE_EQ ( (uint32_t)1, type -> fd . td . dim );
}
FIXTURE_TEST_CASE(Func_SchemaParam_UnresolvedTypeUsed, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function < type T, T X > U8 f ();", "Not a scalar unsigned integer: 'X'" );
}

FIXTURE_TEST_CASE(Func_SchemaParam_UsedInReturn, AST_Function_Fixture)
{
    MakeAst ( "function < U32 T > U8[T] f ( U8 p );" );
    FunctionAccess fn = GetFunction ( "f" );
    const STypeExpr * type = fn. ReturnType ();
    REQUIRE_EQ ( U8_id, type -> fd . td . type_id );
    REQUIRE ( ! type -> resolved );
    REQUIRE_EQ ( (uint32_t)0, type -> fd . td. dim );
}

FIXTURE_TEST_CASE(Func_SchemaParam_UsedInParam, AST_Function_Fixture)
{
    MakeAst ( "function < U32 T > U8 f ( U8[T] p );" );
    FunctionAccess fn = GetFunction ( "f" );
    const SProduction * param = fn . GetParam ( 0 );
    const STypeExpr * type = reinterpret_cast < const STypeExpr * > ( param -> fd );
    REQUIRE_EQ ( U8_id, type -> fd . td . type_id );
    REQUIRE ( ! type -> resolved );
    REQUIRE_EQ ( (uint32_t)0, type -> fd . td. dim );
}

// sandbox for exploring the old parsing logic
FIXTURE_TEST_CASE(Old_Parse, AST_Function_Fixture)
{
    const VDBManager *mgr;
    REQUIRE_RC ( VDBManagerMakeRead ( & mgr, 0 ) );
    VSchema *schema;
    REQUIRE_RC ( VDBManagerMakeSchema ( mgr, & schema ) );

    string input = "function < U32 T > U8[T] f ( U8 p );";

    REQUIRE_RC ( VSchemaParseText ( schema, 0, input . c_str (), input . length () ) );

    const SFunction* fn = static_cast < const SFunction* > ( VectorGet ( & schema -> func, 0 ) );
    REQUIRE_EQ ( U8_id, ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . type_id );
    REQUIRE ( ! ( ( STypeExpr * ) ( fn -> rt ) ) -> resolved );
    REQUIRE_EQ ( 0u,    ( ( STypeExpr * ) ( fn -> rt ) ) -> fd . td . dim );

    VSchemaRelease ( schema );
    VDBManagerRelease ( mgr );
}

//TODO: optional parameters
//TODO: vararg

//TODO: factory parameters

//TODO: script + untyped - error
//TODO: validate + untyped - error
//TODO: script fn without a body - error
//TODO: non-script fn with a body - error
//TODO: validate + non-void return - error
//TODO: non-validate + void return - error

//TODO: release number for schema function (#1.2.?)
//TODO: release number for extern function (#1.2.?)
//TODO: release number for validate function (#1.2.?)
