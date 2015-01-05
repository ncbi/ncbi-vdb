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

#include "CSRA1_Pileup.h"

typedef struct CSRA1_Pileup CSRA1_Pileup;
#define NGS_PILEUP CSRA1_Pileup
#include "NGS_Pileup.h"
#include "NGS_Cursor.h"

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>

#include <klib/rc.h>

#include "CSRA1_Reference.h"
#include "CSRA1_PileupEvent.h"

#ifndef min
#define min(x, y) ((y) < (x) ? (y) : (x))
#endif

#ifndef max
#define max(x, y) ((y) < (x) ? (x) : (y))
#endif


#define CACHE_IMPL_AS_LIST 1 /* ==1 if using DLList to store a set of cached alignments, otherwise - use array-based cache */
#define USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS 1 /* ==1 if using 1 blob to be able to have persistent pointer to PRIMARY_ALIGNMENT_IDS CellData, otherwise - copy data */
#define USE_SINGLE_BLOB_FOR_ALIGNMENTS 1 /* ==1 if using 1 blob to be able to have persistent pointer to PRIMARY_ALIGNMENT CellData, otherwise - copy data */

#if    USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1\
    || USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1

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

#include <klib/printf.h>

#include "NGS_String.h"

#include <sysalloc.h>

#ifndef countof
#define countof(arr) (sizeof(arr)/sizeof(arr[0]))
#endif

/* --------------------------------- */
/* TODO: here is a copy of test/pileup_dev code, needs to be rearranged later */

enum
{
    COL_REF_START,
    COL_REF_LEN
};
char const* column_names_pa[] =
{
    "REF_START",
    "REF_LEN"
};
uint32_t column_index_pa [ countof (column_names_pa) ];



