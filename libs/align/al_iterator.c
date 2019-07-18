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
#include <align/extern.h>

#include <klib/rc.h>
#include <klib/container.h>
#include <klib/refcount.h>
#include <klib/sort.h>
#include <klib/log.h>
#include <klib/out.h>
#include <insdc/insdc.h>
#include <align/iterator.h>
#include <align/manager.h>
#include <vdb/cursor.h>
#include <sysalloc.h>

#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define COL_READ "READ"
#define COL_HAS_MISMATCH "HAS_MISMATCH"
#define COL_HAS_REF_OFFSET "HAS_REF_OFFSET"
#define COL_REF_OFFSET "REF_OFFSET"

typedef struct pos_offset
{
    uint32_t pos;
    int32_t offset;
} pos_offset;


struct AlignmentIterator
{
    struct AlignMgr const *amgr;   /* the alignment-manager... */

    /* data to walk */
    bool * has_mismatch;    /* copy of HAS_MISMATCH */
    INSDC_4na_bin * read;   /* copy of READ */
    uint32_t read_len;      /* length of READ */
    uint32_t ref_len;       /* length of projection on reference */

    INSDC_coord_zero ref_window_start;  /* where the requested window on the reference starts */
    INSDC_coord_len  ref_window_len;    /* how long the requested window on the reference is */

    pos_offset * pos_ofs;   /* computed by walking HAS_REF_OFFSET, REF_OFFSET ( method 1 ) */
    uint32_t pos_ofs_idx;   /* current index into pos_ofs[] */
    uint32_t pos_ofs_cnt;   /* how many pos_ofs-values are there */

    int32_t abs_ref_start;  /* absolute reference-related start-position   */
    int32_t rel_ref_pos;    /* relative reference-related position ( relative to start of sequence )  */
    uint32_t seq_pos;       /* position on the sequence, as the reference sees it */
    uint32_t skip;          /* how many bases to skip if we are in DELETE */
    uint32_t flags;         /* flags it, when we are in INSERT/DELETE */
    uint32_t inserts;       /* how many inserts */

    KRefcount refcount;
    bool free_on_whack;     /* true if allocated and allowing references */
    uint8_t align [ 3 ];
};


/* forward decl. for private helper functions */
static rc_t compute_posofs(  AlignmentIterator * self,
    const bool * has_ref_offset,
    const int32_t * ref_offset,
    uint32_t ref_offset_len );

static void al_iter_adjust_next( AlignmentIterator * self );

LIB_EXPORT void CC AlignIteratorRecordDestroy ( void *obj, void *data )
{
    /* nothing to do, because there are no sub-allocations etc. ... */
}


