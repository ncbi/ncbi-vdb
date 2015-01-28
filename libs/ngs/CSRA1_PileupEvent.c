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

typedef struct CSRA1_PileupEvent CSRA1_PileupEvent;
#define NGS_PILEUPEVENT CSRA1_PileupEvent
#include "NGS_PileupEvent.h"

#include "CSRA1_PileupEvent.h"
#include "CSRA1_Pileup.h"
#include "NGS_Pileup.h"
#include "NGS_Reference.h"
#include "NGS_ReadCollection.h"
#include "NGS_Id.h"
#include "NGS_String.h"
#include "NGS_Cursor.h"

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>
#include <align/align.h>

#include <klib/printf.h>

#include "NGS_String.h"
#include "NGS_Pileup.h"

#include <sysalloc.h>

struct CSRA1_PileupEvent
{
    NGS_PileupEvent dad;

    /* capture the pileup iterator's position:
       this will be used to validate our iterator */
    int64_t ref_zpos;

    /* current alignment being examined */
    CSRA1_Pileup_Entry * entry;

    /* set to true within "next" */
    bool seen_first;
};

#define CSRA1_PileupEventGetPileup( self ) \
    ( ( CSRA1_Pileup * ) ( self ) -> dad . pileup )

static
void CSRA1_PileupEventStateTest ( const CSRA1_PileupEvent * self, ctx_t ctx, uint32_t lineno )
{
    assert ( self != NULL );

    if ( self -> entry != NULL )
    {
        CSRA1_Pileup * pileup = CSRA1_PileupEventGetPileup ( self );
        assert ( pileup != NULL );

        if ( self -> ref_zpos != pileup -> ref_zpos )
        {
            ctx_event ( ctx, lineno, xc_sev_fail, xc_org_user, xcIteratorUninitialized,
                        "PileupEvent accessed after advancing PileupIterator" );
        }
    }

    else if ( ! self -> seen_first )
    {
        ctx_event ( ctx, lineno, xc_sev_fail, xc_org_user, xcIteratorUninitialized,
                    "PileupEvent accessed before a call to PileupEventIteratorNext()" );
    }
    else
    {
        ctx_event ( ctx, lineno, xc_sev_fail, xc_org_user, xcCursorExhausted, "No more rows available" );
    }
}

#define CHECK_STATE( self, ctx ) \
    CSRA1_PileupEventStateTest ( self, ctx, __LINE__ )


static
void CSRA1_PileupEventWhack ( CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcDestroying );
    NGS_PileupEventWhack ( & self -> dad, ctx );
}

static
const void * CSRA1_PileupEventGetEntry ( const CSRA1_PileupEvent * self, ctx_t ctx,
    CSRA1_Pileup_Entry * entry, uint32_t col_idx )
{
    if ( entry -> cell_data [ col_idx ] != NULL )
    {
        FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
        return CSRA1_PileupGetEntry ( CSRA1_PileupEventGetPileup ( self ), ctx, entry, col_idx );
    }

    return entry -> cell_data [ col_idx ];
}

static
const void * CSRA1_PileupEventGetNonEmptyEntry ( const CSRA1_PileupEvent * self, ctx_t ctx,
    CSRA1_Pileup_Entry * entry, uint32_t col_idx )
{
    if ( entry -> cell_len [ col_idx ] == 0 )
    {
        FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );

        if ( entry -> cell_data [ col_idx ] == NULL )
            CSRA1_PileupGetEntry ( CSRA1_PileupEventGetPileup ( self ), ctx, entry, col_idx );
        
        if ( entry -> cell_len [ col_idx ] == 0 )
        {
            INTERNAL_ERROR ( xcStorageExhausted, "zero-length cell data" );
            return NULL;
        }
    }
    return entry -> cell_data [ col_idx ];
}

static
struct NGS_String * CSRA1_PileupEventGetReferenceSpec ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        return NGS_PileupGetReferenceSpec ( self -> dad . pileup, ctx );
    }
    return NULL;
}

