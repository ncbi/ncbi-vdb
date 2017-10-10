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
    ViewAccess ( const SView* p_fn )
    : m_self ( p_fn )
    {
    }

    uint32_t Version () const { return m_self -> version; }

    uint32_t OverloadCount () const
    {
        const SNameOverload * ovl = static_cast < const SNameOverload * > ( m_self -> name -> u . obj );
        return VectorLength ( & ovl -> items );
    }
    const SNameOverload * GetOverloads () const
    {
        return static_cast < const SNameOverload * > ( m_self -> name -> u . obj );
    }
    const SView * GetOverload ( uint32_t p_idx ) const
    {
        return static_cast < const SView * > ( VectorGet ( & GetOverloads () -> items, p_idx ) );
    }

    uint32_t ParamTableCount () const { return VectorLength ( & m_self -> tables ); }
    const STable * GetParamTable ( uint32_t p_idx ) const
    {
        return static_cast < const STable * > ( VectorGetSymbol ( m_self -> tables, p_idx ) -> u . obj );
    }
    uint32_t ParamViewCount () const { return VectorLength ( & m_self -> views ); }
    const SView * GetParamView ( uint32_t p_idx ) const
    {
        return static_cast < const SView * > ( VectorGetSymbol ( m_self -> views, p_idx ) -> u . obj);
    }

    uint32_t ParentCount () const { return VectorLength ( & m_self -> parents ); }
    const SView * GetParent ( uint32_t p_idx ) const
    {
        return static_cast < const SView * > ( VectorGet ( & m_self -> parents, p_idx ) );
    }

    uint32_t OverrideCount () const { return VectorLength ( & m_self -> overrides ); }
    const SView * GetOverride ( uint32_t p_idx ) const
    {
        return static_cast < const SView * > ( VectorGet ( & m_self -> parents, p_idx ) );
    }

    uint32_t ColumnCount () const { return VectorLength ( & m_self -> col ); }
    const SColumn * GetColumn ( uint32_t p_idx ) const
    {
        return static_cast < const SColumn * > ( VectorGet ( & m_self -> col, p_idx ) );
    }

    uint32_t ColumnNameCount () const { return VectorLength ( & m_self -> cname ); }
    const SNameOverload * GetColumnName ( uint32_t p_idx ) const
    {
        return static_cast < const SNameOverload * > ( VectorGet ( & m_self -> cname, p_idx ) );
    }

    uint32_t ProductionCount () const { return VectorLength ( & m_self -> prod ); }
    const SProduction * GetProduction ( uint32_t p_idx ) const
    {
        return static_cast < const SProduction * > ( VectorGet ( & m_self -> prod, p_idx ) );
    }

    const KSymbol * VectorGetSymbol ( const Vector & p_vect, uint32_t p_idx ) const
    {
        return static_cast < const KSymbol * > ( VectorGet ( & p_vect, p_idx ) );
    }

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

    const STable * GetTable ( uint32_t p_idx )
    {
        return static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, p_idx ) );
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
};

FIXTURE_TEST_CASE(View_OneTable_NoParents, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1 <T t> {}", "v" );

    REQUIRE_NULL ( v . m_self -> name -> dad );
    REQUIRE_EQ ( ( uint32_t ) eView, v . m_self -> name -> type );
    // verify entry in the container of (overloaded) view names
    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> vname ) );
    REQUIRE_EQ ( 1u, v . OverloadCount () );
    REQUIRE_EQ ( v . GetOverloads (), ( const SNameOverload * ) VectorGet ( & GetSchema () -> vname, 0 ) );
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
    REQUIRE_EQ ( 2u, v1 . OverloadCount () );
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
    REQUIRE_EQ ( 2u, v1 . OverloadCount () );
}

FIXTURE_TEST_CASE(View_OlderMinorVersion, AST_View_Fixture)
{   // older minor version is ignored
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1.2 <T t> {}; view v#1.1 <T t> {}", "v" );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> view ) );
    REQUIRE_EQ ( Version ( 1, 2 ), v . Version () );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> vname ) );
    REQUIRE_EQ ( 1u, v . OverloadCount () );
}

FIXTURE_TEST_CASE(View_NewerMinorVersion, AST_View_Fixture)
{   // older minor version is replaced
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1.1 <T t> {}; view v#1.2 <T t> {}", "v" );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> view ) );
    REQUIRE_EQ ( Version ( 1, 2 ), v . Version () );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> vname ) );
    REQUIRE_EQ ( 1u, v . OverloadCount () );
}

