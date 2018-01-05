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

#ifndef _h_unsupported_pileup_estimator_
#define _h_unsupported_pileup_estimator_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct VCursor;

/*--------------------------------------------------------------------------
 * PilepuEstimator
 */
struct PilepuEstimator;

/*--------------------------------------------------------------------------
 * MakePileupEstimator
 *
 *      creates an PileupEstimator-object
 *
 *      self [ OUT ] - the Estimator-object to be created
 *
 *      source [ IN ] - the name of the accession
 *                      ( can be NULL, but we need none-NULL cursors then! )
 *
 *      cursor_cache_size [ IN ] - the cache-size on the cursor's
 *                                  0 ... pick default size
 *                               not used if cursor ( below ) are given
 *
 *      ref_cursor [ IN ] - cursor on the REFERENCE-table
 *                      ( can be NULL, but we need a none-NULL source then! )
 *                      if the cursor is given it has to contain these columns:
 *                          - SEQ_ID
 *                          - SEQ_LEN
 *                          - MAX_SEQ_LEN
 *                          - PRIMARY_ALIGNMENT_IDS
 *                      we expect this cursor to be opened
 *
 *      align_cursor [ IN ] - cursor on the ALIGNMENT-table
 *                      ( can be NULL, but we need a none-NULL source then! )
 *                      if the cursor is given it has to contain these columns:
 *                          - REF_POS
 *                          - REF_LEN
                            - READ_FILTER ( if user_read_filter == true )
 *                      we expect this cursor to be opened
 *
 *
 *		cutoff_value [ IN ] - how many alignments per touched REFERENCE-table
 *                            are too much, it this value is reached while
 *                            running the estimator - a value of MAX_UINT64 is
 *                            returned as result. This should avoid the
 *                            situation that collecting the estimation will
 *                            take too long...
 *                            0 ... no cutoff-check performed
 *
 *		use_read_filter [ IN ] - adds the READ_FILTER - column
 *                            to the list of required columns for the
 *                            align_cursor ( if used )
 *                            and uses the column to filter out alignments
 *                            with SRA_READ_FILTER_REJECT or
 *                            SRA_READ_FILTER_CRITERIA values in this
 *                            column
 *
 */
ALIGN_EXTERN rc_t CC MakePileupEstimator( struct PileupEstimator **self,
        const char * source,
        size_t cursor_cache_size,
        const struct VCursor * ref_cursor,
        const struct VCursor * align_cursor,
		uint64_t cutoff_value,
        bool use_read_filter );

/*--------------------------------------------------------------------------
 * ReleasePileupEstimator
 *
 *      releases an PileupEstimator-object
 * 
 */
ALIGN_EXTERN rc_t CC ReleasePileupEstimator( struct PileupEstimator *self );


/*--------------------------------------------------------------------------
 * RunPileupEstimator
 *
 *      runs the PileupEstimator-object on a given slice
 *
 *      self [ IN ] - the prepared PileupEstimator
 *
 *      refname [ IN ] - the name of the reference ( matches values in SEQ_ID )
 *
 *      slice_start [ IN ] - the zero-based start-position of the slice on the reference
 *
 *      slice_len [ IN ] - the length of the slice
 *
 *      result [ OUT ] - how many alignment-bases would be loaded
 */
ALIGN_EXTERN rc_t CC RunPileupEstimator( struct PileupEstimator *self,
                                         const String * refname,
                                         uint64_t slice_start,
                                         uint32_t slice_len,
                                         uint64_t * result );

/* RunCoverage
 *
 *      runs the PileupEstimator-object to enter the depth on each position
 *      of the requested slice into the coverage-vector
 *
 *      self [ IN ] - the prepared PileupEstimator
 *
 *      refname [ IN ] - the name of the reference ( matches values in SEQ_ID )
 *
 *      slice_start [ IN ] - the zero-based start-position of the slice on the reference
 *
 *      slice_len [ IN ] - the length of the slice
 *
 *      coverage [ OUT ] - the caller-allocated vector of coverage-depth's
 *                         it has to have at least the length of the requested slice
 */
ALIGN_EXTERN rc_t CC RunCoverage( struct PileupEstimator *self,
                                  const String * refname,
                                  uint64_t slice_start,
                                  uint32_t slice_len,
                                  uint32_t * coverage );

/* EstimatorRefCount
 *
 *      how many references are in the run given to the PileupEstimator
 *
 *      self [ IN ] - the prepared PileupEstimator
 *
 *      count [ OUT ] - the number of references
 */
ALIGN_EXTERN rc_t CC EstimatorRefCount( struct PileupEstimator *self, uint32_t * count );


/* EstimatorRefInfo
 *
 *      return information about one reference in the PileupEstimator
 *
 *      self [ IN ] - the prepared PileupEstimator
 *
 *      idx [ IN ] - index of the reference ( 0 ... count-1 )
 *
 *      refname [ OUT ] - pointer to String-struct, will be initialized, do not free
 *
 *      reflen [ OUT ] - length of the reference
 */
ALIGN_EXTERN rc_t CC EstimatorRefInfo( struct PileupEstimator *self,
                                       uint32_t idx,
                                       String * refname,
                                       uint64_t * reflen );


#ifdef __cplusplus
}
#endif

#endif /* _h_unsupported_pileup_estimator_ */
