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
#include <compiler.h>
#include <klib/sort.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <sysalloc.h>

#include <byteswap.h>
#include <os-native.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <math.h>

#include <zlib.h>

#include <stdio.h>
#include <assert.h>

typedef struct {
    size_t size;
    size_t used;
    void *buf;
} szbuf;

static rc_t zlib_compress(szbuf *dst, const void *src, size_t ssize, int32_t strategy, int32_t level) {
    z_stream s;
    int zr;
    rc_t rc = 0;
    
    memset(&s, 0, sizeof(s));
    s.next_in = (void *)src;
    s.avail_in = (uInt)ssize;
    s.next_out = dst->buf;
    s.avail_out = (uInt)dst->size;
    
    dst->used = 0;
    
    zr = deflateInit2(&s, level, Z_DEFLATED, -15, 9, strategy);
    switch (zr) {
    case 0:
        break;
    case Z_MEM_ERROR:
        return RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
    case Z_STREAM_ERROR:
        return RC(rcVDB, rcFunction, rcExecuting, rcParam, rcInvalid);
    default:
        return RC(rcVDB, rcFunction, rcExecuting, rcSelf, rcUnexpected);
    }
    zr = deflate(&s, Z_FINISH);
    switch (zr) {
    case Z_STREAM_END:
        break;
    case Z_OK:
        s.total_out = 0;
        break;
    default:
        rc = RC(rcVDB, rcFunction, rcExecuting, rcSelf, rcUnexpected);
        break;
    }
    zr = deflateEnd(&s);
    if ( zr != Z_OK && s.total_out != 0 )
        rc = RC(rcVDB, rcFunction, rcExecuting, rcSelf, rcUnexpected);
    if ( rc == 0 ) {
        dst->used = (uint32_t)s.total_out;
    }
    return rc;
}

static rc_t zlib_decompress(void *dst, size_t dsize, size_t *psize, const void *src, size_t ssize) {
    z_stream s;
    int zr;
    rc_t rc;
    
    memset(&s, 0, sizeof(s));
    s.next_in = (void *)src;
    s.avail_in = (uInt)ssize;
    s.next_out = dst;
    s.avail_out = (uInt)dsize;
    
    zr = inflateInit2(&s, -15);
    switch (zr) {
    case 0:
        break;
    case Z_MEM_ERROR:
        return RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
    default:
        return RC(rcVDB, rcFunction, rcExecuting, rcNoObj, rcUnexpected);
    }
    zr = inflate(&s, Z_FINISH);
    switch (zr) {
    case Z_STREAM_END:
    case Z_OK:
        *psize = s.total_in;
        rc = 0;
        break;
    case Z_BUF_ERROR:
    case Z_NEED_DICT:
    case Z_DATA_ERROR:
        rc = RC(rcVDB, rcFunction, rcExecuting, rcData, rcCorrupt);
        break;
    case Z_MEM_ERROR:
        rc = RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        break;
    default:
        rc = RC(rcVDB, rcFunction, rcExecuting, rcNoObj, rcUnexpected);
        break;
    }
    if (inflateEnd(&s) == Z_OK) return rc;
    
    return rc == 0 ? RC(rcVDB, rcFunction, rcExecuting, rcData, rcCorrupt) : rc;
}


#define STYPE int8_t
#define USTYPE uint8_t
#define ENCODE encode_i8
#define DECODE decode_i8
#include "irzip.impl.h"
#undef ENCODE
#undef DECODE
#undef STYPE
#undef USTYPE

#define STYPE int16_t
#define USTYPE uint16_t
#define ENCODE encode_i16
#define DECODE decode_i16
#include "irzip.impl.h"
#undef ENCODE
#undef DECODE
#undef STYPE
#undef USTYPE

#define STYPE int32_t
#define USTYPE uint32_t
#define ENCODE encode_i32
#define DECODE decode_i32
#define TRY2SERIES 1
#include "irzip.impl.h"
#undef TRY2SERIES
#undef ENCODE
#undef DECODE
#undef STYPE
#undef USTYPE

#define STYPE int64_t
#define USTYPE uint64_t
#define ENCODE encode_i64
#define DECODE decode_i64
#define TRY2SERIES 1
#include "irzip.impl.h"
#undef TRY2SERIES
#undef ENCODE
#undef DECODE
#undef STYPE
#undef USTYPE

#define STYPE uint8_t
#define USTYPE uint8_t
#define ENCODE encode_u8
#define DECODE decode_u8
#include "irzip.impl.h"
#undef ENCODE
#undef DECODE
#undef STYPE
#undef USTYPE

#define STYPE uint16_t
#define USTYPE uint16_t
#define ENCODE encode_u16
#define DECODE decode_u16
#include "irzip.impl.h"
#undef ENCODE
#undef DECODE
#undef STYPE
#undef USTYPE

#define STYPE uint32_t
#define USTYPE uint32_t
#define ENCODE encode_u32
#define DECODE decode_u32
#define TRY2SERIES 1
#include "irzip.impl.h"
#undef TRY2SERIES
#undef ENCODE
#undef DECODE
#undef STYPE
#undef USTYPE