static
int64_t CSRA1_PileupEventGetReferencePosition ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        return self -> ref_zpos;
    }
    return 0;
}

static
int CSRA1_PileupEventGetMappingQuality ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        const int32_t * MAPQ;
        TRY ( MAPQ = CSRA1_PileupEventGetNonEmptyEntry ( self, ctx, self -> entry, pileup_event_col_MAPQ ) )
        {
            return MAPQ [ 0 ];
        }
    }
    return 0;
}

static
struct NGS_String * CSRA1_PileupEventGetAlignmentId ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        NGS_ReadCollection * coll = self -> dad . pileup -> ref -> coll;
        TRY ( const NGS_String * run = NGS_ReadCollectionGetName ( coll, ctx ) )
        {
            enum NGS_Object obj_type = self -> entry -> secondary ?
                NGSObject_SecondaryAlignment : NGSObject_PrimaryAlignment;
            return NGS_IdMake ( ctx, run, obj_type, self -> entry -> row_id );
        }
    }
    return NULL;
}

static
struct NGS_Alignment * CSRA1_PileupEventGetAlignment ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );

    /* TBD - the REASON this method exists is that we already have
       most of what's needed to create a single Alignment record.
       The intention is to create a mostly pre-populated record
       here, since the user can already grab the id and ask the
       ReadCollection for one. */

    TRY ( NGS_String * id = CSRA1_PileupEventGetAlignmentId ( self, ctx ) )
    {
        /* unfortunately, as of today, our C API takes a NUL-terminated
           string as the alignment id, but we only have an NGS_String. */
        char * idz = NGS_StringMakeNULTerminatedString ( id, ctx );
        NGS_StringRelease ( id, ctx );

        if ( ! FAILED () )
        {
            NGS_ReadCollection * coll = self -> dad . pileup -> ref -> coll;
            struct NGS_Alignment * alignment = NGS_ReadCollectionGetAlignment ( coll, ctx, idz );

            free ( idz );

            if ( ! FAILED () )
                return alignment;
        }
    }

    return NULL;
}

static
int64_t CSRA1_PileupEventGetAlignmentPosition ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        return self -> entry -> seq_idx;
    }

    return 0;
}

static
int64_t CSRA1_PileupEventGetFirstAlignmentPosition ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        return self -> entry -> zstart;
    }

    return 0;
}

static
int64_t CSRA1_PileupEventGetLastAlignmentPosition ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        return self -> entry -> xend - 1;
    }
    return 0;
}

static
int CSRA1_PileupEventGetEventType ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );

    int event_type = 0;

    TRY ( CHECK_STATE ( self, ctx ) )
    {
        const bool * REF_ORIENTATION;

        CSRA1_Pileup_Entry * entry = self -> entry;

        /*
          during "next" we took these steps:
          1. if within a deletion, decrement deletion repeat && exit if ! 0
          2. check HAS_REF_OFFSET. if not false:
             a. a positive REF_OFFSET[ref_offset_idx] indicates a deletion
             b. a negative REF_OFFSET[ref_offset_idx] indicates an insertion
          3. move current offset ahead until ref_pos >= that of pileup
          
          so here, we first detect a deletion event
          next, we detect a match or mismatch by checking HAS_MISMATCH.
          if there was a prior insertion, we or that onto the event.
          if this event starts a new alignment, or start onto event.
          if it ends an alignment, or that onto the event.
        */

        if ( entry -> del_cnt != 0 )
            event_type = NGS_PileupEventType_deletion;
        else
        {
            const bool * HAS_MISMATCH = entry -> cell_data [ pileup_event_col_HAS_MISMATCH ];
            assert ( HAS_MISMATCH != NULL );
            assert ( entry -> seq_idx < entry -> cell_len [ pileup_event_col_HAS_MISMATCH ] );
            event_type = HAS_MISMATCH [ entry -> seq_idx ];
        }

        /* detect prior insertion */
        if ( entry -> ins_cnt != 0 )
            event_type |= NGS_PileupEventType_insertion;

        /* detect initial event */
        if ( self -> ref_zpos == entry -> zstart )
            event_type |= NGS_PileupEventType_start;

        /* detect final event */
        if ( self -> ref_zpos + 1 == entry -> xend )
            event_type |= NGS_PileupEventType_stop;

        /* detect minus strand */
        TRY ( REF_ORIENTATION = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_REF_ORIENTATION ) )
        {
            assert ( REF_ORIENTATION != NULL );
            assert ( entry -> cell_len [ pileup_event_col_REF_ORIENTATION ] == 1 );
            if ( REF_ORIENTATION [ 0 ] )
                event_type |= NGS_PileupEventType_minus_strand;
        }

    }
    
    return event_type;
}

