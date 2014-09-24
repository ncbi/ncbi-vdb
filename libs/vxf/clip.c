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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    union {
        /* integer */
        int8_t i8[2];
        int16_t i16[2];
        int32_t i32[2];
        int64_t i64[2];
    
        /* unsigned integer */
        uint8_t u8[2];
        uint16_t u16[2];
        uint32_t u32[2];
        uint64_t u64[2];
    
        /* floating point */
        float f32[2];
        double f64[2];
    } u;
    uint32_t dim;
} self_t;

#define CLIP_FUNC(FUNC, TYPE, VAL) \
static rc_t CC FUNC(void *Self, const VXformInfo *info, void *Dst, const void *Src, uint64_t n) { \
    const self_t *self = Self; \
    TYPE lower = self->u.VAL[0]; \
    TYPE upper = self->u.VAL[1]; \
    TYPE *dst = Dst; \
    const TYPE *src = Src; \
    size_t i; \
\
    for (i = 0, n *= self->dim; i != n; ++i) { \
        TYPE val = src[i]; \
\
        if (val < lower) \
            val = lower; \
        else if (val > upper) \
            val = upper; \
\
        dst[i] = val; \
    } \
    return 0; \
}

CLIP_FUNC(clip_U8, uint8_t, u8)
CLIP_FUNC(clip_I8, int8_t, i8)
CLIP_FUNC(clip_U16, uint16_t, u16)
CLIP_FUNC(clip_I16, int16_t, i16)
CLIP_FUNC(clip_U32, uint32_t, u32)
CLIP_FUNC(clip_I32, int32_t, i32)
CLIP_FUNC(clip_U64, uint64_t, u64)
CLIP_FUNC(clip_I64, int64_t, i64)
CLIP_FUNC(clip_F32, float, f32)
CLIP_FUNC(clip_F64, double, f64)

static
void CC vxf_free_wrapper( void *ptr )
{
	free( ptr );
}

/* clip
 function < type T > T clip #1.0 < T lower, T upper > ( T in ) = vdb:clip;
 function < type T, U32 dim > T [ dim ] vclip #1.0 < T lower, T upper > ( T [ dim ] in ) = vdb:clip;
 */
VTRANSFACT_IMPL(vdb_clip, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc = 0;
    self_t *self;

    /* check types */
    switch (info->fdesc.desc.domain) {
        case vtdInt:
        case vtdUint:
        case vtdFloat:
            break;
        default:
            return RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
    }
    switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
        case 16:
        case 32:
        case 64:
            break;
        default:
            return RC(rcVDB, rcFunction, rcConstructing, rcType, rcInvalid);
    }
    
    self = malloc(sizeof(*self));
    if (self == NULL)
        return RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
    
    rslt->self = self;
    rslt->whack = vxf_free_wrapper;
    rslt->variant = vftArray;

    self->dim = info->fdesc.fd.td.dim;
    
    switch (info->fdesc.desc.domain) {
        case vtdInt:
            switch (info->fdesc.desc.intrinsic_bits) {
                case 8:
                    rslt->u.af = clip_I8;
                    self->u.i8[0] = *cp->argv[0].data.i8;
                    self->u.i8[1] = *cp->argv[1].data.i8;
                    break;
                case 16:
                    rslt->u.af = clip_I16;
                    self->u.i16[0] = *cp->argv[0].data.i16;
                    self->u.i16[1] = *cp->argv[1].data.i16;
                    break;
                case 32:
                    rslt->u.af = clip_I32;
                    self->u.i32[0] = *cp->argv[0].data.i32;
                    self->u.i32[1] = *cp->argv[1].data.i32;
                    break;
                case 64:
                    rslt->u.af = clip_I64;
                    self->u.i64[0] = *cp->argv[0].data.i64;
                    self->u.i64[1] = *cp->argv[1].data.i64;
                    break;
            }
            break;
        case vtdUint:
            switch (info->fdesc.desc.intrinsic_bits) {
                case 8:
                    rslt->u.af = clip_U8;
                    self->u.u8[0] = *cp->argv[0].data.u8;
                    self->u.u8[1] = *cp->argv[1].data.u8;
                    break;
                case 16:
                    rslt->u.af = clip_U16;
                    self->u.u16[0] = *cp->argv[0].data.u16;
                    self->u.u16[1] = *cp->argv[1].data.u16;
                    break;
                case 32:
                    rslt->u.af = clip_U32;
                    self->u.u32[0] = *cp->argv[0].data.u32;
                    self->u.u32[1] = *cp->argv[1].data.u32;
                    break;
                case 64:
                    rslt->u.af = clip_U64;
                    self->u.u64[0] = *cp->argv[0].data.u64;
                    self->u.u64[1] = *cp->argv[1].data.u64;
                    break;
            }
            break;
        case vtdFloat:
            switch (info->fdesc.desc.intrinsic_bits) {
                case 32:
                    rslt->u.af = clip_F32;
                    self->u.f32[0] = *cp->argv[0].data.f32;
                    self->u.f32[1] = *cp->argv[1].data.f32;
                    break;
                case 64:
                    rslt->u.af = clip_F64;
                    self->u.f64[0] = *cp->argv[0].data.f64;
                    self->u.f64[1] = *cp->argv[1].data.f64;
                    break;
            }
            break;
    }
	return rc;
}
