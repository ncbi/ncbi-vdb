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

#include "CSRA1_ReferenceWindow.h"

typedef struct CSRA1_ReferenceWindow CSRA1_ReferenceWindow;
#define NGS_ALIGNMENT CSRA1_ReferenceWindow

#include "NGS_Alignment.h"
#include "NGS_ReadCollection.h"
#include "NGS_Refcount.h"

#include "NGS_Cursor.h"
#include "NGS_String.h"
#include "NGS_Id.h"

#include "CSRA1_Reference.h"
#include "CSRA1_Alignment.h"

#include <sysalloc.h>

#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>

#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/sort.h>

#include <kfc/ctx.h>
#include <kfc/rsrc.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <limits.h>

#ifndef min
#   define min(a,b) ( (a) < (b) ? (a) : (b) )            
#endif

/*--------------------------------------------------------------------------
 * CSRA1_ReferenceWindow
 */
enum
{
    Primary     = 0,
    Secondary   = 1
};
struct AlignmentInfo
{
    int64_t id; 
                        /* sort order */
    int64_t     pos;    /* asc */
    uint64_t    len;    /* desc */
    int8_t      cat;    /* prim, sec */
    int32_t     mapq;   /* desc */
};
typedef struct AlignmentInfo AlignmentInfo;

struct CSRA1_ReferenceWindow
{
    NGS_Refcount dad;   
    NGS_ReadCollection * coll;
    
    const NGS_Cursor * reference_curs;
    
    bool circular;
    bool primary;
    bool secondary;
    bool within_window; /* if true, exclude alignments that start before slice_offset */
    
    uint32_t chunk_size;
    uint64_t ref_length; /* total reference length in bases */
    uint64_t id_offset;
    
    /* remaining range of chunks in the reference table */
    int64_t ref_begin;
    int64_t ref_end;
    
    /* for use in a slice iterator: */
    /* slice (0, 0) = all */
    uint64_t slice_offset;
    uint64_t slice_size; /* 0 = the rest of the reference */
    /* starting chunks for primary/secondary tables */
    int64_t ref_primary_begin;
    int64_t ref_secondary_begin;
    
    /* false - not positioned on any chunk */
    bool seen_first;
    
    /* alignments against current chunk, sorted in canonical order */
    AlignmentInfo* align_info; 
    size_t align_info_cur;
    size_t align_info_total;
    NGS_Alignment* cur_align; /* cached current alignment, corresponds to align_info_cur */
};


/* Whack
 */
static
void CSRA1_ReferenceWindowWhack ( CSRA1_ReferenceWindow * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcDestroying );
    
    NGS_AlignmentRelease ( self -> cur_align, ctx );
    free ( self -> align_info );
    NGS_CursorRelease ( self -> reference_curs, ctx );
    NGS_RefcountRelease ( & self -> coll -> dad, ctx );
}

static
NGS_Alignment* GetAlignment ( CSRA1_ReferenceWindow* self, ctx_t ctx )
{    
    if ( self -> seen_first && 
         ( self -> circular || self -> ref_begin < self ->ref_end ) && /* for circular references, all chunks are loaded at once */
         self -> align_info_cur < self -> align_info_total )
    {   
        if ( self -> cur_align == NULL )
        {
            TRY ( NGS_String * run_name = NGS_ReadCollectionGetName ( self -> coll, ctx ) )
            {
                TRY ( const NGS_String * id = NGS_IdMake ( ctx, 
                                                           run_name, 
                                                           self -> align_info [ self -> align_info_cur ] . cat == Primary ? 
                                                            NGSObject_PrimaryAlignment:
                                                            NGSObject_SecondaryAlignment, 
                                                           self -> align_info [ self -> align_info_cur ] . id /* + self -> id_offset ? */) )
                {
                    self -> cur_align = NGS_ReadCollectionGetAlignment ( self -> coll, ctx, NGS_StringData ( id, ctx ) );
                    NGS_StringRelease ( id, ctx );
                }
                NGS_StringRelease ( run_name, ctx );
            }
        }
        return self -> cur_align;
    }
    USER_ERROR ( xcIteratorUninitialized, "Invalid alignment" );
    return NULL;
}


