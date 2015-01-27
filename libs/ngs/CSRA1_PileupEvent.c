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

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

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
        return entry -> cell_data [ col_idx ];

    return CSRA1_PileupGetEntry ( CSRA1_PileupEventGetPileup ( self ), ctx, entry, col_idx );
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
        TRY ( MAPQ = CSRA1_PileupEventGetEntry ( self, ctx, self -> entry, pileup_event_col_MAPQ ) )
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

    TRY ( NGS_String * id = CSRA1_PileupEventGetAlignmentId ( self, ctx ) )
    {
        NGS_ReadCollection * coll = self -> dad . pileup -> ref -> coll;

        TRY ( const char * idz = NGS_StringData ( id, ctx ) )
        {
            struct NGS_Alignment * alignment;
            TRY ( alignment = NGS_ReadCollectionGetAlignment ( coll, ctx, idz ) )
            {
                NGS_StringRelease ( id, ctx );
                return alignment;
            }
        }

        NGS_StringRelease ( id, ctx );
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
    TRY ( CHECK_STATE ( self, ctx ) )
    {
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
    }

    return 0;
}

static
char CSRA1_PileupEventGetAlignmentBase ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        UNIMPLEMENTED();
    }
    return 0;
}

static
char CSRA1_PileupEventGetAlignmentQuality ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        UNIMPLEMENTED();
    }
    return 0;
}

static
struct NGS_String * CSRA1_PileupEventGetInsertionBases ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        UNIMPLEMENTED();
    }
    return NULL;
}

static
struct NGS_String * CSRA1_PileupEventGetInsertionQualities ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        UNIMPLEMENTED();
    }
    return NULL;
}

static
unsigned int CSRA1_PileupEventGetRepeatCount ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        UNIMPLEMENTED();
    }
    return 0;
}

static
int CSRA1_PileupEventGetIndelType ( const CSRA1_PileupEvent * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    TRY ( CHECK_STATE ( self, ctx ) )
    {
        UNIMPLEMENTED();
    }
    return 0;
}

static
bool CSRA1_PileupEventEntryInit ( CSRA1_PileupEvent * self, ctx_t ctx, CSRA1_Pileup_Entry * entry )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );

    TRY ( CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_HAS_MISMATCH ) )
    {
        const int32_t * REF_OFFSET;

        /* TBD - scan for right soft-clip, represented by
           string of mismatches at the end of of sequence */
#if 0
#warning need to add parameter for tracking right-clip length, or otherwise adjust
#endif

        TRY ( REF_OFFSET = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_REF_OFFSET ) )
        {
            const bool * HAS_REF_OFFSET;

            /* if there are no offsets, then there are no indels, which means
               that there are only match and mismatch events */
            if ( entry -> cell_len [ pileup_event_col_REF_OFFSET ] == 0 )
                return true;

            /* get HAS_REF_OFFSET */
            TRY ( HAS_REF_OFFSET = CSRA1_PileupEventGetEntry ( self, ctx, entry, pileup_event_col_HAS_REF_OFFSET ) )
            {
                /* check for soft-clip */
                if ( HAS_REF_OFFSET [ 0 ] )
                {
                    /* TBD - mark bad data if  */
                    assert ( REF_OFFSET [ 0 ] < 0 );
                    entry -> ref_off_idx = 1;

                    /* skip over soft-clip */
                    entry -> mismatch_idx = entry -> seq_idx = - REF_OFFSET [ 0 ];
                }

                return true;
            }
        }
    }

    self -> entry = NULL;
    return false;
}

static
bool CSRA1_PileupEventEntryFocus ( CSRA1_PileupEvent * self, ctx_t ctx, CSRA1_Pileup_Entry * entry )
{
    uint32_t prior_seq_idx;
    const int32_t * REF_OFFSET;
    const bool * HAS_REF_OFFSET, * HAS_MISMATCH;

    /* detect being within a deletion event */
    if ( entry -> indel_cnt > 0 )
    {
        if ( -- entry -> indel_cnt > 0 )
            return true;
    }

    /* advance within sequence */
    prior_seq_idx = entry -> seq_idx ++;
    entry -> indel_cnt = 0;
    entry -> mismatch = 0;

    /* adjust mismatch_idx */
    HAS_MISMATCH = entry -> cell_data [ pileup_event_col_HAS_MISMATCH ];
    assert ( HAS_MISMATCH != NULL );
    entry -> mismatch_idx += HAS_MISMATCH [ prior_seq_idx ];

    /* for the case where there are no indels */
    HAS_REF_OFFSET = entry -> cell_data [ pileup_event_col_HAS_REF_OFFSET ];
    if ( HAS_REF_OFFSET == NULL )
        return true;

    /* adjust ref_off_idx */
    entry -> ref_off_idx += HAS_REF_OFFSET [ prior_seq_idx ];

    /* test REF_OFFSET for current event */
    if ( ! HAS_REF_OFFSET [ entry -> seq_idx ] )
        return true;

    REF_OFFSET = entry -> cell_data [ pileup_event_col_REF_OFFSET ];
    assert ( REF_OFFSET != NULL );

    /* extract value */
    entry -> indel_cnt = REF_OFFSET [ entry -> ref_off_idx ];

    /* a positive offset is a deletion */
    if ( entry -> indel_cnt > 0 )
        return true;

    /* a negative offset is an insertion */
    assert ( entry -> indel_cnt < 0 );

#if 0
#warning here
#endif

    return true;
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
        return CSRA1_PileupEventEntryInit ( self, ctx, entry );

    /* this is an entry we've seen before */
    return CSRA1_PileupEventEntryFocus ( self, ctx, entry );
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

