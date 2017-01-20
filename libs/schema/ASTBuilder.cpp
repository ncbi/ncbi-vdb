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

// hide an unfortunately named C function typename()
#define typename __typename
#include "../vdb/schema-parse.h"
#undef typename
#include "../vdb/dbmgr-priv.h"
#include "../vdb/schema-priv.h"
#include "../vdb/schema-expr.h"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast-tokens.h"

using namespace std;

ASTBuilder :: ASTBuilder ()
:   m_debug ( false ),
    m_intrinsic ( 0 ),
    m_schema ( 0 )
{
    VectorInit ( & m_errors, 0, 1024 );

    rc_t rc = VSchemaMakeIntrinsic ( & m_intrinsic );
    if ( rc != 0 )
    {
        ReportError ( "VSchemaMakeIntrinsic failed: %R", rc  );
        m_intrinsic = 0;
    }
    else
    {
        rc = KSymTableInit ( & m_symtab, 0 );
        if ( rc != 0 )
        {
            ReportError ( "KSymTableInit failed: %R", rc );
            VSchemaRelease ( m_intrinsic );
            m_intrinsic = 0;
        }
    }
}


void
ASTBuilder :: DebugOn ()
{
    m_debug = true;
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
    VSchemaRelease ( m_intrinsic );
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

bool
ASTBuilder :: Init()
{
    if ( m_schema != 0 )    // in case we are called more than once
    {
        KSymTableWhack ( & m_symtab);
        VSchemaRelease ( m_schema );
    }
    rc_t rc = VSchemaMake ( & m_schema, m_intrinsic );
    if ( rc != 0 )
    {
        ReportError ( "VSchemaMake failed: %R", rc );
        m_schema = 0;
        return false;
    }

    rc = init_symtab ( & m_symtab, m_schema ); // this pushes the global scope
    if ( rc != 0 )
    {
        ReportError ( "init_symtab failed: %R", rc );
        return false;
    }

    return true;
}

AST *
ASTBuilder :: Build ( const ParseTree& p_root )
{
    if ( m_intrinsic == 0 || ! Init () )
    {
        return 0;
    }

    AST* ret = 0;
    AST_debug = m_debug;
    ParseTreeScanner scanner ( p_root );
    if ( AST_parse ( ret, * this, scanner ) == 0 )
    {
        return ret;
    }

    //TODO: report error(s)
    return 0;
}

rc_t
ASTBuilder :: VectorAppend ( Vector *p_self, uint32_t *p_idx, const void *p_item )
{
    rc_t rc = :: VectorAppend ( p_self, p_idx, p_item );
    if ( rc != 0 )
    {
        ReportError ( "VectorAppend failed: %R", rc );
    }
    return rc;
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
        rc = KSymTableCreateNamespace ( & m_symtab, & ns, & name );
        if ( rc == 0 )
        {
            rc = KSymTablePushNamespace ( & m_symtab, ns );
            if ( rc != 0 )
            {
                ReportError ( "KSymTablePushNamespace failed: %R", rc );
            }
        }
        else
        {
            ReportError ( "KSymTableCreateNamespace failed: %R", rc );
        }
    }

    KSymbol* ret = 0;
    if ( rc == 0 )
    {
        String name;
        p_fqn . GetIdentifier ( name );
        rc = KSymTableCreateSymbol ( & m_symtab, & ret, & name, p_type, p_obj );
        if ( GetRCState ( rc ) == rcExists )
        {
            ReportError ( "Object already declared", p_fqn );
        }
        else if ( rc != 0 )
        {
            ReportError ( "KSymTableCreateSymbol failed: %R", rc );
        }
    }

    for ( uint32_t i = 0 ; i < count; ++ i )
    {
        KSymTablePopNamespace ( & m_symtab );
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
                ReportError ( "KSymTablePushNamespace failed: %R", rc );
                count = i;
                ns_resolved = false;
                break;
            }
        }
    }

    KSymbol * ret = 0;
    if ( ns_resolved )
    {
        const char* ident = p_fqn . GetChild ( count - 1 ) -> GetTokenValue ();
        String name;
        StringInitCString ( & name, ident );
        ret = KSymTableFind ( & m_symtab, & name );
        if ( ret == 0 && p_reportUnknown )
        {
            ReportError ( "Undeclared identifier", p_fqn ); //TODO: add location
        }
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
        assert ( expr -> var == eConstExpr);
        SConstExpr* cexpr = reinterpret_cast < SConstExpr* > ( expr );
        // this may change as more kinds of const expressions are supported
        assert ( cexpr -> td . type_id = IntrinsicTypeId ( "U64" ) );
        ret = cexpr -> u . u64 [ 0 ];
        SExpressionWhack ( expr );
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
        if ( VectorAppend ( & m_schema -> dt, & dt -> id, dt ) == 0 )
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
            free ( dt );
        }
    }
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

        if ( VectorAppend ( & m_schema -> ts, & ts -> id, ts ) == 0 )
        {
            const KSymbol* symbol = CreateFqnSymbol ( p_fqn, eTypeset, ts ); // will add missing namespaces to symtab
            ts -> name = symbol;
            ts -> count = p_typeCount;
        }
        else
        {
            free ( ts );
        }
    }
}