static 
NGS_String* CSRA1_FragmentGetId ( CSRA1_ReferenceWindow * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_FragmentGetId ( (NGS_Fragment*)ref, ctx );
    }
    return NULL;
}

static 
struct NGS_String * CSRA1_FragmentGetSequence ( CSRA1_ReferenceWindow * self, ctx_t ctx, uint64_t offset, uint64_t length )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_FragmentGetSequence ( (NGS_Fragment*)ref, ctx, offset, length );
    }
    return NULL;
}

static 
struct NGS_String * CSRA1_FragmentGetQualities ( CSRA1_ReferenceWindow * self, ctx_t ctx, uint64_t offset, uint64_t length )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_FragmentGetQualities ( (NGS_Fragment*)ref, ctx, offset, length );
    }
    return NULL;
}

static 
bool CSRA1_FragmentIsAligned ( const CSRA1_ReferenceWindow * self, ctx_t ctx )
{
    assert ( self != NULL );
    return true;
}

static 
bool CSRA1_FragmentNext ( CSRA1_ReferenceWindow * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    UNIMPLEMENTED(); /* CSRA1_FragmentNext; should not be called - Alignment is not a FragmentIterator */
    return false;
}

static
NGS_String * CSRA1_ReferenceWindowGetAlignmentId( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetAlignmentId ( ref, ctx );
    }
    return NULL;
}

static
struct NGS_String* CSRA1_ReferenceWindowGetReferenceSpec( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetReferenceSpec ( ref, ctx );
    }
    return NULL;
}

static
int CSRA1_ReferenceWindowGetMappingQuality( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetMappingQuality ( ref, ctx );
    }
    return 0;
}

static
struct NGS_String* CSRA1_ReferenceWindowGetReferenceBases( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetReferenceBases ( ref, ctx );
    }
    return NULL;
}

static
struct NGS_String* CSRA1_ReferenceWindowGetReadGroup( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetReadGroup ( ref, ctx );
    }
    return NULL;
}

static
NGS_String * CSRA1_ReferenceWindowGetReadId( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetReadId ( ref, ctx );
    }
    return NULL;
}

static
struct NGS_String* CSRA1_ReferenceWindowGetClippedFragmentBases( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetClippedFragmentBases ( ref, ctx );
    }
    return NULL;
}

static
struct NGS_String* CSRA1_ReferenceWindowGetClippedFragmentQualities( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetClippedFragmentQualities ( ref, ctx );
    }
    return NULL;
}

static
struct NGS_String* CSRA1_ReferenceWindowGetAlignedFragmentBases( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetAlignedFragmentBases ( ref, ctx );
    }
    return NULL;
}

static
bool CSRA1_ReferenceWindowIsPrimary( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentIsPrimary ( ref, ctx );
    }
    return false;
}

static
int64_t CSRA1_ReferenceWindowGetAlignmentPosition( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetAlignmentPosition ( ref, ctx );
    }
    return 0;
}

static
uint64_t CSRA1_ReferenceWindowGetAlignmentLength( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetAlignmentLength ( ref, ctx );
    }
    return 0;
}

static
bool CSRA1_ReferenceWindowGetIsReversedOrientation( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetIsReversedOrientation ( ref, ctx );
    }
    return false;
}

static
int CSRA1_ReferenceWindowGetSoftClip( CSRA1_ReferenceWindow* self, ctx_t ctx, bool left )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetSoftClip ( ref, ctx, left );
    }
    return 0;
}

static
uint64_t CSRA1_ReferenceWindowGetTemplateLength( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetTemplateLength ( ref, ctx );
    }
    return 0;
}

static
struct NGS_String* CSRA1_ReferenceWindowGetShortCigar( CSRA1_ReferenceWindow* self, ctx_t ctx, bool clipped )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetShortCigar ( ref, ctx, clipped );
    }
    return NULL;
}

static
struct NGS_String* CSRA1_ReferenceWindowGetLongCigar( CSRA1_ReferenceWindow* self, ctx_t ctx, bool clipped )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetLongCigar ( ref, ctx, clipped );
    }
    return NULL;
}

static
char CSRA1_ReferenceWindowGetRNAOrientation( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetRNAOrientation ( ref, ctx );
    }
    return false;
}

