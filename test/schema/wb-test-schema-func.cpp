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
* Unit tests for function declarations in schema, this file is #included into a bigger test suite
*/

class FunctionAccess // encapsulates access to an SFunction in a VSchema
{
public:
    FunctionAccess ( const SFunction* p_fn )
    : m_self ( p_fn )
    {
    }

    const STypeExpr *   ReturnType () const { return reinterpret_cast < const STypeExpr * > ( m_self -> rt ); }
    uint32_t            Version () const { return m_self -> version; }

    uint16_t            MandatoryParamCount () const { return m_self -> func . mand; }
    bool                IsVariadic () const { return m_self -> func . vararg != 0 ; }
    uint32_t            ParamCount () const { return VectorLength ( & m_self -> func . parms ); }
    const SProduction * GetParam ( uint32_t p_idx ) const
    {
        return static_cast < const SProduction * > ( VectorGet ( & m_self -> func . parms, p_idx ) );
    }

    uint32_t SchemaTypeParamCount () const { return VectorLength ( & m_self -> type ); }
    uint32_t SchemaConstParamCount () const { return VectorLength ( & m_self -> schem ); }
    const SIndirectType* GetSchemaTypeParam ( uint32_t p_idx ) const
    {
        return static_cast < const SIndirectType* > ( VectorGet ( & m_self -> type, p_idx ) );
    }
    const SIndirectConst* GetSchemaConstParam ( uint32_t p_idx ) const
    {
        return static_cast < const SIndirectConst* > ( VectorGet ( & m_self -> schem, p_idx ) );
    }

    uint16_t            FactoryMandatoryParamCount () const { return m_self -> fact . mand; }
    bool                FactoryIsVariadic () const { return m_self -> fact . vararg != 0 ; }
    uint32_t            FactoryParamCount () const { return VectorLength ( & m_self -> fact . parms ); }
    const SIndirectConst * FactoryGetParam ( uint32_t p_idx ) const
    {
        return static_cast < const SIndirectConst * > ( VectorGet ( & m_self -> fact . parms, p_idx ) );
    }

    const KSymbol * FactoryId () const { return m_self -> u . ext . fact; }

    bool IsScript () const { return m_self -> script; }
    bool IsValidate () const { return m_self -> validate; }
    bool IsUntyped () const { return m_self -> untyped; }
    bool IsRowLength () const { return m_self -> row_length; }

    const SExpression * ReturnExpr () const { return m_self -> u . script . rtn; }

    uint32_t ProductionCount () const { return VectorLength ( & m_self -> u . script . prod ); }
    const SProduction * Production ( uint32_t p_idx ) const
    {
        return static_cast < const SProduction * > ( VectorGet ( & m_self -> u . script . prod, p_idx ) );
    }

    const SFunction* m_self;
};

class PhysicalAccess;

class AST_Function_Fixture : public AST_Fixture
{
public:

public:
    AST_Function_Fixture ()
    {
    }
    ~AST_Function_Fixture ()
    {
    }

