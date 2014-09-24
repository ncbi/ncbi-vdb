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
#include <kproc/barrier.h>
#include <klib/rc.h>
#include <atomic32.h>
#include <sysalloc.h>

#include <stdlib.h>

/*--------------------------------------------------------------------------
 * KBarrier
 *  a thread synchronization device
 *  detains all callers until the required number has been reached
 */
struct KBarrier
{
    atomic32_t refcount;
    uint32_t barrier;
    uint32_t count;
};


/* Whack
 */
static
rc_t KBarrierWhack ( KBarrier *self )
{
    free ( self );
    return 0;
}


/* Make
 *  create a barrier
 *
 *  "count" [ IN ] - the number of threads to block
 */
LIB_EXPORT rc_t CC KBarrierMake ( KBarrier **bp, uint32_t count )
{
    rc_t rc;
    if ( bp == NULL )
        rc = RC ( rcPS, rcBarrier, rcConstructing, rcParam, rcNull );
    else
    {
        KBarrier *b = malloc ( sizeof * b );
        if ( b == NULL )
            rc = RC ( rcPS, rcBarrier, rcConstructing, rcMemory, rcExhausted );
        else
        {
            atomic32_set ( & b -> refcount, 1 );
            b -> barrier = count;
            b -> count = 0;
            * bp = b;
            return 0;
        }

        * bp = NULL;
    }
    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KBarrierAddRef ( const KBarrier *cself )
{
    if ( cself != NULL )
        atomic32_inc ( & ( ( KBarrier* ) cself ) -> refcount );
    return 0;
}

LIB_EXPORT rc_t CC KBarrierRelease ( const KBarrier *cself )
{
    KBarrier *self = ( KBarrier* ) cself;
    if ( cself != NULL )
    {
        if ( atomic32_dec_and_test ( & self -> refcount ) )
        {
            atomic32_set ( & self -> refcount, 1 );
            return KBarrierWhack ( self );
        }
    }
    return 0;
}


/* Wait
 *  block until the required number of callers has been reached
 */
LIB_EXPORT rc_t CC KBarrierWait ( KBarrier *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcBarrier, rcWaiting, rcSelf, rcNull );

    if ( ++ self -> count < self -> barrier )
    {
        -- self -> count;
        return RC ( rcPS, rcBarrier, rcWaiting, rcThread, rcDeadlock );
    }

    return 0;
}
