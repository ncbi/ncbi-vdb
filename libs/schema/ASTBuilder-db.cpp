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

#include "ASTBuilder.hpp"

#include <klib/symbol.h>

#include "../vdb/schema-parse.h"
#include "../vdb/dbmgr-priv.h"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast.hpp"

using namespace std;

class DatabaseDeclaration // Wrapper around SDatabase
{
public:
    DatabaseDeclaration ( ctx_t ctx, ASTBuilder & p_builder );
    ~DatabaseDeclaration ();

    bool SetName ( ctx_t ctx, const AST_FQN &  p_fqn );
    bool HandleParent ( ctx_t ctx, const AST & p_parents );
    void HandleBody ( ctx_t ctx, const AST & p_body );

private:
    bool HandleOverload ( ctx_t ctx, const KSymbol * p_priorDecl );
    void HandleMemberDb ( ctx_t ctx, const AST & p_member );
    void HandleMemberTable ( ctx_t ctx, const AST & p_member );
    void HandleMemberViewAlias ( ctx_t ctx, const AST & p_member );

private:
    ASTBuilder &    m_builder;
    SDatabase *     m_self;
};

DatabaseDeclaration :: DatabaseDeclaration ( ctx_t ctx, ASTBuilder & p_builder )
:   m_builder ( p_builder )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    m_self = m_builder . Alloc < SDatabase > ( ctx );
    if ( m_self != 0 )
    {
        /* prepare vectors */
        VectorInit ( & m_self -> db, 0, 8 );
        VectorInit ( & m_self -> tbl, 0, 8 );
    }
}

DatabaseDeclaration :: ~DatabaseDeclaration ()
{
}

bool
DatabaseDeclaration :: SetName ( ctx_t ctx, const AST_FQN &  p_fqn )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    if ( m_self != 0 )
    {
       /* version */
        m_self -> version = p_fqn . GetVersion ();
        const KSymbol * priorDecl = m_builder . Resolve ( ctx, p_fqn, false );
        if ( priorDecl == 0 )
        {
            m_self -> name = m_builder . CreateFqnSymbol ( ctx, p_fqn, eDatabase, & m_self );
            if ( m_self -> name != 0 &&
                 m_builder . CreateOverload ( ctx,
                                              * m_self -> name,
                                              m_self,
                                              0,
                                              SDatabaseSort,
                                              m_builder . GetSchema () -> db,
                                              m_builder . GetSchema () -> dname,
                                              m_self -> id ) )
            {
                return true;
            }
        }
        else if ( HandleOverload ( ctx, priorDecl ) )
        {
            // declared previously, this declaration not ignored
            m_self -> name = priorDecl;
            return true;
        }

        SDatabaseWhack ( m_self, 0 );
    }
    return false;
}

bool
DatabaseDeclaration :: HandleParent ( ctx_t ctx, const AST & p_parent )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    /* look for inheritance */
    if ( p_parent . GetTokenType () != PT_EMPTY )
    {
        const AST_FQN & parent = * ToFQN ( & p_parent );
        const KSymbol * parentDecl = m_builder . Resolve ( ctx, parent, true );
        if ( parentDecl -> type != eDatabase )
        {
            m_builder . ReportError ( ctx, "Not a database", parent );
            return false;
        }

        const SDatabase * dad = static_cast < const SDatabase * > ( m_builder . SelectVersion ( ctx, parent, * parentDecl, SDatabaseCmp ) );
        if ( dad != 0 )
        {
            rc_t rc = SDatabaseExtend ( m_self, dad );
            if ( rc != 0 )
            {
                m_builder . ReportRc ( ctx, "SDatabaseExtend", rc );
                return false;
            }
        }
    }
    return true;
}

