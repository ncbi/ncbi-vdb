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
#include <compiler.h>
#include <klib/sort.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <sysalloc.h>
#include <byteswap.h>
#include <os-native.h>

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

#define FTYPE double_t
typedef struct stats_t {
    FTYPE sy, syy, sxy;
    FTYPE sx, sxx;
    FTYPE fit;
    unsigned n;
    unsigned i;
    unsigned x;
} stats_t;

#if 0
static void debug_print_stats(const stats_t *st) {
    fprintf(stderr, "sy: %lf, syy: %lf, sxy: %lf, sx: %lf, sxx: %lf, n: %u\n",
        st->sy, st->syy, st->sxy, st->sx, st->sxx, st->n);
}

static void debug_print_data_u32(const uint32_t Y[], unsigned n) {
    unsigned i;

    for (i = 0; i != n; ++i) {
        fprintf(stderr, "%u\n", Y[i]);
    }
}

static void debug_print_data_u64(const uint64_t Y[], unsigned n) {
    unsigned i;

    for (i = 0; i != n; ++i) {
        fprintf(stderr, "%ju\n", Y[i]);
    }
}
#endif

static FTYPE fitness( const stats_t *st)
{
    double_t var_xy = st->sxy - st->sx * st->sy / (double_t)st->n;
    double_t var_x  = st->sxx - st->sx * st->sx / (double_t)st->n;
    double_t var_y  = st->syy - st->sy * st->sy / (double_t)st->n;
    
    if (st->n <= 2)
        return 1.0;
    if (var_y == 0.0)
        return 1.0;
    if (var_xy != 0.0) {
        if (var_x != 0.0) {
            double_t fit = (var_xy / var_x) * (var_xy / var_y);
            assert(-1.0 <= fit && fit <= 1.0);
            return fit;
        }
    }
    return 0.0;
}

static void regress_params(const stats_t *st, int64_t *dy, int64_t *dx, int64_t *a) {
    double_t beta = (st->sxy - st->sx * st->sy / st->n) / ((double_t)st->n * (st->n - 1) * (st->n + 1) / 12.0);
    double_t beta2;
    int sgn = 1;
    
    if (beta < 0) {
        beta = -beta;
        sgn = -1;
    }
    if (beta > 0 && beta < 1) {
        *dy = sgn * UINT16_MAX;
        *dx = (int64_t)( UINT16_MAX / beta );
    }
    else if (beta < (UINT64_MAX / UINT16_MAX)) {
        *dy = (int64_t)( sgn * beta * UINT16_MAX );
        *dx = UINT16_MAX;
    }
    else {
        *dy = (int64_t)( sgn * beta );
        *dx = 1;
    }
    beta2 = (double_t)*dy / *dx;
    *a = (int64_t)( (st->sy / st->n) - beta2 * (st->n - 1) / 2 );
    
    return;
}

typedef struct {
    unsigned size;
    unsigned used;
    void *buf;
} szbuf;

static void pack_nbuf8(nbuf *x) {
    unsigned i;
    const unsigned n = x->used;
    
    for (i = 0; i != n; ++i)
        x->data.u8[i] = (uint8_t)( x->data.raw[i] - x->min );
    x->var = 4;
}

static void pack_nbuf16(nbuf *x) {
    unsigned i;
    const unsigned n = x->used;
    
    for (i = 0; i != n; ++i)
        x->data.u16[i] = (uint16_t)( x->data.raw[i] - x->min );
    x->var = 3;
}

static void pack_nbuf32(nbuf *x) {
    unsigned i;
    const unsigned n = x->used;
    
    for (i = 0; i != n; ++i)
        x->data.u32[i] = (uint32_t)( x->data.raw[i] - x->min );
    x->var = 2;
}

