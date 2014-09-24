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

#include "syscond-priv.h"
#include <kproc/extern.h>
#include <kproc/sem.h>
#include <kproc/cond.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdlib.h>

#if _DEBUGGING && 0
#define SMSG( msg, ... ) \
    KOutMsg ( msg, __VA_ARGS__ )
#else
#define SMSG( msg, ... ) \
    ( void ) 0
#endif


/*--------------------------------------------------------------------------
 * KSemaphore
 *  a metering device
 */
struct KSemaphore
{
    volatile uint64_t avail;
    volatile uint64_t requested;
    volatile uint64_t min_requested;
    KCondition* cond;
    volatile uint32_t waiting;
    volatile bool uniform;
};


/* Whack
 */
static
rc_t KSemaphoreWhack ( KSemaphore *self )
{
    rc_t rc;

    if ( self -> waiting != 0 )
        return RC ( rcPS, rcSemaphore, rcDestroying, rcSemaphore, rcBusy );

    rc = KConditionRelease ( self -> cond );
    if ( rc != 0 )
        return ResetRCContext ( rc, rcPS, rcSemaphore, rcDestroying );

    free ( self );
    return 0;
}


/* Make
 *
 *  "count" [ IN ] - initial count value
 */
LIB_EXPORT rc_t CC KSemaphoreMake ( KSemaphore **semp, uint64_t count )
{
    rc_t rc;
    if ( semp == NULL )
        rc = RC ( rcPS, rcSemaphore, rcConstructing, rcParam, rcNull );
    else
    {
        KSemaphore *sem = malloc ( sizeof * sem );
        if ( sem == NULL )
            rc = RC ( rcPS, rcSemaphore, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KConditionMake( & sem -> cond );
            if ( rc == 0 )
            {
                sem -> avail = count;
                sem -> requested = 0;
                sem -> min_requested = 0;
                sem -> waiting = 0;
                sem -> uniform = false;

                * semp = sem;
                return 0;
            }

            free ( sem );
        }

        * semp = NULL;
    }
    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KSemaphoreAddRef ( const KSemaphore *cself )
{
    if ( cself != NULL )
        return KConditionAddRef ( cself -> cond );
    return 0;
}

LIB_EXPORT rc_t CC KSemaphoreRelease ( const KSemaphore *cself )
{
    if ( cself != NULL )
    {
        if ( KConditionDropRef ( cself -> cond ) )
            return KSemaphoreWhack ( ( KSemaphore* ) cself );
    }
    return 0;
}


/* Wait
 *  block until a count becomes available
 *
 *  "lock" [ IN ] - externally acquired lock
 */
LIB_EXPORT rc_t CC KSemaphoreWait ( KSemaphore *self, struct KLock *lock )
{
    if ( self == NULL )
        return RC ( rcPS, rcSemaphore, rcWaiting, rcSelf, rcNull );

    if ( self -> avail == 0 )
    {
        if ( ++ self -> waiting == 1 )
        {
            self -> requested = self -> min_requested = 1;
            self -> uniform = true;
        }
        else if ( self -> requested != 1 )
        {
            self -> min_requested = 1;
            self -> uniform = false;
        }

        do
        {
            rc_t rc = KConditionWait ( self -> cond, lock );
            if ( rc != 0 )
            {
                -- self -> waiting;
                return ResetRCContext ( rc, rcPS, rcSemaphore, rcWaiting );
            }
        }
        while ( self -> avail == 0 );

        -- self -> waiting;
    }

    -- self -> avail;
    return 0;
}

/* TimedWait
 *  block until a count becomes available
 *
 *  "lock" [ IN ] - externally acquired lock
 *
 *  "tm" [ IN, NULL OKAY ] - optional timeout where
 *  NULL means timeout value of 0
 */
LIB_EXPORT rc_t CC KSemaphoreTimedWait ( KSemaphore *self,
    struct KLock *lock, struct timeout_t *tm )
{
    if ( self == NULL )
        return RC ( rcPS, rcSemaphore, rcWaiting, rcSelf, rcNull );

    if ( self -> avail == 0 )
    {
        SMSG ( "%s[%p]: avail == 0\n", __func__, self );
        if ( tm == NULL )
        {
            SMSG ( "%s[%p]: non-blocking mode - return timeout exhausted\n", __func__, self );
            return RC ( rcPS, rcSemaphore, rcWaiting, rcTimeout, rcExhausted );
        }

        if ( ++ self -> waiting == 1 )
        {
            SMSG ( "%s[%p]: first waiter\n", __func__, self );
            self -> requested = self -> min_requested = 1;
            self -> uniform = true;
        }
        else if ( self -> requested != 1 )
        {
            SMSG ( "%s[%p]: multiple waiters ( %u )\n", __func__, self, self -> waiting );
            self -> min_requested = 1;
            self -> uniform = false;
        }

        do
        {
            rc_t rc;

            SMSG ( "%s[%p]: wait on condition...\n", __func__, self );
            rc = KConditionTimedWait ( self -> cond, lock, tm );
            SMSG ( "%s[%p]:...done, rc = %R\n", __func__, self, rc );
            if ( rc != 0 )
            {
                SMSG ( "%s[%p]: timed out - decrementing wait count\n", __func__, self );
                -- self -> waiting;
                return ResetRCContext ( rc, rcPS, rcSemaphore, rcWaiting );
            }

            SMSG ( "%s[%p]: condition signaled - avail = %lu\n", __func__, self, self -> avail );
        }
        while ( self -> avail == 0 );

        SMSG ( "%s[%p]: finished waiting\n", __func__, self );
        -- self -> waiting;
    }

    SMSG ( "%s[%p]: decrementing count from %lu\n", __func__, self, self -> avail );
    -- self -> avail;
    return 0;
}


/* Cancel
 *  signal that the count will never increase
 */
LIB_EXPORT rc_t CC KSemaphoreCancel ( KSemaphore *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcSemaphore, rcSignaling, rcSelf, rcNull );

    if ( self -> waiting != 0 )
    {
        SMSG ( "%s[%p]: canceling %u waiters\n", __func__, self, self -> waiting );
        return KConditionBroadcast ( self -> cond );
    }

    SMSG ( "%s[%p]: cancel request ( no waiters )\n", __func__, self );
    return 0;
}


