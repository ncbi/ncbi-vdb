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

#ifndef _h_align_align_access_
#define _h_align_align_access_

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VPath;


/*--------------------------------------------------------------------------
 * constants
 */
typedef uint8_t AlignmentStrandDirection;
enum
{
    asd_Unknown, /* e.g. an error occured */
    asd_Forward = 'F',
    asd_Reverse = 'R'
};


/*--------------------------------------------------------------------------
 * AlignAccessMgr
 *  opaque handle to library
 */
typedef struct AlignAccessMgr AlignAccessMgr;

/* Make
 *  make a manager for read-only access
 */
ALIGN_EXTERN rc_t CC AlignAccessMgrMake ( const AlignAccessMgr **mgr );

/* AddRef
 * Release
 */
ALIGN_EXTERN rc_t CC AlignAccessMgrAddRef ( const AlignAccessMgr *self );
ALIGN_EXTERN rc_t CC AlignAccessMgrRelease ( const AlignAccessMgr *self );



/*--------------------------------------------------------------------------
 * AlignAccessDB
 *  opaque handle to database
 */
typedef struct AlignAccessDB AlignAccessDB;

/* Make
 *  make a read-only database handle from BAM file
 *  where the BAM file is treated as the entire database
 *
 *  "bam" [ IN ] - NUL terminated POSIX path in UTF-8 character set
 *  designating BAM file
 *
 *  "bam_index" [ IN ] - NUL terminated POSIX path in UTF-8 character
 *  set designating BAM index file
 *
 * NB - on Windows, these paths should be converted POSIX-style
 */
ALIGN_EXTERN rc_t CC AlignAccessMgrMakeBAMDB ( const AlignAccessMgr *self,
    const AlignAccessDB **db, struct VPath const *bam );
ALIGN_EXTERN rc_t CC AlignAccessMgrMakeIndexBAMDB ( const AlignAccessMgr *self,
    const AlignAccessDB **db, struct VPath const *bam, struct VPath const *bam_index );

/* AddRef
 * Release
 */
ALIGN_EXTERN rc_t CC AlignAccessDBAddRef ( const AlignAccessDB *self );
ALIGN_EXTERN rc_t CC AlignAccessDBRelease ( const AlignAccessDB *self );



/*--------------------------------------------------------------------------
 * AlignAccessRefSeqEnumerator
 *  an enumerator onto available reference sequences
 */
typedef struct AlignAccessRefSeqEnumerator AlignAccessRefSeqEnumerator;

#define AlignAccessRefSeqEnumeratorEOFCode (RC(rcAlign, rcTable, rcReading, rcRow, rcNotFound))
#define AlignAccessRefSeqEnumeratorIsEOF(RESULT_CODE) (GetRCObject(RESULT_CODE) == rcRow && GetRCState(RESULT_CODE) == rcNotFound)

/* EnumerateRefSequences
 *  make an enumerator of reference sequences visible in database
 *
 *  "refseq_enum" [ OUT ] - return parameter for enumerator
 *
 *  return codes:
 *   object: rcRow, state: rcNotFound - if the index is empty
 */
ALIGN_EXTERN rc_t CC AlignAccessDBEnumerateRefSequences ( const AlignAccessDB *self,
    AlignAccessRefSeqEnumerator **refseq_enum );


/* AddRef
 * Release
 */
ALIGN_EXTERN rc_t CC AlignAccessRefSeqEnumeratorAddRef ( const AlignAccessRefSeqEnumerator *self );
ALIGN_EXTERN rc_t CC AlignAccessRefSeqEnumeratorRelease ( const AlignAccessRefSeqEnumerator *self );

/* advance the enumerator to the next element
 *
 *  return codes:
 *   object: rcRow, state: rcNotFound - if there is no next element
 */
ALIGN_EXTERN rc_t CC AlignAccessRefSeqEnumeratorNext(const AlignAccessRefSeqEnumerator *cself);

/* GetID
 *  get text id of reference sequence
 *
 *  "id_buffer" [ IN ] and "buffer_size" [ IN ] - return buffer for
 *  NUL terminated ASCII id
 *
 *  "id_size" [ OUT, NULL OKAY ] - optional return parameter for length
 */
ALIGN_EXTERN rc_t CC AlignAccessRefSeqEnumeratorGetID ( const AlignAccessRefSeqEnumerator *self,
    char *id_buffer, size_t buffer_size, size_t *id_size );
    
/* GetLength
 *  get text id of reference sequence
 */
ALIGN_EXTERN rc_t CC AlignAccessRefSeqEnumeratorGetLength
    ( const AlignAccessRefSeqEnumerator *self, uint64_t *length );


/*--------------------------------------------------------------------------
 * AlignAccessAlignmentEnumerator
 *  an enumerator onto alignments
 */
typedef struct AlignAccessAlignmentEnumerator AlignAccessAlignmentEnumerator;

#define AlignAccessAlignmentEnumeratorEOFCode (RC(rcAlign, rcTable, rcReading, rcRow, rcNotFound))
#define AlignAccessAlignmentEnumeratorIsEOF(RESULT_CODE) (GetRCObject(RESULT_CODE) == rcRow && GetRCState(RESULT_CODE) == rcNotFound)

/* EnumerateAlignments
 *  make an enumerator of all alignments in database
 *
 *  "align_enum" [ OUT ] - return parameter for enumerator
 *
 *  return codes:
 *   object: rcRow, state: rcNotFound - if there is no first element
 *
 * NB - this is a BAD interface, here only to support dumping a BAM file
 *  it may not be available under all circumstances
 */
