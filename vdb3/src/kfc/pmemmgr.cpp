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

#include "pmemmgr.hpp"
#include <kfc/memory.hpp>
#include <kfc/callstk.hpp>
#include <kfc/caps.hpp>
#include <kfc/rsrc.hpp>

#include <stdlib.h>
#include <string.h>

#if UNIX
#include <sys/resource.h>
#endif

namespace vdb3
{

    const caps_t CONST_MEM_CAPS
        = CAP_PROP_READ
        | CAP_READ
        | CAP_SUBRANGE
        | CAP_CAST
        ;

    const caps_t NEW_MEM_CAPS
        = CONST_MEM_CAPS
        | CAP_WRITE
        | CAP_RESIZE
        ;


    /*------------------------------------------------------------------
     * const_memory_t
     *  an object representing a range of address space
     */
    class const_memory_t : implements MemoryItf
    {
    public:

        virtual void resize ( const bytes_t & new_size, bool clear );
        const_memory_t ( const void * ptr, const bytes_t & size );
        ~ const_memory_t ();

    protected:

        virtual void * get_mapped_memory ( bytes_t * size ) const;

        const void * ptr;
        bytes_t size;
    };

    void const_memory_t :: resize ( const bytes_t & new_size, bool clear )
    {
        FUNC_ENTRY ();
        CONST_THROW ( xc_internal_err, "should never be called" );
    }

    const_memory_t :: const_memory_t ( const void * _ptr, const bytes_t & _size )
        : ptr ( _ptr )
        , size ( _size )
    {
    }

    const_memory_t :: ~ const_memory_t ()
    {
        ptr = 0;
        size = ( U64 ) 0;
    }

    void * const_memory_t :: get_mapped_memory ( bytes_t * _size ) const
    {
        if ( _size != 0 )
            * _size = size;
        return ( void * ) ptr;
    }


    /*------------------------------------------------------------------
     * Memory
     *  an object representing a range of address space
     */
    class Memory : public const_memory_t
    {
    public:

        virtual void resize ( const bytes_t & new_size, bool clear );
        Memory ( void * ptr, const bytes_t & size );
        ~ Memory ();
    };

    void Memory :: resize ( const bytes_t & new_size, bool clear )
    {
        FUNC_ENTRY ();

        MemMgrItf * mmgr = get_mmgr ();
        assert ( mmgr != 0 );

        assert ( ptr != 0 );
        assert ( new_size != size );

        ptr = mmgr -> _resize ( ( void * ) ptr, size, new_size, clear );
        size = new_size;
    }

    Memory :: Memory ( void * _ptr, const bytes_t & _size )
        : const_memory_t ( _ptr, _size )
    {
    }

    Memory :: ~ Memory ()
    {
        FUNC_ENTRY ();

        MemMgrItf * mmgr = get_mmgr ();
        assert ( mmgr != 0 );

        mmgr -> _free ( ( void * ) ptr, size );
    }


    /*------------------------------------------------------------------
     * PrimordMemMgr
     */

    MemMgr PrimordMemMgr :: make_primordial ()
    {
        assert ( callstk != 0 );
        FUNC_ENTRY ();

        // allow for the very first call
        static atomic_t < I32 > latch = 0;
        if ( latch . test_and_set ( 0, 1 ) != 0 )
        {
            const char msg [] = "primordial memory manager exists";
            if ( rsrc == 0 )
                throw msg;
            CONST_THROW ( xc_program_state_violation, msg );
        }

        // determine system memory quota
        size_t quota = ~ ( size_t ) 0;
#if UNIX
        struct rlimit rlim;
        int status = getrlimit ( RLIMIT_AS, & rlim );
        if ( status == 0 )
            quota = rlim . rlim_cur;
#endif

        // allocate the object memory
        PrimordMemMgr * obj;
        obj = ( PrimordMemMgr * ) calloc ( 1, sizeof * obj );
        if ( obj == 0 )
            throw "out of memory allocating primordial memory manager";

        // construct the primordial mmgr
        new ( obj ) PrimordMemMgr ( quota, quota - sizeof * obj );

        // embed self into object
        // DO NOT STORE DUPLICATE
        // as this would introduce a cycle
        obj -> mmgr = obj;
        obj -> obj_size = sizeof * obj;

        // create the reference
        return obj -> make_mmgr_ref ( obj, CAP_RDWR | CAP_ALLOC );
    }


