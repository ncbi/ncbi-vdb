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
    void AddColumn ( const AST & p_type, const AST & p_ident, const AST_Expr & p_expr );

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
        const AST & p = * p_params . GetChild ( i );
        assert ( p . GetTokenType () == PT_VIEWPARAM );
        assert ( p . ChildrenCount () == 2 );
        // child [ 0 ]: fqn_opt_vers, a table or a view
        // child [ 1 ]: ident, for the symtab
        const AST_FQN & fqn = * ToFQN ( p . GetChild ( 0 ) );
        const AST & nameNode = * p . GetChild ( 1 );
        assert ( nameNode . GetTokenType () == PT_IDENT );
        assert ( nameNode . ChildrenCount () == 1 );
        String name;
        StringInitCString ( & name, nameNode . GetChild ( 0 ) -> GetTokenValue () );
        const KSymbol * sym = m_builder . Resolve ( fqn ); // will report unknown name
        if ( sym != 0 )
        {
            switch ( sym -> type )
            {
            case eTable:
                {
                    const STable * versioned = static_cast < const STable * > ( m_builder . SelectVersion ( fqn, * sym, STableCmp ) ); // will report problems
                    if ( versioned != 0 )
                    {
                        KSymbol * sym = m_builder . CreateLocalSymbol ( nameNode, name, eTable, const_cast < STable * > ( versioned ) );
                        if ( sym != 0 )
                        {
                            m_builder . VectorAppend ( m_self -> tables, 0, sym );
                        }
                    }
                }
                break;
            case eView:
                {
                    const SView * versioned = static_cast < const SView * > ( m_builder . SelectVersion ( fqn, * sym, SViewCmp ) ); // will report problems
                    if ( versioned != 0 )
                    {
                        KSymbol * sym = m_builder . CreateLocalSymbol ( nameNode, name, eView, const_cast < SView * > ( versioned ) );
                        if ( sym != 0 )
                        {
                            m_builder . VectorAppend ( m_self -> views, 0, sym );
                        }
                    }
                }
                break;
            default:
                m_builder . ReportError ( "Cannot be used as a view parameter", fqn );
                break;
            }
        }
    }
}

void
ViewDeclaration :: HandleParents ( const AST & p_parents )
{
    if ( p_parents . GetTokenType () == PT_VIEWPARENTS )
    {
        assert ( p_parents . ChildrenCount () == 1 );
        const AST & parents = * p_parents . GetChild ( 0 );
        uint32_t count = parents . ChildrenCount ();
        for ( uint32_t i = 0; i < count; ++i )
        {
            const AST_FQN & parent = * ToFQN ( parents . GetChild ( i ) );
            const KSymbol * parentDecl = m_builder . Resolve ( parent ); // will report unknown name
            if ( parentDecl != 0 )
            {
                if ( parentDecl -> type != eView )
                {
                    m_builder . ReportError ( "A view's parent has to be a view", parent );
                    continue;
                }
                const SView * dad = static_cast < const SView * > ( m_builder . SelectVersion ( parent, * parentDecl, SViewCmp ) );
                if ( dad != 0 )
                {
                    rc_t rc = SViewExtend ( & m_builder . GetSymTab (), m_self, dad );
                    if ( rc != 0 )
                    {
                        if ( GetRCObject(rc) == (enum RCObject)rcTable && GetRCState(rc) == rcExists )
                        {
                            m_builder . ReportError ( "Same view inherited from more than once", parent );
                        } //TODO: check for (rcName, rcExists)
                        else
                        {
                            m_builder . ReportRc ( "SViewExtend", rc );
                            break;
                        }
                    }
                }
            }
        }
    }

}

void
ViewDeclaration :: AddColumn ( const AST & p_type, const AST & p_ident, const AST_Expr & p_expr )
{
    assert ( p_ident . GetTokenType () == PT_IDENT );

    SColumn * c = m_builder . Alloc < SColumn > ();
    if ( c != 0 )
    {
        c -> read_only = true;

        m_builder . TypeSpec ( p_type, c -> td );

        const char * ident = p_ident . GetChild ( 0 ) -> GetTokenValue ();
        String name;
        StringInitCString ( & name, ident );
        KSymbol * priorDecl = const_cast < KSymbol * > ( m_builder . Resolve ( p_ident . GetLocation (), ident, false ) );
        if ( priorDecl == 0 )
        {   // new column: add c to m_self -> col, a new overload to m_self -> cname
            rc_t rc = KSymTableCreateConstSymbol ( & m_builder . GetSymTab (), & c -> name, & name, eColumn, & c );
            if ( rc != 0 )
            {
                m_builder . ReportRc ( "KSymTableCreateConstSymbol", rc );
                SColumnWhack ( c, 0 );
                return;
            }
            if ( ! m_builder .CreateOverload ( c -> name,
                                               c,
                                               SColumnSort,
                                               m_self -> col,
                                               m_self -> cname,
                                               0 ) )
            {
                SColumnWhack ( c, 0 );
                return;
            }
        }
        else // column name seen before
        {
            m_builder . ReportError ( p_ident . GetLocation (), "Object already defined", ident );
            SColumnWhack ( c, 0 );
            return;
        }

        // type checking/conversion is supposed to be done in the run time, like for v1 tables
        c -> read = p_expr . MakeExpression ( m_builder );
    }
}

