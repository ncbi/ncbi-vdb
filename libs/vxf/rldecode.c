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

#include <assert.h>

enum header_flags {
	zero_run = 0,
	one_run = 1,
	byte_run = 2,
	uncompressed = 3
};

static int decode(uint8_t *dst, unsigned dsize, const uint8_t src[], unsigned ssize) {
    int st, type;
    unsigned size, i, j;
    
    for (st = type = 0, i = j = size = 0; i != ssize; ++i) {
        int x = src[i];
        
        switch (st) {
        case 0:
            type = x >> 6;
            size = x & 0x1F;
            if ((x & 0x20) == 0)
                goto DECODED;
            st = 1;
            break;
        case 1:
            size = (size << 7) | (x & 0x7F);
            if ((x & 0x80) == 0)
                goto DECODED;
            st = 2;
            break;
        case 2:
            size = (size << 7) | (x & 0x7F);
            if ((x & 0x80) == 0)
                goto DECODED;
            st = 3;
            break;
        case 3:
            size = (size << 7) | (x & 0x7F);
            if ((x & 0x80) == 0)
                goto DECODED;
            return 1;
        case 4:
            dst[j++] = x;
            if (--size == 0)
                st = 0;
            break;
        case 5:
            memset(dst + j, x, size);
            j += size;
            st = 0;
            break;
        }
        continue;
    DECODED:
        if (j + size > dsize)
            return 1;
        switch (type) {
        case uncompressed:
            if (size == 0) {
                size = ssize - i - 1;
                if (j + size > dsize)
                    return 1;
                memcpy(dst + j, src + i + 1, size);
                return 0;
            }
            st = 4;
            break;
        case zero_run:
            memset(dst + j, 0, size);
            j += size;
            st = 0;
            break;
        case one_run:
            memset(dst + j, 0xFF, size);
            j += size;
            st = 0;
            break;
        default:
            st = 5;
            break;
        }
    }
    return 0;
}

static
rc_t CC rldecode_func(
                   void *Self,
                   const VXformInfo *info,
                   VBlobResult *dst,
                   const VBlobData *src,
                   VBlobHeader *hdr
) {
    int rslt;
    
    rslt = decode(dst->data, (unsigned)(((size_t)dst->elem_count * dst->elem_bits + 7) >> 3),
                  src->data, (unsigned)(((size_t)src->elem_count * src->elem_bits + 7) >> 3));
    if (rslt)
        return RC(rcVDB, rcFunction, rcExecuting, rcData, rcCorrupt);
    return 0;
}

/* vdb:rldecode
 * function any rldecode #1.0 ( rle_fmt in )
 */
VTRANSFACT_IMPL(vdb_rldecode, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftBlob;
    rslt->u.bf = rldecode_func;
    
    return 0;
}

#if TESTING
int rld_test(void *dst, unsigned dsize, const void *src, unsigned ssize) {
    return decode(dst, dsize, src, ssize);
}
#endif
