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
* Unit tests for view declarations in schema, this file is #included into a bigger test suite
*/

FIXTURE_TEST_CASE(Version_2_Empty_Source, AST_Fixture)
{
    AST * ast = MakeAst ( "version 2; table t#1 {};" );
    REQUIRE_NOT_NULL ( ast );
    REQUIRE_EQ ( ( int ) PT_SCHEMA_2_0, ast -> GetTokenType () );
    REQUIRE_EQ ( 2u, ast -> ChildrenCount () );
    REQUIRE_EQ ( ( int ) PT_EMPTY,      ast -> GetChild ( 0 ) -> GetTokenType () ); // declarations in a list
    REQUIRE_EQ ( ( int ) PT_VERSION_2,  ast -> GetChild ( 1 ) -> GetTokenType () );
}

class ViewAccess // encapsulates access to an STable in a VSchema
{
public:
    ViewAccess ( const SView* p_fn ) : m_self ( p_fn ) {}

    uint32_t Version () const { return m_self -> version; }
    uint32_t Id () const { return m_self -> id; }

    const SNameOverload * GetOverloads () const
    {
        return static_cast < const SNameOverload * > ( m_self -> name -> u . obj );
    }

    VdbVector < SView > Overloads () const { return VdbVector < SView > ( GetOverloads () -> items ); }

    VdbVector < const KSymbol > Parameters () const { return VdbVector < const KSymbol > ( m_self -> params ); }

    VdbVector < SViewInstance > Parents () const { return VdbVector < SViewInstance > ( m_self -> parents ); }

    /* overrides ( inherited virtual productions )
       contents are grouped by introducing parent */
    VdbVector < SViewOverrides > Overrides () const { return VdbVector < SViewOverrides > ( m_self -> overrides ); }

    VdbVector < SColumn > Columns () const { return VdbVector < SColumn > ( m_self -> col ); }
    VdbVector < SNameOverload > ColumnNames () const { return VdbVector < SNameOverload > ( m_self -> cname ); }
    VdbVector < SProduction > Productions () const { return VdbVector < SProduction > ( m_self -> prod ); }
    VdbVector < KSymbol > VirtualProductions () const { return VdbVector < KSymbol > ( m_self -> vprods ); }

    const SView * m_self;
};

class AST_View_Fixture : public AST_Fixture
{
public:
    AST_View_Fixture ()
    {
    }
    ~AST_View_Fixture ()
    {
    }

    ViewAccess ParseView ( const char * p_source, const char * p_name, uint32_t p_idx = 0 )
    {
        MakeAst ( p_source );
        const SView * ret = static_cast < const SView* > ( VectorGet ( & GetSchema () -> view, p_idx ) );
        if ( ret == 0 || ret -> name == 0 || string ( p_name ) != ToCppString ( ret -> name -> name ) )
        {
            throw std :: logic_error ( "AST_View_Fixture::ParseTable : wrong name" );
        }
        return ViewAccess ( ret );
    }
    ViewAccess GetView ( uint32_t p_idx )
    {
        return ViewAccess ( static_cast < const SView* > ( VectorGet ( & GetSchema () -> view, p_idx ) ) );
    }

    #define THROW_ON_TRUE(cond) if ( cond ) throw logic_error ( string ( __func__ ) + " : " #cond );

        void VerifySymExpr( const SExpression * p_expr,
                            uint32_t            p_exprType,
                            const string &      p_ident,
                            uint32_t            p_objType )
        {
            THROW_ON_TRUE ( p_expr == 0 );
            THROW_ON_TRUE ( p_exprType != p_expr -> var );
            const SSymExpr * sym = reinterpret_cast < const SSymExpr * > ( p_expr );
            THROW_ON_TRUE ( sym == 0 );
            THROW_ON_TRUE ( sym -> _sym == 0 );
            THROW_ON_TRUE ( p_ident != ToCppString( sym -> _sym -> name ) );
            THROW_ON_TRUE ( p_objType != sym -> _sym -> type );
        }
    #undef THROW_ON_TRUE

};

