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
#include <klib/printf.h>

#include <kfs/directory.h>
#include <kfs/mmap.h>

// hide an unfortunately named C function typename()
#define typename __typename
#include "../vdb/schema-parse.h"
#undef typename
#include "../vdb/dbmgr-priv.h"
#include "../vdb/schema-priv.h"
#include "../vdb/schema-expr.h"

#include "SchemaParser.hpp"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast-tokens.h"

using namespace std;

ASTBuilder :: ASTBuilder ( VSchema * p_schema )
:    m_schema ( p_schema )
{
    assert ( m_schema != 0 );
    VSchemaAddRef ( m_schema );

    VectorInit ( & m_errors, 0, 1024 );

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

static
void CC
WhackMessage ( void *item, void *data )
{
    free ( item );
}

ASTBuilder :: ~ASTBuilder ()
{
    KSymTableWhack ( & m_symtab);
    VSchemaRelease ( m_schema );
    VectorWhack ( & m_errors, WhackMessage, 0 );
}

void
ASTBuilder :: ReportError ( const char* p_fmt, ... )
{
    const unsigned int BufSize = 1024;
    char buf [ BufSize ];

    va_list args;
    va_start ( args, p_fmt );
    string_vprintf ( buf, BufSize, 0, p_fmt, args );
    va_end ( args );

    :: VectorAppend ( & m_errors, 0, string_dup_measure ( buf, 0 ) );
}

void
ASTBuilder :: ReportError ( const char* p_msg, const AST_FQN& p_fqn )
{
    char buf [ 1024 ];
    p_fqn . GetFullName ( buf, sizeof buf );
    ReportError ( "%s: '%s'", p_msg, buf ); //TODO: add location of the original declaration
}

void
ASTBuilder :: ReportRc ( const char* p_msg, rc_t p_rc )
{
    ReportError ( "%s: rc=%R", p_msg, p_rc );
}

AST *
ASTBuilder :: Build ( const ParseTree& p_root, bool p_debugParse )
{
    AST* ret = 0;
    AST_debug = p_debugParse;
    ParseTreeScanner scanner ( p_root );
    if ( AST_parse ( ret, * this, scanner ) == 0 )
    {
        return ret;
    }
    //TODO: report error(s)
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
ASTBuilder :: Resolve ( const char* p_ident, bool p_reportUnknown )
{
    String name;
    StringInitCString ( & name, p_ident );
    KSymbol* ret = KSymTableFind ( & m_symtab, & name );
    if ( ret == 0 && p_reportUnknown )
    {
        ReportError ( "Undeclared identifier: '%s'", p_ident ); //TODO: add location
    }
    return ret;
}

const KSymbol*
ASTBuilder :: Resolve ( const AST_FQN& p_fqn, bool p_reportUnknown )
{
    uint32_t count = p_fqn . ChildrenCount ();
    assert ( count > 0 );
    bool ns_resolved = true;
    for ( uint32_t i = 0 ; i < count - 1; ++ i )
    {
        String name;
        StringInitCString ( & name, p_fqn . GetChild ( i ) -> GetTokenValue () );
        KSymbol *ns;
        if ( i == 0 )
        {
            ns = KSymTableFindGlobal ( & m_symtab, & name );
        }
        else
        {
            ns = KSymTableFind ( & m_symtab, & name );
        }
        if ( ns == 0 )
        {
            if ( p_reportUnknown )
            {
                ReportError ( "Namespace not found: %S", & name );
            }
            count = i;
            ns_resolved = false;
            break;
        }

        rc_t rc = 0;
        if ( rc == 0 )
        {
            rc = KSymTablePushNamespace ( & m_symtab, ns );
            if ( rc != 0 )
            {
                ReportRc ( "KSymTablePushNamespace", rc );
                count = i;
                ns_resolved = false;
                break;
            }
        }
    }

    const KSymbol * ret = 0;
    if ( ns_resolved )
    {
        ret = Resolve ( p_fqn . GetChild ( count - 1 ) -> GetTokenValue (), p_reportUnknown );
    }

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
            ReportError ( "Unsupported in const expressions: %s%d", "", expr -> var ); // dont want the rc_t overload!
            break;
        }
        SExpressionWhack ( expr );
    }
    return ret;
}

