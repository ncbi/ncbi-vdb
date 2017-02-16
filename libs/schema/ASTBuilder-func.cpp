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

class FunctionDeclaration // Wrapper around SFunction
{
public:
    FunctionDeclaration ( ASTBuilder & p_builder );
    ~FunctionDeclaration ();

    BSTree * SchemaScope () { return m_self == 0 ? 0 : & m_self -> sscope; }
    BSTree * FunctionScope () { return m_self == 0 ? 0 : & m_self -> fscope; }

    bool HandleOverload ( const KSymbol * p_priorDecl );

    bool SetName ( const AST_FQN &  p_fqn,
                   uint32_t         p_type,
                   bool             p_canOverload,
                   bool             p_validate );

    void SetSchemaParams ( const AST & p_sig );
    void SetReturnType ( STypeExpr * p_retType );
    void SetFactoryParams ( const AST_ParamSig & p_sig );
    void SetFormalParams ( const AST_ParamSig & p_sig );
    void SetPrologue ( const AST & p_prologue );

private:
    KSymbol * CreateLocalSymbol ( const char* p_name, int p_type, void * p_obj );
    void AddFactoryParams ( Vector& p_sig, const AST & p_params );
    void AddFormalParams ( Vector& p_sig, const AST & p_params );
    SIndirectType * MakeSchemaParamType ( const AST_FQN & p_name );
    SIndirectConst * MakeSchemaParamConst ( const AST_FQN & p_name );
    void HandleStatement ( const AST & p_stmt );

private:
    ASTBuilder &    m_builder;
    SFunction *     m_self;
    const KSymbol * m_priorDecl;
    bool            m_destroy;
};


FunctionDeclaration :: FunctionDeclaration ( ASTBuilder & p_builder )
: m_builder ( p_builder ),
    m_self ( m_builder . Alloc < SFunction > () ),
    m_priorDecl ( 0 ),
    m_destroy ( true )
{
    if ( m_self != 0 )
    {
        memset ( m_self, 0, sizeof * m_self );
        VectorInit ( & m_self -> fact . parms, 0, 8 );
    }
}

FunctionDeclaration :: ~FunctionDeclaration ()
{
    if ( m_self != 0 && m_destroy )
    {
        SFunctionWhack ( m_self, 0 );
    }
}

bool
FunctionDeclaration :: HandleOverload ( const KSymbol * p_priorDecl )
{
    assert ( m_self != 0 );
    assert ( p_priorDecl != 0 );

    m_self -> name = p_priorDecl;

    SNameOverload *name = ( SNameOverload* ) m_self -> name -> u . obj;
    assert ( name != 0 );
    uint32_t idx;
    rc_t rc = VectorInsertUnique ( & name -> items, m_self, & idx, SFunctionSort );
    if ( rc == 0 ) // overload added
    {
        if ( m_builder . VectorAppend ( m_builder . GetSchema () -> func, & m_self -> id, m_self ) )
        {
            return true;
        }
    }
    if ( GetRCState ( rc ) == rcExists )
    {   /* an overload with the same major version exists */
        /* see if new function trumps old */
        SFunction *exist = static_cast < SFunction * > ( VectorGet ( & name -> items, idx ) );
        if ( m_self -> version > exist -> version )
        {
            /* insert our function in name overload */
            void * prior;
            VectorSwap ( & name -> items, idx, m_self, & prior );

            /* if existing is in the same schema... */
            if ( ( const void* ) name == exist -> name -> u . obj )
            {
                /* need to swap with old */
                assert ( exist -> id >= VectorStart ( & m_builder . GetSchema () -> func ) );
                VectorSwap ( & m_builder . GetSchema () -> func, exist -> id, m_self, & prior );
                m_self -> id = exist -> id;
                SFunctionWhack ( (SFunction*)prior, 0 );
            }
            return true;
        }
    }
    else if ( rc != 0 )
    {
        m_builder . ReportError ( "VectorInsertUnique", rc );
    }
    return false;
}

