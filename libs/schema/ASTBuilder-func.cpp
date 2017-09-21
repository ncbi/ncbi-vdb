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

static
void
BSTreeMove ( BSTree & p_dest, const BSTree * p_source )
{   // p_dest is assumed to be uninitialized or empty
    if ( p_source == 0 )
    {
        BSTreeInit ( & p_dest );
    }
    else
    {
        p_dest = * p_source;
    }
    // do not whack p_source!
}

void
ASTBuilder :: SFormParmlistMove ( SFormParmlist & p_dest, SFormParmlist * p_source )
{
    if ( p_source == 0 )
    {
        VectorInit ( & p_dest . parms, 0, 8 );
    }
    else
    {
        p_dest . mand    = p_source -> mand;
        p_dest . vararg  = p_source -> vararg;
        // move formal parameters from p_factory, dispose of the original
        rc_t rc = VectorCopy ( & p_source -> parms, & p_dest . parms );
        if ( rc != 0 )
        {
            ReportError ( "VectorCopy", rc );
        }
        VectorWhack ( & p_source -> parms, 0, 0 );
        SFormParmlistWhack ( p_source, 0 );
        free ( p_source );
    }
}

void
ASTBuilder :: VectorMove ( Vector & p_dest, const Vector * p_source )
{   // p_dest is assumed to be uninitialized or empty
    if ( p_source == 0 )
    {
        VectorInit ( & p_dest, 0, 8 );
    }
    else
    {
        rc_t rc = VectorCopy ( p_source, & p_dest );
        if ( rc != 0 )
        {
            ReportError ( "VectorCopy", rc );
        }
    }
    // do not whack p_source!
}

bool
ASTBuilder :: HandleOverload ( SFunction * p_f, const KSymbol * p_priorDecl )
{
    assert ( p_f != 0 );
    assert ( p_priorDecl != 0 );

    p_f -> name = p_priorDecl;

    SNameOverload *name = ( SNameOverload* ) p_f -> name -> u . obj;
    assert ( name != 0 );
    uint32_t idx;
    rc_t rc = VectorInsertUnique ( & name -> items, p_f, & idx, SFunctionSort );
    if ( rc == 0 ) // overload added
    {
        if ( VectorAppend ( & m_schema -> func, & p_f -> id, p_f ) == 0 )
        {
            return true;
        }
    }
    if ( GetRCState ( rc ) == rcExists )
    {   /* an overload with the same major version exists */
        /* see if new function trumps old */
        SFunction *exist = static_cast < SFunction * > ( VectorGet ( & name -> items, idx ) );
        if ( p_f -> version > exist -> version )
        {
            /* insert our function in name overload */
            void * prior;
            VectorSwap ( & name -> items, idx, p_f, & prior );

            /* if existing is in the same schema... */
            if ( ( const void* ) name == exist -> name -> u . obj )
            {
                /* need to swap with old */
                assert ( exist -> id >= VectorStart ( & m_schema -> func ) );
                VectorSwap ( & m_schema -> func, exist -> id, p_f, & prior );
                p_f -> id = exist -> id;
                SFunctionWhack ( (SFunction*)prior, 0 );
            }
            return true;
        }
    }
    else if ( rc != 0 )
    {
        ReportError ( "VectorInsertUnique", rc );
    }
    return false;
}

void
ASTBuilder :: HandlePrologue ( SFunction & f, const AST & p_prologue )
{
    switch ( p_prologue . GetTokenType () )
    {
        case PT_IDENT:
        {
            const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( p_prologue );
            const KSymbol * priorDecl = Resolve ( fqn, false );
            if ( priorDecl != 0 )
            {
                if ( priorDecl -> type == eFunction || priorDecl -> type == eFactory )
                {
                    f . u. ext . fact = priorDecl;
                }
                else
                {
                    ReportError ( "Cannot be used as factory", fqn );
                }
            }
            else
            {
                f . u. ext . fact = CreateFqnSymbol ( fqn, eFactory, 0 );
            }
        }
        break;
    case PT_EMPTY:
        {
            if ( p_prologue . ChildrenCount () > 0 )
            {
                ReportError ( "Only schema functions can have body: '%S'", & f . name -> name );
            }
        }
        break;
    default:
        assert (false);
    }
}

