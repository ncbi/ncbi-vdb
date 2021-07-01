/*

  vdb3.cmn.nconv-tst

 */

#include <vdb3/kfc/rsrc-mem.hpp>
#include <vdb3/cmn/auto-rgn.hpp>

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

#if HAVE_Z128
    inline Z64 zlo ( Z128 z128 )
    { return ( Z64 ) z128; }

    inline Z64 zhi ( const Z128 & z128 )
    { return ( Z64 ) ( z128 >> 64 ); }

    inline N64 nlo ( const Z128 & z128 )
    { return ( N64 ) z128; }

    inline N64 nhi ( const Z128 & z128 )
    { return ( N64 ) ( z128 >> 64 ); }

    inline N64 nlo ( const N128 & n128 )
    { return ( N64 ) n128; }

    inline N64 nhi ( const N128 & n128 )
    { return ( N64 ) ( n128 >> 64 ); }
#else
    inline Z64 zlo ( const Z128 & z128 )
    { return ( Z64 ) z128 . low64 (); }

    inline Z64 zhi ( const Z128 & z128 )
    { return z128 . high64 (); }

    inline N64 nlo ( const Z128 & z128 )
    { return z128 . low64 (); }

    inline N64 nhi ( const Z128 & z128 )
    { return z128 . high64 (); }

    inline N64 nlo ( const N128 & n128 )
    { return n128 . low64 (); }

    inline N64 nhi ( const N128 & n128 )
    { return n128 . high64 (); }
