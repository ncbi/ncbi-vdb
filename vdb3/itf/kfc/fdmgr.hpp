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

#ifndef _hpp_vdb3_kfc_fdmgr_
#define _hpp_vdb3_kfc_fdmgr_

#ifndef _hpp_vdb3_kfc_ref_
#include <kfc/ref.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    class FileDesc;
    class FDMgr;
    class FileDescImpl;


    /*------------------------------------------------------------------
     * exceptions
     */


    /*------------------------------------------------------------------
     * FDMgrItf
     *  file descriptor manager
     *  an inverted bit of logic that tries to control the number
     *  of Unix file descriptors in use
     */
    interface FDMgrItf
    {

        // make from an open file descriptor
        virtual FileDesc make ( int fd, caps_t caps, bool owned ) = 0;

    protected:

        static FileDesc make_fd_ref ( FileDescImpl * obj, caps_t caps );
        FDMgr make_fdmgr_ref ( Refcount * obj, caps_t caps );
    };


    /*------------------------------------------------------------------
     * FDMgr
     *  file descriptor manager reference
     */
    class FDMgr : public Ref < FDMgrItf >
    {
    public:

        // make from an open file descriptor
        FileDesc make ( int fd, caps_t caps, bool owned = false ) const;

        // C++
        FDMgr ();
        FDMgr ( const FDMgr & r );
        void operator = ( const FDMgr & r );
        FDMgr ( const FDMgr & r, caps_t reduce );

    private:

        // factory
        FDMgr ( Refcount * obj, FDMgrItf * itf, caps_t caps );

        friend interface FDMgrItf;
    };
}

#endif // _hpp_vdb3_kfc_fdmgr_
