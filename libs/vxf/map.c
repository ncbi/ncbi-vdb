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
#include <klib/defs.h>
#include <klib/sort.h>
#include <klib/rc.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* LOOP TYPES
 *  this function can be characterized by its loops
 *
 *  1) single-input that is type-compatible with the output
 *     functions like the unix command "tr" by allowing all
 *     input to pass through, except those matching the "from"
 *     array, which are changed to the corresponding entry in
 *     the "to" array.
 *
 *  2) single-input that is NOT type-compatible with output
 *     functions like a type transformation where ALL input
 *     must be transformed, i.e. there can be no pass through.
 *     this loop requires detection of any unspecified input
 *     and results in an error.
 *
 *  3) dual-input that combines the other two loops, in a sense.
 *     it is like the first loop type in that there are no
 *     points of failure, and like the second in that it has
 *     to test for matches against the "from" array. its exact
 *     behavior is to pass values from the second input to the
 *     output unchanged, unless the corresponding value from the
 *     first input matches anything in the "from" table, in which
 *     case the corresponding "to" value is substituted to dest.
 */


/* type 1: 8-bit to 8-bit map
 *  this is our principal workhorse
 *  there are no points of failure
 *  the map is built initially for identity
 *  and overridden to perform substitutions indicated
 */
static
rc_t CC type1_uint8_t ( void *self, const VXformInfo *info,
    void *vdst, const void *vsrc, uint64_t elem_count )
{
    const uint8_t *map = ( const void* ) self;
    const uint8_t *src = vsrc;
    uint8_t *dst = vdst;

    uint64_t i;
    for ( i = 0; i < elem_count; ++ i )
        dst [ i ] = map [ src [ i ] ];

    return 0;
}

static
rc_t make_type1_uint8_t ( void **selfp, const uint8_t *from, const uint8_t *to, uint32_t map_len )
{
    uint32_t i;

    uint8_t *map = malloc ( 256 );
    if ( map == NULL )
        return RC ( rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted );

    for ( i = 0; i < 256; ++ i )
        map [ i ] = ( uint8_t ) i;

    for ( i = 0; i < map_len; ++ i )
        map [ from [ i ] ] = to [ i ];

    * selfp = map;
    return 0;
}


/* map_t
 *  a structure that holds "from" and "to" arrays
 */
typedef struct map_t map_t;
struct map_t
{
#if _DEBUGGING
    /* array function for type 1 and 2 loops */
    VArrayFunc array;
#endif

    /* row function for type 3 loops */
    void ( * row ) ( const map_t *self, void *dst,
        const void *src, const void *tst, uint64_t elem_count );

    /* map length */
    uint32_t map_len;
    uint32_t align;

    /* ordered "from" array and corresponding "to" array */
    const void *from, * to;
};


/* type 1: linear map
 *  when the arrays are so short as to not benefit from binary search
 */
#define TYPE1_LINEAR_MAP( TYPE )                                \
static                                                          \
rc_t CC type1_linear_ ## TYPE ( void *vself,                    \
    const VXformInfo *info, void *vdst, const void *vsrc,       \
    uint64_t elem_count )                                       \
{                                                               \
    const map_t *self = ( const void* ) vself;                  \
                                                                \
    const TYPE *src = vsrc;                                     \
    TYPE *dst = vdst;                                           \
                                                                \
    const TYPE *from = self -> from;                            \
    const TYPE *to = self -> to;                                \
                                                                \
    uint64_t i;                                                 \
    uint32_t j, map_len = self -> map_len;                      \
    for ( i = 0; i < elem_count; ++ i )                         \
    {                                                           \
        TYPE elem = dst [ i ] = src [ i ];                      \
        for ( j = 0; j < map_len; ++ j )                        \
        {                                                       \
            if ( from [ j ] == elem )                           \
            {                                                   \
                dst [ i ] = to [ j ];                           \
                break;                                          \
            }                                                   \
        }                                                       \
    }                                                           \
                                                                \
    return 0;                                                   \
}

TYPE1_LINEAR_MAP ( uint16_t )
TYPE1_LINEAR_MAP ( uint32_t )
TYPE1_LINEAR_MAP ( uint64_t )
TYPE1_LINEAR_MAP ( float )
TYPE1_LINEAR_MAP ( double )