bool
FunctionDeclaration :: SetName ( const AST_FQN &  p_fqn,
                                 uint32_t         p_type,
                                 bool             p_canOverload,
                                 bool             p_validate )
{
    if ( m_self == 0 )
    {
        return false;
    }

    m_self -> script        = p_type == eScriptFunc;
    m_self -> untyped       = p_type == eUntypedFunc;
    m_self -> row_length    = p_type == eRowLengthFunc;
    m_self -> validate      = p_validate;

    m_self -> version = p_fqn . GetVersion ();
    if ( ( m_self -> version & 0xFFFF ) != 0 && p_type == eFunction )
    {
        m_builder . ReportError ( "Release number is not allowed", p_fqn );
        return false;
    }

    m_priorDecl = m_builder . Resolve ( p_fqn, false );
    if ( m_priorDecl == 0 )
    {
        m_self -> name = m_builder . CreateFqnSymbol ( p_fqn, p_type, m_self );

        SNameOverload *name;
        rc_t rc = SNameOverloadMake ( & name, m_self -> name, 0, 4 );
        if ( rc == 0 )
        {
            rc = VectorInsertUnique ( & name -> items, m_self, 0, SFunctionSort );
            if ( rc == 0 )
            {
                if ( m_builder . VectorAppend ( m_builder . GetSchema () -> func, & m_self -> id, m_self ) )
                {
                    if ( m_builder . VectorAppend ( m_builder . GetSchema () -> fname, & name -> cid . id, name ) )
                    {
                        m_destroy = false;
                        return true;
                    }
                }
            }
            else
            {
                m_builder . ReportError ( "VectorInsertUnique", rc );
            }
            SNameOverloadWhack ( name, 0 );
        }
        else
        {
            m_builder . ReportError ( "SNameOverloadMake", rc );
        }
    }
    else
    {
        if ( m_priorDecl -> type != p_type || ! p_canOverload )
        {
            m_builder . ReportError ( "Declared earlier and cannot be overloaded", p_fqn );
            return false;
        }

        if ( HandleOverload ( m_priorDecl ) ) // declared previously
        {
            m_destroy = false;
            return true;
        }
    }

    return false;
}

void
FunctionDeclaration :: SetReturnType ( STypeExpr * p_retType )
{
    if ( m_self != 0 && p_retType != 0 )
    {
        m_self -> rt = & p_retType -> dad;
    }
}

KSymbol *
FunctionDeclaration :: CreateLocalSymbol ( const char* p_name, int p_type, void * p_obj )
{
    KSymbol * ret = 0;

    String name;
    StringInitCString ( & name, p_name );
    if ( KSymTableFindShallow ( & m_builder . GetSymTab (), & name ) != 0 ||
            KSymTableFindIntrinsic ( & m_builder . GetSymTab (), & name ) )
    {
        m_builder . ReportError ( "Name already in use: '%s'", p_name );
    }
    else
    {
        rc_t rc = KSymTableCreateSymbol ( & m_builder . GetSymTab (), & ret, & name, p_type, p_obj );
        if ( rc != 0 )
        {
            m_builder . ReportError ( "KSymTableCreateSymbol", rc );
        }
    }

    return ret;
}

void
FunctionDeclaration :: AddFactoryParams ( Vector& p_sig, const AST & p_params )
{
    uint32_t count = p_params . ChildrenCount ();
    for ( uint32_t i = 0; i < count; ++i )
    {
        SIndirectConst * param = m_builder . Alloc < SIndirectConst > ();
        if ( param == 0 )
        {
            return;
        }
        const AST_Formal & p = dynamic_cast < const AST_Formal & > ( * p_params . GetChild ( i ) );

        param -> td = 0;
        param -> expr_id = 0;
        param -> name = CreateLocalSymbol ( p . GetIdent (), eFactParam, param );

        STypeExpr * type = m_builder . MakeTypeExpr ( p . GetType () );
        if ( type != 0 )
        {
            param -> td = & type -> dad;
        }

        if ( m_builder . VectorAppend ( p_sig, & param -> pos, param ) )
        {
            param -> expr_id = ++ ( m_builder . GetSchema () -> num_indirect );
        }
        else
        {
            SIndirectConstWhack ( param, NULL );
        }
    }
}

