/*

  vdb3.kfc.rsrc-tm

 */

#pragma once

#include <vdb3/kfc/tmmgr.hpp>

/**
 * @file kfc/rsrc-tm.hpp
 * @brief resource manager with TimeMgr interface
 */

namespace vdb3
{

    /*=====================================================*
     *                      RsrcTime                       *
     *=====================================================*/

    struct RsrcTime
    {
        /**
         * time
         *  @var access to time management resources
         */
        TimeMgr time;

        
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        bool operator == ( const RsrcTime & rsrc ) const noexcept;

        CXX_RULE_OF_FOUR_NE ( RsrcTime );

        RsrcTime ();
        ~ RsrcTime () noexcept;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
