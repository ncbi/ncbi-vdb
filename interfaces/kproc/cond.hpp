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

#ifndef _hpp_kproc_cond_
#define _hpp_kproc_cond_

#ifndef _h_kproc_cond_
#include <kproc/cond.h>
#endif


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
    /* Make
     *  create a condition
     */
    static inline rc_t Make ( KCondition **cond )
        throw ()
    { return KConditionMake ( cond ); }


    /* AddRef
     * Release
     */
    inline rc_t AddRef () const
        throw ()
    { return KConditionAddRef ( this ); }

    inline rc_t Release () const
        throw ()
    { return KConditionRelease ( this ); }


    /* Wait
     *  block on external lock until signaled
     */
    inline rc_t Wait ( struct KLock *lock )
        throw ()
    { return KConditionWait ( this, lock ); }

    inline rc_t Wait ( struct KLock *lock, struct timeout_t *tm )
        throw ()
    { return KConditionTimedWait ( this, lock, tm ); }

    inline rc_t TimedWait ( struct KLock *lock, struct timeout_t *tm )
        throw ()
    { return KConditionTimedWait ( this, lock, tm ); }


    /* Signal
     *  signal waiting threads
     *  awaken at most a single thread
     */
    inline rc_t Signal ()
        throw ()
    { return KConditionSignal ( this ); }


    /* Broadcast
     *  signal waiting threads
     *  awaken all waiting thread
     */
    inline rc_t Broadcast ()
        throw ()
    { return KConditionBroadcast ( this ); }

private:
    KCondition ();
    ~ KCondition ();
    KCondition ( const KCondition& );
    KCondition &operator = ( const KCondition& );
};

#endif // _hpp_kproc_cond_
