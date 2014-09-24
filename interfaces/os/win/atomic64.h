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

#ifndef _h_atomic64_
#define _h_atomic64_

#define WIN32_LEAN_AND_MEAN
#include <WINDOWS.H>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct atomic64_t atomic64_t;
struct atomic64_t
{
    volatile LONGLONG counter;
};

/* int atomic64_read ( const atomic64_t *v ); */
#define atomic64_read( v ) \
    ( ( v ) -> counter )

/* void atomic64_set ( atomic64_t *v, long int i ); */
#define atomic64_set( v, i ) \
    ( ( void ) ( ( ( v ) -> counter ) = ( i ) ) )

/* add to v -> counter and return the prior value */
/* int atomic32_read_and_add ( atomic32_t *v, int i ) */
#define atomic64_read_and_add( v, i ) \
    InterlockedExchangeAdd64 ( & ( v ) -> counter, ( i ) )

/* if no read is needed, define the least expensive atomic add */
#define atomic64_add( v, i ) \
    atomic64_read_and_add ( v, i )

/* add to v -> counter and return the result */
static __inline LONG64 atomic64_add_and_read ( atomic64_t *v, int i )
{
	return atomic64_read_and_add ( v, i ) + i;
}

/* void atomic64_inc ( atomic64_t *v ) */
#define atomic64_inc( v ) \
    InterlockedIncrement64 ( & ( v ) -> counter )

/* void atomic64_dec ( atomic64_t *v ) */
#define atomic64_dec( v ) \
    InterlockedDecrement64 ( & ( v ) -> counter )

/* decrement by one and test result for 0 */
/* int atomic64_dec_and_test ( atomic64_t *v ) */
#define atomic64_dec_and_test( v ) \
    ( InterlockedDecrement64 ( & ( v ) -> counter ) == 0 )

/* when atomic64_dec_and_test uses predecrement, you want
   postincrement to this function. so it isn't very useful */
/* int atomic64_inc_and_test ( atomic64_t *v ) */
#define atomic64_inc_and_test( v ) \
    ( InterlockedIncrement64 ( & ( v ) -> counter ) == 0 )

/* HERE's useful */
#define atomic64_test_and_inc( v ) \
    ( atomic64_read_and_add ( v, 1 ) == 0 )

/* int atomic64_test_and_set ( atomic64_t *v, int s, int t ) */
#define atomic64_test_and_set( v, s, t ) \
    InterlockedCompareExchange64 ( & ( v ) -> counter, ( s ), ( t ) )

/* conditional modifications */
static __inline LONG64 atomic64_read_and_add_lt ( atomic64_t *v, int i, int t )
{
	LONG64 val, val_intern;
	for ( val = atomic64_read ( v ); val < t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

static __inline LONG64 atomic64_read_and_add_le ( atomic64_t *v, int i, int t )
{
	LONG64 val, val_intern;
	for ( val = atomic64_read ( v ); val <= t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

static __inline LONG64 atomic64_read_and_add_eq ( atomic64_t *v, int i, int t )
{
	LONG64 val, val_intern;
	for ( val = atomic64_read( v ); val == t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

static __inline LONG64 atomic64_read_and_add_ne ( atomic64_t *v, int i, int t )
{
	LONG64 val, val_intern;
	for ( val = atomic64_read ( v ); val != t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

static __inline LONG64 atomic64_read_and_add_ge ( atomic64_t *v, int i, int t )
{
	LONG64 val, val_intern;
	for ( val = atomic64_read ( v ); val >= t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

static __inline LONG64 atomic64_read_and_add_gt ( atomic64_t *v, int i, int t )
{
	LONG64 val, val_intern;
	for ( val = atomic64_read ( v ); val > t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

static __inline LONG64 atomic64_read_and_add_odd ( atomic64_t *v, int i )
{
	LONG64 val, val_intern;
	for ( val = atomic64_read ( v ); ( val & 1 ) != 0; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

static __inline LONG64 atomic64_read_and_add_even ( atomic64_t *v, int i )
{
	LONG64 val, val_intern;
	for ( val = atomic64_read ( v ); ( val & 1 ) == 0; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

#ifdef __cplusplus
}
#endif

#endif /* _h_atomic64_ */
