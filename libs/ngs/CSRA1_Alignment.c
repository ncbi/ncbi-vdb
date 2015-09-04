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

#include "CSRA1_Alignment.h"

typedef struct CSRA1_Alignment CSRA1_Alignment;
#define NGS_ALIGNMENT CSRA1_Alignment

#include "NGS_Alignment.h"
#include "NGS_ReadCollection.h"
#include "NGS_Refcount.h"
#include "NGS_Read.h"

#include "NGS_Id.h"
#include "NGS_String.h"
#include "NGS_Cursor.h"

#include "CSRA1_ReadCollection.h"

#include <sysalloc.h>

#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>

#include <klib/printf.h>
#include <klib/rc.h>

#include <kfc/ctx.h>
#include <kfc/rsrc.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <limits.h>

#ifndef min
#   define min(a,b) ( (a) < (b) ? (a) : (b) )            
#endif

/*--------------------------------------------------------------------------
 * CSRA1_Alignment
 */

struct CSRA1_Alignment
{
    NGS_Refcount dad;   
    struct CSRA1_ReadCollection * coll;
    const NGS_String * run_name;
    
    int64_t cur_row;
    int64_t row_max;
    
    const NGS_Cursor * primary_curs;
    const NGS_Cursor * secondary_curs;
    NGS_String * col_data [ align_NUM_COLS ];
    
	uint64_t id_offset;
    
    bool seen_first;
	bool in_primary;
    
    /* for use in slices */ 
	int64_t secondary_start;
	int64_t secondary_max;
};

#define GetCursor( self ) ( self -> in_primary ? self -> primary_curs : self -> secondary_curs )

/* Whack
 */
static
void CSRA1_AlignmentWhack ( CSRA1_Alignment * self, ctx_t ctx )
{
    uint32_t i;
    for ( i = 0; i < align_NUM_COLS; ++ i )
    {
        NGS_StringRelease ( self -> col_data [ i ], ctx );
        self -> col_data [ i ] = NULL;
    }

    NGS_CursorRelease ( self -> primary_curs, ctx );
    NGS_CursorRelease ( self -> secondary_curs, ctx );

    NGS_StringRelease ( self -> run_name, ctx );
    CSRA1_ReadCollectionRelease ( self -> coll, ctx );
}

static
void CSRA1_AlignmentInitRegion ( CSRA1_Alignment * self, 
                                 ctx_t ctx, 
                                 const NGS_Cursor * primary, 
                                 const NGS_Cursor * secondary, 
                                 int64_t start, 
                                 uint64_t count )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