static
bool CSRA1_ReferenceWindowHasMate( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentHasMate ( ref, ctx );
    }
    CLEAR(); /* we do not want HasMate to ever throw, as a favor to C++/Java front ends */
    return false;
}

static
struct NGS_String* CSRA1_ReferenceWindowGetMateAlignmentId( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetMateAlignmentId ( ref, ctx );
    }
    return 0;
}

static
CSRA1_ReferenceWindow* CSRA1_ReferenceWindowGetMateAlignment( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return ( CSRA1_ReferenceWindow * ) NGS_AlignmentGetMateAlignment( ref, ctx );
    }
    return NULL;
}

static
struct NGS_String* CSRA1_ReferenceWindowGetMateReferenceSpec( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetMateReferenceSpec ( ref, ctx );
    }
    return NULL;
}

static
bool CSRA1_ReferenceWindowGetMateIsReversedOrientation( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_Alignment* ref = GetAlignment ( self, ctx ) )
    {
        return NGS_AlignmentGetMateIsReversedOrientation ( ref, ctx );
    }
    return false;
}

/*--------------------------------------------------------------------------
 * Iterator
 */
static
int64_t AlignmentSort ( const void * p_a, const void * p_b, void *data )
{
    const struct AlignmentInfo* a = ( const struct AlignmentInfo * ) p_a;
    const struct AlignmentInfo* b = ( const struct AlignmentInfo * ) p_b;
    
    if ( a -> pos < b -> pos )
        return -1;
    else if ( a -> pos > b -> pos )
        return 1;
    
    /* cannot use uint64_t - uint64_t because of possible overflow */
    if ( a -> len < b -> len ) return 1;
    if ( a -> len > b -> len ) return -1;
    
    if ( a -> cat != b -> cat )
        return (int64_t) a -> cat - (int64_t) b -> cat;
    
    /* sort by mapq in reverse order */
    if ( a -> mapq != b -> mapq )
        return (int64_t) b -> mapq - (int64_t) a -> mapq;

    /* use row id as the last resort, to make sorting more predictable */
    return a -> id < b -> id ? -1 : a -> id > b -> id;
}

static
void LoadAlignmentInfo ( CSRA1_ReferenceWindow* self, ctx_t ctx, size_t* idx, int64_t id, bool primary, int64_t offset, uint64_t size )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    TRY ( NGS_String * spec = NGS_StringMake ( ctx, "", 0 ) )
    {
        TRY ( NGS_Alignment* al = CSRA1_AlignmentMake ( ctx, 
                                                        ( struct CSRA1_ReadCollection * ) self -> coll, 
                                                        id, 
                                                        spec, 
                                                        primary, 
                                                        self -> id_offset ) )
        {
            int64_t pos = NGS_AlignmentGetAlignmentPosition ( al, ctx );
            int64_t len = (int64_t) NGS_AlignmentGetAlignmentLength ( al, ctx );
            
            if ( ! self -> within_window || pos >= offset )
            {
                bool overlaps = true;
                
                if ( size > 0 )
                {   /* a slice*/
                    int64_t end_slice =  offset + (int64_t)size;
                    if ( end_slice > self -> ref_length )
                    {
                        end_slice = self -> ref_length;
                    }
                    if ( self -> circular && pos + len >= self -> ref_length ) 
                    {   /* account for possible carryover on a circular reference */
                        pos -= self -> ref_length;
                    }
                    overlaps = pos < end_slice && ( pos + len > offset );                
                }
                
                if ( overlaps )
                {
        /*printf("%li, %li, %i, %li\n", pos, len, NGS_AlignmentGetMappingQuality ( al, ctx ), id);        */
                    self -> align_info [ *idx ] . id = id;
                    self -> align_info [ *idx ] . pos = pos;
                    self -> align_info [ *idx ] . len = len;
                    self -> align_info [ *idx ] . cat = primary ? Primary : Secondary;
                    self -> align_info [ *idx ] . mapq = NGS_AlignmentGetMappingQuality ( al, ctx );
                    ++ ( * idx );
                }
            }
            
            NGS_AlignmentRelease ( al, ctx );
        }
        NGS_StringRelease ( spec, ctx );
    }
}

