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

#include <vdb3/task.hpp>
#include <vdb3/caps.hpp>
#include <vdb3/callstk.hpp>

namespace vdb3
{

    /*------------------------------------------------------------------
     * TaskItf
     */

    Task TaskItf :: make_ref ( Refcount * obj, caps_t caps )
    {
        return Task ( obj, this, caps );
    }

    void * TaskItf :: cast ( Refcount * obj )
    {
        return ( void * ) dynamic_cast < TaskItf * > ( obj );
    }

    /*------------------------------------------------------------------
     * Task
     */

    bool Task :: run ()
    {
        FUNC_ENTRY ();
        TaskItf * itf = get_itf ( CAP_EXECUTE );
        return itf -> run ();
    }

    void Task :: suspend ()
    {
        FUNC_ENTRY ();
        TaskItf * itf = get_itf ( CAP_SUSPEND );
        itf -> suspend ();
    }

    void Task :: checkpoint ()
    {
        FUNC_ENTRY ();
        TaskItf * itf = get_itf ( CAP_READ );
        itf -> checkpoint ();
    }

    void Task :: restart ()
    {
        FUNC_ENTRY ();
        TaskItf * itf = get_itf ( CAP_WRITE );
        itf -> restart ();
    }

    Task :: Task ()
    {
    }

    Task :: Task ( const Task & r )
        : Ref < TaskItf > ( r )
    {
    }

    void Task :: operator = ( const Task & r )
    {
        Ref < TaskItf > :: operator = ( r );
    }

    Task :: Task ( const Task & r, caps_t reduce )
        : Ref < TaskItf > ( r, reduce )
    {
    }

    Task :: Task ( const OpaqueRef & r )
        : Ref < TaskItf > ( r, TaskItf :: cast )
    {
    }

    Task :: Task ( Refcount * obj, TaskItf * itf, caps_t caps )
        : Ref < TaskItf > ( obj, itf, caps )
    {
    }

}