    FunctionAccess ParseFunction ( const char * p_source, const char * p_name, uint32_t p_idx = 0, uint32_t p_type = eFunction )
    {
        const SFunction* ret = 0;
        MakeAst ( p_source );
        if ( m_newParse )
        {
            const KSymbol* sym = VerifySymbol ( p_name, p_type );

            // for functions, sym points to an entry in the overloads table (schema->fname)
            const SNameOverload* name = static_cast < const SNameOverload* > ( sym -> u . obj );
            if ( 1u != VectorLength ( & name -> items ) )
            {
                throw std :: logic_error ( "AST_Function_Fixture::ParseFunction : too many overloads" );
            }
            ret = static_cast < const SFunction* > ( VectorGet ( & name -> items, 0 ) );
            if ( ret -> name == 0 )
            {
                throw std :: logic_error ( "AST_Function_Fixture::ParseFunction : NULL name" );
            }
            if ( string ( p_name ) != ToCppString ( ret -> name -> name ) )
            {
                throw std :: logic_error ( "AST_Function_Fixture::ParseFunction : wrong name" );
            }
        }
        else
        {
            ret = static_cast < const SFunction* > ( VectorGet ( & m_schema -> func, p_idx ) );

            if ( string ( p_name ) != ToCppString ( ret -> name -> name ) )
            {
                throw std :: logic_error ( "AST_Function_Fixture::ParseFunction : wrong name" );
            }
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

    PhysicalAccess ParsePhysical ( const char * p_source, const char * p_name );
};

// untyped

FIXTURE_TEST_CASE(Func_Untyped, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function __untyped f();", "f", 0, eUntypedFunc );
    REQUIRE ( fn . IsUntyped () );
    REQUIRE_NULL ( fn . ReturnType () );
}

FIXTURE_TEST_CASE(Func_UntypedRedeclared, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function __untyped f(); function __untyped f();", "Declared earlier and cannot be overloaded: 'f'");
}

FIXTURE_TEST_CASE(Func_Untyped_Exists, AST_Function_Fixture)
{
    VerifyErrorMessage ( "typedef U8 f; function __untyped f();", "Declared earlier and cannot be overloaded: 'f'" );
}

// row_length

FIXTURE_TEST_CASE(Func_Rowlength, AST_Function_Fixture)
{   // implemented by the same code as untyped, the basic test case will suffice
    FunctionAccess fn = ParseFunction ( "function __row_length f();", "f", 0, eRowLengthFunc );
    REQUIRE ( fn . IsRowLength () );
    REQUIRE_NULL ( fn . ReturnType () );
}

// return type

FIXTURE_TEST_CASE(Func_Scalar_NoParams, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8 f();", "f" );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( 1u,    fn . ReturnType () -> fd . td . dim );

    REQUIRE ( ! fn . IsScript () );
    REQUIRE ( ! fn . IsValidate () );
    REQUIRE ( ! fn . IsUntyped () );
    REQUIRE ( ! fn . IsRowLength () );
}

FIXTURE_TEST_CASE(Func_Array_NoParams, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8[2] f();", "f" );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( 2u,    fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_VarArray_NoParams, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8[*] f();", "f" );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( 0u,    fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_ReturnArray, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8[10] f();", "f" );
    REQUIRE_EQ ( U8_id,         fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( (uint32_t)10,  fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_ReturnTypeset, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "typeset ts { U8 }; function ts f();", "f" );
    REQUIRE_EQ ( (uint32_t)0x40000000,  fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( (uint32_t)1,           fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_ReturnFormat, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "fmtdef fmt; function fmt f();", "f" );
    REQUIRE_EQ ( (uint32_t)1, fn . ReturnType () -> fmt -> id );
    REQUIRE_EQ ( (uint32_t)1, fn . ReturnType () -> fd . fmt );
}

FIXTURE_TEST_CASE(Func_ReturnFormatted, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "fmtdef fmt; function fmt / U8 f();", "f" );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( 1u,    fn . ReturnType () -> fd . td . dim );
    REQUIRE_EQ ( 1u,    fn . ReturnType () -> fd . fmt );
    REQUIRE_EQ ( 1u,    fn . ReturnType () -> fmt -> id );
}

FIXTURE_TEST_CASE(Func_ReturnBad, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U8 f1(); function f1 f2();", "Not a datatype: 'f1'" );
}

// version, overload

FIXTURE_TEST_CASE(Func_WithVersion, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8 f#1.2.3();", "f" );
    REQUIRE_EQ ( (uint32_t) ( 1 << 24 ) | ( 2 << 16 ) | 3, fn . Version () );
}

FIXTURE_TEST_CASE(Func_Redeclared_NoVersion, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8 f();function U16 f();", "f" );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id ); // 2nd decl with the same version ignored
    REQUIRE_EQ ( 1u,    fn . ReturnType () -> fd . td . dim );
}

FIXTURE_TEST_CASE(Func_Redeclared_SameVersion, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8 f#1.2();function U16 f#1.2();", "f" );
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
    if ( m_newParse ) // the old parser fails here!
    {
        FunctionAccess fn = ParseFunction ( "function U8 f#1();function U16 f#1.2();",  "f" );
        REQUIRE_EQ ( U16_id, fn . ReturnType () -> fd . td . type_id ); // 2nd decl used
    }
}

FIXTURE_TEST_CASE(Func_NoChangingReleaseNumberForSimpleFunctions, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U16 f#1(); function U16 f#1.2.4();", "Changing release number is not allowed: 'f'" );
}

// formal parameters

FIXTURE_TEST_CASE(Func_BadParamType, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U8 f1(); function U8 f2( f1 p );", "Not a datatype: 'f1'" );
}
FIXTURE_TEST_CASE(Func_BadParamName, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U8 f(U8 U16);", "Name already in use: 'U16'", 1, 18 );
}

FIXTURE_TEST_CASE(Func_OneParamScalar, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 f(U8 p);", "f" );
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
    FunctionAccess fn = ParseFunction ( "function U8 f(U8[2] p);", "f" );
    const SProduction * param = fn . GetParam ( 0 );
    REQUIRE_NOT_NULL ( param );
    const STypeExpr * type = reinterpret_cast < const STypeExpr * > ( param -> fd );
    REQUIRE_EQ ( (uint32_t)2, type -> fd . td. dim );
}

FIXTURE_TEST_CASE(Func_OneParamVarArray, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8 f(U8[*] p);", "f" );
    const SProduction * param = fn . GetParam ( 0 );
    REQUIRE_NOT_NULL ( param );
    const STypeExpr * type = reinterpret_cast < const STypeExpr * > ( param -> fd );
    REQUIRE_EQ ( (uint32_t)0, type -> fd . td. dim );
}

FIXTURE_TEST_CASE(Func_ControlParam, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8 f( control U8 p);", "f" );
    const SProduction * param = fn . GetParam ( 0 );
    REQUIRE_NOT_NULL ( param );
    REQUIRE ( param -> control );
}

FIXTURE_TEST_CASE(Func_TwoParamsSameName, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U8 f(U8 p, U16 p);", "Name already in use: 'p'" );
}

FIXTURE_TEST_CASE(Func_TwoParams, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8 f(U8 p1, U16 p2);", "f" );
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

FIXTURE_TEST_CASE(Func_OptionalParams, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8 f( * U8 p1, U16 p2);", "f" );
    REQUIRE_EQ ( (uint16_t)0, fn . MandatoryParamCount () );
    REQUIRE ( ! fn .  IsVariadic () );
    REQUIRE_EQ ( (uint32_t)2, fn . ParamCount () );
}
FIXTURE_TEST_CASE(Func_Varargs, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8 f(U8 p, ...);", "f" );
    REQUIRE_EQ ( (uint16_t)1, fn . MandatoryParamCount () );
    REQUIRE ( fn .  IsVariadic () );
    REQUIRE_EQ ( (uint32_t)1, fn . ParamCount () );
}
FIXTURE_TEST_CASE(Func_OprionalAndVarargs, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction ( "function U8 f( * U8 p, ...);", "f" );
    REQUIRE_EQ ( (uint16_t)0, fn . MandatoryParamCount () );
    REQUIRE ( fn .  IsVariadic () );
    REQUIRE_EQ ( (uint32_t)1, fn . ParamCount () );
}
FIXTURE_TEST_CASE(Func_MandatoryOprionalAndVarargs, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 f(U8 p1, * U16 p2, ...);", "f" );
    REQUIRE_EQ ( (uint16_t)1, fn . MandatoryParamCount () );
    REQUIRE ( fn .  IsVariadic () );
    REQUIRE_EQ ( (uint32_t)2, fn . ParamCount () );
}
FIXTURE_TEST_CASE(Func_FactoryVarargsWithBody, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function U8 f<U8 p, ...>() { return 1; }",
                         "Function with factory varargs cannot have a body: 'f'",
                         1, 30 );
}

FIXTURE_TEST_CASE(Func_Body_NonScript, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "function U8 f#1(); function U8 f#2() { return 1; };", "Overload cannot have a body: 'f'" );
}
FIXTURE_TEST_CASE(Func_Overload_NonScript, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "function U8 f#1() { return 1; } function U8 f#2();", "Overload has to have a body: 'f'", 1, 45 );
}

// schema signature

FIXTURE_TEST_CASE(Func_SchemaParam_Type, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function < type T > T f ( T p1 );", "f" );
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
    FunctionAccess fn = ParseFunction  ( "function < U32 T > U8 f ( U8 p );", "f" );
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
    VerifyErrorMessage ( "function < ascii X > U8 f ();", "Not a scalar unsigned integer: 'X'", 1, 18 );
}
FIXTURE_TEST_CASE(Func_SchemaParam_IntArray, AST_Function_Fixture)
{
    VerifyErrorMessage ( "function < U8[2] X > U8 f ();", "Not a scalar unsigned integer: 'X'" );
}
FIXTURE_TEST_CASE(Func_SchemaParam_IntPseudoArray, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function < U8[1] X > U8 f ();", "f" );
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
    FunctionAccess fn = ParseFunction  ( "function < U32 T > U8[T] f ( U8 p );", "f" );
    const STypeExpr * type = fn. ReturnType ();
    REQUIRE_EQ ( U8_id, type -> fd . td . type_id );
    REQUIRE ( ! type -> resolved );
    REQUIRE_EQ ( (uint32_t)0, type -> fd . td. dim );
}

FIXTURE_TEST_CASE(Func_SchemaParam_UsedInParam, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function < U32 T > U8 f ( U8[T] p );", "f" );
    const SProduction * param = fn . GetParam ( 0 );
    const STypeExpr * type = reinterpret_cast < const STypeExpr * > ( param -> fd );
    REQUIRE_EQ ( U8_id, type -> fd . td . type_id );
    REQUIRE ( ! type -> resolved );
    REQUIRE_EQ ( (uint32_t)0, type -> fd . td. dim );
}

// factory signature

FIXTURE_TEST_CASE(Func_FactoryParam_Empty, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 f <> ( U8 p1 );", "f" );
    REQUIRE_EQ ( (uint16_t)0, fn . FactoryMandatoryParamCount () );
    REQUIRE ( ! fn . FactoryIsVariadic () );
    REQUIRE_EQ ( (uint32_t)0, fn . FactoryParamCount ());
}
FIXTURE_TEST_CASE(Func_FactoryParam_Mandatory, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 f <U16 fp1, U32 fp2 > ();", "f" );
    REQUIRE_EQ ( (uint16_t)2, fn . FactoryMandatoryParamCount () );
    REQUIRE ( ! fn . FactoryIsVariadic () );
    REQUIRE_EQ ( (uint32_t)2, fn . FactoryParamCount ());
    const SIndirectConst * p1 = fn . FactoryGetParam ( 0 );
    REQUIRE_EQ ( string ( "fp1" ), ToCppString ( p1 -> name -> name ) );
    REQUIRE_EQ ( U16_id, reinterpret_cast < const STypeExpr * > ( p1 -> td ) -> dt -> id );
    REQUIRE_EQ ( (uint32_t)1, p1 -> expr_id );
    REQUIRE_EQ ( (uint32_t)0, p1 -> pos );
}
FIXTURE_TEST_CASE(Func_FactoryParam_Optional, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 f < * U16 fp > ();", "f" );
    REQUIRE_EQ ( (uint16_t)0, fn . FactoryMandatoryParamCount () );
    REQUIRE_EQ ( (uint32_t)1, fn . FactoryParamCount ());
}
FIXTURE_TEST_CASE(Func_FactoryParam_Vararg, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 f <U16 fp, ... > ();", "f" );
    REQUIRE ( fn . FactoryIsVariadic () );
}
FIXTURE_TEST_CASE(Func_FactoryParam_Used, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 f <U16 fp> ( U8[fp] p1 );", "f" );
    const SProduction * param = fn . GetParam ( 0 );
    const STypeExpr * type = reinterpret_cast < const STypeExpr * > ( param -> fd );
    REQUIRE_EQ ( (uint32_t)eIndirectExpr, type -> dim -> var );
    REQUIRE_EQ ( U8_id, type -> fd . td . type_id );
    REQUIRE ( ! type -> resolved );
    REQUIRE_EQ ( (uint32_t)0, type -> fd . td. dim );
}

// Factory spec in prologue

FIXTURE_TEST_CASE(Func_FactorySpec_Undefined, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 fn() = fact;", "fn" );
    const KSymbol * factId = fn . FactoryId ();
    REQUIRE_NOT_NULL ( factId );
    REQUIRE_EQ ( string ( "fact" ), ToCppString ( factId -> name ) );
    REQUIRE_EQ ( (uint32_t)eFactory, factId -> type );
}

FIXTURE_TEST_CASE(Func_FactorySpec_DefinedBefore, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 f1(); function U8 f2() = f1;", "f2", 1 );
    const KSymbol * factId = fn . FactoryId ();
    REQUIRE_NOT_NULL ( factId );
    REQUIRE_EQ ( string ( "f1" ), ToCppString ( factId -> name ) );
    REQUIRE_EQ ( (uint32_t)eFunction, factId -> type );
}

FIXTURE_TEST_CASE(Func_FactorySpec_Reused, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 f1() = fact; function U8 f2() = fact;", "f2", 1 );
    const KSymbol * factId = fn . FactoryId ();
    REQUIRE_EQ ( string ( "fact" ), ToCppString ( factId -> name ) );
    REQUIRE_EQ ( (uint32_t)eFactory, factId -> type );
}

FIXTURE_TEST_CASE(Func_FactorySpec_DefinedAfter, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "function U8 f1() = fact; function U8 fact();", "Declared earlier and cannot be overloaded: 'fact'" );
}

