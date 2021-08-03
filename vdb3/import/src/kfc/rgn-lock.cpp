/*

  vdb3.kfc.rgn-lock

 */

#include <vdb3/kfc/rgn-lock.hpp>

namespace vdb3
{
    MRgnLocker :: MRgnLocker ( const MRgnLockRef & _lock ) noexcept
        : lock ( _lock )
        , mrgn ( lock -> getMRgn () )
    {
    }
    
    MRgnLocker :: ~ MRgnLocker () noexcept
    {
    }

    CRgnLocker :: CRgnLocker ( const CRgnLockRef & _lock ) noexcept
        : lock ( _lock )
        , crgn ( lock -> getCRgn () )
    {
    }
    
    CRgnLocker :: ~ CRgnLocker () noexcept
    {
    }
}
