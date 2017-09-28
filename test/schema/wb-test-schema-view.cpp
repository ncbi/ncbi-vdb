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
        return static_cast < const STable * > ( VectorGet ( & m_self -> tables, p_idx ) );
    }
    uint32_t ParamViewCount () const { return VectorLength ( & m_self -> views ); }
    const SView * GetParamView ( uint32_t p_idx ) const
    {
        return static_cast < const SView * > ( VectorGet ( & m_self -> views, p_idx ) );
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
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1 <T t> { U8 p=1; }", "v" );

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
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1.2.3 <T t> { U8 p=1; }", "v" );
    REQUIRE_EQ ( Version ( 1, 2, 3 ), v . Version () );
}

FIXTURE_TEST_CASE(View_Version_Redeclared, AST_Fixture)
{
    VerifyErrorMessage ( "version 2; table T#1 {}; view v#1.2.3 <T t> { U8 p=1; }; view v#1.2.3 <T t> { U8 p=1; }",
                              "This version is already declared: 'v'" );
}

FIXTURE_TEST_CASE(View_NewerMajorVersion, AST_View_Fixture)
{   // both version are registered
    ViewAccess v1 = ParseView ( "version 2; table T#1 {}; view v#1 <T t> { U8 p=1; }; view v#2 <T t> { U8 p=1; }", "v" );
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
    ViewAccess v1 = ParseView ( "version 2; table T#1 {}; view v#2 <T t> { U8 p=1; }; view v#1 <T t> { U8 p=1; }", "v" );
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
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1.2 <T t> { U8 p=1; }; view v#1.1 <T t> { U8 p=1; }", "v" );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> view ) );
    REQUIRE_EQ ( Version ( 1, 2 ), v . Version () );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> vname ) );
    REQUIRE_EQ ( 1u, v . OverloadCount () );
}

FIXTURE_TEST_CASE(View_NewerMinorVersion, AST_View_Fixture)
{   // older minor version is replaced
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1.1 <T t> { U8 p=1; }; view v#1.2 <T t> { U8 p=1; }", "v" );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> view ) );
    REQUIRE_EQ ( Version ( 1, 2 ), v . Version () );

    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema () -> vname ) );
    REQUIRE_EQ ( 1u, v . OverloadCount () );
}

// Parameters
FIXTURE_TEST_CASE(View_Parameter_Table, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view v#1 <T t> { U8 p=1; }", "v" );
    REQUIRE_EQ ( 1u, v . ParamTableCount () );
    REQUIRE_EQ ( 0u, v . ParamViewCount () );
    REQUIRE_EQ ( GetTable ( 0 ), v . GetParamTable ( 0 ) );
}

FIXTURE_TEST_CASE(View_Parameter_View, AST_View_Fixture)
{
    ViewAccess v = ParseView ( "version 2; table T#1 {}; view x#1 <T t> { U8 p=1; }; view v#1 <T t> { U8 p=1; }", "v", 1 );
    REQUIRE_EQ ( 0u, v . ParamTableCount () );
    REQUIRE_EQ ( 1u, v . ParamViewCount () );
    REQUIRE_EQ ( GetView ( 0 ) .  m_self, v . GetParamView ( 0 ) );
}

//TODO: both a table and a view as parameters
//TODO: not a table or a view as parameter
//TODO: undefined name as parameter
//TODO: nonexistent version of a table as parameter
//TODO: nonexistent version of a view  as parameter
//TODO: duplicate name in parameters

// View parents
//TODO: multiple views as parents
//TODO: same view as multiple parents
//TODO: name clashes betsween parents' columns/productions

// View body
//TODO: read-only columns only
//TODO: .ident not allowed
//TODO: column name clash with parents
//TODO: column/production name clash within body
//TODO: overloading parents' column
//TODO: overloading own column
//TODO: overloading own production
//TODO: resolving parent's virtual productions
