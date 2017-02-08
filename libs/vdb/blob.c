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
 
#define TRACK_REFERENCES 0

#include "page-map.h"
#include "blob-headers.h"
#include "blob.h"
#include "blob-priv.h"
#include <klib/rc.h>
#include <klib/defs.h>
#include <byteswap.h>
#include <klib/data-buffer.h>
#include <klib/container.h>
#include <klib/vlen-encode.h>
#include <klib/vector.h>
#include <kdb/btree.h>
#include <vdb/schema.h>
#include <vdb/xform.h>
#include <klib/log.h>
#include <sysalloc.h>
#include <bitstr.h>

#include <kproc/timeout.h>
#include <kproc/lock.h>
#include <kproc/cond.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <os-native.h>

#include <stdio.h> /* temp for debugging */
#define PHYSPROD_INDEX_OFFSET 1000000000
#if _DEBUGGING
void VBlobCheckIntegrity ( const VBlob *self )
{
    if ( self != NULL )
    {
        rc_t rc = KDataBufferCheckIntegrity ( & self -> data );
        if ( rc != 0 )
        {
            fprintf ( stderr, "AAAAAH!\n" );
        }
    }
}
#endif

rc_t VBlobNew ( VBlob **lhs, int64_t start_id, int64_t stop_id, const char *name ) {
    VBlob *y;
    
    if ( name == NULL )
        name = "";
#if VBLOG_HAS_NAME
    *lhs = y = malloc(sizeof(*y) + strlen(name));
#else
    *lhs = y = calloc(1, sizeof(*y));
#endif
    if (y) {
        KRefcountInit(&y->refcount, 1, "VBlob", "new", name);
        y->start_id = start_id;
        y->stop_id = stop_id;
        y->data.elem_bits = 1;
        y->byte_order = vboNative;
#if VBLOG_HAS_NAME
        y->pm = NULL;
        y->headers = NULL;
        y->spmc = NULL;
        memset(&y->data, 0, sizeof(y->data));
        y->no_cache = 0;
        strcpy(&(((char *)y->name)[0]), name);
#endif
        
        return 0;
    }
    return RC(rcVDB, rcBlob, rcConstructing, rcMemory, rcExhausted);
}
rc_t VBlobNewAsArray(struct VBlob **lhs, int64_t start_id, int64_t stop_id, uint32_t rowlen, uint32_t elem_bits)
{
	VBlob *y;
	rc_t rc = VBlobNew(&y,start_id,stop_id,NULL);
	if(rc == 0){
		rc = KDataBufferMake(&y->data,elem_bits,rowlen*(stop_id-start_id+1));
		if(rc == 0){
			rc=PageMapNewFixedRowLength(&y->pm,stop_id-start_id+1,rowlen);
			if(rc == 0){
				*lhs = y;
				return 0;
			}
			PageMapRelease(y->pm);
		}
		VBlobRelease(y);
	}
	return rc;
}

static rc_t VBlobDestroy( VBlob *that ) {
    if (that->spmc) {
        int i;
        
        for (i = 0; i != that->spmc->n; ++i)
            PageMapRelease(that->spmc->pm[i]);
        free(that->spmc);
    }
    KDataBufferWhack(&that->data);
    BlobHeadersRelease(that->headers);
    PageMapRelease(that->pm);
    free(that);
    return 0;
}

static rc_t decode_header_byte_v2(
                                  uint8_t header_byte,
                                  uint8_t *variant,
                                  uint8_t *adjust,
                                  VByteOrder *byte_order
) {
    *adjust = (8 - (header_byte & 7)) & 7; header_byte >>= 3;
    *byte_order = (header_byte & 1) ? vboBigEndian : vboLittleEndian;
    header_byte >>= 1;
    *variant = header_byte & 3;
    header_byte >>= 2;
    return header_byte == 2 ? 0 : RC(rcVDB, rcBlob, rcReading, rcData, rcBadVersion);
}

static rc_t decode_header_v2_0(
                               const uint8_t *src,
                               uint64_t ssize,
                               uint32_t *hdr_size,
                               uint32_t *map_size,
                               uint32_t *offset
) {
    *offset = 3;
    if (ssize < *offset)
        return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
    
    *hdr_size = src[1];
    *map_size = src[2];
    return 0;
}

static rc_t decode_header_v2_1(
                               const uint8_t *src,
                               uint64_t ssize,
                               uint32_t *hdr_size,
                               uint32_t *map_size,
                               uint32_t *offset
) {
    *offset = 4;
    if (ssize < *offset)
        return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
    
    *hdr_size = src[1];
    *map_size = (uint32_t)src[2] | ((uint32_t)src[3] << 8);
    return 0;
}

static rc_t decode_header_v2_2(
                               const uint8_t *src,
                               uint64_t ssize,
                               uint32_t *hdr_size,
                               uint32_t *map_size,
                               uint32_t *offset
) {
    *offset = 6;
    if (ssize < *offset)
        return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
    
    *hdr_size = src[1];
    *map_size = (uint32_t)src[2] | ((uint32_t)src[3] << 8) | ((uint32_t)src[4] << 16) | ((uint32_t)src[5] << 24);
    return 0;
}

static rc_t decode_header_v2_3(
                               const uint8_t *src,
                               uint64_t ssize,
                               uint32_t *hdr_size,
                               uint32_t *map_size,
                               uint32_t *offset
) {
    *offset = 9;
    if (ssize < *offset)
        return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
    
    *hdr_size = (uint32_t)src[1] | ((uint32_t)src[2] << 8) | ((uint32_t)src[3] << 16) | ((uint32_t)src[4] << 24);
    *map_size = (uint32_t)src[5] | ((uint32_t)src[6] << 8) | ((uint32_t)src[7] << 16) | ((uint32_t)src[8] << 24);
    return 0;
}

static rc_t decode_header_v2(
                             const uint8_t *src,
                             uint64_t ssize,
                             uint32_t *hdr_size,
                             uint32_t *map_size,
                             uint32_t *offset,
                             uint8_t *adjust,
                             VByteOrder *byte_order
) {
    rc_t rc;
    uint8_t variant;
    
    if (ssize == 0)
        return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);

    rc = decode_header_byte_v2(src[0], &variant, adjust, byte_order);
    if (rc)
        return rc;

    switch (variant) {
    case 0:
        return decode_header_v2_0(src, ssize, hdr_size, map_size, offset);
    case 1:
        return decode_header_v2_1(src, ssize, hdr_size, map_size, offset);
    case 2:
        return decode_header_v2_2(src, ssize, hdr_size, map_size, offset);
    case 3:
        return decode_header_v2_3(src, ssize, hdr_size, map_size, offset);
    default:
        return RC(rcVDB, rcBlob, rcConstructing, rcData, rcBadVersion);
    }
}

