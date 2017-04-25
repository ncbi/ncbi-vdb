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
#include <ctype.h>
#include <os-native.h>

#include <klib/symbol.h>
#include <klib/printf.h>
#include <klib/rc.h>

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

#include "ASTBuilder.hpp"

using namespace std;

// AST

SchemaToken st = { PT_EMPTY, NULL, 0, 0, 0 };

AST :: AST ()
: ParseTree ( st )
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

AST :: AST ( const Token * p_token, AST * p_child1 )
: ParseTree ( * p_token )
{
    assert ( p_child1 != 0 );
    AddNode ( p_child1 );
}

AST :: AST ( const Token * p_token, AST * p_child1, AST * p_child2 )
: ParseTree ( * p_token )
{
    assert ( p_child1 != 0 );
    AddNode ( p_child1 );
    assert ( p_child2 != 0 );
    AddNode ( p_child2 );
}

AST :: AST ( const Token * p_token, AST * p_child1, AST * p_child2, AST * p_child3 )
: ParseTree ( * p_token )
{
    assert ( p_child1 != 0 );
    AddNode ( p_child1 );
    assert ( p_child2 != 0 );
    AddNode ( p_child2 );
    assert ( p_child3 != 0 );
    AddNode ( p_child3 );
}

AST :: AST ( const Token * p_token,
             AST * p_child1,
             AST * p_child2,
             AST * p_child3,
             AST * p_child4,
             AST * p_child5 )
: ParseTree ( * p_token )
{
    assert ( p_child1 != 0 );
    AddNode ( p_child1 );
    assert ( p_child2 != 0 );
    AddNode ( p_child2 );
    assert ( p_child3 != 0 );
    AddNode ( p_child3 );
    assert ( p_child4 != 0 );
    AddNode ( p_child4 );
    assert ( p_child5 != 0 );
    AddNode ( p_child5 );
}

AST :: AST ( const Token * p_token,
             AST * p_child1,
             AST * p_child2,
             AST * p_child3,
             AST * p_child4 )
: ParseTree ( * p_token )
{
    assert ( p_child1 != 0 );
    AddNode ( p_child1 );
    assert ( p_child2 != 0 );
    AddNode ( p_child2 );
    assert ( p_child3 != 0 );
    AddNode ( p_child3 );
    assert ( p_child4 != 0 );
    AddNode ( p_child4 );
}

AST :: AST ( const Token * p_token,
             AST * p_child1,
             AST * p_child2,
             AST * p_child3,
             AST * p_child4,
             AST * p_child5,
             AST * p_child6 )
: ParseTree ( * p_token )
{
    assert ( p_child1 != 0 );
    AddNode ( p_child1 );
    assert ( p_child2 != 0 );
    AddNode ( p_child2 );
    assert ( p_child3 != 0 );
    AddNode ( p_child3 );
    assert ( p_child4 != 0 );
    AddNode ( p_child4 );
    assert ( p_child5 != 0 );
    AddNode ( p_child5 );
    assert ( p_child6 != 0 );
    AddNode ( p_child6 );
}

void
AST :: AddNode ( AST * p_child )
{
    AddChild ( p_child );
}

void
AST :: AddNode ( const Token * p_child )
{
    AddChild ( new AST ( p_child ) );
}

// AST_Schema

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


// AST_FQN

