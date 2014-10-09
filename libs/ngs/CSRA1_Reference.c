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

#include "CSRA1_Reference.h"

typedef struct CSRA1_Reference CSRA1_Reference;
#define NGS_REFERENCE CSRA1_Reference
#include "NGS_Reference.h"

#include "NGS_ReadCollection.h"
#include "NGS_Alignment.h"

#include "NGS_String.h"
#include "NGS_Cursor.h"

#include "SRA_Statistics.h"

#include "CSRA1_ReferenceWindow.h"
#include "CSRA1_Pileup.h"

#include <kfc/ctx.h>
#include <kfc/rsrc.h>
#include <kfc/except.h>
#include <kfc/xc.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/refcount.h>

#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/schema.h>
#include <vdb/vdb-priv.h>

#include <stddef.h>
#include <assert.h>

#include <strtol.h>

#include <sysalloc.h>

/*--------------------------------------------------------------------------
 * CSRA1_Reference
 */
static void                     CSRA1_ReferenceWhack ( CSRA1_Reference * self, ctx_t ctx );
static NGS_String *             CSRA1_ReferenceGetCommonName ( CSRA1_Reference * self, ctx_t ctx );
static NGS_String *             CSRA1_ReferenceGetCanonicalName ( CSRA1_Reference * self, ctx_t ctx );
static bool                     CSRA1_ReferenceGetIsCircular ( const CSRA1_Reference * self, ctx_t ctx );
static uint64_t                 CSRA1_ReferenceGetLength ( CSRA1_Reference * self, ctx_t ctx );
static struct NGS_String *      CSRA1_ReferenceGetBases ( CSRA1_Reference * self, ctx_t ctx, uint64_t offset, uint64_t size );
static struct NGS_String *      CSRA1_ReferenceGetChunk ( CSRA1_Reference * self, ctx_t ctx, uint64_t offset, uint64_t size );
static struct NGS_Alignment*    CSRA1_ReferenceGetAlignment ( CSRA1_Reference * self, ctx_t ctx, const char * alignmentId );
static struct NGS_Alignment*    CSRA1_ReferenceGetAlignments ( CSRA1_Reference * self, ctx_t ctx, bool wants_primary, bool wants_secondary );
static uint64_t                 CSRA1_ReferenceGetAlignmentCount ( const CSRA1_Reference * self, ctx_t ctx, bool wants_primary, bool wants_secondary );
static struct NGS_Alignment*    CSRA1_ReferenceGetAlignmentSlice ( CSRA1_Reference * self, ctx_t ctx, uint64_t offset, uint64_t size, bool wants_primary, bool wants_secondary );
static struct NGS_Pileup*       CSRA1_ReferenceGetPileups ( CSRA1_Reference * self, ctx_t ctx, bool wants_primary, bool wants_secondary );
static struct NGS_Pileup*       CSRA1_ReferenceGetPileupSlice ( CSRA1_Reference * self, ctx_t ctx, uint64_t offset, uint64_t size, bool wants_primary, bool wants_secondary );
struct NGS_Statistics*          CSRA1_ReferenceGetStatistics ( const CSRA1_Reference * self, ctx_t ctx );
static bool                     CSRA1_ReferenceIteratorNext ( CSRA1_Reference * self, ctx_t ctx );

static NGS_Reference_vt CSRA1_Reference_vt_inst =
{
    /* NGS_Refcount */
    { CSRA1_ReferenceWhack },
    
    /* NGS_Reference */
    CSRA1_ReferenceGetCommonName,
    CSRA1_ReferenceGetCanonicalName,
    CSRA1_ReferenceGetIsCircular,
    CSRA1_ReferenceGetLength,
    CSRA1_ReferenceGetBases,
    CSRA1_ReferenceGetChunk,
    CSRA1_ReferenceGetAlignment,
    CSRA1_ReferenceGetAlignments,
    CSRA1_ReferenceGetAlignmentCount,
    CSRA1_ReferenceGetAlignmentSlice,
    CSRA1_ReferenceGetPileups,
    CSRA1_ReferenceGetPileupSlice,
    CSRA1_ReferenceGetStatistics,
    
    /* NGS_ReferenceIterator */
    CSRA1_ReferenceIteratorNext,
};

struct CSRA1_Reference
{
    NGS_Reference dad;   
    NGS_ReadCollection * coll;
    
