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
#include <klib/vector.h>
#include <klib/refcount.h>
#include <klib/sort.h>
#include <klib/text.h>
#include <klib/out.h>
#include <insdc/insdc.h>
#include <align/manager.h>
#include <align/iterator.h>
#include <sysalloc.h>

#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


typedef struct window
{
    INSDC_coord_zero first;
    INSDC_coord_len len;
} window;


typedef struct pi_entry
{
    DLNode n;                       /* to have it in a DLList */
    PlacementIterator *pi;          /* the placement-iterator we have added */
    window nxt_avail;               /* the next available position of the placement-iterator */
} pi_entry;


typedef struct pi_window
{
    DLNode n;                       /* to have it in a DLList */
    window w;                       /* the window of the placement-iterator */
    DLList pi_entries;              /* it has a DLList of pi_entry-struct's */
    uint32_t count;                 /* how many entries do we have */
} pi_window;


typedef struct pi_ref
{
    DLNode n;                       /* to have it in a DLList */
    char * name;                    /* the name of the reference it referes to */
    window outer;                   /* the sum of all windows it has... */
    bool outer_initialized;         /* has the outer-window been initialized */
    DLList pi_windows;              /* it has a DLList of pi_window-struct's */
} pi_ref;


struct PlacementSetIterator
{
    KRefcount refcount;
    struct AlignMgr const *amgr;    /* the alignment-manager... ( right now: we store it, but that's it )*/
    DLList pi_refs;                 /* a list of references we have to iterate over... */
    pi_ref * current_ref;           /* what is the current reference, we are handling ? */
    pi_window * current_window;     /* what is the current window, we are handling ? */
    pi_entry * current_entry;       /* what is the current pi-entry, we are handling ? */
};


/* =================================================================================================== */


LIB_EXPORT rc_t CC AlignMgrMakePlacementSetIterator ( struct AlignMgr const *self,
    PlacementSetIterator **iter )
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
            PlacementSetIterator * psi = calloc( sizeof * psi, 1 );
            if ( psi == NULL )
                rc = RC( rcAlign, rcIterator, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = AlignMgrAddRef ( self );
                if ( rc == 0 )
                {
                    KRefcountInit( &psi->refcount, 1, "PlacementSetIterator", "Make", "align" );
                    psi->amgr = self;
                    psi->current_ref = NULL;          /* we don't know that yet */
                    psi->current_window = NULL;
                    psi->current_entry = NULL;
                    DLListInit( &psi->pi_refs );
                }
            }
            if ( rc == 0 )
                *iter = psi;
            else
                free( psi );
        }
    }

    return rc;
}


/* =================================================================================================== */


static int cmp_pchar( const char * a, const char * b )
{
    int res = 0;
    if ( ( a != NULL )&&( b != NULL ) )
    {
        size_t len_a = string_size( a );
        size_t len_b = string_size( b );
        res = string_cmp ( a, len_a, b, len_b, ( len_a < len_b ) ? len_b : len_a );
    }
    return res;
}


/* =================================================================================================== */


typedef struct pi_ref_cb_ctx
{
    const char * name;
    pi_ref *res;
} pi_ref_cb_ctx;


static bool CC find_pi_ref_callback( DLNode *n, void *data )
{
    pi_ref_cb_ctx *ctx = ( pi_ref_cb_ctx * )data;
    pi_ref * pr = ( pi_ref * ) n;
    bool res = ( cmp_pchar( ctx->name, pr->name ) == 0 );
    if ( res )
    {
        ctx->res = pr;
    }
    return res;
}


static pi_ref * find_pi_ref( const DLList * list, const char * name )
{
    pi_ref_cb_ctx ctx;
    ctx.res = NULL;
    ctx.name = name;
    DLListDoUntil ( list, false, find_pi_ref_callback, &ctx );
    return ctx.res;
}


/* =================================================================================================== */