#define STYPE uint64_t
#define USTYPE uint64_t
#define ENCODE encode_u64
#define DECODE decode_u64
#define TRY2SERIES 1
#include "irzip.impl.h"
#undef TRY2SERIES
#undef ENCODE
#undef DECODE
#undef STYPE
#undef USTYPE


typedef rc_t (*encode_f)(uint8_t dst[], size_t dsize, size_t *used,
                         int64_t *Min, int64_t *Slope, uint8_t *series_count,uint8_t *planes, const void *Y, unsigned N);

struct self_t {
    encode_f f;
};

static
struct self_t selfs[8] = {
    { (encode_f)encode_u8 },
    { (encode_f)encode_i8 },
    { (encode_f)encode_u16 },
    { (encode_f)encode_i16 },
    { (encode_f)encode_u32 },
    { (encode_f)encode_i32 },
    { (encode_f)encode_u64 },
    { (encode_f)encode_i64 },
};



static
rc_t CC irzip(
               void *Self,
               const VXformInfo *info,
               VBlobResult *dst,
               const VBlobData *src,
               VBlobHeader *hdr
) {
    rc_t rc = 0;
    const struct self_t *self = Self;
    size_t dsize;
    int64_t min[2],slope[2];
    uint8_t planes;
    uint8_t series_count = 1;

    assert(src->elem_count >> 32 == 0);
    assert(((dst->elem_count * dst->elem_bits + 7) >> 3) >> 32 == 0);
    dsize = (uint32_t)((dst->elem_count * dst->elem_bits + 7) >> 3);
    
   
    rc = self->f(dst->data, dsize, &dsize, min, slope, &series_count, &planes, src->data, (unsigned)src->elem_count);
#if 0
    if(/*src->elem_bits ==64 &&*/ dsize > 0){
	printf("%2d:irzip_elem_bits=%.8f\telem=%d\tstart=%ld\tslope=%lx\tplanes=%d\n",(int)src->elem_bits,dsize*8./src->elem_count,(int)src->elem_count,min,slope,planes);
    }
#endif
    
    VBlobHeaderSetVersion(hdr, (series_count > 1)?3:2);
    if ( rc == 0 ) {
	rc = VBlobHeaderOpPushTail(hdr, planes);
	if ( rc == 0) {
		rc = VBlobHeaderArgPushTail(hdr, min[0]);
		if ( rc == 0 ) {
			rc = VBlobHeaderArgPushTail(hdr, slope[0]);
			if ( rc == 0 ){
				if(series_count > 1){
					rc = VBlobHeaderArgPushTail(hdr, min[1]);
					if(rc == 0) rc = VBlobHeaderArgPushTail(hdr, slope[1]);
				}
				dst->byte_order = vboNative;
				dst->elem_bits = 1;
				dst->elem_count = dsize << 3;
			}
		}
	}
   }
    return rc;
}

/* 
 function izip_fmt izip #2.1 ( izip_set in )
 */
VTRANSFACT_IMPL(vdb_izip, 2, 1, 1) (const void *Self, const VXfactInfo *info,
                                     VFuncDesc *rslt, const VFactoryParams *cp,
                                     const VFunctionParams *dp )
{
/*
* 2.1.1: bug fix for #VDB-539, bad conversions in encode_u64
*/

    if (dp->argc != 1) {
#if _DEBUGGING
        fprintf(stderr, "dp->argc = %u != 1\n", dp->argc);
#endif
        return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
    }

    rslt->variant = vftBlob;
    rslt->u.bf = irzip;

    switch (dp->argv[0].desc.domain) {
    case vtdInt:
        switch (dp->argv[0].desc.intrinsic_bits) {
        case 8:
            rslt->self = &selfs[1];
            break;
        case 16:
            rslt->self = &selfs[3];
            break;
        case 32:
            rslt->self = &selfs[5];
            break;
        case 64:
            rslt->self = &selfs[7];
            break;
        default:
#if _DEBUGGING
            fprintf(stderr, "intrinsic_bits = %u != (8|16|32|64)\n", dp->argv[0].desc.intrinsic_bits);
#endif
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            break;
        }
        break;
    case vtdUint:
        switch (dp->argv[0].desc.intrinsic_bits) {
        case 8:
            rslt->self = &selfs[0];
            break;
        case 16:
            rslt->self = &selfs[2];
            break;
        case 32:
            rslt->self = &selfs[4];
            break;
        case 64:
            rslt->self = &selfs[6];
            break;
        default:
#if _DEBUGGING
            fprintf(stderr, "intrinsic_bits = %u != (8|16|32|64)\n", dp->argv[0].desc.intrinsic_bits);
#endif
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            break;
        }
        break;
    default:
#if _DEBUGGING
        fprintf(stderr, "domain != vtdInt or vtdUint\n");
#endif
        return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
    }

    return 0;
}

typedef rc_t (*decode_f)(void *dst, unsigned N, int64_t* min, int64_t* slope, uint8_t series_count,uint8_t planes, const uint8_t src[], size_t ssize);

