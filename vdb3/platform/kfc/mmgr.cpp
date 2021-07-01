/*

  vdb3.kfc.mmgr

 */

#include <vdb3/kfc/mmgr.hpp>

namespace vdb3
{
    
    MemMgr & MemMgr :: operator = ( MemMgr && _mgr ) noexcept
    {
        mgr = std :: move ( _mgr . mgr );
        return self;
    }
    
    MemMgr & MemMgr :: operator = ( const MemMgr & _mgr ) noexcept
    {
        mgr = _mgr . mgr;
        return self;
    }
    
    MemMgr :: MemMgr ( MemMgr && _mgr ) noexcept
        : mgr ( std :: move ( _mgr . mgr ) )
    {
    }
    
    MemMgr :: MemMgr ( const MemMgr & _mgr ) noexcept
        : mgr ( _mgr . mgr )
    {
    }
    
    MemMgr :: MemMgr () noexcept
    {
    }
    
    MemMgr :: ~ MemMgr () noexcept
    {
    }
    
    MemMgr & MemMgr :: operator = ( const MemMgrRef & _mgr ) noexcept
    {
        mgr = _mgr;
        return self;
    }

    MemMgr :: MemMgr ( const MemMgrRef & _mgr ) noexcept
        : mgr ( _mgr )
    {
    }

}
