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
#include <klib/log.h>
#include <vdb/xform.h>

#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>

#ifndef ZSTD_MIN_LEVEL
#define ZSTD_MIN_LEVEL (-131072)
#endif

#ifndef ZSTD_MAX_LEVEL
#define ZSTD_MAX_LEVEL 22
#endif

#include <stdio.h>

static rc_t invoke_zstd(void *dst, uint32_t *dsize, const void *src, uint32_t ssize, ZSTD_CCtx * self)
{
    size_t size = ZSTD_compress2( self, dst, (size_t)dsize, src, (size_t)ssize);
    if ( ZSTD_isError( size ) )
    {
        rc_t rc = RC(rcXF, rcFunction, rcExecuting, rcSelf, rcUnexpected);
        PLOGERR(klogErr, (klogErr, rc, "ZSTD_compress2: error: $(err)", "err=%s", ZSTD_getErrorName( size )));
        return rc;
    }
    PLOGMSG(klogInfo, (klogInfo, "ZSTD_compress2: from $(in) to $(out)", "in=%u,out=%u", ssize, size));

    *dsize = (uint32_t)size;
    return 0;
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
    struct self_t *self = Self;

    /* input bits */
    uint64_t sbits = ( uint64_t) src -> elem_count * src -> elem_bits;

    /* input bytes */
    uint32_t ssize = ( uint32_t ) ( ( sbits + 7 ) >> 3 );

    /* required output size */
    uint32_t dsize = ( uint32_t ) ( ( ( size_t ) dst -> elem_count * dst->elem_bits + 7 ) >> 3 );

    if ( ( sbits & 7 ) == 0 )
        /* version 1 is byte-aligned */
        VBlobHeaderSetVersion ( hdr, 1 );
    else
    {
        VBlobHeaderSetVersion ( hdr, 2 );
        VBlobHeaderArgPushTail ( hdr, ( int64_t ) ( sbits & 7 ) );
    }

    rc = invoke_zstd( dst -> data, & dsize, src -> data, ssize, (ZSTD_CCtx *)self);
    if (rc == 0)
    {
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
void CC vxf_zstd_wrapper( void *ptr )
{
    ZSTD_freeCCtx( ptr );
}

/* zstd
 * function zstd_fmt zstd #1.0 < I32 level > ( any in );
 */
VTRANSFACT_IMPL(vdb_zstd, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    ZSTD_CCtx * ctx = ZSTD_createCCtx();
    if ( ctx )
    {
        // ZSTD_bounds bounds = ZSTD_cParam_getBounds(ZSTD_c_strategy);
        // PLOGMSG(klogWarn, (klogWarn, "ZSTD_cParam_getBounds(ZSTD_c_strategy): $(v1)..$(v2)", "v1=%i,v2=%i", bounds.lowerBound, bounds.upperBound));
        // bounds = ZSTD_cParam_getBounds(ZSTD_c_compressionLevel);
        // PLOGMSG(klogWarn, (klogWarn, "ZSTD_c_compressionLevel(ZSTD_c_strategy): $(v1)..$(v2)", "v1=%i,v2=%i", bounds.lowerBound, bounds.upperBound));

        if ( cp -> argc > 0 )
        {
            // NB failing construction leads to problems in VDB (VDB-4468).
            // simply trying to set bad parameters does not seem to worry zstd. Report and continue.

            // strategy
            int32_t v = cp -> argv [ 0 ] . data . i32 [ 0 ];
            size_t zr = ZSTD_CCtx_setParameter( ctx, ZSTD_c_strategy, v );
            if ( ZSTD_isError( zr ) )
            {
                PLOGMSG(klogWarn, (klogWarn, "ZSTD_CCtx_setParameter(ZSTD_c_strategy=$(v)): $(err)", "v=%i,err=%s", v, ZSTD_getErrorName( zr )));
            }
            if ( cp -> argc > 1 )
            {   // compression level
                v = cp -> argv [ 1 ] . data . i32 [ 0 ];
                zr = ZSTD_CCtx_setParameter( ctx, ZSTD_c_compressionLevel, v );
                if ( ZSTD_isError( zr ) )
                {
                    PLOGMSG(klogWarn, (klogWarn, "ZSTD_CCtx_setParameter(ZSTD_c_compressionLevel=$(v)): $(err)", "v=%i,err=%s", v, ZSTD_getErrorName( zr )));
                }
            }
        }

        rslt->self = ctx;
        rslt->whack = vxf_zstd_wrapper;
        rslt->variant = vftBlob;
        rslt->u.bf = zstd_func;

        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
