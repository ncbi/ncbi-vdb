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

#include "xform-priv.h"
#include "blob-priv.h"
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <arch-impl.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <assert.h>

/* copy
 *  just perform an addref on input blob
 */
static
rc_t CC copy ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    * rslt = ( VBlob* ) argv [ 0 ];
    (void)VBlobAddRef (*rslt );
    return 0;
}

/* casting object
 *  uses a three-step process for converting:
 *  1. read source into canonical fmt
 *  2. convert domains as required
 *  3. write from canonical fmt to dest
 */
typedef struct Cast Cast;
struct Cast
{
    /* read source to canonical fmt */
    union
    {
        int ( * num ) ( const void *src, void *dst );
        int ( * chr ) ( uint32_t *ch, const void *begin, const void *end );
    } read;

    /* transform from one domain to another */
    union
    {
        rc_t ( * num ) ( const void *src, void *dst );
    } convert;

    /* write from canonical fmt to destination */
    union
    {
        int ( * num ) ( const void *src,  void *dst );
        int ( * chr ) ( void *begin, void *end, uint32_t ch );
    } write;

    /* intrinsic dim of source */
    uint32_t intrinsic_dim;

    /* intrinsic sizes of src and dst */
    uint32_t src_bits, dst_bits;
};

static
rc_t CC cast_num ( void *data, const VXformInfo *info,
    void *dst, const void *src, uint64_t elem_count )
{
    const Cast *self = ( const void* ) data;

    /* since we are performing intrinsic-level operations,
       convert the array length into intrinsic count */
    uint32_t i;
    uint64_t count = elem_count * self -> intrinsic_dim;

    /* treat source and destination as byte pointers */
    int bytes;
    uint8_t *dp = dst;
    const uint8_t *sp = src;

    /* read and conversion buffers */
    union { int64_t i64; uint64_t u64; double f64; } buff [ 2 ];

    /* loop without conversion */
    if ( self -> convert . num == NULL )
    {
        for ( i = 0; i < count; ++ i )
        {
            /* read into canonical fmt */
            bytes = ( * self -> read . num ) ( sp, & buff [ 0 ] );
            sp += bytes;

            /* write into destination fmt */
            bytes = ( * self -> write . num ) ( & buff [ 0 ], dp );
            if ( bytes < 0 )
                return RC ( rcVDB, rcType, rcCasting, rcRange, rcExcessive );
            dp += bytes;
        }
    }

    /* loop with conversion */
    else
    {
        for ( i = 0; i < count; ++ i )
        {
            rc_t rc;

            /* read into canonical fmt */
            bytes = ( * self -> read . num ) ( sp, & buff [ 0 ] );
            sp += bytes;

            /* convert domains */
            rc = ( * self -> convert . num ) ( & buff [ 0 ], & buff [ 1 ] );
            if ( rc != 0 )
                return rc;

            /* write into destination fmt */
            bytes = ( * self -> write . num ) ( & buff [ 1 ], dp );
            if ( bytes < 0 )
                return RC ( rcVDB, rcType, rcCasting, rcRange, rcExcessive );
            dp += bytes;
        }
    }

    return 0;
}

static
rc_t resize_dst ( KDataBuffer *dst, uint8_t **dpos, uint8_t **dend,
    const uint8_t *sbase, const uint8_t *spos, const uint8_t *send )
{
    size_t offset = * dpos - ( uint8_t* ) dst -> base;
    double expand = ( double ) ( send - sbase ) / ( spos - sbase );
    rc_t rc = KDataBufferResize ( dst, ( uint32_t ) ( expand * offset ) + 32 );
    if ( rc == 0 )
    {
        * dpos = & ( ( uint8_t* ) dst -> base ) [ offset ];
        * dend = & ( ( uint8_t* ) dst -> base ) [ dst -> elem_count ];
    }
    return rc;
}

