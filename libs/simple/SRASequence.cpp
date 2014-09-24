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
#include "SRAMem.hpp"
#include "sra-sequence.h"

namespace sra
{

    /*----------------------------------------------------------------------
     * SRASequence
     *  a nucleotide sequence
     */

    /* name
     *  returns a textual identifier if available
     *  or a numeric version otherwise
     */
    String SRASequence :: name () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve name and check for errors
        :: SRAString seq_name = :: SRASequenceName ( self, & x );
        SRAExceptionCheckError ( x );

        // convert C sequence name into a C++ String
        return StringMake ( seq_name );
    }
        
    /* isTechnical
     *  true if the sequence represents a technical read
     */
    bool SRASequence :: isTechnical () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        bool is_technical = :: SRASequenceIsTechnical ( self, & x );
        SRAExceptionCheckError ( x );

        return is_technical;
    }

    /* isReverse
     *  true if reverse read
     */
    bool SRASequence :: isReverse () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        bool is_reverse = :: SRASequenceIsReverse ( self, & x );
        SRAExceptionCheckError ( x );

        return is_reverse;
    }

    /* length
     *  return the number of bases in sequence
     */
    uint64_t SRASequence :: length () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check for errors
        uint64_t length = :: SRASequenceLength ( self, & x );
        SRAExceptionCheckError ( x );

        return length;
    }

    /* bases
     *  returns bases using IUPAC character representation
     */
    String SRASequence :: bases () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve bases and check for errors
        uint64_t remaining;
        :: SRAString bases = :: SRASequenceBases ( self, & x, 0, & remaining );
        SRAExceptionCheckError ( x );

        // convert C sequence into a C++ String
        if ( remaining == 0 )
            return StringMake ( bases );

        // need to build a string from chunks
        uint64_t total = bases . size;
        String b = StringMake ( bases );
        do
        {
            bases = :: SRASequenceBases ( self, & x, total, & remaining );
            SRAExceptionCheckError ( x );
            total += bases . size;
            StringAppend ( b, bases );
        }
        while ( remaining != 0 );
        return b;
    }

    /* colorSpaceNative
     *  returns true if the native sequence is in color-space
     */
    bool SRASequence :: colorSpaceNative () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve value and check error
        bool is_native = :: SRASequenceColorSpaceNative ( self, & x );
        SRAExceptionCheckError ( x );

        return is_native;
    }

    /* hasColorSpace
     *  returns true if sequence can return color-space
     */
    bool SRASequence :: hasColorSpace () const
        throw ()
    {
        return :: SRASequenceHasColorSpace ( self );
    }

    /* colorSpaceKey
     *  returns key ( starting base ) for first read
     */
    char SRASequence :: colorSpaceKey () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve single key
        char key = :: SRASequenceColorSpaceKey ( self, & x );
        SRAExceptionCheckError ( x );

        return key;
    }

    /* colorSpace
     *  returns color-space representation
     *  NB - may fail on some sequences
     */
    String SRASequence :: colorSpace () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve color-space and check for errors
        uint64_t remaining;
        :: SRAString colors = :: SRASequenceColorSpace ( self, & x, 0, & remaining );
        SRAExceptionCheckError ( x );

        // convert C sequence into a C++ String
        if ( remaining == 0 )
            return StringMake ( colors );

        // need to build a string from chunks
        uint64_t total = colors . size;
        String b = StringMake ( colors );
        do
        {
            colors = :: SRASequenceColorSpace ( self, & x, total, & remaining );
            SRAExceptionCheckError ( x );
            total += colors . size;
            StringAppend ( b, colors );
        }
        while ( remaining != 0 );
        return b;
    }

    /* qualities
     *  returns per-base/color phred quality scores
     */
    std :: vector < uint8_t > SRASequence :: qualities () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // retrieve qualities and check for errors
        uint64_t remaining;
        :: SRAMem phred = :: SRASequenceQualities ( self, & x, 0, & remaining );
        SRAExceptionCheckError ( x );

        // convert C qualities into a C++ vector
        if ( remaining == 0 )
            return VectorMake < uint8_t > ( phred );

        // allocate a vector to hold results
        size_t total = phred . size + ( size_t ) remaining;
        std :: vector < uint8_t > v ( total );

        // copy in current set of scores
        size_t i;
        for ( i = 0; i < phred . size; ++ i )
            v [ i ] = ( ( const uint8_t* ) phred . addr ) [ i ];
        :: SRAMemWhack ( & phred );

        // fetch and copy remainder
        do
        {
            phred = :: SRASequenceQualities ( self, & x, i, & remaining );
            SRAExceptionCheckError ( x );

            size_t j;
            for ( j = 0; j < phred . size; ++ j )
                v [ i + j ] = ( ( const uint8_t* ) phred . addr ) [ j ];

            :: SRAMemWhack ( & phred );

            i += j;
        }
        while ( remaining != 0 );
        return v;
    }

    /* hasPrimaryAlignment
     *  returns true iff any part of sequence is aligned,
     *  and the sequence is capable of returning an SRAAlignment
     */
    bool SRASequence :: hasPrimaryAlignment () const
        throw ()
    {
        return :: SRASequenceHasPrimaryAlignment ( self );
    }

    /* primaryAlignment
     *  if the sequence is aligned and the alignment is available
     *  assign to an SRAAlignment
     */
    SRAAlignmentRef SRASequence :: primaryAlignment () const
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // create alignment object and check for errors
        SRAAlignmentRef a = :: SRASequencePrimaryAlignment ( self, & x );
        SRAExceptionCheckError ( x );

        return a;
    }

    // C++ cruft
    SRASequence & SRASequence :: operator = ( SRASequenceRef s )
        throw ()
    {
        // release the old
        :: SRASequenceRelease ( self );

        // accept the new
        self = s;

        return * this;
    }

    SRASequence :: SRASequence ( SRASequenceRef s )
            throw ()
        : self ( s )
    {
    }

    SRASequence & SRASequence :: operator = ( const SRASequence & s )
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // duplicate other guy's reference
        SRASequenceRef dup = :: SRASequenceDuplicate ( s . self, & x );
        SRAExceptionCheckError ( x );

        // release our current reference
        :: SRASequenceRelease ( self );

        // accept the new
        self = dup;

        return * this;
    }

    SRASequence :: SRASequence ( const SRASequence & s )
            throw ( SRAException )
        : self ( 0 )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // duplicate other guy's reference
        self = :: SRASequenceDuplicate ( s . self, & x );
        SRAExceptionCheckError ( x );
    }

    SRASequence :: ~ SRASequence ()
        throw ()
    {
        self = :: SRASequenceRelease ( self );
    }


    /*------------------------------------------------------------------
     * SRASequenceIterator
     */

    /* next
     *  advance to the next sequence
     *  returns false if no more sequences are available
     */
    bool SRASequenceIterator :: next ()
        throw ( SRAException )
    {
        // initialize C exception structure
        :: SRAException x;
        SRAExceptionInit ( & x );

        // advance the iterator
        bool has_next = :: SRASequenceIteratorNext ( self, & x );
        SRAExceptionCheckError ( x );

        return has_next;
    }


    SRASequenceIterator :: SRASequenceIterator ( SRASequenceRef r )
            throw ()
        : SRASequence ( r )
    {
    }

    SRASequenceIterator & SRASequenceIterator :: operator = ( const SRASequenceIterator & i )
        throw ( SRAException )
    {
        SRASequence :: operator = ( i );
        return * this;
    }

    SRASequenceIterator :: SRASequenceIterator ( const SRASequenceIterator & i )
            throw ( SRAException )
        : SRASequence ( 0 )
    {
        SRASequence :: operator = ( i );
    }

    SRASequenceIterator :: ~ SRASequenceIterator ()
        throw ()
    {
    }

}
