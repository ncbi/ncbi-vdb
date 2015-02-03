/* pileup dev: main.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>

#include <kapp/main.h>

#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>

#include <klib/printf.h>

#include <klib/rc.h>

#include "my_utils.h"

#if USE_BLOBS == 1
#include <vdb/blob.h>
#endif

#ifndef min
#define min(x, y) ((y) < (x) ? (y) : (x))
#endif

#ifndef max
#define max(x, y) ((y) < (x) ? (x) : (y))
#endif

#if 0

enum
{
    COL_REF_START,
    COL_REF_LEN
};
enum
{
    COL_MAX_SEQ_LEN,
    COL_PRIMARY_ALIGNMENT_IDS,
    COL_SEQ_START,
    COL_NAME
};


static rc_t init_column_index (
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

#if USE_BLOBS == 1
/* opens blob containing given row id
   if blob_ref contains given id already - do nothing
   otherwise - close current blob and open new one
*/
static rc_t open_blob_for_current_id (
    int64_t id,
    VCursor const* cursor_ref, VBlob const** blob_ref, uint32_t col_idx,
    char* error_buf,
    size_t error_buf_size
    )
{
    rc_t rc = 0;
    if (*blob_ref == NULL)
    {
        rc = VCursorGetBlobDirect ( cursor_ref, blob_ref, id, col_idx);
        if ( rc != 0)
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VCursorGetBlobDirect(init) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return rc;
        }
    }
    else
    {
        int64_t start_id;
        uint64_t count;
        rc = VBlobIdRange ( *blob_ref, & start_id, & count );
        if ( rc != 0)
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VBlobIdRange failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return rc;
        }

        if (id >= start_id + (int64_t)count || id < start_id)
        {
            VBlobRelease ( *blob_ref );
            *blob_ref = NULL;

            rc = VCursorGetBlobDirect ( cursor_ref, blob_ref, id, col_idx);
            if ( rc != 0)
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: VCursorGetBlobDirect failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
        }
    }
    return rc;
}
#endif


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
    /*rc = VTableCreateCachedCursorRead ( *table_ref, cursor_ref, (size_t)64 << 30 );*/
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
    /*rc = VTableCreateCachedCursorRead ( *table_pa, cursor_pa, (size_t)64 << 30 );*/
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


static rc_t add_ref_row_to_cache (
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
    int64_t slice_start = pileup_state->slice_start;

    -- seq_start; /* SEQ_START is one-based coord, and we will need zero-based one */
    /* Making slice_start and slice_end relative to the current reference row_id */

    slice_start -= seq_start;
    ref_pos -= seq_start;

    for (; i < (size_t)pa_count; ++i)
    {
        /* Read current PRIMARY_ALIGNMENT: REF_START and REF_LEN
           if it intersects slice - add this primary allignment to the cache
        */
        int64_t ref_end;

#if USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1
        rc = open_blob_for_current_id ( pa_ids[i],
            cursor_pa, & pileup_state->blob_alignments_ref_start,
            column_index_pa [COL_REF_START],
            error_buf, error_buf_size );
        if (rc != 0)
            return rc;

        rc = VBlobRead ( pileup_state->blob_alignments_ref_start,
            pa_ids[i], sizeof (ref_start) * 8, & ref_start, 1, & row_len );
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VBlobRead(pa, %s) failed with error: 0x%08x (%u) [%R]",
                column_names_pa[COL_REF_START], rc, rc, rc);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return rc;
        }

        rc = open_blob_for_current_id ( pa_ids[i],
            cursor_pa, & pileup_state->blob_alignments_ref_len,
            column_index_pa [COL_REF_LEN],
            error_buf, error_buf_size );
        if (rc != 0)
            return rc;

        rc = VBlobRead ( pileup_state->blob_alignments_ref_len,
            pa_ids[i], sizeof (ref_len) * 8, & ref_len, 1, & row_len );
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VBlobRead(pa, %s) failed with error: 0x%08x (%u) [%R]",
                column_names_pa[COL_REF_LEN], rc, rc, rc);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return rc;
        }
