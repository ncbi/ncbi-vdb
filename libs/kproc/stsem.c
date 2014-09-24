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
#include <kproc/sem.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdlib.h>


/*--------------------------------------------------------------------------
 * KSemaphore
 *  a metering device
 */
struct KSemaphore
{
    uint64_t avail;
    atomic32_t refcount;
};


/* Whack
 */
static
rc_t KSemaphoreWhack ( KSemaphore *self )
{
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
            sem -> avail = count;
            atomic32_set ( & sem -> refcount, 1 );
            * semp = sem;
            return 0;
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
        atomic32_inc ( & ( ( KSemaphore* ) cself ) -> refcount );
    return 0;
}

LIB_EXPORT rc_t CC KSemaphoreRelease ( const KSemaphore *cself )
{
    KSemaphore *self = ( KSemaphore* ) cself;
    if ( cself != NULL )
    {
        if ( atomic32_dec_and_test ( & self -> refcount ) )
            return KSemaphoreWhack ( self );
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
        return RC ( rcPS, rcSemaphore, rcWaiting, rcThread, rcDeadlock );

    -- self -> avail;
    return 0;
}

LIB_EXPORT rc_t CC KSemaphoreTimedWait ( KSemaphore *self,
    struct KLock *lock, struct timeout_t *tm )
{
    if ( self == NULL )
        return RC ( rcPS, rcSemaphore, rcWaiting, rcSelf, rcNull );

    if ( self -> avail == 0 )
    {
        if ( tm == NULL )
            return RC ( rcPS, rcSemaphore, rcWaiting, rcTimeout, rcNull );

        return RC ( rcPS, rcSemaphore, rcWaiting, rcThread, rcDeadlock );
    }

    -- self -> avail;
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
        return RC ( rcPS, rcSemaphore, rcWaiting, rcThread, rcDeadlock );

    self -> avail -= count;
    return 0;
}

LIB_EXPORT rc_t CC KSemaphoreTimedAlloc ( KSemaphore *self,
    struct KLock *lock, uint64_t count, struct timeout_t *tm )
{
    if ( self == NULL )
        return RC ( rcPS, rcSemaphore, rcWaiting, rcSelf, rcNull );

    if ( self -> avail < count )
    {
        if ( tm == NULL )
            return RC ( rcPS, rcSemaphore, rcWaiting, rcTimeout, rcNull );

        return RC ( rcPS, rcSemaphore, rcWaiting, rcThread, rcDeadlock );
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
