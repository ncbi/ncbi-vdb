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
#include "../vdb/schema-expr.h"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast-tokens.h"

using namespace std;

class ViewDeclaration // Wrapper around SView
{
public:
    ViewDeclaration ( ASTBuilder & p_builder );
    ~ViewDeclaration ();

    bool SetName ( const AST_FQN &  p_fqn );
    void HandleParameters ( const AST & p_params );
    void HandleParents ( const AST & p_parents );
    void HandleBody ( const AST & p_body );

    BSTree * Scope () { return m_self == 0 ? 0 : & m_self -> scope; }

private:
    bool HandleOverload ( const AST_FQN & p_fqn, const KSymbol * p_priorDecl );

    ASTBuilder &    m_builder;
    SView *         m_self;
};

ViewDeclaration :: ViewDeclaration ( ASTBuilder & p_builder )
:   m_builder ( p_builder ),
    m_self ( m_builder . Alloc < SView > () )
{
    //TODO: prepare vectors
}

ViewDeclaration :: ~ViewDeclaration ()
{
    // m_self gets added to the schema or freed on error, no need to free here
}

bool
ViewDeclaration :: SetName ( const AST_FQN &  p_fqn )
{
    assert ( m_self != 0 );

    m_self -> version = p_fqn . GetVersion ();

    const KSymbol * priorDecl = m_builder . Resolve ( p_fqn, false );
    if ( priorDecl == 0 )
    {
        m_self -> name = m_builder . CreateFqnSymbol ( p_fqn, eView, m_self );
        if ( m_self -> name != 0 &&
                m_builder . CreateOverload ( m_self -> name,
                                            m_self,
                                            SViewSort,
                                            m_builder . GetSchema () -> view,
                                            m_builder . GetSchema () -> vname,
                                            & m_self -> id ) )
        {
            return true;
        }
    }
    else if ( HandleOverload ( p_fqn, priorDecl ) )
    {   // declared previously, this declaration not ignored
        m_self -> name = priorDecl;
        return true;
    }

    SViewWhack ( m_self, 0 );
    m_self = 0;
    return false;
}

bool
ViewDeclaration :: HandleOverload ( const AST_FQN & p_fqn, const KSymbol * p_priorDecl )
{
    assert ( p_priorDecl != 0 );

    Vector & views = m_builder . GetSchema () -> view;

    SNameOverload *name = ( SNameOverload* ) p_priorDecl -> u . obj;
    assert ( name != 0 );
    uint32_t idx;
    rc_t rc = VectorInsertUnique ( & name -> items, m_self, & idx, SViewSort );
    if ( rc == 0 ) // overload added
    {
        return m_builder . VectorAppend ( views, & m_self -> id, m_self );
    }
    if ( GetRCState ( rc ) == rcExists )
    {   /* an overload with the same major version exists */
        /* see if new function trumps old */
        SView *exist = static_cast < SView * > ( VectorGet ( & name -> items, idx ) );
        if ( m_self -> version == exist -> version )
        {
            m_builder . ReportError ( "This version is already declared", p_fqn );
        }
        else if ( m_self -> version > exist -> version )
        {
            /* insert our function in name overload */
            void * prior;
            VectorSwap ( & name -> items, idx, m_self, & prior );

            /* if existing is in the same schema... */
            if ( ( const void* ) name == exist -> name -> u . obj )
            {
                /* need to swap with old */
                assert ( exist -> id >= VectorStart ( & views ) );
                VectorSwap ( & views, exist -> id, m_self, & prior );
                m_self -> id = exist -> id;
                SViewWhack ( ( SView * ) prior, 0 );
            }
            return true;
        }
    }
    else if ( rc != 0 )
    {
        m_builder . ReportRc ( "VectorInsertUnique", rc );
    }
    return false;
}

void
ViewDeclaration :: HandleParameters ( const AST & p_params )
{
    uint32_t count = p_params . ChildrenCount ();
    for ( uint32_t i = 0; i < count; ++i )
    {
        const AST & p = * p_sig . GetChild ( i );
        assert ( p . GetTokenType () == PT_VIEWPARAM );
        assert ( p . ChildrenCount () == 2 );
        // child [ 0 ]: PT_TABLE or PT_VIEW
        // child [ 1 ]: ident
    }
}

void
ViewDeclaration :: HandleParents ( const AST & p_parents )
{
}

void
ViewDeclaration :: HandleBody ( const AST & p_body )
{
}

AST *
ASTBuilder :: ViewDef ( const Token * p_token, AST_FQN * p_name, AST * p_params, AST * p_parents, AST * p_body )
{
    AST * ret = new AST ( p_token, p_name, p_params, p_parents, p_body );

    ViewDeclaration view ( * this );
    assert ( p_name != 0 );
    if ( view . SetName ( * p_name ) )
    {
        rc_t rc = KSymTablePushScope ( & m_symtab, & view . Scope () ); //TODO: add parents
        if ( rc == 0 )
        {
            assert ( p_params != 0 );
            view. HandleParameters ( * p_params );
            assert ( p_parents != 0 );
            view. HandleParents ( * p_parents );
            assert ( p_body != 0 );
            view. HandleBody ( * p_body );

            KSymTablePopScope ( tbl );
        }
        else
        {
            m_builder . ReportRc ( "VectorInsertUnique", rc );
        }
    }
    return ret;
}