static rc_t encode_header_v1(
                             uint8_t *dst,
                             uint64_t dsize,
                             uint64_t *used,
                             uint32_t row_length,
                             bitsz_t data_size,
                             VByteOrder byte_order
) {
    /* byte-order goes in bits 0..1 */
    uint8_t header_byte = byte_order & 3;
    if ( header_byte == vboNative )
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        header_byte = ( uint8_t) vboLittleEndian;
#else
        header_byte = ( uint8_t) vboBigEndian;
#endif
    }

    /* blob size adjust goes in bits 2..4 */
    header_byte |= ( ( 8 - ( data_size & 7 ) ) & 7 ) << 2;
    
    /* row-length code goes in bits 5..6 */
    if ( row_length == 1 ) {
        header_byte |= 3 << 5;
        * used = 1;
        if ( dsize < * used )
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
    }
    else if (row_length < 0x100) {
        *used = 2;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
        dst[1] = ( uint8_t ) row_length;
    }
    else if (row_length < 0x10000) {
        header_byte |= 1 << 5;
        *used = 3;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
        dst[1] = ( uint8_t ) row_length;
        dst[2] = ( uint8_t ) ( row_length >> 8 );
    }
    else {
        header_byte |= 2 << 5;
        *used = 5;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
        dst[1] = ( uint8_t ) row_length;
        dst[2] = ( uint8_t ) ( row_length >> 8 );
        dst[3] = ( uint8_t ) ( row_length >> 16 );
        dst[4] = ( uint8_t ) ( row_length >> 24 );
    }
    return 0;
}

static rc_t encode_header_v2(
                             uint8_t *dst,
                             uint64_t dsize,
                             uint64_t *used,
                             uint64_t hdr_size,
                             uint64_t map_size,
                             bitsz_t data_size
) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t header_byte = 0x80 | ( (uint8_t)data_size & 7 );
#else
    uint8_t header_byte = 0x88 | ( (uint8_t)data_size & 7 );
#endif
    
    assert(hdr_size >> 32 == 0);
    assert(map_size >> 32 == 0);
    
    if ((hdr_size >> 8) == 0) {
        if ((map_size >> 8) == 0) {
            *used = 3;
            if (dsize < *used)
                return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
            
            dst[0] = header_byte;
            dst[1] = hdr_size;
            dst[2] = map_size;
        }
        else if ((map_size >> 16) == 0) {
            *used = 4;
            if (dsize < *used)
                return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
            
            dst[0] = header_byte | 0x10;
            dst[1] = hdr_size;
            dst[2] = map_size;
            dst[3] = map_size >> 8;
        }
        else {
            *used = 6;
            if (dsize < *used)
                return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);

            dst[0] = header_byte | 0x20;
            dst[1] = hdr_size;
            dst[2] = map_size;
            dst[3] = map_size >> 8;
            dst[4] = map_size >> 16;
            dst[5] = map_size >> 24;
        }
    }
    else {
        *used = 9;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);

        dst[0] = header_byte | 0x30;

        dst[1] = hdr_size;
        dst[2] = hdr_size >> 8;
        dst[3] = hdr_size >> 16;
        dst[4] = hdr_size >> 24;

        dst[5] = map_size;
        dst[6] = map_size >> 8;
        dst[7] = map_size >> 16;
        dst[8] = map_size >> 24;
    }
    return 0;
}

static
    rc_t PageMapProcessRequestLock(PageMapProcessRequest *self)
{
    rc_t rc = 0;
    if(self)
    {
        /*** no plans to wait here the thread should be released by now ****/
        rc = KLockAcquire(self->lock);
        if(rc == 0){
            if(self->state != ePMPR_STATE_NONE){ /*** thread is not released yet **/
                assert(0); /** should not happen ***/
                KLockUnlock(self->lock);
                rc=RC(rcVDB, rcPagemap, rcConstructing, rcThread, rcBusy);
            }
        }
    }
    else
        rc = RC(rcVDB,rcPagemap, rcConstructing, rcSelf, rcNull);

    return rc;
}

static
void PageMapProcessRequestLaunch(PageMapProcessRequest *self)
{
	KConditionSignal ( self -> cond );
	KLockUnlock(self -> lock);
}

rc_t PageMapProcessGetPagemap(const PageMapProcessRequest *cself,struct PageMap **pm)
{
	rc_t rc=RC(rcVDB,rcPagemap, rcConstructing, rcSelf, rcNull);
        if(cself){
	    PageMapProcessRequest *self=(PageMapProcessRequest*)cself;
	    if(self->lock == NULL){
		/** NOT LOCKABLE **/
		rc=0;
	    } else if((rc = KLockAcquire ( self->lock ))==0){
CHECK_AGAIN:
		switch(self->state){
		 case ePMPR_STATE_DESERIALIZE_REQUESTED:
			/*fprintf(stderr,"Waiting for pagemap %p\n",cself->lock);*/
			rc = KConditionWait ( self -> cond, self -> lock );
                        goto CHECK_AGAIN;
		 case ePMPR_STATE_DESERIALIZE_DONE:
			assert(self->pm);
			/*fprintf(stderr,"Pagemap %p Used R:%6d|DR:%d|LR:%d\n",self->lock, self->pm->row_count,self->pm->data_recs,self->pm->leng_recs);*/
			*pm=self->pm;
			self->pm = NULL;
			KDataBufferWhack(&self->data);
			self->row_count = 0;
			self->state = ePMPR_STATE_NONE;
			KConditionSignal(self->cond);
			KLockUnlock(self -> lock);
			break;
		case ePMPR_STATE_NONE: /* not requested */
			KLockUnlock(self -> lock);
		    rc = 0;
		    break;
		 default: /** should never happen ***/
			assert(0);
			KLockUnlock(self -> lock);
			return RC(rcVDB, rcPagemap, rcConverting, rcParam, rcInvalid );
		}
	    }
        }
        return rc;
}


static
rc_t VBlobCreateFromData_v2(
                            VBlob **lhs,
                            const KDataBuffer *data,
                            int64_t start_id, int64_t stop_id,
                            uint32_t elem_bits, PageMapProcessRequest *pmpr
) {
    uint64_t ssize = data->elem_count;
    uint32_t hsize;
    uint32_t msize;
    uint32_t offset;
    VByteOrder byte_order;
    uint8_t adjust;
    VBlob *y;
    uint32_t data_offset;
    uint32_t pagemap_offset;
    bitsz_t databits;
    uint32_t elem_count;
    uint8_t *src = data->base;
    rc_t rc;
    
    rc = decode_header_v2(src, ssize, &hsize, &msize, &offset, &adjust, &byte_order);
    if (rc)
        return rc;

    if (ssize < offset + hsize + msize)
        return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
    
    pagemap_offset = offset + hsize;
    data_offset = pagemap_offset + msize;
    assert(data_offset <= ssize);
    ssize -= data_offset;
    databits = (ssize << 3) - adjust;
    assert(databits % elem_bits == 0);
    elem_count = (uint32_t)( databits / elem_bits );

    rc = VBlobNew(&y, start_id, stop_id, NULL);
    TRACK_BLOB (VBlobNew, y);
    if (rc == 0) {
        if (hsize)
            rc = BlobHeadersCreateFromData(&y->headers, src+offset , hsize);
        if (rc == 0) {
            if (msize > 0) {
                if(pmpr != NULL && PageMapProcessRequestLock(pmpr)==0) {
                    KDataBufferSub(data, &pmpr->data, pagemap_offset, msize);
                    pmpr->row_count = BlobRowCount(y);
                    pmpr->state = ePMPR_STATE_DESERIALIZE_REQUESTED;
                    /*fprintf(stderr,"Pagemap %p Requested R:%6d|SZ:%d|%ld:%ld\n",pmpr->lock, pmpr->row_count,msize,start_id, stop_id);*/
                    PageMapProcessRequestLaunch(pmpr);
                }
                else {
                    KDataBuffer tdata;
                    KDataBufferSub(data, &tdata, pagemap_offset, msize);
                    rc = PageMapDeserialize(&y->pm, tdata.base,tdata.elem_count, BlobRowCount(y));
                    KDataBufferWhack(&tdata);
                }
            }
            if (rc == 0) {
                KDataBufferSub(data, &y->data, data_offset, ssize);
                y->data.elem_bits = elem_bits;
                y->data.elem_count = elem_count;
                y->byte_order = byte_order;
                *lhs = y;
                return 0;
            }
        }
        (void)VBlobRelease(y);
        TRACK_BLOB (VBlobRelease, y);
    }
    return rc;
}

