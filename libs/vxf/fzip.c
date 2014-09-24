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
#include <string.h>
#include <zlib.h>

#include <assert.h>

#define ROUND_TO_NEAREST 1
#define ENCODING 1
#include "fsplit-join.impl.h"

struct self_t {
    int32_t mantissa;
};

static rc_t invoke_zlib(void *dst, uint64_t *dsize, const void *src, uint32_t ssize, int32_t strategy, int32_t level) {
    z_stream s;
    int zr;
    rc_t rc = 0;
    
    memset(&s, 0, sizeof(s));
    s.next_in = (void *)src;
    s.avail_in = ssize;
    s.next_out = dst;
    s.avail_out = (uInt)*dsize;
    
    *dsize = 0;
    zr = deflateInit2(&s, level, Z_DEFLATED, -15, 9, strategy);
    switch (zr) {
        case 0:
            break;
        case Z_MEM_ERROR:
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        case Z_STREAM_ERROR:
            return RC(rcXF, rcFunction, rcExecuting, rcParam, rcInvalid);
        default:
            return RC(rcXF, rcFunction, rcExecuting, rcSelf, rcUnexpected);
    }
    zr = deflate(&s, Z_FINISH);
    switch (zr) {
        case Z_STREAM_END:
            break;
        case Z_OK:
            s.total_out = 0;
            break;
        default:
            rc = RC(rcXF, rcFunction, rcExecuting, rcSelf, rcUnexpected);
            break;
    }
    zr = deflateEnd(&s);
    if (zr != Z_OK)
        rc = RC(rcXF, rcFunction, rcExecuting, rcSelf, rcUnexpected);
    if (rc == 0) {
        assert(s.total_out <= UINT32_MAX);
        *dsize = (uint32_t)s.total_out;
    }
    return rc;
}

static
rc_t CC f32zip_func(
              void *Self,
              const VXformInfo *info,
              VBlobResult *dst,
              const VBlobData *Src,
              VBlobHeader *hdr
) {
    rc_t rc;
    const struct self_t *self = Self;
    uint64_t dsize = (dst->elem_count * dst->elem_bits + 7) >> 3;
    uint64_t element_count = (Src->elem_count * Src->elem_bits) >> 5;
    uint64_t man_bytes;
    KDataBuffer scratch;
    
    assert(element_count >> 32 == 0);
    assert(dsize >> 32 == 0);
    rc = KDataBufferMakeBytes(&scratch, element_count << 2);
    if (rc)
        return rc;
    
    VBlobHeaderSetVersion(hdr, 0);
    VBlobHeaderOpPushTail(hdr, 0);
    VBlobHeaderArgPushTail(hdr, self->mantissa);
    
    man_bytes = split_and_pack_f32(
                                   Src->data, (uint32_t)element_count,
                                   self->mantissa,
                                   scratch.base);
    assert((element_count + man_bytes) >> 32 == 0);
    rc = invoke_zlib(dst->data, &dsize, scratch.base, (uint32_t)(element_count + man_bytes), Z_RLE, Z_BEST_SPEED);
    KDataBufferWhack(&scratch);
    if (rc == 0) {
        dst->elem_bits = 1;
        dst->byte_order = vboNone;
        if (dsize)
            dst->elem_count = dsize << 3;
        else
            rc = RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
    }
    return rc;
}

static
void CC vxf_fzip_wrapper( void *ptr )
{
	free( ptr );
}

/* vdb:fzip
 * function fzip_fmt vdb:fzip #1.0 < U32 mantissa > ( fzip_set in );
 */
VTRANSFACT_IMPL(vdb_fzip, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    struct self_t *ctx;
    uint32_t mantissa = 0;

    assert(cp->argc == 1);
    
    mantissa = cp->argv[0].data.u32[0];
    if (1 > mantissa || mantissa > 24)
        return RC(rcXF, rcFunction, rcConstructing, rcParam, rcInvalid);
    
    ctx = malloc(sizeof(*ctx));
    if (ctx) {
        ctx->mantissa = mantissa;
       
        rslt->self = ctx;
        rslt->whack = vxf_fzip_wrapper;
        rslt->variant = vftBlob;
        rslt->u.bf = f32zip_func;
        
        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