// AST nodes

AST_Schema :: AST_Schema ()
:   m_version ( 0 )
{
}


AST_Schema :: AST_Schema ( const Token * p_token, AST* p_decls /*NULL OK*/ )
:   AST ( p_token ),
    m_version ( 0 )
{
    if ( p_decls != 0 )
    {
        MoveChildren ( * p_decls );
        delete p_decls;
    }
}

void
AST_Schema :: SetVersion ( const char* ) // version specified as "#maj[.min[.rel]]]"
{
    assert ( false );
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
            ReportError ( "Not a datatype: '%s'", p_baseType -> GetChild ( 0 ) -> GetTokenValue () ); //TODO: add location
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
    for ( uint32_t i = 0; i < count; ++i )
    {
        const AST * spec = p_typeSpecs -> GetChild ( i );
        if ( spec -> GetTokenType () == PT_IDENT )
        {   // scalar
            const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( * spec );
            const KSymbol * type = Resolve ( fqn ); // will report unknown name
            if ( type != 0 )
            {
                switch ( type -> type )
                {
                case eDatatype:
                    {
                        VTypedecl td;
                        const SDatatype * typeDef = static_cast < const SDatatype * > ( type -> u . obj );

                        td . type_id = typeDef -> id;
                        td . dim = 1;
                        if ( ! TypeSetAddType ( * this, tree, td, typeCount ) )
                        {
                            goto EXIT;
                        }
                    }
                    break;
                case eTypeset:
                    {
                        const STypeset * typeset = static_cast < const STypeset * > ( type -> u . obj );
                        for ( uint16_t j = 0; j < typeset -> count; ++j )
                        {
                            if ( ! TypeSetAddType ( *this, tree, typeset -> td [ j ], typeCount ) )
                            {
                                goto EXIT;
                            }
                        }
                    }
                    break;
                default:
                    {
                        ReportError ( "Not a datatype", fqn );
                    }
                    continue;
                }
            }
        }
        else // fqn [ const-expr ]
        {
            assert ( spec -> ChildrenCount () == 2 );
            const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( * spec -> GetChild ( 0 ) );
            const AST_Expr & dim = dynamic_cast < const AST_Expr & > ( * spec -> GetChild ( 1 ) );
            const KSymbol * type = Resolve ( fqn ); // will report unknown name
            if ( type != 0 )
            {
                if ( type -> type != eDatatype )
                {
                    ReportError ( "Not a datatype", fqn );
                    continue;
                }
                const SDatatype * typeDef = static_cast < const SDatatype * > ( type -> u . obj );
                VTypedecl td;
                td . type_id    = typeDef -> id;
                td . dim        = EvalConstExpr ( dim );

                if ( ! TypeSetAddType ( * this, tree, td, typeCount ) )
                {
                    goto EXIT;
                }
            }
        }
    }

    if ( existing != 0 )
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

EXIT:
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
                    free ( fmt );
                    return ret;
                }
                fmt -> super = static_cast < const SFormat * > ( super -> u . obj );
            }
        }

        /* insert into vector */
        if ( VectorAppend ( & m_schema -> fmt, & fmt -> id, fmt ) == 0 )
        {   // create a symtab entry, link fmt to it
            fmt -> name = CreateFqnSymbol ( * p_fqn, eFormat, fmt ); // will add missing namespaces to symtab
        }
        else
        {
            free ( fmt );
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
                if ( VectorAppend ( & m_schema -> cnst, & cnst -> id, cnst ) == 0 )
                {
                    cnst -> name = CreateFqnSymbol ( * p_fqn, eConstant, cnst );
                    cnst -> expr = p_expr -> EvaluateConst ( *this ); // will report problems
                    const SDatatype * typeDef = static_cast < const SDatatype * > ( type -> u . obj );
                    cnst -> td . type_id = typeDef -> id;
                    cnst -> td . dim = 1;
                }
                else
                {
                    free ( cnst );
                }
            }
        }
        else // fqn dim
        {
            //TBD
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
        VectorAppend ( & m_schema -> alias, 0, CreateFqnSymbol ( * p_newName, sym -> type, sym -> u . obj ) );
    }

    return ret;
}

