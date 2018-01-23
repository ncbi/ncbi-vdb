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

#include <vdb/extern.h>

#include "schema-priv.h"
#include "schema-parse.h"
#include "schema-expr.h"
#include "schema-dump.h"

#include <klib/symbol.h>
#include <klib/symtab.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * SExpression
 */
#if SLVL >= 1
void SExpressionWhack ( const SExpression *cself )
{
    SExpression *self = ( SExpression* ) cself;
    if ( self != NULL && atomic32_dec_and_test ( & self -> refcount ) )
    {
        switch ( self -> var )
        {
            case eTypeExpr:
            {
                STypeExpr *x = ( STypeExpr* ) self;
                SExpressionWhack ( x -> dim );
                break;
            }
            case eFuncExpr:
            case eScriptExpr:
            {
                SFuncExpr *x = ( SFuncExpr* ) self;
                VectorWhack ( & x -> schem, SExpressionVWhack, NULL );
                VectorWhack ( & x -> pfact, SExpressionVWhack, NULL );
                VectorWhack ( & x -> pfunc, SExpressionVWhack, NULL );
                break;
            }
            case ePhysEncExpr:
            {
                SPhysEncExpr *x = ( SPhysEncExpr* ) self;
                VectorWhack ( & x -> schem, SExpressionVWhack, NULL );
                VectorWhack ( & x -> pfact, SExpressionVWhack, NULL );
                break;
            }
            case eNegateExpr:
            {
                SUnaryExpr *x = ( SUnaryExpr* ) self;
                SExpressionWhack ( x -> expr );
                break;
            }
            case eCastExpr:
            case eCondExpr:
            {
                SBinExpr *x = ( SBinExpr* ) self;
                SExpressionWhack ( x -> left );
                SExpressionWhack ( x -> right );
                break;
            }
            case eVectorExpr:
            {
                SVectExpr *x = ( SVectExpr* ) self;
                VectorWhack ( & x -> expr, SExpressionVWhack, NULL );
                break;
            }
            case eMembExpr:
            {
                SMembExpr *x = ( SMembExpr* ) self;
                SExpressionWhack ( x -> rowId );
                break;
            }
        }

        free ( self );
    }
}

void CC SExpressionVWhack ( void *item, void *ignore )
{
    SExpression *self = item;
    SExpressionWhack ( self );
}
#endif

/* Mark
 */
#if SLVL >= 1
void CC SExpressionMark ( void * item, void * data )
{
    const SExpression * self = item;
    const VSchema * schema = data;
    if ( self != NULL ) switch ( self -> var )
    {
    case eTypeExpr:
        STypeExprMark ( ( const STypeExpr* ) self, schema );
        break;
    case eConstExpr:
        VSchemaTypeMark ( schema, ( ( const SConstExpr* ) self ) -> td . type_id );
        break;
    case eIndirectExpr:
    case eParamExpr:
    case eProdExpr:
    case eFwdExpr:
    case eFuncParamExpr:
    case eColExpr:
    case ePhysExpr:
        SSymExprMark ( ( const SSymExpr* ) self, schema );
        break;
    case eFuncExpr:
    case eScriptExpr:
        SFuncExprMark ( ( const SFuncExpr* ) self, schema );
        break;
    case ePhysEncExpr:
        SPhysEncExprMark ( ( const SPhysEncExpr* ) self, schema );
        break;
    case eNegateExpr:
        SExpressionMark ( ( void * )( ( const SUnaryExpr* ) self ) -> expr, data );
        break;
    case eCastExpr:
    case eCondExpr:
        SExpressionMark ( ( void * )( ( const SBinExpr* ) self ) -> left, data );
        SExpressionMark ( ( void * )( ( const SBinExpr* ) self ) -> right, data );
        break;
    case eVectorExpr:
        VectorForEach ( & ( ( const SVectExpr* ) self ) -> expr, false,
                        SExpressionMark, data );
        break;
    case eMembExpr:
        assert (false); //TODO SMembExprMark
        break;
    }
}
#endif

/* Dump
 */
#if SLVL >= 1
bool CC SExpressionListDump ( void *item, void *data )
{
    SDumper *b = data;
    const SExpression *self = ( const void* ) item;

    SDumperSep ( b );
    b -> rc = SExpressionDump ( self, b );
    SDumperSepString ( b, SDumperMode ( b ) == sdmCompact ? "," : ", " );

    return ( b -> rc != 0 ) ? true : false;
}

rc_t SExpressionBracketListDump ( const Vector *expr, SDumper *b,
    const char *begin, const char *end )
{
    SDumperSepString ( b, begin );
    if ( VectorDoUntil ( expr, false, SExpressionListDump, b ) )
        return b -> rc;
    return SDumperPrint ( b, end );
}

static
rc_t STypeExprDump ( const STypeExpr *self, SDumper *b )
{
    rc_t rc;
    if ( self -> fmt != NULL )
    {
        rc = SFormatDump ( self -> fmt, b );
        if ( rc != 0 )
            return rc;

        if ( self -> dt == NULL && self -> ts == NULL && self -> id == NULL )
            return 0;

        rc = SDumperWrite ( b, "/", 1 );
        if ( rc != 0 )
            return rc;
    }

    if ( self -> dt != NULL )
        rc = SDatatypeDump ( self -> dt, b );
    else if ( self -> ts != NULL )
        rc = STypesetDump ( self -> ts, b );
#if SLVL >= 3
    else if ( self -> id != NULL )
        rc = SIndirectTypeDump ( self -> id, b );
#endif
    else
        rc = SDumperWrite ( b, "NULL", 4 );

    if ( rc == 0 )
    {
        if ( SDumperMode ( b ) == sdmCompact )
        {
            if ( self -> dim != NULL )
                rc = SDumperPrint ( b, "[%E]", self -> dim );
            else if ( self -> fd . td . dim == 0 )
                rc = SDumperPrint ( b, "[*]" );
        }
        else
        {
            if ( self -> dim != NULL )
                rc = SDumperPrint ( b, " [ %E ]", self -> dim );
            else if ( self -> fd . td . dim == 0 )
                rc = SDumperPrint ( b, " [ * ]" );
        }
    }

    return rc;
}