// Parameters
FIXTURE_TEST_CASE(View_Parameter_Table, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1 <T t> {}", "v" );
    REQUIRE_EQ ( 1u, v . ParamTableCount () );
    REQUIRE_EQ ( 0u, v . ParamViewCount () );
    REQUIRE_EQ ( GetTable ( 0 ), v . GetParamTable ( 0 ) );
}

FIXTURE_TEST_CASE(View_Parameter_View, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view X#1 <T t> {}; view v#1 <X v> {}", "v", 1 );
    REQUIRE_EQ ( 0u, v . ParamTableCount () );
    REQUIRE_EQ ( 1u, v . ParamViewCount () );
    REQUIRE_EQ ( GetView ( 0 ) .  m_self, v . GetParamView ( 0 ) );
}

FIXTURE_TEST_CASE(View_Parameter_TableAndView, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view X#1 <T t> {}; view v#1 <T t, X v> {}", "v", 1 );
    REQUIRE_EQ ( 1u, v . ParamTableCount () );
    REQUIRE_EQ ( 1u, v . ParamViewCount () );
    REQUIRE_EQ ( GetTable ( 0 ), v . GetParamTable ( 0 ) );
    REQUIRE_EQ ( GetView ( 0 ) .  m_self, v . GetParamView ( 0 ) );
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
    REQUIRE_EQ ( GetTable ( 0 ), v . GetParamTable ( 0 ) );
}

FIXTURE_TEST_CASE(View_Parameter_VersionedView, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view a:V#1 <T t> {}; "
        "view a:V#2 <T t> {}; "
        "view v#1 <a:V#2 v> {}", "v", 2 );
    REQUIRE_EQ ( GetView ( 1 ) . m_self, v . GetParamView ( 0 ) );
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
    REQUIRE_EQ ( 0u, v . ParentCount () );
}

FIXTURE_TEST_CASE(View_Parents_Multiple, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view V1#1 <T t> {}; "
        "view V2#2 <T t> {}; "
        "view W#1 <T v> = V1, V2 {}",
        "W", 2 );
    REQUIRE_EQ ( 2u, v . ParentCount () );
    REQUIRE_EQ ( GetView ( 0 ) . m_self, v . GetParent ( 0 ) );
    REQUIRE_EQ ( GetView ( 1 ) . m_self, v . GetParent ( 1 ) );
}

FIXTURE_TEST_CASE(View_Parents_Unknown, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> = Z {}; ",
        "Undeclared identifier: 'Z'" );
}

FIXTURE_TEST_CASE(View_Parents_Duplicate, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> {}; "
        "view W#1 <T v> = V, V {}",
        "Same view inherited from more than once: 'V'" );
}

FIXTURE_TEST_CASE(View_Parents_NotAView, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> {}; "
        "view W#1 <T v> = V, T {}",
        "A view's parent has to be a view: 'T'" );
}

FIXTURE_TEST_CASE(View_Parents_Redundant, AST_View_Fixture)
{   // a parent in an ancestry more than once
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view V1#1 <T t> {}"
        "view V2#1 <T t> = V1 {}"
        "view V3#1 <T t> = V2, V1 {}", // V1 does not collide with itself here
        "V3", 2 );
    REQUIRE_EQ ( 2u, v . ParentCount () );
}

// View body: columns

FIXTURE_TEST_CASE(View_Column, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view W#1 <T v> { column U8 c = 1; }", "W" );
    REQUIRE_EQ ( 1u, v . ColumnCount () );
    const SColumn & c = * v . GetColumn ( 0 );
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
    REQUIRE_EQ ( 0u, c . cid . ctx );
    REQUIRE_EQ ( 0u, c . cid . id );
    REQUIRE ( ! c . dflt );
    REQUIRE ( c . read_only );
    REQUIRE ( ! c . simple );

    REQUIRE_EQ ( ( const void * ) v . GetColumnName ( 0 ), c . name -> u . obj );
}

FIXTURE_TEST_CASE(View_Column_Redeclared, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; view W#1 <T v> { column U8 c = 1; column U8 c = 2; }",
        "Object already defined: 'c'" );
}

FIXTURE_TEST_CASE(View_Column_Reference, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view W#1 <T v> { column U8 c1 = 1; column U8 c2 = c1; }", "W" );
    REQUIRE_EQ ( 2u, v . ColumnCount () );
    const SColumn & c = * v . GetColumn ( 1 );
    REQUIRE_NOT_NULL ( c . name );
    REQUIRE_EQ ( string ("c2"), ToCppString ( c . name -> name ) );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_EQ ( ( uint32_t ) eColExpr, c . read -> var );
}

