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
#include <kproc/timeout.h>
#include <kproc/lock.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * KLock
 *  a mutual exclusion lock
 */

/* Whack
 */
static
rc_t KLockWhack ( KLock *self )
{
    if ( CloseHandle ( self -> mutex ) )
    {
        free ( self );
        return 0;
    }

    switch ( GetLastError () )
    {
    case ERROR_INVALID_HANDLE:
        return RC ( rcPS, rcLock, rcDestroying, rcLock, rcInvalid );
    }

    return RC ( rcPS, rcLock, rcDestroying, rcNoObj, rcUnknown );
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
            lock -> mutex = CreateMutex ( NULL, false, NULL );
            if ( lock -> mutex != NULL )
            {
                atomic32_set ( & lock -> refcount, 1 );
                * lockp = lock;
                return 0;
            }

            switch ( GetLastError () )
            {
            default:
                rc = RC ( rcPS, rcLock, rcConstructing, rcNoObj, rcUnknown );
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
    if ( self == NULL )
        return RC ( rcPS, rcLock, rcLocking, rcSelf, rcNull );

    switch ( WaitForSingleObject ( self -> mutex, INFINITE ) )
    {
    case WAIT_ABANDONED:
    case WAIT_OBJECT_0:
        return 0;
    case WAIT_TIMEOUT:
        return RC ( rcPS, rcLock, rcLocking, rcTimeout, rcExhausted );
    }

    switch ( GetLastError () )
    {
    case ERROR_INVALID_HANDLE:
        return RC ( rcPS, rcLock, rcLocking, rcLock, rcInvalid );
    }

    return RC ( rcPS, rcLock, rcLocking, rcNoObj, rcUnknown );
}

/* Unlock
 *  releases lock
 */
LIB_EXPORT rc_t CC KLockUnlock ( KLock *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcLock, rcUnlocking, rcSelf, rcNull );

    if ( ReleaseMutex ( self -> mutex ) )
        return 0;

    switch ( GetLastError () )
    {
    case ERROR_INVALID_HANDLE:
        return RC ( rcPS, rcLock, rcUnlocking, rcLock, rcInvalid );
    }

    return RC ( rcPS, rcLock, rcUnlocking, rcNoObj, rcUnknown );
}


/*--------------------------------------------------------------------------
 * KTimedLock
 *  a mutual exclusion lock with support for timed Acquire
 */

/* Whack
 */
static
rc_t KTimedLockWhack ( KTimedLock *self )
{
    if ( CloseHandle ( self -> mutex ) )
    {
        free ( self );
        return 0;
    }

    switch ( GetLastError () )
    {
    case ERROR_INVALID_HANDLE:
        return RC ( rcPS, rcLock, rcDestroying, rcLock, rcInvalid );
    }

    return RC ( rcPS, rcLock, rcDestroying, rcNoObj, rcUnknown );
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
            lock -> mutex = CreateMutex ( NULL, false, NULL );
            if ( lock -> mutex != NULL )
            {
                atomic32_set ( & lock -> refcount, 1 );
                * lockp = lock;
                return 0;
            }

            switch ( GetLastError () )
            {
            default:
                rc = RC ( rcPS, rcLock, rcConstructing, rcNoObj, rcUnknown );
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
    if ( tm == NULL )
        return KLockAcquire ( self );
    
    if ( self == NULL )
        return RC ( rcPS, rcLock, rcLocking, rcSelf, rcNull );

    switch ( WaitForSingleObject( self -> mutex, tm -> mS ) )
    {
    case WAIT_ABANDONED:
    case WAIT_OBJECT_0:
        return 0;
    case WAIT_TIMEOUT:
        if ( tm -> mS != 0 )
            return RC ( rcPS, rcLock, rcLocking, rcLock, rcBusy );
        return RC ( rcPS, rcLock, rcLocking, rcTimeout, rcExhausted );
    }

    switch ( GetLastError () )
    {
    case ERROR_INVALID_HANDLE:
        return RC ( rcPS, rcLock, rcLocking, rcLock, rcInvalid );
    }

    return RC ( rcPS, rcLock, rcLocking, rcNoObj, rcUnknown );
}


/* Unlock
 *  releases lock
 */
LIB_EXPORT rc_t CC KTimedLockUnlock ( KTimedLock *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcLock, rcUnlocking, rcSelf, rcNull );

    if ( ReleaseMutex ( self -> mutex ) )
        return 0;

    switch ( GetLastError () )
    {
    case ERROR_INVALID_HANDLE:
        return RC ( rcPS, rcLock, rcUnlocking, rcLock, rcInvalid );
    }

    return RC ( rcPS, rcLock, rcUnlocking, rcNoObj, rcUnknown );
}


