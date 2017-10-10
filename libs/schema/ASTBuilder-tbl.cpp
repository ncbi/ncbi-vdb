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

class TableDeclaration // Wrapper around STable
{
public:
    TableDeclaration ( ASTBuilder & p_builder );
    ~TableDeclaration ();

    bool SetName ( const AST_FQN &  p_fqn );
    void HandleParents ( const AST & p_parents );
    void HandleBody ( const AST & p_body );

private:
    bool HandleOverload ( const KSymbol * p_priorDecl );
    void AddColumn ( const AST & p_modifiers, const AST & p_decl, const AST * p_default );
    void AddPhysicalColumn ( const AST & p_decl, bool p_static );
    void AddUntyped ( const AST_FQN & p_fqn );
    SExpression * MakePhysicalEncodingSpec ( const KSymbol & p_sym,
                                             const AST_FQN & p_fqn,
                                             const AST * p_schemaArgs,
                                             const AST * p_factoryArgs,
                                             VTypedecl & p_type );
    bool MakePhysicalColumnType ( const AST &      p_schemaArgs,
                                  const AST_FQN &  p_fqn_opt_vers,
                                  const AST &      p_factoryArgs,
                                  SPhysMember &    p_col );
    bool HandleTypedColumn ( SColumn & p_col, const AST & p_typedCol );

private:
    ASTBuilder &    m_builder;
    STable *        m_self;
};

TableDeclaration :: TableDeclaration ( ASTBuilder & p_builder )
:   m_builder ( p_builder ),
    m_self ( m_builder . Alloc < STable > () )
{
    if ( m_self != 0 )
    {
        /* prepare vectors */
        VectorInit ( & m_self -> parents, 0, 4 );
        VectorInit ( & m_self -> overrides, 0, 4 );
        VectorInit ( & m_self -> col, 0, 16 );
        VectorInit ( & m_self -> cname, 0, 16 );
        VectorInit ( & m_self -> phys, 0, 16 );
        VectorInit ( & m_self -> prod, 0, 64 );
        VectorInit ( & m_self -> vprods, 1, 16 );
        VectorInit ( & m_self -> syms, 1, 32 );
    }
}

TableDeclaration :: ~TableDeclaration ()
{
    // m_self gets added to the schema or freed on error, no need to free here
}

bool
TableDeclaration :: SetName ( const AST_FQN & p_fqn )
{
    assert ( m_self != 0 );

    m_self -> version = p_fqn . GetVersion ();

    const KSymbol * priorDecl = m_builder . Resolve ( p_fqn, false );
    if ( priorDecl == 0 )
    {
        m_self -> name = m_builder . CreateFqnSymbol ( p_fqn, eTable, m_self );
        if ( m_self -> name != 0 &&
                m_builder . CreateOverload ( m_self -> name,
                                            m_self,
                                            STableSort,
                                            m_builder . GetSchema () -> tbl,
                                            m_builder . GetSchema () -> tname,
                                            & m_self -> id ) )
        {
            return true;
        }
    }
    else if ( HandleOverload ( priorDecl ) )
    {   // declared previously, this declaration not ignored
        m_self -> name = priorDecl;
        return true;
    }

    STableWhack ( m_self, 0 );
    m_self = 0;
    return false;
}

