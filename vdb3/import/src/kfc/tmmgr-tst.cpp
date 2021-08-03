/*

  vdb3.kfc.tmmgr-tst

 */

#include "tmmgr.cpp"
#if LINUX
#include "linux-tmmgr.cpp"
#elif MAC
#include "mac-tmmgr.cpp"
#endif

#include "ktrace.cpp"
#include "memset_s.cpp"
#include "ascii.cpp"
#include "z128.cpp"
#include "fmt-base.cpp"
#include "xp.cpp"
#include "except.cpp"
#include "xbt.cpp"
#include "time.cpp"
#include "ref.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    
    TEST ( TimeMgrTest, constructor_destructor )
    {
        TimeMgr tmmgr;
    }
    
    TEST ( TimeMgrTest, now )
    {
        TimeMgr tmmgr = makeTimeMgr ();
        Timestamp tm = tmmgr . now ();
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
