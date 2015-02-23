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

#ifndef _hpp_vdb3_kfc_fd_
#define _hpp_vdb3_kfc_fd_

#ifndef _hpp_vdb3_kfc_refcount_
#include <kfc/refcount.hpp>
#endif

#ifndef _hpp_vdb3_kfc_stream_
#include <kfc/stream.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */


    /*------------------------------------------------------------------
     * exceptions
     */


    /*------------------------------------------------------------------
     * FileDescImpl
     *  a Unix file-descriptor
     */
    class FileDescImpl : public Refcount
        , implements StreamItf
    {
    public:

        // StreamItf
        virtual bytes_t read ( const bytes_t & num_bytes,
            Mem & dst, const bytes_t & start );
        virtual bytes_t write ( const bytes_t & num_bytes,
            const Mem & src, const bytes_t & start );
        virtual bytes_t get_mtu () const;

        // C++
        ~ FileDescImpl ();

    private:

        FileDescImpl ( int fd, bool owned );

        int fd;
        bool owned;

        friend class PrimordFDMgr;
    };


    /*------------------------------------------------------------------
     * FileDesc
     *  a reference to a Unix file-descriptor
     */
    class FileDesc : public Ref < FileDescImpl >
    {
    public:

        FileDesc ();
        FileDesc ( const FileDesc & r );
        void operator = ( const FileDesc & r );
        FileDesc ( const FileDesc & r, caps_t reduce );

    private:

        FileDesc ( FileDescImpl * obj, caps_t caps );

        friend interface FDMgrItf;
    };
}

#endif // _hpp_vdb3_kfc_fd_
