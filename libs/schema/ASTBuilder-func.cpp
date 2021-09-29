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

#include "ASTBuilder-func.hpp"

#include <klib/symbol.h>
#include <klib/printf.h>

#include "../vdb/schema-parse.h"
#include "../vdb/dbmgr-priv.h"
#include "../vdb/schema-expr.h"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast-tokens.h"

using namespace std;

FunctionDeclaration :: FunctionDeclaration ( ASTBuilder & p_builder )
:   m_builder ( p_builder ),
    m_self ( m_builder . Alloc < SFunction > () ),
    m_destroy ( true )
{
    if ( m_self != 0 )
    {
        VectorInit ( & m_self -> fact . parms, 0, 8 );
    }
}

FunctionDeclaration :: FunctionDeclaration ( ASTBuilder & p_builder, SFunction & p_func )
:   m_builder ( p_builder ),
    m_self ( & p_func ),
    m_destroy ( false )
{
    memset ( m_self, 0, sizeof * m_self );
    VectorInit ( & m_self -> fact . parms, 0, 8 );
    m_self -> script        = true;
}

FunctionDeclaration :: ~FunctionDeclaration ()
{
    if ( m_self != 0 && m_destroy )
    {
        SFunctionWhack ( m_self, 0 );
    }
}

bool
FunctionDeclaration :: HandleOverload ( const AST_FQN & p_fqn, const KSymbol *  p_priorDecl )
{
    assert ( p_priorDecl != 0 );

    Vector & functions = m_builder . GetSchema () -> func;

    SNameOverload *name = ( SNameOverload* ) p_priorDecl -> u . obj;
    assert ( name != 0 );

    SFunction *exist = static_cast < SFunction * > ( VectorGet ( & name -> items, 0 ) );
    assert ( exist != 0 );
    if ( exist -> script )
    {
        if ( ! m_self -> script)
        {
            m_builder . ReportError ( p_fqn . GetLocation (), "Overload has to have a body", p_priorDecl -> name );
        }
    }
    else if ( m_self -> script)
    {
        m_builder . ReportError ( p_fqn . GetLocation (), "Overload cannot have a body", p_priorDecl -> name );
    }

    uint32_t idx;
    rc_t rc = VectorInsertUnique ( & name -> items, m_self, & idx, SFunctionSort );
    if ( rc == 0 ) // overload added
    {
        return m_builder . VectorAppend ( functions, & m_self -> id, m_self );
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
                assert ( exist -> id >= VectorStart ( & functions ) );
                VectorSwap ( & functions, exist -> id, m_self, & prior );
                m_self -> id = exist -> id;
                SFunctionWhack ( (SFunction*)prior, 0 );
            }
            return true;
        }
    }
    else if ( rc != 0 )
    {
        m_builder . ReportRc ( "VectorInsertUnique", rc );
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

    const KSymbol * priorDecl = m_builder . Resolve ( p_fqn, false );

    m_self -> version = p_fqn . GetVersion ();
    if ( ( m_self -> version & 0xFFFF ) != 0 && priorDecl != 0 && priorDecl -> type == eFunction )
    {
        m_builder . ReportError ( "Changing release number is not allowed", p_fqn );
        return false;
    }

    if ( priorDecl == 0 )
    {
        m_self -> name = m_builder . CreateFqnSymbol ( p_fqn, p_type, m_self );
        if ( m_self -> name != 0 )
        {
            if ( m_builder . CreateOverload ( m_self -> name,
                                              m_self,
                                              SFunctionSort,
                                              m_builder . GetSchema () -> func,
                                              m_builder . GetSchema () -> fname,
                                              & m_self -> id ) )
            {
                m_destroy = false;
                return true;
            }
        }
    }
    else
    {
        if ( ! p_canOverload  || priorDecl -> type == eFactory )
        {
            m_builder . ReportError ( "Declared earlier and cannot be overloaded", p_fqn );
            return false;
        }

        if ( HandleOverload ( p_fqn, priorDecl ) )
        {   // declared previously, this version not ignored
            m_self -> name = priorDecl;
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
        const AST * p = p_params . GetChild ( i );
        assert ( p != 0 );
        assert ( p -> ChildrenCount () == 3 ); // type, ident, control or PT_EMPTY

        STypeExpr * type = m_builder . MakeTypeExpr ( * p -> GetChild ( 0 ) );
        if ( type != 0 )
        {
            param -> td = & type -> dad;
        }

        param -> name = m_builder . CreateLocalSymbol ( * p -> GetChild ( 1 ),
                                                        p -> GetChild ( 1 ) -> GetTokenValue (),
                                                        eFactParam,
                                                        param );

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
FunctionDeclaration :: SetFactoryParams ( const AST & p_sig )
{
    if ( m_self != 0 && p_sig . GetTokenType () != PT_EMPTY )
    {   // children: mandatory, optional, isVariadic
        assert ( p_sig . ChildrenCount () == 3 );
        m_self -> fact . mand = p_sig . GetChild ( 0 ) -> ChildrenCount ();
        AddFactoryParams ( m_self -> fact . parms, * p_sig . GetChild ( 0 ) );
        AddFactoryParams ( m_self -> fact . parms, * p_sig . GetChild ( 1 ) );
        m_self -> fact . vararg = p_sig . GetChild ( 2 ) -> GetTokenType () != PT_EMPTY;
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
        const AST * p = p_params . GetChild ( i );
        assert ( p != 0 );
        assert ( p -> ChildrenCount () == 3 ); // type, ident, control or PT_EMPTY

        STypeExpr * type = m_builder . MakeTypeExpr ( * p -> GetChild ( 0 ) );
        if ( type != 0 )
        {
            param -> fd = & type -> dad;
        }

        param -> name = m_builder . CreateLocalSymbol ( * p -> GetChild ( 1 ),
                                                        p -> GetChild ( 1 ) -> GetTokenValue (),
                                                        eFuncParam,
                                                        param );

        param -> control = p -> GetChild ( 2 ) -> GetTokenType () != PT_EMPTY;

        if ( ! m_builder . VectorAppend ( p_sig, & param -> cid . id, param ) )
        {
            SProductionWhack ( param, NULL );
        }
    }
}

void
FunctionDeclaration :: SetFormalParams ( const AST & p_sig )
{
    if ( m_self != 0 && p_sig . GetTokenType () != PT_EMPTY )
    {   // children: mandatory, optional, isVariadic
        assert ( p_sig . ChildrenCount () == 3 );
        m_self -> func . mand = p_sig . GetChild ( 0 ) -> ChildrenCount ();
        AddFormalParams ( m_self -> func . parms, * p_sig . GetChild ( 0 ) );
        AddFormalParams ( m_self -> func . parms, * p_sig . GetChild ( 1 ) );
        m_self -> func . vararg = p_sig . GetChild ( 2 ) -> GetTokenType () != PT_EMPTY;
    }
}

void
FunctionDeclaration :: SetPhysicalParams ()
{
    if ( m_self != 0 )
    {
        VectorInit ( & m_self -> func . parms, 0, 1 );
        m_self -> func . mand     = 1;
        m_self -> func . vararg   = false;

        /* create special input symbol */
        SProduction * parm = m_builder . Alloc < SProduction > ();
        if ( parm != 0 && m_builder . VectorAppend ( m_self -> func . parms, & parm -> cid . id, parm ) )
        {
            /* create special input symbol */
            String symstr;
            CONST_STRING ( & symstr, "@" );
            rc_t rc = KSymTableCreateConstSymbol ( & m_builder . GetSymTab (), & parm -> name, & symstr, eFuncParam, parm );
            if ( rc == 0 )
            {
                VectorInit ( & m_self -> u . script . prod, 0, 8 );
            }
            else
            {
                m_builder . ReportRc ( "KSymTableCreateConstSymbol", rc );
            }
        }
        else
        {
            SProductionWhack ( parm, 0 );
        }
    }
}

SIndirectType *
FunctionDeclaration :: MakeSchemaParamType ( const AST_FQN & p_name )
{
    SIndirectType *ret = m_builder . Alloc < SIndirectType > ();
    if ( ret != 0 )
    {   /* create symbol */
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
            m_builder . ReportRc ( "KSymTableCreateConstSymbol", rc );
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
    {   /* create symbol */
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
            m_builder . ReportRc ( "KSymTableCreateConstSymbol", rc );
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
            SIndirectType * formal = MakeSchemaParamType ( * ToFQN ( p . GetChild ( 0 ) ) );
            if ( formal != 0 )
            {
                /* record positional */
                if ( m_builder . VectorAppend ( m_self -> type, 0, formal ) )
                {
                    formal -> pos = i;
                    continue;
                }
                SIndirectTypeWhack ( formal, 0 );
            }
            break;
        }
        else // value
        {
            STypeExpr * type = m_builder . MakeTypeExpr ( * p . GetChild ( 0 ) );
            if ( type != 0 )
            {
                const AST_FQN & ident = * ToFQN ( p . GetChild ( 1 ) );
                // scalar unsigned int type required
                if ( type -> dt != 0 &&
                    type -> dt -> domain == ddUint &&
                    type -> fd . td. dim == 1 )
                {
                    SIndirectConst * formal = MakeSchemaParamConst ( ident );
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
                    m_builder . ReportError ( ident . GetLocation (), "Not a scalar unsigned integer", nameStr );
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
    case KW_return:
        {
            if ( m_self -> u . script . rtn == 0 )
            {
                assert ( p_stmt . ChildrenCount () == 1 );
                m_self -> u . script . rtn = ToExpr ( p_stmt . GetChild ( 0 ) ) -> MakeExpression ( m_builder );
            }
            else
            {
                m_builder . ReportError ( p_stmt . GetLocation (),
                                          "Multiple return statements in a function",
                                          m_self -> name -> name );
            }
        }
        break;
    case PT_PRODSTMT:
        {
            assert ( p_stmt . ChildrenCount () == 3 );
            const AST * ident = p_stmt . GetChild ( 1 );
            assert ( ident -> ChildrenCount () == 1 );
            m_builder . AddProduction ( * ident,
                                        m_self -> u . script . prod,
                                        ident -> GetChild ( 0 ) -> GetTokenValue (),
                                        * ToExpr ( p_stmt . GetChild ( 2 ) ),
                                        p_stmt . GetChild ( 0 ) );
        }
        break;
    case PT_EMPTY:
        break;
    default:
        m_builder . ReportError ( p_stmt . GetLocation (), "Unsupported statement type", p_stmt . GetTokenType () );
        break;
    }
}

void
FunctionDeclaration :: HandleScript ( const AST & p_body, const String & p_funcName )
{
    uint32_t stmtCount = p_body . ChildrenCount ();
    for ( uint32_t i = 0 ; i < stmtCount; ++ i )
    {
        HandleStatement ( * p_body . GetChild ( i ) );
    }
    if ( m_self -> script && m_self -> u . script . rtn == 0 )
    {
        m_builder . ReportError ( p_body . GetLocation (), "Schema function does not contain a return statement", p_funcName );
    }
}

void
FunctionDeclaration :: SetPrologue ( const AST & p_prologue )
{
    switch ( p_prologue . GetTokenType () )
    {
    case PT_IDENT:
        {   // renaming
            const AST_FQN & fqn = * ToFQN ( & p_prologue );
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
        {   // function body
            if ( p_prologue . ChildrenCount () > 0 )
            {
                if ( m_self -> fact . vararg )
                {
                    m_builder . ReportError ( p_prologue . GetLocation (),
                                              "Function with factory varargs cannot have a body",
                                              m_self -> name -> name );
                }
                else
                {
                    rc_t rc = KSymTablePushScope ( & m_builder . GetSymTab (), SchemaScope () );
                    if ( rc == 0 )
                    {
                        rc = KSymTablePushScope ( & m_builder . GetSymTab (), FunctionScope () );
                        if ( rc == 0 )
                        {
                            HandleScript ( p_prologue, m_self -> name -> name );
                            KSymTablePopScope ( & m_builder . GetSymTab () );
                        }
                        else
                        {
                            m_builder . ReportRc ( "KSymTablePushScope", rc );
                        }
                        KSymTablePopScope ( & m_builder . GetSymTab () );
                    }
                    else
                    {
                        m_builder . ReportRc ( "KSymTablePushScope", rc );
                    }
                }
            }
        }
        break;
    default:
        assert (false);
    }
}

// Function-building methods from ASTBuilder

AST *
ASTBuilder :: UntypedFunctionDecl ( const Token* p_token, AST_FQN* p_name )
{
    AST * ret = new AST ( p_token, p_name );
    FunctionDeclaration ( *this ) . SetName ( * p_name, eUntypedFunc, false, false );
    return ret;
}

AST *
ASTBuilder :: RowlenFunctionDecl ( const Token* p_token, AST_FQN* p_name )
{
    AST * ret = new AST ( p_token, p_name );
    FunctionDeclaration ( *this ) . SetName ( * p_name, eRowLengthFunc, false, false );
    return ret;
}

AST *
ASTBuilder :: FunctionDecl ( const Token *  p_token,
                             bool           p_script,
                             AST *          p_schema,
                             AST *          p_returnType,
                             AST_FQN *      p_name,
                             AST *          p_fact,
                             AST *          p_params,
                             AST *          p_prologue )
{
    AST * ret = new AST ( p_token, p_schema, p_returnType, p_name, p_fact, p_params, p_prologue );

    bool isValidate = p_token -> GetType () == PT_VALIDATE;

    FunctionDeclaration func ( *this );
    if ( func . SetName ( * p_name,
                          ( p_script || ( p_prologue -> GetTokenType () == PT_EMPTY && p_prologue -> ChildrenCount () > 0 ) )? eScriptFunc : eFunction,
                          true,
                          isValidate ) )
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
                ReportRc ( "KSymTablePushScope", rc );
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
            if ( p_params -> GetChild ( 0 ) -> ChildrenCount () != 2 )
            {
                ReportError ( "Validate functions have to have 2 mandatory parameters", * p_name);
            }
            if ( p_params -> GetChild ( 1 ) -> ChildrenCount () != 0 )
            {
                ReportError ( "Validate functions cannot have optional parameters", * p_name);
            }
            if ( p_params -> GetChild ( 2 ) -> GetTokenType () != PT_EMPTY )
            {
                ReportError ( "Validate functions cannot have variable parameters", * p_name);
            }
        }
        else if ( isVoid )
        {
            ReportError ( "Only validate functions can return void", * p_name );
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
            ReportRc ( "KSymTablePushScope", rc );
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
