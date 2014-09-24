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
#include <kproc/thread.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdlib.h>

/*--------------------------------------------------------------------------
 * KThread
 *  a CPU execution thread
 */
struct KThread
{
    atomic32_t refcount;
    rc_t rc;
};


/* Whack
 */
static
rc_t KThreadWhack ( KThread *self )
{
    free ( self );
    return 0;
}


/* Make
 *  create and run a thread
 *
 *  "run_thread" [ IN ] - thread entrypoint
 *
 *  "data" [ IN, OPAQUE ] - user-supplied thread data
 */
LIB_EXPORT rc_t CC KThreadMake ( KThread **tp,
    rc_t ( * run_thread ) ( const KThread*, void* ), void *data )
{
    rc_t rc;
    if ( tp == NULL )
        rc = RC ( rcPS, rcThread, rcCreating, rcParam, rcNull );
    else
    {
        if ( run_thread == NULL )
            rc = RC ( rcPS, rcThread, rcCreating, rcFunction, rcNull );
        else
        {
            KThread *t = malloc ( sizeof * t );
            if ( t == NULL )
                rc = RC ( rcPS, rcThread, rcCreating, rcMemory, rcExhausted );
            else
            {
                /* run function on current thread */
                atomic32_set ( & t -> refcount, 1 );
                t -> rc = ( * run_thread ) ( t, data );
                * tp = t;
                return 0;
            }
        }

        * tp = NULL;
    }
    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KThreadAddRef ( const KThread *cself )
{
    if ( cself != NULL )
        atomic32_inc ( & ( ( KThread* ) cself ) -> refcount );
    return 0;
}

LIB_EXPORT rc_t CC KThreadRelease ( const KThread *cself )
{
    KThread *self = ( KThread* ) cself;
    if ( cself != NULL )
    {
        if ( atomic32_dec_and_test ( & self -> refcount ) )
            return KThreadWhack ( self );
    }
    return 0;
}


/* Cancel
 *  signal the thread to finish
 */
LIB_EXPORT rc_t CC KThreadCancel ( KThread *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcThread, rcSignaling, rcSelf, rcNull );
    return RC ( rcPS, rcThread, rcSignaling, rcThread, rcDestroyed );
}


/* Wait
 *  waits for a thread to exit
 *
 *  "status" [ OUT ] - return parameter for thread's exit code
 */
LIB_EXPORT rc_t CC KThreadWait ( KThread *self, rc_t *status )
{
    if ( self == NULL )
        return RC ( rcPS, rcThread, rcWaiting, rcSelf, rcNull );
    if ( status != NULL )
        * status = self -> rc;
    return 0;
}


/* Detach
 *  allow thread to run independently of group
 */
LIB_EXPORT rc_t CC KThreadDetach ( KThread *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcThread, rcDetaching, rcSelf, rcNull );
    return RC ( rcPS, rcThread, rcDetaching, rcThread, rcDestroyed );
}
