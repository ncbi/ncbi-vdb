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

#ifndef _hpp_vdb3_kfc_memory_
#define _hpp_vdb3_kfc_memory_

#ifndef _hpp_vdb3_kfc_refcount_
#include <kfc/refcount.hpp>
#endif

#ifndef _hpp_vdb3_kfc_ref_
#include <kfc/ref.hpp>
#endif

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    interface MemMgrItf;
    struct CopyUntilRslt;


    /*------------------------------------------------------------------
     * MemoryItf
     *  an interface representing a block of storage
     */
    interface MemoryItf : public Refcount
    {

        // change the size of the memory allocation
        virtual void resize ( const bytes_t & new_size, bool clear ) = 0;

    protected:

        // TEMPORARY - this allows us to access mapped memory
        // it will not necessarily work for all memory types,
        // and specifically may require having Mem work with
        // offsets and sizes rather than pointers.
        virtual void * get_mapped_memory ( bytes_t * size ) const = 0;

        MemMgrItf * get_mmgr () const;

        static void * cast ( Refcount * obj );

        friend class Mem;
    };


    /*------------------------------------------------------------------
     * Mem
     *  an object representing a range of address space
     */
    class Mem : public Ref < MemoryItf >
    {
    public:

        // size of allocation
        bytes_t size () const;

        // search for a byte
        index_t find_first ( U8 byte ) const;
        index_t find_first ( U8 byte, index_t offset ) const;
        index_t find_first ( U8 byte, index_t offset, const bytes_t & size ) const;

        // return a subrange of memory
        Mem subrange ( index_t offset ) const;
        Mem subrange ( index_t offset, const bytes_t & size ) const;

        // alter memory size
        void resize ( const bytes_t & size, bool clear );

        // transfer data between memory ranges
        bytes_t copy ( const bytes_t & amount, index_t dst_offset,
            const Mem & src, index_t src_offset );
        CopyUntilRslt copy_until ( const bytes_t & amount, U8 stop_byte,
            index_t dst_offset, const Mem & src, index_t src_offset );

        // fill memory with repeats of single byte
        bytes_t fill ( count_t repeat, index_t offset, U8 byte );

        // comparison
        bool operator == ( const Mem & m ) const;
        bool operator != ( const Mem & m ) const;

        // C++
        Mem ();
        ~ Mem ();
        Mem ( const Mem & r );
        void operator = ( const Mem & r );
        Mem ( const Mem & r, caps_t reduce );

        // dynamic
        Mem ( const OpaqueRef & r );

    private:

        // factory
        Mem ( Refcount * obj, MemoryItf * itf, caps_t caps );
#if 0
        Mem ( const void * ptr, const bytes_t & size );
#endif
        void * ptr;
        bytes_t bytes;

        friend interface MemMgrItf;

        friend class ConstString;
        friend class OpaquePtr;
        friend class OpaqueArray;
    };

    struct CopyUntilRslt
    {
        bytes_t num_copied;
        bool stop_byte_found;

        CopyUntilRslt () {}
        CopyUntilRslt ( const bytes_t & b, bool f )
            : num_copied ( b ), stop_byte_found ( f ) {}
    };

}

#endif // _hpp_vdb3_kfc_memory_
