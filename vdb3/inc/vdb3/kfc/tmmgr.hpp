/*

  vdb3.kfc.tmmgr

 */

#pragma once

#include <vdb3/kfc/defs.hpp>
#include <vdb3/kfc/except.hpp>
#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/time.hpp>

/**
 * @file kfc/tmmgr.hpp
 * @brief time manager interface
 */

namespace vdb3
{
    
    /*=====================================================*
     *                     TimeMgrItf                      *
     *=====================================================*/

    /**
     * TimeMgrItf
     *  @brief access to time resources
     */
    interface TimeMgrItf
    {
        /**
         * now
         *  @brief return the current timestamp
         */
        MSG ( Timestamp now () const );

        /* local
         *  @brief transform Timestamp into local CalTime
         */
        MSG ( CalTime local ( const Timestamp & ts ) const );

        /* utc
         *  @brief transform Timestamp into utc CalTime
         */
        MSG ( CalTime utc ( const Timestamp & ts ) const );

        /**
         * sleep
         *  @brief sleep for a period of time or until signal
         *  @param tm is not a timestamp but a quantity of time
         *  @return NanoSeconds remaining
         */
        MSG ( NanoSeconds sleep ( const TimeBase & tm ) const );

        /**
         * ~TimeMgrItf
         *  @brief releases and destroys object
         */
        virtual ~ TimeMgrItf () noexcept {}
        
    };

    /**
     * @typedef TimeMgrRef
     * @brief a shared reference to an TimeMgrItf
     */
    typedef IRef < TimeMgrItf > TimeMgrRef;

    
    /*=====================================================*
     *                       TimeMgr                       *
     *=====================================================*/

    class TimeMgr
    {
    public:

        /**
         * now
         * @brief return the current timestamp
         */
        inline Timestamp now () const
        { return mgr -> now (); }

        /* local
         *  @brief transform Timestamp into local CalTime
         */
        inline CalTime local ( const Timestamp & ts ) const
        { return mgr -> local ( ts ); }

        /* utc
         *  @brief transform Timestamp into utc CalTime
         */
        inline CalTime utc ( const Timestamp & ts ) const
        { return mgr -> utc ( ts ); }

        /**
         * sleep
         *  @brief sleep for a period of time or until signal
         *  @param tm is not a timestamp but a quantity of time
         */
        inline NanoSeconds sleep ( const TimeBase & tm ) const
        { return mgr -> sleep ( tm ); }


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_EIGHT_EQOPS_NE ( TimeMgr, mgr );

    private:
        
        TimeMgrRef mgr;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