static
char CSRA1_PileupEventGetAlignmentBase ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        CSRA1_Pileup * pileup;
        CSRA1_Pileup_Entry * entry = self -> entry;
        const bool * HAS_MISMATCH = entry -> cell_data [ pileup_event_col_HAS_MISMATCH ];

        if ( entry -> del_cnt != 0 )
            return '-';

        assert ( HAS_MISMATCH != NULL );
        assert ( entry -> seq_idx < entry -> cell_len [ pileup_event_col_HAS_MISMATCH ] );

        if ( HAS_MISMATCH [ entry -> seq_idx ] )
        {
            if ( entry -> mismatch == 0 )
            {
                const INSDC_dna_text * MISMATCH;
                TRY ( MISMATCH = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_MISMATCH ) )
                {
                    if ( entry -> mismatch_idx < entry -> cell_len [ pileup_event_col_MISMATCH ] )
                        entry -> mismatch = MISMATCH [ entry -> mismatch_idx ];
                }
            }

            return entry -> mismatch;
        }

        pileup = CSRA1_PileupEventGetPileup ( self );
        if ( pileup -> ref_base == 0 )
        {
            if ( pileup -> ref_chunk_bases == NULL )
            {
                const void * base;
                uint32_t elem_bits, boff, row_len;
                ON_FAIL ( NGS_CursorCellDataDirect ( pileup -> ref . curs, ctx, pileup -> ref_chunk_id,
                    reference_READ, & elem_bits, & base, & boff, & row_len ) )
                {
                    return 0;
                }

                pileup -> ref_chunk_bases = base;
                assert ( row_len == pileup -> ref . max_seq_len ||
                         pileup -> ref_chunk_xend - pileup -> ref . max_seq_len + row_len >= pileup -> slice_xend );
            }

            assert ( pileup -> ref . max_seq_len != 0 );
            pileup -> ref_base = pileup -> ref_chunk_bases [ self -> ref_zpos % pileup -> ref . max_seq_len ]; 
        }

        return pileup -> ref_base;
    }
    return 0;
}

static
char CSRA1_PileupEventGetAlignmentQuality ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        const INSDC_quality_phred * QUALITY;

        CSRA1_Pileup_Entry * entry = self -> entry;

        if ( entry -> del_cnt != 0 )
            return '!';
        
        TRY ( QUALITY = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_QUALITY ) )
        {
            assert ( QUALITY != NULL );
            assert ( entry -> seq_idx < entry -> cell_len [ pileup_event_col_QUALITY ] );
            return ( char ) ( QUALITY [ entry -> seq_idx ] + 33 );
        }
    }
    return 0;
}

