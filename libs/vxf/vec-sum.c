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
#include <klib/data-buffer.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

typedef void (*math_f)(void *const dst,
                       void const *const src, uint32_t const offset,
                       uint32_t const vec_length, uint32_t const vec_count);
typedef struct self_t {
    uint32_t vec_length;
    math_f f;
} self_t;

#define FUNC(VALTYPE) F_ ## VALTYPE

#define FUNC_DEF(VALTYPE) \
static void FUNC(VALTYPE)(void *Dst, void const *Src, uint32_t const offset, uint32_t const vec_length, uint32_t const vec_count) { \
    VALTYPE sum; \
    VALTYPE *const dst = (VALTYPE *)Dst; \
    VALTYPE const *const src = &((VALTYPE const *)Src)[offset]; \
    uint32_t i; \
    uint32_t j; \
    uint32_t k; \
    \
    for (i = k = 0; i != vec_count; ++i) { \
        for (sum = 0, j = 0; j != vec_length; ++j, ++k) \
            sum += src[k]; \
        dst[i] = sum; \
    } \
}

FUNC_DEF(float)
FUNC_DEF(double)
FUNC_DEF(uint8_t)
FUNC_DEF(uint16_t)
FUNC_DEF(uint32_t)
FUNC_DEF(uint64_t)
FUNC_DEF(int8_t)
FUNC_DEF(int16_t)
FUNC_DEF(int32_t)
FUNC_DEF(int64_t)

static
rc_t CC vec_sum_row_func(void *const Self,
                         VXformInfo const *info,
                         int64_t const row_id,
                         VRowResult *const rslt,
                         uint32_t const argc,
                         VRowData const argv[])
{
    self_t const *self = Self;
    rc_t rc;
    
    assert(argc == 1);
    
    rslt->data->elem_bits = rslt->elem_bits;
    rc = KDataBufferResize(rslt->data, rslt->elem_count = 1);
    if (rc == 0) {
        self->f(rslt->data->base,
                argv[0].u.data.base, argv[0].u.data.first_elem,
                argv[0].u.data.elem_count, 1);
    }
    return rc;
}

static
rc_t CC vec_sum_array_func(
                void *Self,
                const VXformInfo *info,
                void *dst,
                const void *src,
                uint64_t elem_count
) {
    const self_t *self = Self;
    
    assert(elem_count % self->vec_length == 0);
    assert((elem_count / self->vec_length) >> 32 == 0);
    self->f(dst, src, 0, self->vec_length, (uint32_t)(elem_count / self->vec_length));
    return 0;
}

static
void CC vxf_vec_sum_wrapper( void *ptr )
{
	free( ptr );
}

rc_t vec_sum_make(self_t **const rslt,
                  VXfactInfo const *const info,
                  VFunctionParams const *const dp)
{
    self_t *self;
    rc_t rc = 0;
    
    self = malloc(sizeof(*self));
    if (self == NULL)
        rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
    else {
        self->vec_length = dp->argv[0].fd.td.dim;
        
        switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
            switch (info->fdesc.desc.domain) {
            case vtdInt:
                self->f = FUNC(int8_t);
                break;
            case vtdUint:
                self->f = FUNC(uint8_t);
                break;
            default:
                rc = RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            break;
        case 16:
            switch (info->fdesc.desc.domain) {
            case vtdInt:
                self->f = FUNC(int16_t);
                break;
            case vtdUint:
                self->f = FUNC(uint16_t);
                break;
            default:
                rc = RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            break;
        case 32:
            switch (info->fdesc.desc.domain) {
            case vtdInt:
                self->f = FUNC(int32_t);
                break;
            case vtdUint:
                self->f = FUNC(uint32_t);
                break;
            case vtdFloat:
                self->f = FUNC(float);
                break;
            default:
                rc = RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            break;
        case 64:
            switch (info->fdesc.desc.domain) {
            case vtdInt:
                self->f = FUNC(int64_t);
                break;
            case vtdUint:
                self->f = FUNC(uint64_t);
                break;
            case vtdFloat:
                self->f = FUNC(double);
                break;
            default:
                rc = RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            break;
        default:
            rc = RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
        }
        if (rc) {
            free(self);
            self = NULL;
        }
    }
    *rslt = self;
    return rc;
}

VTRANSFACT_IMPL(vdb_vec_sum, 1, 0, 0)(const void *Self,
                                      const VXfactInfo *info,
                                      VFuncDesc *rslt,
                                      const VFactoryParams *cp,
                                      const VFunctionParams *dp)
{
    self_t *self;
    rc_t rc = vec_sum_make(&self, info, dp);
    
    if (rc == 0) {
        rslt->self = self;
        rslt->whack = vxf_vec_sum_wrapper;
        rslt->variant = vftRow;
        rslt->u.rf = vec_sum_row_func;
    }
    return rc;
}

/*
 function < type T, U32 dim >
 T vec_sum #1.0 ( T [ dim ] in )
 */
VTRANSFACT_IMPL(vdb_fixed_vec_sum, 1, 0, 0)(const void *Self,
                                            const VXfactInfo *info,
                                            VFuncDesc *rslt,
                                            const VFactoryParams *cp,
                                            const VFunctionParams *dp)
{
    self_t *self;
    rc_t rc = vec_sum_make(&self, info, dp);
    
    if (rc == 0) {
        rslt->self = self;
        rslt->whack = vxf_vec_sum_wrapper;
        rslt->variant = vftArray;
        rslt->u.af = vec_sum_array_func;
    }
    return rc;
}
