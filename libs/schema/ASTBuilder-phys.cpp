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

#include <klib/symbol.h>
#include <klib/printf.h>

#include "../vdb/schema-parse.h"
#include "../vdb/dbmgr-priv.h"
#include "../vdb/schema-expr.h"

#include "ASTBuilder-func.hpp"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast.hpp"

using namespace std;

// Physical encoding rules

class PhysicalDeclaration // Wrapper around SPhysical
{
public:
    PhysicalDeclaration ( ctx_t ctx, ASTBuilder & p_builder, const AST & p_returnType );
    ~PhysicalDeclaration ();

    bool SetName ( ctx_t ctx, const AST_FQN &  p_fqn );

    void SetParams ( ctx_t ctx, const AST & p_schemaParams, const AST & p_factParams );

    void HandleBody ( ctx_t ctx, const AST & p_body, FunctionDeclaration & );
    void HandleRowLength ( ctx_t ctx, const AST & p_body );

    FunctionDeclaration & GetDecode () { return m_decode; }
    FunctionDeclaration & GetEncode () { return m_encode; }

    bool NoHeader () const { return m_self -> no_hdr; }
    void SetReadOnly ( bool p_yes ) { m_self -> read_only = p_yes; }

private:
    bool HandleOverload ( ctx_t ctx, const KSymbol * p_priorDecl );

private:
    ASTBuilder &            m_builder;
    const AST &             m_returnType;

    SPhysical *             m_self;
    FunctionDeclaration     m_decode;
    FunctionDeclaration     m_encode;

    bool m_delete;
};

PhysicalDeclaration :: PhysicalDeclaration ( ctx_t ctx, ASTBuilder & p_builder, const AST & p_returnType )
:   m_builder ( p_builder ),
    m_returnType ( p_returnType . GetTokenType () == PT_NOHEADER ? * p_returnType . GetChild ( 0 ) : p_returnType ),
    m_self ( m_builder . Alloc < SPhysical > ( ctx ) ),
    m_decode ( p_builder, m_self -> decode ),
    m_encode ( p_builder, m_self -> encode ),
    m_delete ( false )
{
    if ( m_self != 0 )
    {
        m_self -> encode . script = true;
        m_self -> decode . script = true;
        m_self -> no_hdr = p_returnType . GetTokenType () == PT_NOHEADER;
    }
}

PhysicalDeclaration :: ~PhysicalDeclaration ()
{
    if ( m_delete )
    {
        SPhysicalWhack ( m_self, 0 );
    }
}

bool
PhysicalDeclaration :: HandleOverload ( ctx_t ctx, const KSymbol * p_priorDecl )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_priorDecl != 0 );

    Vector & functions = m_builder . GetSchema () -> phys;

    SNameOverload *name = ( SNameOverload* ) p_priorDecl -> u . obj;
    assert ( name != 0 );
    uint32_t idx;
    rc_t rc = VectorInsertUnique ( & name -> items, m_self, & idx, SPhysicalSort );
    if ( rc == 0 ) // overload added
    {
        return m_builder . VectorAppend ( ctx, functions, & m_self -> id, m_self );
    }
    if ( GetRCState ( rc ) == rcExists )
    {   /* an overload with the same major version exists */
        /* see if new function trumps old */
        SPhysical *exist = static_cast < SPhysical * > ( VectorGet ( & name -> items, idx ) );
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
                SPhysicalWhack ( (SPhysical*)prior, 0 );
            }
            return true;
        }
    }
    else if ( rc != 0 )
    {
        m_builder . ReportRc ( ctx, "VectorInsertUnique", rc );
    }
    return false;
}

bool
PhysicalDeclaration :: SetName ( ctx_t ctx, const AST_FQN &  p_name )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    m_self -> version = p_name . GetVersion ();

    const KSymbol* priorDecl = m_builder . Resolve ( ctx, p_name, false );
    if ( priorDecl == 0 )
    {
        m_self -> name = m_builder . CreateFqnSymbol ( ctx, p_name, ePhysical, m_self );
        if ( m_self -> name == 0 ||
             ! m_builder . CreateOverload ( ctx,
                                            * m_self -> name,
                                            m_self,
                                            0,
                                            SPhysicalSort,
                                            m_builder . GetSchema () -> phys,
                                            m_builder . GetSchema () -> pname,
                                            m_self -> id ) )
        {
            m_delete = true;
            return false;
        }
    }
    else
    {
        if ( priorDecl -> type != ePhysical )
        {
            m_builder . ReportError ( ctx, "Declared earlier and cannot be overloaded", p_name );
            m_delete = true;
            return false;
        }

        if ( ! HandleOverload ( ctx, priorDecl ) )
        {
            m_delete = true;
            return false;
        }
        // declared previously, this declaration not ignored
        m_self -> name = priorDecl;
    }
    return true;
}

