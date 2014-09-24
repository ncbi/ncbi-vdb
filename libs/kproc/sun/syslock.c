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

#include <kproc/extern.h>
#include "syslock-priv.h"
#include <os-native.h>
#include <kproc/timeout.h>
#include <kproc/lock.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <errno.h>

/*--------------------------------------------------------------------------
 * KLock
 *  a POSIX-style mutual exclusion lock
 */

/* Whack
 */
static
rc_t KLockWhack ( KLock *self )
{
    int status = pthread_mutex_destroy ( & self -> mutex );
    switch ( status )
    {
    case 0:
        break;
    case EBUSY:
        return RC ( rcPS, rcLock, rcDestroying, rcLock, rcBusy );
    case EINVAL:
        return RC ( rcPS, rcLock, rcDestroying, rcLock, rcInvalid );
    default:
        return RC ( rcPS, rcLock, rcDestroying, rcNoObj, rcUnknown );
    }

    free ( self );
    return 0;
}


/* Make
 *  make a simple mutex
 */
LIB_EXPORT rc_t CC KLockMake ( KLock **lockp )
{
    rc_t rc;
    if ( lockp == NULL )
        rc = RC ( rcPS, rcLock, rcConstructing, rcParam, rcNull );
    else
    {
        KLock *lock = malloc ( sizeof * lock );
        if ( lock == NULL )
            rc = RC ( rcPS, rcLock, rcConstructing, rcMemory, rcExhausted );
        else
        {
            int status = pthread_mutex_init ( & lock -> mutex, NULL );
            if ( status == 0 )
            {
                atomic32_set ( & lock -> refcount, 1 );
                * lockp = lock;
                return 0;
            }

            /* pthread_mutex_init is documented as always returning 0 */
            rc = RC ( rcPS, rcLock, rcConstructing, rcNoObj, rcUnknown );

            free ( lock );
        }

        * lockp = NULL;
    }
    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KLockAddRef ( const KLock *cself )
{
    if ( cself != NULL )
        atomic32_inc ( & ( ( KLock* ) cself ) -> refcount );
    return 0;
}

LIB_EXPORT rc_t CC KLockRelease ( const KLock *cself )
{
    KLock *self = ( KLock* ) cself;
    if ( cself != NULL )
    {
        if ( atomic32_dec_and_test ( & self -> refcount ) )
        {
            atomic32_set ( & self -> refcount, 1 );
            return KLockWhack ( self );
        }
    }
    return 0;
}


/* Acquire
 *  acquires lock
 */
LIB_EXPORT rc_t CC KLockAcquire ( KLock *self )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcLock, rcLocking, rcSelf, rcNull );

    status = pthread_mutex_lock ( & self -> mutex );
    switch ( status )
    {
    case 0:
        break;
    case EDEADLK:
        return RC ( rcPS, rcLock, rcLocking, rcThread, rcDeadlock );
    case EINVAL:
        return RC ( rcPS, rcLock, rcLocking, rcLock, rcInvalid );
    default:
        return RC ( rcPS, rcLock, rcLocking, rcNoObj, rcUnknown );
    }

    return 0;
}

LIB_EXPORT rc_t CC KLockTimedAcquire ( KLock *self, timeout_t *tm )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcLock, rcLocking, rcSelf, rcNull );

    status = pthread_mutex_trylock ( & self -> mutex );
    switch ( status )
    {
    case 0:
        return 0;
    case EBUSY:
        if ( tm != NULL )
            break;
        return RC ( rcPS, rcLock, rcLocking, rcLock, rcBusy );
    case EINVAL:
        return RC ( rcPS, rcLock, rcLocking, rcLock, rcInvalid );
    default:
        return RC ( rcPS, rcLock, rcLocking, rcNoObj, rcUnknown );
    }

    if ( ! tm -> prepared )
        TimeoutPrepare ( tm );

    status = pthread_mutex_timedlock ( & self -> mutex, & tm -> ts );
    switch ( status )
    {
    case 0:
        break;
    case ETIMEDOUT:
        return RC ( rcPS, rcLock, rcLocking, rcTimeout, rcExhausted );
    case EINVAL:
        return RC ( rcPS, rcLock, rcLocking, rcTimeout, rcInvalid );
    default:
        return RC ( rcPS, rcLock, rcLocking, rcNoObj, rcUnknown );
    }

    return 0;
}