void
DatabaseDeclaration :: HandleBody ( ctx_t ctx, const AST & p_body )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    rc_t rc = push_db_scope ( & m_builder . GetSymTab (), m_self );
    if ( rc == 0 )
    {
        rc = KSymTablePushScope ( & m_builder . GetSymTab (), & m_self -> scope );
        if ( rc == 0 )
        {
            uint32_t count = p_body . ChildrenCount ();
            for ( uint32_t i = 0 ; i < count; ++ i )
            {
                const AST & member = * p_body . GetChild ( i );
                switch ( member . GetTokenType () )
                {
                case PT_DBMEMBER:
                    HandleMemberDb ( ctx, member );
                    break;

                case PT_TBLMEMBER:
                    HandleMemberTable ( ctx, member );
                    break;

                case PT_ALIASMEMBER:
                    HandleMemberViewAlias ( ctx, member );
                    break;

                case PT_EMPTY:
                    break;

                default:
                    assert ( false );
                }
            }
            KSymTablePopScope ( & m_builder . GetSymTab () );
        }
        else
        {
            m_builder . ReportRc ( ctx, "KSymTablePushScope", rc);
        }

        pop_db_scope ( & m_builder . GetSymTab (), m_self );
    }
    else
    {
        m_builder . ReportRc ( ctx, "push_db_scope", rc);
    }
}

bool
DatabaseDeclaration :: HandleOverload ( ctx_t ctx, const KSymbol * p_priorDecl )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_priorDecl != 0 );

    Vector & dbs = m_builder . GetSchema () -> db;

    SNameOverload *name = ( SNameOverload* ) p_priorDecl -> u . obj;
    assert ( name != 0 );

    uint32_t idx;
    rc_t rc = VectorInsertUnique ( & name -> items, m_self, & idx, SDatabaseSort );
    if ( rc == 0 ) // overload added
    {
        return m_builder . VectorAppend ( ctx, dbs, & m_self -> id, m_self );
    }
    if ( GetRCState ( rc ) == rcExists )
    {   /* an overload with the same major version exists */
        /* see if new table trumps old */
        const SDatabase *exist = static_cast < const SDatabase * > ( VectorGet ( & name -> items, idx ) );
        const SDatabase *newer;
        rc = SDatabaseCompare ( exist, m_self, & newer, false );
        if ( rc == 0 && newer == m_self )
        {
            /* put the new one in place of the existing */
            void * ignore;
            VectorSwap ( & name -> items, idx, m_self, & ignore );
            m_self -> id = exist -> id;

            if ( m_builder . VectorAppend ( ctx, dbs, & m_self -> id, m_self ) )
            {
                /* TODO - need to update parent/child relationships */
                return true;
            }
        }
    }
    else if ( rc != 0 )
    {
        m_builder . ReportRc ( ctx, "VectorInsertUnique", rc );
    }
    return false;
}

void
DatabaseDeclaration :: HandleMemberDb ( ctx_t ctx, const AST & p_member )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_member . ChildrenCount () == 3 );

    SDBMember * m = m_builder . Alloc < SDBMember > ( ctx );
    if ( m != 0 )
    {
        if ( p_member . GetChild ( 0 ) -> GetTokenType () == KW_template )
        {
            m -> tmpl = true;
        }

        const AST_FQN & type = * ToFQN ( p_member . GetChild ( 1 ) );
        const KSymbol * dbName = m_builder . Resolve ( ctx, type );
        if ( dbName != 0 )
        {
            if ( dbName -> type == eDatabase )
            {
                if ( dbName != m_self -> name )
                {
                    String memName;
                    const AST & ident = * p_member . GetChild ( 2 );
                    assert ( ident . GetChild ( 0 ) != 0 );
                    StringInitCString ( & memName, ident . GetChild ( 0 ) -> GetTokenValue () );
                    rc_t rc = KSymTableCreateConstSymbol ( & m_builder . GetSymTab (), & m -> name, & memName, eDBMember, m );
                    if ( rc == 0 )
                    {
                        m -> db = static_cast < const SDatabase * > ( m_builder . SelectVersion ( ctx, type, * dbName, SDatabaseCmp ) );
                        if ( m -> db != 0 )
                        {
                            m_builder . VectorAppend ( ctx, m_self -> db, & m -> cid . id, m );
                            return;
                        }
                    }
                    else if ( GetRCState ( rc ) == rcExists )
                    {
                        m_builder . ReportError ( ctx, ident . GetLocation (), "Member already exists", memName );
                    }
                    else
                    {
                        m_builder . ReportRc ( ctx, "KSymTableCreateConstSymbol", rc);
                    }
                }
                else
                {
                    m_builder . ReportError ( ctx, "Database declared but not defined", type );
                }
            }
            else
            {
                m_builder . ReportError ( ctx, "Not a database", type );
            }
        }
        SDBMemberWhack ( m, 0 );
    }
}