typedef struct Alignment_CacheItem
{
#if CACHE_IMPL_AS_LIST == 1
    DLNode node; /* list node */
#endif
    int64_t row_id;
    int64_t start;
    uint64_t len;
    uint32_t seq_start; /* zero-based! Each alignment's REF_START (start) is relative to corresponding REFERENCE.SEQ_START, so we have to store this relation here */
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

typedef struct PileupIteratorState
{
    /* Static data */
    int64_t reference_start_id;
    int64_t reference_last_id;

    uint64_t total_row_count; /* row count of all references' row ids */

    int64_t slice_start;
    uint64_t slice_length;

    int64_t slice_start_id;
    int64_t slice_end_id;

    /* Blobs that we want to have manual control over */
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
    VBlob const* blob_alignment_ids; /* Here we store the blob containig current reference row */
#endif
#if USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1
    VBlob const* blob_alignments_ref_start;
    VBlob const* blob_alignments_ref_len;
#endif

    uint32_t max_seq_len;
    char ref_name [64]; /* TODO: now we have start and end ref id, so we have to get rid of this field */

    /* Current State */
    uint32_t current_seq_start;

    uint64_t ref_pos;
    Alignment_Cache cache_alignment;        /* Alignments intersecting ref_pos */

    size_t size_alignment_ids;
    size_t next_alignment_idx; /* index in alignment_ids pointing to the first id that has not been cached yet */
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS != 1
    size_t capacity_alignment_ids;
    int64_t* alignment_ids; /* alignment_ids of the current reference row_id */
#else
    int64_t const* alignment_ids; /* alignment_ids of the current reference row_id */
#endif

} PileupIteratorState;

/* Forward declarations can be removed now, and 'static' can be added to each function */
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

/* --------------------------------------- */

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



static rc_t init_column_index (
    ctx_t ctx,
    VCursor const* cursor,
    char const* const* column_names, uint32_t* column_index, size_t column_count
    )
{
    rc_t rc = 0;
    size_t i;
    for ( i = 0; i < column_count; ++i )
    {
        rc = VCursorAddColumn ( cursor, & column_index [i], column_names [i] );
        if ( rc != 0 )
        {
            INTERNAL_ERROR ( xcCursorCreateFailed,
                "ERROR: VCursorAddColumn - [%s] failed with error: 0x%08x (%u) [%R]",
                column_names [i], rc, rc, rc);
            break;
        }
    }

    return rc;
}

static void release_vdb_objects (
    VTable const** table_pa,
    VCursor const** cursor_pa
    )
{
    /* TODO: process *Release-functions error return codes */
    if ( *cursor_pa)
    {
        VCursorRelease ( *cursor_pa );
        *cursor_pa = NULL;
    }
    if ( *table_pa )
    {
        VTableRelease ( *table_pa );
        *table_pa = NULL;
    }
}


rc_t init_vdb_objects (
    ctx_t ctx,
    VDatabase const* db,
    VTable const** table_pa,
    VCursor const** cursor_pa,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa
    )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );

    rc_t rc = 0;

    rc = VDatabaseOpenTableRead ( db, table_pa, "PRIMARY_ALIGNMENT" );
    if ( rc != 0 )
    {
        INTERNAL_ERROR ( xcTableOpenFailed, 
            "ERROR: VDatabaseOpenTableRead(PRIMARY_ALIGNMENT) failed with error: 0x%08x (%u) [%R]", 
            rc, rc, rc );

        return rc;
    }

    rc = VTableCreateCursorRead ( *table_pa, cursor_pa );
    /*rc = VTableCreateCachedCursorRead ( *table_pa, cursor_pa, (size_t)64 << 30 );*/
    if ( rc != 0 )
    {
        INTERNAL_ERROR ( xcCursorCreateFailed,
            "ERROR: VTableCreateCursorRead(pa) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);

        return rc;
    }

    rc = init_column_index ( ctx, *cursor_pa, column_names_pa, column_index_pa, column_count_pa );
    if ( rc != 0 )
    {
        return rc;
    }

    rc = VCursorOpen ( *cursor_pa );
    if ( rc != 0 )
    {
        INTERNAL_ERROR ( xcCursorOpenFailed,
            "ERROR: VCursorOpen(pa) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);

        return rc;
    }

    return rc;
}

#if USE_BLOBS == 1
/* opens blob containing given row id
   if blob_ref contains given id already - do nothing
   otherwise - close current blob and open new one
*/
static rc_t open_blob_for_current_id (
    ctx_t ctx,
    int64_t id,
    VCursor const* cursor, VBlob const** blob, uint32_t col_idx
    )
{
    rc_t rc = 0;
    if (*blob == NULL)
    {
        rc = VCursorGetBlobDirect ( cursor, blob, id, col_idx);
        if ( rc != 0)
        {
            INTERNAL_ERROR ( xcCursorAccessFailed,
                "ERROR: VCursorGetBlobDirect(init) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
            return rc;
        }
    }
    else
    {
        int64_t start_id;
        uint64_t count;
        rc = VBlobIdRange ( *blob, & start_id, & count );
        if ( rc != 0)
        {
            INTERNAL_ERROR ( xcCursorAccessFailed,
                "ERROR: VBlobIdRange failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
            return rc;
        }

        if (id >= start_id + (int64_t)count || id < start_id)
        {
            VBlobRelease ( *blob );
            *blob = NULL;

            rc = VCursorGetBlobDirect ( cursor, blob, id, col_idx);
            if ( rc != 0)
            {
                INTERNAL_ERROR ( xcCursorAccessFailed,
                    "ERROR: VCursorGetBlobDirect failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
                return rc;
            }
        }
    }
    return rc;
}
#endif


static rc_t add_ref_row_to_cache (
    ctx_t ctx,
    PileupIteratorState* pileup_state,
    VCursor const* cursor_pa, uint32_t seq_start,
    uint64_t ref_pos,
    int64_t const* pa_ids, uint32_t pa_count,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa
    )
{
    size_t i = 0;
    rc_t rc = 0;
    int32_t ref_start;
    uint32_t ref_len; /* TODO: fix types */
    uint32_t row_len; /* TODO: fix types */
    int64_t slice_start = pileup_state->slice_start;

    -- seq_start; /* SEQ_START is one-based coord, and we will need zero-based one */
    /* Making slice_start and slice_end relative to the current reference row_id */

    slice_start -= seq_start;
    ref_pos -= seq_start;

    for (; i < (size_t)pa_count; ++i)
    {
        /* Read current PRIMARY_ALIGNMENT: REF_START and REF_LEN
           if it intersects slice - add this primary alignment to the cache
        */
        int64_t ref_end;

#if USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1
        rc = open_blob_for_current_id ( ctx, pa_ids[i],
            cursor_pa, & pileup_state->blob_alignments_ref_start,
            column_index_pa [COL_REF_START]);
        if (rc != 0)
            return rc;

        rc = VBlobRead ( pileup_state->blob_alignments_ref_start,
            pa_ids[i], sizeof (ref_start) * 8, & ref_start, 1, & row_len );
        if ( rc != 0 )
        {
            INTERNAL_ERROR ( xcCursorAccessFailed,
                "ERROR: VBlobRead(pa, %s) failed with error: 0x%08x (%u) [%R]",
                column_names_pa[COL_REF_START], rc, rc, rc);
            return rc;
        }

        rc = open_blob_for_current_id ( ctx, pa_ids[i],
            cursor_pa, & pileup_state->blob_alignments_ref_len,
            column_index_pa [COL_REF_LEN]);
        if (rc != 0)
            return rc;

        rc = VBlobRead ( pileup_state->blob_alignments_ref_len,
            pa_ids[i], sizeof (ref_len) * 8, & ref_len, 1, & row_len );
        if ( rc != 0 )
        {
            INTERNAL_ERROR ( xcCursorAccessFailed,
                "ERROR: VBlobRead(pa, %s) failed with error: 0x%08x (%u) [%R]",
                column_names_pa[COL_REF_LEN], rc, rc, rc);
            return rc;
        }
#else
        /* TODO: this branch might not work due to transfer from test to ngs */
        rc = VCursorReadDirect ( cursor_pa, pa_ids[i], column_index_pa [COL_REF_START],
                                 sizeof (ref_start) * 8, & ref_start, 1, & row_len );
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VCursorReadDirect(pa, %s) failed with error: 0x%08x (%u) [%R]",
                column_names_pa[COL_REF_START], rc, rc, rc);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return rc;
        }
        rc = VCursorReadDirect ( cursor_pa, pa_ids[i], column_index_pa [COL_REF_LEN],
                                 sizeof (ref_len) * 8, & ref_len, 1, & row_len );
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VCursorReadDirect(pa, %s) failed with error: 0x%08x (%u) [%R]",
                column_names_pa[COL_REF_LEN], rc, rc, rc);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return rc;
        }
#endif

        ref_end = ref_start + (int64_t)ref_len;

        /* skip all alignments that are to the left of slice (if we have slice specified) */
        if ( pileup_state->slice_length && ref_end < slice_start)
            continue;

        /* stop processing current alignments that are to the right of slice (if we have slice specified) */
        if ( pileup_state->slice_length && ref_start > (slice_start + (int64_t)pileup_state->slice_length))
        {
            /*pileup_state->next_alignment_idx = pileup_state->size_alignment_ids;*/
            break;
        }

        /* if alignment intersects slice_start - cache it */
        if ( ref_start <= (int64_t)ref_pos && ref_end >= (int64_t)ref_pos )
        {
            rc = Alignment_Add ( & pileup_state->cache_alignment, pa_ids[i], ref_start, ref_len, seq_start );
            if ( rc != 0 )
            {
                INTERNAL_ERROR ( xcCursorAccessFailed,
                    "ERROR: Alignment_Add (%lld, %d, %u, %u) failed with error: 0x%08x (%u) [%R], cache_size=%zu, ref_start_id=%lld, name=\"%s\", ref_pos=%llu",
                    pa_ids[i], ref_start, ref_len, seq_start, rc, rc, rc,
                    pileup_state->cache_alignment.size,
                    pileup_state->reference_start_id,
                    pileup_state->ref_name,
                    pileup_state->ref_pos);
                return rc;
            }
        }
        else if ( ref_start > ref_pos )
        {
            /*Alignment_AddPrefetch ();*/
/*            rc = Alignment_Add ( & pileup_state->cache_alignment, pa_ids[i], ref_start, ref_len, seq_start );
            if ( rc != 0 )
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: Alignment_Add failed with error: 0x%08x (%u)", rc, rc);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
            ++i;*/
            break; /* This is only for alignments ordered by ref_start !!*/
        }
    }
    pileup_state->next_alignment_idx =
        & pa_ids[i] - pileup_state->alignment_ids; /* TODO: i-th alignment will be read again on the next step, so consider caching it here and ignoring it when looking it up for cached items relevant to current ref_pos */

    return rc;
}