FIXTURE_TEST_CASE(View_OneTable_NoParents, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1 <T t> {}", "v" );

    REQUIRE_NULL ( v . m_self -> name -> dad );
    REQUIRE_EQ ( ( uint32_t ) eView, v . m_self -> name -> type );
    // verify entry in the container of (overloaded on version) view names
    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> vname ) );
    REQUIRE_EQ ( 1u, v . Overloads () . Count () );
    REQUIRE_EQ ( v . GetOverloads (), ( const SNameOverload * ) VectorGet ( & GetSchema () -> vname, 0 ) );
}

FIXTURE_TEST_CASE(View_Id, AST_View_Fixture)
{   // both version are registered
    ViewAccess v1 = ParseView ( "version 2; table T#1 {}; view v#1 <T t> {}; view v#2 <T t> {}", "v" );
    ViewAccess v2 = GetView ( 1 );

    REQUIRE_EQ ( 2u, VectorLength ( & GetSchema () -> view ) );
    REQUIRE_EQ ( 0u, v1 . Id () );
    REQUIRE_EQ ( 1u, v2 . Id () );
}

// Version

FIXTURE_TEST_CASE(View_Version, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1.2.3 <T t> {}", "v" );
    REQUIRE_EQ ( Version ( 1, 2, 3 ), v . Version () );
}

FIXTURE_TEST_CASE(View_Version_Redeclared, AST_Fixture)
{
    VerifyErrorMessage ( "version 2; table T#1 {}; view v#1.2.3 <T t> {}; view v#1.2.3 <T t> {}",
                              "This version is already declared: 'v'" );
}

FIXTURE_TEST_CASE(View_NewerMajorVersion, AST_View_Fixture)
{   // both version are registered
    ViewAccess v1 = ParseView ( "version 2; table T#1 {}; view v#1 <T t> {}; view v#2 <T t> {}", "v" );
    ViewAccess v2 = GetView ( 1 );

    REQUIRE_EQ ( 2u, VectorLength ( & GetSchema () -> view ) );
    REQUIRE_EQ ( Version ( 1 ), v1 . Version () );
    REQUIRE_EQ ( Version ( 2 ), v2 . Version () );

    // 1 entry in the table of view overloads, the entry points to the 2 versions of the view, the views point back
    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> vname ) );
    REQUIRE_EQ ( 2u, v1 . Overloads () . Count () );
    REQUIRE_EQ ( v1 . GetOverloads (), v2 . GetOverloads () );
    REQUIRE_EQ ( v1 . GetOverloads (), ( const SNameOverload * ) VectorGet ( & GetSchema () -> vname, 0 ) );
}

FIXTURE_TEST_CASE(View_OlderMajorVersion, AST_View_Fixture)
{   // both version are registered
    ViewAccess v1 = ParseView ( "version 2; table T#1 {}; view v#2 <T t> {}; view v#1 <T t> {}", "v" );
    ViewAccess v2 = GetView ( 1 );

    REQUIRE_EQ ( 2u, VectorLength ( & GetSchema () -> view ) );
    REQUIRE_EQ ( Version ( 2 ), v1 . Version () );
    REQUIRE_EQ ( Version ( 1 ), v2 . Version () );

    // 1 entry in the table of view overloads, the entry points to the 2 versions of the view, the views point back
    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> vname ) );
    REQUIRE_EQ ( 2u, v1 . Overloads () . Count () );
}

FIXTURE_TEST_CASE(View_OlderMinorVersion, AST_View_Fixture)
{   // older minor version is ignored
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1.2 <T t> {}; view v#1.1 <T t> {}", "v" );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> view ) );
    REQUIRE_EQ ( Version ( 1, 2 ), v . Version () );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> vname ) );
    REQUIRE_EQ ( 1u, v . Overloads () . Count () );
}

FIXTURE_TEST_CASE(View_NewerMinorVersion, AST_View_Fixture)
{   // older minor version is replaced
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1.1 <T t> {}; view v#1.2 <T t> {}", "v" );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> view ) );
    REQUIRE_EQ ( Version ( 1, 2 ), v . Version () );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> vname ) );
    REQUIRE_EQ ( 1u, v . Overloads () . Count () );
}

// Parameters
FIXTURE_TEST_CASE(View_Parameter_Table, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1 <T t> {}", "v" );
    REQUIRE_EQ ( 1u, v . Parameters () . Count () );
    REQUIRE_EQ ( (uint32_t)eTable, v . Parameters () . Get ( 0 ) -> type );
    REQUIRE_EQ ( (const void *) GetTable ( 0 ), v . Parameters () . Get ( 0 ) -> u . obj );
}