typedef struct pi_window_cb_ctx
{
    window *w;
    pi_window *res;
} pi_window_cb_ctx;


static bool CC find_pi_window_callback( DLNode *n, void *data )
{
    pi_window_cb_ctx *ctx = ( pi_window_cb_ctx * )data;
    pi_window * pw = ( pi_window * ) n;
    bool res = ( pw->w.first == ctx->w->first && pw->w.len == ctx->w->len );
    if ( res )
        ctx->res = pw;
    return res;
}


static pi_window * find_pi_window( const DLList * list, window * w )
{
    pi_window_cb_ctx ctx;
    ctx.res = NULL;
    ctx.w = w;
    DLListDoUntil ( list, false, find_pi_window_callback, &ctx );
    return ctx.res;
}


/* =================================================================================================== */


static rc_t make_pi_window( pi_window ** pw, DLList * list, window * w )
{
    rc_t rc = 0;
    *pw = calloc( 1, sizeof ** pw );
    if ( *pw == NULL )
        rc = RC( rcAlign, rcIterator, rcConstructing, rcMemory, rcExhausted );
    else
    {
        (*pw)->w.first = w->first;
        (*pw)->w.len = w->len;
        DLListInit( &( (*pw)->pi_entries ) );
        DLListPushTail ( list, ( DLNode * )(*pw) );
    }
    return rc;
}


static rc_t add_to_pi_window( pi_window * pw, PlacementIterator *pi )
{
    rc_t rc = 0;
    pi_entry * pie = calloc( 1, sizeof *pie );
    if ( pie == NULL )
        rc = RC( rcAlign, rcIterator, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = PlacementIteratorNextAvailPos ( pi, &(pie->nxt_avail.first), &(pie->nxt_avail.len) );
        if ( rc == 0 )
        {
            pie->pi = pi;  /* store the placement-iterator in it's entry-struct */
            DLListPushTail ( &pw->pi_entries, ( DLNode * )pie );
            pw->count += 1;
        }
        else
        {
            free( pie );
            ALIGN_DBG( "PlacementIter has no placements...", 0 );
        }
    }
    return rc;
}


/* =================================================================================================== */


static rc_t make_pi_ref( pi_ref ** pr, DLList * list, const char * name )
{
    rc_t rc = 0;
    *pr = calloc( 1, sizeof ** pr );
    if ( *pr == NULL )
        rc = RC( rcAlign, rcIterator, rcConstructing, rcMemory, rcExhausted );
    else
    {
        (*pr)->name = string_dup_measure ( name, NULL );
        DLListInit( &( (*pr)->pi_windows ) );
        DLListPushTail ( list, ( DLNode * )(*pr) );
    }
    return rc;
}


static rc_t add_to_pi_ref( pi_ref * pr, window * w, PlacementIterator *pi )
{
    rc_t rc = 0;
    pi_window * pw = find_pi_window( &pr->pi_windows, w );

    if ( pw == NULL )
        rc = make_pi_window( &pw, &pr->pi_windows, w );
    if ( rc == 0 )
        rc = add_to_pi_window( pw, pi );

    if ( rc == 0 )
    {
        /* keep track of the outer window... */
        if ( DLListHead( &pr->pi_windows ) == NULL )
        {
            /* first window ?*/
            pr->outer.first = w->first;
            pr->outer.len = w->len;
        }
        else
        {
            if ( w->first < pr->outer.first )
                pr->outer.first = w->first;
            if ( w->first + w->len > pr->outer.first + pr->outer.len )
                pr->outer.len = ( ( w->first + w->len ) - pr->outer.first ) + 1;
        }
    }
    else if ( ( pw != NULL )&&( GetRCState( rc ) == rcDone ) )
    {
        /* add_to_pi_window() was not successful because iterator has no
           alignments int the requested window, that means we have to delete
           the window if it is empty */
        if ( pw->count == 0 )
        {
            /* first we have to take the pw out of the pr->pi_windows - list...
               it was pushed at the tail of it, so we pop it from there */
            DLListPopTail( &pr->pi_windows );
            /* because it is empty ( count == 0 ) we can just free it now */
            free( pw );
        }
    }
    return rc;
}


