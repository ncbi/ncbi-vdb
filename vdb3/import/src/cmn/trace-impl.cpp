/*

  vdb3.cmn.trace-impl


 */

#include "trace-impl.hpp"

namespace vdb3
{

    /*=====================================================*
     *                   TraceWriterImpl                   *
     *=====================================================*/
    
    CText TraceWriterImpl :: write ( const CText & msg )
    {
        if ( ! qid )
            tracer . write ( lvl, file, func, line, msg );
        else
            tracer . write ( qid, lvl, file, func, line, msg );
        
        return msg;
    }
    
    void TraceWriterImpl :: flush ()
    {
        tracer . flush ();
    }
        
    TraceWriterImpl :: TraceWriterImpl ( const Tracer & t, Z32 _lvl,
            const char * _file, const char * _func, N32 _line ) noexcept
        : tracer ( t )
        , file ( _file )
        , func ( _func )
        , line ( _line )
        , lvl ( _lvl )
    {
    }
        
    TraceWriterImpl :: TraceWriterImpl ( const Tracer & t, const TraceQueueId & _qid, Z32 _lvl,
            const char * _file, const char * _func, N32 _line ) noexcept
        : tracer ( t )
        , qid ( _qid )
        , file ( _file )
        , func ( _func )
        , line ( _line )
        , lvl ( _lvl )
    {
    }
    
    TraceWriterImpl :: ~ TraceWriterImpl () noexcept
    {
        file = func = "";
        line = 0;
        lvl = 0;
    }
    
    /*=====================================================*
     *                     TraceMgrImpl                    *
     *=====================================================*/
    
    Z32 TraceMgrImpl :: getThreshold () const noexcept
    {
        // TBD - read lock
        return lvl;
    }
    
    Z32 TraceMgrImpl :: getThreshold ( const TraceQueueId & qid ) const noexcept
    {
        // TBD - read lock
        N32 id = qid -> getId ();
        if ( id != 0 && ( size_t ) id <= qlvl . size () )
            return qlvl [ id - 1 ];
        return -1;
    }
    
    void TraceMgrImpl :: setThreshold ( Z32 _lvl )
    {
        // TBD - write lock
        if ( _lvl >= 0 )
            lvl = _lvl;
    }
    
    void TraceMgrImpl :: setThreshold ( const TraceQueueId & qid, Z32 _lvl )
    {
        // TBD - write lock
        if ( _lvl >= 0 )
        {
            // make a log-level entry
            N32 id = qid -> getId ();

            // extend our table as required
            if ( ( size_t ) id > qlvl . size () )
            {
                qlvl . reserve ( id );
                for ( N32 i = ( N32 ) qlvl . size (); i < id; ++ i )
                    qlvl . push_back ( -1 );
            }

            // update existing entry
            assert ( id > 0 );
            qlvl [ id - 1 ] = _lvl;
        }
    }
    
    TextStreamWriterRef TraceMgrImpl :: msg ( const Tracer & tracer, Z32 level,
        const char * file, const char * func, N32 line ) noexcept
    {
        return new TraceWriterImpl ( tracer, level, file, func, line );
    }
    
    TextStreamWriterRef TraceMgrImpl :: msg ( const Tracer & tracer,
        const TraceQueueId & qid, Z32 level,
        const char * file, const char * func, N32 line ) noexcept
    {
        return new TraceWriterImpl ( tracer, qid, level, file, func, line );
    }

    TraceMgrRef TraceMgrImpl :: clone () const
    {
        return new TraceMgrImpl ( self );
    }

    TraceMgrImpl :: TraceMgrImpl ()
        : lvl ( dbg_trace_level )
    {
        KTRACE ( TRACE_PRG, "constructing TraceMgr @ 0x%zx", ( size_t ) this );
    }

    TraceMgrImpl :: TraceMgrImpl ( const TraceMgrImpl & mgr )
        : lvl ( mgr . lvl )
    {
        KTRACE ( TRACE_PRG, "constructing TraceMgr @ 0x%zx", ( size_t ) this );
        for ( Z32 l : mgr . qlvl )
            qlvl . push_back ( l );
    }
    
    TraceMgrImpl :: ~ TraceMgrImpl () noexcept
    {
        KTRACE ( TRACE_PRG, "destroying TraceMgr @ 0x%zx", ( size_t ) this );
    }
}
