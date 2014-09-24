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
#include <vdb/vdb-priv.h>
#include <klib/data-buffer.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <endian.h>
#include <byteswap.h>
#include <string.h>
#include <zlib.h>
#include <assert.h>

static rc_t invoke_zlib(void *dst, size_t dsize, const void *src, size_t ssize, int windowBits)
{
    int zr;
    rc_t rc = 0;

    z_stream s;
    memset ( & s, 0, sizeof s );

    s.next_in = (void *)src;
    s.avail_in = ssize;
    s.next_out = dst;
    s.avail_out = dsize;
    
    zr = inflateInit2(&s, windowBits);
    switch (zr)
    {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        default:
            return RC(rcXF, rcFunction, rcExecuting, rcNoObj, rcUnexpected);
    }
    zr = inflate(&s, Z_FINISH);
    switch (zr)
    {
    case Z_STREAM_END:
        break;
    case Z_OK:         /* progress but not complete */
    case Z_BUF_ERROR:  /* no progress - need more output buffer */
        rc = RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        break;
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
    switch (zr)
    {
        case Z_OK:
            break;
        default:
            if (rc == 0)
                rc = RC(rcXF, rcFunction, rcExecuting, rcData, rcCorrupt);
    }

    return rc;
}

static
rc_t unzip_func_v1(
                   const VXformInfo *info,
                   VBlobResult *dst,
                   const VBlobData *src
) {
    dst->byte_order = src->byte_order;
    return invoke_zlib(dst->data, (((size_t)dst->elem_count * dst->elem_bits + 7) >> 3),
                       src->data, (((size_t)src->elem_count * src->elem_bits + 7) >> 3),
                       -15);
}

static
rc_t unzip_func_v2(
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
        rc = invoke_zlib(dst->data, (((size_t)dst->elem_count) >> 3),
                         src->data, (((size_t)src->elem_count * src->elem_bits + 7) >> 3),
                         -15);

        /* if the original, uncompressed source was NOT byte aligned,
           back off the rounded up byte and add in the original bit count */
        if ( rc == 0 && trailing != 0 )
            dst -> elem_count -= 8 - trailing;
    }

    return rc;
}

static
rc_t CC legacy_unzip_func ( void *self, const VXformInfo *info,
    VLegacyBlobResult *rslt, const KDataBuffer *src )
{
    rc_t rc;
    KDataBuffer *dst = rslt -> dst;
    const uint32_t *in = src -> base;
#if __BYTE_ORDER == __BIG_ENDIAN
    uint64_t bits = bswap_32 ( in [ 0 ] );
#else
    uint64_t bits = in [ 0 ];
#endif

    do
    {
        size_t const bytes = ( size_t ) ( ( bits + 7 ) >> 3 ) + 64;
        
        if ( ( ( uint64_t ) bytes << 3 ) < bits )
        {
            rc = RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
        }

        dst -> elem_bits = 8;
        rc = KDataBufferResize ( dst, bytes );
        if ( rc != 0 )
            break;

        rc = invoke_zlib ( dst -> base, bytes, & in [ 1 ], (size_t)KDataBufferBytes ( src ) - 4, 15 );
        if ( rc == 0 )
        {
            dst -> elem_bits = 1;
            dst -> elem_count = bits;
            break;
        }

        /* in hopes that this is a 32-bit overflow, try increasing bits */
        bits += ( uint64_t ) 1U << 32;
        if ( ( bits >> 32 ) == 16 )
            break;
    }
    while ( GetRCState ( rc ) == rcInsufficient && GetRCObject ( rc ) == (enum RCObject)rcBuffer );

    return rc;
}


static
rc_t CC unzip_func(
                void *Self,
                const VXformInfo *info,
                VBlobResult *dst,
                const VBlobData *src,
                VBlobHeader *hdr
) {
    int const version = VBlobHeaderVersion(hdr);
    
    switch (version) {
    case 1:
        return unzip_func_v1(info, dst, src);
        break;
    case 2:
        return unzip_func_v2(info, dst, src, hdr);
        break;
    default:
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcBadVersion);
    }
}

/* unzip
 *  function any unzip #1.0 ( zlib_fmt in );
 */
VTRANSFACT_IMPL ( vdb_unzip, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftBlob;
    rslt->u.bf = unzip_func;

    return 0;
}

/* NCBI:unzip
 *  function any NCBI:unzip #1.0 ( NCBI:zlib_encoded_t in );
 */
VTRANSFACT_IMPL ( NCBI_unzip, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    VNoHdrBlobFunc f = legacy_unzip_func;
    rslt->variant = vftLegacyBlob;
    rslt->u.bf = ( VBlobFunc ) f;

    return 0;
}
