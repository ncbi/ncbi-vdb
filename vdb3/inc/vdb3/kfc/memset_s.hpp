/*

  vdb3.kfc.memset_s

 */

#pragma once

// include this ONLY to bring in stdclib memset_s() if it is available
#include <vdb3/kfc/defs.hpp>

namespace vdb3
{
    /**
     * memset_s
     * @brief fills a memory buffer with a single byte up to requested limit
     * @brief upon exception, fills entire non-null buffer with byte before throwing
     * @param dest a non-null pointer to destsz bytes
     * @param destsz the number of bytes designated by dest
     * @param count the number of bytes to be overwritten
     * @exception NullArgumentException if "dest" is null
     * @exception SizeViolation if "count" exceeds RSIZE_MAX
     * @exception OverflowException if "count" exceeds "destsz"
     * @return 0 on success
     */
    int memset_s ( void * dest, size_t destsz, int ch, size_t count );
}
