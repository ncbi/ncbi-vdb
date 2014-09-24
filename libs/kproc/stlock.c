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
#include <kproc/lock.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdlib.h>

/*--------------------------------------------------------------------------
 * KLock
 *  a POSIX-style mutual exclusion lock
 */
struct KLock
{
    atomic32_t refcount;
    int32_t count;
};


/* Whack
 */
static
rc_t KLockWhack ( KLock *self )
{
    if ( self -> count > 0 )
        return RC ( rcPS, rcLock, rcDestroying, rcLock, rcBusy );

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
            atomic32_set ( & lock -> refcount, 1 );
            lock -> count = 0;
            * lockp = lock;
            return 0;
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

    ++ self -> count;

    return 0;
}

LIB_EXPORT rc_t CC KLockTimedAcquire ( KLock *self, struct timeout_t *tm )
{
    if ( self == NULL )
        return RC ( rcPS, rcLock, rcLocking, rcSelf, rcNull );

    ++ self -> count;

    return 0;
}

/* Unlock
 *  releases lock
 */
LIB_EXPORT rc_t CC KLockUnlock ( KLock *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcLock, rcUnlocking, rcSelf, rcNull );

    /* pthread_mutex_unlock's man page states that the only
       unlock error we could see is that another thread tries
       to release a lock. what if the lock is not locked? */
    if ( self -> count <= 0 )
        return RC ( rcPS, rcLock, rcUnlocking, rcThread, rcIncorrect );

    -- self -> count;

    return 0;
}


/*--------------------------------------------------------------------------
 * KRWLock
 *  a POSIX-style read/write lock
 */
struct KRWLock
{
    atomic32_t refcount;
    int32_t count;
};


/* Whack
 */
static
rc_t KRWLockWhack ( KRWLock *self )
{
    if ( self -> count != 0 )
        return RC ( rcPS, rcRWLock, rcDestroying, rcRWLock, rcBusy );

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
            atomic32_set ( & lock -> refcount, 1 );
            lock -> count = 0;
            * lockp = lock;
            return 0;
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
    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    if ( self -> count < 0 )
        return RC ( rcPS, rcRWLock, rcLocking, rcThread, rcDeadlock );

    ++ self -> count;
    return 0;
}

LIB_EXPORT rc_t CC KRWLockTimedAcquireShared ( KRWLock *self, struct timeout_t *tm )
{
    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    if ( self -> count < 0 )
    {
        if ( tm == NULL )
            return RC ( rcPS, rcLock, rcLocking, rcRWLock, rcBusy );
        return RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted );
    }

    ++ self -> count;
    return 0;
}


/* AcquireExcl
 *  acquires write ( exclusive ) lock
 */
LIB_EXPORT rc_t CC KRWLockAcquireExcl ( KRWLock *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    if ( self -> count != 0 )
        return RC ( rcPS, rcRWLock, rcLocking, rcThread, rcDeadlock );

    -- self -> count;
    return 0;
}

LIB_EXPORT rc_t CC KRWLockTimedAcquireExcl ( KRWLock *self, struct timeout_t *tm )
{
    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcLocking, rcSelf, rcNull );

    if ( self -> count != 0 )
    {
        if ( tm == NULL )
            return RC ( rcPS, rcLock, rcLocking, rcRWLock, rcBusy );
        return RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted );
    }

    -- self -> count;
    return 0;
}


/* Unlock
 *  releases lock
 */
LIB_EXPORT rc_t CC KRWLockUnlock ( KRWLock *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcRWLock, rcUnlocking, rcSelf, rcNull );

    if ( self -> count < 0 )
        self -> count = 0;
    else if ( self -> count > 0 )
        -- self -> count;
    else
        return RC ( rcPS, rcRWLock, rcUnlocking, rcThread, rcIncorrect );

    return 0;
}
