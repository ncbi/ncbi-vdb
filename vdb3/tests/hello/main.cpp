#include <iostream>

#include <vdb3/cmn/rsrc.hpp>
#include <vdb3/kfc/ktrace.hpp>

using namespace vdb3;

int main ( int argc, const char * argv [], const char * envp []  )
{
    try
    {
        CAPTURE_TOOL ( argv [ 0 ] );
        CAPTURE_EARLY_TRACE_LEVEL ( argc, argv ); // use -vvvvv to get the most detailed trace

        Rsrc rsrcMgr;
        // TimeMgr & time ( rsrcMgr . time );
        // MemMgr & mem ( rsrcMgr . mmgr );
        // TraceMgr & trace ( rsrcMgr . trace );
        // LogMgr & log ( rsrcMgr . log );

        //std :: cout << "Trace threshold = " << rsrcMgr . trace.getThreshold () << std::endl;

        CTextLiteral hello( "Hello VDB3!" );
        rsrcMgr . log . setThreshold( ::LOG_NOTICE );
        // goes to stderr via the default (PLoggerImpl) logger
        rsrcMgr . log . msg( rsrcMgr, rsrcMgr . logger, ::LOG_NOTICE, hello . addr() );
    }

    //TODO: get cout from RsrcMgr?
    catch ( const Exception & ex )
    {
        std :: cout << "VDB3 Exception: " << ex.what() << std::endl;
        std :: cout << ex.cause() << std::endl;
        std :: cout << ex.context() << std::endl;
        std :: cout << ex.file() << std::endl;
        std :: cout << ex.function() << std::endl;
        std :: cout << ex.line() << std::endl;
        std :: cout << ex.problem() << std::endl;
        std :: cout << ex.status() << std::endl;
        std :: cout << ex.suggestion() << std::endl;
        return 3;
    }
    catch ( std :: exception & ex )
    {
        std :: cout << "oops of a standard kind..." << ex.what() << std::endl;
        return 2;
    }
    catch ( ... )
    {
        std :: cout << "oops of an unknown kind..." << std::endl;
        return 1;
    }

    return 0;
}