AST_FQN :: AST_FQN ( const Token* p_token )
:   AST ( p_token ),
    m_version ( 0 )
{
    assert ( p_token -> GetType () == PT_IDENT );
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

    p_buf [ p_bufSize ] = 0;
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

// AST_Expr

AST_Expr :: AST_Expr ( const Token* p_token )
: AST ( p_token )
{
}

AST_Expr :: AST_Expr ( AST_FQN* p_fqn )
: AST ( & p_fqn -> GetToken () )
{
    AddNode ( p_fqn );
}

AST_Expr :: AST_Expr ( AST_Expr* p_fqn )
{
    AddNode ( p_fqn );
}

AST_Expr :: AST_Expr ( Token :: TokenType p_type )    // '@' etc
{
    SchemaToken st = { p_type, NULL, 0, 0, 0 };
    SetToken ( Token ( st ) );
}

SExpression *
AST_Expr :: EvaluateConst ( ASTBuilder & p_builder ) const
{   //TODO. for now, only handles a literal int constant and a direct reference to a schema parameter
    SExpression * ret = MakeExpression ( p_builder );
    if ( ret != 0 )
    {
        switch ( ret -> var )
        {
        case eConstExpr:
            break;
        case eVectorExpr:
            //TODO: make sure all componenet are const
            break;
        default:
            p_builder . ReportError ( "Not a constant expression" );
            SExpressionWhack ( ret );
            ret = 0;
            break;
        }
    }
    return ret;
}

static
SExpression * SSymExprMake ( ASTBuilder & p_builder, uint32_t p_type, const KSymbol* p_sym )
{
    SSymExpr *x = p_builder . Alloc < SSymExpr > ();
    if ( x == 0 )
    {
        return 0;
    }

    x -> dad . var = p_type;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> _sym = p_sym;
    x -> alt = false;

    return & x -> dad;
}

SExpression *
AST_Expr :: MakeSymExpr ( ASTBuilder & p_builder, const KSymbol* p_sym ) const
{
    if ( p_sym != 0 )
    {
        switch ( p_sym -> type )
        {
        case eSchemaParam :
        case eFactParam :
            return SSymExprMake ( p_builder, eIndirectExpr, p_sym );
        case eProduction:
            return SSymExprMake ( p_builder, eProdExpr, p_sym );
        case eFuncParam :
            return SSymExprMake ( p_builder, eParamExpr, p_sym );
        case eColumn :
            return SSymExprMake ( p_builder, eColExpr, p_sym );
        case ePhysMember:
            return SSymExprMake ( p_builder, ePhysExpr, p_sym );
        case eFunction :
            p_builder . ReportError ( "Function expressions are not yet implemented" );
            break;
        default:
            p_builder . ReportError ( "Object cannot be used in this context: '%S'", & p_sym -> name );
            break;
        }
    }
    return 0;
}

/* hex_to_int
 *  where 'c' is known to be hex
 */
static
unsigned int CC hex_to_int ( char c )
{
    int i = c - '0';
    if ( c > '9' )
    {
        if ( c < 'a' )
            i = c - 'A' + 10;
        else
            i = c - 'a' + 10;
    }

    assert ( i >= 0 && i < 16 );
    return i;
}

SExpression *
AST_Expr :: MakeUnsigned ( ASTBuilder & p_builder ) const
{
    assert ( GetTokenType () == PT_UINT );
    SConstExpr * x = p_builder . Alloc < SConstExpr > ( sizeof * x - sizeof x -> u + sizeof x -> u . u64 [ 0 ] );
    if ( x != 0 )
    {
        assert ( ChildrenCount () == 1 );
        const char * val = GetChild ( 0 ) -> GetTokenValue ();
        uint64_t i64 = 0;
        switch ( GetChild ( 0 ) -> GetTokenType () )
        {
        case DECIMAL:
            {
                uint32_t i = 0;
                while ( val [ i ] != 0 )
                {
                    i64 *= 10;
                    i64 += val [ i ] - '0';
                    ++ i;
                }
            }
            break;
        case HEX:
            {
                uint32_t i = 2;
                while ( val [ i ] != 0 )
                {
                    i64 <<= 4;
                    i64 += hex_to_int ( val [ i ] );
                    ++ i;
                }
            }
            break;
        case OCTAL:
            {
                uint32_t i = 1;
                while ( val [ i ] != 0 )
                {
                    i64 <<= 3;
                    i64 += val [ i ] - '0';
                    ++ i;
                }
            }
            break;
        default:
            assert ( 0 );
        }

        x -> u . u64 [ 0 ] = i64;
        x -> dad . var = eConstExpr;
        atomic32_set ( & x -> dad . refcount, 1 );
        x -> td . type_id = p_builder . IntrinsicTypeId ( "U64" );
        x -> td . dim = 1;

        return & x -> dad;
    }
    return 0;
}

SExpression *
AST_Expr :: MakeFloat ( ASTBuilder & p_builder ) const
{
    assert ( GetTokenType () == FLOAT || GetTokenType () == EXP_FLOAT );
    SConstExpr * x = p_builder . Alloc < SConstExpr > ( sizeof * x - sizeof x -> u + sizeof x -> u . u64 [ 0 ] );
    if ( x != 0 )
    {
        const char * val = GetTokenValue ();
        char * end;
        double f64 = strtod ( val, & end );
        if ( ( end - val ) != ( int ) string_size ( val ) )
        {
            p_builder . ReportError ( "Invalid floating point constant" );
            return 0;
        }

        x -> u . f64 [ 0 ] = f64;
        x -> dad . var = eConstExpr;
        atomic32_set ( & x -> dad . refcount, 1 );
        x -> td . type_id = p_builder . IntrinsicTypeId ( "F64" );
        x -> td . dim = 1;

        return & x -> dad;
    }
    return 0;
}

SExpression *
AST_Expr :: MakeString ( ASTBuilder & p_builder ) const
{
    assert ( GetTokenType () == STRING );
    const char * val = GetTokenValue ();
    size_t size = string_size ( val ) - 2; // minus quotes
    SConstExpr * x = p_builder . Alloc < SConstExpr > ( sizeof * x - sizeof x -> u + size + 1 );
    if ( x != 0 )
    {
        string_copy ( x -> u . utf8, size + 1, val + 1, size ); // add 1 for NUL
        x -> dad . var = eConstExpr;
        atomic32_set ( & x -> dad . refcount, 1 );
        x -> td . type_id = p_builder . IntrinsicTypeId ( "ascii" );
        x -> td . dim = ( uint32_t ) size;

        return & x -> dad;
    }
    return 0;
}

SExpression *
AST_Expr :: MakeEscapedString ( ASTBuilder & p_builder ) const
{
    assert ( GetTokenType () == ESCAPED_STRING );
    const char * val = GetTokenValue ();
    size_t size = string_size ( val ) - 2; // minus quotes
    SConstExpr * x = p_builder . Alloc < SConstExpr > ( sizeof * x - sizeof x -> u + size + 1 );
    if ( x != 0 )
    {
        char * buffer = x -> u . utf8;
        uint32_t j = 0 ;
        uint32_t i = 1; // skip the opening quote
        while ( i <= size )
        {
            if ( val [ i ] == '\\' )
            {
                ++ i;
                if ( i > size )
                {
                    break;
                }

                switch ( val [ i ] )
                {
                    /* control characters */
                case 'n':
                    buffer [ j ] = '\n';
                    break;
                case 't':
                    buffer [ j ] = '\t';
                    break;
                case 'r':
                    buffer [ j ] = '\r';
                    break;
                case '0':
                    buffer [ j ] = '\0';
                    break;

                case 'a':
                    buffer [ j ] = '\a';
                    break;
                case 'b':
                    buffer [ j ] = '\b';
                    break;
                case 'v':
                    buffer [ j ] = '\v';
                    break;
                case 'f':
                    buffer [ j ] = '\f';
                    break;

                case 'x': case 'X':
                    /* expect 2 additional hex characters */
                    if ( ( i + 2 ) < size &&
                        isxdigit ( val [ i + 1 ] ) &&
                        isxdigit ( val [ i + 2 ] ) )
                    {
                        /* go ahead and convert */
                        buffer [ j ] = ( char )
                            ( ( hex_to_int ( val [ i + 1 ] ) << 4 ) |
                                hex_to_int ( val [ i + 2 ] ) );
                        i += 2;
                        break;
                    }
                    /* no break */

                    /* just quote self */
                default:
                    buffer [ j ] = val [ i ];
                }
            }
            else
            {
                buffer [ j ] = val [ i ];
            }

            ++ j;
            ++ i;
        }
        buffer [ j ] = 0;

        x -> dad . var = eConstExpr;
        atomic32_set ( & x -> dad . refcount, 1 );
        x -> td . type_id = p_builder . IntrinsicTypeId ( "ascii" );
        x -> td . dim = j;

        return & x -> dad;
    }
    return 0;
}

SExpression *
AST_Expr :: MakeVectorConstant ( ASTBuilder & p_builder ) const
{
    assert ( GetTokenType () == PT_CONSTVECT );
    SVectExpr * x = p_builder . Alloc < SVectExpr > ();
    if ( x != 0 )
    {
        x -> dad . var = eVectorExpr;
        atomic32_set ( & x -> dad . refcount, 1 );
        VectorInit ( & x -> expr, 0, 16 );

        assert ( ChildrenCount () == 1 );
        const AST & values = * GetChild ( 0 );
        uint32_t count = values . ChildrenCount ();
        bool good = true;
        for ( uint32_t i = 0 ; i != count; ++i )
        {
            const AST_Expr * v = dynamic_cast < const AST_Expr * > ( values . GetChild ( i ) );
            assert ( v != 0 );
            SExpression * vx = v -> EvaluateConst ( p_builder );
            if ( vx == 0 )
            {
                good = false;
                break;
            }
            if ( vx -> var == eVectorExpr )
            {
                p_builder . ReportError ( "Nested vector constants are not allowed" );
                good = false;
                break;
            }
            if ( ! p_builder . VectorAppend ( x -> expr, 0, vx ) )
            {
                SExpressionWhack ( vx );
                good = false;
                break;
            }
        }

        if ( good )
        {
            return & x -> dad;
        }

        SExpressionWhack ( & x -> dad );
    }
    return 0;
}

SExpression *
AST_Expr :: MakeExpression ( ASTBuilder & p_builder ) const
{   //TODO: complete
    switch ( GetTokenType () )
    {
    case PT_EMPTY: // expr [ | expr | ... ]
        {
            uint32_t count = ChildrenCount ();
            assert ( count > 0 );
            const AST_Expr * left = dynamic_cast < const AST_Expr * > (  GetChild ( 0 ) );
            assert ( left != 0 );

            SExpression * xp = left -> MakeExpression ( p_builder );
            for ( uint32_t i = 0; i < count - 1; ++i )
            {
                const AST_Expr * right = dynamic_cast < const AST_Expr * > (  GetChild ( i + 1 ) );
                assert ( right != 0 );
                SBinExpr * x = p_builder . Alloc < SBinExpr > ();
                if ( x == NULL )
                {
                    SExpressionWhack ( xp );
                    return 0;
                }
                x -> dad . var = eCondExpr;
                atomic32_set ( & x -> dad . refcount, 1 );
                x -> left = xp;
                x -> right = right -> MakeExpression ( p_builder );
                if ( x -> right == 0 )
                {
                    SExpressionWhack ( xp );
                    return 0;
                }
                xp = & x -> dad;
            }

            return xp;
        }

    case PT_UINT:
        return MakeUnsigned ( p_builder );

    case FLOAT:
    case EXP_FLOAT:
        return MakeFloat ( p_builder );

    case STRING:
        return MakeString ( p_builder );
    case ESCAPED_STRING:
        return MakeEscapedString ( p_builder );

    case PT_CONSTVECT:
        return MakeVectorConstant ( p_builder );
        break;

    case PT_IDENT:
        {
            const AST_FQN* fqn = dynamic_cast < const AST_FQN * > ( GetChild ( 0 ) );
            assert ( fqn != 0 );
            return MakeSymExpr ( p_builder, p_builder . Resolve ( * fqn ) );
        }

    case PHYSICAL_IDENTIFIER_1_0 :
        {
            const KSymbol * sym = p_builder . Resolve ( GetTokenValue (), false );
            if ( sym != 0 )
            {
                return MakeSymExpr ( p_builder, sym );
            }
            else
            {
                SSymExpr * x = p_builder . Alloc < SSymExpr > ();
                x -> _sym = p_builder . CreateConstSymbol ( GetTokenValue (), eForward, NULL );
                if (x -> _sym != 0 )
                {
                    x -> dad . var = eFwdExpr;
                    atomic32_set ( & x -> dad . refcount, 1 );
                    x -> alt = false;
                    return & x -> dad;
                }
                SExpressionWhack ( & x -> dad );
            }
        }
        break;

    case PT_AT:
        return MakeSymExpr ( p_builder, p_builder . Resolve ( "@" ) );

    case PT_FUNCEXPR:
        {   // schema_parms_opt fqn_opt_vers factory_parms_opt func_parms_opt
            assert ( ChildrenCount () == 4 );

            SFuncExpr * fx = p_builder . Alloc < SFuncExpr > ();
            if ( fx != 0 )
            {
                /* initialize */
                fx -> dad . var = eFuncExpr;
                atomic32_set ( & fx -> dad . refcount, 1 );
                fx -> func = NULL;
                VectorInit ( & fx -> schem, 0, 4 );
                VectorInit ( & fx -> pfact, 0, 8 );
                VectorInit ( & fx -> pfunc, 0, 8 );
                fx -> version = 0;
                fx -> version_requested = false;
                fx -> untyped = false;

                if ( p_builder . FillSchemaParms ( * GetChild ( 0 ), fx -> schem ) &&
                     p_builder . FillFactoryParms ( * GetChild ( 2 ), fx -> pfact ) &&
                     p_builder . FillArguments ( * GetChild ( 3 ), fx -> pfunc) )
                {
                    assert ( GetChild ( 1 ) -> GetTokenType () == PT_IDENT );
                    const AST_FQN * fqn = dynamic_cast < const AST_FQN * > ( GetChild ( 1 ) );
                    assert ( fqn != 0 );
                    const KSymbol * sym = p_builder . Resolve ( * fqn, true );
                    if ( sym != 0 )
                    {
                        const SNameOverload * vf = static_cast < const SNameOverload * > ( sym -> u . obj );
                        switch ( vf -> name -> type )
                        {
                        case eScriptFunc:
                            fx -> dad . var = eScriptExpr;
                            // fall through
                        case eFunction:
                            fx -> version = fqn -> GetVersion ();
                            fx -> version_requested = fx -> version != 0;
                            fx -> func = static_cast < const SFunction * > ( p_builder . SelectVersion ( * sym, SFunctionCmp, & fx -> version ) );
                            if ( fx -> func != 0 )
                            {
                                return & fx -> dad;
                            }
                            break;

                        default:
                            p_builder . ReportError ( "Not a function", * fqn );
                            break;
                        }
                    }
                }
                SExpressionWhack ( & fx -> dad );
            }
        }
        break;

    default:
        p_builder . ReportError ( "Not yet implemented" );
        break;
    }
    return 0;
}

// AST_ParamSig

AST_ParamSig :: AST_ParamSig ( const Token * p_token, AST * p_mandatory, AST * p_optional, bool p_variadic )
:   AST ( p_token ),
    m_isVariadic ( p_variadic )
{
    AddNode ( p_mandatory == 0 ? new AST () : p_mandatory );
    AddNode ( p_optional == 0 ? new AST () : p_optional );
}

// AST_Formal

AST_Formal :: AST_Formal ( const Token * p_token, AST * p_typespec, const Token* p_id, bool p_control )
:   AST ( p_token ),
    m_hasControl ( p_control )
{
    AddNode ( p_typespec );
    AddNode ( p_id );
}
