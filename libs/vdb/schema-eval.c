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

#include <klib/symbol.h>
#include <klib/symtab.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <bitstr.h>
#include <atomic32.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VSchema
 */


/* CacheIntrinsicTypeId
 *  for id-caching
 *
 *  reads id atomically from "cache" and returns if non-zero
 *
 *  otherwise retrieves type id by name lookup, and sets
 *  value atomically in "cache" before returning.
 */
uint32_t VSchemaCacheIntrinsicTypeId ( const VSchema *self,
    atomic32_t *cache, const char *type_name )
{
    uint32_t id;

    assert ( self != NULL );
    assert ( cache != NULL );
    assert ( type_name != NULL && type_name [ 0 ] != 0 );

    /* retrieve cached value */
    id = atomic32_read ( cache );
    if ( id == 0 )
    {
        String name;
        uint32_t prev;
        const KSymbol *sym;
        const SDatatype *dt;

        /* find intrinsic schema */
        const VSchema *intrinsic = self;
        while ( intrinsic -> dad != NULL )
            intrinsic = intrinsic -> dad;

        /* find datatype U32 */
        StringInitCString ( & name, type_name );
        sym = ( const KSymbol* )
            BSTreeFind ( & intrinsic -> scope, & name, KSymbolCmp );
        assert ( sym != NULL );
        dt = sym -> u . obj;

        /* this SHOULD be idempotent */
        prev = atomic32_test_and_set ( cache, ( int ) dt -> id, id );
        assert ( prev == id || prev == dt -> id );

        id = dt -> id;
    }

    return id;
}


/*--------------------------------------------------------------------------
 * constant numeric expressions
 *
 *  built-in typecasting allows for the chain
 *    bool => uint => int => float
 *
 *  built-in size promotion is automatic from 8 => 16 => 32 => 64 bits
 *
 *  constant size-demotion is allowed due to knowledge of constant value
 *
 *  explicit casts may reverse type promotion rules
 */


/* 64 bit interchange union
 */
typedef union nx nx;
union nx
{
    uint64_t u64;
    int64_t i64;
    double f64;
};


/* readers
 *  access data in 8, 16, 32 or 64 bit form
 *  return data in 64 bit form
 *  advance data pointer
 */
static
const void *read_bool ( nx *data, const void *p )
{
    const bool *src = p;
    data -> u64 = src [ 0 ];
    return src + 1;
}

static
const void *read_U8 ( nx *data, const void *p )
{
    const uint8_t *src = p;
    data -> u64 = src [ 0 ];
    return src + 1;
}

static
const void *read_U16 ( nx *data, const void *p )
{
    const uint16_t *src = p;
    data -> u64 = src [ 0 ];
    return src + 1;
}

static
const void *read_U32 ( nx *data, const void *p )
{
    const uint32_t *src = p;
    data -> u64 = src [ 0 ];
    return src + 1;
}

static
const void *read_U64 ( nx *data, const void *p )
{
    const uint64_t *src = p;
    data -> u64 = src [ 0 ];
    return src + 1;
}

static
const void *read_I8 ( nx *data, const void *p )
{
    const int8_t *src = p;
    data -> i64 = src [ 0 ];
    return src + 1;
}

static
const void *read_I16 ( nx *data, const void *p )
{
    const int16_t *src = p;
    data -> i64 = src [ 0 ];
    return src + 1;
}

static
const void *read_I32 ( nx *data, const void *p )
{
    const int32_t *src = p;
    data -> i64 = src [ 0 ];
    return src + 1;
}

static
const void *read_I64 ( nx *data, const void *p )
{
    const int64_t *src = p;
    data -> i64 = src [ 0 ];
    return src + 1;
}

static
const void *read_F32 ( nx *data, const void *p )
{
    const float *src = p;
    data -> f64 = src [ 0 ];
    return src + 1;
}

static
const void *read_F64 ( nx *data, const void *p )
{
    const double *src = p;
    data -> f64 = src [ 0 ];
    return src + 1;
}

/* writers
 *  write data in 8, 16, 32, or 64 bit form
 *  receive data in 64 bit form
 *  advance pointers
 */
static
void *write_bool ( void *p, const nx *data )
{
    bool *dst = p;
    dst [ 0 ] = data -> u64 ? true : false;
    return dst + 1;
}

static
void *write_U8 ( void *p, const nx *data )
{
    uint8_t *dst = p;
    dst [ 0 ] = ( uint8_t ) data -> u64;
    if ( ( uint64_t ) dst [ 0 ] != data -> u64 )
        return NULL;
    return dst + 1;
}