static
decode_f uselfs[8] = {
    (decode_f)decode_u8,
    (decode_f)decode_i8,
    (decode_f)decode_u16,
    (decode_f)decode_i16,
    (decode_f)decode_u32,
    (decode_f)decode_i32,
    (decode_f)decode_u64,
    (decode_f)decode_i64,
};

static
rc_t iunzip_func_v1(
                    void *Self,
                    const VXformInfo *info,
                    VBlobResult *dst,
                    const VBlobData *src,
                    int64_t* min,int64_t* slope,uint8_t series_count,
                    uint8_t planes
                    )
{
    uint32_t ssize;
    rc_t rc;
    
    assert(dst->elem_count >> 32 == 0);
    assert(((src->elem_count * src->elem_bits + 7) >> 3) >> 32 == 0);
    ssize = (uint32_t)((src->elem_count * src->elem_bits + 7) >> 3);
    
    dst->byte_order = vboNative;
    
    rc = uselfs[(uintptr_t)(Self)](dst->data, (unsigned)dst->elem_count, min, slope, series_count, planes,src->data, ssize);
    return rc;
}

extern rc_t CC iunzip_func_v0(
                              void *Self,
                              const VXformInfo *info,
                              VBlobResult *dst,
                              const VBlobData *src
                              );
    
static
rc_t CC iunzip(
            void *Self,
            const VXformInfo *info,
            VBlobResult *dst,
            const VBlobData *src,
            VBlobHeader *hdr
            )
{
    switch (VBlobHeaderVersion(hdr)) {
    case 0:
        return iunzip_func_v0(Self, info, dst, src);
    case 1: 
    case 2: 
    case 3:
	{
        int64_t min[2],slope[2];
        uint8_t planes;
	uint8_t series_count=1;
        rc_t rc;
        
        rc = VBlobHeaderOpPopHead(hdr, &planes);
        if (rc == 0) {
            rc = VBlobHeaderArgPopHead(hdr, min);
            if (rc == 0) {
		rc = VBlobHeaderArgPopHead(hdr, slope);
		if (rc != 0) slope[0] = 0;
		else {
			rc = VBlobHeaderArgPopHead(hdr, min + 1);
			if(rc==0) rc= VBlobHeaderArgPopHead(hdr, slope+1);
			if(rc==0) series_count=2;
		}
                return iunzip_func_v1(Self, info, dst, src, min,slope, series_count, planes);
            }
	}
        return rc;
    }
    default:
        return RC(rcVDB, rcFunction, rcExecuting, rcParam, rcBadVersion);
    }
}

/* 
 function izip_set iunzip #2.1 ( izip_fmt in )
 */
VTRANSFACT_IMPL(vdb_iunzip, 2, 1, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftBlob;
    rslt->u.bf = iunzip;
    
    switch (info->fdesc.desc.domain) {
    case vtdInt:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
            rslt->self = (void *)1;
            break;
        case 16:
            rslt->self = (void *)3;
            break;
        case 32:
            rslt->self = (void *)5;
            break;
        case 64:
            rslt->self = (void *)7;
            break;
        default:
#if _DEBUGGING
            fprintf(stderr, "intrinsic_bits = %u != (8|16|32|64)\n", info->fdesc.desc.intrinsic_bits);
#endif
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            break;
        }
        break;
    case vtdUint:
        switch (info->fdesc.desc.intrinsic_bits) {
        case 8:
            rslt->self = (void *)0;
            break;
        case 16:
            rslt->self = (void *)2;
            break;
        case 32:
            rslt->self = (void *)4;
            break;
        case 64:
            rslt->self = (void *)6;
            break;
        default:
#if _DEBUGGING
            fprintf(stderr, "intrinsic_bits = %u != (8|16|32|64)\n", info->fdesc.desc.intrinsic_bits);
#endif
            return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
            break;
        }
        break;
    default:
#if _DEBUGGING
        fprintf(stderr, "domain != vtdInt or vtdUint\n");
#endif
        return RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
    }
    return 0;
}

#if TESTING
int test_encode(const int32_t Y[], unsigned N) {
    uint8_t *dst;
    size_t dsize;
    size_t temp;
    rc_t rc = 0;
    int64_t min,slope;
    uint8_t planes;
    
    temp = N * sizeof(Y[0]);
    assert(temp >> 32 == 0);
    dst = malloc(dsize = (unsigned)temp);
    if (dst == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);

    rc = encode_i32(dst, dsize, &dsize, &min, &slope, &planes, Y, N);
    
    {
        int32_t *X;
        
        X = malloc(N * sizeof(Y[0]));
        if (X) {
            rc = decode_i32( X, N, min, slope, planes, dst, dsize);
            if (rc == 0) {
                rc = memcmp(Y, X, N * sizeof(Y[0])) == 0 ? 0 : RC(rcXF, rcFunction, rcExecuting, rcFunction, rcInvalid);
            }
            free(X);
        }
    }

    free(dst);
    if (rc)
        fprintf(stdout, "test failed!\n");
    return rc;
}
#endif

