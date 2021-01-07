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

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
typedef struct atomic64_t atomic64_t;
struct atomic64_t
{
    volatile long int counter;
};

/* int atomic64_read ( const atomic64_t *v ); */
#define atomic64_read( v ) \
    ( ( v ) -> counter )

/* void atomic64_set ( atomic64_t *v, long int i ); */
#define atomic64_set( v, i ) \
    ( ( void ) ( ( ( v ) -> counter ) = ( i ) ) )

/* add to v -> counter and return the prior value */
static __inline__ long int atomic64_read_and_add ( atomic64_t *v, long int i )
{
    return __sync_fetch_and_add( & v -> counter, i );
}

/* if no read is needed, define the least expensive atomic add */
#define atomic64_add( v, i ) \
    atomic64_read_and_add ( v, i )

/* add to v -> counter and return the result */
static __inline__ long int atomic64_add_and_read ( atomic64_t *v, long int i )
{
    return __sync_add_and_fetch( & v -> counter, i );
}

/* just don't try to find out what the result was */
static __inline__ void atomic64_inc ( atomic64_t *v )
{
    atomic64_add( v, 1 );
}

static __inline__ void atomic64_dec ( atomic64_t *v )
{
    atomic64_add( v, -1 );
}

/* decrement by one and test result for 0 */
static __inline__ int atomic64_dec_and_test ( atomic64_t *v )
{
    return __sync_add_and_fetch( & v -> counter, -1 ) == 0;
}

/* when atomic64_dec_and_test uses predecrement, you want
   postincrement to this function. so it isn't very useful */
static __inline__ int atomic64_inc_and_test ( atomic64_t *v )
{
    return __sync_add_and_fetch( & v -> counter, 1 ) == 0;
}

/* HERE's useful */
#define atomic64_test_and_inc( v ) \
    ( atomic64_read_and_add ( v, 1L ) == 0 )

static __inline__ long int atomic64_test_and_set ( atomic64_t *v, long int newval, long int oldval )
{
    return __sync_val_compare_and_swap( & v -> counter, oldval, newval ); //NB: newval/oldval switched around
}

/* conditional modifications */
static __inline__
long int atomic64_read_and_add_lt ( atomic64_t *v, long int i, long int t )
{
	int val, val_intern;
	for ( val = atomic64_read ( v ); val < t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

#define atomic64_add_if_lt( v, i, t ) \
    ( atomic64_read_and_add_lt ( v, i, t ) < ( t ) )

static __inline__
long int atomic64_read_and_add_le ( atomic64_t *v, long int i, long int t )
{
	int val, val_intern;
	for ( val = atomic64_read ( v ); val <= t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

#define atomic64_add_if_le( v, i, t ) \
    ( atomic64_read_and_add_le ( v, i, t ) <= ( t ) )

static __inline__
long int atomic64_read_and_add_eq ( atomic64_t *v, long int i, long int t )
{
	int val, val_intern;
	for ( val = atomic64_read ( v ); val == t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

#define atomic64_add_if_eq( v, i, t ) \
    ( atomic64_read_and_add_eq ( v, i, t ) == ( t ) )

static __inline__
long int atomic64_read_and_add_ne ( atomic64_t *v, long int i, long int t )
{
	int val, val_intern;
	for ( val = atomic64_read ( v ); val != t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

#define atomic64_add_if_ne( v, i, t ) \
    ( atomic64_read_and_add_ne ( v, i, t ) != ( t ) )

static __inline__
long int atomic64_read_and_add_ge ( atomic64_t *v, long int i, long int t )
{
	int val, val_intern;
	for ( val = atomic64_read ( v ); val >= t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

#define atomic64_add_if_ge( v, i, t ) \
    ( atomic64_read_and_add_ge ( v, i, t ) >= ( t ) )

static __inline__
long int atomic64_read_and_add_gt ( atomic64_t *v, long int i, long int t )
{
	int val, val_intern;
	for ( val = atomic64_read ( v ); val > t; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

#define atomic64_add_if_gt( v, i, t ) \
    ( atomic64_read_and_add_gt ( v, i, t ) > ( t ) )

static __inline__
long int atomic64_read_and_add_odd ( atomic64_t *v, long int i )
{
	int val, val_intern;
	for ( val = atomic64_read ( v ); val & 1; val = val_intern )
	{
		val_intern = atomic64_test_and_set ( v, val + i, val );
		if ( val_intern == val )
			break;
	}
	return val;
}

static __inline__
long int atomic64_read_and_add_even ( atomic64_t *v, long int i )
{
	int val, val_intern;
	for ( val = atomic64_read ( v ); ! ( val & 1 ); val = val_intern )
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
