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

#include "sra-exception.h"
#include "sra-object-priv.h"
#include "sra-string.h"

#include <klib/rc.h>

#include <assert.h>


/*--------------------------------------------------------------------------
 * SRAObject
 */

/* Whack
 */
static
void SRAObjectWhack ( SRAObject *self )
{
    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        ( * self -> vt -> v1 . whack ) ( self );
        break;
    }
}

/* Release
 *  NULL is handled without problems
 *  return value is always NULL
 */
LIB_EXPORT SRAObject* CC SRAObjectRelease ( const SRAObject *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "SRAObject" ) )
        {
        case krefWhack:
            SRAObjectWhack ( ( SRAObject* ) self );
            break;
        }
    }

    return NULL;
}

/* Duplicate
 */
LIB_EXPORT SRAObject* CC SRAObjectDuplicate ( const SRAObject *self, SRAException *x )
{
    if ( self != NULL )
    {
        assert ( x != NULL );

        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . dup ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* SliceByReference
 *  locate the reference by given spec
 *  create a sub-object that is isolated to a single reference
 */
LIB_EXPORT SRAObject* CC SRAObjectSliceByReference ( const SRAObject *self,
    SRAException *x, const SRAString *referenceSpec )
{
    rc_t rc;

    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcConstructing ) );
    else if ( referenceSpec == NULL )
    {
        rc = RC ( rcSRA, rcFile, rcConstructing, rcToken, rcNull );
        SRAExceptionError ( x, 0, rc, "SRAObject: bad referenceSpec" );
    }
    else if ( referenceSpec -> size == 0 )
    {
        rc = RC ( rcSRA, rcFile, rcConstructing, rcToken, rcEmpty );
        SRAExceptionError ( x, 0, rc, "SRAObject: bad referenceSpec" );
    }
    else if ( referenceSpec -> addr == NULL )
    {
        rc = RC ( rcSRA, rcFile, rcConstructing, rcToken, rcCorrupt );
        SRAExceptionError ( x, 0, rc, "SRAObject: bad referenceSpec" );
    }
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . slice_by_reference ) ( self, x, referenceSpec );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}

/* SliceByRange
 *  confined to a single reference slice
 *  create a sub-object that is isolated to a coordinate range upon a single reference
 */
LIB_EXPORT SRAObject* CC SRAObjectSliceByRange ( const SRAObject *self, SRAException *x,
    uint64_t reference_start, uint64_t reference_length, bool fully_contained )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcConstructing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . slice_by_range ) ( self, x, reference_start, reference_length, fully_contained );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* Open
 *  process all of the slicing and contatenation criteria
 *  internally create structures required
 */
LIB_EXPORT void CC SRAObjectOpen ( SRAObject *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcOpening ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            ( * self -> vt -> v1 . open ) ( self, x );
            break;
        default:
            SRAExceptionBadInterface ( x );
        }
    }
}

/* Close
 *  release all internal state
 *  TBD - does it become re-openable-ish?
 */
LIB_EXPORT void CC SRAObjectClose ( SRAObject *self )
{
    if ( self != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            ( * self -> vt -> v1 . close ) ( self );
            break;
        }
    }
}


/* GetReferenceIterator
 *  returns an iterator on all reference
 */
