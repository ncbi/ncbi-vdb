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

#ifndef _hpp_vdb3_kfc_memmgr_
#define _hpp_vdb3_kfc_memmgr_

#ifndef _hpp_vdb3_kfc_except_
#include <kfc/except.hpp>
#endif

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
    class Mem;
    class MemMgr;
    class Memory;
    class Refcount;
    interface MemoryItf;


    /*------------------------------------------------------------------
     * exceptions
     */


    /*------------------------------------------------------------------
     * MemMgrItf
     *  memory manager interface
     */
    interface MemMgrItf : public Refcount
    {
        // allocate memory
        virtual Mem alloc ( const bytes_t & size, bool clear ) = 0;

        // make a block of constant memory
        // TODO: Does this mark a previously allocated block of memory constant
        // or create a write-never block?
        virtual Mem make_const ( const void * ptr, const bytes_t & size ) = 0;

        // Allocate an aligned block of memory, alignment must be a power of
        // 2 and size must be an integral multiple of alignment.
        // TODO: Support clear, optional fill bytes (or -1 for don't memset)?
        virtual Mem aligned_alloc ( const size_t alignment, const bytes_t & size ) = 0;

        // Reallocate a Mem object to size, will shallow copy contents.
        // TODO: How to return failure? Exceptions allowed?
        // TODO: Fill unused bytes?
        virtual void realloc ( Mem & mem, const size_t size) = 0;

        // Return the rounded up allocation that would occur if size requested
        virtual size_t nallocx ( const size_t size) = 0;

        // Pretty print statistics, probably as newline separated Tag=Values.
        // If any pair will exceed accumulated buf_size, stop at previous pair.
        virtual void allocstats ( char * buf, const size_t buf_size) = 0;

        // TODO: Allocator option setter/getter.
        // TODO: Can also set via TBD $ENVVAR (NCBI_MEMMGR?)
        // possible options:
        //   alloc_uninit_bytes   Fill allocated space with
        //   alloc_free_bytes     Fill free space with

        // support for C++ new and delete
        void * _new ( size_t bytes );
        static void _delete ( void * ptr );

    protected:

        // allocate raw memory
        virtual void * _alloc ( const bytes_t & size, bool clear ) = 0;

        // resize or reallocate raw memory
        virtual void * _resize ( void * ptr, const bytes_t & old_size,
            const bytes_t & new_size, bool clear ) = 0;

        // free raw memory
        virtual void _free ( void * ptr, const bytes_t & size ) = 0;

        // create MemMgr reference using friend status
        MemMgr make_mmgr_ref ( Refcount * obj, caps_t caps );

        // create Mem reference using friend status
        static Mem make_mem_ref ( Refcount * obj, MemoryItf * itf, caps_t caps );

        friend class Memory;
    };


    /*------------------------------------------------------------------
     * MemMgr
     *  memory manager reference
     */
    class MemMgr : public Ref < MemMgrItf >
    {
    public:

        // allocate memory
        Mem alloc ( const bytes_t & size, bool clear ) const;

        // make a block of constant memory
        Mem make_const ( const void * ptr, const bytes_t & size ) const;

        // C++
        MemMgr ();
        // TODO: pool support, here or in interface?
        //MemMgr (int pool=0);
        //TODO: ~MemMgr() destroys pool or add .release()?

        // TODO: Create a new pool if copying/assigning from existing MemMgr?
        MemMgr ( const MemMgr & r );
        void operator = ( const MemMgr & r );
        MemMgr ( const MemMgr & r, caps_t reduce );

    private:

        // support for "new" and "delete" operators
        void * _new ( size_t bytes ) const;
        void _delete ( void * ptr ) const;

        // initialization by memmgr_t
        MemMgr ( Refcount * obj, MemMgrItf * itf, caps_t caps );

        friend interface MemMgrItf;
        friend class Refcount;
    };

}

#endif // _hpp_vdb3_kfc_memmgr_
