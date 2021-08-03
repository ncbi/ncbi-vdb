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
#include <klib/log.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <pthread.h>
#include <stdlib.h>
#include <errno.h>


/*--------------------------------------------------------------------------
 * KThread
 *  a CPU execution thread
 */
struct KThread
{
    /* thread entrypoint and data */
    rc_t ( * run ) ( const KThread*, void* );
    void *data;

    pthread_t thread;
    atomic32_t waiting;
    atomic32_t refcount;
    rc_t rc;
    bool join;
};


/* Whack
 */
static
rc_t KThreadWhack ( KThread *self )
{
    if ( self -> join )
    {
        void *td;
        pthread_join ( self -> thread, & td );
    }

    free ( self );
    return 0;
}


/* Run
 */
static
void *KThreadRun ( void *td )
{
    KThread *self = td;

    /* run the function */
    self -> rc = ( * self -> run ) ( self, self -> data );

    /* release thread's reference */
    if ( atomic32_dec_and_test ( & self -> refcount ) )
        free ( self );

    return NULL;
}

#ifndef GUARD_SIZE
#define GUARD_SIZE (1 * 1024 * 1014)
#endif
#ifndef GUARD_SIZE_RETRY_DEFAULT
#define GUARD_SIZE_RETRY_DEFAULT 0
#endif

/* Make
 *  create and run a thread
 *
 *  "run_thread" [ IN ] - thread entrypoint
 *
 *  "data" [ IN, OPAQUE ] - user-supplied thread data
 */
LIB_EXPORT rc_t CC KThreadMakeStackSize ( KThread **tp,
    rc_t ( CC * run_thread ) ( const KThread*, void* ), void *data,
    size_t stacksize )
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
            KThread *t = calloc (1, sizeof * t);
            if ( t == NULL )
                rc = RC ( rcPS, rcThread, rcCreating, rcMemory, rcExhausted );
            else
            {
                size_t guardsize = GUARD_SIZE;

                /* finish constructing thread */
                t -> run = run_thread;
                t -> data = data;
                atomic32_set ( & t -> waiting, 0 );
                atomic32_set ( & t -> refcount, 2 );
                t -> join = true;
                *tp = t; /* it is not thread-safe to assign this AFTER the thread has started */

                for ( ; ; ) {
                    int status;
                    pthread_attr_t attr;
                    
                    pthread_attr_init(&attr); // initializes to default values
                    if (stacksize != 0)
                    {
                        size_t default_stacksize = 0;

                        pthread_attr_getstacksize(&attr, &default_stacksize);
                        pthread_attr_setstacksize(&attr, stacksize);
                        pLogMsg(klogDebug, "requesting stack size $(sz), default was $(ds)", "sz=%zu,ds=%zu", stacksize, default_stacksize);
                    }
                    if (guardsize != 0) {
                        size_t default_guardsize = 0;

                        pthread_attr_getguardsize(&attr, &default_guardsize);
                        pthread_attr_setguardsize(&attr, guardsize);
                        pLogMsg(klogDebug, "requesting guard size $(sz), default was $(ds)", "sz=%zu,ds=%zu", guardsize, default_guardsize);
                    }

                    /* attempt to create thread */
                    status = pthread_create ( & t -> thread, &attr, KThreadRun, t );
                    pthread_attr_destroy(&attr);
                    if ( status == 0 )
                    {
                        return 0;
                    }

                    /* see why we failed */
                    switch ( status )
                    {
                    case EAGAIN:
                        rc = RC ( rcPS, rcThread, rcCreating, rcThread, rcExhausted );
                        break;
#if GUARD_SIZE_RETRY_DEFAULT
                    case EINVAL:
                        if (guardsize != 0) {
                            guardsize = 0;
#if GUARD_SIZE
                            pLogMsg(klogWarn, "Requesting a guard size $(sz) failed, retrying with default size...", "sz=%zu", GUARD_SIZE);
                            continue; /* try again with default guard page size */
#endif
                        }
#if GUARD_SIZE
                        else {
                            LogMsg(klogErr, "Requesting the default guard size failed, thread creation failed.");
                        }
#endif
                        /* fallthrough */
#endif /* GUARD_SIZE_RETRY_DEFAULT */
                    default:
                        rc = RC ( rcPS, rcThread, rcCreating, rcNoObj, rcUnknown );
                    }
                    break;
                }
                free ( t );
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
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcThread, rcSignaling, rcSelf, rcNull );

    status = pthread_cancel ( self -> thread );
    switch ( status )
    {
    case 0:
        break;
    case ESRCH:
        self -> join = false;
        return RC ( rcPS, rcThread, rcSignaling, rcThread, rcDestroyed );
    default:
        return RC ( rcPS, rcThread, rcSignaling, rcNoObj, rcUnknown );
    }

    return 0;
}


/* Wait
 *  waits for a thread to exit
 *
 *  "status" [ OUT ] - return parameter for thread's exit code
 */
LIB_EXPORT rc_t CC KThreadWait ( KThread *self, rc_t *out )
{
    void *td;
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcThread, rcWaiting, rcSelf, rcNull );

    /* prevent multiple waiters */
    if ( atomic32_test_and_set ( & self -> waiting, 0, 1 ) != 0 )
        return RC ( rcPS, rcThread, rcWaiting, rcThread, rcBusy );

    status = pthread_join ( self -> thread, & td );

    /* release waiter lock */                                                                                                                                                                                   
    atomic32_set ( & self -> waiting, 0 );

    switch ( status )
    {
    case 0:
        break;
    case ESRCH:
        return RC ( rcPS, rcThread, rcWaiting, rcThread, rcDestroyed );
    case EINVAL:
        return RC ( rcPS, rcThread, rcWaiting, rcThread, rcDetached );
    case EDEADLK:
        return RC ( rcPS, rcThread, rcWaiting, rcThread, rcDeadlock );
    default:
        return RC ( rcPS, rcThread, rcWaiting, rcNoObj, rcUnknown );
    }

    self -> join = false;

    if ( td == PTHREAD_CANCELED )
        self -> rc = RC ( rcPS, rcThread, rcWaiting, rcThread, rcCanceled );

    if ( out != NULL )
        * out = self -> rc;

    return 0;
}


/* Detach
 *  allow thread to run independently of group
 */
LIB_EXPORT rc_t CC KThreadDetach ( KThread *self )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcThread, rcDetaching, rcSelf, rcNull );

    status = pthread_detach ( self -> thread );
    switch ( status )
    {
    case 0:
    case EINVAL:
        break;
    case ESRCH:
        return RC ( rcPS, rcThread, rcDetaching, rcThread, rcDestroyed );
    default:
        return RC ( rcPS, rcThread, rcDetaching, rcNoObj, rcUnknown );
    }

    self -> join = false;
    return 0;
}