LIB_EXPORT struct SRA_Reference* CC SRAObjectGetReferenceIterator ( const SRAObject *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_reference_iterator ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* GetPrimaryAlignmentIterator
 *  returns an iterator on all primary alignments
 */
LIB_EXPORT struct SRA_Alignment* CC SRAObjectGetPrimaryAlignmentIterator ( const SRAObject *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_primary_alignment_iterator ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}

/* GetPrimaryAlignmentRangeIterator
 *  returns an iterator on selected primary alignments
 */
LIB_EXPORT struct SRA_Alignment* CC SRAObjectGetPrimaryAlignmentRangeIterator ( const SRAObject *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_primary_alignment_range_iterator ) ( self, x, startIdx, count );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* GetSecondaryAlignmentIterator
 *  returns an iterator on all secondary alignments
 */
LIB_EXPORT struct SRA_Alignment* CC SRAObjectGetSecondaryAlignmentIterator ( const SRAObject *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_secondary_alignment_iterator ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}

/* GetSecondaryAlignmentRangeIterator
 *  returns an iterator on selected secondary alignments
 */
LIB_EXPORT struct SRA_Alignment* CC SRAObjectGetSecondaryAlignmentRangeIterator ( const SRAObject *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_secondary_alignment_range_iterator ) ( self, x, startIdx, count );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* GetAlignmentIterator
 *  returns an iterator on all alignments
 */
LIB_EXPORT struct SRA_Alignment* CC SRAObjectGetAlignmentIterator ( const SRAObject *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_alignment_iterator ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* GetAlignedSequenceIterator
 *  returns an iterator on aligned sequences
 */
LIB_EXPORT struct SRA_Sequence* CC SRAObjectGetAlignedSequenceIterator ( const SRAObject *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_aligned_sequence_iterator ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}

/* GetAlignedSequenceRangeIterator
 *  returns an iterator on selected aligned sequences
 */
LIB_EXPORT struct SRA_Sequence* CC SRAObjectGetAlignedSequenceRangeIterator ( const SRAObject *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_aligned_sequence_range_iterator ) ( self, x, startIdx, count );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* GetUnalignedSequenceIterator
 *  returns an iterator on unaligned sequences
 */
LIB_EXPORT struct SRA_Sequence* CC SRAObjectGetUnalignedSequenceIterator ( const SRAObject *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_unaligned_sequence_iterator ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}

/* GetUnalignedSequenceRangeIterator
 *  returns an iterator on selected unaligned sequences
 */
LIB_EXPORT struct SRA_Sequence* CC SRAObjectGetUnalignedSequenceRangeIterator ( const SRAObject *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_unaligned_sequence_range_iterator ) ( self, x, startIdx, count );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* GetSequenceIterator
 *  returns an iterator on all sequences
 */
LIB_EXPORT struct SRA_Sequence* CC SRAObjectGetSequenceIterator ( const SRAObject *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_sequence_iterator ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* GetAlignedSpotIterator
 */
LIB_EXPORT struct SRA_Spot* CC SRAObjectGetAlignedSpotIterator ( const SRAObject *self,
    SRAException *x, bool partially_aligned )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_aligned_spot_iterator ) ( self, x, partially_aligned );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}

/* GetAlignedSpotRangeIterator
 */
LIB_EXPORT struct SRA_Spot* CC SRAObjectGetAlignedSpotRangeIterator ( const SRAObject *self,
    SRAException *x, uint64_t startIdx, uint64_t count, bool partially_aligned )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_aligned_spot_range_iterator ) ( self, x, startIdx, count, partially_aligned );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* GetUnalignedSpotIterator
 */
LIB_EXPORT struct SRA_Spot* CC SRAObjectGetUnalignedSpotIterator ( const SRAObject *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_unaligned_spot_iterator ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}

/* GetUnalignedSpotRangeIterator
 */
LIB_EXPORT struct SRA_Spot* CC SRAObjectGetUnalignedSpotRangeIterator ( const SRAObject *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_unaligned_spot_range_iterator ) ( self, x, startIdx, count );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* GetSpotIterator
 */
LIB_EXPORT struct SRA_Spot* CC SRAObjectGetSpotIterator ( const SRAObject *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_spot_iterator ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}

/* GetSpotRangeIterator
 */
LIB_EXPORT struct SRA_Spot* CC SRAObjectGetSpotRangeIterator ( const SRAObject *self,
    SRAException *x, uint64_t startIdx, uint64_t count )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcAccessing ) );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . get_spot_range_iterator ) ( self, x, startIdx, count );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* Init
 *  returns true on success
 */
