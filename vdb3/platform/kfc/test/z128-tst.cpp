/*

  vdb3.kfc.z128x-tst


 */

#include <vdb3/kfc/z128.hpp>

#if ! HAVE_Z128
#include "z128.cpp"
#endif

#include "ktrace.cpp"
#include "defs.cpp"

#include <gtest/gtest.h>

namespace vdb3
{

#if ! HAVE_Z128

    /*=================================================*
     *                     Z128Base                    *
     *=================================================*/

    TEST ( Z128BaseTest, simple_constructor_destructor )
    {
        Z128Base z128;
    }

    TEST ( Z128BaseTest, negative_shift )
    {
        Z64 z64 ( 123 );
        Z128Base z128 ( z64 );

        // negative shifts are undefined
        // they "could" be promoted to unsigned,
        // creating a nearly infinite shift
        // or they could be interpreted by direction
        z64 <<= -1;
        z128 . shl ( -1LL );

#if NEG_SHIFT_REVERSES_DIRECTION
        // g++ on ARM is saying it's equivalent to shift right
        EXPECT_EQ ( z64, 123LL >> 1 );
        EXPECT_EQ ( z128 . hi, 0LL );
        EXPECT_EQ ( z128 . low, 123ULL >> 1 );
#else
        // compiler promotes to unsigned first
        EXPECT_EQ ( z64, 0LL );
        EXPECT_EQ ( z128 . hi, 0LL );
        EXPECT_EQ ( z128 . low, 0ULL );
#endif
        N64 n64 ( 123 );
        N128Base n128 ( n64 );

        // negative shifts are undefined
        // they "could" be promoted to unsigned,
        // creating a nearly infinite shift
        // or they could be interpreted by direction
        n64 <<= -1;
        n128 . shl ( Z128Base ( -1LL ) );

#if NEG_SHIFT_REVERSES_DIRECTION
        // g++ on ARM is saying it's equivalent to shift right
        EXPECT_EQ ( n64, 123ULL >> 1 );
        EXPECT_EQ ( n128 . hi, 0ULL );
        EXPECT_EQ ( n128 . low, 123ULL >> 1 );
#else
        // compiler promotes to unsigned first
        EXPECT_EQ ( n64, 0ULL );
        EXPECT_EQ ( n128 . hi, 0ULL );
        EXPECT_EQ ( n128 . low, 0ULL );
#endif
    }

#endif // ! HAVE_Z128

    /*=================================================*
     *                       Z128                      *
     *=================================================*/

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

    TEST ( Z128Test, simple_constructor_destructor )
    {
        Z128 z128;
        (void)z128;
        N128 n128;
        (void)n128;
    }

    TEST ( Z128Test, simple_object_size )
    {
        Z128 z128;
        N128 n128;

        EXPECT_EQ ( sizeof z128, 16U );
        EXPECT_EQ ( sizeof n128, 16U );
    }