static void pack_nbuf(nbuf *x) {
    if ((x->max - x->min) >> 8 == 0) {
        pack_nbuf8(x);
    }
    else if ((x->max - x->min) >> 16 == 0) {
        pack_nbuf16(x);
    }
    else if ((x->max - x->min) >> 32 == 0) {
        pack_nbuf32(x);
    }
    else {
        unsigned i;
        const unsigned n = x->used;
        
        for (i = 0; i != n; ++i)
            x->data.u64[i] = x->data.raw[i] - x->min;
        x->var = 1;
    }
}

static rc_t zlib_compress(szbuf *dst, const void *src, uint32_t ssize, int32_t strategy, int32_t level) {
    z_stream s;
    int zr;
    rc_t rc = 0;
    
    memset(&s, 0, sizeof(s));
    s.next_in = (void *)src;
    s.avail_in = ssize;
    s.next_out = dst->buf;
    s.avail_out = (uInt)dst->size;
    
    dst->used = 0;
    
    zr = deflateInit2(&s, level, Z_DEFLATED, -15, 9, strategy);
    switch (zr) {
    case 0:
        break;
    case Z_MEM_ERROR:
        return RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
    case Z_STREAM_ERROR:
        return RC(rcVDB, rcFunction, rcExecuting, rcParam, rcInvalid);
    default:
        return RC(rcVDB, rcFunction, rcExecuting, rcSelf, rcUnexpected);
    }
    zr = deflate(&s, Z_FINISH);
    switch (zr) {
    case Z_STREAM_END:
        break;
    case Z_OK:
        s.total_out = 0;
        break;
    default:
        rc = RC(rcVDB, rcFunction, rcExecuting, rcSelf, rcUnexpected);
        break;
    }
    zr = deflateEnd(&s);
    if ( zr != Z_OK && s.total_out != 0 )
        rc = RC(rcVDB, rcFunction, rcExecuting, rcSelf, rcUnexpected);
    if (rc == 0) {
        assert(s.total_out <= UINT32_MAX);
        dst->used = (uint32_t)s.total_out;
    }
    return rc;
}

static
void encode_types(uint8_t *packed, const uint8_t type[], unsigned n) {
    unsigned i;
    unsigned j;
    unsigned k;
    
    for (i = k = 0, j = 1; i != n; ++i) {
        if (type[i])
            k |= j;
        j <<= 1;
        if (j == 0x100) {
            j = 1;
            packed[i / 8] = k;
            k = 0;
        }
    }
    packed[i / 8] = k;
}

