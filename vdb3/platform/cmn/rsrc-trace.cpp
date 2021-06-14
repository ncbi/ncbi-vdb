/*

  vdb3.cmn.rsrc-trace

 */

#include <vdb3/cmn/rsrc-trace.hpp>
#include <vdb3/kfc/ktrace.hpp>

#include "ptracer.hpp"
#include "trace-impl.hpp"

namespace vdb3
{
    bool RsrcTrace :: operator == ( const RsrcTrace & rsrc ) const noexcept
    {
        return RsrcKfc :: operator == ( rsrc )
            && tracer == rsrc . tracer
            && trace == rsrc . trace
            ;
    }
    
    RsrcTrace & RsrcTrace :: operator = ( RsrcTrace && rsrc ) noexcept
    {
        RsrcMem :: operator = ( std :: move ( rsrc ) );
        tracer = std :: move ( rsrc . tracer );
        trace = std :: move ( rsrc . trace );
        return self;
    }
    
    RsrcTrace & RsrcTrace :: operator = ( const RsrcTrace & rsrc ) noexcept
    {
        RsrcMem :: operator = ( rsrc );
        tracer = rsrc . tracer;
        trace = rsrc . trace;
        return self;
    }
    
    RsrcTrace :: RsrcTrace ( RsrcTrace && rsrc ) noexcept
        : RsrcKfc ( std :: move ( rsrc ) )
        , tracer ( std :: move ( rsrc . tracer ) )
        , trace ( std :: move ( rsrc . trace ) )
    {
    }
    
    RsrcTrace :: RsrcTrace ( const RsrcTrace & rsrc ) noexcept
        : RsrcKfc ( rsrc )
        , tracer ( rsrc . tracer )
        , trace ( rsrc . trace )
    {
    }
    
    RsrcTrace :: RsrcTrace ()
    {
        KTRACE ( TRACE_PRG, "creating primordial tracing facility" );
        TracerRef t ( new PTracerImpl );
        TraceMgrRef tr ( new TraceMgrImpl );
        tracer = t;
        trace = tr;
        KTRACE ( TRACE_GEEK, "created primordial tracing facility" );
    }
    
    RsrcTrace :: ~ RsrcTrace () noexcept
    {
        KTRACE ( TRACE_PRG, "releasing tracer and trace manager" );
    }
}