/*printf("CSRA1_AlignmentInitRegion(primary=%p, secondary=%p, start=%ld, count=%lu, offset=%lu)\n", 
        (void*)primary, (void*)secondary, start, count, self -> id_offset);    */

    /* split the requested region across primary/secondary table, adjust the boundaries as necessary */
    if ( ! FAILED () )
    {
        if ( primary != NULL )
        {
            int64_t primary_start;
            uint64_t primary_count;
            TRY ( NGS_CursorGetRowRange ( primary, ctx, & primary_start, & primary_count ) )
            {
                uint64_t table_end;
                
                if ( start < primary_start ) 
                {
                    count -= ( primary_start - start );
                    start = primary_start;
                }
                
                table_end = primary_start + primary_count;
                
                if ( start < (int64_t)table_end )
                {
                    self -> cur_row = start;
                    self -> row_max = min ( table_end, (uint64_t)start + min ( count, primary_count ) );
                
                    if ( self -> row_max == table_end ) 
                    {   /* a part of the range is in the secondary; reduce the count by the number or records from primary */
                        count -= (uint64_t) ( (int64_t)self -> row_max - self -> cur_row );
                        start = 1; /* this will be the starting rowId in the secondary */
                    }
                }
                else
                {   /* the entire range is beyond the primary cursor;
                       set up so that the first call to Next() will go into the secondary cursor */
                    self -> cur_row = self -> row_max = table_end;
                    start -= self -> id_offset; /* this will be the starting rowId in the secondary */
                    self -> in_primary = false;
                }
            }
        }
        else 
        {   /* primary not requested */
            if ( start <= (int64_t)self -> id_offset )
            {   /* range overlaps the primary ID space; adjust the range to exclude primary */
                count -= ( self -> id_offset - start + 1 );
                start = 1;
            }
            else
            {
                start -= self -> id_offset; /* this will be the starting rowId in the secondary */
            }
            /* make sure the first call to Next() will switch to the secondary cursor */
            self -> cur_row = self -> row_max = self -> id_offset + 1; 
            self -> in_primary = false;
        }
        
        if ( ! FAILED () && secondary != NULL )
        {
            int64_t  secondary_start;
            uint64_t secondary_count;
            TRY ( NGS_CursorGetRowRange ( secondary, ctx, & secondary_start, & secondary_count ) )
            {
                uint64_t table_end;
                if ( start < secondary_start ) 
                {
                    count -= ( secondary_start - start );
                    start = secondary_start;
                }

                table_end = secondary_start + secondary_count;
                
                if ( start < (int64_t)table_end )
                {
                    self -> secondary_start = start;
                    self -> secondary_max = min ( table_end, (uint64_t)start + min ( count, secondary_count ) );
                }
                else
                {
                    self -> secondary_start = self -> secondary_max = table_end;
                }
            }
            if ( ! self -> in_primary )
            {
                self -> cur_row = self -> secondary_start;
                self -> row_max = self -> secondary_max; 
            }
        }
/*    printf("cur_row=%ld, row_max=%lu, secondary_start=%ld, secondary_max=%lu)\n", 
            self->cur_row, self->row_max, self -> secondary_start, self -> secondary_max);    */
    }
}

NGS_String * CSRA1_AlignmentGetAlignmentId( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcAccessing );
    
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }
    
    /* if current row is valid, read data */
    if ( self -> cur_row < self -> row_max )
    {
        if ( self -> in_primary )
        {
            return NGS_IdMake ( ctx, self -> run_name, NGSObject_PrimaryAlignment, self -> cur_row );
        }
        else
        {
            return NGS_IdMake ( ctx, self -> run_name, NGSObject_SecondaryAlignment, self -> cur_row + self -> id_offset );
        }
    }
    
    USER_ERROR ( xcCursorExhausted, "No more rows available" );
    return NULL;
}

struct NGS_String* CSRA1_AlignmentGetReferenceSpec( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }
    
    return NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, align_REF_NAME );
}

int CSRA1_AlignmentGetMappingQuality( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return 0;
    }
    return NGS_CursorGetInt32 ( GetCursor ( self ), ctx, self -> cur_row, align_MAPQ );
}

struct NGS_String* CSRA1_AlignmentGetReferenceBases( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }
    return NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, align_REF_READ );
}

struct NGS_String* CSRA1_AlignmentGetReadGroup( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }
    else
    {
        TRY ( NGS_String* ret = NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, align_SPOT_GROUP ) )
        {
            return ret;
        }
        CATCH_ALL ()
        {
            CLEAR();
        }
        return NGS_StringMake ( ctx, "", 0 );
    }
}

NGS_String * CSRA1_AlignmentGetReadId( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return 0;
    }

    {
        TRY ( int64_t readId = NGS_CursorGetInt64 ( GetCursor ( self ), ctx, self -> cur_row, align_SEQ_SPOT_ID ) )
        {
            return NGS_IdMake ( ctx, self -> run_name, NGSObject_Read, readId );
        }
    }
    return NULL;
}

struct NGS_String* CSRA1_AlignmentGetClippedFragmentBases( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }

    return NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, align_CLIPPED_READ );
}