static rc_t initialize_ref_pos (
    ctx_t ctx,
    PileupIteratorState* pileup_state,
    NGS_Cursor const* curs_ref, VCursor const* cursor_pa,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa
    )
{
    int64_t row_id;
    uint64_t row_count;
    uint32_t dummy;
    void const* buf;
    rc_t rc;

    uint32_t max_seq_len, row_len;

    NGS_CursorGetRowRange ( curs_ref, ctx, & row_id, & row_count );

    if ( row_count < 1 )
    {
        USER_ERROR ( xcRowNotFound, "There is no rows in REFERENCE table");

        return (rc_t)(-1);
    }
    pileup_state->total_row_count = row_count;

    /* We don't know the reference end id use its name to notice the moment when it changes - this will be the end */
    NGS_CursorCellDataDirect ( curs_ref, ctx, pileup_state->reference_start_id, reference_NAME,
        & dummy, & buf, NULL, & row_len );
    if ( FAILED () )
    {
        rc = ctx->rc;
        INTERNAL_ERROR( xcCursorAccessFailed,
            "ERROR: NGS_CursorCellDataDirect(ref) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
        return ctx->rc;
    }
    string_copy ( pileup_state->ref_name, countof(pileup_state->ref_name), (char const*)buf, row_len );
    pileup_state->ref_name[ min ( countof(pileup_state->ref_name) - 1, row_len) ] = '\0';

    /* Read MAX_SEQ_LEN from the start_row_id and assume that it's the same for all the rest */
    NGS_CursorCellDataDirect ( curs_ref, ctx, pileup_state->reference_start_id, reference_MAX_SEQ_LEN,
        & dummy, & buf, NULL, & row_len );
    max_seq_len = *(uint32_t*)buf;
    if ( FAILED () )
    {
        rc = ctx->rc;
        INTERNAL_ERROR( xcCursorAccessFailed,
            "ERROR: NGS_CursorCellDataDirect(ref-seq_start) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
        return ctx->rc;
    }

    if ( row_len < 1 )
    {
        USER_ERROR ( xcRowNotFound, "There is no MAX_SEQ_LEN column for row_id=%ld in REFERENCE table", row_id);
        return (rc_t)(-1);
    }
    pileup_state->max_seq_len = max_seq_len;

    pileup_state->slice_start_id = pileup_state->reference_start_id + pileup_state->slice_start/max_seq_len;
    pileup_state->slice_end_id = pileup_state->slice_length != 0 ?
        pileup_state->reference_start_id + (pileup_state->slice_start + (int64_t)pileup_state->slice_length)/max_seq_len :
        (int64_t)pileup_state->total_row_count;

    /* lazy add of PRIMARY_ALIGNMENT_IDS column */
    NGS_CursorCellDataDirect ( curs_ref, ctx, pileup_state->reference_start_id,
        reference_PRIMARY_ALIGNMENT_IDS, & dummy, & buf, NULL, & row_len );
    if ( FAILED () )
    {
        rc = ctx->rc;
        INTERNAL_ERROR( xcCursorAccessFailed,
            "ERROR: NGS_CursorCellDataDirect(pa) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
        return ctx->rc;
    }


    {
        int64_t current_id = max (pileup_state->reference_start_id, pileup_state->slice_start_id - 10);
        int64_t stop_id = pileup_state->slice_start_id;
        uint32_t seq_start;
        uint32_t dummy;
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS != 1
        int64_t const* alignment_ids;
#endif

        for (; ; ++current_id)
        {
            /* We don't know the current reference end_id
               read it's name and break when it changes

               TODO: Now we have access to end_id - use it
            */

            char ref_name[ countof (pileup_state->ref_name) ];
            NGS_CursorCellDataDirect ( curs_ref, ctx, current_id, reference_NAME,
                & dummy, & buf, NULL, & row_len );
            if ( FAILED () )
            {
                rc = ctx->rc;
                INTERNAL_ERROR( xcCursorAccessFailed,
                    "ERROR: NGS_CursorCellDataDirect(ref) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
                return ctx->rc;
            }
            string_copy ( ref_name, countof(ref_name), (char const*)buf, row_len );
            ref_name[ min ( countof(ref_name) - 1, row_len) ] = '\0';

            if ( current_id > stop_id || string_cmp ( ref_name, string_size ( ref_name ), 
                                                      pileup_state -> ref_name, string_size ( pileup_state -> ref_name ), 
                                                      string_size ( ref_name ) ) )
                break;

#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
            rc = open_blob_for_current_id ( ctx, current_id,
                NGS_CursorGetVCursor (curs_ref), & pileup_state->blob_alignment_ids,
                NGS_CursorGetColumnIndex ( curs_ref, reference_PRIMARY_ALIGNMENT_IDS));
            if (rc != 0)
                return rc;
#endif

            /* Read REFERENCE row's SEQ_START column to know the offset */

            seq_start = NGS_CursorGetUInt32 (curs_ref, ctx, current_id, reference_SEQ_START);
            if ( FAILED () )
            {
                rc = ctx->rc;
                INTERNAL_ERROR( xcCursorAccessFailed,
                    "ERROR: VCursorReadDirect(ref-seq_start) failed with error: 0x%08x (%u) [%R]",
                    rc, rc, rc);
                return ctx->rc;
            }
            pileup_state->current_seq_start = seq_start;

            /* Read REFERENCE row's PRIMARY_ALIGNMENT_IDS column to iterate through them */
            /* elem_bits = sizeof (*pileup_state->alignment_ids) * 8;*/
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
            rc = VBlobCellData ( pileup_state->blob_alignment_ids, current_id,
                & dummy, ( const void** ) & pileup_state->alignment_ids, NULL, & row_len );
            if ( rc != 0 )
            {
                INTERNAL_ERROR ( xcCursorAccessFailed,
                    "ERROR: VBlobCellData(ref-pa_ids) failed with error: 0x%08x (%u) [%R], row_len=%u",
                    rc, rc, rc, row_len);
                return rc;
            }
            pileup_state -> size_alignment_ids = row_len;
#else

            /* TODO: this branch isn't working now after moving code from test to ngs */

            rc = VCursorCellDataDirect ( cursor_ref, current_id,
                        column_index_ref [COL_PRIMARY_ALIGNMENT_IDS],
                        NULL,
                        (void const**)(& alignment_ids), 0, & row_len );

            /*rc = VCursorReadDirect ( cursor_ref, current_id,
                        column_index_ref [COL_PRIMARY_ALIGNMENT_IDS],
                        sizeof (*pileup_state->alignment_ids) * 8,
                        pileup_state->alignment_ids,
                        countof (pileup_state->alignment_ids), & row_len );*/
            if ( rc != 0 )
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: VCursorCellDataDirect(ref-pa_ids) failed with error: 0x%08x (%u) [%R], row_len=%u",
                    rc, rc, rc, row_len);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
            rc = PileupIteratorState_SetAlignmentIds ( pileup_state, alignment_ids, row_len );
            if ( rc != 0 )
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: PileupIteratorState_SetAlignmentIds failed with error: 0x%08x (%u), row_len=%u",
                    rc, rc, row_len);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
#endif
            pileup_state->next_alignment_idx = 0;

            /* For each PRIMARY_ALIGNMENT_ID in alignment_ids: read its start, length and
               cache it if it intersects the starting position
            */
            rc = add_ref_row_to_cache ( ctx, pileup_state, cursor_pa, seq_start,
                        pileup_state->slice_start,
                        pileup_state->alignment_ids, row_len,
                        column_names_pa, column_index_pa, column_count_pa);
            if ( rc != 0 )
                return rc;
        }
    }

    return rc;
}


static void remove_unneeded_alignments (PileupIteratorState* pileup_state, uint64_t ref_pos)
{
    /*int64_t max_removed_id = 0;*/
#if CACHE_IMPL_AS_LIST == 1
    Alignment_CacheItem* item = ( Alignment_CacheItem* ) DLListHead ( & pileup_state->cache_alignment.list_alignments );

    for (; item != NULL; )
    {
        uint64_t local_ref_pos = ref_pos - item->seq_start;
        if ( item->start + item->len <= local_ref_pos ) /* not "<" here because local_ref_pos here is the old position that will be incremented right after we exit from this function */
        {
            Alignment_CacheItem* item_cur = item;
            item = (Alignment_CacheItem*) DLNodeNext( & item->node );

            DLListUnlink ( & pileup_state->cache_alignment.list_alignments, & item_cur->node);
            Alignment_CacheItemWhack ( & item_cur->node, NULL );
            -- pileup_state->cache_alignment.size;
        }
        else
            item = (Alignment_CacheItem*) DLNodeNext( & item->node );
    }
#else
    size_t i_src, i_dst;
    Alignment_Cache* cache = & pileup_state->cache_alignment;
    size_t size = cache->size;

    for (i_src = 0; i_src < size; ++ i_src)
    {
        Alignment_CacheItem const* item = & cache->data [i_src];
        uint64_t local_ref_pos = ref_pos - item->seq_start;
        if ( item->start + item->len > local_ref_pos ) /* not ">=" here because local_ref_pos here is the old position that will be incremented right after we exit from this function */
            break;
    }

    for (i_dst = 0; i_src < size; ++ i_src)
    {
        Alignment_CacheItem const* item = & cache->data [i_src];
        uint64_t local_ref_pos = ref_pos - item->seq_start;
        if ( item->start + item->len > local_ref_pos ) /* not ">=" here because local_ref_pos here is the old position that will be incremented right after we exit from this function */
        {
            if (i_dst != i_src)
                cache->data [i_dst] = cache->data [i_src];
            ++ i_dst;
        }
    }

    cache->size = i_dst;

#endif

}


static bool next_pileup (
    ctx_t ctx,
    PileupIteratorState* pileup_state,
    NGS_Cursor const* curs_ref, VCursor const* cursor_pa,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa
    )
{
    int64_t ref_row_id; /* current row_id */
    int64_t prev_ref_row_id;
    uint64_t ref_pos = pileup_state->ref_pos;
    rc_t rc;

    /* TODO: check the case when slice_end is beyond the reference end*/
    if ( pileup_state->slice_length && pileup_state->ref_pos == pileup_state->slice_start + pileup_state->slice_length )
    {
        return false;
    }

    /* drop cached alignments that we will not need anymore */
    remove_unneeded_alignments ( pileup_state, ref_pos );

    /* Check if we moved to the next reference row_id,
       if yes - read it and add appropriate alignments to cache
    */

    prev_ref_row_id = pileup_state->reference_start_id + ref_pos / pileup_state->max_seq_len;
    ++ ref_pos;
    ref_row_id = pileup_state->reference_start_id + ref_pos / pileup_state->max_seq_len;

    if ( ref_row_id != prev_ref_row_id ) /* moved to the next row_id */
    {
        uint32_t dummy;
        uint32_t row_len;
        uint32_t seq_start;
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS != 1
        int64_t const* alignment_ids;
#endif

        /* TODO: use LastRowId from reference*/
        char ref_name[ countof (pileup_state->ref_name) ];
        void const* buf;

        if ( ref_row_id < pileup_state->slice_start_id || ref_row_id > pileup_state->slice_end_id )
        {
            return false;
        }

        NGS_CursorCellDataDirect ( curs_ref, ctx, ref_row_id, reference_NAME,
            & dummy, & buf, NULL, & row_len );
        if ( FAILED () )
        {
            rc = ctx->rc;
            INTERNAL_ERROR( xcCursorAccessFailed,
                "ERROR: NGS_CursorCellDataDirect(ref) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
            return false;
        }
        string_copy ( ref_name, countof(ref_name), (char const*)buf, row_len );
        ref_name[ min ( countof(ref_name) - 1, row_len) ] = '\0';
        if ( string_cmp ( ref_name, string_size ( ref_name ), 
                          pileup_state->ref_name, string_size ( pileup_state->ref_name ), 
                          string_size ( ref_name ) ) )
        {
            return false;
        }

#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
        rc = open_blob_for_current_id ( ctx, ref_row_id,
            NGS_CursorGetVCursor(curs_ref), & pileup_state->blob_alignment_ids,
            NGS_CursorGetColumnIndex ( curs_ref, reference_PRIMARY_ALIGNMENT_IDS));
        if (rc != 0)
            return false;
#endif

        /* Read new SEQ_START */

        seq_start = NGS_CursorGetUInt32 (curs_ref, ctx, ref_row_id, reference_SEQ_START);
        if ( FAILED () )
        {
            rc = ctx->rc;
            INTERNAL_ERROR( xcCursorAccessFailed,
                "ERROR: VCursorReadDirect(ref-seq_start) failed with error: 0x%08x (%u) [%R]",
                rc, rc, rc);
            return false;
        }
        pileup_state->current_seq_start = seq_start;

        /* Read REFERENCE row's PRIMARY_ALIGNMENT_IDS column to iterate through them */
        /* elem_bits = sizeof (*pileup_state->alignment_ids) * 8; */
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
        rc = VBlobCellData ( pileup_state->blob_alignment_ids, ref_row_id,
            & dummy, ( const void ** ) & pileup_state->alignment_ids, NULL, & row_len );
        if ( rc != 0 )
        {
            INTERNAL_ERROR( xcCursorAccessFailed,
                "ERROR: VBlobCellData(ref-pa_ids) failed with error: 0x%08x (%u) [%R], row_len=%u",
                rc, rc, rc, row_len);
            return false;
        }
        pileup_state -> size_alignment_ids = row_len;
#else
        /* TODO: this branch isn't working now after moving code from test to ngs */
        rc = VCursorCellDataDirect ( cursor_ref, ref_row_id,
                    column_index_ref [COL_PRIMARY_ALIGNMENT_IDS],
                    NULL,
                    (void const**)(& alignment_ids), 0, & row_len );

        /*rc = VCursorReadDirect ( cursor_ref, ref_row_id,
                                 column_index_ref [COL_PRIMARY_ALIGNMENT_IDS],
                                 sizeof (*pileup_state->alignment_ids) * 8,
                                 pileup_state->alignment_ids,
                                 countof (pileup_state->alignment_ids),
                                 & row_len );*/
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VCursorCellDataDirect(ref-pa_ids) failed with error: 0x%08x (%u) [%R], row_len=%u",
                rc, rc, rc, row_len);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return false;
        }
        rc = PileupIteratorState_SetAlignmentIds ( pileup_state, alignment_ids, row_len );
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: PileupIteratorState_SetAlignmentIds failed with error: 0x%08x (%u), row_len=%u",
                rc, rc, row_len);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return rc;
        }
#endif
        pileup_state->next_alignment_idx = 0;

        /* For each PRIMARY_ALIGNMENT_ID in pa_ids: read its start, length and
           cache it if it intersects the slice
        */
        rc = add_ref_row_to_cache ( ctx, pileup_state, cursor_pa, seq_start, ref_pos,
                pileup_state->alignment_ids, row_len,
                column_names_pa, column_index_pa, column_count_pa);
        if ( rc != 0 )
            return false;

        /*pileup_state -> seq_start = seq_start;*/
    }
    else
    {
        /* read remaining alignment_ids and check if they must be cached */
        size_t count = pileup_state->size_alignment_ids - pileup_state->next_alignment_idx;
        if (count > 0)
        {
            rc_t rc = add_ref_row_to_cache ( ctx, pileup_state, cursor_pa,
                pileup_state->current_seq_start, ref_pos,
                & pileup_state->alignment_ids[ pileup_state->next_alignment_idx ],
                (uint32_t)count,
                column_names_pa, column_index_pa, column_count_pa);
            if ( rc != 0 )
                return false;
        }
    }

    ++ pileup_state->ref_pos;
    return true;
}