static
struct NGS_String * CSRA1_PileupEventGetInsertionBases ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        CSRA1_Pileup_Entry * entry = self -> entry;

        /* handle the case where there is no insertion */
        if ( entry -> ins_cnt == 0 )
        {
            return NGS_StringMake ( ctx, "", 0 );
        }
        else
        {
            /* allocate a buffer for the NGS_String */
            char * buffer = calloc ( 1, entry -> ins_cnt + 1 );
            if ( buffer == NULL )
                SYSTEM_ERROR ( xcNoMemory, "allocating %zu bytes", entry -> ins_cnt + 1 );
            else
            {
                const INSDC_dna_text * MISMATCH;
                const bool * HAS_MISMATCH = entry -> cell_data [ pileup_event_col_HAS_MISMATCH ];
                assert ( HAS_MISMATCH != NULL );

                /* it is "possible" but not likely that we may not need the MISMATCH cell.
                   this would be the case if there was an insertion that exactly repeated
                   a region of the reference, such that there were no mismatches in it.
                   but even so, it should not be a problem to prefetch MISMATCH */
                TRY ( MISMATCH = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_MISMATCH ) )
                {
                    uint32_t ins_start = entry -> seq_idx - entry -> ins_cnt;
                    uint32_t seq_idx, mismatch_idx = entry -> mismatch_idx;

                    assert ( MISMATCH != 0 );

                    /* fill in the buffer with each entry from mismatch */
                    for ( seq_idx = entry -> seq_idx - 1; seq_idx >= ins_start; -- seq_idx )
                    {
                        if ( HAS_MISMATCH [ seq_idx ] )
                            buffer [ seq_idx - ins_start ] = MISMATCH [ -- mismatch_idx ];
                    }

                    /* if there are some to be filled from reference */
                    if ( entry -> mismatch_idx - mismatch_idx != entry -> ins_cnt )
                    {
                        CSRA1_Pileup * pileup = CSRA1_PileupEventGetPileup ( self );

                        /* a little more complex than we'd like here...
                           chances are quite good that the matched portion of the reference
                           is in our current chunk, but it's not guaranteed,
                           nor is it guaranteed to be in a single chunk. */

                        uint32_t str_len = entry -> ins_cnt;
                        const INSDC_dna_text * READ = pileup -> ref_chunk_bases;
                        int64_t ins_ref_zstart = self -> ref_zpos - ( int64_t ) str_len;
                        int64_t ins_ref_last = self -> ref_zpos - 1;
                        int64_t ins_ref_start_id = ins_ref_zstart / pileup -> ref . max_seq_len + pileup -> reference_start_id;
                        int64_t ins_ref_last_id = ins_ref_last / pileup -> ref . max_seq_len + pileup -> reference_start_id;

                        /* try to take advantage of the chunk that's loaded right now */
                        if ( READ != NULL && pileup -> ref_chunk_id == ins_ref_last_id )
                        {
                            /* most common case - everything within this chunk */
                            if ( ins_ref_start_id == ins_ref_last_id )
                            {
                                uint32_t ref_off = ( uint32_t ) ( ins_ref_zstart % pileup -> ref . max_seq_len );
                                for ( seq_idx = 0; seq_idx < str_len; ++ seq_idx )
                                {
                                    if ( buffer [ seq_idx ] == 0 )
                                        buffer [ seq_idx ] = READ [ ref_off + seq_idx ];
                                }
                                goto buffer_complete;
                            }
                            /* less common case - share only part of this chunk */
                            else
                            {
                                uint32_t seq_off = str_len - ( uint32_t ) ( self -> ref_zpos % pileup -> ref . max_seq_len );
                                for ( seq_idx = seq_off; seq_idx < str_len; ++ seq_idx )
                                {
                                    if ( buffer [ seq_idx ] == 0 )
                                        buffer [ seq_idx ] = READ [ seq_idx - seq_off ];
                                }

                                str_len = seq_off;
                                -- ins_ref_last_id;
                            }
                        }

                        pileup -> ref_chunk_bases = NULL;
                        pileup -> ref_base = 0;

                        /* walk from ins_ref_start_id to ins_ref_last_id */
                        for ( seq_idx = 0; ins_ref_start_id <= ins_ref_last_id; ++ ins_ref_start_id )
                        {
                            const void * base;
                            uint32_t limit, seq_off, row_len;
                            ON_FAIL ( NGS_CursorCellDataDirect ( pileup -> ref . curs, ctx, ins_ref_start_id,
                                reference_READ, & limit, & base, & seq_off, & row_len ) )
                            {
                                READ = NULL;
                                break;
                            }

                            READ = base;

                            limit = str_len - seq_idx;
                            if ( limit > row_len )
                                limit = row_len;

                            limit += seq_idx;

                            for ( seq_off = seq_idx; seq_idx < limit; ++ seq_idx )
                            {
                                if ( buffer [ seq_idx ] == 0 )
                                    buffer [ seq_idx ] = READ [ seq_idx - seq_off ];
                            }

                            /* we stopped either due to:
                               1. end of string, or
                               2. end of chunk - in which case
                                  a. must not be an end chunk, i.e. has row_len == MAX_SEQ_LEN, and
                                  b. we must loop again
                            */
                            assert ( seq_idx == str_len || ( row_len == pileup -> ref . max_seq_len && ins_ref_start_id < ins_ref_last_id ) );
                        }

                        /* finally, if at this point we have cached the READ for reference
                           and within our current chunk, save it on the pileup */
                        if ( ins_ref_last_id == pileup -> ref_chunk_id )
                            pileup -> ref_chunk_bases = READ;
                    }

                    if ( ! FAILED () )
                    {
                        NGS_String * bases;

                    buffer_complete:

                        TRY ( bases = NGS_StringMakeOwned ( ctx, buffer, entry -> ins_cnt ) )
                        {
                            return bases;
                        }
                    }
                }

                free ( buffer );
            }
        }
    }

    return NULL;
}

