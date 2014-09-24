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

#define SPECIAL ((uncompressed) << 6)

#define MAX_RUN_LEN ((size_t)(((size_t)1) << (5 + 3 * 7)))
#define HEAD_SIZE (4)

static __inline__
unsigned LENGTH_SIZE(size_t len) {
    return len < 1 ? 0 : len < 1UL << 6 ? 1 : len < 1UL << 13 ? 2 : len < 1UL << 20 ? 3 : len < 1UL << 27 ? 4 : 0;
}

static __inline__
int OUTPUT_SIZE(int type, int len) {
    switch (type) {
    case zero_run:
    case one_run:
        return LENGTH_SIZE(len);
    case byte_run:
        return LENGTH_SIZE(len) + 1;
    case uncompressed:
        return LENGTH_SIZE(len) + len;
    default:
        return len + 1;
    }
}

static __inline__
int f_HEADER(uint8_t HEAD[], uint8_t FLAG, int rlen) {
    if (rlen < 1UL << 5) {
        HEAD[0] = (FLAG << 6) | rlen;
        return 1;
    }
    if (rlen < 1UL << 12) {
        HEAD[0] = (FLAG << 6) | (1 << 5) | (rlen >> 7);
        HEAD[1] =                           rlen         & 0x7F;
        return 2;
    }
    if (rlen < 1UL << 19) {
        HEAD[0] = (FLAG << 6) | (1 << 5) | ( rlen >> 14);
        HEAD[1] =               (1 << 7) | ((rlen >>  7) & 0x7F);
        HEAD[2] =                            rlen        & 0x7F;
        return 3;
    }
        HEAD[0] = (FLAG << 6) | (1 << 5) | ( rlen >> 21);
        HEAD[1] =               (1 << 7) | ((rlen >> 14) & 0x7F);
        HEAD[2] =               (1 << 7) | ((rlen >>  7) & 0x7F);
        HEAD[3] =                            rlen        & 0x7F;
        return 4;
}

#define HEADER(Y, HEAD, FLAG, LEN) Y = f_HEADER(HEAD, FLAG, LEN)

static __inline__ int rle_write(uint8_t *dst, const uint8_t *dend, const uint8_t *src, int type, size_t len) {
	uint8_t head[HEAD_SIZE + 1];
	int i = 0;
	
	if (type == SPECIAL) {
		if (dst + len + 1 > dend)
			return 0;

		*dst++ = SPECIAL;
		memcpy(dst, src, len);
		return (int)len + 1;
	}
    if (len >= MAX_RUN_LEN) {
        i = rle_write(dst, dend, src, type, len >> 1);
        if (i == 0)
            return 0;
        if (type == uncompressed)
            src += len >> 1;
        return i + rle_write(dst + i, dend, src, type, len - (len >> 1));
    }
	
	HEADER(i, head, type, (int)len);

	if (type == byte_run)
		++i;
	if (dst + i > dend)
		return 0;
	switch (type) {
    case byte_run:
        head[i - 1] = *src;
    case zero_run:
    case one_run:
        memcpy(dst, head, i);
        return i;
    default:
        break;
	}
	if (dst + i + len > dend)
		return 0;
	memcpy(dst, head, i);
	memcpy(dst + i, src, len);
	return i + (int)len;
}

#define WRITE(SRC, TYPE, END) do { \
    int i; \
    i = rle_write(dst, dend, (SRC), (TYPE), (END) - (SRC)); \
    if (i == 0) { \
        i = rle_write(dst, dend, (SRC), SPECIAL, send - (SRC)); \
        if (i == 0) { \
            return -1; \
        } \
    } \
    dst += i; \
} while(0)

static int encode(uint8_t *Dst, unsigned dsize, unsigned *psize, const uint8_t *src, unsigned ssize) {
    uint8_t *dst = Dst;
	const uint8_t *const dend = dst + dsize;
	const uint8_t *const send = src + ssize;
	const uint8_t *cp = src, *mark = NULL;
	uint8_t last;
	int type;
    unsigned size1, size2, size3;
	
	while (last = *cp, ++cp != send) {
		if (*cp == last) {
			if (mark == NULL)
				mark = cp - 1;
			else
				continue;
		}
		else {
			if (mark == NULL)
				continue;
			if (mark != src) {
				if (mark - src == 1) {
					if (*src == 0)
						type = zero_run;
					else if (*src == 0xFF)
						type = one_run;
					else
						type = uncompressed;
				}
				else
					type = uncompressed;
				size1 = OUTPUT_SIZE(type, (int)( mark - src ) );
				size2 = OUTPUT_SIZE(*mark == 0 ? zero_run : *mark == 0xFF ? one_run : byte_run, (int)(cp - mark) );
				size3 = OUTPUT_SIZE(uncompressed, (int)(cp - src) );
				if (size1 + size2 >= size3) {
					mark = NULL;
					continue;
				}
				WRITE(src, type, mark);
			}
			WRITE(mark, *mark == 0 ? zero_run : *mark == 0xFF ? one_run : byte_run, cp);
			mark = NULL;
			src = cp;
		}
	}
	if (src != cp) {
		if (mark == NULL)
			WRITE(src, SPECIAL, cp);
		else {
			if (mark != src) {
				if (mark - src == 1) {
					if (*src == 0)
						type = zero_run;
					else if (*src == 0xFF)
						type = one_run;
					else
						type = uncompressed;
				}
				else
					type = uncompressed;
				WRITE(src, type, mark);
			}
			WRITE(mark, *mark == 0 ? zero_run : *mark == 0xFF ? one_run : byte_run, cp);
		}
	}
    *psize = (unsigned)(dst - Dst);
	return 0;
}

static
rc_t CC rlencode_func(
              void *Self,
              const VXformInfo *info,
              VBlobResult *dst,
              const VBlobData *src,
              VBlobHeader *hdr
) {
    unsigned sz;
    int rc;
    
    VBlobHeaderSetVersion(hdr, 0);
    rc = encode(dst->data, (unsigned)(((size_t)dst->elem_count * dst->elem_bits + 7) >> 3), &sz,
                src->data, (unsigned)(((size_t)src->elem_count * src->elem_bits + 7) >> 3));
    if (rc)
        return RC(rcVDB, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
    dst->elem_bits = 1;
    dst->byte_order = vboNone;
    dst->elem_count = sz << 3;

    return 0;
}

/* vdb:rlencode
 * function rle_fmt rlencode #1.0 ( any in )
 */
VTRANSFACT_IMPL(vdb_rlencode, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftBlob;
    rslt->u.bf = rlencode_func;
    
    return 0;
}

#if TESTING
int rle_test(void *dst, unsigned dsize, const void *src, unsigned ssize) {
    unsigned dummy;
    return encode(dst, dsize, &dummy, src, ssize);
}
#endif

