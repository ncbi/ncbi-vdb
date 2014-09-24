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

#include "blob.h"
#include "blob-headers.h"
#include "page-map.h"
#include "blob-priv.h"
#include "xform-priv.h"
#include "prod-priv.h"

#include <vdb/xform.h>
#include <vdb/schema.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/debug.h>
#include <atomic32.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define RC_NOT_SAME RC(rcVDB, rcBlob, rcValidating, rcData, rcCorrupt)

typedef struct float_compare_t {
    unsigned sig_bits;
} float_compare_t;

static
rc_t CC blob_compare(
                  void *self,
                  const VRowData *orig,
                  const VRowData *test
) {
    const bitsz_t o_ofs = (size_t)orig->u.data.first_elem * orig->u.data.elem_bits;
    const bitsz_t t_ofs = (size_t)test->u.data.first_elem * orig->u.data.elem_bits;
    const bitsz_t bits = (size_t)orig->u.data.elem_count * orig->u.data.elem_bits;
    int cmp;

    if (orig->u.data.elem_bits % 8 == 0) {
        cmp = memcmp(
                   &((const char *)orig->u.data.base)[o_ofs >> 3],
                   &((const char *)test->u.data.base)[t_ofs >> 3],
                   bits >> 3
            );
    }
    else {
        cmp = bitcmp(orig->u.data.base, o_ofs, test->u.data.base, t_ofs, bits);
    }
    if(cmp)
	return RC_NOT_SAME;
    return 0;

}
static bool equal_f32( const float A[], const float B[], uint64_t elem_count, unsigned sig_bits ) {
    unsigned i;
    const int shift = 24 - sig_bits;
    
    assert(shift > 0);
    
    for (i = 0; i != elem_count; ++i) {
        int expa;
        int expb;
        uint32_t mana;
        uint32_t manb;
        float a = A[i];
        float b = B[i];
        int diff;
        
        if (a < 0) {
            a = -a;
            b = -b;
            if (b < 0)
                return false;
        }

        a = frexpf(a, &expa);
        b = frexpf(b, &expb);
        
        if (expa < expb) {
            ++expa;
            a /= 2.0;
        }
        else if (expb < expa) {
            ++expb;
            b /= 2.0;
        }
        if (expa != expb)
            return false;
        
        mana = (uint32_t)ldexpf( a, 24 );
        manb = (uint32_t)ldexpf( b, 24 );
        
        diff = (mana >> shift) - (manb >> shift);

        if (-1 > diff || diff > 1)
            return false;
    }
    return true;
}

static
rc_t CC blob_compare_f32(
                      void *Self,
                      const VRowData *orig,
                      const VRowData *test
) {
    float_compare_t *self = Self;
    
    if (!equal_f32(
                   &((const float *)orig->u.data.base)[orig->u.data.first_elem],
                   &((const float *)test->u.data.base)[test->u.data.first_elem],
                   orig->u.data.elem_count,
                   self->sig_bits
               ))
    {
        return RC_NOT_SAME;
    }
    
    return 0;
}

static
void CC vfunc_free ( void * self )
{
    free (self);
}

VTRANSFACT_BUILTIN_IMPL(vdb_compare, 1, 0, 0) (
                                       const void *self,
                                       const VXfactInfo *info,
                                       VFuncDesc *rslt,
                                       const VFactoryParams *cp,
                                       const VFunctionParams *dp
) 
{
    VTypedesc type;
    assert ( dp->argc == 2 );

    rslt->variant = prodFuncBuiltInCompare;
    
#if 0
    if (dp->argv[0].desc.domain != dp->argv[1].desc.domain ||
        dp->argv[0].desc.intrinsic_dim != dp->argv[1].desc.intrinsic_dim ||
        dp->argv[0].desc.intrinsic_bits != dp->argv[1].desc.intrinsic_bits
    ) {
        return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
    }
#endif
    type = dp->argv[0].desc;
    
    if (type.domain == vtdFloat) {
        if (cp->argc > 0) {
            unsigned sig_bits = cp->argv[0].data.u32[0];

            if (sig_bits == 0) {
                return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            }
            switch (type.intrinsic_bits) {
            case 32:
                if (sig_bits < 24)
                    VFUNCDESC_INTERNAL_FUNCS(rslt)->cf = blob_compare_f32;
                break;
            case 64:
#if 0
                if (sig_bits < 52)
                    VFUNCDESC_INTERNAL_FUNCS(rslt)->cf = blob_compare_f64;
#endif
                break;
            default:
                break;
            }
            if (VFUNCDESC_INTERNAL_FUNCS(rslt)->cf) {
                float_compare_t *Self;
            
                Self = malloc(sizeof *Self);
                if (Self == NULL)
                    return RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);

                Self->sig_bits = sig_bits;
            
                rslt->self = Self;
                rslt->whack = vfunc_free;
                return 0;
            }
        }
        else {
            VDB_DEBUG (("warning: using identity compare for floating point types won't work if using fzip\n"));
        }
    }
    VFUNCDESC_INTERNAL_FUNCS(rslt)->cf = blob_compare;
    return 0;
}

static
rc_t CC true_func(
                  void *self,
                  const VRowData *orig,
                  const VRowData *test
) {
    return 0;
}

VTRANSFACT_BUILTIN_IMPL(vdb_no_compare, 1, 0, 0) (
                                       const void *self,
                                       const VXfactInfo *info,
                                       VFuncDesc *rslt,
                                       const VFactoryParams *cp,
                                       const VFunctionParams *dp
) 
{
    rslt->variant = prodFuncBuiltInCompare;
    VFUNCDESC_INTERNAL_FUNCS(rslt)->cf = true_func;
    return 0;
}
