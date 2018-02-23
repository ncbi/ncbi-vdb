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

// hide an unfortunately named C function typename()
#define typename __typename
#include "../vdb/schema-parse.h"
#undef typename
#include "../vdb/schema-priv.h"
#include "../vdb/dbmgr-priv.h"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast-tokens.h"

using namespace std;

bool
ASTBuilder :: HandleDbOverload ( const SDatabase *  p_db,
                                 uint32_t           p_version,
                                 const KSymbol *    p_priorDecl,
                                 uint32_t *         p_id )
{
    assert ( p_db != 0 );
    assert ( p_priorDecl != 0 );
    assert ( p_id != 0 );

    Vector & dbs = GetSchema () -> db;

    SNameOverload *name = ( SNameOverload* ) p_priorDecl -> u . obj;
    assert ( name != 0 );

    uint32_t idx;
    rc_t rc = VectorInsertUnique ( & name -> items, p_db, & idx, SDatabaseSort );
    if ( rc == 0 ) // overload added
    {
        return VectorAppend ( dbs, p_id, p_db );
    }
    if ( GetRCState ( rc ) == rcExists )
    {   /* an overload with the same major version exists */
        /* see if new table trumps old */
        const SDatabase *exist = static_cast < const SDatabase * > ( VectorGet ( & name -> items, idx ) );
        const SDatabase *newer;
        rc = SDatabaseCompare ( exist, p_db, & newer, false );
        if ( rc == 0 && newer == p_db )
        {
            /* put the new one in place of the existing */
            void * ignore;
            VectorSwap ( & name -> items, idx, p_db, & ignore );
            * p_id = exist -> id;

            if ( VectorAppend ( dbs, p_id, p_db ) )
            {
                /* TODO - need to update parent/child relationships */
                return true;
            }
        }
    }
    else if ( rc != 0 )
    {
        ReportRc ( "VectorInsertUnique", rc );
    }
    return false;
}

void
ASTBuilder :: HandleDbMemberDb ( SDatabase & p_db, const AST & p_member )
{
    assert ( p_member . ChildrenCount () == 3 );

    SDBMember * m = Alloc < SDBMember > ();
    if ( m != 0 )
    {
        if ( p_member . GetChild ( 0 ) -> GetTokenType () == KW_template )
        {
            m -> tmpl = true;
        }

        const AST_FQN & type = * ToFQN ( p_member . GetChild ( 1 ) );
        const KSymbol * dbName = Resolve ( type );
        if ( dbName != 0 )
        {
            if ( dbName -> type == eDatabase )
            {
                if ( dbName != p_db . name )
                {
                    String memName;
                    const AST & ident = * p_member . GetChild ( 2 );
                    assert ( ident . GetChild ( 0 ) != 0 );
                    StringInitCString ( & memName, ident . GetChild ( 0 ) -> GetTokenValue () );
                    rc_t rc = KSymTableCreateConstSymbol ( & GetSymTab (), & m -> name, & memName, eDBMember, m );
                    if ( rc == 0 )
                    {
                        m -> db = static_cast < const SDatabase * > ( SelectVersion ( type, * dbName, SDatabaseCmp ) );
                        if ( m -> db != 0 )
                        {
                            VectorAppend ( p_db . db, & m -> cid . id, m );
                            return;
                        }
                    }
                    else if ( GetRCState ( rc ) == rcExists )
                    {
                        ReportError ( ident . GetLocation (), "Member already exists", memName );
                    }
                    else
                    {
                        ReportRc ( "KSymTableCreateConstSymbol", rc);
                    }
                }
                else
                {
                    ReportError ( "Database declared but not defined", type );
                }
            }
            else
            {
                ReportError ( "Not a database", type );
            }
        }
        SDBMemberWhack ( m, 0 );
    }
}

