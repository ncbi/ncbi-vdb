/*

  vdb3.cmn.ptracer


 */

#include "ptracer.hpp"
#include <vdb3/kfc/ktrace.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                     PTracerImpl                     *
     *=====================================================*/

    std :: atomic_flag PTracerImpl :: latch = ATOMIC_FLAG_INIT;

    TraceQueueId PTracerImpl :: getQueueId ( const String & name ) const
    {
        // TBD - must be protected by read-lock
        
        auto it = qmap . find ( name );
        if ( it == qmap . end () )
        {
            throw NotFoundException (
                XP ( XLOC )
                << xprob
                << "failed to get queue id"
                << xcause
                << "queue '"
                << name
                << "' does not exist"
                );
        }
            
        return it -> second;
    }
    
    TraceQueueId PTracerImpl :: makeQueue ( const String & name )
    {
        // TBD - must be protected by write-lock
        
        auto it = qmap . find ( name );
        if ( it != qmap . end () )
            return it -> second;

        TraceQueueId qid = makeQueueId ( name, ++ qids );
        qmap . emplace ( name, qid );

        return qid;
    }
    
    void PTracerImpl :: write ( Z32 lvl, const char * file,
        const char * func, N32 line, const CText & msg ) const noexcept
    {
        print_trace ( lvl, file, line, func, "%.*s"
                      , ( int ) msg . size ()
                      , msg . addr ()
            );
    }
    
    void PTracerImpl :: write ( const TraceQueueId & qid, Z32 lvl,
        const char * file, const char * func, N32 line, const CText & msg ) const noexcept
    {
        const CText & qn = qid -> getName () . data ();
        
        const char * qname = qn . addr ();
        size_t qsize = qn . size ();
        
        queue_trace ( lvl, file, line, qname, qsize, func, "%.*s"
                      , ( int ) msg . size ()
                      , msg . addr ()
            );
    }

    void PTracerImpl :: flush () const
    {
    }

    PTracerImpl :: PTracerImpl ()
        : qids ( 0 )
    {
        KTRACE ( TRACE_PRG, "acquiring primordial Tracer latch" );
        if ( latch . test_and_set () )
        {
            throw PermissionViolation (
                XP ( XLOC, rc_logic_err )
                << "primordial tracer is already initialized"
                );
        }

        KTRACE ( TRACE_PRG, "constructing primordial Tracer @ 0x%zx", ( size_t ) this );
    }
    
    PTracerImpl :: ~ PTracerImpl () noexcept
    {
        KTRACE ( TRACE_PRG, "destroying primordial Tracer @ 0x%zx", ( size_t ) this );

        KTRACE ( TRACE_PRG, "clearing primordial Tracer latch" );
        latch . clear ();
    }
}
