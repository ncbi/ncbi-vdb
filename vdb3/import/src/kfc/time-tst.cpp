/*

  vdb3.kfc.time-tst

 */

#include "time.cpp"

#include "ktrace.cpp"
#include "memset_s.cpp"
#include "ascii.cpp"
#include "z128.cpp"
#include "fmt-base.cpp"
#include "xp.cpp"
#include "except.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    TEST ( TimeTest, constructor_destructor_nS )
    {
        NanoSeconds nS_1 = 10;
        NanoSeconds nS_2 = 10.5;
        EXPECT_EQ ( ( Z64 ) nS_1, 10LL );
        EXPECT_EQ ( ( Z64 ) nS_2, 10LL );
    }
    
    TEST ( TimeTest, constructor_destructor_uS )
    {
        MicroSeconds uS_1 = 10;
        MicroSeconds uS_2 = 10.5;
        EXPECT_EQ ( ( Z64 ) uS_1, 10LL );
        EXPECT_EQ ( ( Z64 ) uS_2, 11LL );
    }
    
    TEST ( TimeTest, constructor_destructor_mS )
    {
        MilliSeconds mS_1 = 10;
        MilliSeconds mS_2 = 10.25;
        MicroSeconds uS_1 = mS_2;
        EXPECT_EQ ( ( Z64 ) uS_1, 10250LL );
        EXPECT_EQ ( ( Z64 ) mS_1, 10LL );
        EXPECT_EQ ( ( Z64 ) mS_2, 10LL );
    }
    
    TEST ( TimeTest, constructor_destructor_S )
    {
        Seconds S_1 = 10;
        Seconds S_2 = 10.25;
        MicroSeconds uS_1 = S_2;
        EXPECT_EQ ( ( Z64 ) uS_1, 10250000LL );
        EXPECT_EQ ( ( Z64 ) S_1, 10LL );
        EXPECT_EQ ( ( Z64 ) S_2, 10LL );
    }
    
    TEST ( TimeTest, arithmetic_1 )
    {
        Seconds S_1 = 10;
        Seconds S_2 = 10.25;
        MicroSeconds uS_1 = S_1 + S_2;
        EXPECT_EQ ( ( Z64 ) uS_1, 20250000LL );
        S_1 += 2;
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
