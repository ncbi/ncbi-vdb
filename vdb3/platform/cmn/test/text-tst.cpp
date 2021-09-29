/*

  vdb3.cmn.text-tst

 */

#include "text-utf8.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    TEST ( TextUTF8Test, default_rgn_is_null_and_empty )
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
