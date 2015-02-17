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

#ifndef _hpp_vdb3_kfc_atomic_ia32_
#define _hpp_vdb3_kfc_atomic_ia32_

#ifndef _hpp_vdb3_kfc_atomic_
#error "do not include this file directly"
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * atomic_t < I32 >
     * atomic_t < U32 >
     *  universal to all Intel/AMD architectures
     *  particular to either 32-bit ( inc and dec )
     *  or to sign ( compared read_and_add )
     */

    /* inc
     *  requires "incl" rather than "incq"
     */
    template <> inline
    void atomic_t < I32 > :: inc ()
    {
        __asm__ __volatile__
        (
        "lock;"
            "incl %0"
            : "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
    }

    template <> inline
    void atomic_t < U32 > :: inc ()
    {
        __asm__ __volatile__
        (
        "lock;"
            "incl %0"
            : "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
    }

    /* dec
     *  requires "decl" rather than "decq"
     */
    template <> inline
    void atomic_t < I32 > :: dec ()
    {
        __asm__ __volatile__
        (
        "lock;"
            "decl %0"
            : "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
    }

    template <> inline
    void atomic_t < U32 > :: dec ()
    {
        __asm__ __volatile__
        (
        "lock;"
            "decl %0"
            : "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
    }

    /* inc_and_test
     *  "incl" vs "incq"
     */
    template <> inline
    bool atomic_t < I32 > :: inc_and_test ()
    {
        bool rtn;
        __asm__ __volatile__
        (
        "lock;"
            "incl %1;"
            "sete %0"
            : "=r" ( rtn ), "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
        return rtn;
    }

    template <> inline
    bool atomic_t < U32 > :: inc_and_test ()
    {
        bool rtn;
        __asm__ __volatile__
        (
        "lock;"
            "incl %1;"
            "sete %0"
            : "=r" ( rtn ), "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
        return rtn;
    }

    /* dec_and_test
     *  "decl" vs "decq"
     */
    template <> inline
    bool atomic_t < I32 > :: dec_and_test ()
    {
        bool rtn;
        __asm__ __volatile__
        (
        "lock;"
            "decl %1;"
            "sete %0"
            : "=r" ( rtn ), "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
        return rtn;
    }

    template <> inline
    bool atomic_t < U32 > :: dec_and_test ()
    {
        bool rtn;
        __asm__ __volatile__
        (
        "lock;"
            "decl %1;"
            "sete %0"
            : "=r" ( rtn ), "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
        return rtn;
    }

    /* read_and_add_cc
     *  these are universal for size,
     *  but need differentiation based upon sign
     *  since sign cannot be specified apart from size,
     *  generate 2 specializations of each
     */
    template <> inline
    I32 atomic_t < I32 > :: read_and_add_lt ( I32 cmp, I32 val )
    {
        I32 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"       // rtn - cmp
            "mov %4, %1;"
            "jge 2f;"           // skip if rtn >= cmp signed
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b;"
        "2:"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    template <> inline
    U32 atomic_t < U32 > :: read_and_add_lt ( U32 cmp, U32 val )
    {
        U32 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"       // rtn - cmp
            "mov %4, %1;"
            "jae 2f;"           // skip if rtn >= cmp unsigned
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b;"
        "2:"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    template <> inline
    I32 atomic_t < I32 > :: read_and_add_le ( I32 cmp, I32 val )
    {
        I32 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"       // rtn - cmp
            "mov %4, %1;"
            "jg 2f;"            // skip if rtn > cmp signed
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b;"
        "2:"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    template <> inline
    U32 atomic_t < U32 > :: read_and_add_le ( U32 cmp, U32 val )
    {
        U32 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"       // rtn - cmp
            "mov %4, %1;"
            "ja 2f;"            // skip if rtn > cmp unsigned
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b;"
        "2:"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    template <> inline
    I32 atomic_t < I32 > :: read_and_add_ge ( I32 cmp, I32 val )
    {
        I32 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"       // rtn - cmp
            "mov %4, %1;"
            "jl 2f;"            // skip if rtn < cmp signed
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b;"
        "2:"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    template <> inline
    U32 atomic_t < U32 > :: read_and_add_ge ( U32 cmp, U32 val )
    {
        U32 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"       // rtn - cmp
            "mov %4, %1;"
            "jb 2f;"            // skip if rtn < cmp unsigned
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b;"
        "2:"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    template <> inline
    I32 atomic_t < I32 > :: read_and_add_gt ( I32 cmp, I32 val )
    {
        I32 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"       // rtn - cmp
            "mov %4, %1;"
            "jle 2f;"           // skip if rtn <= cmp signed
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b;"
        "2:"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    template <> inline
    U32 atomic_t < U32 > :: read_and_add_gt ( U32 cmp, U32 val )
    {
        U32 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"       // rtn - cmp
            "mov %4, %1;"
            "jbe 2f;"           // skip if rtn <= cmp unsigned
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b;"
        "2:"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    /*------------------------------------------------------------------
     * atomic_t < T >
     *  universal to all Intel/AMD architectures
     */

    template < class T > inline
    T atomic_t < T > :: read () const
    {
        // on Intel/AMD, the read is already atomic
        return counter;
    }

    template < class T > inline
    void atomic_t < T > :: set ( T val )
    {
        // on Intel/AMD, the write here is already atomic
        counter = val;
    }

    template < class T > inline
    T atomic_t < T > :: read_and_add ( T val )
    {
        T rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "mov %3, %1;"
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( val )
        );
        return rtn;
    }

    template < class T > inline
    T atomic_t < T > :: add_and_read ( T val )
    {
        T rtn, cmp;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "mov %3, %1;"
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b"
            : "=&a" ( cmp ), "=&r" ( rtn )
            : "r" ( & this -> counter ), "r" ( val )
        );
        return rtn;
    }

    template < class T > inline
    void atomic_t < T > :: add ( T val )
    {
        // Intel/AMD architecture favors read_and_add
        // because of the way cmpxchg works
        read_and_add ( val );
    }

    template < class T > inline
    bool atomic_t < T > :: test_and_inc ()
    {
        // Intel/AMD architecture does not do pretest and inc
        // but it does do pre-read and add
        return read_and_add ( 1 ) == 0;
    }


    template < class T > inline
    T atomic_t < T > :: test_and_set ( T cmp, T val )
    {
        T rtn;
        __asm__ __volatile__
        (
        "lock;"
            "cmpxchg %3, (%1)"
            : "=a" ( rtn )
            : "r" ( & this -> counter ), "a" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    template < class T > inline
    T atomic_t < T > :: read_and_add_eq ( T cmp, T val )
    {
        T rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"
            "mov %4, %1;"
            "jne 2f;"
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b;"
        "2:"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    template < class T > inline
    T atomic_t < T > :: read_and_add_ne ( T cmp, T val )
    {
        T rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"
            "mov %4, %1;"
            "je 2f;"
            "add %0, %1;"
        "lock;"
            "cmpxchg %1, (%2);"
            "jne 1b;"
        "2:"
            : "=&a" ( rtn ), "=&r" ( sum )
            : "r" ( & this -> counter ), "r" ( cmp ), "r" ( val )
        );
        return rtn;
    }

    template < class T > inline
    atomic_t < T > :: atomic_t ( const atomic_t < T > & a )
        : counter ( a . read () )
    {
    }

    template < class T > inline
    void atomic_t < T > :: operator = ( const atomic_t < T > & a )
    {
        set ( a . read () );
    }
}

#endif // _hpp_vdb3_kfc_atomic_ia32_
