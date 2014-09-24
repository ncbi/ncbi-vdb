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
#include <vdb/table.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


typedef void (*swap_f)(void *dst, uint64_t offset, const void *src, const void *Cntrl, uint64_t vec_count);
typedef struct self_t {
    swap_f f;
} self_t;

#define SWAP_0(A, B) A = B
#define SWAP_1(A, B) do { (A).data[0] = (B).data[1]; (A).data[1] = (B).data[0]; (A).data[2] = (B).data[2]; (A).data[3] = (B).data[3]; } while (0)
#define SWAP_2(A, B) do { (A).data[0] = (B).data[2]; (A).data[1] = (B).data[1]; (A).data[2] = (B).data[0]; (A).data[3] = (B).data[3]; } while (0)
#define SWAP_3(A, B) do { (A).data[0] = (B).data[3]; (A).data[1] = (B).data[1]; (A).data[2] = (B).data[2]; (A).data[3] = (B).data[0]; } while (0)

#define FUNC(VALTYPE) F_ ## VALTYPE

#define FUNC_DEF(VALTYPE)                                               \
    static void FUNC(VALTYPE)(void *Dst, uint64_t offset, const void *Src, const void *Cntrl, uint64_t count) { \
        typedef struct { VALTYPE data[4]; } data4_t;                    \
        data4_t *dst = (data4_t *)Dst;                                  \
        const data4_t *src = (const data4_t *)Src;                      \
        const uint8_t *cntrl = (const uint8_t *)Cntrl;                  \
        uint64_t i;                                                     \
                                                                        \
        dst += offset;                                                  \
        for (i = 0; i != count; ++i) {                                  \
            switch(cntrl[i]) {                                          \
            default:                                                    \
            case 0:                                                     \
                SWAP_0(dst[i], src[i]);                                 \
                break;                                                  \
            case 1:                                                     \
                SWAP_1(dst[i], src[i]);                                 \
                break;                                                  \
            case 2:                                                     \
                SWAP_2(dst[i], src[i]);                                 \
                break;                                                  \
            case 3:                                                     \
                SWAP_3(dst[i], src[i]);                                 \
                break;                                                  \
            }                                                           \
        }                                                               \
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
rc_t CC swap_drvr (
                void *Self,
                const VXformInfo *info,
                int64_t row_id,
                const VFixedRowResult *rslt,
                uint32_t argc,
                const VRowData argv []
) {
    const uint8_t *src;
    const uint8_t *cntrl;
    const self_t *self = (const self_t *)Self;
    
    assert(argv[0].u.data.elem_bits % 8 == 0);
    assert(argv[1].u.data.elem_bits % 8 == 0);
    
    src    = argv[0].u.data.base;
    src   += (argv[0].u.data.first_elem * argv[0].u.data.elem_bits) / 8;

    cntrl  = argv[1].u.data.base;
    cntrl += (argv[1].u.data.first_elem * argv[1].u.data.elem_bits) / 8;
    
    self->f(rslt->base, rslt->first_elem, src, cntrl, rslt->elem_count);
    
    return 0;
}

static
void CC sraxf_swap_free_wrapper( void *ptr )
{
	free( ptr );
}

/* 
 * function < type T > T [ 4 ] NCBI:SRA:swap #1 ( T [ 4 ] in, U8 called );
 */
VTRANSFACT_IMPL ( NCBI_SRA_swap, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
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
        rslt->whack = sraxf_swap_free_wrapper;
        rslt->u.pf = swap_drvr;
        rslt->variant = vftFixedRow;
        
        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
