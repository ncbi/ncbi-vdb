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

#ifndef _h_simple_SRAObject_
#define _h_simple_SRAObject_

#ifndef _h_simple_extern_
#include <simple/extern.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct SRA_Spot;
struct SRAString;
struct SRAException;
struct SRA_Reference;
struct SRA_Alignment;
struct SRA_Sequence;

/*--------------------------------------------------------------------------
 * SRAObject
 */
typedef struct SRA_Object SRAObject;


/* Release
 *  NULL is handled without problems
 *  return value is always NULL
 */
SIMPLE_EXTERN SRAObject* CC SRAObjectRelease ( const SRAObject *self );

/* Duplicate
 */
SIMPLE_EXTERN SRAObject* CC SRAObjectDuplicate ( const SRAObject *self, struct SRAException *x );


/* MakeFromSpec
 *  uses "spec" to locate and open object
 *  creates an SRAObject from it
 *  uses static internal manager to allocate return object
 */
SIMPLE_EXTERN SRAObject* CC SRAObjectMakeFromSpec ( struct SRAException *x, struct SRAString const *spec );

/* MakeConcatenation
 *  creates an SRAObject from the concatenation of two existing objects
 *  uses static internal manager to allocate return object
 */
SIMPLE_EXTERN SRAObject* CC SRAObjectMakeConcatenation ( struct SRAException *x,
    const SRAObject *a, const SRAObject *b );


/* SliceByReference
 *  locate the reference by given spec
 *  create a sub-object that is isolated to a single reference
 */
SIMPLE_EXTERN SRAObject* CC SRAObjectSliceByReference ( const SRAObject *self,
    struct SRAException *x, struct SRAString const *referenceSpec );

/* SliceByRange
 *  confined to a single reference slice
 *  create a sub-object that is isolated to a coordinate range upon a single reference
 */
SIMPLE_EXTERN SRAObject* CC SRAObjectSliceByRange ( const SRAObject *self, struct SRAException *x,
    uint64_t reference_start, uint64_t reference_length, bool fully_contained );


/* Open
 *  process all of the slicing and contatenation criteria
 *  internally create structures required
 */
SIMPLE_EXTERN void CC SRAObjectOpen ( SRAObject *self, struct SRAException *x );

/* Close
 *  release all internal state
 *  TBD - does it become re-openable-ish?
 */
SIMPLE_EXTERN void CC SRAObjectClose ( SRAObject *self );


/* GetReferenceIterator
 *  returns an iterator on all reference
 */
SIMPLE_EXTERN struct SRA_Reference* CC SRAObjectGetReferenceIterator ( const SRAObject *self, struct SRAException *x );


/* GetPrimaryAlignmentIterator
 *  returns an iterator on all primary alignments
 */
SIMPLE_EXTERN struct SRA_Alignment* CC SRAObjectGetPrimaryAlignmentIterator ( const SRAObject *self, struct SRAException *x );

/* GetPrimaryAlignmentRangeIterator
 *  returns an iterator on selected primary alignments
 */
SIMPLE_EXTERN struct SRA_Alignment* CC SRAObjectGetPrimaryAlignmentRangeIterator ( const SRAObject *self,
    struct SRAException *x, uint64_t startIdx, uint64_t count );


/* GetSecondaryAlignmentIterator
 *  returns an iterator on all secondary alignments
 */
SIMPLE_EXTERN struct SRA_Alignment* CC SRAObjectGetSecondaryAlignmentIterator ( const SRAObject *self, struct SRAException *x );

/* GetSecondaryAlignmentRangeIterator
 *  returns an iterator on selected secondary alignments
 */
SIMPLE_EXTERN struct SRA_Alignment* CC SRAObjectGetSecondaryAlignmentRangeIterator ( const SRAObject *self,
    struct SRAException *x, uint64_t startIdx, uint64_t count );


/* GetAlignmentIterator
 *  returns an iterator on all alignments
 */
SIMPLE_EXTERN struct SRA_Alignment* CC SRAObjectGetAlignmentIterator ( const SRAObject *self, struct SRAException *x );


/* GetAlignedSequenceIterator
 *  returns an iterator on aligned sequences
 */
SIMPLE_EXTERN struct SRA_Sequence* CC SRAObjectGetAlignedSequenceIterator ( const SRAObject *self, struct SRAException *x );

/* GetAlignedSequenceRangeIterator
 *  returns an iterator on selected aligned sequences
 */
SIMPLE_EXTERN struct SRA_Sequence* CC SRAObjectGetAlignedSequenceRangeIterator ( const SRAObject *self,
    struct SRAException *x, uint64_t startIdx, uint64_t count );


/* GetUnalignedSequenceIterator
 *  returns an iterator on unaligned sequences
 */
SIMPLE_EXTERN struct SRA_Sequence* CC SRAObjectGetUnalignedSequenceIterator ( const SRAObject *self, struct SRAException *x );

/* GetUnalignedSequenceRangeIterator
 *  returns an iterator on selected unaligned sequences
 */
SIMPLE_EXTERN struct SRA_Sequence* CC SRAObjectGetUnalignedSequenceRangeIterator ( const SRAObject *self,
    struct SRAException *x, uint64_t startIdx, uint64_t count );


/* GetSequenceIterator
 *  returns an iterator on all sequences
 */
SIMPLE_EXTERN struct SRA_Sequence* CC SRAObjectGetSequenceIterator ( const SRAObject *self, struct SRAException *x );


/* GetAlignedSpotIterator
 */
SIMPLE_EXTERN struct SRA_Spot* CC SRAObjectGetAlignedSpotIterator ( const SRAObject *self,
    struct SRAException *x, bool partially_aligned );

/* GetAlignedSpotRangeIterator
 */
SIMPLE_EXTERN struct SRA_Spot* CC SRAObjectGetAlignedSpotRangeIterator ( const SRAObject *self,
    struct SRAException *x, uint64_t startIdx, uint64_t count, bool partially_aligned );


/* GetUnalignedSpotIterator
 */
SIMPLE_EXTERN struct SRA_Spot* CC SRAObjectGetUnalignedSpotIterator ( const SRAObject *self, struct SRAException *x );

/* GetUnalignedSpotRangeIterator
 */
SIMPLE_EXTERN struct SRA_Spot* CC SRAObjectGetUnalignedSpotRangeIterator ( const SRAObject *self,
    struct SRAException *x, uint64_t startIdx, uint64_t count );


/* GetSpotIterator
 */
SIMPLE_EXTERN struct SRA_Spot* CC SRAObjectGetSpotIterator ( const SRAObject *self, struct SRAException *x );

/* GetSpotRangeIterator
 */
SIMPLE_EXTERN struct SRA_Spot* CC SRAObjectGetSpotRangeIterator ( const SRAObject *self,
    struct SRAException *x, uint64_t startIdx, uint64_t count );


#ifdef __cplusplus
}
#endif

#endif /* _h_simple_SRAObject_ */