static
rc_t SConstExprDump ( const SConstExpr *self, SDumper *b )
{
    rc_t rc = 0;
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;
    const SDatatype *dt = VSchemaFindTypeid ( b -> schema, self -> td . type_id );
    uint32_t bits = dt -> size;
    uint32_t i, dim = SDatatypeIntrinsicDim ( dt );
    assert ( dim != 0 && ( bits % dim ) == 0 );
    bits /= dim;
    dim *= self -> td . dim;

    /* open a vector expression */
    if ( dt -> domain < ddAscii && dim > 1 )
    {
        rc = SDumperPrint ( b, compact ? "[" : "[ " );
        if ( rc != 0 )
            return rc;
    }

    SDumperSepString ( b, "" );

    switch ( dt -> domain )
    {
    case ddBool:
        for ( i = 0; i < dim; ++ i )
        {
            rc = SDumperPrint ( b, "\v%s", self -> u . b [ i ] ? "true" : "false" );
            if ( rc != 0 )
                return rc;
            SDumperSepString ( b, compact ? "," : ", " );
        }
        break;
    case ddUint:
        switch ( bits )
        {
        case 8:
            for ( i = 0; i < dim; ++ i )
            {
                rc = SDumperPrint ( b, "\v%u", self -> u . u8 [ i ] );
                if ( rc != 0 )
                    return rc;
                SDumperSepString ( b, compact ? "," : ", " );
            }
            break;
        case 16:
            for ( i = 0; i < dim; ++ i )
            {
                rc = SDumperPrint ( b, "\v%u", self -> u . u16 [ i ] );
                if ( rc != 0 )
                    return rc;
                SDumperSepString ( b, compact ? "," : ", " );
            }
            break;
        case 32:
            for ( i = 0; i < dim; ++ i )
            {
                rc = SDumperPrint ( b, "\v%u", self -> u . u32 [ i ] );
                if ( rc != 0 )
                    return rc;
                SDumperSepString ( b, compact ? "," : ", " );
            }
            break;
        case 64:
            for ( i = 0; i < dim; ++ i )
            {
                rc = SDumperPrint ( b, "\v%lu", self -> u . u64 [ i ] );
                if ( rc != 0 )
                    return rc;
                SDumperSepString ( b, compact ? "," : ", " );
            }
            break;
        }
        break;
    case ddInt:
        switch ( bits )
        {
        case 8:
            for ( i = 0; i < dim; ++ i )
            {
                rc = SDumperPrint ( b, "\v%d", self -> u . i8 [ i ] );
                if ( rc != 0 )
                    return rc;
                SDumperSepString ( b, compact ? "," : ", " );
            }
            break;
        case 16:
            for ( i = 0; i < dim; ++ i )
            {
                rc = SDumperPrint ( b, "\v%d", self -> u . i16 [ i ] );
                if ( rc != 0 )
                    return rc;
                SDumperSepString ( b, compact ? "," : ", " );
            }
            break;
        case 32:
            for ( i = 0; i < dim; ++ i )
            {
                rc = SDumperPrint ( b, "\v%d", self -> u . i32 [ i ] );
                if ( rc != 0 )
                    return rc;
                SDumperSepString ( b, compact ? "," : ", " );
            }
            break;
        case 64:
            for ( i = 0; i < dim; ++ i )
            {
                rc = SDumperPrint ( b, "\v%ld", self -> u . i64 [ i ] );
                if ( rc != 0 )
                    return rc;
                SDumperSepString ( b, compact ? "," : ", " );
            }
            break;
        }
        break;
    case ddFloat:
        switch ( bits )
        {
        case 32:
            for ( i = 0; i < dim; ++ i )
            {
                rc = SDumperPrint ( b, "\v%f", ( double ) self -> u . f32 [ i ] );
                if ( rc != 0 )
                    return rc;
                SDumperSepString ( b, compact ? "," : ", " );
            }
            break;
        case 64:
            for ( i = 0; i < dim; ++ i )
            {
                rc = SDumperPrint ( b, "\v%f", self -> u . f64 [ i ] );
                if ( rc != 0 )
                    return rc;
                SDumperSepString ( b, compact ? "," : ", " );
            }
            break;
        }
        break;
    case ddAscii:
    {
        const char *end, *p;

        rc = SDumperWrite ( b, "'", 1 );

    do_ascii:
        for ( p = self -> u . ascii, end = p + dim; rc == 0 && p < end; )
        {
            const char *start = p;
            while ( p < end && isprint ( * p ) )
                ++ p;
            rc = SDumperWrite ( b, start, p - start );
            if ( p == end )
                break;

            switch ( * p )
            {
            case '\n':
                rc = SDumperWrite ( b, "\\n", 2 );
                break;
            case '\r':
                rc = SDumperWrite ( b, "\\", 2 );
                break;
            case '\t':
                rc = SDumperWrite ( b, "\\t", 2 );
                break;
            default:
                rc = SDumperPrint ( b, "\\x%x%x", ( * ( uint8_t* ) p ) / 16, * p & 15 );
            }
            ++ p;
        }
        if ( rc == 0 )
            rc = SDumperWrite ( b, "'", 1 );
        return rc;
    }
    case ddUnicode:
    {
        rc = SDumperWrite ( b, "'", 1 );
        if ( rc != 0 )
            return rc;

        if ( bits == 8 )
            goto do_ascii;

        for ( i = 0; i < dim; ++ i )
        {
            char buff [ 8 ];
            uint32_t ch = ( bits == 16 ) ?
                ( uint32_t )  self -> u . utf16 [ i ] :
                self -> u . utf32 [ i ];

            if ( ch >= 128 )
                sprintf ( buff, "\\u%04x", ch );
            else if ( isprint ( ( int ) ch ) )
                buff [ 0 ] = ( char ) ch, buff [ 1 ] = 0;
            else switch ( ch )
            {
            case '\n':
                sprintf ( buff, "\\n" );
                break;
            case '\r':
                sprintf ( buff, "\\r" );
                break;
            case '\t':
                sprintf ( buff, "\\t" );
                break;
            default:
                sprintf ( buff, "\\x%02x", ch );
            }

            rc = SDumperPrint ( b, buff );
            if ( rc != 0 )
                return rc;
        }
        if ( rc == 0 )
            rc = SDumperWrite ( b, "'", 1 );
        return rc;
    }}

    if ( rc == 0 && dim > 1 )
        rc = SDumperPrint ( b, compact ? "]" : " ]" );

    return rc;
}

#if SLVL >= 3
static
rc_t SFuncExprDump ( const SFuncExpr *self, SDumper *b )
{
    rc_t rc;
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    if ( VectorLength ( & self -> schem ) != 0 )
    {
        if ( compact )
            rc = SExpressionBracketListDump ( & self -> schem, b, "<", ">" );
        else
            rc = SExpressionBracketListDump ( & self -> schem, b, "< ", " > " );
        if ( rc != 0 )
            return rc;
    }

    rc = SFunctionDump ( self -> func, b );
    if ( rc != 0 )
        return rc;

    rc = SDumperVersion ( b, self -> version_requested ?
        self -> version : self -> func -> version );
    if ( rc != 0 )
        return rc;

    if ( VectorLength ( & self -> pfact ) != 0 )
    {
        if ( compact )
            rc = SExpressionBracketListDump ( & self -> pfact, b, "<", ">" );
        else
            rc = SExpressionBracketListDump ( & self -> pfact, b, " < ", " >" );
        if ( rc != 0 )
            return rc;
    }

    if ( VectorLength ( & self -> pfunc ) == 0 )
        rc = SDumperPrint ( b, compact ? "()" : " ()" );
    else if ( compact )
        rc = SExpressionBracketListDump ( & self -> pfunc, b, "(", ")" );
    else
        rc = SExpressionBracketListDump ( & self -> pfunc, b, " ( ", " )" );

    return rc;
}
#endif

#if SLVL >= 7
static
rc_t SPhysEncExprDump ( const SPhysEncExpr *self, SDumper *b )
{
    rc_t rc;
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    if ( VectorLength ( & self -> schem ) != 0 )
    {
        if ( compact )
            rc = SExpressionBracketListDump ( & self -> schem, b, "<", ">" );
        else
            rc = SExpressionBracketListDump ( & self -> schem, b, "< ", " > " );
        if ( rc != 0 )
            return rc;
    }

    rc = SPhysicalDump ( self -> phys, b );
    if ( rc != 0 )
        return rc;

    if ( self -> version_requested )
        rc = SDumperVersion ( b, self -> version );
    else
        rc = SDumperVersion ( b, self -> phys -> version );
    if ( rc != 0 )
        return rc;

    if ( VectorLength ( & self -> pfact ) != 0 )
    {
        if ( compact )
            rc = SExpressionBracketListDump ( & self -> pfact, b, "<", ">" );
        else
            rc = SExpressionBracketListDump ( & self -> pfact, b, " < ", " >" );
        if ( rc != 0 )
            return rc;
    }

    return rc;
}
#endif