void
FunctionDeclaration :: SetFactoryParams ( const AST_ParamSig & p_sig )
{
    if ( m_self != 0 )
    {
        m_self -> fact . mand     = p_sig . GetMandatory () . ChildrenCount ();
        m_self -> fact . vararg   = p_sig . IsVariadic ();

        AddFactoryParams ( m_self -> fact . parms, p_sig . GetMandatory () );
        AddFactoryParams ( m_self -> fact . parms, p_sig . GetOptional () );
    }
}

void
FunctionDeclaration :: AddFormalParams ( Vector& p_sig, const AST & p_params )
{
    uint32_t count = p_params . ChildrenCount ();
    for ( uint32_t i = 0; i < count; ++i )
    {
        SProduction * param = m_builder . Alloc < SProduction > ();
        if ( param == 0 )
        {
            return;
        }
        const AST_Formal & p = dynamic_cast < const AST_Formal & > ( * p_params . GetChild ( i ) );

        param -> control = p . HasControl ();
        param -> name = CreateLocalSymbol ( p . GetIdent (), eFuncParam, param );
        param -> expr = 0;
        param -> trigger = false;

        STypeExpr * type = m_builder . MakeTypeExpr ( p . GetType () );
        if ( type != 0 )
        {
            param -> fd = & type -> dad;
        }

        if ( ! m_builder . VectorAppend ( p_sig, & param -> cid . id, param ) )
        {
            SProductionWhack ( param, NULL );
        }
    }
}

void
FunctionDeclaration :: SetFormalParams ( const AST_ParamSig & p_sig )
{
    if ( m_self != 0 )
    {
        VectorInit ( & m_self -> func . parms, 0, 8 );
        m_self -> func . mand     = p_sig . GetMandatory () . ChildrenCount ();
        m_self -> func . vararg   = p_sig . IsVariadic ();

        AddFormalParams ( m_self -> func . parms, p_sig . GetMandatory () );
        AddFormalParams ( m_self -> func . parms, p_sig . GetOptional () );
    }
}

SIndirectType *
FunctionDeclaration :: MakeSchemaParamType ( const AST_FQN & p_name )
{
    SIndirectType *ret = m_builder . Alloc < SIndirectType > ();
    if ( ret != 0 )
    {
        /* create symbol */
        String nameStr;
        p_name . GetIdentifier ( nameStr );
        rc_t rc = KSymTableCreateConstSymbol ( & m_builder . GetSymTab (), & ret -> name, & nameStr, eSchemaType, ret );
        if ( rc == 0 )
        {
            if ( m_builder . VectorAppend ( m_builder . GetSchema () -> pt, & ret -> id, ret ) )
            {
                /* initialize to raw format,
                undefined type, and no dimension */
                ret -> type_id = ++ ( m_builder . GetSchema () -> num_indirect );
                return ret;
            }
        }
        else
        {
            m_builder . ReportError ( "KSymTableCreateConstSymbol", rc );
        }
        SIndirectTypeWhack ( ret, 0 );
    }
    return 0;
}

SIndirectConst *
FunctionDeclaration :: MakeSchemaParamConst ( const AST_FQN & p_name )
{
    SIndirectConst * ret = m_builder . Alloc < SIndirectConst > ();
    if ( ret != 0 )
    {
        /* create symbol */
        String nameStr;
        p_name . GetIdentifier ( nameStr );
        rc_t rc = KSymTableCreateConstSymbol ( & m_builder . GetSymTab (), & ret -> name, & nameStr, eSchemaParam, ret );
        if ( rc == 0 )
        {
            ret -> expr_id = ++ ( m_builder . GetSchema () -> num_indirect );
            return ret;
        }
        else
        {
            m_builder . ReportError ( "KSymTableCreateConstSymbol", rc );
        }
        SIndirectConstWhack ( ret, 0 );
    }
    return 0;
}

