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

#include <kfc/stream.hpp>
#include <kfc/callstk.hpp>
#include <kfc/except.hpp>
#include <kfc/caps.hpp>
#include <kfc/rsrc.hpp>

namespace vdb3
{

    /*------------------------------------------------------------------
     * StreamItf
     *  an interface representing a stream of bytes ( bits? )
     */

    bytes_t StreamItf :: read ( const bytes_t & num_bytes,
        Mem & dst, const bytes_t & start )
    {
        // should never have had the capabilities to issue message
        // so this is an unsupported capability
        // that should not have been granted
        CONST_THROW ( xc_caps_over_extended_err, "unsupported read message" );
    }

    bytes_t StreamItf :: write ( const bytes_t & num_bytes,
        const Mem & src, const bytes_t & start )
    {
        // should never have had the capabilities to issue message
        // so this is an unsupported capability
        // that should not have been granted
        CONST_THROW ( xc_caps_over_extended_err, "unsupported write message" );
    }

    Stream StreamItf :: make_ref ( Refcount * obj, caps_t caps )
    {
        return Stream ( obj, this, caps );
    }

    void * StreamItf :: cast ( Refcount * obj )
    {
        return ( void * ) dynamic_cast < StreamItf * > ( obj );
    }


    /*------------------------------------------------------------------
     * Stream
     *  an object representing a stream of bytes ( bits? )
     */

    const U64 dflt_mtu = 128 * 1024;

    // copy from source
    // return the number of bytes actually copied
    bytes_t Stream :: copy ( const Stream & src ) const
    {
        FUNC_ENTRY ();

        // a null ref should act like nothing was there
        // the behavior of /dev/null is a different concept
        if ( null_ref () )
            return bytes_t ( 0 );

        // access stream
        StreamItf * itf = get_itf ( CAP_PROP_READ | CAP_WRITE );

        // allocate buffer
        bytes_t mtu = itf -> get_mtu ();
        Mem buffer = rsrc -> mmgr . alloc ( mtu, false );

        // read from source
        bytes_t num_read = src . read ( mtu, buffer, 0 );
        if ( num_read == ( U64 ) 0 )
            return num_read;

        // write everything read
        return write_all ( num_read, buffer, 0 );

    }

    bytes_t Stream :: copy ( const bytes_t & num_bytes, const Stream & src ) const
    {
        FUNC_ENTRY ();

        // a null ref should act like nothing was there
        // the behavior of /dev/null is a different concept
        if ( null_ref () )
            return bytes_t ( 0 );

        // access stream
        StreamItf * itf = get_itf ( CAP_PROP_READ | CAP_WRITE );

        // allocate buffer
        bytes_t mtu = itf -> get_mtu ();
        if ( num_bytes < mtu )
            mtu = num_bytes;
        Mem buffer = rsrc -> mmgr . alloc ( mtu, false );

        // read from source
        bytes_t num_read = src . read ( mtu, buffer, 0 );
        if ( num_read == ( U64 ) 0 )
            return num_read;

        // write everything read
        return write_all ( num_read, buffer, 0 );
    }

    bytes_t Stream :: copy_all ( const Stream & src ) const
    {
        FUNC_ENTRY ();

        // a null ref should act like nothing was there
        // the behavior of /dev/null is a different concept
        if ( null_ref () )
            return bytes_t ( 0 );

        // access stream
        StreamItf * itf = get_itf ( CAP_PROP_READ | CAP_WRITE );

        // allocate buffer
        bytes_t mtu = itf -> get_mtu ();
        Mem buffer = rsrc -> mmgr . alloc ( mtu, false );

        // read from source
        bytes_t num_read = src . read ( mtu, buffer, 0 );
        if ( num_read == ( U64 ) 0 )
            return num_read;

        // write everything read
        bytes_t total = write_all ( num_read, buffer, 0 );
        assert ( total == num_read );

        // enter loop to copy until end of stream
        while ( 1 )
        {
            num_read = src . read ( mtu, buffer, 0 );
            if ( num_read == ( U64 ) 0 )
                break;

            bytes_t num_writ = write_all ( num_read, buffer, 0 );
            assert ( num_writ == num_read );
            total += num_writ;
        }

        return total;
    }

    bytes_t Stream :: copy_all ( const bytes_t & num_bytes, const Stream & src ) const
    {
        FUNC_ENTRY ();

        // a null ref should act like nothing was there
        // the behavior of /dev/null is a different concept
        if ( null_ref () )
            return bytes_t ( 0 );

        // access stream
        StreamItf * itf = get_itf ( CAP_PROP_READ | CAP_WRITE );

        // allocate buffer
        bytes_t mtu = itf -> get_mtu ();
        if ( num_bytes < mtu )
            mtu = num_bytes;
        Mem buffer = rsrc -> mmgr . alloc ( mtu, false );

        // read from source
        bytes_t num_read = src . read ( mtu, buffer, 0 );
        if ( num_read == ( U64 ) 0 )
            return num_read;

        // write everything read
        bytes_t total = write_all ( num_read, buffer, 0 );
        assert ( total == num_read );

        // enter loop to copy until end of stream
        while ( 1 )
        {
            // TBD - must read with non-blocking timeout
            num_read = src . read ( mtu, buffer, 0 );
            if ( num_read == ( U64 ) 0 )
                break;

            bytes_t num_writ = write_all ( num_read, buffer, 0 );
            assert ( num_writ == num_read );
            total += num_writ;
        }

        return total;
    }