rc_t SExpressionDump ( const SExpression *self, SDumper *b )
{
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    if ( self == NULL )
        return SDumperWrite ( b, "NULL", 4 );

    switch ( self -> var )
    {
    case eTypeExpr:
        return STypeExprDump ( ( const STypeExpr* ) self, b );
    case eConstExpr:
        return SConstExprDump ( ( const SConstExpr* ) self, b );
#if SLVL >= 3
    case eIndirectExpr:
    case eParamExpr:
    case eProdExpr:
    case eFwdExpr:
    case eFuncParamExpr:
    case eColExpr:
    case ePhysExpr:
    {
        const SSymExpr *x = ( const SSymExpr* ) self;
        if ( x -> alt )
            return SDumperPrint ( b, "@%N", x -> _sym );
        return KSymbolDump ( x -> _sym, b );
    }
    case eFuncExpr:
#if SLVL >= 4
    case eScriptExpr:
#endif
        return SFuncExprDump ( ( const SFuncExpr* ) self, b );
#endif
#if SLVL >= 7
    case ePhysEncExpr:
        return SPhysEncExprDump ( ( const SPhysEncExpr* ) self, b );
#endif
    case eNegateExpr:
    {
        const SUnaryExpr *x = ( const SUnaryExpr* ) self;
        return SDumperPrint ( b, "-%E", x -> expr );
    }
    case eCastExpr:
    {
        const SBinExpr *x = ( const SBinExpr* ) self;
        return SDumperPrint ( b, compact ? "(%E)%E" : "( %E ) %E", x -> left, x -> right );
    }
    case eCondExpr:
    {
        const SBinExpr *x = ( const SBinExpr* ) self;
        return SDumperPrint ( b, compact ? "%E|%E" : "%E | %E", x -> left, x -> right );
    }
    case eVectorExpr:
    {
        const SVectExpr *x = ( const SVectExpr* ) self;
        if ( compact )
            return SExpressionBracketListDump ( & x -> expr, b, "[", "]" );
        return SExpressionBracketListDump ( & x -> expr, b, "[ ", " ]" );
    }
    case eMembExpr:
        assert (false); //TODO: SMembExprDump
        break;
    }

    return SDumperPrint ( b, "EXPR-UNKNOWN" );

}

bool SExpressionVDump ( void *item, void *data )
{
    SDumper *b = data;
    const SExpression *self = ( const void* ) item;

    b -> rc = SExpressionDump ( self, b );

    return ( b -> rc != 0 ) ? true : false;
}
#endif /* SLVL >= 1 */


/*--------------------------------------------------------------------------
 * STypeExpr
 */

/* Mark
 */
void STypeExprMark ( const STypeExpr *self, const VSchema *schema )
{
    if ( self -> fmt != NULL )
        SFormatMark ( self -> fmt );
    if ( self -> dt != NULL )
        SDatatypeMark ( self -> dt );
    if ( self -> ts != NULL )
        STypesetMark ( self -> ts, schema );
    if ( self -> dim )
        SExpressionMark ( ( void * ) self -> dim, ( void * ) schema );
}


/*--------------------------------------------------------------------------
 * SSymExpr
 */

/* Make
 *  used when creating implicit expressions
 */
rc_t SSymExprMake ( const SExpression **xp, const KSymbol *sym, uint32_t var )
{
    SSymExpr *x = malloc ( sizeof * x );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    x -> dad . var = var;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> _sym = sym;
    x -> alt = false;

    * xp = & x -> dad;
    return 0;
}

/* Mark
 */
void SSymExprMark ( const SSymExpr *self, const VSchema *schema )
{
    switch ( self -> _sym -> type )
    {
    case eConstant:
        SConstantMark ( self -> _sym -> u . obj );
        break;
    }
}


/*--------------------------------------------------------------------------
 * SFuncExpr
 */

/* Mark
 */
void SFuncExprMark ( const SFuncExpr *self, const VSchema *schema )
{
    SFunctionMark ( ( void * )self -> func, ( void * )schema );
}


/*--------------------------------------------------------------------------
 * SPhysEncExpr
 */

/* ImplicitPhysEncExpr
 *  create expression object
 */
rc_t VSchemaImplicitPhysEncExpr ( VSchema *self,
    VTypedecl *td, const SExpression **expr, const char *text, const char *ctx )
{
    KSymTable tbl;
    rc_t rc = init_symtab ( & tbl, self );
    if ( rc == 0 )
    {
        KToken t;
        KTokenText tt;
        KTokenSource src;

        SchemaEnv env;
        SchemaEnvInit ( & env, EXT_SCHEMA_LANG_VERSION );

        KTokenTextInitCString ( & tt, text, ctx );
        KTokenSourceInit ( & src, & tt );
        next_token ( & tbl, & src, & t );

        rc = phys_encoding_expr ( & tbl, & src, & t, & env, self, td, expr );

        KSymTableWhack ( & tbl );
    }
    return rc;
}

/* Mark
 */
void SPhysEncExprMark ( const SPhysEncExpr *self, const VSchema *schema )
{
    SPhysicalMark ( ( void * )self -> phys, ( void * )schema );
}


/*--------------------------------------------------------------------------
 * VSchema
 */

#if SLVL >= 1
static
rc_t const_vect_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SVectExpr **xp )
{
    rc_t rc;
    SVectExpr *x = malloc ( sizeof * x );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    x -> dad . var = eVectorExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    VectorInit ( & x -> expr, 0, 16 );

    do
    {
        SExpression *vx;
        rc = const_expr ( tbl, src, next_token ( tbl, src, t ),
            env, self, ( const SExpression** ) & vx );
        if ( rc != 0 )
            break;

        if ( vx -> var == eVectorExpr )
        {
            SExpressionWhack ( vx );
            rc = KTokenExpected ( t, klogErr, "scalar constant" );
            break;
        }

        rc = VectorAppend ( & x -> expr, NULL, vx );
        if ( rc != 0 )
        {
            SExpressionWhack ( vx );
            break;
        }
    }
    while ( t -> id == eComma );

    if ( rc != 0 )
        SExpressionWhack ( & x -> dad );
    else
    {
        * xp = x;
        rc = expect ( tbl, src, t, eRightSquare, "]", true );
    }

    return rc;
}
#endif

/*
 * const-expr         = <constname>
 *                    | CONST-VALUE
 */
