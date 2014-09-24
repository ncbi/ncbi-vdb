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

#include <simple/sra.hpp>

#include "SRAException.hpp"
#include "SRAString.hpp"

#include "sra-spot.h"

namespace sra
{

    /*----------------------------------------------------------------------
     * SRASpot
     */

    /* platform
     *  return platform name as string
     */
    String SRASpot :: platform () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        :: SRAString sra_str = :: SRASpotPlatform ( spot, & x );
        SRAExceptionCheckError ( x );

        return StringMake ( sra_str );
    }

    /* spotGroup
     *  returns a spot group name if known
     *  returns an empty string otherwise
     */
    String SRASpot :: spotGroup () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        :: SRAString sra_str = :: SRASpotGroup ( spot, & x );
        SRAExceptionCheckError ( x );

        return StringMake ( sra_str );
    }

    /* id
     *  returns the spot's numeric identifier
     */
    int64_t SRASpot :: id () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        int64_t id = :: SRASpotId ( spot, & x );
        SRAExceptionCheckError ( x );

        return id;
    }

    /* numReads
     *  return the number of individual sub-sequences
     */
    uint32_t SRASpot :: numReads () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        uint32_t num_reads = :: SRASpotNumReads ( spot, & x );
        SRAExceptionCheckError ( x );

        return num_reads;
    }

    /* reads
     *  return all reads - mixed technical and biological
     */
    SRASequenceIterator SRASpot :: reads () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        SRASequenceRef r = :: SRASpotReads ( spot, & x );
        SRAExceptionCheckError ( x );

        return SRASequenceIterator ( r );
    }

    /* numBioReads
     *  the number of biological reads
     */
    uint32_t SRASpot :: numBioReads () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        uint32_t num_reads = :: SRASpotNumBioReads ( spot, & x );
        SRAExceptionCheckError ( x );

        return num_reads;
    }

    /* bioReads
     *  return only biological reads
     *  if "trimmed" is true, apply quality clipping if indicated
     */
    SRASequenceIterator SRASpot :: bioReads ( bool trimmed ) const
    throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        SRASequenceRef r = :: SRASpotBioReads ( spot, & x, trimmed );
        SRAExceptionCheckError ( x );

        return SRASequenceIterator ( r );
    }

    // C++ support

    SRASpot & SRASpot :: operator = ( SRASpotRef r )
        throw ()
    {
        SRASequence :: operator = ( :: SRASpotToSequence ( r ) );
        spot = r;
        return * this;
    }

    SRASpot :: SRASpot ( SRASpotRef r )
            throw ()
        : SRASequence ( :: SRASpotToSequence ( r ) )
        , spot ( r )
    {
    }

    SRASpot & SRASpot :: operator = ( const SRASpot & s )
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        SRASpotRef r = :: SRASpotDuplicate ( s . spot, & x );
        SRAExceptionCheckError ( x );

        // take new value
        return * this = r;
    }

    SRASpot :: SRASpot ( const SRASpot & s )
            throw ( SRAException )
        : SRASequence ( 0 )
        , spot ( 0 )
    {
        * this = s;
    }

    SRASpot :: ~ SRASpot ()
        throw ()
    {
        spot = 0;
    }


    /*------------------------------------------------------------------
     * SRASpotIterator
     */

    /* next
     *  advance to the next spot
     *  returns false if no more are available
     */
    bool SRASpotIterator :: next ()
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        bool has_next = :: SRASpotIteratorNext ( spot, & x );
        SRAExceptionCheckError ( x );

        return has_next;
    }


    SRASpotIterator :: SRASpotIterator ( SRASpotRef r )
            throw ()
        : SRASpot ( r )
    {
    }

    SRASpotIterator & SRASpotIterator :: operator = ( const SRASpotIterator & i )
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        SRASpotRef r = :: SRASpotIteratorDuplicate ( i . spot, & x );
        SRAExceptionCheckError ( x );

        // take new value
        SRASpot :: operator = ( r );
        return * this;
    }

    SRASpotIterator :: SRASpotIterator ( const SRASpotIterator & i )
            throw ( SRAException )
        : SRASpot ( 0 )
    {
        * this = i;
    }

    SRASpotIterator :: ~ SRASpotIterator ()
        throw ()
    {
    }

}