static rc_t encode_decoded(struct encoded *y, const struct decoded *x) {
    rc_t rc = 0;
    uint8_t *temp;
    unsigned temp_size;
    szbuf zbuf;
    
    memset(y, 0, sizeof(*y));
    y->flags = x->size_type << 2;
    y->data_count = x->data_count;
    y->u.izipped.segments = x->lines + x->outliers;
    y->u.izipped.outliers = x->outliers ? x->outlier->used : 0;
    
    y->u.izipped.min_diff = x->diff->min;
    y->u.izipped.min_length = x->length->min;
    y->u.izipped.min_dy = x->dy->min;
    y->u.izipped.min_dx = x->dx->min;
    y->u.izipped.min_a = x->a->min;
    y->u.izipped.min_outlier = x->outlier->min;
    
    if (x->outliers) {
        temp = malloc((temp_size = (x->lines + x->outliers + 7) / 8) + 1);
        if (temp == NULL)
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);

        encode_types(temp, x->type, x->lines + x->outliers);
        
        zbuf.buf = malloc(zbuf.size = temp_size);
        if (zbuf.buf == NULL)
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);

        zbuf.used = 0;
        rc = zlib_compress(&zbuf, temp, temp_size, Z_DEFAULT_STRATEGY, Z_BEST_SPEED);
        if (rc) {
            free(zbuf.buf);
            free(temp);
            return rc;
        }
        if (zbuf.used > 0 && zbuf.used < zbuf.size) {
            free(temp);
            SET_TYPE_ZIPPED(y->u.izipped);
            y->u.izipped.type = zbuf.buf;
            y->u.izipped.type_size = zbuf.used;
        }
        else {
            free(zbuf.buf);
            y->u.izipped.type = temp;
            y->u.izipped.type_size = temp_size;
        }
    }
    else {
        SET_TYPE_ABSENT(y->u.izipped);
        SET_OUTLIER_ABSENT(y->u.izipped);
        y->u.izipped.type_size = 0;
    }

    if (x->diff->min == x->diff->max) {
        SET_DIFF_CONSTANT(y->u.izipped);
        y->u.izipped.diff_size = x->diff->used;
    }
    else {
        zbuf.buf = malloc(zbuf.size = nbuf_size(x->diff));
        if (zbuf.buf == NULL)
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        zbuf.used = 0;
        
        rc = zlib_compress(&zbuf, &x->diff->data, zbuf.size, Z_FILTERED, Z_BEST_SPEED);
        if (rc) {
            free(zbuf.buf);
            return rc;
        }
        if (zbuf.used > 0 && zbuf.used < zbuf.size) {
            SET_DIFF_ZIPPED(y->u.izipped);
            y->u.izipped.diff = zbuf.buf;
            y->u.izipped.diff_size = zbuf.used;
        }
        else {
            y->u.izipped.diff = x->diff->data.u8;
            y->u.izipped.diff_size = zbuf.size;
            free(zbuf.buf);
        }
    }
    
    if (x->length->min == x->length->max) {
        SET_LENGTH_CONSTANT(y->u.izipped);
        y->u.izipped.length_size = x->length->used;
    }
    else {
        zbuf.buf = malloc(zbuf.size = nbuf_size(x->length));
        if (zbuf.buf == NULL)
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        zbuf.used = 0;
        
        rc = zlib_compress(&zbuf, &x->length->data, zbuf.size, Z_DEFAULT_STRATEGY, Z_BEST_SPEED);
        if (rc) {
            free(zbuf.buf);
            return rc;
        }
        if (zbuf.used > 0 && zbuf.used < zbuf.size) {
            SET_LENGTH_ZIPPED(y->u.izipped);
            y->u.izipped.length = zbuf.buf;
            y->u.izipped.length_size = zbuf.used;
        }
        else {
            y->u.izipped.length = x->length->data.u8;
            y->u.izipped.length_size = zbuf.size;
            free(zbuf.buf);
        }
    }
    
    if (x->dy->min == x->dy->max) {
        SET_DY_CONSTANT(y->u.izipped);
        y->u.izipped.dy_size = x->dy->used;
    }
    else {
        zbuf.buf = malloc(zbuf.size = nbuf_size(x->dy));
        if (zbuf.buf == NULL)
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        zbuf.used = 0;
        
        rc = zlib_compress(&zbuf, &x->dy->data, zbuf.size, Z_DEFAULT_STRATEGY, Z_BEST_SPEED);
        if (rc) {
            free(zbuf.buf);
            return rc;
        }
        if (zbuf.used > 0 && zbuf.used < zbuf.size) {
            SET_DY_ZIPPED(y->u.izipped);
            y->u.izipped.dy = zbuf.buf;
            y->u.izipped.dy_size = zbuf.used;
        }
        else {
            y->u.izipped.dy = x->dy->data.u8;
            y->u.izipped.dy_size = zbuf.size;
            free(zbuf.buf);
        }
    }
    
    if (x->dx->min == x->dx->max) {
        SET_DX_CONSTANT(y->u.izipped);
        y->u.izipped.dx_size = x->dx->used;
    }
    else {
        zbuf.buf = malloc(zbuf.size = nbuf_size(x->dx));
        if (zbuf.buf == NULL)
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        zbuf.used = 0;
        
        rc = zlib_compress(&zbuf, &x->dx->data, zbuf.size, Z_DEFAULT_STRATEGY, Z_BEST_SPEED);
        if (rc) {
            free(zbuf.buf);
            return rc;
        }
        if (zbuf.used > 0 && zbuf.used < zbuf.size) {
            SET_DX_ZIPPED(y->u.izipped);
            y->u.izipped.dx = zbuf.buf;
            y->u.izipped.dx_size = zbuf.used;
        }
        else {
            y->u.izipped.dx = x->dx->data.u8;
            y->u.izipped.dx_size = zbuf.size;
            free(zbuf.buf);
        }
    }
    
    if (x->a->min == x->a->max) {
        SET_A_CONSTANT(y->u.izipped);
        y->u.izipped.a_size = x->a->used;
    }
    else {
        zbuf.buf = malloc(zbuf.size = nbuf_size(x->a));
        if (zbuf.buf == NULL)
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        zbuf.used = 0;
        
        rc = zlib_compress(&zbuf, &x->a->data, zbuf.size, Z_DEFAULT_STRATEGY, Z_BEST_SPEED);
        if (rc) {
            free(zbuf.buf);
            return rc;
        }
        if (zbuf.used > 0 && zbuf.used < zbuf.size) {
            SET_A_ZIPPED(y->u.izipped);
            y->u.izipped.a = zbuf.buf;
            y->u.izipped.a_size = zbuf.used;
        }
        else {
            y->u.izipped.a = x->a->data.u8;
            y->u.izipped.a_size = zbuf.size;
            free(zbuf.buf);
        }
    }
    
    if (x->outliers != 0) {
        zbuf.buf = malloc(zbuf.size = nbuf_size(x->outlier));
        if (zbuf.buf == NULL)
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        zbuf.used = 0;
        
        rc = zlib_compress(&zbuf, &x->outlier->data, zbuf.size, Z_DEFAULT_STRATEGY, Z_BEST_SPEED);
        if (rc) {
            free(zbuf.buf);
            return rc;
        }
        if (zbuf.used > 0 && zbuf.used < zbuf.size) {
            SET_OUTLIER_ZIPPED(y->u.izipped);
            y->u.izipped.outlier = zbuf.buf;
            y->u.izipped.outlier_size = zbuf.used;
        }
        else {
            y->u.izipped.outlier = x->outlier->data.u8;
            y->u.izipped.outlier_size = zbuf.size;
            free(zbuf.buf);
        }
    }
    
    return 0;
}