    TEST ( Z128Test, constructors )
    {
        Z128 z1 ( Z16_MAX );
        Z128 z2 ( N16_MAX );
        Z128 z3 ( Z32_MAX );
        Z128 z4 ( N32_MAX );
        Z128 z5 ( Z64_MAX );
        Z128 z6 ( N64_MAX );
        Z128 z7 ( Z128_MAX );
        Z128 z8 ( N128_MAX );

        EXPECT_EQ ( nlo ( z1 ), 0x7FFFULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );
        EXPECT_EQ ( nlo ( z2 ), 0xFFFFULL );
        EXPECT_EQ ( nhi ( z2 ), 0ULL );
        EXPECT_EQ ( nlo ( z3 ), 0x7FFFFFFFULL );
        EXPECT_EQ ( nhi ( z3 ), 0ULL );
        EXPECT_EQ ( nlo ( z4 ), 0xFFFFFFFFULL );
        EXPECT_EQ ( nhi ( z4 ), 0ULL );
        EXPECT_EQ ( nlo ( z5 ), 0x7FFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nhi ( z5 ), 0ULL );
        EXPECT_EQ ( nlo ( z6 ), 0xFFFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nhi ( z6 ), 0ULL );
        EXPECT_EQ ( nlo ( z7 ), 0xFFFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nhi ( z7 ), 0x7FFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nlo ( z8 ), 0xFFFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nhi ( z8 ), 0xFFFFFFFFFFFFFFFFULL );

        N128 n1 ( Z16_MAX );
        N128 n2 ( N16_MAX );
        N128 n3 ( Z32_MAX );
        N128 n4 ( N32_MAX );
        N128 n5 ( Z64_MAX );
        N128 n6 ( N64_MAX );
        N128 n7 ( Z128_MAX );
        N128 n8 ( N128_MAX );

        EXPECT_EQ ( nlo ( n1 ), 0x7FFFULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );
        EXPECT_EQ ( nlo ( n2 ), 0xFFFFULL );
        EXPECT_EQ ( nhi ( n2 ), 0ULL );
        EXPECT_EQ ( nlo ( n3 ), 0x7FFFFFFFULL );
        EXPECT_EQ ( nhi ( n3 ), 0ULL );
        EXPECT_EQ ( nlo ( n4 ), 0xFFFFFFFFULL );
        EXPECT_EQ ( nhi ( n4 ), 0ULL );
        EXPECT_EQ ( nlo ( n5 ), 0x7FFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nhi ( n5 ), 0ULL );
        EXPECT_EQ ( nlo ( n6 ), 0xFFFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nhi ( n6 ), 0ULL );
        EXPECT_EQ ( nlo ( n7 ), 0xFFFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nhi ( n7 ), 0x7FFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nlo ( n8 ), 0xFFFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nhi ( n8 ), 0xFFFFFFFFFFFFFFFFULL );
    }

    TEST ( Z128Test, simple_add_assign )
    {
        Z128 z1 ( Z16_MAX );
        z1 += 1;
        EXPECT_EQ ( nlo ( z1 ), ( N64 ) Z16_MAX + 1 );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z1 = N64_MAX;
        EXPECT_EQ ( nlo ( z1 ), 0xFFFFFFFFFFFFFFFFULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );
        z1 += 1;
        EXPECT_EQ ( nlo ( z1 ), 0ULL );
        EXPECT_EQ ( nhi ( z1 ), 1ULL );
    }

