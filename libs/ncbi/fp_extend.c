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
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <klib/pack.h>
#include <zlib.h>

struct self_t {
    uint32_t keep;
};

static unsigned int read_uint(const uint8_t **Src, bitsz_t *N) {
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

typedef struct {
	uint8_t 	mbits;/* mantissa bits to keep */
	uint8_t		sign; /* if set then mantissa is signed */
	uint8_t		version;
    uint8_t     exp_not_compressed;
} fp_trunc_key;

typedef struct {
	uint32_t	padding;
	uint32_t	count;
	fp_trunc_key cmn;
} fp_trunc_key_v0;

typedef struct {
	uint64_t	count;
	fp_trunc_key cmn;
} fp_trunc_key_v1;

static rc_t decode(const fp_trunc_key *self, float dst[], uint32_t dsize, const uint8_t *src, uint32_t ssize, uint32_t count) {
    rc_t rc;
    size_t dummy;
    const int8_t *exp;
    KDataBuffer buf;
    bitsz_t packed_bits
    
    if (ssize < sizeof(count))
        return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
    
    if (count < dsize)
        return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
    
    packed_bits = (size_t)count * self->mbits;
    if (((packed_bits + 7) >> 3) > ssize)
        return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);

    rc = Unpack(self->mbits, 32, src, 0, packed_bits, 0, dst, dsize * sizeof(dst[0]), &dummy);
    if (rc)
        return rc;
    
    ssize -= (packed_bits + 7) >> 3;
    exp = (const void *)&src[(packed_bits + 7) >> 3];
    
    if (self->exp_not_compressed) {
        memset(&buf, 0, sizeof(buf));
        rc = 0;
    }
    else {
        bitsz_t dummy2;
        
        rc = KDataBufferMake(&buf, 8, count);
        if (rc)
            return rc;
        rc = unzip(buf.base, count, &dummy2, exp, ssize);
        exp = buf.base;
    }
    if (rc == 0) {
        const uint32_t *man = (const uint32_t *)dst;
        unsigned i;
        
        if (self->sign == 0) {
            for (i = 0; i != count; ++i)
                dst[i] = ldexp(man[i], exp[i] - self->mbits);
        }
        else {
            const uint32_t sign_extend = ~((1UL << (self->mbits - 1)) - 1);
            
            for (i = 0; i != count; ++i) {
                float x = ldexp(man[i], exp[i] - self->mbits + 1);
                
                if (man[i] & sign_extend)
                    dst[i] = -x;
                else
                    dst[i] = x;
            }
        }

    }
    KDataBufferWhack(&buf);
    
    return 0;
}

static rc_t uncompress_f(const struct self_t *self, float dst[], uint32_t dsize, const uint8_t *src, uint32_t ssize) {
    const union {
        fp_trunc_key_v1 v1;
        fp_trunc_key_v0 v0;
    } *u;

    if (ssize < sizeof(*u))
        return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
    u = (const void *)&src[ssize - sizeof(*u)];

    switch (u->v0.cmn.version) {
    case 0:
        return decode(&u->v0.cmn, dst, dsize, src, ssize - sizeof(u->v0), u->v0.count);
    case 1:
        return decode( &u->v1.cmn, dst, dsize, src, ssize - sizeof(u->v1), (uint32_t)u->v1.count );
    default:
        return RC(rcXF, rcFunction, rcExecuting, rcData, rcBadVersion);
    }
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
    uint32_t ssize = (src->elem_count * src->elem_bits + 7) >> 3;
    
    rc = uncompress_f(Self, dst->data, dst->elem_count, src->data, ssize);
    if (rc == 0)
    	dst->byte_order = vboNative;
    
	return 0;
}

/* 
 function F32 NCBI:fp_extend #1.0 < U32 bits > ( NCBI:fp_encoded_t in );
 */
VTRANSFACT_IMPL(NCBI_fp_extend, 1, 0, 0)(const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp)
{
    struct self_t *self;
    
    if (cp->argc < 1)
        return RC(rcXF, rcFunction, rcConstructing, rcParam, rcInsufficient);

    if (cp->argc < 1)
        return RC(rcXF, rcFunction, rcConstructing, rcParam, rcExcessive);
        
    self = malloc(sizeof(*self));
    if (self == NULL)
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);

    self->keep = cp->argv[0].data.u32[0];
    rslt->self = self;
    rslt->whack = free;
    rslt->variant = vftBlob;
    rslt->u.bf = blob_func;

    return 0;
}
