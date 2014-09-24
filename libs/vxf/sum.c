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
#include <vdb/xform.h>
#include <arch-impl.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* optional constant */
typedef union sum_data sum_data;
union sum_data
{
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;

    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;

    float f32;
    double f64;
};

#define FULL_SUM_NAME( T )                                               \
    full_sum_ ## T
#define FULL_SUM( T, k )                                                 \
static                                                                   \
rc_t CC FULL_SUM_NAME ( T ) ( void *data,                                   \
    const VXformInfo *info, int64_t row_id, const VFixedRowResult *rslt, \
    uint32_t argc, const VRowData argv [] )                              \
{                                                                        \
    uint32_t i, j;                                                       \
    const sum_data *self = ( const void* ) data;                         \
                                                                         \
    T *dst = rslt -> base;                                               \
    const T *a = argv [ 0 ] . u . data . base;                           \
    const T *b = argv [ 1 ] . u . data . base;                           \
                                                                         \
    for ( i = 0, dst += rslt -> first_elem,                              \
              a += argv [ 0 ] . u . data . first_elem,                   \
              b += argv [ 1 ] . u . data . first_elem;                   \
          i < rslt -> elem_count; ++ i )                                 \
    {                                                                    \
        dst [ i ] = a [ i ] + b [ i ] + self -> k;                       \
        for ( j = 2; j < argc; ++ j )                                    \
        {                                                                \
            const T *c = argv [ j ] . u . data . base;                   \
            dst [ i ] += c [ argv [ j ] . u . data . first_elem + i ];   \
        }                                                                \
    }                                                                    \
                                                                         \
    return 0;                                                            \
}

FULL_SUM ( int8_t, i8 )
FULL_SUM ( int16_t, i16 )
FULL_SUM ( int32_t, i32 )
FULL_SUM ( int64_t, i64 )
FULL_SUM ( uint8_t, u8 )
FULL_SUM ( uint16_t, u16 )
FULL_SUM ( uint32_t, u32 )
FULL_SUM ( uint64_t, u64 )
FULL_SUM ( float, f32 )
FULL_SUM ( double, f64 )

static VFixedRowFunc full_sum_func [] =
{
    FULL_SUM_NAME ( uint8_t ),
    FULL_SUM_NAME ( uint16_t ),
    FULL_SUM_NAME ( uint32_t ),
    FULL_SUM_NAME ( uint64_t ),
    FULL_SUM_NAME ( int8_t ),
    FULL_SUM_NAME ( int16_t ),
    FULL_SUM_NAME ( int32_t ),
    FULL_SUM_NAME ( int64_t ),
    NULL,
    NULL,
    FULL_SUM_NAME ( float ),
    FULL_SUM_NAME ( double )
};

#define CONST_SUM_NAME( T )                                              \
    const_sum_ ## T
#define CONST_SUM( T, k )                                                \
static                                                                   \
rc_t CC CONST_SUM_NAME ( T ) ( void *data, const VXformInfo *info,          \
    void *rslt, const void *src, uint64_t elem_count )                   \
{                                                                        \
    uint32_t i;                                                          \
    const sum_data *self = ( const void* ) data;                         \
                                                                         \
    T *dst = rslt;                                                       \
    const T *a = src;                                                    \
                                                                         \
    for ( i = 0; i < elem_count; ++ i )                                  \
    {                                                                    \
        dst [ i ] = a [ i ] + self -> k;                                 \
    }                                                                    \
                                                                         \
    return 0;                                                            \
}

CONST_SUM ( int8_t, i8 )
CONST_SUM ( int16_t, i16 )
CONST_SUM ( int32_t, i32 )
CONST_SUM ( int64_t, i64 )
CONST_SUM ( uint8_t, u8 )
CONST_SUM ( uint16_t, u16 )
CONST_SUM ( uint32_t, u32 )
CONST_SUM ( uint64_t, u64 )
CONST_SUM ( float, f32 )
CONST_SUM ( double, f64 )

static VArrayFunc const_sum_func [] =
{
    CONST_SUM_NAME ( uint8_t ),
    CONST_SUM_NAME ( uint16_t ),
    CONST_SUM_NAME ( uint32_t ),
    CONST_SUM_NAME ( uint64_t ),
    CONST_SUM_NAME ( int8_t ),
    CONST_SUM_NAME ( int16_t ),
    CONST_SUM_NAME ( int32_t ),
    CONST_SUM_NAME ( int64_t ),
    NULL,
    NULL,
    CONST_SUM_NAME ( float ),
    CONST_SUM_NAME ( double )
};

#define NO_CONST_NAME( T )                                               \
    no_const_ ## T
#define NO_CONST( T )                                                    \
static                                                                   \
rc_t CC NO_CONST_NAME ( T ) ( void *data,                                   \
    const VXformInfo *info, int64_t row_id, const VFixedRowResult *rslt, \
    uint32_t argc, const VRowData argv [] )                              \
{                                                                        \
    uint32_t i;                                                          \
                                                                         \
    T *dst = rslt -> base;                                               \
    const T *a = argv [ 0 ] . u . data . base;                           \
    const T *b = argv [ 1 ] . u . data . base;                           \
                                                                         \
    for ( i = 0, dst += rslt -> first_elem,                              \
              a += argv [ 0 ] . u . data . first_elem,                   \
              b += argv [ 1 ] . u . data . first_elem;                   \
          i < rslt -> elem_count; ++ i )                                 \
    {                                                                    \
        dst [ i ] = a [ i ] + b [ i ];                                   \
    }                                                                    \
                                                                         \
    return 0;                                                            \
}

