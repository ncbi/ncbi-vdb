/*

  vdb3.kfc.defs

 */

#include <vdb3/kfc/defs.hpp>

#if HAVE_Z128
Z128 __bswap_128 ( N128 n ) noexcept
{
    Z128 rtn ( __bswap_64 ( ( N64 ) n ) );
    rtn <<= 64;
    rtn |= __bswap_64 ( ( N64 ) ( n >> 64 ) );
    return rtn;
}
#endif