static
void *write_U16 ( void *p, const nx *data )
{
    uint16_t *dst = p;
    dst [ 0 ] = ( uint16_t ) data -> u64;
    if ( ( uint64_t ) dst [ 0 ] != data -> u64 )
        return NULL;
    return dst + 1;
}

static
void *write_U32 ( void *p, const nx *data )
{
    uint32_t *dst = p;
    dst [ 0 ] = ( uint32_t ) data -> u64;
    if ( ( uint64_t ) dst [ 0 ] != data -> u64 )
        return NULL;
    return dst + 1;
}

static
void *write_U64 ( void *p, const nx *data )
{
    uint64_t *dst = p;
    dst [ 0 ] = data -> u64;
    return dst + 1;
}

static
void *write_I8 ( void *p, const nx *data )
{
    int8_t *dst = p;
    dst [ 0 ] = ( int8_t ) data -> i64;
    if ( ( int64_t ) dst [ 0 ] != data -> i64 )
        return NULL;
    return dst + 1;
}

static
void *write_I16 ( void *p, const nx *data )
{
    int16_t *dst = p;
    dst [ 0 ] = ( int16_t ) data -> i64;
    if ( ( int64_t ) dst [ 0 ] != data -> i64 )
        return NULL;
    return dst + 1;
}

static
void *write_I32 ( void *p, const nx *data )
{
    int32_t *dst = p;
    dst [ 0 ] = ( int32_t ) data -> i64;
    if ( ( int64_t ) dst [ 0 ] != data -> i64 )
        return NULL;
    return dst + 1;
}

static
void *write_I64 ( void *p, const nx *data )
{
    int64_t *dst = p;
    dst [ 0 ] = data -> i64;
    return dst + 1;
}

static
void *write_F32 ( void *p, const nx *data )
{
    double e;
    float *dst = p;
    dst [ 0 ] = ( float ) data -> f64;
    e = ( double ) dst [ 0 ] - data -> f64;
    if ( -1.0 < e || e > 1.0 )
        return NULL;
    return dst + 1;
}

static
void *write_F64 ( void *p, const nx *data )
{
    double *dst = p;
    dst [ 0 ] = data -> f64;
    return dst + 1;
}


/* converters
 *  convert from one data type to another
 */
static
nx *null_cvt ( nx *data )
{
    return data;
}

static
nx *uint_int ( nx *data )
{
    if ( data -> i64 < 0 )
        return NULL;
    return data;
}

static
nx *uint_float ( nx *data )
{
    if ( data -> i64 < 0 )
        return NULL;
    data -> f64 = (double)data -> i64;
    return data;
}

static
nx *int_float ( nx *data )
{
    data -> f64 = (double)data -> i64;
    return data;
}

static
nx *float_int ( nx *data )
{
    data -> i64 = ( int64_t ) data -> f64;
    return data;
}

static
nx *float_uint ( nx *data )
{
    data -> i64 = ( int64_t ) data -> f64;
    if ( data -> i64 < 0 )
        return NULL;
    return data;
}

static
nx *int_uint ( nx *data )
{
    if ( data -> i64 < 0 )
        return NULL;
    return data;
}

/* eval-numeric-expr
 *  tries to evaluate a constant numeric expression against type
 *  returns non-zero error code if failed
 */