void
ASTBuilder :: DeclareFunction ( const AST_FQN& p_fqn, uint32_t p_type, struct STypeExpr * p_retType )
{
    const KSymbol * priorDecl = Resolve ( p_fqn, false );
    if ( priorDecl != 0 && priorDecl -> type != p_type )
    {
        ReportError ( "Declared earlier with a different type", p_fqn );
    }
    else
    {
        SFunction *f = Alloc < SFunction > ();
        if ( f != 0 )
        {
            memset ( f, 0, sizeof * f );
            if ( p_retType != 0 )
            {
                f -> rt = & p_retType -> dad;
            }
            VectorInit ( & f -> fact . parms, 0, 8 );
            VectorInit ( & f -> func . parms, 0, 8 );
            VectorInit ( & f -> type, 0, 8 );
            f -> version = p_fqn . GetVersion ();
            VectorInit ( & f -> schem, 0, 8 );

            if ( priorDecl == 0 )
            {
                const KSymbol * sym = CreateFqnSymbol ( p_fqn, p_type, f );
                f -> name = sym;

                SNameOverload *name;
                rc_t rc = SNameOverloadMake ( & name, f -> name, 0, 4 );
                if ( rc == 0 )
                {
                    rc = VectorInsertUnique ( & name -> items, f, 0, SFunctionSort );
                    if ( rc == 0 )
                    {
                        if ( VectorAppend ( & m_schema -> func, & f -> id, f ) == 0 )
                        {
                            if ( VectorAppend ( & m_schema -> fname, & name -> cid . id, name ) == 0 )
                            {
                                return;
                            }
                        }
                    }
                    else
                    {
                        ReportError ( "VectorInsertUnique failed: %R", rc );
                    }
                    SNameOverloadWhack ( name, 0 );
                }
                else
                {
                    ReportError ( "SNameOverloadMake failed: %R", rc );
                }
            }
            else // declared previously
            {
                f -> name = priorDecl;

                SNameOverload *name = ( SNameOverload* ) f -> name -> u . obj;
                assert ( name != 0 );
                uint32_t idx;
                rc_t rc = VectorInsertUnique ( & name -> items, f, & idx, SFunctionSort );
                if ( rc == 0 ) // overload added
                {
                    if ( VectorAppend ( & m_schema -> func, & f -> id, f ) == 0 )
                    {
                        return;
                    }
                }
                if ( GetRCState ( rc ) == rcExists )
                {   /* an overload with the same major version exists */
                    /* see if new function trumps old */
                    SFunction *exist = static_cast < SFunction * > ( VectorGet ( & name -> items, idx ) );
                    if ( f -> version > exist -> version )
                    {
                        /* insert our function in name overload */
                        void * ignore;
                        VectorSwap ( & name -> items, idx, f, & ignore );

                        /* if existing is in the same schema... */
                        if ( ( const void* ) name == exist -> name -> u . obj )
                        {
                            /* need to swap with old */
                            assert ( exist -> id >= VectorStart ( & m_schema -> func ) );
                            VectorSwap ( & m_schema -> func, exist -> id, f, & ignore );
                            f -> id = exist -> id;
                        }
                    }
                }
                else if ( rc != 0 )
                {
                    ReportError ( "VectorInsertUnique failed: %R", rc );
                }
            }
            SFunctionWhack ( f, 0 );
        }
    }
}