// Extern functions
FIXTURE_TEST_CASE(Func_Extern, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "extern function U8 f();", "f" );
    REQUIRE ( ! fn . IsScript () );
    REQUIRE ( ! fn . IsValidate () );
    REQUIRE ( ! fn . IsUntyped () );
    REQUIRE ( ! fn . IsRowLength () );
}

// Schema (script) functions

FIXTURE_TEST_CASE(Func_Script, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "schema function U8 f() { return 1; };", "f", 0 , eScriptFunc );
    REQUIRE ( fn . IsScript () );
    REQUIRE ( ! fn . IsValidate () );
    REQUIRE ( ! fn . IsUntyped () );
    REQUIRE ( ! fn . IsRowLength () );

    const SExpression * ret = fn . ReturnExpr ();
    REQUIRE_NOT_NULL ( ret );
    REQUIRE_EQ ( (uint32_t)eConstExpr, ret -> var );
    REQUIRE_EQ ( (uint64_t)1, reinterpret_cast < const SConstExpr * > ( ret ) -> u . u64 [ 0 ] );
}

FIXTURE_TEST_CASE(Func_ScriptWithouSchema, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function U8 f() { return 1; };", "f", 0 , eScriptFunc );
    REQUIRE ( fn . IsScript () );
}

FIXTURE_TEST_CASE(Func_ExternRedeclaredAsScript, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "function U8 f(); schema function U8 f() { return 1; };", "Overload cannot have a body: 'f'", 1, 37 );
}