static VArrayFunc type1_linear_funcs [ 5 ] =
{
    type1_linear_uint16_t,
    type1_linear_uint32_t,
    type1_linear_uint64_t,
    type1_linear_float,
    type1_linear_double
};


/* type 1: binary map
 *  when the arrays are long enough to warrant binary search
 */
#define TYPE1_BINARY_MAP( TYPE )                                \
static                                                          \
rc_t CC type1_binary_ ## TYPE ( void *vself,                    \
    const VXformInfo *info, void *vdst, const void *vsrc,       \
    uint64_t elem_count )                                       \
{                                                               \
    const map_t *self = ( const void* ) vself;                  \
                                                                \
    const TYPE *src = vsrc;                                     \
    TYPE *dst = vdst;                                           \
                                                                \
    const TYPE *from = self -> from;                            \
    const TYPE *to = self -> to;                                \
                                                                \
    uint64_t i;                                                 \
    for ( i = 0; i < elem_count; ++ i )                         \
    {                                                           \
        TYPE elem = dst [ i ] = src [ i ];                      \
                                                                \
        uint32_t lower = 0, upper = self -> map_len;            \
        while ( lower < upper )                                 \
        {                                                       \
            uint32_t j = ( lower + upper ) >> 1;                \
            TYPE cur = from [ j ];                              \
            if ( elem == cur )                                  \
            {                                                   \
                dst [ i ] = to [ j ];                           \
                break;                                          \
            }                                                   \
            if ( elem < cur )                                   \
                upper = j;                                      \
            else                                                \
                lower = j + 1;                                  \
        }                                                       \
    }                                                           \
                                                                \
    return 0;                                                   \
}

TYPE1_BINARY_MAP ( uint16_t )
TYPE1_BINARY_MAP ( uint32_t )
TYPE1_BINARY_MAP ( uint64_t )
TYPE1_BINARY_MAP ( float )
TYPE1_BINARY_MAP ( double )

static VArrayFunc type1_binary_funcs [ 5 ] =
{
    type1_binary_uint16_t,
    type1_binary_uint32_t,
    type1_binary_uint64_t,
    type1_binary_float,
    type1_binary_double
};

/* type 2: 8-bit to N-bit
 *  allowed to perform a lookup test
 */
#define TYPE2_8BIT_MAP( DTYPE )                                 \
static                                                          \
rc_t CC type2_uint8_t_to_ ## DTYPE ( void *vself,               \
    const VXformInfo *info, void *vdst, const void *vsrc,       \
    uint64_t elem_count )                                       \
{                                                               \
    const map_t *self = ( const void* ) vself;                  \
                                                                \
    const uint8_t *src = vsrc;                                  \
    DTYPE *dst = vdst;                                          \
                                                                \
    const uint8_t *from = self -> from;                         \
    const DTYPE *to = self -> to;                               \
                                                                \
    uint64_t i;                                                 \
    for ( i = 0; i < elem_count; ++ i )                         \
    {                                                           \
        int j = src [ i ];                                      \
        if ( ! from [ j ] )                                     \
        {                                                       \
            return RC ( rcVDB, rcFunction, rcExecuting, rcConstraint, rcViolated );              \
        }                                                       \
        dst [ i ] = to [ j ];                                   \
    }                                                           \
                                                                \
    return 0;                                                   \
}

TYPE2_8BIT_MAP ( uint8_t )
TYPE2_8BIT_MAP ( uint16_t )
TYPE2_8BIT_MAP ( uint32_t )
TYPE2_8BIT_MAP ( uint64_t )

/* type2: binary map
 *  due to the combinatorial explosion,
 *  just implement the binary version
 */