static
rc_t CC cast_chr ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    const Cast *self = ( const void* ) data;

    int bytes;
    uint32_t buff [ 2 ];
    uint8_t *dpos, *dend;
    const uint8_t *sbase, *spos, *send;

    /* determine source boundaries */
    sbase = argv [ 0 ] . u . data . base;
    sbase += (size_t)argv [ 0 ] . u . data . first_elem *
        argv [ 0 ] . u . data . elem_bits >> 3;
    send = & sbase [ (size_t)argv [ 0 ] . u . data . elem_count *
        argv [ 0 ] . u . data . elem_bits >> 3 ];
    spos = sbase;

    /* resize the destination blob */
    rc = KDataBufferCast ( rslt -> data, rslt -> data, 8, true );
    if ( rc == 0 )
        rc = KDataBufferResize ( rslt -> data, (uint32_t)( send - sbase ) );
    if ( rc != 0 )
        return rc;

    /* get the destination boundaries */
    dpos = rslt -> data -> base;
    dend = dpos + rslt -> data -> elem_count;
    assert ( rslt -> data -> bit_offset == 0 );

    while ( spos < send )
    {
        /* read into utf32 */
        bytes = ( * self -> read . chr ) ( & buff [ 0 ], spos, send );
        if ( bytes <= 0 )
            return RC ( rcVDB, rcType, rcCasting, rcData, rcCorrupt );
        spos += bytes;

        /* write into destination */
        bytes = ( * self -> write . chr ) ( dpos, dend, buff [ 0 ] );
        if ( bytes <= 0 )
        {
            if ( bytes < 0 )
                return RC ( rcVDB, rcType, rcCasting, rcData, rcCorrupt );
            rc = resize_dst ( rslt -> data, & dpos, & dend, sbase, spos, send );
            if ( rc != 0 )
                return rc;
        }
        dpos += bytes;
    }

    /* return the number of elements and reset the data buffer */
    rslt -> elem_count = ( uint32_t ) ( dpos - ( uint8_t* ) rslt -> data -> base );
    if ( rslt -> elem_bits > 8 )
        rslt -> elem_count /= rslt -> elem_bits >> 3;

    return 0;
}

/* read_num
 *  numeric readers convert size
 */
#define READ_NUM( S, D ) \
static int read_ ## S ( const void *src, void *dst ) \
{ * ( D* ) dst = * ( const S* ) src; return sizeof ( S ); }

READ_NUM ( int8_t, int64_t )
READ_NUM ( int16_t, int64_t )
READ_NUM ( int32_t, int64_t )
READ_NUM ( int64_t, int64_t )

READ_NUM ( uint8_t, uint64_t )
READ_NUM ( uint16_t, uint64_t )
READ_NUM ( uint32_t, uint64_t )
READ_NUM ( uint64_t, uint64_t )

READ_NUM ( float, double )
READ_NUM ( double, double )

typedef int ( * read_num_func ) ( const void*, void* );
static read_num_func read_num_tbl [] =
{
    read_uint8_t, read_uint16_t, read_uint32_t, read_uint64_t,
    read_int8_t, read_int16_t, read_int32_t, read_int64_t,
    NULL, NULL, read_float, read_double
};

/* write_num
 *  numeric writers convert size and check limit
 */
#define WRITE_NUM( S, D ) \
static int write_ ## D ( const void *src, void *dst ) \
{ * ( D* ) dst = * ( const S* ) src; return sizeof ( D ); }

#define WRITE_SNUM( S, D, PLIM, NLIM ) \
static int write_ ## D ( const void *src, void *dst ) \
{ \
    S s = * ( const S* ) src; \
    if ( s > PLIM || s < NLIM ) \
        return -1; \
    * ( D* ) dst = ( D )s; \
    return sizeof ( D ); \
}

#define WRITE_UNUM( S, D, LIM ) \
static int write_ ## D ( const void *src, void *dst ) \
{ \
    S s = * ( const S* ) src; \
    if ( s > LIM ) \
        return -1; \
    * ( D* ) dst = ( D )s; \
    return sizeof ( D ); \
}

WRITE_SNUM ( int64_t, int8_t, INT8_MAX, INT8_MIN )
WRITE_SNUM ( int64_t, int16_t, INT16_MAX, INT16_MIN )
WRITE_SNUM ( int64_t, int32_t, INT32_MAX, INT32_MIN )
WRITE_NUM ( int64_t, int64_t )

WRITE_UNUM ( uint64_t, uint8_t, UINT8_MAX )
WRITE_UNUM ( uint64_t, uint16_t, UINT16_MAX )
WRITE_UNUM ( uint64_t, uint32_t, UINT32_MAX )
WRITE_NUM ( uint64_t, uint64_t )

WRITE_SNUM ( double, float, FLT_MAX, FLT_MIN )
WRITE_NUM ( double, double )

typedef int ( * write_num_func ) ( const void*, void* );
static write_num_func write_num_tbl [] =
{
    write_uint8_t, write_uint16_t, write_uint32_t, write_uint64_t,
    write_int8_t, write_int16_t, write_int32_t, write_int64_t,
    NULL, NULL, write_float, write_double
};

/* convert_num
 */
static
rc_t convert_int ( const void *src, void *dst )
{
    if ( ( * ( int64_t* ) dst = * ( const int64_t* ) src ) < 0 )
        return RC ( rcVDB, rcType, rcCasting, rcRange, rcExcessive );
    return 0;
}