void
ASTBuilder :: HandleDbMemberTable ( SDatabase & p_db, const AST & p_member )
{
    assert ( p_member . ChildrenCount () == 3 );

    STblMember * m = Alloc < STblMember > ();
    if ( m != 0 )
    {
        if ( p_member . GetChild ( 0 ) -> GetTokenType () == KW_template )
        {
            m -> tmpl = true;
        }

        const AST_FQN & type = * ToFQN ( p_member . GetChild ( 1 ) );
        const KSymbol * tblName = Resolve ( type );
        if ( tblName != 0 )
        {
            if ( tblName -> type == eTable )
            {
                String memName;
                assert ( p_member . GetChild ( 2 ) -> GetChild ( 0 ) != 0 );
                StringInitCString ( & memName, p_member . GetChild ( 2 ) -> GetChild ( 0 ) -> GetTokenValue () );
                rc_t rc = KSymTableCreateConstSymbol ( & GetSymTab (), & m -> name, & memName, eDBMember, m );
                if ( rc == 0 )
                {
                    m -> tbl = static_cast < const STable * > ( SelectVersion ( type, * tblName, STableCmp ) );
                    if ( m -> tbl != 0 )
                    {
                        VectorAppend ( p_db . tbl, & m -> cid . id, m );
                        return;
                    }
                }
                else if ( GetRCState ( rc ) == rcExists )
                {
                    ReportError ( p_member . GetLocation (), "Member already exists", memName );
                }
                else
                {
                    ReportRc ( "KSymTableCreateConstSymbol", rc);
                }
            }
            else
            {
                ReportError ( "Not a table", type );
            }
        }
        STblMemberWhack ( m, 0 );
    }
}

void
ASTBuilder :: HandleDbBody ( SDatabase & p_db, const AST & p_body )
{
    rc_t rc = push_db_scope ( & GetSymTab (), & p_db );
    if ( rc == 0 )
    {
        rc = KSymTablePushScope ( & GetSymTab (), & p_db . scope );
        if ( rc == 0 )
        {
            uint32_t count = p_body . ChildrenCount ();
            for ( uint32_t i = 0 ; i < count; ++ i )
            {
                const AST & member = * p_body . GetChild ( i );
                switch ( member . GetTokenType () )
                {
                case PT_DBMEMBER:
                    HandleDbMemberDb ( p_db, member );
                    break;

                case PT_TBLMEMBER:
                    HandleDbMemberTable ( p_db, member );
                    break;

                case PT_EMPTY:
                    break;

                default:
                    assert ( false );
                }
            }
            KSymTablePopScope ( & GetSymTab () );
        }
        else
        {
            ReportRc ( "KSymTablePushScope", rc);
        }

        pop_db_scope ( & GetSymTab (), & p_db );
    }
    else
    {
        ReportRc ( "push_db_scope", rc);
    }
}


AST *
ASTBuilder :: DatabaseDef ( const Token * p_token, AST_FQN * p_fqn, AST * p_parent, AST * p_body )
{
    AST * ret = new AST ( p_token, p_fqn, p_parent, p_body );

    SDatabase * db = Alloc < SDatabase > ();
    if ( db != 0 )
    {
        /* prepare vectors */
        VectorInit ( & db -> db, 0, 8 );
        VectorInit ( & db -> tbl, 0, 8 );

        /* version */
        db -> version = p_fqn -> GetVersion ();
        const KSymbol * priorDecl = Resolve ( * p_fqn, false );
        if ( priorDecl == 0 )
        {
            db -> name = CreateFqnSymbol ( * p_fqn, eDatabase, db );
            if ( db -> name == 0 ||
                 ! CreateOverload ( db -> name,
                                    db,
                                    SDatabaseSort,
                                    m_schema -> db,
                                    m_schema -> dname,
                                    & db -> id ) )
            {
                SDatabaseWhack ( db, 0 );
                return ret;
            }
        }
        else
        {
            if ( ! HandleDbOverload ( db, db -> version, priorDecl, & db -> id ) )
            {
                SDatabaseWhack ( db, 0 );
                return ret;
            }

            // declared previously, this declaration not ignored
            db -> name = priorDecl;
        }

        /* look for inheritance */
        if ( p_parent -> GetTokenType () != PT_EMPTY )
        {
            const AST_FQN & parent = * ToFQN ( p_parent );
            const KSymbol * parentDecl = Resolve ( parent, true );
            if ( parentDecl -> type != eDatabase )
            {
                ReportError ( "Not a database", parent );
                return ret;
            }

            const SDatabase * dad = static_cast < const SDatabase * > ( SelectVersion ( parent, * parentDecl, SDatabaseCmp ) );
            if ( dad != 0 )
            {
                rc_t rc = SDatabaseExtend ( db, dad );
                if ( rc != 0 )
                {
                    ReportRc ( "SDatabaseExtend", rc );
                    return ret;
                }
            }
        }

        HandleDbBody ( * db, * p_body );
    }

    return ret;
}