static void free_encoded(const struct encoded *self) {
    switch (self->flags & 3) {
    case 0:
        if (FLAG_OUTLIER(self->u.izipped) == DATA_ZIPPED)
            free((void *)self->u.izipped.outlier);
        if (FLAG_A(self->u.izipped) == DATA_ZIPPED)
            free((void *)self->u.izipped.a);
        if (FLAG_DX(self->u.izipped) == DATA_ZIPPED)
            free((void *)self->u.izipped.dx);
        if (FLAG_DY(self->u.izipped) == DATA_ZIPPED)
            free((void *)self->u.izipped.dy);
        if (FLAG_LENGTH(self->u.izipped) == DATA_ZIPPED)
            free((void *)self->u.izipped.length);
        if (FLAG_DIFF(self->u.izipped) == DATA_ZIPPED)
            free((void *)self->u.izipped.diff);
        if (FLAG_TYPE(self->u.izipped) != DATA_ABSENT)
            free((void *)self->u.izipped.type);
        break;
    default:
        break;
    }
}

#define SERIALZE16(X) do { if (i + 2 > dsize) return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient); memmove(dst + i, &x->u.izipped.X, 2); i += 2; } while(0)
#define SERIALZE32(X) do { if (i + 4 > dsize) return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient); memmove(dst + i, &x->u.izipped.X, 4); i += 4; } while(0)
#define SERIALZE64(X) do { if (i + 8 > dsize) return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient); memmove(dst + i, &x->u.izipped.X, 8); i += 8; } while(0)