struct NGS_String* CSRA1_AlignmentGetClippedFragmentQualities( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }

    {
        NGS_String* phred = NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, align_CLIPPED_QUALITY );
        /* convert to ascii-33 */
        size_t size = NGS_StringSize ( phred, ctx ); 
        char * copy = malloc ( size + 1 );
        if ( copy == NULL )
        {
            SYSTEM_ERROR ( xcNoMemory, 
                           "allocating %u bytes for %s row %ld", 
                           size + 1, "CLIPPED_QUALITY", self -> cur_row );
            NGS_StringRelease ( phred, ctx );
            return NULL;
        }
        else
        {
            NGS_String* ret;
            const char* orig = NGS_StringData ( phred, ctx );
            size_t i;
            for ( i = 0; i < size ; ++ i )
                copy [ i ] = ( char ) ( (uint8_t)(orig [ i ]) + 33 );
            copy [ i ] = 0;

            ret = NGS_StringMakeOwned ( ctx, copy, size );
            if ( FAILED () )
                free ( copy );
            NGS_StringRelease ( phred, ctx );
            return ret;
        }
    }
}

struct NGS_String* CSRA1_AlignmentGetAlignedFragmentBases( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }

    return NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, align_READ );
}

bool CSRA1_AlignmentIsPrimary( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
	
    assert ( self );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return false;
    }

    return self -> in_primary;
}

int64_t CSRA1_AlignmentGetAlignmentPosition( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return 0;
    }

    return NGS_CursorGetInt32 ( GetCursor ( self ), ctx, self -> cur_row, align_REF_POS);
}

uint64_t CSRA1_AlignmentGetAlignmentLength( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return 0;
    }

    return NGS_CursorGetInt32 ( GetCursor ( self ), ctx, self -> cur_row, align_REF_LEN);
}

bool CSRA1_AlignmentGetIsReversedOrientation( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return false;
    }

    return NGS_CursorGetBool ( GetCursor ( self ), ctx, self -> cur_row, align_REF_ORIENTATION);
}

int CSRA1_AlignmentGetSoftClip( CSRA1_Alignment* self, ctx_t ctx, bool left )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return 0;
    }

    return NGS_CursorGetInt32 ( GetCursor ( self ), ctx, self -> cur_row, left ? align_LEFT_SOFT_CLIP : align_RIGHT_SOFT_CLIP );
}

uint64_t CSRA1_AlignmentGetTemplateLength( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return 0;
    }

    return NGS_CursorGetInt32 ( GetCursor ( self ), ctx, self -> cur_row, align_TEMPLATE_LEN);
}

struct NGS_String* CSRA1_AlignmentGetShortCigar( CSRA1_Alignment* self, ctx_t ctx, bool clipped )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }

    return NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, clipped ? align_CLIPPED_CIGAR_SHORT: align_CIGAR_SHORT );
}

struct NGS_String* CSRA1_AlignmentGetLongCigar( CSRA1_Alignment* self, ctx_t ctx, bool clipped )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }

    return NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, clipped ? align_CLIPPED_CIGAR_LONG : align_CIGAR_LONG );
}

char CSRA1_AlignmentGetRNAOrientation( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
    }
    else
    {
        TRY ( char ret = NGS_CursorGetChar ( GetCursor ( self ), ctx, self -> cur_row, align_RNA_ORIENTATION ) )
        {
            return ret;
        }
        CATCH_ALL ()
        {
            CLEAR();
        }
    }
    return '?';
}

bool CSRA1_AlignmentHasMate( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_WARNING ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return false;
    }

    TRY ( NGS_CursorGetInt64 ( GetCursor ( self ), ctx, self -> cur_row, align_MATE_ALIGN_ID ) )
    {
        return true;
    }
    CATCH_ALL ()
    {
        CLEAR();
    }
    return false;
}

NGS_String * CSRA1_AlignmentGetMateAlignmentId( CSRA1_Alignment* self, ctx_t ctx )
{
    int64_t mateId;
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return 0;
    }

    TRY ( mateId = NGS_CursorGetInt64 ( GetCursor ( self ), ctx, self -> cur_row, align_MATE_ALIGN_ID ) )
    {
        return NGS_IdMake ( ctx, 
                            self -> run_name, 
                            self -> in_primary ? NGSObject_PrimaryAlignment : NGSObject_SecondaryAlignment, 
                            mateId /* + self -> id_offset ? */);
    }
    return NULL;
}

