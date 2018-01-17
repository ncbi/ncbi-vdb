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

#include <kfs/directory.h>
#include <kfs/mmap.h>

#include "../vdb/schema-parse.h"
#include "../vdb/dbmgr-priv.h"
#include "../vdb/schema-expr.h"

#include "SchemaParser.hpp"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast-tokens.h"

using namespace std;

// data types, constants, etc.

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
                uint32_t dimension = p_dimension == 0 ? 1 : ( uint32_t ) EvalConstExpr ( * p_dimension );
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
TypeExprFillTypeId ( ASTBuilder & p_builder, const AST & p_node, STypeExpr & p_expr, const KSymbol & p_sym )
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
        p_builder . ReportError ( p_node . GetLocation (), "Not a datatype", p_sym . name );
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
            fqn = ToFQN ( & p_type );
            ret -> fd . td . dim = 1;
        }
        break;
    case PT_ARRAY : // fqn [ const-expr | * ]
        {
            const AST & arrayType = p_type;
            assert ( arrayType . ChildrenCount () == 2 );
            fqn = ToFQN ( arrayType . GetChild ( 0 ) );
            const AST & dimension = * arrayType . GetChild ( 1 );
            if ( dimension . GetTokenType() == PT_EMPTY )
            {
                ret -> fd . td . dim = 0;
            }
            else
            {
                SExpression * expr = ToExpr ( & dimension ) -> MakeExpression ( * this ); // will report problems
                if ( expr != 0 )
                {
                    switch ( expr -> var )
                    {
                    case eConstExpr :
                        {
                            SConstExpr* cexpr = reinterpret_cast < SConstExpr* > ( expr );
                            // this may change as more kinds of const expressions are supported
                            assert ( cexpr -> td . type_id == IntrinsicTypeId ( "U64" ) );
                            ret -> fd . td . dim = ( uint32_t ) cexpr -> u . u64 [ 0 ];
                            ret -> dim = expr;
                        }
                        break;
                    case eIndirectExpr:
                        {
                            ret -> fd . td . dim = 0;
                            ret -> dim = expr;
                            ret -> resolved = false;
                            break;
                        }
                    default:
                        ReportError ( dimension . GetLocation (), "Not allowed in array subscripts", expr -> var );
                        SExpressionWhack ( expr );
                        break;
                    }
                }
            }
        }
        break;
    case PT_TYPEEXPR :  // fqn (format) / fqn (type)
        {
            fqn = ToFQN ( p_type . GetChild ( 0 ) );
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

                fqn = ToFQN ( p_type . GetChild ( 1 ) ); // has to be a type!
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
            TypeExprFillTypeId ( * this, * fqn, * ret, * type );
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
                const KSymbol * sym = Resolve ( * ToFQN ( & parm ) ); // will report unknown name
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
                        ret -> fd . td . dim = 1;
                        TypeExprFillTypeId ( * this, parm, * ret, * sym );
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
                            ReportError ( parm . GetLocation (),
                                          "Schema argument constant has to be an unsigned integer scalar",
                                          sym -> name );
                            return false;
                        }
                    }
                    break;

                /* schema or factory constant must be uint
                but may not yet be completely resolved */
                case eSchemaParam:
                case eFactParam:
                    //TODO: return indirect_const_expr ( tbl, src, t, env, self, v );
                    assert ( false );
                    break;

                default:
                    ReportError ( parm. GetChild ( 0 ) -> GetLocation (), "Cannot be used as a schema parameter", sym -> name );
                    return false;
                }
            }
            break;

        case PT_UINT :
            VectorAppend ( p_v, 0, ToExpr ( & parm ) -> MakeUnsigned ( * this ) );
            break;

        case PT_ARRAY:
            VectorAppend ( p_v, 0, MakeTypeExpr ( parm ) );
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
        SExpression * expr = ToExpr ( p_parms . GetChild ( i ) ) -> MakeExpression ( * this );
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
                ReportError ( p_parms . GetChild ( i ) -> GetLocation (), "Cannot be used as a factory parameter" );
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
        const AST_Expr * parm = ToExpr ( p_parms . GetChild ( i ) );
        // allowed tags: PT_AT, PHYSICAL_IDENTIFIER_1_0, PT_CAST, PT_IDENT, PT_MEMBEREXPR
        // for PT_IDENT, allowed object types: eFuncParam, eProduction, eIdent, eForward, eVirtual, eColumn, ePhysMember
        SExpression * expr = 0;
        switch ( parm -> GetTokenType () )
        {
        case '@':
        case PT_IDENT:
        case PHYSICAL_IDENTIFIER_1_0:
        case PT_MEMBEREXPR:
            expr = parm -> MakeExpression ( * this );
            break;
        default:
            ReportError ( p_parms . GetChild ( i ) -> GetLocation (), "Cannot be used as a function call parameter" );
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
            ReportError ( "Not a datatype", * p_baseType );
        }
        else
        {
            uint32_t count = p_newTypes -> ChildrenCount ();
            for ( uint32_t i = 0; i < count; ++i )
            {
                const AST * newType = p_newTypes -> GetChild ( i );
                if ( newType -> GetTokenType () == PT_IDENT )
                {
                    DeclareType ( * ToFQN ( newType ), * baseType, 0 ); // will report duplicate definition
                }
                else // fqn [ const-expr ]
                {
                    assert ( newType -> ChildrenCount () == 2 );
                    DeclareType ( * ToFQN ( newType -> GetChild ( 0 ) ),
                                  * baseType,
                                  ToExpr ( newType -> GetChild ( 1 ) ) ); // will report duplicate definition
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
        const AST_FQN & fqn = * ToFQN ( & p_spec );
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
        const AST_FQN & fqn = * ToFQN ( p_spec . GetChild ( 0 ) );
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
            p_td.dim = (uint32_t) EvalConstExpr(*ToExpr(p_spec.GetChild(1)));
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
            const KSymbol * type = Resolve ( * ToFQN ( p_type ) ); // will report unknown name
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
        else // fqn [ const-expr ]
        {
            assert ( p_type -> GetTokenType () == PT_ARRAY );
            assert ( p_type -> ChildrenCount () == 2 ); // fqn expr
            const AST_FQN & fqn = * ToFQN ( p_type -> GetChild ( 0 ) );
            const KSymbol * sym = Resolve ( fqn ); // will report unknown name
            if ( sym != 0 )
            {
                if ( sym -> type != eDatatype )
                {
                    ReportError ( "Not a datatype", fqn );
                    return 0;
                }
                if ( VectorAppend ( m_schema -> cnst, & cnst -> id, cnst ) )
                {
                    cnst -> name = CreateFqnSymbol ( * p_fqn, eConstant, cnst );
                    cnst -> expr = p_expr -> EvaluateConst ( *this ); // will report problems
                    const SDatatype * typeDef = static_cast < const SDatatype * > ( sym -> u . obj );
                    cnst -> td . type_id    = typeDef -> id;
                    cnst->td.dim = (uint32_t) EvalConstExpr(*ToExpr(p_type->GetChild(1)));
                }
            }
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
ASTBuilder :: OpenIncludeFile ( const Token :: Location & p_loc, const char * p_fmt, ... )
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
        ReportError ( p_loc, "Could not open include file", path );
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
    char * unquoted = string_dup ( quoted + 1, string_size ( quoted ) - 2 );
    if ( unquoted != 0 )
    {
        const KFile * f = OpenIncludeFile ( p_token -> GetLocation (), "%s", unquoted );
        if ( f != 0 )
        {
            SchemaParser parser;
            if ( parser . ParseFile ( f, unquoted ) )
            {
                delete Build ( * parser . GetParseTree (), unquoted, false );
            }
            KFileRelease ( f );
        }
        free ( unquoted );
    }
    else
    {
        ReportInternalError ( "ASTBuilder :: Include () : string_dup() failed" );
    }

    return ret;
}