static
rc_t VBlobCreateFromData_v1(
                            VBlob **lhs,
                            const KDataBuffer *data,
                            int64_t start_id, int64_t stop_id,
                            uint32_t elem_bits
) {
    const uint8_t *src = data->base;
    uint64_t ssize = data->elem_count;
    uint8_t header;
    rc_t rc;
    VBlob *y;
    VByteOrder byte_order;
    uint32_t offset;
    int adjust;
    int rls; /* row length size */
    uint64_t row_len;
    bitsz_t databits;
    
    if (ssize == 0)
        return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
    
    header = *src;
    byte_order = header & 3; header >>= 2;
    adjust = header & 7;
    header >>= 3;
    rls = header & 3;
    
    /* convert rls from a code to an actual length */
    rls = "\x01\x02\x04\x00" [ rls ];

    /* adjust offset */
    offset = rls + 1;

    /* handle special code where row length is implicitly 1 */
    if ( rls == 0 )
        row_len = 1;

    /* ensure sufficient header bytes */
    else if ( ssize < offset )
        return RC(rcVDB, rcBlob, rcConstructing, rcData, rcInsufficient);
    else
    {
        /* produce little-endian 64-bit row-length */
        row_len = 0;
        memmove ( & row_len, & src [ 1 ], rls );

#if __BYTE_ORDER != __LITTLE_ENDIAN
        /* correct for big-endian */
        row_len = bswap_64 ( row_len );
#endif
    }

    ssize -= offset;
    databits = (ssize << 3) - adjust;
    assert(databits % elem_bits == 0);

    rc = VBlobNew(&y, start_id, stop_id, NULL);
    TRACK_BLOB (VBLobNew, y);
    if (rc == 0) {

        uint64_t row_count = BlobRowCount ( y );

        /* test for badly formed row-length */
        if ( rls == 4 )
        {
            assert ( row_len != 0 );
            if ( row_len * row_count != databits / elem_bits )
            {
                /* we can fix a length if we know the count */
                if ( row_count != 0 )
                    row_len = ( databits / elem_bits ) / row_count;
                else
                {
                    /* rely on code to handle legacy blobs in prod-cmn.c:VFunctionProdCallByteswap */
                    row_len = 0;
                }
            }
        }

        if ( row_len != 0 )
            rc = PageMapNewFixedRowLength( &y->pm, row_count, row_len );
        if (rc == 0) {
            KDataBufferSub(data, &y->data, offset, ssize);
            y->data.elem_bits = elem_bits;
            y->data.elem_count = (uint32_t)( databits / elem_bits );
            y->byte_order = byte_order;
			
            *lhs = y;
            return 0;
        }
        /* like a call to VBlobRelease (y); */
        TRACK_BLOB (VBlobRelease-free, y);
        free(y);
    }
    return rc;
}

static void VBlobOptimize_UnRLE_uint8_t(VBlob **vblobp)
{
	VBlob   *sblob  = *vblobp;
	VBlob *vblob;
	rc_t rc = VBlobNewAsArray(&vblob,sblob -> start_id, sblob -> stop_id,sblob->pm->length[0],sblob->data.elem_bits);
	if( rc == 0){
		uint32_t i,j;
		uint8_t *src=sblob->data.base;
		uint8_t *dst=vblob->data.base;
		for(i=j=0;i<sblob->pm->data_recs;i++){
			row_count_t data_len=sblob->pm->data_run[i];
			while(data_len > 0){
				assert(j < sblob->pm->row_count);
				dst[j++] = src[i];
				data_len--;
			}
		}
		vblob->pm->optimized = eBlobPageMapOptimizedSucceeded;
		VBlobRelease(sblob);
		(*vblobp) = vblob;
	}
}
static void VBlobOptimize_UnRLE_uint16_t(VBlob **vblobp)
{
	VBlob   *sblob  = *vblobp;
	VBlob *vblob;
	rc_t rc = VBlobNewAsArray(&vblob,sblob -> start_id, sblob -> stop_id,sblob->pm->length[0],sblob->data.elem_bits);
	if( rc == 0) {
		uint32_t i,j;
		uint16_t *src=sblob->data.base;
		uint16_t *dst=vblob->data.base;
		for(i=j=0;i<sblob->pm->data_recs;i++){
			row_count_t data_len=sblob->pm->data_run[i];
			while(data_len > 0){
				assert(j < sblob->pm->row_count);
				dst[j++] = src[i];
				data_len--;
			}
		}
		vblob->pm->optimized = eBlobPageMapOptimizedSucceeded;
		VBlobRelease(sblob);
		(*vblobp) = vblob;
	}
}
static void VBlobOptimize_UnRLE_uint32_t(VBlob **vblobp)
{
	VBlob   *sblob  = *vblobp;
	VBlob *vblob;
	rc_t rc = VBlobNewAsArray(&vblob,sblob -> start_id, sblob -> stop_id,sblob->pm->length[0],sblob->data.elem_bits);
	if( rc == 0) {
		uint32_t i,j;
		uint32_t *src=sblob->data.base;
		uint32_t *dst=vblob->data.base;
		for(i=j=0;i<sblob->pm->data_recs;i++){
			row_count_t data_len=sblob->pm->data_run[i];
			while(data_len > 0){
				assert(j < sblob->pm->row_count);
				dst[j++] = src[i];
				data_len--;
			}
		}
		vblob->pm->optimized = eBlobPageMapOptimizedSucceeded;
		VBlobRelease(sblob);
		(*vblobp) = vblob;
	}
}
static void VBlobOptimize_UnRLE_uint64_t(VBlob **vblobp)
{
	VBlob   *sblob  = *vblobp;
	VBlob *vblob;
	rc_t rc = VBlobNewAsArray(&vblob,sblob -> start_id, sblob -> stop_id,sblob->pm->length[0],sblob->data.elem_bits);
	if( rc == 0) {
		uint32_t i,j;
		uint64_t *src=sblob->data.base;
		uint64_t *dst=vblob->data.base;
		for(i=j=0;i<sblob->pm->data_recs;i++){
			row_count_t data_len=sblob->pm->data_run[i];
			while(data_len > 0){
				assert(j < sblob->pm->row_count);
				dst[j++] = src[i];
				data_len--;
			}
		}
		vblob->pm->optimized = eBlobPageMapOptimizedSucceeded;
		VBlobRelease(sblob);
		(*vblobp) = vblob;
	}
}


