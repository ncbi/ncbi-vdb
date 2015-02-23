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

#ifndef _hpp_vdb3_kfc_task_
#define _hpp_vdb3_kfc_task_

#ifndef _hpp_vdb3_kfc_exceot_
#include <kfc/except.hpp>
#endif

#ifndef _hpp_vdb3_kfc_ref_
#include <kfc/ref.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards and externs
     */
    class Task;
    class Refcount;


    /*------------------------------------------------------------------
     * exceptions
     */
    XC_DECLARE ( xc_task_busy_err, runtime_err );
    XC_DECLARE ( xc_task_invalid_err, xc_internal_err );


    /*------------------------------------------------------------------
     * TaskItf
     */
    interface TaskItf
    {

        // gives task time to run on a thread
        // returns true if task is or has completed
        virtual bool run () = 0;

        // ask task to cooperatively suspend its execution
        virtual void suspend () = 0;

        // checkpointing and restoration
        // TBD - these have to be given an object for saving state
        virtual void checkpoint () = 0;
        virtual void restart () = 0;

    protected:

        // creating a Task object
        Task make_ref ( Refcount * obj, caps_t caps );

    private:

        static void * cast ( Refcount * obj );

        friend class Task;
    };


    /*------------------------------------------------------------------
     * Task
     */
    class Task : Ref < TaskItf >
    {
    public:

        // gives task time to run on a thread
        // returns true if the task is or has completed
        bool run ();

        // ask task to cooperatively suspend its execution
        void suspend ();

        // checkpointing and restoration
        // TBD - these have to be given an object for saving state
        void checkpoint ();
        void restart ();

        // C++
        Task ();
        Task ( const Task & r );
        void operator = ( const Task & r );
        Task ( const Task & r, caps_t reduce );

        // dynamic
        Task ( const OpaqueRef & r );

    private:

        // construct a task reference
        Task ( Refcount * obj, TaskItf * itf, caps_t caps );

        friend class TaskItf;

    };
}

#endif // _hpp_vdb3_kfc_task_
