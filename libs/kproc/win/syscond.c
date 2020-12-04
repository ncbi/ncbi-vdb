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

#include <os-native.h>
#include <kproc/timeout.h>
#include <kproc/cond.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include <time.h> /* struct timespec */

/* #include <stdio.h> */


/*

This Windows condition implementation is taken from 
"Strategies for Implementing POSIX Condition Variables on Win32"
by Douglas C. Schmidt
( http://www.cs.wustl.edu/~schmidt/win32-cv-1.html ) :

3.4. The SignalObjectAndWait Solution
The implementation relies on the Windows NT 4.0 SignalObjectAndWait function,
thus it is not available in Windows CE, Windows '95, or Windows NT 3.51.

The code was taken from the article.

The C++ source code for POSIX condition variable on Win32
described in the article is freely available with the ACE framework
at //www.cs.wustl.edu/~schmidt/ACE.html.
It is possible that the latter is better.

*/


typedef struct
{
  int waiters_count_;
  // Number of waiting threads.

  CRITICAL_SECTION waiters_count_lock_;
  // Serialize access to <waiters_count_>.

  HANDLE sema_;
  // Semaphore used to queue up threads waiting for the condition to
  // become signaled. 

  HANDLE waiters_done_;
  // An auto-reset event used by the broadcast/signal thread to wait
  // for all the waiting thread(s) to wake up and be released from the
  // semaphore. 

  size_t was_broadcast_;
  // Keeps track of whether we were broadcasting or signaling.  This
  // allows us to optimize the code if we're just signaling.
} pthread_cond_t;


/*--------------------------------------------------------------------------
 * KCondition
 *  a POSIX-style condition object
 *  ( requires an external lock object )
 *
 *  usage: the user first acquires an external lock. then, depending upon
 *  the operation, will either test for a condition or establish it, where
 *  the former involves the potential to wait for a signal and the latter
 *  to generate a signal using the external lock for blocking.
 */
struct KCondition
{
    pthread_cond_t cond;

    atomic32_t refcount;
};


typedef HANDLE pthread_mutex_t;
typedef struct pthread_condattr_t { char dummy; } pthread_condattr_t;

int 
pthread_cond_init (pthread_cond_t *cv,
                   const pthread_condattr_t *dummy)
{
  cv->waiters_count_ = 0;
  cv->was_broadcast_ = 0;
  cv->sema_ = CreateSemaphore (NULL,       // no security
                                0,          // initially 0
                                0x7fffffff, // max count
                                NULL);      // unnamed 
  InitializeCriticalSection (&cv->waiters_count_lock_);
  cv->waiters_done_ = CreateEvent (NULL,  // no security
                                   FALSE, // auto-reset
                                   FALSE, // non-signaled initially
                                   NULL); // unnamed
  return 0;
}

int pthread_cond_destroy (pthread_cond_t *cv) {
    CloseHandle(cv->sema_);
    DeleteCriticalSection(&cv->waiters_count_lock_);
    CloseHandle(cv->waiters_done_);
    memset(cv, 0, sizeof *cv);
    return 0;
}

#if _MSC_VER < 1900
#ifndef timespec
struct timespec
  {
    time_t tv_sec;            /* Seconds.  */
    long int tv_nsec;           /* Nanoseconds.  */
  };
#endif
#endif
typedef struct timespec timespec;

int pthread_cond_waitImpl (pthread_cond_t *cv, 
                   pthread_mutex_t *external_mutex,
                   const struct timespec *abstime,
                   bool infinite)
{
    int last_waiter;
    DWORD dwMilliseconds = INFINITE;
    DWORD ret;

    // Avoid race conditions.
    EnterCriticalSection (&cv->waiters_count_lock_);
    cv->waiters_count_++;
    LeaveCriticalSection (&cv->waiters_count_lock_);

    // This call atomically releases the mutex and waits on the
    // semaphore until <pthread_cond_signal> or <pthread_cond_broadcast>
    // are called by another thread.
    if (!infinite && abstime != NULL)
    { dwMilliseconds = ( DWORD ) ( abstime->tv_sec * 1000 + abstime->tv_nsec / 1000000 ) ; }
    ret = SignalObjectAndWait (*external_mutex, cv->sema_, dwMilliseconds, FALSE);

    // Reacquire lock to avoid race conditions.
    EnterCriticalSection (&cv->waiters_count_lock_);

    // We're no longer waiting...
    cv->waiters_count_--;

    // Check to see if we're the last waiter after <pthread_cond_broadcast>.
    last_waiter = cv->was_broadcast_ && cv->waiters_count_ == 0;

    LeaveCriticalSection (&cv->waiters_count_lock_);

    // If we're the last waiter thread during this particular broadcast
    // then let all the other threads proceed.
    if (last_waiter)
        // This call atomically signals the <waiters_done_> event and waits until
        // it can acquire the <external_mutex>.
        // This is required to ensure fairness.
        SignalObjectAndWait (cv->waiters_done_, *external_mutex, INFINITE, FALSE);
    else {
        // Always regain the external mutex since that's the guarantee we
        // give to our callers.
        /*      fprintf(stderr, "%s: WaitForSingleObject...\n", __func__); */
        WaitForSingleObject (*external_mutex, INFINITE);
        /*      fprintf(stderr, "... %s: WaitForSingleObject\n", __func__); */
    }

    switch ( ret )
    {
    case WAIT_OBJECT_0:
        break;
    case WAIT_TIMEOUT:
        return ETIMEDOUT;
    default:
        return EINVAL;
    }

    return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cv,
              pthread_mutex_t *external_mutex,
              const struct timespec *abstime)
{
  return pthread_cond_waitImpl(cv, external_mutex, abstime, false);
}

