/*

  vdb3.cmn.rsrc_log

 */

#pragma once

#include <vdb3/cmn/rsrc-trace.hpp>
#include <vdb3/cmn/logger.hpp>
#include <vdb3/cmn/log.hpp>

/**
 * @file cmn/rsrc-log.hpp
 * @brief resource manager with LogMgr interface
 */

namespace vdb3
{
    
    /*=====================================================*
     *                       RsrcLog                       *
     *=====================================================*/

    struct RsrcLog : RsrcTrace
    {

        /**
         * logger
         *  @var logging output
         */
        Logger logger;

        /**
         * log
         *  @var logging output
         */
        LogMgr log;

        
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        bool operator == ( const RsrcLog & rsrc ) const noexcept;

        CXX_RULE_OF_FOUR_NE ( RsrcLog );

        RsrcLog ();
        ~ RsrcLog () noexcept;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