ALIGN_EXTERN rc_t CC AlignAccessDBEnumerateAlignments ( const AlignAccessDB *self,
    AlignAccessAlignmentEnumerator **align_enum );


/* WindowedAlignments
 *  make an enumerator of alignments within a window of a reference sequence
 *
 *  "align_enum" [ OUT ] - return parameter for enumerator
 *
 *  "refseq_id" [ IN ] - NUL terminated ASCII string giving reference sequence id
 *
 *  "pos" [ IN ] and "wsize" [ IN ] - starting position and size of window
 *  on reference sequence
 *
 *  return codes:
 *   object: rcRow, state: rcNotFound - if there is no first element
 */
ALIGN_EXTERN rc_t CC AlignAccessDBWindowedAlignments ( const AlignAccessDB *self,
    AlignAccessAlignmentEnumerator **align_enum, const char *refseq_id,
    uint64_t pos, uint64_t wsize );


/* AddRef
 * Release
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorAddRef ( const AlignAccessAlignmentEnumerator *self );
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorRelease ( const AlignAccessAlignmentEnumerator *self );


/* Next
 * advance the enumerator to the next element
 *
 *  return codes:
 *   object: rcRow, state: rcNotFound - if there is no next element
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorNext ( const AlignAccessAlignmentEnumerator *self );


/* GetRefSeqID
 *  return id of reference sequence
 *
 *  "id_buffer" [ IN ] and "buffer_size" [ IN ] - return buffer for
 *  NUL terminated ASCII id
 *
 *  "id_size" [ OUT, NULL OKAY ] - optional return parameter for length
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetRefSeqID ( const AlignAccessAlignmentEnumerator *self,
    char *id_buffer, size_t buffer_size, size_t *id_size );


/* GetRefSeqPos
 *  return starting position of alignment on reference sequence
 *
 *  "start_pos" [ OUT ] - return parameter for starting position
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetRefSeqPos ( const AlignAccessAlignmentEnumerator *self,
    uint64_t *start_pos );
    
/* GetRefSeqPos
 *  return starting position of alignment on reference sequence
 *
 *  "start_pos" [ OUT ] - return parameter for starting position
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetRefSeqLen ( const AlignAccessAlignmentEnumerator *self,
                                                                 uint32_t *ref_len );

    
/* GetShortSeqID
 *  return accession id of aligned short sequence
 *
 *  "id_buffer" [ IN ] and "buffer_size" [ IN ] - return buffer for
 *  NUL terminated ASCII id
 *
 *  "id_size" [ OUT, NULL OKAY ] - optional return parameter for length
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetShortSeqAccessionID
    ( const AlignAccessAlignmentEnumerator *self, char *id_buffer, size_t buffer_size, size_t *id_size );

    
/* GetShortSeqID
 *  return id of aligned short sequence
 *
 *  "id_buffer" [ IN ] and "buffer_size" [ IN ] - return buffer for
 *  NUL terminated ASCII id
 *
 *  "id_size" [ OUT, NULL OKAY ] - optional return parameter for length
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetShortSeqID
    ( const AlignAccessAlignmentEnumerator *self, char *id_buffer, size_t buffer_size, size_t *id_size );


/* GetMapQuality
 *  return sequence alignment mapping quality score
 *
 *  "score" [ OUT ] - return parameter for mapping quality score (phred-like)
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetMapQuality ( const AlignAccessAlignmentEnumerator *self,
                                             uint8_t *score );
    
/* GetCIGAR
 *  return sequence alignment information
 *
 *  "start_pos" [ OUT, NULL OKAY ] - return parameter for starting position
 *
 *  "cigar_buffer" [ IN, NULL OKAY ] and "buffer_size" [ IN ] - return buffer for CIGAR NUL-terminated
 *  alignment description; null okay if cigar_size not null
 *  
 *  "cigar_size" [ OUT, NULL OKAY ] - optional return parameter for CIGAR string length; null ok
 *  if cigar_buffer not null
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetCIGAR ( const AlignAccessAlignmentEnumerator *self,
    uint64_t *start_pos, char *cigar_buffer, size_t buffer_size, size_t *cigar_size );


/* GetShortSequence
 *  return short sequence
 *
 *  "seq_buffer" [ IN ] and "buffer_size" [ IN ] - return buffer for short sequence
 *  
 *  "seq_size" [ OUT ] - return parameter for sequence length
 *
 * NB - BAM states that the sequence will be bases in FASTA.
 *  we will be extending this interface to provide type information
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetShortSequence ( const AlignAccessAlignmentEnumerator *self,
    char *seq_buffer, size_t buffer_size, size_t *seq_size );


/* GetStrandDirection
 *  returns property
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetStrandDirection
    ( const AlignAccessAlignmentEnumerator *self, AlignmentStrandDirection *result );

    
/* GetIsPaired
 *  returns paired property
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetIsPaired
    ( const AlignAccessAlignmentEnumerator *self, bool *result );

/* GetIsFirstInPair
 *  get property
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetIsFirstInPair
    ( const AlignAccessAlignmentEnumerator *self, bool *result );

/* GetIsSecondInPair
 *  get property
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetIsSecondInPair
    ( const AlignAccessAlignmentEnumerator *self, bool *result );

    
/* GetRecordID
 *  get property
 *
 * A locally unique identifier for each record.  When enumerators from the same
 * AlignAccessDB object fetch the same record, they will return the same ID.
 */
ALIGN_EXTERN rc_t CC AlignAccessAlignmentEnumeratorGetRecordID
    ( const AlignAccessAlignmentEnumerator *self, uint64_t *result );
    
#ifdef __cplusplus
}
#endif

#endif /* _h_align_align_access_ */