struct NGS_String* CSRA1_AlignmentGetMateReferenceSpec( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }

    return NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, align_MATE_REF_NAME);
}

bool CSRA1_AlignmentGetMateIsReversedOrientation( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return false;
    }

    return NGS_CursorGetBool ( GetCursor ( self ), ctx, self -> cur_row, align_MATE_REF_ORIENTATION);
}

/*--------------------------------------------------------------------------
 * NGS_AlignmentIterator
 */
static
bool CSRA1_AlignmentIteratorNext ( CSRA1_Alignment* self, ctx_t ctx )
{
    assert ( self != NULL );
    
    if ( !self -> seen_first )
    {
        self -> seen_first = true;
    }
    else
    {
        ++ self -> cur_row;
    }
    
    if ( self -> cur_row < self -> row_max )
    {
        return true;
    }

    /* see if we need to switch over to the next cursor */ 
    if ( self -> in_primary && self -> secondary_curs != NULL )
    {
        self -> in_primary = false;

        self -> cur_row = self -> secondary_start;
        self -> row_max = self -> secondary_max;
        
        return self -> cur_row < self -> row_max;
    }
    
    return false;
}

static CSRA1_Alignment* CSRA1_AlignmentGetMateAlignment( CSRA1_Alignment* self, ctx_t ctx );

static 
NGS_String* CSRA1_FragmentGetId ( CSRA1_Alignment * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    assert ( self != NULL );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }
    
    {
        TRY ( int32_t fragId = NGS_CursorGetInt32 ( GetCursor ( self ), ctx, self -> cur_row, align_SEQ_READ_ID ) )
        {
            return NGS_IdMakeFragment ( ctx, self -> run_name, true, self -> cur_row, fragId - 1 );
        }
    }
    return NULL;
}

static 
struct NGS_String * CSRA1_FragmentGetSequence ( CSRA1_Alignment * self, ctx_t ctx, uint64_t offset, uint64_t length )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    NGS_String * seq;
    
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }

    TRY ( seq = NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, align_RAW_READ ) )
    {
        TRY ( NGS_String * sub = NGS_StringSubstrOffsetSize ( seq, ctx, offset, length ) )
        {
            NGS_StringRelease ( seq, ctx );
            seq = sub;
        }
    }
    
    return seq;
}

static 
struct NGS_String * CSRA1_FragmentGetQualities ( CSRA1_Alignment * self, ctx_t ctx, uint64_t offset, uint64_t length )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    NGS_String * seq;
    
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return NULL;
    }

    TRY ( seq = NGS_CursorGetString ( GetCursor ( self ), ctx, self -> cur_row, align_SAM_QUALITY ) )
    {
        TRY ( NGS_String * sub = NGS_StringSubstrOffsetSize ( seq, ctx, offset, length ) )
        {
            NGS_StringRelease ( seq, ctx );
            seq = sub;
        }
    }
    
    return seq;
}

static
bool CSRA1_FragmentIsPaired ( CSRA1_Alignment * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    int64_t id;
    int32_t idx;
    bool ret = false;

    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return false;
    }

    TRY ( id = NGS_CursorGetInt64 ( GetCursor ( self ), ctx, self -> cur_row, align_MATE_ALIGN_ID ) )
    {
    }
    CATCH_ALL ()
    {
        /* if we failed, it means that MATE_ALIGN_ID column is empty, so lets assign it to 0 and move forward */
        CLEAR();
        id = 0;
    }
    
    /* if MATE_ALIGN_ID != 0, it's paired */
    if ( id != 0 )
        return true;
    
    TRY ( idx = NGS_CursorGetInt32 ( GetCursor ( self ), ctx, self -> cur_row, align_SEQ_READ_ID ) )
    {
        /* if SEQ_READ_ID > 1, it's paired. */
        if ( idx > 1 )
            return true;
        
        TRY ( id = NGS_CursorGetInt64 ( GetCursor ( self ), ctx, self -> cur_row, align_SEQ_SPOT_ID ) )
        {
            NGS_String * readId;
            
            /* otherwise, have to get spot id and consult SEQUENCE table */
            TRY ( readId = NGS_IdMake ( ctx, self -> run_name, NGSObject_Read, id ) )
            {
                const char * readIdStr = NGS_StringData ( readId, ctx );
                TRY ( NGS_Read * read = NGS_ReadCollectionGetRead ( ( NGS_ReadCollection * ) self -> coll, ctx, readIdStr ) )
                {
                    uint32_t numFragments = NGS_ReadNumFragments(read, ctx);
                    ret = numFragments > 1;
                    
                    NGS_ReadRelease ( read, ctx );
                }
                
                NGS_StringRelease ( readId, ctx );
            }
        }
    }

    return ret;
}

