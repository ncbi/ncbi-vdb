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

#ifndef _hpp_vdb3_kfc_atomic_x86_64_
#define _hpp_vdb3_kfc_atomic_x86_64_

#ifndef _hpp_vdb3_kfc_atomic_
#error "do not include this file directly"
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * atomic_t < I64 >
     * atomic_t < U64 >
     *  universal to all Intel/AMD architectures
     *  particular to either 64-bit ( inc and dec )
     *  or to sign ( compared read_and_add )
     */

    /* inc
     *  requires "incq" rather than "incl"
     */
    template <> inline
    void atomic_t < I64 > :: inc ()
    {
        __asm__ __volatile__
        (
        "lock;"
            "incq %0"
            : "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
    }

    template <> inline
    void atomic_t < U64 > :: inc ()
    {
        __asm__ __volatile__
        (
        "lock;"
            "incq %0"
            : "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
    }

    /* dec
     *  requires "decq" rather than "decl"
     */
    template <> inline
    void atomic_t < I64 > :: dec ()
    {
        __asm__ __volatile__
        (
        "lock;"
            "decq %0"
            : "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
    }

    template <> inline
    void atomic_t < U64 > :: dec ()
    {
        __asm__ __volatile__
        (
        "lock;"
            "decq %0"
            : "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
    }

    /* inc_and_test
     *  "incq" vs "incl"
     */
    template <> inline
    bool atomic_t < I64 > :: inc_and_test ()
    {
        bool rtn;
        __asm__ __volatile__
        (
        "lock;"
            "incq %1;"
            "sete %0"
            : "=r" ( rtn ), "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
        return rtn;
    }

    template <> inline
    bool atomic_t < U64 > :: inc_and_test ()
    {
        bool rtn;
        __asm__ __volatile__
        (
        "lock;"
            "incq %1;"
            "sete %0"
            : "=r" ( rtn ), "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
        return rtn;
    }

    /* dec_and_test
     *  "decq" vs "decl"
     */
    template <> inline
    bool atomic_t < I64 > :: dec_and_test ()
    {
        bool rtn;
        __asm__ __volatile__
        (
        "lock;"
            "decq %1;"
            "sete %0"
            : "=r" ( rtn ), "=m" ( this -> counter )
            : "m" ( this -> counter )

        );
        return rtn;
    }

    template <> inline
    bool atomic_t < U64 > :: dec_and_test ()
    {
        bool rtn;
        __asm__ __volatile__
        (
        "lock;"
            "decq %1;"
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
    I64 atomic_t < I64 > :: read_and_add_lt ( I64 cmp, I64 val )
    {
        I64 rtn, sum;
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
    U64 atomic_t < U64 > :: read_and_add_lt ( U64 cmp, U64 val )
    {
        U64 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"       // rtn - cmp
            "mov %4, %1;"
            "jnc 2f;"           // skip if rtn >= cmp unsigned
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
    I64 atomic_t < I64 > :: read_and_add_le ( I64 cmp, I64 val )
    {
        I64 rtn, sum;
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
    U64 atomic_t < U64 > :: read_and_add_le ( U64 cmp, U64 val )
    {
        U64 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %0, %3;"       // cmp - rtn ( NB! )
            "mov %4, %1;"
            "jc 2f;"            // skip if cmp < rtn unsigned
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
    I64 atomic_t < I64 > :: read_and_add_ge ( I64 cmp, I64 val )
    {
        I64 rtn, sum;
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
    U64 atomic_t < U64 > :: read_and_add_ge ( U64 cmp, U64 val )
    {
        U64 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %3, %0;"       // rtn - cmp
            "mov %4, %1;"
            "jc 2f;"            // skip if rtn < cmp unsigned
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
    I64 atomic_t < I64 > :: read_and_add_gt ( I64 cmp, I64 val )
    {
        I64 rtn, sum;
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
    U64 atomic_t < U64 > :: read_and_add_gt ( U64 cmp, U64 val )
    {
        U64 rtn, sum;
        __asm__ __volatile__
        (
            "mov (%2), %0;"
        "1:"
            "cmp %0, %3;"       // cmp - rtn ( NB! )
            "mov %4, %1;"
            "jnc 2f;"           // skip if cmp >= rtn unsigned
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
}

#endif // _hpp_vdb3_kfc_atomic_x86_64_
