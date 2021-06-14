/*

  vdb3.kfc.decflt-tst

 */

#include "decflt.cpp"

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

    TEST ( DecimalFloatTest, Z64_constants )
    {
        EXPECT_LT ( Z64_MAX_MANT, Z64_MAX );
        EXPECT_GT ( Z64_MIN_MANT, Z64_MIN );

        const Z64 Z64_MAX_DIV10 = Z64_MAX / 10;
        const Z64 Z64_MIN_DIV10 = ( Z64_MIN + 1 ) / 10;
        EXPECT_GT ( Z64_MAX_MANT, Z64_MAX_DIV10 );
        EXPECT_LT ( Z64_MIN_MANT, Z64_MIN_DIV10 );

        Z64 z64_max_mant = 0;
        for ( Z32 i = 0; i < Z64_MAX_EXP; ++ i )
        {
            EXPECT_LE ( z64_max_mant, Z64_MAX_DIV10 );
            z64_max_mant *= 10;
            z64_max_mant += 9;
        }
        EXPECT_GT ( z64_max_mant, Z64_MAX_DIV10 );
        EXPECT_EQ ( z64_max_mant, Z64_MAX_MANT );
        EXPECT_EQ ( - z64_max_mant, Z64_MIN_MANT );
        EXPECT_EQ ( - Z64_MAX_EXP, Z64_MIN_EXP );
    }

    TEST ( DecimalFloatTest, N64_constants )
    {
        const N64 N64_MAX_DIV10 = N64_MAX / 10;
        N64 n64_max_mant = 0;
        for ( Z32 i = 0; i < N64_MAX_EXP; ++ i )
        {
            EXPECT_LE ( n64_max_mant, N64_MAX_DIV10 );
            n64_max_mant *= 10;
            n64_max_mant += 9;
        }
        EXPECT_GT ( n64_max_mant, N64_MAX_DIV10 );
        EXPECT_LT ( n64_max_mant, N64_MAX );
    }

    TEST ( DecimalFloatTest, Z128_constants )
    {
        EXPECT_LT ( Z128_MAX_MANT, Z128_MAX );
        EXPECT_GT ( Z128_MIN_MANT, Z128_MIN );

        const Z128 Z128_MAX_DIV10 ( Z128_MAX / 10 );
        const Z128 Z128_MIN_DIV10 ( ( Z128_MIN + 1 ) / 10 );
        EXPECT_GT ( Z128_MAX_MANT, Z128_MAX_DIV10 );
        EXPECT_LT ( Z128_MIN_MANT, Z128_MIN_DIV10 );

        Z128 z128_max_mant ( 0 );
        for ( Z32 i = 0; i < Z128_MAX_EXP; ++ i )
        {
            EXPECT_LE ( z128_max_mant, Z128_MAX_DIV10 );
            z128_max_mant *= 10;
            z128_max_mant += 9;
        }
        EXPECT_GT ( z128_max_mant, Z128_MAX_DIV10 );
        EXPECT_EQ ( z128_max_mant, Z128_MAX_MANT );
        EXPECT_EQ ( - z128_max_mant, Z128_MIN_MANT );
        EXPECT_EQ ( - Z128_MAX_EXP, Z128_MIN_EXP );
    }

    TEST ( DecimalFloatTest, N128_constants )
    {
        const N128 N128_MAX_DIV10 ( N128_MAX / 10 );
        N128 n128_max_mant ( 0 );
        for ( Z32 i = 0; i < N128_MAX_EXP; ++ i )
        {
            EXPECT_LE ( n128_max_mant, N128_MAX_DIV10 );
            n128_max_mant *= 10;
            n128_max_mant += 9;
        }
        EXPECT_GT ( n128_max_mant, N128_MAX_DIV10 );
        EXPECT_LT ( n128_max_mant, N128_MAX );
    }

    TEST ( DecimalFloatTest, NORM_constants )
    {
        const Z64 Z64_MAX_DIV10 ( Z64_MAX / 10 );
        Z64 z64_norm_fact ( 1 );
        for ( Z32 i = 0; i > NORM_EXP; -- i )
        {
            EXPECT_LE ( z64_norm_fact, Z64_MAX_DIV10 );
            z64_norm_fact *= 10;
        }
        EXPECT_EQ ( z64_norm_fact, Z64_NORM_FACT );
        EXPECT_EQ ( ( R64 ) z64_norm_fact, R64_NORM_FACT );
    }

    TEST ( DecimalFloatTest, DecConst_Z64_results )
    {
        EXPECT_EQ ( DecConst < Z64 > :: minVal (), Z64_MIN_MANT );
        EXPECT_EQ ( DecConst < Z64 > :: maxVal (), Z64_MAX_MANT );
        EXPECT_EQ ( DecConst < Z64 > :: minExp (), Z64_MIN_EXP );
        EXPECT_EQ ( DecConst < Z64 > :: maxExp (), Z64_MAX_EXP );

        const Z64 Z64_MAX_DIV10 = Z64_MAX / 10;

        Z64 factor = 1;
        EXPECT_EQ ( DecConst < Z64 > :: exp10 ( 0 ), factor );
        for ( Z32 i = 1; i <= Z64_MAX_EXP; ++ i )
        {
            EXPECT_LE ( factor, Z64_MAX_DIV10 );
            factor *= 10;
            EXPECT_EQ ( DecConst < Z64 > :: exp10 ( i ), factor );
        }
    }

    TEST ( DecimalFloatTest, DecConst_Z128_results )
    {
        EXPECT_EQ ( DecConst < Z128 > :: minVal (), Z128_MIN_MANT );
        EXPECT_EQ ( DecConst < Z128 > :: maxVal (), Z128_MAX_MANT );
        EXPECT_EQ ( DecConst < Z128 > :: minExp (), Z128_MIN_EXP );
        EXPECT_EQ ( DecConst < Z128 > :: maxExp (), Z128_MAX_EXP );

        const Z128 Z128_MAX_DIV10 = Z128_MAX / 10;

        Z32 i;
        Z128 factor ( 1 );
        EXPECT_EQ ( DecConst < Z128 > :: exp10 ( 0 ), factor );
        for ( i = 1; i <= Z64_MAX_EXP; ++ i )
        {
            EXPECT_LE ( factor, Z128_MAX_DIV10 );
            factor *= 10;
            EXPECT_EQ ( DecConst < Z128 > :: exp10 ( i ), factor );
        }
        for ( ; i <= Z128_MAX_EXP; ++ i )
        {
            EXPECT_LE ( factor, Z128_MAX_DIV10 );
            factor *= 10;
            EXPECT_EQ ( DecConst < Z128 > :: exp10 ( i ), factor );
        }
    }

    TEST ( DecimalFloatTest, constructor_destructor )
    {
        DecFltPoint < Z64 > x;
        EXPECT_EQ ( NORM_EXP, x . getExponent () );
        EXPECT_EQ ( 0LL, x . getMantissa64 () );
    }

    TEST ( DecimalFloatTest, construct_from_int )
    {
        DecFltPoint < Z64 > x ( 123 );
        EXPECT_EQ ( NORM_EXP, x . getExponent () );
        EXPECT_EQ ( 123000000LL, x . getMantissa64 () );
        EXPECT_EQ ( 123LL, x . toZ64 () );

        x = 123456789LL;
        EXPECT_EQ ( NORM_EXP, x . getExponent () );
        EXPECT_EQ ( 123456789000000LL, x . getMantissa64 () );
        EXPECT_EQ ( 123456789LL, x . toZ64 () );

        x = 1234567898765432LL;
        EXPECT_EQ ( -2, x . getExponent () );
        EXPECT_EQ ( 123456789876543200LL, x . getMantissa64 () );
        EXPECT_EQ ( 1234567898765432LL, x . toZ64 () );

        Z128 y ( 1234567898765432LL );
        y *= 1000000;
        // 1234567898765432000000
        EXPECT_EQ ( 66LL, zhi ( y ) );
        EXPECT_EQ ( 17082789900601593344ULL, nlo ( y ) );
        x = y;
        EXPECT_EQ ( 4, x . getExponent () );
        EXPECT_EQ ( 123456789876543200LL, x . getMantissa64 () );
        EXPECT_THROW ( x . toZ64 (), OverflowException );
    }

    TEST ( DecimalFloatTest, construct_from_float )
    {
        DecFltPoint < Z64 > x ( 10.0 );
        EXPECT_EQ ( NORM_EXP, x . getExponent () );
        EXPECT_EQ ( 10000000LL, x . getMantissa64 () );
        EXPECT_EQ ( 10LL, x . toZ64 () );

        x = 7.5;
        EXPECT_EQ ( NORM_EXP, x . getExponent () );
        EXPECT_EQ ( 7500000LL, x . getMantissa64 () );
        EXPECT_EQ ( 7LL, x . toZ64 () );

        x = 1.1;
        EXPECT_EQ ( -17, x . getExponent () );
        EXPECT_EQ ( 110000000000000016LL, x . getMantissa64 () );
        EXPECT_EQ ( 1LL, x . toZ64 () );

        x = 0.0012345;
        EXPECT_EQ ( -7, x . getExponent () );
        EXPECT_EQ ( 12345LL, x . getMantissa64 () );
        EXPECT_EQ ( 0LL, x . toZ64 () );

        x = 9876543210.0012345678;
        EXPECT_EQ ( -8, x . getExponent () );
        EXPECT_EQ ( 987654321000123392LL, x . getMantissa64 () );
        EXPECT_EQ ( 9876543210LL, x . toZ64 () );

        x = -9876543210.0012345678;
        EXPECT_EQ ( -8, x . getExponent () );
        EXPECT_EQ ( -987654321000123392LL, x . getMantissa64 () );
        EXPECT_EQ ( -9876543210LL, x . toZ64 () );
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