static
void LoadAlignmentIndex ( CSRA1_ReferenceWindow* self, ctx_t ctx, int64_t row_id, uint32_t id_col_idx, const int64_t**  p_base, uint32_t* p_length )
{
    const void * base;
    uint32_t elem_bits, boff, row_len;
    TRY ( NGS_CursorCellDataDirect ( self -> reference_curs, 
                                     ctx,
                                     row_id, 
                                     id_col_idx, 
                                     & elem_bits, 
                                     & base, 
                                     & boff, 
                                     & row_len ) )
    {
        assert ( elem_bits == 64 );
        assert ( boff == 0 );
        *p_base = ( const int64_t* ) base;
        *p_length = row_len;
    }
}

static
int64_t AlignmentSortCircular ( const void * p_a, const void * p_b, void *data )
{
    const struct AlignmentInfo* a = ( const struct AlignmentInfo * ) p_a;
    const struct AlignmentInfo* b = ( const struct AlignmentInfo * ) p_b;
    
    uint64_t total = *(uint64_t*)data;
    int64_t a_start = a -> pos;
    int64_t b_start = b -> pos;
    if ( ( (uint64_t)a-> pos ) + a -> len > total )
    {
        a_start -= total;
    }
    if ( ( (uint64_t)b -> pos ) + b -> len > total )
    {
        b_start -= total;
    }
    
    if ( a_start < b_start )
        return -1;
    else if ( a_start > b_start )
        return 1;
    
    /* cannot use uint64_t - uint64_t because of possible overflow */
    if ( a -> len < b -> len ) return 1;
    if ( a -> len > b -> len ) return -1;

    if ( a -> cat != b -> cat )
        return (int64_t) a -> cat - (int64_t) b -> cat;
    
    /* sort by mapq in reverse order */
    if ( a -> mapq != b -> mapq )
        return (int64_t) b -> mapq - (int64_t) a -> mapq;

    /* use row id as the last resort, to make sorting more predictable */
    return a -> id < b -> id ? -1 : a -> id > b -> id;
}

static
void LoadAlignments ( CSRA1_ReferenceWindow* self, ctx_t ctx, int64_t chunk_row_id, int64_t offset, uint64_t size )
{   /* append alignments for the specified chunk to self -> align_info */
    const int64_t* primary_idx = NULL;
    uint32_t primary_idx_end = 0;
    const int64_t* secondary_idx = NULL;
    uint32_t secondary_idx_end = 0;
    uint32_t total_added = 0;

    if ( self -> primary && self -> ref_primary_begin <= chunk_row_id )
    {   
        ON_FAIL ( LoadAlignmentIndex ( self, ctx, chunk_row_id, reference_PRIMARY_ALIGNMENT_IDS, & primary_idx, & primary_idx_end ) ) 
            return;
    }        

    if ( self -> secondary && self -> ref_secondary_begin <= chunk_row_id )
    {   
        ON_FAIL ( LoadAlignmentIndex ( self, ctx, chunk_row_id, reference_SECONDARY_ALIGNMENT_IDS, & secondary_idx, & secondary_idx_end ) ) 
            return;
    }        

    total_added = primary_idx_end + secondary_idx_end;
    if ( total_added > 0 )
    {   
        self -> align_info = realloc ( self -> align_info, ( self -> align_info_total + total_added ) * sizeof ( * self -> align_info ) );
        if ( self -> align_info == NULL ) 
        {
            SYSTEM_ERROR ( xcNoMemory, "allocating circular CSRA1_ReferenceWindow chunk" );
            return;
        }
        else
        {
            uint32_t i;
            for ( i = 0; i < primary_idx_end; ++i )
            {
                ON_FAIL ( LoadAlignmentInfo( self, ctx, & self -> align_info_total, primary_idx [ i ], true, offset, size ) ) 
                    return;
            }
            for ( i = 0; i < secondary_idx_end; ++i )
            {
                ON_FAIL ( LoadAlignmentInfo( self, ctx, & self -> align_info_total, secondary_idx [ i ] + self -> id_offset, false, offset, size ) ) 
                    return;
            }
        }
    }
    /* now self -> align_info_total is the actual number of alignments currently loaded into self->align_info (can be less than allocated for) */
}

