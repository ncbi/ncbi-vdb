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

#include <kfc/fd.hpp>
#include <kfc/callstk.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/caps.hpp>
#include <kfc/except.hpp>
#include <kfc/syserr.hpp>
#include <kfc/array.hpp>

#if UNIX
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#else
#error "unsupported target platform"
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * FileDescImpl
     *  a Unix file-descriptor
     */

    // StreamItf
    bytes_t FileDescImpl :: read ( const bytes_t & num_bytes,
        Mem & dst, const bytes_t & start )
    {
        FUNC_ENTRY ();

        // parameters have already been checked
        assert ( start < dst . size () );
        assert ( start + num_bytes <= dst . size () );

        // interfacing with native OS requires exceptional access
        Array < U8 > a = dst;
        void * addr = & a [ start ];

#if UNIX
        ssize_t num_read = 0;
        while ( 1 )
        {
            num_read = :: read ( fd, addr, ( size_t ) ( U64 ) num_bytes );
            if ( num_read >= 0 )
                break;

            int _fd = fd;
            int status = errno;
            switch ( status )
            {
            case EINTR:
                break;
            case EBADF:
                fd = -1;
                owned = false;
                THROW ( xc_param_err, "bad fd: %d", _fd );
            case EFAULT:
                // super-internal error, wow.
                THROW ( xc_internal_err, "bad address from meory: 0x016X", ( U64 ) addr );
            case EAGAIN:
                // this is essentially a timeout error
            case EINVAL:
            case EIO:
            case EISDIR:
            default:
                THROW_OSERR ( read, status );
            }
        }

        return bytes_t ( num_read );
#endif
    }

    bytes_t FileDescImpl :: write ( const bytes_t & num_bytes,
        const Mem & src, const bytes_t & start )
    {
        FUNC_ENTRY ();

        // parameters have already been checked
        assert ( start < src . size () );
        assert ( start + num_bytes <= src . size () );

        // interfacing with native OS requires exceptional access
        const Array < U8 > a = src;
        const void * addr = & a [ start ];

#if UNIX
        ssize_t num_writ = 0;
        while ( 1 )
        {
            num_writ = :: write ( fd, addr, ( size_t ) ( U64 ) num_bytes );
            if ( num_writ >= 0 )
                break;

            int _fd = fd;
            int status = errno;
            switch ( status )
            {
            case EINTR:
                break;
            case ENOSPC:
                THROW ( xc_no_mem, "attempt to write %lu bytes exceeds volume limits", ( U64 ) num_bytes );
            case EFBIG:
                THROW ( xc_bounds_err, "attempt to write %lu bytes exceeds limit", ( U64 ) num_bytes );
            case EBADF:
                fd = -1;
                owned = false;
                THROW ( xc_param_err, "bad fd: %d", _fd );
            case EFAULT:
                // super-internal error, wow.
                THROW ( xc_internal_err, "bad address from meory: 0x016X", ( U64 ) addr );
            case EAGAIN:
                // this is essentially a timeout error
            case EINVAL:
            case EIO:
            case EISDIR:
            default:
                THROW_OSERR ( write, status );
            }
        }

        return bytes_t ( num_writ );
#endif
    }

    bytes_t FileDescImpl :: get_mtu () const
    {
        FUNC_ENTRY ();

        // TBD - determine proper MTU for device
        return bytes_t ( 4096 );
    }

    // C++
    FileDescImpl :: FileDescImpl ( int _fd, bool _owned )
        : fd ( _fd )
        , owned ( _owned )
    {
        assert ( _fd >= 0 );
    }

    FileDescImpl :: ~ FileDescImpl ()
    {
        if ( owned )
            close ( fd );
    }


    /*------------------------------------------------------------------
     * FileDesc
     *  a reference to a Unix file-descriptor
     */

    FileDesc :: FileDesc ()
    {
    }

    FileDesc :: FileDesc ( const FileDesc & r )
        : Ref < FileDescImpl > ( r )
    {
    }

    void FileDesc :: operator = ( const FileDesc & r )
    {
        Ref < FileDescImpl > :: operator = ( r );
    }

    FileDesc :: FileDesc ( const FileDesc & r, caps_t reduce )
        : Ref < FileDescImpl > ( r, reduce )
    {
    }

    FileDesc :: FileDesc ( FileDescImpl * obj, caps_t caps )
        : Ref < FileDescImpl > ( obj, caps )
    {
    }

}
