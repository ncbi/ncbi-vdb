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
#include <klib/rc.h>

#include "xform-priv.h"
#include "blob-priv.h"
#include "blob-headers.h"
#include "blob.h"
#include "page-map.h"

#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>


/* Create
 *
 * Create a new blob.  A new data buffer is sized to
 * 'element_count' * sizeof datatype; its contents are
 * uninitialized.
 */
static
rc_t VBlobCreateEncode ( VBlob **blobp, const VBlob *in, uint32_t max_rl_bytes,uint8_t *avg, uint32_t elem_bytes )
{
    rc_t rc = VBlobNew ( blobp, in -> start_id, in -> stop_id, "delta_average" );
    if ( rc == 0 )
    {
        VBlob *blob = * blobp;
        rc = PageMapAddRef ( in -> pm );
	if(rc == 0) {
		blob->pm = in -> pm ;
		if(max_rl_bytes > 0){
			rc = KDataBufferMake ( & blob -> data, in -> data . elem_bits, in -> data . elem_count );
		} else {
			rc = KDataBufferSub(&in->data, &blob->data, 0, UINT64_MAX);
		}
		if ( rc == 0 ){
			rc = BlobHeadersCreateChild(in->headers, &blob->headers);
			if ( rc == 0) {
				VBlobHeader *hdr = BlobHeadersGetHdrWrite(blob->headers);
				assert(hdr!=NULL);
				if(hdr != 0){
					int i;
					VBlobHeaderSetVersion (hdr, 1);
					rc = VBlobHeaderArgPushTail(hdr, max_rl_bytes);
					assert(rc == 0);
					rc = VBlobHeaderArgPushTail(hdr, elem_bytes);
                                        assert(rc == 0);
					for(i=0; i< max_rl_bytes; i++){
						rc = VBlobHeaderOpPushTail(hdr,avg[i]);
						assert(rc==0);
					}
					blob -> byte_order = in -> byte_order;
					VBlobHeaderRelease(hdr);
					return 0;
				}
			}
		}
		PageMapRelease( blob->pm);
        }
	VBlobRelease ( blob );
        * blobp = NULL;
    }
    return rc;
}

static
rc_t VBlobCreateDecode ( VBlob **blobp, const VBlob *in, uint32_t *max_rl_bytes,uint8_t **avg, uint32_t *elem_bytes)
{
    rc_t rc = VBlobNew ( blobp, in -> start_id, in -> stop_id, "undelta_average" );
    if ( rc == 0 )
    {
        VBlob *blob = * blobp;
        rc = PageMapAddRef ( in -> pm );
	if(rc == 0) {
		VBlobHeader *hdr;
		blob->pm = in -> pm ;
                blob->headers = (BlobHeaders *)BlobHeadersGetNextFrame(in->headers);
		BlobHeadersAddRef(blob->headers);
		hdr = BlobHeadersGetHeader(in->headers);
		if(hdr || VBlobHeaderVersion(hdr)!=1) {
			int64_t len;
			int64_t eb;
			rc = VBlobHeaderArgPopHead(hdr, &len);
			if( rc == 0)
				rc = VBlobHeaderArgPopHead(hdr, &eb);
			if( rc == 0){
				*max_rl_bytes=len;
				*elem_bytes = eb;
				if(len > 0){
					rc = KDataBufferMake ( & blob -> data, in -> data . elem_bits, in -> data . elem_count );
					if ( rc == 0){
						*avg=malloc(len);
						if(*avg){
							int i;
							for(i=0;i<len && rc == 0;i++){
								rc = VBlobHeaderOpPopHead(hdr,(*avg)+i);
							}
							if(rc == 0){
								VBlobHeaderRelease(hdr);
								return 0;
							} else {
								rc =  RC(rcVDB, rcFunction, rcExecuting, rcHeader, rcCorrupt) ;
							}
						} else {
							rc =  RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted) ;
						}
					}
				} else {
					rc = KDataBufferSub(&in->data, &blob->data, 0, UINT64_MAX);
					if(rc == 0){
						VBlobHeaderRelease(hdr);
						return 0;
					}
				}
				VBlobHeaderRelease(hdr);
			} else {
				rc =  RC(rcVDB, rcFunction, rcExecuting, rcHeader, rcCorrupt) ;
			}
		} else {
			rc =  RC(rcVDB, rcFunction, rcExecuting, rcHeader, rcCorrupt) ;
		}
		PageMapRelease( blob->pm);
        }
	VBlobRelease ( blob );
        * blobp = NULL;
    }
    return rc;
}