void VBlobPageMapOptimize ( VBlob **vblobp)
{
	VBlob   *sblob  = *vblobp;
	PageMap *pm     = sblob->pm;
	if(pm->optimized != eBlobPageMapOptimizedNone) return; /* do not optimize previously optimized blobs */
	pm->optimized = eBlobPageMapOptimizedFailed; /*** prevent future optimization if none of the algorithms succeeds ***/

	if(pm->leng_recs == 1)
    {
		if( pm->length[0] * sblob->data.elem_bits == 8 /** 1 byte data ***/
		    &&  pm->data_recs < pm->row_count       /** rle was used ***/
		    &&  pm->data_recs * 6 > pm->row_count){ /** but not super efficiently **/
			/*printf("OPTIMIZATION: UnRLE(8)\n");*/
			VBlobOptimize_UnRLE_uint8_t(vblobp); 
			return;
		}
		if( pm->length[0] * sblob->data.elem_bits == 16 /** 2 byte data ***/
		    &&  pm->data_recs < pm->row_count       /** rle was used ***/
		    &&  pm->data_recs * 2 > pm->row_count){ /** but not super efficiently **/
			/*printf("OPTIMIZATION: UnRLE(16)\n");*/
			VBlobOptimize_UnRLE_uint16_t(vblobp);
			return;
		}
		if( pm->length[0] * sblob->data.elem_bits == 32 /** 4 byte data ***/
		    &&  pm->data_recs < pm->row_count       /** rle was used ***/
		    &&  pm->data_recs * 15 > pm->row_count * 10 ){ /** but not super efficiently **/
			/*printf("OPTIMIZATION: UnRLE(32)\n");*/
			VBlobOptimize_UnRLE_uint32_t(vblobp);
			return;
		}
		if( pm->length[0] * sblob->data.elem_bits == 64 /** 8 byte data ***/
		    &&  pm->data_recs < pm->row_count       /** rle was used ***/
		    &&  pm->data_recs * 11 > pm->row_count * 10 ){ /** but not super efficiently **/
			/*printf("OPTIMIZATION: UnRLE(64)\n");*/
			VBlobOptimize_UnRLE_uint64_t(vblobp);
			return;
		}

	}

	if(pm->row_count > 1024 && (sblob->data.elem_bits & 7) == 0)
    {
		elem_count_t	minlen,maxlen;
		elem_count_t	elem_sz = sblob->data.elem_bits/8;
		rc_t rc = PageMapRowLengthRange(pm, &minlen,&maxlen);
		if( rc == 0  && maxlen*elem_sz > 2 && maxlen*elem_sz <= 2048 /* do not optimize super large entries */){ 
		/******* TRY dictionary **/
			int64_t		limit_vocab_size;
			uint32_t	vocab_cnt=0;
			KBTree		*vocab_key2id = NULL;
			bool		valid_data_run = (pm->row_count != pm->data_recs);
			uint32_t	*data_offset = NULL;
			KDataBuffer     new_data;

			rc=KDataBufferMake(&new_data,sblob->data.elem_bits,0); /** no allocation - shoulf not fail at all **/
			assert(rc==0);
/*******************
* another formula
* the savings should not be less than the waste on pointers into data vocabulary
* it is assumed that the data offsets will cost us not more than 2 bytes on disk
* the nasty left part is the average number of bytes in a row
* limit_vocab_size = ((int64_t)sblob->data.elem_count*elem_sz - 2*pm->row_count) * sblob->data.elem_count / pm->data_recs / elem_sz;
****************/
			limit_vocab_size = ((int64_t)sblob->data.elem_count*elem_sz +(valid_data_run?pm->data_recs*2:0) -  2*pm->row_count)
					   * pm->data_recs / (int64_t)sblob->data.elem_count / elem_sz;
			

			if(limit_vocab_size <=1 ){
				rc = RC(rcVDB, rcBlob, rcConstructing, rcId, rcOutofrange);
			}
			if(limit_vocab_size > pm->data_recs/4){
				limit_vocab_size = pm->data_recs/4;  /* to limit the amount of work **/
			}
#define MAX_VOCAB_SIZE 2048
			if(limit_vocab_size > MAX_VOCAB_SIZE) limit_vocab_size=MAX_VOCAB_SIZE;
			if(rc == 0)
				rc=KBTreeMakeUpdate(&vocab_key2id, NULL, 256*1024*1024,
				      false, kbtOpaqueKey,
				      minlen?minlen*elem_sz:1,maxlen*elem_sz, sizeof ( uint32_t ), NULL);
			if(rc == 0){
				uint32_t i,j;
				uint8_t  *src;
				uint32_t dst_offset;
				uint32_t vocab_arr[MAX_VOCAB_SIZE];
				data_offset = malloc(pm->data_recs * sizeof(*data_offset));
				if(data_offset == NULL){
					rc = RC(rcVDB, rcBlob, rcConstructing, rcMemory, rcExhausted);
				}
				for(i=j=0,src = sblob->data.base,dst_offset=0;i<pm->leng_recs && rc == 0 ;i++){
					row_count_t leng_run = pm->leng_run[i];
					if(pm->length[i] == 0){/*** trivial case, can point anywhere - no vocabulary**/
						for(;leng_run > 0;j++){
							row_count_t data_run = valid_data_run?pm->data_run[j]:1;
							assert(leng_run >= data_run);
							data_offset[j] = 0; /**  point to offset=0 **/
							leng_run -=data_run;
						}
					} else for(;leng_run > 0 && rc == 0;j++){
						row_count_t data_run = valid_data_run?pm->data_run[j]:1;
						bool wasInserted;
						uint64_t tmp_id = vocab_cnt;
						assert(leng_run >= data_run);
						leng_run -=data_run;
						rc = KBTreeEntry(vocab_key2id, &tmp_id, &wasInserted, src, pm->length[i]*elem_sz);
						if(rc == 0){
							if(wasInserted){
								vocab_cnt++;
								if(vocab_cnt == limit_vocab_size) {
									rc = RC(rcVDB, rcBlob, rcConstructing, rcId, rcTooBig);
								} else {
									vocab_arr[tmp_id] = dst_offset; /** save offset */
									data_offset[j]    = dst_offset;
									rc = KDataBufferResize(&new_data,dst_offset+pm->length[i]);
									if(rc == 0){
										memmove((uint8_t*)new_data.base+dst_offset*elem_sz,src,pm->length[i]*elem_sz);
										dst_offset += pm->length[i];
									}
								}
							} else {
								assert(vocab_arr[tmp_id] <= dst_offset);
								data_offset[j] = vocab_arr[tmp_id]; /** retrieve offset **/
							}
						}
						src +=pm->length[i]*elem_sz;
						assert(src - (uint8_t*)sblob->data.base <= sblob->data.elem_count * elem_sz);
					}
				}
				if(rc == 0 ){
					PageMap *new_pm;
					/*printf("OPTIMIZATION LEVEL REACHED: vocab:%d,rows:%d,data_recs:%d\n",vocab_cnt ,pm->row_count,pm->data_recs);*/
					rc=PageMapToRandomAccess(&new_pm,pm,data_offset); 
					if(rc == 0){
						/** DONE ***/
						VBlob *vblob;
						rc=VBlobNew(&vblob, sblob->start_id, sblob->stop_id, NULL);
						if(rc==0){
							vblob->pm = new_pm;
							vblob->pm->optimized = eBlobPageMapOptimizedSucceeded;
							vblob->data = new_data;
							VBlobRelease(sblob);
							*vblobp=vblob;
							free(data_offset);
							KBTreeRelease(vocab_key2id);
							return;
						}
						PageMapRelease(new_pm);
						
					}
				} else {
					/*printf("NO OPTIMIZATION: vocab:%d,rows:%d,data_recs:%d\n",vocab_cnt ,pm->row_count,pm->data_recs);*/
				}
				if(data_offset) free(data_offset);
				if(vocab_key2id) KBTreeRelease  ( vocab_key2id );
			}
            KDataBufferWhack(&new_data);
		}
	}

}


