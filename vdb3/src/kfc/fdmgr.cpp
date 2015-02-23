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

#include <kfc/fdmgr.hpp>
#include <kfc/fd.hpp>
#include <kfc/callstk.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/except.hpp>
#include <kfc/caps.hpp>

namespace vdb3
{

    /*------------------------------------------------------------------
     * FDMgrItf
     *  file descriptor manager
     *  an inverted bit of logic that tries to control the number
     *  of Unix file descriptors in use
     */

    FileDesc FDMgrItf :: make_fd_ref ( FileDescImpl * obj, caps_t caps )
    {
        return FileDesc ( obj, caps );
    }

    FDMgr FDMgrItf :: make_fdmgr_ref ( Refcount * obj, caps_t caps )
    {
        return FDMgr ( obj, this, caps );
    }


    /*------------------------------------------------------------------
     * FDMgr
     *  file descriptor manager reference
     */


    FileDesc FDMgr :: make ( int fd, caps_t caps, bool owned ) const
    {
        FUNC_ENTRY ();

        // should examine the fd to be open, etc.
        if ( fd < 0 )
            THROW ( xc_param_err, "bad fd: %d", fd );

        // limit caps
        caps &= CAP_PROP_READ | CAP_PROP_WRITE | CAP_RDWR | CAP_RESIZE;

        // ask manager to create one
        FDMgrItf * itf = get_itf ( CAP_ALLOC );
        return itf -> make ( fd, caps, owned );
    }

    FDMgr :: FDMgr ()
    {
    }

    FDMgr :: FDMgr ( const FDMgr & r )
        : Ref < FDMgrItf > ( r )
    {
    }

    void FDMgr :: operator = ( const FDMgr & r )
    {
        Ref < FDMgrItf > :: operator = ( r );
    }

    FDMgr :: FDMgr ( const FDMgr & r, caps_t reduce )
        : Ref < FDMgrItf > ( r, reduce )
    {
    }

    FDMgr :: FDMgr ( Refcount * obj, FDMgrItf * itf, caps_t caps )
        : Ref < FDMgrItf > ( obj, itf, caps )
    {
    }

}