#define TYPE2_BINARY_MAP( STYPE, DTYPE )                        \
static                                                          \
rc_t CC type2_ ## STYPE ## _to_ ## DTYPE ( void *vself,         \
    const VXformInfo *info, void *vdst, const void *vsrc,       \
    uint64_t elem_count )                                       \
{                                                               \
    const map_t *self = ( const void* ) vself;                  \
                                                                \
    const STYPE *src = vsrc;                                    \
    DTYPE *dst = vdst;                                          \
                                                                \
    const STYPE *from = self -> from;                           \
    const DTYPE *to = self -> to;                               \
                                                                \
    uint64_t i;                                                 \
    for ( i = 0; i < elem_count; ++ i )                         \
    {                                                           \
        STYPE cur, elem = src [ i ];                            \
                                                                \
        uint32_t j, lower = 0, upper = self -> map_len;         \
        while ( lower < upper )                                 \
        {                                                       \
            j = ( lower + upper ) >> 1;                         \
            cur = from [ j ];                                   \
            if ( elem == cur )                                  \
                goto FOUND;                                     \
            if ( elem < cur )                                   \
                upper = j;                                      \
            else                                                \
                lower = j + 1;                                  \
        }                                                       \
        return RC( rcVDB, rcFunction, rcExecuting, rcConstraint, rcViolated );                  \
FOUND:                                                          \
        dst [ i ] = to [ j ];                                   \
    }                                                           \
                                                                \
    return 0;                                                   \
}

TYPE2_BINARY_MAP ( uint16_t, uint8_t )
TYPE2_BINARY_MAP ( uint16_t, uint16_t )
TYPE2_BINARY_MAP ( uint16_t, uint32_t )
TYPE2_BINARY_MAP ( uint16_t, uint64_t )

TYPE2_BINARY_MAP ( uint32_t, uint8_t )
TYPE2_BINARY_MAP ( uint32_t, uint16_t )
TYPE2_BINARY_MAP ( uint32_t, uint32_t )
TYPE2_BINARY_MAP ( uint32_t, uint64_t )

TYPE2_BINARY_MAP ( uint64_t, uint8_t )
TYPE2_BINARY_MAP ( uint64_t, uint16_t )
TYPE2_BINARY_MAP ( uint64_t, uint32_t )
TYPE2_BINARY_MAP ( uint64_t, uint64_t )

TYPE2_BINARY_MAP ( float, uint8_t )
TYPE2_BINARY_MAP ( float, uint16_t )
TYPE2_BINARY_MAP ( float, uint32_t )
TYPE2_BINARY_MAP ( float, uint64_t )

TYPE2_BINARY_MAP ( double, uint8_t )
TYPE2_BINARY_MAP ( double, uint16_t )
TYPE2_BINARY_MAP ( double, uint32_t )
TYPE2_BINARY_MAP ( double, uint64_t )

static VArrayFunc type2_funcs [ 6 ] [ 4 ] =
{
    /* uint8_t */
    {
        type2_uint8_t_to_uint8_t,
        type2_uint8_t_to_uint16_t,
        type2_uint8_t_to_uint32_t,
        type2_uint8_t_to_uint64_t
    },

    /* uint16_t */
    {
        type2_uint16_t_to_uint8_t,
        type2_uint16_t_to_uint16_t,
        type2_uint16_t_to_uint32_t,
        type2_uint16_t_to_uint64_t
    },

    /* uint32_t */
    {
        type2_uint32_t_to_uint8_t,
        type2_uint32_t_to_uint16_t,
        type2_uint32_t_to_uint32_t,
        type2_uint32_t_to_uint64_t
    },

    /* uint64_t */
    {
        type2_uint64_t_to_uint8_t,
        type2_uint64_t_to_uint16_t,
        type2_uint64_t_to_uint32_t,
        type2_uint64_t_to_uint64_t
    },

    /* float */
    {
        type2_float_to_uint8_t,
        type2_float_to_uint16_t,
        type2_float_to_uint32_t,
        type2_float_to_uint64_t
    },

    /* double */
    {
        type2_double_to_uint8_t,
        type2_double_to_uint16_t,
        type2_double_to_uint32_t,
        type2_double_to_uint64_t
    }
};


/* type 3: 8-bit to N-bit
 *  allowed to perform a lookup test
 */
#define TYPE3_8BIT_MAP( DTYPE )                                 \
static                                                          \
void type3_uint8_t_to_ ## DTYPE ( const map_t *self,            \
    void *vdst, const void *vsrc, const void *vtst,             \
    uint64_t elem_count )                                       \
{                                                               \
    const uint8_t *tst = vtst;                                  \
    const DTYPE *src = vsrc;                                    \
    DTYPE *dst = vdst;                                          \
                                                                \
    const uint8_t *from = self -> from;                         \
    const DTYPE *to = self -> to;                               \
                                                                \
    uint64_t i;                                                 \
    for ( i = 0; i < elem_count; ++ i )                         \
    {                                                           \
        int j = tst [ i ];                                      \
        dst [ i ] = src [ i ];                                  \
        if ( from [ j ] )                                       \
            dst [ i ] = to [ j ];                               \
    }                                                           \
}