#else
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
        /*printf ("alignment row_id=%lld, align interval: [%ld, %lu], slice: [%lld, %llu]",
            pa_ids[i], ref_start, ref_end, slice_start, slice_end);*/

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
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: Alignment_Add (%lld, %d, %u, %u) failed with error: 0x%08x (%u) [%R], cache_size=%zu, ref_start_id=%lld, name=\"%s\", len=%llu, ref_pos=%llu",
                    pa_ids[i], ref_start, ref_len, seq_start, rc, rc, rc,
                    pileup_state->cache_alignment.size,
                    pileup_state->reference_start_id,
                    pileup_state->ref_name,
                    /*pileup_state->reference_length,*/999,
                    pileup_state->ref_pos);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
            /*printf (" CACHED\n");*/
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
            /*printf (" ignored\n");*/
        }
    }
    pileup_state->next_alignment_idx =
        & pa_ids[i] - pileup_state->alignment_ids; /* TODO: i-th alignment will be read again on the next step, so consider caching it here and ignoring it when looking it up for cached items relevant to current ref_pos */

    /*printf ("Updated cache size=%zu\n", pileup_state->cache_alignment.size);*/
    
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

    rc_t rc = VCursorIdRange ( cursor_ref, 0, & row_id, & row_count );

    /*printf ("REFERENCE table: row_id=%lld, row_count=%llu\n", row_id, row_count);*/


    if ( row_count < 1 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "There is no rows in REFERENCE table");
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return (rc_t)(-1);
    }
    pileup_state->total_row_count = row_count;

    /* We don't know the reference end id use its name to notice the moment when it changes - this will be the end */
    rc = VCursorReadDirect ( cursor_ref, pileup_state->reference_start_id, column_index_ref [COL_NAME],
        sizeof (pileup_state->ref_name[0]) * 8, pileup_state->ref_name, countof(pileup_state->ref_name), & row_len );
    if ( rc != 0 )
    {
        rc_t res = string_printf ( error_buf, error_buf_size, NULL,
            "ERROR: VCursorReadDirect(ref) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
        if (res == rcBuffer || res == rcInsufficient)
            error_buf [ error_buf_size - 1 ] = '\0';

        return rc;
    }
    pileup_state->ref_name[ min ( countof(pileup_state->ref_name) - 1, row_len) ] = '\0';

    /* Read MAX_SEQ_LEN from the start_row_id and assume that it's the same for all the rest */
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

    pileup_state->slice_start_id = pileup_state->reference_start_id + pileup_state->slice_start/max_seq_len;
    pileup_state->slice_end_id = pileup_state->slice_length != 0 ?
        pileup_state->reference_start_id + (pileup_state->slice_start + (int64_t)pileup_state->slice_length)/max_seq_len :
        (int64_t)pileup_state->total_row_count;

    printf ("slice position range: [%lld, %llu]\n", pileup_state->slice_start, pileup_state->slice_start + pileup_state->slice_length);
    /*printf ("slice id range: [%lld, %lld]\n", slice_start_id, slice_end_id);*/

    /* Read reference slice_start_id,
       read OVERLAP_*_POS to find out how
       many rows we need to read ahead of slice_start_id
       TODO: this is not implemented yet, insted we read just 10 rows ahead
    */


    /* Set cursor to <read_ahead_rows> rows ahead of slice_start_id
       and cache corresponding PRIMARY_ALIGNMENTS
    */


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
            */
            char ref_name[ countof (pileup_state->ref_name) ];
            rc = VCursorReadDirect ( cursor_ref, current_id, column_index_ref [COL_NAME],
                sizeof (ref_name[0]) * 8, ref_name, countof(ref_name), & row_len );
            if ( rc != 0 )
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: VCursorReadDirect(ref) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
            ref_name[ min ( countof(ref_name) - 1, row_len) ] = '\0';
            if ( current_id > stop_id || strcmp (ref_name, pileup_state->ref_name) )
                break;

#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
            rc = open_blob_for_current_id ( current_id,
                cursor_ref, & pileup_state->blob_alignment_ids,
                column_index_ref [COL_PRIMARY_ALIGNMENT_IDS],
                error_buf, error_buf_size );
            if (rc != 0)
                return rc;
#endif

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
            pileup_state->current_seq_start = seq_start;

            /* Read REFERENCE row's PRIMARY_ALIGNMENT_IDS column to iterate through them */
            /* elem_bits = sizeof (*pileup_state->alignment_ids) * 8;*/
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
            rc = VBlobCellData ( pileup_state->blob_alignment_ids, current_id,
                & dummy, & pileup_state->alignment_ids, NULL, & row_len );
            if ( rc != 0 )
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: VBlobCellData(ref-pa_ids) failed with error: 0x%08x (%u) [%R], row_len=%u",
                    rc, rc, rc, row_len);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
            pileup_state -> size_alignment_ids = row_len;
