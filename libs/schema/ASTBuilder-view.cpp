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

    const SView * GetSelf () const { return m_self; }

    bool SetName ( const AST_FQN &  p_fqn );
    void HandleParameters ( const AST & p_params );
    void HandleParents ( const AST & p_parents );
    void HandleBody ( const AST & p_body );

    BSTree * Scope () { return m_self == 0 ? 0 : & m_self -> scope; }

private:
    bool HandleOverload ( const AST_FQN & p_fqn, const KSymbol * p_priorDecl );
    void AddColumn ( const AST & p_type, const AST & p_ident, const AST_Expr & p_expr );
    bool Extend ( const Token :: Location & p_loc, const SView *dad, const AST & p_params );
    bool CheckForCollisions ( const SView & p_table, const String *& p_name );
    bool CopyColumnNames ( const SNameOverload *orig );
    bool ScanVirtuals ( const Token :: Location & p_loc, Vector & p_byParent );
    void HandleStatement ( const AST & p_stmt );
    bool AddNewColumn ( SColumn & p_col, String & p_name );
    bool InitParentInstance( SViewInstance * p_self, const SView * p_dad, const AST & p_params );

    ASTBuilder &    m_builder;
    SView *         m_self;
};

ViewDeclaration :: ViewDeclaration ( ASTBuilder & p_builder )
:   m_builder ( p_builder ),
    m_self ( m_builder . Alloc < SView > () )
{
    // prepare vectors
    VectorInit ( & m_self -> params, 0, 4 );
    VectorInit ( & m_self -> parents, 0, 4 );
    VectorInit ( & m_self -> overrides, 0, 4 );
    VectorInit ( & m_self -> col, 0, 16 );
    VectorInit ( & m_self -> cname, 0, 16 );
    VectorInit ( & m_self -> prod, 0, 64 );
    VectorInit ( & m_self -> vprods, 1, 16 );
    VectorInit ( & m_self -> syms, 1, 32 );
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
            const void * versioned = 0;
            switch ( sym -> type )
            {
            case eTable:
                versioned = m_builder . SelectVersion ( fqn, * sym, STableCmp );
                break;
            case eView:
                versioned = m_builder . SelectVersion ( fqn, * sym, SViewCmp );
                break;
            default:
                m_builder . ReportError ( "Cannot be used as a view parameter", fqn );
                break;
            }
            if ( versioned != 0 )
            {
                KSymbol * local = m_builder . CreateLocalSymbol ( nameNode, name, sym -> type, versioned );
                if ( local != 0 )
                {
                    m_builder . VectorAppend ( m_self -> params, 0, local );
                }
            }
        }
    }
}

bool
ViewDeclaration :: CheckForCollisions ( const SView & p_view, const String *& p_name )
{
    /* test column names */
    uint32_t start = VectorStart ( & p_view . cname );
    uint32_t count = VectorLength ( & p_view . cname );
    for ( uint32_t i = 0; i < count; ++ i )
    {
        const SNameOverload * no = static_cast < const SNameOverload * > ( VectorGet ( & p_view . cname, start + i ) );
        if  ( ! m_builder . CheckForColumnCollision ( no -> name ) )
        {
            p_name = & no -> name -> name;
            return false;
        }
    }

    /* test production names */
    start = VectorStart ( & p_view .prod );
    count = VectorLength ( & p_view . prod );
    for ( uint32_t i = 0; i < count; ++ i )
    {
        const SProduction * prod = static_cast < const SProduction * > ( VectorGet ( & p_view . prod, start + i ) );
        if  ( ! m_builder . CheckForColumnCollision ( prod -> name ) )
        {
            p_name = & prod -> name -> name;
            return false;
        }
    }

    return true;
}

