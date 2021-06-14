/*

  vdb3.cmn.buffer

 */

#include <vdb3/cmn/buffer.hpp>
#include <vdb3/kfc/ktrace.hpp>
#include <vdb3/kfc/busy.hpp>
#include <vdb3/kfc/mmgr.hpp>
#include <vdb3/kfc/rsrc-mem.hpp>

namespace vdb3
{

    /*=====================================================*
     *                     BufferImpl                      *
     *=====================================================*/
    
    struct BufferImpl
        : Refcount
        , BufferItf
    {
        METH ( const MRgn & getBuffer () const noexcept )
        {
            return rgn;
        }
        
        METH ( size_t getSize () const noexcept )
        {
            return sz;
        }
        
        METH ( void setCapacity ( size_t bytes ) )
        {
            size_t cap = rgn . size ();
        
            KTRACE ( TRACE_GEEK, "changing capacity from %zu to %zu bytes"
                     , cap
                     , bytes
                );
        
            if ( bytes != cap )
            {
                XLocker xlock ( busy );

                // figure a capacity for the allocation
                size_t acap = ( bytes + 4095 ) & ~ ( size_t ) 4095;
                if ( acap != lock -> getMRgn () . size () )
                {
                    // release the region lock on Mem
                    lock . release ();

                    // resize the memory block to new capacity, either larger or smaller
                    mem . resize ( acap );

                    // reacquire the exclusive region lock
                    lock = mem . mapMRgn ();
                }

                // copy out the new region
                rgn = lock -> getMRgn () . subRgn ( 0, bytes );

                // limit the size to new capacity
                if ( sz > bytes )
                    sz = bytes;
            }
        }
        
        METH ( void setSize ( size_t bytes ) )
        {
            size_t cap = rgn . size ();

            KTRACE ( TRACE_GEEK, "changing size from %zu to %zu bytes", sz, bytes );

            if ( bytes > cap )
            {
                throw SizeViolation (
                    XP ( XLOC )
                    << xprob
                    << "failed to set buffer size"
                    << xcause
                    << "payload size of "
                    << bytes
                    << " bytes exceeds buffer capacity of "
                    << cap
                    );
            }
            sz = bytes;
        }
        
        METH ( bool wipeBeforeDelete () const noexcept )
        {
            return mem . wipeBeforeDelete ();
        }
        
        METH ( void wipe () )
        {
            setSize ( 0 );
            rgn . fill ( 0 );
        }
        
        METH ( Mem releaseMem () )
        {
            XLocker xlock ( busy );

            // no longer own memory region
            lock . release ();

            // resize to stated size
            mem . resize ( sz );

            // copy region
            Mem m = mem;

            // disconnect
            mem . release ();

            // whack rgn
            rgn . MRgn :: ~ MRgn ();

            return m;
        }
        
        BufferImpl ( const RsrcKfc & rsrc, size_t initial_capacity )
            : mem ( rsrc . mmgr . alloc ( initial_capacity, true, true ) )
            , lock ( mem . mapMRgn () )
            , rgn ( lock -> getMRgn () )
            , sz ( 0 )
        {
        }
        
        METH ( ~ BufferImpl () noexcept )
        {
            sz = 0;
        }
        
        Mem mem;
        MRgnLockRef lock;
        BusyLock busy;
        MRgn rgn;
        size_t sz;
    };
        
    /*=====================================================*
     *                        Buffer                       *
     *=====================================================*/
    
    /**
     * increaseCapacity
     * @brief increase buffer size minimally by amount specified
     * @param amt amount of size increment
     */
    Buffer & Buffer :: increaseCapacity ( size_t amt )
    {
        size_t cap = getCapacity ();
        
        KTRACE ( TRACE_GEEK, "increasing capacity by %zu bytes"
                , amt
            );

        // detect rollover - this will fail anyway
        // but let it fail within setCapacity
        if ( SIZE_T_MAX - amt < cap )
            return setCapacity ( SIZE_T_MAX );

        return setCapacity ( cap + amt );
    }

    /**
     * ensureAdditionalCapacity
     * @brief ensure that there is capacity for additional amount
     * @param amt amount of size increment
     */
    Buffer & Buffer :: ensureAdditionalCapacity ( size_t amt )
    {
        size_t cap = getCapacity ();
        size_t sz = getSize ();
        
        KTRACE ( TRACE_GEEK, "ensuring additional capacity of %zu bytes"
                , amt
            );

        // detect rollover for "sz + amt" expression
        if ( SIZE_T_MAX - amt < sz )
            return setCapacity ( SIZE_T_MAX );

        // only if requested additional capacity would exceed current
        if ( sz + amt > cap )
            setCapacity ( sz + amt );

        return self;
    }

    /**
     * reinitialize
     * @brief resets capacity to indicated amount and size to 0
     */
    Buffer & Buffer :: reinitialize ( size_t amt )
    {
        // how 'bout this for easy to debug code?
        return setSize ( 0 ) . setCapacity ( amt ) . wipe ();
    }

    Buffer & Buffer :: operator = ( Buffer && b ) noexcept
    {
        r = std :: move ( b . r );
        return self;
    }

    Buffer & Buffer :: operator = ( const Buffer & b ) noexcept
    {
        r = b . r;
        return self;
    }

    Buffer :: Buffer ( Buffer && b ) noexcept
        : r ( std :: move ( b . r ) )
    {
    }

    Buffer :: Buffer ( const Buffer & b ) noexcept
        : r ( b . r )
    {
    }

    Buffer :: Buffer () noexcept
    {
    }

    Buffer :: ~ Buffer () noexcept
    {
    }

    Buffer & Buffer :: operator = ( const BufferRef & b ) noexcept
    {
        r = b;
        return self;
    }

    Buffer :: Buffer ( const BufferRef & b ) noexcept
        : r ( b )
    {
    }

    Buffer :: Buffer ( const RsrcKfc & rsrc, size_t initial_capacity )
        : r ( new BufferImpl ( rsrc, initial_capacity ) )
    {
    }
}
