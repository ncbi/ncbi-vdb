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

#ifndef _h_klib_ksort_macro_
#define _h_klib_ksort_macro_

#include <limits.h>
#include <stdlib.h>


/* Copyright (C) 1991,1992,1996,1997,1999,2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Douglas C. Schmidt (schmidt@ics.uci.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* If you consider tuning this algorithm, you should consult first:
   Engineering a sort function; Jon Bentley and M. Douglas McIlroy;
   Software - Practice and Experience; Vol. 23 (11), 1249-1265, 1993.  */

/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#ifndef KSORT_MAX_THRESH
#define KSORT_MAX_THRESH 4
#endif

/* Stack node declarations used to store unfulfilled partition obligations. */
#ifndef ksort_stack_node_declared
#define ksort_stack_node_declared 1
typedef struct ksort_stack_node ksort_stack_node;
struct ksort_stack_node
{
    char *lo;
    char *hi;
};
#endif

/* The next 4 #defines implement a very fast in-line stack abstraction. */
/* The stack needs log (total_elements) entries (we could even subtract
   log(MAX_THRESH)).  Since total_elements has type size_t, we get as
   upper bound for log (total_elements):
   bits per byte (CHAR_BIT) * sizeof(size_t).  */

#ifndef KSORT_STACK_SIZE
#define KSORT_STACK_SIZE	(CHAR_BIT * sizeof(size_t))
#endif

#ifndef KSORT_PUSH
#define KSORT_PUSH(low, high)	((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#endif

#ifndef KSORT_POP
#define	KSORT_POP(low, high)	((void) (--top, (low = top->lo), (high = top->hi)))
#endif

#ifndef KSORT_STACK_EMPTY
#define	KSORT_STACK_EMPTY	( stack >= top )
#endif

#ifndef KSORT_SWAP
#define KSORT_SWAP( a, b, off, size )           \
    do                                          \
    {                                           \
        register size_t i, end = off + size;    \
        register char *__a = (a), *__b = (b);   \
        for ( i = off; i < end; ++ i )          \
        {                                       \
            char __tmp = __a [ i ];             \
            __a [ i ] = __b [ i ];              \
            __b [ i ] = __tmp;                  \
        }                                       \
    } while (0)
#endif

#define KSORT_TSWAP( T, a, b )                  \
    do                                          \
    {                                           \
        T tmp = * ( const T* ) ( a );           \
        * ( T* ) ( a ) = * ( const T* ) ( b );  \
        * ( T* ) ( b ) = tmp;                   \
    }                                           \
    while ( 0 )


/* Order size using quicksort.  This implementation incorporates
   four optimizations discussed in Sedgewick:

   1. Non-recursive, using an explicit stack of pointer that store the
   next array partition to sort.  To save time, this maximum amount
   of space required to store an array of SIZE_MAX is allocated on the
   stack.  Assuming a 32-bit (64 bit) integer for size_t, this needs
   only 32 * sizeof(stack_node) == 256 bytes (for 64 bit: 1024 bytes).
   Pretty cheap, actually.

   2. Chose the pivot element using a median-of-three decision tree.
   This reduces the probability of selecting a bad pivot value and
   eliminates certain extraneous comparisons.

   3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
   insertion sort to order the MAX_THRESH items within each partition.
   This is a big win, since insertion sort is faster for small, mostly
   sorted array segments.

   4. The larger of the two sub-partitions is always pushed onto the
   stack first, with the algorithm then concentrating on the
   smaller partition.  This *guarantees* no more than log (total_elems)
   stack size is needed (actually O(1) in this case)!  */

