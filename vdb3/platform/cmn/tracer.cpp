/*

  vdb3.cmn.tracer


 */

#include <vdb3/cmn/tracer.hpp>

namespace vdb3
{

    
    /*=====================================================*
     *                      TraceQueue                     *
     *=====================================================*/

    TraceQueue :: TraceQueue ( const String & _nm, N32 _id ) noexcept
        : nm ( _nm )
        , id ( _id )
    {
    }
    
    TraceQueue :: ~ TraceQueue ()
    {
    }
    
    /*=====================================================*
     *                        Tracer                       *
     *=====================================================*/

    TraceQueueId Tracer :: getQueueId ( const String & name ) const
    {
        // TBD - may maintain a cache here
        return tracer -> getQueueId ( name );
    }


    TraceQueueId Tracer :: makeQueue ( const String & name ) const
    {
        // TBD - may maintain a cache here
        return tracer -> makeQueue ( name );
    }


    Tracer & Tracer :: operator = ( Tracer && l ) noexcept
    {
        tracer = std :: move ( l . tracer );
        return self;
    }
    
    Tracer & Tracer :: operator = ( const Tracer & l ) noexcept
    {
        tracer = l . tracer;
        return self;
    }
    
    Tracer :: Tracer ( Tracer && l ) noexcept
        : tracer ( std :: move ( l . tracer ) )
    {
    }
    
    Tracer :: Tracer ( const Tracer & l ) noexcept
        : tracer ( l . tracer )
    {
    }

    Tracer :: Tracer () noexcept
    {
    }
    
    Tracer :: ~ Tracer () noexcept
    {
    }

    Tracer & Tracer :: operator = ( const TracerRef & l ) noexcept
    {
        tracer = l;
        return self;
    }
    
    Tracer :: Tracer ( const TracerRef & l ) noexcept
        : tracer ( l )
    {
    }

}
