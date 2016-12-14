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

#ifndef _hpp_vdb3_kfc_atomic_
#define _hpp_vdb3_kfc_atomic_

#ifndef _hpp_vdb3_kfc_defs_
#include <kfc/defs.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * atomic_t < T >
     *  an atomicly accessed/modified integer
     */
    template < class T >
    class atomic_t
    {
    public:

        // return counter;
        T read () const;
        // counter = val;
        void set ( T val );

        // counter += 1;
        void inc ();
        // counter -= 1;
        void dec ();
        // return ++ counter == 0;
        bool inc_and_test ();
        // return -- counter == 0;
        bool dec_and_test ();
        // return counter ++ = 0;
        bool test_and_inc ();

        // ret = counter; if ( counter == cmp ) counter = val; return ret;
        T test_and_set ( T cmp, T val );

        // counter += val;
        void add ( T val );
        // ret = counter; counter += val; return ret;
        T read_and_add ( T val );
        // return counter += val;
        T add_and_read ( T val );

        // ret = counter; if ( counter < cmp ) counter =+ val; return ret;
        T read_and_add_lt ( T cmp, T val );
        // ret = counter; if ( counter <= cmp ) counter = val; return ret;
        T read_and_add_le ( T cmp, T val );
        // ret = counter; if ( counter == cmp ) counter = val; return ret;
        T read_and_add_eq ( T cmp, T val );
        // ret = counter; if ( counter != cmp ) counter = val; return ret;
        T read_and_add_ne ( T cmp, T val );
        // ret = counter; if ( counter >= cmp ) counter = val; return ret;
        T read_and_add_ge ( T cmp, T val );
        // ret = counter; if ( counter > cmp ) counter = val; return ret;
        T read_and_add_gt ( T cmp, T val );

        // subtraction versions
        inline void sub ( T val ) { add ( - val ); }
        inline T read_and_sub ( T val )
        { return read_and_add ( - val ); }
        inline T sub_and_read ( T val )
        { return add_and_read ( - val ); }
        inline T read_and_sub_lt ( T cmp, T val )
        { return read_and_add_lt ( cmp, - val ); }
        inline T read_and_sub_le ( T cmp, T val )
        { return read_and_add_le ( cmp, - val ); }
        inline T read_and_sub_eq ( T cmp, T val )
        { return read_and_add_eq ( cmp, - val ); }
        inline T read_and_sub_ne ( T cmp, T val )
        { return read_and_add_ne ( cmp, - val ); }
        inline T read_and_sub_ge ( T cmp, T val )
        { return read_and_add_ge ( cmp, - val ); }
        inline T read_and_sub_gt ( T cmp, T val )
        { return read_and_add_gt ( cmp, - val ); }

        // C++ operator overloads
        inline void operator ++ () { inc (); }
        inline void operator -- () { dec (); }
        inline void operator = ( T val ) { set ( val ); }
        inline void operator += ( T val ) { add ( val ); }
        inline void operator -= ( T val ) { add ( - val ); }
        inline operator T () const { return read (); }

        atomic_t ( T val )
            : counter ( val ) {}

        atomic_t ( const atomic_t < T > & a );
        void operator = ( const atomic_t < T > & a );

    private:

        volatile T counter;
    };
}

#if defined __x86_64__
#include <kfc/atomic-x86_64.hpp>
#include <kfc/atomic-ia32.hpp>
#elif defined i686 || defined i386
#include <kfc/atomic-ia32.hpp>
#else
#error "no implementation exists for the target architecture"
#endif

#endif // _hpp_vdb3_kfc_atomic_