#if SLVL >= 1
LIB_EXPORT rc_t CC eval_numeric_expr ( const VSchema *self, const VTypedecl *td,
    const SConstExpr *expr, SConstExpr **xp, bool force )
{
    SConstExpr *x;
    uint32_t sbits, sdim;
    uint32_t i, dbits, ddim;

    void *dst;
    const void *src;

    const void* ( * reader ) ( nx*, const void* );
    void* ( * writer ) ( void*, const nx* );
    nx* ( * cvt ) ( nx* );

    /* source and destination types of constant vector */
    const SDatatype *sdt = VSchemaFindTypeid ( self, expr -> td . type_id );
    const SDatatype *ddt = VSchemaFindTypeid ( self, td -> type_id );
    if ( sdt == NULL || ddt == NULL )
        return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcNotFound );

    /* size of source and destination types */
    sbits = sdt -> size;
    dbits = ddt -> size;

    /* regardless of how derived the specified types may be,
       get their dimensions in terms of intrinsic types */
    sdim = SDatatypeIntrinsicDim ( sdt );
    ddim = SDatatypeIntrinsicDim ( ddt );

    /* convert stated sizes to intrinsic element size */
    assert ( sdim != 0 && ( sbits % sdim ) == 0 );
    sbits /= sdim;
    assert ( ddim != 0 && ( dbits % ddim ) == 0 );
    dbits /= ddim;

    /* test for cast compatibility */
    if ( ! force ) switch ( ddt -> domain )
    {
    case ddBool:
        /* only bool casts to bool */
        if ( sdt -> domain != ddBool )
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );
        break;
    case ddUint: case ddInt: case ddFloat:
        if ( sdt -> domain > ddt -> domain )
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );
        break;
    default:
        return SILENT_RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnexpected );
    }

    /* reset dimensions to account for explicit values */
    sdim *= expr -> td . dim;
    i = ddim * td -> dim;

    /* destination dim must divide source dim evenly */
    if ( i == 0 || sdim < i || ( sdim % i ) != 0 )
        return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );

    /* determine reader */
    switch ( sdt -> domain )
    {
    case ddBool:
        reader = read_bool; break;
    case ddUint:
        switch ( sbits )
        {
        case 8:  reader = read_U8; break;
        case 16: reader = read_U16; break;
        case 32: reader = read_U32; break;
        case 64: reader = read_U64; break;
        default:
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnexpected );
        }
        break;
    case ddInt:
        switch ( sbits )
        {
        case 8:  reader = read_I8; break;
        case 16: reader = read_I16; break;
        case 32: reader = read_I32; break;
        case 64: reader = read_I64; break;
        default:
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnexpected );
        }
        break;
    case ddFloat:
        switch ( sbits )
        {
        case 32: reader = read_F32; break;
        case 64: reader = read_F64; break;
        default:
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnexpected );
        }
        break;
    default:
        /* here to quiet compiler complaints */
        reader = NULL;
    }

    /* determine writer */
    switch ( ddt -> domain )
    {
    case ddBool:
        writer = write_bool; break;
    case ddUint:
        switch ( dbits )
        {
        case 8:  writer = write_U8; break;
        case 16: writer = write_U16; break;
        case 32: writer = write_U32; break;
        case 64: writer = write_U64; break;
        default:
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnexpected );
        }
        break;
    case ddInt:
        switch ( dbits )
        {
        case 8:  writer = write_I8; break;
        case 16: writer = write_I16; break;
        case 32: writer = write_I32; break;
        case 64: writer = write_I64; break;
        default:
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnexpected );
        }
        break;
    case ddFloat:
        switch ( dbits )
        {
        case 32: writer = write_F32; break;
        case 64: writer = write_F64; break;
        default:
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnexpected );
        }
        break;
    default:
        writer = NULL;
    }

    /* determine converter */
    if ( sdt -> domain == ddt -> domain )
        cvt = null_cvt;
    else switch ( ddt -> domain )
    {
    case ddBool:
    case ddUint:
        switch ( sdt -> domain )
        {
        case ddInt:
            cvt = int_uint; break;
        case ddFloat:
            cvt = float_uint; break;
        default:
            cvt = null_cvt;
        }
        break;
    case ddInt:
        switch ( sdt -> domain )
        {
        case ddFloat:
            cvt = float_int; break;
        default:
            cvt = uint_int; break;
        }
        break;
    case ddFloat:
        switch ( sdt -> domain )
        {
        case ddInt:
            cvt = int_float; break;
        default:
            cvt = uint_float; break;
        }
        break;
    default:
        cvt = null_cvt;
    }

    /* create output object */
    {
	unsigned int alloc_size;

	alloc_size=(((size_t)dbits * sdim + 7) >> 3);
	if(alloc_size < sizeof(x->u)){ /** don't go below size of union ***/
		alloc_size=sizeof(*x);
	} else { /** overallocate here ***/
		alloc_size+=sizeof(*x)-sizeof(x->u);
	}
	alloc_size = (alloc_size+3)&~3; /** align to 4 bytes **/
    	x = malloc ( alloc_size );
	if ( x == NULL )
            return RC ( rcVDB, rcExpression, rcEvaluating, rcMemory, rcExhausted );
	memset(x,0,alloc_size);
    }

    /* copy, and perform type conversion */
    src = & expr -> u;
    dst = & x -> u;
    for ( i = 0; i < sdim; ++ i )
    {
        nx data;
        const nx *dp;

        /* read element */
        src = ( * reader ) ( & data, src );

        /* type-promote */
        dp = ( * cvt ) ( & data );
        if ( dp == NULL )
        {
            free ( x );
            return RC ( rcVDB, rcExpression, rcEvaluating, rcConstraint, rcViolated );
        }

        /* write element */
        dst = ( * writer ) ( dst, dp );
        if ( dst == NULL )
        {
            free ( x );
            return RC ( rcVDB, rcExpression, rcEvaluating, rcConstraint, rcViolated );
        }
    }

    /* it is done */
    x -> dad . var = eConstExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> td . type_id = td -> type_id;
    x -> td . dim = sdim / ddim;
    * xp = x;
    return 0;
}
#endif


