/*

  vdb3.cmn.obj

 */

#include <vdb3/cmn/obj.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                         Obj                         *
     *=====================================================*/

    Obj & Obj :: operator = ( Obj && _obj ) noexcept
    {
        obj = std :: move ( _obj . obj );
        return self;
    }
    
    Obj & Obj :: operator = ( const Obj & _obj ) noexcept
    {
        obj = _obj . obj;
        return self;
    }
    
    Obj :: Obj ( Obj && _obj ) noexcept
        : obj ( std :: move ( _obj . obj ) )
    {
    }
    
    Obj :: Obj ( const Obj & _obj ) noexcept
        : obj ( _obj . obj )
    {
    }
    
    Obj :: Obj () noexcept
    {
    }
    
    Obj :: ~ Obj () noexcept
    {
    }

    Obj :: Obj ( const ObjRef & _obj ) noexcept
        : obj ( _obj )
    {
    }

    /*=====================================================*
     *                         MObj                         *
     *=====================================================*/

    MObj & MObj :: operator = ( MObj && _obj ) noexcept
    {
        obj = std :: move ( _obj . obj );
        return self;
    }
    
    MObj & MObj :: operator = ( const MObj & _obj ) noexcept
    {
        obj = _obj . obj;
        return self;
    }
    
    MObj :: MObj ( MObj && _obj ) noexcept
        : obj ( std :: move ( _obj . obj ) )
    {
    }
    
    MObj :: MObj ( const MObj & _obj ) noexcept
        : obj ( _obj . obj )
    {
    }
    
    MObj :: MObj () noexcept
    {
    }
    
    MObj :: ~ MObj () noexcept
    {
    }

    MObj :: MObj ( const MObjRef & _obj ) noexcept
        : obj ( _obj )
    {
    }

}
