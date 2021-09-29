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
* Unit tests for database declarations in schema, this file is #included into a bigger test suite
*/

class DbAccess // encapsulates access to an STable in a VSchema
{
public:
    DbAccess ( const SDatabase* p_db )
    : m_self ( p_db )
    {
    }

    const SDatabase * Parent () { return m_self -> dad; }
    uint32_t Version () const { return m_self -> version; }
    uint32_t Id () const { return m_self -> id; }

    const BSTree & Scope () const { return m_self -> scope; }

    uint32_t DbMemberCount () const { return VectorLength ( & m_self -> db ); }
    const SDBMember * GetDbMember ( uint32_t p_idx ) const
    {
        return static_cast < const SDBMember * > ( VectorGet ( & m_self -> db, p_idx ) );
    }

    uint32_t TableMemberCount () const { return VectorLength ( & m_self -> tbl ); }
    const STblMember * GetTableMember ( uint32_t p_idx ) const
    {
        return static_cast < const STblMember * > ( VectorGet ( & m_self -> tbl, p_idx ) );
    }

    uint32_t OverloadCount () const
    {
        const SNameOverload * ovl = static_cast < const SNameOverload * > ( m_self -> name -> u . obj );
        return VectorLength ( & ovl -> items );
    }
    const SDatabase * GetOverload ( uint32_t p_idx ) const
    {
        const SNameOverload * ovl = static_cast < const SNameOverload * > ( m_self -> name -> u . obj );
        return static_cast < const SDatabase * > ( VectorGet ( & ovl -> items, p_idx ) );
    }

    const SDatabase * m_self;
};


class AST_Db_Fixture : public AST_Fixture
{
public:
    AST_Db_Fixture ()
    {
    }
    ~AST_Db_Fixture ()
    {
    }

    DbAccess ParseDatabase ( const char * p_source, const char * p_name, uint32_t p_idx = 0 )
    {
        MakeAst ( p_source );

        const SDatabase * ret = static_cast < const SDatabase* > ( VectorGet ( & GetSchema () -> db, p_idx ) );
        if ( ret == 0 )
        {
            throw std :: logic_error ( "AST_Db_Fixture::ParseDatabase : DB not found" );
        }
        if ( ret -> name == 0 || string ( p_name ) != ToCppString ( ret -> name -> name ) )
        {
            throw std :: logic_error ( "AST_Db_Fixture::ParseDatabase : wrong name" );
        }
        return DbAccess ( ret );
    }
};

FIXTURE_TEST_CASE(DB_Empty_NoParent, AST_Db_Fixture)
{
    DbAccess db = ParseDatabase ( "database d#1.2 { };", "d" );
    REQUIRE_NULL ( db . Parent () );
    REQUIRE_EQ ( Version ( 1, 2 ), db .Version () );
    REQUIRE_EQ ( 0u, db .Id () );
    REQUIRE_EQ ( 1u, VectorLength ( & GetSchema() -> dname ) );
    REQUIRE_EQ ( 1u, db . OverloadCount () );
    REQUIRE_EQ ( db . m_self, db . GetOverload ( 0 ) );
}

FIXTURE_TEST_CASE(DB_Parent, AST_Db_Fixture)
{
    DbAccess db = ParseDatabase ( "database p#1 {}; database d#1 = p#1 {};", "d", 1 );
    REQUIRE_EQ ( 1u, db .Id () );
    REQUIRE_NOT_NULL ( db . Parent () );
    REQUIRE_EQ ( 0u, db . Parent () -> id );
}

FIXTURE_TEST_CASE(DB_ParentNotADatabase, AST_Table_Fixture)
{
    VerifyErrorMessage ( "table p#1 {}; database d#1 = p#1 {};", "Not a database: 'p'" );
}

FIXTURE_TEST_CASE(DB_NewVersion, AST_Db_Fixture)
{
    DbAccess db = ParseDatabase ( "database d#1.0 { }; database d#1.1 { };", "d", 1 );
    REQUIRE_EQ ( Version ( 1, 1 ), db .Version () ); // second decl accepted
}
FIXTURE_TEST_CASE(DB_OldVersion, AST_Db_Fixture)
{
    REQUIRE_THROW ( ParseDatabase ( "database d#1.1 { }; database d#1.0 { };", "d", 1 ) ); // second decl ignored
}