TYPE3_8BIT_MAP ( uint8_t )
TYPE3_8BIT_MAP ( uint16_t )
TYPE3_8BIT_MAP ( uint32_t )
TYPE3_8BIT_MAP ( uint64_t )

/* type3: binary map
 *  due to the combinatorial explosion,
 *  just implement the binary version
 */
#define TYPE3_BINARY_MAP( STYPE, DTYPE )                        \
static                                                          \
void type3_ ## STYPE ## _to_ ## DTYPE ( const map_t *self,      \
    void *vdst, const void *vsrc, const void *vtst,             \
    uint64_t elem_count )                                       \
{                                                               \
    const STYPE *tst = vtst;                                    \
    const DTYPE *src = vsrc;                                    \
    DTYPE *dst = vdst;                                          \
                                                                \
    const STYPE *from = self -> from;                           \
    const DTYPE *to = self -> to;                               \
                                                                \
    uint64_t i;                                                 \
    for ( i = 0; i < elem_count; ++ i )                         \
    {                                                           \
        STYPE elem = tst [ i ];                                 \
        uint32_t lower = 0, upper = self -> map_len;            \
                                                                \
        dst [ i ] = src [ i ];                                  \
                                                                \
        while ( lower < upper )                                 \
        {                                                       \
            uint32_t j = ( lower + upper ) >> 1;                \
            STYPE cur = from [ j ];                             \
            if ( elem == cur )                                  \
            {                                                   \
                dst [ i ] = to [ j ];                           \
                break;                                          \
            }                                                   \
            if ( elem < cur )                                   \
                upper = j;                                      \
            else                                                \
                lower = j + 1;                                  \
        }                                                       \
    }                                                           \
}

TYPE3_BINARY_MAP ( uint16_t, uint8_t )
TYPE3_BINARY_MAP ( uint16_t, uint16_t )
TYPE3_BINARY_MAP ( uint16_t, uint32_t )
TYPE3_BINARY_MAP ( uint16_t, uint64_t )

TYPE3_BINARY_MAP ( uint32_t, uint8_t )
TYPE3_BINARY_MAP ( uint32_t, uint16_t )
TYPE3_BINARY_MAP ( uint32_t, uint32_t )
TYPE3_BINARY_MAP ( uint32_t, uint64_t )

TYPE3_BINARY_MAP ( uint64_t, uint8_t )
TYPE3_BINARY_MAP ( uint64_t, uint16_t )
TYPE3_BINARY_MAP ( uint64_t, uint32_t )
TYPE3_BINARY_MAP ( uint64_t, uint64_t )

TYPE3_BINARY_MAP ( float, uint8_t )
TYPE3_BINARY_MAP ( float, uint16_t )
TYPE3_BINARY_MAP ( float, uint32_t )
TYPE3_BINARY_MAP ( float, uint64_t )

TYPE3_BINARY_MAP ( double, uint8_t )
TYPE3_BINARY_MAP ( double, uint16_t )
TYPE3_BINARY_MAP ( double, uint32_t )
TYPE3_BINARY_MAP ( double, uint64_t )

typedef void ( * type3_func ) ( const map_t*, void*, const void*, const void*, uint64_t );
static type3_func type3_funcs [ 6 ] [ 4 ] =
{
    /* uint8_t */
    {
        type3_uint8_t_to_uint8_t,
        type3_uint8_t_to_uint16_t,
        type3_uint8_t_to_uint32_t,
        type3_uint8_t_to_uint64_t
    },

    /* uint16_t */
    {
        type3_uint16_t_to_uint8_t,
        type3_uint16_t_to_uint16_t,
        type3_uint16_t_to_uint32_t,
        type3_uint16_t_to_uint64_t
    },

    /* uint32_t */
    {
        type3_uint32_t_to_uint8_t,
        type3_uint32_t_to_uint16_t,
        type3_uint32_t_to_uint32_t,
        type3_uint32_t_to_uint64_t
    },

    /* uint64_t */
    {
        type3_uint64_t_to_uint8_t,
        type3_uint64_t_to_uint16_t,
        type3_uint64_t_to_uint32_t,
        type3_uint64_t_to_uint64_t
    },

    /* float */
    {
        type3_float_to_uint8_t,
        type3_float_to_uint16_t,
        type3_float_to_uint32_t,
        type3_float_to_uint64_t
    },

    /* double */
    {
        type3_double_to_uint8_t,
        type3_double_to_uint16_t,
        type3_double_to_uint32_t,
        type3_double_to_uint64_t
    }
};


