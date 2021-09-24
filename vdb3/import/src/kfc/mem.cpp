/*

  vdb3.kfc.mem

 */

#include <vdb3/kfc/mem.hpp>

namespace vdb3
{
    
    Mem & Mem :: operator = ( Mem && _mem ) noexcept
    {
        mem = std :: move ( _mem . mem );
        return self;
    }
    
    Mem & Mem :: operator = ( const Mem & _mem ) noexcept
    {
        mem = _mem . mem;
        return self;
    }
    
    Mem :: Mem ( Mem && _mem ) noexcept
        : mem ( std :: move ( _mem . mem ) )
    {
    }
    
    Mem :: Mem ( const Mem & _mem ) noexcept
        : mem ( _mem . mem )
    {
    }
    
    Mem :: Mem () noexcept
    {
    }
    
    Mem :: ~ Mem () noexcept
    {
    }
    
    Mem & Mem :: operator = ( const MemRef & _mem ) noexcept
    {
        mem = _mem;
        return self;
    }

    Mem :: Mem ( const MemRef & _mem ) noexcept
        : mem ( _mem )
    {
    }

}