FIXTURE_TEST_CASE(Func_Script_FullVersion, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "schema function U8 f#1.2.3() { return 1; };", "f", 0 , eScriptFunc );
    REQUIRE_EQ ( (uint32_t)(1 << 24 | 2 << 16 | 3 ), fn . Version () );
}

FIXTURE_TEST_CASE(Func_Script_MultipleReturns, AST_Function_Fixture)
{
    VerifyErrorMessage ( "schema function U8 f() { return 1; return 2; };", "Multiple return statements in a function: 'f'", 1, 36 );
}

FIXTURE_TEST_CASE(Func_Script_SimpleProduction, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "schema function U8 f() { U8 v = 1; return v; };", "f", 0 , eScriptFunc );
    REQUIRE_EQ ( (uint32_t)1, fn. ProductionCount () );

    const SProduction * prod = fn . Production ( 0 );
    REQUIRE_NOT_NULL ( prod );
    // verify type
    REQUIRE_NOT_NULL ( prod -> fd );
    REQUIRE_EQ ( (uint32_t)eTypeExpr, prod -> fd -> var );
    REQUIRE_EQ ( U8_id, ( ( const STypeExpr * ) ( prod -> fd ) ) -> dt -> id );
    // verify name
    REQUIRE_EQ ( string ( "v" ), string ( prod -> name -> name . addr ) );
    // verify right hand side
    REQUIRE_NOT_NULL ( prod -> expr );
    REQUIRE_EQ ( (uint32_t)eConstExpr, prod -> expr -> var );

    // verify reference to "v" in the return expression
    REQUIRE_EQ ( (uint32_t)eProdExpr, fn . ReturnExpr () -> var );
}

