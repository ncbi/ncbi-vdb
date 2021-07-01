/*

  vdb3.cmn.plogger


 */

#pragma once

#include <vdb3/cmn/logger.hpp>

#include <map>

namespace vdb3
{
    
    /*=====================================================*
     *                     PLoggerImpl                     *
     *=====================================================*/

    /**
     * PLoggerImpl
     *  @brief simply logs to stderr or std::cerr
     */
    struct PLoggerImpl
        : Refcount
        , LoggerItf
        , LogQueueFactory
    {
        METH ( String getHostname () const noexcept );
        METH ( String getProcname () const noexcept );
        METH ( LogQueueId getQueueId ( const String & name ) const );
        METH ( LogQueueId makeQueue ( const String & name ) );
        METH ( void write ( LogLevel lvl, pid_t pid,
            const Timestamp & ts, const CText & msg ) const );
        METH ( void write ( const LogQueueId & qid, LogLevel lvl,
            pid_t pid, const Timestamp & ts, const CText & msg ) const );
        METH ( void flush () const ) {}

        PLoggerImpl ( const String & hostname, const String & procname );
        virtual ~ PLoggerImpl () noexcept;

        static std :: atomic_flag latch;
        
        String hostname;
        String procname;

        // TBD - these must be protected by r/w lock
        std :: map < String, LogQueueId > qmap;
        N32 qids;
    };
}