#if SLVL >= 1
static
rc_t negate_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SExpression **xp )
{
    rc_t rc = const_expr ( tbl, src, next_token ( tbl, src, t ), env, self, xp );
    if ( rc == 0 )
    {
        SUnaryExpr *x;
        SConstExpr *cx;
        const SSymExpr *sx;
        const SExpression *td;
        const SDatatype *dt;

        static atomic32_t s_I8_id;
        static atomic32_t s_I16_id;
        static atomic32_t s_I32_id;
        static atomic32_t s_I64_id;

        switch ( ( * xp ) -> var )
        {
        case eConstExpr:
            cx = * ( SConstExpr** ) xp;
            if ( cx -> td . dim < 2 )
            {
                dt = VSchemaFindTypeid ( self, cx -> td . type_id );
                if ( dt != NULL ) switch ( dt -> domain )
                {
                case vtdUint:
                    switch ( dt -> size )
                    {
                    case 8:
                        cx -> td . type_id  = VSchemaCacheIntrinsicTypeId ( self, & s_I8_id, "I8" );
                        break;
                    case 16:
                        cx -> td . type_id  = VSchemaCacheIntrinsicTypeId ( self, & s_I16_id, "I16" );
                        break;
                    case 32:
                        cx -> td . type_id  = VSchemaCacheIntrinsicTypeId ( self, & s_I32_id, "I32" );
                        break;
                    case 64:
                        cx -> td . type_id  = VSchemaCacheIntrinsicTypeId ( self, & s_I64_id, "I64" );
                        break;
                    }
                    /* no break */
                case vtdInt:
                    switch ( dt -> size )
                    {
                    case 8:
                        cx -> u . i8 [ 0 ] = - cx -> u . i8 [ 0 ];
                        return 0;
                    case 16:
                        cx -> u . i16 [ 0 ] = - cx -> u . i16 [ 0 ];
                        return 0;
                    case 32:
                        cx -> u . i32 [ 0 ] = - cx -> u . i32 [ 0 ];
                        return 0;
                    case 64:
                        cx -> u . i64 [ 0 ] = - cx -> u . i64 [ 0 ];
                        return 0;
                    }
                    break;
                case vtdFloat:
                    switch ( dt -> size )
                    {
                    case 32:
                        cx -> u . f32 [ 0 ] = - cx -> u . f32 [ 0 ];
                        return 0;
                    case 64:
                        cx -> u . f64 [ 0 ] = - cx -> u . f64 [ 0 ];
                        return 0;
                    }
                    break;
                }
            }
            break;

        case eIndirectExpr:
            /* if type is known, at least verify domain */
            sx = * ( const SSymExpr** ) xp;
            td = ( ( const SIndirectConst* ) sx -> _sym -> u . obj ) -> td;
            if ( td != NULL )
            {
                const STypeExpr *tx = ( const STypeExpr* ) td;
                if ( tx-> dad . var == eTypeExpr && tx -> resolved )
                {
                    /* cannot have formats, but this is verified elsewhere */
                    if ( tx -> fd . fmt == 0 && tx -> fd . td . dim < 2 )
                    {
                        /* determine domain */
                        dt = VSchemaFindTypeid ( self, tx -> fd . td . type_id );
                        if ( dt != NULL ) switch ( dt -> domain )
                        {
                        case vtdUint:
                            KTokenExpected ( t, klogWarn, "signed integer" );
                        case vtdInt:
                        case vtdFloat:
                            goto introduce_negate_expr;
                        }
                    }
                }

                break;
            }

        introduce_negate_expr:
            x = malloc ( sizeof * x );
            if ( x == NULL )
            {
                SExpressionWhack ( & sx -> dad );
                return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
            }

            x -> dad . var = eNegateExpr;
            atomic32_set ( & x -> dad . refcount, 1 );
            x -> expr = & sx -> dad;
            * xp = & x -> dad;
            return 0;

        case eNegateExpr:
            /* double negate */
            x = * ( SUnaryExpr** ) xp;
            * xp = x -> expr;
            free ( x );
            return 0;

        default:
            SExpressionWhack ( * xp );
            return KTokenExpected ( t, klogErr, "integer or floating point constant" );
        }

        rc = RC ( rcVDB, rcSchema, rcParsing, rcType, rcIncorrect );
        KTokenRCExplain ( t, klogErr, rc );
    }

    return rc;
}

static
rc_t bool_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SConstExpr **xp )
{
    static atomic32_t s_bool_id;

    SConstExpr *x = malloc ( sizeof * x - sizeof x -> u + sizeof x -> u . b [ 0 ] );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    assert ( t -> id == kw_true || t -> id == kw_false );
    x -> u . b [ 0 ] = ( t -> id == kw_true );

    x -> dad . var = eConstExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> td . type_id = VSchemaCacheIntrinsicTypeId ( self, & s_bool_id, "bool" );
    x -> td . dim = 1;

    * xp = x;

    next_token ( tbl, src, t );
    return 0;
}

static
rc_t uint_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SConstExpr **xp )
{
    rc_t rc;
    static atomic32_t s_U64_id;

    SConstExpr *x = malloc ( sizeof * x - sizeof x -> u + sizeof x -> u . u64 [ 0 ] );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    rc = KTokenToU64 ( t, & x -> u . u64 [ 0 ] );
    if ( rc != 0 )
    {
        free ( x );
        return rc;
    }

    x -> dad . var = eConstExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> td . type_id = VSchemaCacheIntrinsicTypeId ( self, & s_U64_id, "U64" );
    x -> td . dim = 1;

    * xp = x;

    next_token ( tbl, src, t );
    return 0;
}

static
rc_t float_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SConstExpr **xp )
{
    rc_t rc;
    static atomic32_t s_F64_id;

    SConstExpr *x = malloc ( sizeof * x - sizeof x -> u + sizeof x -> u . f64 [ 0 ] );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    rc = KTokenToF64 ( t, & x -> u . f64 [ 0 ] );
    if ( rc != 0 )
    {
        free ( x );
        return rc;
    }

    x -> dad . var = eConstExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> td . type_id = VSchemaCacheIntrinsicTypeId ( self, & s_F64_id, "F64" );
    x -> td . dim = 1;

    * xp = x;

    next_token ( tbl, src, t );
    return 0;
}

static
rc_t string_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SConstExpr **xp )
{
    rc_t rc;

    size_t size = t -> str . size + 1;
    SConstExpr *x = malloc ( sizeof * x - sizeof x -> u + size );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    rc = KTokenToString ( t, x -> u . utf8, size, & size );
    if ( rc != 0 )
    {
        free ( x );
        return rc;
    }

    x -> dad . var = eConstExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> td . dim = ( uint32_t ) size;

    if ( t -> id == eString || ( size_t ) string_len ( x -> u . utf8, size ) == size )
    {
        static atomic32_t s_ascii_id;
        x -> td . type_id = VSchemaCacheIntrinsicTypeId ( self, & s_ascii_id, "ascii" );
    }
    else
    {
        static atomic32_t s_utf8_id;
        x -> td . type_id = VSchemaCacheIntrinsicTypeId ( self, & s_utf8_id, "utf8" );
    }

    * xp = x;

    next_token ( tbl, src, t );
    return 0;
}

static
rc_t sym_const_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SConstExpr **xp )
{
    const KSymbol *sym = t -> sym;
    const SConstant *cnst = sym -> u . obj;

    assert ( cnst -> expr != NULL );
    atomic32_inc ( & ( ( SExpression* ) cnst -> expr ) -> refcount );
    * xp = ( SConstExpr* ) cnst -> expr;

    next_token ( tbl, src, t );
    return 0;
}
#endif /* SLVL >= 1 */


#if SLVL >= 3
LIB_EXPORT rc_t CC indirect_const_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SExpression **xp )
{
    SSymExpr *x = malloc ( sizeof *x );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    x -> dad . var = eIndirectExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> _sym = t -> sym;
    x -> alt = false;
    * xp = & x -> dad;

    next_token ( tbl, src, t );
    return 0;
}
#endif /* SLVL >= 1 */

/*
 * type-expr          = <typeset>
 *                    | <fmtdecl>
 *                    | <fmtname> '/' <typeset>
 */
