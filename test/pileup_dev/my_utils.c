/* my_utils.c */

#include <stdio.h>
#include <string.h>

#include "my_utils.h"

#if 0

#if CACHE_IMPL_AS_LIST == 1
void CC Alignment_CacheItemWhack ( DLNode* n, void* data )
{
    free ( n );
}
#endif
void Alignment_InitCacheWithNull ( Alignment_Cache* self )
{
    self -> size = 0;

#if CACHE_IMPL_AS_LIST == 1
    DLListInit ( & self->list_alignments );
#else
    self -> capacity = 128;
    self -> data = NULL;
#endif
}

void Alignment_Init ( Alignment_Cache* self )
{
    self -> size = 0;

#if CACHE_IMPL_AS_LIST == 1
    DLListInit ( & self->list_alignments );
#else
    self -> capacity = 128;
    self -> data = (Alignment_CacheItem*) malloc ( self -> capacity * sizeof (Alignment_CacheItem) );
#endif

}

void Alignment_Release ( Alignment_Cache* self )
{

#if CACHE_IMPL_AS_LIST == 1

    DLListWhack ( & self->list_alignments, Alignment_CacheItemWhack, NULL );
    self -> size = 0;

#else
    if ( self -> data != NULL )
    {
        free ( self -> data );
        self -> data = NULL;
        self -> capacity = 128;
    }
#endif

}

void Alignment_ResetCache ( Alignment_Cache* self )
{
    self -> size = 0;

#if CACHE_IMPL_AS_LIST == 1
    DLListWhack ( & self->list_alignments, Alignment_CacheItemWhack, NULL );
#endif
}

rc_t Alignment_Add ( Alignment_Cache* self, int64_t row_id, int64_t start, uint64_t len, uint32_t seq_start )
{
#if CACHE_IMPL_AS_LIST == 1
    Alignment_CacheItem* item = (Alignment_CacheItem*) malloc (sizeof (Alignment_CacheItem));

    if ( item == NULL )
        return RC ( rcRuntime, rcData, rcAllocating, rcMemory, rcExhausted );

    item->row_id = row_id;
    item->start = start;
    item->len = len;
    item->seq_start = seq_start;

    DLListPushTail ( & self->list_alignments, & item -> node );

    ++self->size;

#else
    size_t size = self->size;

    if ( size == self->capacity )
    {
        void* new_data = realloc ( self->data, sizeof (Alignment_CacheItem) * self->capacity*2 );
        if ( new_data == NULL )
            return (rc_t)(-1);
        else
        {
            self->capacity *= 2;
            self->data = (Alignment_CacheItem*) new_data;
        }
    }

    self->data [ size ] . row_id = row_id;
    self->data [ size ] . start = start;
    self->data [ size ] . len = len;
    self->data [ size ] . seq_start = seq_start;

    /* TODO: remove this */
    
    /*if ( self->data[ self->size - 1].row_id >=  self->data[ self->size].row_id)
    {
        printf ("WARNING: adding Alignment_CacheItem out of order: prev=%lld, cur=%lld\n",
            self->data[ self->size - 1].row_id, self->data[ self->size].row_id);
        /*return (rc_t)(-2);
    }*/

    ++ self->size;
#endif

    return 0;
}

Alignment_CacheItem const* Alignment_Get ( Alignment_Cache const* self, size_t i )
{
#if CACHE_IMPL_AS_LIST == 1
    /* TODO: it's better not to have this function for list-based cache
        It's here only for testing purposes, for main.c:print_current_state()
    */
    Alignment_CacheItem const* item = ( Alignment_CacheItem const* ) DLListHead ( & self->list_alignments );
    for (; i > 0 && item != NULL; --i, item = ( Alignment_CacheItem const* ) DLNodeNext (& item->node) );

    return item;
#else
    return & self->data [ i ];
#endif
}


#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS != 1
rc_t PileupIteratorState_SetAlignmentIds ( PileupIteratorState* self, int64_t const* alignment_ids, size_t count )
{
    if ( count > self->capacity_alignment_ids )
    {
        self->capacity_alignment_ids *= 2;
        if ( self->capacity_alignment_ids < count )
            self->capacity_alignment_ids = count;
        free ( self->alignment_ids );
        self->alignment_ids = (int64_t*) malloc ( self->capacity_alignment_ids * sizeof (int64_t) );
        if ( self->alignment_ids == NULL )
            return (rc_t)(-1);
    }

    memcpy ( self->alignment_ids, alignment_ids, count * sizeof (int64_t));
    self->size_alignment_ids = count;
    return 0;
}
#endif