/*--------------------------------------------------------------------------
 * KRWLock
 *  a read/write lock
 */
struct KRWLock
{
    HANDLE mutex;
    HANDLE rcond;
    HANDLE wcond;
    HANDLE ack;
    atomic32_t refcount;
    uint32_t rwait;
    uint32_t wwait;
    int32_t busy;
};


/* Whack
 */
static
rc_t KRWLockWhack ( KRWLock *self )
{
    switch ( WaitForSingleObject ( self -> mutex, 0 ) )
    {
    case WAIT_ABANDONED:
    case WAIT_OBJECT_0:
        if ( self -> busy != 0 || self -> rwait != 0 || self -> wwait != 0 )
        {
            ReleaseMutex ( self -> mutex );
            return RC ( rcPS, rcRWLock, rcDestroying, rcRWLock, rcBusy );
        }
        ReleaseMutex ( self -> mutex );
        break;
    case WAIT_TIMEOUT:
        return RC ( rcPS, rcRWLock, rcDestroying, rcRWLock, rcBusy );
    }

    CloseHandle ( self -> ack );
    CloseHandle ( self -> wcond );
    CloseHandle ( self -> rcond );
    CloseHandle ( self -> mutex );

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
            DWORD status;
            lock -> mutex = CreateMutex ( NULL, false, NULL );
            if ( lock -> mutex == NULL )
                status = GetLastError ();
            else
            {
                lock -> rcond = CreateEvent ( NULL, true, false, NULL );
                if ( lock -> rcond == NULL )
                    status = GetLastError ();
                else
                {
                    lock -> wcond = CreateEvent ( NULL, true, false, NULL );
                    if ( lock -> wcond == NULL )
                        status = GetLastError ();
                    else
                    {
                        lock -> ack = CreateEvent ( NULL, true, false, NULL );
                        if ( lock -> ack != NULL )
                        {
                            atomic32_set ( & lock -> refcount, 1 );
                            lock -> rwait = lock -> wwait = 0;
                            lock -> busy = 0;
                            * lockp = lock;
                            return 0;
                        }

                        status = GetLastError ();
                        CloseHandle ( lock -> wcond );
                    }

                    CloseHandle ( lock -> rcond );
                }

                CloseHandle ( lock -> mutex );
            }

            switch ( status )
            {
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
static
rc_t KRWLockAcquireSharedInt ( KRWLock *self, DWORD mS )
{
    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    /* acquire mutex */
    switch ( WaitForSingleObject ( self -> mutex, mS ) )
    {
    case WAIT_ABANDONED:
    case WAIT_OBJECT_0:
        break;
    case WAIT_TIMEOUT:
        return RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted );
    default:
        return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
    }

    /* don't allow readers if writers are waiting */
    if ( self -> rwait != 0 || self -> wwait != 0 || self -> busy < 0 )
    {
        /* indicate a waiting read lock request on our mutex */
        ++ self -> rwait;

        switch ( SignalObjectAndWait ( self -> mutex, self -> rcond, mS, false ) )
        {
        case WAIT_OBJECT_0:

            /* drop wait count under unlock mutex */
            -- self -> rwait;

            /* release unlock and reacquire */
            switch ( SignalObjectAndWait ( self -> ack, self -> mutex, INFINITE, false ) )
            {
            case WAIT_ABANDONED:
            case WAIT_OBJECT_0:
                break;
            case WAIT_FAILED:
                switch ( GetLastError () )
                {
                case ERROR_INVALID_HANDLE:
                    return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcInvalid );
                }
            default:
                return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
            }
            break;

        case WAIT_TIMEOUT:

            /* need to drop count, but don't have mutex */
            switch ( WaitForSingleObject ( self -> mutex, INFINITE ) )
            {
            case WAIT_ABANDONED:
            case WAIT_OBJECT_0:
                break;
            case WAIT_FAILED:
                switch ( GetLastError () )
                {
                case ERROR_INVALID_HANDLE:
                    return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcInvalid );
                }
            default:
                return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
            }

            /* drop count and go */
            -- self -> rwait;
            ReleaseMutex ( self -> mutex );
            return RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted );

        case WAIT_FAILED:
            switch ( GetLastError () )
            {
            case ERROR_INVALID_HANDLE:
                return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcInvalid );
            }

        default:
            return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
        }
    }

    assert ( self -> busy >= 0 );
    ++ self -> busy;

    ReleaseMutex ( self -> mutex );

    return 0;
}

LIB_EXPORT rc_t CC KRWLockAcquireShared ( KRWLock *self )
{
    return KRWLockAcquireSharedInt ( self, INFINITE );
}

