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
#include <klib/text.h>
#include <klib/out.h>
#include <insdc/insdc.h>
#include <align/reference.h>
#include <align/iterator.h>
#include <align/manager.h>
#include <sysalloc.h>

#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define COL_READ "(INSDC:4na:bin)READ"
#define COL_HAS_MISMATCH "(bool)HAS_MISMATCH"
#define COL_HAS_REF_OFFSET "(bool)HAS_REF_OFFSET"
#define COL_REF_OFFSET "(I32)REF_OFFSET"
#define COL_READ_GROUP "(ascii)SEQ_SPOT_GROUP"


typedef struct spot_group
{
    DLNode n;                       /* to have it in a DLList */
    char * name;                    /* the name of the read-group, can be NULL */
    size_t len;                     /* the length of the name */
    DLList records;                 /* has list of PlacementRecords... */
} spot_group;


static rc_t make_spot_group( spot_group ** sg, DLList * list, const char * name, size_t len )
{
    rc_t rc = 0;
    *sg = calloc( 1, sizeof ** sg );
    if ( *sg == NULL )
        rc = RC( rcAlign, rcIterator, rcConstructing, rcMemory, rcExhausted );
    else
    {
        if ( len > 0 && name != NULL )
        {
            (*sg)->name = string_dup( name, len );
            if ( (*sg)->name != NULL )
            {
                (*sg)->len = len;
            }
        }
        /* if name is NULL, the spot-group is initialized with 0 via calloc() */
        DLListInit( &( (*sg)->records ) );
        DLListPushTail ( list, ( DLNode * )(*sg) );
    }
    return rc;
}


static void CC whack_the_placement_record( DLNode *n, void *data )
{    PlacementRecordWhack ( ( PlacementRecord * )n );   }

static void free_spot_group( spot_group *sg )
{
    if ( sg->name != NULL ) free( sg->name );
    DLListWhack ( &sg->records, whack_the_placement_record, NULL );
    free( sg );
}


static void CC whack_the_spot_group( DLNode *n, void *data )
{    free_spot_group ( ( spot_group * )n );   }

static void clear_spot_group_list( DLList * list )
{
    DLListWhack ( list, whack_the_spot_group, NULL );
}


typedef struct find_spot_group_cb_ctx
{
    const char * name;
    size_t len;
    spot_group *res;
} find_spot_group_cb_ctx;


static bool CC find_spot_group_callback( DLNode *n, void *data )
{
    find_spot_group_cb_ctx *ctx = ( find_spot_group_cb_ctx * )data;
    spot_group * sg = ( spot_group * ) n;
    bool res = false;

    if ( ctx->name == NULL || sg->name == NULL )
    {
        res = true;
    }
    else
    {
        res = ( string_cmp ( sg->name, sg->len, 
                             ctx->name, ctx->len, 
                             ( sg->len < ctx->len ) ? ctx->len : sg->len ) == 0 );
    }

    if ( res )
    {
        ctx->res = sg;
    }
    return res;
}

static spot_group * find_spot_group( DLList * list, const char * name, size_t len )
{
    find_spot_group_cb_ctx ctx;
    ctx.res = NULL;
    ctx.name = name;
    ctx.len = len;
    DLListDoUntil ( list, false, find_spot_group_callback, &ctx );
    return ctx.res;
}

static rc_t add_to_spot_groups( DLList * list, const PlacementRecord *rec )
{
    rc_t rc = 0;
    spot_group * sg = find_spot_group( list, rec->spot_group, rec->spot_group_len );
    if ( sg == NULL )
    {
        rc = make_spot_group( &sg, list, rec->spot_group, rec->spot_group_len );
    }
    if ( rc == 0 )
    {
        DLListPushTail ( &sg->records, ( DLNode * )(rec) );
    }
    return rc;
}