/*--------------------------------------------------------------------------
 * constant textual expressions
 *
 *  built-in typecasting allows for the chain
 *    ascii => unicode
 *
 *  built-in size promotion is automatic from 8 => 16 => 32 bits
 *
 *  constant size-demotion is allowed due to knowledge of constant value
 *
 *  explicit casts may reverse type promotion rules
 */


static
const void *read_ascii ( uint32_t *ch, const void *p, const void *end )
{
    const char *src = p;
    if ( src [ 0 ] < 0 )
        return NULL;
    * ch = src [ 0 ];
    return src + 1;
}

static
const void *read_utf8 ( uint32_t *ch, const void *p, const void *end )
{
    const char *src = p;
    int len = utf8_utf32 ( ch, src, end );
    if ( len <= 0 )
        return NULL;
    return src + len;
}

static
const void *read_utf16 ( uint32_t *ch, const void *p, const void *end )
{
    const uint16_t *src = p;
    * ch = src [ 0 ];
    return src + 1;
}

static
const void *read_utf32 ( uint32_t *ch, const void *p, const void *end )
{
    const uint32_t *src = p;
    * ch = src [ 0 ];
    return src + 1;
}

static
void *write_ascii ( void *p, void *end, uint32_t ch )
{
    char *dst = p;
    if ( ch >= 128 )
        ch = '?';
    dst [ 0 ] = ( char ) ch;
    return dst + 1;
}

static
void *write_utf8 ( void *p, void *end, uint32_t ch )
{
    char *dst = p;
    int len = utf32_utf8 ( dst, end, ch );
    if ( len <= 0 )
        return NULL;
    return dst + len;
}

static
void *write_utf16 ( void *p, void *end, uint32_t ch )
{
    uint16_t *dst = p;
    if ( ch > 0x10000 )
        ch = '?';
    dst [ 0 ] = ( uint16_t ) ch;
    return dst + 1;
}

static
void *write_utf32 ( void *p, void *end, uint32_t ch )
{
    uint32_t *dst = p;
    dst [ 0 ] = ch;
    return dst + 1;
}

#if SLVL >= 1
LIB_EXPORT rc_t CC eval_text_expr ( const VSchema *self, const VTypedecl *td,
    const SConstExpr *expr, SConstExpr **xp, bool force )
{
    size_t size;
    uint32_t len;
    SConstExpr *x;
    uint32_t sbits, sdim;
    uint32_t i, dbits, ddim;

    void *dst, *dend;
    const void *src, *send;
    void* ( * writer ) ( void*, void*, uint32_t );
    const void* ( * reader ) ( uint32_t*, const void*, const void* );

    /* source and destination types of constant vector */
    const SDatatype *sdt = VSchemaFindTypeid ( self, expr -> td . type_id );
    const SDatatype *ddt = VSchemaFindTypeid ( self, td -> type_id );
    if ( sdt == NULL || ddt == NULL )
        return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcNotFound );

    /* size of source and destination types */
    sbits = sdt -> size;
    dbits = ddt -> size;

    /* regardless of how derived the specified types may be,
       get their dimensions in terms of intrinsic types */
    sdim = SDatatypeIntrinsicDim ( sdt );
    ddim = SDatatypeIntrinsicDim ( ddt );

    /* convert stated sizes to intrinsic element size */
    assert ( sdim != 0 && ( sbits % sdim ) == 0 );
    sbits /= sdim;
    assert ( ddim != 0 && ( dbits % ddim ) == 0 );
    dbits /= ddim;

    /* test for cast compatibility */
    if ( ! force ) switch ( ddt -> domain )
    {
    case ddAscii:
        /* only ascii casts to ascii */
        if ( sdt -> domain != ddAscii )
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );
        break;
    case ddUnicode:
        if ( sdt -> domain < ddAscii || sdt -> domain > ddt -> domain )
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );
        break;
    default:
        return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnexpected );
    }

    /* restore source dimension */
    sdim *= expr -> td . dim;

    /* determine reader and length */
    len = sdim;
    switch ( sdt -> domain )
    {
    case ddAscii:
        reader = read_ascii; break;
    default:
        switch ( sbits )
        {
        case 8:
            reader = read_utf8;
            len = string_len ( expr -> u . utf8, sdim );
            break;
        case 16:
            reader = read_utf16;
            break;
        case 32:
            reader = read_utf32;
            break;
        default:
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnexpected );
        }
    }

    /* determine writer and size */
    ddim = len;
    switch ( ddt -> domain )
    {
    case ddAscii:
        writer = write_ascii;
        size = len;
        break;
    default:
        switch ( sbits )
        {
        case 8:
            writer = write_utf8;
            switch ( dbits )
            {
            case 16:
                utf16_cvt_string_len ( expr -> u . utf16, len << 1, & size );
                break;
            case 32:
                utf32_cvt_string_len ( expr -> u . utf32, len << 2, & size );
                break;
            default:
                size = sdim;
            }
            ddim = ( uint32_t ) size;
            break;
        case 16:
            writer = write_utf16;
            size = len << 1;
            break;
        case 32:
            writer = write_utf32;
            size = len << 2;
            break;
        default:
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnexpected );
        }
    }

    /* create output object */
    {
	unsigned int alloc_size;

	alloc_size=size;
	if(alloc_size < sizeof(x->u)){ /** don't go below size of union ***/
		alloc_size=sizeof(*x);
	} else { /** overallocate here ***/
		alloc_size+=sizeof(*x)-sizeof(x->u);
	}
	alloc_size = (alloc_size+3)&~3; /** align to 4 bytes **/
    	x = malloc ( alloc_size );
	if ( x == NULL )
		return RC ( rcVDB, rcExpression, rcEvaluating, rcMemory, rcExhausted );
	memset(x,0,alloc_size);
    }


    /* copy, and perform type conversion */
    src = expr -> u . utf8;
    send = & expr -> u . utf8 [ sdim ];
    dst = x -> u . utf8;
    dend = & x -> u . utf8 [ size ];
    for ( i = 0; i < len; ++ i )
    {
        uint32_t ch;

        /* read character */
        src = ( * reader ) ( & ch, src, send );
        if ( src == NULL )
        {
            free ( x );
            return RC ( rcVDB, rcExpression, rcEvaluating, rcData, rcCorrupt );
        }

        /* write character */
        dst = ( * writer ) ( dst, dend, ch );
        if ( dst == NULL )
        {
            free ( x );
            return RC ( rcVDB, rcExpression, rcEvaluating, rcData, rcCorrupt );
        }
    }

    /* it is done */
    x -> dad . var = eConstExpr;
    atomic32_set ( & x -> dad . refcount, 1 );
    x -> td . type_id = td -> type_id;
    x -> td . dim = ddim;
    * xp = x;
    return 0;
}
#endif