rc_t VBlobSerialize ( const VBlob *self, KDataBuffer *result ) {
    uint64_t sz;
    rc_t rc;
    bitsz_t data_bits = KDataBufferBits(&self->data);
    uint64_t data_bytes = KDataBufferBytes(&self->data);
    uint32_t row_length;
    
    if (self->headers == NULL && (row_length = PageMapHasSimpleStructure(self->pm)) != 0) {
        rc = KDataBufferResize(result, 5 + data_bytes);
        if (rc == 0) {

#if _DEBUGGING && 1
            /* temporary assert that we are setting byte_order properly
               in the future, we may allow some functions to issue other
               byte orders, although there is no conceivable reason to do so */
#if __BYTE_ORDER == __LITTLE_ENDIAN
            assert ( self -> byte_order == vboNative || self -> byte_order == vboLittleEndian );
#else
            assert ( self -> byte_order == vboNative || self -> byte_order == vboBigEndian );
#endif
#endif
            rc = encode_header_v1(result->base, result->elem_count, &sz, row_length, data_bits, self->byte_order);
            if (rc == 0) {
                memmove(&((uint8_t *)result->base)[sz], self->data.base, data_bytes);
                result->elem_count = sz + data_bytes;
            }
        }
    }
    else {
        KDataBuffer headers;
        KDataBuffer pagemap;
        
        rc = KDataBufferMakeBytes(&headers, 0);
        if (rc == 0) {
            if (self->headers)
                rc = BlobHeadersSerialize(self->headers, &headers, 0, &sz);
            else
                sz = 0;
            if (rc == 0) {
                headers.elem_count = sz;
                rc = KDataBufferMakeBytes(&pagemap, 0);
                if (rc == 0) {
                    if (self->pm)
                        rc = PageMapSerialize(self->pm, &pagemap, 0, &sz);
                    else
                        sz = 0;
                    if (rc == 0) {
                        pagemap.elem_count = sz;
                        rc = KDataBufferResize(result, 9 + data_bytes + headers.elem_count + pagemap.elem_count);
                        if (rc == 0) {
                            rc = encode_header_v2(result->base, result->elem_count, &sz, headers.elem_count, pagemap.elem_count, data_bits);
                            if (rc == 0) {
                                memmove(&((uint8_t *)result->base)[sz], headers.base, headers.elem_count);
                                sz += headers.elem_count;
                                memmove(&((uint8_t *)result->base)[sz], pagemap.base, pagemap.elem_count);
                                sz += pagemap.elem_count;
                                memmove(&((uint8_t *)result->base)[sz], self->data.base, data_bytes);
                                result->elem_count = sz + data_bytes;
                            }
                        }
                    }
                    KDataBufferWhack(&pagemap);
                }
            }
        }
        KDataBufferWhack(&headers);
    }
    
    return rc;
}

rc_t VBlobCreateFromData ( struct VBlob **lhs,
                         int64_t start_id, int64_t stop_id,
                         const KDataBuffer *src,
                         uint32_t elem_bits , PageMapProcessRequest const *pmpr)
{
    VBlob *y = NULL;
    rc_t rc;
    
    assert(lhs);
    assert(src);
    assert(src->elem_bits == 8);
    assert(src->bit_offset == 0);

    *lhs = 0;

    if ((((const uint8_t *)src->base)[0] & 0x80) == 0)
        rc = VBlobCreateFromData_v1(&y, src, start_id, stop_id, elem_bits);
    else
        rc = VBlobCreateFromData_v2(&y, src, start_id, stop_id, elem_bits, (PageMapProcessRequest*)pmpr);

    if (rc == 0)
        *lhs = y;

    return rc;
}

rc_t VBlobCreateFromSingleRow (
			      struct VBlob **lhs,
			      int64_t start_id, int64_t stop_id,
			      const KDataBuffer *src,
			      VByteOrder byte_order )
{
    VBlob *y;
    rc_t rc;
    
    rc = VBlobNew(&y, start_id, stop_id, NULL);
    TRACK_BLOB (VBlobNew, y);
    if (rc == 0) {
        assert(src->elem_count >> 32 == 0);
        rc = PageMapNewSingle(&y->pm, BlobRowCount(y), (uint32_t)src->elem_count);
        if (rc == 0) {
            rc = KDataBufferSub(src, &y->data, 0, UINT64_MAX);
            if (rc == 0) {
                y->byte_order = byte_order;
                *lhs = y;
                return 0;
            }
        }
        /* should add a release/free? */
    }
    return rc;
}

bool VBlobIsSingleRow( const struct VBlob *self ) {
    return (self->pm && self->pm->data_recs==1) ? true : false;
}

uint32_t VBlobFixedRowLength( const struct VBlob *self ) {
    return self->pm ? PageMapFixedRowLength(self->pm) : 0;
}

#define COMPARE(FORCE, BITS, DBASE, DOFF, SBASE, SOFF, LENGTH) \
    (((FORCE == 0) || (BITS & 7) != 0) ? ( \
        bitcmp(DBASE, DOFF * BITS + FORCE, SBASE, SOFF * BITS, LENGTH * BITS)) : ( \
        memcmp(((const char *)DBASE)+((DOFF * BITS) >> 3), \
               ((const char *)SBASE)+((SOFF * BITS) >> 3), \
               ((LENGTH * BITS) >> 3))))

#define COPY(FORCE, BITS, DBASE, DOFF, SBASE, SOFF, LENGTH) \
    (((FORCE == 0) || (BITS & 7) != 0) ? ( \
        bitcpy(DBASE, DOFF * BITS + FORCE, SBASE, SOFF * BITS, LENGTH * BITS)) : ( \
        (void)memmove(((      char *)DBASE)+((DOFF * BITS) >> 3), \
               ((const char *)SBASE)+((SOFF * BITS) >> 3), \
               ((LENGTH * BITS) >> 3))))

rc_t VBlobAppendRow(VBlob *self,
                    elem_count_t *last_offset,
                    elem_count_t *last_length,
                    const KDataBuffer *src,
                    elem_count_t offset,
                    elem_count_t length,
                    row_count_t repeat_count
                    )
{
    rc_t rc;
    
    if (!PageMapHasRows(self->pm) || length != *last_length ||
        COMPARE(self->data.bit_offset, self->data.elem_bits,
                self->data.base, *last_offset,
                src->base, offset,
                length) != 0
        )
    {
        *last_offset = self->data.elem_count;
        rc = KDataBufferResize(&self->data, *last_offset + length);
        if (rc == 0) {
            COPY(self->data.bit_offset, self->data.elem_bits,
                 self->data.base, *last_offset,
                 src->base, offset, length);
            rc = PageMapAppendRows(self->pm, length, repeat_count, false);
        }
        *last_length = length;
    }
    else
        rc = PageMapAppendRows(self->pm, length, repeat_count, true);
    
    return rc;
}

static rc_t VBlobGetLastRow(VBlob *self, elem_count_t *offset, elem_count_t *length) {
    
    *length = PageMapLastLength(self->pm);
    *offset = self->data.elem_count - *length;

    return 0;
}