    bytes_t Stream :: read ( Mem & dst, index_t start ) const
    {
        FUNC_ENTRY ();

        if ( start < 0 )
            THROW ( xc_param_err, "bad start index: %ld", start );

        bytes_t size = dst . size ();
        if ( ( U64 ) start >= size )
            return bytes_t ( 0 );

        bytes_t to_read = size - ( U64 ) start;

        if ( null_ref () )
            return bytes_t ( 0 );

        StreamItf * itf = get_itf ( CAP_PROP_READ | CAP_READ );

        bytes_t mtu = itf -> get_mtu ();
        if ( mtu < to_read )
            to_read = mtu;

        return itf -> read ( to_read, dst, start );
    }

    bytes_t Stream :: read ( const bytes_t & num_bytes, Mem & dst, index_t start ) const
    {
        FUNC_ENTRY ();

        if ( start < 0 )
            THROW ( xc_param_err, "bad start index: %ld", start );

        bytes_t size = dst . size ();
        if ( ( U64 ) start >= size )
            return bytes_t ( 0 );

        bytes_t to_read = size - ( U64 ) start;
        if ( to_read > num_bytes )
            to_read = num_bytes;

        if ( null_ref () )
            return bytes_t ( 0 );

        StreamItf * itf = get_itf ( CAP_PROP_READ | CAP_READ );

        bytes_t mtu = itf -> get_mtu ();
        if ( mtu < to_read )
            to_read = mtu;

        return itf -> read ( to_read, dst, start );
    }

    bytes_t Stream :: write ( const Mem & src, index_t start ) const
    {
        FUNC_ENTRY ();

        if ( start < 0 )
            THROW ( xc_param_err, "bad start index: %ld", start );

        bytes_t size = src . size ();
        if ( ( U64 ) start >= size )
            return bytes_t ( 0 );

        bytes_t to_write = size - ( U64 ) start;

        if ( null_ref () )
            return bytes_t ( 0 );

        StreamItf * itf = get_itf ( CAP_WRITE );

        bytes_t mtu = itf -> get_mtu ();
        if ( mtu < to_write )
            to_write = mtu;

        return itf -> write ( to_write, src, start );
    }

    bytes_t Stream :: write_all ( const Mem & src, index_t start ) const
    {
        FUNC_ENTRY ();

        if ( start < 0 )
            THROW ( xc_param_err, "bad start index: %ld", start );

        bytes_t size = src . size ();
        if ( ( U64 ) start >= size )
            return bytes_t ( 0 );

        bytes_t all_bytes = size - ( U64 ) start;

        if ( null_ref () )
            THROW ( xc_null_self_err, "wrote 0 of %lu bytes", ( U64 ) all_bytes );

        StreamItf * itf = get_itf ( CAP_WRITE );

        bytes_t mtu = itf -> get_mtu ();
        bytes_t to_write = ( mtu < all_bytes ) ? mtu : all_bytes;

        bytes_t total = itf -> write ( to_write, src, start );
        while ( total < all_bytes )
        {
            to_write = all_bytes - total;
            if ( mtu < to_write )
                to_write = mtu;
            bytes_t num_writ = itf -> write ( to_write, src, start + ( U64 ) total );
            if ( num_writ == ( U64 ) 0 )
                THROW ( xc_transfer_incomplete_err, "wrote %lu of %lu bytes", ( U64 ) total, ( U64 ) all_bytes );
            total += num_writ;
        }

        return total;
    }

    bytes_t Stream :: write_all ( const bytes_t & num_bytes, const Mem & src, index_t start ) const
    {
        FUNC_ENTRY ();

        if ( start < 0 )
            THROW ( xc_param_err, "bad start index: %ld", start );

        bytes_t size = src . size ();
        if ( ( U64 ) start >= size )
            return bytes_t ( 0 );

        bytes_t all_bytes = size - ( U64 ) start;
        if ( all_bytes > num_bytes )
            all_bytes = num_bytes;

        if ( null_ref () )
            THROW ( xc_null_self_err, "wrote 0 of %lu bytes", ( U64 ) all_bytes );

        StreamItf * itf = get_itf ( CAP_WRITE );

        bytes_t mtu = itf -> get_mtu ();
        bytes_t to_write = ( mtu < all_bytes ) ? mtu : all_bytes;

        bytes_t total = itf -> write ( to_write, src, start );
        while ( total < all_bytes )
        {
            to_write = all_bytes - total;
            if ( mtu < to_write )
                to_write = mtu;
            bytes_t num_writ = itf -> write ( to_write, src, start + ( U64 ) total );
            if ( num_writ == ( U64 ) 0 )
                THROW ( xc_transfer_incomplete_err, "wrote %lu of %lu bytes", ( U64 ) total, ( U64 ) all_bytes );
            total += num_writ;
        }

        return total;
    }

    Stream :: Stream ()
    {
    }

    Stream :: Stream ( const Stream & r )
        : Ref < StreamItf > ( r )
    {
    }

    void Stream :: operator = ( const Stream & r )
    {
        Ref < StreamItf > :: operator = ( r );
    }

    Stream :: Stream ( const Stream & r, caps_t reduce )
        : Ref < StreamItf > ( r, reduce )
    {
    }

    Stream :: Stream ( const OpaqueRef & r )
        : Ref < StreamItf > ( r, StreamItf :: cast )
    {
    }

    Stream :: Stream ( Refcount * obj, StreamItf * itf, caps_t caps )
        : Ref < StreamItf > ( obj, itf, caps )
    {
    }
    
}