    TEST ( Z128Test, multiply_and_add )
    {
        Z128 z1 ( 0 );
        EXPECT_EQ ( nlo ( z1 ), 0ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z1 *= 10;
        EXPECT_EQ ( nlo ( z1 ), 0ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );
        z1 += 1;
        EXPECT_EQ ( nlo ( z1 ), 1ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z1 *= 10;
        EXPECT_EQ ( nlo ( z1 ), 10ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );
        z1 += 2;
        EXPECT_EQ ( nlo ( z1 ), 12ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z1 *= 10;
        EXPECT_EQ ( nlo ( z1 ), 120ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );
        z1 += 3;
        EXPECT_EQ ( nlo ( z1 ), 123ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z1 *= 10;
        EXPECT_EQ ( nlo ( z1 ), 1230ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );
        z1 += 4;
        EXPECT_EQ ( nlo ( z1 ), 1234ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );
    }

    TEST ( Z128Test, N128_div_10 )
    {
        N128 n1 ( 1234 );
        EXPECT_EQ ( nlo ( n1 ), 1234ULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );

        n1 /= 10;
        EXPECT_EQ ( nlo ( n1 ), 123ULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );

        n1 /= 10;
        EXPECT_EQ ( nlo ( n1 ), 12ULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );

        n1 /= 10;
        EXPECT_EQ ( nlo ( n1 ), 1ULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );

        n1 /= 10;
        EXPECT_EQ ( nlo ( n1 ), 0ULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );
    }

    TEST ( Z128Test, Z128_div_10 )
    {
        Z128 z1 ( 1234 );
        EXPECT_EQ ( nlo ( z1 ), 1234ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z1 /= 10;
        EXPECT_EQ ( nlo ( z1 ), 123ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z1 /= 10;
        EXPECT_EQ ( nlo ( z1 ), 12ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z1 /= 10;
        EXPECT_EQ ( nlo ( z1 ), 1ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z1 /= 10;
        EXPECT_EQ ( nlo ( z1 ), 0ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );
    }

    TEST ( Z128Test, N128_mod_10 )
    {
        N128 n1 ( 1234 );
        EXPECT_EQ ( nlo ( n1 ), 1234ULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );

        N128 n2 = n1 % 10;
        EXPECT_EQ ( nlo ( n2 ), 4ULL );
        EXPECT_EQ ( nhi ( n2 ), 0ULL );

        n1 /= 10;
        EXPECT_EQ ( nlo ( n1 ), 123ULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );

        n2 = n1 % 10;
        EXPECT_EQ ( nlo ( n2 ), 3ULL );
        EXPECT_EQ ( nhi ( n2 ), 0ULL );

        n1 /= 10;
        EXPECT_EQ ( nlo ( n1 ), 12ULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );

        n2 = n1 % 10;
        EXPECT_EQ ( nlo ( n2 ), 2ULL );
        EXPECT_EQ ( nhi ( n2 ), 0ULL );

        n1 /= 10;
        EXPECT_EQ ( nlo ( n1 ), 1ULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );

        n2 = n1 % 10;
        EXPECT_EQ ( nlo ( n2 ), 1ULL );
        EXPECT_EQ ( nhi ( n2 ), 0ULL );

        n1 /= 10;
        EXPECT_EQ ( nlo ( n1 ), 0ULL );
        EXPECT_EQ ( nhi ( n1 ), 0ULL );

        n2 = n1 % 10;
        EXPECT_EQ ( nlo ( n2 ), 0ULL );
        EXPECT_EQ ( nhi ( n2 ), 0ULL );
    }

    TEST ( Z128Test, Z128_mod_10 )
    {
        Z128 z1 ( 1234 );
        EXPECT_EQ ( nlo ( z1 ), 1234ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        Z128 z2 = z1 % 10;
        EXPECT_EQ ( nlo ( z2 ), 4ULL );
        EXPECT_EQ ( nhi ( z2 ), 0ULL );

        z1 /= 10;
        EXPECT_EQ ( nlo ( z1 ), 123ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z2 = z1 % 10;
        EXPECT_EQ ( nlo ( z2 ), 3ULL );
        EXPECT_EQ ( nhi ( z2 ), 0ULL );

        z1 /= 10;
        EXPECT_EQ ( nlo ( z1 ), 12ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z2 = z1 % 10;
        EXPECT_EQ ( nlo ( z2 ), 2ULL );
        EXPECT_EQ ( nhi ( z2 ), 0ULL );

        z1 /= 10;
        EXPECT_EQ ( nlo ( z1 ), 1ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z2 = z1 % 10;
        EXPECT_EQ ( nlo ( z2 ), 1ULL );
        EXPECT_EQ ( nhi ( z2 ), 0ULL );

        z1 /= 10;
        EXPECT_EQ ( nlo ( z1 ), 0ULL );
        EXPECT_EQ ( nhi ( z1 ), 0ULL );

        z2 = z1 % 10;
        EXPECT_EQ ( nlo ( z2 ), 0ULL );
        EXPECT_EQ ( nhi ( z2 ), 0ULL );
    }

    TEST ( Z128Test, palindrome_byte_swap )
    {
        Z128 val1 ( 0x1234567898765432ULL );
        val1 <<= 64;
        val1 |= 0x3254769878563412ULL;
        Z128 val2 = __bswap_128 ( val1 );
        EXPECT_EQ ( val1, val2 );
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
