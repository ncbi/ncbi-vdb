/*

  vdb3.kfc.tmmgr

 */

#include <vdb3/kfc/tmmgr.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                       TimeMgr                       *
     *=====================================================*/

    TimeMgr & TimeMgr :: operator = ( TimeMgr && tm ) noexcept
    {
        mgr = std :: move ( tm . mgr );
        return self;
    }

    TimeMgr & TimeMgr :: operator = ( const TimeMgr & tm ) noexcept
    {
        mgr = tm . mgr;
        return self;
    }

    TimeMgr :: TimeMgr ( TimeMgr && tm ) noexcept
        : mgr ( std :: move ( tm . mgr ) )
    {
    }

    TimeMgr :: TimeMgr ( const TimeMgr & tm ) noexcept
        : mgr ( tm . mgr )
    {
    }

    TimeMgr :: TimeMgr () noexcept
    {
    }

    TimeMgr :: ~ TimeMgr () noexcept
    {
    }

    TimeMgr & TimeMgr :: operator = ( const TimeMgrRef & tm ) noexcept
    {
        mgr = tm;
        return self;
    }

    TimeMgr :: TimeMgr ( const TimeMgrRef & tm ) noexcept
        : mgr ( tm )
    {
    }
}
