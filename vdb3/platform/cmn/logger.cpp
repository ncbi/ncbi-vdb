/*

  vdb3.cmn.logger


 */

#include <vdb3/cmn/logger.hpp>

namespace vdb3
{

    /*=====================================================*
     *                       LogQueue                      *
     *=====================================================*/

    LogQueue :: LogQueue ( const String & _nm, N32 _id ) noexcept
        : nm ( _nm )
        , id ( _id )
    {
    }
    
    LogQueue :: ~ LogQueue ()
    {
    }

    
    /*=====================================================*
     *                        Logger                       *
     *=====================================================*/

    LogQueueId Logger :: getQueueId ( const String & name ) const
    {
        // TBD - may maintain a cache here
        return logger -> getQueueId ( name );
    }


    LogQueueId Logger :: makeQueue ( const String & name ) const
    {
        // TBD - may maintain a cache here
        return logger -> makeQueue ( name );
    }


    Logger & Logger :: operator = ( Logger && l ) noexcept
    {
        logger = std :: move ( l . logger );
        return self;
    }
    
    Logger & Logger :: operator = ( const Logger & l ) noexcept
    {
        logger = l . logger;
        return self;
    }
    
    Logger :: Logger ( Logger && l ) noexcept
        : logger ( std :: move ( l . logger ) )
    {
    }
    
    Logger :: Logger ( const Logger & l ) noexcept
        : logger ( l . logger )
    {
    }

    Logger :: Logger () noexcept
    {
    }
    
    Logger :: ~ Logger () noexcept
    {
    }

    Logger & Logger :: operator = ( const LoggerRef & l ) noexcept
    {
        logger = l;
        return self;
    }
    
    Logger :: Logger ( const LoggerRef & l ) noexcept
        : logger ( l )
    {
    }

}