    uint32_t chunk_size;
    
    int64_t row_begin;
    int64_t row_end;
    const NGS_Cursor * curs; /* can be NULL if created for an empty iterator */
    uint64_t align_id_offset;
    uint64_t cur_length; /* size of current reference in bases (0 = not yet counted) */
    bool iterating;
    bool seen_first;
};

/* Init
 */
static
void CSRA1_ReferenceInit ( ctx_t ctx, 
                           CSRA1_Reference * ref,
                           NGS_ReadCollection * coll,
                           const char *clsname, 
                           const char *instname,
                           uint64_t align_id_offset )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );

    if ( ref == NULL )
        INTERNAL_ERROR ( xcParamNull, "bad object reference" );
    else
    {
        TRY ( NGS_ReferenceInit ( ctx, & ref -> dad, & CSRA1_Reference_vt_inst, clsname, instname ) )
        {
            ref -> coll = (NGS_ReadCollection *) NGS_RefcountDuplicate ( & coll -> dad, ctx );
            ref -> align_id_offset = align_id_offset;
        }
    }
}

/* Whack
 */
static
void CSRA1_ReferenceWhack ( CSRA1_Reference * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcDestroying );

    NGS_CursorRelease ( self -> curs, ctx );
    
    NGS_RefcountRelease ( & self -> coll -> dad, ctx );
}

NGS_String * CSRA1_ReferenceGetCommonName ( CSRA1_Reference * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self != NULL );
    if ( ! self -> seen_first )
    {
        USER_ERROR ( xcIteratorUninitialized, "Reference accessed before a call to ReferenceIteratorNext()" );
        return NULL;        
    }
    
    return NGS_CursorGetString ( self -> curs, ctx, self -> row_begin, reference_NAME );
}

NGS_String * CSRA1_ReferenceGetCanonicalName ( CSRA1_Reference * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    assert ( self != NULL );
    if ( ! self -> seen_first )
    {
        USER_ERROR ( xcIteratorUninitialized, "Reference accessed before a call to ReferenceIteratorNext()" );
        return NULL;        
    }
    
    return NGS_CursorGetString ( self -> curs, ctx, self -> row_begin, reference_SEQ_ID);
}

bool CSRA1_ReferenceGetIsCircular ( const CSRA1_Reference * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
   
    if ( self -> curs == NULL )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return false;
    }
    if ( ! self -> seen_first )
    {
        USER_ERROR ( xcIteratorUninitialized, "Reference accessed before a call to ReferenceIteratorNext()" );
        return false;        
    }
    
    /* if current row is valid, read data */
    if ( self -> row_begin < self -> row_end )
    {
        return NGS_CursorGetBool ( self -> curs, ctx, self -> row_begin, reference_CIRCULAR );
    }

    return false;
}

static
uint64_t CountRows ( CSRA1_Reference * self, ctx_t ctx, uint32_t colIdx, const void* value, int64_t firstRow, uint64_t end_row)
{   /* count consecutive rows having the same value in column # colIdx as in firstRow, starting and including firstRow */
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    
    uint64_t cur_row = (uint64_t)firstRow + 1;
    while (cur_row < end_row)
    {
        const void * base;
        uint32_t elem_bits, boff, row_len;
        ON_FAIL ( NGS_CursorCellDataDirect ( self -> curs, ctx, cur_row, colIdx, & elem_bits, & base, & boff, & row_len ) )
            return 0;

        if (base != value)
        {
            break;
        }
        
        ++ cur_row;
    }
    return cur_row - firstRow;
}



uint64_t CSRA1_ReferenceGetLength ( CSRA1_Reference * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
    if ( self -> curs == NULL )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return 0;
    }
    if ( ! self -> seen_first )
    {
        USER_ERROR ( xcIteratorUninitialized, "Reference accessed before a call to ReferenceIteratorNext()" );
        return 0;        
    }
    
    if ( self -> cur_length == 0) /* not yet calculated */
    {
        uint64_t chunk_count;
        if ( ! self -> iterating )
        {   /* row_end can be used as the end of reference */
            chunk_count = self -> row_end - self -> row_begin;
        }
        else
        {   /* scan to count chunks */
            
            const void * base;
            uint32_t elem_bits, boff, row_len;
            ON_FAIL ( NGS_CursorCellDataDirect ( self -> curs, ctx, self -> row_begin, reference_NAME, & elem_bits, & base, & boff, & row_len ) )
                return 0;
            
            chunk_count = CountRows( self, ctx, reference_NAME, base, self -> row_begin, self -> row_end );
        }
        
        self -> cur_length =  self -> chunk_size * ( chunk_count - 1 ) + 
                              NGS_CursorGetUInt32 ( self -> curs, 
                                                    ctx, 
                                                    self -> row_begin + chunk_count - 1, 
                                                    reference_SEQ_LEN );
    }
    
    return self -> cur_length;
}