#else

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
            /*pileup_state->size_alignment_ids = row_len;*/
            /*printf ("Read %lu PRIMARY_ALIGNMENT_IDS for REFERENCE row_id=%lld:", row_len, current_id);*/
            {
                /*size_t i = 0;

                for (; i < row_len; ++i)
                    printf(" %lld", pa_ids [i]);*/

                /*printf ("\n");*/
            }

            /* For each PRIMARY_ALIGNMENT_ID in alignment_ids: read its start, length and
               cache it if it intersects the starting position
            */
            rc = add_ref_row_to_cache ( pileup_state, cursor_pa, seq_start,
                        pileup_state->slice_start,
                        pileup_state->alignment_ids, row_len,
                        column_names_pa, column_index_pa, column_count_pa,
                        error_buf, error_buf_size );
            if ( rc != 0 )
                return rc;
        }
    }

    return rc;

}

static void print_current_state (PileupIteratorState const* pileup_state)
{
    size_t i, depth = 0;
    /*Alignment_CacheItem const* cache_item;*/
    /*uint64_t local_ref_pos = pileup_state->ref_pos - (pileup_state -> seq_start - 1);*/
    printf ("Current reference position=%llu, alignments at this positions are:\n", pileup_state->ref_pos);
    for (i = 0; i < pileup_state->cache_alignment.size; ++i)
    {
        Alignment_CacheItem const* cache_item = Alignment_Get ( & pileup_state->cache_alignment, i );
        uint64_t local_ref_pos = pileup_state->ref_pos - cache_item->seq_start;

        if ( cache_item->start <= (int64_t)local_ref_pos &&
             cache_item->start + cache_item->len >= local_ref_pos)
        {
            printf ("row_id=%lld\tstart=%lld\tlen=%llu\n",
               cache_item->row_id, cache_item->start, cache_item->len);
            ++ depth;
        }
    }
    printf ("Depth at position %llu is %zu\n", pileup_state->ref_pos, depth);
}


static void remove_unneeded_alignments (PileupIteratorState* pileup_state, uint64_t ref_pos, char* error_buf, size_t error_buf_size)
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
#if USE_BLOB_CACHE_FOR_ALIGNMENTS == 1
            if ( item->row_id > max_removed_id )
                max_removed_id = item->row_id;
#endif

            DLListUnlink ( & pileup_state->cache_alignment.list_alignments, & item_cur->node);
            Alignment_CacheItemWhack ( & item_cur->node, NULL );
            -- pileup_state->cache_alignment.size;
        }
        else
            item = (Alignment_CacheItem*) DLNodeNext( & item->node );
    }
#else
#if 0
    /* Improved (?) inplace version */

    /* i - current item in current state cache (cache size can decrease during this algorithm, so as i)
           all cache[j]: j < i are already processed in the cache - i.e. they present in new cache state
       
       size - updated size of cache
       gap_start - beginning index of first consecutive elements found to be removed from the current cache
                 (so it's an semi-open interval [start, i) that needs to be removed
                 from the cache)
    */
    size_t i = 0;
    Alignment_Cache* cache = & pileup_state->cache_alignment;
    size_t size = cache->size;
    size_t gap_start = 0;
    for (; i < size;)
    {
        Alignment_CacheItem const* item = & cache->data [i];
        uint64_t local_ref_pos = ref_pos - item->seq_start;
        if ( item->start + item->len > local_ref_pos ) /* not ">=" here because local_ref_pos here is the old position that will be incremented right after we exit from this function */
        {
            /* check if we have a gap before i and if yes - move memory up */
            if ( gap_start != i )
            {
                memmove ( & cache->data[gap_start], & cache->data[i], (size-i)*sizeof(Alignment_CacheItem) );
                size -= i - gap_start;
                i = gap_start;
            }

            ++i;
            gap_start = i;
        }
        else
        {
            ++i;
        }
    }
    if ( gap_start != i )
        size = gap_start;

    cache->size = size;
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
#endif