static
bool CSRA1_FragmentIsAligned ( CSRA1_Alignment * self, ctx_t ctx )
{
    assert ( self != NULL );
    return true;
}

static 
bool CSRA1_FragmentNext ( CSRA1_Alignment * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    if ( ! self -> seen_first ) 
    {
        USER_ERROR ( xcIteratorUninitialized, "Alignment accessed before a call to AlignmentIteratorNext()" );
        return false;
    }

    UNIMPLEMENTED(); /* CSRA1_FragmentNext; should not be called - Alignment is not a FragmentIterator */
    
    return false;
}


static NGS_Alignment_vt CSRA1_Alignment_vt_inst =
{
    {
        {
            /* NGS_Refcount */
            CSRA1_AlignmentWhack
        },

        /* NGS_Fragment */
        CSRA1_FragmentGetId,
        CSRA1_FragmentGetSequence,
        CSRA1_FragmentGetQualities,
        CSRA1_FragmentIsPaired,
        CSRA1_FragmentIsAligned,
        CSRA1_FragmentNext
    },
    
    CSRA1_AlignmentGetAlignmentId,
    CSRA1_AlignmentGetReferenceSpec,
    CSRA1_AlignmentGetMappingQuality,
    CSRA1_AlignmentGetReferenceBases,
    CSRA1_AlignmentGetReadGroup,
    CSRA1_AlignmentGetReadId,
    CSRA1_AlignmentGetClippedFragmentBases,
    CSRA1_AlignmentGetClippedFragmentQualities,
    CSRA1_AlignmentGetAlignedFragmentBases,
    CSRA1_AlignmentIsPrimary,
    CSRA1_AlignmentGetAlignmentPosition,
    CSRA1_AlignmentGetAlignmentLength,
    CSRA1_AlignmentGetIsReversedOrientation,
    CSRA1_AlignmentGetSoftClip,
    CSRA1_AlignmentGetTemplateLength,
    CSRA1_AlignmentGetShortCigar,
    CSRA1_AlignmentGetLongCigar,
    CSRA1_AlignmentGetRNAOrientation,
    CSRA1_AlignmentHasMate,
    CSRA1_AlignmentGetMateAlignmentId,
    CSRA1_AlignmentGetMateAlignment,
    CSRA1_AlignmentGetMateReferenceSpec,
    CSRA1_AlignmentGetMateIsReversedOrientation,

    /* Iterator */
    CSRA1_AlignmentIteratorNext
};

/* Init
 */
static
void CSRA1_AlignmentInit ( NGS_ALIGNMENT* ref, 
                           ctx_t ctx, 
                           struct CSRA1_ReadCollection * coll,
                           const char *clsname, 
                           const char *instname, 
                           const NGS_String * run_name,
                           bool exclusive,
                           bool primary, 
                           bool secondary,
                           uint64_t id_offset )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    if ( ref == NULL )
        INTERNAL_ERROR ( xcParamNull, "bad object reference" );
    else
    {
        TRY ( NGS_AlignmentInit ( ctx, ref, & CSRA1_Alignment_vt_inst, clsname, instname ) )
        {
            if ( primary  )
            {
                ON_FAIL ( ref -> primary_curs = CSRA1_ReadCollectionMakeAlignmentCursor ( coll, 
                                                                                          ctx, 
                                                                                          true, 
                                                                                          exclusive ) )
                    return;
                ref -> in_primary   = true;
            }
            
            if ( secondary )
            {
                ON_FAIL ( ref -> secondary_curs = CSRA1_ReadCollectionMakeAlignmentCursor ( coll, 
                                                                                            ctx, 
                                                                                            false, 
                                                                                            exclusive ) )
                    CLEAR(); /* missing SECONDARY_ALIGNMENTS table is OK */
            }
            
            ref -> id_offset = id_offset;
            
            ON_FAIL ( ref -> coll = CSRA1_ReadCollectionDuplicate ( coll, ctx ) )
                return;
            ON_FAIL ( ref -> run_name = NGS_StringDuplicate ( run_name, ctx ) )
                return;
        }
    }
}


