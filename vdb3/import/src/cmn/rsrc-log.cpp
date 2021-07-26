/*

  vdb3.cmn.rsrc-log

 */

#include <vdb3/cmn/rsrc-log.hpp>
#include <vdb3/cmn/trace.hpp>

#include <unistd.h>
#include "plogger.hpp"
#include "log-impl.hpp"

namespace vdb3
{
    static
    String get_hostname ( const RsrcTrace & rsrc )
    {
        // use a local buffer
        char buffer [ 256 ];

        // hostname
        int status = :: gethostname ( buffer, sizeof buffer );
        if ( status != 0 )
        {
            switch ( errno )
            {
            case ENAMETOOLONG:
                :: strncpy ( buffer, "<HOSTNAME-TOO-LONG>", sizeof buffer );
                break;
            default:
                :: strncpy ( buffer, "<GETHOSTNAME-FAILED>", sizeof buffer );
            }
        }

        return String ( rsrc, buffer );
    }
    
    bool RsrcLog :: operator == ( const RsrcLog & rsrc ) const noexcept
    {
        return RsrcTrace :: operator == ( rsrc )
            && logger == rsrc . logger
            && log == rsrc . log
            ;
    }
    
    RsrcLog & RsrcLog :: operator = ( RsrcLog && rsrc ) noexcept
    {
        RsrcTrace :: operator = ( std :: move ( rsrc ) );
        logger = std :: move ( rsrc . logger );
        log = std :: move ( rsrc . log );
        return self;
    }
    
    RsrcLog & RsrcLog :: operator = ( const RsrcLog & rsrc ) noexcept
    {
        RsrcTrace :: operator = ( rsrc );
        logger = rsrc . logger;
        log = rsrc . log;
        return self;
    }
    
    RsrcLog :: RsrcLog ( RsrcLog && rsrc ) noexcept
        : RsrcTrace ( std :: move ( rsrc ) )
        , logger ( std :: move ( rsrc . logger ) )
        , log ( std :: move ( rsrc . log ) )
    {
    }
    
    RsrcLog :: RsrcLog ( const RsrcLog & rsrc ) noexcept
        : RsrcTrace ( rsrc )
        , logger ( rsrc . logger )
        , log ( rsrc . log )
    {
    }
    
    RsrcLog :: RsrcLog ()
    {
        KTRACE ( TRACE_PRG, "creating primordial log facility" );
        String host = get_hostname ( self );
        String proc ( self, tool_name );
        LoggerRef l = new PLoggerImpl ( host, proc );
        LogMgrRef lr ( new LogMgrImpl );
        logger = l;
        log = lr;
        KTRACE ( TRACE_GEEK, "created primordial log facility" );
    }
    
    RsrcLog :: ~ RsrcLog () noexcept
    {
        KTRACE ( TRACE_PRG, "releasing log manager" );
    }
}
