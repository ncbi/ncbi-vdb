/*

  vdb3.cmn.log-impl


 */

#pragma once

#include <vdb3/cmn/log.hpp>
#include <vdb3/cmn/logger.hpp>
#include <vdb3/cmn/buffmt.hpp>
#include <vdb3/cmn/writer.hpp>

#include <vector>

namespace vdb3
{
    
    /*=====================================================*
     *                    LogWriterImpl                    *
     *=====================================================*/
        
    struct LogWriterImpl
        : Refcount
        , TextStreamWriterItf
    {
        METH ( CText write ( const CText & msg ) );
        METH ( void flush () );

        LogWriterImpl ( const RsrcTime & rsrc, const Logger & l, LogLevel lvl, pid_t pid ) noexcept;
        LogWriterImpl ( const RsrcTime & rsrc, const Logger & l, const LogQueueId & qid, LogLevel lvl, pid_t pid ) noexcept;
        virtual ~ LogWriterImpl () noexcept;

        // this is where we need to transfer data
        Logger logger;

        // an optional LogQueue id
        LogQueueId qid;

        // event timestamp
        Timestamp ts;

        // the priority
        LogLevel lvl;

        // the process id
        pid_t pid;
    };
    
    /*=====================================================*
     *                      LogMgrImpl                     *
     *=====================================================*/

    class LogMgrImpl
        : public Refcount
        , public LogMgrItf
    {
    public:
        
        METH ( LogLevel getThreshold () const noexcept );
        METH ( LogLevel getThreshold ( const LogQueueId & qid ) const noexcept );
        METH ( void setThreshold ( LogLevel lvl ) );
        METH ( void setThreshold ( const LogQueueId & qid, LogLevel lvl ) );
        METH ( TextStreamWriterRef msg ( const RsrcKfc & rsrc, const Logger & logger, LogLevel priority ) );
        METH ( TextStreamWriterRef msg ( const RsrcKfc & rsrc, const Logger & logger, const LogQueueId & qid, LogLevel priority ) );
        METH ( LogMgrRef clone () const );

        LogMgrImpl ();
        LogMgrImpl ( const LogMgrImpl & mgr );
        virtual ~ LogMgrImpl () noexcept;

    private:

        // TBD - protect with r/w lock
        std :: vector < LogLevel > qlvl;
        LogLevel lvl;
        pid_t pid;
    };
}
