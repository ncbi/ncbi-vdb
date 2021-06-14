/*

  vdb3.cmn.rsrc

 */

#pragma once

#include <vdb3/cmn/rsrc-log.hpp>

/**
 * @file cmn/rsrc.hpp
 * @brief resource manager
 */

namespace vdb3
{

    /*=====================================================*
     *                       Rsrc                          *
     *=====================================================*/

    struct Rsrc : RsrcLog
    {
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        bool operator == ( const Rsrc & rsrc ) const noexcept;

        CXX_RULE_OF_FOUR_NE ( Rsrc );

        Rsrc ();
        ~ Rsrc () noexcept;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
