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
#include <klib/rc.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <sysalloc.h>
#include <byteswap.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <math.h>

#include <zlib.h>

#include <stdio.h>
#include <assert.h>

#include "izip-common.h"

static void unpack_nbuf16_swap(nbuf *x) {
    unsigned i;
    
    for (i = x->used; i; --i) {
        x->data.raw[i - 1] = bswap_16(x->data.u16[i - 1]) + x->min;
    }
}

static void unpack_nbuf32_swap(nbuf *x) {
    unsigned i;
    
    for (i = x->used; i; --i) {
        x->data.raw[i - 1] = bswap_32(x->data.u32[i - 1]) + x->min;
    }
}

static void unpack_nbuf8(nbuf *x) {
    unsigned i;
    
    for (i = x->used; i; --i) {
        x->data.raw[i - 1] = x->data.u8[i - 1] + x->min;
    }
}

static void unpack_nbuf16(nbuf *x) {
    unsigned i;
    
    for (i = x->used; i; --i) {
        x->data.raw[i - 1] = x->data.u16[i - 1] + x->min;
    }
}

static void unpack_nbuf32(nbuf *x) {
    unsigned i;
    
    for (i = x->used; i; --i) {
        x->data.raw[i - 1] = x->data.u32[i - 1] + x->min;
    }
}

static void unpack_nbuf(nbuf *x) {
    unsigned i;
    
    switch (x->var) {
    case 4:
        unpack_nbuf8(x);
        break;
    case 3:
        unpack_nbuf16(x);
        break;
    case 2:
        unpack_nbuf32(x);
        break;
    default:
        for (i = x->used; i; --i) {
            x->data.raw[i - 1] = x->data.u64[i - 1] + x->min;
        }
        break;
    }
    x->var = 0;
}

static void unpack_nbuf_swap(nbuf *x) {
    unsigned i;
    
    switch (x->var) {
    case 4:
        unpack_nbuf8(x);
        break;
    case 3:
        unpack_nbuf16_swap(x);
        break;
    case 2:
        unpack_nbuf32_swap(x);
        break;
    default:
        for (i = x->used; ; --i) {
            x->data.raw[i - 1] = bswap_64(x->data.u64[i - 1]) + x->min;
            if (i == 0)
                break;
        }
        break;
    }
    x->var = 0;
}

static rc_t zlib_decompress(void *dst, unsigned dsize, unsigned *psize, const void *src, uint32_t ssize) {
    z_stream s;
    int zr;
    rc_t rc = 0;
    
    memset(&s, 0, sizeof(s));
    s.next_in = (void *)src;
    s.avail_in = ssize;
    s.next_out = dst;
    s.avail_out = dsize;
    
    zr = inflateInit2(&s, -15);
    switch (zr) {
    case 0:
        break;
    case Z_MEM_ERROR:
        return RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
    default:
        return RC(rcVDB, rcFunction, rcExecuting, rcNoObj, rcUnexpected);
    }
    zr = inflate(&s, Z_FINISH);
    switch (zr) {
    case Z_STREAM_END:
        assert(s.total_out <= UINT32_MAX);
        *psize = (uint32_t)s.total_out;
        rc = 0;
        break;
    case Z_OK:
        rc = RC(rcXF, rcFunction, rcExecuting, rcMemory, rcInsufficient);
        break;
    case Z_BUF_ERROR:
    case Z_NEED_DICT:
    case Z_DATA_ERROR:
        rc = RC(rcVDB, rcFunction, rcExecuting, rcData, rcCorrupt);
        break;
    case Z_MEM_ERROR:
        rc = RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        break;
    default:
        rc = RC(rcVDB, rcFunction, rcExecuting, rcNoObj, rcUnexpected);
        break;
    }
    zr = inflateEnd(&s);
    switch (zr) {
    case Z_OK:
        return rc;
    default:
        if (rc == 0)
            {
            return RC(rcVDB, rcFunction, rcExecuting, rcData, rcCorrupt);
            }
        return rc;
    }
    return 0;
}

static
void decode_types(uint8_t dst[], unsigned n, const uint8_t src[])
{
    unsigned i, j, k = 0;
    
    for (i = 0, j = 1; i != n; ++i)
    {
        if (j == 1)
            k = src[i / 8];

        dst[i] = (k & j) == 0 ? 0 : 1;

        j <<= 1;
        if (j == 0x100)
            j = 1;
    }
}