rc_t VBlobAppend(VBlob *self, const VBlob *other) {
    rc_t rc;
    row_count_t offset;
    row_count_t length;
    
    if (self->headers)
        return RC(rcVDB, rcBlob, rcConcatenating, rcSelf, rcInconsistent);
    if (other->headers)
        return RC(rcVDB, rcBlob, rcConcatenating, rcParam, rcInvalid);

    if (self->stop_id + 1 != other->start_id)
        return RC(rcVDB, rcBlob, rcConcatenating, rcId, rcOutofrange);

    if (other->data.elem_bits != self->data.elem_bits)
        return RC(rcVDB, rcBlob, rcConcatenating, rcData, rcInvalid);

#if 0
    fprintf(stderr, "appending %u(%u) (length: %u) to %u(%u) (length: %u) %s\n",
            (unsigned)other->start_id, (unsigned)BlobRowCount(other),
            (unsigned)other->data.elem_count,
            (unsigned)self->start_id, (unsigned)BlobRowCount(self),
            (unsigned)self->data.elem_count,
            self->name);
#endif
    
    rc = VBlobGetLastRow(self, &offset, &length);
    if (rc == 0) {
        PageMapIterator iter;
        
        rc = PageMapNewIterator(other->pm, &iter, 0, -1);
        if (rc == 0) {
            KDataBuffer orig;
            
            rc = KDataBufferMakeWritable(&self->data , &orig);
            if (rc == 0) {
                row_count_t row_count;
                KDataBufferWhack(&self->data);
                self->data = orig;
                do {
                    row_count = PageMapIteratorRepeatCount(&iter);
                    rc = VBlobAppendRow(self, &offset, &length, &other->data,
                                        PageMapIteratorDataOffset(&iter),
                                        PageMapIteratorDataLength(&iter),
                                        row_count);
                } while (rc == 0 && PageMapIteratorAdvance(&iter, row_count));
                if (rc == 0) {
                    self->stop_id = other->stop_id;
                    self->no_cache |= other->no_cache;
                }
            }
        }
    }
    return rc;
}