void PileupIteratorState_Init (PileupIteratorState* self)
{
    self -> size_alignment_ids = 0;
    self -> next_alignment_idx = 0;
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS != 1
    self -> capacity_alignment_ids = 128;
    self -> alignment_ids = (int64_t*) malloc ( self -> capacity_alignment_ids * sizeof (int64_t) );
#else
    self -> alignment_ids = NULL;
    self -> blob_alignment_ids = NULL;
#endif

#if USE_BLOB_CACHE_FOR_ALIGNMENTS == 1
    BlobCache_Init ( & self -> blobs_alignments );
#endif

#if USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1
    self -> blob_alignments_ref_start = NULL;
    self -> blob_alignments_ref_len = NULL;
#endif

}

void PileupIteratorState_Release ( PileupIteratorState* self )
{
    if ( self -> alignment_ids != NULL )
    {
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS != 1
        free ( self -> alignment_ids );
        self -> capacity_alignment_ids = 128;
#endif
        self -> alignment_ids = NULL;
    }
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
    if ( self -> blob_alignment_ids != NULL )
    {
        VBlobRelease ( self -> blob_alignment_ids );
        self -> blob_alignment_ids = NULL;
    }
#endif

#if USE_BLOB_CACHE_FOR_ALIGNMENTS == 1
    BlobCache_Whack ( & self -> blobs_alignments );
#endif

#if USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1
    if ( self -> blob_alignments_ref_start != NULL )
    {
        VBlobRelease ( self -> blob_alignments_ref_start );
        self -> blob_alignments_ref_start = NULL;
    }
    if ( self -> blob_alignments_ref_len != NULL )
    {
        VBlobRelease ( self -> blob_alignments_ref_len );
        self -> blob_alignments_ref_len = NULL;
    }
#endif
}


#if USE_BLOB_CACHE_FOR_ALIGNMENTS == 1
/*size_t BlobCache_LowerBound (BlobCache const* cache, size_t order)
{
    size_t low = 0;
    size_t high = cache->size;
    size_t middle = (high + low) / 2;
    for (; ;)
    {
        if ( cache->blobs[middle].order < order )
            low = middle + 1;
        else
            high = middle;

        middle = (high + low) / 2;
        if (low == high)
            break;
    }
    return middle;
}
size_t BlobCache_UpperBound (BlobCache const* cache, size_t order)
{
    size_t low = 0;
    size_t high = cache->size;
    size_t middle = (high + low) / 2;
    for (; ;)
    {
        if ( cache->blobs[middle].order <= order )
            low = middle + 1;
        else
            high = middle;

        middle = (high + low) / 2;
        if (low == high)
            break;
    }
    return middle;
}*/

void BlobCache_Init (BlobCache* cache)
{
    cache->size = 0;
}

/*void BlocbCache_InsertBlob (BlobCache* cache, size_t order, VBlob const* blob)
{
    if ( cache->size == countof (cache->blobs) )
    {
        VBlobRelease ( cache->blobs[0].blob );
        memmove ( cache->blobs, cache->blobs + 1, cache->size - 1 );
        -- cache->size;
    }
    size_t i = BlobCache_UpperBound ( cache, order );
    if ( i < cache->size )
        memmove ( cache->blobs + i + 1, cache->blobs + i, cache->size - i );

    cache->blobs[i].blob = blob;
    cache->blobs[i].order = order;
    ++ cache->size;
}

void BlocbCache_ReleaseLessThan (BlobCache* cache, size_t order)
{
    size_t bound = BlobCache_LowerBound ( cache, order );
    size_t i;

    for (i = 0; i < bound; ++i)
    {
        VBlobRelease ( cache->blobs[i].blob );
    }

    memmove ( cache->blobs, cache->blobs + i, cache->size - i );
    cache -> size -= i;
}*/

void BlobCache_Whack (BlobCache* cache)
{
    size_t i;
    for (i = 0; i < cache->size; ++i)
    {
        VBlobRelease ( cache->blobs[i].blob );
    }
    cache->size = 0;
}

void BlobCache_Add (BlobCache* cache, VBlob const* blob)
{
    if ( cache->size == countof (cache->blobs) )
    {
        VBlobRelease ( cache->blobs[0].blob );
        memmove ( cache->blobs, cache->blobs + 1, cache->size - 1 );
        -- cache->size;
    }

    cache->blobs[cache->size].blob = blob;
}

#endif

#endif