/* Signal
 *  signal that a count has become available
 */
LIB_EXPORT rc_t CC KSemaphoreSignal ( KSemaphore *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcSemaphore, rcSignaling, rcSelf, rcNull );

    ++ self -> avail;
    if ( self -> waiting != 0 && self -> avail >= self -> min_requested )
    {
        /* whacky logic
           - if everyone has requested the same amount
           - and only one request can be satisfied
           - then signal. otherwise, broadcast */
        if ( self -> uniform && self -> avail / self -> min_requested == 1 )
            return KConditionSignal ( self -> cond );
        return KConditionBroadcast ( self -> cond );
    }

    return 0;
}


/* Alloc
 *  allocate a count
 *  used for resource metering
 *
 *  "lock" [ IN ] - externally acquired lock
 *
 *  "count" [ IN ] - the resource count
 */
LIB_EXPORT rc_t CC KSemaphoreAlloc ( KSemaphore *self,
    struct KLock *lock, uint64_t count )
{
    if ( self == NULL )
        return RC ( rcPS, rcSemaphore, rcWaiting, rcSelf, rcNull );

    if ( self -> avail < count )
    {
        if ( ++ self -> waiting == 1 )
        {
            self -> requested = self -> min_requested = count;
            self -> uniform = true;
        }
        else if ( self -> requested != count )
        {
            if ( self -> min_requested > count )
                self -> min_requested = count;
            self -> uniform = false;
        }

        do
        {
            rc_t rc = KConditionWait ( self -> cond, lock );
            if ( rc != 0 )
            {
                -- self -> waiting;
                return ResetRCContext ( rc, rcPS, rcSemaphore, rcWaiting );
            }
        }
        while ( self -> avail < count );

        -- self -> waiting;
    }

    self -> avail -= count;
    return 0;
}

/* TimedAlloc
 *  allocate a count
 *  used for resource metering
 *
 *  "lock" [ IN ] - externally acquired lock
 *
 *  "count" [ IN ] - the resource count
 *
 *  "tm" [ IN, NULL OKAY ] - optional timeout where
 *  NULL means timeout value of 0
 */
LIB_EXPORT rc_t CC KSemaphoreTimedAlloc ( KSemaphore *self,
    struct KLock *lock, uint64_t count, struct timeout_t *tm )
{
    if ( self == NULL )
        return RC ( rcPS, rcSemaphore, rcWaiting, rcSelf, rcNull );

    if ( self -> avail < count )
    {
        if ( tm == NULL )
            return RC ( rcPS, rcSemaphore, rcWaiting, rcTimeout, rcExhausted );

        if ( ++ self -> waiting == 1 )
        {
            self -> requested = self -> min_requested = count;
            self -> uniform = true;
        }
        else if ( self -> requested != count )
        {
            if ( self -> min_requested > count )
                self -> min_requested = count;
            self -> uniform = false;
        }

        do
        {
            rc_t rc = KConditionTimedWait ( self -> cond, lock, tm );
            if ( rc != 0 )
            {
                -- self -> waiting;
                return ResetRCContext ( rc, rcPS, rcSemaphore, rcWaiting );
            }
        }
        while ( self -> avail < count );

        -- self -> waiting;
    }

    self -> avail -= count;
    return 0;
}


/* Free
 *  signal that resources have become available
 */
LIB_EXPORT rc_t CC KSemaphoreFree ( KSemaphore *self, uint64_t count )
{
    if ( self == NULL )
        return RC ( rcPS, rcSemaphore, rcSignaling, rcSelf, rcNull );

    self -> avail += count;
    if ( self -> waiting != 0 && self -> avail >= self -> min_requested )
    {
        /* whacky logic
           - if everyone has requested the same amount
           - and only one request can be satisfied
           - then signal. otherwise, broadcast */
        if ( self -> uniform && self -> avail / self -> min_requested == 1 )
            return KConditionSignal ( self -> cond );
        return KConditionBroadcast ( self -> cond );
    }

    return 0;
}


/* Count
 *  request the current resource usage
 *  valid only within lock
 *
 *  "count" [ OUT ] - return parameter for current count
 */
LIB_EXPORT rc_t CC KSemaphoreCount ( const KSemaphore *self, uint64_t *count )
{
    rc_t rc;
    if ( count == NULL )
        rc = RC ( rcPS, rcSemaphore, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcPS, rcSemaphore, rcAccessing, rcSelf, rcNull );
        else
        {
            * count = self -> avail;
            return 0;
        }

        * count = 0;
    }
    return rc;
}