static
rc_t serialize_encoded(uint8_t *dst, unsigned dsize, unsigned *psize, const struct encoded *x) {
    unsigned i = 0;
   
    *psize = 0;

    if (i + 1 > dsize)
        return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);    
    dst[i] = x->flags; i += 1;
    
    if (i + 4 > dsize)
        return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);    
    memmove(dst + i, &x->data_count, 4); i += 4;

    switch (x->flags & 0x03) {
    case 3:
    case 2:
        if (i + 8 > dsize)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);    
        memmove(dst + i, &x->u.packed.min, 8); i += 8;
        /* fall thru */
    case 1:
        if (i + x->u.zipped.data_size > dsize)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);    
        memmove(dst + i, x->u.zipped.data, x->u.zipped.data_size); i += x->u.zipped.data_size;
        *psize = i;
        return 0;
    default:
        break;
    }
    
    SERIALZE32(data_flags);
    SERIALZE32(segments);
    SERIALZE32(outliers);
    SERIALZE32(type_size);
    SERIALZE32(diff_size);
    SERIALZE32(length_size);
    SERIALZE32(dy_size);
    SERIALZE32(dx_size);
    SERIALZE32(a_size);
    SERIALZE32(outlier_size);

    SERIALZE64(min_diff);
    SERIALZE64(min_length);
    SERIALZE64(min_dy);
    SERIALZE64(min_dx);
    SERIALZE64(min_a);
    SERIALZE64(min_outlier);
    
    if (FLAG_TYPE(x->u.izipped) != DATA_ABSENT && FLAG_TYPE(x->u.izipped) != DATA_CONSTANT) {
        if (i + x->u.izipped.type_size > dsize)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        memmove(dst + i, x->u.izipped.type, x->u.izipped.type_size); i += x->u.izipped.type_size;
    }
    
    if (FLAG_DIFF(x->u.izipped) != DATA_ABSENT && FLAG_DIFF(x->u.izipped) != DATA_CONSTANT) {
        if (i + x->u.izipped.diff_size > dsize)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        memmove(dst + i, x->u.izipped.diff, x->u.izipped.diff_size); i += x->u.izipped.diff_size;
    }
    
    if (FLAG_LENGTH(x->u.izipped) != DATA_ABSENT && FLAG_LENGTH(x->u.izipped) != DATA_CONSTANT) {
        if (i + x->u.izipped.length_size > dsize)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        memmove(dst + i, x->u.izipped.length, x->u.izipped.length_size); i += x->u.izipped.length_size;
    }
    
    if (FLAG_DY(x->u.izipped) != DATA_ABSENT && FLAG_DY(x->u.izipped) != DATA_CONSTANT) {
        if (i + x->u.izipped.dy_size > dsize)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        memmove(dst + i, x->u.izipped.dy, x->u.izipped.dy_size); i += x->u.izipped.dy_size;
    }
    
    if (FLAG_DX(x->u.izipped) != DATA_ABSENT && FLAG_DX(x->u.izipped) != DATA_CONSTANT) {
        if (i + x->u.izipped.dx_size > dsize)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        memmove(dst + i, x->u.izipped.dx, x->u.izipped.dx_size); i += x->u.izipped.dx_size;
    }
    
    if (FLAG_A(x->u.izipped) != DATA_ABSENT && FLAG_A(x->u.izipped) != DATA_CONSTANT) {
        if (i + x->u.izipped.a_size > dsize)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        memmove(dst + i, x->u.izipped.a, x->u.izipped.a_size); i += x->u.izipped.a_size;
    }
    
    if (FLAG_OUTLIER(x->u.izipped) != DATA_ABSENT && FLAG_OUTLIER(x->u.izipped) != DATA_CONSTANT) {
        if (i + x->u.izipped.outlier_size > dsize)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        memmove(dst + i, x->u.izipped.outlier, x->u.izipped.outlier_size); i += x->u.izipped.outlier_size;
    }
    
    *psize = i;
    return 0;
}

