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
ASTBuilder :: HandleTableOverload ( const STable *   p_table,
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
ASTBuilder :: AddProduction ( Vector & p_list, const char * p_name, const AST_Expr & p_expr, const AST * p_type )
{
    SProduction * prod = Alloc < SProduction > ();
    if ( prod != 0 )
    {
        if ( p_type != 0 )
        {
            STypeExpr * type = MakeTypeExpr ( * p_type );
            if ( type != 0 )
            {
                prod -> fd = & type -> dad;
            }
        }
        else
        {
            prod -> trigger = true;
        }

        prod -> name = CreateLocalSymbol ( p_name, eProduction, prod );
        prod -> expr = p_expr . MakeExpression ( *this ) ;

        if ( ! VectorAppend ( p_list, & prod -> cid . id, prod ) )
        {
            SProductionWhack ( prod, NULL );
        }
    }
}

bool
ASTBuilder :: HandleTypedColumn ( STable & p_table, SColumn & p_col, const AST & p_typedCol )
{
    assert ( p_typedCol . ChildrenCount () >= 1 );
    assert ( p_typedCol . GetChild ( 0 ) -> GetTokenType () == PT_IDENT );
    const char * ident = p_typedCol . GetChild ( 0 ) -> GetChild ( 0 ) -> GetTokenValue ();
    String name;
    StringInitCString ( & name, ident );
    if ( KSymTableFindShallow ( & GetSymTab (), & name ) != 0 ||
            KSymTableFindIntrinsic ( & GetSymTab (), & name ) )
    {
        ReportError ( "Name already in use", ident );
        return false;
    }
    else
    {
        rc_t rc = KSymTableCreateConstSymbol ( & GetSymTab (), & p_col . name, & name, eColumn, 0 );
        if ( rc != 0 )
        {
            ReportRc ( "KSymTableCreateConstSymbol", rc );
            return false;
        }
    }

    // add column name to p_table . cname
    SNameOverload *ovl;
    rc_t rc = SNameOverloadMake ( & ovl, p_col . name, 0, 4 );
    if ( rc == 0 )
    {
        ovl -> cid . ctx = -1;
        if ( ! VectorAppend ( p_table . cname, & ovl -> cid . id, ovl ) )
        {
            SNameOverloadWhack ( ovl, 0 );
            ReportRc ( "SNameOverloadWhack", rc);
            return false;
        }
        rc = VectorInsertUnique ( & ovl -> items, & p_col, 0, SColumnSort );
        if ( rc != 0 )
        {
           ReportRc ( "VectorInsertUnique", rc);
           return false;
        }
    }

    if ( p_col . simple )
    {
        if ( p_col . read_only )
        {
            ReportError ( "Simple column cannot be readonly", ident);
            return false;
        }

        // generate and attach the corresponding physical column
        String physname;
        char physnamebuff [ 256 ];
        /* tack a dot onto the beginning and look up the symbol */
        physnamebuff [ 0 ] = '.';
        memcpy ( & physnamebuff [ 1 ], p_col . name -> name . addr, p_col . name -> name . size );
        StringInit ( & physname, physnamebuff, p_col . name -> name . size + 1, p_col . name -> name . len + 1 );
        KSymbol * sym = KSymTableFind ( & m_symtab, & physname );
        /* if the symbol exists, then this CANNOT be a simple column */
        if ( sym != 0 && ! ( sym -> type == eForward || sym -> type == eVirtual ) )
        {
            /* check for explicit physical type */
            if ( p_col . ptype != 0 )
            {
                ReportError ( "Implicit physical column previously declared", name );
                return false;
            }
            else
            {
                ReportError ( "Missing column read or validate expression", name );
                return false;
            }
        }
        else if ( ( p_col . td . type_id & 0xC0000000 ) != 0 )
        {
            ReportError ( "Simple columns cannot have typeset as type", name );
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
                rc = implicit_physical_member ( & m_symtab, 0, & p_table, & p_col, sym );
                if ( rc != 0 )
                {
                    ReportRc ( "implicit_physical_member", rc);
                    return false;
                }
            }
        }
    }

    return true;
}

