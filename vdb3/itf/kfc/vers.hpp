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

#ifndef _hpp_vdb3_kfc_vers_
#define _hpp_vdb3_kfc_vers_

#ifndef _hpp_vdb3_kfc_defs_
#include <kfc/defs.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards and externs
     */


    /*------------------------------------------------------------------
     * vers_t
     */
    class vers_t
    {
    public:

        U32 maj () const
            { return ( U8 ) ( val >> 24 ); }

        U32 min () const
        { return ( U8 ) ( val >> 16 ); }

        U32 rel () const
        { return ( U8 ) ( val >> 8 ); }

        U32 post () const
        { return ( U8 ) ( val >> 0 ); }

        U32 maj_min () const
        { return ( U16 ) ( val >> 16 ); }

        U32 maj_min_rel () const
        { return ( val >> 8 ); }

        U32 full () const
        { return val; }

        bool operator == ( const vers_t & v ) const
        { return val == v . val; }
        bool operator != ( const vers_t & v ) const
        { return val != v . val; }
        bool operator <  ( const vers_t & v ) const
        { return val < v . val; }
        bool operator <= ( const vers_t & v ) const
        { return val <= v . val; }
        bool operator >= ( const vers_t & v ) const
        { return val >= v . val; }
        bool operator >  ( const vers_t & v ) const
        { return val > v . val; }

        vers_t ();
        vers_t ( U32 maj );
        vers_t ( U32 maj, U32 min );
        vers_t ( U32 maj, U32 min, U32 rel );
        vers_t ( U32 maj, U32 min, U32 rel, U32 post );

        vers_t ( const vers_t & vers )
            : val ( vers . val ) {}
        void operator = ( const vers_t & vers )
            { val = vers . val; }

    private:

        U32 val;

    };
}

#endif // _hpp_vdb3_kfc_rsrc_