struct NGS_String * CSRA1_ReferenceGetBases ( CSRA1_Reference * self, ctx_t ctx, uint64_t offset, uint64_t size )
{   
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
    if ( self -> curs == NULL )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return NULL;
    }
    if ( ! self -> seen_first )
    {
        USER_ERROR ( xcIteratorUninitialized, "Reference accessed before a call to ReferenceIteratorNext()" );
        return NULL;        
    }
    
    {
        uint64_t totalBases = CSRA1_ReferenceGetLength ( self, ctx );
        if ( offset >= totalBases )
        {
            return NGS_StringMake ( ctx, "", 0 );
        }
        else
        {   
            uint64_t basesToReturn = totalBases - offset;
            char* data;
            
            if (size != (size_t)-1 && basesToReturn > size)
                basesToReturn = size;
                
            data = (char*) malloc ( basesToReturn );
            if ( data == NULL )
            {
                SYSTEM_ERROR ( xcNoMemory, "allocating %lu bases", basesToReturn );
                return NGS_StringMake ( ctx, "", 0 );
            }
            else
            {
                size_t cur_offset = 0;
                while ( cur_offset < basesToReturn )
                {
                    /* we will potentially ask for more than available in the current chunk; 
                        CSRA1_ReferenceGetChunkSize will return only as much as is available in the chunk */
                    NGS_String* chunk = CSRA1_ReferenceGetChunk ( self, ctx, offset + cur_offset, basesToReturn - cur_offset );
                    cur_offset += string_copy(data + cur_offset, basesToReturn - cur_offset, 
                                              NGS_StringData ( chunk, ctx ), NGS_StringSize ( chunk, ctx ) );
                    NGS_StringRelease ( chunk, ctx );
                }
                return NGS_StringMakeOwned ( ctx, data, basesToReturn );
            }
        }
    }
}

struct NGS_String * CSRA1_ReferenceGetChunk ( CSRA1_Reference * self, ctx_t ctx, uint64_t offset, uint64_t size )
{   
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
    if ( self -> curs == NULL )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return NULL;
    }
    if ( ! self -> seen_first )
    {
        USER_ERROR ( xcIteratorUninitialized, "Reference accessed before a call to ReferenceIteratorNext()" );
        return NULL;        
    }
    
    if ( offset >= CSRA1_ReferenceGetLength ( self, ctx ) )
    {
        return NGS_StringMake ( ctx, "", 0 );
    }
    else
    {
        const NGS_String* read = NGS_CursorGetString ( self -> curs, ctx, self -> row_begin + offset / self -> chunk_size, reference_READ);
        NGS_String* ret;
        if ( size == (size_t)-1 )
            ret = NGS_StringSubstrOffset ( read, ctx, offset % self -> chunk_size );
        else
            ret = NGS_StringSubstrOffsetSize ( read, ctx, offset % self -> chunk_size, size );
        NGS_StringRelease ( read, ctx );
        return ret;
    }
}