FIXTURE_TEST_CASE(View_Parameter_View, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view X#1 <T t> {}; view v#1 <X v> {}", "v", 1 );
    REQUIRE_EQ ( 1u, v . Parameters () . Count () );
    REQUIRE_EQ ( (uint32_t)eView, v . Parameters () . Get ( 0 ) -> type );
    REQUIRE_EQ ( (const void *) GetView ( 0 ) . m_self, v . Parameters () . Get ( 0 ) -> u . obj );
}

FIXTURE_TEST_CASE(View_Parameter_TableAndView, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view X#1 <T t> {}; view v#1 <T t, X v> {}", "v", 1 );
    REQUIRE_EQ ( 2u, v . Parameters () . Count () );
    REQUIRE_EQ ( (uint32_t)eTable, v . Parameters () . Get ( 0 ) -> type );
    REQUIRE_EQ ( (const void *) GetTable ( 0 ), v . Parameters () . Get ( 0 ) -> u . obj );
    REQUIRE_EQ ( (uint32_t)eView, v . Parameters () . Get ( 1 ) -> type );
    REQUIRE_EQ ( (const void *) GetView ( 0 ) . m_self, v . Parameters () . Get ( 1 ) -> u . obj );
}

FIXTURE_TEST_CASE(View_Parameter_NotTableOrView, AST_View_Fixture)
{
    VerifyErrorMessage ( "version 2; view v#1 <U8 t> {}", "Cannot be used as a view parameter: 'U8'" );
}

FIXTURE_TEST_CASE(View_Parameter_UnknownParameter, AST_View_Fixture)
{
    VerifyErrorMessage ( "version 2; view v#1 <zz t> {}", "Undeclared identifier: 'zz'" );
}

FIXTURE_TEST_CASE(View_Parameter_VersionedTable, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; table T#2 {}; view v#1 <T#1 t> {}", "v" );
    REQUIRE_EQ ( (const void*)GetTable ( 0 ), v . Parameters () . Get ( 0 ) -> u . obj );
}

FIXTURE_TEST_CASE(View_Parameter_VersionedView, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view a:V#1 <T t> {}; "
        "view a:V#2 <T t> {}; "
        "view v#1 <a:V#2 v> {}", "v", 2 );
    REQUIRE_EQ ( (const void*)GetView ( 1 ) . m_self, v . Parameters () . Get ( 0 ) -> u . obj );
}

FIXTURE_TEST_CASE(View_Parameter_BadTableVersion, AST_View_Fixture)
{
    VerifyErrorMessage ( "version 2; table T#1 {}; view v#1 <T#2 t> {}", "Requested version does not exist: 'T#2'" );
}
FIXTURE_TEST_CASE(View_Parameter_BadViewVersion, AST_View_Fixture)
{
    VerifyErrorMessage ( "version 2; table T#1 {}; view X#1 <T t> {}; view v#1 <X#1.1 v> {}",
                         "Requested version does not exist: 'X#1.1'" );
}

FIXTURE_TEST_CASE(View_Parameter_DuplicateName, AST_View_Fixture)
{
    VerifyErrorMessage ( "version 2; table T#1 {}; view X#1 <T t> {}; view v#1 <X t, T t> {}",
                         "Name already in use: 't'" );
}

// View parents
FIXTURE_TEST_CASE(View_Parents_NoParents, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view W#1 <T v> {}", "W" );
    REQUIRE_EQ ( 0u, v . Parents () . Count () );
}