#endif

    TEST ( NumberConversionTest, R32_to_string )
    {
        R32 r32;
        NConv nc;
        
        EXPECT_EQ ( String ( "123.5" ), nc . realToString ( rsrc, r32 = 123.5 ) );
        EXPECT_EQ ( String ( "123.500" ), nc . realToString ( rsrc, r32 = 123.5, 3 ) );
    }

    TEST ( NumberConversionTest, R64_to_string )
    {
        R64 r64;
        NConv nc;
        
        EXPECT_EQ ( String ( "123.5" ), nc . realToString ( rsrc, r64 = 123.5 ) );
        EXPECT_EQ ( String ( "123.500" ), nc . realToString ( rsrc, r64 = 123.5, 3 ) );
    }

    TEST ( NumberConversionTest, Z16_to_string )
    {
        Z16 z16;
        NConv nc;
        
        EXPECT_EQ ( String ( "1234" ), nc . intToString ( rsrc, z16 = 1234 ) );
        EXPECT_EQ ( String ( "-1234" ), nc . intToString ( rsrc, z16 = -1234 ) );
        EXPECT_EQ ( String ( "10011010010" ), nc . intToString ( rsrc, z16 = 1234, 2 ) );
        EXPECT_EQ ( String ( "1111101100101110" ), nc . intToString ( rsrc, z16 = -1234, 2 ) );
        EXPECT_EQ ( String ( "2322" ), nc . intToString ( rsrc, z16 = 1234, 8 ) );
        EXPECT_EQ ( String ( "175456" ), nc . intToString ( rsrc, z16 = -1234, 8 ) );
        EXPECT_EQ ( String ( "4d2" ), nc . intToString ( rsrc, z16 = 1234, 16 ) );
        EXPECT_EQ ( String ( "fb2e" ), nc . intToString ( rsrc, z16 = -1234, 16 ) );
        EXPECT_EQ ( String ( "1b4" ), nc . intToString ( rsrc, z16 = 1234, 30 ) );
        EXPECT_EQ ( String ( "2bdc" ), nc . intToString ( rsrc, z16 = -1234, 30 ) );
        
        EXPECT_EQ ( String ( "32767" ), nc . intToString ( rsrc, Z16_MAX ) );
        EXPECT_EQ ( String ( "-32768" ), nc . intToString ( rsrc, Z16_MIN ) );
        EXPECT_EQ ( String ( "7fff" ), nc . intToString ( rsrc, Z16_MAX, 16 ) );
        EXPECT_EQ ( String ( "8000" ), nc . intToString ( rsrc, Z16_MIN, 16 ) );
    }

    TEST ( NumberConversionTest, Z32_to_string )
    {
        Z32 z32;
        NConv nc;
        
        EXPECT_EQ ( String ( "1234" ), nc . intToString ( rsrc, z32 = 1234 ) );
        EXPECT_EQ ( String ( "-1234" ), nc . intToString ( rsrc, z32 = -1234 ) );
        EXPECT_EQ ( String ( "10011010010" ), nc . intToString ( rsrc, z32 = 1234, 2 ) );
        EXPECT_EQ ( String ( "11111111111111111111101100101110" ),
                    nc . intToString ( rsrc, z32 = -1234, 2 ) );
        EXPECT_EQ ( String ( "2322" ), nc . intToString ( rsrc, z32 = 1234, 8 ) );
        EXPECT_EQ ( String ( "37777775456" ), nc . intToString ( rsrc, z32 = -1234, 8 ) );
        EXPECT_EQ ( String ( "4d2" ), nc . intToString ( rsrc, z32 = 1234, 16 ) );
        EXPECT_EQ ( String ( "fffffb2e" ), nc . intToString ( rsrc, z32 = -1234, 16 ) );
        EXPECT_EQ ( String ( "1b4" ), nc . intToString ( rsrc, z32 = 1234, 30 ) );
        EXPECT_EQ ( String ( "5qmcofc" ), nc . intToString ( rsrc, z32 = -1234, 30 ) );
        
        EXPECT_EQ ( String ( "1234567" ), nc . intToString ( rsrc, z32 = 1234567 ) );
        EXPECT_EQ ( String ( "-1234567" ), nc . intToString ( rsrc, z32 = -1234567 ) );
        EXPECT_EQ ( String ( "100101101011010000111" ),
                    nc . intToString ( rsrc, z32 = 1234567, 2 ) );
        EXPECT_EQ ( String ( "11111111111011010010100101111001" ),
                    nc . intToString ( rsrc, z32 = -1234567, 2 ) );
        EXPECT_EQ ( String ( "4553207" ), nc . intToString ( rsrc, z32 = 1234567, 8 ) );
        EXPECT_EQ ( String ( "37773224571" ), nc . intToString ( rsrc, z32 = -1234567, 8 ) );
        EXPECT_EQ ( String ( "12d687" ), nc . intToString ( rsrc, z32 = 1234567, 16 ) );
        EXPECT_EQ ( String ( "ffed2979" ), nc . intToString ( rsrc, z32 = -1234567, 16 ) );
        EXPECT_EQ ( String ( "1flm7" ), nc . intToString ( rsrc, z32 = 1234567, 30 ) );
        EXPECT_EQ ( String ( "5qkr449" ), nc . intToString ( rsrc, z32 = -1234567, 30 ) );
        
        EXPECT_EQ ( String ( "2147483647" ), nc . intToString ( rsrc, Z32_MAX ) );
        EXPECT_EQ ( String ( "-2147483648" ), nc . intToString ( rsrc, Z32_MIN ) );
        EXPECT_EQ ( String ( "7fffffff" ), nc . intToString ( rsrc, Z32_MAX, 16 ) );
        EXPECT_EQ ( String ( "80000000" ), nc . intToString ( rsrc, Z32_MIN, 16 ) );
    }

    TEST ( NumberConversionTest, Z64_to_string )
    {
        Z64 z64;
        NConv nc;
        
        EXPECT_EQ ( String ( "1234" ), nc . intToString ( rsrc, z64 = 1234 ) );
        EXPECT_EQ ( String ( "-1234" ), nc . intToString ( rsrc, z64 = -1234 ) );
        EXPECT_EQ ( String ( "10011010010" ), nc . intToString ( rsrc, z64 = 1234, 2 ) );
        EXPECT_EQ ( String ( "1111111111111111111111111111111111111111111111111111101100101110" ),
                    nc . intToString ( rsrc, z64 = -1234, 2 ) );
        EXPECT_EQ ( String ( "2322" ), nc . intToString ( rsrc, z64 = 1234, 8 ) );
        EXPECT_EQ ( String ( "1777777777777777775456" ),
                    nc . intToString ( rsrc, z64 = -1234, 8 ) );
        EXPECT_EQ ( String ( "4d2" ), nc . intToString ( rsrc, z64 = 1234, 16 ) );
        EXPECT_EQ ( String ( "fffffffffffffb2e" ), nc . intToString ( rsrc, z64 = -1234, 16 ) );
        EXPECT_EQ ( String ( "1b4" ), nc . intToString ( rsrc, z64 = 1234, 30 ) );
        EXPECT_EQ ( String ( "14l9lkmo30o2jc" ), nc . intToString ( rsrc, z64 = -1234, 30 ) );
        
        EXPECT_EQ ( String ( "1234567" ), nc . intToString ( rsrc, z64 = 1234567 ) );
        EXPECT_EQ ( String ( "-1234567" ), nc . intToString ( rsrc, z64 = -1234567 ) );
        EXPECT_EQ ( String ( "100101101011010000111" ),
                    nc . intToString ( rsrc, z64 = 1234567, 2 ) );
        EXPECT_EQ ( String ( "1111111111111111111111111111111111111111111011010010100101111001" ),
                    nc . intToString ( rsrc, z64 = -1234567, 2 ) );
        EXPECT_EQ ( String ( "4553207" ), nc . intToString ( rsrc, z64 = 1234567, 8 ) );
        EXPECT_EQ ( String ( "1777777777777773224571" ),
                    nc . intToString ( rsrc, z64 = -1234567, 8 ) );
        EXPECT_EQ ( String ( "12d687" ), nc . intToString ( rsrc, z64 = 1234567, 16 ) );
        EXPECT_EQ ( String ( "ffffffffffed2979" ),
                    nc . intToString ( rsrc, z64 = -1234567, 16 ) );
        EXPECT_EQ ( String ( "1flm7" ), nc . intToString ( rsrc, z64 = 1234567, 30 ) );
        EXPECT_EQ ( String ( "14l9lkmo2t8c89" ),
                    nc . intToString ( rsrc, z64 = -1234567, 30 ) );
        
        EXPECT_EQ ( String ( "9223372036854775807" ), nc . intToString ( rsrc, Z64_MAX ) );
        EXPECT_EQ ( String ( "-9223372036854775808" ), nc . intToString ( rsrc, Z64_MIN ) );
        EXPECT_EQ ( String ( "7fffffffffffffff" ), nc . intToString ( rsrc, Z64_MAX, 16 ) );
        EXPECT_EQ ( String ( "8000000000000000" ), nc . intToString ( rsrc, Z64_MIN, 16 ) );
    }

    TEST ( NumberConversionTest, Z128_to_string )
    {
        Z128 z128;
        NConv nc;
        
        EXPECT_EQ ( String ( "1234" ), nc . intToString ( rsrc, z128 = 1234 ) );
        EXPECT_EQ ( String ( "-1234" ), nc . intToString ( rsrc, z128 = -1234 ) );
        EXPECT_EQ ( String ( "10011010010" ), nc . intToString ( rsrc, z128 = 1234, 2 ) );
        EXPECT_EQ ( String ( "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111101100101110" ),
                    nc . intToString ( rsrc, z128 = -1234, 2 ) );
        EXPECT_EQ ( String ( "2322" ), nc . intToString ( rsrc, z128 = 1234, 8 ) );
        EXPECT_EQ ( String ( "3777777777777777777777777777777777777775456" ),
                    nc . intToString ( rsrc, z128 = -1234, 8 ) );
        EXPECT_EQ ( String ( "4d2" ), nc . intToString ( rsrc, z128 = 1234, 16 ) );
        EXPECT_EQ ( String ( "fffffffffffffffffffffffffffffb2e" ), nc . intToString ( rsrc, z128 = -1234, 16 ) );
        EXPECT_EQ ( String ( "1b4" ), nc . intToString ( rsrc, z128 = 1234, 30 ) );
        EXPECT_EQ ( String ( "1a4p5qh8koob2e2gknbn3jbm6rc" ), nc . intToString ( rsrc, z128 = -1234, 30 ) );
        
        EXPECT_EQ ( String ( "1234567" ), nc . intToString ( rsrc, z128 = 1234567 ) );
        EXPECT_EQ ( String ( "-1234567" ), nc . intToString ( rsrc, z128 = -1234567 ) );
        EXPECT_EQ ( String ( "100101101011010000111" ),
                    nc . intToString ( rsrc, z128 = 1234567, 2 ) );
        EXPECT_EQ ( String ( "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111011010010100101111001" ),
                    nc . intToString ( rsrc, z128 = -1234567, 2 ) );
        EXPECT_EQ ( String ( "4553207" ), nc . intToString ( rsrc, z128 = 1234567, 8 ) );
        EXPECT_EQ ( String ( "3777777777777777777777777777777777773224571" ),
                    nc . intToString ( rsrc, z128 = -1234567, 8 ) );
        EXPECT_EQ ( String ( "12d687" ), nc . intToString ( rsrc, z128 = 1234567, 16 ) );
        EXPECT_EQ ( String ( "ffffffffffffffffffffffffffed2979" ),
                    nc . intToString ( rsrc, z128 = -1234567, 16 ) );
        EXPECT_EQ ( String ( "1flm7" ), nc . intToString ( rsrc, z128 = 1234567, 30 ) );
        EXPECT_EQ ( String ( "1a4p5qh8koob2e2gknbn3ja6gg9" ),
                    nc . intToString ( rsrc, z128 = -1234567, 30 ) );

        // 170,141,183,460,469,231,731,687,303,715,884,105,727
        EXPECT_EQ ( String ( "170141183460469231731687303715884105727" ), nc . intToString ( rsrc, Z128_MAX ) );
        // -170,141,183,460,469,231,731,687,303,715,884,105,728
        EXPECT_EQ ( String ( "-170141183460469231731687303715884105728" ), nc . intToString ( rsrc, Z128_MIN ) );
        EXPECT_EQ ( String ( "7fffffffffffffffffffffffffffffff" ), nc . intToString ( rsrc, Z128_MAX, 16 ) );
        EXPECT_EQ ( String ( "80000000000000000000000000000000" ), nc . intToString ( rsrc, Z128_MIN, 16 ) );
    }

    TEST ( NumberConversionTest, bool_to_string )
    {
        NConv nc;
        EXPECT_EQ ( String ( "false" ), nc . boolToString ( false ) );
        EXPECT_EQ ( String ( "true" ), nc . boolToString ( true ) );
        EXPECT_EQ ( String ( "true" ), nc . boolToString ( ( bool ) 123 ) );
    }

    TEST ( NumberConversionTest, unsigned_text_to_Z64 )
    {
        NConv nc;

        EXPECT_EQ ( 9223372036854775807LL,
                    nc . textToZ64 ( CTextLiteral ( "9223372036854775807" ), 10 ) );
    }

    TEST ( NumberConversionTest, DecFltPoint64_from_int_to_Text )
    {
        NConv nc;
        DecFltPoint < Z64 > x ( 123456789LL );
        EXPECT_EQ ( -6, x . getExponent () );
        EXPECT_EQ ( 123456789000000LL, x . getMantissa64 () );
        EXPECT_EQ ( 123456789LL, x . toZ64 () );

        AutoRgn < 256 > stack_buffer;
        MText mrgn = stack_buffer . getMText ();
        MText rslt = nc . DecFlt64ToText ( mrgn, x, 0 );
        EXPECT_EQ ( CTextLiteral ( "123456789" ), CText ( rslt ) );
        rslt = nc . DecFlt64ToText ( mrgn, x, 1 );
        EXPECT_EQ ( CTextLiteral ( "123456789.0" ), CText ( rslt ) );
    }

    TEST ( NumberConversionTest, DecFltPoint64_from_int_to_String )
    {
        NConv nc;
        
        DecFltPoint < Z64 > x ( 123 );
        EXPECT_EQ ( -6, x . getExponent () );
        EXPECT_EQ ( 123000000LL, x . getMantissa64 () );
        EXPECT_EQ ( 123LL, x . toZ64 () );

        x = 123456789LL;
        EXPECT_EQ ( -6, x . getExponent () );
        EXPECT_EQ ( 123456789000000LL, x . getMantissa64 () );
        EXPECT_EQ ( 123456789LL, x . toZ64 () );
        EXPECT_EQ ( String ( "123456789" ), nc . realToString ( rsrc, x, 0 ) );
        EXPECT_EQ ( String ( "123456789.0" ), nc . realToString ( rsrc, x, 1 ) );

        x = 1234567898765432LL;
        EXPECT_EQ ( -2, x . getExponent () );
        EXPECT_EQ ( 123456789876543200LL, x . getMantissa64 () );
        EXPECT_EQ ( 1234567898765432LL, x . toZ64 () );
        EXPECT_EQ ( String ( "1234567898765432.000000" ), nc . realToString ( rsrc, x, 6 ) );
        EXPECT_EQ ( String ( "1234567898765432.0" ), nc . realToString ( rsrc, x ) );

        Z128 y ( 1234567898765432LL );
        y *= 1000000;
        // 1234567898765432000000
        EXPECT_EQ ( 66LL, zhi ( y ) );
        EXPECT_EQ ( 17082789900601593344ULL, nlo ( y ) );
        x = y;
        EXPECT_EQ ( 4, x . getExponent () );
        EXPECT_EQ ( 123456789876543200LL, x . getMantissa64 () );
        EXPECT_THROW ( x . toZ64 (), OverflowException );
        EXPECT_EQ ( String ( "1234567898765432000000.000000" ), nc . realToString ( rsrc, x, 6 ) );
        EXPECT_EQ ( String ( "1234567898765432000000.0" ), nc . realToString ( rsrc, x ) );
        EXPECT_EQ ( String ( "1234567898765432000000.000000000000000000000000000000" ), nc . realToString ( rsrc, x, 30 ) );
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
