/*

  vdb3.cmn.stream

 */

#include <vdb3/cmn/stream.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                        Stream                       *
     *=====================================================*/

    Stream & Stream :: operator = ( Stream && _strm ) noexcept
    {
        strm = std :: move ( _strm . strm );
        return self;
    }
    
    Stream & Stream :: operator = ( const Stream & _strm ) noexcept
    {
        strm = _strm . strm;
        return self;
    }
    
    Stream :: Stream ( Stream && _strm ) noexcept
        : strm ( std :: move ( _strm . strm ) )
    {
    }
    
    Stream :: Stream ( const Stream & _strm ) noexcept
        : strm ( _strm . strm )
    {
    }
    
    Stream :: Stream () noexcept
    {
    }
    
    Stream :: ~ Stream () noexcept
    {
    }

    Stream :: Stream ( const StreamRef & _strm ) noexcept
        : strm ( _strm )
    {
    }
    
    /*=====================================================*
     *                        MStream                       *
     *=====================================================*/

    MStream & MStream :: operator = ( MStream && _strm ) noexcept
    {
        strm = std :: move ( _strm . strm );
        return self;
    }
    
    MStream & MStream :: operator = ( const MStream & _strm ) noexcept
    {
        strm = _strm . strm;
        return self;
    }
    
    MStream :: MStream ( MStream && _strm ) noexcept
        : strm ( std :: move ( _strm . strm ) )
    {
    }
    
    MStream :: MStream ( const MStream & _strm ) noexcept
        : strm ( _strm . strm )
    {
    }
    
    MStream :: MStream () noexcept
    {
    }
    
    MStream :: ~ MStream () noexcept
    {
    }

    MStream :: MStream ( const MStreamRef & _strm ) noexcept
        : strm ( _strm )
    {
    }

}
