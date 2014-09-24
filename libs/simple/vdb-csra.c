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


/* NOTES

   a cSRA object needs to support a model for its REFERENCE table,
   another for its *_ALIGNMENT tables, it will borrow the spot
   implementations for its SEQUENCE table, but will use a separate
   implementation for the reads in the *_ALIGNMENT tables.

   at this particular level, the object-spec has been shown to have
   a hierarchy of VDB:database:csra. From here, the behavior is to
   produce iterators of Reference, Alignment, Sequence, and Spot.

 */

#include <simple/extern.h>

typedef struct cSRAObj cSRAObj;
#define SRAOBJECT cSRAObj
#include "vdb-database.h"
#include "vdb-spot.h"

#include "sra-exception.h"
#include "sra-memmgr.h"
#include "sra-string.h"
#include "sra-spot.h"
#include "sra-object-priv.h"

#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <klib/rc.h>

#include <stdlib.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * cSRAObj
 */
struct cSRAObj
{
    SRAObject dad;
    const VDatabase *db;
};


/* Whack
 */
static
void cSRAObjWhack ( cSRAObj *self )
{
    VDatabaseRelease ( self -> db );
    SRAMemMgrFree ( g_mmgr, self, sizeof * self );
}


/* SliceByReference
 *  locate the reference by given spec
 *  create a sub-object that is isolated to a single reference
 */
static
cSRAObj* CC cSRAObjSliceByReference ( const cSRAObj *self,
    SRAException *x, const SRAString *referenceSpec )
{
}

/* SliceByRange
 *  confined to a single reference slice
 *  create a sub-object that is isolated to a coordinate range upon a single reference
 */
static
cSRAObj* CC cSRAObjSliceByRange ( const cSRAObj *self, SRAException *x,
    uint64_t reference_start, uint64_t reference_length, bool fully_contained )
{
}


/* Open
 *  process all of the slicing and contatenation criteria
 *  internally create structures required
 */
static
void CC cSRAObjOpen ( cSRAObj *self, SRAException *x )
{
}

/* Close
 *  release all internal state
 *  TBD - does it become re-openable-ish?
 */
static
void CC cSRAObjClose ( cSRAObj *self )
{
}


/* GetReferenceIterator
 *  returns an iterator on all reference
 */
static
struct SRA_Reference* CC cSRAObjGetReferenceIterator ( const cSRAObj *self, SRAException *x )
{
}


/* GetPrimaryAlignmentIterator
 *  returns an iterator on all primary alignments
 */
static
struct SRA_Alignment* CC cSRAObjGetPrimaryAlignmentIterator ( const cSRAObj *self, SRAException *x )
{
}

/* GetPrimaryAlignmentRangeIterator
 *  returns an iterator on selected primary alignments
 */
static
struct SRA_Alignment* CC cSRAObjGetPrimaryAlignmentRangeIterator ( const cSRAObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
}


/* GetSecondaryAlignmentIterator
 *  returns an iterator on all secondary alignments
 */
static
struct SRA_Alignment* CC cSRAObjGetSecondaryAlignmentIterator ( const cSRAObj *self, SRAException *x )
{
}

/* GetSecondaryAlignmentRangeIterator
 *  returns an iterator on selected secondary alignments
 */
static
struct SRA_Alignment* CC cSRAObjGetSecondaryAlignmentRangeIterator ( const cSRAObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
}


/* GetAlignmentIterator
 *  returns an iterator on all alignments
 */
static
struct SRA_Alignment* CC cSRAObjGetAlignmentIterator ( const cSRAObj *self, SRAException *x )
{
}


/* GetAlignedSequenceIterator
 *  returns an iterator on aligned sequences
 */
static
struct SRA_Sequence* CC cSRAObjGetAlignedSequenceIterator ( const cSRAObj *self, SRAException *x )
{
}

/* GetAlignedSequenceRangeIterator
 *  returns an iterator on selected aligned sequences
 */
static
struct SRA_Sequence* CC cSRAObjGetAlignedSequenceRangeIterator ( const cSRAObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
}


/* GetUnalignedSequenceIterator
 *  returns an iterator on unaligned sequences
 */
static
struct SRA_Sequence* CC cSRAObjGetUnalignedSequenceIterator ( const cSRAObj *self, SRAException *x )
{
}

/* GetUnalignedSequenceRangeIterator
 *  returns an iterator on selected unaligned sequences
 */
static
struct SRA_Sequence* CC cSRAObjGetUnalignedSequenceRangeIterator ( const cSRAObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
}


/* GetSequenceIterator
 *  returns an iterator on all sequences
 */
static
struct SRA_Sequence* CC cSRAObjGetSequenceIterator ( const cSRAObj *self, SRAException *x )
{
}


/* GetAlignedSpotIterator
 */
static
SRASpot* CC cSRAObjGetAlignedSpotIterator ( const cSRAObj *self,
    SRAException *x, bool partially_aligned )
{
}

/* GetAlignedSpotRangeIterator
 */
static
SRASpot* CC cSRAObjGetAlignedSpotRangeIterator ( const cSRAObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count, bool partially_aligned )
{
}


/* GetSpotIterator
 */
static
SRASpot* CC cSRAObjGetSpotIterator ( const cSRAObj *self, SRAException *x )
{
}

/* GetSpotRangeIterator
 */
static
SRASpot* CC cSRAObjGetSpotRangeIterator ( const cSRAObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
}

/* Make
 */
static SRAObject_vt_v1 cSRAObj_vt =
{
    1, 0,

    /* BEGIN MINOR VERSION 0 */

    cSRAObjWhack,
    SRAObjectAddRef,
    cSRAObjSliceByReference,
    cSRAObjSliceByRange,
    cSRAObjOpen,
    cSRAObjClose,
    cSRAObjGetReferenceIterator,
    cSRAObjGetPrimaryAlignmentIterator,
    cSRAObjGetPrimaryAlignmentRangeIterator,
    cSRAObjGetSecondaryAlignmentIterator,
    cSRAObjGetSecondaryAlignmentRangeIterator,
    cSRAObjGetAlignmentIterator,
    cSRAObjGetAlignedSequenceIterator,
    cSRAObjGetAlignedSequenceRangeIterator,
    cSRAObjGetUnalignedSequenceIterator,
    cSRAObjGetUnalignedSequenceRangeIterator,
    cSRAObjGetSequenceIterator,
    cSRAObjGetAlignedSpotIterator,
    cSRAObjGetAlignedSpotRangeIterator,

    /* treat unaligned as entire table */
    cSRAObjGetSpotIterator,
    cSRAObjGetSpotRangeIterator,

    cSRAObjGetSpotIterator,
    cSRAObjGetSpotRangeIterator

    /* END MINOR VERSION 0 */

};

SRAObject *cSRAObjMake ( SRAException *x, const VDatabase *db )
{
    rc_t rc;
    cSRAObj *o;

    assert ( x != NULL );
    assert ( db != NULL );

    o = SRAMemMgrAlloc ( g_mmgr, x, sizeof * o, true );
    if ( o != NULL )
    {
        if ( SRAObjectInit ( & o -> dad, x, ( const SRAObject_vt* ) & cSRAObj_vt ) )
        {
            rc = VDatabaseAddRef ( db );
            if ( rc == 0 )
            {
                o -> db = db;
                return & o -> dad;
            }

            SRAExceptionError ( x, 0, rc, "failed to attach to VDatabase" );
        }

        SRAMemMgrFree ( g_mmgr, o, sizeof * o );
    }

    return NULL;
}