static
void SetRowId ( CSRA1_Alignment* self, ctx_t ctx, int64_t rowId, bool primary )
{   /* validate the requested rowId */
    if ( rowId <= 0 )
    {
        INTERNAL_ERROR ( xcCursorAccessFailed, 
                         "rowId ( %li ) out of range for %.*s", 
                         rowId, 
                         NGS_StringSize ( self -> run_name, ctx ), 
                         NGS_StringData ( self -> run_name, ctx ) );
    }
    else 
    {
        int64_t id = rowId;
        int64_t  start = 0;
        uint64_t count = 0;
    
        if ( primary )
        {
            if ( self -> primary_curs != NULL )
                ON_FAIL ( NGS_CursorGetRowRange ( self -> primary_curs, ctx, & start, & count ) )
                    return;
        }
        else if ( self -> secondary_curs != NULL )
        {
            ON_FAIL ( NGS_CursorGetRowRange ( self -> secondary_curs, ctx, & start, & count ) )
                return;
            id -= self -> id_offset;
        }
        
        if ( (uint64_t)id >= start + count )
        {
            INTERNAL_ERROR ( xcCursorAccessFailed, 
                             "rowId ( %li ) out of range for %.*s", 
                             rowId, 
                             NGS_StringSize ( self -> run_name, ctx ), 
                             NGS_StringData ( self -> run_name, ctx ) );
        }                
        else
        {
            self -> cur_row = id;
            self -> row_max = id + 1;
        }
    }
}

/* Make
 *  makes a common alignment from VCursor
 */
NGS_Alignment * CSRA1_AlignmentMake ( ctx_t ctx, 
                                        struct CSRA1_ReadCollection * coll,
                                        int64_t alignId, 
                                        const NGS_String * run_name, 
                                        bool primary,
                                        uint64_t id_offset )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );

    CSRA1_Alignment * ref;
    
    ref = calloc ( 1, sizeof * ref );
    if ( ref == NULL )
        SYSTEM_ERROR ( xcNoMemory, 
                       "allocating CSRA1_Alignment(%lu) on '%.*s'", 
                       alignId, 
                       NGS_StringSize ( run_name, ctx ), 
                       NGS_StringData ( run_name, ctx ) );
    else
    {
#if _DEBUGGING
        char instname [ 256 ];
        string_printf ( instname, 
                        sizeof instname, 
                        NULL, 
                        "%.*s(%lu)", 
                        NGS_StringSize ( run_name, ctx ), 
                        NGS_StringData ( run_name, ctx ), 
                        alignId );
        instname [ sizeof instname - 1 ] = 0;
#else
        const char *instname = "";
#endif
        TRY ( CSRA1_AlignmentInit ( ref, ctx, coll, "CSRA1_Alignment", instname, run_name, false, primary, ! primary, id_offset ) )
        {
            TRY ( SetRowId( ref, ctx, alignId, primary ) )
            {
                ref -> seen_first = true;
                return ( NGS_Alignment * ) ref;
            }
            CSRA1_AlignmentWhack ( ref, ctx );
        }
        free ( ref );
    }

    return NULL;
}