FIXTURE_TEST_CASE(Func_Script_NoReturn, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "schema function U8 f() { U8 v = 1; };",
                          "Schema function does not contain a return statement: 'f'",
                          1, 26 );
}

FIXTURE_TEST_CASE(Func_Script_FormattedType, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "fmtdef fmt; schema function U8 f() { fmt / U8 v = 1; return v; };", "f", 0 , eScriptFunc );
    REQUIRE_EQ ( (uint32_t)1, fn. ProductionCount () );

    const SProduction * prod = fn . Production ( 0 );
    REQUIRE_NOT_NULL ( prod );
    // verify type
    REQUIRE_NOT_NULL ( prod -> fd );
    REQUIRE_NOT_NULL ( ( ( const STypeExpr * ) ( prod -> fd ) ) -> fmt );
}

FIXTURE_TEST_CASE(Func_Script_FormattedType_NotFormat, AST_Function_Fixture)
{
    VerifyErrorMessage ( "typedef U16 t; schema function U8 f() { t / U8 v = 1; return v; };", "Not a format: 't'" );
}

// Expressions
// these test cases use script bodies for context

FIXTURE_TEST_CASE(Negate_IndirectSymExpr, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function I8 f <I8 i> () { return -i; }", "f", 0 , eScriptFunc );
    REQUIRE_EQ ( ( uint32_t ) eNegateExpr, fn . ReturnExpr () -> var );
    const SUnaryExpr * e = reinterpret_cast < const SUnaryExpr * > ( fn . ReturnExpr () );
    REQUIRE_NOT_NULL ( e -> expr );
    REQUIRE_EQ ( ( uint32_t ) eIndirectExpr, e -> expr -> var );
}
FIXTURE_TEST_CASE(Negate_Unsigned, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "function U8 f <U8 i> () { return -i; }", "Negation applied to an unsigned integer", 1, 35 );
}
FIXTURE_TEST_CASE(Negate_Double, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "table t#1 { column U8 a; column U8 b = - - a; } ", "Negation applied to a non-const operand", 1, 44 );
}