struct NGS_Alignment* CSRA1_ReferenceGetAlignment ( CSRA1_Reference * self, ctx_t ctx, const char * alignmentIdStr )
{   
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
    if ( self -> curs == NULL )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return NULL;
    }
    if ( ! self -> seen_first )
    {
        USER_ERROR ( xcIteratorUninitialized, "Reference accessed before a call to ReferenceIteratorNext()" );
        return NULL;        
    }

    {
        TRY ( NGS_Alignment* ref = NGS_ReadCollectionGetAlignment ( self -> coll, ctx, alignmentIdStr ) )
        {
            TRY ( NGS_String * spec = NGS_AlignmentGetReferenceSpec( ref, ctx ) )
            {
                TRY ( NGS_String * commonName = CSRA1_ReferenceGetCommonName ( self, ctx ) )
                {
                    if ( string_cmp( NGS_StringData ( spec, ctx ), 
                                     NGS_StringSize ( spec, ctx ),
                                     NGS_StringData ( commonName, ctx ), 
                                     NGS_StringSize ( commonName, ctx ),
                                     (uint32_t)NGS_StringSize ( spec, ctx ) ) == 0 )
                    {
                        NGS_StringRelease ( spec, ctx );
                        NGS_StringRelease ( commonName, ctx );
                        return ref;
                    }
                        
                    USER_ERROR ( xcWrongReference, 
                                "Requested alignment is on a wrong reference: reference '%.*s', alignment has '%.*s'",  
                                NGS_StringSize ( commonName, ctx ), NGS_StringData ( commonName, ctx ),
                                NGS_StringSize ( spec, ctx ), NGS_StringData ( spec, ctx ) );
                    
                    NGS_StringRelease ( commonName, ctx );
                }
                NGS_StringRelease ( spec, ctx );
            }
            NGS_AlignmentRelease ( ref, ctx );
        }
    }
    return NULL;
}

struct NGS_Alignment* CSRA1_ReferenceGetAlignments ( CSRA1_Reference * self, ctx_t ctx, bool wants_primary, bool wants_secondary )
{   
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
    if ( self -> curs == NULL )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return NULL;
    }
    if ( ! self -> seen_first )
    {
        USER_ERROR ( xcIteratorUninitialized, "Reference accessed before a call to ReferenceIteratorNext()" );
        return NULL;        
    }

    {
        TRY ( bool circular = CSRA1_ReferenceGetIsCircular ( self, ctx ) )
        {
            TRY ( uint64_t ref_len = CSRA1_ReferenceGetLength ( self, ctx ) )
            {
                return CSRA1_ReferenceWindowMake ( ctx, 
                                                   self -> coll, 
                                                   self -> curs,
                                                   circular,
                                                   ref_len,
                                                   self -> chunk_size,
                                                   self -> row_begin, 
                                                   self -> row_begin, 
                                                   self -> row_end, 
                                                   0,
                                                   0,
                                                   wants_primary, 
                                                   wants_secondary,
                                                   self -> align_id_offset );
            }
        }
    }
    
    return NULL;
}

uint64_t CSRA1_ReferenceGetAlignmentCount ( const CSRA1_Reference * self, ctx_t ctx, bool wants_primary, bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    UNIMPLEMENTED(); /* CSRA1_ReferenceGetAlignmentCount */
    return 0;
}

/*
    Calculate starting reference chunk to cover alignments overlapping with the slice; 
    separately for primary and secondary alignments
*/
static
void LoadOverlaps ( CSRA1_Reference * self, 
                    ctx_t ctx, 
                    uint32_t chunk_size,
                    uint64_t offset, 
                    int64_t * primary_begin, 
                    int64_t * secondary_begin) 
{
    int64_t first_row = self -> row_begin + offset / chunk_size;
    uint32_t primary_len;
    uint32_t secondary_len;
    int32_t primary_pos;
    int32_t secondary_pos;
    uint32_t offset_in_chunk = offset % chunk_size;
    
    {   /*OVERLAP_REF_LEN*/
        const void* base;
        uint32_t elem_bits, boff, row_len;
        ON_FAIL ( NGS_CursorCellDataDirect ( self -> curs, ctx, first_row, reference_OVERLAP_REF_LEN, & elem_bits, & base, & boff, & row_len ) )
        {   /* no overlap columns, apply 10-chunk lookback */
            CLEAR ();
            if ( first_row > 11 ) 
            {
                *primary_begin = 
                *secondary_begin = first_row - 10;
            }
            else
            {
                *primary_begin = 
                *secondary_begin = 1;
            }
            return;
        }
        
        assert ( elem_bits == 32 );
        assert ( boff == 0 );
        assert ( row_len == 3 );
        
        primary_len     = ( (const uint32_t*)base ) [0];
        secondary_len   = ( (const uint32_t*)base ) [1];
    }
    
    if (primary_len == 0 && secondary_len == 0)
    {
        *primary_begin = *secondary_begin = first_row;
    }
    else
    {   /*OVERLAP_REF_POS*/
        const void* base;
        uint32_t elem_bits, boff, row_len;
        ON_FAIL( NGS_CursorCellDataDirect ( self -> curs, ctx, first_row, reference_OVERLAP_REF_POS, & elem_bits, & base, & boff, & row_len ) )
            return;
            
        assert ( elem_bits == 32 );
        assert ( boff == 0 );
        assert ( row_len == 3 );
        
        primary_pos     = ( (const int32_t*)base ) [0];
        secondary_pos   = ( (const int32_t*)base ) [1];
    
        if ( primary_len == 0 || primary_len < offset_in_chunk )
        {
            * primary_begin = first_row;
        }
        else
        {
            * primary_begin = self -> row_begin + primary_pos / chunk_size;
        }
        
        if ( secondary_len == 0 || secondary_len < offset_in_chunk )
        {
            * secondary_begin = first_row;
        }
        else
        {
            * secondary_begin = self -> row_begin + secondary_pos / chunk_size;
        }
    }
}               