bool
TableDeclaration :: HandleOverload ( const KSymbol * p_priorDecl )
{
    assert ( m_self != 0 );
    assert ( p_priorDecl != 0 );

    Vector & tables = m_builder . GetSchema () -> tbl;

    SNameOverload *name = ( SNameOverload* ) p_priorDecl -> u . obj;
    assert ( name != 0 );

    uint32_t idx;
    rc_t rc = VectorInsertUnique ( & name -> items, m_self, & idx, STableSort );
    if ( rc == 0 ) // overload added
    {
        return m_builder . VectorAppend ( tables, & m_self -> id, m_self );
    }

    if ( GetRCState ( rc ) == rcExists )
    {   /* an overload with the same major version exists */
        /* see if new table trumps old */
        const STable *exist = static_cast < const STable * > ( VectorGet ( & name -> items, idx ) );
        const STable *newer;
        rc = STableCompare ( exist, m_self, & newer, false );
        if ( rc == 0 && newer == m_self )
        {
            /* put the new one in place of the existing */
            void * ignore;
            VectorSwap ( & name -> items, idx, m_self, & ignore );
            m_self -> id = exist -> id;

            if ( m_builder . VectorAppend ( tables, & m_self -> id, m_self ) )
            {   // tell everyone to use new table
                rc = schema_update_tbl_ref ( m_builder . GetSchema (), exist, m_self );
                if ( rc == 0 )
                {
                    return true;
                }
                m_builder . ReportRc ( "schema_update_tbl_ref", rc );
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
TableDeclaration :: HandleParents ( const AST & p_parents )
{
    uint32_t parentCount = p_parents . ChildrenCount ();
    for ( uint32_t i = 0 ; i < parentCount; ++i )
    {
        const AST_FQN & parent = * ToFQN ( p_parents . GetChild ( i ) );
        const KSymbol * parentDecl = m_builder . Resolve ( parent, true );
        if ( parentDecl != 0 )
        {
            if ( parentDecl -> type != eTable )
            {
                m_builder . ReportError ( "A table's parent has to be a table", parent );
                continue;
            }
            const STable * dad = static_cast < const STable * > ( m_builder . SelectVersion ( parent, * parentDecl, STableCmp ) );
            if ( dad != 0 )
            {
                rc_t rc = STableExtend ( & m_builder . GetSymTab (), m_self, dad );
                if ( rc != 0 )
                {   //TODO: check for (rcTable, rcExists), (rcName, rcExists)
                    m_builder . ReportRc ( "STableExtend", rc );
                    break;
                }
            }
        }
    }
}

void
TableDeclaration :: HandleBody ( const AST & p_body )
{
    rc_t rc = push_tbl_scope ( & m_builder . GetSymTab (), m_self );
    if ( rc == 0 )
    {
        /* scan override tables for virtual symbols */
        if ( VectorDoUntil ( & m_self -> overrides, false, STableScanVirtuals, & m_builder . GetSymTab () ) == 0 )
        {
            /* handle table declarations */
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
                        m_builder . AddProduction ( * ident,
                                                    m_self -> prod,
                                                    ident -> GetChild ( 0 ) -> GetTokenValue (),
                                                    * ToExpr ( expr ),
                                                    datatype );
                    }
                    break;

                case PT_COLUMN:
                    // modifiers col_decl [ default ]
                    AddColumn ( * stmt . GetChild ( 0 ), * stmt . GetChild ( 1 ), stmt . GetChild ( 2 ) );
                    break;

                case PT_COLUMNEXPR:
                    if ( m_self -> limit == 0 )
                    {
                        m_self -> limit = ToExpr ( stmt . GetChild ( 0 ) ) -> MakeExpression ( m_builder );
                    }
                    else
                    {
                        m_builder . ReportError ( stmt . GetLocation (), "Limit constraint already specified" );
                    }
                    break;

                case KW_static:
                    assert ( stmt . ChildrenCount () == 1 );
                    AddPhysicalColumn ( * stmt . GetChild ( 0 ), true );
                    break;

                case KW_physical:
                    assert ( stmt . ChildrenCount () == 1 );
                    AddPhysicalColumn ( * stmt . GetChild ( 0 ), false );
                    break;

                case PT_COLUNTYPED:
                    assert ( stmt . ChildrenCount () == 1 );
                    AddUntyped ( * ToFQN ( stmt . GetChild ( 0 ) ) );
                    break;

                case PT_EMPTY:
                    break;

                default:
                    assert ( false );
                }
            }

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
            m_builder . ReportRc ( "STableScanVirtuals", rc );
        }

        pop_tbl_scope ( & m_builder . GetSymTab (), m_self );

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
    }
    else
    {
        m_builder . ReportRc ( "push_tbl_scope", rc );
    }
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

SExpression *
TableDeclaration :: MakePhysicalEncodingSpec ( const KSymbol & p_sym,
                                               const AST_FQN & p_fqn,
                                               const AST * p_schemaArgs,
                                               const AST * p_factoryArgs,
                                               VTypedecl & p_type )
{
    SPhysEncExpr * ret = m_builder . Alloc < SPhysEncExpr > ();
    if ( ret != 0 )
    {
        /* initialize */
        ret -> dad . var = ePhysEncExpr;
        atomic32_set ( & ret -> dad . refcount, 1 );
        ret -> phys = NULL;
        VectorInit ( & ret -> schem, 0, 4 );
        VectorInit ( & ret -> pfact, 0, 8 );

        if ( p_schemaArgs != 0 && ! m_builder. FillSchemaParms ( * p_schemaArgs, ret -> schem ) )
        {
            SExpressionWhack ( & ret -> dad );
            return 0;
        }

        // capture requested version
        assert ( p_sym . type == ePhysical );
        ret -> phys = static_cast < const SPhysical * > ( m_builder . SelectVersion ( p_fqn, p_sym, SPhysicalCmp, & ret -> version ) );
        ret -> version_requested = ret -> version != 0;

        /* evaluate type expression */
        if ( ret -> phys != 0 )
        {
            rc_t rc;
            {
                /* bind schema parameters */
                Vector prior, cx_bind;
                VectorInit ( & cx_bind, 1, m_builder . GetSchema () -> num_indirect ? m_builder . GetSchema () -> num_indirect : 16 );
                rc = SPhysicalBindSchemaParms ( ret -> phys, & prior, & ret -> schem, & cx_bind );
                if ( rc == 0 )
                {
                    const SExpression *tx = ret -> phys -> td;
                    assert ( tx != NULL );
                    rc = STypeExprResolveAsTypedecl ( ( const STypeExpr* ) tx,m_builder .  GetSchema (), & p_type, & cx_bind );
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
                        if ( ! m_builder . VectorAppend ( ret -> pfact, 0, expr . MakeExpression ( m_builder ) ) )
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

void
TableDeclaration :: AddColumn ( const AST & p_modifiers, const AST & p_decl, const AST * p_default )
{
    assert ( p_decl . GetTokenType () == PT_COLDECL );
    assert ( p_decl . ChildrenCount () == 2 );  // typespec typed_col

    SColumn * c = m_builder . Alloc < SColumn > ();
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
                const KSymbol * sym = m_builder . Resolve ( * fqn ); // will report unknown name
                if ( sym != 0 )
                {
                    if ( sym -> type == ePhysical )
                    {
                        c -> ptype = MakePhysicalEncodingSpec ( * sym, * fqn, schemaArgs, factoryArgs, c -> td  );
                    }
                    else
                    {
                        m_builder . ReportError ( "Not a physical encoding", * fqn);
                    }
                }
            }
            break;
        case PT_IDENT:
            {
                const AST_FQN & fqn = * ToFQN ( & type );
                const KSymbol * sym = m_builder . Resolve ( fqn ); // will report unknown name
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
                        m_builder . ReportError ( "Cannot be used as a column type", fqn );
                        break;
                    }
                }
            }
            break;
        default: // likely an array
            m_builder . TypeSpec ( type, c -> td );
            break;
        }

        const AST & typedCol = * p_decl . GetChild ( 1 );
        if ( typedCol . ChildrenCount () == 1 )
        {
            c -> simple = true;
            if ( ! HandleTypedColumn ( * c, typedCol ) )
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
                                c -> read = expr -> MakeExpression ( m_builder );
                                c -> simple = false;
                                break;
                            case KW_validate:
                                c -> validate = expr -> MakeExpression ( m_builder );
                                c -> simple = false;
                                break;
                            case KW_limit:
                                c -> limit = expr -> MakeExpression ( m_builder );
                                break;
                            default:
                                assert ( false );
                            }
                        }
                    }
                    if ( ! HandleTypedColumn ( * c, typedCol ) )
                    {
                        SColumnWhack ( c, 0 );
                        return;
                    }
                }
                break;
            case PT_TYPEDCOLEXPR: // ident = cond_expr
                {
                    assert ( typedCol . ChildrenCount () == 2 );
                    c -> read = ToExpr ( typedCol . GetChild ( 1 ) ) -> MakeExpression ( m_builder );
                    if ( ! HandleTypedColumn ( * c, typedCol ) )
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
    }
 }