static
rc_t CC undelta_average ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    uint32_t  max_rl_bytes = 0;
    uint8_t   *avg = NULL;
    uint32_t elem_bytes = 0;
    /* input blob - schema assures us that it is ready to go */
    const VBlob *in = argv [ 0 ];
  /* allocate output blob of same size */
    rc_t rc = VBlobCreateDecode ( rslt, in,&max_rl_bytes,&avg,&elem_bytes );
    if(rc == 0 && max_rl_bytes > 0){
	uint8_t *dst = (*rslt)->data.base; /** destination pointer **/
	uint8_t *src = in -> data.base;
	int	i,j,k;
	PageMap *pm  = in->pm;

	for(i=j=0,src = in -> data.base;i<pm->leng_recs;i++){
                row_count_t leng_run=pm->leng_run[i];
                elem_count_t len = pm->length[i];
                assert(len<=max_rl_bytes);
                while(leng_run > 0){
                        for(k=0;k<len*elem_bytes;k++,src++,dst++){
                                dst[0] = avg[k] - src[0]; /** symmetrical to encode **/
                        }
                        assert(leng_run >= (pm->data_run?pm->data_run[j]:1));
                        leng_run -= pm->data_run?pm->data_run[j]:1;
			j++;
                }
        }

    }
    if(avg) free(avg);
    return rc;
}

VTRANSFACT_BUILTIN_IMPL ( vdb_undelta_average, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    VFUNCDESC_INTERNAL_FUNCS ( rslt ) -> bfN = undelta_average;
    rslt -> variant = vftBlobN;
    return 0;
}

static
rc_t CC delta_average ( void *self, const VXformInfo *info, int64_t row_id,
    VBlob **rslt, uint32_t argc, const VBlob *argv [] )
{
    rc_t rc;
    /* input blob - schema assures us that it is ready to go */
    const VBlob *in = argv [ 0 ];
    const PageMap *pm = in->pm; 
    uint8_t     *src;
    elem_count_t   min_row_len,max_row_len;
    uint64_t	/*min_rl_bytes,*/ max_rl_bytes;
    uint32_t    *cnts;
    uint32_t    elem_bytes;
    int         i,j,k;
    uint8_t	*avg=NULL;
    uint64_t    sum_rle;
    uint8_t	last;

    if(pm->row_count < 256 || (in->data.elem_bits & 7)!=0){
	return VBlobCreateEncode ( rslt, in, 0,NULL,0 );
    }
    elem_bytes = in->data.elem_bits >> 3;
    PageMapRowLengthRange(pm,&min_row_len,&max_row_len);
    if(max_row_len > 1024){
	return VBlobCreateEncode ( rslt, in, 0,NULL,0 );
    }

    /*** rescale to bytes **/
    /* min_rl_bytes = (min_row_len * in->data.elem_bits) >> 3; */
    max_rl_bytes = (max_row_len * in->data.elem_bits) >> 3;

    cnts=calloc(sizeof(*cnts),256*max_rl_bytes); /** computing counts of values **/
    avg=malloc(max_rl_bytes);

    /** collect sums and averages ***/
    /** using tight loop instead of iterators **/
    for(i=j=0,src = in -> data.base, sum_rle=0,last=0;i<pm->leng_recs;i++){
	row_count_t leng_run=pm->leng_run[i];
	elem_count_t len = pm->length[i];
	assert(len<=max_rl_bytes);
	while(leng_run > 0){
		for(k=0;k<len*elem_bytes;k++,src++){
			if(last==src[0]) sum_rle++;
			cnts[256*k+src[0]]++;
			last=src[0];
		}
		assert(leng_run >= (pm->data_run?pm->data_run[j]:1));
		leng_run -= pm->data_run?pm->data_run[j]:1;
		j++;
	}
    }
    if(sum_rle * 10 > elem_bytes * in->data.elem_count * 3){/** total number of repetitions is high - leave it for zlib RLE ***/
        free(cnts);
        free(avg);
        return VBlobCreateEncode(rslt,in,0,NULL,0);
    }

    for(i=0;i<max_rl_bytes;i++){
	avg[i] = 0;
	for(j=1;j<256;j++){
		if(cnts[256*i+j] > cnts[256*i+avg[i]]){
			avg[i]=j; 
		}
	}
    }
    rc = VBlobCreateEncode ( rslt, in, max_rl_bytes,avg,elem_bytes );
    if (rc == 0 ){
	uint8_t *dst = (*rslt)->data.base; /** destination pointer **/
	for(i=j=0,src = in -> data.base;i<pm->leng_recs;i++){
		row_count_t leng_run=pm->leng_run[i];
		elem_count_t len = pm->length[i];
		assert(len<=max_rl_bytes);
		while(leng_run > 0){
			for(k=0;k < len*elem_bytes;k++,src++,dst++){
				dst[0] = avg[k] - src[0]; /** will do it in this direction **/
			}
			assert(leng_run >= (pm->data_run?pm->data_run[j]:1));
			leng_run -= pm->data_run?pm->data_run[j]:1;
			j++;
		}
	}
     }
    if(cnts) free(cnts);
    if(avg)  free(avg);
    return rc;
}

VTRANSFACT_BUILTIN_IMPL ( vdb_delta_average, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    VFUNCDESC_INTERNAL_FUNCS ( rslt ) -> bfN = delta_average;
    rslt -> variant = vftBlobN;
    return 0;
}
