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

#ifndef _h_csra1_pileup_
#define _h_csra1_pileup_

#ifndef _h_kfc_defs_
#include <kfc/defs.h>
#endif

#ifndef _h_ngs_pileup_
#include "NGS_Pileup.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct VDatabase;
struct VTable;
struct VCursor;
struct VBlob;
struct NGS_Cursor;
struct NGS_Reference;

/*--------------------------------------------------------------------------
 * implementation details 
 */
 
#define CACHE_IMPL_AS_LIST 1 /* ==1 if using DLList to store a set of cached alignments, otherwise - use array-based cache */
#define USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS 1 /* ==1 if using 1 blob to be able to have persistent pointer to PRIMARY_ALIGNMENT_IDS CellData, otherwise - copy data */
#define USE_SINGLE_BLOB_FOR_ALIGNMENTS 1 /* ==1 if using 1 blob to be able to have persistent pointer to PRIMARY_ALIGNMENT CellData, otherwise - copy data */ 

#if CACHE_IMPL_AS_LIST == 1
#include <klib/container.h>
#endif

typedef struct Alignment_CacheItem
{
#if CACHE_IMPL_AS_LIST == 1
    DLNode node; /* list node */
#endif
    int64_t row_id;
    int64_t start;
    uint64_t len;
    uint32_t seq_start; /* zero-based! Each alignment's REF_START (start) is relative to corresponding REFERENCE.SEQ_START, so we have to store this relation here */
} Alignment_CacheItem;

typedef struct Alignment_Cache
{
    size_t size;
#if CACHE_IMPL_AS_LIST == 1
    DLList list_alignments;
#else
    size_t capacity;
    Alignment_CacheItem* data;
#endif

} Alignment_Cache;

struct CSRA1_Pileup
{
    NGS_Pileup dad;   
    
    bool primary;
    bool secondary;

    struct VDatabase const* db;
    struct VTable const* table_pa;

    struct NGS_Cursor const* curs_ref;
    struct VCursor const* cursor_pa;

    /* Static data */
    int64_t reference_start_id;
    int64_t reference_last_id;

    uint64_t total_row_count; /* row count of all references' row ids */

    int64_t slice_start;
    uint64_t slice_length;

    int64_t slice_start_id;
    int64_t slice_end_id;

    /* Blobs that we want to have manual control over */
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
    struct VBlob const* blob_alignment_ids; /* Here we store the blob containig current reference row */
#endif
#if USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1
    struct VBlob const* blob_alignments_ref_start;
    struct VBlob const* blob_alignments_ref_len;
    struct VBlob const* blob_alignments_rd_filter;
#endif

    uint32_t max_seq_len;

    /* Current State */
    uint32_t current_seq_start;

    uint64_t ref_pos;
    Alignment_Cache cache_alignment;        /* Alignments intersecting ref_pos */

    size_t size_alignment_ids;
    size_t next_alignment_idx; /* index in alignment_ids pointing to the first id that has not been cached yet */
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS != 1
    size_t capacity_alignment_ids;
    int64_t* alignment_ids; /* alignment_ids of the current reference row_id */
#else
    int64_t const* alignment_ids; /* alignment_ids of the current reference row_id */
#endif

    bool is_started;    /* NextIterator has been called at least one time */
    bool is_finished;   /* NextIterator has seen the end, no more operations are allowed */
};

/*--------------------------------------------------------------------------
 * Constructors
 */
 
struct NGS_Pileup * CSRA1_PileupIteratorMake( 
    ctx_t ctx,
    struct NGS_Reference* ref,
    struct VDatabase const* db,
    struct NGS_Cursor const* curs_ref,
    int64_t first_row_id, 
    int64_t last_row_id,
    bool wants_primary, 
    bool wants_secondary );

struct NGS_Pileup * CSRA1_PileupIteratorMakeSlice( 
    ctx_t ctx,
    struct NGS_Reference* ref,
    struct VDatabase const* db,
    struct NGS_Cursor const* curs_ref,
    int64_t first_row_id, 
    int64_t last_row_id,
    uint64_t slice_start, 
    uint64_t slice_size,
    bool wants_primary, 
    bool wants_secondary );

#ifdef __cplusplus
}
#endif

#endif /* _h_csra1_pileup_ */
