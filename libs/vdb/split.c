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
#include "blob.h"

#include "blob-headers.h"
#include "page-map.h"
#include "blob-priv.h"
#include "prod-priv.h"
#include "xform-priv.h"

#include <vdb/xform.h>
#include <vdb/schema.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


static
int64_t read_64(const uint8_t **Src, const uint8_t *end, int scale, rc_t *rc) {
	int64_t x = 0;
	uint8_t c;
	const uint8_t *src = *Src;
	int n = 0;
	
	switch (scale) {
	case 1:
		scale = 2;
		break;
	case 2:
		scale = 3;
		break;
	case 4:
		scale = 5;
		break;
	case 8:
		scale = 10;
		break;
	default:
		return 0;
	}
	*rc = 0;
	do {
		if (src >= end) {
			*rc = RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
			return 0;
		}
		if (++n > scale) {
			*rc = RC(rcVDB, rcBlob, rcConstructing, rcData, rcInvalid);
			return 0;
		}
		
		c = *src++;
		if ((c & 0x80) != 0)
			break;
		x <<= 7;
		x |= c;
	} while (1);
    
	c &= 0x7F;
	if ((c & 0x40) != 0) {
		c &= 0x3F;
		x <<= 6;
		x |= c;
		x = -x;
	}
	else {
		x <<= 6;
		x |= c;
	}
	*Src = src;
	return x;
}

static rc_t VBlobSplit_v1_validate_header(const uint8_t *inp, const uint8_t *in_end, unsigned n, const uint8_t **data) {
    rc_t rc;
    unsigned i;
    
    for (i = 0; i != n; ++i) {
        read_64(&inp, in_end, 4, &rc);
        if (rc != 0)
            return rc;
    }
    *data = inp;
    return 0;
}

static rc_t VBlobSplit_v1(VBlob **lhs, const VBlob *self, uint32_t index, uint32_t elem_bits) {
    const uint8_t *inp;
    const uint8_t *in_end;
    const uint8_t *data;
    size_t insize;
    rc_t rc;
    unsigned block_count;
    unsigned i;
    
    inp = self->data.base;
    assert(self->data.bit_offset == 0);
    insize = KDataBufferBytes(&self->data);
    assert(insize > 0);
    in_end = inp + insize;
    
    block_count = (unsigned int)read_64(&inp, in_end, 1, &rc);
    if (rc != 0)
        return rc;
    
    rc = VBlobSplit_v1_validate_header(inp, in_end, block_count, &data);
    if (rc != 0)
        return rc;
    
    for (i = 0; i != block_count; ++i) {
        unsigned block_size = (unsigned int)read_64(&inp, in_end, 4, &rc);
        uint8_t bits;
        uint8_t byte_order;
        uint16_t row_len;
        int64_t start_id;
        uint32_t id_span;
        const char *datatype;
        
        if (data >= in_end || data + 1 >= in_end)
            return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
        bits = data[0];
        byte_order = data[1];
        data += 2;
        
        row_len = (uint16_t)read_64(&data, in_end, 2, &rc);
        if (rc != 0)
            return rc;
        
        start_id = read_64(&data, in_end, 8, &rc);
        if (rc != 0)
            return rc;
        
        id_span = (uint32_t)read_64(&data, in_end, 4, &rc);
        if (rc != 0)
            return rc;
        
        if (data + block_size >= in_end)
            return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
        
        datatype = (const char *)data;
        data = memchr(datatype, 0, block_size);
        if (data == NULL)
            return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInvalid);
        
        ++data;
        if (data >= in_end)
            return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
        
        if (i == index) {
            if (((block_size * 8) - bits) % elem_bits != 0)
                rc = RC(rcVDB, rcBlob, rcConstructing, rcData, rcInvalid);
            else {
                VBlob *y;
                
                rc = VBlobNew(&y, start_id, start_id + id_span, "split");
                if (rc == 0) {
                    KDataBuffer temp;
                    
                    y->byte_order = byte_order;
                    rc = KDataBufferCast(&self->data, &temp, 8, false);
                    if (rc == 0) {
                        KDataBufferSub(&temp, &temp, (uint32_t)( data - (const uint8_t *)self->data.base ), block_size);
                        KDataBufferCast(&temp, &temp, 1, true);
                        temp.elem_count -= bits;
                        rc = KDataBufferCast(&temp, &y->data, elem_bits, false);
                        KDataBufferWhack(&temp);
                        if (rc == 0) {
                            rc = PageMapNewFixedRowLength(&y->pm, BlobRowCount(y), row_len);
                            if (rc == 0) {
                                *lhs = y;
                                return 0;
                            }
                        }
                    }
                    TRACK_BLOB( VBlobRelease, ((VBlob*)y) );
                    (void)VBlobRelease(y);
                }
            }
            return rc;
        }
        data += block_size;
    }
    return RC(rcVDB, rcBlob, rcConstructing, rcParam, rcInvalid);
}

