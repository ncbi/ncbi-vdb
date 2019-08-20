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

ASTBuilder :: ASTBuilder ( VSchema * p_schema )
:   m_schema ( p_schema ),
    m_nextContextId ( 0 ),
    m_view ( 0 )
{
    assert ( m_schema != 0 );
    VSchemaAddRef ( m_schema );

    rc_t rc = KSymTableInit ( & m_symtab, 0 );
    if ( rc != 0 )
    {
        ReportRc ( "KSymTableInit", rc );
    }
    else
    {
        rc = init_symtab ( & m_symtab, m_schema ); // this pushes the global scope
        if ( rc != 0 )
        {
            ReportRc ( "init_symtab", rc );
        }
    }
}

ASTBuilder :: ~ASTBuilder ()
{
    KSymTableWhack ( & m_symtab);
    VSchemaRelease ( m_schema );
}

void
ASTBuilder :: ReportError ( const ErrorReport :: Location & p_loc, const char * p_msg )
{
    m_errors . ReportError ( p_loc, "%s", p_msg );
}

void
ASTBuilder :: ReportError ( const char * p_msg, const AST_FQN& p_fqn )
{
    char buf [ 1024 ];
    p_fqn . GetFullName ( buf, sizeof buf );
    m_errors . ReportError ( p_fqn . GetChild ( 0 ) -> GetToken () . GetLocation (), "%s: '%s'", p_msg, buf );
    //TODO: add location of the original declaration
}

void
ASTBuilder :: ReportError ( const ErrorReport :: Location & p_loc, const char * p_msg, const String & p_str )
{
    m_errors . ReportError ( p_loc, "%s: '%S'", p_msg, & p_str );
}

void
ASTBuilder :: ReportError ( const ErrorReport :: Location & p_loc, const char * p_msg, const char * p_str )
{
    m_errors . ReportError ( p_loc, "%s: '%s'", p_msg, p_str );
}

void
ASTBuilder :: ReportError ( const ErrorReport :: Location & p_loc, const char * p_msg, int64_t p_val )
{
    m_errors . ReportError ( p_loc, "%s: %l", p_msg, p_val );
}

void
ASTBuilder :: ReportRc ( const char* p_msg, rc_t p_rc )
{
    m_errors . ReportInternalError ( "", "%s: rc=%R", p_msg, p_rc );
}

void
ASTBuilder :: ReportInternalError ( const char * p_msg, const char * p_source )
{
    m_errors . ReportInternalError ( p_source, "%s", p_msg );
}

AST *
ASTBuilder :: Build ( const ParseTree& p_root, const char * p_source, bool p_debugParse )
{
    AST* ret = 0;
    AST_debug = p_debugParse;
    ParseTreeScanner scanner ( p_root, p_source );
    if ( AST_parse ( ret, * this, scanner ) == 0 )
    {
        return ret;
    }
    delete ret;
    return 0;
}

bool
ASTBuilder :: VectorAppend ( Vector & p_self, uint32_t *p_idx, const void *p_item )
{
    rc_t rc = :: VectorAppend ( & p_self, p_idx, p_item );
    if ( rc != 0 )
    {
        ReportRc ( "VectorAppend", rc );
        return false;
    }
    return true;
}

