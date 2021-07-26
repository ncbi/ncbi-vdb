/*

  vdb3.kfc.rsrc-mem-tst

 */

#include "rsrc-mem.cpp"

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
#include "tmmgr.cpp"
#if LINUX
#include "linux-tmmgr.cpp"
#elif MAC
#include "mac-tmmgr.cpp"
#endif
#include "rsrc-tm.cpp"
#include "rgn.cpp"
#include "rgn-lock.cpp"
#include "mem.cpp"
#include "mmgr.cpp"
#include "busy.cpp"
#include "pmmgr.cpp"
#include "pmem.cpp"
#include "pmem-rgn-lock.cpp"

#include <gtest/gtest.h>

namespace vdb3
{

    TEST ( RsrcMemTest, constructor_destructor )
    {
        RsrcMem rsrc;
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
