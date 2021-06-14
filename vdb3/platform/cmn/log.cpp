/*

  vdb3.cmn.log


 */

#include <vdb3/cmn/log.hpp>

namespace vdb3
{

    
    /*=====================================================*
     *                        LogMgr                       *
     *=====================================================*/

    /**
     * getThreshold
     *  @return threshold for issuing log messages to queue
     */
    LogLevel LogMgr :: getThreshold ( const LogQueueId & qid ) const noexcept
    {
        return log -> getThreshold ( qid );
    }


    /* setThreshold
     *  @brief set threshold for issuing log messages
     */
    void LogMgr :: setThreshold ( LogLevel _lvl ) const
    {
        log -> setThreshold ( _lvl );

        // cache
        lvl = log -> getThreshold ();
    }


    /* setThreshold
     *  @brief set threshold for issuing log messages to queue
     */
    void LogMgr :: setThreshold ( const LogQueueId & qid, LogLevel _lvl ) const
    {
        log -> setThreshold ( qid, _lvl );
    }

    LogMgrRef LogMgr :: clone () const
    {
        return log -> clone ();
    }

    LogMgr & LogMgr :: operator = ( LogMgr && mgr ) noexcept
    {
        log = std :: move ( mgr . log );
        lvl = mgr . lvl;
        return self;
    }
    
    LogMgr & LogMgr :: operator = ( const LogMgr & mgr ) noexcept
    {
        log = mgr . log;
        lvl = mgr . lvl;
        return self;
    }
    
    LogMgr :: LogMgr ( LogMgr && mgr ) noexcept
        : log ( std :: move ( mgr . log ) )
        , lvl ( mgr . lvl )
    {
    }
    
    LogMgr :: LogMgr ( const LogMgr & mgr ) noexcept
        : log ( mgr . log )
        , lvl ( mgr . lvl )
    {
    }

    LogMgr :: LogMgr () noexcept
        : lvl ( LOG_INVALID )
    {
    }
    
    LogMgr :: ~ LogMgr () noexcept
    {
        lvl = LOG_INVALID;
    }

    LogMgr & LogMgr :: operator = ( const LogMgrRef & _log ) noexcept
    {
        log = _log;
        lvl = _log -> getThreshold ();
        return self;
    }
    
    LogMgr :: LogMgr ( const LogMgrRef & _log ) noexcept
        : log ( _log )
        , lvl ( _log -> getThreshold () )
    {
    }

}