void
ASTBuilder :: DeclareFunction ( const AST_FQN&          p_fqn,
                                uint32_t                p_type,
                                struct STypeExpr *      p_retType,
                                SFormParmlist *         p_factory,
                                SFormParmlist *         p_params,
                                const BSTree *          p_sscope,
                                const Vector *          p_stypes,
                                const Vector *          p_sparams,
                                const BSTree *          p_fscope,
                                bool                    p_canOverload,
                                const AST *             p_prologue )
{
    const KSymbol * priorDecl = Resolve ( p_fqn, false );
    if ( priorDecl != 0 && ( priorDecl -> type != p_type || ! p_canOverload ) )
    {
        free ( p_retType );
        ReportError ( "Declared earlier and cannot be overloaded", p_fqn );
        SFormParmlistWhack ( p_factory, SIndirectConstWhack );
        free ( p_factory );
        SFormParmlistWhack ( p_params, SProductionWhack );
        free ( p_params );
        return;
    }

    SFunction *f = Alloc < SFunction > ();
    if ( f != 0 )
    {
        memset ( f, 0, sizeof * f );
        if ( p_retType != 0 )
        {
            f -> rt = & p_retType -> dad;
        }

        BSTreeMove ( f -> sscope, p_sscope );
        BSTreeMove ( f -> fscope, p_fscope );
        SFormParmlistMove ( f -> fact, p_factory );
        SFormParmlistMove ( f -> func, p_params );
        VectorMove ( f -> type, p_stypes );
        VectorMove ( f -> schem, p_sparams );

        f -> version = p_fqn . GetVersion ();

        if ( ( f -> version & 0xFFFF ) != 0 && p_type == eFunction )
        {
            ReportError ( "Release number is not allowed for simple function", p_fqn );
        }

        f -> script = false; //TODO
        f -> marked = false;
        f -> validate = false; //TODO
        f -> untyped    = p_type == eUntypedFunc;
        f -> row_length = p_type == eRowLengthFunc;

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
                            if ( p_prologue != 0 )
                            {
                                HandlePrologue ( * f, * p_prologue );
                            }
                            return; // success
                        }
                    }
                }
                else
                {
                    ReportError ( "VectorInsertUnique", rc );
                }
                SNameOverloadWhack ( name, 0 );
            }
            else
            {
                ReportError ( "SNameOverloadMake", rc );
            }
        }
        else if ( HandleOverload ( f, priorDecl ) ) // declared previously
        {
            if ( p_prologue != 0 )
            {
                HandlePrologue ( * f, * p_prologue );
            }
            return;
        }
        SFunctionWhack ( f, 0 );
    }
}

AST * ASTBuilder :: UntypedFunctionDecl ( const Token* p_token, AST_FQN* p_name )
{
    AST * ret = new AST ( p_token, p_name );
    DeclareFunction ( * p_name, eUntypedFunc, 0, 0, 0, 0, 0, 0, 0, false, 0 );
    return ret;
}

AST * ASTBuilder :: RowlenFunctionDecl ( const Token* p_token, AST_FQN* p_name )
{
    AST * ret = new AST ( p_token, p_name );
    DeclareFunction ( * p_name, eRowLengthFunc, 0, 0, 0, 0, 0, 0, 0, false, 0 );
    return ret;
}

KSymbol *
ASTBuilder :: CreateParamSymbol ( const char* p_name, int p_type, void * p_obj )
{
    KSymbol * ret = 0;

    String name;
    StringInitCString ( & name, p_name );
    if ( KSymTableFindShallow ( & m_symtab, & name ) != 0 ||
            KSymTableFindIntrinsic ( & m_symtab, & name ) )
    {
        ReportError ( "Cannot be used as a parameter name: '%s'", p_name );
    }
    else
    {
        rc_t rc = KSymTableCreateSymbol ( & m_symtab, & ret, & name, p_type, p_obj );
        if ( rc != 0 )
        {
            ReportError ( "KSymTableCreateSymbol", rc );
        }
    }

    return ret;
}

void
ASTBuilder :: AddFactoryParams ( Vector& p_sig, const AST & p_params )
{
    uint32_t count = p_params . ChildrenCount ();
    for ( uint32_t i = 0; i < count; ++i )
    {
        SIndirectConst * param = Alloc < SIndirectConst > ();
        if ( param == 0 )
        {
            return;
        }
        const AST_Formal & p = dynamic_cast < const AST_Formal & > ( * p_params . GetChild ( i ) );

        param -> td = 0;
        param -> expr_id = 0;
        param -> name = CreateParamSymbol ( p . GetIdent (), eFactParam, param );

        STypeExpr * type = MakeTypeExpr ( p . GetType () );
        if ( type != 0 )
        {
            param -> td = & type -> dad;
        }

        if ( VectorAppend ( & p_sig, & param -> pos, param ) != 0 )
        {
            SIndirectConstWhack ( param, NULL );
        }
        else
        {
            param -> expr_id = ++ m_schema -> num_indirect;
        }
    }
}