int
pthread_cond_wait (pthread_cond_t *cv, 
                   pthread_mutex_t *external_mutex)
{
  return pthread_cond_waitImpl(cv, external_mutex, NULL, true);
}

int
pthread_cond_signal (pthread_cond_t *cv)
{
    int have_waiters;

  EnterCriticalSection (&cv->waiters_count_lock_);
  have_waiters = cv->waiters_count_ > 0;
  LeaveCriticalSection (&cv->waiters_count_lock_);

  // If there aren't any waiters, then this is a no-op.  
  if (have_waiters)
    ReleaseSemaphore (cv->sema_, 1, 0);

  return 0;
}

int
pthread_cond_broadcast (pthread_cond_t *cv)
{
  int have_waiters = 0;

  // This is needed to ensure that <waiters_count_> and <was_broadcast_> are
  // consistent relative to each other.
  EnterCriticalSection (&cv->waiters_count_lock_);

  if (cv->waiters_count_ > 0) {
    // We are broadcasting, even if there is just one waiter...
    // Record that we are broadcasting, which helps optimize
    // <pthread_cond_wait> for the non-broadcast case.
    cv->was_broadcast_ = 1;
    have_waiters = 1;
  }

  if (have_waiters) {
    // Wake up all the waiters atomically.
    ReleaseSemaphore (cv->sema_, cv->waiters_count_, 0);

    LeaveCriticalSection (&cv->waiters_count_lock_);

    // Wait for all the awakened threads to acquire the counting
    // semaphore. 
    WaitForSingleObject (cv->waiters_done_, INFINITE);
    // This assignment is okay, even without the <waiters_count_lock_> held 
    // because no other waiter threads can wake up to access it.
    cv->was_broadcast_ = 0;
  }
  else
    LeaveCriticalSection (&cv->waiters_count_lock_);

  return 0;
}

/*--------------------------------------------------------------------------
 * KCondition
 *  a POSIX-style condition object
 *  ( requires an external lock object )
 *
 *  usage: the user first acquires an external lock. then, depending upon
 *  the operation, will either test for a condition or establish it, where
 *  the former involves the potential to wait for a signal and the latter
 *  to generate a signal using the external lock for blocking.
 */

/* Destroy
 */
rc_t KConditionDestroy ( KCondition *self )
{
    int status = pthread_cond_destroy ( & self -> cond );
    switch ( status )
    {
    case 0:
        break;
    case EBUSY:
        return RC ( rcPS, rcCondition, rcDestroying, rcCondition, rcBusy );
    default:
        return RC ( rcPS, rcCondition, rcDestroying, rcNoObj, rcUnknown );
    }

    return 0;
}


/* Whack
 */
static
rc_t KConditionWhack ( KCondition *self )
{
    rc_t rc = KConditionDestroy ( self );
    if ( rc == 0 )
        free ( self );
    return rc;
}

/* Init
 */
rc_t KConditionInit ( KCondition *self )
{
    int status;

    assert ( self != NULL );
    status = pthread_cond_init ( & self -> cond, NULL );
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

    atomic32_set ( & self -> refcount, 1 );
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
            rc = KConditionInit ( cond );
            if ( rc == 0 )
            {
                * condp = cond;
                return 0;
            }
            
            free ( cond );
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

/* DropRef
 *  manipulates reference counter
 *  returns true if last ref
 */
int KConditionDropRef ( const KCondition *cself )
{
    assert ( cself != NULL );
    return atomic32_dec_and_test ( & ( ( KCondition* ) cself ) -> refcount );
}


/* Wait
 *  block on external lock until signaled
 */
LIB_EXPORT rc_t CC KConditionWait ( KCondition *self, struct KLock *lock )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcSelf, rcNull );
    if ( lock == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcLock, rcNull );

    status = pthread_cond_wait ( & self -> cond, & lock -> mutex );
    switch ( status )
    {
    case 0:
        break;
    default:
        return RC ( rcPS, rcCondition, rcWaiting, rcNoObj, rcUnknown );
    }

    return 0;
}

LIB_EXPORT rc_t CC KConditionTimedWait ( KCondition *self, struct KLock *lock, timeout_t *tm )
{
    int status;
    timespec ts;

    if ( tm == NULL )
        return KConditionWait ( self, lock );

    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcSelf, rcNull );
    if ( lock == NULL )
        return RC ( rcPS, rcCondition, rcWaiting, rcLock, rcNull );

    if ( ! tm -> prepared )
        TimeoutPrepare ( tm );

    memset(&ts, 0, sizeof ts);
    ts.tv_sec = tm -> mS / 1000;
    ts.tv_nsec = ( long ) ( (tm -> mS - ts.tv_sec * 1000) * 1000000 );

    status = pthread_cond_timedwait ( & self -> cond, & lock -> mutex, & ts );
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


/* Signal
 *  signal waiting threads
 *  awaken at most a single thread
 */
LIB_EXPORT rc_t CC KConditionSignal ( KCondition *self )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcSignaling, rcSelf, rcNull );

    status = pthread_cond_signal ( & self -> cond );
    switch ( status )
    {
    case 0:
        break;
    default:
        return RC ( rcPS, rcCondition, rcSignaling, rcNoObj, rcUnknown );
    }

    return 0;
}


/* Broadcast
 *  signal waiting threads
 *  awaken all waiting thread
 */
LIB_EXPORT rc_t CC KConditionBroadcast ( KCondition *self )
{
    int status;

    if ( self == NULL )
        return RC ( rcPS, rcCondition, rcSignaling, rcSelf, rcNull );

    status = pthread_cond_broadcast ( & self -> cond );
    switch ( status )
    {
    case 0:
        break;
    default:
        return RC ( rcPS, rcCondition, rcSignaling, rcNoObj, rcUnknown );
    }

    return 0;
}