bool
TableDeclaration :: HandleTypedColumn ( SColumn & p_col, const AST & p_typedCol )
{
    assert ( p_typedCol . ChildrenCount () >= 1 );
    assert ( p_typedCol . GetChild ( 0 ) -> GetTokenType () == PT_IDENT );
    const char * ident = p_typedCol . GetChild ( 0 ) -> GetChild ( 0 ) -> GetTokenValue ();
    String name;
    StringInitCString ( & name, ident );

    KSymbol * priorDecl = const_cast < KSymbol * > ( m_builder . Resolve ( p_typedCol . GetLocation (), ident, false ) );
    if ( priorDecl == 0 )
    {   // new column: add p_col to m_self -> col, a new overload to m_self -> cname
        rc_t rc = KSymTableCreateConstSymbol ( & m_builder . GetSymTab (), & p_col . name, & name, eColumn, & p_col );
        if ( rc != 0 )
        {
            m_builder . ReportRc ( "KSymTableCreateConstSymbol", rc );
            return false;
        }
        if ( ! m_builder .CreateOverload ( p_col . name,
                                           & p_col,
                                           SColumnSort,
                                           m_self -> col,
                                           m_self -> cname,
                                           0 ) )
        {
            return false;
        }
    }
    else // column name seen before
    {
        switch ( priorDecl -> type )
        {
        case eForward:
        case eVirtual:
            /* if column was forwarded, give it a type */
            p_col . name = priorDecl;
            priorDecl -> type = eColumn;
            if ( ! m_builder .CreateOverload ( p_col . name,
                                               & p_col,
                                               SColumnSort,
                                               m_self -> col,
                                               m_self -> cname,
                                               0 ) )
            {
                return false;
            }
            break;
        case eColumn:
            {
                SNameOverload *name = ( SNameOverload* ) priorDecl -> u . obj;
                if ( VectorFind ( & name -> items, & p_col . td, NULL, SColumnCmp ) != NULL )
                {
                    m_builder . ReportError ( p_typedCol . GetLocation (), "Column already defined", ident );
                    return false;
                }
                p_col . name = priorDecl;
                // add column to m_self -> col
                if ( ! m_builder . VectorAppend ( m_self -> col, 0, & p_col ) )
                {
                    return false;
                }
            }
            break;
        case eIdent:
            /* allow names defined in scopes other than table and intrinsic */
            {
                rc_t rc = KSymTableCreateConstSymbol ( & m_builder .GetSymTab (), & p_col . name, & name, eColumn, 0 );
                if ( rc != 0 )
                {
                    m_builder . ReportRc ( "KSymTableCreateConstSymbol", rc );
                    return false;
                }
            }
            break;
        default:
            m_builder . ReportError ( p_typedCol . GetLocation (), "Columne name is already in use", ident );
            return false;
        }
    }

    // at this point, p_col has been added to the schema structures and should not be destroyed,
    // so below we return true even we report an error
    if ( p_col . simple )
    {
        if ( p_col . read_only )
        {
            m_builder . ReportError ( p_typedCol . GetLocation (), "Simple column cannot be readonly", ident);
        }
        else
        {
            // generate and attach the corresponding physical column
            String physname;
            char physnamebuff [ 256 ];
            /* tack a dot onto the beginning and look up the symbol */
            physnamebuff [ 0 ] = '.';
            memcpy ( & physnamebuff [ 1 ], p_col . name -> name . addr, p_col . name -> name . size );
            StringInit ( & physname, physnamebuff, p_col . name -> name . size + 1, p_col . name -> name . len + 1 );
            KSymbol * sym = KSymTableFind ( & m_builder . GetSymTab (), & physname );
            /* if the symbol exists, then this CANNOT be a simple column */
            if ( sym != 0 && ! ( sym -> type == eForward || sym -> type == eVirtual ) )
            {
                /* check for explicit physical type */
                if ( p_col . ptype != 0 )
                {
                    m_builder . ReportError ( p_typedCol . GetLocation (), "Implicit physical column previously declared", name );
                }
                else
                {
                    m_builder . ReportError ( p_typedCol . GetLocation (), "Missing column read or validate expression", name );
                }
            }
            else if ( ( p_col . td . type_id & 0xC0000000 ) != 0 )
            {
                m_builder . ReportError ( p_typedCol . GetLocation (), "Simple columns cannot have typeset as type", name );
            }
            else
            {
                if ( sym != 0 )
                {
                    sym -> type = ePhysMember;
                }
                else
                {
                    sym = m_builder . CreateLocalSymbol ( p_typedCol, physname, ePhysMember, 0 );
                }

                if ( sym != 0 )
                {
                    rc_t rc = implicit_physical_member ( & m_builder . GetSymTab (), 0, m_self, & p_col, sym );
                    if ( rc != 0 )
                    {
                        m_builder . ReportRc ( "implicit_physical_member", rc);
                    }
                }
            }
        }
    }

    return true;
}

