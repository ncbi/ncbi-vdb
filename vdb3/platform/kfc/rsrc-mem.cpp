/*

  vdb3.kfc.rsrc-mem

 */

#include <vdb3/kfc/rsrc-mem.hpp>
#include <vdb3/kfc/ktrace.hpp>

#include "pmmgr.hpp"

namespace vdb3
{
    
    /*=====================================================*
     *                       RsrcMem                       *
     *=====================================================*/
    
    bool RsrcMem :: operator == ( const RsrcMem & rsrc ) const noexcept
    {
        return RsrcTime :: operator == ( rsrc )
            && mmgr == rsrc . mmgr
            ;
    }
    
    RsrcMem & RsrcMem :: operator = ( RsrcMem && rsrc ) noexcept
    {
        RsrcTime :: operator = ( std :: move ( rsrc ) );
        mmgr = std :: move ( rsrc . mmgr );
        return self;
    }
    
    RsrcMem & RsrcMem :: operator = ( const RsrcMem & rsrc ) noexcept
    {
        RsrcTime :: operator = ( rsrc );
        mmgr = rsrc . mmgr;
        return self;
    }
    
    RsrcMem :: RsrcMem ( RsrcMem && rsrc ) noexcept
        : RsrcTime ( std :: move ( rsrc ) )
        , mmgr ( std :: move ( rsrc . mmgr ) )
    {
    }
    
    RsrcMem :: RsrcMem ( const RsrcMem & rsrc ) noexcept
        : RsrcTime ( rsrc )
        , mmgr ( rsrc . mmgr )
    {
    }
    
    RsrcMem :: RsrcMem ()
    {
        KTRACE ( TRACE_PRG, "creating primordial memory manager" );
        MemMgrRef mmgrr ( new PMemMgrImpl () );
        mmgr = mmgrr;
        KTRACE ( TRACE_GEEK, "created primordial memory manager" );
    }
    
    RsrcMem :: ~ RsrcMem () noexcept
    {
        KTRACE ( TRACE_PRG, "releasing memory manager" );
    }
}
