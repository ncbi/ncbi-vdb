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

#include "syslock-priv.h"
#include "syscond-priv.h"

#include <kproc/extern.h>
#include <os-native.h>
#include <kproc/timeout.h>
#include <kproc/cond.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#if _DEBUGGING && 0
#define CMSG( msg, ... ) \
    KOutMsg ( msg, __VA_ARGS__ )
#else
#define CMSG( msg, ... ) \
    ( void ) 0
#endif


/*--------------------------------------------------------------------------
 * KCondition
 *  a POSIX-style condition object
 *  ( requires an external lock object )
 *
 *  usage: the user first acquires an external lock. then, depending upon
 *  the operation, will either test for a condition or establish it, where
 *  the former involves the potential to wait for a signal and the latter
 *  to generate a signal using the external lock for blocking.
 */


/* Destroy
 */
LIB_EXPORT rc_t CC KConditionDestroy ( KCondition *self )
{
    int status = pthread_cond_destroy ( & self -> cond );
    switch ( status )
    {
    case 0:
        break;
    case EBUSY:
        return RC ( rcPS, rcCondition, rcDestroying, rcCondition, rcBusy );
    default:
        return RC ( rcPS, rcCondition, rcDestroying, rcNoObj, rcUnknown );
    }

    return 0;
}


/* Whack
 */
static
rc_t KConditionWhack ( KCondition *self )
{
    rc_t rc = KConditionDestroy ( self );
    if ( rc == 0 )
        free ( self );
    return rc;
}

/* Init
 */
LIB_EXPORT rc_t CC KConditionInit ( KCondition *self )
{
    int status;

    assert ( self != NULL );
    status = pthread_cond_init ( & self -> cond, NULL );
    switch ( status )
    {
    case 0:
        break;
    case EAGAIN:
        return RC ( rcPS, rcCondition, rcConstructing, rcCondition, rcExhausted );
    case ENOMEM:
        return RC ( rcPS, rcCondition, rcConstructing, rcMemory, rcExhausted );
    case EBUSY:
        return RC ( rcPS, rcCondition, rcConstructing, rcCondition, rcBusy );
    case EINVAL:
        return RC ( rcPS, rcCondition, rcConstructing, rcCondition, rcInvalid );
    default:
        return RC ( rcPS, rcCondition, rcConstructing, rcNoObj, rcUnknown );
    }

    atomic32_set ( & self -> refcount, 1 );
    return 0;
}


/* Make
 *  create a condition
 */
LIB_EXPORT rc_t CC KConditionMake ( KCondition **condp )
{
    rc_t rc;
    if ( condp == NULL )
        rc = RC ( rcPS, rcCondition, rcConstructing, rcParam, rcNull );
    else
    {
        KCondition *cond = malloc ( sizeof * cond );
        if ( cond == NULL )
            rc = RC ( rcPS, rcCondition, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KConditionInit ( cond );
            if ( rc == 0 )
            {
                * condp = cond;
                return 0;
            }
            
            free ( cond );
        }

        * condp = NULL;
    }
    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KConditionAddRef ( const KCondition *cself )
{
    if ( cself != NULL )
        atomic32_inc ( & ( ( KCondition* ) cself ) -> refcount );
    return 0;
}

LIB_EXPORT rc_t CC KConditionRelease ( const KCondition *cself )
{
    KCondition *self = ( KCondition* ) cself;
    if ( cself != NULL )
    {
        if ( atomic32_read_and_add ( & self -> refcount, -1 ) <= 1 )
            return KConditionWhack ( self );
    }
    return 0;
}

/* DropRef
 *  manipulates reference counter
 *  returns true if last ref
 */
LIB_EXPORT int CC KConditionDropRef ( const KCondition *cself )
{
    assert ( cself != NULL );
    return atomic32_dec_and_test ( & ( ( KCondition* ) cself ) -> refcount );
}


/* Wait
 *  block on external lock until signaled
 */
LIB_EXPORT rc_t CC KConditionWait ( KCondition *self, struct KLock *lock )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcSelf, rcNull );
    if ( lock == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcLock, rcNull );

    status = pthread_cond_wait ( & self -> cond, & lock -> mutex );
    switch ( status )
    {
    case 0:
        break;
    default:
        return RC ( rcPS, rcCondition, rcWaiting, rcNoObj, rcUnknown );
    }

    return 0;
}

LIB_EXPORT rc_t CC KConditionTimedWait ( KCondition *self, struct KLock *lock, timeout_t *tm )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcSelf, rcNull );
    if ( lock == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcLock, rcNull );
    if ( tm == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcTimeout, rcNull );

    CMSG ( "%s[%p]: testing timeout structure\n", __func__, self );
    if ( ! tm -> prepared )
    {
        CMSG ( "%s[%p]: preparing timeout...\n", __func__, self );
        TimeoutPrepare ( tm );
        CMSG ( "%s[%p]: ...done preparing timeout\n", __func__, self );
    }

#if 1
    CMSG ( "%s[%p]: calling 'pthread_cond_timedwait ( cond = %p, mutex = %p, ts = %p )'...\n"
           , __func__, self, & self -> cond, & lock -> mutex, & tm -> ts
        );
    status = pthread_cond_timedwait ( & self -> cond, & lock -> mutex, & tm -> ts );
#else
    CMSG ( "%s[%p]: calling 'pthread_cond_wait ( cond = %p, mutex = %p )'...\n"
           , __func__, self, & self -> cond, & lock -> mutex, & tm -> ts
        );
    status = pthread_cond_wait ( & self -> cond, & lock -> mutex );
#endif
    CMSG ( "%s[%p]: ...done, OS status = %!\n", __func__, self, status );
    switch ( status )
    {
    case 0:
        break;
    case ETIMEDOUT:
        return RC ( rcPS, rcCondition, rcWaiting, rcTimeout, rcExhausted );
/*
  The pthread_cond_timedwait() function shall fail if:

  ETIMEDOUT
  The time specified by abstime to pthread_cond_timedwait() has passed.

  The pthread_cond_timedwait() and pthread_cond_wait() functions may fail if:

  EINVAL The value specified by cond, mutex, or abstime is invalid.

  EINVAL Different mutexes were supplied for concurrent pthread_cond_timedwait()
  or pthread_cond_wait() operations on the same condition variable.

  EPERM  The mutex was not owned by the current thread at the time of the call.

  *** These functions shall not return an error code of [EINTR]. ***

 */
    case EINTR:
        return RC ( rcPS, rcCondition, rcWaiting, rcThread, rcInterrupted );
    default:
        return RC ( rcPS, rcCondition, rcWaiting, rcNoObj, rcUnknown );
    }

    return 0;
}


/* Signal
 *  signal waiting threads
 *  awaken at most a single thread
 */
LIB_EXPORT rc_t CC KConditionSignal ( KCondition *self )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcSignaling, rcSelf, rcNull );

    status = pthread_cond_signal ( & self -> cond );
    switch ( status )
    {
    case 0:
        break;
    default:
        return RC ( rcPS, rcCondition, rcSignaling, rcNoObj, rcUnknown );
    }

    return 0;
}


/* Broadcast
 *  signal waiting threads
 *  awaken all waiting thread
 */
LIB_EXPORT rc_t CC KConditionBroadcast ( KCondition *self )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcSignaling, rcSelf, rcNull );

    status = pthread_cond_broadcast ( & self -> cond );
    switch ( status )
    {
    case 0:
        break;
    default:
        return RC ( rcPS, rcCondition, rcSignaling, rcNoObj, rcUnknown );
    }

    return 0;
}