#if USE_BLOB_CACHE_FOR_ALIGNMENTS == 2
    if ( max_removed_id > 0 )
    {
        size_t i_src, i_dst;
        BlobItem* blobs = pileup_state->blobs_alignments.blobs;
        size_t size = pileup_state->blobs_alignments.size;

        for (i_src = 0; i_src < size; ++ i_src)
        {
            BlobItem const* item = & blobs [i_src];
            int64_t start_id;
            uint64_t count;
            rc_t rc = VBlobIdRange ( item->blob, & start_id, & count );
            if ( rc != 0 )
            {
                rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                    "ERROR: VBlobIdRange failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
                if (res == rcBuffer || res == rcInsufficient)
                    error_buf [ error_buf_size - 1 ] = '\0';

                return rc;
            }
            if ( start_id + (int64_t)count > max_removed_id )
            {
                /* Current blob contains ids greater than last removed one
                   we don't want to look further because most probably
                   all the following blobs will also contain ids that we will
                   need to keep in the cache, so we exit immediately
                */
                break;
            }
            else
            {
                VBlobRelease ( item->blob );
                
            }
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
    }
#endif
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
    rc_t rc;

    /* TODO: check the case when slice_end is beyond the reference end*/
    if ( pileup_state->slice_length && pileup_state->ref_pos == pileup_state->slice_start + pileup_state->slice_length )
    {
        error_buf[0] = '\0'; /* indicating that no error has occured */
        return false;
    }

    /* drop cached alignments that we will not need anymore */
    remove_unneeded_alignments ( pileup_state, ref_pos, error_buf, error_buf_size ); /* it's not an issue but this action is not rolled backed in the case of error below */

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

        char ref_name[ countof (pileup_state->ref_name) ];

        /* TODO: consider storing this in pileup_state (don't need to calculate every time)*/
        /*slice_start_id = pileup_state->reference_start_id + pileup_state->slice_start/pileup_state->max_seq_len;
        slice_end_id = pileup_state->slice_length != 0 ?
            pileup_state->reference_start_id + (pileup_state->slice_start + (int64_t)pileup_state->slice_length)/pileup_state->max_seq_len :
            (int64_t)pileup_state->total_row_count;*/
        
        if ( ref_row_id < pileup_state->slice_start_id || ref_row_id > pileup_state->slice_end_id )
        {
            error_buf[0] = '\0'; /* indicating that no error has occured */
            return false;
        }

        rc = VCursorReadDirect ( cursor_ref, ref_row_id, column_index_ref [COL_NAME],
            sizeof (ref_name[0]) * 8, ref_name, countof(ref_name), & row_len );
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VCursorReadDirect(ref) failed with error: 0x%08x (%u) [%R]", rc, rc, rc);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return false;
        }
        ref_name[ min ( countof(ref_name) - 1, row_len) ] = '\0';
        if ( strcmp (ref_name, pileup_state->ref_name) )
        {
            /*Alignment_Init ( & pileup_state->cache_alignment);
            strncpy ( pileup_state->ref_name, ref_name, countof (pileup_state->ref_name) - 1 );
            pileup_state->reference_start_id = ref_row_id;*/

            error_buf[0] = '\0'; /* indicating that no error has occured */
            return false;
        }

#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
        rc = open_blob_for_current_id ( ref_row_id,
            cursor_ref, & pileup_state->blob_alignment_ids,
            column_index_ref [COL_PRIMARY_ALIGNMENT_IDS],
            error_buf, error_buf_size );
        if (rc != 0)
            return false;