FIXTURE_TEST_CASE(View_Column_ReferenceToParamTablesColumn, AST_View_Fixture)
{   // introducing member expressions
    ViewAccess v = ParseView ( "version 2; table T#1 { column U8 c1 = 1; }; view W#1 <T t> { column U8 c2 = t . c1; }", "W" );
    REQUIRE_EQ ( 1u, v . ColumnCount () );
    const SColumn & c = * v . GetColumn ( 0 );
    REQUIRE_NOT_NULL ( c . name );
    REQUIRE_EQ ( string ("c2"), ToCppString ( c . name -> name ) );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_EQ ( ( uint32_t ) eColExpr, c . read -> var );
}
FIXTURE_TEST_CASE(View_Column_ReferenceToParamTablesColumn_PseudoPhysicalToken, AST_View_Fixture)
{   // .b looks like a physical identifier
    ViewAccess v = ParseView ( "version 2; table T#1 { column U8 c1 = 1; }; view W#1 <T t> { column U8 c2 = t .c1; }", "W" );
    REQUIRE_EQ ( ( uint32_t ) eColExpr, v . GetColumn ( 0 ) -> read -> var );
}
FIXTURE_TEST_CASE(View_Column_ReferenceToParamTablesProduction, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 { U8 c1 = 1; }; view W#1 <T t> { column U8 c2 = t . c1; }", "W" );
    REQUIRE_EQ ( ( uint32_t ) eColExpr, v . GetColumn ( 0 ) -> read -> var );
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
    const SColumn & c = * v . GetColumn ( 0 );
    REQUIRE_NOT_NULL ( c . read );
    REQUIRE_EQ ( ( uint32_t ) eColExpr, v . GetColumn ( 0 ) -> read -> var );
}

FIXTURE_TEST_CASE(View_Column_ReferenceToParamViewsColumn_Undefined, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 { U8 c1 = 1; };"
        " view V#1 <T t> { column U8 c2 = t . c1; }"
        " view W#1 <V v> { column U8 c3 = v . c22222222; }",
        "Column/production not found: 'c22222222'" );
}

// View body: productions

FIXTURE_TEST_CASE(View_Production, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view V#1 <T v> { U8 p = 1; }", "V" );
    REQUIRE_EQ ( 1u, v . ProductionCount () );
    const SProduction & p = * v . GetProduction ( 0 );
    REQUIRE_NOT_NULL ( p . name );
    REQUIRE_EQ ( string ("p"), ToCppString ( p . name -> name ) );
    REQUIRE_EQ ( ( uint32_t ) eProduction, p . name -> type );

    REQUIRE_NOT_NULL ( p . expr );
    REQUIRE_EQ ( ( uint32_t ) eConstExpr, p . expr -> var );

    REQUIRE_NOT_NULL ( p . fd ); //TODO: verify: STypeExpr* representing U8
    REQUIRE ( ! p . trigger );
    REQUIRE ( ! p . control );
}

// Inheritance

//TODO: column name clashes with parents
//TODO: column/production names clash within body
//TODO: View_ReferenceToParentsColumn
//TODO: View_ReferenceToParentsProduction

//TODO: redeclaring parents' column (error)

//TODO: resolving parent's virtual productions
//TODO: resolving parent's virtual production as a column (error)

#if NOT_IMPLEMENTED

FIXTURE_TEST_CASE(View_Parents_ColumnsInherited, AST_View_Fixture)
{
    ViewAccess v = ParseView (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { U8 v = 1; }; "
        "view W#1 <T t> { U16 w = 1; }; "
        "view X#1 <T v> = V, W {}",
        "X", 2 );
    REQUIRE_EQ ( 2u, v . ColumnOverloadCount () );
//        const SNameOverload * GetColumnOverload ( uint32_t p_idx ) const
}

//TODO: productions inherited
//TODO: virtual productions inherited
//TODO: multilevel inheritance

FIXTURE_TEST_CASE(View_Parents_NameCollision, AST_View_Fixture)
{
    VerifyErrorMessage (
        "version 2; table T#1 {}; "
        "view V#1 <T t> { U8 p = 1; }; "
        "view W#1 <T t> { U16 p = 1; }; "
        "view X#1 <T v> = V, W {}",
        "" );
}

//TODO: bad version of a parent
#endif

// View body
//TODO: read-only columns only
//TODO: .ident not allowed
//TODO: column name clashes with parents
//TODO: column/production names clash within body
//TODO: refer to parameter's column
//TODO: refer to parent's column
//TODO: refer to parameter's production
//TODO: refer to parent's production
//TODO: overloading parents' column
//TODO: overloading own column
//TODO: overloading own production
//TODO: resolving parent's virtual productions
