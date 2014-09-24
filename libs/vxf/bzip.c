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

#include <string.h>
#include <bzlib.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>
#include <assert.h>

#define BUFFER_GROWTH_RATE (128 * 1024)

typedef struct bzip_t bzip_t;
struct bzip_t
{
    int32_t blockSize100k;
    int32_t workFactor;
};

#if _DEBUGGING
/*
static
void debug_print_bz_stream(const bz_stream *s)
{
    printf("avail_in: %u, avail_out: %u, total_in_lo32: %u, total_in_hi32: %u, total_out_lo32: %u, total_out_hi32: %u\n",
           s->avail_in,
           s->avail_out,
           s->total_in_lo32, s->total_in_hi32,
           s->total_out_lo32, s->total_out_hi32);
}
*/
#endif

static
rc_t invoke_bzip2 ( bzip_t *self, void *dst, uint32_t *dsize, const void *src, uint32_t ssize)
{
    int bzerr;

    bz_stream s;
    memset ( & s, 0, sizeof s );
    bzerr = BZ2_bzCompressInit ( & s, self -> blockSize100k, 0, self -> workFactor );
    if ( bzerr != BZ_OK )
        return RC ( rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted );
    
    s.next_in = (void *)src;
    s.avail_in = ssize;
    s.next_out = dst;
    s.avail_out = *dsize;
    
    *dsize = 0;
    bzerr = BZ2_bzCompress(&s, BZ_FINISH);
    switch (bzerr)
    {
    case BZ_OK:
    case BZ_RUN_OK:
    case BZ_FINISH_OK:
    case BZ_STREAM_END:
        break;
    default:
#if _DEBUGGING
        fprintf(stderr, "BZ2_bzCompress: unexpected bzip2 error %i\n", bzerr);
#endif
        BZ2_bzCompressEnd ( & s );
        return RC(rcXF, rcFunction, rcExecuting, rcSelf, rcUnexpected);
    }

    assert ( s.total_out_hi32 == 0 );
    *dsize = s.total_out_lo32;


    BZ2_bzCompressEnd ( & s );

    return 0;
}

static
rc_t CC bzip_func ( void *Self, const VXformInfo *info,
    VBlobResult *dst, const VBlobData *src, VBlobHeader *hdr )
{
    rc_t rc;
    bzip_t *self = Self;

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

    rc = invoke_bzip2 ( self, dst -> data, & dsize, src -> data, ssize);
    if (rc == 0)
    {
        dst->elem_bits = 1;
        dst->byte_order = src->byte_order;
        if (dsize != 0)
            dst->elem_count = dsize << 3;
        else
            rc = RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
    }

    return rc;
}

/* bzip
 * function bzip2_fmt bzip #1.0 < * U32 blockSize100k, U32 workFactor > ( any in );
 */
VTRANSFACT_IMPL ( vdb_bzip, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    bzip_t *fself;

    int blockSize100k = 5;
    int workFactor = 0;

    if ( cp -> argc > 0 )
    {
        blockSize100k = cp -> argv [ 0 ] . data . i32 [ 0 ];
        if ( blockSize100k < 1 || blockSize100k > 9 )
            return RC(rcXF, rcFunction, rcConstructing, rcParam, rcInvalid);
        if ( cp -> argc > 1 )
        {
            workFactor = cp -> argv [ 1 ] . data . i32 [ 0 ];
            if ( workFactor > 250 )
                return RC(rcXF, rcFunction, rcConstructing, rcParam, rcInvalid);
        }
    }

    fself = malloc ( sizeof *fself );
    if ( fself == NULL )
        return RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
    fself -> blockSize100k = blockSize100k;
    fself -> workFactor = workFactor;

    rslt->self = fself;
    rslt->whack = free;
    rslt->variant = vftBlob;
    rslt->u.bf = bzip_func;
        
    return 0;
}
