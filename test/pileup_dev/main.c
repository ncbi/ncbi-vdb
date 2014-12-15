/* pileup dev: main.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

#include <kapp/main.h>

#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <klib/printf.h>

#include <klib/rc.h>

#include "my_utils.h"

#ifndef min
#define min(x, y) ((y) < (x) ? (y) : (x))
#endif

#ifndef max
#define max(x, y) ((y) < (x) ? (x) : (y))
#endif


enum
{
    COL_REF_START,
    COL_REF_LEN
};
enum
{
    COL_MAX_SEQ_LEN,
    COL_PRIMARY_ALIGNMENT_IDS,
    COL_SEQ_START
};


typedef struct PileupIteratorState
{
    /* Static data */
    int64_t reference_start_id;
    int64_t reference_end_id;
    uint64_t reference_length;
    int64_t slice_start;
    uint64_t slice_length;
    uint32_t max_seq_len;

    /* Current State */
    uint64_t ref_pos;
    /*uint32_t seq_start; /* SEQ_START of the current reference row_id */
    Alignment_Cache cache_alignment;        /* Alignments intersecting slice */
    /*Alignment_Cache cache_alignment_at_pos; /* Alignments intersecting ref_pos */

} PileupIteratorState;


rc_t init_column_index (
    VCursor const* cursor,
    char const* const* column_names, uint32_t* column_index, size_t column_count,
    char* error_buf, size_t error_buf_size
    )
{
    rc_t rc = 0;
    size_t i;
    for ( i = 0; i < column_count; ++i )
    {
        rc = VCursorAddColumn ( cursor, & column_index [i], column_names [i] );
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VCursorAddColumn - [%s] failed with error: 0x%08x (%u) [%R]",
                column_names [i], rc, rc, rc);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';
            break;
        }
    }

    return rc;
}

rc_t init_vdb_objects (
    VDBManager const** mgr,
    VDatabase const** db,
    VTable const** table_ref,
    VTable const** table_pa,
    VCursor const** cursor_ref,
    VCursor const** cursor_pa,
    char const* db_path,
    char const* const* column_names_ref, uint32_t* column_index_ref, size_t column_count_ref,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa,
    char* error_buf,
    size_t error_buf_size
    )
{
    rc_t rc = 0;

    rc = VDBManagerMakeRead ( mgr, NULL );
    if ( rc != 0 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "ERROR: VDBManagerMakeRead failed with error: 0x%08x (%u) [%R]",
            rc, rc, rc);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return rc;
    }

    rc = VDBManagerOpenDBRead ( *mgr, db, NULL, db_path );
    if ( rc != 0 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "ERROR: VDBManagerOpenDBRead(%s) failed with error: 0x%08x (%u) [%R]",
            db_path, rc, rc, rc);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return rc;
    }

    rc = VDatabaseOpenTableRead ( *db, table_ref, "REFERENCE" );
    if ( rc != 0 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "ERROR: VDatabaseOpenTableRead(REFERENCE) failed with error: 0x%08x (%u) [%R]",
            rc, rc, rc);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return rc;
    }

    rc = VDatabaseOpenTableRead ( *db, table_pa, "PRIMARY_ALIGNMENT" );
    if ( rc != 0 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "ERROR: VDatabaseOpenTableRead(PRIMARY_ALIGNMENT) failed with error: 0x%08x (%u) [%R]",
            rc, rc, rc);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return rc;
    }

    rc = VTableCreateCursorRead ( *table_ref, cursor_ref );
    if ( rc != 0 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "ERROR: VTableCreateCursorRead(ref) failed with error: 0x%08x (%u) [%R]",
            rc, rc, rc);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return rc;
    }

    rc = init_column_index ( *cursor_ref, column_names_ref, column_index_ref, column_count_ref, error_buf, error_buf_size );
    if ( rc != 0 )
        return rc;

    rc = VCursorOpen ( *cursor_ref );
    if ( rc != 0 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "ERROR: VCursorOpen(ref) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return rc;
    }

    rc = VTableCreateCursorRead ( *table_pa, cursor_pa );
    if ( rc != 0 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "ERROR: VTableCreateCursorRead(pa) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return rc;
    }

    rc = init_column_index ( *cursor_pa, column_names_pa, column_index_pa, column_count_pa, error_buf, error_buf_size );
    if ( rc != 0 )
        return rc;

    rc = VCursorOpen ( *cursor_pa );
    if ( rc != 0 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "ERROR: VCursorOpen(pa) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return rc;
    }

    return rc;
}