const KSymbol*
ASTBuilder :: CreateFqnSymbol ( const AST_FQN& p_fqn, uint32_t p_type, const void * p_obj )
{
    rc_t rc = 0;
    uint32_t count = p_fqn . NamespaceCount ();
    for ( uint32_t i = 0 ; i < count; ++ i )
    {
        String name;
        StringInitCString ( & name, p_fqn . GetChild ( i ) -> GetTokenValue () );
        KSymbol *ns;
        // will not re-create namespace if already exists
        rc = KSymTableCreateNamespace ( & m_symtab, & ns, & name );
        if ( rc == 0 )
        {
            rc = KSymTablePushNamespace ( & m_symtab, ns );
            if ( rc != 0 )
            {
                ReportRc ( "KSymTablePushNamespace", rc );
            }
        }
        else
        {
            ReportRc ( "KSymTableCreateNamespace", rc );
        }
    }

    KSymbol* ret = 0;
    if ( rc == 0 )
    {
        String name;
        p_fqn . GetIdentifier ( name );
        // will add to the current scope, which is the same as the innermost namespace
        rc = KSymTableCreateSymbol ( & m_symtab, & ret, & name, p_type, p_obj );
        if ( GetRCState ( rc ) == rcExists )
        {
            ReportError ( "Object already declared", p_fqn );
        }
        else if ( rc != 0 )
        {
            ReportRc ( "KSymTableCreateSymbol", rc );
        }
    }

    for ( uint32_t i = 0 ; i < count; ++ i )
    {
        KSymTablePopNamespace ( & m_symtab );
    }

    return ret;
}

KSymbol*
ASTBuilder :: Resolve ( const Token :: Location & p_loc, const char* p_ident, bool p_reportUnknown )
{
    String name;
    StringInitCString ( & name, p_ident );
    KSymbol* ret = KSymTableFind ( & m_symtab, & name );
    if ( ret == 0 && p_reportUnknown )
    {
        ReportError ( p_loc, "Undeclared identifier", p_ident );
    }
    return ret;
}

const KSymbol*
ASTBuilder :: Resolve ( const AST_FQN & p_fqn, bool p_reportUnknown )
{
    uint32_t count = p_fqn . ChildrenCount ();
    assert ( count > 0 );

    if ( count == 1 )
    {   // plain identifier
        return Resolve ( p_fqn .GetLocation (), p_fqn . GetChild ( 0 ) -> GetTokenValue (), p_reportUnknown );
    }

    // work the namespaces
    bool ns_resolved = true;
    for ( uint32_t i = 0 ; i < count - 1; ++ i )
    {
        String name;
        StringInitCString ( & name, p_fqn . GetChild ( i ) -> GetTokenValue () );
        KSymbol *ns = 0;
        if ( i == 0 )
        {
            ns = KSymTableFindGlobal ( & m_symtab, & name );
        }
        else if ( ns_resolved )
        {
            ns = KSymTableFindShallow ( & m_symtab, & name );
        }

        if ( ns == 0 )
        {
            ns_resolved = false; // no need to do any lookup from now on
            // create a new namespace
            rc_t rc = KSymTableCreateNamespace ( & m_symtab, & ns, & name );
            if ( rc != 0 )
            {
                ReportRc ( "KSymTableCreateNamespace", rc );
                count = i;
                break;
            }
        }

        rc_t rc = KSymTablePushNamespace ( & m_symtab, ns );
        if ( rc != 0 )
        {
            ReportRc ( "KSymTablePushNamespace", rc );
            count = i;
            ns_resolved = false;
            break;
        }
    }

    // we pushed all the namespaces; if all of them existed we can look for the identifier in the innermost namespace
    String name;
    p_fqn . GetIdentifier ( name );

    const KSymbol * ret = 0;
    if ( ns_resolved )
    {
        ret = KSymTableFindShallow ( & m_symtab, & name );
    }
    if ( ret == 0 && p_reportUnknown )
    {
        ReportError ( p_fqn . GetChild ( p_fqn . ChildrenCount () - 1 ) -> GetLocation (), "Undeclared identifier", name );
    }

    // pop the namespaces, count is how many to pop
    if ( count > 0 )
    {
        for ( uint32_t i = 0 ; i < count - 1; ++ i )
        {
            KSymTablePopNamespace ( & m_symtab );
        }
    }

    return ret;
}

uint32_t
ASTBuilder :: IntrinsicTypeId ( const char * p_type ) const
{
    String type;
    StringInitCString ( & type, p_type );
    KSymbol* sym = KSymTableFindIntrinsic ( & m_symtab, & type );
    assert ( sym != 0 );
    assert ( sym -> type == eDatatype );
    const SDatatype * dt = static_cast < const SDatatype * > ( sym -> u . obj );
    return dt -> id;
}