#if SLVL >= 1
static
rc_t type_expr_impl ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SExpression **fd, bool vardim )
{
    STypeExpr *x;

    rc_t rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;

    x = malloc ( sizeof *x );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    x -> dad . var = eTypeExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> fmt = NULL;
    x -> dt = NULL;
    x -> ts = NULL;
    x -> id = NULL;
    x -> dim = NULL;
    x -> fd . fmt = 0;
    x -> fd . td . type_id = 0;
    x -> fd . td . dim = 0;
    x -> resolved = true;

    if ( t -> id == eFormat )
    {
        x -> fmt = t -> sym -> u . obj;
        x -> fd . fmt = x -> fmt -> id;
        if ( next_token ( tbl, src, t ) -> id != eFwdSlash )
        {
            * fd = & x -> dad;
            return 0;
        }

        rc = next_fqn ( tbl, src, next_token ( tbl, src, t ), env );
        if ( rc != 0 )
        {
            free ( x );
            return rc;
        }
    }

    switch ( t -> id )
    {
    case eDatatype:
        x -> dt = t -> sym -> u . obj;
        x -> fd . td . type_id = x -> dt -> id;
        break;
    case eTypeset:
        x -> ts = t -> sym -> u . obj;
        x -> fd . td . type_id = x -> ts -> id;
        break;
#if SLVL >= 3
    case eSchemaType:
        x -> id = t -> sym -> u . obj;
        x -> resolved = false;
        break;
#endif
    default:
        free ( x );
        return KTokenExpected ( t, klogErr, "datatype or typeset" );
    }

    if ( next_token ( tbl, src, t ) -> id != eLeftSquare )
        x -> fd . td . dim = 1;
    else
    {
        if ( next_token ( tbl, src, t ) -> id == eAsterisk && vardim )
            next_token ( tbl, src, t );
        else
        {
            const SConstExpr *cx;

            rc = const_expr ( tbl, src, t, env, self, & x -> dim );
            if ( rc != 0 )
            {
                free ( x );
                return rc;
            }

            switch ( x -> dim -> var )
            {
            case eConstExpr:
                cx =  ( const SConstExpr* ) x -> dim;
                if ( cx -> td . dim == 1 )
                {
                    const SDatatype *dt = VSchemaFindTypeid ( self, cx -> td . type_id );
                    if ( dt != NULL && SDatatypeIntrinsicDim ( dt ) == 1 )
                    {
                        if ( dt -> domain == vtdUint )
                        {
                            switch ( dt -> size )
                            {
                            case 64:
                                if ( cx -> u . u64 [ 0 ] <= 0xFFFFFFFF )
                                    x -> fd . td . dim = ( uint32_t ) cx -> u . u64 [ 0 ];
                                break;
                            case 32:
                                x -> fd . td . dim = cx -> u . u32 [ 0 ];
                                break;
                            case 16:
                                x -> fd . td . dim = cx -> u . u16 [ 0 ];
                                break;
                            case 8:
                                x -> fd . td . dim = cx -> u . u8 [ 0 ];
                                break;
                            }
                        }
                        else if ( dt -> domain ==  vtdInt )
                        {
                            switch ( dt -> size )
                            {
                            case 64:
                                if ( cx -> u . i64 [ 0 ] > 0 && cx -> u . i64 [ 0 ] <= 0xFFFFFFFF )
                                    x -> fd . td . dim = ( uint32_t ) cx -> u . i64 [ 0 ];
                                break;
                            case 32:
                                if ( cx -> u . i32 [ 0 ] > 0 )
                                    x -> fd . td . dim = cx -> u . i32 [ 0 ];
                                break;
                            case 16:
                                if ( cx -> u . i16 [ 0 ] > 0 )
                                    x -> fd . td . dim = cx -> u . i16 [ 0 ];
                                break;
                            case 8:
                                if ( cx -> u . i8 [ 0 ] > 0 )
                                    x -> fd . td . dim = cx -> u . i8 [ 0 ];
                                break;
                            }
                        }

                        if ( x -> fd . td . dim > 0 )
                            break;
                    }
                }

                SExpressionWhack ( & x -> dad );
                return KTokenExpected ( t, klogErr, "constant integer expression" );

            case eIndirectExpr:
                x -> resolved = false;
                break;
            default:
                SExpressionWhack ( & x -> dad );
                return KTokenExpected ( t, klogErr, "constant integer expression" );
            }
        }

        rc = expect ( tbl, src, t, eRightSquare, "]", true );
        if ( rc != 0 )
        {
            SExpressionWhack ( & x -> dad );
            return rc;
        }
    }

    * fd = & x -> dad;
    return 0;
}

rc_t type_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SExpression **fd )
{
    return type_expr_impl ( tbl, src, t, env, self, fd, false );
}

rc_t vardim_type_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SExpression **fd )
{
    return type_expr_impl ( tbl, src, t, env, self, fd, true );
}
#endif /* SLVL >= 1 */


/*
 * cast_expr          = '(' <type-expr> ')' <expression>
 */
#if SLVL >= 1
static
rc_t cast_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SBinExpr **xp,
    rc_t ( * expr ) ( KSymTable*, KTokenSource*, KToken*,
        const SchemaEnv*, VSchema*, const SExpression** ) )
{
    rc_t rc;
    SBinExpr *x = malloc ( sizeof * x );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    rc = type_expr ( tbl, src, next_token ( tbl, src, t ), env, self, & x -> left );
    if ( rc == 0 )
    {
        rc = expect ( tbl, src, t, eRightParen, ")", true );
        if ( rc == 0 )
        {
            rc = ( * expr ) ( tbl, src, t, env, self, & x -> right );
            if ( rc == 0 )
            {
                x -> dad . var = eCastExpr;
                atomic32_set ( & x -> dad . refcount, 1 );
                * xp = x;
                return 0;
            }
        }

        SExpressionWhack ( x -> left );
    }

    free ( x );
    return rc;
}


rc_t const_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, const SExpression **xp )
{
    switch ( t -> id )
    {
    case eDecimal:
    case eHex:
    case eOctal:
        return uint_expr ( tbl, src, t, env, self, ( const SConstExpr** ) xp );
    case eFloat:
    case eExpFloat:
        return float_expr ( tbl, src, t, env, self, ( const SConstExpr** ) xp );
    case eString:
    case eEscapedString:
        return string_expr ( tbl, src, t, env, self, ( const SConstExpr** ) xp );
    case eConstant:
        return sym_const_expr ( tbl, src, t, env, self, ( const SConstExpr** ) xp );
#if SLVL >= 4
    case eFactParam:
#endif
#if SLVL >= 3
    case eSchemaParam:
        return indirect_const_expr ( tbl, src, t, env, self, xp );
#endif
    case eLeftSquare:
        return const_vect_expr ( tbl, src, t, env, self, ( const SVectExpr** ) xp );
    case kw_false:
    case kw_true:
        return bool_expr ( tbl, src, t, env, self, ( const SConstExpr** ) xp );
    case eLeftParen:
        return cast_expr ( ( KSymTable* ) tbl, src, t, env,
            ( VSchema* ) self, ( const SBinExpr** ) xp,
            ( rc_t ( * ) ( KSymTable*, KTokenSource*, KToken*,
              const SchemaEnv*, VSchema*, const SExpression** ) ) const_expr );
    case eMinus:
        /* unary minus sign */
        return negate_expr ( tbl, src, t, env, self, xp );
    case ePlus:
        /* unary plus sign */
        return const_expr ( tbl, src, next_token ( tbl, src, t ), env, self, xp );
    }

    return KTokenExpected ( t, klogErr, "boolean, integer, float, string or symbolic constant" );
}
#endif /* SLVL >= 1 */


