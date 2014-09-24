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

#ifndef _h_kproc_lock_
#define _h_kproc_lock_

#ifndef _h_kproc_lock_
#include <kproc/lock.h>
#endif


/*--------------------------------------------------------------------------
 * KLock
 *  a POSIX-style mutual exclusion lock
 */
struct KLock
{
    /* Make
     *  make a simple mutex
     */
    static inline rc_t Make ( KLock **lock )
        throw ()
    { return KLockMake ( lock ); }


    /* AddRef
     * Release
     */
    inline rc_t AddRef () const
        throw ()
    { return KLockAddRef ( this ); }

    inline rc_t Release () const
        throw ()
    { return KLockRelease ( this ); }


    /* Acquire
     *  acquires lock
     */
    inline rc_t Acquire ()
        throw ()
    { return KLockAcquire ( this ); }

    inline rc_t Acquire ( struct timeout_t *tm )
        throw ()
    { return KLockTimedAcquire ( tthis, tm ); }

    inline rc_t TimedAcquire ( struct timeout_t *tm )
        throw ()
    { return KLockTimedAcquire ( tthis, tm ); }

    /* Unlock
     *  releases lock
     */
    inline rc_t Unlock ()
        throw ()
    { return KLockUnlock ( this ): }

private:
    KLock ();
    ~ KLock ();
    KLock ( const KLock& );
    KLock &operator = ( const KLock& );
};


/*--------------------------------------------------------------------------
 * KRWLock
 *  a POSIX-style read/write lock
 */
struct KRWLock
{
    /* Make
     *  make a simple read/write lock
     */
    static inline rc_t Make ( KRWLock **lock )
        throw ()
    { return KRWLockMake ( lock ): }


    /* AddRef
     * Release
     */
    inline rc_t AddRef () const
        throw ()
    { return KRWLockAddRef ( this ); }

    inline rc_t Release () const
        throw ()
    { return KRWLockRelease ( this ); }


    /* AcquireShared
     *  acquires read ( shared ) lock
     */
    inline rc_t AcquireShared ()
        throw ()
    { return KRWLockAcquireShared ( this ); }

    inline rc_t AcquireShared ( struct timeout_t *tm )
        throw ()
    { return KRWLockTimedAcquireShared ( this, tm ); }

    inline rc_t TimedAcquireShared ( struct timeout_t *tm )
        throw ()
    { return KRWLockTimedAcquireShared ( this, tm ); }


    /* AcquireExcl
     *  acquires write ( exclusive ) lock
     */
    inline rc_t AcquireExcl ()
        throw ()
    { return KRWLockAcquireExcl ( this ); }

    inline rc_t AcquireExcl ( struct timeout_t *tm )
        throw ()
    { return KRWLockTimedAcquireExcl ( this, tm ); }

    inline rc_t TimedAcquireExcl ( struct timeout_t *tm )
        throw ()
    { return KRWLockTimedAcquireExcl ( this, tm ); }


    /* Unlock
     *  releases lock
     */
    inline rc_t Unlock ()
        throw ()
    { return KRWLockUnlock ( this ); }

private:
    KRWLock ();
    ~ KRWLock ();
    KRWLock ( const KRWLock& );
    KRWLock &operator = ( const KRWLock& );
};

#endif // _hpp_kproc_lock_
