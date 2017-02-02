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

void
ASTBuilder :: DeclareFunction ( const AST_FQN&          p_fqn,
                                uint32_t                p_type,
                                struct STypeExpr *      p_retType,
                                SFormParmlist *         p_params,
                                const BSTree *          p_sscope,
                                const Vector *          p_stypes,
                                const Vector *          p_sparams,
                                const BSTree *          p_fscope )
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

            if ( p_sscope == 0 )
            {
                BSTreeInit ( & f -> sscope );
            }
            else
            {
                f -> sscope = * p_sscope;
            }

            if ( p_fscope == 0 )
            {
                BSTreeInit ( & f -> fscope );
            }
            else
            {
                f -> fscope = * p_fscope;
            }

            VectorInit ( & f -> fact . parms, 0, 8 );//TODO

            if ( p_params == 0 )
            {
                VectorInit ( & f -> func . parms, 0, 8 );
            }
            else
            {
                f -> func . mand    = p_params -> mand;
                f -> func . vararg  = p_params -> vararg;
                // move formal parameters from p_params, dispose of the original
                rc_t rc = VectorCopy ( & p_params -> parms, & f -> func . parms );
                if ( rc != 0 )
                {
                    ReportError ( "VectorCopy", rc );
                }
                VectorWhack ( & p_params -> parms, 0, 0 );
                SFormParmlistWhack ( p_params, 0 );
                free ( p_params );
            }

            if ( p_stypes == 0 )
            {
                VectorInit ( & f -> type, 0, 8 );
            }
            else
            {
                rc_t rc = VectorCopy ( p_stypes, & f -> type );
                if ( rc != 0 )
                {
                    ReportError ( "VectorCopy", rc );
                }
            }

            f -> version = p_fqn . GetVersion ();

            if ( ( f -> version & 0xFFFF ) != 0 && p_type == eFunction )
            {
                ReportError ( "Release number is not allowed for simple function", p_fqn );
            }

            if ( p_sparams == 0 )
            {
                VectorInit ( & f -> type, 0, 8 );
            }
            else
            {
                rc_t rc = VectorCopy ( p_sparams, & f -> schem );
                if ( rc != 0 )
                {
                    ReportError ( "VectorCopy", rc );
                }
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
                        void * prior;
                        VectorSwap ( & name -> items, idx, f, & prior );

                        /* if existing is in the same schema... */
                        if ( ( const void* ) name == exist -> name -> u . obj )
                        {
                            /* need to swap with old */
                            assert ( exist -> id >= VectorStart ( & m_schema -> func ) );
                            VectorSwap ( & m_schema -> func, exist -> id, f, & prior );
                            f -> id = exist -> id;
                            SFunctionWhack ( (SFunction*)prior, 0 );
                        }
                        return; // success
                    }
                }
                else if ( rc != 0 )
                {
                    ReportError ( "VectorInsertUnique", rc );
                }
            }
            SFunctionWhack ( f, 0 );
        }
    }
}

AST * ASTBuilder :: UntypedFunctionDecl ( const Token* p_token, AST_FQN* p_name )
{
    AST * ret = new AST ( p_token, p_name );
    DeclareFunction ( * p_name, eUntypedFunc, 0, 0, 0, 0, 0, 0 );
    return ret;
}

AST * ASTBuilder :: RowlenFunctionDecl ( const Token* p_token, AST_FQN* p_name )
{
    AST * ret = new AST ( p_token, p_name );
    DeclareFunction ( * p_name, eRowLengthFunc, 0, 0, 0, 0, 0, 0 );
    return ret;
}

