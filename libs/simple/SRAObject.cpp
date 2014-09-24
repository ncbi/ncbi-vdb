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
#include "sra-object.h"

namespace sra
{

    /*----------------------------------------------------------------------
     * SRAObject
     *  represents an SRA-capable object
     */

    /* constructor
     *  tries to locate object described by "spec"
     *  does not fully OPEN object
     */
    SRAObject :: SRAObject ( const String & spec )
        throw ( SRAException )
        : self ( 0 )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // prepare SRAString structure
        :: SRAString sra_str = :: SRAStringMake ( NULL, spec . c_str (), spec . size () );

        // make object and check for errors
        self = :: SRAObjectMakeFromSpec ( & x, & sra_str );
        SRAExceptionCheckError ( x );
    }

    /* combine constructor
     *  combines two objects into a single object
     */
    SRAObject :: SRAObject ( const SRAObject & a, const SRAObject & b )
        throw ( SRAException )
        : self ( 0 )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // make object and check for errors
        self = :: SRAObjectMakeConcatenation ( & x, a . self, b . self );
        SRAExceptionCheckError ( x );
    }

    /* slice
     *  create a sub-selection of object
     *  the sub-selection is based upon reference spec
     *  and/or coordinates within a reference
     */
    SRAObjectRef SRAObject :: slice ( const String & referenceSpec ) const
    throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // prepare SRAString structure
        :: SRAString spec = :: SRAStringMake ( NULL, referenceSpec . c_str (), referenceSpec . size () );

        // create the object reference
        SRAObjectRef o = :: SRAObjectSliceByReference ( self, & x, & spec );
        SRAExceptionCheckError ( x );

        // return the object reference
        return o;
    }

    SRAObjectRef SRAObject :: slice ( uint64_t reference_start,
                                      uint64_t reference_length, bool fully_contained ) const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // create the object reference
        SRAObjectRef o = :: SRAObjectSliceByRange ( self, & x, reference_start, reference_length, fully_contained );
        SRAExceptionCheckError ( x );

        // return the object reference
        return o;
    }

    /* open - IDEMPOTENT
     */
    void SRAObject :: open ()
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // open the object
        :: SRAObjectOpen ( self, & x );
        SRAExceptionCheckError ( x );
    }

    /* close
     *  releases underlying resources
     *  does NOT delete object
     */
    void SRAObject :: close ()
        throw ()
    {
        :: SRAObjectClose ( self );
    }

    /* getReferences
     *  returns an iterator of all references included within the object
     */
    SRAReferenceIterator SRAObject :: getReferences () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get reference iterator
        SRAReferenceRef i = :: SRAObjectGetReferenceIterator ( self, & x );
        SRAExceptionCheckError ( x );

        return SRAReferenceIterator ( i );
    }

    /* primaryAlignments
     *  return an iterator of primary alignments
     *  ordering is based upon object properties
     */
    SRAAlignmentIterator SRAObject :: primaryAlignments () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get alignment iterator
        SRAAlignmentRef i = :: SRAObjectGetPrimaryAlignmentIterator ( self, & x );
        SRAExceptionCheckError ( x );

        return SRAAlignmentIterator ( i );
    }

    SRAAlignmentIterator SRAObject :: primaryAlignments ( uint64_t startIdx, uint64_t count ) const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get alignment iterator
        SRAAlignmentRef i = :: SRAObjectGetPrimaryAlignmentRangeIterator ( self, & x, startIdx, count );
        SRAExceptionCheckError ( x );

        return SRAAlignmentIterator ( i );
    }
        
    /* secondaryAlignments
     *  return an iterator of secondary alignments
     *  ordering is based upon object properties
     */
    SRAAlignmentIterator SRAObject :: secondaryAlignments () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get alignment iterator
        SRAAlignmentRef i = :: SRAObjectGetSecondaryAlignmentIterator ( self, & x );
        SRAExceptionCheckError ( x );

        return SRAAlignmentIterator ( i );
    }

    SRAAlignmentIterator SRAObject :: secondaryAlignments ( uint64_t startIdx, uint64_t count ) const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get alignment iterator
        SRAAlignmentRef i = :: SRAObjectGetSecondaryAlignmentRangeIterator ( self, & x, startIdx, count );
        SRAExceptionCheckError ( x );

        return SRAAlignmentIterator ( i );
    }
        
    /* allAlignments
     *  return an iterator of combined primary and secondary alignments
     *  ordering is based upon object properties
     */
    SRAAlignmentIterator SRAObject :: allAlignments () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get alignment iterator
        SRAAlignmentRef i = :: SRAObjectGetAlignmentIterator ( self, & x );
        SRAExceptionCheckError ( x );

        return SRAAlignmentIterator ( i );
    }

    /* alignedSequences
     *  return an iterator of aligned sequences
     *  ordering is based upon object properties
     */
    SRASequenceIterator SRAObject :: alignedSequences () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get sequence iterator
        SRASequenceRef i = :: SRAObjectGetAlignedSequenceIterator ( self, & x );
        SRAExceptionCheckError ( x );

        return SRASequenceIterator ( i );
    }

    SRASequenceIterator SRAObject :: alignedSequences ( uint64_t startIdx, uint64_t count ) const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get sequence iterator
        SRASequenceRef i = :: SRAObjectGetAlignedSequenceRangeIterator ( self, & x, startIdx, count );
        SRAExceptionCheckError ( x );

        return SRASequenceIterator ( i );
    }

    /* unalignedSequences
     *  return an iterator of unaligned sequences
     */
    SRASequenceIterator SRAObject :: unalignedSequences () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get sequence iterator
        SRASequenceRef i = :: SRAObjectGetUnalignedSequenceIterator ( self, & x );
        SRAExceptionCheckError ( x );

        return SRASequenceIterator ( i );
    }

    SRASequenceIterator SRAObject :: unalignedSequences ( uint64_t startIdx, uint64_t count ) const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get sequence iterator
        SRASequenceRef i = :: SRAObjectGetUnalignedSequenceRangeIterator ( self, & x, startIdx, count );
        SRAExceptionCheckError ( x );

        return SRASequenceIterator ( i );
    }

    /* allSequences
     *  return an iterator of all sequences
     */
    SRASequenceIterator SRAObject :: allSequences () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get sequence iterator
        SRASequenceRef i = :: SRAObjectGetSequenceIterator ( self, & x );
        SRAExceptionCheckError ( x );

        return SRASequenceIterator ( i );
    }


    /* alignedSpots
     *  return an iterator of fully or partially aligned spots
     */
    SRASpotIterator SRAObject :: alignedSpots ( bool partially_aligned ) const
    throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get spot iterator
        SRASpotRef i = :: SRAObjectGetAlignedSpotIterator ( self, & x, partially_aligned );
        SRAExceptionCheckError ( x );

        return SRASpotIterator ( i );
    }

    SRASpotIterator SRAObject :: alignedSpots ( uint64_t startIdx, uint64_t count, bool partially_aligned ) const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get spot iterator
        SRASpotRef i = :: SRAObjectGetAlignedSpotRangeIterator ( self, & x, startIdx, count, partially_aligned );
        SRAExceptionCheckError ( x );

        return SRASpotIterator ( i );
    }

    /* unalignedSpots
     *  return an iterator of fully unaligned spots
     */
    SRASpotIterator SRAObject :: unalignedSpots () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get spot iterator
        SRASpotRef i = :: SRAObjectGetUnalignedSpotIterator ( self, & x );
        SRAExceptionCheckError ( x );

        return SRASpotIterator ( i );
    }

    SRASpotIterator SRAObject :: unalignedSpots ( uint64_t startIdx, uint64_t count ) const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get spot iterator
        SRASpotRef i = :: SRAObjectGetUnalignedSpotRangeIterator ( self, & x, startIdx, count );
        SRAExceptionCheckError ( x );

        return SRASpotIterator ( i );
    }

    /* allSpots
     *  return an iterator of all spots
     */
    SRASpotIterator SRAObject :: allSpots () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get spot iterator
        SRASpotRef i = :: SRAObjectGetSpotIterator ( self, & x );
        SRAExceptionCheckError ( x );

        return SRASpotIterator ( i );
    }

    SRASpotIterator SRAObject :: allSpots ( uint64_t startIdx, uint64_t count ) const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // get spot iterator
        SRASpotRef i = :: SRAObjectGetSpotRangeIterator ( self, & x, startIdx, count );
        SRAExceptionCheckError ( x );

        return SRASpotIterator ( i );
    }
        
    // C++ support

    SRAObject & SRAObject :: operator = ( SRAObjectRef o )
        throw ()
    {
        // release the old
        :: SRAObjectRelease ( self );

        // accept the new
        self = o;

        return * this;
    }

    SRAObject :: SRAObject ( SRAObjectRef o )
        throw ()
        : self ( o )
    {
    }

    SRAObject & SRAObject :: operator = ( const SRAObject & o )
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // duplicate other guy's reference
        SRAObjectRef dup = :: SRAObjectDuplicate ( o . self, & x );
        SRAExceptionCheckError ( x );

        // release our current reference
        :: SRAObjectRelease ( self );

        // accept the new
        self = dup;

        return * this;
    }

    SRAObject :: SRAObject ( const SRAObject & o )
        throw ( SRAException )
        : self ( 0 )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // duplicate other guy's reference
        self = :: SRAObjectDuplicate ( o . self, & x );
        SRAExceptionCheckError ( x );
    }

    SRAObject :: ~ SRAObject ()
    throw ()
    {
        self = :: SRAObjectRelease ( self );
    }

}