static
struct NGS_String * CSRA1_PileupEventGetInsertionQualities ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        CSRA1_Pileup_Entry * entry = self -> entry;

        /* handle the case where there is no insertion */
        if ( entry -> ins_cnt == 0 )
        {
            return NGS_StringMake ( ctx, "", 0 );
        }
        else
        {
            /* allocate a buffer for the NGS_String */
            char * buffer = calloc ( 1, entry -> ins_cnt + 1 );
            if ( buffer == NULL )
                SYSTEM_ERROR ( xcNoMemory, "allocating %zu bytes", entry -> ins_cnt + 1 );
            else
            {
                const INSDC_quality_phred * QUALITY;
                TRY ( QUALITY = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_QUALITY ) )
                {
                    NGS_String * bases;
                    uint32_t i, qstart = entry -> seq_idx - entry -> ins_cnt;

                    assert ( QUALITY != NULL );
                    assert ( entry -> seq_idx <= entry -> cell_len [ pileup_event_col_QUALITY ] );
                    assert ( entry -> seq_idx >= entry -> ins_cnt );

                    for ( i = 0; i < entry -> ins_cnt; ++ i )
                        buffer [ i ] = ( char ) ( QUALITY [ qstart + i ] + 33 );

                    TRY ( bases = NGS_StringMakeOwned ( ctx, buffer, entry -> ins_cnt ) )
                    {
                        return bases;
                    }
                }

                free ( buffer );
            }
        }
    }

    return NULL;
}

static
unsigned int CSRA1_PileupEventGetRepeatCount ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        bool event_type;
        uint32_t repeat, limit;
        const bool * HAS_MISMATCH, * HAS_REF_OFFSET;
        const CSRA1_Pileup_Entry * entry = self -> entry;

        /* handle the easy part first */
        if ( entry -> del_cnt != 0 )
            return entry -> del_cnt;

        /* now, count the number of repeated matches or mismatches,
           WITHOUT any intervening insertions or deletions */
        HAS_MISMATCH = entry -> cell_data [ pileup_event_col_HAS_MISMATCH ];
        HAS_REF_OFFSET = entry -> cell_data [ pileup_event_col_HAS_REF_OFFSET ];
        limit = entry -> xend - ( entry -> zstart + entry -> zstart_adj );

        /* grab the type of event we have now */
        event_type = HAS_MISMATCH [ entry -> seq_idx ];
        
        for ( repeat = 1; repeat < limit; ++ repeat )
        {
            if ( HAS_REF_OFFSET [ entry -> seq_idx + repeat ] )
                break;
            if ( HAS_MISMATCH [ entry -> seq_idx + repeat ] != event_type )
                break;
        }

        return repeat;
    }
    return 0;
}

