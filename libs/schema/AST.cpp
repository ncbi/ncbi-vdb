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

#include "AST.hpp"

#include <strtol.h>
#include <new>

#include <klib/printf.h>

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast.hpp"

#include "ASTBuilder.hpp"

using namespace std;

// AST

AST *
AST :: Make ( ctx_t ctx, const Token* token )
{
    void * ret = malloc ( sizeof ( AST ) );
    if ( ret == 0 )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        SYSTEM_ERROR ( xcNoMemory, "" );
        return 0;
    }
    return new ( ret ) AST ( token );
}

AST * AST :: Make ( ctx_t ctx, Token :: TokenType tokenType )
{
    void * ret = malloc ( sizeof ( AST ) );
    if ( ret == 0 )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        SYSTEM_ERROR ( xcNoMemory, "" );
        return 0;
    }
    return new ( ret ) AST ( tokenType );
}

AST * AST :: Make ( ctx_t ctx )
{
    void * ret = malloc ( sizeof ( AST ) );
    if ( ret == 0 )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        SYSTEM_ERROR ( xcNoMemory, "" );
        return 0;
    }
    return new ( ret ) AST ();
}

AST *
AST :: Make ( ctx_t ctx, const Token * p_token, AST * p_child1 )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_child1 != 0 );
    AST * ret = Make ( ctx, p_token );
    if ( ret != 0 )
    {
        ret -> AddNode ( ctx, p_child1 );
    }
    return ret;
}

AST *
AST :: Make ( ctx_t ctx, const Token * p_token, AST * p_child1, AST * p_child2 )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_child1 != 0 );
    assert ( p_child2 != 0 );
    AST * ret = Make ( ctx, p_token );
    if ( ret != 0 )
    {
        ret -> AddNode ( ctx, p_child1 );
        ret -> AddNode ( ctx, p_child2 );
    }
    return ret;
}

AST *
AST :: Make ( ctx_t ctx, const Token * p_token, AST * p_child1, AST * p_child2, AST * p_child3 )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_child1 != 0 );
    assert ( p_child2 != 0 );
    assert ( p_child3 != 0 );
    AST * ret = Make ( ctx, p_token );
    if ( ret != 0 )
    {
        ret -> AddNode ( ctx, p_child1 );
        ret -> AddNode ( ctx, p_child2 );
        ret -> AddNode ( ctx, p_child3 );
    }
    return ret;
}

AST *
AST :: Make( ctx_t ctx,
             const Token * p_token,
             AST * p_child1,
             AST * p_child2,
             AST * p_child3,
             AST * p_child4 )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_child1 != 0 );
    assert ( p_child2 != 0 );
    assert ( p_child3 != 0 );
    assert ( p_child4 != 0 );
    AST * ret = Make ( ctx, p_token );
    if ( ret != 0 )
    {
        ret -> AddNode ( ctx, p_child1 );
        ret -> AddNode ( ctx, p_child2 );
        ret -> AddNode ( ctx, p_child3 );
        ret -> AddNode ( ctx, p_child4 );
    }
    return ret;
}

AST *
AST :: Make ( ctx_t ctx,
              const Token * p_token,
              AST * p_child1,
              AST * p_child2,
              AST * p_child3,
              AST * p_child4,
              AST * p_child5 )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_child1 != 0 );
    assert ( p_child2 != 0 );
    assert ( p_child3 != 0 );
    assert ( p_child4 != 0 );
    assert ( p_child5 != 0 );
    AST * ret = Make ( ctx, p_token );
    if ( ret != 0 )
    {
        ret -> AddNode ( ctx, p_child1 );
        ret -> AddNode ( ctx, p_child2 );
        ret -> AddNode ( ctx, p_child3 );
        ret -> AddNode ( ctx, p_child4 );
        ret -> AddNode ( ctx, p_child5 );
    }
    return ret;
}

AST *
AST :: Make ( ctx_t ctx,
              const Token * p_token,
              AST * p_child1,
              AST * p_child2,
              AST * p_child3,
              AST * p_child4,
              AST * p_child5,
              AST * p_child6 )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    assert ( p_child1 != 0 );
    assert ( p_child2 != 0 );
    assert ( p_child3 != 0 );
    assert ( p_child4 != 0 );
    assert ( p_child5 != 0 );
    assert ( p_child6 != 0 );
    AST * ret = Make ( ctx, p_token );
    if ( ret != 0 )
    {
        ret -> AddNode ( ctx, p_child1 );
        ret -> AddNode ( ctx, p_child2 );
        ret -> AddNode ( ctx, p_child3 );
        ret -> AddNode ( ctx, p_child4 );
        ret -> AddNode ( ctx, p_child5 );
        ret -> AddNode ( ctx, p_child6 );
    }
    return ret;
}

