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
#include <bzlib.h>

static rc_t invoke_bzip2 ( void *dst, size_t dsize, const void *src, size_t ssize )
{
    int bzerr;

    bz_stream s;
    memset ( & s, 0, sizeof s );
    bzerr = BZ2_bzDecompressInit ( & s, 0, 0 );
    if ( bzerr != BZ_OK )
        return RC ( rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted );

    s.next_in = (void *)src;
    s.avail_in = ssize;
    s.next_out = dst;
    s.avail_out = dsize;
    
    bzerr = BZ2_bzDecompress(&s);
    BZ2_bzDecompressEnd ( & s );

    switch ( bzerr )
    {
    case BZ_OK:
    case BZ_STREAM_END:
        break;
    default:
        return RC(rcXF, rcFunction, rcExecuting, rcNoObj, rcUnexpected);
    }

    return 0;
}

static
rc_t bunzip_func_v1 ( const VXformInfo *info, VBlobResult *dst, const VBlobData *src )
{
    dst->byte_order = src->byte_order;
    return invoke_bzip2 ( dst->data, (((size_t)dst->elem_count * dst->elem_bits + 7) >> 3),
                          src->data, (((size_t)src->elem_count * src->elem_bits + 7) >> 3));
}

static
rc_t bunzip_func_v2 ( const VXformInfo *info,
    VBlobResult *dst, const VBlobData *src, VBlobHeader *hdr )
{
    int64_t trailing;
    rc_t rc = VBlobHeaderArgPopHead ( hdr, & trailing );
    if ( rc == 0 )
    {
        dst -> elem_count *= dst -> elem_bits;
        dst -> byte_order = src -> byte_order;
        dst -> elem_bits = 1;

        rc = invoke_bzip2 ( dst->data, (((size_t)dst->elem_count + 7) >> 3),
                            src->data, (((size_t)src->elem_count * src->elem_bits + 7) >> 3));
        if ( rc == 0 )
        {
            dst -> elem_count &= ( uint64_t ) ~ 7;
            dst -> elem_count |= trailing;
        }
    }

    return rc;
}


static
rc_t CC bunzip_func ( void *Self, const VXformInfo *info,
    VBlobResult *dst, const VBlobData *src, VBlobHeader *hdr )
{
    switch ( VBlobHeaderVersion ( hdr ) )
    {
    case 1:
        return bunzip_func_v1(info, dst, src);
    case 2:
        return bunzip_func_v2(info, dst, src, hdr);
    }

    return RC(rcXF, rcFunction, rcExecuting, rcParam, rcBadVersion);
}

/* bunzip
 *  function any bunzip #1.0 ( bzip2_fmt in );
 */
VTRANSFACT_IMPL ( vdb_bunzip, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftBlob;
    rslt->u.bf = bunzip_func;

    return 0;
}