/*--------------------------------------------------------------------------
 * SConstExpr
 */


/* Resolve
 *  return constant expression as a vector of indicated type
 */
rc_t SConstExprResolveAsBool ( const SConstExpr *self,
    const VSchema *schema, bool *b, uint32_t capacity );
rc_t SConstExprResolveAsI8 ( const SConstExpr *self,
    const VSchema *schema, int8_t *i8, uint32_t capacity );
rc_t SConstExprResolveAsI16 ( const SConstExpr *self,
    const VSchema *schema, int16_t *i16, uint32_t capacity );
rc_t SConstExprResolveAsI32 ( const SConstExpr *self,
    const VSchema *schema, int32_t *i32, uint32_t capacity );
rc_t SConstExprResolveAsI64 ( const SConstExpr *self,
    const VSchema *schema, int64_t *i64, uint32_t capacity );
rc_t SConstExprResolveAsU8 ( const SConstExpr *self,
    const VSchema *schema, uint8_t *u8, uint32_t capacity );
rc_t SConstExprResolveAsU16 ( const SConstExpr *self,
    const VSchema *schema, uint16_t *u16, uint32_t capacity );

rc_t SConstExprResolveAsU32 ( const SConstExpr *self,
    const VSchema *schema, uint32_t *u32, uint32_t capacity, Vector *cx_bind )
{
    if ( capacity != 1 )
        return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUnsupported );
    return eval_uint_expr ( schema, ( const SExpression* ) self, u32, cx_bind );
}

rc_t SConstExprResolveAsU64 ( const SConstExpr *self,
    const VSchema *schema, uint32_t *u64, uint32_t capacity );
rc_t SConstExprResolveAsF32 ( const SConstExpr *self,
    const VSchema *schema, float *f32, uint32_t capacity );
rc_t SConstExprResolveAsF64 ( const SConstExpr *self,
    const VSchema *schema, double *f64, uint32_t capacity );
rc_t SConstExprResolveAsAscii ( const SConstExpr *self,
    const VSchema *schema, char *ascii, size_t capacity );
rc_t SConstExprResolveAsUTF8 ( const SConstExpr *self,
    const VSchema *schema, char *utf8, size_t bytes );
rc_t SConstExprResolveAsUTF16 ( const SConstExpr *self,
    const VSchema *schema, uint16_t *utf16, uint32_t capacity );
rc_t SConstExprResolveAsUTF32 ( const SConstExpr *self,
    const VSchema *schema, uint32_t *utf32, uint32_t capacity );


/*--------------------------------------------------------------------------
 * STypeExpr
 */

