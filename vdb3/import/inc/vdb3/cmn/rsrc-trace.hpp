/*

  vdb3.cmn.rsrc-trace

 */

#pragma once

#include <vdb3/kfc/rsrc-mem.hpp>
#include <vdb3/cmn/tracer.hpp>
#include <vdb3/cmn/trace.hpp>

/**
 * @file cmn/rsrc-trace.hpp
 * @brief resource manager with TraceMgr interface
 */

namespace vdb3
{

    
    /*=====================================================*
     *                      RsrcTrace                      *
     *=====================================================*/

    struct RsrcTrace : RsrcKfc
    {
        /**
         * tracer
         *  @var tracing output
         */
        Tracer tracer;

        /**
         * trace
         *  @var tracing manager
         */
        TraceMgr trace;

        
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        bool operator == ( const RsrcTrace & rsrc ) const noexcept;

        CXX_RULE_OF_FOUR_NE ( RsrcTrace );

        RsrcTrace ();
        ~ RsrcTrace () noexcept;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