/* --------------------------------------- */
struct CSRA1_Pileup
{
    NGS_Pileup dad;   
    const NGS_String * ref_spec;
    
    bool primary;
    bool secondary;

    VDatabase const* db;
    VTable const* table_pa;

    NGS_Cursor const* curs_ref;
    VCursor const* cursor_pa;

    /* TODO: put all the fields of pileup_state into CSRA1_Pileup directly */
    PileupIteratorState pileup_state;

    bool is_started;    /* NextIterator has been called at least one time */
    bool is_finished;   /* NextIterator has seen the end, no more operations are allowed */
};

uint64_t DEBUG_CSRA1_Pileup_GetRefPos (void const* self)
{
    return ((CSRA1_Pileup*)self)->pileup_state.ref_pos;
}

static void                     CSRA1_PileupWhack                   ( CSRA1_Pileup * self, ctx_t ctx );
static struct NGS_String *      CSRA1_PileupGetReferenceSpec        ( const CSRA1_Pileup * self, ctx_t ctx );
static int64_t                  CSRA1_PileupGetReferencePosition    ( const CSRA1_Pileup * self, ctx_t ctx );
static struct NGS_PileupEvent * CSRA1_PileupGetEvents               ( const CSRA1_Pileup * self, ctx_t ctx );
static unsigned int             CSRA1_PileupGetDepth                ( const CSRA1_Pileup * self, ctx_t ctx );
static bool                     CSRA1_PileupIteratorGetNext         ( CSRA1_Pileup * self, ctx_t ctx );