void
ASTBuilder :: AddFormals ( Vector& p_sig, const AST & p_params )
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

        String name;
        StringInitCString ( & name, p . GetIdent () );
        if ( KSymTableFindShallow ( & m_symtab, & name ) != 0 ||
             KSymTableFindIntrinsic ( & m_symtab, & name ) )
        {
            ReportError ( "Cannot be used as a formal parameter name: '%s'", p . GetIdent () );
        }
        else
        {
            KSymbol * sym;
            rc_t rc = KSymTableCreateSymbol ( & m_symtab, & sym, & name, eFuncParam, param );
            if ( rc == 0 )
            {
                param -> name = sym;
            }
            else
            {
                ReportError ( "KSymTableCreateSymbol", rc );
            }
        }

        param -> expr = 0;

        STypeExpr * type = MakeTypeExpr ( p . GetType () );
        if ( type != 0 )
        {
            param -> fd = & type -> dad;
        }

        param -> trigger = false;

        if ( VectorAppend ( & p_sig, & param -> cid . id, param ) != 0 )
        {
            SProductionWhack ( param, NULL );
        }
    }
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

        AddFormals ( ret -> parms, p_sig . GetMandatory () );
        AddFormals ( ret -> parms, p_sig . GetOptional () );
    }

    return ret;
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
            SIndirectType *formal = Alloc < SIndirectType > ();
            if ( formal != 0 )
            {
                /* create symbol */
                const AST_FQN & name = dynamic_cast < const AST_FQN & > ( typeNode );
                String nameStr;
                name . GetIdentifier ( nameStr );
                rc_t rc = KSymTableCreateConstSymbol ( & m_symtab, & formal -> name, & nameStr, eSchemaType, formal );
                if ( rc == 0 )
                {
                    /* record positional */
                    rc = VectorAppend ( & p_types, & formal -> pos, formal );
                    if ( rc == 0 )
                    {
                        /* record formal */
                        rc = VectorAppend ( & m_schema -> pt, & formal -> id, formal );
                        if ( rc == 0 )
                        {
                            /* initialize to raw format,
                            undefined type, and no dimension */
                            formal -> pos = i;
                            formal -> type_id = ++ m_schema -> num_indirect;
                            continue; // success
                        }
                        else
                        {
                            ReportError ( "VectorAppend", rc);
                        }
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
                SIndirectTypeWhack ( formal, 0 );
            }
        }
        else // value
        {
            STypeExpr * type = MakeTypeExpr ( * p . GetChild ( 0 ) );
            if ( type != 0 )
            {
                const AST_FQN & name = dynamic_cast < const AST_FQN & > ( * p . GetChild ( 1 ) );
                String nameStr;
                name . GetIdentifier ( nameStr );
                if ( type -> dt != 0 &&
                     type -> dt -> domain == ddUint &&
                     type -> fd . td. dim == 1 )
                {
                    // scalar unsigned int type required
                    SIndirectConst *formal = Alloc < SIndirectConst > ();
                    if ( formal != 0 )
                    {
                        /* create symbol */
                        rc_t rc = KSymTableCreateConstSymbol ( & m_symtab, & formal -> name, & nameStr, eSchemaParam, formal );
                        if ( rc == 0 )
                        {
                            /* record positional */
                            rc = VectorAppend ( & p_values, & formal -> pos, formal );
                            if ( rc == 0 )
                            {
                                formal -> td = & type -> dad;
                                formal -> expr_id = ++ m_schema -> num_indirect;
                                formal -> pos = i;
                                continue; // success
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
                        SIndirectConstWhack ( formal, 0 );
                    }
                }
                else
                {
                    ReportError ( "Not a scalar unsigned integer: '%S'", & nameStr );
                }
                SExpressionWhack ( & type -> dad );
            }
        }
        break;
    }
}

AST *
ASTBuilder :: FunctionDecl ( const Token*     p_token,
                             AST *            p_schema,
                             AST *            p_returnType,
                             AST_FQN*         p_name,
                             AST*             p_fact,
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
    {   // now, can pop schema params'' scope
        KSymTablePopScope ( & m_symtab );
    }

    if ( p_fact -> GetTokenType () != PT_EMPTY )
    {
        assert ( false );
    }

    BSTree fscope;
    BSTreeInit ( & fscope );
    SFormParmlist * fp = 0;

    rc = KSymTablePushScope ( & m_symtab, & sscope ); // enter schema scope
    if ( rc == 0 )
    {
        rc = KSymTablePushScope ( & m_symtab, & fscope ); // enter function scope
        if ( rc == 0 )
        {
            fp = MakeFormalParams ( * p_params );
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

    DeclareFunction ( * p_name, eFunction, retType, fp, & sscope, & stypes, & sparams, & fscope );
    // made shallow copies of scopes and vectors - no need to whack the contents here
    VectorWhack ( & stypes, 0, 0 );
    VectorWhack ( & sparams, 0, 0 );

    return ret;
}
