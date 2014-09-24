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
#include "sra-sequence-priv.h"

#include <klib/rc.h>

#include <assert.h>


/*--------------------------------------------------------------------------
 * SRASequence
 */

/* Whack
 */
static
void SRASequenceWhack ( SRASequence *self )
{
    const SRASequence_vt *vt = self -> mvt -> seq;
    switch ( vt -> v1 . maj )
    {
    case 1:
        ( * vt -> v1 . whack ) ( self );
        break;
    }
}

/* Release
 *  NULL is handled without problems
 *  return value is always NULL
 */
LIB_EXPORT SRASequence* CC SRASequenceRelease ( const SRASequence *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "SRASequence" ) )
        {
        case krefWhack:
            SRASequenceWhack ( ( SRASequence* ) self );
            break;
        }
    }

    return NULL;
}

/* Duplicate
 */
LIB_EXPORT SRASequence* CC SRASequenceDuplicate ( const SRASequence *self, SRAException *x )
{
    if ( self != NULL )
    {
        const SRASequence_vt *vt = self -> mvt -> seq;

        assert ( x != NULL );

        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . dup ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/* Name
 *  copy out name in UTF-8 character set
 */
LIB_EXPORT SRAString CC SRASequenceName ( const SRASequence *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASequence_vt *vt = self -> mvt -> seq;
        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . name ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return SRAStringMake ( NULL, "", 0 );
}

/* IsTechnical
 */
LIB_EXPORT bool CC SRASequenceIsTechnical ( const SRASequence *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASequence_vt *vt = self -> mvt -> seq;
        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . is_technical ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return false;
}

/* IsReverse
 */
LIB_EXPORT bool CC SRASequenceIsReverse ( const SRASequence *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASequence_vt *vt = self -> mvt -> seq;
        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . is_reverse ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return false;
}

/* Length
 *  the TOTAL length of the sequence
 */
LIB_EXPORT uint64_t CC SRASequenceLength ( const SRASequence *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASequence_vt *vt = self -> mvt -> seq;
        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . length ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return 0;
}

/* Bases
 *  return a pointer to the base starting at offset
 *  with the maximum contiguous bases from that pointer
 *  for chunked sequences, this may only be the length of
 *  the remaining bases within the chunk, but may not give
 *  all of the bases in the sequence.
 */
LIB_EXPORT SRAString CC SRASequenceBases ( const SRASequence *self,
    SRAException *x, uint64_t offset, uint64_t *remaining )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        uint64_t dummy;
        const SRASequence_vt *vt = self -> mvt -> seq;

        if ( remaining == NULL )
            remaining = & dummy;

        * remaining = 0;

        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . bases ) ( self, x, offset, remaining );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return SRAStringMake ( NULL, "", 0 );
}

/* ColorSpaceNative
 *  returns true if the native sequence is in color-space
 */
LIB_EXPORT bool CC SRASequenceColorSpaceNative ( const SRASequence *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASequence_vt *vt = self -> mvt -> seq;
        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . color_space_native ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return false;
}

/* HasColorSpace
 *  returns true if sequence can return color-space
 */
LIB_EXPORT bool CC SRASequenceHasColorSpace ( const SRASequence *self )
{
    if ( self != NULL )
    {
        const SRASequence_vt *vt = self -> mvt -> seq;
        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . has_color_space ) ( self );
        }
    }

    return false;
}

/* ColorSpaceKey
 *  return a base used as color-space key
 *  only relevant for first read
 */
LIB_EXPORT char CC SRASequenceColorSpaceKey ( const SRASequence *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASequence_vt *vt = self -> mvt -> seq;
        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . color_space_key ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return 0;
}

/* ColorSpace
 *  return a pointer to the color starting at offset
 *  with the maximum contiguous colors from that pointer
 *  for chunked sequences, this may only be the length of
 *  the remaining colors within the chunk, but may not give
 *  all of the colors in the sequence.
 */
LIB_EXPORT SRAString CC SRASequenceColorSpace ( const SRASequence *self,
    SRAException *x, uint64_t offset, uint64_t *remaining )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        uint64_t dummy;
        const SRASequence_vt *vt = self -> mvt -> seq;

        if ( remaining == NULL )
            remaining = & dummy;

        * remaining = 0;

        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . color_space ) ( self, x, offset, remaining );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return SRAStringMake ( NULL, "", 0 );
}

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
LIB_EXPORT SRAMem CC SRASequenceQualities ( const SRASequence *self,
    SRAException *x, uint64_t offset, uint64_t *remaining )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        uint64_t dummy;
        const SRASequence_vt *vt = self -> mvt -> seq;

        if ( remaining == NULL )
            remaining = & dummy;

        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . qualities ) ( self, x, offset, remaining );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return SRAMemMake ( NULL, "", 0 );
}

/* HasPrimaryAlignment
 *  returns true iff any part of sequence is aligned,
 *  and the sequence is capable of returning an SRAAlignment
 */
LIB_EXPORT bool CC SRASequenceHasPrimaryAlignment ( const SRASequence *self )
{
    if ( self != NULL )
    {
        const SRASequence_vt *vt = self -> mvt -> seq;
        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . has_primary_alignment ) ( self );
        }
    }

    return false;
}

