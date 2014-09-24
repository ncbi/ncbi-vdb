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

#ifndef _h_simple_SRASpot_
#define _h_simple_SRASpot_

#ifndef _h_simple_extern_
#include <simple/extern.h>
#endif

#ifndef _h_simple_SRAString_
#include "sra-string.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct SRAException;
struct SRA_Sequence;


/*--------------------------------------------------------------------------
 * SRASpot
 */
typedef struct SRA_Spot SRASpot;


/* ToSequence
 *  casting operator to parent interface
 */
SIMPLE_EXTERN struct SRA_Sequence* CC SRASpotToSequence ( const SRASpot *self );


/* Duplicate
 *  separate from SRASequenceDuplicate
 *  may perform other operations, returns derived type
 */
SIMPLE_EXTERN SRASpot* CC SRASpotDuplicate ( const SRASpot *self, struct SRAException *x );


/* Platform
 *  copy out platform name in UTF-8 character set
 */
SIMPLE_EXTERN SRAString CC SRASpotPlatform ( const SRASpot *self, struct SRAException *x );

/* Group
 *  copy out spotgroup name in UTF-8 character set
 */
SIMPLE_EXTERN SRAString CC SRASpotGroup ( const SRASpot *self, struct SRAException *x );

/* Id
 *  spot id
 */
SIMPLE_EXTERN int64_t CC SRASpotId ( const SRASpot *self, struct SRAException *x );

/* NumReads
 *  number of reads in current spot
 */
SIMPLE_EXTERN uint32_t CC SRASpotNumReads ( const SRASpot *self, struct SRAException *x );

/* Reads
 *  returns the reads as a sequence iterator
 */
SIMPLE_EXTERN struct SRA_Sequence* CC SRASpotReads ( const SRASpot *self, struct SRAException *x );

/* NumBioReads
 *  number of biological reads in current spot
 */
SIMPLE_EXTERN uint32_t CC SRASpotNumBioReads ( const SRASpot *self, struct SRAException *x );

/* BioReads
 *  returns the biological reads as a sequence iterator
 *  if "trimmed" is true, apply quality and other cutoffs
 */
SIMPLE_EXTERN struct SRA_Sequence* CC SRASpotBioReads ( const SRASpot *self,
    struct SRAException *x, bool trimmed );


/*--------------------------------------------------------------------------
 * SRASpotIterator
 */

/* Duplicate
 */
SIMPLE_EXTERN SRASpot* CC SRASpotIteratorDuplicate ( const SRASpot *self, struct SRAException *x );

/* Next
 *  advance to the next spot
 *  returns false if no more are available
 */
SIMPLE_EXTERN bool CC SRASpotIteratorNext ( SRASpot *self, struct SRAException *x );


#ifdef __cplusplus
}
#endif

#endif /* _h_simple_SRASpot_ */