static
bool LoadFirstCircular ( CSRA1_ReferenceWindow* self, ctx_t ctx )
{   /* load the first chunk of a circular reference (other chunks will go through LoadNextChunk) */
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    int64_t last_chunk = self -> ref_end - 1;
    assert ( self );
    
    self -> align_info_total = 0;

    /* for windows on circular references, self->ref_begin and and self->ref_end - 1 
        are the rowId's of the first and last chunk of the reference, regardless of slicing */
    if ( ! self -> within_window && self -> ref_begin < last_chunk )
    {   /* load the last chunk of the reference, to cover possible overlaps into the first chunk */
        if ( self -> slice_size == 0 )
        {   /* loading possible overlaps with the first chunk */
            ON_FAIL ( LoadAlignments ( self, ctx, last_chunk, 0, self -> chunk_size ) )
                return false;
        }
        else if ( self -> slice_offset < self -> chunk_size )
        {   /* loading possible overlaps with a slice inside the first chunk */ 
            ON_FAIL ( LoadAlignments ( self, ctx, last_chunk, self -> slice_offset, self -> chunk_size - self -> slice_offset ) )
                return false;
        }
        /* target slice is not in the first chunk, no need to look for overlaps from the end of the reference */
    }
    
    ON_FAIL ( LoadAlignments ( self, ctx, self -> ref_begin, self -> slice_offset, self -> slice_size ) )
        return false;
    
    if ( self -> align_info_total > 0 )
    {
        ksort ( self -> align_info, self -> align_info_total, sizeof ( * self -> align_info ), AlignmentSortCircular, & self -> ref_length );
        self -> align_info_cur = 0;
        return true;
    }
    return false;
}

static
bool LoadNextChunk ( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    assert ( self );
    
    self -> align_info_total = 0;
    while ( self -> ref_begin < self -> ref_end )
    {
        ON_FAIL ( LoadAlignments ( self, ctx, self -> ref_begin, self -> slice_offset, self -> slice_size ) )
            return false;
    
        if ( self -> align_info_total > 0 )
        {
            ksort ( self -> align_info, self -> align_info_total, sizeof ( * self -> align_info ), AlignmentSort, NULL );
            self -> align_info_cur = 0;
        
            return true;
        }

        /* this chunk had no alignments - move to the next one */
        ++ self -> ref_begin;
    }
    
    return false;
} 
 
static
bool CSRA1_ReferenceWindowIteratorNext ( CSRA1_ReferenceWindow* self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    if ( ! self -> seen_first )
    {   /* first call - position on the first alignment */
        self -> seen_first = true;
        if ( self -> circular )
        {   
            return LoadFirstCircular ( self, ctx );
        }
    }
    else 
    {
        /* clear cached alignment*/
        NGS_AlignmentRelease ( self -> cur_align, ctx );
        self -> cur_align = NULL;
        
        ++ self -> align_info_cur;
        if ( self -> align_info_cur < self -> align_info_total )
            return true;
            
        ++ self -> ref_begin;
    }
    
    return LoadNextChunk ( self, ctx );
}

static NGS_Alignment_vt CSRA1_ReferenceWindow_vt_inst =
{
    {
        {   /* NGS_Refcount */
            CSRA1_ReferenceWindowWhack
        },

        /* NGS_Fragment */
        CSRA1_FragmentGetId,
        CSRA1_FragmentGetSequence,
        CSRA1_FragmentGetQualities,
        CSRA1_FragmentIsAligned,
        CSRA1_FragmentNext
    }, 
    
    CSRA1_ReferenceWindowGetAlignmentId,
    CSRA1_ReferenceWindowGetReferenceSpec,
    CSRA1_ReferenceWindowGetMappingQuality,
    CSRA1_ReferenceWindowGetReferenceBases,
    CSRA1_ReferenceWindowGetReadGroup,
    CSRA1_ReferenceWindowGetReadId,
    CSRA1_ReferenceWindowGetClippedFragmentBases,
    CSRA1_ReferenceWindowGetClippedFragmentQualities,
    CSRA1_ReferenceWindowGetAlignedFragmentBases,
    CSRA1_ReferenceWindowIsPrimary,
    CSRA1_ReferenceWindowGetAlignmentPosition,
    CSRA1_ReferenceWindowGetAlignmentLength,
    CSRA1_ReferenceWindowGetIsReversedOrientation,
    CSRA1_ReferenceWindowGetSoftClip,
    CSRA1_ReferenceWindowGetTemplateLength,
    CSRA1_ReferenceWindowGetShortCigar,
    CSRA1_ReferenceWindowGetLongCigar,
    CSRA1_ReferenceWindowGetRNAOrientation,
    CSRA1_ReferenceWindowHasMate,
    CSRA1_ReferenceWindowGetMateAlignmentId,
    CSRA1_ReferenceWindowGetMateAlignment,
    CSRA1_ReferenceWindowGetMateReferenceSpec,
    CSRA1_ReferenceWindowGetMateIsReversedOrientation,

    /* Iterator */
    CSRA1_ReferenceWindowIteratorNext
};