void
DatabaseDeclaration :: HandleMemberTable ( ctx_t ctx, const AST & p_member )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_member . ChildrenCount () == 3 );

    STblMember * m = m_builder . Alloc < STblMember > ( ctx );
    if ( m != 0 )
    {
        if ( p_member . GetChild ( 0 ) -> GetTokenType () == KW_template )
        {
            m -> tmpl = true;
        }

        const AST_FQN & type = * ToFQN ( p_member . GetChild ( 1 ) );
        const KSymbol * tblName = m_builder . Resolve ( ctx, type );
        if ( tblName != 0 )
        {
            if ( tblName -> type == eTable )
            {
                String memName;
                assert ( p_member . GetChild ( 2 ) -> GetChild ( 0 ) != 0 );
                StringInitCString ( & memName, p_member . GetChild ( 2 ) -> GetChild ( 0 ) -> GetTokenValue () );
                rc_t rc = KSymTableCreateConstSymbol ( & m_builder . GetSymTab (), & m -> name, & memName, eTblMember, m );
                if ( rc == 0 )
                {
                    m -> tbl = static_cast < const STable * > ( m_builder . SelectVersion ( ctx, type, * tblName, STableCmp ) );
                    if ( m -> tbl != 0 )
                    {
                        m_builder . VectorAppend ( ctx, m_self -> tbl, & m -> cid . id, m );
                        return;
                    }
                }
                else if ( GetRCState ( rc ) == rcExists )
                {
                    m_builder . ReportError ( ctx, p_member . GetLocation (), "Member already exists", memName );
                }
                else
                {
                    m_builder . ReportRc ( ctx, "KSymTableCreateConstSymbol", rc);
                }
            }
            else
            {
                m_builder . ReportError ( ctx, "Not a table", type );
            }
        }
        STblMemberWhack ( m, 0 );
    }
}

class MemberViewAlias
{
public:
    MemberViewAlias( const AST & p_ast ) : m_ast( p_ast )
    {
        assert ( m_ast . GetTokenType () == PT_ALIASMEMBER );
        assert ( m_ast . ChildrenCount () == 2 );
    }

    // ViewAlias: spec name
    const AST & ViewSpec() const
    {
        const AST * ret = m_ast.GetChild( 0 );
        assert ( ret -> GetTokenType () == PT_VIEWSPEC );
        return * ret;
    }
    const char * MemberName() const
    {
        const AST * n = m_ast.GetChild( 1 );
        assert ( n != nullptr );
        assert ( n -> GetTokenType () == PT_IDENT );
        const AST * ret = n -> GetChild( 0 );
        assert ( ret != nullptr );
        return ret -> GetTokenValue ();
    }

    // ViewSpec: view < params >
    const AST_FQN & ViewSpec_View() const
    {
        const AST_FQN * ret = ToFQN( ViewSpec() . GetChild ( 0 ) );
        assert ( ret != nullptr );
        return * ret;
    }
    const AST & ViewSpec_Params() const
    {
        const AST * ret = ViewSpec() . GetChild ( 1 );
        assert ( ret != nullptr );
        return * ret;
    }

private:
    const AST & m_ast;
};