void
ASTBuilder :: AddFormalParams ( Vector& p_sig, const AST & p_params )
{
    uint32_t count = p_params . ChildrenCount ();
    for ( uint32_t i = 0; i < count; ++i )
    {
        SProduction * param = Alloc < SProduction > ();
        if ( param == 0 )
        {
            return;
        }
        const AST_Formal & p = dynamic_cast < const AST_Formal & > ( * p_params . GetChild ( i ) );

        param -> control = p . HasControl ();
        param -> name = CreateParamSymbol ( p . GetIdent (), eFuncParam, param );
        param -> expr = 0;
        param -> trigger = false;

        STypeExpr * type = MakeTypeExpr ( p . GetType () );
        if ( type != 0 )
        {
            param -> fd = & type -> dad;
        }

        if ( VectorAppend ( & p_sig, & param -> cid . id, param ) != 0 )
        {
            SProductionWhack ( param, NULL );
        }
    }
}

SFormParmlist *
ASTBuilder :: MakeFactoryParams ( const AST_ParamSig & p_sig )
{
    SFormParmlist * ret = Alloc < SFormParmlist > ();
    if ( ret != 0 )
    {
        VectorInit ( & ret -> parms, 0, 8 );
        ret -> mand     = p_sig . GetMandatory () . ChildrenCount ();
        ret -> vararg   = p_sig . IsVariadic ();

        AddFactoryParams ( ret -> parms, p_sig . GetMandatory () );
        AddFactoryParams ( ret -> parms, p_sig . GetOptional () );
    }

    return ret;
}

SFormParmlist *
ASTBuilder :: MakeFormalParams ( const AST_ParamSig & p_sig )
{
    SFormParmlist * ret = Alloc < SFormParmlist > ();
    if ( ret != 0 )
    {
        VectorInit ( & ret -> parms, 0, 8 );
        ret -> mand     = p_sig . GetMandatory () . ChildrenCount ();
        ret -> vararg   = p_sig . IsVariadic ();

        AddFormalParams ( ret -> parms, p_sig . GetMandatory () );
        AddFormalParams ( ret -> parms, p_sig . GetOptional () );
    }

    return ret;
}

SIndirectType *
ASTBuilder :: MakeSchemaParamType ( const AST_FQN & p_name )
{
    SIndirectType *ret = Alloc < SIndirectType > ();
    if ( ret != 0 )
    {
        /* create symbol */
        String nameStr;
        p_name . GetIdentifier ( nameStr );
        rc_t rc = KSymTableCreateConstSymbol ( & m_symtab, & ret -> name, & nameStr, eSchemaType, ret );
        if ( rc == 0 )
        {
            /* record formal */
            rc = VectorAppend ( & m_schema -> pt, & ret -> id, ret );
            if ( rc == 0 )
            {
                /* initialize to raw format,
                undefined type, and no dimension */
                ret -> type_id = ++ m_schema -> num_indirect;
                return ret;
            }
            else
            {
                ReportError ( "VectorAppend", rc);
            }
        }
        else
        {
            ReportError ( "KSymTableCreateConstSymbol", rc );
        }
        SIndirectTypeWhack ( ret, 0 );
    }
    return 0;
}

SIndirectConst *
ASTBuilder :: MakeSchemaParamConst ( const AST_FQN & p_name )
{
    SIndirectConst * ret = Alloc < SIndirectConst > ();
    if ( ret != 0 )
    {
        /* create symbol */
        String nameStr;
        p_name . GetIdentifier ( nameStr );
        rc_t rc = KSymTableCreateConstSymbol ( & m_symtab, & ret -> name, & nameStr, eSchemaParam, ret );
        if ( rc == 0 )
        {
            ret -> expr_id = ++ m_schema -> num_indirect;
            return ret;
        }
        else
        {
            ReportError ( "KSymTableCreateConstSymbol", rc );
        }
        SIndirectConstWhack ( ret, 0 );
    }
    return 0;
}