void
FunctionDeclaration :: SetSchemaParams ( const AST & p_sig )
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
                if ( m_builder . VectorAppend ( m_self -> type, 0, formal ) )
                {
                    formal -> pos = i;
                }
                else
                {
                    SIndirectTypeWhack ( formal, 0 );
                    break;
                }
            }
        }
        else // value
        {
            STypeExpr * type = m_builder . MakeTypeExpr ( * p . GetChild ( 0 ) );
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
                        if ( m_builder . VectorAppend ( m_self -> schem, 0, formal ) )
                        {
                            formal -> td = & type -> dad;
                            formal -> pos = i;
                            continue; // success
                        }
                        SIndirectConstWhack ( formal, 0 );
                    }
                }
                else
                {
                    String nameStr;
                    ident . GetIdentifier ( nameStr );
                    m_builder . ReportError ( "Not a scalar unsigned integer: '%S'", & nameStr );
                }
                SExpressionWhack ( & type -> dad );
            }
            break;
        }
    }
}

void
FunctionDeclaration :: HandleStatement ( const AST & p_stmt )
{
    switch ( p_stmt . GetTokenType () )
    {
    case PT_RETURN:
        {
            if ( m_self -> u . script . rtn != 0 )
            {
                m_builder . ReportError ( "Multiple return statements in a function: '%S'", & m_self -> name -> name );
            }
            else
            {
                assert ( p_stmt . ChildrenCount () == 1 );
                const AST_Expr & expr = * dynamic_cast < const AST_Expr * > ( p_stmt . GetChild ( 0 ) ) ;
                m_self -> u . script . rtn = expr . MakeExpression ( m_builder );
            }
            break;
        }
    case PT_PRODSTMT:
    {
        assert ( p_stmt . ChildrenCount () == 3 ); // type-expr, ident, cond-expr
        SProduction * prod = m_builder . Alloc < SProduction > ();
        if ( prod != 0 )
        {
            memset ( prod, 0 , sizeof * prod );
            STypeExpr * type = m_builder . MakeTypeExpr ( * p_stmt . GetChild ( 0 ) );
            if ( type != 0 )
            {
                prod -> fd = & type -> dad;
            }

            prod -> name = CreateLocalSymbol ( p_stmt . GetChild ( 1 ) -> GetTokenValue (), eProduction, prod );
            const AST_Expr & expr = * dynamic_cast < const AST_Expr * > ( p_stmt . GetChild ( 2 ) ) ;
            prod -> expr = expr . MakeExpression ( m_builder ) ;

            if ( ! m_builder . VectorAppend ( m_self -> u . script . prod, & prod -> cid . id, prod ) )
            {
                SProductionWhack ( prod, NULL );
            }
        }
        break;
    }
    default:
        m_builder . ReportError ("Unsupported statement type: %i", p_stmt . GetTokenType () );
        break;
    }
}

void
FunctionDeclaration :: SetPrologue ( const AST & p_prologue )
{
    switch ( p_prologue . GetTokenType () )
    {
    case PT_IDENT:
        {
            const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( p_prologue );
            const KSymbol * priorDecl = m_builder . Resolve ( fqn, false );
            if ( priorDecl != 0 )
            {
                if ( priorDecl -> type == eFunction || priorDecl -> type == eFactory )
                {
                    m_self -> u. ext . fact = priorDecl;
                }
                else
                {
                    m_builder . ReportError ( "Cannot be used as factory", fqn );
                }
            }
            else
            {
                m_self -> u. ext . fact = m_builder . CreateFqnSymbol ( fqn, eFactory, 0 );
            }
        }
        break;
    case PT_EMPTY:
        {
            uint32_t stmtCount = p_prologue . ChildrenCount ();
            if ( stmtCount > 0 && ! m_self -> script )
            {
                m_builder . ReportError ( "Only schema functions can have body: '%S'", & m_self -> name -> name );
            }
            else
            {
                rc_t rc = KSymTablePushScope ( & m_builder . GetSymTab (), SchemaScope () );
                if ( rc == 0 )
                {
                    rc = KSymTablePushScope ( & m_builder . GetSymTab (), FunctionScope () );
                    if ( rc == 0 )
                    {
                        for ( uint32_t i = 0 ; i < stmtCount; ++ i )
                        {
                            HandleStatement ( * p_prologue . GetChild ( i ) );
                        }
                        if ( m_self -> script && m_self -> u . script . rtn == 0 )
                        {
                            m_builder . ReportError ( "Schema function does not contain a return statement: '%S'", & m_self -> name -> name );
                        }
                        KSymTablePopScope ( & m_builder . GetSymTab () );
                    }
                    else
                    {
                        m_builder . ReportError ( "KSymTablePushScope", rc );
                    }
                    KSymTablePopScope ( & m_builder . GetSymTab () );
                }
                else
                {
                    m_builder . ReportError ( "KSymTablePushScope", rc );
                }
            }
        }
        break;
    default:
        assert (false);
    }
}