FIXTURE_TEST_CASE(DB_DbMember, AST_Db_Fixture)
{
    DbAccess db = ParseDatabase ( "database p#1 {}; database d#1 { database p m_p; };", "d", 1 );
    REQUIRE_EQ ( 1u, db . DbMemberCount () );
    const SDBMember * m = db . GetDbMember ( 0 );
    REQUIRE_NOT_NULL ( m );
    REQUIRE_NOT_NULL ( m -> name );
    REQUIRE_EQ ( string ( "m_p" ), ToCppString ( m -> name -> name ) );
    REQUIRE ( ! m -> tmpl );
    REQUIRE_NOT_NULL ( m -> db );
    REQUIRE_EQ ( 0u, m -> db -> id );
    const KSymbol * sym = reinterpret_cast < const KSymbol * > ( BSTreeFirst ( & db . Scope () ) );
    REQUIRE_NOT_NULL ( sym );
    REQUIRE_EQ ( string ( "m_p" ), ToCppString ( sym -> name ) );
    REQUIRE_EQ ( (uint32_t)eDBMember, sym -> type );
}

FIXTURE_TEST_CASE(DB_DbMemberDeclaredTwice, AST_Table_Fixture)
{
    VerifyErrorMessage ( "database p#1 {}; database d#1 { database p m_p; database p m_p; };",
                         "Member already exists: 'm_p'",
                         1, 60 );
}

//TODO: table member declared twice

FIXTURE_TEST_CASE(DB_ItselfAsMember, AST_Table_Fixture)
{
    VerifyErrorMessage ( "database d#1 { database d m_p; };", "Database declared but not defined: 'd'" );
}

FIXTURE_TEST_CASE(DB_DbMemberNotADatabase, AST_Table_Fixture)
{
    VerifyErrorMessage ( "table p#1 {}; database d#1 { database p m_p; };", "Not a database: 'p'" );
}

FIXTURE_TEST_CASE(DB_DbMemberVersionDoesNotExist, AST_Table_Fixture)
{
    VerifyErrorMessage ( "database p#1 {}; database d#1 { database p#2 m_p; };",
                         "Requested version does not exist: 'p#2'" );
}

FIXTURE_TEST_CASE(DB_DbMemberTemplate, AST_Db_Fixture)
{
    DbAccess db = ParseDatabase ( "database p#1 {}; database d#1 { template database p m_p; };", "d", 1 );
    REQUIRE_EQ ( 1u, db . DbMemberCount () );
    const SDBMember * m = db . GetDbMember ( 0 );
    REQUIRE ( m -> tmpl );
}

FIXTURE_TEST_CASE(DB_TableMember, AST_Db_Fixture)
{
    DbAccess db = ParseDatabase ( "table t#1 {}; database d#1 { table t m_p; };", "d", 0 );
    REQUIRE_EQ ( 1u, db . TableMemberCount () );
    const STblMember * m = db . GetTableMember ( 0 );
    REQUIRE_NOT_NULL ( m );
    REQUIRE_NOT_NULL ( m -> name );
    REQUIRE_EQ ( string ( "m_p" ), ToCppString ( m -> name -> name ) );
    REQUIRE ( ! m -> tmpl );
    REQUIRE_NOT_NULL ( m -> tbl );
    REQUIRE_EQ ( 0u, m -> tbl -> id );
    const KSymbol * sym = reinterpret_cast < const KSymbol * > ( BSTreeFirst ( & db . Scope () ) );
    REQUIRE_NOT_NULL ( sym );
    REQUIRE_EQ ( string ( "m_p" ), ToCppString ( sym -> name ) );
    REQUIRE_EQ ( (uint32_t)eTblMember, sym -> type );
}

FIXTURE_TEST_CASE(DB_TableMemberTemplate, AST_Db_Fixture)
{
    DbAccess db = ParseDatabase ( "table t#1 {}; database d#1 { template table t m_p; };", "d", 0 );
    REQUIRE_EQ ( 1u, db . TableMemberCount () );
    const STblMember * m = db . GetTableMember ( 0 );
    REQUIRE ( m -> tmpl );
}