FIXTURE_TEST_CASE(UnaryPlus, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function I8 f <I8 i> () { return + i; }", "f", 0 , eScriptFunc );
    REQUIRE_EQ ( ( uint32_t ) eIndirectExpr, fn . ReturnExpr () -> var );
}

FIXTURE_TEST_CASE(Cast, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "function I8 f <U8 i> () { return ( I8 ) i; }", "f", 0 , eScriptFunc );
    REQUIRE_EQ ( ( uint32_t ) eCastExpr, fn . ReturnExpr () -> var );
    const SBinExpr * e = reinterpret_cast < const SBinExpr * > ( fn . ReturnExpr () );
    REQUIRE_NOT_NULL ( e -> left );
    REQUIRE_EQ ( ( uint32_t ) eTypeExpr, e -> left -> var);
    {
        const STypeExpr * te =  reinterpret_cast < const STypeExpr * > ( e -> left );
        REQUIRE_NOT_NULL ( te );
        REQUIRE_NOT_NULL ( te -> dt );
        REQUIRE_EQ ( string ( "I8" ), ToCppString ( te -> dt -> name -> name ) );
    }

    REQUIRE_NOT_NULL ( e -> right );
    REQUIRE_EQ ( ( uint32_t ) eIndirectExpr, e -> right -> var );
    {
        const SSymExpr * se =  reinterpret_cast < const SSymExpr * > ( e -> right );
        REQUIRE_NOT_NULL ( se );
        REQUIRE_EQ ( string ( "i" ), ToCppString ( se -> _sym -> name ) );
    }
}

// Validate functions

FIXTURE_TEST_CASE(Func_Validate, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "validate function void f(U8 a, U8 b);", "f", 0 , eFunction );
    REQUIRE ( ! fn . IsScript () );
    REQUIRE ( fn . IsValidate () );
    REQUIRE ( ! fn . IsUntyped () );
    REQUIRE ( ! fn . IsRowLength () );
}

FIXTURE_TEST_CASE(Func_NonValidate_void, AST_Function_Fixture)
{
   VerifyErrorMessage  ( "function void f(U8 a, U8 b);", "Only validate functions can return void: 'f'" );
}

FIXTURE_TEST_CASE(Func_Validate_NonVoid, AST_Function_Fixture)
{
   VerifyErrorMessage  ( "validate function U8 f(U8 a, U8 b);", "Validate functions have to return void: 'f'" );
}

FIXTURE_TEST_CASE(Func_Validate_OptionalParameters, AST_Function_Fixture)
{
   VerifyErrorMessage  ( "validate function void f(U8 a, U8 b, *U8 c);", "Validate functions cannot have optional parameters: 'f'" );
}

FIXTURE_TEST_CASE(Func_Validate_Not2Parameters, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "validate function void f(U8 a);", "Validate functions have to have 2 mandatory parameters: 'f'" );
}

FIXTURE_TEST_CASE(Func_Validate_Varargs, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "validate function void f(U8 a, U8 b, ...);", "Validate functions cannot have variable parameters: 'f'" );
}

FIXTURE_TEST_CASE(Func_Validate_FactorySpec, AST_Function_Fixture)
{
    FunctionAccess fn = ParseFunction  ( "validate function void f(U8 a, U8 b) = fact;", "f", 0 , eFunction );
    const KSymbol * factId = fn . FactoryId ();
    REQUIRE_NOT_NULL ( factId );
    REQUIRE_EQ ( string ( "fact" ), ToCppString ( factId -> name ) );
    REQUIRE_EQ ( (uint32_t)eFactory, factId -> type );
}

// Physical

class PhysicalAccess
{
public:
    PhysicalAccess ( const SPhysical * p_fn )
    : m_self ( p_fn )
    {
    }

    const STypeExpr * ReturnType () const { return reinterpret_cast < const STypeExpr * > ( m_self -> td ); }
    uint32_t          Version () const { return m_self -> version; }

