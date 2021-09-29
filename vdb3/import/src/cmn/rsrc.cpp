/*

  vdb3.cmn.rsrc

 */

#include <vdb3/cmn/rsrc.hpp>
#include <vdb3/cmn/trace.hpp>

#if MAX_RSRC_MGR >= RSRC_MGR_MMGR
#include "pmmgr.hpp"
#endif

#if MAX_RSRC_MGR >= RSRC_MGR_LOG
#include <unistd.h>
#include "plog.hpp"
#endif

namespace cmn
{
    std :: atomic_flag Rsrc :: latch = ATOMIC_FLAG_INIT;
        
    Rsrc & Rsrc :: operator = ( Rsrc && rsrc ) noexcept
    {
#if MAX_RSRC_MGR >= RSRC_MGR_MMGR
        mmgr = std :: move ( rsrc . mmgr );
#endif
        return self;
    }
    
    Rsrc & Rsrc :: operator = ( const Rsrc & rsrc ) noexcept
    {
#if MAX_RSRC_MGR >= RSRC_MGR_MMGR
        mmgr = rsrc . mmgr;
#endif
        return self;
    }
    
    Rsrc :: Rsrc ( Rsrc && rsrc ) noexcept
#if MAX_RSRC_MGR >= RSRC_MGR_MMGR
        : mmgr ( std :: move ( rsrc . mmgr ) )
#endif
    {
    }
    
    Rsrc :: Rsrc ( const Rsrc & rsrc ) noexcept
#if MAX_RSRC_MGR >= RSRC_MGR_MMGR
        : mmgr ( rsrc . mmgr )
#endif
    {
    }
    
    Rsrc :: Rsrc ( const char * proc_path )
    {
#if ! TESTING
        if ( latch . test_and_set () )
        {
            throw PermissionViolation (
                XP ( XLOC, rc_logic_err )
                << "global resource manager block is already initialized"
                );
        }
#endif
        
#if MAX_RSRC_MGR >= RSRC_MGR_MMGR
        TRACE ( TRACE_PRG, "creating primordial memory manager" );
        MemMgrRef mmgrr ( new PMemMgrImpl () );
        mmgr = mmgrr;
        TRACE ( TRACE_GEEK, "created primordial memory manager" );
#endif
        
#if MAX_RSRC_MGR >= RSRC_MGR_LOG
        TRACE ( TRACE_PRG, "creating primordial log facility" );
        CText proc_path_txt = CTextLiteral ( proc_path );
        CharLoc slash = proc_path_txt . revFind ( '/' );
        if ( proc_path_txt . contains ( slash ) )
        {
            slash = proc_path_txt . nextLoc ( slash );
            proc_path_txt = proc_path_txt . subRgn ( slash );
        }
        String procname ( mmgr, proc_path_txt );
        LogMgrRef logr ( new PLogMgrImpl ( mmgr, procname, :: getpid ()) );
        log = logr;
        TRACE ( TRACE_GEEK, "created primordial log facility" );
#endif
    }
    
    Rsrc :: ~ Rsrc () noexcept
    {
        TRACE ( TRACE_PRG, "releasing all managers" );
    }
}