FIXTURE_TEST_CASE(View_Parents_Multiple, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view V1#1 <T t> {}; "
        "view V2#1 <T t, V1 v> {}; "
        "view W#1 <T t, V1 v> = V1<t>, V2<t, v> {}",
        "W", 2 );
    REQUIRE_EQ ( 2u, v . Parents () . Count () );
    REQUIRE_EQ ( GetView ( 0 ) . m_self, v . Parents () . Get ( 0 ) -> dad );
    REQUIRE_EQ ( GetView ( 1 ) . m_self, v . Parents () . Get ( 1 ) -> dad );
    // verify W's parents and their argiments
    VdbVector < const KSymbol > v1 ( v . Parents () . Get ( 0 ) -> params );
    REQUIRE_EQ ( 1u, v1 . Count () );
    REQUIRE_EQ ( (uint32_t)eTable, v1 . Get ( 0 ) -> type );
    REQUIRE_EQ ( string("t"), ToCppString ( v1 . Get ( 0 ) -> name ) );
    VdbVector < const KSymbol > v2 ( v . Parents () . Get ( 1 ) -> params );
    REQUIRE_EQ ( 2u, v2 . Count () );
    REQUIRE_EQ ( (uint32_t)eTable, v2 . Get ( 0 ) -> type );
    REQUIRE_EQ ( string("t"), ToCppString ( v2 . Get ( 0 ) -> name ) );
    REQUIRE_EQ ( (uint32_t)eView,  v2 . Get ( 1 ) -> type );
    REQUIRE_EQ ( string("v"), ToCppString ( v2 . Get ( 1 ) -> name ) );
}

FIXTURE_TEST_CASE(View_Parents_Unknown, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> = Z<t> {}; ",
        "Undeclared identifier: 'Z'" );
}

FIXTURE_TEST_CASE(View_Parents_Duplicate, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> {}; "
        "view W#1 <T v> = V<v>, V<v> {}",
        "Same view inherited from more than once: 'V'" );
}

FIXTURE_TEST_CASE(View_Parents_NotAView, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> {}; "
        "view W#1 <T v> = V<v>, T<v> {}",
        "A view's parent has to be a view: 'T'" );
}

FIXTURE_TEST_CASE(View_Parents_BadParameter, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> {}; "
        "const U8 c = 1;"
        "view W#1 <T v> = V<c> {}",
        "A view's parameter has to be a table or a view: 'c'" );
}

FIXTURE_TEST_CASE(View_Parents_Diamond, AST_View_Fixture)
{   // a parent in an ancestry more than once
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V1#1 <T t> {}"
        "view V2#1 <T t> = V1<t> {}"
        "view V3#1 <T t> = V2<t>, V1<t> {}",
        "Same view inherited from more than once: 'V1'");
}

FIXTURE_TEST_CASE(View_Parents_BadVersion, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> {}; "
        "view W#1 <T v> = V#2<v> {}",
        "Requested version does not exist: 'V#2'" );
}

// View body: columns

FIXTURE_TEST_CASE(View_Column, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view W#1 <T v> { column U8 c = 1; }", "W" );
    REQUIRE_EQ ( 1u, v . Columns () . Count () );
    const SColumn & c = * v . Columns () . Get ( 0 );
    REQUIRE_NOT_NULL ( c . name );
    REQUIRE_EQ ( ( uint32_t ) eColumn, c . name -> type );

    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, c . read -> var );

    REQUIRE_EQ ( string ("c"), ToCppString ( c . name -> name ) );
    REQUIRE_NULL ( c . validate );
    REQUIRE_NULL ( c . limit );
    REQUIRE_NULL ( c . ptype );
    REQUIRE_EQ ( U8_id, c . td . type_id );
    REQUIRE_EQ ( 1u, c . td . dim);
    REQUIRE_EQ ( 1u, c . cid . ctx ); // T;s contextId is 0, W's is 1
    REQUIRE_EQ ( 0u, c . cid . id );
    REQUIRE ( ! c . dflt );
    REQUIRE ( c . read_only );
    REQUIRE ( ! c . simple );

    REQUIRE_EQ ( ( const void * ) v . ColumnNames () . Get ( 0 ), c . name -> u . obj );
}

FIXTURE_TEST_CASE(View_Column_ColumnExists, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {};"
        "view W#1 <T v> { column U8 c = 1; column U8 c = 2; }",
        "Column already defined: 'c'" );
}