SExpression *
ASTBuilder :: MakePhysicalEncodingSpec ( const KSymbol & p_sym,
                                         const AST_FQN & p_fqn,
                                         const AST * p_schemaArgs,
                                         const AST * p_factoryArgs,
                                         VTypedecl & p_type )
{
    SPhysEncExpr * ret = Alloc < SPhysEncExpr > ();
    if ( ret != 0 )
    {
        /* initialize */
        ret -> dad . var = ePhysEncExpr;
        atomic32_set ( & ret -> dad . refcount, 1 );
        ret -> phys = NULL;
        VectorInit ( & ret -> schem, 0, 4 );
        VectorInit ( & ret -> pfact, 0, 8 );

        if ( p_schemaArgs != 0 && ! FillSchemaParms ( * p_schemaArgs, ret -> schem ) )
        {
            SExpressionWhack ( & ret -> dad );
            return 0;
        }

        // capture requested version
        assert ( p_sym . type == ePhysical );
        ret -> version = p_fqn . GetVersion ();
        ret -> version_requested = ret -> version != 0;
        ret -> phys = static_cast < const SPhysical * > ( SelectVersion ( p_sym, SPhysicalCmp, & ret -> version ) );

        /* evaluate type expression */
        if ( ret -> phys != 0 )
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
            {
                if ( p_factoryArgs != 0 )
                {    // populate ret -> pfact
                    uint32_t count = p_factoryArgs -> ChildrenCount ();
                    for ( uint32_t i = 0; i < count; ++ i )
                    {
                        const AST_Expr & expr = * static_cast < const AST_Expr * > ( p_factoryArgs -> GetChild ( i ) );
                        if ( ! VectorAppend ( ret -> pfact, 0, expr . MakeExpression ( * this ) ) )
                        {
                            SExpressionWhack ( & ret -> dad );
                            return 0;
                        }
                    }
                }
                return & ret -> dad;
            }
        }
        SExpressionWhack ( & ret -> dad );
    }
    return 0;
}

static
void
HandleColumnModifiers ( const AST & p_modifiers, bool & p_default, bool & p_readonly )
{
    p_default = false;
    p_readonly = false;
    uint32_t modCount = p_modifiers . ChildrenCount ();
    for ( uint32_t i = 0 ; i < modCount; ++i )
    {
        switch ( p_modifiers . GetChild ( i ) -> GetTokenType () )
        {
        case KW_default:
            p_default = true;
            break;
        case KW_extern:
            break;
        case KW_readonly:
            p_readonly = true;
            break;
        default:
            assert ( false );
        }
    }
}