static NGS_Pileup_vt CSRA1_Pileup_vt_inst =
{
    {
        /* NGS_Refcount */
        CSRA1_PileupWhack
    },

    CSRA1_PileupGetReferenceSpec,    
    CSRA1_PileupGetReferencePosition,
    CSRA1_PileupGetEvents,           
    CSRA1_PileupGetDepth,            
    CSRA1_PileupIteratorGetNext,     
};

void CSRA1_PileupInit ( CSRA1_Pileup * self, 
                        ctx_t ctx, 
                        const char *clsname, 
                        const char *instname, 
                        const NGS_String* ref_spec, 
                        bool wants_primary, 
                        bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    assert ( self );
    assert ( ref_spec );
    
    TRY ( NGS_PileupInit ( ctx, & self -> dad, & CSRA1_Pileup_vt_inst, clsname, instname ) ) 
    {
        self -> ref_spec = NGS_StringDuplicate ( ref_spec, ctx );
        self -> primary = wants_primary;
        self -> secondary = wants_secondary;

        self->db         = NULL;
        self->table_pa   = NULL;
        self->curs_ref   = NULL;
        self->cursor_pa  = NULL;

        self -> pileup_state.ref_name [0] = '\0';

        self -> is_started = false;
        self -> is_finished = false;

        
#if CACHE_IMPL_AS_LIST == 1
        self->pileup_state.cache_alignment.size = 0;
        Alignment_InitCacheWithNull ( & self->pileup_state.cache_alignment );
#else
        self->pileup_state.cache_alignment.data = NULL;
#endif
        self->pileup_state.alignment_ids = NULL;
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
        self->pileup_state.blob_alignment_ids = NULL;
#endif
#if USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1
        self->pileup_state.blob_alignments_ref_start = NULL;
        self->pileup_state.blob_alignments_ref_len = NULL;
#endif

    }
}

