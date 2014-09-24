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

#ifndef _h_atomic32_
#define _h_atomic32_

#include <sys/atomic.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __inline__
#define __inline__ inline
#endif

/*
 * mimic common structure
 */
typedef struct atomic32_t atomic32_t;
struct atomic32_t
{
    volatile uint32_t counter;
};

/* int atomic32_read ( const atomic32_t *v ); */
#define atomic32_read( v ) \
    ( ( v ) -> counter )

/* void atomic32_set ( atomic32_t *v, int i ); */
#define atomic32_set( v, i ) \
    ( ( void ) ( ( ( v ) -> counter ) = ( i ) ) )

/* add to v -> counter and return the prior value
   int atomic32_read_and_add ( atomic32_t *v, int i ) */
#define atomic32_read_and_add( v, i ) \
    ( atomic_add_32_nv ( & ( v ) -> counter, ( int32_t ) ( i ) ) - ( i ) )

/* if no read is needed, define the least expensive atomic add */
#define atomic32_add( v, i ) \
    atomic_add_32 ( & ( v ) -> counter, ( int32_t ) ( i ) )

/* add to v -> counter and return the result
   int atomic32_add_and_read ( atomic32_t *v, int i ) */
#define atomic32_add_and_read( v, i ) \
    atomic_add_32_nv ( & ( v ) -> counter, ( int32_t ) ( i ) )

/* void atomic32_inc ( atomic32_t *v ) */
#define atomic32_inc( v ) \
    atomic_inc_32 ( & ( v ) -> counter )

/* void atomic32_dec ( atomic32_t *v ) */
#define atomic32_dec( v ) \
    atomic_dec_32 ( & ( v ) -> counter )

/* decrement by one and test result for 0
  int atomic32_dec_and_test ( atomic32_t *v ) */
#define atomic32_dec_and_test( v ) \
    ( atomic_dec_32_nv ( & ( v ) -> counter ) == 0 )

/* when atomic32_dec_and_test uses predecrement, you want
   postincrement to this function. so it isn't very useful
   int atomic32_inc_and_test ( atomic32_t *v ) */
#define atomic32_inc_and_test( v ) \
    ( atomic_inc_32_nv ( & ( v ) -> counter ) == 0 )

/* HERE's useful */
#define atomic32_test_and_inc( v ) \
    ( atomic_inc_32_nv ( & ( v ) -> counter ) == 1 )

/* int atomic32_test_and_set ( atomic32_t *v, int s, int t ) */
#define atomic32_test_and_set( v, s, t ) \
    atomic_cas_32 ( & ( v ) -> counter, ( t ), ( s ) )

/* conditional modifications */
static __inline__
int atomic32_read_and_add_lt ( atomic32_t *v, int i, int t )
{
    int val, rtn;
    for ( rtn = atomic32_read ( v ); rtn < t; rtn = val )
    {
        val = atomic32_test_and_set ( v, rtn + i, rtn );
	if ( val == rtn )
            break;
    }
    return rtn;
}

#define atomic32_add_if_lt( v, i, t ) \
    ( atomic32_read_and_add_lt ( v, i, t ) < ( t ) )

static __inline__
int atomic32_read_and_add_le ( atomic32_t *v, int i, int t )
{
    int val, rtn;
    for ( rtn = atomic32_read ( v ); rtn <= t; rtn = val )
    {
        val = atomic32_test_and_set ( v, rtn + i, rtn );
	if ( val == rtn )
            break;
    }
    return rtn;
}

#define atomic32_add_if_le( v, i, t ) \
    ( atomic32_read_and_add_le ( v, i, t ) <= ( t ) )

static __inline__
int atomic32_read_and_add_eq ( atomic32_t *v, int i, int t )
{
    return atomic32_test_and_set ( v, t + i, t );
}

#define atomic32_add_if_eq( v, i, t ) \
    ( atomic32_read_and_add_eq ( v, i, t ) == ( t ) )

static __inline__
int atomic32_read_and_add_ne ( atomic32_t *v, int i, int t )
{
    int val, rtn;
    for ( rtn = atomic32_read ( v ); rtn != t; rtn = val )
    {
        val = atomic32_test_and_set ( v, rtn + i, rtn );
	if ( val == rtn )
            break;
    }
    return rtn;
}

#define atomic32_add_if_ne( v, i, t ) \
    ( atomic32_read_and_add_ne ( v, i, t ) != ( t ) )

static __inline__
int atomic32_read_and_add_ge ( atomic32_t *v, int i, int t )
{
    int val, rtn;
    for ( rtn = atomic32_read ( v ); rtn >= t; rtn = val )
    {
        val = atomic32_test_and_set ( v, rtn + i, rtn );
	if ( val == rtn )
            break;
    }
    return rtn;
}

#define atomic32_add_if_ge( v, i, t ) \
    ( atomic32_read_and_add_ge ( v, i, t ) >= ( t ) )

static __inline__
int atomic32_read_and_add_gt ( atomic32_t *v, int i, int t )
{
    int val, rtn;
    for ( rtn = atomic32_read ( v ); rtn > t; rtn = val )
    {
        val = atomic32_test_and_set ( v, rtn + i, rtn );
	if ( val == rtn )
            break;
    }
    return rtn;
}

#define atomic32_add_if_gt( v, i, t ) \
    ( atomic32_read_and_add_gt ( v, i, t ) > ( t ) )

static __inline__
int atomic32_read_and_add_odd ( atomic32_t *v, int i )
{
    int val, rtn;
    for ( rtn = atomic32_read ( v ); ( rtn & 1 ) != 0; rtn = val )
    {
        val = atomic32_test_and_set ( v, rtn + i, rtn );
	if ( val == rtn )
            break;
    }
    return rtn;
}

static __inline__
int atomic32_read_and_add_even ( atomic32_t *v, int i )
{
    int val, rtn;
    for ( rtn = atomic32_read ( v ); ( rtn & 1 ) == 0; rtn = val )
    {
        val = atomic32_test_and_set ( v, rtn + i, rtn );
	if ( val == rtn )
            break;
    }
    return rtn;
}

#ifdef __cplusplus
}
#endif

#endif /* _h_atomic32_ */