FIXTURE_TEST_CASE(View_Column_Overloaded, AST_View_Fixture)
{   // with a different type, not an error but an overload
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view W#1 <T v> { column U8 c = 1; column U16 c = 2; }",
        "W" );
    REQUIRE_EQ ( 1u, v . ColumnNames () . Count () );
    const SNameOverload * ovl = v . ColumnNames () . Get ( 0 );
    REQUIRE_NOT_NULL ( ovl );
    REQUIRE_EQ ( string ("c"), ToCppString ( ovl -> name -> name ) );
    REQUIRE_EQ ( 1u, ovl -> cid . ctx );
    REQUIRE_EQ ( 0u, ovl -> cid . id );
    const SColumn * col = v . Columns () . Get ( 0 );
    REQUIRE_NOT_NULL ( col );
    REQUIRE_EQ ( ovl -> cid . ctx, col -> cid . ctx );
    REQUIRE_EQ ( ovl -> cid . id, col -> cid . id );

    VdbVector < SColumn > names ( ovl -> items );
    REQUIRE_EQ ( 2u, names . Count () );
    REQUIRE_EQ ( v . Columns () . Get ( 0 ), names . Get ( 0 ) );
    REQUIRE_EQ ( v . Columns () . Get ( 1 ), names . Get ( 1 ) );

    col = v . Columns () . Get ( 1 );
    REQUIRE_NOT_NULL ( col );
    REQUIRE_EQ ( ovl -> cid . ctx, col -> cid . ctx );
    REQUIRE_EQ ( ovl -> cid . id, col -> cid . id );
}

FIXTURE_TEST_CASE(View_Column_Reference, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view W#1 <T v> { column U8 c1 = 1; column U8 c2 = c1; }", "W" );
    REQUIRE_EQ ( 2u, v . Columns () . Count () );
    const SColumn & c = * v . Columns () . Get ( 1 );
    REQUIRE_NOT_NULL ( c . name );
    REQUIRE_EQ ( string ("c2"), ToCppString ( c . name -> name ) );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_EQ ( ( uint32_t ) eColExpr, c . read -> var );
}

FIXTURE_TEST_CASE(View_Column_ReferenceToParamTablesColumn, AST_View_Fixture)
{   // introducing member expressions!
    ViewAccess v = ParseView ( "version 2; table T#1 { column U8 c1 = 1; }; view W#1 <T t0, T t1> { column U8 c2 = t1 . c1; }", "W" );
    REQUIRE_EQ ( 1u, v . Columns () . Count () );
    const SColumn & c = * v . Columns () . Get ( 0 );
    REQUIRE_NOT_NULL ( c . name );
    REQUIRE_EQ ( string ("c2"), ToCppString ( c . name -> name ) );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_EQ ( ( uint32_t ) eMembExpr, c . read -> var );
    const SMembExpr * expr = reinterpret_cast < const SMembExpr * > ( c . read );
    REQUIRE_EQ ( v .m_self, expr -> view );
    REQUIRE_EQ ( 1u, expr -> paramId );
    REQUIRE_EQ ( string("c1"), ToCppString ( expr -> member -> name ) );
}
FIXTURE_TEST_CASE(View_Column_ReferenceToParamTablesColumn_PseudoPhysicalToken, AST_View_Fixture)
{   // .b looks like a physical identifier
    ViewAccess v = ParseView ( "version 2; table T#1 { column U8 c1 = 1; }; view W#1 <T t> { column U8 c2 = t .c1; }", "W" );
    REQUIRE_EQ ( ( uint32_t ) eMembExpr, v . Columns () . Get ( 0 ) -> read -> var );
}
FIXTURE_TEST_CASE(View_Column_ReferenceToParamTablesProduction, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 { U8 c1 = 1; }; view W#1 <T t> { column U8 c2 = t . c1; }", "W" );
    REQUIRE_EQ ( ( uint32_t ) eMembExpr, v . Columns () . Get ( 0 ) -> read -> var );
}

FIXTURE_TEST_CASE(View_Join_Shorthand, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2;"
        "table T1#1 { column I64 c1; };"
        "table T2#1 { column U8 c2; };"
        "view X#1 <T1 t1, T2 t2> { column U8 c3 = t2 [ t1 . c1 ] . c2; }; ",
        "X" );

    REQUIRE_EQ ( 1u, v . Columns () . Count () );
    const SColumn & c = * v . Columns () . Get ( 0 );
    REQUIRE_NOT_NULL ( c . name );
    REQUIRE_EQ ( string ("c3"), ToCppString ( c . name -> name ) );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_EQ ( ( uint32_t ) eMembExpr, c . read -> var );
    const SMembExpr * expr = reinterpret_cast < const SMembExpr * > ( c . read );
    REQUIRE_EQ ( v . m_self, expr -> view );
    REQUIRE_EQ ( 1u, expr -> paramId ); // t2
    REQUIRE_EQ ( string("c2"), ToCppString ( expr -> member -> name ) );

    REQUIRE_NOT_NULL ( expr -> rowId );
    const SMembExpr * rowId = reinterpret_cast < const SMembExpr * > ( expr ->  rowId );
    REQUIRE_EQ ( v . m_self, rowId -> view );
    REQUIRE_EQ ( 0u, rowId -> paramId ); // t1
    REQUIRE_EQ ( string("c1"), ToCppString ( rowId -> member -> name ) );
    REQUIRE_NULL ( rowId -> rowId );
}