/* Resolve
 *  resolve type expression to either a VTypedecl or VFormatdecl
 */
rc_t STypeExprResolveAsFormatdecl ( const STypeExpr *self,
    const VSchema *schema, VFormatdecl *fd, Vector *cx_bind )
{
    rc_t rc;
    uint32_t dim;

    /* if self is resolved, then the format, type
       and dimension are completely resolved */
    if ( self -> resolved )
    {
        * fd = self -> fd;
        return 0;
    }

#if SLVL < 3
    return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );
#else
    /* the format is always completely resolved,
       this leaves type and/or dimension */

    if ( self -> id != NULL )
    {
        /* the type needs to be resolved */
        const STypeExpr *type = ( const STypeExpr* ) VectorGet ( cx_bind, self -> id -> type_id );
        if ( type == NULL )
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUndefined );
        if ( type -> dad . var != eTypeExpr )
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );
        rc = STypeExprResolveAsFormatdecl ( type, schema, fd, cx_bind );
        if ( rc != 0 )
            return rc;

        /* at this point, "fd" has some format - which we'll clobber,
           a type and dim from the indirect type just resolved */
    }
    else
    {
        /* take known type and default dimension */
        fd -> td . type_id = self -> fd . td . type_id;
        fd -> td . dim = 1;
    }

    /* always take our format */
    fd -> fmt = self -> fd . fmt;

    /* test for unresolved dimension */
    dim = self -> fd . td . dim;
    if ( dim == 0 && self -> dim != NULL )
    {
        const SExpression *dx = self -> dim;
        while ( dx != NULL )
        {
            const SIndirectConst *ic;

            if ( dx -> var == eConstExpr )
            {
                rc = SConstExprResolveAsU32 ( ( const SConstExpr* ) dx, schema, & dim, 1, cx_bind );
                if ( rc != 0 )
                    return rc;
                break;
            }

            if ( dx -> var != eIndirectExpr )
                return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );

            ic = ( ( const SSymExpr* ) dx ) -> _sym -> u . obj;
            assert ( ic != NULL );

            dx = ( const SExpression* ) VectorGet ( cx_bind, ic -> expr_id );
        }

        /* must have non-zero dim */
        if ( dim == 0 )
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcUndefined );
    }

    /* factor in our dimension */
    fd -> td . dim *= dim;
    return 0;
#endif
}

rc_t STypeExprResolveAsTypedecl ( const STypeExpr *self,
    const VSchema *schema, VTypedecl *td, Vector *cx_bind )
{
    VFormatdecl fd;
    rc_t rc = STypeExprResolveAsFormatdecl ( self, schema, & fd, cx_bind );
    if ( rc == 0 )
    {
        if ( fd . fmt != 0 )
            return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );
        * td = fd . td;
    }
    return rc;
}


/*--------------------------------------------------------------------------
 * general constant expression evaluation
 */
#if SLVL >= 1

/* eval-type-expr
 *  should be able to boil things down to defined type vector
 */
static
rc_t eval_type_expr ( const VSchema *self, const VTypedecl *td,
    const STypeExpr *expr, SExpression **xp )
{
    PLOGMSG( klogWarn, ( klogWarn, "TDB: $(msg)", "msg=handle type expression" ));
    return -1;
}

/* eval-indirect-expr
 */
static
rc_t eval_indirect_expr ( const VSchema *self, const VTypedecl *td,
    const SSymExpr *expr, SExpression **xp, Vector *cx_bind )
{
    const SIndirectConst *ic = expr -> _sym -> u . obj;

    /* if the expression is there */
    if ( ic -> expr_id != 0 )
    {
        const SExpression *ic_expr = ( const SExpression* ) VectorGet ( cx_bind, ic -> expr_id );
        if ( ic_expr != NULL )
            return eval_const_expr ( self, td, ic_expr, xp, cx_bind );
    }

    /* just return self */
    * xp = & ( ( SSymExpr* ) expr ) -> dad;
    atomic32_inc ( & ( ( SSymExpr* ) expr ) -> dad . refcount );
    return 0;
}


/* eval-const-cast-expr
 */
static
rc_t eval_const_cast_expr ( const VSchema *self, const VTypedecl *td,
    const SExpression *expr, SExpression **xp )
{
    PLOGMSG( klogWarn, ( klogWarn, "TDB: $(msg)", "msg=handle const cast expression" ));
    return -1;
}


/* eval-func-param-expr
 */
static
rc_t eval_func_param_expr ( const VSchema *self, const VTypedecl *td,
    const SExpression *expr, SExpression **xp )
{
    PLOGMSG( klogWarn, ( klogWarn, "TDB: $(msg)", "msg=handle function expression" ));
    return -1;
}

