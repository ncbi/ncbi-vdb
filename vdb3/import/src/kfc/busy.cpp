/*

  vdb3.kfc.busy

 */

#include <vdb3/kfc/busy.hpp>
#include <vdb3/kfc/ktrace.hpp>

namespace vdb3
{
#if BUSY_LOCK_IS_Z64
    inline
    bool compare_exchange ( Z64 & busy, Z64 & cur, const Z64 val )
    {
        bool same = ( busy == cur );
        cur = busy;
        if ( same )
            busy = val;
        return same;
    }
#elif BUSY_LOCK_IS_STD_ATOMIC_Z64

    inline
    bool compare_exchange ( std :: atomic < Z64 > & busy, Z64 & cur, const Z64 val )
    {
#if x86_64
        // I don't know how correct this is
        return busy . compare_exchange_weak ( cur, val );
#else
        // I know that weak doesn't work on ARM
        return busy . compare_exchange_strong ( cur, val );
#endif
    }
#else
#error "incomplete source"
#endif

    BusyLock :: BusyLock () noexcept
        : busy ( 0 )
    {
        KTRACE ( TRACE_GEEK, "constructing lock %p", this );
    }

    BusyLock :: ~ BusyLock () noexcept
    {
        KTRACE ( TRACE_GEEK, "destroying lock %p", this );
        // TBD - detect cases where value was true
        // and... throw? log?
#if BUSY_LOCK_IS_Z64
        busy = 0;
#endif
    }

    BusyLock & BusyLock :: acquireShared () const
    {
        KTRACE ( TRACE_GEEK, "acquiring shared lock %p", this );
        Z64 cur = 0;
        bool success = compare_exchange ( busy, cur, 1 );
        while ( ! success && cur > 0  )
        {
            Z64 next = cur + 1;
            if ( next < 0 )
            {
                throw ShareCountViolation (
                    XP ( XLOC )
                    << "object sharing count would exceed limit"
                    );
            }
            success = compare_exchange ( busy, cur, next );
        }
        if ( ! success )
        {
            throw ObjectBusyException (
                XP ( XLOC )
                << "object is busy"
                );
        }

        return * const_cast < BusyLock * > ( this );
    }

    BusyLock & BusyLock :: acquireExclusive () const
    {
        KTRACE ( TRACE_GEEK, "acquiring exclusive lock %p", this );
        Z64 cur = 0;
        if ( ! compare_exchange ( busy, cur, -1 ) )
        {
            throw ObjectBusyException (
                XP ( XLOC )
                << "object is busy"
                );
        }

        return * const_cast < BusyLock * > ( this );
    }

    BusyLock & BusyLock :: acquireAnyLock () const
    {
        KTRACE ( TRACE_GEEK, "acquiring any lock %p", this );
        Z64 cur = 0;
        bool success= compare_exchange ( busy, cur, 1 );
        while ( ! success )
        {
            Z64 next = ( cur < 0 ) ? cur - 1 : cur + 1;
            if ( cur > 0 && next < 0 )
            {
                throw ShareCountViolation (
                    XP ( XLOC )
                    << "object sharing count would exceed limit"
                    );
            }
            success = compare_exchange ( busy, cur, next );
        }

        return * const_cast < BusyLock * > ( this );
    }

    void BusyLock :: releaseShared () noexcept
    {
        KTRACE ( TRACE_GEEK, "releasing shared lock %p", this );
        Z64 cur = 1;
        bool success = compare_exchange ( busy, cur, 0 );
        while ( ! success && cur > 0 )
        {
            Z64 next = cur - 1;
            success = compare_exchange ( busy, cur, next );
        }
    }

    void BusyLock :: releaseExclusive () noexcept
    {
        KTRACE ( TRACE_GEEK, "releasing exclusive lock %p", this );
        Z64 cur = -1;
        bool success = compare_exchange ( busy, cur, 0 );
        while ( ! success && cur < 0 )
        {
            Z64 next = cur + 1;
            success = compare_exchange ( busy, cur, next );
        }
    }

    void BusyLock :: releaseAnyLock () noexcept
    {
        KTRACE ( TRACE_GEEK, "releasing any lock %p", this );
        Z64 cur = -2;
        bool success = compare_exchange ( busy, cur, -1 );
        while ( ! success && cur != 0 )
        {
            Z64 next = ( cur < 0 ) ? cur + 1 : cur - 1;
            success = compare_exchange ( busy, cur, next );
        }
    }
}
