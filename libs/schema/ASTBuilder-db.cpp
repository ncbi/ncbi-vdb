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
#include "schema-ast-tokens.h"

using namespace std;

class DatabaseDeclaration // Wrapper around SDatabase
{
public:
    DatabaseDeclaration ( ASTBuilder & p_builder );
    ~DatabaseDeclaration ();

    bool SetName ( const AST_FQN &  p_fqn );
    bool HandleParent ( const AST & p_parents );
    void HandleBody ( const AST & p_body );

private:
    bool HandleOverload ( const KSymbol * p_priorDecl );
    void HandleMemberDb ( const AST & p_member );
    void HandleMemberTable ( const AST & p_member );

private:
    ASTBuilder &    m_builder;
    SDatabase *     m_self;
};

DatabaseDeclaration :: DatabaseDeclaration ( ASTBuilder & p_builder )
:   m_builder ( p_builder ),
    m_self ( m_builder . Alloc < SDatabase > () )
{
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
DatabaseDeclaration :: SetName ( const AST_FQN &  p_fqn )
{
    if ( m_self != 0 )
    {
       /* version */
        m_self -> version = p_fqn . GetVersion ();
        const KSymbol * priorDecl = m_builder . Resolve ( p_fqn, false );
        if ( priorDecl == 0 )
        {
            m_self -> name = m_builder . CreateFqnSymbol ( p_fqn, eDatabase, & m_self );
            if ( m_self -> name != 0 &&
                 m_builder . CreateOverload ( m_self -> name,
                                              m_self,
                                              SDatabaseSort,
                                              m_builder . GetSchema () -> db,
                                              m_builder . GetSchema () -> dname,
                                              & m_self -> id ) )
            {
                return true;
            }
        }
        else if ( HandleOverload ( priorDecl ) )
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
DatabaseDeclaration :: HandleParent ( const AST & p_parent )
{
    /* look for inheritance */
    if ( p_parent . GetTokenType () != PT_EMPTY )
    {
        const AST_FQN & parent = * ToFQN ( & p_parent );
        const KSymbol * parentDecl = m_builder . Resolve ( parent, true );
        if ( parentDecl -> type != eDatabase )
        {
            m_builder . ReportError ( "Not a database", parent );
            return false;
        }

        const SDatabase * dad = static_cast < const SDatabase * > ( m_builder . SelectVersion ( parent, * parentDecl, SDatabaseCmp ) );
        if ( dad != 0 )
        {
            rc_t rc = SDatabaseExtend ( m_self, dad );
            if ( rc != 0 )
            {
                m_builder . ReportRc ( "SDatabaseExtend", rc );
                return false;
            }
        }
    }
    return true;
}

void
DatabaseDeclaration :: HandleBody ( const AST & p_body )
{
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
                    HandleMemberDb ( member );
                    break;

                case PT_TBLMEMBER:
                    HandleMemberTable ( member );
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
            m_builder . ReportRc ( "KSymTablePushScope", rc);
        }

        pop_db_scope ( & m_builder . GetSymTab (), m_self );
    }
    else
    {
        m_builder . ReportRc ( "push_db_scope", rc);
    }
}

bool
DatabaseDeclaration :: HandleOverload ( const KSymbol * p_priorDecl )
{
    assert ( p_priorDecl != 0 );

    Vector & dbs = m_builder . GetSchema () -> db;

    SNameOverload *name = ( SNameOverload* ) p_priorDecl -> u . obj;
    assert ( name != 0 );

    uint32_t idx;
    rc_t rc = VectorInsertUnique ( & name -> items, m_self, & idx, SDatabaseSort );
    if ( rc == 0 ) // overload added
    {
        return m_builder . VectorAppend ( dbs, & m_self -> id, m_self );
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

            if ( m_builder . VectorAppend ( dbs, & m_self -> id, m_self ) )
            {
                /* TODO - need to update parent/child relationships */
                return true;
            }
        }
    }
    else if ( rc != 0 )
    {
        m_builder . ReportRc ( "VectorInsertUnique", rc );
    }
    return false;
}

void
DatabaseDeclaration :: HandleMemberDb ( const AST & p_member )
{
    assert ( p_member . ChildrenCount () == 3 );

    SDBMember * m = m_builder . Alloc < SDBMember > ();
    if ( m != 0 )
    {
        if ( p_member . GetChild ( 0 ) -> GetTokenType () == KW_template )
        {
            m -> tmpl = true;
        }

        const AST_FQN & type = * ToFQN ( p_member . GetChild ( 1 ) );
        const KSymbol * dbName = m_builder . Resolve ( type );
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
                        m -> db = static_cast < const SDatabase * > ( m_builder . SelectVersion ( type, * dbName, SDatabaseCmp ) );
                        if ( m -> db != 0 )
                        {
                            m_builder . VectorAppend ( m_self -> db, & m -> cid . id, m );
                            return;
                        }
                    }
                    else if ( GetRCState ( rc ) == rcExists )
                    {
                        m_builder . ReportError ( ident . GetLocation (), "Member already exists", memName );
                    }
                    else
                    {
                        m_builder . ReportRc ( "KSymTableCreateConstSymbol", rc);
                    }
                }
                else
                {
                    m_builder . ReportError ( "Database declared but not defined", type );
                }
            }
            else
            {
                m_builder . ReportError ( "Not a database", type );
            }
        }
        SDBMemberWhack ( m, 0 );
    }
}

void
DatabaseDeclaration :: HandleMemberTable ( const AST & p_member )
{
    assert ( p_member . ChildrenCount () == 3 );

    STblMember * m = m_builder . Alloc < STblMember > ();
    if ( m != 0 )
    {
        if ( p_member . GetChild ( 0 ) -> GetTokenType () == KW_template )
        {
            m -> tmpl = true;
        }

        const AST_FQN & type = * ToFQN ( p_member . GetChild ( 1 ) );
        const KSymbol * tblName = m_builder . Resolve ( type );
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
                    m -> tbl = static_cast < const STable * > ( m_builder . SelectVersion ( type, * tblName, STableCmp ) );
                    if ( m -> tbl != 0 )
                    {
                        m_builder . VectorAppend ( m_self -> tbl, & m -> cid . id, m );
                        return;
                    }
                }
                else if ( GetRCState ( rc ) == rcExists )
                {
                    m_builder . ReportError ( p_member . GetLocation (), "Member already exists", memName );
                }
                else
                {
                    m_builder . ReportRc ( "KSymTableCreateConstSymbol", rc);
                }
            }
            else
            {
                m_builder . ReportError ( "Not a table", type );
            }
        }
        STblMemberWhack ( m, 0 );
    }
}

AST *
ASTBuilder :: DatabaseDef ( const Token * p_token, AST_FQN * p_fqn, AST * p_parent, AST * p_body )
{
    AST * ret = new AST ( p_token, p_fqn, p_parent, p_body );

    DatabaseDeclaration db ( * this );
    if ( db . SetName ( * p_fqn ) )
    {
        if ( db . HandleParent ( * p_parent ) )
        {
            db . HandleBody ( * p_body );
        }
    }

    return ret;
}