static
rc_t convert_int_float ( const void *src, void *dst )
{ * ( double* ) dst = * ( const int64_t* ) src; return 0; }

static
rc_t convert_uint_float ( const void *src, void *dst )
{ * ( double* ) dst = * ( const uint64_t* ) src; return 0; }

static
rc_t convert_float_int ( const void *src, void *dst )
{
    double d = * ( const double* ) src;
    if ( d > INT64_MAX || d < INT64_MIN )
        return RC ( rcVDB, rcType, rcCasting, rcRange, rcExcessive );
    * ( int64_t* ) dst = ( int64_t ) d;
    return 0;
}

static
rc_t convert_float_uint ( const void *src, void *dst )
{
    double d = * ( const double* ) src;
    if ( d > UINT64_MAX )
        return RC ( rcVDB, rcType, rcCasting, rcRange, rcExcessive );
    * ( uint64_t* ) dst = ( uint64_t ) d;
    return 0;
}

typedef rc_t ( * convert_num_func ) ( const void*, void* );
static convert_num_func convert_num_tbl [] =
{
    /* unsigned integer conversion */
    NULL, convert_int, convert_uint_float,
    /* signed integer conversion */
    convert_int, NULL, convert_int_float,
    /* floating point conversion */
    convert_float_int, convert_float_uint, NULL
};


/* read_chr
 *  characters readers convert size and encoding
 */
static int read_ascii ( uint32_t *ch,  const void *begin, const void *end )
{
    char s;
    if ( ( const char* ) begin >= ( const char* ) end )
        return 0;
    s = * ( const char* ) begin;
    if ( s < 0 )
        return -1;
    * ch = s;
    return 1;
}

#define read_utf8 \
    ( int ( * ) ( uint32_t*, const void*, const void* ) ) utf8_utf32

static int read_utf16 ( uint32_t *ch, const void *begin, const void *end )
{
    if ( ( const uint16_t* ) begin >= ( const uint16_t* ) end )
        return 0;
    /* yes, I know this is wrong - but don't blame me
       that the UNICODE folks can't produce a uni-code
       character with 16 bits... ( GO, KLINGON! ) */
    * ch = * ( const uint16_t* ) begin;
    return 2;
}

static int read_utf32 ( uint32_t *ch, const void *begin, const void *end )
{
    if ( ( const uint32_t* ) begin >= ( const uint32_t* ) end )
        return 0;
    * ch = * ( const uint32_t* ) begin;
    return 4;
}

typedef int ( * read_chr_func ) ( uint32_t*, const void*, const void* );
static read_chr_func read_chr_tbl [] =
{
    /* ascii reads */
    read_ascii, NULL, NULL,
    /* unicode reads */
    read_utf8, read_utf16, read_utf32
};

/* write_chr
 *  character writers convert size and encoding
 */
static int write_ascii ( void *begin, void *end, uint32_t ch )
{
    if ( ( char* ) begin >= ( char* ) end )
        return 0;
    * ( char* ) begin = ( ch >= 128 ) ? '.' : ( char ) ch;
    return 1;
}

#define write_utf8 \
    ( int ( * ) ( void*, void*, uint32_t ) ) utf32_utf8

static int write_utf16 ( void *begin, void *end, uint32_t ch )
{
    if ( ( uint16_t* ) begin >= ( uint16_t* ) end )
        return 0;
    if ( ch >= 0x10000 )
        return -1;
    * ( uint16_t* ) begin = ( uint16_t ) ch;
    return 2;
}

static int write_utf32 ( void *begin, void *end, uint32_t ch )
{
    if ( ( uint32_t* ) begin >= ( uint32_t* ) end )
        return 0;
    * ( uint32_t* ) begin = ch;
    return 4;
}

typedef int ( * write_chr_func ) ( void*, void*, uint32_t );
static write_chr_func write_chr_tbl [] =
{
    /* ascii writes */
    write_ascii, NULL, NULL,
    /* unicode writes */
    write_utf8, write_utf16, write_utf32
};


/* cast
 *  this operation is equivalent to a C forced cast, as it allows
 *  reinterpretation of data both in size and type, although
 *  it does not convert text to integer or vice-versa...
 */
static
rc_t make_self ( VFuncDesc *rslt, Cast **fself )
{
    * fself = calloc ( 1, sizeof ** fself );
    if ( * fself == NULL )
        return RC ( rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted );

    rslt -> self = * fself;
    rslt -> whack = free;
    return 0;
}