#define KSORT( PBASE, TOTAL_ELEMS, ELEM_SIZE, SWAP_OFFSET, SWAP_SIZE )            \
    register char *base_ptr = ( char* ) ( PBASE );                                \
    const size_t max_thresh = KSORT_MAX_THRESH * ( ELEM_SIZE );                   \
    if ( ( TOTAL_ELEMS ) < 2)                                                     \
        return;                                                                   \
    if ( ( TOTAL_ELEMS ) > KSORT_MAX_THRESH)                                      \
    {                                                                             \
        char *lo = base_ptr;                                                      \
        char *hi = & lo [ ( ELEM_SIZE ) * ( ( TOTAL_ELEMS ) - 1 ) ];              \
        ksort_stack_node stack [ KSORT_STACK_SIZE ];                              \
        ksort_stack_node *top = stack;                                            \
        KSORT_PUSH (NULL, NULL);                                                  \
        while ( ! KSORT_STACK_EMPTY )                                             \
        {                                                                         \
            char *left_ptr;                                                       \
            char *right_ptr;                                                      \
            char *mid = lo + ( ELEM_SIZE ) * ( ( hi - lo ) / ( ELEM_SIZE ) >> 1 );\
            if ( CMP ( ( void* ) mid, ( void* ) lo ) < 0 )                        \
                SWAP ( mid, lo, ( SWAP_OFFSET ), ( SWAP_SIZE ) );                 \
            if ( CMP ( ( void* ) hi, ( void* ) mid) < 0 )                         \
                SWAP ( mid, hi, ( SWAP_OFFSET ), ( SWAP_SIZE ) );                 \
            else                                                                  \
                goto jump_over;                                                   \
            if ( CMP ( ( void* ) mid, ( void* ) lo ) < 0 )                        \
                SWAP ( mid, lo, ( SWAP_OFFSET ), ( SWAP_SIZE ) );                 \
        jump_over:                                                                \
            left_ptr  = lo + ( ELEM_SIZE );                                       \
            right_ptr = hi - ( ELEM_SIZE );                                       \
            do                                                                    \
            {                                                                     \
                while ( CMP ( ( void* ) left_ptr, ( void* ) mid ) < 0 )           \
                    left_ptr += ( ELEM_SIZE );                                    \
                while ( CMP ( ( void* ) mid, ( void* ) right_ptr ) < 0 )          \
                    right_ptr -= ( ELEM_SIZE );                                   \
                if ( left_ptr < right_ptr )                                       \
                {                                                                 \
                    SWAP ( left_ptr, right_ptr, ( SWAP_OFFSET ), ( SWAP_SIZE ) ); \
                    if ( mid == left_ptr )                                        \
                        mid = right_ptr;                                          \
                    else if ( mid == right_ptr )                                  \
                        mid = left_ptr;                                           \
                    left_ptr += ( ELEM_SIZE );                                    \
                    right_ptr -= ( ELEM_SIZE );                                   \
                }                                                                 \
                else if ( left_ptr == right_ptr )                                 \
                {                                                                 \
                    left_ptr += ( ELEM_SIZE );                                    \
                    right_ptr -= ( ELEM_SIZE );                                   \
                    break;                                                        \
                }                                                                 \
            }                                                                     \
            while ( left_ptr <= right_ptr );                                      \
            if ((size_t) (right_ptr - lo) <= max_thresh)                          \
            {                                                                     \
                if ((size_t) (hi - left_ptr) <= max_thresh)                       \
                    KSORT_POP (lo, hi);                                           \
                else                                                              \
                    lo = left_ptr;                                                \
            }                                                                     \
            else if ((size_t) (hi - left_ptr) <= max_thresh)                      \
                hi = right_ptr;                                                   \
            else if ((right_ptr - lo) > (hi - left_ptr))                          \
            {                                                                     \
                KSORT_PUSH (lo, right_ptr);                                       \
                lo = left_ptr;                                                    \
            }                                                                     \
            else                                                                  \
            {                                                                     \
                KSORT_PUSH (left_ptr, hi);                                        \
                hi = right_ptr;                                                   \
            }                                                                     \
        }                                                                         \
    }                                                                             \
    {                                                                             \
        register char *run_ptr;                                                   \
        char *const end_ptr = & base_ptr [(ELEM_SIZE)*((TOTAL_ELEMS)-1)];         \
        char *tmp_ptr = base_ptr;                                                 \
        char *thresh = base_ptr + max_thresh;                                     \
        if ( thresh > end_ptr )                                                   \
            thresh = end_ptr;                                                     \
        for (run_ptr = tmp_ptr+(ELEM_SIZE);run_ptr<=thresh;run_ptr+=(ELEM_SIZE))  \
            if ( CMP ( ( void* ) run_ptr, ( void* ) tmp_ptr ) < 0 )               \
                tmp_ptr = run_ptr;                                                \
        if (tmp_ptr != base_ptr)                                                  \
            SWAP ( tmp_ptr, base_ptr, ( SWAP_OFFSET ), ( SWAP_SIZE ) );           \
        tmp_ptr = base_ptr + ( ELEM_SIZE );                                       \
        for ( run_ptr = base_ptr + ( 2 * ( ELEM_SIZE ) );                         \
              run_ptr <= end_ptr; tmp_ptr = run_ptr, run_ptr += ( ELEM_SIZE ) )   \
        {                                                                         \
            for ( ; tmp_ptr >= base_ptr; tmp_ptr -= ( ELEM_SIZE ) )               \
            {                                                                     \
                if ( CMP ( run_ptr, tmp_ptr ) >= 0 )                              \
                    break;                                                        \
            }                                                                     \
            tmp_ptr += ( ELEM_SIZE );                                             \
            if ( tmp_ptr != run_ptr )                                             \
            {                                                                     \
                char *trav;                                                       \
                trav = run_ptr + ( ELEM_SIZE );                                   \
                while ( --trav >= run_ptr )                                       \
                {                                                                 \
                    char c = *trav;                                               \
                    char *hi, *lo;                                                \
                    for (hi=lo=trav; (lo-=(ELEM_SIZE)) >= tmp_ptr; hi=lo)         \
                        *hi = *lo;                                                \
                    *hi = c;                                                      \
                }                                                                 \
            }                                                                     \
        }                                                                         \
    }                                                                             \
    ( void ) 0

