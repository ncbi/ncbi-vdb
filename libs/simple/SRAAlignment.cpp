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

namespace sra
{

    /*----------------------------------------------------------------------
     * SRAAlignment
     */

    /* querySequence
     *  get the query sequence
     *  this is generally the sequence aligned against the reference
     */
    SRASequence SRAAlignment :: querySequence () const
        throw ( SRAException )
    {
        throw SRAException ( "not implemented" );
    }

    /* reference
     *  get the reference sequence
     */
    SRAReference SRAAlignment :: reference () const
        throw ( SRAException )
    {
        throw SRAException ( "not implemented" );
    }

    /* referenceStart
     *  returns the alignment's zero-based starting coordinate on reference
     */
    uint64_t SRAAlignment :: referenceStart () const
        throw ( SRAException )
    {
        throw SRAException ( "not implemented" );
    }

    /* referenceLength
     *  returns the alignment's projected length upon reference
     */
    uint64_t SRAAlignment :: referenceLength () const
        throw ( SRAException )
    {
        throw SRAException ( "not implemented" );
    }

    /* mappingQuality
     *  how closely aligned are the sequences
     */
    int64_t SRAAlignment :: mappingQuality () const
        throw ( SRAException )
    {
        throw SRAException ( "not implemented" );
    }

    /* hasMate
     */
    bool SRAAlignment :: hasMate () const
        throw ()
    {
        return false;
    }

    /* mate
     *  retrieve mate if available
     */
    SRAAlignmentRef SRAAlignment :: mate () const
        throw ( SRAException )
    {
        throw SRAException ( "not implemented" );
    }

    /* need to tell spot-id and read-idx of sequence
       this could be available on the sequence object itself, but isn't.
       it's very cheap in our case.
    */

    /* OTHER STUFF
       read group ( should be modeled? should be from sequence? )
       is read paired
       is pair proper
       is read on negative strand
       is ( or is not ) primary alignment
       is duplicate
       is mate unmapped
       is mate on negative strand
       mate alignment start
    */

    // C++ support

    SRAAlignment & SRAAlignment :: operator = ( SRAAlignmentRef a )
        throw ()
    {
        return * this;
    }

    SRAAlignment :: SRAAlignment ( SRAAlignmentRef a )
        throw ()
    {
    }

    SRAAlignment & SRAAlignment :: operator = ( const SRAAlignment & a )
        throw ( SRAException )
    {
        throw SRAException ( "not implemented" );
    }

    SRAAlignment :: SRAAlignment ( const SRAAlignment & a )
        throw ( SRAException )
    {
        throw SRAException ( "not implemented" );
    }

    SRAAlignment :: ~ SRAAlignment ()
        throw ()
    {
    }


    /*------------------------------------------------------------------
     * SRAAlignmentIterator
     */

    /* next
     *  advance to the next alignment
     */
    bool SRAAlignmentIterator :: next ()
        throw ( SRAException )
    {
        throw SRAException ( "not implemented" );
    }

    // C++ support

    SRAAlignmentIterator :: SRAAlignmentIterator ( SRAAlignmentRef a )
            throw ()
        : SRAAlignment ( a )
    {
    }

    SRAAlignmentIterator & SRAAlignmentIterator :: operator = ( const SRAAlignmentIterator & i )
        throw ( SRAException )
    {
        throw SRAException ( "not implemented" );
    }

    SRAAlignmentIterator :: SRAAlignmentIterator ( const SRAAlignmentIterator & i )
            throw ( SRAException )
        : SRAAlignment ( 0 )
    {
        throw SRAException ( "not implemented" );
    }

    SRAAlignmentIterator :: ~ SRAAlignmentIterator ()
        throw ()
    {
    }

}
