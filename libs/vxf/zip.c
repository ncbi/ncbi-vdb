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
#include <zlib.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef Z_MIN_LEVEL
#define Z_MIN_LEVEL Z_DEFAULT_COMPRESSION
#endif

#ifndef Z_MAX_LEVEL
#define Z_MAX_LEVEL Z_BEST_COMPRESSION
#endif

#ifndef Z_MIN_STRATEGY
#define Z_MIN_STRATEGY Z_DEFAULT_STRATEGY
#endif

#ifndef Z_MAX_STRATEGY
#define Z_MAX_STRATEGY Z_RLE
#endif

#include <stdio.h>
#include <assert.h>

#define BUFFER_GROWTH_RATE (64 * 1024)

struct self_t {
    int32_t strategy;
    int32_t level;
};

#if _DEBUGGING
/*
static void debug_print_z_stream(const z_stream *s) {
    printf("avail_in: %lu, avail_out: %lu, total_in: %lu, total_out: %lu\n",
           (unsigned long)s->avail_in,
           (unsigned long)s->avail_out,
           (unsigned long)s->total_in,
           (unsigned long)s->total_out);
}
*/
#endif

static rc_t invoke_zlib(void *dst, uint32_t *dsize, const void *src, uint32_t ssize, int32_t strategy, int32_t level) {
    z_stream s;
    int zr;
    rc_t rc = 0;
    
    memset(&s, 0, sizeof(s));
    s.next_in = (void *)src;
    s.avail_in = ssize;
    s.next_out = dst;
    s.avail_out = *dsize;
    
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
#if _DEBUGGING
        fprintf(stderr, "deflateInit2: unexpected zlib error %i: %s (strategy: %i, level: %i)\n", zr, s.msg, strategy, level);
#endif
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
#if _DEBUGGING
        fprintf(stderr, "deflate: unexpected zlib error %i: %s\n", zr, s.msg);
#endif
        rc = RC(rcXF, rcFunction, rcExecuting, rcSelf, rcUnexpected);
        break;
    }
    zr = deflateEnd(&s);
    if (zr != Z_OK && s.total_out != 0) {
#if _DEBUGGING
        fprintf(stderr, "deflateEnd: unexpected zlib error %i: %s\n", zr, s.msg);
#endif
        rc = RC(rcXF, rcFunction, rcExecuting, rcSelf, rcUnexpected);
    }
    if (rc == 0) {
        assert(s.total_out <= UINT32_MAX);
        *dsize = (uint32_t)s.total_out;
    }
    return rc;
}

static
rc_t CC zip_func(
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

    rc = invoke_zlib ( dst -> data, & dsize, src -> data, ssize, self->strategy, self->level);
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
void CC vxf_zip_wrapper( void *ptr )
{
	free( ptr );
}

/* zip
 * function zlib_fmt zip #1.0 < * I32 strategy, I32 level > ( any in );
 */
VTRANSFACT_IMPL(vdb_zip, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    struct self_t *ctx;

    int strategy = Z_RLE;
    int level = Z_BEST_SPEED;

    if ( cp -> argc > 0 )
    {
        strategy = cp -> argv [ 0 ] . data . i32 [ 0 ];
        if ( strategy < Z_MIN_STRATEGY || strategy > Z_MAX_STRATEGY )
            return RC(rcXF, rcFunction, rcConstructing, rcParam, rcInvalid);
        if ( cp -> argc > 1 )
        {
            level = cp -> argv [ 1 ] . data . i32 [ 0 ];
            if ( level < Z_MIN_LEVEL || level > Z_MAX_LEVEL )
                return RC(rcXF, rcFunction, rcConstructing, rcRange, rcInvalid);
        }
    }

    ctx = malloc(sizeof(*ctx));
    if (ctx) {
        ctx->strategy = strategy;
        ctx->level = level;
       
        rslt->self = ctx;
        rslt->whack = vxf_zip_wrapper;
        rslt->variant = vftBlob;
        rslt->u.bf = zip_func;
        
        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