void release_vdb_objects (
    VDBManager const* mgr,
    VDatabase const* db,
    VTable const* table_ref,
    VTable const* table_pa,
    VCursor const* cursor_ref,
    VCursor const* cursor_pa
    )
{
    /* TODO: process *Release-functions error return codes */
    if ( cursor_pa)
        VCursorRelease ( cursor_pa );
    if ( cursor_ref )
        VCursorRelease ( cursor_ref );
    if ( table_pa )
        VTableRelease ( table_pa );
    if ( table_ref )
        VTableRelease ( table_ref );
    if ( db )
        VDatabaseRelease ( db );
    if ( mgr )
        VDBManagerRelease ( mgr );
}


rc_t initial_add_pa_to_cache (
    PileupIteratorState* pileup_state,
    VCursor const* cursor_pa, uint32_t seq_start,
    int64_t const* pa_ids, uint32_t pa_count,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa,
    char* error_buf,
    size_t error_buf_size
    )
{
    size_t i = 0;
    rc_t rc = 0;
    int32_t ref_start;
    uint32_t ref_len; /* TODO: fix types */
    uint32_t row_len; /* TODO: fix types */
    int64_t slice_start = pileup_state->slice_start;
    int64_t slice_end = slice_start + (int64_t)pileup_state->slice_length;

    -- seq_start; /* SEQ_START is one-based coord, and we will need zero-based one */
    /* Making slice_start and slice_end relative to the current reference row_id */

    slice_start -= seq_start;
    slice_end -= seq_start;

    for (; i < (size_t)pa_count; ++i)
    {
        /* Read current PRIMARY_ALIGNMENT: REF_START and REF_LEN
           if it intersects slice - add this primary allignment to the cache
        */
        int64_t ref_end;

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

        ref_end = ref_start + (int64_t)ref_len;
        /*printf ("alignment row_id=%lld, align interval: [%ld, %lu], slice: [%lld, %llu]",
            pa_ids[i], ref_start, ref_end, slice_start, slice_end);*/

        if ( ! ((ref_start < slice_start && ref_end < slice_start) ||
                (ref_start > slice_end && ref_end > slice_end))
           )
        {
            rc = Alignment_Add ( & pileup_state->cache_alignment, pa_ids[i], ref_start, ref_len, seq_start );
            if ( rc != 0 )
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: Alignment_Add failed with error: 0x%08x (%u)", rc, rc);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
            /*printf (" CACHED\n");*/
        }
        else
        {
            /*printf (" ignored\n");*/
        }
    }
    
    return rc;
}

rc_t update_alignment_cache_to_cache (
    PileupIteratorState* pileup_state,
    VCursor const* cursor_pa, uint32_t seq_start,
    uint64_t ref_pos,
    int64_t const* pa_ids, uint32_t pa_count,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa,
    char* error_buf,
    size_t error_buf_size
    )
{
    size_t i = 0;
    rc_t rc = 0;
    int32_t ref_start;
    uint32_t ref_len; /* TODO: fix types */
    uint32_t row_len; /* TODO: fix types */
    int64_t slice_end = pileup_state->slice_start + (int64_t)pileup_state->slice_length;

    -- seq_start; /* SEQ_START is one-based coord, and we will need zero-based one */
    /* Making slice_start and slice_end relative to the current reference row_id */

    slice_end -= seq_start;
    ref_pos -= seq_start;

    for (; i < (size_t)pa_count; ++i)
    {
        /* Read current PRIMARY_ALIGNMENT: REF_START and REF_LEN
           if it intersects slice - add this primary allignment to the cache
        */
        int64_t ref_end;

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

        ref_end = ref_start + (int64_t)ref_len;
        /*printf ("alignment row_id=%lld, align interval: [%ld, %lu], slice: [%lld, %llu]",
            pa_ids[i], ref_start, ref_end, slice_start, slice_end);*/

        if ( ! ((ref_start < ref_pos && ref_end < ref_pos) ||
                (ref_start > slice_end && ref_end > slice_end))
           )
        {
            rc = Alignment_Add ( & pileup_state->cache_alignment, pa_ids[i], ref_start, ref_len, seq_start );
            if ( rc != 0 )
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: Alignment_Add failed with error: 0x%08x (%u)", rc, rc);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
            /*printf (" CACHED\n");*/
        }
        else
        {
            /*printf (" ignored\n");*/
        }
    }

    printf ("Updated cache size=%zu\n", pileup_state->cache_alignment.size);
    
    return rc;
}