/* PrimaryAlignment
 *  attempt to create a primary alignment object
 */
LIB_EXPORT struct SRA_Alignment* CC SRASequencePrimaryAlignment ( const SRASequence *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASequence_vt *vt = self -> mvt -> seq;
        switch ( vt -> v1 . maj )
        {
        case 1:
            return ( * vt -> v1 . primary_alignment ) ( self, x );
        default:
            SRAExceptionBadInterface ( x );
        }
    }

    return NULL;
}


/*--------------------------------------------------------------------------
 * SRASequenceIterator
 */

/* Duplicate
 */
LIB_EXPORT SRASequence* CC SRASequenceIteratorDuplicate ( const SRASequence *self, SRAException *x )
{
    if ( self != NULL )
    {
        const SRASequenceIterator_vt *vt = self -> mvt -> it;

        assert ( x != NULL );

        if ( vt != NULL )
        {
            switch ( vt -> v1 . maj )
            {
            case 1:
                return ( * vt -> v1 . dup ) ( self, x );
            }
        }

        SRAExceptionBadInterface ( x );
    }

    return NULL;
}

/* Next
 *  advance to the next sequence
 *  returns false if no more sequences are available
 */
LIB_EXPORT bool CC SRASequenceIteratorNext ( SRASequence *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcPositioning ) );
    else
    {
        const SRASequenceIterator_vt *vt = self -> mvt -> it;
        if ( vt != NULL )
        {
            switch ( vt -> v1 . maj )
            {
            case 1:
                return ( * vt -> v1 . next ) ( self, x );
            }
        }

        SRAExceptionBadInterface ( x );
    }

    return false;
}


/* Init
 *  returns true on success
 */
bool SRASequenceInit ( SRASequence *self, SRAException *x, const SRASequence_mvt *mvt )
{
    rc_t rc;

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcConstructing ) );
    else if ( mvt == NULL || mvt -> seq == NULL )
    {
        rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcNull );
        SRAExceptionError ( x, 0, rc, "SRASequence: bad vTable" );
    }
    else
    {
        const SRASequence_vt *seq = mvt -> seq;
        const SRASequenceIterator_vt *it = mvt -> it;

        KRefcountInit ( & self -> refcount, 1, "SRASequence", "init", "" );
        self -> mvt = mvt;

        switch ( seq -> v1 . maj )
        {
        case 0:
            rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcInvalid );
            SRAExceptionError ( x, 0, rc, "SRASequence: uninitialized vTable" );
            return false;
        case 1:
            switch ( seq -> v1 . min )
            {
            case 0:
#if _DEBUGGING
                if ( seq -> v1 . whack                 == NULL ||
                     seq -> v1 . dup                   == NULL ||
                     seq -> v1 . name                  == NULL ||
                     seq -> v1 . is_technical          == NULL ||
                     seq -> v1 . is_reverse            == NULL ||
                     seq -> v1 . length                == NULL ||
                     seq -> v1 . bases                 == NULL ||
                     seq -> v1 . color_space_native    == NULL ||
                     seq -> v1 . has_color_space       == NULL ||
                     seq -> v1 . color_space           == NULL ||
                     seq -> v1 . qualities             == NULL ||
                     seq -> v1 . has_primary_alignment == NULL ||
                     seq -> v1 . primary_alignment     == NULL )
                {
                    rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcCorrupt );
                    SRAExceptionError ( x, 0, rc, "SRASequence: uninitialized v1.0 vTable" );
                    return false;
                }
#endif
                break;
            default:
                rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcBadVersion );
                SRAExceptionError ( x, 0, rc, "SRASequence: unrecognized vTable version ( 1.%u )",
                                    seq -> v1 . min );
                return false;
            }
            break;
        default:
            rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcBadVersion );
            SRAExceptionError ( x, 0, rc, "SRASequence: unrecognized vTable major version ( %u )",
                                seq -> v1 . maj );
            return false;
        }

        /* allow for an object that is NOT an iterator */
        if ( it == NULL )
            return true;

        switch ( it -> v1 . maj )
        {
        case 0:
            rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcInvalid );
            SRAExceptionError ( x, 0, rc, "SRASequenceIterator: uninitialized vTable" );
            break;
        case 1:
            switch ( it -> v1 . min )
            {
            case 0:
#if _DEBUGGING
                if ( it -> v1 . dup  == NULL ||
                     it -> v1 . next == NULL )
                {
                    rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcCorrupt );
                    SRAExceptionError ( x, 0, rc, "SRASequenceIterator: uninitialized v1.0 vTable" );
                    break;
                }
#endif
                return true;
            default:
                rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcBadVersion );
                SRAExceptionError ( x, 0, rc, "SRASequenceIterator: unrecognized vTable version ( 1.%u )",
                                    it -> v1 . min );
            }
            break;
        default:
            rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcBadVersion );
            SRAExceptionError ( x, 0, rc, "SRASequenceIterator: unrecognized vTable major version ( %u )",
                                it -> v1 . maj );
        }
    }

    return false;
}
