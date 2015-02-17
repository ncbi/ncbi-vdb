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

#include "ptimemgr.hpp"
#include <kfc/time.hpp>
#include <kfc/callstk.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/caps.hpp>
#include <kfc/syserr.hpp>

#if UNIX
#include <sys/time.h>
#include <errno.h>
#else
#error "unsupported target platform"
#endif

namespace vdb3
{


    /*------------------------------------------------------------------
     * ptimemgr_t
     *  primordial time manager
     */

    TimeMgr PrimordTimeMgr :: make_primordial ()
    {
        FUNC_ENTRY ();
        PrimordTimeMgr * obj = new PrimordTimeMgr;
        return obj -> make_tmmgr_ref ( obj, CAP_READ );
    }

    timestamp_t PrimordTimeMgr :: cur_time () const
    {
        FUNC_ENTRY ();

#if UNIX
        struct timeval tv;
        int status =  gettimeofday ( & tv, 0 );
        if ( status != 0 )
            THROW_OSERR ( gettimeofday, errno );

        seconds_t sec = tv . tv_sec;
        uS_t uS = tv . tv_usec;
        uS += sec;

        return make_timestamp ( uS );
#else
#error no implementation for cur_time on this platform
#endif
    }

    PrimordTimeMgr :: PrimordTimeMgr ()
    {
    }

}
