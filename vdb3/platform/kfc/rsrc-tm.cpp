/*

  vdb3.kfc.rsrc-tm

 */

#include <vdb3/kfc/rsrc-tm.hpp>
#include <vdb3/kfc/ktrace.hpp>

#include "kfc-priv.hpp"

namespace vdb3
{
    
    /*=====================================================*
     *                      RsrcTime                       *
     *=====================================================*/

    bool RsrcTime :: operator == ( const RsrcTime & rsrc ) const noexcept
    {
        return time == rsrc . time;
    }

    RsrcTime & RsrcTime :: operator = ( RsrcTime && rsrc ) noexcept
    {
        time = std :: move ( rsrc . time );
        return self;
    }

    RsrcTime & RsrcTime :: operator = ( const RsrcTime & rsrc ) noexcept
    {
        time = rsrc . time;
        return self;
    }

    RsrcTime :: RsrcTime ( RsrcTime && rsrc ) noexcept
        : time ( std :: move ( rsrc . time ) )
    {
    }

    RsrcTime :: RsrcTime ( const RsrcTime & rsrc ) noexcept
        : time ( rsrc . time )
    {
    }

    RsrcTime :: RsrcTime ()
    {
        KTRACE ( TRACE_PRG, "creating system time manager block @ %p", this );
        time = makeTimeMgr ();
        KTRACE ( TRACE_GEEK, "created system time manager block @ %p", this );
    }

    RsrcTime :: ~ RsrcTime () noexcept
    {
        KTRACE ( TRACE_PRG, "releasing time manager block @ %p", this );
    }
}