uint64_t
ASTBuilder :: EvalConstExpr ( const AST_Expr & p_expr )
{
    uint64_t ret = 0;
    SExpression * expr = p_expr . EvaluateConst ( *this ); // will report problems
    if ( expr != 0 )
    {
        switch ( expr -> var )
        {
        case eConstExpr :
            {
                SConstExpr* cexpr = reinterpret_cast < SConstExpr* > ( expr );
                // this may change as more kinds of const expressions are supported
                assert ( cexpr -> td . type_id = IntrinsicTypeId ( "U64" ) );
                ret = cexpr -> u . u64 [ 0 ];
            }
            break;
        default:
            ReportError ( p_expr . GetLocation (), "Unsupported in const expressions", expr -> var );
            break;
        }
        SExpressionWhack ( expr );
    }
    return ret;
}

const void *
ASTBuilder :: SelectVersion ( const AST_FQN & p_fqn,
                              const KSymbol & p_ovl,
                              int64_t ( CC * p_cmp ) ( const void *item, const void *n ),
                              uint32_t * p_version )
{
    const SNameOverload *name = static_cast < const SNameOverload * > ( p_ovl . u . obj );
    const void * ret = 0;
    uint32_t version = p_fqn . GetVersion ();
    if ( version != 0 )
    {
        ret = VectorFind ( & name -> items, & version, NULL, p_cmp );
        if ( ret == 0 )
        {
            m_errors . ReportError ( p_fqn . GetLocation (),
                                     "Requested version does not exist: '%S#%V'",
                                     & p_ovl . name, version );
        }
    }
    else
    {
        ret = VectorLast ( & name -> items );
    }
    if ( p_version != 0 )
    {
        * p_version = version;
    }
    return ret;
}

KSymbol *
ASTBuilder :: CreateLocalSymbol ( const AST & p_node, const String & p_name, int p_type, const void * p_obj )
{
    KSymbol * ret = 0;

    if ( KSymTableFindShallow ( & GetSymTab (), & p_name ) != 0 ||
            KSymTableFindIntrinsic ( & GetSymTab (), & p_name ) )
    {
        ReportError ( p_node . GetLocation (), "Name already in use", p_name );
    }
    else
    {
        rc_t rc = KSymTableCreateSymbol ( & GetSymTab (), & ret, & p_name, p_type, p_obj );
        if ( rc != 0 )
        {
            ReportRc ( "KSymTableCreateSymbol", rc );
        }
    }

    return ret;
}

KSymbol *
ASTBuilder :: CreateLocalSymbol ( const AST & p_node, const char* p_name, int p_type, const void * p_obj )
{
    String name;
    StringInitCString ( & name, p_name );
    return CreateLocalSymbol ( p_node, name, p_type, p_obj );
}

KSymbol *
ASTBuilder :: CreateConstSymbol ( const char* p_name, int p_type, const void * p_obj )
{
    KSymbol * ret = 0;
    String name;
    StringInitCString ( & name, p_name );
    rc_t rc = KSymTableCreateConstSymbol ( & GetSymTab (), & ret, & name, p_type, p_obj );
    if ( rc != 0 )
    {
        ReportRc ( "KSymTableCreateSymbol", rc );
    }
    return ret;
}

bool
ASTBuilder :: CreateOverload ( const KSymbol *  p_name,
                               const void *     p_object,
                               int64_t          (CC *p_sort)(const void *, const void *),
                               Vector &         p_objects,
                               Vector &         p_names,
                               uint32_t *       p_id )
{
    assert ( p_name != 0 );
    SNameOverload * ovl;
    rc_t rc = SNameOverloadMake ( & ovl, p_name, 0, 4 );
    if ( rc == 0 )
    {
        rc = VectorInsertUnique ( & ovl -> items, p_object, 0, p_sort );
        if ( rc == 0 )
        {
            if ( VectorAppend ( p_objects, p_id, p_object ) )
            {
                if ( VectorAppend ( p_names, & ovl -> cid . id, ovl ) )
                {
                    ovl -> cid . ctx = -1;
                    return true;
                }
            }
        }
        else
        {
            ReportRc ( "VectorInsertUnique", rc );
        }
        SNameOverloadWhack ( ovl, 0 );
    }
    else
    {
        ReportRc ( "SNameOverloadMake", rc );
    }
    return false;
}

