/*

  vdb3.kfc.pmem

 */

#include "pmem.hpp"
#include "pmem-rgn-lock.hpp"
#include "pmmgr.hpp"

#include <vdb3/kfc/ktrace.hpp>

namespace vdb3
{

    size_t PMemImpl :: size () const
    {
        // don't care if it's already locked
        // just ensure it stays that way
        ALocker lock ( busy );

        return rgn . size ();
    }
    
    void PMemImpl :: resize ( size_t new_size )
    {
        // must have exclusive access
        XLocker lock ( busy );

        // must be resizeable
        if ( ! isResizable () )
        {
            throw ConstraintViolation (
                XP ( XLOC )
                << xprob
                << "failed to resize memory region"
                << xcause
                << "memory is not resizeable"
                );
        }

        // ask the memory manager to resize
        KTRACE ( TRACE_PRG, "resizing memory region from %zu to %zu bytes", rgn . size (), new_size );
        mmgr -> reallocRgn ( rgn, ( size_t ) new_size, clear, wipe );
    }
    
    MRgnLockRef PMemImpl :: mapMRgn ()
    {
        KTRACE ( TRACE_PRG, "locking mutable memory region" );
        return new PMemMRgnLockImpl ( this );
    }
    
    CRgnLockRef PMemImpl :: mapCRgn () const
    {
        KTRACE ( TRACE_PRG, "mapping constant memory region" );
        return new PMemCRgnLockImpl ( this );
    }

    PMemImpl :: PMemImpl ( const IRef < PMemMgrImpl > & _mmgr, const MRgn & _rgn, bool _clear, bool _wipe ) noexcept
        : mmgr ( _mmgr )
        , rgn ( _rgn )
        , writable ( true )
        , resizeable ( true )
        , clear ( _clear )
        , wipe ( _wipe )
    {
        KTRACE ( TRACE_PRG
                , "constructing PMemImpl with %zu byte %sregion, attached reference to PMemMgrImpl"
                , rgn . size ()
                , ( clear ? "zeroed " : "" )
            );
    }
    
    PMemImpl :: ~ PMemImpl () noexcept
    {
        KTRACE ( TRACE_PRG, "freeing%s memory", ( clear ? " and wiping" : "" ) );
        mmgr -> freeRgn ( rgn, wipe );

        KTRACE ( TRACE_PRG
                , "auto-releasing reference to PMemMgrImpl @ 0x%zx and destroying PMemImpl"
                , ( size_t ) mmgr . get ()
            );
    }

}