void
ASTBuilder :: AddColumn ( STable & p_table, const AST & p_modifiers, const AST & p_decl, const AST * p_default )
{
    assert ( p_decl . GetTokenType () == PT_COLDECL );
    assert ( p_decl . ChildrenCount () == 2 );  // typespec typed_col

    SColumn * c = Alloc < SColumn > ();
    if ( c != 0 )
    {
        HandleColumnModifiers ( p_modifiers, c -> dflt, c -> read_only);

        const AST & type = * p_decl . GetChild ( 0 );
        switch ( type . GetTokenType () )
        {   // p_decl . GetChild ( 0 ) represents a type or a physical encoding
        case PT_PHYSENCREF:
            {
                const AST * schemaArgs = 0;
                const AST_FQN * fqn = 0;
                const AST * factoryArgs = 0;
                switch ( type . ChildrenCount () )
                {
                    case 3: // schema_parms fqn_opt_vers factory_parms_opt
                        schemaArgs = type . GetChild ( 0 );
                        fqn = ToFQN ( type . GetChild ( 1 ) );
                        assert ( fqn -> GetTokenType () == PT_IDENT );
                        factoryArgs = type . GetChild ( 2 );
                        break;
                    case 2: // fqn_vers_opt factory_parms_opt
                        fqn = ToFQN ( type . GetChild ( 0 ) );
                        assert ( fqn -> GetTokenType () == PT_IDENT );
                        factoryArgs = type . GetChild ( 1 );
                        break;
                    default:
                        assert ( false );
                }
                const KSymbol * sym = Resolve ( * fqn ); // will report unknown name
                if ( sym -> type == ePhysical )
                {
                    c -> ptype = MakePhysicalEncodingSpec ( * sym, * fqn, schemaArgs, factoryArgs, c -> td  );
                }
                else
                {
                    ReportError ( "Not a physical encoding", * fqn);
                }
            }
            break;
        case PT_IDENT:
            {
                const AST_FQN & fqn = * ToFQN ( & type );
                const KSymbol * sym = Resolve ( fqn ); // will report unknown name
                if ( sym != 0 )
                {
                    switch ( sym -> type )
                    {
                    case eDatatype:
                        {
                            const SDatatype * typeDef = static_cast < const SDatatype * > ( sym -> u . obj );
                            c -> td . type_id = typeDef -> id;
                            c -> td . dim = 1;
                        }
                        break;
                    case ePhysical:
                        c -> ptype = MakePhysicalEncodingSpec ( * sym, fqn, 0, 0, c -> td  );
                        break;
                    default:
                        ReportError ( "Cannot be used as a column type", fqn );
                        break;
                    }
                }
            }
            break;
        default: // likely an array
            TypeSpec ( type, c -> td );
            break;
        }

        const AST & typedCol = * p_decl . GetChild ( 1 );
        if ( typedCol . ChildrenCount () == 1 )
        {
            c -> simple = true;
            if ( ! HandleTypedColumn ( p_table, * c, typedCol ) )
            {
                SColumnWhack ( c, 0 );
                return;
            }
        }
        else
        {
            switch ( typedCol . GetTokenType () )
            {
            case  PT_TYPEDCOL: // ident { stmts }
                {
                    assert ( typedCol . ChildrenCount () == 2 );
                    const AST & body = * typedCol . GetChild ( 1 );
                    c -> simple = true; // will override if see 'read'' or 'validate'
                    uint32_t count = body . ChildrenCount ();
                    for ( uint32_t i = 0; i < count; ++i )
                    {
                        const AST & node = * body . GetChild ( i );
                        if ( node . GetTokenType () != PT_EMPTY )
                        {
                            assert ( node . GetTokenType () == PT_COLSTMT );
                            assert ( node . ChildrenCount () == 2 );
                            const AST_Expr * expr = ToExpr ( node . GetChild ( 1 ) );
                            switch ( node . GetChild ( 0 ) -> GetTokenType () )
                            {
                            case KW_read:
                                c -> read = expr -> MakeExpression ( * this );
                                c -> simple = false;
                                break;
                            case KW_validate:
                                c -> validate = expr -> MakeExpression ( * this );
                                c -> simple = false;
                                break;
                            case KW_limit:
                                c -> limit = expr -> MakeExpression ( * this );
                                break;
                            default:
                                assert ( false );
                            }
                        }
                    }
                    if ( ! HandleTypedColumn ( p_table, * c, typedCol ) )
                    {
                        SColumnWhack ( c, 0 );
                        return;
                    }
                }
                break;
            case PT_TYPEDCOLEXPR: // ident = cond_expr
                {
                    assert ( typedCol . ChildrenCount () == 2 );
                    c -> read = ToExpr ( typedCol . GetChild ( 1 ) ) -> MakeExpression ( * this );
                    if ( ! HandleTypedColumn ( p_table, * c, typedCol ) )
                    {
                        SColumnWhack ( c, 0 );
                        return;
                    }
                }
                break;
            default:
                assert ( false );
            }
        }

        if ( VectorAppend ( p_table . col, & c -> cid . id, c ) )
        {
            return;
        }
        SColumnWhack ( c, 0 );
    }
}

bool
ASTBuilder :: MakePhysicalColumnType ( const AST &      p_schemaArgs,
                                       const AST_FQN &  p_fqn_opt_vers,
                                       const AST &      p_factoryArgs,
                                       SPhysMember &    p_col )
{
    const KSymbol * sym = Resolve ( p_fqn_opt_vers ); // will report unknown name
    if ( sym != 0 )
    {
        switch ( sym -> type )
        {
        case eDatatype:
            {
                const SDatatype * typeDef = static_cast < const SDatatype * > ( sym -> u . obj );
                p_col . td . type_id = typeDef -> id;
                p_col . td . dim = 1;
            }
            return true;
        case ePhysical:
            p_col . type = MakePhysicalEncodingSpec ( * sym, p_fqn_opt_vers,
                                                      & p_schemaArgs,
                                                      & p_factoryArgs,
                                                      p_col . td  );
            return true;
        default:
            ReportError ( "Cannot be used as a physical column type", p_fqn_opt_vers );
            break;
        }
    }
    return false;
}

