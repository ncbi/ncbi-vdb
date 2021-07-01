/*

  vdb3.cmn.trace


 */

#include <vdb3/cmn/trace.hpp>
#include <vdb3/kfc/ktrace.hpp>

namespace vdb3
{

    
    /*=====================================================*
     *                       TraceMgr                      *
     *=====================================================*/

    /**
     * getThreshold
     *  @return threshold for issuing trace messages to queue
     */
    Z32 TraceMgr :: getThreshold ( const TraceQueueId & qid ) const noexcept
    {
        return tm -> getThreshold ( qid );
    }


    /* setThreshold
     *  @brief set threshold for issuing trace messages
     */
    void TraceMgr :: setThreshold ( Z32 level ) const
    {
        tm -> setThreshold ( level );
        lvl = level;
    }


    /* setThreshold
     *  @brief set threshold for issuing trace messages to queue
     */
    void TraceMgr :: setThreshold ( const TraceQueueId & qid, Z32 level ) const
    {
        tm -> setThreshold ( qid, level );
    }

    TraceMgrRef TraceMgr :: clone () const
    {
        return tm -> clone ();
    }

    TraceMgr & TraceMgr :: operator = ( TraceMgr && mgr ) noexcept
    {
        tm = std :: move ( mgr . tm );
        lvl = mgr . lvl;
        return self;
    }
    
    TraceMgr & TraceMgr :: operator = ( const TraceMgr & mgr ) noexcept
    {
        tm = mgr . tm;
        lvl = mgr . lvl;
        return self;
    }
    
    TraceMgr :: TraceMgr ( TraceMgr && mgr ) noexcept
        : tm ( std :: move ( mgr . tm ) )
        , lvl ( mgr . lvl )
    {
    }
    
    TraceMgr :: TraceMgr ( const TraceMgr & mgr ) noexcept
        : tm ( mgr . tm )
        , lvl ( mgr . lvl )
    {
    }

    TraceMgr :: TraceMgr () noexcept
        : lvl ( -1 )
    {
    }
    
    TraceMgr :: ~ TraceMgr () noexcept
    {
        lvl = -1;
    }

    TraceMgr & TraceMgr :: operator = ( const TraceMgrRef & _tm ) noexcept
    {
        tm = _tm;
        lvl = _tm -> getThreshold ();
        return self;
    }
    
    TraceMgr :: TraceMgr ( const TraceMgrRef & _tm ) noexcept
        : tm ( _tm )
        , lvl ( _tm -> getThreshold () )
    {
    }

}