const void *
ASTBuilder :: SelectVersion ( const KSymbol & p_ovl, int64_t ( CC * p_cmp ) ( const void *item, const void *n ), uint32_t * p_version )
{
    const SNameOverload *name = static_cast < const SNameOverload * > ( p_ovl . u . obj );
    assert ( p_version != 0 );
    const void * ret = 0;
    if ( * p_version != 0 )
    {
        ret = VectorFind ( & name -> items, p_version, NULL, p_cmp );
        if ( ret == 0 )
        {
            ReportError ( "Requested version does not exist: '%S#%V'", & p_ovl . name, * p_version );
        }
    }
    else
    {
        ret = VectorLast ( & name -> items );
    }
    return ret;
}

void
ASTBuilder :: DeclareType ( const AST_FQN& p_fqn, const KSymbol& p_super, const AST_Expr* p_dimension )
{
    /* capture supertype */
    assert ( p_super . type == eDatatype );
    const SDatatype * super = static_cast < const SDatatype * > ( p_super . u . obj );

    /* allocate a datatype */
    SDatatype * dt = Alloc < SDatatype > ();
    if ( dt != 0 )
    {
        /* insert into type vector */
        if ( VectorAppend ( m_schema -> dt, & dt -> id, dt ) )
        {
            // create a symtab entry
            const KSymbol* symbol = CreateFqnSymbol ( p_fqn, eDatatype, dt ); // will add missing namespaces to symtab
            if ( symbol != 0 )
            {
                /* fill it out from super-type */
                dt -> super     = super;
                dt -> byte_swap = super -> byte_swap;
                dt -> name      = symbol;
                uint32_t dimension = p_dimension == 0 ? 1 : EvalConstExpr ( * p_dimension );
                dt -> size      = super -> size * dimension;
                dt -> dim       = dimension;
                dt -> domain    = super -> domain;
            }
        }
        else
        {
            SDatatypeWhack ( dt, 0 );
        }
    }
}

static
void
TypeExprInit ( STypeExpr & p_expr )
{
    p_expr . dad . var = eTypeExpr;
    atomic32_set ( & p_expr . dad . refcount, 1 );
    p_expr . fmt = 0;
    p_expr . dt = 0;
    p_expr . ts = 0;
    p_expr . id = 0;
    p_expr . dim = 0;
    p_expr . fd . fmt = 0;
    p_expr . resolved = true;
}

static
void
TypeExprFillTypeId ( ASTBuilder & p_builder, STypeExpr & p_expr, const KSymbol & p_sym )
{
    switch ( p_sym . type )
    {
    case eDatatype:
        p_expr . dt                   = static_cast < const SDatatype * > ( p_sym . u . obj );
        p_expr . fd . td . type_id    = p_expr . dt -> id;
        break;
    case eTypeset:
        p_expr . ts                   = static_cast < const STypeset * > ( p_sym . u . obj );
        p_expr . fd . td . type_id    = p_expr . ts -> id;
        break;
    case eFormat:
        p_expr . fmt                  = static_cast < const SFormat * > ( p_sym . u . obj );
        p_expr . fd . fmt             = p_expr . fmt -> id;
        break;
    case eSchemaType:
        p_expr . id                   = static_cast < const SIndirectType * > ( p_sym . u . obj );
        p_expr . fd . td . type_id    = p_expr . id -> id;
        p_expr . resolved             = false;
        break;
    default:
        p_builder . ReportError ( "Not a datatype: '%S'", & p_sym . name );
        break;
    }
}

