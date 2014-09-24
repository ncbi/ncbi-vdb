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

#include <sysalloc.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <bitstr.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef void (*norm_f)(void *dst, const void *src, const void *Cntrl, uint32_t vec_count);
typedef struct self_t {
    norm_f f;
} self_t;

#define NORM_0(A, B) do { \
                            (A).data[0] = (B).data[0]; \
                            (A).data[1] = (B).data[1] - (B).data[0]; \
                            (A).data[2] = (B).data[2]; \
                            (A).data[3] = (B).data[3]; \
                        } while (0)
#define NORM_1(A, B) A = B
#define NORM_2(A, B) do { \
                            (A).data[0] = (B).data[0]; \
                            (A).data[1] = (B).data[1]; \
                            (A).data[2] = (B).data[2]; \
                            (A).data[3] = (B).data[3] - (B).data[2]; \
                        } while (0)
#define NORM_3(A, B) A = B

#define FUNC(VALTYPE) F_ ## VALTYPE

#define FUNC_DEF(VALTYPE) \
static void FUNC(VALTYPE)(void *Dst, const void *Src, const void *Cntrl, uint32_t count) { \
    typedef struct { VALTYPE data[4]; } data4_t; \
    data4_t *dst = (data4_t *)Dst; \
    const data4_t *src = (const data4_t *)Src; \
    const uint8_t *cntrl = (const uint8_t *)Cntrl; \
    uint32_t i; \
    \
    for (i = 0; i != count; ++i) { \
        switch(cntrl[i]) { \
        default: \
        case 0: \
            NORM_0(dst[i], src[i]); \
            break; \
        case 1: \
            NORM_1(dst[i], src[i]); \
            break; \
        case 2: \
            NORM_2(dst[i], src[i]); \
            break; \
        case 3: \
            NORM_3(dst[i], src[i]); \
            break; \
        }\
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
rc_t CC normal_drvr (
                void *Self,
                const VXformInfo *info,
                int64_t row_id,
                VRowResult *rslt,
                uint32_t argc,
                const VRowData argv []
) {
    rc_t rc = 0;
    const void *src;
    const void *cntrl;
    const self_t *self = (const self_t *)Self;
    
    assert(argv[0].u.data.elem_bits % 8 == 0);
    assert(argv[1].u.data.elem_bits % 8 == 0);
    
    src   = &((const uint8_t *)argv[0].u.data.base)[(argv[0].u.data.first_elem * argv[0].u.data.elem_bits) / 8];
    cntrl = &((const uint8_t *)argv[1].u.data.base)[(argv[1].u.data.first_elem * argv[1].u.data.elem_bits) / 8];
    
    rslt->elem_bits = rslt->data->elem_bits = argv[0].u.data.elem_bits;
    rc = KDataBufferResize(rslt->data, argv[0].u.data.elem_count);
    if (rc)
        return rc;
    rslt->elem_count = rslt->data->elem_count;
    rslt->data->bit_offset = 0;
    
    self->f(rslt->data->base, src, cntrl, rslt->elem_count);
    
    return 0;
}

static
void CC sraxf_normalize_free_wrapper( void *ptr )
{
	free( ptr );
}

/* 
 * function < type T > T [ 4 ] NCBI:SRA:normalize #1 ( T [ 4 ] intensities, U8 called );
 */
VTRANSFACT_IMPL ( NCBI_SRA_normalize, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    self_t *self;
    
    switch (info->fdesc.desc.domain) {
    case vtdInt:
    case vtdUint:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
        case 16:
        case 32:
        case 64:
            break;
        default:
            return RC(rcXF, rcFunction, rcConstructing, rcType, rcInvalid);
            break;
        }
        break;
    case vtdFloat:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 32:
        case 64:
            break;
        default:
            return RC(rcXF, rcFunction, rcConstructing, rcType, rcInvalid);
            break;
        }
        break;
    default:
        return RC(rcXF, rcFunction, rcConstructing, rcType, rcInvalid);
        break;
    }
    
    self = malloc(sizeof(*self));
    if (self) {
        switch (info->fdesc.desc.domain) {
        case vtdInt:
            switch (info->fdesc.desc.intrinsic_bits) {
            case 8:
                self->f = FUNC(int8_t);
                break;
            case 16:
                self->f = FUNC(int16_t);
                break;
            case 32:
                self->f = FUNC(int32_t);
                break;
            case 64:
                self->f = FUNC(int64_t);
                break;
            default:
                break;
            }
            break;
        case vtdUint:
            switch (info->fdesc.desc.intrinsic_bits) {
            case 8:
                self->f = FUNC(uint8_t);
                break;
            case 16:
                self->f = FUNC(uint16_t);
                break;
            case 32:
                self->f = FUNC(uint32_t);
                break;
            case 64:
                self->f = FUNC(uint64_t);
                break;
            default:
                break;
            }
            break;
        case vtdFloat:
            switch (info->fdesc.desc.intrinsic_bits) {
            case 32:
                self->f = FUNC(float);
                break;
            case 64:
                self->f = FUNC(double);
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
        
        rslt->self = self;
        rslt->whack = sraxf_normalize_free_wrapper;
        rslt->u.rf = normal_drvr;
        rslt->variant = vftRow;
        
        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