void
ASTBuilder :: MakeSchemaParams ( const AST & p_sig, Vector & p_types, Vector & p_values )
{
    uint32_t count = p_sig . ChildrenCount ();
    for ( uint32_t i = 0; i < count; ++i )
    {
        const AST & p = * p_sig . GetChild ( i );
        if ( p . ChildrenCount () == 1 ) // type
        {
            const AST & typeNode = * p . GetChild ( 0 );
            SIndirectType *formal = MakeSchemaParamType ( dynamic_cast < const AST_FQN & > ( typeNode ) );
            if ( formal != 0 )
            {
                /* record positional */
                rc_t rc = VectorAppend ( & p_types, 0, formal );
                if ( rc == 0 )
                {
                    formal -> pos = i;
                }
                else
                {
                    ReportError ( "VectorAppend", rc);
                    SIndirectTypeWhack ( formal, 0 );
                    break;
                }
            }
        }
        else // value
        {
            STypeExpr * type = MakeTypeExpr ( * p . GetChild ( 0 ) );
            if ( type != 0 )
            {
                const AST_FQN & ident = dynamic_cast < const AST_FQN & > ( * p . GetChild ( 1 ) );
                // scalar unsigned int type required
                if ( type -> dt != 0 &&
                     type -> dt -> domain == ddUint &&
                     type -> fd . td. dim == 1 )
                {
                    SIndirectConst *formal = MakeSchemaParamConst ( ident );
                    if ( formal != 0 )
                    {
                        /* record positional */
                        rc_t rc = VectorAppend ( & p_values, 0, formal );
                        if ( rc == 0 )
                        {
                            formal -> td = & type -> dad;
                            formal -> pos = i;
                            continue; // success
                        }
                        ReportError ( "VectorAppend", rc);
                        SIndirectConstWhack ( formal, 0 );
                    }
                }
                else
                {
                    String nameStr;
                    ident . GetIdentifier ( nameStr );
                    ReportError ( "Not a scalar unsigned integer: '%S'", & nameStr );
                }
                SExpressionWhack ( & type -> dad );
            }
            break;
        }
    }
}

AST *
ASTBuilder :: FunctionDecl ( const Token*     p_token,
                             AST *            p_schema,
                             AST *            p_returnType,
                             AST_FQN*         p_name,
                             AST_ParamSig*    p_fact,
                             AST_ParamSig*    p_params,
                             AST*             p_prologue )
{
    AST * ret = new AST ( p_token, p_schema, p_returnType, p_name, p_fact, p_params, p_prologue );

    BSTree sscope;
    BSTreeInit ( & sscope );
    Vector stypes;
    VectorInit ( & stypes, 0 , 8 );
    Vector sparams;
    VectorInit ( & sparams, 0 , 8 );
    rc_t rc = 0;
    bool hasSchemaParms = p_schema -> ChildrenCount () != 0;
    if ( hasSchemaParms )
    {
        rc = KSymTablePushScope ( & m_symtab, & sscope );
        if ( rc == 0 )
        {
            MakeSchemaParams ( * p_schema, stypes, sparams );
        }
        else
        {
            ReportError ( "KSymTablePushScope", rc );
            hasSchemaParms = false; // do not pop later on
        }
    }

    // if hasSchemaParms, we are still in its scope, in order to evaluate the return type
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
        retType = MakeTypeExpr ( * p_returnType );
    }

    if ( hasSchemaParms )
    {   // now, can pop schema params' scope
        KSymTablePopScope ( & m_symtab );
    }

    BSTree fscope;
    BSTreeInit ( & fscope );
    SFormParmlist * formals = 0;
    SFormParmlist * factory = 0;

    rc = KSymTablePushScope ( & m_symtab, & sscope ); // enter schema scope
    if ( rc == 0 )
    {
        rc = KSymTablePushScope ( & m_symtab, & fscope ); // enter function scope
        if ( rc == 0 )
        {
            if ( p_fact -> GetTokenType () != PT_EMPTY )
            {
                factory = MakeFactoryParams ( * p_fact );
            }

            formals = MakeFormalParams ( * p_params );
            KSymTablePopScope ( & m_symtab );
        }
        else
        {
            ReportError ( "KSymTablePushScope", rc );
        }
        KSymTablePopScope ( & m_symtab );
    }
    else
    {
        ReportError ( "KSymTablePushScope", rc );
    }

    DeclareFunction ( * p_name, eFunction, retType, factory, formals, & sscope, & stypes, & sparams, & fscope, true, p_prologue );
    // made shallow copies of scopes and vectors - no need to whack the contents here
    VectorWhack ( & stypes, 0, 0 );
    VectorWhack ( & sparams, 0, 0 );

    return ret;
}
