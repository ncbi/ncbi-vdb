/*

  vdb3.kfc.trace-tst

 */

#include "ktrace.cpp"

#include <google/gtest.h>

namespace vdb3
{
    TEST ( TRACETest, bling )
    {
    }
}

extern "C"
{
    int main ( int argc, const char * argv [], const char * envp []  )
    {
        CAPTURE_TOOL ( argv [ 0 ] );
        CAPTURE_EARLY_TRACE_LEVEL ( argc, argv );
        testing :: InitGoogleTest ( & argc, ( char ** ) argv );
        return RUN_ALL_TESTS ();
    }
}