struct eval_vector_param_expr_pb
{
    rc_t rc;
    const VSchema *self;
    const VTypedecl *td;
    Vector *cx_bind;
    Vector v;
};

static
void CC vector_free ( void *item, void *data )
{
    free ( item );
}

static
bool CC do_eval_vector_param_expr ( void *item, void *data )
{
    struct eval_vector_param_expr_pb *pb = data;
    SExpression *rslt = NULL;

    pb -> rc = eval_const_expr ( pb -> self, pb -> td, item, & rslt, pb -> cx_bind );
    if ( pb -> rc != 0 )
        return true;

    switch ( rslt -> var )
    {
    case eConstExpr:
        VectorAppend ( & pb -> v, NULL, rslt );
        break;
    case eVectorExpr:
        free ( rslt );
        break;
    default:
        free ( rslt );
        pb -> rc = RC ( rcVDB, rcExpression, rcEvaluating, rcExpression, rcUnexpected );
        return true;
    }

    return false;
}

/* eval-vector-param-expr
 */
static
rc_t eval_vector_param_expr ( const VSchema *self, const VTypedecl *td,
    const SVectExpr *expr, SExpression **xp, Vector *cx_bind )
{
    struct eval_vector_param_expr_pb pb;

    *xp = NULL;

    pb . rc = 0;
    pb . self = self;
    pb . td = td;
    pb . cx_bind = cx_bind;
    VectorInit ( & pb . v, 0, 32 );

    VectorDoUntil ( & expr -> expr, 0, do_eval_vector_param_expr, & pb );
    if ( pb . rc == 0 )
    {
        SConstExpr *rslt;
        uint32_t elem_count = VectorLength ( & pb . v );
        uint32_t elem_bits;
        size_t alloc_size;

        {
            const SDatatype *dt = VSchemaFindTypeid ( self, td -> type_id );
            elem_bits = dt -> size;
        }

        alloc_size = ( ( ( size_t ) elem_bits * elem_count + 7 ) >> 3 );

        /** don't go below size of union ***/
        if ( alloc_size < sizeof rslt -> u )
            alloc_size = sizeof * rslt;
        /** overallocate here ***/
        else
            alloc_size += sizeof * rslt - sizeof rslt -> u ;

        /** align to 4 bytes **/
        alloc_size = ( alloc_size + 3 ) & ~ 3;
        rslt = calloc ( alloc_size, 1 );
        if ( rslt == NULL )
            pb . rc = RC ( rcVDB, rcExpression, rcEvaluating, rcMemory, rcExhausted );
        else
        {
            uint32_t i;

            rslt -> dad . var = eConstExpr;
            atomic32_set ( & rslt -> dad . refcount, 1 );
            rslt -> td = *td;
            rslt -> td . dim = elem_count;

            for ( i = 0; i != elem_count; ++i )
            {
                const SConstExpr *y = VectorGet ( & pb . v, i );
                bitcpy ( & rslt -> u, i * elem_bits, & y -> u, 0, elem_bits );
            }

            *xp = & rslt -> dad;
        }
    }

    VectorWhack( & pb . v, vector_free, 0 );

    return pb . rc;
}


/* eval-const-expr
 *  tries to evaluate a constant expression against type
 *  returns non-zero error code if failed
 */
rc_t eval_const_expr ( const VSchema *self, const VTypedecl *td,
    const SExpression *expr, SExpression **xp, Vector *cx_bind )
{
    rc_t rc;
    const SConstExpr *s;

    switch ( expr -> var )
    {
    case eTypeExpr:
        return eval_type_expr ( self, td, ( const STypeExpr* ) expr, xp );
    case eConstExpr:
        s = ( const SConstExpr* ) expr;
        break;
#if SLVL >= 3
    case eIndirectExpr:
        return eval_indirect_expr ( self, td, ( const SSymExpr* ) expr, xp, cx_bind );
#endif
#if SLVL >= 4
    case eFuncParamExpr:
        return eval_func_param_expr ( self, td, expr, xp );
#endif
    case eCastExpr:
        return eval_const_cast_expr ( self, td, expr, xp );
    case eVectorExpr:
        return eval_vector_param_expr(self, td, ( const SVectExpr* ) expr, xp, cx_bind );
    default:
        *xp = NULL;
        return RC ( rcVDB, rcExpression, rcEvaluating, rcExpression, rcUnexpected );
    }

    /* try to evaluate as numeric */
    rc = eval_numeric_expr ( self, td, s, ( SConstExpr** ) xp, false );

    /* fall over to text */
    if ( rc != 0 && GetRCState ( rc ) == rcUnexpected && GetRCObject ( rc ) == (enum RCObject)rcType )
        rc = eval_text_expr ( self, td, s, ( SConstExpr** ) xp, false );

    return rc;
}

