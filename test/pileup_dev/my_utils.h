/* my_utils.h */

#include <stdint.h>

#include <klib/rc.h>

#ifndef countof
#define countof(arr) (sizeof(arr)/sizeof(arr[0]))
#endif

#if 0

/* Some defines to be able to switch between different implementations for comparison */
#define CACHE_IMPL_AS_LIST 1 /* ==1 if using DLList to store a set of cached alignments, otherwise - use array-based cache */
#define USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS 1 /* ==1 if using 1 blob to be able to have persistent pointer to PRIMARY_ALIGNMENT_IDS CellData, otherwise - copy data */
#define USE_BLOB_CACHE_FOR_ALIGNMENTS 0 /* == 1 if using blobs to access *_ALIGNMENTS table*/
#define USE_SINGLE_BLOB_FOR_ALIGNMENTS 1 /* ==1 if using 1 blob to be able to have persistent pointer to PRIMARY_ALIGNMENT CellData, otherwise - copy data */
#define USE_SINGLE_BLOB_FOR_HAS_MISMATCH 0 /* HAS_MISMATCH and MISMATCH columns*/

#if    USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1\
    || USE_BLOB_CACHE_FOR_ALIGNMENTS == 1\
    || USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1\
    || USE_SINGLE_BLOB_FOR_ALIGNMENT_MISMATCH == 1

#define USE_BLOBS 1
#else
#define USE_BLOBS 0
#endif

#if CACHE_IMPL_AS_LIST == 1
#include <klib/container.h>
#endif

#if USE_BLOBS == 1
#include <vdb/blob.h>
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

typedef struct Alignment_CacheItem
{
#if CACHE_IMPL_AS_LIST == 1
    DLNode node; /* list node */
#endif
    int64_t row_id;
    int64_t start;
    uint64_t len;
    uint32_t seq_start; /* zero-based! Each alignment's REF_START (start) is relative to corresponding REFERENCE.SEQ_START, so we have to store this relation here */

#if USE_SINGLE_BLOB_FOR_ALIGNMENT_MISMATCH == 1
    char const* has_mismatch;
    char const* mismatch;
#else
    char has_mismatch[32];
    char mismatch[128];
#endif
    size_t size_has_mismatch;
    size_t size_mismatch;

} Alignment_CacheItem;

typedef struct Alignment_Cache
{
    size_t size;
#if CACHE_IMPL_AS_LIST == 1
    DLList list_alignments;
#else
    size_t capacity;
    Alignment_CacheItem* data;
#endif

} Alignment_Cache;


#if USE_BLOB_CACHE_FOR_ALIGNMENTS == 1
typedef struct BlobItem
{
    VBlob const* blob;
    /*size_t order;*/
} BlobItem;

typedef struct BlobCache
{
    size_t size;
    BlobItem blobs[1024];
} BlobCache;

/*size_t BlobCache_LowerBound (BlobCache const* cache, size_t order);
size_t BlobCache_UpperBound (BlobCache const* cache, size_t order);*/
void BlobCache_Init (BlobCache* cache);
/*void BlocbCache_InsertBlob (BlobCache* cache, size_t order, VBlob const* blob);*/
void BlobCache_Whack (BlobCache* cache);
/*void BlocbCache_ReleaseLessThan (BlobCache* cache, size_t order);*/
void BlobCache_Add (BlobCache* cache, VBlob const* blob);

#endif

typedef struct PileupIteratorState
{
    /* Static data */
    int64_t reference_start_id;
    /*uint64_t reference_length; /* TODO: it's not used - remove it */

    uint64_t total_row_count; /* row count of all references' row ids */

    int64_t slice_start;
    uint64_t slice_length;

    int64_t slice_start_id;
    int64_t slice_end_id;

    /* Blobs that we want to have manual control over */
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
    VBlob const* blob_alignment_ids; /* Here we store the blob containig current reference row */
#endif

    uint32_t max_seq_len;
    char ref_name [64]; /* need this to determine the moment when switching to different reference during iteration */

    /* Current State */
    uint32_t current_seq_start;


    uint64_t ref_pos;
    Alignment_Cache cache_alignment;        /* Alignments intersecting slice */

    size_t size_alignment_ids;
    size_t next_alignment_idx; /* index in alignment_ids pointing to the first id that has not been cached in data yet */
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS != 1
    size_t capacity_alignment_ids;
    int64_t* alignment_ids; /* alignment_ids of the current reference row_id */
#else
    int64_t const* alignment_ids; /* alignment_ids of the current reference row_id */
#endif

#if USE_BLOB_CACHE_FOR_ALIGNMENTS == 1
    BlobCache blobs_alignments;
#endif
#if USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1
    VBlob const* blob_alignments_ref_start;
    VBlob const* blob_alignments_ref_len;
#endif

} PileupIteratorState;

void Alignment_Init ( Alignment_Cache* self );
void Alignment_InitCacheWithNull ( Alignment_Cache* self );
void Alignment_Release ( Alignment_Cache* self );
void Alignment_ResetCache ( Alignment_Cache* self );
rc_t Alignment_Add ( Alignment_Cache* self, int64_t row_id, int64_t start, uint64_t len, uint32_t seq_start );
Alignment_CacheItem const* Alignment_Get ( Alignment_Cache const* self, size_t i );

#if CACHE_IMPL_AS_LIST == 1
void CC Alignment_CacheItemWhack ( DLNode* n, void* data );
#endif

void PileupIteratorState_Init (PileupIteratorState* self);
rc_t PileupIteratorState_SetAlignmentIds ( PileupIteratorState* self, int64_t const* alignment_ids, size_t count );
void PileupIteratorState_Release ( PileupIteratorState* self );

#ifdef __cplusplus
}
#endif

#endif