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

#ifndef _h_atomic32_
#include "atomic32.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int atomic_int;
typedef atomic32_t atomic_t;

/* ( * v ) */
#define atomic_read( v ) \
    atomic32_read ( v )

/* ( * v ) = i */
#define atomic_set( v, i ) \
    atomic32_set ( v, i )

/* prior = ( * v ), ( * v ) += i, prior */
#define atomic_read_and_add( v, i ) \
    atomic32_read_and_add ( v, i )

/* ( * v ) += i */
#define atomic_add( v, i ) \
    atomic32_add ( v, i )

/* ( * v ) += i */
#define atomic_add_and_read( v, i ) \
    atomic32_add_and_read ( v, i )

/* ( void ) ++ ( * v ) */
#define atomic_inc( v ) \
    atomic32_inc ( v )

/* ( void ) -- ( * v ) */
#define atomic_dec( v ) \
    atomic32_dec ( v )

/* -- ( * v ) == 0 */
#define atomic_dec_and_test( v ) \
    atomic32_dec_and_test ( v )

/* ++ ( * v ) == 0
   when atomic_dec_and_test uses predecrement, you want
   postincrement to this function. so it isn't very useful */
#define atomic_inc_and_test( v ) \
    atomic32_inc_and_test ( v )

/* ( * v ) -- == 0
   HERE's useful */
#define atomic_test_and_inc( v ) \
    atomic32_test_and_inc ( v )

/* prior = ( * v ), ( * v ) = ( prior == t ? s : prior ), prior */
#define atomic_test_and_set( v, s, t ) \
    atomic32_test_and_set ( v, s, t )

/* N.B. - THIS FUNCTION IS FOR 32 BIT PTRS ONLY */
static __inline__ void *atomic_test_and_set_ptr ( void *volatile *v, void *s, void *t )
{
    void *rtn = * v;
    if ( rtn == t )
        * v = s;
    return rtn;
}

/* val = ( * v ), ( ( * v ) = ( val < t ) ? val + i : val ), val */
#define atomic_read_and_add_lt( v, i, t ) \
    atomic32_read_and_add_lt ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( val <= t ) ? val + i : val ), val */
#define atomic_read_and_add_le( v, i, t ) \
    atomic32_read_and_add_le ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( val == t ) ? val + i : val ), val */
#define atomic_read_and_add_eq( v, i, t ) \
    atomic32_read_and_add_eq ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( val != t ) ? val + i : val ), val */
#define atomic_read_and_add_ne( v, i, t ) \
    atomic32_read_and_add_ne ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( val >= t ) ? val + i : val ), val */
#define atomic_read_and_add_ge( v, i, t ) \
    atomic32_read_and_add_ge ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( val > t ) ? val + i : val ), val */
#define atomic_read_and_add_gt( v, i, t ) \
    atomic32_read_and_add_gt ( v, i, t )

/* val = ( * v ), ( ( * v ) = ( ( val & 1 ) == 1 ) ? val + i : val ), val */
#define atomic_read_and_add_odd( v, i ) \
    atomic32_read_and_add_odd ( v, i )

/* val = ( * v ), ( ( * v ) = ( ( val & 1 ) == 0 ) ? val + i : val ), val */
#define atomic_read_and_add_even( v, i ) \
    atomic32_read_and_add_even ( v, i )

/* DEPRECATED */

/* val = ( * v ), ( * v ) = ( val < t ? val + i : val ), ( val < t ? 1 : 0 ) */
#define atomic_add_if_lt( v, i, t ) \
    atomic32_add_if_lt ( v, i, t )

/* val = ( * v ), ( * v ) = ( val <= t ? val + i : val ), ( val <= t ? 1 : 0 ) */
#define atomic_add_if_le( v, i, t ) \
    atomic32_add_if_le ( v, i, t )

/* val = ( * v ), ( * v ) = ( val == t ? val + i : val ), ( val == t ? 1 : 0 ) */
#define atomic_add_if_eq( v, i, t ) \
    atomic32_add_if_eq ( v, i, t )

/* val = ( * v ), ( * v ) = ( val >= t ? val + i : val ), ( val >= t ? 1 : 0 ) */
#define atomic_add_if_ge( v, i, t ) \
    atomic32_add_if_ge ( v, i, t )

/* val = ( * v ), ( * v ) = ( val > t ? val + i : val ), ( val > t ? 1 : 0 ) */
#define atomic_add_if_gt( v, i, t ) \
    atomic32_add_if_gt ( v, i, t )

#undef LOCK

#ifdef __cplusplus
}
#endif

#endif /* _h_atomic_ */