bool SRAObjectInit ( SRAObject *self, SRAException *x, const SRAObject_vt *vt )
{
    rc_t rc;

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcFile, rcConstructing ) );
    else if ( vt == NULL )
    {
        rc = RC ( rcSRA, rcFile, rcConstructing, rcInterface, rcNull );
        SRAExceptionError ( x, 0, rc, "SRAObject: bad vTable" );
    }
    else
    {
        KRefcountInit ( & self -> refcount, 1, "SRAObject", "init", "" );
        self -> vt = vt;

        switch ( vt -> v1 . maj )
        {
        case 0:
            rc = RC ( rcSRA, rcFile, rcConstructing, rcInterface, rcInvalid );
            SRAExceptionError ( x, 0, rc, "SRAObject: uninitialized vTable" );
            break;
        case 1:
            switch ( vt -> v1 . min )
            {
            case 0:
#if _DEBUGGING
                if ( vt -> v1 . whack                                  == NULL ||
                     vt -> v1 . dup                                    == NULL ||
                     vt -> v1 . slice_by_reference                     == NULL ||
                     vt -> v1 . slice_by_range                         == NULL ||
                     vt -> v1 . open                                   == NULL ||
                     vt -> v1 . close                                  == NULL ||
                     vt -> v1 . get_reference_iterator                 == NULL ||
                     vt -> v1 . get_primary_alignment_iterator         == NULL ||
                     vt -> v1 . get_primary_alignment_range_iterator   == NULL ||
                     vt -> v1 . get_secondary_alignment_iterator       == NULL ||
                     vt -> v1 . get_secondary_alignment_range_iterator == NULL ||
                     vt -> v1 . get_alignment_iterator                 == NULL ||
                     vt -> v1 . get_aligned_sequence_iterator          == NULL ||
                     vt -> v1 . get_aligned_sequence_range_iterator    == NULL ||
                     vt -> v1 . get_unaligned_sequence_iterator        == NULL ||
                     vt -> v1 . get_unaligned_sequence_range_iterator  == NULL ||
                     vt -> v1 . get_sequence_iterator                  == NULL ||
                     vt -> v1 . get_aligned_spot_iterator              == NULL ||
                     vt -> v1 . get_aligned_spot_range_iterator        == NULL ||
                     vt -> v1 . get_unaligned_spot_iterator            == NULL ||
                     vt -> v1 . get_unaligned_spot_range_iterator      == NULL ||
                     vt -> v1 . get_spot_iterator                      == NULL ||
                     vt -> v1 . get_spot_range_iterator                == NULL )
                {
                    rc = RC ( rcSRA, rcFile, rcConstructing, rcInterface, rcCorrupt );
                    SRAExceptionError ( x, 0, rc, "SRAObject: uninitialized v1.0 vTable" );
                    break;
                }
#endif
                return true;
            default:
                rc = RC ( rcSRA, rcFile, rcConstructing, rcInterface, rcBadVersion );
                SRAExceptionError ( x, 0, rc, "SRAObject: unrecognized vTable version ( 1.%u )",
                                    vt -> v1 . min );
            }
            break;
        default:
            rc = RC ( rcSRA, rcFile, rcConstructing, rcInterface, rcBadVersion );
            SRAExceptionError ( x, 0, rc, "SRAObject: unrecognized vTable major version ( %u )",
                                vt -> v1 . maj );
        }
    }

    return false;
}

/* AddRef
 *  default behavior for Duplicate
 */
SRAObject* CC SRAObjectAddRef ( const SRAObject *self, SRAException *x )
{
    if ( self != NULL )
    {
        rc_t rc;

        switch ( KRefcountAdd ( & self -> refcount, "SRAObject" ) )
        {
        case krefLimit:
            rc = RC ( rcFS, rcFile, rcAttaching, rcRange, rcExcessive );
            break;
        case krefNegative:
            rc = RC ( rcFS, rcFile, rcAttaching, rcSelf, rcInvalid );
            break;
        default:
            return ( SRAObject* ) self;
        }

        assert ( x != NULL );
        SRAExceptionError ( x, 0, rc, "SRAObject: failed to duplicate" );
    }

    return NULL;
}