#if _DEBUGGING
static
rc_t CC type12_driver ( void *xself, const VXformInfo *info,
    void *dst, const void *src, uint64_t elem_count )
{
    map_t *self = xself;
    return ( * self -> array ) ( self, info, dst, src, elem_count );
}
#endif

static
rc_t CC type3_driver ( void *xself,
    const VXformInfo *info, int64_t row_id,
    const VFixedRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    const map_t *self = ( const void* ) xself;

    /* base pointers to pages */
    uint8_t *dst = rslt -> base;
    const uint8_t *tst = argv [ 0 ] . u . data . base;
    const uint8_t *src = argv [ 1 ] . u . data . base;

    /* offset by bytes to first element of row */
    dst += ( rslt -> elem_bits * rslt -> first_elem ) >> 3;
    tst += ( argv [ 0 ] . u . data . elem_bits * argv [ 0 ] . u . data . first_elem ) >> 3;
    src += ( argv [ 1 ] . u . data . elem_bits * argv [ 1 ] . u . data . first_elem ) >> 3;

    /* invoke function */
    ( * self -> row ) ( self, dst, src, tst, rslt -> elem_count );
    return 0;
}


/* make
 *  make a map_t structure
 */
#define MAKE_8BIT_MAP_T( DTYPE )                                        \
static                                                                  \
rc_t make_map_t_uint8_t_to_ ## DTYPE ( map_t **mapp,                    \
    const void *vfrom, const void *vto, uint32_t map_len )              \
{                                                                       \
    uint32_t i;                                                         \
    uint8_t *tst;                                                       \
    DTYPE *dto;                                                         \
    const uint8_t *from = vfrom;                                        \
    const DTYPE *sto = vto;                                             \
    map_t *map = calloc ( 1, sizeof * map +                             \
        ( sizeof * tst + sizeof * dto ) * 256 );                        \
    if ( map == NULL )                                                  \
    {                                                                   \
        return RC ( rcVDB, rcFunction, rcConstructing,                  \
            rcMemory, rcExhausted );                                    \
    }                                                                   \
                                                                        \
    tst = ( uint8_t* ) ( map + 1 );                                     \
    dto = ( DTYPE* ) & tst [ 256 ];                                     \
                                                                        \
    map -> row = NULL;                                                  \
    map -> map_len = map_len;                                           \
    map -> align = 0;                                                   \
    map -> from = ( void* ) tst;                                        \
    map -> to = ( void* ) dto;                                          \
    * mapp = map;                                                       \
                                                                        \
    for ( i = 0; i < map_len; ++ i )                                    \
    {                                                                   \
        int j = from [ i ];                                             \
        tst [ j ] = 1;                                                  \
        dto [ j ] = sto [ i ];                                          \
    }                                                                   \
                                                                        \
    return 0;                                                           \
}

MAKE_8BIT_MAP_T ( uint8_t )
MAKE_8BIT_MAP_T ( uint16_t )
MAKE_8BIT_MAP_T ( uint32_t )
MAKE_8BIT_MAP_T ( uint64_t )

#define MAP_T_CMP( TYPE )                                               \
static                                                                  \
int64_t CC map_t_cmp_ ## TYPE ( const void *a, const void *b, void *data )     \
{                                                                       \
    const TYPE *from = ( const TYPE* ) data;                            \
    int left = * ( const uint16_t* ) a;                                 \
    int right = * ( const uint16_t* ) b;                                \
    if ( from [ left ] < from [ right ] )                               \
        return -1;                                                      \
    return from [ left ] > from [ right ];                              \
}

MAP_T_CMP ( uint16_t )
MAP_T_CMP ( uint32_t )
MAP_T_CMP ( uint64_t )
MAP_T_CMP ( float )
MAP_T_CMP ( double )

