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

#include <kproc/q-extern.h>
#include <kproc/queue.h>
#include <kproc/timeout.h>
#include <kproc/lock.h>
#include <kproc/sem.h>
#include <klib/out.h>
#include <klib/status.h>
#include <klib/rc.h>
#include <atomic32.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <assert.h>

#if _DEBUGGING && 1
#define QMSG( msg, ... ) \
    KOutMsg ( msg, __VA_ARGS__ )
#else
#define QMSG( msg, ... ) \
    ( void ) 0
#endif

/*--------------------------------------------------------------------------
 * KQueue
 *  a simple thread-safe queue structure supporting push/pop operation
 *  makes use of semaphore objects for synchronization
 */
struct KQueue
{
    KSemaphore *rc;
    KSemaphore *wc;

    KLock *rl;
    KLock *wl;

    uint32_t capacity;
    uint32_t bmask, imask;
    volatile uint32_t read, write;
    atomic32_t refcount;
    volatile bool sealed;
    uint8_t align [ 7 ];
    void *buffer [ 16 ];
};


/* Whack
 */
static
rc_t KQueueWhack ( KQueue *self )
{
    rc_t rc;
    QMSG ( "%s: releasing write semaphore\n", __func__ );
    rc = KSemaphoreRelease ( self -> wc );
    if ( rc == 0 )
    {
        QMSG ( "%s: releasing read semaphore\n", __func__ );
        KSemaphoreRelease ( self -> rc );
        QMSG ( "%s: releasing write lock\n", __func__ );
        KLockRelease ( self -> wl );
        QMSG ( "%s: releasing read lock\n", __func__ );
        KLockRelease ( self -> rl );
        free ( self );
        QMSG ( "%s: done\n", __func__ );
    }
    return rc;
}

/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KQueueAddRef ( const KQueue *cself )
{
    if ( cself != NULL )
        atomic32_inc ( & ( ( KQueue* ) cself ) -> refcount );
    return 0;
}

LIB_EXPORT rc_t CC KQueueRelease ( const KQueue *cself )
{
    KQueue *self = ( KQueue* ) cself;
    if ( cself != NULL )
    {
        if ( atomic32_dec_and_test ( & self -> refcount ) )
            return KQueueWhack ( self );
    }
    return 0;
}

/* Make
 * create an empty queue object
 *
 *  "capacity" [ IN ] - minimum queue length
 *  always expands to a power of 2, i.e. providing
 *  a length of 10 will result in a length of 16.
 */
LIB_EXPORT rc_t CC KQueueMake ( KQueue **qp, uint32_t capacity )
{
    rc_t rc;
    if ( qp == NULL )
        rc = RC ( rcCont, rcQueue, rcConstructing, rcParam, rcNull );
    else
    {
        KQueue *q;

        uint32_t cap = 1;
        while ( cap < capacity )
            cap += cap;

        q = malloc ( sizeof * q - sizeof q -> buffer + cap * sizeof q -> buffer [ 0 ] );
        if ( q == NULL )
            rc = RC ( rcCont, rcQueue, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KSemaphoreMake ( & q -> rc, 0 );
            if ( rc == 0 )
            {
                rc = KSemaphoreMake ( & q -> wc, cap );
                if ( rc == 0 )
                {
                    rc = KLockMake ( & q -> rl );
                    if ( rc == 0 )
                    {
                        rc = KLockMake ( & q -> wl );
                        if ( rc == 0 )
                        {
                            q -> capacity = cap;
                            q -> bmask = cap - 1;
                            q -> imask = ( cap + cap ) - 1;
                            q -> read = q -> write = 0;
                            atomic32_set ( & q -> refcount, 1 );
                            q -> sealed = false;

                            QMSG ( "%s: created queue with capacity %u, "
                                   "bmask %#032b, imask %#032b.\n"
                                   , __func__, q -> capacity, q -> bmask, q -> imask
                                );

                            * qp = q;
                            return 0;
                        }

                        KLockRelease ( q -> rl );
                    }

                    KSemaphoreRelease ( q -> wc );
                }

                KSemaphoreRelease ( q -> rc );
            }
            free ( q );
        }
        * qp = NULL;
    }
    return rc;
}