void
ASTBuilder :: AddPhysicalColumn ( STable & p_table, const AST & p_decl, bool p_static )
{
    assert ( p_decl . GetTokenType () == PT_PHYSMBR );
    assert ( p_decl . ChildrenCount () >= 2 );  // typespec typed_col [ init ]

    SPhysMember * c = Alloc < SPhysMember > ();
    if ( c != 0 )
    {
        const AST * colDef = p_decl . GetChild ( 0 ); //col_schema_parms_opt fqn_opt_vers factory_parms_opt
        assert ( colDef != 0 );
        assert ( colDef -> ChildrenCount () == 3 );

        if ( MakePhysicalColumnType ( * colDef -> GetChild ( 0 ),
                                      * ToFQN ( colDef -> GetChild ( 1 ) ),
                                      * colDef -> GetChild ( 2 ),
                                      * c ) )
        {
            const char * ident = p_decl . GetChild ( 1 ) -> GetTokenValue();
            KSymbol * sym = Resolve ( ident, false ); // will not report unknown name
            if ( sym == 0 )
            {
                String name;
                StringInitCString ( & name, ident );
                rc_t rc = KSymTableCreateConstSymbol ( & GetSymTab (), & c -> name, & name, ePhysMember, c);
                if ( rc == 0 )
                {

                    if ( p_decl . ChildrenCount () == 3 )
                    {
                        c -> expr = ToExpr ( p_decl . GetChild ( 2 ) ) -> MakeExpression ( * this );
                    }

                    c -> stat = p_static;

                    if ( VectorAppend ( p_table . phys, & c -> cid . id, c ) )
                    {
                        return;
                    }
                }
                ReportRc ( "KSymTableCreateConstSymbol", rc );
            }
            else if ( sym -> type == eForward )
            {   // phys column was predeclared
                c -> name = sym;
                sym -> u . obj = c;
                sym -> type = ePhysMember;
                if ( VectorAppend ( p_table . phys, & c -> cid . id, c ) )
                {
                    return;
                }
            }
            else
            {   // redefinition
                ReportError ( "Physical column already defined", ident );
            }
        }

        SPhysMemberWhack ( c, 0 );
    }
}

void
ASTBuilder :: AddUntyped ( STable & p_table, const AST_FQN & p_fqn )
{
    const KSymbol * sym = Resolve ( p_fqn );
    if ( sym != 0 )
    {
        if ( sym -> type == eUntypedFunc )
        {
            const SNameOverload * name = static_cast < const SNameOverload * > ( sym -> u . obj );
            p_table . untyped = static_cast < const SFunction * > ( VectorLast ( & name -> items ) );
        }
        else
        {
            ReportError ( "Not an untyped function", p_fqn );
        }
    }
}

void
ASTBuilder :: HandleTableParents ( STable & p_table, const AST & p_parents )
{
    uint32_t parentCount = p_parents . ChildrenCount ();
    for ( uint32_t i = 0 ; i < parentCount; ++i )
    {
        const AST_FQN & parent = * ToFQN ( p_parents . GetChild ( i ) );
        const KSymbol * parentDecl = Resolve ( parent, true );
        if ( parentDecl != 0 )
        {
            uint32_t vers = parent . GetVersion ();
            const STable * dad = static_cast < const STable * > ( SelectVersion ( * parentDecl, STableCmp, & vers ) );
            if ( dad != 0 )
            {
                rc_t rc = STableExtend ( & m_symtab, & p_table, dad );
                if ( rc != 0 )
                {
                    ReportRc ( "STableExtend", rc );
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
            case PT_PRODTRIGGER:
                {
                    const AST * datatype;
                    const AST * ident;
                    const AST * expr;
                    switch ( stmt . ChildrenCount () )
                    {
                    case 2: // trigger
                        datatype    = 0;
                        ident       = stmt . GetChild ( 0 );
                        expr        = stmt . GetChild ( 1 );
                        break;
                    case 3: // has datatype
                        datatype    = stmt . GetChild ( 0 );
                        ident       = stmt . GetChild ( 1 );
                        expr        = stmt . GetChild ( 2 );
                        break;
                    default:
                        assert ( false );
                    }
                    assert ( ident -> ChildrenCount () == 1 );
                    AddProduction ( p_table . prod,
                                    ident -> GetChild ( 0 ) -> GetTokenValue (),
                                    * ToExpr ( expr ),
                                    datatype );
                }
                break;

            case PT_COLUMN:
                // modifiers col_decl [ default ]
                AddColumn ( p_table, * stmt . GetChild ( 0 ), * stmt . GetChild ( 1 ), stmt . GetChild ( 2 ) );
                break;

            case PT_COLUMNEXPR:
                if ( p_table . limit == 0 )
                {
                    assert ( stmt . ChildrenCount () == 1 );
                    p_table . limit = ToExpr ( stmt . GetChild ( 0 ) ) -> MakeExpression ( * this );
                }
                else
                {
                    ReportError ( "Limit constraint already specified" );
                }
                break;

            case KW_static:
                assert ( stmt . ChildrenCount () == 1 );
                AddPhysicalColumn ( p_table, * stmt . GetChild ( 0 ), true );
                break;

            case KW_physical:
                assert ( stmt . ChildrenCount () == 1 );
                AddPhysicalColumn ( p_table, * stmt . GetChild ( 0 ), false );
                break;

            case PT_COLUNTYPED:
                assert ( stmt . ChildrenCount () == 1 );
                AddUntyped ( p_table, * ToFQN ( stmt . GetChild ( 0 ) ) );
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

        table_set_context ( table );
    }

    return ret;
}

