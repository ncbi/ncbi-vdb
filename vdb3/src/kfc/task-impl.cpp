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

#include <vdb3/task-impl.hpp>
#include <vdb3/callstk.hpp>
#include <vdb3/except.hpp>
#include <vdb3/caps.hpp>

#include <string.h>

namespace vdb3
{

    /*------------------------------------------------------------------
     * rsrc_stk_t
     *  a class of general utility, but for now private to task
     */
    class rsrc_stk_t
    {
    public:

        rsrc_stk_t ( const Rsrc * new_rsrc )
            : old_rsrc ( rsrc )
        { rsrc = new_rsrc; }

        ~ rsrc_stk_t ()
        { rsrc = old_rsrc; }

    private:

        const Rsrc * volatile old_rsrc;
    };

    /*------------------------------------------------------------------
     * TaskImpl
     */

    bool TaskImpl :: run ()
    {
        FUNC_ENTRY ();

        for ( task_state_t cur = state . read (); cur != ts_complete; )
        {
            // hold this value for comparison of our assumption
            task_state_t cmp = cur;
            task_state_t trans = ts_invalid;

            switch ( cur )
            {
            case ts_unprepared:

                // going to enter preparation state
                trans = ts_preparing;
                cur = state . test_and_set ( cmp, trans );
                if ( cur == cmp )
                {
                    // this thread now has exclusive ability to prepare task
                    cur = cmp = trans;
                    {
                        rsrc_stk_t stk ( & task_rsrc );
                        prepare ();
                    }

                    // preparation completed
                    // transition to initial run state
                    trans = ts_initial;

                    cur = state . test_and_set ( cmp, trans );
                    if ( cur == ts_presuspend )
                        cur = state . test_and_set ( cmp = cur, trans );

                }
                break;

            case ts_updating:
                CONST_THROW ( xc_task_busy_err, "task is currently being updated" );

            case ts_preparing:
            case ts_presuspend:
                CONST_THROW ( xc_task_busy_err, "task is currently preparing" );

            case ts_initial:
            case ts_ready:
            case ts_suspended:

                // going to enter running state
                trans = ts_running;
                cur = state . test_and_set ( cmp, trans );
                if ( cur == cmp )
                {
                    cur = trans;

                    // this thread now has exclusive ability to run task
                    bool done = false;
                    {
                        // push task resources onto stack
                        rsrc_stk_t stk ( & task_rsrc );

                        // run/resume the task
                        done = ( cmp == ts_suspended ) ? resume () : execute ();
                    }

                    // the run completed
                    // transition based upon indication of done
                    trans = done ? ts_complete : ts_ready;

                    // clean up state
                    cur = state . test_and_set ( cmp = cur, trans );
                    if ( cur == ts_suspending )
                        cur = state . test_and_set ( cmp = cur, trans );

                    // execution was successful, regardless of final state
                    return done;
                }
                break;

            case ts_running:
            case ts_suspending:
                CONST_THROW ( xc_task_busy_err, "task is currently executing" );

            default:
                assert ( cur != ts_complete );
                THROW ( xc_task_invalid_err, "task is in an invalid state: ( 0x%X )", state . read () );
            }

            if ( cur == cmp )
                cur = trans;
        }

        return true;
    }

    void TaskImpl :: suspend ()
    {
        FUNC_ENTRY ();

        for ( task_state_t cur = state . read (); cur != ts_complete; )
        {
            task_state_t cmp = cur;
            task_state_t trans = ts_invalid;

            switch ( cur )
            {
            case ts_unprepared:
                return;

            case ts_updating:
                CONST_THROW ( xc_task_busy_err, "task is currently being updated" );

            case ts_preparing:
                trans = ts_presuspend;
                cur = state . test_and_set ( cmp, trans );
                break;

            case ts_presuspend:
            case ts_initial:
                return;

            case ts_ready:
                trans = ts_suspended;
                cur = state . test_and_set ( cmp, trans );
                break;

            case ts_suspended:
                return;

            case ts_running:
                trans = ts_suspending;
                cur = state . test_and_set ( cmp, trans );
                break;

            case ts_suspending:
                return;

            default:
                assert ( cur != ts_complete );
                THROW ( xc_task_invalid_err, "task is in an invalid state: ( 0x%X )", cur );
            }

            if ( cur == cmp )
                cur = trans;
        }
    }

    void TaskImpl :: checkpoint ()
    {
        FUNC_ENTRY ();

        // the task must be in initial or suspended state

        CONST_THROW ( xc_unimplemented_err, "unimplemented" );
    }

    void TaskImpl :: restart ()
    {
        FUNC_ENTRY ();

        // the task must be in initial or suspended state

        CONST_THROW ( xc_unimplemented_err, "unimplemented" );
    }

    TaskImpl :: ~ TaskImpl ()
    {
    }

    bool TaskImpl :: prepare ()
    {
        // nothing to do
        return true;
    }

    bool TaskImpl :: resume ()
    {
        FUNC_ENTRY ();

        // nothing special about this resume
        // just run the task
        return execute ();
    }

    void TaskImpl :: save ()
    {
        // no state to save
    }

    void TaskImpl :: restore ()
    {
        // no state to be restored
    }

    TaskImpl :: TaskImpl ()
        : task_rsrc ( RCAP_ALL )
        , state ( ts_unprepared )
    {
    }

    TaskImpl :: TaskImpl ( rcaps_t caps )
        : task_rsrc ( caps )
        , state ( ts_unprepared )
    {
    }

    void TaskImpl :: set_rsrc ( const Rsrc & rsrc )
    {
        FUNC_ENTRY ();

        for ( task_state_t cur = state . read (); cur != ts_complete; )
        {
            task_state_t cmp = cur;
            task_state_t trans = ts_invalid;

            switch ( cur )
            {
            case ts_preparing:
            case ts_presuspend:
            case ts_running:
            case ts_suspending:

                trans = ts_updating;
                cur = state . test_and_set ( cmp, trans );
                if ( cur == cmp )
                {
                    cur = trans;
                    try
                    {
                        task_rsrc = rsrc;
                    }
                    catch ( ... )
                    {
                        state = cmp;
                        throw;
                    }

                    state = cmp;
                    return;
                }
                break;

            case ts_unprepared:
            case ts_updating:
            case ts_initial:
            case ts_ready:
            case ts_suspended:
                CONST_THROW ( xc_program_state_violation, "cannot set task resources in current state" );

            default:
                THROW ( xc_task_invalid_err, "task is in an invalid state: ( 0x%X )", cur );
            }

            if ( cur == cmp )
                cur = trans;
        }
    }

    void TaskImpl :: set_complete ()
    {
        FUNC_ENTRY ();

        for ( task_state_t cur = state . read (); cur != ts_complete; )
        {
            task_state_t cmp = cur;
            task_state_t trans = ts_invalid;

            switch ( cur )
            {
            case ts_preparing:
            case ts_presuspend:
            case ts_running:
            case ts_suspending:
                trans = ts_complete;
                cur = state . test_and_set ( cur, trans );
                break;

            case ts_unprepared:
            case ts_initial:
            case ts_ready:
            case ts_suspended:
                CONST_THROW ( xc_program_state_violation, "cannot set task resources in current state" );

            default:
                THROW ( xc_task_invalid_err, "task is in an invalid state: ( 0x%X )", cur );
            }

            if ( cur == cmp )
                cur = trans;
        }
    }
}