static rc_t VBlobSplit_v2(VBlob **lhs, const VBlob *self, uint32_t index, uint32_t elem_bits, const VBlobHeader *hdr) {
    unsigned doffset;
    unsigned dlength;
    unsigned num_inputs;
    unsigned num_maps;
    unsigned map;
    unsigned i;
    int64_t val;
    rc_t rc;
    VBlob *y;
    PageMap *pm;
    BlobHeaders *headers;
    
    rc = VBlobHeaderArgPopHead(hdr, &val);
    if (rc)
        return rc;
    num_inputs = (unsigned)val;
    
    if ( (unsigned)index > num_inputs )
        return RC(rcVDB, rcBlob, rcConstructing, rcParam, rcInvalid);
    
    rc = VBlobHeaderArgPopHead(hdr, &val);
    if (rc)
        return rc;
    num_maps = (unsigned)val;
    
    for (map = 0, i = 0; i != num_inputs; ++i) {
        rc = VBlobHeaderArgPopHead(hdr, &val);
        if (rc)
            break;
        if (i == index)
            map = (unsigned)val;
    }
    if (rc)
        return rc;
    if (map > num_maps || map < 0)
        return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInvalid);
    
    if (self->spmc == 0) {
        ((VBlob *)self)->spmc = malloc(sizeof(*self->spmc) - sizeof(self->spmc->pm) + num_maps * sizeof(self->spmc->pm[0]));
        if (self->spmc) {
            self->spmc->n = num_maps;
            for (doffset = i = 0; i != num_maps; ++i) {
                rc = VBlobHeaderArgPopHead(hdr, &val);
                if (rc)
                    break;
                rc = PageMapDeserialize(&self->spmc->pm[i], &((uint8_t *)self->data.base)[doffset], (uint32_t)val, BlobRowCount(self));
                if (rc)
                    break;
                doffset += (unsigned)val;
            }
        }
        else
            return RC(rcVDB, rcBlob, rcConstructing, rcMemory, rcExhausted);
    }
    else {
        for (doffset = i = 0; i != num_maps; ++i) {
            rc = VBlobHeaderArgPopHead(hdr, &val);
            if (rc)
                break;
            doffset += (unsigned)val;
        }
    }
    if (rc)
        return rc;
    pm = self->spmc->pm[map];
    
    for (i = 0; rc == 0 && i != num_inputs; ++i) {
        rc = VBlobHeaderArgPopHead(hdr, &val);
        if ( rc )
            break;
        if ( i == index ) {
            if (val)
                rc = BlobHeadersCreateFromData(&headers, &((uint8_t *)self->data.base)[doffset], (uint32_t)val);
            else
                headers = NULL;
        }
        doffset += (unsigned)val;
    }
    
    for (dlength = 0, i = 0; rc == 0 && i != num_inputs; ++i) {
        rc = VBlobHeaderArgPopHead(hdr, &val);
        if (rc)
            break;
        if (i == index) {
            dlength = (unsigned)val;
            break;
        }
        doffset += ( ( (unsigned)val + 7 ) >> 3 );
    }
    if (rc)
        return rc;
    assert(dlength != 0);
    
    rc = VBlobNew(&y, self->start_id, self->stop_id, "split");
    if (rc == 0) {
        KDataBuffer buffer;
        
        rc = KDataBufferCast(&self->data, &buffer, 8, false);
        if (rc == 0) {
            KDataBufferSub( &buffer, &buffer, (uint32_t)doffset, (uint32_t)( (dlength + 7) >> 3 ) );
            KDataBufferCast(&buffer, &buffer, 1, true);
            buffer.elem_count = (uint32_t)dlength;
            rc = KDataBufferCast(&buffer, &y->data, elem_bits, false);
            KDataBufferWhack(&buffer);
            if (rc == 0) {
                y->pm = pm;
                PageMapAddRef(y->pm);
                y->headers = (BlobHeaders *)headers;
                
                *lhs = y;
                return 0;
            }
        }
        TRACK_BLOB( VBlobRelease, ((VBlob*)y) );
        (void)VBlobRelease(y);
    }
    return rc;
}

LIB_EXPORT rc_t CC VBlobBufferSplit(VBlob **lhs, const VBlob *self, uint32_t index, uint32_t elem_bits) {
    rc_t rc;
    
    if (self->headers) {
        const VBlobHeader *hdr = BlobHeadersGetHeader(self->headers);
		
        if (hdr) {
            switch (VBlobHeaderVersion(hdr)) {
            case 1:
                rc = VBlobSplit_v2(lhs, self, index, elem_bits, hdr);
                break;
            default:
                rc = RC(rcVDB, rcBlob, rcConstructing, rcData, rcBadVersion);
                break;
            }
            VBlobHeaderRelease(hdr);
            return rc;
        }
        return RC(rcVDB, rcBlob, rcConstructing, rcMemory, rcExhausted);
    }
    else {
        return VBlobSplit_v1(lhs, self, index, elem_bits);
    }
}

typedef struct self_t {
    uint32_t index;
} self_t;

/* split
 *  any split #1.0 < U32 idx > ( merged_fmt in );
 *  any NCBI:split #1.0 < U32 idx > ( merged_t in );
 */
static
rc_t CC split_impl( void *self,
                const VXformInfo *info,
                int64_t row_id,
                VBlob **rslt,
                uint32_t argc,
                const VBlob *argv[] )
{
    return VBlobBufferSplit(rslt, argv[0], ((const self_t *)self)->index, VTypedescSizeof(&info->prod->desc));
}

static
void CC vfunc_free ( void * self )
{
    free ( self );
}

VTRANSFACT_BUILTIN_IMPL(vdb_split, 1, 0, 0) (const void *self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->self = malloc(sizeof(self_t));
    if (rslt->self) {
        ((self_t *)rslt->self)->index = cp->argv[0].data.u32[0];
        rslt->whack = vfunc_free;
        rslt->variant = vftBlobN;
        VFUNCDESC_INTERNAL_FUNCS(rslt)->bfN = split_impl;
        return 0;
    }
    return RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
