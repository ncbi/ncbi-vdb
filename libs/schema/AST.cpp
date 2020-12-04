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

#include "../vdb/schema-parse.h"
#include "../vdb/dbmgr-priv.h"
#include "../vdb/schema-expr.h"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast-tokens.h"

#include "ASTBuilder.hpp"

using namespace std;

// AST

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
    AST_FQN * ret = dynamic_cast < AST_FQN * > ( p_ast );
    assert ( ret != 0 );
    return ret;
}

const AST_FQN *
ncbi :: SchemaParser :: ToFQN ( const AST * p_ast)
{
    assert ( p_ast != 0 );
    const AST_FQN * ret = dynamic_cast < const AST_FQN * > ( p_ast );
    assert ( ret != 0 );
    return ret;
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
    SetToken ( Token ( p_type ) );
}

SExpression *
AST_Expr :: EvaluateConst ( ASTBuilder & p_builder ) const
{
    SExpression * ret = MakeExpression ( p_builder );
    if ( ret != 0 )
    {
        switch ( ret -> var )
        {
        case eConstExpr:
            break;
        case eVectorExpr:
            // MakeVectorConstant() makes sure all elements are const
            break;
        default:
            p_builder . ReportError ( GetLocation (), "Not a constant expression" );
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
        case eForward:
        case eVirtual:
            return SSymExprMake ( p_builder, eFwdExpr, p_sym );
        case eConstant:
        {
            const SConstant * cnst = reinterpret_cast < const SConstant * > ( p_sym -> u . obj );
            assert ( cnst -> expr != NULL );
            atomic32_inc ( & const_cast < SExpression * > ( cnst -> expr ) -> refcount );
            return const_cast < SExpression * > ( cnst -> expr );
        }
        case eFunction :
            p_builder . ReportError ( GetLocation (), "Function expressions are not yet implemented" );
            break;
        default:
            p_builder . ReportError ( GetLocation (), "Object cannot be used in this context", p_sym -> name );
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
    assert ( GetTokenType () == FLOAT_ || GetTokenType () == EXP_FLOAT );
    SConstExpr * x = p_builder . Alloc < SConstExpr > ( sizeof * x - sizeof x -> u + sizeof x -> u . u64 [ 0 ] );
    if ( x != 0 )
    {
        const char * val = GetTokenValue ();
        char * end;
        double f64 = strtod ( val, & end );
        if ( ( end - val ) != ( int ) string_size ( val ) )
        {
            p_builder . ReportError ( GetLocation (), "Invalid floating point constant" );
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
            SExpression * vx = ToExpr ( values . GetChild ( i ) ) -> EvaluateConst ( p_builder );
            if ( vx == 0 )
            {
                good = false;
                break;
            }
            if ( vx -> var == eVectorExpr )
            {
                p_builder . ReportError ( GetLocation (), "Nested vector constants are not allowed" );
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
AST_Expr :: MakeBool ( ASTBuilder & p_builder ) const
{
    SConstExpr * x = p_builder . Alloc < SConstExpr > ( sizeof * x - sizeof x -> u + sizeof x -> u . b [ 0 ] );
    if ( x != 0 )
    {
        x -> u . b [ 0 ] = GetTokenType () == KW_true;
        x -> dad . var = eConstExpr;
        atomic32_set ( & x -> dad . refcount, 1 );
        x -> td . type_id = p_builder . IntrinsicTypeId ( "bool" );
        x -> td . dim = 1;

        return & x -> dad;
    }
    return 0;
}

SExpression *
AST_Expr :: MakeNegate ( ASTBuilder & p_builder ) const
{
    assert ( GetTokenType () == PT_NEGATE );
    assert ( ChildrenCount () == 1 );

    SExpression * xp = ToExpr ( GetChild ( 0 ) ) -> MakeExpression ( p_builder );
    if ( xp != 0 )
    {
        switch ( xp -> var )
        {
        case eConstExpr:
            {
                SConstExpr * cx = reinterpret_cast < SConstExpr * > ( xp );
                if ( cx -> td . dim < 2 )
                {
                    const SDatatype *dt = VSchemaFindTypeid ( p_builder . GetSchema(), cx -> td . type_id );
                    if ( dt != NULL )
                    {
                        static atomic32_t s_I8_id;
                        static atomic32_t s_I16_id;
                        static atomic32_t s_I32_id;
                        static atomic32_t s_I64_id;

                        switch ( dt -> domain )
                        {
                        case vtdUint:
                            switch ( dt -> size )
                            {
                            case 8:
                                cx -> td . type_id  = VSchemaCacheIntrinsicTypeId ( p_builder . GetSchema(), & s_I8_id, "I8" );
                                break;
                            case 16:
                                cx -> td . type_id  = VSchemaCacheIntrinsicTypeId ( p_builder . GetSchema(), & s_I16_id, "I16" );
                                break;
                            case 32:
                                cx -> td . type_id  = VSchemaCacheIntrinsicTypeId ( p_builder . GetSchema(), & s_I32_id, "I32" );
                                break;
                            case 64:
                                cx -> td . type_id  = VSchemaCacheIntrinsicTypeId ( p_builder . GetSchema(), & s_I64_id, "I64" );
                                break;
                            }
                            /* no break */
                        case vtdInt:
                            switch ( dt -> size )
                            {
                            case 8:
                                cx -> u . i8 [ 0 ] = - cx -> u . i8 [ 0 ];
                                break;
                            case 16:
                                cx -> u . i16 [ 0 ] = - cx -> u . i16 [ 0 ];
                                break;
                            case 32:
                                cx -> u . i32 [ 0 ] = - cx -> u . i32 [ 0 ];
                                break;
                            case 64:
                                cx -> u . i64 [ 0 ] = - cx -> u . i64 [ 0 ];
                                break;
                            }
                            break;
                        case vtdFloat:
                            switch ( dt -> size )
                            {
                            case 32:
                                cx -> u . f32 [ 0 ] = - cx -> u . f32 [ 0 ];
                                break;
                            case 64:
                                cx -> u . f64 [ 0 ] = - cx -> u . f64 [ 0 ];
                                break;
                            }
                            break;
                        }

                        return xp;
                    }
                    // const expression of an unknown type, must have reported an error already
                }
                else
                {
                    p_builder . ReportError ( GetLocation (), "Negation applied to a non-scalar" );
                }
            }
            break;

        case eIndirectExpr:
            {   /* if type is known, at least verify domain */
                const SSymExpr * sx = ( const SSymExpr* ) xp;
                const SExpression * td = ( ( const SIndirectConst* ) sx -> _sym -> u . obj ) -> td;
                if ( td != NULL )
                {
                    const STypeExpr *tx = ( const STypeExpr* ) td;
                    if ( tx-> dad . var == eTypeExpr && tx -> resolved )
                    {
                        /* cannot have formats, but this is verified elsewhere */
                        if ( tx -> fd . fmt == 0 && tx -> fd . td . dim < 2 )
                        {
                            /* determine domain */
                            const SDatatype *dt = VSchemaFindTypeid ( p_builder . GetSchema(), tx -> fd . td . type_id );
                            if ( dt != NULL && dt -> domain == vtdUint )
                            {
                                p_builder . ReportError ( GetLocation (), "Negation applied to an unsigned integer" );
                            }
                        }
                    }
                }

                SUnaryExpr * x = p_builder . Alloc < SUnaryExpr > ();
                if ( x != 0 )
                {
                    x -> dad . var = eNegateExpr;
                    atomic32_set ( & x -> dad . refcount, 1 );
                    x -> expr = xp;
                    return & x -> dad;
                }
            }
            break;

        default:
            p_builder . ReportError ( GetLocation (), "Negation applied to a non-const operand" );
            break;
        }
        SExpressionWhack ( xp );
    }

    return 0;
}

SExpression *
AST_Expr :: MakeCast ( ASTBuilder & p_builder ) const
{
    assert ( GetTokenType () == PT_CASTEXPR );
    assert ( ChildrenCount () == 2 );

    STypeExpr * type = p_builder . MakeTypeExpr ( * GetChild ( 0 ) );
    if ( type != 0 )
    {
        SExpression * expr = ToExpr ( GetChild ( 1 ) ) -> MakeExpression ( p_builder );
        if ( expr != 0 )
        {
            SBinExpr * x = p_builder . Alloc < SBinExpr > ();
            if ( x != 0 )
            {
                x -> dad . var = eCastExpr;
                atomic32_set ( & x -> dad . refcount, 1 );
                x -> left = & type -> dad;
                x -> right = expr;
                return & x -> dad;
            }
            SExpressionWhack ( expr );
        }
        SExpressionWhack ( & type -> dad );
    }

    return 0;
}

static
SExpression *
SMembExprMake ( ASTBuilder & p_builder, const KSymbol* p_obj, const KSymbol* p_mem, const SExpression * p_rowId )
{
    SMembExpr *x = p_builder . Alloc < SMembExpr > ();
    if ( x == 0 )
    {
        return 0;
    }

    x -> dad . var = eMembExpr;
    atomic32_set ( & x -> dad . refcount, 1 );

    x -> view = p_builder . GetView ();

    // link to the corresponding parameter of the current view
    uint32_t start = VectorStart ( & x -> view -> params );
    uint32_t count = VectorLength ( & x -> view -> params );
    for ( uint32_t i = 0; i < count; ++i )
    {
        if ( VectorGet ( & x -> view -> params, start + i ) == p_obj )
        {
            x -> paramId = start + i;
            break;
        }
    }

    x -> member = p_mem;
    x -> rowId = p_rowId;

    return & x -> dad;
}

static
SExpression *
MakeSMembExpr ( ASTBuilder & p_builder, const AST & p_struc, const AST & p_member, const AST_Expr * p_rowId = 0 )
{
    assert ( p_struc . GetTokenType () == PT_IDENT );
    assert ( p_struc . ChildrenCount () == 1 );
    assert ( p_member . GetTokenType () == PT_IDENT );
    assert ( p_member . ChildrenCount () == 1 );

    const KSymbol * sym = p_builder . Resolve ( p_struc . GetChild ( 0 ) -> GetLocation (),
                                                p_struc . GetChild ( 0 ) -> GetTokenValue (),
                                                true );
    if ( sym != 0 )
    {
        const SExpression * rowId = 0;
        if ( p_rowId != 0 )
        {
            rowId = p_rowId -> MakeExpression ( p_builder );
            if ( rowId == 0 )
            {
                return 0;
            }
        }

        switch ( sym -> type )
        {
        case eTable:
            {
                const STable * t = static_cast < const STable * > ( sym -> u . obj );
                // find member . GetChild ( 0 ) in t -> scope
                String memName;
                StringInitCString ( & memName, p_member . GetChild ( 0 ) -> GetTokenValue () );
                const KSymbol * mem = ( const KSymbol* ) BSTreeFind ( & t -> scope, & memName, KSymbolCmp );
                if ( mem != 0 )
                {
                    assert ( mem -> type == eColumn || mem -> type == eProduction );
                    return SMembExprMake ( p_builder, sym, mem, rowId );
                }
                else
                {
                    p_builder . ReportError ( p_member . GetLocation (), "Column/production not found", memName );
                }
            }
            break;
        case eView:
            {
                const SView * v = static_cast < const SView * > ( sym -> u . obj );
                // find member . GetChild ( 0 ) in v -> scope
                String memName;
                StringInitCString ( & memName, p_member . GetChild ( 0 ) -> GetTokenValue () );
                const KSymbol * mem = ( const KSymbol* ) BSTreeFind ( & v -> scope, & memName, KSymbolCmp );
                if ( mem != 0 )
                {
                    assert ( mem -> type == eColumn || mem -> type == eProduction );
                    return SMembExprMake ( p_builder, sym, mem, rowId );
                }
                else
                {
                    p_builder . ReportError ( p_member . GetLocation (), "Column/production not found", memName );
                }
            }
            break;
    default:
            //error
            break;
        }
    }

    return 0;
}

SExpression *
AST_Expr :: MakeMember ( ASTBuilder & p_builder ) const
{
    assert ( GetTokenType () == PT_MEMBEREXPR );
    assert ( ChildrenCount () == 2 ); // ident, ident
    return MakeSMembExpr ( p_builder, * GetChild ( 0 ), * GetChild ( 1 ) );
}

SExpression *
AST_Expr :: MakeJoin ( ASTBuilder & p_builder ) const
{
    assert ( GetTokenType () == PT_JOINEXPR );
    assert ( ChildrenCount () == 3 ); // ident, rowid-expr, ident
    return MakeSMembExpr ( p_builder,
                           * GetChild ( 0 ),
                           * GetChild ( 2 ),
                           ToExpr ( GetChild ( 1 ) ) );
}

SExpression *
AST_Expr :: MakeExpression ( ASTBuilder & p_builder ) const
{
    switch ( GetTokenType () )
    {
    case PT_EMPTY: // expr [ | expr | ... ]
        {
            uint32_t count = ChildrenCount ();
            assert ( count > 0 );

            SExpression * xp = ToExpr (  GetChild ( 0 ) ) -> MakeExpression ( p_builder );
            for ( uint32_t i = 0; i < count - 1; ++i )
            {
                SBinExpr * x = p_builder . Alloc < SBinExpr > ();
                if ( x == NULL )
                {
                    SExpressionWhack ( xp );
                    return 0;
                }
                x -> dad . var = eCondExpr;
                atomic32_set ( & x -> dad . refcount, 1 );
                x -> left = xp;
                x -> right = ToExpr (  GetChild ( i + 1 ) ) -> MakeExpression ( p_builder );
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

    case FLOAT_:
    case EXP_FLOAT:
        return MakeFloat ( p_builder );

    case STRING:
        return MakeString ( p_builder );
    case ESCAPED_STRING:
        return MakeEscapedString ( p_builder );

    case PT_CONSTVECT:
        return MakeVectorConstant ( p_builder );

    case KW_true:
    case KW_false:
        return MakeBool ( p_builder );

    case PT_IDENT:
        {
            const AST_FQN * fqn = ToFQN ( GetChild ( 0 ) );
            if ( fqn != 0 )
            {
                const KSymbol * sym = p_builder . Resolve ( * fqn, false );
                if ( sym != 0 )
                {
                    return MakeSymExpr ( p_builder, sym );
                }
                else
                {   // Resolve() has created all the required namespaces; create the identifier as a forward reference
                    SSymExpr * x = p_builder . Alloc < SSymExpr > ();
                    x -> _sym = p_builder . CreateFqnSymbol ( * fqn, eForward, NULL );
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
        }

    case PHYSICAL_IDENTIFIER_1_0 :
        {
            const KSymbol * sym = p_builder . Resolve ( GetLocation (), GetTokenValue (), false );
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

    case '@':
        return MakeSymExpr ( p_builder, p_builder . Resolve ( GetLocation (), "@" ) );

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
                    const AST_FQN & fqn = * ToFQN ( GetChild ( 1 ) );
                    const KSymbol * sym = p_builder . Resolve ( fqn, true );
                    if ( sym != 0 )
                    {
                        const SNameOverload * vf = static_cast < const SNameOverload * > ( sym -> u . obj );
                        switch ( vf -> name -> type )
                        {
                        case eScriptFunc:
                            fx -> dad . var = eScriptExpr;
                            // fall through
                        case eFunction:
                            fx -> func = static_cast < const SFunction * > ( p_builder . SelectVersion ( fqn, * sym, SFunctionCmp, & fx -> version ) );
                            if ( fx -> func != 0 )
                            {
                                fx -> version_requested = fx -> version != 0;
                                return & fx -> dad;
                            }
                            break;

                        default:
                            p_builder . ReportError ( "Not a function", fqn );
                            break;
                        }
                    }
                }
                SExpressionWhack ( & fx -> dad );
            }
        }
        break;

    case PT_NEGATE:
        return MakeNegate ( p_builder );

    case PT_CASTEXPR:
        return MakeCast ( p_builder );

    case PT_MEMBEREXPR:
        return MakeMember ( p_builder );

    case PT_JOINEXPR:
        return MakeJoin ( p_builder );

    default:
        p_builder . ReportError ( GetLocation (), "Not yet implemented" );
        break;
    }
    return 0;
}

AST_Expr  *
ncbi :: SchemaParser :: ToExpr ( AST * p_ast)
{
    assert ( p_ast != 0 );
    AST_Expr * ret = dynamic_cast < AST_Expr * > ( p_ast );
    assert ( ret != 0 );
    return ret;
}

const AST_Expr *
ncbi :: SchemaParser :: ToExpr ( const AST * p_ast)
{
    assert ( p_ast != 0 );
    const AST_Expr * ret = dynamic_cast < const AST_Expr * > ( p_ast );
    assert ( ret != 0 );
    return ret;
}