STypeExpr *
ASTBuilder :: MakeTypeExpr ( const AST & p_type )
{
    STypeExpr * ret = Alloc < STypeExpr > ();
    if ( ret == 0 )
    {
        return 0;
    }
    TypeExprInit ( * ret );

    const AST_FQN * fqn = 0;
    switch ( p_type . GetTokenType () )
    {
    case PT_IDENT : // scalar
        {
            fqn = dynamic_cast < const AST_FQN * > ( & p_type );
            ret -> fd . td . dim = 1;
        }
        break;
    case PT_ARRAY : // fqn [ const-expr | * ]
        {
            const AST & arrayType = p_type;
            assert ( arrayType . ChildrenCount () == 2 );
            fqn = dynamic_cast < const AST_FQN * > ( arrayType . GetChild ( 0 ) );
            const AST & dimension = * arrayType . GetChild ( 1 );
            if ( dimension . GetTokenType() == PT_EMPTY )
            {
                ret -> fd . td . dim = 0;
            }
            else
            {
                const AST_Expr & dimExpr = dynamic_cast < const AST_Expr & > ( dimension );
                SExpression * expr = dimExpr . MakeExpression ( * this ); // will report problems
                if ( expr != 0 )
                {
                    switch ( expr -> var )
                    {
                    case eConstExpr :
                        {
                            SConstExpr* cexpr = reinterpret_cast < SConstExpr* > ( expr );
                            // this may change as more kinds of const expressions are supported
                            assert ( cexpr -> td . type_id = IntrinsicTypeId ( "U64" ) );
                            ret -> fd . td . dim = cexpr -> u . u64 [ 0 ];
                        }
                        SExpressionWhack ( expr );
                        break;
                    case eIndirectExpr:
                        {
                            ret -> fd . td . dim = 0;
                            ret -> dim = expr;
                            ret -> resolved = false;
                            break;
                        }
                    default:
                        ReportError ( "Not allowed in array subscripts: %s%d", "", expr -> var ); // dont want the rc_t overload!
                        SExpressionWhack ( expr );
                        break;
                    }
                }
            }
        }
        break;
    case PT_TYPEEXPR :  // fqn (format) / fqn (type)
        {
            fqn = dynamic_cast < const AST_FQN * > ( p_type . GetChild ( 0 ) );
            const KSymbol * fmt = Resolve ( * fqn ); // will report unknown name
            if ( fmt -> type != eFormat )
            {
                ReportError ( "Not a format", *fqn );
                fqn = 0;
            }
            else
            {
                ret -> fmt = static_cast < const SFormat * > ( fmt -> u . obj );
                ret -> fd . fmt = ret -> fmt -> id;
                ret -> fd . td . dim = 1;

                fqn = dynamic_cast < const AST_FQN * > ( p_type . GetChild ( 1 ) ); // has to be a type!
            }
        }
        break;
    default:
        assert ( false ); // should not happen
        break;
    }

    if ( fqn != 0 )
    {
        const KSymbol * type = Resolve ( * fqn ); // will report unknown name
        if ( type != 0 )
        {
            TypeExprFillTypeId ( * this, * ret, * type );
        }
    }

    return ret;
}

bool
ASTBuilder :: FillSchemaParms ( const AST & p_parms, Vector & p_v )
{
    uint32_t count = p_parms . ChildrenCount ();
    for ( uint32_t i = 0; i < count; ++ i )
    {
        const AST & parm = * p_parms . GetChild ( i );
        switch ( parm . GetTokenType () )
        {
        case PT_IDENT :
            {
                assert ( parm . ChildrenCount () == 1 );
                const KSymbol * sym = Resolve ( parm. GetChild ( 0 ) -> GetTokenValue () ); // will report unknown name
                if ( sym == 0 )
                {
                    return false;
                }

                switch ( sym -> type )
                {
                /* type parameter */
                case eFormat:
                case eDatatype:
                case eTypeset:
                case eSchemaType:
                    {
                        STypeExpr * ret = Alloc < STypeExpr > ();
                        if ( ret == 0 )
                        {
                            return false;
                        }
                        TypeExprInit ( * ret );
                        TypeExprFillTypeId ( * this, * ret, * sym );
                        if ( ! VectorAppend ( p_v, 0, ret ) )
                        {
                            SExpressionWhack ( & ret -> dad );
                            return false;
                        }
                    }
                    break;

                case eConstant:
                    {
                        const SConstant * cnst = static_cast < const SConstant * > ( sym -> u . obj );
                        assert ( cnst -> expr != NULL );
                        const SDatatype *dt = VSchemaFindTypeid ( m_schema, cnst -> td . type_id );
                        assert ( dt != 0 );
                        if ( dt -> domain == ddUint && dt -> dim == 1 )
                        {
                            atomic32_inc ( & ( ( SExpression* ) cnst -> expr ) -> refcount );
                            if ( ! VectorAppend ( p_v, 0, cnst -> expr ) )
                            {
                                atomic32_dec ( & ( ( SExpression* ) cnst -> expr ) -> refcount );
                                return false;
                            }
                        }
                        else
                        {
                            ReportError ( "Schema argument constant has to be an unsigned integer scalar: '%S'",
                                            & sym -> name );
                            return false;
                        }
                    }
                    break;

                /* schema or factory constant must be uint
                but may not yet be completely resolved */
                case eSchemaParam:
                case eFactParam:
                    //return indirect_const_expr ( tbl, src, t, env, self, v );
                    assert ( false );
                    break;

                default:
                    ReportError ( "Cannot be used as a schema parameter: '%S'", & sym -> name );
                    return false;
                }
            }
            break;

        case PT_UINT :
            VectorAppend ( p_v, 0, dynamic_cast < const AST_Expr & > ( parm ) . MakeUnsigned ( * this ) );
            break;

        case PT_ARRAY:
            assert ( false );  //TODO
            break;

        default:
            assert ( false );
        }
    }
    return true;
}