NO_CONST ( int8_t )
NO_CONST ( int16_t )
NO_CONST ( int32_t )
NO_CONST ( int64_t )
NO_CONST ( uint8_t )
NO_CONST ( uint16_t )
NO_CONST ( uint32_t )
NO_CONST ( uint64_t )
NO_CONST ( float )
NO_CONST ( double )

static VFixedRowFunc no_const_func [] =
{
    NO_CONST_NAME ( uint8_t ),
    NO_CONST_NAME ( uint16_t ),
    NO_CONST_NAME ( uint32_t ),
    NO_CONST_NAME ( uint64_t ),
    NO_CONST_NAME ( int8_t ),
    NO_CONST_NAME ( int16_t ),
    NO_CONST_NAME ( int32_t ),
    NO_CONST_NAME ( int64_t ),
    NULL,
    NULL,
    NO_CONST_NAME ( float ),
    NO_CONST_NAME ( double )
};

static
rc_t CC no_sum ( void *data, const VXformInfo *info,
    void *dst, const void *src, uint64_t elem_count )
{
    memcpy ( dst, src, (size_t)( elem_count * VTypedescSizeof ( & info -> fdesc . desc ) >> 3 ) );
    return 0;
}

static
void CC vxf_sum_wrapper( void *ptr )
{
	free( ptr );
}

/* sum
 *  return the sum of inputs
 *
 *  "T" [ TYPE ] - input and output data type
 *  must be member of numeric_set
 *
 *  "k" [ CONST, DEFAULT 0 ] - optional constant
 *  to be added
 *
 *  "a" [ DATA ] - left-most operand
 *
 * SYNOPSIS:
 *  incorporates "k" into expression for every row
 *  returns sum or sumerence of inputs for all rows
 */
VTRANSFACT_IMPL ( vdb_sum, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    bool has_const;
    int32_t size_idx;
    sum_data k;

    /* "T" must be member of numeric_set */
    switch ( info -> fdesc . desc . domain )
    {
    case vtdUint:
    case vtdInt:
    case vtdFloat:
        break;
    default:
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );
    }

    /* TBD - eventually support vector sums
       for today, check that dim of T is 1 */
    if ( dp -> argv [ 0 ] . desc . intrinsic_dim != 1 )
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );

    /* the only numeric types we support are between 8 and 64 bits */
    size_idx = uint32_lsbit ( dp -> argv [ 0 ] . desc . intrinsic_bits ) - 3;
    if ( size_idx < 0 || size_idx > 3 || ( ( dp -> argv [ 0 ] . desc . intrinsic_bits &
                                             ( dp -> argv [ 0 ] . desc . intrinsic_bits - 1 ) ) != 0 ) )
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcIncorrect );


    /* there are 5 variants:
       1. no constant ( or is 0 ) and single input
       2. non-zero constant and single input
       3. no constant ( or is 0 ) and dual input
       4. non-zero constant and dual input
       5. more than 2 inputs */
    has_const = false;
    if ( cp -> argc == 0 )
        memset ( & k, 0, sizeof k );
    else switch ( cp -> argv [ 0 ] . desc . domain )
    {
    case vtdUint:
    case vtdInt:
        switch ( cp -> argv [ 0 ] . desc . intrinsic_bits )
        {
        case 8:
            if ( ( k . u8 = cp -> argv [ 0 ] . data . u8 [ 0 ] ) != 0 )
                has_const = true;
            break;
        case 16:
            if ( ( k . u16 = cp -> argv [ 0 ] . data . u16 [ 0 ] ) != 0 )
                has_const = true;
            break;
        case 32:
            if ( ( k . u32 = cp -> argv [ 0 ] . data . u32 [ 0 ] ) != 0 )
                has_const = true;
            break;
        case 64:
            if ( ( k . u64 = cp -> argv [ 0 ] . data . u64 [ 0 ] ) != 0 )
                has_const = true;
            break;
        }
        break;

    case vtdFloat:
        switch ( cp -> argv [ 0 ] . desc . intrinsic_bits )
        {
        case 32:
            if ( ( k . f32 = cp -> argv [ 0 ] . data . f32 [ 0 ] ) != 0.0 )
                has_const = true;
            break;
        case 64:
            if ( ( k . f64 = cp -> argv [ 0 ] . data . f64 [ 0 ] ) != 0.0 )
                has_const = true;
            break;
        }
        break;
    }

    if ( has_const || dp -> argc > 2 )
    {
        sum_data *pb = malloc ( sizeof * pb );
        if ( pb == NULL )
            return RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
        * pb = k;
        
        rslt -> self = pb;
        rslt -> whack = vxf_sum_wrapper;

        if ( dp -> argc > 1 )
        {
            rslt -> u . pf = full_sum_func [ ( dp -> argv [ 0 ] . desc . domain - vtdUint ) * 4 + size_idx ];
            rslt -> variant = vftFixedRow;
        }
        else
        {
            rslt -> u . af = const_sum_func [ ( dp -> argv [ 0 ] . desc . domain - vtdUint ) * 4 + size_idx ];
            rslt -> variant = vftArray;
        }
    }
    else
    {
        if ( dp -> argc > 1 )
        {
            rslt -> u . pf = no_const_func [ ( dp -> argv [ 0 ] . desc . domain - vtdUint ) * 4 + size_idx ];
            rslt -> variant = vftFixedRow;
        }
        else
        {
            rslt -> u . af = no_sum;
            rslt -> variant = vftArray;
        }
    }

    return 0;
}