struct NGS_Alignment* CSRA1_ReferenceGetAlignmentSlice ( CSRA1_Reference * self, 
                                                         ctx_t ctx, 
                                                         uint64_t offset, 
                                                         uint64_t size, 
                                                         bool wants_primary, 
                                                         bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
    if ( self -> curs == NULL )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return NULL;
    }
    if ( ! self -> seen_first )
    {
        USER_ERROR ( xcIteratorUninitialized, "Reference accessed before a call to ReferenceIteratorNext()" );
        return NULL;        
    }
    
    if ( size == 0 )
    {
        TRY ( NGS_String * str = NGS_StringMake ( ctx, "", 0) )
        {
            NGS_Alignment* ret = NGS_AlignmentMakeNull ( ctx, str );
            NGS_StringRelease ( str, ctx );
            return ret;
        }
    }

    {
        TRY ( bool circular = CSRA1_ReferenceGetIsCircular ( self, ctx ) )
        {
            TRY ( uint64_t ref_len = CSRA1_ReferenceGetLength ( self, ctx ) )
            {
                if ( circular )
                {   /* for a circular reference, always look at the whole of it 
                       (to account for alignments that start near the end and overlap with the first chunk) */
                    return CSRA1_ReferenceWindowMake ( ctx, 
                                                       self -> coll, 
                                                       self -> curs,
                                                       true, /* circular */
                                                       ref_len,
                                                       self -> chunk_size,
                                                       self->row_begin, /*primary_begin*/
                                                       self->row_begin, /*secondary_begin*/
                                                       self -> row_end, 
                                                       offset,
                                                       size,
                                                       wants_primary, 
                                                       wants_secondary,
                                                       self -> align_id_offset );
                }
                else
                {   /* for non-circular references, restrict the set of chunks to go through */
                    int64_t primary_begin   = self->row_begin;
                    int64_t secondary_begin = self->row_begin;

                    /* calculate the row range taking "overlaps" into account */
                    TRY ( LoadOverlaps ( self, ctx, self -> chunk_size, offset, & primary_begin, & secondary_begin ) )
                    {
                        /* calculate the last chunk (same for all types of alignments) */
                        int64_t end = self -> row_begin + ( offset + size - 1 ) / self -> chunk_size + 1;
                        if ( end > self -> row_end )
                            end = self -> row_end;
                            
                        return CSRA1_ReferenceWindowMake ( ctx, 
                                                           self -> coll, 
                                                           self -> curs,
                                                           false,
                                                           ref_len,
                                                           self -> chunk_size,
                                                           primary_begin, 
                                                           secondary_begin,
                                                           end, 
                                                           offset,
                                                           size,
                                                           wants_primary, 
                                                           wants_secondary,
                                                           self -> align_id_offset );
                    }
                }
            }
        }
        return NULL;
    }
}                                                         

struct NGS_Pileup* CSRA1_ReferenceGetPileups ( CSRA1_Reference * self, ctx_t ctx, bool wants_primary, bool wants_secondary )
{   
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );

    assert ( self );
    if ( self -> curs == NULL )
    {
        USER_ERROR ( xcCursorExhausted, "No more rows available" );
        return NULL;
    }
    if ( ! self -> seen_first )
    {
        USER_ERROR ( xcIteratorUninitialized, "Reference accessed before a call to ReferenceIteratorNext()" );
        return NULL;        
    }

    {   //TODO: GetName or GetCanonicalName?
        TRY ( NGS_String* spec = CSRA1_ReferenceGetCommonName ( self, ctx ) ) 
        {
            struct NGS_Pileup* ret = CSRA1_PileupIteratorMake ( ctx, spec, wants_primary, wants_secondary );
            NGS_StringRelease ( spec, ctx );
            return ret;
        }
    }
    return NULL;
}