void CSRA1_PileupWhack ( CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );

    release_vdb_objects ( & self -> table_pa, & self -> cursor_pa );
    Alignment_Release ( & self -> pileup_state.cache_alignment );
    PileupIteratorState_Release ( & self -> pileup_state );

    self -> is_started = false;
    self -> is_finished = true;
   
    NGS_StringRelease ( self -> ref_spec, ctx );

    VDatabaseRelease ( self -> db );
    self -> db = NULL;

    NGS_CursorRelease ( self -> curs_ref, ctx );
    self -> curs_ref = NULL;
}

struct NGS_String * CSRA1_PileupGetReferenceSpec ( const CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    if ( ! self -> is_started )
    {
        USER_ERROR ( xcIteratorUninitialized, "Pileup accessed before a call to PileupIteratorNext()" );
        return NULL;
    }
    else if ( self -> is_finished )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return NULL;
    }
    
    return NGS_StringDuplicate ( self -> ref_spec, ctx );
}

int64_t CSRA1_PileupGetReferencePosition ( const CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    if ( ! self -> is_started )
    {
        USER_ERROR ( xcIteratorUninitialized, "Pileup accessed before a call to PileupIteratorNext()" );
        return 0;
    }
    else if ( self -> is_finished )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return 0;
    }
    
    return self -> pileup_state . ref_pos;
}

