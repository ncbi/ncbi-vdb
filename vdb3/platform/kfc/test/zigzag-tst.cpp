/*

  vdb3.kfc.zigzag-tst

 */

#include "zigzag.cpp"

#include "ktrace.cpp"
#include "z128.cpp"
#include "memset_s.cpp"
#include "ascii.cpp"
#include "fmt-base.cpp"
#include "xp.cpp"
#include "except.cpp"
#include "xbt.cpp"

#include <gtest/gtest.h>

#include <ios>
#include <iomanip>

namespace vdb3
{

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

    std :: ostream & operator << ( std :: ostream & o, const Z128 & z )
    {
        char buffer [ 128 ];
        buffer [ sizeof buffer - 1 ] = 0;

        Z128 x ( z );
        const char * sign = "";
        if ( x < 0 )
        {
            sign = "-";
            x = - x;
        }

        size_t i;
        for ( i = sizeof buffer - 1; i > 0; )
        {
            -- i;
            buffer [ i ] = ( char ) ( ( Z64 ) ( x % 10 ) ) + '0';

            x /= 10;
            if ( x == 0 )
                break;
        }

        return o << sign << & buffer [ i ];
    }

    TEST ( ZigZagBaseTest, constructor_destructor_64 )
    {
        ZigZagBase < N64 > zz64;
        EXPECT_EQ ( 0U, zz64 . magnitude () );
    }

    TEST ( ZigZagBaseTest, constructor_destructor_128 )
    {
        ZigZagBase < N128 > zz128;
        EXPECT_EQ ( 0U, zz128 . magnitude () );
    }

    TEST ( ZigZagBaseTest, small_values_64 )
    {
        ZigZagBase < N64 > zz64_a ( 1LL );
        EXPECT_EQ ( 2U, zz64_a . magnitude () );
        EXPECT_EQ ( 2ULL, zz64_a . toRawN64 () );
        EXPECT_EQ ( 1LL, zz64_a . toZ64 () );

        zz64_a . cpy ( 7LL );
        EXPECT_EQ ( 4U, zz64_a . magnitude () );
        EXPECT_EQ ( 14ULL, zz64_a . toRawN64 () );
        EXPECT_EQ ( 7LL, zz64_a . toZ64 () );

        ZigZagBase < N64 > zz64_b ( -20LL );
        EXPECT_EQ ( 6U, zz64_b . magnitude () );
        EXPECT_EQ ( 39ULL, zz64_b . toRawN64 () );
        EXPECT_EQ ( -20LL, zz64_b . toZ64 () );

        zz64_b . cpy ( -7LL );
        EXPECT_EQ ( 4U, zz64_b . magnitude () );
        EXPECT_EQ ( 13ULL, zz64_b . toRawN64 () );
        EXPECT_EQ ( -7LL, zz64_b . toZ64 () );
    }

    TEST ( ZigZagBaseTest, small_values_128 )
    {
        ZigZagBase < N128 > zz128_a ( 1LL );
        EXPECT_EQ ( 2U, zz128_a . magnitude () );
        N128 n128_a ( zz128_a . toRawN128 () );
        EXPECT_EQ ( 0ULL, nhi ( n128_a ) );
        EXPECT_EQ ( 2ULL, nlo ( n128_a ) );
        EXPECT_EQ ( 1LL, zz128_a . toZ64 () );

        zz128_a  . cpy ( 7LL );
        EXPECT_EQ ( 4U, zz128_a . magnitude () );
        n128_a = zz128_a . toRawN128 ();
        EXPECT_EQ ( 0ULL, nhi ( n128_a ) );
        EXPECT_EQ ( 14ULL, nlo ( n128_a ) );
        EXPECT_EQ ( 7LL, zz128_a . toZ64 () );

        ZigZagBase < N128 > zz128_b ( -20LL );
        EXPECT_EQ ( 6U, zz128_b . magnitude () );
        N128 n128_b ( zz128_b . toRawN128 () );
        EXPECT_EQ ( 0ULL, nhi ( n128_b ) );
        EXPECT_EQ ( 39ULL, nlo ( n128_b ) );
        EXPECT_EQ ( -20LL, zz128_b . toZ64 () );

        zz128_b . cpy ( -7LL );
        EXPECT_EQ ( 4U, zz128_b . magnitude () );
        n128_b = zz128_b . toRawN128 ();
        EXPECT_EQ ( 0ULL, nhi ( n128_b ) );
        EXPECT_EQ ( 13ULL, nlo ( n128_b ) );
        EXPECT_EQ ( -7LL, zz128_b . toZ64 () );
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