    /* alloc
     */
    Mem PrimordMemMgr :: alloc ( const bytes_t & size, bool clear )
    {
        FUNC_ENTRY ();

        // allocate a raw block
        void * block = ( size == ( U64 ) 0 ) ? 0 : _alloc ( size, clear );

        // create the memory object
        Memory * obj = new Memory ( block, size );

        // create the reference
        return make_mem_ref ( obj, obj, NEW_MEM_CAPS );
    }

    Mem PrimordMemMgr :: make_const ( const void * ptr, const bytes_t & size )
    {
        FUNC_ENTRY ();

        // create the const memory object
        const_memory_t * obj = new const_memory_t ( ptr, size );

        // create the reference
        return make_mem_ref ( obj, obj, CONST_MEM_CAPS );
    }

    void * PrimordMemMgr :: _alloc ( const bytes_t & size, bool clear )
    {
        FUNC_ENTRY ();

        size_t bytes = size;

        // allocate from quota
        if ( avail . read_and_sub_ge ( size, size ) < size )
            THROW ( xc_mem_quota, "memory quota exhausted allocating %lu bytes", ( U64 ) size );

        // allocate using process memory manager
        void * ptr = clear ? calloc ( 1, bytes ) : malloc ( bytes );
        if ( ptr == 0 )
        {
            // return bytes to quota
            avail += bytes;

            // failure
            THROW ( xc_no_mem, "process memory exhausted allocating %zu bytes", bytes );
        }

       return ptr;

    }

    void * PrimordMemMgr :: _resize ( void * old_ptr, const bytes_t & old_size, const bytes_t & new_size, bool clear )
    {
        FUNC_ENTRY ();

        // nothing to do if there is no size change
        if ( old_size == new_size )
            return old_ptr;

        // check for quota
        if ( ( avail + old_size ) < new_size )
            THROW ( xc_mem_quota, "memory quota exhausted reallocating %lu to %lu bytes", ( U64 ) old_size, ( U64 ) new_size );

        size_t new_bytes = new_size;

        // not supposed to be called with bad values
        assert ( old_ptr != 0 || old_size == ( U64 ) 0 );

        // reallocate using process memory manager
        // alternatively, we could always allocate and copy, destroying old
        void * new_ptr = realloc ( old_ptr, new_bytes );
        if ( new_ptr == 0 )
            THROW ( xc_no_mem, "process memory exhausted reallocating %lu to %lu bytes", ( U64 ) old_size, ( U64 ) new_size );

        // update bytes remaining
        if ( old_size > new_size )
            avail += old_size - new_size;
        else
        {
            avail -= new_size - old_size;

            // clear extended area
            if ( clear )
                memset ( & ( ( char* ) new_ptr ) [ old_size ], 0, new_size - old_size );
        }

        return new_ptr;
    }

    void PrimordMemMgr :: _free ( void * ptr, const bytes_t & size )
    {
        // not supposed to be called with bad values
        assert ( ptr != 0 || size == ( U64 ) 0 );

        // return to the process memory manager
        // alternatively, could clear before free
        free ( ptr );

        // update bytes remaining
        if ( ( void * ) this != ptr )
            avail += size;
    }

    PrimordMemMgr :: PrimordMemMgr ( const bytes_t & q, const bytes_t & a )
        : quota ( q )
        , avail ( a )
    {
    }

    PrimordMemMgr :: ~ PrimordMemMgr ()
    {
        // TBD - can test if avail != quota
        quota = 0;
        avail = 0;
    }

    void PrimordMemMgr :: operator delete ( void * ptr )
    {
        free ( ptr );
    }
}