FIXTURE_TEST_CASE(View_Column_ReferenceToParamTablesColumn_Undefined, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 { column U8 c1 = 1; }; view W#1 <T t> { column U8 c2 = t . c11111; }",
        "Column/production not found: 'c11111'" );
}

FIXTURE_TEST_CASE(View_Column_ReferenceToParamViewsColumn, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 { U8 c1 = 1; };"
        " view V#1 <T t> { column U8 c2 = t . c1; }"
        " view W#1 <V v> { column U8 c3 = v . c2; }",
    "W", 1 );
    const SColumn & c = * v . Columns () . Get ( 0 );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_EQ ( ( uint32_t ) eMembExpr, c . read -> var );
}

FIXTURE_TEST_CASE(View_Column_ReferenceToParamViewsColumn_Undefined, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 { U8 c1 = 1; };"
        " view V#1 <T t> { column U8 c2 = t . c1; }"
        " view W#1 <V v> { column U8 c3 = v . c22222222; }",
        "Column/production not found: 'c22222222'" );
}

FIXTURE_TEST_CASE(View_Column_Context, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {};"
        " view V#1 <T t> {}"
        " view W#1 <T t> { column U8 c1 = 1; column U8 c2 = 2; }",
    "W", 1 );
    const SNameOverload * ovl = v . ColumnNames () . Get ( 0 );
    REQUIRE_NOT_NULL ( ovl );
    REQUIRE_EQ ( 2u, ovl -> cid . ctx ); // W'1 contextId is 2
    REQUIRE_EQ ( 0u, ovl -> cid . id );
    const SColumn * col = v . Columns () . Get ( 0 );
    REQUIRE_NOT_NULL ( col );
    REQUIRE_EQ ( ovl -> cid . ctx, col -> cid . ctx );
    REQUIRE_EQ ( ovl -> cid . id, col -> cid . id );

    ovl = v . ColumnNames () . Get ( 1 );
    REQUIRE_NOT_NULL ( ovl );
    REQUIRE_EQ ( 2u, ovl -> cid . ctx );
    REQUIRE_EQ ( 1u, ovl -> cid . id );
    col = v . Columns () . Get ( 1 );
    REQUIRE_NOT_NULL ( col );
    REQUIRE_EQ ( ovl -> cid . ctx, col -> cid . ctx );
    REQUIRE_EQ ( ovl -> cid . id, col -> cid . id );
}

FIXTURE_TEST_CASE(View_ForwardReference, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; view V#1 <T v> { column U16 a = p; column U8 p = 1; }", "V" );
    REQUIRE_EQ ( 2u, v . Columns () . Count () );
    const SColumn * a = v . Columns () . Get ( 0 );
    VerifySymExpr ( a -> read, eColExpr, "p", eColumn );
}

FIXTURE_TEST_CASE(View_ColumnDecl_OutsideIdent, AST_View_Fixture)
{
    ParseView ("version 2; table T#1 {}; const U8 c = 1; view V#1 <T v> { column U8 c = 1; }", "V" );
}

// View body: productions

FIXTURE_TEST_CASE(View_Production, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view V#1 <T v> { U8 p = 1; }", "V" );
    REQUIRE_EQ ( 1u, v . Productions () . Count () );
    const SProduction & p = * v . Productions () . Get ( 0 );
    REQUIRE_NOT_NULL ( p . name );
    REQUIRE_EQ ( string ("p"), ToCppString ( p . name -> name ) );
    REQUIRE_EQ ( ( uint32_t ) eProduction, p . name -> type );

    REQUIRE_NOT_NULL ( p . expr );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, p . expr -> var );

    REQUIRE_NOT_NULL ( p . fd );
    REQUIRE_EQ ( ( uint32_t ) eTypeExpr, p . fd -> var );
    const STypeExpr * type = reinterpret_cast < const STypeExpr * > ( p . fd );
    REQUIRE_EQ ( U8_id, type -> dt -> id );
    REQUIRE ( ! p . trigger );
    REQUIRE ( ! p . control );
}