#define DESERIAL16(X) do { if (i + 2 > n) return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient); memmove(&y->u.izipped.X, &src[i], 2); i += 2; if (swap) y->u.izipped.X = bswap_16(y->u.izipped.X); } while (0)
#define DESERIAL32(X) do { if (i + 4 > n) return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient); memmove(&y->u.izipped.X, &src[i], 4); i += 4; if (swap) y->u.izipped.X = bswap_32(y->u.izipped.X); } while (0)
#define DESERIAL64(X) do { if (i + 8 > n) return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient); memmove(&y->u.izipped.X, &src[i], 8); i += 8; if (swap) y->u.izipped.X = bswap_64(y->u.izipped.X); } while (0)

static rc_t deserialize_encoded(struct encoded *y, const uint8_t src[], unsigned n, int swap) {
    unsigned i = 0;
    
    memset(y, 0, sizeof(*y));
    
    if (i + 1 > n)
        return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
    y->flags = src[i]; ++i;
    
    if (i + 4 > n)
        return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
    memmove(&y->data_count, &src[i], 4); i += 4;
    if (swap)
        y->data_count = bswap_32(y->data_count);
    
    switch (y->flags & 0x03) {
    case 2:
    case 3:
        if (i + 8 > n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
        memmove(&y->u.packed.min, &src[i], 8); i += 8;
        if (swap)
            y->u.packed.min = bswap_64(y->u.packed.min);
    case 1:
        y->u.zipped.data_size = n - i;
        y->u.zipped.data = &src[i];
        return 0;
    default:
        break;
    }
    
    DESERIAL32(data_flags);
    DESERIAL32(segments);
    DESERIAL32(outliers);
    
    DESERIAL32(type_size);
    DESERIAL32(diff_size);
    DESERIAL32(length_size);
    DESERIAL32(dy_size);
    DESERIAL32(dx_size);
    DESERIAL32(a_size);
    DESERIAL32(outlier_size);
    
    DESERIAL64(min_diff);
    DESERIAL64(min_length);
    DESERIAL64(min_dy);
    DESERIAL64(min_dx);
    DESERIAL64(min_a);
    DESERIAL64(min_outlier);
    
    if (FLAG_TYPE(y->u.izipped) != DATA_ABSENT && FLAG_TYPE(y->u.izipped) != DATA_CONSTANT) {
        if (i + y->u.izipped.type_size > n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
        y->u.izipped.type = &src[i]; i += y->u.izipped.type_size;
    }
    
    if (FLAG_DIFF(y->u.izipped) != DATA_ABSENT && FLAG_DIFF(y->u.izipped) != DATA_CONSTANT) {
        if (i + y->u.izipped.diff_size > n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
        y->u.izipped.diff = &src[i]; i += y->u.izipped.diff_size;
    }
    
    if (FLAG_LENGTH(y->u.izipped) != DATA_ABSENT && FLAG_LENGTH(y->u.izipped) != DATA_CONSTANT) {
        if (i + y->u.izipped.length_size > n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
        y->u.izipped.length = &src[i]; i += y->u.izipped.length_size;
    }
    
    if (FLAG_DY(y->u.izipped) != DATA_ABSENT && FLAG_DY(y->u.izipped) != DATA_CONSTANT) {
        if (i + y->u.izipped.dy_size > n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
        y->u.izipped.dy = &src[i]; i += y->u.izipped.dy_size;
    }
    
    if (FLAG_DX(y->u.izipped) != DATA_ABSENT && FLAG_DX(y->u.izipped) != DATA_CONSTANT) {
        if (i + y->u.izipped.dx_size > n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
        y->u.izipped.dx = &src[i]; i += y->u.izipped.dx_size;
    }
    
    if (FLAG_A(y->u.izipped) != DATA_ABSENT && FLAG_A(y->u.izipped) != DATA_CONSTANT) {
        if (i + y->u.izipped.a_size > n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
        y->u.izipped.a = &src[i]; i += y->u.izipped.a_size;
    }
    
    if (FLAG_OUTLIER(y->u.izipped) != DATA_ABSENT && FLAG_OUTLIER(y->u.izipped) != DATA_CONSTANT) {
        if (i + y->u.izipped.outlier_size > n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
        y->u.izipped.outlier = &src[i];
        /* i += y->u.izipped.outlier_size; */
    }
    
    return 0;
}

#define BITS_TO_VARIANT(X, BITS) do { switch(BITS) { case 8: (X)->var = 4; break; case 16: (X)->var = 3; break; case 32: (X)->var = 2; break; case 64: (X)->var = 1; break; default: return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid); } } while (0);

static rc_t decode_encoded(struct decoded *y, const struct encoded *x) {
    unsigned i;
    unsigned type = x->flags & 0x3;
    rc_t rc;
    const uint8_t *src;
    uint8_t *hsrc;
    unsigned elem_bits;
    unsigned n = 0;
    
    memset(y, 0, sizeof(*y));
    y->data_count = x->data_count;
    y->size_type = (x->flags >> 2) & 3;
    
    y->diff = alloc_raw_nbuf(y->data_count);
    if (y->diff == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
    
    if (type) {
        if ((type & 1) != 0)  {
            rc = zlib_decompress(y->diff->data.u8, 8 * y->data_count, &n, x->u.zipped.data, x->u.zipped.data_size );
            if (rc)
                return rc;
            if ((type & 2) != 0)
                y->diff->min = x->u.packed.min;
        }
        else {
            y->diff->min = x->u.packed.min;
            memmove(y->diff->data.u8, x->u.packed.data, n = x->u.packed.data_size);
        }
        elem_bits = n * 8 / x->data_count;
        if (elem_bits * x->data_count / 8 != n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
        BITS_TO_VARIANT(y->diff, elem_bits);
        if (type == 1 && 4 - y->diff->var != y->size_type) {
#if _DEBUGGING
            fprintf(stderr, "decode_encoded: var = %i, size_type = %u\n", (int)y->diff->var, (unsigned)y->size_type);
#endif
            return RC(rcXF, rcFunction, rcExecuting, rcRange, rcExcessive);
        }
        y->diff->used = x->data_count;
        return 0;
    }
    
    y->type = malloc(x->u.izipped.segments);
    if (y->type == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
    
    if (x->u.izipped.outliers) {
        hsrc = NULL;
        
        if (FLAG_TYPE(x->u.izipped) == DATA_ZIPPED) {
            hsrc = malloc(x->u.izipped.segments);
            if (hsrc == NULL)
                return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
            rc = zlib_decompress(hsrc, x->u.izipped.segments, &i, x->u.izipped.type, x->u.izipped.type_size);
            if (rc) {
                free(hsrc);
                return rc;
            }
            src = hsrc;
        }
        else
            src = x->u.izipped.type;
        decode_types(y->type, x->u.izipped.segments, src);
        if (hsrc) free(hsrc);
        for (n = i = 0; i != x->u.izipped.segments; ++i) {
            if (y->type[i])
                ++n;
        }
        y->lines = x->u.izipped.segments - n;
        y->outliers = n;
    }
    else {
        memset(y->type, 0, x->u.izipped.segments);
        y->lines = x->u.izipped.segments;
        y->outliers = 0;
    }
    
    y->diff->min = x->u.izipped.min_diff;
    if (FLAG_DIFF(x->u.izipped) == DATA_CONSTANT) {
        y->diff->used = x->u.izipped.diff_size;
        memset(y->diff->data.raw, 0, nbuf_size(y->diff));
    }
    else {
        if (FLAG_DIFF(x->u.izipped) == DATA_ZIPPED) {
            rc = zlib_decompress(y->diff->data.u8, y->diff->size * 8, &n, x->u.izipped.diff, x->u.izipped.diff_size);
            if (rc)
                return rc;
        }
        else {
            n = x->u.izipped.diff_size;
            memmove(y->diff->data.u8, x->u.izipped.diff, n);
        }

        if ( y->diff->size != 0 )
            elem_bits = (n * 8) / y->diff->size;
        else
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);

        if (elem_bits * y->diff->size / 8 != n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
        BITS_TO_VARIANT(y->diff, elem_bits);
        y->diff->used = n >> (4 - y->diff->var);
    }
    
    y->length = alloc_nbuf(y->lines + y->outliers, 2);
    if (y->length == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
    
    y->length->min = x->u.izipped.min_length;
    if (FLAG_LENGTH(x->u.izipped) == DATA_CONSTANT) {
        y->length->used = y->lines + y->outliers;
        memset(y->length->data.raw, 0, nbuf_size(y->length));
    }
    else {
        if (FLAG_LENGTH(x->u.izipped) == DATA_ZIPPED) {
            rc = zlib_decompress(y->length->data.u8, y->length->size * 4, &n, x->u.izipped.length, x->u.izipped.length_size);
            if (rc)
                return rc;
        }
        else {
            n = x->u.izipped.length_size;
            memmove(y->length->data.u8, x->u.izipped.length, n);
        }
        elem_bits = (n * 8) / (y->lines + y->outliers);
        if (elem_bits * (y->lines + y->outliers) / 8 != n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
        BITS_TO_VARIANT(y->length, elem_bits);
        y->length->used = n >> (4 - y->length->var);
    }
    
    y->dy = alloc_nbuf(y->lines, 1);
    if (y->dy == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
    
    y->dy->min = x->u.izipped.min_dy;
    if (FLAG_DY(x->u.izipped) == DATA_CONSTANT) {
        y->dy->used = y->lines;
        memset(y->dy->data.raw, 0, nbuf_size(y->dy));
    }
    else {
        if (FLAG_DY(x->u.izipped) == DATA_ZIPPED) {
            rc = zlib_decompress(y->dy->data.u8, y->dy->size * 8, &n, x->u.izipped.dy, x->u.izipped.dy_size);
            if (rc)
                return rc;
        }
        else {
            n = x->u.izipped.dy_size;
            memmove(y->dy->data.u8, x->u.izipped.dy, n);
        }
        elem_bits = (n * 8) / y->lines;
        if (elem_bits * y->lines / 8 != n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
        BITS_TO_VARIANT(y->dy, elem_bits);
        y->dy->used = n >> (4 - y->dy->var);
    }
    
    y->dx = alloc_nbuf(y->lines, 1);
    if (y->dx == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
    
    y->dx->min = x->u.izipped.min_dx;
    if (FLAG_DX(x->u.izipped) == DATA_CONSTANT) {
        y->dx->used = y->lines;
        memset(y->dx->data.raw, 0, nbuf_size(y->dx));
    }
    else {
        if (FLAG_DX(x->u.izipped) == DATA_ZIPPED) {
            rc = zlib_decompress(y->dx->data.u8, y->dx->size * 8, &n, x->u.izipped.dx, x->u.izipped.dx_size);
            if (rc)
                return rc;
        }
        else {
            n = x->u.izipped.dx_size;
            memmove(y->dx->data.u8, x->u.izipped.dx, n);
        }
        elem_bits = (n * 8) / y->lines;
        if (elem_bits * y->lines / 8 != n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
        BITS_TO_VARIANT(y->dx, elem_bits);
        y->dx->used = n >> (4 - y->dx->var);
    }
    
    y->a = alloc_nbuf(y->lines, 1);
    if (y->a == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
    
    y->a->min = x->u.izipped.min_a;
    if (FLAG_A(x->u.izipped) == DATA_CONSTANT) {
        y->a->used = y->lines;
        memset(y->a->data.raw, 0, nbuf_size(y->a));
    }
    else {
        if (FLAG_A(x->u.izipped) == DATA_ZIPPED) {
            rc = zlib_decompress(y->a->data.u8, y->a->size * 8, &n, x->u.izipped.a, x->u.izipped.a_size);
            if (rc)
                return rc;
        }
        else {
            n = x->u.izipped.a_size;
            memmove(y->a->data.u8, x->u.izipped.a, n);
        }
        elem_bits = (n * 8) / y->lines;
        if (elem_bits * y->lines / 8 != n)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
        BITS_TO_VARIANT(y->a, elem_bits);
        y->a->used = n >> (4 - y->a->var);
    }
    
    if (y->outliers) {
        y->outlier = alloc_nbuf(x->u.izipped.outliers, 1);
        if (y->outlier == NULL)
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        
        y->outlier->min = x->u.izipped.min_outlier;
        if (FLAG_OUTLIER(x->u.izipped) == DATA_CONSTANT) {
            y->outlier->used = y->outliers;
            memset(y->outlier->data.raw, 0, nbuf_size(y->outlier));
        }
        else {
            if (FLAG_OUTLIER(x->u.izipped) == DATA_ZIPPED) {
                rc = zlib_decompress(y->outlier->data.u8, y->outlier->size * 8, &n, x->u.izipped.outlier, x->u.izipped.outlier_size);
                if (rc)
                    return rc;
            }
            else {
                n = x->u.izipped.outlier_size;
                memmove(y->outlier->data.u8, x->u.izipped.outlier, n);
            }
            elem_bits = (n * 8) / x->u.izipped.outliers;
            if (elem_bits * x->u.izipped.outliers / 8 != n)
                return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
            BITS_TO_VARIANT(y->outlier, elem_bits);
            y->outlier->used = n >> (4 - y->outlier->var);
        }
    }
    
    return 0;
}

#define STYPE uint8_t
#define DECODE decode_u8
#include "izip-decode.impl.h"
#undef STYPE
#undef DECODE

#define STYPE int8_t
#define DECODE decode_i8
#include "izip-decode.impl.h"
#undef STYPE
#undef DECODE

#define STYPE uint16_t
#define DECODE decode_u16
#include "izip-decode.impl.h"
#undef STYPE
#undef DECODE

#define STYPE int16_t
#define DECODE decode_i16
#include "izip-decode.impl.h"
#undef STYPE
#undef DECODE

#define STYPE uint32_t
#define DECODE decode_u32
#include "izip-decode.impl.h"
#undef STYPE
#undef DECODE

#define STYPE int32_t
#define DECODE decode_i32
#include "izip-decode.impl.h"
#undef STYPE
#undef DECODE

#define STYPE uint64_t
#define DECODE decode_u64
#include "izip-decode.impl.h"
#undef STYPE
#undef DECODE

#define STYPE int64_t
#define DECODE decode_i64
#include "izip-decode.impl.h"
#undef STYPE
#undef DECODE

typedef rc_t (*decode_f)(void *dst, unsigned dsize, const uint8_t src[], unsigned N, int swap);

static
decode_f selfs[8] = {
    (decode_f)decode_u8,
    (decode_f)decode_i8,
    (decode_f)decode_u16,
    (decode_f)decode_i16,
    (decode_f)decode_u32,
    (decode_f)decode_i32,
    (decode_f)decode_u64,
    (decode_f)decode_i64,
};

#if 0
rc_t ex_decode8( void *dst, unsigned dsize, uint8_t *src, unsigned ssize )
{
    return selfs[0].f( dst, dsize, src, ssize, 0 );
}

rc_t ex_decode16( void *dst, unsigned dsize, uint8_t *src, unsigned ssize )
{
    return selfs[2].f( dst, dsize, src, ssize, 0 );
}

rc_t ex_decode32( void *dst, unsigned dsize, uint8_t *src, unsigned ssize )
{
    return selfs[4].f( dst, dsize, src, ssize, 0 );
}

rc_t ex_decode64( void *dst, unsigned dsize, uint8_t *src, unsigned ssize )
{
    return selfs[6].f( dst, dsize, src, ssize, 0 );
}
#endif

rc_t CC iunzip_func_v0(
                    void *Self,
                    const VXformInfo *info,
                    VBlobResult *dst,
                    const VBlobData *src
) {
    uint32_t ssize;
    
    assert(dst->elem_count >> 32 == 0);
    assert(((src->elem_count * src->elem_bits + 7) >> 3) >> 32 == 0);
    ssize = (uint32_t)((src->elem_count * src->elem_bits + 7) >> 3);
    
    dst->byte_order = vboNative;
    switch (src->byte_order) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    case vboBigEndian:
#else
    case vboBigEndian:
#endif
        return selfs[(uintptr_t)Self](dst->data, (uint32_t)dst->elem_count,
                       src->data, ssize,
                       1);
    default:
        return selfs[(uintptr_t)Self](dst->data, (uint32_t)dst->elem_count,
                       src->data, ssize,
                       0);
    }
}

#if 0

static
rc_t CC iunzip(
            void *Self,
            const VXformInfo *info,
            VBlobResult *dst,
            const VBlobData *src,
            VBlobHeader *hdr
) {
    switch (VBlobHeaderVersion(hdr)) {
    case 0:
        return iunzip_func_v0(Self, info, dst, src);
    default:
        return RC(rcVDB, rcFunction, rcExecuting, rcParam, rcBadVersion);
    }
}

/* 
 function izip_set iunzip #1.0 ( izip_fmt in )
 */
VTRANSFACT_IMPL(vdb_iunzip, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftBlob;
    rslt->u.bf = iunzip;
    
    switch (info->fdesc.desc.domain) {
    case vtdInt:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
            rslt->self = (void *)1;
            break;
        case 16:
            rslt->self = (void *)3;
            break;
        case 32:
            rslt->self = (void *)5;
            break;
        case 64:
            rslt->self = (void *)7;
            break;
        default:
#if _DEBUGGING
            fprintf(stderr, "intrinsic_bits = %u != (8|16|32|64)\n", info->fdesc.desc.intrinsic_bits);
#endif
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            break;
        }
        break;
    case vtdUint:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
            rslt->self = (void *)0;
            break;
        case 16:
            rslt->self = (void *)2;
            break;
        case 32:
            rslt->self = (void *)4;
            break;
        case 64:
            rslt->self = (void *)6;
            break;
        default:
#if _DEBUGGING
            fprintf(stderr, "intrinsic_bits = %u != (8|16|32|64)\n", info->fdesc.desc.intrinsic_bits);
#endif
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            break;
        }
        break;
    default:
#if _DEBUGGING
        fprintf(stderr, "domain != vtdInt or vtdUint\n");
#endif
        return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
    }
    return 0;
}
#endif

#if TESTING
int test_decode(int32_t *dst, unsigned dsize, const uint8_t *src, unsigned ssize, int swap) {
    return decode_i32(dst, dsize, src, ssize, 0);
}
#endif
