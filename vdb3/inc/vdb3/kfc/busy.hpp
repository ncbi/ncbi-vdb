/*

  vdb3.kfc.busy

 */

#pragma once

#include <vdb3/kfc/except.hpp>

#include <atomic>

/**
 * @file kfc/busy.hpp
 * @brief simple busy-lock for mutable objects
 */

namespace vdb3
{

    /*=====================================================*
     *                      BusyLock                       *
     *=====================================================*/
    
    /**
     * @class BusyLock
     * @brief an atomic element to maintain busy-state
     */
    class BusyLock
    {
    public:

        BusyLock & operator = ( BusyLock && lock ) = delete;
        BusyLock & operator = ( const BusyLock & lock ) = delete;
        BusyLock ( BusyLock && lock ) = delete;
        BusyLock ( const BusyLock & lock ) = delete;

        BusyLock () noexcept;
        ~ BusyLock () noexcept;

    private:

        BusyLock & acquireShared () const;
        BusyLock & acquireExclusive () const;
        BusyLock & acquireAnyLock () const;
        void releaseShared () noexcept;
        void releaseExclusive () noexcept;
        void releaseAnyLock () noexcept;

#if NO_ATOMIC_OPS || NO_BUSY_LOCK
#define BUSY_LOCK_IS_Z64 1
        mutable Z64 busy;
#else
#define BUSY_LOCK_IS_STD_ATOMIC_Z64 1
        mutable std :: atomic < Z64 > busy;
#endif

        friend class SLocker;
        friend class XLocker;
        friend class ALocker;
    };


    /*=====================================================*
     *                       SLocker                       *
     *=====================================================*/
    
    /**
     * @class SLocker
     * @brief acquires and releases shared lock
     */
    class SLocker
    {
    public:

        SLocker & operator = ( SLocker && locker ) = delete;
        SLocker & operator = ( const SLocker & locker ) = delete;
        SLocker ( SLocker && locker ) = delete;
        SLocker ( const SLocker & locker ) = delete;
        SLocker () = delete;

        inline
        explicit SLocker ( const BusyLock & lock )
            : busy ( lock . acquireShared () ) {}
        
        inline
        ~ SLocker () noexcept
        { busy . releaseShared (); }

    private:

        BusyLock & busy;
    };


    /*=====================================================*
     *                       XLocker                       *
     *=====================================================*/
    
    /**
     * @class XLocker
     * @brief acquires and releases exclusive lock
     */
    class XLocker
    {
    public:

        XLocker & operator = ( XLocker && locker ) = delete;
        XLocker & operator = ( const XLocker & locker ) = delete;
        XLocker ( XLocker && locker ) = delete;
        XLocker ( const XLocker & locker ) = delete;
        XLocker () = delete;
        
        inline
        explicit XLocker ( const BusyLock & lock )
            : busy ( lock . acquireExclusive () ) {}
        
        inline
        ~ XLocker () noexcept
        { busy . releaseExclusive (); }

    private:

        BusyLock & busy;
    };


    /*=====================================================*
     *                       ALocker                       *
     *=====================================================*/
    
    /**
     * @class ALocker
     * @brief acquires and releases any kind of lock
     */
    class ALocker
    {
    public:

        ALocker & operator = ( ALocker && locker ) = delete;
        ALocker & operator = ( const ALocker & locker ) = delete;
        ALocker ( ALocker && locker ) = delete;
        ALocker ( const ALocker & locker ) = delete;
        ALocker () = delete;
        
        inline
        explicit ALocker ( const BusyLock & lock )
            : busy ( lock . acquireAnyLock () ) {}
        
        inline
        ~ ALocker () noexcept
        { busy . releaseAnyLock (); }

    private:

        BusyLock & busy;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

    DECLARE_EXCEPTION ( ObjectBusyException, InternalError );
    DECLARE_EXCEPTION ( ShareCountViolation, ConstraintViolation );
}