#endif

        /* Read new SEQ_START */
        rc = VCursorReadDirect ( cursor_ref, ref_row_id,
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
        pileup_state->current_seq_start = seq_start;

        /* Read REFERENCE row's PRIMARY_ALIGNMENT_IDS column to iterate through them */
        /* elem_bits = sizeof (*pileup_state->alignment_ids) * 8; */
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
        rc = VBlobCellData ( pileup_state->blob_alignment_ids, ref_row_id,
            & dummy, & pileup_state->alignment_ids, NULL, & row_len );
        if ( rc != 0 )
        {
            rc_t res = string_printf ( error_buf, error_buf_size, NULL,
                "ERROR: VBlobCellData(ref-pa_ids) failed with error: 0x%08x (%u) [%R], row_len=%u",
                rc, rc, rc, row_len);
            if (res == rcBuffer || res == rcInsufficient)
                error_buf [ error_buf_size - 1 ] = '\0';

            return false;
        }
        pileup_state -> size_alignment_ids = row_len;
#else
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
        /*pileup_state->size_alignment_ids = row_len;*/

        printf ("Read %lu PRIMARY_ALIGNMENT_IDS for REFERENCE row_id=%lld\n", row_len, ref_row_id);

        /* For each PRIMARY_ALIGNMENT_ID in pa_ids: read its start, length and
           cache it if it intersects the slice
        */
        rc = add_ref_row_to_cache ( pileup_state, cursor_pa, seq_start, ref_pos,
                pileup_state->alignment_ids, row_len,
                column_names_pa, column_index_pa, column_count_pa,
                error_buf, error_buf_size );
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
            rc_t rc = add_ref_row_to_cache ( pileup_state, cursor_pa,
                pileup_state->current_seq_start, ref_pos,
                & pileup_state->alignment_ids[ pileup_state->next_alignment_idx ],
                (uint32_t)count,
                column_names_pa, column_index_pa, column_count_pa,
                error_buf, error_buf_size );
            if ( rc != 0 )
                return false;
        }
    }

    ++ pileup_state->ref_pos;
    return true;
}

void run (char const* db_path)
{
    VDBManager const* mgr = NULL;
    VDatabase const* db = NULL;

    VTable const* table_ref = NULL;
    VTable const* table_pa = NULL; 
    
    VCursor const* cursor_ref = NULL;
    VCursor const* cursor_pa = NULL;

    char error_buf [512] = "";

    rc_t rc = 0;

    char const* column_names_ref[] =
    {
        "MAX_SEQ_LEN",
        "PRIMARY_ALIGNMENT_IDS",
        "SEQ_START",
        "NAME"
    };
    uint32_t column_index_ref [ countof (column_names_ref) ];
    char const* column_names_pa[] =
    {
        "REF_START",
        "REF_LEN"
    };
    uint32_t column_index_pa [ countof (column_names_pa) ];
    PileupIteratorState pileup_state;

    Alignment_Init ( & pileup_state.cache_alignment );
    PileupIteratorState_Init ( & pileup_state );

    pileup_state.ref_name [0] = '\0';
    pileup_state.reference_start_id = 1032;
    /*pileup_state.reference_length = 72482;*/
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
        Alignment_Release ( & pileup_state.cache_alignment );
        PileupIteratorState_Release ( & pileup_state );
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
        Alignment_Release ( & pileup_state.cache_alignment );
        PileupIteratorState_Release ( & pileup_state );
        return;
    }

    {
        printf ("Slice start at position=%lld, alignment cache size=%zu, alignment_ids size=%zu (uncached: %zu)\n",
            pileup_state.slice_start, pileup_state.cache_alignment.size,
            pileup_state.size_alignment_ids, pileup_state.next_alignment_idx);

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
    Alignment_Release ( & pileup_state.cache_alignment );
    PileupIteratorState_Release ( & pileup_state );
}


void test ( char const* db_path );