static
int CSRA1_PileupEventGetIndelType ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        CSRA1_Pileup_Entry * entry = self -> entry;

        const bool * HAS_REF_OFFSET = entry -> cell_data [ pileup_event_col_HAS_REF_OFFSET ];
        if ( HAS_REF_OFFSET [ entry -> seq_idx ] )
        {
            CSRA1_Pileup * pileup = CSRA1_PileupEventGetPileup ( self );
            CSRA1_Pileup_AlignCursorData * cd = entry -> secondary ? & pileup -> sa : & pileup -> pa;
            if ( ! cd -> missing_REF_OFFSET_TYPE )
            {
                const NCBI_align_ro_type * REF_OFFSET_TYPE;
                TRY ( REF_OFFSET_TYPE = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_REF_OFFSET_TYPE ) )
                {
                    assert ( REF_OFFSET_TYPE != NULL );
                    assert ( entry -> ref_off_idx < entry -> cell_len [ pileup_event_col_REF_OFFSET_TYPE ] );
                    switch ( REF_OFFSET_TYPE [ entry -> ref_off_idx ] )
                    {
                    case NCBI_align_ro_normal:
                    case NCBI_align_ro_soft_clip:
                        break;
                    case NCBI_align_ro_intron_plus:
                        return NGS_PileupIndelType_intron_plus;
                    case NCBI_align_ro_intron_minus:
                        return NGS_PileupIndelType_intron_minus;
                    case NCBI_align_ro_intron_unknown:
                        return NGS_PileupIndelType_intron_unknown;
                    case NCBI_align_ro_complete_genomics:
                        if ( entry -> ins_cnt != 0 )
                            return NGS_PileupIndelType_read_overlap;
                        assert ( entry -> del_cnt != 0 );
                        return NGS_PileupIndelType_read_gap;
                    }
                }
                CATCH_ALL ()
                {
                    CLEAR ();
                    cd -> missing_REF_OFFSET_TYPE = true;
                }
            }
        }

        return NGS_PileupIndelType_normal;
    }
    return 0;
}

static
bool CSRA1_PileupEventEntryAdvance ( CSRA1_PileupEvent * self, CSRA1_Pileup_Entry * entry )
{
    /* within a deletion */
    if ( entry -> del_cnt != 0 )
    {
        -- entry -> del_cnt;
        entry -> zstart_adj += entry -> seen_first;
    }
    else
    {
        const bool * HAS_MISMATCH = entry -> cell_data [ pileup_event_col_HAS_MISMATCH ];
        const bool * HAS_REF_OFFSET = entry -> cell_data [ pileup_event_col_HAS_REF_OFFSET ];

        /* recover from previous position */
        if ( entry -> seen_first )
        {
            uint32_t prior_seq_idx = entry -> seq_idx ++;

            /* adjust mismatch_idx */
            assert ( HAS_MISMATCH != NULL );
            assert ( prior_seq_idx < entry -> cell_len [ pileup_event_col_HAS_MISMATCH ] );
            entry -> mismatch_idx += HAS_MISMATCH [ prior_seq_idx ];

            /* for the case where there are indels */
            assert ( HAS_REF_OFFSET != NULL );
            assert ( prior_seq_idx < entry -> cell_len [ pileup_event_col_HAS_REF_OFFSET ] );
            entry -> ref_off_idx += HAS_REF_OFFSET [ prior_seq_idx ];

            /* move to the next reference position */
            ++ entry -> zstart_adj;
        }

        /* if the current sequence address is beyond end, bail */
        if ( entry -> seq_idx >= entry -> cell_len [ pileup_event_col_HAS_REF_OFFSET ] )
            return false;

        /* adjust alignment */
        if ( HAS_REF_OFFSET [ entry -> seq_idx ] )
        {
            int32_t indel_cnt;

            const int32_t * REF_OFFSET = entry -> cell_data [ pileup_event_col_REF_OFFSET ];
            assert ( REF_OFFSET != NULL );

            indel_cnt = REF_OFFSET [ entry -> ref_off_idx ];
            if ( indel_cnt < 0 )
            {
                entry -> ins_cnt += - indel_cnt;

                /* insertion - clip to SEQUENCE length */
                if ( ( uint32_t ) entry -> ins_cnt > entry -> cell_len [ pileup_event_col_HAS_REF_OFFSET ] )
                    entry -> ins_cnt = ( int32_t ) entry -> cell_len [ pileup_event_col_HAS_REF_OFFSET ];
            }

            else
            {
                entry -> del_cnt = indel_cnt;

                /* deletion - clip to PROJECTION length */
                if ( ( int64_t ) indel_cnt > entry -> xend - ( entry -> zstart + entry -> zstart_adj ) )
                    entry -> del_cnt = ( int32_t ) ( entry -> xend - ( entry -> zstart + entry -> zstart_adj ) );
            }

            /* adjust alignment */
            entry -> zstart_adj += ( int32_t ) entry -> del_cnt - ( int32_t ) entry -> ins_cnt;
        }
    }

    entry -> seen_first = true;
    return true;
}

