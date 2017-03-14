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
bool
ASTBuilder :: HandleTableOverload ( const STable *     p_table,
                                       uint32_t         p_version,
                                       const KSymbol *  p_priorDecl,
                                       uint32_t *       p_id )
{
    assert ( p_table != 0 );
    assert ( p_priorDecl != 0 );
    assert ( p_id != 0 );

    Vector & tables = GetSchema () -> tbl;

    SNameOverload *name = ( SNameOverload* ) p_priorDecl -> u . obj;
    assert ( name != 0 );

    uint32_t idx;
    rc_t rc = VectorInsertUnique ( & name -> items, p_table, & idx, STableSort );
    if ( rc == 0 ) // overload added
    {
        return VectorAppend ( tables, p_id, p_table );
    }
    if ( GetRCState ( rc ) == rcExists )
    {   /* an overload with the same major version exists */
        /* see if new table trumps old */
        const STable *exist = static_cast < const STable * > ( VectorGet ( & name -> items, idx ) );
        const STable *newer;
        rc = STableCompare ( exist, p_table, & newer, false );
        if ( rc == 0 && newer == p_table )
        {
            /* put the new one in place of the existing */
            void * ignore;
            VectorSwap ( & name -> items, idx, p_table, & ignore );
            * p_id = exist -> id;

            if ( VectorAppend ( tables, p_id, p_table ) )
            {   // tell everyone to use new table
                rc = schema_update_tbl_ref ( m_schema, exist, p_table );
                if ( rc == 0 )
                {
                    return true;
                }
                ReportError ( "schema_update_tbl_ref", rc);
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
ASTBuilder :: AddProduction ( Vector & p_list, const AST & p_prod )
{
    assert ( p_prod . ChildrenCount () == 3 ); // type-expr, ident, cond-expr
    SProduction * prod = Alloc < SProduction > ();
    if ( prod != 0 )
    {
        STypeExpr * type = MakeTypeExpr ( * p_prod . GetChild ( 0 ) );
        if ( type != 0 )
        {
            prod -> fd = & type -> dad;
        }

        const AST& ident = * p_prod . GetChild ( 1 );
        assert ( ident . ChildrenCount () == 1 );
        prod -> name = CreateLocalSymbol ( ident . GetChild ( 0 ) -> GetTokenValue (), eProduction, prod );

        const AST_Expr & expr = * dynamic_cast < const AST_Expr * > ( p_prod . GetChild ( 2 ) ) ;
        prod -> expr = expr . MakeExpression ( *this ) ;

        if ( ! VectorAppend ( p_list, & prod -> cid . id, prod ) )
        {
            SProductionWhack ( prod, NULL );
        }
    }
}

bool
ASTBuilder :: HandleSimpleColumn ( STable & p_table, SColumn & c, const AST & typedCol )
{
    assert ( typedCol . ChildrenCount () == 1 );
    assert ( typedCol . GetChild ( 0 ) -> GetTokenType () == PT_IDENT );
    const char * ident = typedCol . GetChild ( 0 ) -> GetChild ( 0 ) -> GetTokenValue ();

    if ( c . read_only )
    {
        ReportError ( "Simple column cannot be readonly: '%s'", ident);
        return false;
    }
    c . simple = true;

    String name;
    StringInitCString ( & name, ident );
    if ( KSymTableFindShallow ( & GetSymTab (), & name ) != 0 ||
            KSymTableFindIntrinsic ( & GetSymTab (), & name ) )
    {
        ReportError ( "Name already in use: '%s'", ident );
        return false;
    }
    else
    {
        rc_t rc = KSymTableCreateConstSymbol ( & GetSymTab (), & c . name, & name, eColumn, 0 );
        if ( rc != 0 )
        {
            ReportRc ( "KSymTableCreateConstSymbol", rc );
            return false;
        }
    }

    // add column name to p_table . cname
    SNameOverload *ovl;
    rc_t rc = SNameOverloadMake ( & ovl, c . name, 0, 4 );
    if ( rc == 0 )
    {
        ovl -> cid . ctx = -1;
        if ( ! VectorAppend ( p_table . cname, & ovl -> cid . id, ovl ) )
        {
            SNameOverloadWhack ( ovl, 0 );
            ReportRc ( "SNameOverloadWhack", rc);
            return false;
        }
        rc = VectorInsertUnique ( & ovl -> items, & c, 0, SColumnSort );
        if ( rc != 0 )
        {
           ReportRc ( "VectorInsertUnique", rc);
           return false;
        }
    }

    // generate and attach the corresponding physical column
    String physname;
    char physnamebuff [ 256 ];
    /* tack a dot onto the beginning and look up the symbol */
    physnamebuff [ 0 ] = '.';
    memcpy ( & physnamebuff [ 1 ], c . name -> name . addr, c . name -> name . size );
    StringInit ( & physname, physnamebuff, c . name -> name . size + 1, c . name -> name . len + 1 );
    KSymbol * sym = KSymTableFind ( & m_symtab, & physname );

    /* if the symbol exists, then this CANNOT be a simple column */
    if ( sym != 0 && ! ( sym -> type == eForward || sym -> type == eVirtual ) )
    {
        /* check for explicit physical type */
        if ( c . ptype != 0 )
        {
            ReportError ( "Implicit physical column previously declared: '%S'", & name );
            return false;
        }
        else
        {
            ReportError ( "Missing column read or validate expression: '%S'", & name );
            return false;
        }
    }
    else if ( ( c . td . type_id & 0xC0000000 ) != 0 )
    {
        ReportError ( "Simple columns cannot have typeset as type: '%S'", & name );
        return false;
    }
    else
    {
        if ( sym != 0 )
        {
            sym -> type = ePhysMember;
        }
        else
        {
            sym = CreateLocalSymbol ( physname, ePhysMember, 0 );
        }
        if ( sym != 0 )
        {
            rc = implicit_physical_member ( & m_symtab, 0, & p_table, & c, sym );
            if ( rc != 0 )
            {
                ReportRc ( "implicit_physical_member", rc);
                return false;
            }
        }
    }
    return true;
}

SExpression *
ASTBuilder :: MakePhysicalEncodingSpec ( const AST & p_node, VTypedecl & p_type )
{
    assert ( p_node . ChildrenCount () == 3 ); // schema_parms fqn_opt_vers factory_parms_opt
    SPhysEncExpr * ret = Alloc < SPhysEncExpr > ();
    if ( ret != 0 )
    {
        /* initialize */
        ret -> dad . var = ePhysEncExpr;
        atomic32_set ( & ret -> dad . refcount, 1 );
        ret -> phys = NULL;
        VectorInit ( & ret -> schem, 0, 4 );
        VectorInit ( & ret -> pfact, 0, 8 );

        FillSchemaParms ( * p_node . GetChild ( 0 ), ret -> schem );

        // resolve fqn_opt_vers, has to be ePhysical, capture requested version
        assert ( p_node . GetChild ( 1 ) -> GetTokenType () == PT_IDENT );
        const AST_FQN & fqn = * dynamic_cast < const AST_FQN * > ( p_node . GetChild ( 1 ) );
        const KSymbol * sym = Resolve ( fqn );

        const SNameOverload *name = static_cast < const SNameOverload * > ( sym -> u . obj );
        ret -> version = fqn . GetVersion ();
        if ( ret -> version != 0 )
        {
            ret -> phys = static_cast < const SPhysical * > ( VectorFind ( & name -> items, & ret -> version, NULL, SPhysicalCmp ) );
            ret -> version_requested = true;
        }
        else
        {
            ret -> phys = static_cast < const SPhysical * > ( VectorLast ( & name -> items ) );
            ret -> version_requested = false;
        }

        /* evaluate type expression */
        if ( ret -> phys == 0 )
        {
            ReportError ( "Requested version does not exist: '%S#%V'", & sym -> name, ret -> version );
        }
        else
        {
            rc_t rc;
            {
                /* bind schema parameters */
                Vector prior, cx_bind;
                VectorInit ( & cx_bind, 1, GetSchema () -> num_indirect ? GetSchema () -> num_indirect : 16 );
                rc = SPhysicalBindSchemaParms ( ret -> phys, & prior, & ret -> schem, & cx_bind );
                if ( rc == 0 )
                {
                    const SExpression *tx = ret -> phys -> td;
                    assert ( tx != NULL );
                    rc = STypeExprResolveAsTypedecl ( ( const STypeExpr* ) tx, GetSchema (), & p_type, & cx_bind );
                    SPhysicalRestSchemaParms ( ret -> phys, & prior, & cx_bind );
                }
                VectorWhack ( & cx_bind, NULL, NULL );
            }

            if ( rc == 0 )
            {    // populate ret -> pfact
                const AST & fact = * p_node . GetChild ( 2 );
                uint32_t count = fact . ChildrenCount ();
                for ( uint32_t i = 0; i < count; ++ i )
                {
                    const AST_Expr & expr = * static_cast < const AST_Expr * > ( fact . GetChild ( i ) );
                    if ( ! VectorAppend ( ret -> pfact, 0, expr . MakeExpression ( * this ) ) )
                    {
                        SExpressionWhack ( & ret -> dad );
                        return 0;
                    }
                }
                return & ret -> dad;
            }
        }
        SExpressionWhack ( & ret -> dad );
    }
    return 0;
}


void
ASTBuilder :: AddColumn ( STable & p_table, const AST & p_modifiers, const AST & p_decl, const AST * p_default )
{
    assert ( p_decl . GetTokenType () == PT_COLDECL );
    assert ( p_decl . ChildrenCount () == 2 );  // typespec typed_col

    SColumn * c = Alloc < SColumn > ();
    if ( c != 0 )
    {
        // process modifiers
        uint32_t modCount = p_modifiers . ChildrenCount ();
        for ( uint32_t i = 0 ; i < modCount; ++i )
        {
            switch ( p_modifiers . GetChild ( i ) -> GetTokenType () )
            {
            case KW_default:
                c -> dflt = true;
                break;
            case KW_extern:
                break;
            case KW_readonly:
                c -> read_only = true;
                break;
            default:
                assert ( false );
            }
        }

        if ( p_decl . GetChild ( 0 ) -> GetTokenType () == PT_PHYSENCREF )
        {
            c -> ptype = MakePhysicalEncodingSpec ( * p_decl . GetChild ( 0 ), c -> td  );
        }
        else // p_decl . GetChild ( 0 ) represents a type
        {
            TypeSpec ( * p_decl . GetChild ( 0 ), c -> td );
        }

        const AST & typedCol = * p_decl . GetChild ( 1 );
        if ( typedCol . ChildrenCount () == 1 )
        {
            if ( ! HandleSimpleColumn ( p_table, * c, typedCol ) )
            {
                SColumnWhack ( c, 0 );
                return;
            }
        }
        else
        {
            //TODO: process other kinds of typed_col
            //assert ( false );
        }
        if ( VectorAppend ( p_table . col, & c -> cid . id, c ) )
        {
            return;
        }
        SColumnWhack ( c, 0 );
    }
}

void
ASTBuilder :: HandleTableParents ( STable & p_table, const AST & p_parents )
{
    uint32_t parentCount = p_parents . ChildrenCount ();
    for ( uint32_t i = 0 ; i < parentCount; ++i )
    {
        const AST_FQN * parent = dynamic_cast < const AST_FQN * > ( p_parents . GetChild ( i ) );
        if ( parent != 0 )
        {
            const KSymbol * parentDecl = Resolve ( * parent, true );
            if ( parentDecl != 0 )
            {
                uint32_t vers = parent -> GetVersion ();
                const STable * dad;
                const SNameOverload * name = static_cast < const SNameOverload * > ( parentDecl -> u . obj );
                if ( vers == 0 )
                {
                    dad = static_cast < const STable * > ( VectorLast ( & name -> items ) );
                }
                else
                {
                    dad = static_cast < const STable * > ( VectorFind ( & name -> items, & vers, NULL, STableCmp ) );
                }
                if ( dad != 0 )
                {
                    rc_t rc = STableExtend ( & m_symtab, & p_table, dad );
                    if ( rc != 0 )
                    {
                        ReportRc ( "STableExtend", rc );
                        break;
                    }
                }
                else
                {
                    ReportError ( "Parent table not found", * parent );
                    break;
                }
            }
        }
    }
}

void
ASTBuilder :: HandleTableBody ( STable & p_table, const AST & p_body )
{
    rc_t rc = KSymTablePushScope ( & GetSymTab (), & p_table . scope );
    if ( rc == 0 )
    {
        uint32_t count = p_body . ChildrenCount ();
        for ( uint32_t i = 0 ; i < count; ++ i )
        {
            const AST & stmt = * p_body . GetChild ( i );
            switch ( stmt . GetTokenType () )
            {
            case PT_PRODSTMT:
                AddProduction ( p_table . prod, stmt );
                break;
            case PT_PRODTRIGGER:
                assert ( false );
                break;
            case PT_COLUMN:
                // modifiers col_decl [ default ]
                AddColumn ( p_table, * stmt . GetChild ( 0 ), * stmt . GetChild ( 1 ), stmt . GetChild ( 2 ) );
                break;
            case PT_COLUMNEXPR:
                //TODO: process modifiers
                assert ( false );
                break;
            default:
                assert ( false );
            }
        }

        STableScanData pb;
        pb . self = & p_table;
        pb . rc = 0;

        /* scan table scope for unresolved forward references */
        if ( BSTreeDoUntil ( & p_table . scope, false, table_fwd_scan, & pb ) )
        {
            ReportRc ( "table_fwd_scan", pb . rc );
        }

        KSymTablePopScope ( & GetSymTab () );
    }
}

AST *
ASTBuilder ::  TableDef ( const Token * p_token, AST_FQN * p_fqn, AST * p_parents, AST * p_body )
{
    AST * ret = new AST ( p_token, p_fqn, p_parents, p_body );

    STable * table = Alloc < STable > ();
    if ( table != 0 )
    {
        /* prepare vectors */
        VectorInit ( & table -> parents, 0, 4 );
        VectorInit ( & table -> overrides, 0, 4 );
        VectorInit ( & table -> col, 0, 16 );
        VectorInit ( & table -> cname, 0, 16 );
        VectorInit ( & table -> phys, 0, 16 );
        VectorInit ( & table -> prod, 0, 64 );
        VectorInit ( & table -> vprods, 1, 16 );
        VectorInit ( & table -> syms, 1, 32 );

        table -> version = p_fqn -> GetVersion ();

        const KSymbol * priorDecl = Resolve ( * p_fqn, false );
        if ( priorDecl == 0 )
        {
            table -> name = CreateOverload ( * p_fqn, table, eTable, STableSort, m_schema -> tbl, m_schema -> tname, & table -> id );
            if ( table -> name == 0 )
            {
                STableWhack ( table, 0 );
                return ret;
            }
        }
        else
        {
            if ( ! HandleTableOverload ( table, table -> version, priorDecl, & table -> id ) )
            {
                STableWhack ( table, 0 );
                return ret;
            }
            // declared previously, this declaration not ignored
            table -> name = priorDecl;
        }

        HandleTableParents ( * table, * p_parents );
        HandleTableBody ( * table, * p_body );

        //TODO: set table id on all members (see table_set_context())
    }

    return ret;
}