rc_t initialize_ref_pos (
    PileupIteratorState* pileup_state,
    VCursor const* cursor_ref, VCursor const* cursor_pa,
    char const* const* column_names_ref, uint32_t* column_index_ref, size_t column_count_ref,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa,
    char* error_buf,
    size_t error_buf_size
    )
{
    int64_t row_id;
    uint64_t row_count;

    uint32_t max_seq_len, row_len;
    int64_t pa_ids[16*1024]; /* TODO: consider dynamically allocated buffer */

    int64_t slice_start_id, slice_end_id;

    rc_t rc = VCursorIdRange ( cursor_ref, 0, & row_id, & row_count );

    printf ("REFERENCE table: row_id=%lld, row_count=%llu\n", row_id, row_count);


    if ( row_count < 1 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "There is no rows in REFERENCE table");
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return (rc_t)(-1);
    }

    /* Read NAX_SEQ_LEN from the start_row_id and assume that it's the same for all the rest */

    rc = VCursorReadDirect ( cursor_ref, pileup_state->reference_start_id, column_index_ref [COL_MAX_SEQ_LEN],
                             sizeof (max_seq_len) * 8, & max_seq_len, 1, & row_len );
    if ( rc != 0 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "ERROR: VCursorReadDirect(ref) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return rc;
    }
    pileup_state->max_seq_len = max_seq_len;

    if ( row_len < 1 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "There is no MAX_SEQ_LEN column for row_id=%lld in REFERENCE table", row_id);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return (rc_t)(-1);
    }

    printf ("MAX_SEQ_LEN=%lu\n", max_seq_len);

    slice_start_id = pileup_state->reference_start_id + pileup_state->slice_start/max_seq_len;
    slice_end_id = pileup_state->reference_start_id + (pileup_state->slice_start + pileup_state->slice_length)/max_seq_len;

    printf ("slice position range: [%lld, %llu]\n", pileup_state->slice_start, pileup_state->slice_start + pileup_state->slice_length);
    printf ("slice id range: [%lld, %lld]\n", slice_start_id, slice_end_id);

    /* Read reference slice_start_id,
       read OVERLAP_*_POS to find out how
       many rows we need to read ahead of slice_start_id
       TODO: this is not implemented yet, insted we read just 10 rows ahead
    */


    /* Set cursor to <read_ahead_rows> rows ahead of slice_start_id
       and cache corresponding PRIMARY_ALIGNMENTS
    */

    {
        int64_t current_id = max (pileup_state->reference_start_id, slice_start_id - 10);
        int64_t stop_id = min (pileup_state->reference_end_id, slice_start_id);
        uint32_t seq_start;

        /* TODO: there is no checking for boundaries (end_*_id <= total rows and so on)*/

        for (; current_id  <= stop_id; ++current_id)
        {
            /* Read REFERENCE row's SEQ_START column to know the offset */
            rc = VCursorReadDirect ( cursor_ref, current_id,
                                     column_index_ref [COL_SEQ_START],
                                     sizeof (seq_start) * 8, & seq_start, 1, & row_len );
            if ( rc != 0 )
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: VCursorReadDirect(ref-seq_start) failed with error: 0x%08x (%u) [%R]",
                    rc, rc, rc);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
            /*pileup_state -> seq_start = seq_start;*/

            /* Read REFERENCE row's PRIMARY_ALIGNMENT_IDS column to iterate through them */
            rc = VCursorReadDirect ( cursor_ref, current_id,
                                     column_index_ref [COL_PRIMARY_ALIGNMENT_IDS],
                                     sizeof (*pa_ids) * 8, pa_ids, countof (pa_ids), & row_len );
            if ( rc != 0 )
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: VCursorReadDirect(ref-pa_ids) failed with error: 0x%08x (%u) [%R]",
                    rc, rc, rc);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
            printf ("Read %lu PRIMARY_ALIGNMENT_IDS for REFERENCE row_id=%lld:", row_len, current_id);
            {
                /*size_t i = 0;

                for (; i < row_len; ++i)
                    printf(" %lld", pa_ids [i]);*/

                printf ("\n");
            }

            /* For each PRIMARY_ALIGNMENT_ID in pa_ids: read its start, length and
               cache it if it intersects the slice
            */
            rc = initial_add_pa_to_cache ( pileup_state, cursor_pa, seq_start,
                        pa_ids, row_len, column_names_pa, column_index_pa, column_count_pa,
                        error_buf, error_buf_size );
            if ( rc != 0 )
                return rc;
        }
    }

    return rc;

}