static int64_t CC comp_position(const void *A, const void *B, void * ignored) {
    const stats_t *a = A;
    const stats_t *b = B;
    
    return (int64_t)a->x - (int64_t)b->x;
}

static int64_t CC comp_fitness(const void *A, const void *B, void * ignored) {
    const stats_t **a = (const stats_t **)A;
    const stats_t **b = (const stats_t **)B;
    
    return (**a).fit < (**b).fit ?
        -1 : (**b).fit < (**a).fit ? 1 : (int64_t)(**a).i - (int64_t)(**b).i;
}

static void merge(stats_t *dst, const stats_t *L, const stats_t *R) {
    stats_t temp = *L;
    
    temp.sx  += R->sx;
    temp.sxx += R->sxx;
    temp.sy  += R->sy;
    temp.syy += R->syy;
    temp.sxy += R->sxy;
    temp.n   += R->n;
    temp.fit  = fitness( &temp );
    
    *dst = temp;
}

#define OUTLIER_LIMIT (12)
#define CHUNK_SIZE (16)
#define SANITY_CHECK 1

/* for signed operations */
#define ABS(X) ((uint64_t)(X >= 0 ? (X) : (-(X))))

#define STYPE int8_t
#define ANALYZE analyze_i8
#define ENCODE encode_i8
#include "izip-encode.impl.h"
#undef STYPE
#undef ANALYZE
#undef ENCODE

#define STYPE int16_t
#define ANALYZE analyze_i16
#define ENCODE encode_i16
#include "izip-encode.impl.h"
#undef STYPE
#undef ANALYZE
#undef ENCODE

#define STYPE int32_t
#define ANALYZE analyze_i32
#define ENCODE encode_i32
#include "izip-encode.impl.h"
#undef STYPE
#undef ANALYZE
#undef ENCODE

#define STYPE int64_t
#define ANALYZE analyze_i64
#define ENCODE encode_i64
#include "izip-encode.impl.h"
#undef STYPE
#undef ANALYZE
#undef ENCODE


/* for unsigned operations */
#undef ABS
#define ABS(X) ((uint64_t)(X))

#define STYPE uint8_t
#define ANALYZE analyze_u8
#define ENCODE encode_u8
#include "izip-encode.impl.h"
#undef STYPE
#undef ANALYZE
#undef ENCODE

#define STYPE uint16_t
#define ANALYZE analyze_u16
#define ENCODE encode_u16
#include "izip-encode.impl.h"
#undef STYPE
#undef ANALYZE
#undef ENCODE

#define STYPE uint32_t
#define ANALYZE analyze_u32
#define ENCODE encode_u32
#include "izip-encode.impl.h"
#undef STYPE
#undef ANALYZE
#undef ENCODE

#define STYPE uint64_t
#define ANALYZE analyze_u64
#define ENCODE encode_u64
#include "izip-encode.impl.h"
#undef STYPE
#undef ANALYZE
#undef ENCODE

typedef rc_t (*encode_f)(uint8_t *dst, unsigned dsize, unsigned *psize, const void *Y, unsigned N, int DUMP);

struct self_t {
    encode_f f;
};

static
struct self_t selfs[8] = {
    { (encode_f)encode_u8 },
    { (encode_f)encode_i8 },
    { (encode_f)encode_u16 },
    { (encode_f)encode_i16 },
    { (encode_f)encode_u32 },
    { (encode_f)encode_i32 },
    { (encode_f)encode_u64 },
    { (encode_f)encode_i64 },
};

rc_t ex_encode8( void *dst, unsigned dsize, uint8_t *src,
                 unsigned ssize_in_u8, unsigned *written )
{
    return selfs[0].f( dst, dsize, written, src, ssize_in_u8, 0 );
}

rc_t ex_encode16( void *dst, unsigned dsize, uint16_t *src,
                  unsigned ssize_in_u16, unsigned *written )
{
    return selfs[2].f( dst, dsize, written, src, ssize_in_u16, 0 );
}

