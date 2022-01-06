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

#ifndef _h_blob_priv_
#define _h_blob_priv_

#include <vdb/blob-priv.h>

#define TRACKING_BLOBS 0
#if TRACKING_BLOBS
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef struct PageMapProcessRequest{
    struct PageMap *pm;        /**** deserialized form **/
    KDataBuffer data;   /**** serialized   form **/
    uint32_t    row_count;
    uint64_t    elem_count;
    rc_t rc;            /**** results **/

    volatile enum {
        ePMPR_STATE_NONE=0,
        ePMPR_STATE_SERIALIZE_REQUESTED,
        ePMPR_STATE_DESERIALIZE_REQUESTED,
        ePMPR_STATE_SERIALIZE_DONE,
        ePMPR_STATE_DESERIALIZE_DONE,
	ePMPR_STATE_EXIT
    } state;            /**** request state      **/
    struct KLock      *lock;
    struct KCondition *cond;
} PageMapProcessRequest;


struct VBlobPageMapCache {
    int n;
    struct PageMap *pm[1];
};

#if TRACKING_BLOBS
/* #define TRACK_BLOB(f,p) fprintf(stderr, "%s %p %d %s:%d:%s\n",#f,(void*)p,*(int*)(&p->refcount),__FILE__,__LINE__,__func__) */
#define TRACK_BLOB(f,p) fprintf(stderr, "+++++ %s %p %d %s:%d\n",#f,(void*)(p),p?*(int*)(&(p)->refcount):0,__func__,__LINE__)
#else
#define TRACK_BLOB(f,p)
#endif

rc_t VBlobNew( VBlob **lhs, int64_t start_id, int64_t stop_id, const char *name );

/* use inline-able addref and release on blobs
   within the library for efficiency */
#if 1
#define VBlobAddRef( self ) \
    ( ( ( self ) == NULL ) ? \
      0 : ( atomic32_inc ( & ( self ) -> refcount ), 0 ) )
#define VBlobRelease( self ) \
    ( ( ( self ) == NULL || atomic32_read_and_add_gt ( & ( self ) -> refcount, -1, 1 ) > 1 ) ? \
      0 : VBlobRelease ( self ) )
#endif

#if _DEBUGGING
void VBlobCheckIntegrity ( const VBlob *self );
#else
#define VBlobCheckIntegrity( self ) \
  ( void ) 0
#endif

rc_t VBlobCreateFromData(
                         struct VBlob **lhs,
                         int64_t start_id, int64_t stop_id,
                         const KDataBuffer *src,
                         uint32_t elem_bits,
			 PageMapProcessRequest const *pmpr
);

rc_t VBlobCreateFromSingleRow(
                              struct VBlob **lhs,
                              int64_t start_id, int64_t stop_id,
                              const KDataBuffer *src,
                              VByteOrder byte_order
);
/*** VBlob as a new array; meaning fixed len,data_run(..)=1 **/
rc_t VBlobNewAsArray(struct VBlob **lhs, int64_t start_id, int64_t stop_id, uint32_t rowlen, uint32_t elem_bits);

rc_t VBlobSerialize( const VBlob *self, KDataBuffer *result );

bool VBlobIsSingleRow( const VBlob *self );

uint32_t VBlobFixedRowLength( const struct VBlob *self );

rc_t VBlobAppend( struct VBlob *self, const struct VBlob *other );
/**** finds start_id in a blob and builds sub-blob for the number or repetitions ***/
rc_t VBlobSubblob( const struct VBlob *self,struct VBlob **sub, int64_t start_id );

void VBlobPageMapOptimize( struct VBlob **self );

#define LAST_BLOB_CACHE_SIZE 256


typedef struct VBlobMRUCache VBlobMRUCache; /** forward declaration **/
typedef  struct VBlobMRUCacheCursorContext {  /** to be used to pass cache context down to production level ***/
	VBlobMRUCache * cache;
	uint32_t	col_idx;
} VBlobMRUCacheCursorContext;

VBlobMRUCache * VBlobMRUCacheMake(uint64_t capacity );
void VBlobMRUCacheDestroy( VBlobMRUCache *self );
const VBlob* VBlobMRUCacheFind(const VBlobMRUCache *cself, uint32_t col_idx, int64_t row_id);
rc_t VBlobMRUCacheSave(const VBlobMRUCache *cself, uint32_t col_idx, const VBlob *blob);

uint64_t VBlobMRUCacheGetCapacity(const VBlobMRUCache *cself);
uint64_t VBlobMRUCacheSetCapacity(VBlobMRUCache *self,uint64_t capacity );

void VBlobMRUCacheSuspendFlush(VBlobMRUCache *self);
void VBlobMRUCacheResumeFlush (VBlobMRUCache *self);


rc_t PageMapProcessGetPagemap(const PageMapProcessRequest *self,struct PageMap **pm);


#ifdef __cplusplus
}
#endif
#endif /* _h_blob_priv_ */
