/*

  vdb3.cmn.rsrc

 */

#include <vdb3/cmn/rsrc.hpp>

namespace vdb3
{
    Rsrc :: Rsrc ()
    {
    }

    Rsrc :: Rsrc( const vdb3::Rsrc & rsrc ) noexcept
    : RsrcLog ( rsrc )
    {
    }

    Rsrc :: Rsrc( vdb3::Rsrc && rsrc ) noexcept
    : RsrcLog ( rsrc )
    {
    }

    Rsrc :: ~ Rsrc () noexcept
    {
    }

    Rsrc & Rsrc :: operator = ( const Rsrc & rsrc ) noexcept
    {
        RsrcLog::operator = ( rsrc );
        return self;
    }

    Rsrc & Rsrc :: operator = ( Rsrc && rsrc ) noexcept
    {
        RsrcLog::operator = ( rsrc );
        return self;
    }
}