/*
 adapted from the above macro, it is a mixture of qsort and insert sort
 all access to the sortee are via supplied macros, all of which work on index
 values.
 this code requires that indices of the array are contiguous integers,
 random-accessible, and are >= 0 and < N
 
Required macros:
 LESS(INDEX_1, INDEX_2)
    provides the ordering for the sort

 SAVE(INDEX)
    arranges to save the value at INDEX, probably just copy it to a temporary

 COPY(DST_INDEX, SRC_INDEX)
    copy the value at SRC_INDEX to DST_INDEX

 LOAD(INDEX)
    load the value saved by the SAVE macro

 The last 3 are used for moving elements around.  For example, swap(a, b) would
 be:
    SAVE(a); COPY(a, b); LOAD(b);
 They are always used like this, e.g. save an element, copy some elements while
 overwriting the position of the saved one, and then restore the saved element
 at its new position.  This sort has more ways to move things around than swap.

Example:
static void sort_int(size_t const N, int array[N])
{
#define SAVE(A) int const tmp = array[A]
#define LOAD(A) ((void)(array[A] = tmp))
#define COPY(DST, SRC) ((void)(array[(DST)] = array[(SRC)]))
#define LESS(A, B) (array[A] < array[B])
    ASORT(N)
#undef SAVE
#undef LOAD
#undef COPY
#undef LESS
}
*/

#define ASORT_INSERT_SORT_2(L)                                                  \
    if (LESS((L)+1, (L))) { SAVE((L)+1); COPY((L)+1, (L)); LOAD((L)); }

#define ASORT_INSERT_SORT_3(L)                                                  \
    if (LESS((L)+2, (L))) {                                                     \
        SAVE((L)+2); COPY((L)+2, (L)+1); COPY((L)+1, (L)); LOAD((L));           \
    } else ASORT_INSERT_SORT_2((L)+1)

#define ASORT_INSERT_SORT_4(L)                                                  \
    if (LESS((L)+3, (L))) {                                                     \
        SAVE((L)+3); COPY((L)+3, (L)+2); COPY((L)+2, (L)+1);                    \
        COPY((L)+1, (L)); LOAD((L));                                            \
    } else ASORT_INSERT_SORT_3((L)+1)

#define ASORT_INSERT_SORT_5(L)                                                  \
    if (LESS((L)+4, (L))) {                                                     \
        SAVE((L)+4); COPY((L)+4, (L)+3); COPY((L)+3, (L)+2);                    \
        COPY((L)+2, (L)+1); COPY((L)+1, (L)); LOAD((L));                        \
    } else ASORT_INSERT_SORT_4((L)+1)

#define ASORT_INSERT_SORT_6(L)                                                  \
    if (LESS((L)+5, (L))) {                                                     \
        SAVE((L)+5); COPY((L)+5, (L)+4); COPY((L)+4, (L)+3);                    \
        COPY((L)+3, (L)+2); COPY((L)+2, (L)+1); COPY((L)+1, (L));               \
        LOAD((L));                                                              \
    } else ASORT_INSERT_SORT_5((L)+1)

