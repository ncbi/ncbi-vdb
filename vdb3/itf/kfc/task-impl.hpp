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

#ifndef _hpp_vdb3_kfc_task_impl_
#define _hpp_vdb3_kfc_task_impl_

#ifndef _hpp_vdb3_kfc_except_
#include <kfc/except.hpp>
#endif

#ifndef _hpp_vdb3_kfc_refcount_
#include <kfc/refcount.hpp>
#endif

#ifndef _hpp_vdb3_kfc_task_
#include <kfc/task.hpp>
#endif

#ifndef _hpp_vdb3_kfc_rsrc_
#include <kfc/rsrc.hpp>
#endif

#ifndef _hpp_vdb3_kfc_atomic_
#include <kfc/atomic.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards and externs
     */


    /*------------------------------------------------------------------
     * exceptions
     */


    /*------------------------------------------------------------------
     * TaskImpl
     */
    class TaskImpl : public Refcount
        , implements TaskItf
    {
    public:

        // gives task time to run on a thread
        // returns true if task is or has completed
        virtual bool run ();

        // ask task to cooperatively suspend its execution
        virtual void suspend ();

        // checkpointing and restoration
        // TBD - these have to be given an object for saving state
        virtual void checkpoint ();
        virtual void restart ();

        ~ TaskImpl ();

    protected:

        // called to perform any initialization
        // in preparation for first run
        // returns true if successful
        virtual bool prepare ();

        // run the task until suspended or complete
        // return true upon completion
        virtual bool execute () = 0;

        // resume the task until suspended again or complete
        // return true upon completion
        virtual bool resume ();

        // checkpointing operations
        virtual void save ();
        virtual void restore ();


        // construction
        TaskImpl ();
        TaskImpl ( rcaps_t caps );

        // resetting resource managers
        void set_rsrc ( const Rsrc & rsrc );

        // allow task to set state to complete
        void set_complete ();

    private:

        enum task_state_t
        {
            ts_unprepared,
            ts_updating,
            ts_preparing,
            ts_presuspend,
            ts_initial,
            ts_ready,
            ts_suspended,
            ts_running,
            ts_suspending,
            ts_complete,

            // force enum to be 32 bits
            ts_invalid = 0xFFFFFFFF
        };

        Rsrc task_rsrc;
        atomic_t < task_state_t > state;
    };
}

#endif // _hpp_vdb3_kfc_task_impl_
