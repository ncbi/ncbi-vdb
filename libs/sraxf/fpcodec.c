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

#include <vdb/xform.h>
#include <vdb/schema.h>
#include <vdb/vdb-priv.h>
#include <klib/data-buffer.h>
#include <klib/pack.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <endian.h>
#include <byteswap.h>
#include <string.h>
#include <zlib.h>
#include <assert.h>
#include <math.h>

static
rc_t
hufDecodeBytes_zlib(void *dst, size_t dsize, bitsz_t *psize, const void *src, bitsz_t ssize)
{
    int result;
    rc_t rc;
	z_stream c_stream; /* decompression stream */

	memset(&c_stream,0,sizeof(c_stream));

	c_stream.next_in   = (Bytef*)src;
	c_stream.next_out  = (Bytef*)dst;
	c_stream.avail_in  = (uInt)( ( ssize + 7 ) >> 3 );
	c_stream.avail_out = (uInt)dsize;

	if (psize)
        *psize = 0;

    result = inflateInit(&c_stream);
    switch (result)
    {
    case Z_OK:
            
        result = inflate ( & c_stream, Z_FINISH );
        switch (result)
        {
        case Z_STREAM_END:
                
            result = inflateEnd ( & c_stream );
            if ( result == Z_OK )
            {
                if ( psize != NULL )
                    * psize = c_stream.total_out << 3;
                return 0;
            }
            return RC ( rcXF, rcFunction, rcDecoding, rcParam, rcInvalid );
                
        case Z_OK:
        case Z_BUF_ERROR:
            rc = RC (rcXF, rcFunction, rcDecoding, rcBuffer, rcInsufficient );
            break;

        case Z_NEED_DICT:
        case Z_DATA_ERROR:
            rc = RC (rcXF, rcFunction, rcDecoding, rcData, rcCorrupt);
            break;
        
        case Z_MEM_ERROR:
            rc = RC(rcXF, rcFunction, rcDecoding, rcMemory, rcExhausted);
            break;
        
        case Z_STREAM_ERROR:
        default:
            rc = RC (rcXF, rcFunction, rcDecoding, rcNoObj, rcUnexpected);
            break;
        }

        inflateEnd ( & c_stream );
        break;
            
    case Z_MEM_ERROR:
        rc = RC(rcXF, rcFunction, rcDecoding, rcMemory, rcExhausted);
        break;
            
    case Z_VERSION_ERROR:
    default:
        rc = RC (rcXF, rcFunction, rcDecoding, rcNoObj, rcUnexpected);
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
	uint32_t	count;
	fp_trunc_key cmn;
} fp_trunc_key_v0;

typedef struct {
	uint64_t	count;
	fp_trunc_key cmn;
} fp_trunc_key_v1;

static
rc_t CC fp_extend ( void *self, const VXformInfo *info,
    VLegacyBlobResult *rslt, const KDataBuffer *in )
{
    rc_t rc;
    bitsz_t bsize;
    uint64_t i, count;
    uint32_t count32;
    size_t tmpsize, psize;

    int32_t *dst;
    int8_t  *dexp;

    KDataBuffer *out = rslt -> dst;
    uint32_t keep = ( uint32_t ) ( size_t ) self;

    fp_trunc_key tkey;
    size_t ssize = KDataBufferBytes ( in );
    const uint8_t *src = in -> base;

    if(ssize < sizeof(tkey))
        return RC(rcXF, rcFunction, rcUnpacking, rcData, rcUnexpected);
    ssize -= sizeof(tkey); 
    memcpy(&tkey,src+ssize,sizeof(tkey));

    if(tkey.mbits > 24 || tkey.mbits > keep) 
        return RC(rcXF, rcFunction, rcUnpacking, rcData, rcUnexpected);

    switch ( tkey . version )
    {
    case 0:
        if ( ssize < sizeof count32 )
            return RC(rcXF, rcFunction, rcUnpacking, rcData, rcUnexpected);
        ssize -= sizeof count32;
        memcpy ( & count32, src+ssize, sizeof count32 );
        count = count32;
        break;
    case 1:
        if ( ssize < sizeof count )
            return RC(rcXF, rcFunction, rcUnpacking, rcData, rcUnexpected);
        ssize -= sizeof count;
        memcpy ( & count, src+ssize, sizeof count );
        break;
    default:
        return RC ( rcXF, rcFunction, rcUnpacking, rcBlob, rcBadVersion );
    }

    psize = ( ( (size_t)count * tkey.mbits + 7 ) >> 3 );
    if(psize > ssize)
        return RC(rcXF, rcFunction, rcUnpacking, rcData, rcUnexpected);

    out -> elem_bits = 32;
    rc = KDataBufferResize ( out, (uint32_t)count );
    if ( rc != 0 )
        return rc;

    dst = out -> base;    

    /* unpack mantissa */
    rc = Unpack ( tkey.mbits, 32, src, 0, (bitsz_t)( count * tkey.mbits ), 
		          NULL, dst, KDataBufferBytes ( out ), & tmpsize );
    if ( rc != 0 )
        return rc;

    assert ( KDataBufferBytes ( out ) == tmpsize );

    rslt -> byte_order = vboNative;
    ssize -= psize;
        
    dexp = malloc( (size_t)count );
    if(dexp==NULL)
        return RC(rcXF, rcFunction, rcUnpacking, rcMemory, rcExhausted);

    if ( tkey.exp_not_compressed )
        memcpy( dexp, src+psize, (size_t)count );
    else
        /*** uncompress the exponent ***/
        rc = hufDecodeBytes_zlib( dexp, (size_t)count, &bsize, src+psize, ssize << 3 );

    if(rc == 0)
    {
        float *fdst;

        /* combine mantissa and exponent */
        if(tkey.sign==0)
        {
            for (fdst=(float*)dst, i = 0; i < count; ++ i )
                fdst[i]=ldexpf(dst[i],dexp[i]-tkey.mbits);/*** the highest bit is not a sign ***/
        }
        else
        {
            uint32_t sign_extend= ~((1<<(tkey.mbits-1))-1);
            for (fdst=(float*)dst, i = 0; i < count; ++ i )
            {
                if(dst[i]&sign_extend)
                {
                    float m=(float)(int)(dst[i]|sign_extend);
                    fdst[i]=ldexpf(m,dexp[i]-tkey.mbits+1);
                }
                else
                {
                    float m=(float)dst[i];
                    fdst[i]=ldexpf(m,dexp[i]-tkey.mbits+1);
                }
            }
        }
    }

    free(dexp);
    
    return rc;
}

/* F32 NCBI:fp_extend #1.0 < U32 bits > ( NCBI:fp_encoded_t in );
 */
VTRANSFACT_IMPL ( NCBI_fp_extend, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    VNoHdrBlobFunc f = fp_extend;

    /* test bit range */
    uint32_t bits = cp -> argv [ 0 ] . data . u32 [ 0 ];
    if ( bits < 1 || bits > 23 )
        return RC ( rcVDB, rcFunction, rcConstructing, rcRange, rcIncorrect );

    /* pass bits to function */
    rslt -> self = ( void* ) ( size_t ) bits;
    rslt -> u . bf = ( VBlobFunc ) f;
    rslt -> variant = vftLegacyBlob;

    return 0;
}
