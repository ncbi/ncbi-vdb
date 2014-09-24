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
#include "syscond-priv.h"
#include <kproc/timeout.h>
#include <kproc/lock.h>
#include <kproc/cond.h>
#include <os-native.h>
#include <kproc/lock.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <errno.h>

/*--------------------------------------------------------------------------
 * pthread_mutex
 */
static
rc_t pthread_mutex_whack ( pthread_mutex_t *mutex )
{
    int status = pthread_mutex_destroy ( mutex );
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
    return 0;
}

static
rc_t pthread_mutex_acquire ( pthread_mutex_t *mutex )
{
/*    pthread_t t = pthread_self();
    fprintf(stdout, "pthread_mutex_lock(%p), thread=%x\n", mutex, t);*/
    int status = pthread_mutex_lock ( mutex );
/*fprintf(stdout, "pthread_mutex_lock, thread=%x, status = %d\n", t, status);*/
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

static
int pthread_mutex_tryacquire ( pthread_mutex_t *mutex )
{
/*    pthread_t t = pthread_self();
fprintf(stdout, "pthread_mutex_trylock(%p), thread=%x\n", mutex, t);*/
    int status = pthread_mutex_trylock ( mutex );
/*fprintf(stdout, "pthread_mutex_trylock, thread=%x, status = %d\n", t, status);*/
    return status;
}

static
rc_t pthread_mutex_release ( pthread_mutex_t *mutex )
{
/*    pthread_t t = pthread_self();
fprintf(stdout, "pthread_mutex_unlock(%p), thread=%x\n", mutex, t);*/
    int status = pthread_mutex_unlock ( mutex );
/*fprintf(stdout, "pthread_mutex_unlock, thread=%x, status = %d\n", t, status);*/
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
 * pthread_condition
 */
static
rc_t pthread_condition_init ( pthread_cond_t *cond )
 {
    int status = pthread_cond_init ( cond, NULL );
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

    return 0;
}

static
rc_t pthread_condition_whack ( pthread_cond_t *cond )
{
    int status = pthread_cond_destroy ( cond );
    switch ( status )
    {
    case 0:
        break;
    case EBUSY:
        return RC ( rcPS, rcLock, rcDestroying, rcCondition, rcBusy );
    case EINVAL:
        return RC ( rcPS, rcLock, rcDestroying, rcCondition, rcInvalid );
    default:
        return RC ( rcPS, rcLock, rcDestroying, rcNoObj, rcUnknown );
    }
    return 0;
}

static
rc_t pthread_condition_wait ( pthread_cond_t *cond, pthread_mutex_t *mutex )
{
/*    pthread_t t = pthread_self();
fprintf(stdout, "pthread_cond_wait(%p, %p), thread=%x\n", cond, mutex, t);*/
    int status = pthread_cond_wait( cond, mutex );
/*fprintf(stdout, "pthread_cond_wait, thread=%x, status = %d\n", t, status);*/
    
    switch ( status )
    {
    case 0:
        break;
    default:
        return RC ( rcPS, rcLock, rcSignaling, rcCondition, rcUnknown );
    }
    return 0;
}

static
rc_t pthread_condition_timedwait ( pthread_cond_t *cond, pthread_mutex_t *mutex, struct timespec *ts )
{
    int status = pthread_cond_timedwait ( cond, mutex, ts );
    switch ( status )
    {
    case 0:
        break;
    case ETIMEDOUT:
        return RC ( rcPS, rcCondition, rcWaiting, rcTimeout, rcExhausted );
    case EINTR:
        return RC ( rcPS, rcCondition, rcWaiting, rcThread, rcInterrupted );
    default:
        return RC ( rcPS, rcCondition, rcWaiting, rcNoObj, rcUnknown );
    }

    return 0;
}

static
rc_t pthread_condition_signal( pthread_cond_t *cond )
{
    int status = pthread_cond_signal ( cond );
    switch ( status )
    {
    case 0:
        break;
    default:
        return RC ( rcPS, rcCondition, rcSignaling, rcNoObj, rcUnknown );
    }

    return 0;
}

static
rc_t pthread_condition_broadcast ( pthread_cond_t *cond )
{
    int status = pthread_cond_broadcast ( cond );
    switch ( status )
    {
    case 0:
        break;
    default:
        return RC ( rcPS, rcCondition, rcSignaling, rcNoObj, rcUnknown );
    }

    return 0;
}

/*--------------------------------------------------------------------------
 * KLock
 *  a POSIX-style mutual exclusion lock
 */

/* Destroy
 */
static
rc_t KLockDestroy ( KLock *self )
{
    return pthread_mutex_whack ( & self -> mutex );
}

/* Whack
 */
static
rc_t KLockWhack ( KLock *self )
{
    rc_t rc = KLockDestroy ( self );
    if ( rc == 0 )
        free ( self );
    return rc;
}

/* Init
 */
static
rc_t KLockInit ( KLock *self )
{
    int status = pthread_mutex_init ( & self -> mutex, NULL );
    switch ( status )
    {
    case 0:
        atomic32_set ( & self -> refcount, 1 );
        return 0;
    case EAGAIN:
        return RC ( rcPS, rcLock, rcConstructing, rcResources, rcInsufficient );
    case ENOMEM:
        return RC ( rcPS, rcLock, rcConstructing, rcMemory, rcInsufficient );
    }

    return RC ( rcPS, rcLock, rcConstructing, rcNoObj, rcUnknown );
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
            rc = KLockInit ( lock );
            if ( rc == 0 )
            {
                * lockp = lock;
                return 0;
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
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcPS, rcLock, rcLocking, rcSelf, rcNull );
    else
    {
        rc = pthread_mutex_acquire ( & self -> mutex );
    }

    return rc;
}

/* Unlock
 *  releases lock
 */
LIB_EXPORT rc_t CC KLockUnlock ( KLock *self )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcPS, rcLock, rcUnlocking, rcSelf, rcNull );
        
    /* release the guy */
    rc = pthread_mutex_release ( & self -> mutex );

    return rc;
}

