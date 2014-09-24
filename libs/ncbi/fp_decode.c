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

#include <ncbi/extern.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>
#include <vdb/types.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <compiler.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <zlib.h>

static unsigned int read_uint( const uint8_t **Src, bitsz_t *N ) {
	unsigned y = 0, x;
	const uint8_t *src = *Src;
	unsigned n = (unsigned int)*N;
	
	do {
		if (!n)
			break;
		x = *src++;
		n -= 8;
		y <<= 7;
		y |= x & 0x7F;
	} while(x & 0x80);
	*Src = src;
	*N = n;
	return y;	
}

static rc_t unzip(void *dst, size_t dsize, bitsz_t *psize, const void *src, bitsz_t ssize) {
    int result;
    rc_t rc;
	z_stream c_stream; /* decompression stream */
    
	memset(&c_stream,0,sizeof(c_stream));
    
	c_stream.next_in   = (Bytef*)src;
	c_stream.next_out  = (Bytef*)dst;
	c_stream.avail_in  = (uInt)((ssize + 7)>> 3);
	c_stream.avail_out = (uInt)dsize;
    
	if (psize)
        *psize = 0;
    
    result = inflateInit(&c_stream);
    switch (result) {
    case Z_OK:
        result = inflate(&c_stream, Z_FINISH);
        switch (result) {
        case Z_STREAM_END:
            result = inflateEnd(& c_stream);
            if (result == Z_OK) {
                if (psize != NULL)
                    *psize = c_stream.total_out << 3;
                return 0;
            }
            return RC(rcXF, rcFunction, rcExecuting, rcParam, rcInvalid);

        case Z_OK:
        case Z_BUF_ERROR:
            rc = RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
            break;
            
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
            rc = RC(rcXF, rcFunction, rcExecuting, rcData, rcCorrupt);
            break;
            
        case Z_MEM_ERROR:
            rc = RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
            
        case Z_STREAM_ERROR:
        default:
            rc = RC(rcXF, rcFunction, rcExecuting, rcParam, rcInvalid);
            break;
        }
        inflateEnd(& c_stream);
        break;
        
    case Z_MEM_ERROR:
        rc = RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        break;
        
    case Z_VERSION_ERROR:
    default:
        rc = RC(rcXF, rcFunction, rcExecuting, rcParam, rcInvalid);
        break;
    }
    
    return rc;
}

static rc_t uncompress_f(float dst[], uint32_t dsize, const uint8_t *src, bitsz_t ssize) {
	uint32_t count;
	uint16_t version;
	rc_t rc;
	uint8_t *enc = NULL;
    unsigned actsize;
	
	version = read_uint(&src, &ssize);
	if (!ssize)
		return RC(rcXF, rcFunction, rcExecuting, rcData, rcCorrupt);
	if (version == 0x0100) {
		count = read_uint(&src, &ssize);
		actsize = count * sizeof(float);
		if (dsize < actsize)
			return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
		if (!ssize)
			return RC(rcXF, rcFunction, rcExecuting, rcData, rcCorrupt);
        
		read_uint(&src, &ssize);
		if (!ssize)
			return RC(rcXF, rcFunction, rcExecuting, rcData, rcCorrupt);
        rc = 0;
    }
    else {
        size_t esize;
        
        if (version != 0x0101)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcBadVersion);

        count = read_uint(&src, &ssize);
        actsize = count * sizeof(float);
        if (dsize < actsize)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
        if (!ssize)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcCorrupt);

        esize = read_uint(&src, &ssize);
        if (!ssize)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcCorrupt);
        
        enc = malloc(esize);
        if (enc == NULL)
            return RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);

        rc = unzip(enc, esize, &esize, src, ssize);
        src = enc;
    }
    if (rc == 0) {
        float last;
        unsigned i;
        unsigned j;
        
        for (j = i = 0, last = 0.0; i != count; ++i, j += 4) {
            union { float f; uint32_t u; } y;
            
            y.u = (((((src[j + 0] << 8) | src[j + 1]) << 8) | src[j + 2]) << 8) | src[j + 3];
            if ( isnan(y.f) ) {
                j += 4;
                y.u = (((((src[j + 0] << 8) | src[j + 1]) << 8) | src[j + 2]) << 8) | src[j + 3];
                last = 0.0;
            }
            dst[i] = (last += y.f);
        }
    }
    if (enc)
        free(enc);
	return rc;
}

static
rc_t blob_func(
               void *Self,
               const VXformInfo *info,
               VBlobResult *dst,
               const VBlobData *src,
               VBlobHeader *hdr               
) {
    rc_t rc;
    size_t dsize = ((size_t)dst->elem_count * dst->elem_bits + 7) >> 3;
    bitsz_t ssize = (size_t)src->elem_count * src->elem_bits;

    rc = uncompress_f(dst->data, dsize, src->data, ssize);
    if (rc == 0)
    	dst->byte_order = vboNative;

	return 0;
}

/* 
 function F32 NCBI:fp_decode #1.0(fp_encoded_t in);
 */
VTRANSFACT_IMPL(NCBI_fp_decode, 1, 0, 0)(const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp) {
    rslt->variant = vftBlob;
    rslt->u.bf = blob_func;
	return 0;
}
