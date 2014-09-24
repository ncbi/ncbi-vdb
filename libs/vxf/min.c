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

typedef void (*func_t)(void *, const void *, const void *, uint64_t);

typedef struct self_t {
    func_t f;
} self_t;

#define FUNC(VALTYPE) F_ ## VALTYPE

#define FUNC_DEF(VALTYPE) \
static void FUNC(VALTYPE)(void *Dst, const void *A, const void *B, uint64_t n) { \
    VALTYPE *dst = (VALTYPE *)Dst; \
    const VALTYPE *a = (const VALTYPE *)A; \
    const VALTYPE *b = (const VALTYPE *)B; \
    uint64_t i; \
    \
    for (i = 0; i != n; ++i) \
        dst[i] = a[i] < b[i] ? a[i] : b[i]; \
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
rc_t CC fixed_row_func(
                      void *Self,
                      const VXformInfo *info,
                      int64_t row_id,
                      const VFixedRowResult *rslt,
                      uint32_t argc,
                      const VRowData argv[]
) {
    const self_t *self = Self;

    /* base pointers to pages */
    uint8_t *dst = rslt -> base;
    const uint8_t *a = argv [ 0 ] . u . data . base;
    const uint8_t *b = argv [ 1 ] . u . data . base;

    /* offset by bytes to first element of row */
    dst += ( rslt -> elem_bits * rslt -> first_elem ) >> 3;
    a += ( argv [ 0 ] . u . data . elem_bits * argv [ 0 ] . u . data . first_elem ) >> 3;
    b += ( argv [ 1 ] . u . data . elem_bits * argv [ 1 ] . u . data . first_elem ) >> 3;
    
    self -> f ( dst, a, b, rslt -> elem_count );
    return 0;
}

static
void CC vxf_min_wrapper( void *ptr )
{
	free( ptr );
}

/* 
 */
VTRANSFACT_IMPL(vdb_min, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc = 0;
    self_t *self;

	self = malloc(sizeof(self_t));
    if (self == NULL)
        return RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
        
    rslt->self = self;
    rslt->whack = vxf_min_wrapper;
    rslt->variant = vftFixedRow;
    rslt->u.pf = fixed_row_func;

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

    if (rc)
        free(self);
    return rc;
}
