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

#ifndef _h_simple_SRASequence_
#define _h_simple_SRASequence_

#ifndef _h_simple_extern_
#include <simple/extern.h>
#endif

#ifndef _h_simple_SRAString_
#include "sra-string.h"
#endif

#ifndef _h_simple_SRAMem_
#include "sra-mem.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct SRAException;
struct SRA_Alignment;


/*--------------------------------------------------------------------------
 * SRASequence
 */
typedef struct SRA_Sequence SRASequence;


/* Release
 *  NULL is handled without problems
 *  return value is always NULL
 */
SIMPLE_EXTERN SRASequence* CC SRASequenceRelease ( const SRASequence *self );

/* Duplicate
 */
SIMPLE_EXTERN SRASequence* CC SRASequenceDuplicate ( const SRASequence *self, struct SRAException *x );


/* Name
 *  copy out name in UTF-8 character set
 */
SIMPLE_EXTERN SRAString CC SRASequenceName ( const SRASequence *self, struct SRAException *x );

/* IsTechnical
 */
SIMPLE_EXTERN bool CC SRASequenceIsTechnical ( const SRASequence *self, struct SRAException *x );

/* IsReverse
 */
SIMPLE_EXTERN bool CC SRASequenceIsReverse ( const SRASequence *self, struct SRAException *x );

/* Length
 *  the TOTAL length of the sequence
 */
SIMPLE_EXTERN uint64_t CC SRASequenceLength ( const SRASequence *self, struct SRAException *x );

/* Bases
 *  return a pointer to the base starting at offset
 *  with the maximum contiguous bases from that pointer
 *  for chunked sequences, this may only be the length of
 *  the remaining bases within the chunk, but may not give
 *  all of the bases in the sequence.
 */
SIMPLE_EXTERN SRAString CC SRASequenceBases ( const SRASequence *self,
    struct SRAException *x, uint64_t offset, uint64_t *remaining );

/* ColorSpaceNative
 *  returns true if the native sequence is in color-space
 */
SIMPLE_EXTERN bool CC SRASequenceColorSpaceNative ( const SRASequence *self, struct SRAException *x );

/* HasColorSpace
 *  returns true if sequence can return color-space
 */
SIMPLE_EXTERN bool CC SRASequenceHasColorSpace ( const SRASequence *self );

/* ColorSpaceKey
 *  return a base used as color-space key
 *  only relevant for first read
 */
SIMPLE_EXTERN char CC SRASequenceColorSpaceKey ( const SRASequence *self, struct SRAException *x );

/* ColorSpace
 *  return a pointer to the color starting at offset
 *  with the maximum contiguous colors from that pointer
 *  for chunked sequences, this may only be the length of
 *  the remaining colors within the chunk, but may not give
 *  all of the colors in the sequence.
 */
SIMPLE_EXTERN SRAString CC SRASequenceColorSpace ( const SRASequence *self,
    struct SRAException *x, uint64_t offset, uint64_t *remaining );

/* Qualities
 *  return a pointer to the binary phred qualities starting
 *  at offset with the maximum contiguous qualities from that
 *  pointer for chunked sequences, this may only be the length
 *  of the remaining qualities within the chunk, but may not give
 *  all of the qualities in the sequence.
 *
 *  NB - generally it will be advantageous to retrieve bases
 *  and qualities together
 */
SIMPLE_EXTERN SRAMem CC SRASequenceQualities ( const SRASequence *self,
    struct SRAException *x, uint64_t offset, uint64_t *remaining );

/* HasPrimaryAlignment
 *  returns true iff any part of sequence is aligned,
 *  and the sequence is capable of returning an SRAAlignment
 */
SIMPLE_EXTERN bool CC SRASequenceHasPrimaryAlignment ( const SRASequence *self );

/* PrimaryAlignment
 *  attempt to create a primary alignment object
 */
SIMPLE_EXTERN struct SRA_Alignment* CC SRASequencePrimaryAlignment ( const SRASequence *self,
    struct SRAException *x );


/*--------------------------------------------------------------------------
 * SRASequenceIterator
 */

/* Duplicate
 */
SIMPLE_EXTERN SRASequence* CC SRASequenceIteratorDuplicate ( const SRASequence *self, struct SRAException *x );

/* Next
 *  advance to the next sequence
 *  returns false if no more sequences are available
 */
SIMPLE_EXTERN bool CC SRASequenceIteratorNext ( SRASequence *self, struct SRAException *x );


#ifdef __cplusplus
}
#endif

#endif /* _h_simple_SRASequence_ */