/* Unlock
 *  releases lock
 */
LIB_EXPORT rc_t CC KLockUnlock ( KLock *self )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcLock, rcUnlocking, rcSelf, rcNull );

    status = pthread_mutex_unlock ( & self -> mutex );
    switch ( status )
    {
    case 0:
        break;
    case EPERM:
        return RC ( rcPS, rcLock, rcUnlocking, rcThread, rcIncorrect );
    case EINVAL:
        return RC ( rcPS, rcLock, rcUnlocking, rcLock, rcInvalid );
    default:
        return RC ( rcPS, rcLock, rcUnlocking, rcNoObj, rcUnknown );
    }

    return 0;
}


/*--------------------------------------------------------------------------
 * KRWLock
 *  a POSIX-style read/write lock
 */
struct KRWLock
{
    pthread_rwlock_t lock;
    atomic32_t refcount;
};


/* Whack
 */
static
rc_t KRWLockWhack ( KRWLock *self )
{
    int status = pthread_rwlock_destroy ( & self -> lock );
    switch ( status )
    {
    case 0:
        break;
    case EBUSY:
        return RC ( rcPS, rcRWLock, rcDestroying, rcRWLock, rcBusy );
    case EINVAL:
        return RC ( rcPS, rcRWLock, rcDestroying, rcRWLock, rcInvalid );
    default:
        return RC ( rcPS, rcRWLock, rcDestroying, rcNoObj, rcUnknown );
    }

    free ( self );
    return 0;
}


/* Make
 *  make a simple read/write lock
 */
LIB_EXPORT rc_t CC KRWLockMake ( KRWLock **lockp )
{
    rc_t rc;

    if ( lockp == NULL )
        rc = RC ( rcPS, rcRWLock, rcConstructing, rcParam, rcNull );
    else
    {
        KRWLock *lock = malloc ( sizeof * lock );
        if ( lock == NULL )
            rc = RC ( rcPS, rcRWLock, rcConstructing, rcMemory, rcExhausted );
        else
        {
            int status = pthread_rwlock_init ( & lock -> lock, NULL );
            if ( status == 0 )
            {
                atomic32_set ( & lock -> refcount, 1 );
                * lockp = lock;
                return 0;
            }

            switch ( status )
            {
            case EAGAIN:
                rc = RC ( rcPS, rcRWLock, rcConstructing, rcRWLock, rcExhausted );
                break;
            case ENOMEM:
                rc = RC ( rcPS, rcRWLock, rcConstructing, rcMemory, rcExhausted );
                break;
            case EPERM:
                rc = RC ( rcPS, rcRWLock, rcConstructing, rcProcess, rcUnauthorized );
                break;
            case EBUSY:
                rc = RC ( rcPS, rcRWLock, rcConstructing, rcRWLock, rcBusy );
                break;
            default:
                rc = RC ( rcPS, rcRWLock, rcConstructing, rcNoObj, rcUnknown );
            }

            free ( lock );
        }

        * lockp = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KRWLockAddRef ( const KRWLock *cself )
{
    if ( cself != NULL )
        atomic32_inc ( & ( ( KRWLock* ) cself ) -> refcount );
    return 0;
}

LIB_EXPORT rc_t CC KRWLockRelease ( const KRWLock *cself )
{
    KRWLock *self = ( KRWLock* ) cself;
    if ( cself != NULL )
    {
        if ( atomic32_dec_and_test ( & self -> refcount ) )
        {
            atomic32_set ( & self -> refcount, 1 );
            return KRWLockWhack ( self );
        }
    }
    return 0;
}


/* AcquireShared
 *  acquires read ( shared ) lock
 */
LIB_EXPORT rc_t CC KRWLockAcquireShared ( KRWLock *self )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    status = pthread_rwlock_rdlock ( & self -> lock );
    switch ( status )
    {
    case 0:
        break;
    case EDEADLK:
        return RC ( rcPS, rcRWLock, rcLocking, rcThread, rcDeadlock );
    case EAGAIN:
        return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcExhausted );
    case EINVAL:
        return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcInvalid );
    default:
        return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
    }

    return 0;
}

