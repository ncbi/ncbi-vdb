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

#define DECODING 1
#include "fsplit-join.impl.h"

static rc_t invoke_zlib(void *dst, uint32_t dsize, uint32_t *psize, const void *src, uint32_t ssize) {
    z_stream s;
    int zr;
    rc_t rc;
    
    memset(&s, 0, sizeof(s));
    s.next_in = (void *)src;
    s.avail_in = ssize;
    s.next_out = dst;
    s.avail_out = dsize;
    
    *psize = 0;
    zr = inflateInit2(&s, -15);
    switch (zr) {
    case 0:
        break;
    case Z_MEM_ERROR:
        return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
    default:
        return RC(rcXF, rcFunction, rcExecuting, rcNoObj, rcUnexpected);
    }
    zr = inflate(&s, Z_FINISH);
    switch (zr) {
    case Z_STREAM_END:
        assert(s.total_out <= UINT32_MAX);
        *psize = (uint32_t)s.total_out;
        rc = 0;
        break;
    case Z_OK:
        rc = RC(rcXF, rcFunction, rcExecuting, rcMemory, rcInsufficient);
        break;
    case Z_BUF_ERROR:
    case Z_NEED_DICT:
    case Z_DATA_ERROR:
        rc = RC(rcXF, rcFunction, rcExecuting, rcData, rcCorrupt);
        break;
    case Z_MEM_ERROR:
        rc = RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        break;
    default:
        rc = RC(rcXF, rcFunction, rcExecuting, rcNoObj, rcUnexpected);
        break;
    }
    zr = inflateEnd(&s);
    switch (zr) {
    case Z_OK:
        return rc;
    default:
        if (rc == 0)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcCorrupt);
        return rc;
    }
}

static
rc_t f32unzip_func_v0(
                   const VXformInfo *info,
                   VBlobResult *Dst,
                   const VBlobData *src,
                   const VBlobHeader *hdr
) {
    rc_t rc;
    uint8_t mantissa;
    int64_t val;
    float *dst = Dst->data;
    uint32_t elem_count;
    const bitsz_t dbits = Dst->elem_count * Dst->elem_bits;
    KDataBuffer scratch;
    
    rc = KDataBufferMakeBytes(&scratch, (dbits + 7) >> 3);
    if (rc == 0) {
        assert(scratch.elem_count >> 32 == 0);
        rc = invoke_zlib(scratch.base, (uint32_t)scratch.elem_count, &elem_count,
                         src->data, ((size_t)src->elem_count * src->elem_bits + 7) >> 3);
        if (rc == 0) {
            scratch.elem_count = elem_count;
            
            VBlobHeaderArgPopHead(hdr, &val);
            mantissa = (uint8_t)val;
            
            elem_count = (uint32_t)((8 * (size_t)scratch.elem_count) / (8 + mantissa));
            if ((size_t)elem_count * 32 > dbits)
                rc = RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
            else {
                Dst->elem_bits = 32;
                Dst->elem_count = elem_count;
                Dst->byte_order = vboNative;
                
                unpack_and_join_f32(scratch.base, elem_count, mantissa, dst);
            }
        }
    }
    KDataBufferWhack(&scratch);
    return rc;
}

static
rc_t CC f32unzip_func(
                void *Self,
                const VXformInfo *info,
                VBlobResult *dst,
                const VBlobData *src,
                VBlobHeader *hdr
) {
    uint8_t bits;
    
    switch (VBlobHeaderVersion(hdr)) {
    case 0:
        VBlobHeaderOpPopHead(hdr, &bits);
        switch (bits) {
        case 0:
            return f32unzip_func_v0(info, dst, src, hdr);
        }
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcUnexpected);
    default:
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcBadVersion);
    }
}

/* unzip
 function
 fzip_set funzip #1.0 ( fzip_fmt in );
 */
VTRANSFACT_IMPL(vdb_funzip, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftBlob;
    rslt->u.bf = f32unzip_func;

    return 0;
}
