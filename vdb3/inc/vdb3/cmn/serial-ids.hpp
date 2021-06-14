/*

  vdb3.cmn.serial-ids

 */

#pragma once

#include <vdb3/cmn/defs.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                     SerialType                      *
     *=====================================================*/

    // abstract
    const SerialType SERIAL_NULL         = 0;
    const SerialType SERIAL_UNTYPED      = 1;

    // intrinsic
    const SerialType SERIAL_NATURAL      = 10;
    const SerialType SERIAL_INTEGER      = 11;
    const SerialType SERIAL_BINARY_REAL  = 12;
    const SerialType SERIAL_DECIMAL_REAL = 13; // an intrinsic tuple
    const SerialType SERIAL_BOOLEAN      = 14;
    const SerialType SERIAL_TEXT         = 15; // an intrinsic tuple
    const SerialType SERIAL_TIME         = 16;
    const SerialType SERIAL_TIMESTAMP    = 17;

    // fs
    const SerialType SERIAL_FSPATH       = 30;

    // net
    const SerialType SERIAL_IPADDR       = 40;
    const SerialType SERIAL_IPENDPOINT   = 41;

    // application-specific start id
    const SerialType SERIAL_APPSTART     = 1000;
}