static rc_t get_idx_and_read( struct VCursor const *curs,
                              const char * name,
                              int64_t row_id,
                              const void ** base,
                              uint32_t * len )
{
    uint32_t column_idx;
    rc_t rc = VCursorGetColumnIdx ( curs, &column_idx, "%s", name );
    if ( rc != 0 )
        LOGERR( klogInt, rc, "VCursorGetColumnIdx() failed" );
    else
    {
        uint32_t elem_bits, boff, len_intern;
        const void * ptr;
        rc = VCursorCellDataDirect ( curs, row_id, column_idx, 
                                     &elem_bits, &ptr, &boff, &len_intern );
        if ( rc != 0 )
            LOGERR( klogInt, rc, "VCursorCellDataDirect() failed" );
        else
        {
            assert( boff == 0 );
            if ( len != NULL ) *len = len_intern;
            if ( base != NULL ) *base = ptr;
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC AlignIteratorRecordPopulate ( void *obj,
    const PlacementRecord *placement, struct VCursor const *curs, 
    INSDC_coord_zero ref_window_start, INSDC_coord_len ref_window_len, void *data )
{
    /* read the data required to build a Alignment-Iterator,
       then create the Alignment-Iterator into the already allocated memory */
    const void * base;
    const void * base_has_ref_offset;
    const void * base_ref_offset;

    INSDC_coord_len data_len;
    uint32_t ref_offset_len;
    rc_t rc;

    AlignmentIterator *iter = ( AlignmentIterator * ) obj;
    AlignMgr *almgr = ( AlignMgr * ) data;

    uint8_t * ptr = ( void* ) iter;
    ptr += ( sizeof *iter );

    rc = get_idx_and_read( curs, COL_HAS_MISMATCH, placement->id, &base, &data_len );
    if ( rc == 0 )
    {
        /* copy HAS_MISMATCH into place, point the header-value to it, advance */
        memmove( ptr, base, data_len );
        iter->has_mismatch = (bool *)ptr;
        ptr += data_len;
    }

    if ( rc == 0 )
    {
        rc = get_idx_and_read( curs, COL_READ, placement->id, &base, &data_len );
        /* copy READ into place, point the header-value to it, advance */
        if ( rc == 0 )
        {
            memmove( ptr, base, data_len );
            iter->read = ( INSDC_4na_bin * )ptr;
            iter->read_len = data_len;
            iter->abs_ref_start = placement->pos;
            iter->ref_len = placement->len;
            iter->rel_ref_pos = 0;
            iter->seq_pos = 0;
            ptr += data_len;
            iter->pos_ofs = (pos_offset *)ptr;
            iter->ref_window_start = ref_window_start;
            iter->ref_window_len = ref_window_len;
            iter->free_on_whack = false;
        }
    }

    if ( rc == 0 )
    {
        rc = get_idx_and_read( curs, COL_HAS_REF_OFFSET, placement->id, &base_has_ref_offset, &data_len );
    }
    if ( rc == 0 )
    {
        rc = get_idx_and_read( curs, COL_REF_OFFSET, placement->id, &base_ref_offset, &ref_offset_len );
    }
    if ( rc == 0 )
    {
        rc = compute_posofs( iter,
                             ( const bool * )base_has_ref_offset, 
                             ( const int32_t * )base_ref_offset, 
                             ref_offset_len );
    }
    if ( rc == 0 )
    {
        if ( iter->pos_ofs_idx == 0 )
            al_iter_adjust_next( iter );
        KRefcountInit( &iter->refcount, 1, "AlignmentIterator", "Make", "align" );
        data_len = 0;
        while( ( iter->abs_ref_start + iter->rel_ref_pos ) < ref_window_start && rc == 0 )
        {
            rc = AlignmentIteratorNext ( iter );
            data_len++;
        }
    }
    if ( rc == 0 )  {   rc = AlignMgrAddRef ( almgr );  }
    if ( rc == 0 )  {   iter->amgr = almgr;  }

/*
    if ( rc == 0 )
    {
        OUTMSG(( "placement[%lu] at %u (w=%u) next(%u) \n", 
                 placement->id, iter->abs_ref_start, ref_window_start, data_len ));
    }
*/
    return rc;
}


LIB_EXPORT rc_t CC AlignIteratorRecordSize ( struct VCursor const *curs, int64_t row_id, size_t *size, void *data )
{
    uint32_t ref_offset_len, read_len;

    rc_t rc = get_idx_and_read( curs, COL_REF_OFFSET, row_id, NULL, &ref_offset_len );
    if ( rc == 0 )
        rc = get_idx_and_read( curs, COL_READ, row_id, NULL, &read_len );

    if ( rc == 0 )
    {
        AlignmentIterator * ali = NULL;
        size_t po_size = ( ( sizeof *(ali->pos_ofs) ) * ( ref_offset_len ) );
        *size = ( ( sizeof *ali ) + ( read_len * 2 ) + po_size );
    }

    return rc;
}


LIB_EXPORT rc_t CC AlignMgrMakeAlignmentIterator ( struct AlignMgr const *self,
    AlignmentIterator **iter,
    bool copy,
    INSDC_coord_zero ref_pos,
    INSDC_coord_len ref_len,
    const INSDC_4na_bin *read,
    INSDC_coord_len read_len,
    const bool *has_mismatch,
    const bool *has_ref_offset,
    const int32_t *ref_offset,
    uint32_t ref_offset_len,
    INSDC_coord_zero ref_window_start,
    INSDC_coord_len ref_window_len )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcConstructing, rcSelf, rcNull );
    else
    {
        if ( iter == NULL || has_mismatch == NULL || has_ref_offset == NULL ||
             read == NULL || read_len == 0 )
            rc = RC( rcAlign, rcIterator, rcConstructing, rcParam, rcNull );
        else
        {
            AlignmentIterator * ali = NULL;
            size_t po_size = ( ( sizeof *(ali->pos_ofs) ) * ( ref_offset_len ) );
            size_t full_size = ( sizeof *ali ) + ( read_len * 2 ) + po_size;
      
            uint8_t * ptr = calloc( full_size, 1 );
            *iter = NULL;
            if ( ptr == NULL )
                rc = RC( rcAlign, rcIterator, rcConstructing, rcMemory, rcExhausted );
            else
            {
                ali = ( AlignmentIterator * )ptr;
                ptr += ( sizeof *ali );

                KRefcountInit( &ali->refcount, 1, "AlignmentIterator", "Make", "align" );

                /* copy HAS_MISMATCH into place, point the header-value to it, advance */
                memmove( ptr, has_mismatch, read_len );
                ali->has_mismatch = (bool *)ptr;
                ptr += read_len;

                /* copy READ into place, point the header-value to it, advance */
                memmove( ptr, read, read_len );
                ali->read = ( INSDC_4na_bin * )ptr;
                ptr += read_len;

                ali->read_len = read_len;
                ali->ref_len = ref_len;
                ali->abs_ref_start = ref_pos;
                ali->pos_ofs = (pos_offset *)ptr;
                ali->ref_window_start = ref_window_start;
                ali->ref_window_len = ref_window_len;
                ali->free_on_whack = true;

                rc = compute_posofs( ali, has_ref_offset, ref_offset, ref_offset_len );
                if ( ali->pos_ofs_idx == 0 )
                    al_iter_adjust_next( ali );

                if ( rc == 0 )
                {
                    rc = AlignMgrAddRef ( self );
                    if ( rc == 0 )
                        ali->amgr = self;
                }
            }

            if ( rc == 0 )
                *iter = ali;
            else
                free( ali );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC AlignmentIteratorAddRef( const AlignmentIterator * cself )
{
    rc_t rc = 0;
    if ( cself == NULL )
        rc = RC( rcAlign, rcIterator, rcAttaching, rcSelf, rcNull );
    else
    {
        if ( KRefcountAdd( &cself->refcount, "AlignmentIterator" ) != krefOkay )
        {
            rc = RC( rcAlign, rcIterator, rcAttaching, rcError, rcUnexpected );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC AlignmentIteratorRelease( const AlignmentIterator * cself )
{
    rc_t rc = 0;
    if ( cself == NULL )
        rc = RC( rcAlign, rcIterator, rcReleasing, rcSelf, rcNull );
    else
    {
        if ( KRefcountDrop( &cself->refcount, "AlignmentIterator" ) == krefWhack )
        {
            AlignmentIterator * self = ( AlignmentIterator * ) cself;
            AlignMgrRelease ( self->amgr );
            if ( self->free_on_whack )
                free( self );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC AlignmentIteratorNext ( AlignmentIterator *self )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcPositioning, rcSelf, rcNull );
    else
    {
        self->flags = 0;
        self->rel_ref_pos++;

        if ( self->rel_ref_pos >= self->ref_len )
            rc = SILENT_RC( rcAlign, rcIterator, rcPositioning, rcItem, rcDone );
        else
        {
            if ( self->skip > 0 )
            {
                self->flags |=  align_iter_skip;
                self->skip--;
            }
            else if ( self->inserts > 0 )
            {
                self->seq_pos += ( self->inserts + 1 );
                self->inserts = 0;
            }
            else
            {
                self->seq_pos++;
            }
            al_iter_adjust_next( self );
        }
    }
    return rc;
}


LIB_EXPORT int32_t CC AlignmentIteratorState ( const AlignmentIterator *self,
                                               INSDC_coord_zero *seq_pos )
{
    uint32_t res = align_iter_invalid;
    if ( self != NULL )
    {
        INSDC_coord_zero pos = self->seq_pos;
        if ( pos < self->read_len )
        {
            res = ( self->read[ pos ] & 0x0F );

            if ( self->rel_ref_pos < 1 )
                res |= align_iter_first;

            if ( self->rel_ref_pos == ( self->ref_len - 1 ) )
                res |= align_iter_last;

            if ( !self->has_mismatch[ pos ] )
                res |= align_iter_match;

            res |= self->flags;

            if ( seq_pos != NULL )
                *seq_pos = pos;
        }
    }
    return res;
}


LIB_EXPORT rc_t CC AlignmentIteratorPosition ( const AlignmentIterator *self,
                                               INSDC_coord_zero *pos )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcPositioning, rcSelf, rcNull );
    else
    {
        if ( pos == NULL )
            rc = RC( rcAlign, rcIterator, rcPositioning, rcParam, rcNull );
        else
        {
            *pos = ( self->abs_ref_start + self->rel_ref_pos );
        }
    }
    return rc;
}


LIB_EXPORT uint32_t CC AlignmentIteratorBasesInserted
    ( const AlignmentIterator *self, const INSDC_4na_bin **bases )
{
    uint32_t res = 0;
    if ( ( self != NULL )&&( bases != NULL ) )
    {
        res = self->inserts;
        *bases = &( self->read[ self->seq_pos + 1 ] );
    }
    return res;
}


LIB_EXPORT uint32_t CC AlignmentIteratorBasesDeleted
    ( const AlignmentIterator *self, INSDC_coord_zero *pos )
{
    uint32_t res = 0;
    if ( ( self != NULL )&&( pos != NULL ) )
    {
        res = ( self->skip );
        *pos = ( self->abs_ref_start + self->rel_ref_pos + 1 ); 
    }
    return res;
}


/* ============ private static functions ============ */
static rc_t compute_posofs(  AlignmentIterator * self,
    const bool * has_ref_offset, const int32_t * ref_offset, uint32_t ref_offset_len )
{
    rc_t rc = 0;

    self->pos_ofs_idx = 0;
    self->pos_ofs_cnt = ref_offset_len;
    if ( ref_offset_len > 0 )
    {
        int32_t shift = 0;
        uint32_t seq_position;
        uint32_t src = 0;
        uint32_t dst = 0;

        if ( has_ref_offset[ 0 ] && ( ref_offset[ 0 ] < 0 ) )
        {
            shift = ref_offset[ src++ ];
            self->seq_pos = -( shift );
            self->pos_ofs_cnt--;
        }

        seq_position = self->seq_pos;
        while( seq_position < self->read_len && rc == 0 )
        {
            if ( has_ref_offset[ seq_position ] )
            {
                /* we do have to process a reference-offset ! */
                if ( src < ref_offset_len )
                {
                    /* we do have a ref-offset value available... */
                    int32_t ro = ref_offset[ src++ ];
                    if ( ro == 0 )
                    {
                        /* zero-values in REF_OFFSET are an error ! */
                        rc = RC( rcAlign, rcIterator, rcConstructing, rcItem, rcNull );
                    }
                    else
                    {
                        /* ref-offset is positive: DELETE against the reference */
                        self->pos_ofs[ dst ].pos = ( seq_position + shift - 1 );
                        self->pos_ofs[ dst++ ].offset = ro;
                        shift += ro;

                        /* !!! CHANGE on May 04 2012 !!!
                           the unused bits in has_ref_offset after an insert ( negative ro )
                           are used now to hint the position of a "B"-cigar-string-case
                           for cSRA-files created Complete-Genomic-Submissions
                           that means we have to jump forward with seq_position in this case!
                        */
                        if ( ro < 0 )
                        {
							/* Jan 15 2015 */
							/* fix the increment in case of 2 subsequent inserts ( which should be merged, but are not ) */
							
                            seq_position -= ( ro + 1 );
                        }
                    }
                }
                else
                {
                    /* if has_ref_offset has more flags than ref_offset has values... */
                    rc = RC( rcAlign, rcIterator, rcConstructing, rcItem, rcTooBig );
                }
            }
            ++seq_position;
        }

/*      OUTMSG(( "pos_ofs:" ));
        for ( src = 0; src < self->pos_ofs_cnt; ++src )
        {
            OUTMSG(( "[%u/%u]", self->pos_ofs[ src ].pos, self->pos_ofs[ src ].offset ));
        }
        OUTMSG(( "\n" )); */
    }
    return rc;
}


static void al_iter_adjust_next( AlignmentIterator * self )
{
    if ( self->pos_ofs_idx < self->pos_ofs_cnt )
    {
        if ( self->rel_ref_pos == self->pos_ofs[ self->pos_ofs_idx ].pos )
        {
            /* we have to adjust... */
            int32_t ro = self->pos_ofs[ self->pos_ofs_idx++ ].offset;
            if ( ro < 0 )
            {
                self->flags |= align_iter_insert;
                self->inserts = (-ro);
            }
            else
            {
                self->flags |= align_iter_delete;
                self->skip = ro;
            }
        }
    }
}