/*
 * func-param-expr    = <funcname>
 */
#if SLVL >= 4
static
rc_t sym_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SSymExpr **xp, uint32_t var, bool alt )
{
    SSymExpr *x = malloc ( sizeof * x );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    x -> dad . var = var;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> _sym = t -> sym;
    x -> alt = alt;
    * xp = x;

    next_token ( tbl, src, t );
    return 0;
}

#define func_param_expr( tbl, src, t, env, self, xp ) \
    sym_expr ( tbl, src, t, env, self, xp, eFuncParamExpr, false )

#endif


/*
 * param-expr         = <func-param>
 *
 */
#if SLVL >= 4

#define param_expr( tbl, src, t, env, self, xp ) \
    sym_expr ( tbl, src, t, env, self, xp, eParamExpr, false )

#endif


/*
 * prod-expr          = <production>
 *
 */
#if SLVL >= 4

#define prod_expr( tbl, src, t, env, self, xp ) \
    sym_expr ( tbl, src, t, env, self, xp, eProdExpr, false )

#endif


/*
 * fwd-expr           = ID
 */
#if SLVL >= 4
static
rc_t fwd_decl_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SSymExpr **xp, bool alt )
{
    rc_t rc;
    SSymExpr *x = malloc ( sizeof * x );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    rc = KSymTableCreateConstSymbol ( tbl, & x -> _sym, & t -> str, eForward, NULL );
    if ( rc != 0 )
    {
        free ( x );
        return rc;
    }

    x -> dad . var = eFwdExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> alt = alt;
    * xp = x;

    next_token ( tbl, src, t );
    return 0;
}

#define fwd_ref_expr( tbl, src, t, env, self, xp, alt ) \
    sym_expr ( tbl, src, t, env, self, xp, eFwdExpr, alt )

#endif


/*
 * column-expr        = <column>
 */
#if SLVL >= 8

#define column_expr( tbl, src, t, env, self, xp, alt ) \
    sym_expr ( tbl, src, t, env, self, xp, eColExpr, alt )

#endif


/*
 * physical-expr      = KCOL
 */
#if SLVL >= 7

#define physical_expr( tbl, src, t, env, self, xp ) \
    sym_expr ( tbl, src, t, env, self, xp, ePhysExpr, false )

#endif


/*
 * schema-parms       = <schema-param> [ ',' <schema-parms> ]
 * schema-param       = [ ID '=' ]  <schema-value>
 * schema-value       = <fmtdecl>
 *                    | <uint-expr>
 */
#if SLVL >= 4
static
rc_t schema_value ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SExpression **v )
{
    /* could be a typename */
    if ( t -> id == eNamespace )
    {
        rc_t rc = next_fqn ( tbl, src, t, env );
        if ( rc != 0 )
            return rc;
    }

    /* param value */
    switch ( t -> id )
    {
    /* type parameter */
    case eFormat:
    case eDatatype:
    case eTypeset:
    case eSchemaType:
        return type_expr ( tbl, src, t, env, self, v );

    /* manifest uint constant */
    case eDecimal:
    case eHex:
    case eOctal:
        return uint_expr ( tbl, src, t, env, self, ( const SConstExpr** ) v );

    /* symbolic constant must be uint */
    case eConstant:
        return sym_const_expr ( tbl, src, t, env, self, ( const SConstExpr** ) v );

    /* schema or factory constant must be uint
       but may not yet be completely resolved */
    case eSchemaParam:
    case eFactParam:
        return indirect_const_expr ( tbl, src, t, env, self, v );
    }

    return KTokenExpected ( t, klogErr, "data type or unsigned int constant" );
}
#endif

/*
 * fact-parms         = <fact-param> [ ',' <fact-parms> ]
 * fact-param         = [ ID '=' ] <fact-value>         ( "ID =" not implemented )
 * fact-value         = <func-name>
 *                    | '[' <fact-const-list> ']'
 */
#if SLVL >= 4
static
rc_t fact_value ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SExpression **v )
{
    if ( t -> id == eNamespace )
    {
        rc_t rc = next_fqn ( tbl, src, t, env );
        if ( rc != 0 )
            return rc;
    }

    switch ( t -> id )
    {
        /* const-expression */
    case eDecimal:
    case eHex:
    case eOctal:
        return uint_expr ( tbl, src, t, env, self, ( const SConstExpr** ) v );
    case eFloat:
    case eExpFloat:
        return float_expr ( tbl, src, t, env, self, ( const SConstExpr** ) v );
    case eString:
    case eEscapedString:
        return string_expr ( tbl, src, t, env, self, ( const SConstExpr** ) v );
    case eConstant:
        return sym_const_expr ( tbl, src, t, env, self, ( const SConstExpr** ) v );
    case eSchemaParam:
    case eFactParam:
        return indirect_const_expr ( tbl, src, t, env, self, v );
    case eLeftSquare:
        return const_vect_expr ( tbl, src, t, env, self, ( const SVectExpr** ) v );
    case kw_false:
    case kw_true:
        return bool_expr ( tbl, src, t, env, self, ( const SConstExpr** ) v );

        /* cast expression */
    case eLeftParen:
        return cast_expr ( tbl, src, t, env, self, ( const SBinExpr** ) v, fact_value );

        /* function name */
    case eFunction:
        return func_param_expr ( tbl, src, t, env, self, ( const SSymExpr** ) v );

    case eMinus:
        /* unary minus sign */
        return negate_expr ( tbl, src, t, env, self, v );
    case ePlus:
        /* unary plus sign */
        return const_expr ( tbl, src, next_token ( tbl, src, t ), env, self, v );
    }

    return KTokenExpected ( t, klogErr, "constant, function param or cast operator" );
}

static
rc_t old_fact_value ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SExpression **v )
{
    switch ( t -> id )
    {
        /* const-expression */
    case eDecimal:
    case eHex:
    case eOctal:
        return uint_expr ( tbl, src, t, env, self, ( const SConstExpr** ) v );
    case eFloat:
    case eExpFloat:
        return float_expr ( tbl, src, t, env, self, ( const SConstExpr** ) v );
    case eString:
    case eEscapedString:
        return string_expr ( tbl, src, t, env, self, ( const SConstExpr** ) v );
    case eMinus:
        return negate_expr ( tbl, src, t, env, self, v );
    case ePlus:
        return old_fact_value ( tbl, src, next_token ( tbl, src, t ), env, self, v );
    }

    return RC ( rcVDB, rcSchema, rcParsing, rcNoObj, rcNoErr );
}
#endif

/*
 *    func-parms         = <func-param> [ ',' <func-parms> ]
 *    func-param         = <colname>
 *                       | <physname>
 *                       | <production>
 *                       | <parmname>
 */