void
PhysicalDeclaration :: SetParams ( ctx_t ctx, const AST & p_schemaParams, const AST & p_factParams )
{   // schema/factory parameters are evaluated once for decode and then copied over to encode
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    rc_t rc = KSymTablePushScope ( & m_builder . GetSymTab (), m_decode . SchemaScope () );
    if ( rc != 0 )
    {
        m_builder . ReportRc ( ctx, "KSymTablePushScope", rc );
        return;
    }

    m_decode . SetSchemaParams ( ctx, p_schemaParams );
    m_decode . SetFactoryParams ( ctx, p_factParams );

    /* copy schema and factory parameters
        NB - must be cleared before destruction ( see SPhysicalWhack() ) */
    SFunction & enc = * m_encode . GetSFunction ();
    const SFunction & dec = * m_decode . GetSFunction ();
    enc . sscope = dec . sscope;
    enc . type   = dec . type;
    enc . schem  = dec . schem;
    enc . fact   = dec . fact;

    /* clone factory parameter symbols */
    if ( BSTreeDoUntil ( & enc . fscope, false, KSymbolCopyScope, const_cast < BSTree * > ( & dec . fscope ) ) )
    {
        m_builder . ReportRc ( ctx, "FunctionDeclaration::CopyParams BSTreeDoUntil", RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted ) );
    }

    /* interpret return type within schema param scope */
    m_self -> td = & m_builder . MakeTypeExpr ( ctx, m_returnType ) -> dad;

    KSymTablePopScope ( & m_builder . GetSymTab () );
}

void
PhysicalDeclaration :: HandleBody ( ctx_t ctx, const AST & p_body, FunctionDeclaration & p_func )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    rc_t rc = KSymTablePushScope ( & m_builder . GetSymTab (), p_func . SchemaScope () );
    if ( rc != 0 )
    {
        m_builder . ReportRc ( ctx, "KSymTablePushScope", rc );
        return;
    }

    rc = KSymTablePushScope ( & m_builder . GetSymTab (), p_func . FunctionScope () );
    if ( rc == 0 )
    {
        p_func . SetPhysicalParams ( ctx );  /* simulate a schema function signature */
        p_func . HandleScript ( ctx, p_body, m_self -> name -> name );

        KSymTablePopScope ( & m_builder . GetSymTab () );
    }
    else
    {
        m_builder . ReportRc ( ctx, "KSymTablePushScope", rc );
    }

    KSymTablePopScope ( & m_builder . GetSymTab () );
}

void
PhysicalDeclaration :: HandleRowLength ( ctx_t ctx, const AST & p_body )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    const AST_FQN & b = * ToFQN ( & p_body );
    const KSymbol * rl = m_builder . Resolve ( ctx, b, true );
    if ( rl != 0 )
    {
        if ( rl -> type == eRowLengthFunc )
        {
            const SNameOverload * name = static_cast < const SNameOverload * > ( rl -> u . obj );
            m_self -> row_length = static_cast < SFunction * > ( VectorLast ( & name -> items ) );
        }
        else
        {
            m_builder . ReportError ( ctx, b . GetLocation (), "Not a row_length function", rl -> name );
        }
    }
}

AST *
ASTBuilder :: PhysicalDecl ( ctx_t ctx,
                             const Token *  p_token,
                             AST *          p_schema,
                             AST *          p_returnType,
                             AST_FQN *      p_name,
                             AST *          p_fact,
                             AST *          p_body )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_body != 0 );

    AST * ret = AST :: Make ( ctx, p_token, p_schema, p_returnType, p_name, p_fact, p_body );

    PhysicalDeclaration decl ( ctx, * this, * p_returnType );
    if ( decl . SetName ( ctx, * p_name ) )
    {
        decl . SetParams ( ctx, * p_schema, * p_fact );

        if ( p_body -> GetTokenType () == PT_PHYSSTMT ) // shorthand for decode-only
        {
            decl . HandleBody ( ctx, * p_body -> GetChild ( 0 ), decl . GetDecode () );
            decl . SetReadOnly ( true );
        }
        else
        {   // a sequence of decode, encode, row_length. No repeats, decode has to be present
            // PT_empty are allowed and ignored
            bool hasDecode = false;
            bool hasEncode = false;
            bool hasRowLength = false;
            uint32_t count = p_body -> ChildrenCount ();
            for ( uint32_t i = 0; i < count; ++i )
            {
                const AST & node = * p_body -> GetChild ( i );
                switch ( node . GetTokenType () )
                {
                case KW_decode:
                    if ( ! hasDecode )
                    {
                        decl . HandleBody ( ctx, * node . GetChild ( 0 ), decl . GetDecode () );
                        hasDecode = true;
                    }
                    else
                    {
                        ReportError ( ctx, "Multiply defined decode()", * p_name );
                    }
                    break;
                case KW_encode:
                    if ( hasEncode )
                    {
                        ReportError ( ctx, "Multiply defined encode()", * p_name );
                    }
                    else if ( decl . NoHeader () )
                    {
                        ReportError ( ctx, "__no_header cannot define enable()", * p_name );
                    }
                    else
                    {
                        decl . HandleBody ( ctx, * node . GetChild ( 0 ), decl . GetEncode () );
                        hasEncode = true;
                    }
                    break;
                case KW___row_length:
                    if ( hasRowLength )
                    {
                        ReportError ( ctx, "Multiply defined __row_length()", * p_name );
                    }
                    else
                    {
                        decl . HandleRowLength ( ctx, * node . GetChild ( 0 ) );
                        hasRowLength = true;
                    }
                    break;
                case PT_EMPTY:
                    break;
                default:
                    assert ( false );
                    break;
                }
            }

            decl . SetReadOnly ( ! hasEncode );
            if ( ! hasDecode )
            {
                ReportError ( ctx, "Missing decode()", * p_name );
            }
        }
    }
    return ret;
}