/* Push
 *  add an object to the queue
 *
 *  "item" [ IN, OPAQUE ] - pointer to item being queued
 *
 *  "tm" [ IN, NULL OKAY ] - pointer to system specific timeout
 *  structure. if the queue is full, wait for indicated period
 *  of time for space to become available, or return status
 *  code indicating a timeout. when NULL and queue is full,
 *  Push will time out immediately and return status code.
 */
LIB_EXPORT rc_t CC KQueuePush ( KQueue *self, const void *item, timeout_t *tm )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcCont, rcQueue, rcInserting, rcSelf, rcNull );
    if ( self -> sealed )
    {
        QMSG ( "%s: failed to insert into queue due to seal\n", __func__ );
        return RC ( rcCont, rcQueue, rcInserting, rcQueue, rcReadonly );
    }
    if ( item == NULL )
        return RC ( rcCont, rcQueue, rcInserting, rcTimeout, rcNull );

    QMSG ( "%s: acquiring write lock ( %p )...\n", __func__, self -> wl );
    rc = KLockAcquire ( self -> wl );
    QMSG ( "%s: ...done, rc = %R\n", __func__, rc );
    if ( rc == 0 )
    {
        QMSG ( "%s: waiting on write semaphore...\n", __func__ );
        rc = KSemaphoreTimedWait ( self -> wc, self -> wl, tm );
        QMSG ( "%s: ...done, rc = %R.\n", __func__, rc );

        if ( rc == 0 )
        {
            uint32_t w;

            /* re-check the seal */
            if ( self -> sealed )
            {
                QMSG ( "%s: queue has been sealed\n", __func__ );

                /* not a disaster if semaphore not signaled */
                QMSG ( "%s: signaling write semaphore\n", __func__ );
                KSemaphoreSignal ( self -> wc );
                QMSG ( "%s: unlocking write lock\n", __func__ );
                KLockUnlock ( self -> wl );

                QMSG ( "%s: failed to insert into queue due to seal\n", __func__ );
                return RC ( rcCont, rcQueue, rcInserting, rcQueue, rcReadonly );
            }

            /* insert item */
            w = self -> write & self -> imask;
            QMSG ( "%s: write index is %u, masked against 0x%x\n", __func__, w, self -> imask );
            self -> buffer [ w & self -> bmask ] = ( void* ) item;
            QMSG ( "%s: inserted item into buffer [ %u ], using mask 0x%x\n", __func__, w & self -> bmask, self -> bmask );
            self -> write = w + 1;

            QMSG ( "%s: unlocking write lock ( %p ).\n", __func__, self -> wl );
            KLockUnlock ( self -> wl );

            /* let listeners know about item */
            QMSG ( "%s: acquiring read lock ( %p )\n", __func__, self -> rl );
            if ( KLockAcquire ( self -> rl ) == 0 )
            {
                QMSG ( "%s: signaling read semaphore\n", __func__ );
                KSemaphoreSignal ( self -> rc );
                QMSG ( "%s: unlocking read lock ( %p )\n", __func__, self -> rl );
                KLockUnlock ( self -> rl );
            }
        }
        else
        {
            QMSG ( "%s: unlocking write lock ( %p ).\n", __func__, self -> wl );
            KLockUnlock ( self -> wl );
        }
    }

    return rc;
}

/* Pop
 *  pop an object from queue
 *
 *  "item" [ OUT, OPAQUE* ] - return parameter for popped item
 *
 *  "tm" [ IN, NULL OKAY ] - pointer to system specific timeout
 *  structure. if the queue is empty, wait for indicated period
 *  of time for an object to become available, or return status
 *  code indicating a timeout. when NULL and queue is empty,
 *  Pop will time out immediately and return status code.
 */