rc_t ex_encode32( void *dst, unsigned dsize, uint32_t *src,
                  unsigned ssize_in_u32, unsigned *written )
{
    return selfs[4].f( dst, dsize, written, src, ssize_in_u32, 0 );
}

rc_t ex_encode64( void *dst, unsigned dsize, uint64_t *src,
                  unsigned ssize_in_u64, unsigned *written )
{
    return selfs[6].f( dst, dsize, written, src, ssize_in_u64, 0 );
}

#if 0
static
rc_t CC izip(
               void *Self,
               const VXformInfo *info,
               VBlobResult *dst,
               const VBlobData *src,
               VBlobHeader *hdr
) {
    rc_t rc = 0;
    const struct self_t *self = Self;
    uint32_t dsize;
    
    VBlobHeaderSetVersion(hdr, 0);

    assert(src->elem_count >> 32 == 0);
    assert(((dst->elem_count * dst->elem_bits + 7) >> 3) >> 32 == 0);
    dsize = (uint32_t)((dst->elem_count * dst->elem_bits + 7) >> 3);
    rc = self->f(dst->data, (uint32_t)dsize, &dsize, src->data, (uint32_t)src->elem_count, 0);
    if (dsize && rc == 0) {
        dst->byte_order = vboNative;
        dst->elem_bits = 1;
        dst->elem_count = dsize << 3;
        return 0;
    }
    return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
}

/* 
 function izip_fmt izip #1.0 ( izip_set in )
 */
VTRANSFACT_IMPL(vdb_izip, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    if (dp->argc != 1) {
#if _DEBUGGING
        fprintf(stderr, "dp->argc = %u != 1\n", dp->argc);
#endif
        return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
    }

    rslt->variant = vftBlob;
    rslt->u.bf = izip;

    switch (dp->argv[0].desc.domain) {
    case vtdInt:
        switch (dp->argv[0].desc.intrinsic_bits) {
        case 8:
            rslt->self = &selfs[1];
            break;
        case 16:
            rslt->self = &selfs[3];
            break;
        case 32:
            rslt->self = &selfs[5];
            break;
        case 64:
            rslt->self = &selfs[7];
            break;
        default:
#if _DEBUGGING
            fprintf(stderr, "intrinsic_bits = %u != (8|16|32|64)\n", dp->argv[0].desc.intrinsic_bits);
#endif
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            break;
        }
        break;
    case vtdUint:
        switch (dp->argv[0].desc.intrinsic_bits) {
        case 8:
            rslt->self = &selfs[0];
            break;
        case 16:
            rslt->self = &selfs[2];
            break;
        case 32:
            rslt->self = &selfs[4];
            break;
        case 64:
            rslt->self = &selfs[6];
            break;
        default:
#if _DEBUGGING
            fprintf(stderr, "intrinsic_bits = %u != (8|16|32|64)\n", dp->argv[0].desc.intrinsic_bits);
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
extern int test_decode(int32_t *, unsigned, const uint8_t *, unsigned, int);

int test_encode(const int32_t Y[], unsigned N) {
    uint8_t *dst;
    unsigned dsize;
    size_t temp;
    rc_t rc = 0;
    
    temp = N * sizeof(Y[0]);
    assert(temp >> 32 == 0);
    dst = malloc(dsize = (unsigned)temp);
    if (dst == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
    rc = encode_i32(dst, dsize, &dsize, Y, N, 0);

    {
        int32_t *X;
        
        X = malloc(N * sizeof(Y[0]));
        if (X) {
            rc = test_decode(X, N, dst, dsize, 0);
            if (rc == 0) {
                rc = memcmp(Y, X, N * sizeof(Y[0])) == 0 ? 0 : RC(rcXF, rcFunction, rcExecuting, rcFunction, rcInvalid);
            }
            free(X);
        }
    }

    free(dst);
    if (rc)
        fprintf(stdout, "test failed!\n");
    return rc;
}
#endif
