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
#include <klib/data-buffer.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <sysalloc.h>

#include <bitstr.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#define DECODER_NAME(T) outlier_encode_ ## T
#define DECODER(T) static rc_t CC DECODER_NAME(T) (void *self, const VXformInfo *info, void *Dst, const void *Src, uint64_t elem_count) \
{ \
    T *dst = (T *)Dst; \
    T const *src = (T const *)Src; \
    T const outlier = *(T const *)self; \
    uint64_t i; \
    for (i = 0; i != elem_count; ++i) { \
        T const y = src[i]; \
        if (y & 1) \
            dst[i] = outlier; \
        else \
            dst[i] = y >> 1; \
    } \
    return 0; \
}

DECODER(uint8_t)
DECODER(uint16_t)
DECODER(uint32_t)
DECODER(uint64_t)

/*
 function < type T > T outlier_decode #1.0 < T outlier > ( T y );
 */

VTRANSFACT_IMPL ( vdb_outlier_decode, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
                                                 VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    void *self;
    
    if (info->fdesc.desc.domain != vtdInt || cp->argv[0].count != 1)
        return RC(rcXF, rcFunction, rcConstructing, rcType, rcIncorrect);
        
    switch (VTypedescSizeof(&cp->argv[0].desc))
    {
    case 8:
    case 16:
    case 32:
    case 64:
        break;
    default:
        return RC(rcXF, rcFunction, rcConstructing, rcType, rcIncorrect);
        break;
    }
    
    self = malloc(sizeof(uint64_t));
    if (self == NULL)
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
        
    switch (VTypedescSizeof(&cp->argv[0].desc))
    {
    case 8:
        *(uint8_t *)self = cp->argv[0].data.u8[0];
        rslt->u.af = DECODER_NAME(uint8_t);
        break;
    case 16:
        *(uint16_t *)self = cp->argv[0].data.u16[0];
        rslt->u.af = DECODER_NAME(uint16_t);
        break;
    case 32:
        *(uint32_t *)self = cp->argv[0].data.u32[0];
        rslt->u.af = DECODER_NAME(uint32_t);
        break;
    case 64:
        *(uint64_t *)self = cp->argv[0].data.u64[0];
        rslt->u.af = DECODER_NAME(uint64_t);
        break;
    default:
        break;
    }

    rslt->self = self;
    rslt->whack = free;
    rslt->variant = vftArray;
    return 0;
}
