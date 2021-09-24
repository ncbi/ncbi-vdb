/*

  vdb3.kfc.defs-tst

 */

#include "defs.cpp"
#include "ktrace.cpp"

#include <gtest/gtest.h>

namespace vdb3
{
    TEST ( DefsTest, real_size )
    {
        EXPECT_EQ ( sizeof ( R32 ), 4U );
        EXPECT_EQ ( sizeof ( R64 ), 8U );
#if HAVE_R128
        EXPECT_EQ ( sizeof ( R128 ), 16U );
#endif
    }

    TEST ( DefsTest, integer_size )
    {
        EXPECT_EQ ( sizeof ( Z8 ), 1U );
        EXPECT_EQ ( sizeof ( Z16 ), 2U );
        EXPECT_EQ ( sizeof ( Z32 ), 4U );
        EXPECT_EQ ( sizeof ( Z64 ), 8U );
#if HAVE_Z128
        EXPECT_EQ ( sizeof ( Z128 ), 16U );
#endif
    }

    TEST ( DefsTest, natural_size )
    {
        EXPECT_EQ ( sizeof ( N8 ), 1U );
        EXPECT_EQ ( sizeof ( N16 ), 2U );
        EXPECT_EQ ( sizeof ( N32 ), 4U );
        EXPECT_EQ ( sizeof ( N64 ), 8U );
#if HAVE_Z128
        EXPECT_EQ ( sizeof ( N128 ), 16U );
#endif
    }

    TEST ( DefsTest, natural_constants )
    {
        EXPECT_EQ ( sizeof N8_MAX, 1U );
        EXPECT_EQ ( ( N8 ) ( N8_MAX + 1 ), ( N8 ) 0U );
        EXPECT_EQ ( sizeof N16_MAX, 2U );
        EXPECT_EQ ( ( N16 ) ( N16_MAX + 1 ), ( N16 ) 0U );
        EXPECT_EQ ( sizeof N32_MAX, 4U );
        EXPECT_EQ ( ( N32 ) ( N32_MAX + 1 ), ( N32 ) 0U );
        EXPECT_EQ ( sizeof N64_MAX, 8U );
        EXPECT_EQ ( ( N64 ) ( N64_MAX + 1 ), ( N64 ) 0U );
#if HAVE_Z128
        EXPECT_EQ ( sizeof N128_MAX, 16U );
        EXPECT_EQ ( ( N128 ) ( N128_MAX + 1 ), ( N128 ) 0U );
#endif
    }

    TEST ( DefsTest, integer_constants )
    {
        EXPECT_EQ ( sizeof Z8_MAX, 1U );
        EXPECT_EQ ( ( N8 ) Z8_MAX + 1, ( N8 ) Z8_MIN );
        EXPECT_EQ ( sizeof Z16_MAX, 2U );
        EXPECT_EQ ( ( N16 ) Z16_MAX + 1, ( N16 ) Z16_MIN );
        EXPECT_EQ ( sizeof Z32_MAX, 4U );
        EXPECT_EQ ( ( N32 ) Z32_MAX + 1, ( N32 ) Z32_MIN );
        EXPECT_EQ ( sizeof Z64_MAX, 8U );
        EXPECT_EQ ( ( N64 ) Z64_MAX + 1, ( N64 ) Z64_MIN );
#if HAVE_Z128
        EXPECT_EQ ( sizeof Z128_MAX, 16U );
        EXPECT_EQ ( ( N128 ) Z128_MAX + 1, ( N128 ) Z128_MIN );
#endif
    }

    TEST ( DefsTest, byte_swaps )
    {
        /* gcc includes try to be exceedingly clever about this,
           which is sort of weird... they have versions that detect
           constants and do one thing, so I am not sure these are
           testing real code.

           on x86-64 I have to fiddle around because of their treatment
           of 16-bit byteswaps.
        */

        // this doesn't work
        //EXPECT_EQ ( __bswap_16 ( 0x1234U ), 0x3412U );

        // this DOES work but may be less portable... ?
        //EXPECT_EQ ( __builtin_bswap16 ( 0x1234U ), 0x3412U );

        // this works and is clearly portable
        N16 n16 = __bswap_16 ( 0x1234U );
        EXPECT_EQ ( n16, 0x3412U );

        // these work as expected because they're supported in
        // the architecture instruction set
        EXPECT_EQ ( __bswap_32 ( 0x12345678U ), 0x78563412U );
        EXPECT_EQ ( __bswap_64 ( 0x1234567898765432ULL ), 0x3254769878563412ULL );

#if HAVE_Z128
        Z128 val1 ( 0x1234567898765432ULL );
        val1 <<= 64;
        val1 |= 0x3254769878563412ULL;
        Z128 val2 = __bswap_128 ( val1 );
        EXPECT_EQ ( val1, val2 );
#endif
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
