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

#ifndef _hpp_vdb3_kfc_stream_
#define _hpp_vdb3_kfc_stream_

#ifndef _hpp_vdb3_kfc_ref_
#include <kfc/ref.hpp>
#endif

#ifndef _hpp_vdb3_kfc_except_
#include <kfc/except.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    class Mem;
    class Stream;
    class Refcount;


    /*------------------------------------------------------------------
     * exceptions
     */
    XC_DECLARE ( xc_transfer_incomplete_err, runtime_err );


    /*------------------------------------------------------------------
     * StreamItf
     *  an interface representing a stream of bytes ( bits? )
     */
    interface StreamItf
    {

        // traditional writing operation
        virtual bytes_t read ( const bytes_t & amount,
            Mem & dst, const bytes_t & dst_offset );
        virtual bytes_t write ( const bytes_t & amount,
            const Mem & src, const bytes_t & src_offset );

        // indicate the preferred chunk size
        virtual bytes_t get_mtu () const = 0;

    protected:

        Stream make_ref ( Refcount * obj, caps_t caps );

    private:

        static void * cast ( Refcount * obj );

        friend class Stream;
    };


    /*------------------------------------------------------------------
     * Stream
     *  an object representing a stream of bytes ( bits? )
     */
    class Stream : public Ref < StreamItf >
    {
    public:

        // copy from source
        // return the number of bytes actually copied
        bytes_t copy ( const Stream & src ) const;
        bytes_t copy ( const bytes_t & amount,
            const Stream & src ) const;
        bytes_t copy_all ( const Stream & src ) const;
        bytes_t copy_all ( const bytes_t & amount,
            const Stream & src ) const;

        // read data into a memory buffer
        bytes_t read ( Mem & dst, index_t dst_offset = 0 ) const;
        bytes_t read ( const bytes_t & amount,
            Mem & dst, index_t dst_offset ) const;
        bytes_t read_all ( Mem & dst, index_t dst_offset = 0 ) const;
        bytes_t read_all ( const bytes_t & amount,
            Mem & dst, index_t dst_offset ) const;

        // write data from memory buffer
        bytes_t write ( const Mem & src, index_t src_offset = 0 ) const;
        bytes_t write ( const bytes_t & amount,
            const Mem & src, index_t src_offset ) const;
        bytes_t write_all ( const Mem & src, index_t src_offset = 0 ) const;
        bytes_t write_all ( const bytes_t & amount,
            const Mem & src, index_t src_offset ) const;

        // C++
        Stream ();
        Stream ( const Stream & r );
        void operator = ( const Stream & r );
        Stream ( const Stream & r, caps_t reduce );

        // dynamic
        Stream ( const OpaqueRef & r );

    private:

        // factory
        Stream ( Refcount * obj, StreamItf * itf, caps_t caps );

        friend interface StreamItf;
    };

}

#endif // _hpp_vdb3_kfc_stream_
