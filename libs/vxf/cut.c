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
#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

typedef struct self_t {
    void (*f)(const struct self_t *, void *, const void *, size_t);
    uint32_t type_size;
    uint32_t dim;
    uint32_t n;
    uint32_t idx[7];
} self_t;

static
void cut8(const struct self_t *self, void *Dst, const void *Src, size_t elem_count) {
    uint8_t *dst = Dst;
    const uint8_t *src = Src;
    int i;
    int j;
    int doff;
    int soff;
    
    for (doff = soff = i = 0; i != elem_count; ++i, doff += self->n, soff += self->dim) {
        for (j = 0; j != self->n; ++j) {
            dst[doff + j] = src[soff + self->idx[j]];
        }
    }
}

static
void cut16(const struct self_t *self, void *Dst, const void *Src, size_t elem_count) {
    uint16_t *dst = Dst;
    const uint16_t *src = Src;
    int i;
    int j;
    int doff;
    int soff;
    
    for (doff = soff = i = 0; i != elem_count; ++i, doff += self->n, soff += self->dim) {
        for (j = 0; j != self->n; ++j) {
            dst[doff + j] = src[soff + self->idx[j]];
        }
    }
}

static
void cut32(const struct self_t *self, void *Dst, const void *Src, size_t elem_count) {
    uint32_t *dst = Dst;
    const uint32_t *src = Src;
    int i;
    int j;
    int doff;
    int soff;
    
    for (doff = soff = i = 0; i != elem_count; ++i, doff += self->n, soff += self->dim) {
        for (j = 0; j != self->n; ++j) {
            dst[doff + j] = src[soff + self->idx[j]];
        }
    }
}

static
void cut64(const struct self_t *self, void *Dst, const void *Src, size_t elem_count) {
    uint64_t *dst = Dst;
    const uint64_t *src = Src;
    int i;
    int j;
    int doff;
    int soff;
    
    for (doff = soff = i = 0; i != elem_count; ++i, doff += self->n, soff += self->dim) {
        for (j = 0; j != self->n; ++j) {
            dst[doff + j] = src[soff + self->idx[j]];
        }
    }
}

static
void cut_bytes(const struct self_t *self, void *Dst, const void *Src, size_t elem_count) {
    uint8_t *dst = Dst;
    const uint8_t *src = Src;
    int i;
    int j;
    int doff;
    int soff;
    const int sz = self->type_size >> 3;
    const int di = sz * self->n;
    const int si = sz * self->dim;
    
    for (doff = soff = i = 0; i != elem_count; ++i, doff += di, soff += si) {
        for (j = 0; j != self->n; ++j) {
            memcpy(dst + doff + j * sz, src + soff + self->idx[j] * sz, sz);
        }
    }
}

static
void cut_bits(const struct self_t *self, void *dst, const void *src, size_t elem_count) {
    int i;
    int j;
    int doff;
    int soff;
    const int sz = self->type_size;
    const int di = sz * self->n;
    const int si = sz * self->dim;
    
    for (doff = soff = i = 0; i != elem_count; ++i, doff += di, soff += si) {
        for (j = 0; j != self->n; ++j) {
            bitcpy(dst, doff + j * sz, src, soff + self->idx[j] * sz, sz);
        }
    }
}

static
rc_t CC cut_driver (
                    void *Self,
                    const VXformInfo *info,
                    void *dst,
                    const void *src,
                    uint64_t elem_count
                    )
{
    const self_t *self = Self;
    
    self->f( self, dst, src, elem_count );
    return 0;
}

static
void CC vxf_cut_wrapper( void *ptr )
{
	free( ptr );
}

/* cut
 * function < type T >
 * T [ * ] cut #1.0 < U32 idx, ... > ( T [ * ] in );
 */
VTRANSFACT_IMPL(vdb_cut, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    uint32_t dim = cp->argc;
    int i;
    self_t *ctx;
    
    assert(dim != 0);
    for (i = 0; i != dim; ++i) {
        if (*(cp->argv[i].data.u32) >= dp->argv[0].fd.td.dim)
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
    }
    
    ctx = malloc ( sizeof * ctx - sizeof ctx -> idx + dim * sizeof ctx -> idx [ 0 ] );
    if (ctx == NULL)
        return RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);

    ctx->type_size = VTypedescSizeof ( & info -> fdesc . desc ) / dim;
    ctx->dim = VTypedescSizeof ( & dp->argv[0].desc ) / ctx->type_size;
    ctx->n = dim;

    for (i = 0; i != dim; ++i)
    {
        ctx->idx[i] = cp->argv[i].data.u32[0];
        if ( ctx->idx[i] >= ctx->dim )
        {
            free ( ctx );
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
        }
    }
    
    rslt->self = ctx;
    rslt->whack = vxf_cut_wrapper;
    rslt->variant = vftArray;
    rslt->u.af = cut_driver;
    
    switch (ctx->type_size) {
        case 8:
            ctx->f = cut8;
            break;
        case 16:
            ctx->f = cut16;
            break;
        case 32:
            ctx->f = cut32;
            break;
        case 64:
            ctx->f = cut64;
            break;
        default:
            ctx->f = (ctx->type_size & 7) == 0 ? cut_bytes : cut_bits;
            break;
    }
    
	return 0;
}