LIB_EXPORT rc_t CC KQueuePop ( KQueue *self, void **item, timeout_t *tm )
{
    rc_t rc;

    if ( item == NULL )
        rc = RC ( rcCont, rcQueue, rcRemoving, rcParam, rcNull );
    else
    {
        * item = NULL;

        if ( self == NULL )
            rc = RC ( rcCont, rcQueue, rcRemoving, rcSelf, rcNull );
        else
        {
            QMSG ( "%s: acquiring read lock ( %p )\n", __func__, self -> rl );
            rc = KLockAcquire ( self -> rl );
            if ( rc == 0 )
            {
                QMSG ( "%s: waiting on read semaphore...\n", __func__ );
                rc = KSemaphoreTimedWait ( self -> rc, self -> rl, self -> sealed ? NULL : tm );
                QMSG ( "%s: ...done, rc = %R.\n", __func__, rc );

                if ( rc == 0 )
                {
                    uint32_t r, idx;

                    /* got an element */
                    QMSG ( "%s: asserting  self -> read ( %u ) != self -> write ( %u )\n",
                           __func__, self -> read, self -> write
                        );
                    assert ( self -> read != self -> write );

                    /* read element */
                    r = self -> read & self -> imask;
                    QMSG ( "%s: read index is %u, masked against 0x%x\n", __func__, r, self -> imask );
                    idx = r & self -> bmask;
                    * item = self -> buffer [ idx ];
                    QMSG ( "%s: read item from buffer [ %u ], using mask 0x%x\n", __func__, idx, self -> bmask );
                    self -> buffer [ idx ] = NULL;
                    self -> read = r + 1;

                    QMSG ( "%s: unlocking read lock. ( %p )\n", __func__, self -> rl );
                    KLockUnlock ( self -> rl );

                    /* let write know there's a free slot available */
                    QMSG ( "%s: acquiring write lock ( %p )\n", __func__, self -> wl );
                    if ( KLockAcquire ( self -> wl ) == 0 )
                    {
                        QMSG ( "%s: signaling write semaphore\n", __func__ );
                        KSemaphoreSignal ( self -> wc );
                        QMSG ( "%s: unlocking write lock ( %p )\n", __func__, self -> wl );
                        KLockUnlock ( self -> wl );
                    }
                }
                else
                {
                    QMSG ( "%s: unlocking read lock. ( %p )\n", __func__, self -> rl );
                    KLockUnlock ( self -> rl );

                    if ( self -> sealed )
                    {
                        switch ( ( int ) GetRCObject ( rc ) )
                        {
                        case ( int ) rcTimeout:
                        case ( int ) rcSemaphore:
                            rc = RC ( rcCont, rcQueue, rcRemoving, rcData, rcDone );
                            QMSG ( "%s: resetting rc to %R\n", __func__, rc );
                            break;
                        }
                    }
                }
            }
        }
    }

    return rc;
}

/* Sealed
 *  ask if the queue has been closed off
 *  meaning there will be no further push operations
 *
 *  NB - if "self" is NULL, the return value is "true"
 *  since a NULL queue cannot accept items via push
 */
LIB_EXPORT bool CC KQueueSealed ( const KQueue *self )
{
    QMSG ( "%s called\n", __func__ );
    if ( self != NULL )
        return self -> sealed;
    return false;
}


/* Seal
 *  indicate that the queue has been closed off
 *  meaning there will be no further push operations
 *  if "writes" is true, and no further pop operations
 *  otherwise.
 */
LIB_EXPORT rc_t CC KQueueSeal ( KQueue *self )
{
    rc_t rc = 0;

    QMSG ( "%s called\n", __func__ );

    if ( self == NULL )
        return RC ( rcCont, rcQueue, rcFreezing, rcSelf, rcNull );

    self -> sealed = true;

#if 1
    QMSG ( "%s: acquiring write lock ( %p )\n", __func__, self -> wl );
    rc = KLockAcquire ( self -> wl );
    if ( rc == 0 )
    {
        QMSG ( "%s: canceling write semaphore...\n", __func__ );
        rc = KSemaphoreCancel ( self -> wc );
        QMSG ( "%s: ...done, rc = %R.\n", __func__, rc );
        KLockUnlock ( self -> wl );

        if ( rc == 0 )
        {
            QMSG ( "%s: acquiring read lock ( %p )\n", __func__, self -> rl );
            rc = KLockAcquire ( self -> rl );
            if ( rc == 0 )
            {
                QMSG ( "%s: canceling read semaphore...\n", __func__ );
                rc = KSemaphoreCancel ( self -> rc );
                QMSG ( "%s: ...done, rc = %R.\n", __func__, rc );
                KLockUnlock ( self -> rl );
            }
        }
    }
#endif

    return rc;
}