/*--------------------------------------------------------------------------
 * KTimedLock
 *  a POSIX-style mutual exclusion lock with support for timed acquire
 */

/* Destroy
 */
static
rc_t KTimedLockDestroy ( KTimedLock *self )
{
    rc_t rc = pthread_mutex_whack ( & self -> mutex );
    if ( rc == 0 )
    {
        pthread_mutex_whack ( & self -> cond_lock );
        pthread_condition_whack ( & self -> cond );
    }
    return rc;
}

/* Whack
 */
static
rc_t KTimedLockWhack ( KTimedLock *self )
{
    rc_t rc = KTimedLockDestroy ( self );
    if ( rc == 0 )
        free ( self );
    return rc;
}

/* Init
 */
static
rc_t KTimedLockInit ( KTimedLock *self )
{
    int status = pthread_mutex_init ( & self -> mutex, NULL );
    if ( status == 0 )
    {
        status = pthread_mutex_init ( & self -> cond_lock, NULL );
        if ( status == 0 )
        {
            status = pthread_cond_init ( & self -> cond, NULL );
            if ( status == 0 )
            {
                self -> waiters = 0;
                atomic32_set ( & self -> refcount, 1 );
                return 0;
            }

            pthread_mutex_destroy ( & self -> cond_lock );
        }

        pthread_mutex_destroy ( & self -> mutex );
    }

    switch ( status )
    {
    case EAGAIN:
        return RC ( rcPS, rcLock, rcConstructing, rcResources, rcInsufficient );
    case ENOMEM:
        return RC ( rcPS, rcLock, rcConstructing, rcMemory, rcInsufficient );
    }

    return RC ( rcPS, rcLock, rcConstructing, rcNoObj, rcUnknown );
 }


/* Make
 *  make a simple mutex
 */
