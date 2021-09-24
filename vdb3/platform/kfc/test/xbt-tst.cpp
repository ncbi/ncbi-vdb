/*

  vdb3.kfc.xbt-tst


 */

#include "xbt.cpp"

#include "ktrace.cpp"
#include "memset_s.cpp"
#include "ascii.cpp"
#include "z128.cpp"
#include "fmt-base.cpp"
#include "xp.cpp"
#include "except.cpp"

#include <gtest/gtest.h>

#include <string>
#include <cstring>

namespace vdb3
{
    static void func1 ()
    {
        throw NullReferenceException (
            XP ( XLOC )
            << "it's not really NULL"
            );
    }

    static void func2 ()
    {
        func1 ();
    }

    TEST ( XBackTraceTest, simple_backtrace )
    {
        try
        {
            func2 ();
        }
        catch ( Exception & x )
        {
            std :: cerr
                << "\n\n"
                << "CAUGHT EXCEPTION:\n\n"
                << XBackTrace ( x )
                << "\n\n"
                ;
        }
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