/* =================================================================================================== */


LIB_EXPORT rc_t CC PlacementSetIteratorAddPlacementIterator ( PlacementSetIterator *self,
    PlacementIterator *pi )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcConstructing, rcSelf, rcNull );
    else
    {
        if ( pi == NULL  )
            rc = RC( rcAlign, rcIterator, rcConstructing, rcParam, rcNull );
        else
        {
            const char * name;      /* what reference are we aligning against */
            window w;               /* where does the pi start/end, against said reference */

            /* to find the name of the reference used, important for adding the iterator */
            rc = PlacementIteratorRefWindow ( pi, &name, &(w.first), &(w.len) );
            if ( rc == 0 )
            {
                pi_ref * pr = find_pi_ref( &self->pi_refs, name );
                /* if we do not have a pi_ref yet with this name: make one! */
                if ( pr == NULL )
                    rc = make_pi_ref( &pr, &self->pi_refs, name );
                /* add the placement-iterator to the newly-made or existing pi_ref! */
                if ( rc == 0 )
                    rc = add_to_pi_ref( pr, &w, pi );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC PlacementSetIteratorAddRef ( const PlacementSetIterator *cself )
{
    rc_t rc = 0;
    if ( cself == NULL )
        rc = RC( rcAlign, rcIterator, rcAttaching, rcSelf, rcNull );
    else
    {
        if ( KRefcountAdd( &cself->refcount, "PlacementSetIterator" ) != krefOkay )
        {
            rc = RC( rcAlign, rcIterator, rcAttaching, rcError, rcUnexpected );
        }
    }
    return rc;
}


/* =================================================================================================== */


static void CC pi_entry_whacker( DLNode *n, void *data )
{
    pi_entry * pie = ( pi_entry * )n;
    if ( pie->pi != NULL )
    {
        PlacementIteratorRelease ( pie->pi );
        pie->pi = NULL;
    }
    free( pie );
}

static void CC pi_window_whacker( DLNode *n, void *data )
{
    pi_window * pw = ( pi_window * )n;
    DLListWhack ( &pw->pi_entries, pi_entry_whacker, NULL );
    free( pw );
}

static void CC pi_ref_whacker( DLNode *n, void *data )
{
    pi_ref * pr = ( pi_ref * )n;
    DLListWhack ( &pr->pi_windows, pi_window_whacker, NULL );
    free( pr->name );
    free( pr );
}


static void pl_set_iter_clear_curr_ref_window( PlacementSetIterator *self )
{
    if ( self->current_window != NULL )
    {
        pi_window_whacker( (DLNode *)self->current_window, NULL );
        self->current_window = NULL;
    }
}


static void pl_set_iter_clear_curr_ref( PlacementSetIterator *self )
{
    if ( self->current_ref != NULL )
    {
        pi_ref_whacker( (DLNode *)self->current_ref, NULL );
        self->current_ref = NULL;
    }
}

/* =================================================================================================== */


LIB_EXPORT rc_t CC PlacementSetIteratorRelease ( const PlacementSetIterator *cself )
{
    rc_t rc = 0;
    if ( cself == NULL )
        rc = RC( rcAlign, rcIterator, rcReleasing, rcSelf, rcNull );
    else
    {
        if ( KRefcountDrop( &cself->refcount, "PlacementSetIterator" ) == krefWhack )
        {
            PlacementSetIterator * self = ( PlacementSetIterator * ) cself;

            pl_set_iter_clear_curr_ref_window( self );
            pl_set_iter_clear_curr_ref( self );

            /* release the DLList of pi-ref's and the pi's in it... */
            DLListWhack ( &self->pi_refs, pi_ref_whacker, NULL );

            AlignMgrRelease ( self->amgr );

            free( self );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC PlacementSetIteratorNextReference ( PlacementSetIterator *self,
    INSDC_coord_zero *first_pos, INSDC_coord_len *len, struct ReferenceObj const ** refobj )
{
    rc_t rc = 0;
    if ( refobj != NULL ) { *refobj = NULL; }

    if ( self == NULL )
        return RC( rcAlign, rcIterator, rcReleasing, rcSelf, rcNull );

    pl_set_iter_clear_curr_ref_window( self );
    pl_set_iter_clear_curr_ref( self );
    self->current_entry = NULL;     /* what is the current pi-entry, we are handling ? */

    /* !!! here we are taking the next reference from the top of the list */
    self->current_ref = ( pi_ref * )DLListPopHead ( &self->pi_refs );

    if ( self->current_ref == NULL )
    {
        return SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
    }

    if ( first_pos != NULL ) *first_pos = self->current_ref->outer.first;
    if ( len != NULL) *len = self->current_ref->outer.len;

    /* if the caller wants to know the ref-obj... */
    if ( refobj != NULL )
    {
        pi_window *pw = ( pi_window * )DLListHead( &(self->current_ref->pi_windows) );
        if ( pw != NULL )
        {
            pi_entry * pie = ( pi_entry * )DLListHead( &(pw->pi_entries) );
            if ( pie != NULL )
            {
                rc = PlacementIteratorRefObj( pie->pi, refobj );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC PlacementSetIteratorNextWindow ( PlacementSetIterator *self,
    INSDC_coord_zero *first_pos, INSDC_coord_len *len )
{
    rc_t rc = 0;
    if ( first_pos != NULL ) { *first_pos = 0; }
    if ( len != NULL ) { *len = 0; }

    if ( self == NULL )
        return RC( rcAlign, rcIterator, rcReleasing, rcSelf, rcNull );

    self->current_entry = NULL;     /* what is the current pi-entry, we are handling ? */

    if ( self->current_ref == NULL )
    {
        return SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
    }

    pl_set_iter_clear_curr_ref_window( self );

    /* !!! here we are taking the next window from the top of the list */
    self->current_window = ( pi_window * )DLListPopHead ( &(self->current_ref->pi_windows) );

    /* check if we have reached the last window on this reference... */
    if ( self->current_window == NULL )
    {
        return SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
    }

    /* point to the first entry in this window... */
    self->current_entry = ( pi_entry * )DLListHead( &(self->current_window->pi_entries) );

    /* if the caller wants to know first_pos / len */
    if ( first_pos != NULL )
    {
        *first_pos = self->current_window->w.first;
    }
    if ( len != NULL )
    {
        *len = self->current_window->w.len;
    }

    return rc;
}

typedef struct pi_ref_nxt_avail_pos_ctx
{
    uint32_t count;
    INSDC_coord_zero min_pos;
    INSDC_coord_len min_len;
    bool min_pos_initialized;
    rc_t rc;
} pi_ref_nxt_avail_pos_ctx;

static void CC nxt_avail_pos_cb( DLNode * n, void * data )
{
    pi_ref_nxt_avail_pos_ctx * ctx = ( pi_ref_nxt_avail_pos_ctx * ) data;
    if ( ctx->rc == 0 )
    {
        pi_entry * pie = ( pi_entry * )n;
        rc_t rc = PlacementIteratorNextAvailPos ( pie->pi, &(pie->nxt_avail.first), &(pie->nxt_avail.len) );
        if ( rc == 0 )
        {
/*            OUTMSG(( "nxt_avail.first=%u w.last=%u\n", pie->nxt_avail.first, pie->w.last )); */
            ( ctx->count )++;
            if ( ctx->min_pos_initialized )
            {
                if ( pie->nxt_avail.first < ctx->min_pos )
                {
                    ctx->min_pos = pie->nxt_avail.first;
                    ctx->min_len = pie->nxt_avail.len;
                }
            }
            else
            {
                ctx->min_pos = pie->nxt_avail.first;
                ctx->min_len = pie->nxt_avail.len;
                ctx->min_pos_initialized = true;
            }
        }
        else
        {
            if ( GetRCState( rc ) != rcDone )
                ctx->rc = rc;
        }
    }
}


LIB_EXPORT rc_t CC PlacementSetIteratorNextAvailPos ( const PlacementSetIterator *cself,
    INSDC_coord_zero *pos, INSDC_coord_len *len )
{
    rc_t rc = 0;
    if ( cself == NULL )
        rc = RC( rcAlign, rcIterator, rcAccessing, rcSelf, rcNull );
    else
    {
        if ( pos == NULL )
            rc = RC( rcAlign, rcIterator, rcAccessing, rcParam, rcNull );
        else
        {
            PlacementSetIterator *self = ( PlacementSetIterator * )cself;
            if ( self->current_ref == NULL || self->current_window == NULL )
            {
                rc = SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
            }
            else
            {
                /* loop through all the pi_entry int the current_pi_ref */
                pi_ref_nxt_avail_pos_ctx ctx;
                ctx.count = 0;
                ctx.rc = 0;
                ctx.min_pos = 0;
                ctx.min_len = 0;
                ctx.min_pos_initialized = false;
                DLListForEach ( &(self->current_window->pi_entries),
                                false, nxt_avail_pos_cb, &ctx );
                rc = ctx.rc;
                if ( ctx.count == 0 )
                {
                    rc = SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
                }
                else
                {
                    *pos = ctx.min_pos;
                    if ( len != NULL )
                    {
                        *len = ctx.min_len;
                    }
                }
            } 
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC PlacementSetIteratorNextRecordAt ( PlacementSetIterator *self,
    INSDC_coord_zero pos, const PlacementRecord **rec )
{
    rc_t rc = 0;
    pi_window * pw;
    bool done;

    if ( rec == NULL )
        return RC( rcAlign, rcIterator, rcAccessing, rcParam, rcNull );
    *rec = NULL;
    if ( self == NULL )
        return RC( rcAlign, rcIterator, rcAccessing, rcSelf, rcNull );
    if ( self->current_ref == NULL )
    {
        /* no more reference to iterator over! the iterator is done! */
        return SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
    }
    if ( self->current_window == NULL )
    {
        /* no more windows to iterator over! the iterator is done! */
        return SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone );
    }

    pw = self->current_window;
    done = false;
    do
    {
        if ( self->current_entry == NULL )
        {
            self->current_entry = ( pi_entry * )DLListHead( &(pw->pi_entries) );
        }
        done = ( self->current_entry == NULL );
        rc = ( done ? SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone ) : 0 );
        if ( rc == 0 )
        {
            rc = PlacementIteratorNextRecordAt ( self->current_entry->pi, pos, rec );
            done = ( GetRCState( rc ) != rcDone );
            if ( !done )
            {
                self->current_entry = ( pi_entry * )DLNodeNext( ( DLNode * )self->current_entry );
                done = ( self->current_entry == NULL );
                rc = ( done ? SILENT_RC( rcAlign, rcIterator, rcAccessing, rcOffset, rcDone ) : 0 );
            }
        }
    } while ( !done );
    return rc;
}


LIB_EXPORT rc_t CC PlacementSetIteratorNextIdAt ( PlacementSetIterator *self,
    INSDC_coord_zero pos, int64_t *row_id, INSDC_coord_len *len )
{
    const PlacementRecord *rec;
    rc_t rc = PlacementSetIteratorNextRecordAt ( self, pos, &rec );
    if ( rc == 0 )
    {
        if ( row_id != NULL ) *row_id = rec->id;
        if ( len != NULL ) *len = rec->len;
    }
    return rc;
}