void
ViewDeclaration :: HandleBody ( const AST & p_body )
{
    rc_t rc = push_view_scope ( & m_builder . GetSymTab (), m_self );
    if ( rc == 0 )
    {
#if NOT_SURE_IF_NEEDED
        /* scan override tables for virtual symbols */
        if ( VectorDoUntil ( & m_self -> overrides, false, SViewScanVirtuals, & m_builder . GetSymTab () ) == 0 )
        {
#endif
            /* handle table declarations */
            uint32_t count = p_body . ChildrenCount ();
            for ( uint32_t i = 0 ; i < count; ++ i )
            {
                const AST & stmt = * p_body . GetChild ( i );
                switch ( stmt . GetTokenType () )
                {
                case PT_PRODSTMT:
                    {   // typespec ident cond_expr
                        assert ( stmt . ChildrenCount () == 3 );
                        const AST * datatype = stmt . GetChild ( 0 );
                        const AST * ident = stmt . GetChild ( 1 );
                        const AST * expr = stmt . GetChild ( 2 );
                        assert ( ident -> ChildrenCount () == 1 );
                        m_builder . AddProduction ( * ident,
                                                    m_self -> prod,
                                                    ident -> GetChild ( 0 ) -> GetTokenValue (),
                                                    * ToExpr ( expr ),
                                                    datatype );
                    }
                    break;
                case PT_COLUMN:
                    {   // typespec ident cond_expr
                        assert ( stmt . ChildrenCount () == 3 );
                        AddColumn ( * stmt . GetChild ( 0 ), * stmt . GetChild ( 1 ), * ToExpr ( stmt . GetChild ( 2 ) ) );
                    }
                    break;

                case PT_EMPTY:
                    break;

                default:
                    assert ( false );
                }
            }
#if NOT_SURE_IF_NEEDED
            STableScanData pb;
            pb . self = m_self;
            pb . rc = 0;

            /* scan table scope for unresolved forward references */
            if ( BSTreeDoUntil ( & m_self -> scope, false, table_fwd_scan, & pb ) )
            {
                m_builder . ReportRc ( "table_fwd_scan", pb . rc );
            }
        }
        else
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
            m_builder . ReportRc ( "STableViewVirtuals", rc );
        }
#endif

        pop_view_scope ( & m_builder . GetSymTab (), m_self );

#if NOT_SURE_IF_NEEDED
        /* fix forward references */
        if ( rc == 0 )
        {
            rc = table_fix_forward_refs ( m_self );
            if ( rc != 0 )
            {
                m_builder . ReportRc ( "table_fix_forward_refs", rc );
            }
        }

        table_set_context ( m_self );
#endif
    }
    else
    {
        m_builder . ReportRc ( "push_view_scope", rc );
    }
}

AST *
ASTBuilder :: ViewDef ( const Token * p_token, AST_FQN * p_name, AST * p_params, AST * p_parents, AST * p_body )
{
    AST * ret = new AST ( p_token, p_name, p_params, p_parents, p_body );

    ViewDeclaration view ( * this );
    assert ( p_name != 0 );
    if ( view . SetName ( * p_name ) )
    {
        rc_t rc = KSymTablePushScope ( & m_symtab, view . Scope () ); //TODO: add parents
        if ( rc == 0 )
        {
            assert ( p_params != 0 );
            view. HandleParameters ( * p_params );
            assert ( p_parents != 0 );
            view. HandleParents ( * p_parents );
            assert ( p_body != 0 );
            view. HandleBody ( * p_body );

            KSymTablePopScope ( & m_symtab );
        }
        else
        {
            ReportRc ( "KSymTablePushScope", rc );
        }
    }
    return ret;
}