LIB_EXPORT rc_t CC KRWLockTimedAcquireShared ( KRWLock *self, timeout_t *tm )
{
    return KRWLockAcquireSharedInt ( self, tm != NULL ? tm -> mS : 0 );
}


/* AcquireExcl
 *  acquires write ( exclusive ) lock
 */
static
rc_t KRWLockAcquireExclInt ( KRWLock *self, DWORD mS )
{
    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    /* acquire mutex */
    switch ( WaitForSingleObject ( self -> mutex, mS ) )
    {
    case WAIT_ABANDONED:
    case WAIT_OBJECT_0:
        break;
    case WAIT_TIMEOUT:
        return RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted );
    default:
        return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
    }

    if ( self -> wwait != 0 || self -> busy != 0 )
    {
        /* indicate a waiting write lock request on our mutex */
        ++ self -> wwait;

        switch ( SignalObjectAndWait ( self -> mutex, self -> wcond, mS, false ) )
        {
        case WAIT_OBJECT_0:

            /* drop wait count under unlock mutex */
            -- self -> wwait;

            /* release unlock and reacquire */
            switch ( SignalObjectAndWait ( self -> ack, self -> mutex, INFINITE, false ) )
            {
            case WAIT_ABANDONED:
            case WAIT_OBJECT_0:
                break;
            case WAIT_FAILED:
                switch ( GetLastError () )
                {
                case ERROR_INVALID_HANDLE:
                    return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcInvalid );
                }
            default:
                return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
            }
            break;

        case WAIT_TIMEOUT:

            /* need to drop count, but don't have mutex */
            switch ( WaitForSingleObject ( self -> mutex, INFINITE ) )
            {
            case WAIT_ABANDONED:
            case WAIT_OBJECT_0:
                break;
            case WAIT_FAILED:
                switch ( GetLastError () )
                {
                case ERROR_INVALID_HANDLE:
                    return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcInvalid );
                }
            default:
                return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
            }

            /* drop count and go */
            -- self -> wwait;
            ReleaseMutex ( self -> mutex );
            return RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted );

        case WAIT_FAILED:
            switch ( GetLastError () )
            {
            case ERROR_INVALID_HANDLE:
                return RC ( rcPS, rcRWLock, rcLocking, rcRWLock, rcInvalid );
            }

        default:
            return RC ( rcPS, rcRWLock, rcLocking, rcNoObj, rcUnknown );
        }
    }

    assert ( self -> busy == 0 );
    -- self -> busy;

    ReleaseMutex ( self -> mutex );

    return 0;
}

LIB_EXPORT rc_t CC KRWLockAcquireExcl ( KRWLock *self )
{
    return KRWLockAcquireExclInt ( self, INFINITE );
}

LIB_EXPORT rc_t CC KRWLockTimedAcquireExcl ( KRWLock *self, timeout_t *tm )
{
    return KRWLockAcquireExclInt ( self, tm != NULL ? tm -> mS : 0 );
}


/* Unlock
 *  releases lock
 */
LIB_EXPORT rc_t CC KRWLockUnlock ( KRWLock *self )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcUnlocking, rcSelf, rcNull );

    /* acquire mutex */
    switch ( WaitForSingleObject ( self -> mutex, INFINITE ) )
    {
    case WAIT_ABANDONED:
    case WAIT_OBJECT_0:
        break;
    case WAIT_TIMEOUT:
        return RC ( rcPS, rcRWLock, rcUnlocking, rcTimeout, rcExhausted );
    default:
        return RC ( rcPS, rcRWLock, rcUnlocking, rcNoObj, rcUnknown );
    }

    /* adjust busy */
    if ( self -> busy > 0 )
        -- self -> busy;
    else if ( self -> busy < 0 )
    {
        ++ self -> busy;
        assert ( self -> busy == 0 );
    }

    /* detect a zero crossing and waiters */
    if ( self -> busy != 0 || ( self -> wwait == 0 && self -> rwait == 0 ) )
        rc = 0;
    else
    {
        HANDLE cond = self -> wwait != 0 ? self -> wcond : self -> rcond;
        switch ( SignalObjectAndWait ( cond, self -> ack, INFINITE, false ) )
        {
        case WAIT_OBJECT_0:
            rc = 0;
            break;
        case WAIT_FAILED:
            switch ( GetLastError () )
            {
            case ERROR_INVALID_HANDLE:
                rc = RC ( rcPS, rcRWLock, rcUnlocking, rcRWLock, rcInvalid );
                break;
            default:
                rc = RC ( rcPS, rcRWLock, rcUnlocking, rcNoObj, rcUnknown );
            }
            break;

        default:
            rc = RC ( rcPS, rcRWLock, rcUnlocking, rcNoObj, rcUnknown );
        }
    }

    ReleaseMutex ( self -> mutex );
    return rc;
}