bool
ASTBuilder :: FillFactoryParms ( const AST & p_parms, Vector & p_v )
{
    uint32_t count = p_parms . ChildrenCount ();
    for ( uint32_t i = 0; i < count; ++ i )
    {
        const AST_Expr * parm = dynamic_cast < const AST_Expr * > ( p_parms . GetChild ( i ) );
        assert ( parm != 0 );
        SExpression * expr = parm -> MakeExpression ( * this );
        if ( expr != 0 )
        {
            // allowed:
            // eConstExpr, eIndirectExpr, eVectorExpr, eCastExpr, eFuncParamExpr, eNegateExpr
            switch ( expr -> var )
            {
            case eConstExpr:
            case eIndirectExpr:
            case eVectorExpr:
            case eCastExpr:
            case eFuncParamExpr:
            case eNegateExpr:
                if ( ! VectorAppend ( p_v, 0, expr ) )
                {
                    SExpressionWhack ( expr );
                    return false;
                }
                break;
            default:
                ReportError ( "Cannot be used as a factory parameter" ); // TODO: identify the culprit
                break;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool
ASTBuilder :: FillArguments ( const AST & p_parms, Vector & p_v )
{
    uint32_t count = p_parms . ChildrenCount ();
    for ( uint32_t i = 0; i < count; ++ i )
    {
        const AST_Expr * parm = dynamic_cast < const AST_Expr * > ( p_parms . GetChild ( i ) );
        assert ( parm != 0 );
        // allowed tags: PT_AT, PHYSICAL_IDENTIFIER_1_0, PT_CAST, PT_IDENT
        // for PT_IDENT, allowed object types: eFuncParam, eProduction, eIdent, eForward, eVirtual, eColumn, ePhysMember
        SExpression * expr = 0;
        switch ( parm -> GetTokenType () )
        {
        case PT_AT:
        case PT_IDENT:
        case PHYSICAL_IDENTIFIER_1_0:
            expr = parm -> MakeExpression ( * this );
            break;
        default:
            ReportError ( "Cannot be used as a function call parameter" ); // TODO: identify the culprit
            break;
        }
        if ( expr == 0 )
        {
            return false;
        }
        if ( ! VectorAppend ( p_v, 0, expr ) )
        {
            SExpressionWhack ( expr );
            return false;
        }
    }
    return true;
}

/*--------------------------------------------------------------------------
 * STypesetMbr
 *  a typedecl that can be tested for uniqueness
 */
typedef struct STypesetMbr STypesetMbr;
struct STypesetMbr
{
    BSTNode n;
    VTypedecl td;
};

static
void CC STypesetPopulate ( BSTNode *n, void *data )
{
    const STypesetMbr *mbr = ( const STypesetMbr* ) n;
    STypeset *ts = static_cast < STypeset * > ( data );
    ts -> td [ ts -> count ++ ] = mbr -> td;
}

/* Cmp
 * Sort
 */
static
int64_t VTypedeclCmp ( const VTypedecl *a, const VTypedecl *b )
{
    if ( a -> type_id != b -> type_id )
        return ( int64_t ) a -> type_id - ( int64_t ) b -> type_id;
    return ( int64_t ) a -> dim - ( int64_t ) b -> dim;
}

static
int64_t CC STypesetMbrSort ( const BSTNode *item, const BSTNode *n )
{
    const STypesetMbr *a = ( const STypesetMbr* ) item;
    const STypesetMbr *b = ( const STypesetMbr* ) n;
    return VTypedeclCmp ( & a -> td, & b -> td );
}

void
ASTBuilder :: DeclareTypeSet ( const AST_FQN & p_fqn, const BSTree & p_types, uint32_t p_typeCount )
{
    STypeset *ts = Alloc < STypeset > ( sizeof * ts - sizeof ts -> td + p_typeCount * sizeof ts -> td [ 0 ] );
    if ( ts != 0 )
    {
        ts -> count = 0;
        BSTreeForEach ( & p_types, false, STypesetPopulate, ts );

        if ( VectorAppend ( m_schema -> ts, & ts -> id, ts ) )
        {
            const KSymbol* symbol = CreateFqnSymbol ( p_fqn, eTypeset, ts ); // will add missing namespaces to symtab
            ts -> name = symbol;
            ts -> count = p_typeCount;
        }
        else
        {
            STypesetWhack ( ts, 0 );
        }
    }
}

AST *
ASTBuilder :: TypeDef ( const Token * p_token, AST_FQN* p_baseType, AST* p_newTypes )
{   //TODO: do we need to keep all these subtrees beyond the population of symtab?
    AST * ret = new AST ( p_token, p_baseType, p_newTypes );

    const KSymbol * baseType = Resolve ( * p_baseType ); // will report unknown name
    if ( baseType != 0 )
    {
        if ( baseType -> type != eDatatype )
        {
            ReportError ( "Not a datatype", p_baseType -> GetChild ( 0 ) -> GetTokenValue () ); //TODO: add location
            //TODO: recover? pretend it is "any"?
        }
        else
        {
            uint32_t count = p_newTypes -> ChildrenCount ();
            for ( uint32_t i = 0; i < count; ++i )
            {
                const AST * newType = p_newTypes -> GetChild ( i );
                if ( newType -> GetTokenType () == PT_IDENT )
                {
                    const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( * newType );
                    DeclareType ( fqn, * baseType, 0 ); // will report duplicate definition
                }
                else // fqn [ const-expr ]
                {
                    assert ( newType -> ChildrenCount () == 2 );
                    const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( * newType -> GetChild ( 0 ) );
                    const AST_Expr & dim = dynamic_cast < const AST_Expr & > ( * newType -> GetChild ( 1 ) );
                    DeclareType ( fqn, * baseType, & dim ); // will report duplicate definition
                }
            }
        }
    }
    return ret;
}

static
bool
TypeSetAddType ( ASTBuilder & p_builder, BSTree & p_tree, const VTypedecl & p_type, uint32_t & p_typeCount )
{
    STypesetMbr * mbr = p_builder . Alloc < STypesetMbr > ();
    if ( mbr == 0 )
    {
        return false;
    }

    mbr -> td = p_type;

    /* ignore/allow duplicates */
    BSTNode * exist;
    if ( BSTreeInsertUnique ( & p_tree, & mbr -> n, & exist, STypesetMbrSort ) != 0 )
    {
        free ( mbr );
    }
    else
    {
        ++ p_typeCount;
    }
    return true;
}

const KSymbol *
ASTBuilder :: TypeSpec ( const AST & p_spec, VTypedecl & p_td )
{
    const KSymbol * ret = 0;
    if ( p_spec . GetTokenType () == PT_IDENT )
    {   // scalar
        const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( p_spec );
        ret = Resolve ( fqn ); // will report unknown name
        if ( ret != 0 )
        {
            switch ( ret -> type )
            {
            case eDatatype:
                {
                    const SDatatype * typeDef = static_cast < const SDatatype * > ( ret -> u . obj );
                    p_td . type_id = typeDef -> id;
                    p_td . dim = 1;
                }
                break;
            case eTypeset:
                {
                    const STypeset * typeset = static_cast < const STypeset * > ( ret -> u . obj );
                    p_td . type_id = typeset -> id;
                    p_td . dim = 1;
                }
                break;
            default:
                {
                    ReportError ( "Not a datatype", fqn );
                }
                return 0;
            }
        }
    }
    else // fqn [ const-expr ]
    {
        assert ( p_spec . GetTokenType () == PT_ARRAY );
        assert ( p_spec . ChildrenCount () == 2 ); // fqn expr
        const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( * p_spec . GetChild ( 0 ) );
        const AST_Expr & dim = dynamic_cast < const AST_Expr & > ( * p_spec . GetChild ( 1 ) );
        ret = Resolve ( fqn ); // will report unknown name
        if ( ret != 0 )
        {
            if ( ret -> type != eDatatype )
            {
                ReportError ( "Not a datatype", fqn );
                return 0;
            }
            const SDatatype * typeDef = static_cast < const SDatatype * > ( ret -> u . obj );
            p_td . type_id    = typeDef -> id;
            p_td . dim        = EvalConstExpr ( dim );
        }
    }
    return ret;
}

AST *
ASTBuilder :: TypeSet ( const Token* p_token, AST_FQN * p_name, AST * p_typeSpecs )
{
    AST * ret = new AST ( p_token, p_name, p_typeSpecs );

    const KSymbol * existing = Resolve ( * p_name, false );

    // traverse p_typeSpecs, add to tree
    BSTree tree;
    BSTreeInit ( & tree );

    uint32_t typeCount = 0;
    uint32_t count = p_typeSpecs -> ChildrenCount ();
    bool error = false;
    for ( uint32_t i = 0; i < count; ++i )
    {
        const AST * spec = p_typeSpecs -> GetChild ( i );
        VTypedecl td;
        const KSymbol * type = TypeSpec ( * spec, td );
        if ( type != 0  )
        {
            if ( type -> type == eDatatype )
            {
                if ( ! TypeSetAddType ( * this, tree, td, typeCount ) )
                {
                    error = true;
                }
            }
            else
            {
                assert ( type -> type == eTypeset );
                const STypeset * typeset = static_cast < const STypeset * > ( type -> u . obj );
                for ( uint16_t j = 0; j < typeset -> count; ++j )
                {
                    if ( ! TypeSetAddType ( *this, tree, typeset -> td [ j ], typeCount ) )
                    {
                        error = true;
                    }
                }
            }
        }
        else
        {
            error = true;
        }
    }

    if ( ! error && existing != 0 )
    {
        if ( existing -> type != eTypeset )
        {
            ReportError ( "Already declared and is not a typeset", * p_name );
        }
        else
        {   // allow benign redefine
            const STypeset * orig = static_cast < const STypeset * > ( existing -> u . obj );
            if ( orig -> count != typeCount )
            {
                ReportError ( "Typeset already declared differently", * p_name );
            }
            else
            {
                BSTNode* node = BSTreeFirst ( &tree );
                for ( uint32_t i = 0; i < typeCount; ++ i )
                {
                    assert ( node != 0 );
                    STypesetMbr * mbr = reinterpret_cast < STypesetMbr * > ( node );
                    if ( VTypedeclCmp ( & orig -> td [ i ], & mbr -> td ) != 0 )
                    {
                        ReportError ( "Typeset already declared differently", * p_name );
                        break;
                    }
                    node = BSTNodeNext ( node );
                }
            }
        }
    }
    else
    {
        DeclareTypeSet ( * p_name, tree, typeCount );
    }

    BSTreeWhack ( & tree, BSTreeMbrWhack, 0 );

    return ret;
}

AST *
ASTBuilder :: FmtDef ( const Token* p_token, AST_FQN* p_fqn, AST_FQN* p_super_opt )
{
    AST * ret = new AST ( p_token, p_fqn );
    if ( p_super_opt != 0 )
    {
        ret -> AddNode ( p_super_opt );
    }

    SFormat * fmt = Alloc < SFormat > ();
    if ( fmt != 0 )
    {
        // superfmt
        fmt -> super = 0;
        if ( p_super_opt != 0 )
        {
            const KSymbol* super = Resolve ( * p_super_opt ); // will report undefined
            if ( super != 0 )
            {
                if ( super -> type != eFormat )
                {
                    ReportError ( "Not a format", * p_super_opt );
                    SFormatWhack ( fmt, 0 );
                    return ret;
                }
                fmt -> super = static_cast < const SFormat * > ( super -> u . obj );
            }
        }

        /* insert into vector */
        if ( VectorAppend ( m_schema -> fmt, & fmt -> id, fmt ) )
        {   // create a symtab entry, link fmt to it
            fmt -> name = CreateFqnSymbol ( * p_fqn, eFormat, fmt ); // will add missing namespaces to symtab
        }
        else
        {
            SFormatWhack ( fmt, 0 );
        }
    }

    return ret;
}

AST *
ASTBuilder :: ConstDef  ( const Token* p_token, AST* p_type, AST_FQN* p_fqn, AST_Expr* p_expr )
{
    AST * ret = new AST ( p_token, p_type, p_fqn, p_expr );

    SConstant *cnst = Alloc < SConstant > ();
    if ( cnst != 0 )
    {
        if ( p_type -> GetTokenType () == PT_IDENT )
        {   // scalar
            const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( * p_type );
            const KSymbol * type = Resolve ( fqn ); // will report unknown name
            if ( type != 0 )
            {
                if ( VectorAppend ( m_schema -> cnst, & cnst -> id, cnst ) )
                {
                    cnst -> name = CreateFqnSymbol ( * p_fqn, eConstant, cnst );
                    cnst -> expr = p_expr -> EvaluateConst ( *this ); // will report problems
                    const SDatatype * typeDef = static_cast < const SDatatype * > ( type -> u . obj );
                    cnst -> td . type_id = typeDef -> id;
                    cnst -> td . dim = 1;
                }
                else
                {
                    SConstantWhack ( cnst, 0 );
                }
            }
        }
        else // fqn dim
        {
            //TODO - use ArraySpec()
        }
    }

    return ret;
}

AST *
ASTBuilder :: AliasDef  ( const Token* p_token, AST_FQN* p_name, AST_FQN* p_newName )
{
    AST * ret = new AST ( p_token, p_name, p_newName );

    const KSymbol * sym = Resolve ( * p_name ); // will report unknown name
    if ( sym != 0 )
    {
        VectorAppend ( m_schema -> alias, 0, CreateFqnSymbol ( * p_newName, sym -> type, sym -> u . obj ) );
    }

    return ret;
}

void
ASTBuilder :: AddIncludePath ( const char * path )
{
    rc_t rc = VSchemaAddIncludePath ( m_schema, "%s", path );
    if ( rc != 0 )
    {
        ReportRc ( "VSchemaAddIncludePath", rc );
    }
}

const KFile *
ASTBuilder :: OpenIncludeFile ( const char * p_fmt, ... )
{
    const KFile * ret = 0;
    va_list args;
    va_start ( args, p_fmt );

    char path [ 4096 ];
    rc_t rc = 0;
    /* open file using include paths */
    rc = VSchemaOpenFile ( m_schema, & ret, path, sizeof path, p_fmt, args );

    if ( rc != 0 )
    {   /* try to open the file according to current directory */
        KDirectory *wd;
        rc = KDirectoryNativeDir ( & wd );
        if ( rc == 0 )
        {
            rc = VSchemaTryOpenFile ( m_schema, wd, & ret, path, sizeof path, p_fmt, args );
            // ret == 0 if was included previously
            KDirectoryRelease ( wd );
        }
        else
        {
            ReportRc ( "KDirectoryNativeDir", rc );
        }
    }

    // if the file was found ...
    if ( rc == 0 )
    {
        if ( ret != 0 ) // file was included for the 1st time
        {
            const KMMap *mm;
            rc = KMMapMakeRead ( & mm, ret );
            if ( rc == 0 )
            {
                size_t size;
                const void *addr;
                rc = KMMapAddrRead ( mm, & addr );
                if ( rc == 0 )
                {
                    rc = KMMapSize ( mm, & size );
                    if ( rc == 0 )
                    {
                        rc = VIncludedPathMake ( & m_schema -> paths, & m_schema -> file_count, path );
                        if ( rc != 0 )
                        {
                            ReportRc ( "VIncludedPathMake", rc );
                        }
                    }
                    else
                    {
                        ReportRc ( "KMMapSize", rc );
                    }
                }
                else
                {
                    ReportRc ( "KMMapAddrRead", rc );
                }

                KMMapRelease ( mm );
            }
            else
            {
                ReportRc ( "KMMapMakeRead", rc );
            }
        }
        // else: file was included previously, now ignored
    }
    else
    {
        ReportError ( "Could not open include file '%s'", path );
    }

    va_end ( args );
    return ret;
}

AST *
ASTBuilder :: Include ( const Token * p_token, const Token * p_filename )
{
    AST * ret = new AST ( p_token );
    assert ( p_filename != 0 );
    ret -> AddNode ( p_filename );

    const char * quoted = p_filename -> GetValue ();
    const KFile * f = OpenIncludeFile ( "%.*s", string_size ( quoted ) - 2, quoted + 1 );
    if ( f != 0 )
    {
        SchemaParser parser;
        if ( parser . ParseFile ( f ) )
        {
            delete Build ( * parser . GetParseTree (), false );
        }
        KFileRelease ( f );
    }

    return ret;
}
