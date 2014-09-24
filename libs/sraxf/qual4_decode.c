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
#include "qual4_codec.h"
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>

#include <assert.h>

static size_t qual4_decode(
                           qual4 *dst,
                           size_t dcount,
                           const uint8_t *src,
                           size_t ssize,
                           const int8_t qmin,
                           const int8_t qmax
) {
	int st;
	int st2;
    size_t i;
    size_t j;

	static const qual4 all_bad = { -5,  -5,  -5,  -5 };

    qual4 is_good;
    is_good [ 0 ] = qmax;
    is_good [ 1 ] = qmin;
    is_good [ 2 ] = qmin;
    is_good [ 3 ] = qmin;
	
	for (st = st2 = 0, j = i = 0; i != ssize && j < dcount; ++i) {
        int val = src[i] - 40;
        
		switch (st) {
		case 0:
            if (src[i] < known_bad) {
                dst[j][0] = val;
                st = 1;
            }
            else if (src[i] == known_bad)
                memcpy(&dst[j][0], all_bad, 4);
            else if (src[i] == known_good)
                memcpy(&dst[j][0], is_good, 4);
            else {
                st2 = src[i];
                st = 4;
            }
			break;
		case 1:
			dst[j][1] = val;
			++st;
			break;
		case 2:
			dst[j][2] = val;
			++st;
			break;
		case 3:
			dst[j][3] = val;
			st = 0;
			break;
        case 4:
            switch (st2) {
            case pattern_a_1:
                dst[j][0] = val;
				dst[j][1] = -val;
				dst[j][2] = qmin;
				dst[j][3] = qmin;
                break;
            case pattern_a_2:
                dst[j][0] = val;
				dst[j][1] = qmin;
				dst[j][2] = -val;
				dst[j][3] = qmin;
                break;
            case pattern_a_3:
                dst[j][0] = val;
				dst[j][1] = qmin;
				dst[j][2] = qmin;
				dst[j][3] = -val;
                break;
            case pattern_b_1:
                dst[j][0] = val;
				dst[j][1] = -val + 1;
				dst[j][2] = qmin;
				dst[j][3] = qmin;
                break;
            case pattern_b_2:
                dst[j][0] = val;
				dst[j][1] = qmin;
				dst[j][2] = -val + 1;
				dst[j][3] = qmin;
                break;
            case pattern_b_3:
                dst[j][0] = val;
				dst[j][1] = qmin;
				dst[j][2] = qmin;
				dst[j][3] = -val + 1;
                break;
            case pattern_c_1:
                dst[j][0] = val;
				dst[j][1] = -val - 1;
				dst[j][2] = qmin;
				dst[j][3] = qmin;
                break;
            case pattern_c_2:
                dst[j][0] = val;
				dst[j][1] = qmin;
				dst[j][2] = -val - 1;
				dst[j][3] = qmin;
                break;
            case pattern_c_3:
                dst[j][0] = val;
				dst[j][1] = qmin;
				dst[j][2] = qmin;
				dst[j][3] = -val - 1;
                break;
            default:
                return 0;
            }
            st = 0;
            break;
		}
        if (st == 0)
            ++j;
	}
	return j;
}

static
rc_t CC qual4_decode_func(
                       void *Self,
                       const VXformInfo *info,
                       VBlobResult *dst,
                       const VBlobData *src,
                       VBlobHeader *hdr
) {
    size_t rcount;
    int qmin = -40;
    int qmax = 40;
    
    if (hdr) {
        rc_t rc;
        uint8_t val;
        
        rc = VBlobHeaderOpPopHead(hdr, &val);
        if (rc == 0) {
            qmin = val - 40;
            rc = VBlobHeaderOpPopHead(hdr, &val);
            if (rc == 0) {
                qmax = val - 40;
            }
        }
    }
    
    rcount = qual4_decode( dst->data, dst->elem_count,
                           src->data, 
						   ( ( (size_t)src->elem_count * src->elem_bits + 7 ) >> 3 ),
                           qmin, qmax );
    
    if (rcount == dst->elem_count) {
        dst->byte_order = vboNone;
        return 0;
    }
    return RC(rcSRA, rcFunction, rcExecuting, rcData, rcInvalid);
}

/*
 * function NCBI:SRA:swapped_qual4 NCBI:SRA:qual4_decode ( NCBI:SRA:encoded_qual4 in );
 */
VTRANSFACT_IMPL(NCBI_SRA_qual4_decode, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftBlob;
    rslt->u.bf = qual4_decode_func;

    return 0;
}

static
rc_t CC legacy_qual4_decode_func ( void *self, const VXformInfo *info, VLegacyBlobResult *rslt, const KDataBuffer *src )
{
    unsigned dbytes = *(uint32_t *)src->base;
    
#if __BYTE_ORDER == __BIG_ENDIAN
    dbytes = bswap_32 (dbytes);
#endif
    if ((dbytes & 3) == 0) {
        rc_t rc;
        unsigned rcount;
        
        rslt->dst->elem_bits = 32;
        rc = KDataBufferResize(rslt->dst, dbytes >> 2);
        if (rc)
            return rc;

        rcount = (unsigned int)qual4_decode( rslt->dst->base, rslt->dst->elem_count,
                               ( ( const uint8_t * )src->base ) + 4, 
							   (unsigned int)( KDataBufferBytes( src ) - 4 ),
                               -40, 40);
        if (rcount == rslt->dst->elem_count) {
            rslt->byte_order = vboNone;
            return 0;
        }
    }
    return RC(rcSRA, rcFunction, rcExecuting, rcData, rcInvalid);
}

VTRANSFACT_IMPL ( NCBI_SRA_qual4_decompress_v1, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    VNoHdrBlobFunc f = legacy_qual4_decode_func;
    rslt->variant = vftLegacyBlob;
    rslt->u.bf = ( VBlobFunc ) f;

    return 0;
}

#if TESTING
int test_decode(const uint8_t src[], unsigned ssize, int8_t Y[], unsigned N) {
    size_t n = qual4_decode(Y, N / 4, src, ssize, -40, 40);
    
    return 0;
}
#endif