static struct NGS_Pileup* CSRA1_ReferenceGetPileupSlice ( CSRA1_Reference * self, ctx_t ctx, uint64_t offset, uint64_t size, bool wants_primary, bool wants_secondary )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcReading );
    UNIMPLEMENTED(); /* CSRA1_ReferenceGetPileupSlice */
    return NULL;
}

struct NGS_Statistics* CSRA1_ReferenceGetStatistics ( const CSRA1_Reference * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );
    /* for now, return an empty stats object */
    return SRA_StatisticsMake ( ctx );
}

static 
bool CSRA1_ReferenceFind ( CSRA1_Reference * self, ctx_t ctx, const char * spec, int64_t* firstRow, uint64_t* rowCount )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );

    assert ( self );
    assert ( self -> curs );
    assert ( spec );

    {   /* use index on reference name if available */
        TRY ( const VTable* table = NGS_CursorGetTable ( self -> curs, ctx ) )
        {
            const KIndex *index;
            rc_t rc = VTableOpenIndexRead( table, & index, "i_name" );
            VTableRelease( table );
            if ( rc == 0 )
            {
                rc = KIndexFindText ( index, spec, firstRow, rowCount, NULL, NULL );
                KIndexRelease ( index );
                if ( rc == 0 )
                    return true;
            }
        }
    }
    /* index not available - do a table scan */
    {
        int64_t cur_row;
        int64_t end_row;
        uint64_t total_row_count;
        String specStr;
        
        StringInitCString( &specStr, spec );
        TRY ( NGS_CursorGetRowRange ( self -> curs, ctx, & cur_row, & total_row_count ) )
        {
            end_row = cur_row + total_row_count;
            while ( cur_row < end_row )
            {
                const void * base;
                uint32_t elem_bits, boff, row_len;
                ON_FAIL ( NGS_CursorCellDataDirect ( self -> curs, ctx, cur_row, reference_NAME, & elem_bits, & base, & boff, & row_len ) )
                    return false;
                    
                {
                    String name;
                    StringInit( &name, base, string_len(base, row_len), row_len);

                    assert ( elem_bits == 8 );
                    assert ( boff == 0 );
                    
                    if ( StringCompare ( & name, & specStr ) == 0 )
                    {
                        *firstRow = cur_row;
                        *rowCount = CountRows( self, ctx, reference_NAME, base, * firstRow, end_row );
                        return true;
                    }
                }

                ++cur_row;
            }
        }
    }
    
    return false;
}

NGS_Reference * CSRA1_ReferenceMake ( ctx_t ctx, 
                                      struct NGS_ReadCollection * coll,
                                      const struct NGS_Cursor * curs, 
                                      const char * spec,
                                      uint64_t align_id_offset )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );

    assert ( coll != NULL );
    assert ( curs != NULL );
    
    {
        TRY ( NGS_String * collName = NGS_ReadCollectionGetName ( coll, ctx ) )
        {
            CSRA1_Reference * ref = calloc ( 1, sizeof * ref );
            if ( ref == NULL )
                SYSTEM_ERROR ( xcNoMemory, "allocating CSRA1_Reference(%s) on '%S'", spec, collName );
            else
            {
#if _DEBUGGING
                char instname [ 256 ];
                string_printf ( instname, sizeof instname, NULL, "%S(%s)", collName, spec );
                instname [ sizeof instname - 1 ] = 0;
#else
                const char *instname = "";
#endif
                TRY ( CSRA1_ReferenceInit ( ctx, ref, coll, "CSRA1_Reference", instname, align_id_offset ) )
                {
                    uint64_t rowCount;
                    
                    ref -> curs = NGS_CursorDuplicate ( curs, ctx );
                    
                    
                    /* find requested name */
                    if ( CSRA1_ReferenceFind ( ref, ctx, spec, & ref -> row_begin, & rowCount ) )
                    {
                        TRY ( ref -> chunk_size = NGS_CursorGetUInt32 ( ref -> curs, ctx, ref -> row_begin, reference_MAX_SEQ_LEN ) )
                        {
                            ref -> iterating = false;
                            ref -> row_end = ref -> row_begin + rowCount;
                            ref -> seen_first = true;
                            NGS_StringRelease ( collName, ctx );
                            return ( NGS_Reference * ) ref;
                        }
                    }      
                    
                    INTERNAL_ERROR ( xcRowNotFound, "Reference not found ( NAME = %s )", spec );
                    CSRA1_ReferenceWhack ( ref, ctx );
                }

                free ( ref );
            }
            NGS_StringRelease ( collName, ctx );
        }
    }

    return NULL;
}