static uint32_t remove_invalid_records( const struct ReferenceObj * const refobj,
                                        DLList * list, INSDC_coord_zero pos )
{
    uint32_t res = 0;
    spot_group * sg = ( spot_group * )DLListHead( list );
    while ( sg != NULL )
    {
        spot_group *nxt = ( spot_group * )DLNodeNext( ( DLNode * )sg );
        PlacementRecord *rec = ( PlacementRecord * )DLListHead( &sg->records );
        while ( rec != NULL )
        {
            PlacementRecord *nxt_rec = ( PlacementRecord * )DLNodeNext( ( DLNode * )rec );
            INSDC_coord_zero end_pos = ( rec->pos + rec->len );
            bool remove = ( end_pos <= pos );
            if ( !remove )
            {
                AlignmentIterator * al_iter = PlacementRecordCast ( rec, placementRecordExtension0 );
                int32_t state = AlignmentIteratorState ( al_iter, NULL );
                remove = ( ( state & align_iter_invalid ) == align_iter_invalid );
            }
            if ( remove )
            {
                DLListUnlink ( &sg->records, ( DLNode * )rec );
                PlacementRecordWhack ( rec );
            }
            else
            {
                res++;
            }
            rec = nxt_rec;
        }
        sg = nxt;
    }
    return res;
}


static void inc_alignment_iterators( DLList * list, INSDC_coord_zero pos )
{
    spot_group * sg = ( spot_group * )DLListHead( list );
    while ( sg != NULL )
    {
        spot_group *nxt = ( spot_group * )DLNodeNext( ( DLNode * )sg );
        PlacementRecord *rec = ( PlacementRecord * )DLListHead( &sg->records );
        while ( rec != NULL )
        {
            PlacementRecord *nxt_rec = ( PlacementRecord * )DLNodeNext( ( DLNode * )rec );
            AlignmentIterator * al_iter = PlacementRecordCast ( rec, placementRecordExtension0 );
            if ( rec->pos <= pos && al_iter != NULL )
            {
                AlignmentIteratorNext ( al_iter );
            }
            rec = nxt_rec;
        }
        sg = nxt;
    }
}

/* ======================================================================================== */


struct ReferenceIterator
{
    KRefcount refcount;
    struct AlignMgr const *amgr;

    DLList spot_groups;                     /* has a list of spot-groups... */

    int32_t min_mapq;                       /* has a minimum mapq-value... */
    PlacementRecordExtendFuncs ext_func;    /* has a struct with record-extension-functions from client*/
    PlacementRecordExtendFuncs int_func;    /* has a struct with record-extension-functions for itself*/

    uint32_t depth;                         /* how many records are in the list */
    INSDC_coord_zero current_pos;           /* what is the current ref-position on the current ref. */
    INSDC_coord_zero last_pos;              /* what is the current ref-position on the current ref. */
    INSDC_coord_zero nxt_avail_pos;         /* what is the next available ref-position on the current ref. */
    spot_group *current_spot_group;         /* what is the next spot-group to be handled */
    PlacementRecord *current_rec;           /* the current-record at the current position */
    bool need_init;                         /* do we need to init for the first next()-call */
    PlacementSetIterator * pl_set_iter;     /* holds a list of placement-iterators */
    struct ReferenceObj const * refobj;     /* cached result of ReferenceIteratorNextReference(...) */
};


LIB_EXPORT void CC RefIterRecordDestroy ( void *obj, void *data )
{
    /* nothing to do, because there are no sub-allocations etc. ... */
}


LIB_EXPORT rc_t CC RefIterRecordSize ( struct VCursor const *curs,
    int64_t row_id, size_t * size, void *data, void * placement_ctx )
{
    /* discover the size of the ref-iter-part to be allocated... */
    return AlignIteratorRecordSize ( curs, row_id, size, data );
}


LIB_EXPORT rc_t CC RefIterRecordPopulate ( void *obj,
    const PlacementRecord *placement, struct VCursor const *curs,
    INSDC_coord_zero ref_window_start, INSDC_coord_len ref_window_len, void *data, void * placement_ctx )
{
    return AlignIteratorRecordPopulate ( obj, placement, curs, ref_window_start, ref_window_len, data );
}