FIXTURE_TEST_CASE(View_Production_ClashWithColumn, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; view V#1 <T v> { column U8 c = 1; U8 c = 1; }",
        "Production name is already in use: 'c'" );
}

FIXTURE_TEST_CASE(View_Production_IntroducingVirtual, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view V#1 <T v> { column U8 p = vp; }", "V" );
    REQUIRE_EQ ( 1u, v . VirtualProductions () . Count () );
    REQUIRE_EQ ( string ("vp"), ToCppString ( v . VirtualProductions () . Get ( 0 ) -> name ) );
}

FIXTURE_TEST_CASE(View_Production_ForwardReference_DefinedAsProduction, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view V#1 <T v> { U16 a = p; U8 p = 1; }", "V" );
    REQUIRE_EQ ( 2u, v . Productions () . Count () );
    const SProduction * a = v . Productions () . Get ( 0 );
    VerifySymExpr ( a -> expr, eProdExpr, "p", eProduction );
    // once defined, p is not virtual anymore
    REQUIRE_EQ ( 0u, v . VirtualProductions () . Count  () );
}

// Inheritance

FIXTURE_TEST_CASE(View_Parents_WrongNumberOfParams, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { column U8  v = 1; }; "
        "view W#1 <T t> { column U16 w = 1; }; "
        "view X#1 <T t> = V<t>, W<t, V> {}",
        "Wrong number of parameters for a view instantiation");
}

FIXTURE_TEST_CASE(View_Parents_WrongParamType, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { column U8  v = 1; }; "
        "view W#1 <T t> { column U16 w = 1; }; "
        "view X#1 <T t, W w> = V < w > {}",
        "Wrong type of a view's parameter: 'w'");
}

FIXTURE_TEST_CASE(View_Parents_ColumnsInherited, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { column U8  v = 1; }; "
        "view W#1 <T t> { column U16 w = 1; }; "
        "view X#1 <T t> = V<t>, W<t> {}",
        "X", 2 );
    REQUIRE_EQ ( 2u, v . ColumnNames () . Count () );
    // v and w are seen as X's columns
    REQUIRE_EQ ( string ("v"), ToCppString ( v . ColumnNames () . Get ( 0 ) -> name -> name ) );
    REQUIRE_EQ ( string ("w"), ToCppString ( v . ColumnNames () . Get ( 1 ) -> name -> name ) );
}

FIXTURE_TEST_CASE(View_Parents_ProductionsInherited, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { U8  v = 1; }; "
        "view X#1 <T t> = V<t> { U8 _v = v; }",
        "X", 1 );
    // v does not count as X's production but is accessible
    REQUIRE_EQ ( 1u, v . Productions () . Count () );
    REQUIRE_EQ ( string ("_v"), ToCppString ( v . Productions () . Get ( 0 ) -> name -> name ) );
}

FIXTURE_TEST_CASE(View_Parents_ColumnCollision, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { column U8 p = 1; }; "
        "view W#1 <T t> { column U8 p = 2; }; "
        "view X#1 <T t> = V<t>, W<t> {}",
        "Duplicate symbol in parent view hierarchy: 'p'" );
}

FIXTURE_TEST_CASE(View_Parents_ProductionCollision, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { U8 p = 1; }; "
        "view W#1 <T t> { U8 p = 2; }; "
        "view X#1 <T t> = V<t>, W<t> {}",
        "Duplicate symbol in parent view hierarchy: 'p'" );
}

FIXTURE_TEST_CASE(View_Parents_VirtualProductionDefined, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { U8 p = v; }; "
        "view X#1 <T t> = V<t> { U8 v = 1; }",
        "X", 1 );
    REQUIRE_EQ ( 1u, v . Productions () . Count () );
    REQUIRE_EQ ( string ("v"), ToCppString ( v . Productions () . Get ( 0 ) -> name -> name ) );

    // verify p and its reference to v
    const SProduction * p = GetView ( 0 ) . Productions () . Get ( 0 );
    REQUIRE_NOT_NULL ( p );
    VerifySymExpr ( p -> expr, eFwdExpr, "v", eVirtual );
}