#if SLVL >= 4
static
rc_t param_value ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SExpression **v )
{
    KToken t2;
    bool alt = false;

    /* look for special modifiers */
    switch ( t -> id )
    {
#if SLVL >= 7
    case ePeriod:
        /* detect special syntax for physical column names */
        physical_name ( tbl, src, t, env );
        break;
#endif
    case eAtSign:

        /* detect alternate column source */
        switch ( next_token ( tbl, src, & t2 ) -> id )
        {
#if SLVL >= 8
        case eColumn:
#endif
        case eIdent:
        case eForward:
        case eVirtual:
            * t = t2;
            alt = true;
            break;

        default:
            KTokenSourceReturn ( src, & t2 );
        }
        break;
    }

    switch ( t -> id )
    {
    /* handle function or script params, named productions */
#if SLVL >= 5
    case eAtSign:
        t -> sym = KSymTableFind ( tbl, & t -> str );
        if ( t -> sym == NULL )
            break;
        t -> id = t -> sym -> type;
        /* no break */
#endif
    case eFuncParam:
        return param_expr ( tbl, src, t, env, self, ( const SSymExpr** ) v );
    case eProduction:
        return prod_expr ( tbl, src, t, env, self, ( const SSymExpr** ) v );

    /* forward reference */
    case eIdent:
        return fwd_decl_expr ( tbl, src, t, env, self, ( const SSymExpr** ) v, alt );
    case eForward:
    case eVirtual:
        return fwd_ref_expr ( tbl, src, t, env, self, ( const SSymExpr** ) v, alt );
#if SLVL >= 8
    /* column expression */
    case eColumn:
        return column_expr ( tbl, src, t, env, self, ( const SSymExpr** ) v, alt );
#endif
#if SLVL >= 7
    /* physical column expression */
    case ePhysMember:
        return physical_expr ( tbl, src, t, env, self, ( const SSymExpr** ) v );
#endif
    /* cast expression */
    case eLeftParen:
        return cast_expr ( tbl, src, t, env, self, ( const SBinExpr** ) v, param_value );
    }

    return KTokenExpected ( t, klogErr, "column or production name or cast operator" );
}

static
rc_t param_list ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, Vector *v,
    rc_t ( * value ) ( KSymTable*, KTokenSource*, KToken*,
        const SchemaEnv*, VSchema*, const SExpression** ) )
{
    rc_t rc;
    const SExpression *x;

    while ( 1 )
    {
        /* param value */
        rc = ( * value ) ( tbl, src, t, env, self, & x );
        if ( rc != 0 )
            return rc;

        /* record parameter */
        rc = VectorAppend ( v, NULL, x );
        if ( rc != 0 )
        {
            SExpressionWhack ( x );
            return KTokenRCExplain ( t, klogInt, rc );
        }

        if ( t -> id != eComma )
            break;

        next_token ( tbl, src, t );
    }

    return 0;
}

static
rc_t func_expression ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFuncExpr *fx, const KSymbol **sym )
{
    rc_t rc;

    /* schema-parms */
    if ( t -> id == eLeftAngle )
    {
        next_token ( tbl, src, t );
        rc = param_list ( tbl, src, t, env, self, & fx -> schem, schema_value );
        if ( rc == 0 )
            rc = expect ( tbl, src, t, eRightAngle, ">", true );
        if ( rc != 0 )
            return rc;
    }

    /* locate function or schema */
    rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;

    if ( t -> id != eScriptFunc && t -> id != eFunction )
        return KTokenExpected ( t, klogErr, "function" );
    * sym = t -> sym;

    /* look for requested version */
    if ( next_token ( tbl, src, t ) -> id == eHash )
    {
        fx -> version_requested = true;
        rc = maj_min_rel ( tbl, src, next_token ( tbl, src, t ),
            env, self,  & fx -> version, false );
        if ( rc != 0 )
            return rc;
    }

    /* look for factory params */
    if ( t -> id == eLeftAngle && ! env -> mixed_fact_param_list )
    {
        next_token ( tbl, src, t );
        rc = param_list ( tbl, src, t, env, self, & fx -> pfact, fact_value );
        if ( rc == 0 )
            rc = expect ( tbl, src, t, eRightAngle, ">", true );
        if ( rc != 0 )
            return rc;
    }

    /* function params */
    if ( t -> id != eLeftParen )
        return KTokenExpected ( t, klogErr, "(" );
    if ( next_token ( tbl, src, t ) -> id != eRightParen )
    {
        /* handle old-style fact params */
        if ( env -> mixed_fact_param_list )
        {
            rc = param_list ( tbl, src, t, env, self, & fx -> pfact, old_fact_value );
            if ( GetRCState ( rc ) == rcNoErr )
                rc = 0;
        }

        /* handle normal function params */
        if ( rc == 0 )
            rc = param_list ( tbl, src, t, env, self, & fx -> pfunc, param_value );
    }
    if ( rc == 0 )
        rc = expect ( tbl, src, t, eRightParen, ")", true );

    return rc;
}

static
rc_t func_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SFuncExpr **xp )
{
    rc_t rc;
    const KSymbol *sym = NULL;

    SFuncExpr *fx = malloc ( sizeof * fx );
    if ( fx == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

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

    /* parse expression */
    rc = func_expression ( tbl, src, t, env, self, fx, & sym );
    if ( rc == 0 )
    {
        const SNameOverload *vf = sym -> u . obj;

        /* match expression */
        if ( vf -> name -> type == eScriptFunc )
            fx -> dad . var = eScriptExpr;

        if ( fx -> version_requested )
        {
            fx -> func =
                VectorFind ( & vf -> items, & fx -> version, NULL, SFunctionCmp );
        }
        else
        {
            fx -> func = VectorLast ( & vf -> items );
        }

        * xp = fx;
        return 0;
    }

    SExpressionWhack ( & fx -> dad );
    return rc;
}
#endif /* SLVL >= 4 */

/*
 * expression         = <param-value>
 *                    | <func-expr>
 *                    | '(' <type-expr> ')' <expression>
 */
#if SLVL >= 4
rc_t expression ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SExpression **xp )
{
    rc_t rc;

    if ( t -> id == eNamespace )
    {
        rc = next_fqn ( tbl, src, t, env );
        if ( rc != 0 )
            return rc;
    }

#if SLVL >= 7
    /* detect special syntax for physical column names */
    if ( t -> id == ePeriod )
        physical_name ( tbl, src, t, env );
#endif

    switch ( t -> id )
    {
    /* handle function or script params, named productions */
#if SLVL >= 5
    case eAtSign:
        t -> sym = KSymTableFind ( tbl, & t -> str );
        if ( t -> sym == NULL )
            break;
        t -> id = t -> sym -> type;
        /* no break */
#endif
    case eFuncParam:
        return param_expr ( tbl, src, t, env, self, ( const SSymExpr** ) xp );
    case eProduction:
        return prod_expr ( tbl, src, t, env, self, ( const SSymExpr** ) xp );

    /* forward reference */
    case eIdent:
        return fwd_decl_expr ( tbl, src, t, env, self, ( const SSymExpr** ) xp, false );
    case eForward:
    case eVirtual:
        return fwd_ref_expr ( tbl, src, t, env, self, ( const SSymExpr** ) xp, false );
#if SLVL >= 8
    /* column expression */
    case eColumn:
        return column_expr ( tbl, src, t, env, self, ( const SSymExpr** ) xp, false );
#endif
#if SLVL >= 7
    /* physical column expression */
    case ePhysMember:
        return physical_expr ( tbl, src, t, env, self, ( const SSymExpr** ) xp );
#endif
    /* function expression */
    case eLeftAngle:
    case eFunction:
    case eScriptFunc:
        return func_expr ( tbl, src, t, env, self, ( const SFuncExpr** ) xp );

    /* cast expression */
    case eLeftParen:
        return cast_expr ( tbl, src, t, env, self,
            ( const SBinExpr** ) xp, expression );

    /* type expression */
    case eFormat:
    case eDatatype:
    case eTypeset:
    case eSchemaType:
        return type_expr ( tbl, src, t, env, self, xp );

    /* in case expression gets used for const-expression */
    case eDecimal:
    case eHex:
    case eOctal:
        return uint_expr ( tbl, src, t, env, self, ( const SConstExpr** ) xp );
    case eFloat:
    case eExpFloat:
        return float_expr ( tbl, src, t, env, self, ( const SConstExpr** ) xp );
    case eString:
    case eEscapedString:
        return string_expr ( tbl, src, t, env, self, ( const SConstExpr** ) xp );
    case eConstant:
        return sym_const_expr ( tbl, src, t, env, self, ( const SConstExpr** ) xp );
    case eSchemaParam:
    case eFactParam:
        return indirect_const_expr ( tbl, src, t, env, self, xp );
    case eLeftSquare:
        return const_vect_expr ( tbl, src, t, env, self, ( const SVectExpr** ) xp );
    case kw_false:
    case kw_true:
        return bool_expr ( tbl, src, t, env, self, ( const SConstExpr** ) xp );

    case eMinus:
        /* unary minus sign */
        return negate_expr ( tbl, src, t, env, self, xp );
    case ePlus:
        /* unary plus sign */
        return const_expr ( tbl, src, next_token ( tbl, src, t ), env, self, xp );
    }

    return KTokenExpected ( t, klogErr, "production name, column name, function or cast expression" );
}
#endif /* SLVL >= 4 */


