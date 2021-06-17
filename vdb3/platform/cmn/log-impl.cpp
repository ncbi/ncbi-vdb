/*

  vdb3.cmn.plog


 */

#include <vdb3/cmn/log-impl.hpp>
#include <vdb3/cmn/plogger.hpp>

#include <unistd.h>

namespace vdb3
{

    static const LogLevel LOG_DFLT = LOG_ERR;

    /*=====================================================*
     *                    LogWriterImpl                    *
     *=====================================================*/

    CText LogWriterImpl :: write ( const CText & msg )
    {
        if ( ! qid )
            logger . write ( lvl, pid, ts, msg );
        else
            logger . write ( qid, lvl, pid, ts, msg );

        return msg;
    }

    void LogWriterImpl :: flush ()
    {
        logger . flush ();
    }

    LogWriterImpl :: LogWriterImpl ( const RsrcTime & rsrc,
            const Logger & l, LogLevel _lvl, pid_t _pid ) noexcept
        : logger ( l )
        , ts ( rsrc . time . now () )
        , lvl ( _lvl )
        , pid ( _pid )
    {
    }

    LogWriterImpl :: LogWriterImpl ( const RsrcTime & rsrc,
            const Logger & l, const LogQueueId & _qid, LogLevel _lvl, pid_t _pid ) noexcept
        : logger ( l )
        , qid ( _qid )
        , ts ( rsrc . time . now () )
        , lvl ( _lvl )
        , pid ( _pid )
    {
    }

    LogWriterImpl :: ~ LogWriterImpl () noexcept
    {
    }

    /*=====================================================*
     *                     LogMgrImpl                      *
     *=====================================================*/

    LogLevel LogMgrImpl :: getThreshold () const noexcept
    {
        return lvl;
    }

    LogLevel LogMgrImpl :: getThreshold ( const LogQueueId & qid ) const noexcept
    {
        N32 id = qid -> getId ();
        if ( id != 0 && ( size_t ) id <= qlvl . size () )
            return qlvl [ id - 1 ];
        return LOG_INVALID;
    }

    void LogMgrImpl :: setThreshold ( LogLevel _lvl )
    {
        if ( _lvl >= LOG_EMERG && _lvl <= LOG_DEBUG )
            lvl = _lvl;
    }

    void LogMgrImpl :: setThreshold ( const LogQueueId & qid, LogLevel _lvl )
    {
        if ( _lvl >= LOG_EMERG && _lvl <= LOG_DEBUG )
        {
            // make a log-level entry
            N32 id = qid -> getId ();

            // extend our table as required
            if ( ( size_t ) id > qlvl . size () )
            {
                qlvl . reserve ( id );
                for ( N32 i = ( N32 ) qlvl . size (); i < id; ++ i )
                    qlvl . push_back ( LOG_INVALID );
            }

            // update existing entry
            assert ( id > 0 );
            qlvl [ id - 1 ] = _lvl;
        }
    }

    TextStreamWriterRef LogMgrImpl :: msg ( const RsrcKfc & rsrc,
        const Logger & logger, LogLevel priority )
    {
        return new LogWriterImpl ( rsrc, logger, priority, pid );
    }

    TextStreamWriterRef LogMgrImpl :: msg ( const RsrcKfc & rsrc,
        const Logger & logger, const LogQueueId & qid, LogLevel priority )
    {
        return new LogWriterImpl ( rsrc, logger, qid, priority, pid );
    }

    LogMgrRef LogMgrImpl :: clone () const
    {
        return new LogMgrImpl ( self );
    }

    LogMgrImpl :: LogMgrImpl ()
        : lvl ( LOG_DFLT )
        , pid ( getpid () )
    {
    }

    LogMgrImpl :: LogMgrImpl ( const LogMgrImpl & mgr )
        : lvl ( mgr . lvl )
        , pid ( mgr . pid )
    {
        for ( LogLevel l : mgr . qlvl )
            qlvl . push_back ( l );
    }

    LogMgrImpl :: ~ LogMgrImpl () noexcept
    {
    }

}
