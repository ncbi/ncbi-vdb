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

#ifndef _h_atomic_
#define _h_atomic_

#define DFLT_ATOMIC_BITS 32

#ifndef _h_atomic32_
#include "atomic32.h"
#endif

#if ! defined _h_atomic64_ && _ARCH_BITS == 64
#include "atomic64.h"
#endif

typedef struct atomic_ptr_t atomic_ptr_t;
struct atomic_ptr_t
{
    void * volatile ptr;
};

#ifdef __cplusplus
extern "C" {
#endif

#if DFLT_ATOMIC_BITS == 32
#define ATOMIC_NAME( suffix ) \
    atomic32_ ## suffix
typedef LONG atomic_int;
#else
#define ATOMIC_NAME( suffix ) \
    atomic64_ ## suffix
typedef LONGLONG atomic_int;
#endif

typedef struct ATOMIC_NAME ( t ) atomic_t;

/* ( * v ) */
#define atomic_read( v ) \
    ATOMIC_NAME ( read ) ( v )

/* ( * v ) = i */
#define atomic_set( v, i ) \
    ATOMIC_NAME ( set ) ( v, i )

/* prior = ( * v ), ( * v ) += i, prior */
#define atomic_read_and_add( v, i ) \
    ATOMIC_NAME ( read_and_add ) ( v, i )

/* ( * v ) += i */
#define atomic_add( v, i ) \
    ATOMIC_NAME ( add ) ( v, i )

/* ( * v ) += i */
#define atomic_add_and_read( v, i ) \
    ATOMIC_NAME ( add_and_read ) ( v, i )

/* ( void ) ++ ( * v ) */
#define atomic_inc( v ) \
    ATOMIC_NAME ( inc ) ( v )

/* ( void ) -- ( * v ) */
#define atomic_dec( v ) \
    ATOMIC_NAME ( dec ) ( v )

/* -- ( * v ) == 0 */
#define atomic_dec_and_test( v ) \
    ATOMIC_NAME ( dec_and_test ) ( v )

/* ++ ( * v ) == 0
   when atomic_dec_and_test uses predecrement, you want
   postincrement to this function. so it isn't very useful */
#define atomic_inc_and_test( v ) \
    ATOMIC_NAME ( inc_and_test ) ( v )

/* ( * v ) -- == 0
   HERE's useful */
#define atomic_test_and_inc( v ) \
    ATOMIC_NAME ( test_and_inc ) ( v )

/* prior = ( * v ), ( * v ) = ( prior == t ? s : prior ), prior */
#define atomic_test_and_set( v, s, t ) \
    ATOMIC_NAME ( test_and_set ) ( v, s, t )

/* THESE FUNCTIONS ARE universal in the case of Windows, it uses the sizeof(ptr) */
    
/* int atomic_read_ptr ( const atomic_ptr_t *v ); */
#define atomic_read_ptr( v ) \
    ( ( v ) -> ptr )

#define atomic_test_and_set_ptr( v, s, t ) \
    InterlockedCompareExchangePointer( & ( v ) -> ptr, ( s ), ( t ) )

/* val = ( * v ), ( ( * v ) = ( val < t ) ? val + i : val ), val */
#define atomic_read_and_add_lt( v, i, t ) \
    ATOMIC_NAME ( read_and_add_lt ) ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( val <= t ) ? val + i : val ), val */
#define atomic_read_and_add_le( v, i, t ) \
    ATOMIC_NAME ( read_and_add_le ) ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( val == t ) ? val + i : val ), val */
#define atomic_read_and_add_eq( v, i, t ) \
    ATOMIC_NAME ( read_and_add_eq ) ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( val != t ) ? val + i : val ), val */
#define atomic_read_and_add_ne( v, i, t ) \
    ATOMIC_NAME ( read_and_add_ne ) ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( val >= t ) ? val + i : val ), val */
#define atomic_read_and_add_ge( v, i, t ) \
    ATOMIC_NAME ( read_and_add_ge ) ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( val > t ) ? val + i : val ), val */
#define atomic_read_and_add_gt( v, i, t ) \
    ATOMIC_NAME ( read_and_add_gt ) ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( ( val & 1 ) == 1 ) ? val + i : val ), val */
#define atomic_read_and_add_odd( v, i ) \
    ATOMIC_NAME ( read_and_add_odd ) ( v, i )

/* val = ( * v ), ( ( * v ) = ( ( val & 1 ) == 0 ) ? val + i : val ), val */
#define atomic_read_and_add_even( v, i ) \
    ATOMIC_NAME ( read_and_add_even ) ( v, i )

#ifdef __cplusplus
}
#endif

#endif /* _h_atomic_ */
