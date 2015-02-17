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

#include <kfc/timemgr.hpp>
#include <kfc/time.hpp>
#include <kfc/callstk.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/caps.hpp>


namespace vdb3
{


    /*------------------------------------------------------------------
     * TimeMgrItf
     *  time manager
     */

    void TimeMgrItf :: prepare_timeout ( const timeout_t & tm ) const
    {
#if UNIX
        FUNC_ENTRY ();

        // POSIX needs to add the delay to current time
        // to produce an absolute timeout
        timestamp_t cur = cur_time ();

        // as funny as this looks, we have mutable members
        tm . to = tm . delay + cur;
#endif
    }

    timestamp_t TimeMgrItf :: make_timestamp ( const nS_t & nS )
    {
        return timestamp_t ( nS );
    }

    TimeMgr TimeMgrItf :: make_tmmgr_ref ( Refcount * obj, caps_t caps )
    {
        return TimeMgr ( obj, this, caps );
    }


    /*------------------------------------------------------------------
     * TimeMgr
     *  time manager reference
     */

    timestamp_t TimeMgr :: cur_time () const
    {
        FUNC_ENTRY ();
        TimeMgrItf * itf = get_itf ( CAP_READ );
        return itf -> cur_time ();
    }

    void TimeMgr :: prepare_timeout ( const timeout_t & tm ) const
    {
        FUNC_ENTRY ();
        TimeMgrItf * itf = get_itf ( CAP_READ );
        itf -> prepare_timeout ( tm );
    }

    TimeMgr :: TimeMgr ()
    {
    }

    TimeMgr :: TimeMgr ( const TimeMgr & r )
        : Ref < TimeMgrItf > ( r )
    {
    }

    void TimeMgr :: operator = ( const TimeMgr & r )
    {
        Ref < TimeMgrItf > :: operator = ( r );
    }

    TimeMgr :: TimeMgr ( Refcount * obj, TimeMgrItf * itf, caps_t caps )
        : Ref < TimeMgrItf > ( obj, itf, caps )
    {
    }
}
