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

#ifndef _hpp_kproc_sem_
#define _hpp_kproc_sem_

#ifndef _h_kproc_sem_
#include <kproc/sem.h>
#endif


/*--------------------------------------------------------------------------
 * KSemaphore
 *  a metering device
 */
struct KSemaphore
{
    /* Make
     *
     *  "count" [ IN ] - initial count value
     */
    static inline rc_t Make ( KSemaphore **sem, uint64_t count )
        throw ()
    { return KSemaphoreMake ( sem, count ); }


    /* AddRef
     * Release
     */
    inline rc_t AddRef () const
        throw ()
    { return KSemaphoreAddRef ( this ); }

    inline rc_t Release () const
        throw ()
    { return KSemaphoreRelease ( this ); }


    /* Wait
     *  block until a count becomes available
     *
     *  "lock" [ IN ] - externally acquired lock
     */
    inline rc_t Wait ( struct KLock *lock )
        throw ()
    { return KSemaphoreWait ( this, lock ); }

    inline rc_t Wait ( struct KLock *lock, struct timeout_t *tm )
        throw ()
    { return KSemaphoreTimedWait ( this, lock, tm ); }

    inline rc_t TimedWait ( struct KLock *lock, struct timeout_t *tm )
        throw ()
    { return KSemaphoreTimedWait ( this, lock, tm ); }


    /* Signal
     *  signal that a count has become available
     */
    inline rc_t Signal ()
        throw ()
    { return KSemaphoreSignal ( this ); }


    /* Alloc
     *  allocate a count
     *  used for resource metering
     *
     *  "lock" [ IN ] - externally acquired lock
     *
     *  "count" [ IN ] - the resource count
     */
    inline rc_t Alloc ( struct KLock *lock, uint64_t count )
        throw ()
    { return KSemaphoreAlloc ( this, lock, count ); }

    inline rc_t Alloc ( struct KLock *lock, uint64_t count, struct timeout_t *tm )
        throw ()
    { return KSemaphoreTimedAlloc ( this, lock, count, tm ); }

    inline rc_t TimedAlloc ( struct KLock *lock, uint64_t count, struct timeout_t *tm )
        throw ()
    { return KSemaphoreTimedAlloc ( this, lock, count, tm ); }


    /* Free
     *  signal that resources have become available
     */
    inline rc_t Free ( uint64_t count )
        throw ()
    { return KSemaphoreFree ( this, count ); }


    /* Count
     *  request the current resource usage
     *  valid only within lock
     *
     *  "count" [ OUT ] - return parameter for current count
     */
    inline rc_t Count ( uint64_t *count ) const
        throw ()
    { return KSemaphoreCount ( this, count ); }

private:
    KSemaphore ();
    ~ KSemaphore ();
    KSemaphore ( const KSemaphore& );
    KSemaphore &operator = ( const KSemaphore& );
};

#endif // _hpp_kproc_sem_
