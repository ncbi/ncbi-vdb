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

#include <zstd.h>

#include <klib/rc.h>
#include <vdb/xform.h>
#include <klib/log.h>

typedef ZSTD_DCtx self_t;

static rc_t invoke_zstd(void *dst, size_t dsize, const void *src, size_t ssize, ZSTD_DCtx *dctx)
{
    size_t zr = ZSTD_decompressDCtx(dctx, dst, dsize, src, ssize);
    if (ZSTD_isError(zr)) {
        rc_t rc = RC(rcXF, rcFunction, rcExecuting, rcSelf, rcUnexpected);
        PLOGERR(klogErr, (klogErr, rc, "ZSTD_decompressDCtx: error: $(err)", "err=%s", ZSTD_getErrorName(zr)));
        return rc;
    }
    return 0;
}

static
rc_t unzstd_func_v1(self_t *self,
                   const VXformInfo *info,
                   VBlobResult *dst,
                   const VBlobData *src
) {
    dst->byte_order = src->byte_order;
    return invoke_zstd(dst->data, (((size_t)dst->elem_count * dst->elem_bits + 7) >> 3),
                       src->data, (((size_t)src->elem_count * src->elem_bits + 7) >> 3),
                       self);
}

static
rc_t unzstd_func_v2(self_t *self,
                   const VXformInfo *info,
                   VBlobResult *dst,
                   const VBlobData *src,
                   VBlobHeader *hdr
) {
    int64_t trailing;
    rc_t rc = VBlobHeaderArgPopHead ( hdr, & trailing );
    if ( rc == 0 )
    {
        dst -> elem_count *= dst -> elem_bits;
        dst -> byte_order = src -> byte_order;
        dst -> elem_bits = 1;

        /* the feed to zlib MUST be byte aligned
           so the output must be as well */
        assert ( ( dst -> elem_count & 7 ) == 0 );
        rc = invoke_zstd(dst->data, (((size_t)dst->elem_count) >> 3),
                         src->data, (((size_t)src->elem_count * src->elem_bits + 7) >> 3),
                         self);

        /* if the original, uncompressed source was NOT byte aligned,
           back off the rounded up byte and add in the original bit count */
        if ( rc == 0 && trailing != 0 )
            dst -> elem_count -= 8 - trailing;
    }

    return rc;
}

static
rc_t CC unzstd_func(
                void *Self,
                const VXformInfo *info,
                VBlobResult *dst,
                const VBlobData *src,
                VBlobHeader *hdr
) {
    int const version = VBlobHeaderVersion(hdr);

    switch (version) {
    case 1:
        return unzstd_func_v1(Self, info, dst, src);
        break;
    case 2:
        return unzstd_func_v2(Self, info, dst, src, hdr);
        break;
    default:
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcBadVersion);
    }
}

static
void CC vxf_zstd_wrapper(void *ptr)
{
    ZSTD_freeDCtx(ptr);
}

/* unzstd
 *  function any unzstd #1.0 ( zstd_fmt in );
 */
VTRANSFACT_IMPL ( vdb_unzstd, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    self_t *ctx = ZSTD_createDCtx();
    if (ctx) {
        rslt->self = ctx;
        rslt->whack = vxf_zstd_wrapper;
        rslt->variant = vftBlob;
        rslt->u.bf = unzstd_func;

        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