    FunctionAccess Encode () const { return FunctionAccess ( & m_self -> encode ); }
    FunctionAccess Decode () const { return FunctionAccess ( & m_self -> decode ); }
    FunctionAccess RowLength () const { return FunctionAccess ( m_self -> row_length ); } // can be NULL

    bool IsReadOnly () const { return m_self -> read_only; }

    const SPhysical * m_self;
};

PhysicalAccess
AST_Function_Fixture :: ParsePhysical ( const char * p_source, const char * p_name )
{
    const SPhysical* ret = 0;
    if ( m_newParse )
    {
        MakeAst ( p_source );
        const KSymbol* sym = VerifySymbol ( p_name, ePhysical );

        // for physical functions, sym points to an entry in the overloads table (schema->pname)
        const SNameOverload* name = static_cast < const SNameOverload* > ( sym -> u . obj );
        if ( 0 == VectorLength ( & name -> items ) )
        {
            throw std :: logic_error ( "AST_Function_Fixture::ParsePhysical : no overloads" );
        }
        ret = static_cast < const SPhysical* > ( VectorGet ( & name -> items, 0 ) );
        if ( string ( p_name ) != ToCppString ( ret -> name -> name ) )
        {
            throw std :: logic_error ( "AST_Function_Fixture::ParsePhysical : wrong name" );
        }
    }
    else if ( OldParse ( p_source ) )
    {
        ret = static_cast < const SPhysical* > ( VectorGet ( & m_schema -> phys, 0 ) );
        if ( string ( p_name ) != ToCppString ( ret -> name -> name ) )
        {
            throw std :: logic_error ( "AST_Function_Fixture::ParsePhysical : wrong name" );
        }
    }
    return PhysicalAccess ( ret );
}

FIXTURE_TEST_CASE(Func_Physical_Simple, AST_Function_Fixture)
{
    PhysicalAccess fn = ParsePhysical  ( "physical U8 f#1.2 = { return 1; }", "f" );

    REQUIRE_NOT_NULL ( fn . ReturnType () );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id );
    REQUIRE_EQ ( 1u,    fn . ReturnType () -> fd . td . dim );

    REQUIRE_EQ ( (uint32_t) ( 1 << 24 ) | ( 2 << 16 ), fn . Version () );

    REQUIRE_NULL ( fn . Encode () . ReturnExpr () );
    REQUIRE_NOT_NULL ( fn . Decode () . ReturnExpr () );

    REQUIRE ( fn . IsReadOnly () );
}

FIXTURE_TEST_CASE(Func_Physical_Redeclared, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "function U8 f(); physical U16 f#1.2 = { return 2; }", "Declared earlier and cannot be overloaded: 'f'" );
}

FIXTURE_TEST_CASE(Func_Physical_OverloadSameVersion, AST_Function_Fixture)
{
    PhysicalAccess fn = ParsePhysical  ( "physical U8 f#1.2 = { return 1; } physical U16 f#1.2 = { return 2; }", "f" );
    REQUIRE_EQ ( 1u, VectorLength ( & m_schema -> phys ) );
    REQUIRE_NOT_NULL ( fn . ReturnType () );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id ); // 2nd version ignored
}
FIXTURE_TEST_CASE(Func_Physical_OverloadOlderVersion, AST_Function_Fixture)
{
    PhysicalAccess fn = ParsePhysical  ( "physical U8 f#1.2 = { return 1; } physical U16 f#1.1 = { return 2; }", "f" );
    REQUIRE_EQ ( 1u, VectorLength ( & m_schema -> phys ) );

    REQUIRE_NOT_NULL ( fn . ReturnType () );
    REQUIRE_EQ ( U8_id, fn . ReturnType () -> fd . td . type_id ); // 2nd version ignored (lower minor)
}

FIXTURE_TEST_CASE(Func_Physical_OverloadOlderMajorVersion, AST_Function_Fixture)
{
    ParsePhysical  ("physical U8  f#2 = { return 1; } physical U16 f#1 = { return 2; }\n", "f" );
    REQUIRE_EQ ( 2u, VectorLength ( & m_schema -> phys ) ); // both major versions present
}

FIXTURE_TEST_CASE(Func_Physical_OverloadNewerVersion, AST_Function_Fixture)
{
    if ( m_newParse ) // the old parser fails here!
    {
        PhysicalAccess fn = ParsePhysical  ( "physical U8 f#1.2 = { return 1; } physical U16 f#1.3 = { return 2; }", "f" );
        REQUIRE_NOT_NULL ( fn . ReturnType () );
        REQUIRE_EQ ( U16_id, fn . ReturnType () -> fd . td . type_id ); // 2nd version chosen
    }
}