#define MAKE_MAP_T( STYPE, DTYPE )                                      \
static                                                                  \
rc_t make_map_t_ ## STYPE ## _to_ ## DTYPE ( map_t **mapp,              \
    const void *vfrom, const void *vto, uint32_t map_len )              \
{                                                                       \
    uint32_t i;                                                         \
    uint16_t buff [ 64 ], *ord = buff;                                  \
                                                                        \
    DTYPE *dto;                                                         \
    STYPE *dfrom;                                                       \
                                                                        \
    const STYPE *sfrom = vfrom;                                         \
    const DTYPE *sto = vto;                                             \
    map_t *map = malloc ( sizeof * map +                                \
        ( sizeof * dfrom + sizeof * dto ) * map_len );                  \
    if ( map == NULL )                                                  \
    {                                                                   \
        return RC ( rcVDB, rcFunction, rcConstructing,                  \
            rcMemory, rcExhausted );                                    \
    }                                                                   \
                                                                        \
    dfrom = ( STYPE* ) ( map + 1 );                                     \
    dto = ( DTYPE* ) & dfrom [ map_len ];                               \
                                                                        \
    map -> row = NULL;                                                  \
    map -> map_len = map_len;                                           \
    map -> align = 0;                                                   \
    map -> from = ( void* ) dfrom;                                      \
    map -> to = ( void* ) dto;                                          \
    * mapp = map;                                                       \
                                                                        \
    if ( map_len > sizeof buff / sizeof buff [ 0 ] )                    \
    {                                                                   \
        assert ( map_len < 0x10000 );                                   \
        ord = malloc ( sizeof * ord * map_len );                        \
        if ( ord == NULL )                                              \
        {                                                               \
            free ( map );                                               \
            return RC ( rcVDB, rcFunction, rcConstructing,              \
                rcMemory, rcExhausted );                                \
        }                                                               \
    }                                                                   \
                                                                        \
    for ( i = 0; i < map_len; ++ i )                                    \
        ord [ i ] = ( uint16_t ) i;                                     \
    ksort ( ord, map_len, sizeof ord [ 0 ],                             \
        map_t_cmp_ ## STYPE, ( void* ) sfrom );                          \
                                                                        \
    for ( i = 0; i < map_len; ++ i )                                    \
    {                                                                   \
        int j = ord [ i ];                                              \
        dfrom [ i ] = sfrom [ j ];                                      \
        dto [ i ] = sto [ j ];                                          \
    }                                                                   \
                                                                        \
    if ( ord != buff )                                                  \
        free ( ord );                                                   \
                                                                        \
    return 0;                                                           \
}

MAKE_MAP_T ( uint16_t, uint8_t )
MAKE_MAP_T ( uint16_t, uint16_t )
MAKE_MAP_T ( uint16_t, uint32_t )
MAKE_MAP_T ( uint16_t, uint64_t )

MAKE_MAP_T ( uint32_t, uint8_t )
MAKE_MAP_T ( uint32_t, uint16_t )
MAKE_MAP_T ( uint32_t, uint32_t )
MAKE_MAP_T ( uint32_t, uint64_t )

MAKE_MAP_T ( uint64_t, uint8_t )
MAKE_MAP_T ( uint64_t, uint16_t )
MAKE_MAP_T ( uint64_t, uint32_t )
MAKE_MAP_T ( uint64_t, uint64_t )

MAKE_MAP_T ( float, uint8_t )
MAKE_MAP_T ( float, uint16_t )
MAKE_MAP_T ( float, uint32_t )
MAKE_MAP_T ( float, uint64_t )

MAKE_MAP_T ( double, uint8_t )
MAKE_MAP_T ( double, uint16_t )
MAKE_MAP_T ( double, uint32_t )
MAKE_MAP_T ( double, uint64_t )


typedef rc_t ( * make_func ) ( map_t**, const void*, const void*, uint32_t );
static make_func make_funcs [ 6 ] [ 4 ] =
{
    /* uint8_t */
    {
        make_map_t_uint8_t_to_uint8_t,
        make_map_t_uint8_t_to_uint16_t,
        make_map_t_uint8_t_to_uint32_t,
        make_map_t_uint8_t_to_uint64_t
    },

    /* uint16_t */
    {
        make_map_t_uint16_t_to_uint8_t,
        make_map_t_uint16_t_to_uint16_t,
        make_map_t_uint16_t_to_uint32_t,
        make_map_t_uint16_t_to_uint64_t
    },

    /* uint32_t */
    {
        make_map_t_uint32_t_to_uint8_t,
        make_map_t_uint32_t_to_uint16_t,
        make_map_t_uint32_t_to_uint32_t,
        make_map_t_uint32_t_to_uint64_t
    },

    /* uint64_t */
    {
        make_map_t_uint64_t_to_uint8_t,
        make_map_t_uint64_t_to_uint16_t,
        make_map_t_uint64_t_to_uint32_t,
        make_map_t_uint64_t_to_uint64_t
    },

    /* float */
    {
        make_map_t_float_to_uint8_t,
        make_map_t_float_to_uint16_t,
        make_map_t_float_to_uint32_t,
        make_map_t_float_to_uint64_t
    },

    /* double */
    {
        make_map_t_double_to_uint8_t,
        make_map_t_double_to_uint16_t,
        make_map_t_double_to_uint32_t,
        make_map_t_double_to_uint64_t
    }
};

/* domain_and_size_code
 *  converts sizes 8..64 into 0..3
 *  converts domains into integer or float
 *  combines the two into a single integer
 *  returns -1 if something is simply unsupported
 */
static
int domain_and_size_code ( const VTypedesc *desc )
{
    int size, type;
    switch ( desc -> intrinsic_bits )
    {
    case 8:
        size = 0;
        break;
    case 16:
        size = 1;
        break;
    case 32:
        size = 2;
        break;
    case 64:
        size = 3;
        break;
    default:
        return -1;
    }

    switch ( desc -> domain )
    {
    case vtdBool:
    case vtdUint:
    case vtdInt:
        type = 0;
        break;
    case vtdFloat:
        type = 4;
        break;
    case vtdAscii:
    case vtdUnicode:
        type = 0;
        break;
    default:
        return -1;
    }

    return type + size;
}


/* map
 *  translate input elements
 *  behaves much like the Unix "tr" command
 *  except that charsets are not [currently] supported
 *
 *  "A" [ TYPE ] - input data type, e.g. "ascii"
 *
 *  "B" [ TYPE ] - output data type, e.g. "ascii" or "U8"
 *
 *  "from" [ CONST ] - set of key values.
 *
 *  "to" [ CONST ] - set of mapped values,
 *  where length ( from ) === length ( to )
 *
 *  "in" [ DATA ] - input data to be matched against keys
 *  in "from". also serves as source data when "src" is omitted
 *
 *  "src" [ DATA, OPTIONAL ] - source data to be edited by
 *  substituting "to" values when corresponding "in" value
 *  matches key in "from". if omitted, "in" is used.
 *
 * USAGE:
 *  to upper case letters from a given alphabet
 *    ascii upper = < ascii, ascii > map < 'acgtn', 'ACGTN' > ( in );
 *
 *  to translate from ascii to binary
 *    U8 bin = < ascii, U8 > map < 'ACGTN', [ 0, 1, 2, 3, 0 ] > ( in );
 *
 *  to alter certain values of a column based upon values in another
 *    U8 n_encoded = < ascii, U8 > map < 'N', 0 > ( read, quality );
 *
 * CAVEATS:
 *  the full canonical mode of operation uses separate inputs
 *  for key matching and output source.
 *
 *  when a single input is specified:
 *   - sizeof ( A ) must equal sizeof ( B )
 *   - A must be a proper subset of B -OR-
 *   - "from" keys must match every possible "in" value ( total substitution )
 */
static
rc_t vdb_map_type1_8bit_fact ( VFuncDesc *rslt, const VFactoryParams *cp )
{
    rc_t rc = make_type1_uint8_t ( & rslt -> self,
        cp -> argv [ 0 ] . data . u8,
        cp -> argv [ 1 ] . data . u8,
        cp -> argv [ 0 ] . count );
    if ( rc != 0 )
        return rc;

    rslt -> whack = free;
    rslt -> u. af = type1_uint8_t;
    rslt -> variant = vftArray;

    return 0;
}

static
rc_t vdb_map_type1_fact ( VFuncDesc *rslt, const VFactoryParams *cp )
{
    rc_t rc;
    int code;
    map_t *self;

    /* look for special case of 8-bit to 8-bit mapping */
    if ( cp -> argv [ 0 ] . desc . intrinsic_bits == 8 )
        return vdb_map_type1_8bit_fact ( rslt, cp );

    /* determine the domain and size code */
    code = domain_and_size_code ( & cp -> argv [ 0 ] . desc );
    if ( code < 0 )
        return RC ( rcVDB, rcFunction, rcConstructing, rcType, rcUnsupported );

    /* create the object */
    rc = ( * make_funcs [ code ] [ code & 3 ] )
        ( & self,
          cp -> argv [ 0 ] . data . u8,
          cp -> argv [ 1 ] . data . u8,
          cp -> argv [ 0 ] . count );
    if ( rc != 0 )
        return rc;

    /* 8=>8 can be discounted */
    -- code;

    /* determine whether it makes sense to perform a binary search */
    rslt -> u. af = ( cp -> argv [ 0 ] . count < 8 ) ?
        type1_linear_funcs [ code ] : type1_binary_funcs [ code ];
    
    rslt -> self = self;
    rslt -> whack = free;
    rslt -> variant = vftArray;

#if _DEBUGGING
    self -> array = rslt -> u . af;
    rslt -> u . af = type12_driver;
#endif

    return 0;
}

static
rc_t vdb_map_type2_fact ( VFuncDesc *rslt, const VFactoryParams *cp )
{
    rc_t rc;
    map_t *self;
    int code1, code2;

    /* determine the domain and size codes */
    code1 = domain_and_size_code ( & cp -> argv [ 0 ] . desc );
    code2 = domain_and_size_code ( & cp -> argv [ 1 ] . desc );
    if ( code1 < 0 ||  code2 < 0 )
        return RC ( rcVDB, rcFunction, rcConstructing, rcType, rcUnsupported );

    /* create the object */
    rc = ( * make_funcs [ code1 ] [ code2 & 3 ] )
        ( & self,
          cp -> argv [ 0 ] . data . u8,
          cp -> argv [ 1 ] . data . u8,
          cp -> argv [ 0 ] . count );
    if ( rc != 0 )
        return rc;
    
    rslt -> self = self;
    rslt -> whack = free;
    rslt -> u. af = type2_funcs [ code1 ] [ code2 & 3 ];
    rslt -> variant = vftArray;

#if _DEBUGGING
    self -> array = rslt -> u . af;
    rslt -> u . af = type12_driver;
#endif

    return 0;
}

static
rc_t vdb_map_type3_fact ( VFuncDesc *rslt, const VFactoryParams *cp )
{
    rc_t rc;
    map_t *self;
    int code1, code2;

    /* determine the domain and size codes */
    code1 = domain_and_size_code ( & cp -> argv [ 0 ] . desc );
    code2 = domain_and_size_code ( & cp -> argv [ 1 ] . desc );
    if ( code1 < 0 ||  code2 < 0 )
        return RC ( rcVDB, rcFunction, rcConstructing, rcType, rcUnsupported );

    /* create the object */
    rc = ( * make_funcs [ code1 ] [ code2 & 3 ] )
        ( & self,
          cp -> argv [ 0 ] . data . u8,
          cp -> argv [ 1 ] . data . u8,
          cp -> argv [ 0 ] . count );
    if ( rc != 0 )
        return rc;

    self -> row = type3_funcs [ code1 ] [ code2 & 3 ];
    
    rslt -> self = self;
    rslt -> whack = free;
    rslt -> u. pf = type3_driver;
    rslt -> variant = vftFixedRow;

    return 0;
}

VTRANSFACT_IMPL ( vdb_map, 1, 0, 0 ) ( const void *ignore, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* the compiler has no way of knowing that the map dimensions must match */
    if ( cp -> argv [ 0 ] . count != cp -> argv [ 1 ] . count )
        return RC ( rcVDB, rcFunction, rcConstructing, rcConstraint, rcViolated );

    /* the loop type is determined by the number of inputs and input type */
    if ( dp -> argc == 2 )
        return vdb_map_type3_fact ( rslt, cp );

    /* since two schema types A and B are given, the compiler has no way
       of knowing that it should check for a relationship between them */
    if ( VTypedeclToTypedecl ( & dp -> argv [ 0 ] . fd . td, info -> schema, & info -> fdesc . fd . td, NULL, NULL ) )
        return vdb_map_type1_fact ( rslt, cp );

    return vdb_map_type2_fact ( rslt, cp );
}