void
ASTBuilder :: AddProduction ( const AST & p_node, Vector & p_list, const char * p_name, const AST_Expr & p_expr, const AST * p_type )
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

        prod -> name = Resolve ( p_node . GetLocation (), p_name, false );
        if ( prod -> name == 0 )
        {
            prod -> name = CreateLocalSymbol ( p_node, p_name, eProduction, prod );
        }
        else if ( prod -> name -> type != eForward && prod -> name -> type != eVirtual )
        {
            ReportError ( p_node . GetLocation (), "Production name is already in use", p_name );
            SProductionWhack ( prod, NULL );
            return;
        }
        KSymbol * sym = const_cast < KSymbol * > ( prod -> name );
        sym -> type = eProduction;
        sym -> u . obj = prod;

        prod -> expr = p_expr . MakeExpression ( *this ) ;

        if ( ! VectorAppend ( p_list, & prod -> cid . id, prod ) )
        {
            SProductionWhack ( prod, NULL );
        }
    }
}

bool
ASTBuilder :: CheckForColumnCollision ( const KSymbol *sym )
{
    const KSymbol *found = KSymTableFindSymbol ( &  GetSymTab (), sym );
    if ( found != NULL && found != sym )
    {
        if ( found -> type == eColumn && sym -> type == eColumn )
        {   /* when colliding columns originated in the same
                table/view, consider them to be compatible extensions */
            const SNameOverload * found_col = static_cast < const SNameOverload * > ( found -> u . obj );
            const SNameOverload * sym_col = static_cast < const SNameOverload * > ( sym -> u . obj );
            assert ( sym_col != NULL && found_col != NULL );
            if ( sym_col -> cid . ctx == found_col -> cid . ctx )
            {
                return ! SOverloadTestForTypeCollision ( sym_col, found_col );
            }
        }
        return false;
    }
    return true;
}

bool
ASTBuilder :: ScanVirtuals ( const Token :: Location & p_loc, Vector & p_byParent )
{
    uint32_t start = VectorStart ( & p_byParent );
    uint32_t count = VectorLength ( & p_byParent );
    for ( uint32_t i = 0; i < count; ++ i )
    {
        uint32_t idx = start + i;
        const KSymbol * orig = static_cast < const KSymbol * > ( VectorGet ( & p_byParent, idx ) );
        assert ( orig != NULL );
        if ( orig -> type == eVirtual )
        {
            void *ignore;

            /* since the virtual productions in one parent could be
               defined by another parent, test for the possibility */
            const KSymbol *def = KSymTableFindSymbol ( & GetSymTab (), orig );
            if ( def != NULL )
            {
                if ( def -> type == eProduction || def -> type == eVirtual )
                {
                    VectorSwap ( & p_byParent, idx, def, & ignore );
                }
                else
                {
                    ReportError ( p_loc,
                                  "a virtual production from one parent defined as non-production in another",
                                  def -> name );
                    return false;
                }
            }
            else
            {
                /* copy the original */
                BSTree * scope = static_cast < BSTree * > ( VectorLast ( & GetSymTab () . stack ) );
                const KSymbol *copy;
                rc_t rc = KSymbolCopy ( scope, & copy, orig );
                if ( rc != 0 )
                {
                    ReportRc ( "KSymbolCopy", rc );
                    return false;
                }

                /* replace the parent virtual with an updatable copy */
                VectorSwap ( & p_byParent, idx, copy, & ignore );
            }
        }
    }
    return true;
}