struct NGS_PileupEvent * CSRA1_PileupGetEvents ( const CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    if ( ! self -> is_started )
    {
        USER_ERROR ( xcIteratorUninitialized, "Pileup accessed before a call to PileupIteratorNext()" );
        return NULL;
    }
    else if ( self -> is_finished )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return NULL;
    }
    
    return CSRA1_PileupEventIteratorMake ( ctx, self -> ref_spec, self -> primary, self -> secondary );
}

unsigned int CSRA1_PileupGetDepth ( const CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    if ( ! self -> is_started )
    {
        USER_ERROR ( xcIteratorUninitialized, "Pileup accessed before a call to PileupIteratorNext()" );
        return 0;
    }
    else if ( self -> is_finished )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return 0;
    }
    
    return (unsigned int) self -> pileup_state . cache_alignment . size;
}

bool CSRA1_PileupIteratorGetNext ( CSRA1_Pileup * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    rc_t rc;

    if ( self -> is_started )
    {
        bool ret = next_pileup ( ctx, & self -> pileup_state,
            self -> curs_ref, self -> cursor_pa,
            column_names_pa, column_index_pa, countof (column_names_pa));
        if ( ! ret )
        {
            self -> is_finished = true;
        }
        return ret;
    }
    else 
    {
        rc = initialize_ref_pos ( ctx, & self -> pileup_state,
            self -> curs_ref, self -> cursor_pa,
            column_names_pa, column_index_pa, countof (column_names_pa));
        if (rc == 0)
        {
            self -> is_started = true;
            self -> is_finished = false;
            return true;
        }
        else
        {
            self -> is_started = true;
            self -> is_finished = true;
            return false;
        }
    }
}