rc_t VBlobSubblob( const struct VBlob *self,struct VBlob **sub, int64_t start_id )
{
    rc_t rc;
    KDataBuffer  kd;
    PageMapIterator pmi;
    
    if (start_id < self->start_id || start_id > self->stop_id)
        return RC(rcVDB, rcBlob, rcConverting, rcId, rcOutofrange);
    
    rc=PageMapNewIterator(self->pm,&pmi, 0, -1);
    if(rc == 0){
        if(PageMapIteratorAdvance(&pmi,start_id-self->start_id)){
            row_count_t numrep = PageMapIteratorRepeatCount(&pmi);
            elem_count_t offset = PageMapIteratorDataOffset(&pmi);
            elem_count_t length = PageMapIteratorDataLength(&pmi);
            
#if 0
            fprintf(stderr, "splitting %u(%u) (offset: %u, length: %u) from %s\n",
                    (unsigned)start_id, (unsigned)numrep,
                    (unsigned)offset, (unsigned)length,
                    self->name);
#endif
            if (numrep == 0){
                return RC(rcVDB, rcBlob, rcConverting, rcRow, rcEmpty);
            }
            
            rc = KDataBufferSub(&self->data, &kd, offset, length);
            if(rc == 0){
                int64_t	stop_id;

                stop_id = start_id + numrep - 1;

                rc = VBlobCreateFromSingleRow(sub, start_id, stop_id, &kd, self->byte_order);
                KDataBufferWhack(&kd);
            }
        } else {
            rc = RC(rcVDB, rcBlob, rcConverting, rcId, rcOutofrange);
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VBlobIdRange ( const VBlob *self,
    int64_t *first, uint64_t *count )
{
    rc_t rc;

    if ( first == NULL && count == NULL )
        rc = RC ( rcVDB, rcBlob, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcBlob, rcAccessing, rcSelf, rcNull );
        else
        {
            if ( first != NULL )
                * first = self -> start_id;
            if ( count != NULL )
                * count = self -> stop_id + 1 - self -> start_id;
            return 0;
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC VBlobCellData ( const VBlob *self, int64_t row_id,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len )
{
    rc_t rc;
    uint32_t dummy [ 3 ];

    if ( elem_bits == NULL )
        elem_bits = & dummy [ 0 ];
    if ( boff == NULL )
        boff = & dummy [ 1 ];
    if ( row_len == NULL )
        row_len = & dummy [ 2 ];

    if ( base == NULL )
        rc = RC ( rcVDB, rcBlob, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcBlob, rcAccessing, rcSelf, rcNull );
        else if ( row_id < self -> start_id || self -> stop_id < row_id )
            rc = RC ( rcVDB, rcBlob, rcAccessing, rcRange, rcInvalid );
        else
        {
            uint64_t start;

            /* TBD - this may be wrong */
            * elem_bits = self -> data . elem_bits;
            * row_len = PageMapGetIdxRowInfo ( self -> pm, ( uint32_t ) ( row_id - self -> start_id ), boff, NULL );
            start = ( uint64_t ) boff [ 0 ] * elem_bits [ 0 ];
            * base = ( uint8_t* ) self -> data . base + ( start >> 3 );
            * boff = ( uint32_t ) start & 7;

            return 0;
        }

        * base = NULL;
    }

    * elem_bits = * boff = * row_len = 0;

    return rc;
}

/* a copy of VCursorRead() */
LIB_EXPORT rc_t CC VBlobRead ( const VBlob *self, int64_t row_id,
    uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len )
{
    rc_t rc = 0;
    const void* base = NULL;
    uint32_t boff = 0;
    uint32_t elem_size = 0;
    uint64_t to_read = 0;
    uint64_t bsize = 0;

    if ( self == NULL )
    {   return RC ( rcVDB, rcBlob, rcReading, rcSelf, rcNull ); }

    if ( buffer == NULL || row_len == NULL )
    {   return RC ( rcVDB, rcBlob, rcReading, rcParam, rcNull ); }

    if ( elem_bits == 0 || ( elem_bits & 7 ) != 0 )
    {   return RC ( rcVDB, rcCursor, rcReading, rcParam, rcInvalid ); }

    rc = VBlobCellData ( self, row_id, &elem_size, &base, &boff, row_len );
    if ( rc != 0 )
    {   return rc; }
    if ( elem_size < elem_bits && elem_bits % elem_size != 0 )
    {   return RC ( rcVDB, rcCursor, rcReading, rcType, rcInconsistent ); }
    if ( elem_size > elem_bits && elem_size % elem_bits != 0 )
    {   return RC ( rcVDB, rcCursor, rcReading, rcType, rcInconsistent ); }
    if ( * row_len == 0 )
    {   return 0; }
    if ( blen == 0 )
    {   return RC ( rcVDB, rcCursor, rcReading, rcBuffer, rcInsufficient ); }

    to_read = * row_len * elem_size;
    bsize = blen * elem_bits;

    /* always return the required buffer size */
    * row_len = ( uint32_t ) ( to_read / elem_bits );

    /* detect buffer too small */
    if ( to_read > bsize ) {
        rc = RC ( rcVDB, rcCursor, rcReading, rcBuffer, rcInsufficient );
        to_read = bsize;
    }

    /* copy out data up to limit */
    assert ( boff == 0 );
    memmove ( buffer, base, ( size_t ) ( to_read >> 3 ) );

    return rc;
}

LIB_EXPORT rc_t CC VBlobReadBits ( const VBlob *self, int64_t row_id,
    uint32_t elem_bits, uint32_t start, void *buffer, uint32_t off,
    uint32_t blen, uint32_t *num_read, uint32_t *remaining )
{
    rc_t rc = 0;
    const void* base = NULL;
    uint32_t boff = 0;
    uint32_t elem_size = 0;
    uint64_t to_read = 0;
    uint64_t bsize = 0;
    uint32_t row_len = 0;
    uint32_t dummy;
    if ( remaining == NULL )
    {   remaining = & dummy; }

    if ( self == NULL )
    {   return RC ( rcVDB, rcBlob, rcReading, rcSelf, rcNull ); }

    if ( buffer == NULL || num_read == NULL )
    {   return RC ( rcVDB, rcBlob, rcReading, rcParam, rcNull ); }

    if ( elem_bits == 0 || ( elem_bits & 7 ) != 0  )
    {   return RC ( rcVDB, rcBlob, rcReading, rcParam, rcInvalid ); }

    rc = VBlobCellData ( self, row_id, &elem_size, &base, &boff, &row_len );
    if ( rc != 0 )
    {   return rc; }
    if ( elem_size < elem_bits && elem_bits % elem_size != 0 )
    {   return RC ( rcVDB, rcCursor, rcReading, rcType, rcInconsistent ); }
    if ( elem_size > elem_bits && elem_size % elem_bits != 0 )
    {   return RC ( rcVDB, rcCursor, rcReading, rcType, rcInconsistent ); }
    if ( row_len == 0 ) {
        * num_read = * remaining = 0;
        return 0;
    }
    if ( * num_read == 0 )
    {   return 0; }
    to_read = row_len * elem_size;
    bsize = blen * elem_bits;

    if ( blen == 0 ) {
        * num_read = 0;
        * remaining = ( uint32_t ) ( to_read / elem_bits );
        return 0;
    }
    bsize = blen * elem_size;
    if ( to_read <= bsize )
    {   * remaining = 0; }
    else {
        * remaining = (uint32_t)( ( to_read - bsize ) / elem_bits );
        to_read = bsize;
    }
    bitcpy ( buffer, off, base, boff, ( bitsz_t ) to_read );
    * num_read = ( uint32_t ) ( to_read / elem_bits );

    return rc;
}

#undef VBlobAddRef
#undef VBlobRelease

LIB_EXPORT rc_t CC VBlobAddRef ( const VBlob *self )
{
    if ( self != NULL )
        KRefcountAdd(&self->refcount, "VBlob");
    return 0;
}

LIB_EXPORT rc_t CC VBlobRelease ( const VBlob *self )
{
    rc_t rc = 0;

    if ( self != 0 )
    {
        switch ( KRefcountDrop(&self->refcount, "VBlob") )
        {
        case krefWhack:
            return VBlobDestroy ( (VBlob*) self );
        case krefNegative:
            rc = RC (rcVDB, rcBlob, rcDestroying, rcBlob, rcExcessive);
            PLOGERR ( klogInt, (klogInt, rc, "Released a blob $(B) with no more references",
                      PLOG_P(self)));
            break;
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VBlobSize ( const VBlob * self, size_t * bytes )
{
    rc_t rc;

    if ( bytes == NULL )
        rc = RC ( rcVDB, rcBlob, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcBlob, rcAccessing, rcSelf, rcNull );
        else
        {
            const PageMap * pm = self -> pm;
            size_t blob_size = sizeof * self + KDataBufferBytes ( & self -> data );
            if ( self -> pm != NULL )
            {
                blob_size += KDataBufferBytes ( & pm -> cstorage )
                          +  KDataBufferBytes ( & pm -> dstorage )
                          +  KDataBufferBytes ( & pm -> istorage )
                          ;
            }

            * bytes = blob_size;
            return 0;
        }

        * bytes = 0;
    }

    return rc;
}

/*--------------------------------------------------------------------------
 * VBlobCache
 */
typedef struct VBlobCache VBlobCache;
struct VBlobCache
{
    DLNode ln;
    size_t size;
    const VBlob *blob;
    uint32_t col_idx;
};

static
rc_t VBlobCacheWhack (uint64_t start_id, const void *n, void *ignore )
{
    VBlobCache *self = ( VBlobCache* ) n;
    assert(start_id == self->blob->start_id); 
    VBlobRelease ( ( VBlob* ) self -> blob );
    free ( self );
    return 0;
}

static
rc_t VBlobCacheMake ( VBlobCache **bcp, const VBlob *blob, uint32_t col_idx, size_t blob_size )
{
    VBlobCache *bc = malloc ( sizeof * bc );
    if ( bc == NULL )
        return RC ( rcVDB, rcCursor, rcReading, rcMemory, rcExhausted );

    bc -> size = blob_size;
    bc -> blob = blob;
    bc -> col_idx = col_idx;
    VBlobAddRef(blob);
    * bcp = bc;
    return 0;
}

typedef struct VBlobLast {
	const VBlob *b1;
	const VBlob *b2;
} VBlobLast;

struct VBlobMRUCache { /* read-only blob cache */
    Vector v_cache; /*** cache VDB columns ***/
    Vector p_cache; /*** cache physical columns ***/
    DLList lru;
    size_t capacity;
    size_t contents;
    /* last blob cache */
    VBlobLast v_last[LAST_BLOB_CACHE_SIZE]; /** last blob to be cached per given col_idx, limiting col_idx  **/
    VBlobLast p_last[LAST_BLOB_CACHE_SIZE]; /** last physical blob to be cached per given col_idx, limiting col_idx  **/
	bool suspend_flush;
};


VBlobMRUCache * VBlobMRUCacheMake(uint64_t capacity )
{
    VBlobMRUCache *self = NULL;
    if(capacity > 0){
	self = malloc(sizeof(*self));
        if(self){
		VectorInit ( & self -> v_cache, 1, 16);
		VectorInit ( & self -> p_cache, 1, 16);
		DLListInit ( & self -> lru );
		memset(self -> v_last,0,LAST_BLOB_CACHE_SIZE*sizeof(*self -> v_last));
		memset(self -> p_last,0,LAST_BLOB_CACHE_SIZE*sizeof(*self -> p_last));
		self->capacity = capacity;
		self->contents = 0;
		self->suspend_flush = false;
	}
   }
   return self;
}

void VBlobMRUCacheItemDestroy( void *item, void *data )
{
    if ( item != NULL ) {
	KVectorVisitPtr (item, false, VBlobCacheWhack, data );
	KVectorRelease(item);
    }
}


void VBlobMRUCacheDestroy( VBlobMRUCache *self )
{
    if(self){
	int i;
	VectorWhack ( & self -> v_cache, VBlobMRUCacheItemDestroy, NULL );
	VectorWhack ( & self -> p_cache, VBlobMRUCacheItemDestroy, NULL );
	DLListInit ( & self -> lru );
	for(i=0;i<LAST_BLOB_CACHE_SIZE;i++){
	    VBlobRelease(self -> p_last[i].b1);
	    VBlobRelease(self -> p_last[i].b2);
	    VBlobRelease(self -> v_last[i].b1);
	    VBlobRelease(self -> v_last[i].b2);
	}
	free(self);
    }
}

static VBlobCache * find_in_kvector(const KVector *cself,int64_t id)
{
	VBlobCache *bc;
	int64_t     id_prev;
	rc_t rc=KVectorGetPrevPtr(cself,(uint64_t*)&id_prev,(uint64_t)id+1,(void**)&bc);
	if(rc == 0 && bc){
		assert(bc->blob->start_id == id_prev );
		assert(bc->blob->start_id <= id);
		if( id >= bc->blob->start_id && id <= bc->blob->stop_id){
			return bc;
		}
	} 
	return NULL;
}

const VBlob* VBlobMRUCacheFind(const VBlobMRUCache *cself, uint32_t col_idx, int64_t row_id)
{
    VBlobMRUCache *self = (VBlobMRUCache*)cself;
    const VBlob* blob;
    KVector  *cache;
    bool    is_phys=false;
    VBlobLast   *last_blobs;

    if(col_idx > PHYSPROD_INDEX_OFFSET){
	is_phys=true;
	last_blobs = self->p_last;
	col_idx -= PHYSPROD_INDEX_OFFSET;
    } else {
	is_phys=false;
	last_blobs = self->v_last;
    } 

    if(col_idx <= LAST_BLOB_CACHE_SIZE){
	blob = last_blobs[col_idx-1].b1;
	if(blob && row_id >= blob->start_id && row_id <= blob->stop_id){
		return blob;
	}
	blob = last_blobs[col_idx-1].b2;
	if(blob && row_id >= blob->start_id && row_id <= blob->stop_id){
		last_blobs[col_idx-1].b2 = last_blobs[col_idx-1].b1;
		last_blobs[col_idx-1].b1 = blob;
		return blob;
	}
    }
    cache = is_phys?VectorGet(&cself->p_cache,col_idx):VectorGet(&cself->v_cache,col_idx);
    if(cache) {
	    /* check cache for entry */
	    VBlobCache *bc = find_in_kvector ( cache, row_id );
	    if ( bc != NULL )
	    {
		/* save in MRU */
		if(col_idx <= LAST_BLOB_CACHE_SIZE) {
			if(last_blobs[col_idx-1].b2) VBlobRelease(last_blobs[col_idx-1].b2);
			last_blobs[col_idx-1].b2 = last_blobs[col_idx-1].b1;
			last_blobs[col_idx-1].b1 = bc->blob;
			if(VBlobAddRef ((VBlob*)bc->blob)!=0)
				return NULL;
		}
		/* maintain LRU */
		DLListUnlink  (&self->lru,&bc->ln);
		DLListPushHead(&self->lru,&bc->ln);
		/* ask column to read from blob */
		return bc->blob;
	    }
    }
    return NULL;
}

static rc_t  insert_unique_into_kvector(VBlobMRUCache *self,KVector *cache,int64_t id, const VBlobCache *bc, VBlobCache **existing)
{
	rc_t rc=KVectorGetPtr(cache,id,(void**)existing);
	if(rc == 0 && *existing){ /*found*/

		assert(existing[0]->blob->start_id == bc->blob->start_id);
		if(existing[0]->blob->stop_id < bc->blob->stop_id){/** new blob is bigger - replace with ned blob **/
			DLListUnlink  (&self->lru,&(existing[0]->ln));
			VBlobCacheWhack (existing[0]->blob->start_id, existing[0], NULL );
		} else {
			return RC ( rcVDB, rcVector, rcInserting, rcBlob, rcExists );
		}
	}
        return KVectorSetPtr(cache,id,bc);
}


rc_t VBlobMRUCacheSave(const VBlobMRUCache *cself, uint32_t col_idx, const VBlob *blob)
{
    rc_t   rc;
    size_t blob_size =sizeof(VBlobCache) + sizeof(VBlob);
    VBlobCache *bc=NULL;
    VBlobMRUCache *self = (VBlobMRUCache*)cself;

    if(blob->no_cache) return 0;

    blob_size  += KDataBufferBytes(&blob->data);
    if (blob->pm != NULL) {
        blob_size += 
                  KDataBufferBytes(&blob->pm->cstorage)
                + KDataBufferBytes(&blob->pm->dstorage)
                + KDataBufferBytes(&blob->pm->istorage);
    }
    /** auto-raise capacity for large blob **/
    if(blob_size > self -> capacity) self -> capacity = blob_size;

    

    /* now cache the blob */
    rc = VBlobCacheMake ( & bc, blob, col_idx, blob_size );
    if ( rc == 0 ) {
        /* attempt a unique insertion */
        VBlobCache *existing;
        KVector *cache;
        VBlobLast   *last_blobs;

	if(col_idx > PHYSPROD_INDEX_OFFSET){
		last_blobs = self->p_last;
		col_idx -= PHYSPROD_INDEX_OFFSET;
		cache = VectorGet(&cself->p_cache,col_idx);
		if(cache==NULL){
			KVectorMake(&cache);
			VectorSet(&self->p_cache,col_idx,cache);
		}
	} else {
		last_blobs = self->v_last;
		cache = VectorGet(&cself->v_cache,col_idx);
		if(cache==NULL){
			KVectorMake(&cache);
			VectorSet(&self->v_cache,col_idx,cache);
		}
	}
	
        rc = insert_unique_into_kvector(self,cache,bc->blob->start_id,bc,&existing);
        if ( rc != 0 ){
            VBlobCacheWhack (bc->blob->start_id, bc, NULL );
			rc = 0;
		} else {
				/* remember as last used  **/
			if(col_idx <= LAST_BLOB_CACHE_SIZE) {
				if(last_blobs[col_idx-1].b2) VBlobRelease(last_blobs[col_idx-1].b2);
				last_blobs[col_idx-1].b2 = last_blobs[col_idx-1].b1;
				last_blobs[col_idx-1].b1 = bc->blob;
				rc = VBlobAddRef ((VBlob*)bc->blob);
				if(rc != 0)
				   return rc;
			}
			/* perform accounting */
			self -> contents += blob_size;
			if(!self->suspend_flush) while ( self -> contents > self -> capacity )
			{
				/* get least recently used */
				DLNode *last = DLListPopTail ( & self -> lru );
				if ( last == NULL )
						break;
				/* drop blob */
				existing = ( VBlobCache* ) last;
				if(existing->col_idx > PHYSPROD_INDEX_OFFSET){
					cache = VectorGet(&cself->p_cache,existing->col_idx-PHYSPROD_INDEX_OFFSET);
				} else {
					cache = VectorGet(&cself->v_cache,existing->col_idx);
				}
				KVectorUnset(cache,existing->blob->start_id);
				self -> contents -= existing -> size;
				VBlobCacheWhack (existing->blob->start_id,existing,NULL);
			}
			/* insert at head of list */
			DLListPushHead ( & self -> lru, & bc -> ln ); 
		}
    }
    return 0;
}

uint64_t VBlobMRUCacheGetCapacity(const VBlobMRUCache *cself)
{
	if(cself){
		return cself->capacity;
	}
	return 0;
}
uint64_t VBlobMRUCacheSetCapacity(VBlobMRUCache *self,uint64_t capacity )
{
	uint64_t old_capacity=0;
	if(self){
		old_capacity = self->capacity;
		self->capacity=capacity;
	}
	return old_capacity;
}
void VBlobMRUCacheSuspendFlush(VBlobMRUCache *self)
{
	self->suspend_flush=true;
}
	
void VBlobMRUCacheResumeFlush(VBlobMRUCache *self)
{
	self->suspend_flush=false;
}

