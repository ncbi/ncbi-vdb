/*

  vdb3.cmn.defs


 */

#pragma once

#include <vdb3/kfc/defs.hpp>
#include <vdb3/kfc/ktrace.hpp>

namespace vdb3
{

    /*=====================================================*
     *                       FORWARDS                      *
     *=====================================================*/

    class Fmt;
    class String;
    class Serial;
    class Deserial;
    struct RsrcTrace;
    struct RsrcLog;
    
    /*=====================================================*
     *                       TYPEDEFS                      *
     *=====================================================*/

    typedef RsrcLog RsrcCmn;

    /**
     * @typedef SerialType
     * @brief a durable constant for serialization
     */
    typedef N32 SerialType;
}
