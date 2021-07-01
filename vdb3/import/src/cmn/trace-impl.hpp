/*

  vdb3.cmn.trace-impl


 */

#pragma once

#include <vdb3/cmn/trace.hpp>
#include <vdb3/cmn/tracer.hpp>
#include <vdb3/cmn/buffmt.hpp>

#include <atomic>
#include <vector>

/**
 * @file cmn/ptrace.hpp
 * @brief primordial trace manager
 */

namespace vdb3
{
    
    /*=====================================================*
     *                   TraceWriterImpl                   *
     *=====================================================*/
        
    struct TraceWriterImpl
        : Refcount
        , TextStreamWriterItf
    {
        METH ( CText write ( const CText & msg ) );
        METH ( void flush () );
        
        TraceWriterImpl ( const Tracer & t, Z32 lvl,
            const char * file, const char * func, N32 line ) noexcept;
        TraceWriterImpl ( const Tracer & t, const TraceQueueId & qid, Z32 lvl,
            const char * file, const char * func, N32 line ) noexcept;
        virtual ~ TraceWriterImpl () noexcept;

        // this is where we need to transfer data
        Tracer tracer;

        // an optional TraceQueue id
        TraceQueueId qid;

        // file and func
        const char * file;
        const char * func;

        // line number
        N32 line;

        // the priority
        Z32 lvl;
    };
    
    /*=====================================================*
     *                     TraceMgrImpl                    *
     *=====================================================*/

    struct TraceMgrImpl
        : Refcount
        , TraceMgrItf
    {
        METH ( Z32 getThreshold () const noexcept );
        METH ( Z32 getThreshold ( const TraceQueueId & qid ) const noexcept );
        METH ( void setThreshold ( Z32 level ) );
        METH ( void setThreshold ( const TraceQueueId & qid, Z32 level ) );
        METH ( TextStreamWriterRef msg ( const Tracer & tracer, Z32 level,
            const char * file, const char * func, N32 line ) noexcept );
        METH ( TextStreamWriterRef msg ( const Tracer & tracer,
            const TraceQueueId & qid, Z32 level,
            const char * file, const char * func, N32 line ) noexcept );
        METH ( TraceMgrRef clone () const );

        TraceMgrImpl ();
        TraceMgrImpl ( const TraceMgrImpl & mgr );
        virtual ~ TraceMgrImpl () noexcept;
        

    private:
        
        // TBD - these must be protected by r/w lock
        std :: vector < Z32 > qlvl;
        Z32 lvl;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