static void CC RefIterDestroyRecPart( void *obj, void *data )
{
    AlignmentIterator *iter = ( AlignmentIterator * )obj;
    if ( iter != NULL )
        AlignmentIteratorRelease( iter );
}

LIB_EXPORT rc_t CC AlignMgrMakeReferenceIterator ( struct AlignMgr const *self,
    ReferenceIterator **iter, const PlacementRecordExtendFuncs *ext_1, int32_t min_mapq )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcConstructing, rcSelf, rcNull );
    else
    {
        if ( iter == NULL  )
            rc = RC( rcAlign, rcIterator, rcConstructing, rcParam, rcNull );
        else
        {
            ReferenceIterator * refi = calloc( sizeof * refi, 1 );
            if ( refi == NULL )
                rc = RC( rcAlign, rcIterator, rcConstructing, rcMemory, rcExhausted );
            else
            {
                KRefcountInit( &refi->refcount, 1, "ReferenceIterator", "Make", "align" );
                refi->min_mapq = min_mapq;
                if ( ext_1 != NULL )
                {
                    refi->ext_func.data = ext_1->data;
                    refi->ext_func.destroy = ext_1->destroy;
                    refi->ext_func.populate = ext_1->populate;
                    refi->ext_func.alloc_size = ext_1->alloc_size;
                    refi->ext_func.fixed_size = ext_1->fixed_size;
                }

                refi->int_func.data = ( void * )self;
                refi->int_func.destroy = RefIterDestroyRecPart;
                refi->int_func.populate = RefIterRecordPopulate;
                refi->int_func.alloc_size = RefIterRecordSize; 

                DLListInit( &(refi->spot_groups) );
                rc = AlignMgrMakePlacementSetIterator ( self, &refi->pl_set_iter );
                refi->need_init = true;
            }

            if ( rc == 0 )
                rc = AlignMgrAddRef ( self );

            if ( rc == 0 )
            {
                refi->amgr = self;
                *iter = refi;
            }
            else
                free( refi );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorAddRef ( const ReferenceIterator *self )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcAttaching, rcSelf, rcNull );
    else
    {
        if ( KRefcountAdd( &self->refcount, "ReferenceIterator" ) != krefOkay )
        {
            rc = RC( rcAlign, rcIterator, rcAttaching, rcError, rcUnexpected );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorRelease ( const ReferenceIterator *cself )
{
    rc_t rc = 0;
    if ( cself == NULL )
        rc = RC( rcAlign, rcIterator, rcReleasing, rcSelf, rcNull );
    else
    {
        if ( KRefcountDrop( &cself->refcount, "ReferenceIterator" ) == krefWhack )
        {
            ReferenceIterator * self = ( ReferenceIterator * ) cself;
            /* we 'own' the records! - we have to destroy them, if some are left in here */
            clear_spot_group_list( &self->spot_groups );
            rc = PlacementSetIteratorRelease ( self->pl_set_iter );
            AlignMgrRelease ( self->amgr );
            free( self );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorAddPlacementIterator( ReferenceIterator *self,
    PlacementIterator *pi )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcConstructing, rcSelf, rcNull );
    else
    {
        if ( pi == NULL )
            rc = RC( rcAlign, rcIterator, rcConstructing, rcParam, rcNull );
        else
        {
            rc = PlacementSetIteratorAddPlacementIterator ( self->pl_set_iter, pi );
        }
    }
    return rc;
}


#define ALIGN_COL_COUNT 4

static const char * align_cols[ ALIGN_COL_COUNT ] = 
{ COL_REF_OFFSET, COL_HAS_REF_OFFSET, COL_HAS_MISMATCH, COL_READ };


static rc_t prepare_align_cursor( struct VCursor const *align )
{
    rc_t rc = 0;
    uint32_t i, throw_away_idx;

    for ( i = 0; i < ALIGN_COL_COUNT && rc == 0; ++i ) {
        rc =VCursorAddColumn ( align, &throw_away_idx, "%s", align_cols[ i ] );
        if( GetRCState(rc) == rcExists ) {
            rc = 0;
        }
    }
        
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorAddPlacements( ReferenceIterator *self,
     struct ReferenceObj const *ref_obj, INSDC_coord_zero ref_pos, INSDC_coord_len ref_len,
     struct VCursor const *ref, struct VCursor const *align, align_id_src ids,
     const char * spot_group, void * placement_ctx )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcConstructing, rcSelf, rcNull );
    else
    {
        if ( ref_obj == NULL )
            rc = RC( rcAlign, rcIterator, rcConstructing, rcParam, rcNull );
        else
        {
            if ( align != NULL )
                rc = prepare_align_cursor( align );

            if ( rc == 0 )
            {
                PlacementIterator *pi;

                rc = ReferenceObj_MakePlacementIterator ( ref_obj, &pi, ref_pos, ref_len, self->min_mapq,
                        ref, align, ids, &self->int_func, &self->ext_func, spot_group, placement_ctx );
                if ( rc == 0 )
                {
                    rc = PlacementSetIteratorAddPlacementIterator ( self->pl_set_iter, pi );
                    if ( GetRCState( rc ) == rcDone ) { rc = 0; }
                }
            }
        }
    }
    return rc;
}


static rc_t fill_recordlist( ReferenceIterator *self, INSDC_coord_zero pos )
{
    rc_t rc;
    do
    {
        const PlacementRecord *rec;
        /* from the placement-set-iterator into our list... */
        rc = PlacementSetIteratorNextRecordAt ( self->pl_set_iter, pos, &rec );
        if ( rc == 0 )
        {
            if ( rec->pos == pos )
            {
                self->depth++;
                rc = add_to_spot_groups( &self->spot_groups, rec );
            }
            else
                PlacementRecordWhack ( rec );
        }
    } while( rc == 0 );

    if ( GetRCState( rc ) == rcDone ) { rc = 0; }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorNextReference ( ReferenceIterator *self,
    INSDC_coord_zero *first_pos, INSDC_coord_len *len, struct ReferenceObj const ** refobj )
{
    rc_t rc = 0;

    if ( refobj != NULL )
        *refobj = NULL;

    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcAccessing, rcSelf, rcNull );
    else
    {
        struct ReferenceObj const * robj;
        rc = PlacementSetIteratorNextReference ( self->pl_set_iter, first_pos, len, &robj );
        clear_spot_group_list( &self->spot_groups );
        if ( rc == 0 )
        {
            /* cache the returned refobj in order to get to reference-bases later... */
            self->refobj = robj;
            self->need_init = true;
        }
        else
        {
            self->refobj = NULL;
        }

        if ( refobj != NULL )
        {
            *refobj = self->refobj;
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorNextWindow ( ReferenceIterator *self,
    INSDC_coord_zero *first_pos, INSDC_coord_len *len )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcAccessing, rcSelf, rcNull );
    else
    {
        rc = PlacementSetIteratorNextWindow ( self->pl_set_iter, first_pos, len );
        clear_spot_group_list( &self->spot_groups );
        if ( rc == 0 )
        {
            self->need_init = true;
            self->current_pos = *first_pos;
            self->current_spot_group = NULL;
            self->last_pos = self->current_pos + *len - 1;
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorNextSpotGroup ( ReferenceIterator *self,
    const char ** name, size_t * len )
{
    rc_t rc = 0;

    if ( self->current_spot_group == NULL )
    {
        self->current_spot_group = ( spot_group * )DLListHead( &self->spot_groups );
        if ( self->current_spot_group == NULL )
        {
            rc = SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
        }
    }
    else
    {
        spot_group *nxt  = ( spot_group * )DLNodeNext( ( DLNode * ) self->current_spot_group );
        if ( nxt == NULL )
        {
            rc = SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
        }
        else
        {
            self->current_spot_group = nxt;
        }
    }
    self->current_rec = NULL;

    if ( rc == 0 && self->current_spot_group != NULL )
    {
        if ( name != NULL )
        {
            *name = self->current_spot_group->name;
        }
        if ( len != NULL )
        {
            *len = self->current_spot_group->len;
        }
    }
    return rc;
}


/* iterates the PlacementSetIterator forward to reach self->current_pos on the reference */
static rc_t first_ref_iter_nxt_pos( ReferenceIterator *self, bool skip_empty )
{
    rc_t rc;
    bool loop;
    self->need_init = false;
    do
    {
        rc = PlacementSetIteratorNextAvailPos ( self->pl_set_iter, &self->nxt_avail_pos, NULL );
        loop = ( rc == 0 );
        if ( loop )
        {
            loop = ( self->nxt_avail_pos <= self->current_pos );
            if ( loop )
            {
                rc = fill_recordlist( self, self->nxt_avail_pos );
                self->depth = remove_invalid_records( self->refobj, &self->spot_groups, self->nxt_avail_pos - 1 );
            }
        }
    } while ( loop );

    if ( skip_empty && self->nxt_avail_pos > self->current_pos && self->depth == 0 )
    {
        self->current_pos = self->nxt_avail_pos;
    }
    self->depth = remove_invalid_records( self->refobj, &self->spot_groups, self->current_pos );

    if ( GetRCState( rc ) == rcDone ) rc = 0;
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorNextPos ( ReferenceIterator *self, bool skip_empty )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcAccessing, rcSelf, rcNull );
    else
    {
        self->current_rec = NULL;
        if ( self->need_init )
        {
            rc = first_ref_iter_nxt_pos( self, skip_empty );
        }
        else
        {
            /* increment the current position */
            self->current_pos++;

            if ( self->current_pos <= self->last_pos )
            {
                /* jump over gaps, if requested ... */
                if ( self->depth == 0 && skip_empty )
                {
                    self->current_pos = self->nxt_avail_pos;
                }

                /* increment the internal alignment-iterator of every placement-record */
                inc_alignment_iterators( &self->spot_groups, self->current_pos );

                /* loop through the list to look if we have to remove records,
                   that do end before this new position */
                self->depth = remove_invalid_records( self->refobj, &self->spot_groups, self->current_pos );

                rc = fill_recordlist( self, self->current_pos );
                if ( rc == 0 )
                {
                    self->current_spot_group = NULL;
                    /* set our sights to the next position... */
                    rc = PlacementSetIteratorNextAvailPos ( self->pl_set_iter, &self->nxt_avail_pos, NULL );
                    if ( GetRCState( rc ) == rcDone )
                    {
                        if ( self->depth > 0 )
                        {
                            rc = 0;
                        }
                        else if ( !skip_empty )
                        {
                            if ( self->current_pos <= self->last_pos ) rc = 0;
                        }
                    }
                }
            }
            else
            {
                rc = SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
                clear_spot_group_list( &self->spot_groups );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorPosition ( const ReferenceIterator *self,
    INSDC_coord_zero *pos, uint32_t * depth, INSDC_4na_bin * base )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcAccessing, rcSelf, rcNull );
    else
    {
        /* return position, many records our record-list, and the base at this position */
        if ( pos != NULL )
        {
            *pos = self->current_pos;
        }

        if ( depth != NULL )
        {
            *depth = self->depth;
        }

        if ( base != NULL )
        {
            uint32_t written;
            *base = 0;
            /* problem! how to get the base if depth == 0 */
            if ( self->current_rec != NULL )
            {
                rc = ReferenceObj_Read( self->current_rec->ref, self->current_pos, 1, base, &written );
            }
            else if ( self->refobj != NULL )
            {
                rc = ReferenceObj_Read( self->refobj, self->current_pos, 1, base, &written );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorNextPlacement ( ReferenceIterator *self,
    const PlacementRecord **rec )
{
    rc_t rc = 0;
    if ( self == NULL )
    {
        rc = RC( rcAlign, rcIterator, rcAccessing, rcSelf, rcNull );
    }
    else if ( rec == NULL )
    {
        rc = RC( rcAlign, rcIterator, rcAccessing, rcParam, rcNull );
    }
    else
    {
        if ( self->current_spot_group == NULL )
        {
            rc = SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
        }
        else
        {
            if ( self->current_rec == NULL )
            {
                self->current_rec = ( PlacementRecord * )DLListHead( &self->current_spot_group->records );
            }
            else
            {
                self->current_rec = ( PlacementRecord * )DLNodeNext( ( DLNode * )self->current_rec );
            }

            if ( self->current_rec == NULL )
            {
                rc = SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
            }
            else
            {
                *rec = self->current_rec;
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC ReferenceIteratorGetPlacement ( ReferenceIterator *self,
    const PlacementRecord **rec )
{
    rc_t rc = 0;
    if ( self == NULL )
    {
        rc = RC( rcAlign, rcIterator, rcAccessing, rcSelf, rcNull );
    }
    else if ( rec == NULL )
    {
        rc = RC( rcAlign, rcIterator, rcAccessing, rcParam, rcNull );
    }
    else
    {
        if ( self->current_spot_group == NULL )
        {
            rc = SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
        }
        else
        {
            if ( self->current_rec != NULL )
            {
                /* remove the 'previous' current-rec! */
                DLListPopHead ( &self->current_spot_group->records );
                PlacementRecordWhack ( self->current_rec );
                self->depth--;
                self->current_rec = NULL;
            }

            self->current_rec = ( PlacementRecord * )DLListHead( &self->current_spot_group->records );
            if ( self->current_rec == NULL )
            {
                rc = SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
            }
            else
            {
                *rec = self->current_rec;
            }
        }
    }
    return rc;
}


LIB_EXPORT int32_t CC ReferenceIteratorState ( const ReferenceIterator *self,
    INSDC_coord_zero *seq_pos )
{
    int32_t res = align_iter_invalid;
    if ( seq_pos != NULL )
    {
        *seq_pos = 0;
    }
    if ( self != NULL )
    {
        /* PlacementRecordCast returns NULL if self->current_rec is NULL */
        AlignmentIterator * al_iter = PlacementRecordCast ( self->current_rec, placementRecordExtension0 );
        if ( al_iter != NULL )
            res = AlignmentIteratorState ( al_iter, seq_pos );
    }
    return res;
}


LIB_EXPORT uint32_t CC ReferenceIteratorBasesInserted ( const ReferenceIterator *self,
    const INSDC_4na_bin **bases )
{
    uint32_t res = align_iter_invalid;
    if ( bases != NULL )
    {
        *bases = NULL;
    }
    if ( self != NULL )
    {
        /* PlacementRecordCast returns NULL if self->current_rec is NULL */
        AlignmentIterator * al_iter = PlacementRecordCast ( self->current_rec, placementRecordExtension0 );
        if ( al_iter != NULL )
            res = AlignmentIteratorBasesInserted( al_iter, bases );
    }
    return res;
}


LIB_EXPORT uint32_t CC ReferenceIteratorBasesDeleted ( const ReferenceIterator *self,
    INSDC_coord_zero *pos, const INSDC_4na_bin **bases )
{
    uint32_t res = align_iter_invalid;
    if ( bases != NULL )
    {
        *bases = NULL;
    }
    if ( self != NULL )
    {
        /* PlacementRecordCast returns NULL if self->current_rec is NULL */
        AlignmentIterator * al_iter = PlacementRecordCast ( self->current_rec, placementRecordExtension0 );
        if ( al_iter != NULL )
        {
            INSDC_coord_zero temp_pos;
            res = AlignmentIteratorBasesDeleted( al_iter, &temp_pos );
            if ( ( res & align_iter_invalid ) != align_iter_invalid )
            {
                if ( pos != NULL ) { *pos = temp_pos; }
                /* where to get the reference-bases from ? PlacementRecord.ref ! */
                if ( res > 0 && bases != NULL )
                {
                    uint8_t * buffer = malloc( res );
                    if ( buffer != NULL )
                    {
                        INSDC_coord_len written;
                        rc_t rc = ReferenceObj_Read( self->current_rec->ref, temp_pos, res, buffer, &written );
                        if ( rc == 0 )
                        {
                            *bases = buffer;
                        }
                    }
                }
            }
        }
    }
    return res;
}