/* eval-uint-expr
 *  special const expression evaluator for uint32_t
 */
rc_t eval_uint_expr ( const VSchema *self,
    const SExpression *expr, uint32_t *value, Vector *cx_bind )
{
    rc_t rc;
    VTypedecl td;
    SConstExpr *x;

    /* capture runtime value for U32 */
    static atomic32_t s_U32_id;
    uint32_t U32_id = VSchemaCacheIntrinsicTypeId ( self, & s_U32_id, "U32" );

    /* evaluate expression against type */
    td . type_id = U32_id;
    td . dim = 1;
    rc = eval_const_expr ( self, & td, expr, ( SExpression** ) & x, cx_bind );
    if ( rc != 0 )
        return rc;

    /* verify dimensionality */
    assert ( x -> dad . var == eConstExpr );
    if ( x -> td . dim != 1 )
        return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );

    /* got it */
    * value = x -> u . u32 [ 0 ];
    free ( x );
    return 0;
}


/* eval-uint64-expr
 *  special const expression evaluator for uint32_t
 */
rc_t eval_uint64_expr ( const VSchema *self,
    const SExpression *expr, uint64_t *value, Vector *cx_bind )
{
    rc_t rc;
    VTypedecl td;
    SConstExpr *x;

    /* capture runtime value for U64 */
    static atomic32_t s_U64_id;
    uint64_t U64_id = VSchemaCacheIntrinsicTypeId ( self, & s_U64_id, "U64" );

    /* evaluate expression against type */
    td . type_id = U64_id;
    td . dim = 1;
    rc = eval_const_expr ( self, & td, expr, ( SExpression** ) & x, cx_bind );
    if ( rc != 0 )
        return rc;

    /* verify dimensionality */
    assert ( x -> dad . var == eConstExpr );
    if ( x -> td . dim != 1 )
        return RC ( rcVDB, rcExpression, rcEvaluating, rcType, rcIncorrect );

    /* got it */
    * value = x -> u . u64 [ 0 ];
    free ( x );
    return 0;
}


/* eval-expr-syntax
 *  examine expression syntax
 *  fixes forward references
 */
static
bool CC eval_vect_expr_syntax ( void *item, void *data )
{
    rc_t *rc = data;
    SExpression *x = item;
    * rc = eval_expr_syntax ( x );
    return ( * rc != 0 ) ? true : false;
}

static
rc_t eval_type_expr_syntax ( const STypeExpr *expr )
{
    if ( expr -> dad . var != eTypeExpr )
        return RC ( rcVDB, rcExpression, rcValidating, rcExpression, rcIncorrect );
    return 0;
}

static
rc_t eval_fwd_expr_syntax ( SSymExpr *expr )
{
    const KSymbol *sym = expr -> _sym;
    assert ( sym != NULL );
    switch ( sym -> type )
    {
    case eFuncParam:
        expr -> dad . var = eParamExpr;
        break;
    case eProduction:
        expr -> dad . var = eProdExpr;
        break;
    case eColumn:
        expr -> dad . var = eColExpr;
        break;
    case ePhysMember:
        expr -> dad . var = ePhysExpr;
        break;
    case eForward:
        return RC ( rcVDB, rcSchema, rcValidating, rcName, rcUndefined );
    case eVirtual:
        break;

    default:
        return RC ( rcVDB, rcSchema, rcValidating, rcExpression, rcInvalid );
    }

    return 0;
}

rc_t eval_expr_syntax ( const SExpression *expr )
{
    rc_t rc;

    if ( expr == NULL )
        rc = RC ( rcVDB, rcExpression, rcValidating, rcSelf, rcNull );
    else switch ( expr -> var )
    {
    case eFwdExpr:
        rc = eval_fwd_expr_syntax ( ( SSymExpr* ) expr );
        break;
    case eCastExpr:
        rc = eval_type_expr_syntax ( ( const STypeExpr* ) ( ( const SBinExpr* ) expr ) -> left );
        if ( rc == 0 )
            rc = eval_expr_syntax ( ( ( const SBinExpr* ) expr ) -> right );
        break;
    case eVectorExpr:
        rc = 0;
        VectorDoUntil ( & ( ( const SVectExpr* ) expr ) -> expr, false, eval_vect_expr_syntax, & rc );
        break;
    case eCondExpr:
        rc = eval_expr_syntax ( ( ( const SBinExpr* ) expr ) -> left );
        if ( rc == 0 )
            rc = eval_expr_syntax ( ( ( const SBinExpr* ) expr ) -> right );
        break;
    default:
        rc = 0;
    }

    return rc;
}

#endif