FIXTURE_TEST_CASE(View_Parents_VirtualProductionDefinedAcrossParents, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { column U8 c = v; }; "
        "view W#1 <T t> { U8 v = 1; }; "
        "view X#1 <T t> = V<t>, W<t> {}",
        "X", 2 );
    REQUIRE_EQ ( 0u, v . Productions () . Count () );
    // verify c and its reference to W.v
    const SColumn * p = GetView ( 0 ) . Columns () . Get ( 0 );
    REQUIRE_NOT_NULL ( p );
    VerifySymExpr ( p -> read, eFwdExpr, "v", eVirtual );
}

FIXTURE_TEST_CASE(View_Parents_InheritedVirtualProduction, AST_View_Fixture)
{
    ViewAccess t = ParseView (
        "version 2; table T#1 {}; "
        "view granddad #1 <T t> { U8 a = v; }; "
        "view dad #1 <T t> = granddad<t> { U8 b = v; }; "
        "view t #1 <T t> = dad<t> { }",
        "t", 2 );

    // verify overrides
    // dad
    ViewAccess dad = GetView ( 1 );
    REQUIRE_EQ ( 1u, dad . Overrides () . Count () ); // inherits from granddad
    VdbVector < const KSymbol > dadOvr ( dad . Overrides () . Get ( 0 ) -> by_parent );
    REQUIRE_EQ ( 1u, dadOvr . Count () ); // 1 override copied from granddad

    // t
    REQUIRE_EQ ( 2u, t . Overrides () . Count () ); // v inherits from dad and granddad

    VdbVector < const KSymbol > tDadOvr ( t . Overrides () . Get ( 0 ) -> by_parent );
    REQUIRE_EQ ( 1u, tDadOvr . Count () ); // 1 override copied from granddad
    REQUIRE_EQ ( string ( "v" ), ToCppString ( tDadOvr . Get ( 0 ) -> name ) );

    VdbVector < const KSymbol > tGdadOvr ( t . Overrides () . Get ( 1 ) -> by_parent );
    REQUIRE_EQ ( 0u, tGdadOvr . Count () ); // no overrides copied from granddad


    // v is not resolved in t
    REQUIRE_EQ ( 0u, t . VirtualProductions () . Count  () );
}

FIXTURE_TEST_CASE(View_Parents_VirtualProductionDefinedAcrossParentsAsColumn, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { U8 a = v; }; "
        "view W#1 <T t> { column U8 v = 1; }; "
        "view X#1 <T t> = V<t>, W<t> {}",
        "a virtual production from one parent defined as non-production in another: 'v'" );
}

FIXTURE_TEST_CASE(View_Parents_RedeclaringParentsColumn, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view W#1 <T t>         { column U8 c = 1; }; "
        "view X#1 <T t> = W<t>  { column U8 c = 2; }",
        "Column already defined: 'c'" );
}

FIXTURE_TEST_CASE(View_Parents_OverloadingParentsColumn, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view W#1 <T t>         { column U8 c = 1; }; "
        "view X#1 <T t> = W<t>  { column U16 c = 2; }",
        "X", 1 );
    ViewAccess dad = GetView ( 0 );

    REQUIRE_EQ ( 1u, v . Columns () . Count () );
    REQUIRE_EQ ( 1u, v . ColumnNames () . Count () );
    const SNameOverload * ovl = v . ColumnNames () . Get ( 0 );
    REQUIRE_NOT_NULL ( ovl );
    REQUIRE_NOT_NULL ( ovl -> name );
    REQUIRE_EQ ( string ("c"), ToCppString ( ovl -> name -> name ) );
    REQUIRE_EQ ( 1u, ovl -> cid . ctx ); // inherited from W whose contexId is 1
    REQUIRE_EQ ( 0u, ovl -> cid . id );

    VdbVector < SColumn > names ( ovl -> items );
    REQUIRE_EQ ( 2u, names . Count () );
    REQUIRE_EQ ( dad . Columns () . Get ( 0 ), names . Get ( 0 ) );
    REQUIRE_EQ ( v . Columns () . Get ( 0 ), names . Get ( 1 ) );
}

