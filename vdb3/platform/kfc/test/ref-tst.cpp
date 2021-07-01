/*

  vdb3.kfc.ref-tst

 */

#include "ref.cpp"

#include "ktrace.cpp"
#include "memset_s.cpp"
#include "ascii.cpp"
#include "z128.cpp"
#include "fmt-base.cpp"
#include "xp.cpp"
#include "except.cpp"
#include "xbt.cpp"

#include <gtest/gtest.h>

namespace vdb3
{
    struct O
    {
        int x;
    };

    struct R : Refcount
    {
        int x;
    };

    TEST ( XRefTest, constructor_destructor )
    {
        XRef < O > ref;
        EXPECT_TRUE ( ! ref );
    }

    TEST ( SRefTest, constructor_destructor )
    {
        SRef < O > ref;
        EXPECT_TRUE ( ! ref );
    }

    TEST ( IRefTest, constructor_destructor )
    {
        IRef < R > ref;
        EXPECT_TRUE ( ! ref );
    }

    TEST ( IRefTest, assign_obj_ptr )
    {
        IRef < R > ref;
        EXPECT_TRUE ( ! ref );

        ref = new R;
        EXPECT_FALSE ( ! ref );
        EXPECT_EQ ( ref . use_count (), 1 );
    }

    TEST ( IRefTest, access_obj_ptr )
    {
        IRef < R > ref;
        EXPECT_TRUE ( ! ref );

        ref = new R;
        EXPECT_FALSE ( ! ref );
        EXPECT_NO_THROW ( ref -> x = 10 );
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