struct NGS_Pileup* CSRA1_PileupIteratorMake ( ctx_t ctx,
    VDatabase const* db, NGS_Cursor const* curs_ref,
    const NGS_String* ref_spec,
    int64_t first_row_id, int64_t last_row_id,
    bool wants_primary, bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    CSRA1_Pileup * ref;
    rc_t rc = 0;

    assert ( db != NULL );
    assert ( curs_ref != NULL );

    ref = calloc ( 1, sizeof * ref );
    if ( ref == NULL )
        SYSTEM_ERROR ( xcNoMemory, 
                       "allocating CSRA1_Pileup on '%.*s'", 
                       NGS_StringSize ( ref_spec, ctx ), 
                       NGS_StringData ( ref_spec, ctx ) );
    else
    {
#if _DEBUGGING
        char instname [ 256 ];
        string_printf ( instname, 
                        sizeof instname, 
                        NULL, 
                        "%.*s", 
                        NGS_StringSize ( ref_spec, ctx ), 
                        NGS_StringData ( ref_spec, ctx ) );
        instname [ sizeof instname - 1 ] = 0;
#else
        const char *instname = "";
#endif
        TRY ( CSRA1_PileupInit ( ref, ctx, "CSRA1_Pileup", instname, ref_spec, wants_primary, wants_secondary ) )
        {
            ref -> db = db;
            VDatabaseAddRef ( ref -> db );

            ref -> curs_ref = NGS_CursorDuplicate ( curs_ref, ctx );

            Alignment_Init ( & ref->pileup_state.cache_alignment );
            PileupIteratorState_Init ( & ref->pileup_state );


            ref -> pileup_state.reference_start_id = first_row_id;
            ref -> pileup_state.reference_last_id = last_row_id;

            /* TODO: add slice boundaries*/
            ref -> pileup_state.slice_start = 0; /*20000-10;*/
            ref -> pileup_state.slice_length = 0; /*32;*/

            ref -> pileup_state.ref_pos = ref -> pileup_state.slice_start;

            rc = init_vdb_objects ( ctx,
                    ref->db, & ref->table_pa,
                    & ref->cursor_pa,
                    column_names_pa, column_index_pa, countof (column_names_pa));

            if ( rc == 0 )
            {
                return ( NGS_Pileup* ) ref;
            }
            else
            {
                CSRA1_PileupWhack ( ref, ctx );
            }
        }
        free ( ref );
    }

    return NULL;
}