static
bool CSRA1_PileupEventEntryFocus ( CSRA1_PileupEvent * self, CSRA1_Pileup_Entry * entry )
{
    /* we need the entry to be fast-forwarded */
    int32_t ref_zpos_adj = self -> ref_zpos - entry -> zstart;

    /* always lose any insertion, forget cached values */
    entry -> ins_cnt = 0;
    entry -> mismatch = 0;

    /* walk forward */
    while ( ref_zpos_adj > entry -> zstart_adj )
    {
        /* go forward one step */
        if ( ! CSRA1_PileupEventEntryAdvance ( self, entry ) )
            return false;
    }

    return true;
}

static
void CSRA1_PileupEventEntryInit ( CSRA1_PileupEvent * self, ctx_t ctx, CSRA1_Pileup_Entry * entry )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );

    const bool * HAS_MISMATCH;

    TRY ( HAS_MISMATCH = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_HAS_MISMATCH ) )
    {
        const int32_t * REF_OFFSET;

        TRY ( REF_OFFSET = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_REF_OFFSET ) )
        {
            const bool * HAS_REF_OFFSET;

            /* if there are no offsets, then there are no indels, which means
               that there are only match and mismatch events */
            if ( entry -> cell_len [ pileup_event_col_REF_OFFSET ] == 0 )
                return;

            /* get HAS_REF_OFFSET */
            TRY ( HAS_REF_OFFSET = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_HAS_REF_OFFSET ) )
            {
                /* check for left soft-clip */
                while ( HAS_REF_OFFSET [ entry -> seq_idx ] && REF_OFFSET [ entry -> ref_off_idx ] < 0 )
                {
                    uint32_t i, count = - REF_OFFSET [ entry -> ref_off_idx ++ ];

                    /* safety check */
                    if ( ( int64_t ) count > entry -> xend - entry -> zstart )
                        count = ( uint32_t ) ( entry -> xend - entry -> zstart );

                    /* skip over soft-clip */
                    for ( i = 0; i < count; ++ i )
                        entry -> mismatch_idx += HAS_MISMATCH [ i ];

                    entry -> seq_idx = count;
                }

                /* detect a deletion */
                if ( HAS_REF_OFFSET [ entry -> seq_idx ] && REF_OFFSET [ entry -> ref_off_idx ] > 0 )
                {
                    entry -> del_cnt = REF_OFFSET [ entry -> ref_off_idx ];

                    /* safety check */
                    if ( ( int64_t ) entry -> del_cnt > entry -> xend - entry -> zstart )
                        entry -> del_cnt = ( uint32_t ) ( entry -> xend - entry -> zstart );
                }

                return;
            }
        }
    }

    self -> entry = NULL;
}