// Function-related methods from ASTBuilder

AST * ASTBuilder :: UntypedFunctionDecl ( const Token* p_token, AST_FQN* p_name )
{
    AST * ret = new AST ( p_token, p_name );
    FunctionDeclaration ( *this ) . SetName ( * p_name, eUntypedFunc, false, false );
    return ret;
}

AST * ASTBuilder :: RowlenFunctionDecl ( const Token* p_token, AST_FQN* p_name )
{
    AST * ret = new AST ( p_token, p_name );
    FunctionDeclaration ( *this ) . SetName ( * p_name, eRowLengthFunc, false, false );
    return ret;
}

AST *
ASTBuilder :: FunctionDecl ( const Token*     p_token,
                             bool             p_script,
                             AST *            p_schema,
                             AST *            p_returnType,
                             AST_FQN*         p_name,
                             AST_ParamSig*    p_fact,
                             AST_ParamSig*    p_params,
                             AST*             p_prologue )
{
    AST * ret = new AST ( p_token, p_schema, p_returnType, p_name, p_fact, p_params, p_prologue );

    bool isValidate = p_token -> GetType () == PT_VALIDATE;

    FunctionDeclaration func ( *this );
    if ( func . SetName ( * p_name, p_script ? eScriptFunc : eFunction, true, isValidate ) )
    {
        rc_t rc = 0;
        bool hasSchemaParms = p_schema -> ChildrenCount () != 0;
        if ( hasSchemaParms )
        {
            rc = KSymTablePushScope ( & m_symtab, func . SchemaScope () );
            if ( rc == 0 )
            {
                func . SetSchemaParams ( * p_schema );
            }
            else
            {
                ReportError ( "KSymTablePushScope", rc );
                hasSchemaParms = false; // do not pop later on
            }
        }

        // if hasSchemaParms, we are still in its scope, in order to evaluate the return type
        bool isVoid = p_returnType -> ChildrenCount () == 0;
        if ( isValidate )
        {   // require return type to be void, 2 mandatory parameters, no optional or varargs
            if ( ! isVoid )
            {
                ReportError ( "Validate functions have to return void", * p_name );
            }
            if ( p_params -> GetMandatory () . ChildrenCount () != 2 )
            {
                ReportError ( "Validate functions have to have 2 mandatory parameters", * p_name);
            }
            if ( p_params -> GetOptional () . ChildrenCount () != 0 )
            {
                ReportError ( "Validate functions cannot have optional parameters", * p_name);
            }
            if ( p_params -> IsVariadic () )
            {
                ReportError ( "Validate functions cannot have variable parameters", * p_name);
            }
        }
        else if ( isVoid )
        {
            ReportError ( "Only validate functions can return void", * p_name );
        }
        else if ( p_returnType -> GetChild ( 0 ) -> GetTokenType () == PT_TYPEEXPR )
        {
            assert ( false ); //TODO: fqn / fqn
        }
        else
        {
            func . SetReturnType ( MakeTypeExpr ( * p_returnType ) );
        }

        rc = KSymTablePushScope ( & m_symtab, func . FunctionScope () );
        if ( rc == 0 )
        {
            if ( p_fact -> GetTokenType () != PT_EMPTY )
            {
                func . SetFactoryParams ( * p_fact );
            }

            func . SetFormalParams ( * p_params );

            KSymTablePopScope ( & m_symtab );
        }
        else
        {
            ReportError ( "KSymTablePushScope", rc );
        }

        if ( hasSchemaParms )
        {   // now, can pop schema params' scope
            KSymTablePopScope ( & m_symtab );
        }

        if ( p_prologue != 0 )
        {
            func . SetPrologue ( * p_prologue );
        }
    }

    return ret;
}
