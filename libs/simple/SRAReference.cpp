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
     * SRAReference
     */

    /* name
     *  returns a textual identifier if available
     *  or a numeric version otherwise
     */
    String SRAReference :: name () const
        throw ( SRAException )
    {
        throw SRAException ( "unimplemented" );
    }

    /* accession
     *  TBD - explain
     */
    String SRAReference :: accession () const
        throw ( SRAException )
    {
        throw SRAException ( "unimplemented" );
    }

    /* isCircular
     */
    bool SRAReference :: isCircular () const
        throw ( SRAException )
    {
        throw SRAException ( "unimplemented" );
    }

    /* baseCount
     */
    uint64_t SRAReference :: baseCount () const
        throw ( SRAException )
    {
        throw SRAException ( "unimplemented" );
    }

    /* bases
     *  returns bases using IUPAC character representation
     *
     *  "first" is a zero-based starting position,
     *  "length" gives the total number of bases, which will be limited
     *   to the reference boundaries unless circular
     */
    String SRAReference :: bases ( uint64_t first, uint64_t length ) const
        throw ( SRAException )
    {
        throw SRAException ( "unimplemented" );
    }

    // C++ support
        
    SRAReference & SRAReference :: operator = ( SRAReferenceRef r )
        throw ()
    {
        return * this;
    }

    SRAReference :: SRAReference ( SRAReferenceRef r )
        throw ()
        : SRASequence ( 0 )
    {
    }

    SRAReference & SRAReference :: operator = ( const SRAReference & r )
        throw ( SRAException )
    {
        throw SRAException ( "unimplemented" );
    }

    SRAReference :: SRAReference ( const SRAReference & r )
        throw ( SRAException )
        : SRASequence ( 0 )
    {
        throw SRAException ( "unimplemented" );
    }

    SRAReference :: ~ SRAReference ()
        throw ()
    {
    }


    /*------------------------------------------------------------------
     * SRAReferenceIterator
     */

    /* next
     *  advance to the next reference
     *  returns false if no more are available
     */
    bool SRAReferenceIterator :: next ()
        throw ( SRAException )
    {
        throw SRAException ( "unimplemented" );
    }

    // C++ support

    SRAReferenceIterator :: SRAReferenceIterator ( SRAReferenceRef r )
            throw ()
        : SRAReference ( r )
    {
    }

    SRAReferenceIterator & SRAReferenceIterator :: operator = ( const SRAReferenceIterator & i )
        throw ( SRAException )
    {
        throw SRAException ( "unimplemented" );
    }

    SRAReferenceIterator :: SRAReferenceIterator ( const SRAReferenceIterator & i )
            throw ( SRAException )
        : SRAReference ( 0 )
    {
    }

    SRAReferenceIterator :: ~ SRAReferenceIterator ()
        throw ()
    {
    }
}