CSRA1_Alignment* CSRA1_AlignmentGetMateAlignment( CSRA1_Alignment* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    TRY ( int64_t mate_row = NGS_CursorGetInt64 ( GetCursor ( self ), ctx, self -> cur_row, align_MATE_ALIGN_ID ) )
    {
        if ( ! self -> in_primary )
        {
            mate_row += self -> id_offset;
        }
        
        {
            TRY ( NGS_String * mate_id = NGS_IdMake( ctx, 
                                                     self -> run_name, 
                                                     self -> in_primary ? NGSObject_PrimaryAlignment : NGSObject_SecondaryAlignment, 
                                                     mate_row ) )
            {
                CSRA1_Alignment* ret = (CSRA1_Alignment*) 
                    NGS_ReadCollectionGetAlignment ( CSRA1_ReadCollectionToNGS_ReadCollection ( self -> coll, ctx ), 
                                                     ctx, 
                                                     NGS_StringData ( mate_id, ctx ) );
                NGS_StringRelease (mate_id, ctx );
                return ret;
            }
        }
    }
    return NULL;
}

NGS_Alignment * CSRA1_AlignmentIteratorMake ( ctx_t ctx, 
                                              struct CSRA1_ReadCollection * coll,
                                              bool primary, 
                                              bool secondary, 
                                              const NGS_String * run_name, 
                                              uint64_t id_offset )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );

    CSRA1_Alignment * ref;

    ref = calloc ( 1, sizeof * ref );
    if ( ref == NULL )
        SYSTEM_ERROR ( xcNoMemory, 
                       "allocating NGS_AlignmentIterator on '%.*s'", 
                       NGS_StringSize ( run_name, ctx ), 
                       NGS_StringData ( run_name, ctx ) );
    else
    {
#if _DEBUGGING
        char instname [ 256 ];
        string_printf ( instname, 
                        sizeof instname, 
                        NULL, 
                        "%.*s", 
                        NGS_StringSize ( run_name, ctx ), 
                        NGS_StringData ( run_name, ctx ) );
        instname [ sizeof instname - 1 ] = 0;
#else
        const char *instname = "";
#endif
        TRY ( CSRA1_AlignmentInit ( ref, ctx, coll, "NGS_AlignmentIterator", instname, run_name, true, primary, secondary, id_offset ) )
        {
            TRY ( CSRA1_AlignmentInitRegion ( ref, ctx, ref -> primary_curs, ref -> secondary_curs, 0, ULLONG_MAX ) )
            {
                return ( NGS_Alignment * ) ref;
            }
            CSRA1_AlignmentWhack ( ref, ctx );
        }

        free ( ref );
    }

    return NULL;
}

NGS_Alignment * CSRA1_AlignmentRangeMake ( ctx_t ctx, 
                                           struct CSRA1_ReadCollection * coll,
                                           bool primary, 
                                           bool secondary, 
                                           const NGS_String * run_name, 
                                           uint64_t id_offset,
                                           int64_t first, 
                                           uint64_t count)
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );

    CSRA1_Alignment * ref;

    ref = calloc ( 1, sizeof * ref );
    if ( ref == NULL )
        SYSTEM_ERROR ( xcNoMemory, 
                       "allocating NGS_AlignmentRange on '%.*s'", 
                       NGS_StringSize ( run_name, ctx ), 
                       NGS_StringData ( run_name, ctx ) );
    else
    {
#if _DEBUGGING
        char instname [ 256 ];
        string_printf ( instname, 
                        sizeof instname, 
                        NULL, 
                        "%.*s", 
                        NGS_StringSize ( run_name, ctx ), 
                        NGS_StringData ( run_name, ctx ) );
        instname [ sizeof instname - 1 ] = 0;
#else
        const char *instname = "";
#endif
        TRY ( CSRA1_AlignmentInit ( ref, ctx, coll, "NGS_AlignmentRange", instname, run_name, true, primary, secondary, id_offset ) )
        {
            TRY ( CSRA1_AlignmentInitRegion ( ref, ctx, ref -> primary_curs, ref -> secondary_curs, first, count ) )
            {
                return ( NGS_Alignment * ) ref;
            }
            CSRA1_AlignmentWhack ( ref, ctx );
        }

        free ( ref );
    }

    return NULL;
}

