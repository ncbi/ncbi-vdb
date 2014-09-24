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

static size_t qual4_encode(
						   uint8_t *Dst,
                           size_t dsize,
						   const qual4 src [],
						   size_t count,
                           int8_t qmin,
                           int8_t qmax
                           )
{
	static const qual4 all_bad = { -5, -5, -5, -5 };
	uint8_t *dst = Dst;
    const uint8_t * const dend = & ( ( uint8_t* ) Dst ) [ dsize ];
	int i;
	qual4 
        pat_a_1, pat_a_2, pat_a_3,
        pat_b_1, pat_b_2, pat_b_3,
        pat_c_1, pat_c_2, pat_c_3;
    qual4 is_good;

    is_good[0] = qmax;
    is_good[1] = is_good[2] = is_good[3] = qmin;
	
	pat_a_1[2] = pat_a_1[3] = pat_a_2[1] = pat_a_2[3] = pat_a_3[1] = pat_a_3[2] = qmin;
	pat_b_1[2] = pat_b_1[3] = pat_b_2[1] = pat_b_2[3] = pat_b_3[1] = pat_b_3[2] = qmin;
	pat_c_1[2] = pat_c_1[3] = pat_c_2[1] = pat_c_2[3] = pat_c_3[1] = pat_c_3[2] = qmin;
	
	for (i = 0; i != count; ++i) {
		qual4 in_val;
		qual4 out_val;
		int codes;
		
        memcpy(in_val, src + i, 4);
		if (in_val[0] > qmax || in_val[0] < qmin ||
            in_val[1] > qmax || in_val[1] < qmin ||
            in_val[2] > qmax || in_val[2] < qmin ||
            in_val[3] > qmax || in_val[3] < qmin)
        {
			if (in_val[0] + 40 < 0 || in_val[0] + 40 >= cb_last)
				goto IS_BAD;
			else
				goto QUOTE_IT;
		}
		
		do {
			if (*(uint32_t *)in_val == *(uint32_t *)all_bad) {
			IS_BAD:
				codes = 1;
				out_val[0] = known_bad;
				break;
			}
			if (*(uint32_t *)in_val == *(uint32_t *)is_good) {
				codes = 1;
				out_val[0] = known_good;
				break;
			}
			
			codes = 2;
			out_val[1] = in_val[0] + 40;
			
			pat_a_3[3] = pat_a_2[2] = pat_a_1[1] = -(pat_a_1[0] = pat_a_2[0] = pat_a_3[0] = in_val[0]);
			if (*(uint32_t *)in_val == *(uint32_t *)pat_a_1) {
				out_val[0] = pattern_a_1;
				break;
			}
			if (*(uint32_t *)in_val == *(uint32_t *)pat_a_2) {
				out_val[0] = pattern_a_2;
				break;
			}
			if (*(uint32_t *)in_val == *(uint32_t *)pat_a_3) {
				out_val[0] = pattern_a_3;
				break;
			}
			
			pat_b_3[3] = pat_b_2[2] = pat_b_1[1] = -(pat_b_1[0] = pat_b_2[0] = pat_b_3[0] = in_val[0]) + 1;
			if (*(uint32_t *)in_val == *(uint32_t *)pat_b_1) {
				out_val[0] = pattern_b_1;
				break;
			}
			if (*(uint32_t *)in_val == *(uint32_t *)pat_b_2) {
				out_val[0] = pattern_b_2;
				break;
			}
			if (*(uint32_t *)in_val == *(uint32_t *)pat_b_3) {
				out_val[0] = pattern_b_3;
				break;
			}
			
			pat_c_3[3] = pat_c_2[2] = pat_c_1[1] = -(pat_c_1[0] = pat_c_2[0] = pat_c_3[0] = in_val[0]) - 1;
			if (*(uint32_t *)in_val == *(uint32_t *)pat_c_1) {
				out_val[0] = pattern_c_1;
				break;
			}
			if (*(uint32_t *)in_val == *(uint32_t *)pat_c_2) {
				out_val[0] = pattern_c_2;
				break;
			}
			if (*(uint32_t *)in_val == *(uint32_t *)pat_c_3) {
				out_val[0] = pattern_c_3;
				break;
			}
		QUOTE_IT:
			codes = 4;
			out_val[0] = in_val[0] + 40;
			out_val[1] = in_val[1] + 40;
			out_val[2] = in_val[2] + 40;
			out_val[3] = in_val[3] + 40;
		} while (0);
        
        if (dst + codes > dend)
            return dst + codes - Dst;
        
        memcpy(dst, out_val, codes);
		dst += codes;
	}
	return dst - Dst;
}

static
rc_t CC qual4_encode_func(
                       void *Self,
                       const VXformInfo *info,
                       VBlobResult *dst,
                       const VBlobData *Src,
                       VBlobHeader *hdr
) {
    size_t n;
    unsigned i;
    const int8_t *src = Src->data;
    int qmin = src[0];
    int qmax = src[0];
    rc_t rc;
    
    n = ((size_t)Src->elem_count * Src->elem_bits + 7) >> 3;
    for (i = 1; i < n; ++i) {
        int val = src[i];
        
        if (val < -40 || val > 40)
            continue;
        
        if (qmax < val)
            qmax = val;
        if (qmin > val)
            qmin = val;
    }
    if (qmax > 40)
        qmax = 40;
    if (qmin < -40)
        qmin = -40;
    
    rc = VBlobHeaderOpPushTail(hdr, qmin + 40);
    if (rc == 0) {
        rc = VBlobHeaderOpPushTail(hdr, qmax + 40);
        if (rc == 0) {
            n = qual4_encode(dst->data, (dst->elem_count * dst->elem_bits + 7) >> 3,
                             (const qual4 *)src, 
                             n / 4,
                             qmin, qmax );
            
            if (n > (dst->elem_count * dst->elem_bits + 7) >> 3)
                return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);

            dst->elem_count = n * 8 / dst->elem_bits;
            dst->byte_order = vboNone;
            
            return 0;
        }
    }
    return rc;
}

/*
 * function NCBI:SRA:encoded_qual4 NCBI:SRA:qual4_encode #1 ( NCBI:SRA:swapped_qual4 in )
 */
VTRANSFACT_IMPL(NCBI_SRA_qual4_encode, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftBlob;
    rslt->u.bf = qual4_encode_func;

    return 0;
}

#if TESTING
int test_encode(const int8_t Y[], unsigned N, uint8_t dst[], unsigned dsize) {
    size_t n = qual4_encode(dst, dsize, Y, N / 4, -40, 40);

    return n <= N ? n : 0;
}
#endif
