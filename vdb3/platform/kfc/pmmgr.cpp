/*

  vdb3.kfc.pmmgr

 */

#include "pmmgr.hpp"
#include "pmem.hpp"

#include <vdb3/kfc/ktrace.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                     PMemMgrImpl                     *
     *=====================================================*/
    
    std :: atomic_flag PMemMgrImpl :: latch = ATOMIC_FLAG_INIT;
    
    MemRef PMemMgrImpl :: alloc ( size_t bytes, bool clear, bool wipe )
    {
        MRgn rgn = allocRgn ( bytes, clear );
        try
        {
            KTRACE ( TRACE_PRG, "creating PMemImpl object: clear = %s, wipe = %s"
                    , clear ? "true" : "false"
                    , wipe ? "true" : "false" );
            return new PMemImpl ( this,  rgn, clear, wipe );
        }
        catch ( ... )
        {
            KTRACE ( TRACE_QA, "deleting %zu bytes due to exception", bytes );
            freeRgn ( rgn, false );
            throw;
        }
    }

    
    MRgn PMemMgrImpl :: allocRgn ( size_t bytes, bool clear )
    {
        byte_t * ptr;
        try
        {
            KTRACE ( TRACE_PRG, "allocating %zu bytes", bytes );
            ptr = new byte_t [ bytes ];
        }
        catch ( ... )
        {
            throw MemoryExhausted (
                XP ( XLOC )
                << "out of memory allocating "
                << bytes
                << " bytes"
                );
        }

        KTRACE ( TRACE_GEEK, "creating MRgn" );
        MRgn rgn = makeMRgn ( ptr, bytes );

        if ( clear )
        {
            KTRACE ( TRACE_PRG, "zeroing %zu bytes", bytes );
            rgn . fill ( 0 );
        }

        return rgn;
    }

    void PMemMgrImpl :: reallocRgn ( MRgn & old_rgn, size_t new_size, bool clear, bool wipe )
    {
        // nothing to be done if size is same
        if ( old_rgn . size () == new_size )
        {
            KTRACE ( TRACE_PRG, "new size ( %zu ) is identical to existing size", new_size );
            return;
        }

        // allocate a new region without touching old
        MRgn new_rgn = allocRgn ( new_size, false );

        // copy over data
        KTRACE ( TRACE_PRG, "copying from old to new region" );
        MRgn copied = new_rgn . copy ( old_rgn );

        // clear any trailing area
        if ( clear && copied . size () < old_rgn . size () )
        {
            MRgn tail = new_rgn . subRgn ( copied . size () );
            KTRACE ( TRACE_PRG, "zeroing %zu trailing bytes in new region", tail . size () );
            tail . fill ( 0 );
        }

        // free the old region now
        // this creates a race condition that must be handled outside
        freeRgn ( old_rgn, wipe );

        // now update with the new allocation
        KTRACE ( TRACE_PRG, "updating Mem region with new pointer and size" );
        old_rgn = new_rgn;
    }

    void PMemMgrImpl :: freeRgn ( MRgn & rgn, bool wipe ) noexcept
    {
        // save the data
        MRgn old ( rgn );

        // invalidate the region now
        // meaning to null out the members, since it is externally sourced
        KTRACE ( TRACE_PRG, "destroying Mem region" );
        rgn . MRgn :: ~ MRgn ();

        // wipe before deleting
        if ( wipe )
        {
            KTRACE ( TRACE_PRG, "wiping %zu bytes in old block", old . size () );
            old . fill ( 0 );
        }

        // delete the memory
        KTRACE ( TRACE_PRG, "deleting %zu bytes in old block", old . size () );
        delete [] old . addr ();
    }

    PMemMgrImpl :: PMemMgrImpl ()
    {
        KTRACE ( TRACE_PRG, "acquiring primordial MemMgr latch" );
        if ( latch . test_and_set () )
        {
            throw PermissionViolation (
                XP ( XLOC, rc_logic_err )
                << "primordial memory manager is already initialized"
                );
        }

        KTRACE ( TRACE_PRG, "constructing primordial MemMgr @ 0x%zx", ( size_t ) this );
    }

    
    PMemMgrImpl :: ~ PMemMgrImpl () noexcept
    {
        KTRACE ( TRACE_PRG, "destroying primordial MemMgr @ 0x%zx", ( size_t ) this );

        KTRACE ( TRACE_PRG, "clearing primordial MemMgr latch" );
        latch . clear ();
    }

}