#define ASORT_INSERT_SORT_7(L)                                                  \
    if (LESS((L)+6, (L))) {                                                     \
        SAVE((L)+6); COPY((L)+6, (L)+5); COPY((L)+5, (L)+4);                    \
        COPY((L)+4, (L)+3); COPY((L)+3, (L)+2); COPY((L)+2, (L)+1);             \
        COPY((L)+1, (L)); LOAD((L));                                            \
    } else ASORT_INSERT_SORT_6((L)+1)

#define ASORT_INSERT_SORT_LIN(L, N)                                             \
    size_t a = (L); size_t const b = (L) + (N) - 1;                             \
    do { if (LESS(b, a)) {                                                      \
            size_t bb = b;                                                      \
            SAVE(b);                                                            \
            while (a < bb) { COPY(bb, bb-1); --bb; }                            \
            LOAD(a);                                                            \
            break;                                                              \
    }} while (++a < b)

#define ASORT_INSERT_SORT(L, N)                                                 \
    if (N > 1) { ASORT_INSERT_SORT_2(L) }                                       \
    if (N > 2) { ASORT_INSERT_SORT_3(L) }                                       \
    if (N > 3) { ASORT_INSERT_SORT_4(L) }                                       \
    if (N > 4) { ASORT_INSERT_SORT_5(L) }                                       \
    if (N > 5) { ASORT_INSERT_SORT_6(L) }                                       \
    if (N > 6) {                                                                \
        size_t M; for (M = 7; ; ++M) {                                          \
            ASORT_INSERT_SORT_LIN(L, M); if (M == N) break;                     \
        }}

#define ASORT(N)                                                                \
    struct { size_t L, R; } stack[8*sizeof(size_t)];                            \
    unsigned top = 0;                                                           \
    size_t L = 0;                                                               \
    size_t R = N - 1;                                                           \
    for ( ; ; ) {                                                               \
        size_t const n = R + 1 - L;                                             \
        if (n > 7) { /* this is qsort */                                        \
            size_t mp = L + (n >> 1);                                           \
            size_t ll = L + 1;                                                  \
            size_t rr = R - 1;                                                  \
            if (LESS(mp, L)) {                                                  \
                SAVE(mp); COPY(mp, L); LOAD(L);                                 \
            }                                                                   \
            if (LESS(R, mp)) {                                                  \
                SAVE(R); COPY(R, mp); LOAD(mp);                                 \
                if (LESS(mp, L)) {                                              \
                    SAVE(mp); COPY(mp, L); LOAD(L);                             \
                }                                                               \
            }                                                                   \
            do {                                                                \
                while (LESS(ll, mp))                                            \
                    ++ll;                                                       \
                while (LESS(mp, rr))                                            \
                    --rr;                                                       \
                if (ll < rr) {                                                  \
                    SAVE(ll); COPY(ll, rr); LOAD(rr);                           \
                    if (mp == ll)                                               \
                        mp = rr;                                                \
                    else if (mp == rr)                                          \
                        mp = ll;                                                \
                    ++ll;                                                       \
                    --rr;                                                       \
                }                                                               \
                else if (ll == rr) {                                            \
                    ++ll;                                                       \
                    --rr;                                                       \
                    break;                                                      \
                }                                                               \
            } while (ll <= rr);                                                 \
            { /* the split is [L..rr][ll..R], the larger is deferred */         \
                size_t const ls = R + 1 - ll;                                   \
                size_t const rs = rr + 1 - L;                                   \
                if (ls < 2)                                                     \
                    R = rr;                                                     \
                else if (rs < 2)                                                \
                    L = ll;                                                     \
                else if (ls < rs) {                                             \
                    stack[top].L = L;                                           \
                    stack[top].R = rr;                                          \
                    ++top;                                                      \
                    L = ll;                                                     \
                }                                                               \
                else {                                                          \
                    stack[top].L = ll;                                          \
                    stack[top].R = R;                                           \
                    ++top;                                                      \
                    R = rr;                                                     \
                }                                                               \
            }                                                                   \
        }                                                                       \
        else {                                                                  \
            ASORT_INSERT_SORT(L, n);                                            \
            /* the partition [L..R] is now sorted */                            \
            if (top == 0) break;                                                \
            /* recall a deferred partition */                                   \
            --top;                                                              \
            L = stack[top].L;                                                   \
            R = stack[top].R;                                                   \
        }                                                                       \
    }                                                                           \
    ((void)0)

#endif /* _h_klib_ksort_macro_ */