void
AST :: Destroy ( AST * self )
{
    if ( self != 0 )
    {
        self -> ~AST();
        free ( self );
    }
}

AST :: AST ()
: ParseTree ( Token ( PT_EMPTY ) )
{
}

AST :: AST ( Token :: TokenType p_tokenType ) // no-value token
: ParseTree ( Token ( p_tokenType ) )
{
}

AST :: AST ( const Token * p_token )
: ParseTree ( * p_token )
{
}

AST :: ~AST ()
{
}

void
AST :: AddNode ( ctx_t ctx, AST * p_child )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    AddChild ( ctx, p_child );
}

void
AST :: AddNode ( ctx_t ctx, const Token * p_child )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    AddChild ( ctx, AST :: Make ( ctx, p_child ) );
}

// AST_FQN
AST_FQN *
AST_FQN :: Make ( ctx_t ctx, const Token* token )
{
    void * ret = malloc ( sizeof ( AST_FQN ) );
    if ( ret == 0 )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        SYSTEM_ERROR ( xcNoMemory, "" );
        return 0;
    }
    return new ( ret ) AST_FQN ( token );
}

AST_FQN :: AST_FQN ( const Token* p_token )
:   AST ( p_token ),
    m_version ( 0 )
{
    assert ( p_token -> GetType () == PT_IDENT );
}

AST_FQN :: ~AST_FQN ()
{
}

uint32_t
AST_FQN :: NamespaceCount() const
{
    uint32_t count = ChildrenCount ();
    return count > 0 ? ChildrenCount () - 1 : 0;
}

void
AST_FQN :: GetIdentifier ( String & p_str ) const
{
    uint32_t count = ChildrenCount ();
    if ( count > 0 )
    {
        StringInitCString ( & p_str, GetChild ( count - 1 ) -> GetTokenValue () );
    }
    else
    {
        CONST_STRING ( & p_str, "" );
    }
}

void
AST_FQN :: GetFullName ( char* p_buf, size_t p_bufSize ) const
{
    GetPartialName ( p_buf, p_bufSize, ChildrenCount () );
}

void
AST_FQN :: GetPartialName ( char* p_buf, size_t p_bufSize, uint32_t p_lastMember ) const
{
    uint32_t count = ChildrenCount ();
    if ( p_lastMember < count )
    {
        count = p_lastMember + 1;
    }
    size_t offset = 0;
    for ( uint32_t i = 0 ; i < count; ++ i )
    {
        size_t num_writ;
        rc_t rc = string_printf ( p_buf + offset, p_bufSize - offset - 1, & num_writ, "%s%s",
                                  GetChild ( i ) -> GetTokenValue (),
                                  i == count - 1 ? "" : ":" );
        offset += num_writ;
        if ( rc != 0 )
        {
            break;
        }
    }

    p_buf [ p_bufSize - 1 ] = 0;
}

void
AST_FQN :: SetVersion ( const char* p_version )
{   // assume the token comes from a scanner which guarantees correctness
    assert ( p_version != 0 );
    assert ( p_version [ 0 ] == '#' );
    const char* str = p_version + 1;
    uint32_t len = string_measure ( str, 0 );
    const char *dot = string_chr ( str, len, '.' );
    m_version = strtou32 ( str, 0, 10 ) << 24;
    if ( dot != 0 )
    {
        str = dot + 1;
        len = string_measure ( str, 0 );
        dot = string_chr ( str, len, '.' );
        m_version |= strtou32 ( str, 0, 10 ) << 16;
        if ( dot != 0 )
        {
            m_version |= strtou32 ( dot + 1, 0, 10 );
        }
    }
}

AST_FQN *
ncbi :: SchemaParser :: ToFQN ( AST * p_ast)
{
    assert ( p_ast != 0 );
    if ( p_ast -> GetTokenType () == PT_IDENT )
    {
        return static_cast < AST_FQN * > ( p_ast );
    }
    return 0;
}

const AST_FQN *
ncbi :: SchemaParser :: ToFQN ( const AST * p_ast)
{
    assert ( p_ast != 0 );
    if ( p_ast -> GetTokenType () == PT_IDENT )
    {
        return static_cast < const AST_FQN * > ( p_ast );
    }
    return 0;
}