void
DatabaseDeclaration :: HandleMemberViewAlias ( ctx_t ctx, const AST & p_member )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );

    MemberViewAlias alias( p_member );

    SViewAliasMember * m = m_builder . Alloc < SViewAliasMember > ( ctx );
    if ( m != 0 )
    {
        const KSymbol * viewName = m_builder . Resolve ( ctx, alias.ViewSpec_View() );
        if ( viewName != 0 )
        {
            if ( viewName -> type == eView )
            {
                String memName;
                StringInitCString ( & memName, alias . MemberName() );
                rc_t rc = KSymTableCreateConstSymbol ( & m_builder . GetSymTab (), & m -> name, & memName, eViewAliasMember, m );
                if ( rc == 0 )
                {
                    m -> view . dad = static_cast < const SView * > ( m_builder . SelectVersion ( ctx, alias.ViewSpec_View(), * viewName, SViewCmp ) );
                    assert( m -> view . dad );

                    // verify parameter tables/views
                    const AST & params = alias.ViewSpec_Params();
                    uint32_t count = params . ChildrenCount ();

                    const Vector & formalParams = m -> view . dad -> params;
                    if ( VectorLength( & formalParams ) != count )
                    {
                        m_builder . ReportError ( ctx, "Incorrect number of view parameters", alias.ViewSpec_View() );
                    }
                    else
                    {
                        for ( uint32_t i = 0; i < count; ++i )
                        {
                            const AST_FQN & param = * ToFQN ( params . GetChild ( i ) );
                            assert ( param . GetTokenType () == PT_IDENT );
                            const KSymbol * formal = static_cast< const KSymbol * > ( VectorGet( & formalParams, i ) );

                            const KSymbol * paramDecl = m_builder . Resolve ( ctx, param ); // will report unknown name
                            if ( paramDecl != 0 )
                            {
                                switch ( paramDecl -> type )
                                {
                                case eTblMember:
                                {
                                    //check compatibility
                                    if ( formal -> type == eTable )
                                    {
                                        const STable * param_table = ((const STblMember *) paramDecl -> u . obj) -> tbl;
                                        if ( STableIsA ( param_table, (const STable *) formal -> u . obj ) )
                                        {
                                            m_builder . VectorAppend ( ctx, m -> view . params, nullptr, paramDecl );
                                            break;
                                        }
                                    }
                                    m_builder . ReportError ( ctx, "View parameter type mismatch", param );
                                    break;
                                }
                                case eViewAliasMember:
                                {
                                    //check compatibility
                                    if ( formal -> type == eView )
                                    {
                                        const SView * param_view = ((const SViewAliasMember *) paramDecl -> u . obj) -> view . dad;
                                        if ( SViewIsA ( param_view, (const SView *) formal -> u . obj ) )
                                        {
                                            m_builder . VectorAppend ( ctx, m -> view . params, nullptr, paramDecl );
                                            break;
                                        }
                                    }
                                    m_builder . ReportError ( ctx, "View parameter type mismatch", param );
                                    break;
                                }
                                default:
                                    m_builder . ReportError ( ctx, "Not a table/view member", param );
                                    break;
                                }
                            }
                        }

                        m_builder . VectorAppend ( ctx, m_self -> aliases, & m -> cid . id, m );
                        return;
                    }
                }
                else if ( GetRCState ( rc ) == rcExists )
                {
                    m_builder . ReportError ( ctx, p_member . GetLocation (), "Member already exists", memName );
                }
                else
                {
                    m_builder . ReportRc ( ctx, "KSymTableCreateConstSymbol", rc);
                }
            }
            else
            {
                m_builder . ReportError ( ctx, "Not a view", alias.ViewSpec_View() );
            }
        }
        SViewAliasMemberWhack ( m, 0 );
    }
}

AST *
ASTBuilder :: DatabaseDef ( ctx_t ctx, const Token * p_token, AST_FQN * p_fqn, AST * p_parent, AST * p_body )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    AST * ret = AST :: Make ( ctx, p_token, p_fqn, p_parent, p_body );

    DatabaseDeclaration db ( ctx, * this );
    if ( db . SetName ( ctx, * p_fqn ) )
    {
        if ( db . HandleParent ( ctx, * p_parent ) )
        {
            db . HandleBody ( ctx, * p_body );
        }
    }

    return ret;
}