#if 0
void test_arr ()
{
    BlobCache cache;
    size_t i = 0;
    size_t i2 = 0;
    size_t order = 5;

    /* 0-size */
    printf ("Searching in 0-size array:\n");
    cache.size = 0;
    
    i = BlobCache_UpperBound ( & cache, order );
    i2 = BlobCache_LowerBound ( & cache, order );
    printf (" upper_bound found at pos=%lu\n", i);
    printf (" lower_bound found at pos=%lu\n", i2);

    /* 1-size less*/
    printf ("Searching in 1-size less array:\n");
    cache.size = 1;
    cache.blobs[0].order = order - 2;
    
    i = BlobCache_UpperBound ( & cache, order );
    i2 = BlobCache_LowerBound ( & cache, order );
    if (i == cache.size)
        printf (" upper_bound found at pos=%lu (end)\n", i);
    else
        printf (" upper_bound found at pos=%lu: %lu\n", i, cache.blobs[i].order);

    if (i2 == cache.size)
        printf (" lower_bound found at pos=%lu (end)\n", i2);
    else
        printf (" lower_bound found at pos=%lu: %lu\n", i2, cache.blobs[i2].order);

    /* 1-size equal*/
    printf ("Searching in 1-size equal array:\n");
    cache.size = 1;
    cache.blobs[0].order = order;
    
    i = BlobCache_UpperBound ( & cache, order );
    i2 = BlobCache_LowerBound ( & cache, order );
    if (i == cache.size)
        printf (" upper_bound found at pos=%lu (end)\n", i);
    else
        printf (" upper_bound found at pos=%lu: %lu\n", i, cache.blobs[i].order);

    if (i2 == cache.size)
        printf (" lower_bound found at pos=%lu (end)\n", i2);
    else
        printf (" lower_bound found at pos=%lu: %lu\n", i2, cache.blobs[i2].order);

    /* 1-size greater*/
    printf ("Searching in 1-size greater array:\n");
    cache.size = 1;
    cache.blobs[0].order = order + 2;
    
    i = BlobCache_UpperBound ( & cache, order );
    i2 = BlobCache_LowerBound ( & cache, order );
    if (i == cache.size)
        printf (" upper_bound found at pos=%lu (end)\n", i);
    else
        printf (" upper_bound found at pos=%lu: %lu\n", i, cache.blobs[i].order);

    if (i2 == cache.size)
        printf (" lower_bound found at pos=%lu (end)\n", i2);
    else
        printf (" lower_bound found at pos=%lu: %lu\n", i2, cache.blobs[i2].order);

    /* odd-size */
    printf ("Searching in odd-size array:\n");
    cache.size = 5;
    cache.blobs[0].order = order - 3;
    cache.blobs[1].order = order;
    cache.blobs[2].order = order;
    cache.blobs[3].order = order + 3;
    cache.blobs[4].order = order + 4;
    
    i = BlobCache_UpperBound ( & cache, order );
    i2 = BlobCache_LowerBound ( & cache, order );
    if (i == cache.size)
        printf (" upper_bound found at pos=%lu (end)\n", i);
    else
        printf (" upper_bound found at pos=%lu: %lu\n", i, cache.blobs[i].order);

    if (i2 == cache.size)
        printf (" lower_bound found at pos=%lu (end)\n", i2);
    else
        printf (" lower_bound found at pos=%lu: %lu\n", i2, cache.blobs[i2].order);

    /* even-size */
    printf ("Searching in even-size array:\n");
    cache.size = 6;
    cache.blobs[0].order = order + 1;
    cache.blobs[1].order = order + 1;
    cache.blobs[2].order = order + 1;
    cache.blobs[3].order = order + 1;
    cache.blobs[4].order = order + 1;
    cache.blobs[5].order = order + 1;
    
    i = BlobCache_UpperBound ( & cache, order );
    i2 = BlobCache_LowerBound ( & cache, order );
    if (i == cache.size)
        printf (" upper_bound found at pos=%lu (end)\n", i);
    else
        printf (" upper_bound found at pos=%lu: %lu\n", i, cache.blobs[i].order);

    if (i2 == cache.size)
        printf (" lower_bound found at pos=%lu (end)\n", i2);
    else
        printf (" lower_bound found at pos=%lu: %lu\n", i2, cache.blobs[i2].order);
}

#endif
#endif /* global off */

void NGS_Test ( char const*);

ver_t CC KAppVersion ( void )
{
    return 0;
}

rc_t CC KMain ( int argc, char *argv [] )
{
    //char const* db_path = "SRR1640559";//long
    char const* db_path = "SRR341578";

    if (argc == 2)
        db_path = argv[1];

    printf ("Running PileupIterator for accession=\"%s\"\n", db_path);

    /*run(db_path);*/

    NGS_Test (db_path);

    return 0;
}