FIXTURE_TEST_CASE(Func_Physical_Decode, AST_Function_Fixture)
{
    PhysicalAccess fn = ParsePhysical  ( "physical U8 f#1.2 { decode { return 1; } }", "f" );

    REQUIRE_NULL ( fn . Encode () . ReturnExpr () );
    REQUIRE_NOT_NULL ( fn . Decode () . ReturnExpr () );
    REQUIRE_NULL ( fn . RowLength () . m_self );
    REQUIRE ( fn . IsReadOnly () );
}

FIXTURE_TEST_CASE(Func_Physical_EncodeDecode, AST_Function_Fixture)
{
    PhysicalAccess fn = ParsePhysical  ( "physical U8 f#1 { decode { return 1; } encode { return 1; } }", "f" );

    REQUIRE_NOT_NULL ( fn . Encode () . ReturnExpr () );
    REQUIRE_NOT_NULL ( fn . Decode () . ReturnExpr () );
    REQUIRE_NULL ( fn . RowLength () . m_self );
    REQUIRE ( ! fn . IsReadOnly () );
}

FIXTURE_TEST_CASE(Func_Physical_DecodeRowLen, AST_Function_Fixture)
{
    PhysicalAccess fn = ParsePhysical  ( "function __row_length rl(); physical __no_header U8 f#1 { decode { return 1; } __row_length = rl(); }", "f" );

    REQUIRE_NULL ( fn . Encode () . ReturnExpr () );
    REQUIRE_NOT_NULL ( fn . Decode () . ReturnExpr () );
    REQUIRE_NOT_NULL ( fn . RowLength () . m_self );
    REQUIRE ( fn . IsReadOnly () );
}

FIXTURE_TEST_CASE(Func_Physical_NotARowlen, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "function U8 rl(); physical __no_header U8 f#1 { decode { return 1; } __row_length = rl() }",
                          "Not a row_length function: 'rl'",
                          1, 85 );
}

FIXTURE_TEST_CASE(Func_Physical_EncodeNoDecode, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "function __row_length rl(); physical U8 f#1 { encode { return 1; } __row_length = rl(); }", "Missing decode(): 'f'" );
}
FIXTURE_TEST_CASE(Func_Physical_RowlenNoDecode, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "function __row_length rl(); physical __no_header U8 f#1 { __row_length = rl(); }", "Missing decode(): 'f'" );
}
FIXTURE_TEST_CASE(Func_Physical_EncodeRepeated, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "physical U8 f#1 { encode { return 1; } decode { return 1; } encode { return 1; } }", "Multiply defined encode(): 'f'" );
}
FIXTURE_TEST_CASE(Func_Physical_DecodeRepeated, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "physical U8 f#1 { decode { return 1; } decode { return 1; } encode { return 1; } }", "Multiply defined decode(): 'f'" );
}
FIXTURE_TEST_CASE(Func_Physical_RowlenRepeated, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "function __row_length rl(); physical __no_header U8 f#1 { decode { return 1; } __row_length = rl();  __row_length = rl(); }", "Multiply defined __row_length(): 'f'" );
}
FIXTURE_TEST_CASE(Func_Physical_NoHeaderWithEncode, AST_Function_Fixture)
{
    VerifyErrorMessage  ( "physical __no_header U8 f#1 { decode { return 1; } encode { return 1; } }", "__no_header cannot define enable(): 'f'" );
}

FIXTURE_TEST_CASE(Func_Physical_At, AST_Function_Fixture)
{
    PhysicalAccess fn = ParsePhysical  ( "physical U8 f#1.2 { decode { return @; } }", "f" );
    const SExpression * expr = fn . Decode () . ReturnExpr ();
    REQUIRE_NOT_NULL ( expr );
    REQUIRE_EQ ( (uint32_t) eParamExpr, expr -> var );
    const SSymExpr * sym = reinterpret_cast < const SSymExpr * > ( expr );
    REQUIRE_EQ ( string ( "@" ), ToCppString ( sym -> _sym -> name ) );
}

FIXTURE_TEST_CASE(Func_Physical_SchemaParams, AST_Function_Fixture)
{
    PhysicalAccess fn = ParsePhysical  ( "physical <type T> T f#1.2 { decode { return 1; } }", "f" );
    REQUIRE_EQ ( 1u, fn . Decode () . SchemaTypeParamCount () );
}