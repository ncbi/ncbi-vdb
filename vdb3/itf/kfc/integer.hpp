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

#ifndef _hpp_vdb3_kfc_integer_
#define _hpp_vdb3_kfc_integer_

#ifndef _hpp_vdb3_kfc_defs_
#include <kfc/defs.hpp>
#endif

#ifndef ALLOW_RAW_CONSTANTS
#define ALLOW_RAW_CONSTANTS 1
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    class bitsz_t;


    /*------------------------------------------------------------------
     * integer_t
     *  a semantically typed integer
     *  generally capable of most things
     *  that can be done with an integer
     */
    template < class IT, class VT >
    class integer_t
    {
    public:

        // conversion to raw number
        operator VT () const
        { return val; }
        VT value () const
        { return val; }

        // arithmetic
        IT operator + ( const IT & i ) const
        { return IT ( val + i . val ); }
        IT operator - ( const IT & i ) const
        { return IT ( val - i . val ); }
        IT operator * ( const IT & i ) const
        { return IT ( val * i . val ); }
        IT operator / ( const IT & i ) const;
        IT operator % ( const IT & i ) const;

        // updating arithmetic
        IT & operator += ( const IT & i )
        { val += i . val; return * ( IT * ) this; }
        IT & operator -= ( const IT & i )
        { val -= i . val; return * ( IT * ) this; }
        IT & operator *= ( const IT & i )
        { val *= i . val; return * ( IT * ) this; }
        IT & operator /= ( const IT & i );
        IT & operator %= ( const IT & i );

        IT & operator ++ ()
        { ++ val; return * ( IT * ) this; }
        IT & operator -- ()
        { -- val; return * ( IT * ) this; }

        // equality and relational operators
        bool operator == ( const IT & i ) const
        { return val == i . val; }
        bool operator != ( const IT & i ) const
        { return val != i . val; }
        bool operator <= ( const IT & i ) const
        { return val <= i . val; }
        bool operator >= ( const IT & i ) const
        { return val >= i . val; }
        bool operator < ( const IT & i ) const
        { return val < i . val; }
        bool operator > ( const IT & i ) const
        { return val > i . val; }

#if ALLOW_RAW_CONSTANTS
        IT operator + ( VT i ) const
        { return IT ( val + i ); }
        IT operator - ( VT i ) const
        { return IT ( val - i ); }
        IT operator * ( VT i ) const
        { return IT ( val * i ); }
        IT operator / ( VT i ) const;
        IT operator % ( VT i ) const;
        IT & operator += ( VT i )
        { val += i; return * ( IT * ) this; }
        IT & operator -= ( VT i )
        { val -= i; return * ( IT * ) this; }
        IT & operator *= ( VT i )
        { val *= i; return * ( IT * ) this; }
        IT & operator /= ( VT i );
        IT & operator %= ( VT i );
        bool operator == ( VT i ) const
        { return val == i; }
        bool operator != ( VT i ) const
        { return val != i; }
        bool operator <= ( VT i ) const
        { return val <= i; }
        bool operator >= ( VT i ) const
        { return val >= i; }
        bool operator < ( VT i ) const
        { return val < i; }
        bool operator > ( VT i ) const
        { return val > i; }

        IT & operator = ( VT i )
        { val = i; return * ( IT * ) this; }
#endif

        integer_t ()
            : val ( 0 ) {}
        ~ integer_t ()
            { val = 0; }

        integer_t ( VT i )
            : val ( i ) {}
        integer_t ( IT & i )
            : val ( i . val ) {}
        IT & operator = ( IT & i )
            { val = i . val; return * ( IT * ) this; }

    private:

        VT val;
    };

    /*------------------------------------------------------------------
     * bytes_t
     *  a semantically typed integer
     *  represents the size of some entity in bytes
     */
    class bytes_t : public integer_t < bytes_t, U64 >
    {
    public:

        bytes_t () {}
        bytes_t ( U64 bytes )
            : integer_t < bytes_t, U64 > ( bytes ) {}
        explicit bytes_t ( const bytes_t & bytes )
            : integer_t < bytes_t, U64 > ( bytes ) {}
        explicit bytes_t ( const bitsz_t & bits );
    };


    /*------------------------------------------------------------------
     * bitsz_t
     *  a semantically typed integer
     *  represents the size of some entity in bits
     */
    class bitsz_t : public integer_t < bitsz_t, U64 >
    {
    public:

        bitsz_t () {}
        bitsz_t ( U64 bits )
            : integer_t < bitsz_t, U64 > ( bits ) {}
        explicit bitsz_t ( const bitsz_t & bits )
            : integer_t < bitsz_t, U64 > ( bits ) {}
        explicit bitsz_t ( const bytes_t & bytes );
    };

}

#endif // _hpp_vdb3_kfc_integer_