LIB_EXPORT rc_t CC KRWLockTimedAcquireShared ( KRWLock *self, timeout_t *tm )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    status = pthread_rwlock_tryrdlock ( & self -> lock );
    switch ( status )
    {
    case 0:
        return 0;
    case EBUSY:
        if ( tm != NULL )
            break;
        return RC ( rcPS, rcLock, rcLocking, rcRWLock, rcBusy );
    case EAGAIN:
        return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcExhausted );
    case EINVAL:
        return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcInvalid );
    default:
        return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
    }

    if ( ! tm -> prepared )
        TimeoutPrepare ( tm );

    status = pthread_rwlock_timedrdlock ( & self -> lock, & tm -> ts );
    switch ( status )
    {
    case 0:
        break;
    case ETIMEDOUT:
        return RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted );
    case EAGAIN:
        return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcExhausted );
    case EDEADLK:
        return RC ( rcPS, rcRWLock, rcLocking, rcThread, rcDeadlock );
    case EINVAL:
        return RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcInvalid );
    default:
        return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
    }

    return 0;
}


/* AcquireExcl
 *  acquires write ( exclusive ) lock
 */
LIB_EXPORT rc_t CC KRWLockAcquireExcl ( KRWLock *self )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    status = pthread_rwlock_wrlock ( & self -> lock );
    switch ( status )
    {
    case 0:
        break;
    case EDEADLK:
        return RC ( rcPS, rcRWLock, rcLocking, rcThread, rcDeadlock );
    case EINVAL:
        return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcInvalid );
    default:
        return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
    }

    return 0;
}

LIB_EXPORT rc_t CC KRWLockTimedAcquireExcl ( KRWLock *self, timeout_t *tm )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    status = pthread_rwlock_trywrlock ( & self -> lock );
    switch ( status )
    {
    case 0:
        return 0;
    case EBUSY:
        if ( tm != NULL )
            break;
        return RC ( rcPS, rcRWLock, rcLocking, rcLock, rcBusy );
    case EINVAL:
        return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcInvalid );
    default:
        return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
    }

    if ( ! tm -> prepared )
        TimeoutPrepare ( tm );

    status = pthread_rwlock_timedwrlock ( & self -> lock, & tm -> ts );
    switch ( status )
    {
    case 0:
        break;
    case ETIMEDOUT:
        return RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted );
    case EDEADLK:
        return RC ( rcPS, rcRWLock, rcLocking, rcThread, rcDeadlock );
    case EINVAL:
        return RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcInvalid );
    default:
        return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
    }

    return 0;
}


/* Unlock
 *  releases lock
 */
LIB_EXPORT rc_t CC KRWLockUnlock ( KRWLock *self )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcUnlocking, rcSelf, rcNull );

    status = pthread_rwlock_unlock ( & self -> lock );
    switch ( status )
    {
    case 0:
        break;
    case EPERM:
        return RC ( rcPS, rcRWLock, rcUnlocking, rcThread, rcIncorrect );
    case EINVAL:
        return RC ( rcPS, rcRWLock, rcUnlocking, rcRWLock, rcInvalid );
    default:
        return RC ( rcPS, rcRWLock, rcUnlocking, rcNoObj, rcUnknown );
    }

    return 0;
}