static
bool CSRA1_PileupEventIteratorNext ( CSRA1_PileupEvent * self, ctx_t ctx )
{
    CSRA1_Pileup_Entry * entry;
    CSRA1_Pileup * pileup = CSRA1_PileupEventGetPileup ( self );
    assert ( pileup != NULL );

    /* integrity check */
    if ( self -> ref_zpos != pileup -> ref_zpos )
    {
        FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
        USER_ERROR ( xcIteratorUninitialized, "PileupEvent accessed after advancing PileupIterator" );
        return false;
    }

    /* go to next entry */
    if ( ! self -> seen_first )
        self -> seen_first = true;
    else if ( self -> entry != NULL )
        self -> entry = ( CSRA1_Pileup_Entry * ) DLNodeNext ( & self -> entry -> node );

    /* detect end of pileup */
    entry = self -> entry;
    if ( self -> entry == NULL )
        return false;

    /* detect new entry */
    if ( entry -> cell_data [ pileup_event_col_REF_OFFSET ] == NULL )
    {
        ON_FAIL ( CSRA1_PileupEventEntryInit ( self, ctx, entry ) )
            return false;
        assert ( self -> entry != NULL );
    }

    /* this is an entry we've seen before */
    return CSRA1_PileupEventEntryFocus ( self, entry );
}

static NGS_PileupEvent_vt CSRA1_PileupEvent_vt_inst =
{
    {
        /* NGS_Refcount */
        CSRA1_PileupEventWhack
    },

    CSRA1_PileupEventGetReferenceSpec,
    CSRA1_PileupEventGetReferencePosition,
    CSRA1_PileupEventGetMappingQuality,
    CSRA1_PileupEventGetAlignmentId,
    CSRA1_PileupEventGetAlignment,
    CSRA1_PileupEventGetAlignmentPosition,
    CSRA1_PileupEventGetFirstAlignmentPosition,
    CSRA1_PileupEventGetLastAlignmentPosition,
    CSRA1_PileupEventGetEventType,
    CSRA1_PileupEventGetAlignmentBase,
    CSRA1_PileupEventGetAlignmentQuality,
    CSRA1_PileupEventGetInsertionBases,
    CSRA1_PileupEventGetInsertionQualities,
    CSRA1_PileupEventGetRepeatCount,
    CSRA1_PileupEventGetIndelType,
    
    CSRA1_PileupEventIteratorNext,
};

static
void CSRA1_PileupEventInit ( CSRA1_PileupEvent * self, ctx_t ctx, 
    const char * clsname, const char * instname, CSRA1_Pileup * pileup )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    assert ( self != NULL );
    
    TRY ( NGS_PileupEventInit ( ctx, & self -> dad, & CSRA1_PileupEvent_vt_inst, clsname, instname, & pileup -> dad ) )
    {
        self -> ref_zpos = pileup -> ref_zpos;
        self -> entry = ( CSRA1_Pileup_Entry * ) DLListHead ( & pileup -> align . pileup );
    }
}

NGS_PileupEvent * CSRA1_PileupEventIteratorMake ( ctx_t ctx, CSRA1_Pileup * pileup )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    CSRA1_PileupEvent * obj = calloc ( 1, sizeof * obj );
    if ( obj == NULL )
    {
        NGS_String* ref_spec = NGS_PileupGetReferenceSpec ( & pileup -> dad, ctx );
        SYSTEM_ERROR ( xcNoMemory, 
                       "allocating CSRA1_PileupEventIterator on '%.*s'", 
                       NGS_StringSize ( ref_spec, ctx ), 
                       NGS_StringData ( ref_spec, ctx ) );
        NGS_StringRelease ( ref_spec, ctx );
    }
    else
    {
#if _DEBUGGING
        NGS_String * ref_spec = NGS_PileupGetReferenceSpec ( & pileup -> dad, ctx );
        char instname [ 256 ];
        string_printf ( instname, 
                        sizeof instname, 
                        NULL, 
                        "%.*s", 
                        NGS_StringSize ( ref_spec, ctx ), 
                        NGS_StringData ( ref_spec, ctx ) );
        instname [ sizeof instname - 1 ] = 0;
        NGS_StringRelease ( ref_spec, ctx );
#else
        const char *instname = "";
#endif
        TRY ( CSRA1_PileupEventInit ( obj, ctx, "CSRA1_PileupEvent", instname, pileup ) )
        {
            return & obj -> dad;
        }

        CSRA1_PileupEventWhack ( obj, ctx );
        free ( obj );
    }

    return NULL;
}

