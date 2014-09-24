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

#ifndef _hpp_kproc_thread_
#define _hpp_kproc_thread_

#ifndef _h_kproc_thread_
#include <kproc/thread.h>
#endif


/*--------------------------------------------------------------------------
 * KThread
 *  a CPU execution thread
 */
struct KThread
{
    /* Make
     *  create and run a thread
     *
     *  "run_thread" [ IN ] - thread entrypoint
     *
     *  "data" [ IN, OPAQUE ] - user-supplied thread data
     */
    static inline rc_t Make ( KThread **t,
             rc_t ( CC * run_thread ) ( const KThread *, void * ), void * data )
        throw ()
    { return KThreadMake ( t, run_thread, data ); }


    /* AddRef
     * Release
     */
    inline rc_t AddRef () const
        throw ()
    { return KThreadAddRef ( this ); }

    inline rc_t Release () const
        throw ()
    { return KThreadRelease ( this ); }


    /* Cancel
     *  signal the thread to finish
     */
    inline rc_t Cancel ()
        throw ()
    { return KThreadCancel ( this ); }


    /* Wait
     *  waits for a thread to exit
     *
     *  "status" [ OUT ] - return parameter for thread's exit code
     */
    inline rc_t Wait ( rc_t *status )
        throw ()
    { return KThreadWait ( this, status ); }


    /* Detach
     *  allow thread to run independently of group
     */
    inline rc_t Detach ()
        throw ()
    { return KThreadDetach ( this ); }

private:
    KThread ();
    ~ KThread ();
    KThread ( const KThread& );
    KThread &operator = ( const KThread& );
};

#endif // _hpp_kproc_thread_
