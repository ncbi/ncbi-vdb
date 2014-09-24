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

#include <compiler.h>
#include <math.h>

typedef void (*math_f)(void *dst, const void *src, size_t elem_count);
typedef union self_t {
    void *vp;
    math_f f;
} self_t;

static void F32_to_I8(void *Dst, const void *Src, size_t elem_count) {
    int8_t *dst = Dst;
    const float *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (int8_t)FLOAT_FUNC(src[i]);
}

static void F32_to_U8(void *Dst, const void *Src, size_t elem_count) {
    uint8_t *dst = Dst;
    const float *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (uint8_t)FLOAT_FUNC(src[i]);
}

static void F32_to_I16(void *Dst, const void *Src, size_t elem_count) {
    int16_t *dst = Dst;
    const float *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (int16_t)FLOAT_FUNC(src[i]);
}

static void F32_to_U16(void *Dst, const void *Src, size_t elem_count) {
    uint16_t *dst = Dst;
    const float *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (uint16_t)FLOAT_FUNC(src[i]);
}

static void F32_to_I32(void *Dst, const void *Src, size_t elem_count) {
    int32_t *dst = Dst;
    const float *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (int32_t)FLOAT_FUNC(src[i]);
}

static void F32_to_U32(void *Dst, const void *Src, size_t elem_count) {
    uint32_t *dst = Dst;
    const float *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (uint32_t)FLOAT_FUNC(src[i]);
}

static void F32_to_I64(void *Dst, const void *Src, size_t elem_count) {
    int64_t *dst = Dst;
    const float *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (int64_t)FLOAT_FUNC(src[i]);
}

static void F32_to_U64(void *Dst, const void *Src, size_t elem_count) {
    uint64_t *dst = Dst;
    const float *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (uint64_t)FLOAT_FUNC(src[i]);
}

static void F32_to_F32(void *Dst, const void *Src, size_t elem_count) {
    float *dst = Dst;
    const float *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = FLOAT_FUNC(src[i]);
}

static void F32_to_F64(void *Dst, const void *Src, size_t elem_count) {
    double *dst = Dst;
    const float *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = FLOAT_FUNC(src[i]);
}

static void F64_to_I8(void *Dst, const void *Src, size_t elem_count) {
    int8_t *dst = Dst;
    const double *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (int8_t)DOUBLE_FUNC(src[i]);
}

static void F64_to_U8(void *Dst, const void *Src, size_t elem_count) {
    uint8_t *dst = Dst;
    const double *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (uint8_t)DOUBLE_FUNC(src[i]);
}

static void F64_to_I16(void *Dst, const void *Src, size_t elem_count) {
    int16_t *dst = Dst;
    const double *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (int16_t)DOUBLE_FUNC(src[i]);
}

static void F64_to_U16(void *Dst, const void *Src, size_t elem_count) {
    uint16_t *dst = Dst;
    const double *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (uint16_t)DOUBLE_FUNC(src[i]);
}

static void F64_to_I32(void *Dst, const void *Src, size_t elem_count) {
    int32_t *dst = Dst;
    const double *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (int32_t)DOUBLE_FUNC(src[i]);
}

static void F64_to_U32(void *Dst, const void *Src, size_t elem_count) {
    uint32_t *dst = Dst;
    const double *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (uint32_t)DOUBLE_FUNC(src[i]);
}

static void F64_to_I64(void *Dst, const void *Src, size_t elem_count) {
    int64_t *dst = Dst;
    const double *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (int64_t)DOUBLE_FUNC(src[i]);
}

static void F64_to_U64(void *Dst, const void *Src, size_t elem_count) {
    uint64_t *dst = Dst;
    const double *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (uint64_t)DOUBLE_FUNC(src[i]);
}

static void F64_to_F32(void *Dst, const void *Src, size_t elem_count) {
    float *dst = Dst;
    const double *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = (float)DOUBLE_FUNC(src[i]);
}

static void F64_to_F64(void *Dst, const void *Src, size_t elem_count) {
    double *dst = Dst;
    const double *src = Src;
    int i;
    
    for (i = 0; i != elem_count; ++i)
        dst[i] = DOUBLE_FUNC(src[i]);
}

static
rc_t CC array_func(
                void *Self,
                const VXformInfo *info,
                void *dst,
                const void *src,
                uint64_t num_elements
                )
{
    self_t hack;

    hack.vp = Self;
    hack.f(dst, src, ( size_t ) num_elements);
    return 0;
}

static rc_t factory( const void *self, const VXfactInfo *info, 
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    VTypedesc src_desc;
    rc_t rc;
    self_t hack;

    rc = VSchemaDescribeTypedecl(info->schema, &src_desc, &dp->argv[0].fd.td);
    if (rc)
        return rc;
    
    rslt->variant = vftArray;
    rslt->u.af = array_func;
    
    switch (src_desc.intrinsic_bits) {
    case 32:
        switch (info->fdesc.desc.domain) {
        case vtdInt:
            switch (info->fdesc.desc.intrinsic_bits) {
            case 8:
                hack.f = F32_to_I8;
                break;
            case 16:
                hack.f = F32_to_I16;
                break;
            case 32:
                hack.f = F32_to_I32;
                break;
            case 64:
                hack.f = F32_to_I64;
                break;
            default:
                return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            break;
        case vtdUint:
            switch (info->fdesc.desc.intrinsic_bits) {
            case 8:
                hack.f = F32_to_U8;
                break;
            case 16:
                hack.f = F32_to_U16;
                break;
            case 32:
                hack.f = F32_to_U32;
                break;
            case 64:
                hack.f = F32_to_U64;
                break;
            default:
                return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            break;
        case vtdFloat:
            switch (info->fdesc.desc.intrinsic_bits) {
            case 32:
                hack.f = F32_to_F32;
                break;
            case 64:
                hack.f = F32_to_F64;
                break;
            default:
                return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            break;
        default:
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
        }
        break;
    case 64:
        switch (info->fdesc.desc.domain) {
        case vtdInt:
            switch (info->fdesc.desc.intrinsic_bits) {
            case 8:
                hack.f = F64_to_I8;
                break;
            case 16:
                hack.f = F64_to_I16;
                break;
            case 32:
                hack.f = F64_to_I32;
                break;
            case 64:
                hack.f = F64_to_I64;
                break;
            default:
                return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            break;
        case vtdUint:
            switch (info->fdesc.desc.intrinsic_bits) {
            case 8:
                hack.f = F64_to_U8;
                break;
            case 16:
                hack.f = F64_to_U16;
                break;
            case 32:
                hack.f = F64_to_U32;
                break;
            case 64:
                hack.f = F64_to_U64;
                break;
            default:
                return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            break;
        case vtdFloat:
            switch (info->fdesc.desc.intrinsic_bits) {
            case 32:
                hack.f = F64_to_F32;
                break;
            case 64:
                hack.f = F64_to_F64;
                break;
            default:
                return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            break;
        default:
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
        }
        break;
    default:
        return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
    }
    rslt->self = hack.vp;
    return 0;
}