AST * ASTBuilder :: UntypedFunctionDecl ( const Token* p_token, AST_FQN* p_name )
{
    AST * ret = new AST ( p_token, p_name );
    DeclareFunction ( * p_name, eUntypedFunc, 0 );
    return ret;
}

AST * ASTBuilder :: RowlenFunctionDecl ( const Token* p_token, AST_FQN* p_name )
{
    AST * ret = new AST ( p_token, p_name );
    DeclareFunction ( * p_name, eRowLengthFunc, 0 );
    return ret;
}

STypeExpr *
ASTBuilder :: MakeReturnType ( const AST & p_type )
{
    STypeExpr * ret = Alloc < STypeExpr > ();
    ret -> dad . var = eTypeExpr;
    atomic32_set ( & ret -> dad . refcount, 1 );
    ret -> fmt = 0;
    ret -> dt = 0;
    ret -> ts = 0;
    ret -> id = 0;
    ret -> dim = 0;
    ret -> fd . fmt = 0;
    ret -> resolved = true;

    const AST_FQN * fqn;
    uint64_t dim;
    if ( p_type . GetChild ( 0 ) -> GetTokenType () == PT_IDENT )
    {   // scalar
        fqn = dynamic_cast < const AST_FQN * > ( p_type . GetChild ( 0 ) );
        dim = 1;
    }
    else // fqn [ const-expr | * ]
    {
        const AST & arrayType = * p_type . GetChild ( 0 );
        assert ( arrayType . GetTokenType () == PT_ARRAY );
        assert ( arrayType . ChildrenCount () == 2 );
        fqn = dynamic_cast < const AST_FQN * > ( arrayType . GetChild ( 0 ) );
        const AST & dimension = * arrayType . GetChild ( 1 );
        if ( dimension . GetTokenType() == PT_EMPTY )
        {
            dim = 0;
        }
        else
        {
            const AST_Expr & dimExpr = dynamic_cast < const AST_Expr & > ( dimension );
            dim = EvalConstExpr ( dimExpr );
        }
    }

    const KSymbol * type = Resolve ( * fqn ); // will report unknown name
    if ( type != 0 )
    {
        switch ( type -> type )
        {
        case eDatatype:
            ret -> dt                   = static_cast < const SDatatype * > ( type -> u . obj );
            ret -> fd . td . type_id    = ret -> dt -> id;
            ret -> fd . td . dim        = dim;
            break;
        case eTypeset:
        case eSchemaType:
            assert ( false );
            break;
        default:
            ReportError ( "Not a datatype", fqn );
            break;
        }
    }

    return ret;
}

AST * ASTBuilder :: FunctionDecl ( const Token* p_token,
                                   AST *        p_schema_opt,
                                   AST *        p_returnType,
                                   AST_FQN*     p_name,
                                   AST*         p_fact_opt,
                                   AST*         p_params,
                                   AST*         p_prologue )
{
    AST * ret = new AST ( p_token, p_schema_opt, p_returnType, p_name, p_fact_opt, p_params, p_prologue );

    if ( p_schema_opt -> GetTokenType () != PT_EMPTY )
    {
        assert ( false );
    }

    STypeExpr * retType = 0;
    if ( p_returnType -> ChildrenCount () == 0 ) // void
    {
        assert ( false ); // only for "validate"
    }
    else if ( p_returnType -> GetChild ( 0 ) -> GetTokenType () == PT_TYPEEXPR )
    {
        assert ( false ); // fqn / fqn
    }
    else
    {
        retType = MakeReturnType ( * p_returnType );
    }

    if ( p_fact_opt -> GetTokenType () != PT_EMPTY )
    {
        assert ( false );
    }

    if ( p_params -> ChildrenCount () != 0 )
    {
        assert ( false );
    }

    DeclareFunction ( * p_name, eFunction, retType );

    return ret;
}
