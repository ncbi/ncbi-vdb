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
#include <klib/log.h>
#include <sysalloc.h>

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <zstd.h>

#include <stdio.h>
#include <assert.h>

typedef ZSTD_CCtx self_t;

static rc_t invoke_zstd(void *dst, size_t *dsize, const void *src, uint32_t ssize, ZSTD_CCtx* cctx)
{
    size_t const zr = ZSTD_compressCCtx(cctx, dst, *dsize, src, ssize, ZSTD_CLEVEL_DEFAULT);
    if (ZSTD_isError(zr)) {
        rc_t rc = RC(rcXF, rcFunction, rcExecuting, rcSelf, rcUnexpected);
        PLOGERR(klogErr, (klogErr, rc, "ZSTD_compressCCtx: error: $(err)", "err=%s", ZSTD_getErrorName(zr)));
        return rc;
    }
    else {
        *dsize = zr;
        return 0;
    }
}

static
rc_t CC zstd_func(
              void *Self,
              const VXformInfo *info,
              VBlobResult *dst,
              const VBlobData *src,
              VBlobHeader *hdr
) {
    rc_t rc;
    self_t *self = Self;

    /* input bits */
    uint64_t sbits = ((uint64_t)src->elem_count) * src->elem_bits;

    /* input bytes */
    uint64_t ssize = (sbits + 7) >> 3;

    /* required output size */
    size_t dsize = (((size_t)dst->elem_count) * dst->elem_bits + 7) >> 3;

    if ((sbits & 7) == 0)
        /* version 1 is byte-aligned */
        VBlobHeaderSetVersion (hdr, 1);
    else
    {
        VBlobHeaderSetVersion (hdr, 2);
        VBlobHeaderArgPushTail (hdr, (int64_t) (sbits & 7));
    }

    rc = invoke_zstd(dst->data, &dsize, src->data, ssize, self);
    if (rc == 0) {
        dst->elem_bits = 1;
        dst->byte_order = src->byte_order;
        if (dsize)
            dst->elem_count = dsize << 3;
        else
            rc = RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
    }
    return rc;
}

static
void CC vxf_zstd_wrapper(void *ptr)
{
    ZSTD_freeCCtx(ptr);
}

/* zstd
 * function zstd_fmt zstd #1.0 (any in);
 */
VTRANSFACT_IMPL(vdb_zstd, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp)
{
    self_t *ctx = ZSTD_createCCtx();

    if (ctx) {
        rslt->self = ctx;
        rslt->whack = vxf_zstd_wrapper;
        rslt->variant = vftBlob;
        rslt->u.bf = zstd_func;
        
        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