LIB_EXPORT rc_t CC KTimedLockMake ( KTimedLock **lockp )
{
    rc_t rc;
    if ( lockp == NULL )
        rc = RC ( rcPS, rcLock, rcConstructing, rcParam, rcNull );
    else
    {
        KTimedLock *lock = malloc ( sizeof * lock );
        if ( lock == NULL )
            rc = RC ( rcPS, rcLock, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KTimedLockInit ( lock );
            if ( rc == 0 )
            {
                * lockp = lock;
                return 0;
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
LIB_EXPORT rc_t CC KTimedLockAddRef ( const KTimedLock *cself )
{
    if ( cself != NULL )
        atomic32_inc ( & ( ( KTimedLock* ) cself ) -> refcount );
    return 0;
}

LIB_EXPORT rc_t CC KTimedLockRelease ( const KTimedLock *cself )
{
    KTimedLock *self = ( KTimedLock* ) cself;
    if ( cself != NULL )
    {
        if ( atomic32_dec_and_test ( & self -> refcount ) )
        {
            atomic32_set ( & self -> refcount, 1 );
            return KTimedLockWhack ( self );
        }
    }
    return 0;
}


/* Acquire
 *  acquires lock
 */
LIB_EXPORT rc_t CC KTimedLockAcquire ( KTimedLock *self, timeout_t *tm )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcPS, rcLock, rcLocking, rcSelf, rcNull );

    if ( tm == NULL )
        return pthread_mutex_acquire ( & self -> mutex );

    /* this is ugly, but don't want to prepare inside lock */
    if ( ! tm -> prepared )
        TimeoutPrepare ( tm );

    rc = pthread_mutex_acquire ( & self -> cond_lock );
    if ( rc == 0 )
    {
        int status = pthread_mutex_tryacquire ( & self -> mutex );
        if ( status == EBUSY )
        {
            while ( 1 )
            {
                ++ self -> waiters;
                status = pthread_cond_timedwait ( & self -> cond, & self -> cond_lock, & tm -> ts );
                -- self -> waiters;

                if ( status == EINTR )
                    continue;
                if ( status != 0 )
                    break;
                status = pthread_mutex_tryacquire ( & self -> mutex );
                if ( status != EBUSY )
                    break;
            }
        }

        pthread_mutex_release ( & self -> cond_lock );

        switch ( status )
        {
        case 0:
            break;
        case ETIMEDOUT:
            rc = RC ( rcPS, rcLock, rcLocking, rcTimeout, rcExhausted );
            break;
        case EBUSY:
            rc = RC ( rcPS, rcLock, rcLocking, rcLock, rcBusy );
            break;
        case EINVAL:
            rc = RC ( rcPS, rcLock, rcLocking, rcLock, rcInvalid );
            break;
        default:
            rc = RC ( rcPS, rcLock, rcLocking, rcNoObj, rcUnknown );
        }
    }

    return rc;
}

/* Unlock
 *  releases lock
 */
LIB_EXPORT rc_t CC KTimedLockUnlock ( KTimedLock *self )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcPS, rcLock, rcUnlocking, rcSelf, rcNull );

    rc = pthread_mutex_acquire ( & self -> cond_lock );
    if ( rc == 0 )
    {
        /* release the guy */
        rc = pthread_mutex_release ( & self -> mutex );

        if ( self -> waiters != 0 )
            pthread_condition_signal ( & self -> cond );

        pthread_mutex_release ( & self -> cond_lock );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KRWLock
 *  a POSIX-style read/write lock
 */
struct KRWLock
{
    KLock lock;
    pthread_cond_t rcond;
    pthread_cond_t wcond;
    uint32_t rwait;
    uint32_t wwait;
    int32_t count;
    atomic32_t refcount;
    
    /* used in KRWLockTimedAcquire */
    pthread_mutex_t timed_lock;
    pthread_cond_t  timed_cond;
    uint32_t        timed_waiters;
};


/* Whack
 */
static
rc_t KRWLockWhack ( KRWLock *self )
{
    rc_t rc;
    if ( self -> count || self -> rwait || self -> wwait )
        return RC ( rcPS, rcRWLock, rcDestroying, rcRWLock, rcBusy );

    rc = KLockDestroy ( & self -> lock );
    if ( rc == 0 )
    {
        pthread_cond_destroy ( & self -> rcond );
        pthread_cond_destroy ( & self -> wcond );
       
        pthread_cond_destroy ( & self -> timed_cond );
        pthread_mutex_whack ( & self -> timed_lock );
        
        free ( self );
    }

    return rc;
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
            rc = KLockInit ( & lock -> lock );
            if ( rc == 0 )
            {
                rc = pthread_condition_init ( & lock -> rcond );
                if ( rc == 0 )
                {
                    rc = pthread_condition_init ( & lock -> wcond );
                    if ( rc == 0 )
                    {
                        rc = pthread_condition_init ( & lock -> timed_cond );
                        if ( rc == 0 )
                        {
                            int status = pthread_mutex_init ( & lock -> timed_lock, NULL );
                            if ( status == 0 )
                            {
                            
                                lock -> rwait = lock -> wwait = 0;
                                lock -> count = 0;
                                atomic32_set ( & lock -> refcount, 1 );
                                lock -> timed_waiters = 0;
                                * lockp = lock;
                                return 0;
                            }
                            pthread_cond_destroy ( & lock -> timed_cond );
                        }
                        pthread_cond_destroy ( & lock -> wcond );
                    }

                    pthread_cond_destroy ( & lock -> rcond );
                }

                KLockDestroy ( & lock -> lock );
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
    rc_t rc;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    rc = KLockAcquire ( & self -> lock );
    if ( rc == 0 )
    {
        ++ self -> rwait;
        while ( self -> count < 0 || self -> wwait != 0 )
        {
            rc = pthread_condition_wait ( & self -> rcond, & self -> lock . mutex );
            if ( rc != 0 )
                break;
        }
        -- self -> rwait;

        if ( rc == 0 )
        {
            ++ self -> count;
        }

        KLockUnlock ( & self -> lock );
    }

    return rc;
}

static
rc_t KRWLockTimedAcquire( KRWLock *self, timeout_t *tm )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcPS, rcLock, rcLocking, rcSelf, rcNull );

    if ( tm == NULL )
        return pthread_mutex_acquire ( & self -> lock . mutex );

    /* this is ugly, but don't want to prepare inside lock */
    if ( ! tm -> prepared )
        TimeoutPrepare ( tm );

    rc = pthread_mutex_acquire ( & self -> timed_lock );
    if ( rc == 0 )
    {
        int status = pthread_mutex_tryacquire ( & self -> lock . mutex );
        if ( status == EBUSY )
        {
            while ( 1 )
            {
                ++ self -> timed_waiters;
                status = pthread_cond_timedwait ( & self -> timed_cond, & self -> timed_lock, & tm -> ts );
                -- self -> timed_waiters;

                if ( status == EINTR )
                    continue;
                if ( status != 0 )
                    break;
                status = pthread_mutex_tryacquire ( & self -> lock . mutex );
                if ( status != EBUSY )
                    break;
            }
        }

        pthread_mutex_release ( & self -> timed_lock );

        switch ( status )
        {
        case 0:
            break;
        case ETIMEDOUT:
            rc = RC ( rcPS, rcLock, rcLocking, rcTimeout, rcExhausted );
            break;
        case EBUSY:
            rc = RC ( rcPS, rcLock, rcLocking, rcLock, rcBusy );
            break;
        case EINVAL:
            rc = RC ( rcPS, rcLock, rcLocking, rcLock, rcInvalid );
            break;
        default:
            rc = RC ( rcPS, rcLock, rcLocking, rcNoObj, rcUnknown );
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC KRWLockTimedAcquireShared ( KRWLock *self, timeout_t *tm )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    rc = KRWLockTimedAcquire ( self, tm );
    if ( rc == 0 )
    {
        ++ self -> rwait;
        while ( self -> count < 0 || self -> wwait != 0 )
        {
            rc = pthread_condition_timedwait ( & self -> rcond, & self -> lock .  mutex, & tm -> ts );
            if ( rc != 0 )
            {
                if ( GetRCState ( rc ) == rcExhausted && GetRCObject ( rc ) == rcTimeout )
                    rc = ResetRCContext ( rc, rcPS, rcRWLock, rcLocking );
                break;
            }
        }
        -- self -> rwait;

        if ( rc == 0 )
        {
            ++ self -> count;
        }

        KLockUnlock ( & self -> lock );
    }

    return rc;
}

/* AcquireExcl
 *  acquires write ( exclusive ) lock
 */
LIB_EXPORT rc_t CC KRWLockAcquireExcl ( KRWLock *self )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    rc = KLockAcquire ( & self -> lock );
    if ( rc == 0 )
    {
        ++ self -> wwait;
        while ( self -> count != 0 )
        {
            rc = pthread_condition_wait ( & self -> wcond, & self -> lock . mutex );
            if ( rc != 0 )
                break;
        }
        -- self -> wwait;

        if ( rc == 0 )
        {
            self -> count = -1;
        }

        KLockUnlock ( & self -> lock );
    }

    return rc;
}

LIB_EXPORT rc_t CC KRWLockTimedAcquireExcl ( KRWLock *self, timeout_t *tm )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    rc = KRWLockTimedAcquire ( self, tm );
    if ( rc == 0 )
    {
        ++ self -> wwait;
        while ( self -> count != 0 )
        {
            rc = pthread_condition_timedwait ( & self -> wcond, & self -> lock . mutex, & tm -> ts );
            if ( rc != 0 )
            {
                if ( GetRCState ( rc ) == rcExhausted && GetRCObject ( rc ) == rcTimeout )
                    rc = ResetRCContext ( rc, rcPS, rcRWLock, rcLocking );
                break;
            }
        }
        -- self -> wwait;

        if ( rc == 0 )
        {
            self -> count = -1;
        }

        KLockUnlock ( & self -> lock );
    }

    return rc;
}

/* Unlock
 *  releases lock
 */
LIB_EXPORT rc_t CC KRWLockUnlock ( KRWLock *self )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcUnlocking, rcSelf, rcNull );

    rc = KLockAcquire ( & self -> lock );
    if ( rc == 0 )
    {
        /* release the count */
        if ( self -> count < 0 )
            self -> count = 0;
        else if ( self -> count > 0 )
            -- self -> count;

        /* if there are writers waiting... */
        if ( self -> wwait != 0 )
        {
            /* don't bother unless the lock is free */
            if ( self -> count == 0 )
                pthread_condition_signal ( & self -> wcond );
        }

        /* if there are readers waiting */
        else if ( self -> rwait != 0 )
        {
            /* any number of readers can come through now */
            pthread_condition_broadcast ( & self -> rcond );
        }

        KLockUnlock ( & self -> lock );
    }

    return rc;
}