PileupIteratorState pileup_state;


void print_current_state (PileupIteratorState const* pileup_state)
{
    size_t i, depth = 0;
    Alignment_CacheItem const* cache_item;
    /*uint64_t local_ref_pos = pileup_state->ref_pos - (pileup_state -> seq_start - 1);*/
    printf ("Current reference position=%llu, alignments at this positions are:\n", pileup_state->ref_pos);
    for (i = 0; i < pileup_state->cache_alignment.size; ++i)
    {
        Alignment_CacheItem const* cache_item = Alignment_Get ( & pileup_state->cache_alignment, i );
        uint64_t local_ref_pos = pileup_state->ref_pos - cache_item->seq_start;

        if ( cache_item->start <= local_ref_pos &&
             cache_item->start + cache_item->len >= local_ref_pos)
        {
            printf ("row_id=%lld\tstart=%lld\tlen=%llu\n",
               cache_item->row_id, cache_item->start, cache_item->len);
            ++ depth;
        }
    }
    printf ("Depth at position %llu is %zu\n", pileup_state->ref_pos, depth);
}


void remove_unneeded_alignments (PileupIteratorState* pileup_state, uint64_t ref_pos)
{
    /* TODO: this is not optimal version, needs to be rewritten */

    size_t i, size_old;
    Alignment_Cache* cache = & pileup_state->cache_alignment;
    static Alignment_CacheItem cache_data_old [ countof (cache->data) ];
    size_t debug_count = 0;

    memcpy ( cache_data_old, cache->data, cache->size * sizeof (cache_data_old[0]) );
    size_old = cache->size;

    Alignment_Init ( cache );

    /*ref_pos -= pileup_state->seq_start + 1;*/

    for (i = 0; i < size_old; ++ i )
    {
        Alignment_CacheItem const* item = & cache_data_old [i];
        uint64_t local_ref_pos = ref_pos - item->seq_start;
        if ( item->start + item->len > local_ref_pos )
            Alignment_Add ( cache, item->row_id, item->start, item->len, item->seq_start );
        else
        {
            //printf ("Removing item (start=%lld, end=%lld): ");
            ++ debug_count;
        }
    }
    printf ("Removed %zu elements (check: %zu)\n", debug_count, size_old - cache->size);
}

bool nextPileup (
    PileupIteratorState* pileup_state,
    VCursor const* cursor_ref, VCursor const* cursor_pa,
    char const* const* column_names_ref, uint32_t* column_index_ref, size_t column_count_ref,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa,
    char* error_buf,
    size_t error_buf_size
    )
{
    int64_t ref_row_id; /* current row_id */
    int64_t prev_ref_row_id;
    uint64_t ref_pos = pileup_state->ref_pos;

    if ( pileup_state->ref_pos == pileup_state->slice_start + pileup_state->slice_length )
    {
        error_buf[0] = '\0'; /* indicating that no error has occured */
        return false;
    }

    /* drop cached alignments that we will not need anymore */
    remove_unneeded_alignments ( pileup_state, ref_pos ); /* it's not an issue but this action is not rolled backed in the case of error below */

    /* Check if we moved to the next reference row_id,
       if yes - read it and add appropriate alignments to cache
    */

    prev_ref_row_id = pileup_state->reference_start_id + ref_pos / pileup_state->max_seq_len;
    ++ ref_pos;
    ref_row_id = pileup_state->reference_start_id + ref_pos / pileup_state->max_seq_len;

    if ( ref_row_id != prev_ref_row_id ) /* moved to the next row_id */
    {
        int64_t pa_ids[16*1024]; /* TODO: consider dynamically allocated buffer */
        uint32_t row_len;
        uint32_t seq_start;

        /* Read new SEQ_START */
        rc_t rc = VCursorReadDirect ( cursor_ref, ref_row_id,
                                 column_index_ref [COL_SEQ_START],
                                 sizeof (seq_start) * 8, & seq_start, 1, & row_len );
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VCursorReadDirect(ref-seq_start) failed with error: 0x%08x (%u) [%R]",
                rc, rc, rc);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return false;
        }

        /* Read REFERENCE row's PRIMARY_ALIGNMENT_IDS column to iterate through them */
        rc = VCursorReadDirect ( cursor_ref, ref_row_id,
                                 column_index_ref [COL_PRIMARY_ALIGNMENT_IDS],
                                 sizeof (*pa_ids) * 8, pa_ids, countof (pa_ids), & row_len );
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VCursorReadDirect(ref-pa_ids) failed with error: 0x%08x (%u) [%R]",
                rc, rc, rc);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return false;
        }
        printf ("Read %lu PRIMARY_ALIGNMENT_IDS for REFERENCE row_id=%lld\n", row_len, ref_row_id);

        /* For each PRIMARY_ALIGNMENT_ID in pa_ids: read its start, length and
           cache it if it intersects the slice
        */
        rc = update_alignment_cache_to_cache ( pileup_state, cursor_pa, seq_start, ref_pos,
                pa_ids, row_len, column_names_pa, column_index_pa, column_count_pa,
                error_buf, error_buf_size );
        if ( rc != 0 )
            return false;

        /*pileup_state -> seq_start = seq_start;*/
    }

    ++ pileup_state->ref_pos;
    return true;
}