/*--------------------------------------------------------------------------
 * NGS_ReferenceIterator
 */

/* Make
 */
NGS_Reference * CSRA1_ReferenceIteratorMake ( ctx_t ctx, 
                                                    struct NGS_ReadCollection * coll, 
                                                    const struct NGS_Cursor * curs,
                                                    uint64_t align_id_offset )
{
    FUNC_ENTRY ( ctx, rcSRA, rcCursor, rcConstructing );

    assert ( coll != NULL );
    assert ( curs != NULL );
    
    {
        TRY ( NGS_String * collName = NGS_ReadCollectionGetName ( coll, ctx ) )
        {
            CSRA1_Reference * ref = calloc ( 1, sizeof * ref );
            if ( ref == NULL )
                SYSTEM_ERROR ( xcNoMemory, "allocating CSRA1_ReferenceIterator on '%S'", collName );
            else
            {
#if _DEBUGGING
                char instname [ 256 ];
                string_printf ( instname, sizeof instname, NULL, "%S", collName );
                instname [ sizeof instname - 1 ] = 0;
#else
                const char *instname = "";
#endif
                TRY ( CSRA1_ReferenceInit ( ctx, ref, coll, "CSRA1_Reference", instname, align_id_offset ) )
                {
                    uint64_t row_count;
                   
                    ref -> curs = curs;
                    
                    TRY ( NGS_CursorGetRowRange ( ref -> curs, ctx, & ref -> row_begin, & row_count ) )
                    {
                        TRY ( ref -> chunk_size = NGS_CursorGetUInt32 ( ref -> curs, ctx, ref -> row_begin, reference_MAX_SEQ_LEN ) )
                        {
                            ref -> iterating    = true;
                            ref -> row_end      = ref -> row_begin + row_count;
                            ref -> seen_first   = false;
                            NGS_StringRelease ( collName, ctx );
                            return & ref -> dad;
                        }
                    }
                    CSRA1_ReferenceWhack ( ref, ctx );
                }

                free ( ref );
            }
            NGS_StringRelease ( collName, ctx );
        }
    }

    return NULL;
}

/* Next
 */
bool CSRA1_ReferenceIteratorNext ( CSRA1_Reference * self, ctx_t ctx )
{
    assert ( self != NULL );
    
    if ( self -> curs == NULL )
        return false;

    if ( self -> seen_first )
    {   /* skip to the next reference */
        NGS_String* ngs_prevName = NGS_CursorGetString ( self -> curs, ctx, self -> row_begin, reference_NAME );
        String prevName;
        StringInit ( &prevName, 
                     NGS_StringData ( ngs_prevName, ctx ), 
                     NGS_StringSize ( ngs_prevName, ctx ), 
                     string_len ( NGS_StringData ( ngs_prevName, ctx ), NGS_StringSize ( ngs_prevName, ctx ) ) );
        ++ self -> row_begin;
        
        while ( self -> row_begin < self -> row_end )
        {
            const void * base;
            uint32_t elem_bits, boff, row_len;
            ON_FAIL ( NGS_CursorCellDataDirect ( self -> curs, ctx, self -> row_begin, reference_NAME, & elem_bits, & base, & boff, & row_len ) )
                return false;

            {
                String name;
                StringInit( &name, base, string_len(base, row_len), row_len);

                assert ( elem_bits == 8 );
                assert ( boff == 0 );
                
                if ( StringCompare ( & name, & prevName ) != 0 )
                {
                    break;
                }
            }

            ++ self -> row_begin;
        }
        
        self -> cur_length = 0;
        NGS_StringRelease ( ngs_prevName, ctx );
    }
    else
    {
        self -> seen_first = true;
    }
    return ( self -> row_begin < self -> row_end );
}