VTRANSFACT_BUILTIN_IMPL ( vdb_cast, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    Cast *fself;
    int32_t src_size_idx, dst_size_idx;

    /* cast cannot change intrinsic dimension. it is tempting to
       consider the ability to cast B1[8] to U8[1], but then what
       would it mean to cast B1[1] to U8[1] or B1[8] to U8[8] ? */
    if ( info -> fdesc . desc . intrinsic_dim != dp -> argv [ 0 ] . desc . intrinsic_dim )
        return RC ( rcVDB, rcFunction, rcConstructing, rcType, rcIncorrect );

    /* cast is often used to ensure a source has a given type
       meaning that in many cases no cast is required */
    if ( info -> fdesc . desc . domain == dp -> argv [ 0 ] . desc . domain )
    {
        if ( info -> fdesc . desc . intrinsic_bits == dp -> argv [ 0 ] . desc . intrinsic_bits )
        {
            /* input and output are already binary compatible */
            rslt -> variant = vftBlobN;
            VFUNCDESC_INTERNAL_FUNCS(rslt)->bfN = copy;
            return 0;
        }
    }

    src_size_idx = uint32_lsbit ( dp -> argv [ 0 ] . desc . intrinsic_bits );
    dst_size_idx = uint32_lsbit ( info -> fdesc . desc . intrinsic_bits );

    /* numeric <-> numeric casts are supported */
    if ( info -> fdesc . desc . domain >= vtdUint &&
         info -> fdesc . desc . domain <= vtdFloat &&
         dp -> argv [ 0 ] . desc . domain >= vtdUint &&
         dp -> argv [ 0 ] . desc . domain <= vtdFloat )
    {

        /* byte-aligned offsets will be values 3..6
           values < 3 are not byte-aligned
           values > 6 are >= 128 bits and not handled */
        if ( src_size_idx >= 3 && src_size_idx <= 6 &&
             dst_size_idx >= 3 && dst_size_idx <= 6 )
        {
            rc = make_self ( rslt, & fself );
            if ( rc != 0 )
                return rc;

            /* get reader */
            fself -> read . num = read_num_tbl
                [ ( dp -> argv [ 0 ] . desc . domain - vtdUint ) * 4 + src_size_idx - 3 ];
            if ( fself -> read . num != NULL )
            {
                /* get writer */
                fself -> write . num = write_num_tbl
                    [ ( info -> fdesc . desc . domain - vtdUint ) * 4 + dst_size_idx - 3 ];
                if ( fself -> write . num != NULL )
                {
                    /* get optional converter */
                    fself -> convert . num = convert_num_tbl
                        [ ( dp -> argv [ 0 ] . desc . domain - vtdUint ) * ( vtdFloat - vtdUint + 1 ) +
                          info -> fdesc . desc . domain - vtdUint ];

                    /* set intrinsic dimension */
                    fself -> intrinsic_dim = info -> fdesc . desc . intrinsic_dim;

                    /* done */
                    rslt -> u . af = cast_num;
                    rslt -> variant = vftArray;
                    return 0;
                }
            }

            free ( fself );
            rslt -> whack = NULL;
        }
    }

    /* text <-> text casts are supported */
    if ( info -> fdesc . desc . domain >= vtdAscii &&
         info -> fdesc . desc . domain <= vtdUnicode &&
         dp -> argv [ 0 ] . desc . domain >= vtdAscii &&
         dp -> argv [ 0 ] . desc . domain <= vtdUnicode )
    {
        if ( src_size_idx >= 3 && src_size_idx <= 5 &&
             dst_size_idx >= 3 && dst_size_idx <= 5 )
        {
            rc = make_self ( rslt, & fself );
            if ( rc != 0 )
                return rc;

            /* get reader */
            fself -> read . chr = read_chr_tbl
                [ ( dp -> argv [ 0 ] . desc . domain - vtdAscii ) * 3 + src_size_idx - 3 ];
            if ( fself -> read . chr != NULL )
            {
                /* get writer */
                fself -> write . chr = write_chr_tbl
                    [ ( info -> fdesc . desc . domain - vtdAscii ) * 4 + dst_size_idx - 3 ];
                if ( fself -> write . chr != NULL )
                {
                    /* set source and dest sizes */
                    fself -> src_bits = 1 << src_size_idx;
                    fself -> dst_bits = 1 << dst_size_idx;

                    /* done */
                    rslt -> u . rf = cast_chr;
                    rslt -> variant = vftRow;
                    return 0;
                }
            }

            free ( fself );
            rslt -> whack = NULL;
        }
    }

    /* everything else is unsupported */
    return RC ( rcVDB, rcFunction, rcConstructing, rcFunction, rcUnsupported );
}