void run ()
{
    VDBManager const* mgr = NULL;
    VDatabase const* db = NULL;

    VTable const* table_ref = NULL;
    VTable const* table_pa = NULL; 
    
    VCursor const* cursor_ref = NULL;
    VCursor const* cursor_pa = NULL;

    char const db_path[] = "SRR341578";
    char error_buf [512] = "";

    rc_t rc = 0;

    char const* column_names_ref[] =
    {
        "MAX_SEQ_LEN",
        "PRIMARY_ALIGNMENT_IDS",
        "SEQ_START"
    };
    uint32_t column_index_ref [ countof (column_names_ref) ];
    char const* column_names_pa[] =
    {
        "REF_START",
        "REF_LEN"
    };
    uint32_t column_index_pa [ countof (column_names_pa) ];

    Alignment_Init ( & pileup_state.cache_alignment );
    /*Alignment_Init ( & pileup_state.cache_alignment_at_pos );*/

    pileup_state.reference_start_id = 1032;
    pileup_state.reference_end_id = 1046;
    pileup_state.reference_length = 72482;
    pileup_state.slice_start = 20000-10;//18000;
    pileup_state.slice_length = 32;

    pileup_state.ref_pos = pileup_state.slice_start;

    rc = init_vdb_objects ( &mgr, &db, &table_ref, &table_pa, &cursor_ref, &cursor_pa,
                            db_path,
                            column_names_ref, column_index_ref, countof (column_names_ref),
                            column_names_pa, column_index_pa, countof (column_names_pa),
                            error_buf, sizeof (error_buf) );

    if ( rc != 0 )
    {
        printf ( "%s\n", error_buf );
        release_vdb_objects ( mgr, db, table_ref, table_pa, cursor_ref, cursor_pa );
        return;
    }

    rc = initialize_ref_pos ( & pileup_state, cursor_ref, cursor_pa,
        column_names_ref, column_index_ref, countof (column_names_ref),
        column_names_pa, column_index_pa, countof (column_names_pa),
        error_buf, sizeof (error_buf) );
    if ( rc != 0 )
    {
        printf ( "%s\n", error_buf );
        release_vdb_objects ( mgr, db, table_ref, table_pa, cursor_ref, cursor_pa );
        return;
    }

    {
        size_t i;
        Alignment_CacheItem const* cache_item;
        printf ("Slice start at position=%lld, alignment cache size=%zu\n", pileup_state.slice_start, pileup_state.cache_alignment.size);

        for (;;)
        {
            bool has_next;
            print_current_state ( & pileup_state );
            printf ("\n---------------------------------------\n");

            has_next = nextPileup ( & pileup_state, cursor_ref, cursor_pa,
                            column_names_ref, column_index_ref, countof (column_index_ref),
                            column_names_pa, column_index_pa, countof (column_index_pa),
                            error_buf, countof (error_buf) );
            if ( !has_next )
            {
                if ( error_buf[0] )
                    printf ( "%s\n", error_buf );

                break;
            }
        }
    }
    
    /* Can now iterate throug Pileup */

    release_vdb_objects ( mgr, db, table_ref, table_pa, cursor_ref, cursor_pa );
}

ver_t CC KAppVersion ( void )
{
    return 0;
}

rc_t CC KMain ( int argc, char *argv [] )
{
    /*printf ( "Pileup is under constructon...\n" );

    run();*/

    return 0;
}