bool
TableDeclaration :: MakePhysicalColumnType ( const AST &      p_schemaArgs,
                                       const AST_FQN &  p_fqn_opt_vers,
                                       const AST &      p_factoryArgs,
                                       SPhysMember &    p_col )
{
    const KSymbol * sym = m_builder . Resolve ( p_fqn_opt_vers ); // will report unknown name
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
            m_builder . ReportError ( "Cannot be used as a physical column type", p_fqn_opt_vers );
            break;
        }
    }
    return false;
}

void
TableDeclaration :: AddPhysicalColumn ( const AST & p_decl, bool p_static )
{
    assert ( p_decl . GetTokenType () == PT_PHYSMBR );
    assert ( p_decl . ChildrenCount () >= 2 );  // typespec typed_col [ init ]

    SPhysMember * c = m_builder . Alloc < SPhysMember > ();
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
            const AST & physIdent = * p_decl . GetChild ( 1 );
            const char * ident = physIdent . GetTokenValue();
            KSymbol * sym = m_builder . Resolve ( physIdent . GetLocation (), ident, false ); // will not report unknown name
            if ( sym == 0 )
            {
                String name;
                StringInitCString ( & name, ident );
                rc_t rc = KSymTableCreateConstSymbol ( & m_builder . GetSymTab (), & c -> name, & name, ePhysMember, c);
                if ( rc == 0 )
                {

                    if ( p_decl . ChildrenCount () == 3 )
                    {
                        c -> expr = ToExpr ( p_decl . GetChild ( 2 ) ) -> MakeExpression ( m_builder );
                    }

                    c -> stat = p_static;

                    if (m_builder .  VectorAppend ( m_self -> phys, & c -> cid . id, c ) )
                    {
                        return;
                    }
                }
                m_builder . ReportRc ( "KSymTableCreateConstSymbol", rc );
            }
            else if ( sym -> type == eForward || sym -> type == eVirtual )
            {   // phys column was predeclared
                c -> name = sym;
                sym -> u . obj = c;
                sym -> type = ePhysMember;
                if ( p_decl . ChildrenCount () == 3 )
                {
                    c -> expr = ToExpr ( p_decl . GetChild ( 2 ) ) -> MakeExpression ( m_builder );
                }

                c -> stat = p_static;

                if ( m_builder . VectorAppend ( m_self -> phys, & c -> cid . id, c ) )
                {
                    return;
                }
            }
            else
            {   // redefinition
                m_builder . ReportError ( physIdent . GetLocation (), "Physical column already defined", ident );
            }
        }

        SPhysMemberWhack ( c, 0 );
    }
}

void
TableDeclaration :: AddUntyped ( const AST_FQN & p_fqn )
{
    const KSymbol * sym = m_builder . Resolve ( p_fqn );
    if ( sym != 0 )
    {
        if ( sym -> type == eUntypedFunc )
        {
            const SNameOverload * name = static_cast < const SNameOverload * > ( sym -> u . obj );
            m_self -> untyped = static_cast < const SFunction * > ( VectorLast ( & name -> items ) );
        }
        else
        {
            m_builder . ReportError ( "Not an untyped function", p_fqn );
        }
    }
}

AST *
ASTBuilder ::  TableDef ( const Token * p_token, AST_FQN * p_fqn, AST * p_parents, AST * p_body )
{
    AST * ret = new AST ( p_token, p_fqn, p_parents, p_body );

    TableDeclaration table ( * this );
    assert ( p_fqn != 0 );
    if ( table . SetName ( * p_fqn ) )
    {
        assert ( p_parents != 0 );
        table. HandleParents ( * p_parents );
        assert ( p_body != 0 );
        table. HandleBody ( * p_body );
    }

    return ret;
}