bool
ViewDeclaration :: CopyColumnNames ( const SNameOverload *orig )
{
    rc_t rc;
    SView *self= m_self;
    SNameOverload *copy;
    const KSymbol *sym = ( const KSymbol* ) BSTreeFind ( & self -> scope, & orig -> name -> name, KSymbolCmp );
    if ( sym == NULL )
    {
        rc = SNameOverloadCopy ( & self -> scope, & copy, orig );
        if ( rc == 0 )
        {
            if ( ! m_builder . VectorAppend ( self -> cname, & copy -> cid . id, copy ) )
            {
                SNameOverloadWhack ( copy, NULL );
                return false;
            }
        }
        else
        {
            m_builder . ReportRc ( "SNameOverloadCopy", rc );
            return false;
        }
    }
    else
    {
        copy = ( SNameOverload* ) sym -> u . obj;
        assert ( copy -> cid . ctx == orig -> cid . ctx );
        rc = VectorMerge ( & copy -> items, true, & orig -> items, SColumnSort );
        if ( rc != 0 )
        {
            m_builder . ReportRc ( "VectorMerge", rc );
            return false;
        }
    }

    return true;
}

bool
ViewDeclaration :: InitParentInstance( SViewInstance * p_inst, const SView * p_parent, const AST & p_params )
{
    p_inst -> dad = p_parent;
    uint32_t count = p_params . ChildrenCount ();
    VectorInit ( & p_inst -> params, 0, count );
    if ( count != VectorLength ( & p_parent -> params ) )
    {
        m_builder . ReportError ( p_params . GetLocation(), "Wrong number of parameters for a view instantiation" );
        return false;
    }
    for ( uint32_t i = 0; i < count; ++i )
    {
        const AST_FQN & ident = * ToFQN ( p_params . GetChild ( i ) );
        const KSymbol * sym = m_builder . Resolve ( ident );
        if ( sym != 0 )
        {
            switch ( sym -> type )
            {
            case eTable:
            case eView:
                {   /* verify type */
                    const KSymbol * parentsParam = static_cast < const KSymbol * > ( VectorGet ( & p_parent -> params, i ) );
                    if ( parentsParam -> u . obj != sym -> u . obj )
                    {
                        m_builder . ReportError ( "Wrong type of a view's parameter", ident );
                        return false;
                    }
                    if ( ! m_builder . VectorAppend ( p_inst -> params, 0, sym ) )
                    {
                        return false;
                    }
                }
                break;
            default:
                m_builder . ReportError ( "A view's parameter has to be a table or a view", ident );
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

static
int64_t
CC SViewNameCmp ( const void * key, const void * n )
{
    const SView * a = ( const SView * ) key;
    const SViewInstance * b = (const SViewInstance *)n;
    return StringCompare ( & a -> name -> name, & b -> dad -> name -> name );
}

bool
ViewDeclaration :: Extend ( const Token :: Location & p_loc, const SView * p_dad, const AST & p_params )
{
    /* reject if direct parent already in the ancestry */
    if ( VectorFind ( & m_self -> parents, p_dad, NULL, SViewNameCmp ) != NULL ||
         VectorFind ( & m_self -> overrides, & p_dad -> id, NULL, SViewOverridesCmp ) != NULL )
    {
        m_builder . ReportError ( p_loc, "Same view inherited from more than once", p_dad -> name -> name );
        return false;
    }

    /* test for any collisions */
    rc_t rc = push_view_scope ( & m_builder . GetSymTab (), m_self );
    if ( rc != 0 )
    {
        m_builder . ReportRc ( "push_tbl_scope", rc );
        return false;
    }

    const String * name;
    if ( ! CheckForCollisions ( * p_dad, name ) )
    {
        m_builder . ReportError ( p_loc, "Duplicate symbol in parent view hierarchy", * name );
        pop_view_scope ( & m_builder . GetSymTab (), m_self );
        return false;
    }

    pop_view_scope ( & m_builder . GetSymTab (), m_self );

    {   /* add "dad" and grandparents to parent list */
        SViewInstance * parent = m_builder . Alloc < SViewInstance > ();
        if ( parent == 0 )
        {
            return false;
        }
        if ( ! InitParentInstance ( parent, p_dad, p_params ) ||
            ! m_builder . VectorAppend ( m_self -> parents, NULL, parent ) )
        {   
            SViewInstanceWhack ( parent, 0 );
            return false;
        }
    }

    { /* copy column names from parent - should already contain all grandparents */
        uint32_t start = VectorStart ( & p_dad -> cname );
        uint32_t count = VectorLength ( & p_dad -> cname );
        for ( uint32_t i = 0; i < count; ++ i )
        {
            const SNameOverload * ovl = static_cast < const SNameOverload * > ( VectorGet ( & p_dad -> cname, start + i ) );
            if ( ! CopyColumnNames ( ovl ) )
            {
                return false;
            }
        }
    }

    /* add "dad" to overrides */
    rc = SViewOverridesMake ( & m_self -> overrides, p_dad, & p_dad -> vprods );
    if ( rc == 0 )
    {
        /* add all grandparents */
        uint32_t start = VectorStart ( & p_dad -> overrides );
        uint32_t count = VectorLength ( & p_dad -> overrides );
        for ( uint32_t i = 0; i < count; ++ i )
        {
            const SViewOverrides * ovr = static_cast < const SViewOverrides * > ( VectorGet ( & p_dad -> overrides, start + i ) );
            rc = SViewOverridesMake ( & m_self -> overrides, ovr -> dad, & ovr -> by_parent );
            if ( rc != 0 && GetRCState ( rc ) != rcExists)
            {
                m_builder . ReportRc ( "STableOverridesMake", rc );
                return false;
            }
        }
    }
    else if ( GetRCState ( rc ) != rcExists )
    {
        m_builder . ReportRc ( "STableOverridesMake", rc );
        return false;
    }

    return true;
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
            const AST & viewParent = * parents . GetChild ( i );
            assert ( viewParent . GetTokenType () == PT_VIEWPARENT );
            assert ( viewParent . ChildrenCount () == 2 );
            // PT_VIEWPARENT: fqn_opt_vers view_parent_parms
            const AST_FQN & parent = * ToFQN ( viewParent . GetChild ( 0 ) );
            const KSymbol * parentDecl = m_builder . Resolve ( parent ); // will report unknown name
            if ( parentDecl != 0 )
            {
                if ( parentDecl -> type == eView )
                {
                    const SView * dad = static_cast < const SView * > ( m_builder . SelectVersion ( parent, * parentDecl, SViewCmp ) );
                    if ( dad != 0 )
                    {
                        Extend ( parent . GetLocation (), dad, * viewParent . GetChild ( 1 ) );
                    }
                }
                else
                {
                    m_builder . ReportError ( "A view's parent has to be a view", parent );
                }
            }
        }
    }
}

bool
ViewDeclaration :: AddNewColumn ( SColumn & p_col, String & p_name )
{   // new column: add p_col to m_self -> col, a new overload to m_self -> cname
    rc_t rc = KSymTableCreateConstSymbol ( & m_builder . GetSymTab (), & p_col . name, & p_name, eColumn, & p_col );
    if ( rc != 0 )
    {
        m_builder . ReportRc ( "KSymTableCreateConstSymbol", rc );
        return false;
    }
    return m_builder .CreateOverload ( p_col . name,
                                       & p_col,
                                       SColumnSort,
                                       m_self -> col,
                                       m_self -> cname,
                                       & p_col . cid . id );
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
        {
            if ( ! AddNewColumn ( * c, name ) )
            {
                SColumnWhack ( c, 0 );
                return;
            }
        }
        else // column name seen before
        {
            switch ( priorDecl -> type )
            {
            case eForward:
                {
                    c -> name = priorDecl;
                    priorDecl -> type = eColumn;
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
                break;
            case eColumn:
                {
                    SNameOverload * ovl = ( SNameOverload* ) priorDecl -> u . obj;
                    if ( VectorFind ( & ovl -> items, & c -> td, NULL, SColumnCmp ) != NULL )
                    {
                        m_builder . ReportError ( p_ident . GetLocation (), "Column already defined", ident );
                        SColumnWhack ( c, 0 );
                        return;
                    }
                    c -> name = priorDecl;
                    // add column to m_self -> col
                    if ( ! m_builder . VectorAppend ( m_self -> col, 0, c ) )
                    {
                        SColumnWhack ( c, 0 );
                        return;
                    }
                    // add a column overload
                    rc_t rc = VectorInsertUnique ( & ovl -> items, c, NULL, SColumnSort );
                    if ( rc != 0 )
                    {
                        m_builder . ReportRc ( "VectorInsertUnique", rc );
                        SColumnWhack ( c, 0 );
                        return;
                    }
                }
                break;
            case eVirtual:
                m_builder . ReportError ( p_ident . GetLocation (), "Virtual production defined as a column", ident );
                SColumnWhack ( c, 0 );
                return;
            default:
                /* allow names defined in scopes other than table and intrinsic */
                if ( KSymTableFindShallow ( & m_builder . GetSymTab (), & name ) != 0 ||
                    KSymTableFindIntrinsic ( & m_builder . GetSymTab (), & name ) )
                {
                    m_builder . ReportError ( p_ident . GetLocation (), "Column name already in use", ident );
                    SColumnWhack ( c, 0 );
                    return;
                }
                else if ( ! AddNewColumn ( * c, name ) )
                {
                    SColumnWhack ( c, 0 );
                    return;
                }
                break;
            }
        }

        // type checking/conversion is supposed to be done in the run time, like for v1 tables
        c -> read = p_expr . MakeExpression ( m_builder );
    }
}

void
ViewDeclaration :: HandleStatement ( const AST & p_stmt )
{
    switch ( p_stmt . GetTokenType () )
    {
    case PT_PRODSTMT:
        {   // typespec ident cond_expr
            assert ( p_stmt . ChildrenCount () == 3 );
            const AST * datatype = p_stmt . GetChild ( 0 );
            const AST * ident = p_stmt . GetChild ( 1 );
            const AST * expr = p_stmt . GetChild ( 2 );
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
            assert ( p_stmt . ChildrenCount () == 3 );
            AddColumn ( * p_stmt . GetChild ( 0 ), * p_stmt . GetChild ( 1 ), * ToExpr ( p_stmt . GetChild ( 2 ) ) );
        }
        break;

    case PT_EMPTY:
        break;

    default:
        assert ( false );
    }
}

void
ViewDeclaration :: HandleBody ( const AST & p_body )
{
    rc_t rc = push_view_scope ( & m_builder . GetSymTab (), m_self );
    if ( rc == 0 )
    {
        /* scan override views for virtual symbols */
        uint32_t start = VectorStart ( & m_self -> overrides );
        uint32_t count = VectorLength ( & m_self -> overrides );
        for ( uint32_t i = 0; i < count; ++ i )
        {
            SViewOverrides * ov = static_cast < SViewOverrides * > ( VectorGet ( & m_self -> overrides, start + i ) );
            if ( ! m_builder . ScanVirtuals ( p_body . GetLocation (), ov -> by_parent ) )
            {
                pop_view_scope ( & m_builder . GetSymTab (), m_self );
                return;
            }
        }

        /* handle view declarations */
        count = p_body . ChildrenCount ();
        for ( uint32_t i = 0 ; i < count; ++ i )
        {
            HandleStatement ( * p_body . GetChild ( i ) );
        }

        SViewScanData pb;
        pb . self = m_self;
        pb . rc = 0;

        /* scan view scope for unresolved forward references */
        if ( BSTreeDoUntil ( & m_self -> scope, false, view_fwd_scan, & pb ) )
        {
            m_builder . ReportRc ( "view_fwd_scan", pb . rc );
        }

        pop_view_scope ( & m_builder . GetSymTab (), m_self );

        /* fix forward references */
        if ( rc == 0 )
        {
            rc = view_fix_forward_refs ( m_self );
            if ( rc != 0 )
            {
                m_builder . ReportRc ( "table_fix_forward_refs", rc );
            }
        }

        view_set_context ( m_self, m_builder . NextContextId () );
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
        rc_t rc = KSymTablePushScope ( & m_symtab, view . Scope () );
        if ( rc == 0 )
        {
            assert ( p_params != 0 );
            view. HandleParameters ( * p_params );
            assert ( p_parents != 0 );
            view. HandleParents ( * p_parents );
            assert ( p_body != 0 );
            m_view = view . GetSelf ();
            view. HandleBody ( * p_body );
            m_view = 0;

            KSymTablePopScope ( & m_symtab );
        }
        else
        {
            ReportRc ( "KSymTablePushScope", rc );
        }
    }
    return ret;
}

