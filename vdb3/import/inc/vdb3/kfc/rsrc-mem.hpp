/*

  vdb3.kfc.rsrc-mem

 */

#pragma once

#include <vdb3/kfc/rsrc-tm.hpp>
#include <vdb3/kfc/mmgr.hpp>

/**
 * @file kfc/rsrc-mem.hpp
 * @brief resource manager with MemMgr interface
 */

namespace vdb3
{

    /*=====================================================*
     *                       RsrcMem                       *
     *=====================================================*/

    struct RsrcMem : RsrcTime
    {
        /**
         * mmgr
         *  @var the source of new memory available within a domain/context
         */
        MemMgr mmgr;

        
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        bool operator == ( const RsrcMem & rsrc ) const noexcept;

        CXX_RULE_OF_FOUR_NE ( RsrcMem );

        RsrcMem ();
        ~ RsrcMem () noexcept;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
