/*

  vdb3.cmn.nconv-sel-tst

 */

#include <vdb3/kfc/rsrc-mem.hpp>

#define NCONV_SEL_TEST 1
#include "nconv.cpp"

#include "buffer.cpp"
#include "text-utf8.cpp"
#include "serial.cpp"
#include "dserial.cpp"
#include "string.cpp"
#include "str-iter.cpp"
#include "str-buffer.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    RsrcMem rsrc;

    TEST ( NumberConversionTest, simple_int_selector )
    {
        NConv nc;
        
        Z16 z16 = nc . stringToInt ( z16, "123" );
        EXPECT_EQ ( -1, z16 );
        Z32 z32 = nc . stringToInt ( z32, "123" );
        EXPECT_EQ ( -2, z32 );
        Z64 z64 = nc . stringToInt ( z64, "123" );
        EXPECT_EQ ( -3, z64 );

        N16 n16 = nc . stringToInt ( n16, "123" );
        EXPECT_EQ ( 1U, n16 );
        N32 n32 = nc . stringToInt ( n32, "123" );
        EXPECT_EQ ( 2U, n32 );
        N64 n64 = nc . stringToInt ( n64, "123" );
        EXPECT_EQ ( 3U, n64 );
    }

    TEST ( NumberConversionTest, large_int_selector )
    {
        NConv nc;
        
        Z128 z128 = nc . stringToInt ( z128, "123" );
        EXPECT_TRUE ( z128 == -4LL );

        N128 n128 = nc . stringToInt ( n128, "123" );
        EXPECT_TRUE ( n128 == 4ULL );
    }

    TEST ( NumberConversionTest, simple_real_selector )
    {
        NConv nc;
        
        R32 r32 = nc . stringToReal ( r32, "123" );
        EXPECT_EQ ( ( float ) 1.0, r32 );
        R64 r64 = nc . stringToReal ( r64, "123" );
        EXPECT_EQ ( ( double ) 2.0, r64 );
        long double ld = nc . stringToReal ( ld, "123" );
        EXPECT_GE ( ld, ( double ) 2.0 );
        EXPECT_LE ( ld, ( double ) 3.0 );
    }

#if HAVE_R128
    TEST ( NumberConversionTest, large_real_selector )
    {
        NConv nc;
        
        R128 r128 = nc . stringToReal ( r128, "123" );
        EXPECT_EQ ( ( R128 ) 3.0, r128 );
    }
#endif
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
