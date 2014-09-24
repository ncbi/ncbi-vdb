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

#include <simple/extern.h>

typedef struct SRATableObj SRATableObj;
#define SRAOBJECT SRATableObj
#include "vdb-sra.h"
#include "vdb-spot.h"

#include "sra-exception.h"
#include "sra-memmgr.h"
#include "sra-string.h"
#include "sra-spot.h"
#include "sra-object-priv.h"

#include <vdb/table.h>
#include <vdb/cursor.h>
#include <klib/rc.h>

#include <stdlib.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * SRAException
 */
static
void *SRAException_BadType ( SRAException *self, const SRASourceLoc *loc, const char *func, uint32_t lineno )
{
    rc_t rc = RC ( rcSRA, rcTable, rcConstructing, rcType, rcIncorrect );
    assert ( self != NULL );
    SRAException_Error ( self, loc, func, lineno, 0, rc, "simple sequence table has no reference alignment" );
    return NULL;
}

#define SRAExceptionBadType( self ) \
    SRAException_BadType ( self, & s_loc, __func__, __LINE__ )


/*--------------------------------------------------------------------------
 * SRATableObj
 */
struct SRATableObj
{
    SRAObject dad;
    const VTable *tbl;
};


/* Whack
 */
static
void SRATableObjWhack ( SRATableObj *self )
{
    VTableRelease ( self -> tbl );
    SRAMemMgrFree ( g_mmgr, self, sizeof * self );
}


/* SliceByReference
 *  locate the reference by given spec
 *  create a sub-object that is isolated to a single reference
 */
static
SRATableObj* CC SRATableObjSliceByReference ( const SRATableObj *self,
    SRAException *x, const SRAString *referenceSpec )
{
    return SRAExceptionBadType ( x );
}

/* SliceByRange
 *  confined to a single reference slice
 *  create a sub-object that is isolated to a coordinate range upon a single reference
 */
static
SRATableObj* CC SRATableObjSliceByRange ( const SRATableObj *self, SRAException *x,
    uint64_t reference_start, uint64_t reference_length, bool fully_contained )
{
    return SRAExceptionBadType ( x );
}


/* Open
 *  process all of the slicing and contatenation criteria
 *  internally create structures required
 */
static
void CC SRATableObjOpen ( SRATableObj *self, SRAException *x )
{
}

/* Close
 *  release all internal state
 *  TBD - does it become re-openable-ish?
 */
static
void CC SRATableObjClose ( SRATableObj *self )
{
}


/* GetReferenceIterator
 *  returns an iterator on all reference
 */
static
struct SRA_Reference* CC SRATableObjGetReferenceIterator ( const SRATableObj *self, SRAException *x )
{
    return SRAExceptionBadType ( x );
}


/* GetPrimaryAlignmentIterator
 *  returns an iterator on all primary alignments
 */
static
struct SRA_Alignment* CC SRATableObjGetPrimaryAlignmentIterator ( const SRATableObj *self, SRAException *x )
{
    return SRAExceptionBadType ( x );
}

/* GetPrimaryAlignmentRangeIterator
 *  returns an iterator on selected primary alignments
 */
static
struct SRA_Alignment* CC SRATableObjGetPrimaryAlignmentRangeIterator ( const SRATableObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    return SRAExceptionBadType ( x );
}


/* GetSecondaryAlignmentIterator
 *  returns an iterator on all secondary alignments
 */
static
struct SRA_Alignment* CC SRATableObjGetSecondaryAlignmentIterator ( const SRATableObj *self, SRAException *x )
{
    return SRAExceptionBadType ( x );
}

/* GetSecondaryAlignmentRangeIterator
 *  returns an iterator on selected secondary alignments
 */
static
struct SRA_Alignment* CC SRATableObjGetSecondaryAlignmentRangeIterator ( const SRATableObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    return SRAExceptionBadType ( x );
}


/* GetAlignmentIterator
 *  returns an iterator on all alignments
 */
static
struct SRA_Alignment* CC SRATableObjGetAlignmentIterator ( const SRATableObj *self, SRAException *x )
{
    return SRAExceptionBadType ( x );
}


/* GetAlignedSequenceIterator
 *  returns an iterator on aligned sequences
 */
static
struct SRA_Sequence* CC SRATableObjGetAlignedSequenceIterator ( const SRATableObj *self, SRAException *x )
{
    return SRAExceptionBadType ( x );
}

/* GetAlignedSequenceRangeIterator
 *  returns an iterator on selected aligned sequences
 */
static
struct SRA_Sequence* CC SRATableObjGetAlignedSequenceRangeIterator ( const SRATableObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    return SRAExceptionBadType ( x );
}


