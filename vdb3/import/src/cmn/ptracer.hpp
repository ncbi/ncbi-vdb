/*

  vdb3.cmn.ptracer


 */

#pragma once

#include <vdb3/kfc/ref.hpp>
#include <vdb3/cmn/defs.hpp>
#include <vdb3/cmn/tracer.hpp>

#include <map>

/**
 * @file cmn/ptracer.hpp
 * @brief primordial trace manager
 */

namespace vdb3
{
    
    /*=====================================================*
     *                     PTracerImpl                     *
     *=====================================================*/

    struct PTracerImpl
        : Refcount
        , TracerItf
        , TraceQueueFactory
    {
        METH ( TraceQueueId getQueueId ( const String & name ) const );
        METH ( TraceQueueId makeQueue ( const String & name ) );
        METH ( void write ( Z32 lvl,
            const char * file, const char * func, N32 line,
            const CText & msg ) const noexcept );
        METH ( void write ( const TraceQueueId & qid, Z32 lvl,
            const char * file, const char * func, N32 line,
            const CText & msg ) const noexcept );
        METH ( void flush () const );

        PTracerImpl ();
        virtual ~ PTracerImpl () noexcept;

        static std :: atomic_flag latch;
        
        // TBD - these must be protected by r/w lock
        std :: map < String, TraceQueueId > qmap;
        N32 qids;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