static
void CSRA1_ReferenceWindowInit ( CSRA1_ReferenceWindow * ref, 
                                 ctx_t ctx, 
                                 NGS_ReadCollection * coll,
                                 const struct NGS_Cursor* curs,
                                 bool circular,
                                 uint64_t ref_length,
                                 uint32_t chunk_size,
                                 int64_t primary_begin_row,
                                 int64_t secondary_begin_row,
                                 int64_t end_row,
                                 uint64_t offset,
                                 uint64_t size, /* 0 - all remaining */
                                 bool primary,
                                 bool secondary,
                                 bool within_window,
                                 uint64_t id_offset )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    
    TRY ( NGS_AlignmentInit ( ctx, ref, & CSRA1_ReferenceWindow_vt_inst, "CSRA1_ReferenceWindow", "" ) )
    {
        TRY ( ref -> coll = (NGS_ReadCollection *) NGS_RefcountDuplicate ( & coll -> dad, ctx ) )
        {
            ref -> reference_curs       = NGS_CursorDuplicate ( curs, ctx );
            ref -> circular             = circular;
            ref -> primary              = primary;
            ref -> secondary            = secondary;
            ref -> within_window        = within_window;
            ref -> chunk_size           = chunk_size;
            ref -> ref_length           = ref_length;
            ref -> id_offset            = id_offset;
            ref -> ref_begin            = min (primary_begin_row, secondary_begin_row);
            ref -> ref_primary_begin    = primary_begin_row;
            ref -> ref_secondary_begin  = secondary_begin_row;
            ref -> ref_end              = end_row;
            ref -> slice_offset         = offset;
            ref -> slice_size           = size;
        }
    }
}                           

/* MakeCommon
 *  makes a common alignment from VCursor
 */
NGS_Alignment * CSRA1_ReferenceWindowMake ( ctx_t ctx, 
                                            struct NGS_ReadCollection * coll,
                                            const struct NGS_Cursor* curs,
                                            bool circular,
                                            uint64_t ref_length,
                                            uint32_t chunk_size,
                                            int64_t primary_begin_row,
                                            int64_t secondary_begin_row,
                                            int64_t end_row,
                                            uint64_t offset,
                                            uint64_t size, /* 0 - all remaining */
                                            bool primary,
                                            bool secondary,
                                            bool within_window,
                                            uint64_t id_offset )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );

    CSRA1_ReferenceWindow * ref;

    assert ( coll != NULL );

    ref = calloc ( 1, sizeof * ref );
    if ( ref == NULL )
        SYSTEM_ERROR ( xcNoMemory, "allocating CSRA1_ReferenceWindow" );
    else
    {
        TRY ( CSRA1_ReferenceWindowInit ( ref, 
                                          ctx, 
                                          coll, 
                                          curs, 
                                          circular,
                                          ref_length,
                                          chunk_size,
                                          primary_begin_row,
                                          secondary_begin_row,
                                          end_row, 
                                          offset,
                                          size, 
                                          primary, 
                                          secondary,
                                          within_window,
                                          id_offset ) ) 
        {
            return ( NGS_Alignment * ) ref;
        }

        free ( ref );
    }

    return NULL;
}
