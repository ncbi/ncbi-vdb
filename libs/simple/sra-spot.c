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
#include "sra-spot-priv.h"

#include <klib/rc.h>

#include <assert.h>

/*--------------------------------------------------------------------------
 * SRASpot
 */


/* ToSequence
 *  casting operator to parent interface
 */
LIB_EXPORT SRASequence* CC SRASpotToSequence ( const SRASpot *self )
{
    return & ( ( SRASpot* ) self ) -> dad;
}


/* Duplicate
 *  separate from SRASequenceDuplicate
 *  may perform other operations, returns derived type
 */
LIB_EXPORT SRASpot* CC SRASpotDuplicate ( const SRASpot *self, SRAException *x )
{
    if ( self != NULL )
    {
        const SRASpot_vt *vt = SRASpotGetVT ( self );

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


/* Platform
 *  copy out platform name in UTF-8 character set
 */
LIB_EXPORT SRAString CC SRASpotPlatform ( const SRASpot *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASpot_vt *vt = SRASpotGetVT ( self );
        if ( vt != NULL )
        {
            switch ( vt -> v1 . maj )
            {
            case 1:
                return ( * vt -> v1 . platform ) ( self, x );
            }
        }

        SRAExceptionBadInterface ( x );
    }

    return SRAStringMake ( NULL, "", 0 );
}

/* Group
 *  copy out spotgroup name in UTF-8 character set
 */
LIB_EXPORT SRAString CC SRASpotGroup ( const SRASpot *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASpot_vt *vt = SRASpotGetVT ( self );
        if ( vt != NULL )
        {
            switch ( vt -> v1 . maj )
            {
            case 1:
                return ( * vt -> v1 . spot_group ) ( self, x );
            }
        }

        SRAExceptionBadInterface ( x );
    }

    return SRAStringMake ( NULL, "", 0 );
}

/* Id
 *  spot id
 */
LIB_EXPORT int64_t CC SRASpotId ( const SRASpot *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASpot_vt *vt = SRASpotGetVT ( self );
        if ( vt != NULL )
        {
            switch ( vt -> v1 . maj )
            {
            case 1:
                return ( * vt -> v1 . id ) ( self, x );
            }
        }

        SRAExceptionBadInterface ( x );
    }

    return 0;
}

/* NumReads
 *  number of reads in current spot
 */
LIB_EXPORT uint32_t CC SRASpotNumReads ( const SRASpot *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASpot_vt *vt = SRASpotGetVT ( self );
        if ( vt != NULL )
        {
            switch ( vt -> v1 . maj )
            {
            case 1:
                return ( * vt -> v1 . num_reads ) ( self, x );
            }
        }

        SRAExceptionBadInterface ( x );
    }

    return 0;
}

/* Reads
 *  returns the reads as a sequence iterator
 */
LIB_EXPORT SRASequence* CC SRASpotReads ( const SRASpot *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASpot_vt *vt = SRASpotGetVT ( self );
        if ( vt != NULL )
        {
            switch ( vt -> v1 . maj )
            {
            case 1:
                return ( * vt -> v1 . reads ) ( self, x );
            }
        }

        SRAExceptionBadInterface ( x );
    }

    return NULL;
}

/* NumBioReads
 *  number of biological reads in current spot
 */
LIB_EXPORT uint32_t CC SRASpotNumBioReads ( const SRASpot *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASpot_vt *vt = SRASpotGetVT ( self );
        if ( vt != NULL )
        {
            switch ( vt -> v1 . maj )
            {
            case 1:
                return ( * vt -> v1 . num_bio_reads ) ( self, x );
            }
        }

        SRAExceptionBadInterface ( x );
    }

    return 0;
}

/* BioReads
 *  returns the biological reads as a sequence iterator
 *  if "trimmed" is true, apply quality and other cutoffs
 */
LIB_EXPORT SRASequence* CC SRASpotBioReads ( const SRASpot *self, SRAException *x, bool trimmed )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcReading ) );
    else
    {
        const SRASpot_vt *vt = SRASpotGetVT ( self );
        if ( vt != NULL )
        {
            switch ( vt -> v1 . maj )
            {
            case 1:
                return ( * vt -> v1 . bio_reads ) ( self, x, trimmed );
            }
        }

        SRAExceptionBadInterface ( x );
    }

    return NULL;
}


/*--------------------------------------------------------------------------
 * SRASpotIterator
 */

/* Duplicate
 */
LIB_EXPORT SRASpot* CC SRASpotIteratorDuplicate ( const SRASpot *self, SRAException *x )
{
    if ( self != NULL )
    {
        const SRASequenceIterator_vt *vt = self -> dad . mvt -> it;

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
 *  advance to the next spot
 *  returns false if no more are available
 */
LIB_EXPORT bool CC SRASpotIteratorNext ( SRASpot *self, SRAException *x )
{
    assert ( x != NULL );

    if ( self == NULL )
        SRAExceptionSelfNull ( x, CTX ( rcSRA, rcCursor, rcPositioning ) );
    else
    {
        const SRASequenceIterator_vt *vt = self -> dad . mvt -> it;
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
 */
bool SRASpotInit ( SRASpot *self, SRAException *x, const SRASpot_mvt *mvt )
{
    if ( SRASequenceInit ( & self -> dad, x, & mvt -> dad ) )
    {
        rc_t rc;
        const SRASpot_vt *vt = mvt -> spot;

        if ( vt == NULL )
        {
            rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcNull );
            SRAExceptionError ( x, 0, rc, "SRASpot: bad vTable" );
        }
        else
        {
            switch ( vt -> v1 . maj )
            {
            case 0:
                rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcInvalid );
                SRAExceptionError ( x, 0, rc, "SRASpot: uninitialized vTable" );
                break;
            case 1:
                switch ( vt -> v1 . min )
                {
                case 0:
#if _DEBUGGING
                    if ( vt -> v1 . dup           == NULL ||
                         vt -> v1 . platform      == NULL ||
                         vt -> v1 . spot_group    == NULL ||
                         vt -> v1 . id            == NULL ||
                         vt -> v1 . num_reads     == NULL ||
                         vt -> v1 . reads         == NULL ||
                         vt -> v1 . num_bio_reads == NULL ||
                         vt -> v1 . bio_reads     == NULL )
                    {
                        rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcCorrupt );
                        SRAExceptionError ( x, 0, rc, "SRASpot: uninitialized v1.0 vTable" );
                        break;
                    }
#endif
                    return true;
                default:
                    rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcBadVersion );
                    SRAExceptionError ( x, 0, rc, "SRASpot: unrecognized vTable version ( 1.%u )",
                                        vt -> v1 . min );
                }
                break;
            default:
                rc = RC ( rcSRA, rcCursor, rcConstructing, rcInterface, rcBadVersion );
                SRAExceptionError ( x, 0, rc, "SRASpot: unrecognized vTable major version ( %u )",
                                    vt -> v1 . maj );
            }
        }
    }

    return false;
}
