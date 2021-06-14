/*

  vdb3.kfc.memset_s-tst


 */

#include "memset_s.cpp"
#include "ktrace.cpp"

#include <gtest/gtest.h>

namespace vdb3
{

    /*=================================================*
     *                    memset_s()                   *
     *=================================================*/

    TEST ( memset_s_test, catch_huge_destsz )
    {
        char buffer [ 8 ];
        EXPECT_ANY_THROW ( memset_s ( buffer, -1, 0, 0 ) );
    }

    TEST ( memset_s_test, catch_nullptr )
    {
        EXPECT_ANY_THROW ( memset_s ( nullptr, 1, 0, 0 ) );
    }

    TEST ( memset_s_test, expect_nullptr_empty )
    {
        EXPECT_NO_THROW ( memset_s ( nullptr, 0, 0, 0 ) );
    }

    TEST ( memset_s_test, catch_huge_count )
    {
        char buffer [ 8 ];
        EXPECT_ANY_THROW ( memset_s ( buffer, sizeof buffer, 0, -1 ) );
    }

    TEST ( memset_s_test, fill_none )
    {
        byte_t mem [ 16 ];
        EXPECT_NO_THROW ( memset_s ( mem, sizeof mem, 'a', 0 ) );
    }

    TEST ( memset_s_test, fill_partial )
    {
        byte_t mem [ 16 ];
        EXPECT_NO_THROW ( memset_s ( mem, sizeof mem, 'a', 10 ) );
    }

    TEST ( memset_s_test, fill_whole_buffer )
    {
        byte_t mem [ 16 ];
        EXPECT_NO_THROW ( memset_s ( mem, sizeof mem, 'a', sizeof mem ) );
    }

    TEST ( memset_s_test, overflow_buffer )
    {
        byte_t mem [ 16 ];
        EXPECT_ANY_THROW ( memset_s ( mem, sizeof mem, 'a', sizeof mem + 1 ) );
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