/*
 * cond-expr          = <expression> [ '|' <cond-expr> ]
 */
#if SLVL >= 4
rc_t cond_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SExpression **xp )
{
    rc_t rc = expression ( tbl, src, t, env, self, xp );
    if ( rc != 0 )
        return rc;

    if ( t -> id == ePipe )
    {
        SBinExpr *x = malloc ( sizeof * x );
        if ( x == NULL )
        {
            SExpressionWhack ( * xp ), * xp = NULL;
            return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        }

        x -> dad . var = eCondExpr;
        atomic32_set ( & x -> dad . refcount, 1 );
        x -> left = * xp;
        x -> right = NULL;
        * xp = & x -> dad;

        rc = cond_expr ( tbl, src, next_token ( tbl, src, t ), env, self, & x -> right );
        if ( rc != 0 )
        {
            SExpressionWhack ( & x -> dad );
            * xp = NULL;
        }
    }

    return rc;
}
#endif /* SLVL >= 4 */

/*
 * untyped-expr       = <fmtdecl> | <untyped-func-name> '(' ')'
 */
#if SLVL >= 6
static
rc_t untyped_func_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SFuncExpr **xp )
{
    rc_t rc;
    const KSymbol *sym = t -> sym;

    /* expect empty parameter list */
    next_token ( tbl, src, t );
    rc = expect ( tbl, src, t, eLeftParen, "(", true );
    if ( rc == 0 )
        rc = expect ( tbl, src, t, eRightParen, ")", true );
    if ( rc == 0 )
    {
        const SNameOverload *vf = sym -> u . obj;

        SFuncExpr *fx = malloc ( sizeof * fx );
        if ( fx == NULL )
            return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

        /* initialize */
        fx -> dad . var = eFuncExpr;
        atomic32_set ( & fx -> dad . refcount, 1 );
        VectorInit ( & fx -> schem, 0, 4 );
        VectorInit ( & fx -> pfact, 0, 8 );
        VectorInit ( & fx -> pfunc, 0, 8 );
        fx -> version = 0;
        fx -> version_requested = false;
        fx -> untyped = true;

        fx -> func = VectorLast ( & vf -> items );

        * xp = fx;
        return 0;
    }

    return rc;
}

LIB_EXPORT rc_t CC untyped_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, const SExpression **xp )
{
    rc_t rc;

    if ( t -> id == eNamespace )
    {
        rc = next_fqn ( tbl, src, t, env );
        if ( rc != 0 )
            return rc;
    }

    switch ( t -> id )
    {
    /* function expression */
    case eUntypedFunc:
        return untyped_func_expr ( tbl, src, t, env, self, ( const SFuncExpr** ) xp );

    /* type expression */
    case eFormat:
        return type_expr ( tbl, src, t, env, self, xp );
    }

    return KTokenExpected ( t, klogErr, "type or function expression" );
}
#endif /* SLVL >= 6 */


/*
 * phys-encoding-expr = [ '<' <schema-parms> '>' ]
 *                      <phys-encoding-name> [ '#' <maj-min-rel> ]
 *                      [ '<' <fact-params> '>' ]
 */
#if SLVL >= 7
static
rc_t phys_enc_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SPhysEncExpr *x, const KSymbol **sym )
{
    rc_t rc;

    /* schema parms */
    if ( t -> id == eLeftAngle )
    {
        next_token ( tbl, src, t );
        rc = param_list ( tbl, src, t, env, self, & x -> schem, schema_value );
        if ( rc == 0 )
            rc = expect ( tbl, src, t, eRightAngle, ">", true );
        if ( rc != 0 )
            return rc;
    }

    /* phys-encoding-name */
    rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;

    if ( t -> id != ePhysical )
        return KTokenExpected ( t, klogErr, "physical encoding" );
    * sym = t -> sym;


    /* look for requested version */
    if ( next_token ( tbl, src, t ) -> id == eHash )
    {
        x -> version_requested = true;
        rc = maj_min_rel ( tbl, src, next_token ( tbl, src, t ),
            env, self,  & x -> version, false );
        if ( rc != 0 )
            return rc;
    }

    /* look for factory params */
    if ( t -> id == eLeftAngle )
    {
        next_token ( tbl, src, t );
        rc = param_list ( tbl, src, t, env, self, & x -> pfact, fact_value );
        if ( rc == 0 )
            rc = expect ( tbl, src, t, eRightAngle, ">", true );
    }

    return rc;
}

rc_t phys_encoding_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, VTypedecl *td, const SExpression **xp )
{
    rc_t rc;
    const KSymbol *sym = NULL;

    SPhysEncExpr *x = malloc ( sizeof * x );
    if ( x == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    /* initialize */
    x -> dad . var = ePhysEncExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> phys = NULL;
    VectorInit ( & x -> schem, 0, 4 );
    VectorInit ( & x -> pfact, 0, 8 );
    x -> version = 0;
    x -> version_requested = false;

    /* parse expression */
    rc = phys_enc_expr ( tbl, src, t, env, self, x, & sym );
    if ( rc == 0 )
    {
        const SNameOverload *name = sym -> u . obj;

        if ( x -> version_requested )
        {
            x -> phys =
                VectorFind ( & name -> items, & x -> version, NULL, SPhysicalCmp );
        }
        else
        {
            x -> phys = VectorLast ( & name -> items );
        }

        /* evaluate type expression */
        if ( x -> phys != NULL && td != NULL )
        {
            /* bind schema parameters */
            Vector prior, cx_bind;
            VectorInit ( & cx_bind, 1, self -> num_indirect ? self -> num_indirect : 16 );
            rc = SPhysicalBindSchemaParms ( x -> phys, & prior, & x -> schem, & cx_bind );
            if ( rc == 0 )
            {
                const SExpression *tx = x -> phys -> td;
                assert ( tx != NULL );
                rc = STypeExprResolveAsTypedecl ( ( const STypeExpr* ) tx, self, td, & cx_bind );
                SPhysicalRestSchemaParms ( x -> phys, & prior, & cx_bind );
            }
            VectorWhack ( & cx_bind, NULL, NULL );
        }

        if ( rc == 0 )
        {
            * xp = & x -> dad;
            return 0;
        }
    }

    SExpressionWhack ( & x -> dad );
    return rc;
}
#endif
