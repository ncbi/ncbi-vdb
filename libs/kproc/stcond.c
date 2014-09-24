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
#include <kproc/cond.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdlib.h>


/*--------------------------------------------------------------------------
 * KCondition
 *  a POSIX-style condition object
 */
struct KCondition
{
    atomic32_t refcount;
};


/* Whack
 */
static
rc_t KConditionWhack ( KCondition *self )
{
    free ( self );
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
            atomic32_set ( & cond -> refcount, 1 );
            * condp = cond;
            return 0;
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
        if ( atomic32_dec_and_test ( & self -> refcount ) )
            return KConditionWhack ( self );
    }
    return 0;
}


/* Wait
 *  block on external lock until signaled
 */
LIB_EXPORT rc_t CC KConditionWait ( KCondition *self, struct KLock *lock )
{
    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcSelf, rcNull );
    if ( lock == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcLock, rcNull );

    return RC ( rcPS, rcCondition, rcWaiting, rcThread, rcDeadlock );
}

LIB_EXPORT rc_t CC KConditionTimedWait ( KCondition *self, struct KLock *lock, struct timeout_t *tm )
{
    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcSelf, rcNull );
    if ( lock == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcLock, rcNull );
    if ( tm == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcTimeout, rcNull );

    return RC ( rcPS, rcCondition, rcWaiting, rcThread, rcDeadlock );
}


/* Signal
 *  signal waiting threads
 *  awaken at most a single thread
 */
LIB_EXPORT rc_t CC KConditionSignal ( KCondition *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcSignaling, rcSelf, rcNull );

    return RC ( rcPS, rcCondition, rcSignaling, rcThread, rcDeadlock );
}


/* Broadcast
 *  signal waiting threads
 *  awaken all waiting thread
 */
LIB_EXPORT rc_t CC KConditionBroadcast ( KCondition *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcSignaling, rcSelf, rcNull );

    return RC ( rcPS, rcCondition, rcSignaling, rcThread, rcDeadlock );
}