/* GetUnalignedSequenceIterator
 *  returns an iterator on unaligned sequences
 */
static
struct SRA_Sequence* CC SRATableObjGetUnalignedSequenceIterator ( const SRATableObj *self, SRAException *x )
{
    return SRAExceptionBadType ( x );
}

/* GetUnalignedSequenceRangeIterator
 *  returns an iterator on selected unaligned sequences
 */
static
struct SRA_Sequence* CC SRATableObjGetUnalignedSequenceRangeIterator ( const SRATableObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    return SRAExceptionBadType ( x );
}


/* GetSequenceIterator
 *  returns an iterator on all sequences
 */
static
struct SRA_Sequence* CC SRATableObjGetSequenceIterator ( const SRATableObj *self, SRAException *x )
{
    return SRAExceptionBadType ( x );
}


/* GetAlignedSpotIterator
 */
static
SRASpot* CC SRATableObjGetAlignedSpotIterator ( const SRATableObj *self,
    SRAException *x, bool partially_aligned )
{
    return SRAExceptionBadType ( x );
}

/* GetAlignedSpotRangeIterator
 */
static
SRASpot* CC SRATableObjGetAlignedSpotRangeIterator ( const SRATableObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count, bool partially_aligned )
{
    return SRAExceptionBadType ( x );
}


/* GetSpotIterator
 */
static
SRASpot* CC SRATableObjGetSpotIterator ( const SRATableObj *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcTable, rcAccessing ) );
    else
    {
        const VCursor *curs;
        rc_t rc = VTableCreateCursorRead ( self -> tbl, & curs );
        if ( rc != 0 )
            SRAExceptionError ( x, 0, rc, "failed to create cursor" );
        else
        {
            SRASpot *it = VDBSpotMakeIterator ( x, curs );
            VCursorRelease ( curs );

            if ( SRAExceptionOK ( x ) )
                return it;
        }
    }

    return NULL;
}

/* GetSpotRangeIterator
 */
static
SRASpot* CC SRATableObjGetSpotRangeIterator ( const SRATableObj *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcTable, rcAccessing ) );
    else
    {
        const VCursor *curs;
        rc_t rc = VTableCreateCursorRead ( self -> tbl, & curs );
        if ( rc != 0 )
            SRAExceptionError ( x, 0, rc, "failed to create cursor" );
        else
        {
            SRASpot *it = VDBSpotMakeRangeIterator ( x, curs, startIdx, count );
            VCursorRelease ( curs );

            if ( SRAExceptionOK ( x ) )
                return it;
        }
    }

    return NULL;
}

/* Make
 */
static SRAObject_vt_v1 SRATableObj_vt =
{
    1, 0,

    /* BEGIN MINOR VERSION 0 */

    SRATableObjWhack,
    SRAObjectAddRef,
    SRATableObjSliceByReference,
    SRATableObjSliceByRange,
    SRATableObjOpen,
    SRATableObjClose,
    SRATableObjGetReferenceIterator,
    SRATableObjGetPrimaryAlignmentIterator,
    SRATableObjGetPrimaryAlignmentRangeIterator,
    SRATableObjGetSecondaryAlignmentIterator,
    SRATableObjGetSecondaryAlignmentRangeIterator,
    SRATableObjGetAlignmentIterator,
    SRATableObjGetAlignedSequenceIterator,
    SRATableObjGetAlignedSequenceRangeIterator,
    SRATableObjGetUnalignedSequenceIterator,
    SRATableObjGetUnalignedSequenceRangeIterator,
    SRATableObjGetSequenceIterator,
    SRATableObjGetAlignedSpotIterator,
    SRATableObjGetAlignedSpotRangeIterator,

    /* treat unaligned as entire table */
    SRATableObjGetSpotIterator,
    SRATableObjGetSpotRangeIterator,

    SRATableObjGetSpotIterator,
    SRATableObjGetSpotRangeIterator

    /* END MINOR VERSION 0 */

};

SRAObject *SRATableObjMake ( SRAException *x, const VTable *tbl )
{
    rc_t rc;
    SRATableObj *o;

    assert ( x != NULL );
    assert ( tbl != NULL );

    o = SRAMemMgrAlloc ( g_mmgr, x, sizeof * o, true );
    if ( o != NULL )
    {
        if ( SRAObjectInit ( & o -> dad, x, ( const SRAObject_vt* ) & SRATableObj_vt ) )
        {
            rc = VTableAddRef ( tbl );
            if ( rc == 0 )
            {
                o -> tbl = tbl;
                return & o -> dad;
            }

            SRAExceptionError ( x, 0, rc, "failed to attach to VTable" );
        }

        SRAMemMgrFree ( g_mmgr, o, sizeof * o );
    }

    return NULL;
}
