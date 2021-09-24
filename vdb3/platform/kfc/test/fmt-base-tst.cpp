/*

  vdb3.kfc.fmt-base-tst

 */

#define MAX_RSRC_MGR 0
#include "fmt-base.cpp"

#include "ktrace.cpp"
#include "ascii.cpp"
#include "z128.cpp"

#include <gtest/gtest.h>

namespace vdb3
{
    class FmtBaseTestFixture
        : public :: testing :: Test
        , public FmtBase
    {
    public:

        virtual void SetUp () override
        {
        }

        virtual void TearDown () override
        {
            str . clear ();
            reset ();
        }

        FmtBaseTestFixture ()
        {
        }

    protected:

        virtual void write ( const TextRgn & val ) override
        {
            str . append ( val . constAddr (), val . size () );
        }

        virtual void fill ( ASCII ch, count_t rpt ) override
        {
            for ( count_t i = 0; i < rpt; ++ i )
                str += ( ch );
        }

        std :: string str;

    };

    TEST_F ( FmtBaseTestFixture, constructor_destructor )
    {
        EXPECT_EQ ( radix, 10U );
        EXPECT_EQ ( min_int_width, 0U );
        EXPECT_EQ ( min_frac_width, 0U );
        EXPECT_EQ ( min_field_width, 0U );
        EXPECT_EQ ( int_left_pad, ' ' );
        EXPECT_EQ ( field_alignment, left );
        EXPECT_EQ ( have_precision, false );
        EXPECT_EQ ( have_alignment, false );
    }

    TEST_F ( FmtBaseTestFixture, set_radix )
    {
        EXPECT_EQ ( radix, 10 );

        EXPECT_NO_THROW ( setRadix ( 13 ) );
        EXPECT_EQ ( radix, 13 );
        EXPECT_NO_THROW ( setRadix ( 1 ) );
        EXPECT_EQ ( radix, 13 );
        EXPECT_NO_THROW ( setRadix ( 35 ) );
        EXPECT_EQ ( radix, 35 );
        EXPECT_NO_THROW ( setRadix ( 45 ) );
        EXPECT_EQ ( radix, 35 );
        EXPECT_NO_THROW ( setRadix ( 36 ) );
        EXPECT_EQ ( radix, 36 );
        EXPECT_NO_THROW ( setRadix ( 37 ) );
        EXPECT_EQ ( radix, 36 );

        EXPECT_EQ ( min_int_width, 0U );
        EXPECT_EQ ( min_frac_width, 0U );
        EXPECT_EQ ( min_field_width, 0U );
        EXPECT_EQ ( int_left_pad, ' ' );
        EXPECT_EQ ( field_alignment, left );
        EXPECT_EQ ( have_precision, false );
        EXPECT_EQ ( have_alignment, false );
    }

    TEST_F ( FmtBaseTestFixture, set_min_int_digits )
    {
        EXPECT_EQ ( min_int_width, 0U );
        EXPECT_EQ ( have_precision, false );

        EXPECT_NO_THROW ( setMinIntegerDigits ( 1 ) );
        EXPECT_EQ ( min_int_width, 1U );
        EXPECT_NO_THROW ( setMinIntegerDigits ( 0 ) );
        EXPECT_EQ ( min_int_width, 0U );
        EXPECT_NO_THROW ( setMinIntegerDigits ( 255 ) );
        EXPECT_EQ ( min_int_width, 255U );
        EXPECT_NO_THROW ( setMinIntegerDigits ( 256 ) );
        EXPECT_EQ ( min_int_width, 255U );

        EXPECT_EQ ( have_precision, true );

        EXPECT_EQ ( radix, 10U );
        EXPECT_EQ ( min_frac_width, 0U );
        EXPECT_EQ ( min_field_width, 0U );
        EXPECT_EQ ( int_left_pad, ' ' );
        EXPECT_EQ ( field_alignment, left );
        EXPECT_EQ ( have_alignment, false );
    }

    TEST_F ( FmtBaseTestFixture, set_min_int_digits_with_fill )
    {
        EXPECT_EQ ( min_int_width, 0U );
        EXPECT_EQ ( int_left_pad, ' ' );
        EXPECT_EQ ( have_precision, false );

        EXPECT_NO_THROW ( setMinIntegerDigits ( 1, 'x' ) );
        EXPECT_EQ ( min_int_width, 1U );
        EXPECT_EQ ( int_left_pad, 'x' );
        EXPECT_NO_THROW ( setMinIntegerDigits ( 0, 'y' ) );
        EXPECT_EQ ( min_int_width, 0U );
        EXPECT_EQ ( int_left_pad, 'y' );
        EXPECT_NO_THROW ( setMinIntegerDigits ( 255 ) );
        EXPECT_EQ ( min_int_width, 255U );
        EXPECT_NO_THROW ( setMinIntegerDigits ( 256 ) );
        EXPECT_EQ ( min_int_width, 255U );
        EXPECT_EQ ( int_left_pad, ' ' );

        EXPECT_EQ ( have_precision, true );

        EXPECT_EQ ( radix, 10U );
        EXPECT_EQ ( min_frac_width, 0U );
        EXPECT_EQ ( min_field_width, 0U );
        EXPECT_EQ ( field_alignment, left );
        EXPECT_EQ ( have_alignment, false );
    }

    TEST_F ( FmtBaseTestFixture, set_min_frac_digits )
    {
        EXPECT_EQ ( min_frac_width, 0U );
        EXPECT_EQ ( have_precision, false );

        EXPECT_NO_THROW ( setMinFractionDigits ( 1 ) );
        EXPECT_EQ ( min_frac_width, 1U );
        EXPECT_NO_THROW ( setMinFractionDigits ( 0 ) );
        EXPECT_EQ ( min_frac_width, 0U );
        EXPECT_NO_THROW ( setMinFractionDigits ( 255 ) );
        EXPECT_EQ ( min_frac_width, 255U );
        EXPECT_NO_THROW ( setMinFractionDigits ( 256 ) );
        EXPECT_EQ ( min_frac_width, 255U );

        EXPECT_EQ ( have_precision, true );

        EXPECT_EQ ( radix, 10U );
        EXPECT_EQ ( min_int_width, 0U );
        EXPECT_EQ ( min_field_width, 0U );
        EXPECT_EQ ( int_left_pad, ' ' );
        EXPECT_EQ ( field_alignment, left );
        EXPECT_EQ ( have_alignment, false );
    }

    TEST_F ( FmtBaseTestFixture, set_min_field_width )
    {
        EXPECT_EQ ( min_field_width, 0U );

        EXPECT_NO_THROW ( setFieldWidth ( 1 ) );
        EXPECT_EQ ( min_field_width, 1U );
        EXPECT_NO_THROW ( setFieldWidth ( 0 ) );
        EXPECT_EQ ( min_field_width, 0U );
        EXPECT_NO_THROW ( setFieldWidth ( 255 ) );
        EXPECT_EQ ( min_field_width, 255U );
        EXPECT_NO_THROW ( setFieldWidth ( 256 ) );
        EXPECT_EQ ( min_field_width, 255U );

        EXPECT_EQ ( radix, 10U );
        EXPECT_EQ ( min_int_width, 0U );
        EXPECT_EQ ( min_frac_width, 0U );
        EXPECT_EQ ( int_left_pad, ' ' );
        EXPECT_EQ ( field_alignment, left );
        EXPECT_EQ ( have_precision, false );
        EXPECT_EQ ( have_alignment, false );
    }

    TEST_F ( FmtBaseTestFixture, set_alignment )
    {
        EXPECT_EQ ( field_alignment, left );
        EXPECT_EQ ( have_alignment, false );

        EXPECT_NO_THROW ( setAlignment ( center ) );
        EXPECT_EQ ( field_alignment, center );
        EXPECT_NO_THROW ( setAlignment ( right ) );
        EXPECT_EQ ( field_alignment, right );
        EXPECT_NO_THROW ( setAlignment ( left ) );
        EXPECT_EQ ( field_alignment, left );
        EXPECT_NO_THROW ( setAlignment ( ( FieldAlign ) ( right + 1 ) ) );
        EXPECT_EQ ( field_alignment, left );

        EXPECT_EQ ( have_alignment, true );

        EXPECT_EQ ( radix, 10U );
        EXPECT_EQ ( min_int_width, 0U );
        EXPECT_EQ ( min_frac_width, 0U );
        EXPECT_EQ ( min_field_width, 0U );
        EXPECT_EQ ( int_left_pad, ' ' );
        EXPECT_EQ ( have_precision, false );
    }

    TEST_F ( FmtBaseTestFixture, put_false )
    {
        EXPECT_NO_THROW ( putBool ( false ) );
        EXPECT_EQ ( str, std :: string ( "false" ) );
    }

    TEST_F ( FmtBaseTestFixture, put_true )
    {
        EXPECT_NO_THROW ( putBool ( true ) );
        EXPECT_EQ ( str, std :: string ( "true" ) );
    }

    TEST_F ( FmtBaseTestFixture, put_SSInt )
    {
        EXPECT_NO_THROW ( putSSInt ( Z16_MAX ) );
        EXPECT_EQ ( str, std :: string ( "32767" ) );
    }

    TEST_F ( FmtBaseTestFixture, put_neg_SSInt )
    {
        EXPECT_NO_THROW ( putSSInt ( Z16_MIN ) );
        EXPECT_EQ ( str, std :: string ( "-32768" ) );
    }

    TEST_F ( FmtBaseTestFixture, put_SInt )
    {
        EXPECT_NO_THROW ( putSInt ( Z32_MAX ) );
        EXPECT_EQ ( str, std :: string ( "2147483647" ) );
    }

    TEST_F ( FmtBaseTestFixture, put_neg_SInt )
    {
        EXPECT_NO_THROW ( putSInt ( Z32_MIN ) );
        EXPECT_EQ ( str, std :: string ( "-2147483648" ) );
    }

#if ARCHBITS == 32
    TEST_F ( FmtBaseTestFixture, put_SLInt )
    {
        EXPECT_NO_THROW ( putSLInt ( Z32_MAX ) );
        EXPECT_EQ ( str, std :: string ( "2147483647" ) );
    }

    TEST_F ( FmtBaseTestFixture, put_neg_SLInt )
    {
        EXPECT_NO_THROW ( putSLInt ( Z32_MIN ) );
        EXPECT_EQ ( str, std :: string ( "-2147483648" ) );
    }
#else
    TEST_F ( FmtBaseTestFixture, put_SLInt )
    {
        EXPECT_NO_THROW ( putSLInt ( Z64_MAX ) );
        EXPECT_EQ ( str, std :: string ( "9223372036854775807" ) );
    }

    TEST_F ( FmtBaseTestFixture, put_neg_SLInt )
    {
        EXPECT_NO_THROW ( putSLInt ( Z64_MIN ) );
        EXPECT_EQ ( str, std :: string ( "-9223372036854775808" ) );
    }
#endif
    TEST_F ( FmtBaseTestFixture, put_SLLInt )
    {
        EXPECT_NO_THROW ( putSLLInt ( Z64_MAX ) );
        EXPECT_EQ ( str, std :: string ( "9223372036854775807" ) );
    }

    TEST_F ( FmtBaseTestFixture, put_neg_SLLInt )
    {
        EXPECT_NO_THROW ( putSLLInt ( Z64_MIN ) );
        EXPECT_EQ ( str, std :: string ( "-9223372036854775808" ) );
    }
#if HAVE_Z128
    TEST_F ( FmtBaseTestFixture, put_SInt128 )
    {
        EXPECT_NO_THROW ( putSInt128 ( Z128_MAX ) );
        EXPECT_EQ ( str, std :: string ( "170141183460469231731687303715884105727" ) );
    }

    TEST_F ( FmtBaseTestFixture, put_neg_SInt128 )
    {
        EXPECT_NO_THROW ( putSInt128 ( Z128_MIN ) );
        EXPECT_EQ ( str, std :: string ( "-170141183460469231731687303715884105728" ) );
    }
#endif

    TEST_F ( FmtBaseTestFixture, put_USInt )
    {
        EXPECT_NO_THROW ( putUSInt ( N16_MAX ) );
        EXPECT_EQ ( str, std :: string ( "65535" ) );
    }

    TEST_F ( FmtBaseTestFixture, put_UInt )
    {
        EXPECT_NO_THROW ( putUInt ( N32_MAX ) );
        EXPECT_EQ ( str, std :: string ( "4294967295" ) );
    }

#if ARCHBITS == 32
    TEST_F ( FmtBaseTestFixture, put_ULInt )
    {
        EXPECT_NO_THROW ( putULInt ( N32_MAX ) );
        EXPECT_EQ ( str, std :: string ( "4294967295" ) );
    }
#else
    TEST_F ( FmtBaseTestFixture, put_ULInt )
    {
        EXPECT_NO_THROW ( putULInt ( N64_MAX ) );
        EXPECT_EQ ( str, std :: string ( "18446744073709551615" ) );
    }
#endif
    TEST_F ( FmtBaseTestFixture, put_ULLInt )
    {
        EXPECT_NO_THROW ( putULLInt ( N64_MAX ) );
        EXPECT_EQ ( str, std :: string ( "18446744073709551615" ) );
    }
#if HAVE_Z128
    TEST_F ( FmtBaseTestFixture, put_UInt128 )
    {
        EXPECT_NO_THROW ( putUInt128 ( N128_MAX ) );
        EXPECT_EQ ( str, std :: string ( "340282366920938463463374607431768211455" ) );
    }
#endif
    TEST_F ( FmtBaseTestFixture, put_F )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 0, '0' ) );
        EXPECT_NO_THROW ( setMinFractionDigits ( 4 ) );
        EXPECT_NO_THROW ( putF ( 1234.56678 ) );
        EXPECT_EQ ( str, std :: string ( "1234.5668" ) );
    }
    TEST_F ( FmtBaseTestFixture, put_D )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 5, '0' ) );
        EXPECT_NO_THROW ( setMinFractionDigits ( 8 ) );
        EXPECT_NO_THROW ( putD ( 1234.56678 ) );
        EXPECT_EQ ( str, std :: string ( "01234.56678000" ) );
    }
    TEST_F ( FmtBaseTestFixture, put_D2 )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 5, '0' ) );
        EXPECT_NO_THROW ( setMinFractionDigits ( 5 ) );
        EXPECT_NO_THROW ( putD ( 1234.56678 ) );
        EXPECT_EQ ( str, std :: string ( "01234.56678" ) );
    }
    TEST_F ( FmtBaseTestFixture, put_D3 )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 4, '0' ) );
        EXPECT_NO_THROW ( setMinFractionDigits ( 5 ) );
        EXPECT_NO_THROW ( putD ( 1234.56678 ) );
        EXPECT_EQ ( str, std :: string ( "1234.56678" ) );
    }
    TEST_F ( FmtBaseTestFixture, binary_conversion )
    {
        EXPECT_NO_THROW ( setRadix ( 2 ) );
        EXPECT_NO_THROW ( putULLInt ( 123456678 ) );
        EXPECT_EQ ( str, std :: string ( "0b111010110111100110010100110" ) );
    }
    TEST_F ( FmtBaseTestFixture, octal_conversion )
    {
        EXPECT_NO_THROW ( setRadix ( 8 ) );
        EXPECT_NO_THROW ( putULLInt ( 123456678 ) );
        EXPECT_EQ ( str, std :: string ( "0o726746246" ) );
    }
    TEST_F ( FmtBaseTestFixture, hex_conversion )
    {
        EXPECT_NO_THROW ( setRadix ( 16 ) );
        EXPECT_NO_THROW ( putULLInt ( 123456678 ) );
        EXPECT_EQ ( str, std :: string ( "0x75bcca6" ) );
    }
    TEST_F ( FmtBaseTestFixture, base36_conversion )
    {
        EXPECT_NO_THROW ( setRadix ( 36 ) );
        EXPECT_NO_THROW ( putULLInt ( 123456678 ) );
        EXPECT_EQ ( str, std :: string ( "21j3t6" ) );
    }
    TEST_F ( FmtBaseTestFixture, left_align_integer )
    {
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( left ) );
        EXPECT_NO_THROW ( putSLLInt ( 1234567 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "1234567             " ) );
    }
    TEST_F ( FmtBaseTestFixture, center_align_integer )
    {
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( center ) );
        EXPECT_NO_THROW ( putSLLInt ( 12345678 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "      12345678      " ) );
    }
    TEST_F ( FmtBaseTestFixture, right_align_integer )
    {
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( right ) );
        EXPECT_NO_THROW ( putSLLInt ( 1234567 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "             1234567" ) );
    }
    TEST_F ( FmtBaseTestFixture, right_align_integer_field_width_dot_fill )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 15, '.' ) );
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( right ) );
        EXPECT_NO_THROW ( putSLLInt ( 1234567 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "     ........1234567" ) );
    }
    TEST_F ( FmtBaseTestFixture, right_align_neg_integer_field_width_dot_fill )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 15, '.' ) );
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( right ) );
        EXPECT_NO_THROW ( putSLLInt ( -1234567 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "     .......-1234567" ) );
    }
    TEST_F ( FmtBaseTestFixture, right_align_neg_integer_field_width_dash_fill )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 15, '-' ) );
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( right ) );
        EXPECT_NO_THROW ( putSLLInt ( -1234567 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "     ------ -1234567" ) );
    }
    TEST_F ( FmtBaseTestFixture, right_align_integer_zero_fill )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 15, '0' ) );
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( right ) );
        EXPECT_NO_THROW ( putSLLInt ( 1234567 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "     000000001234567" ) );
    }
    TEST_F ( FmtBaseTestFixture, right_align_neg_integer_zero_fill )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 15, '0' ) );
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( right ) );
        EXPECT_NO_THROW ( putSLLInt ( -1234567 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "    -000000001234567" ) );
    }
    TEST_F ( FmtBaseTestFixture, left_align_neg_integer_zero_fill )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 15, '0' ) );
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( left ) );
        EXPECT_NO_THROW ( putSLLInt ( -1234567 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "-000000001234567    " ) );
    }
    TEST_F ( FmtBaseTestFixture, center_align_neg_integer_zero_fill )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 15, '0' ) );
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( center ) );
        EXPECT_NO_THROW ( putSLLInt ( -1234567 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "  -000000001234567  " ) );
    }
    TEST_F ( FmtBaseTestFixture, center_align_neg_binary )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 15 ) );
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( center ) );
        EXPECT_NO_THROW ( setRadix ( 2 ) );
        EXPECT_NO_THROW ( putSLLInt ( -1234 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( "    -0b10011010010  " ) );
    }
    TEST_F ( FmtBaseTestFixture, center_align_neg_binary_zero_fill )
    {
        EXPECT_NO_THROW ( setMinIntegerDigits ( 15, '0' ) );
        EXPECT_NO_THROW ( setFieldWidth ( 20 ) );
        EXPECT_NO_THROW ( setAlignment ( center ) );
        EXPECT_NO_THROW ( setRadix ( 2 ) );
        EXPECT_NO_THROW ( putSLLInt ( -1234 ) );
        EXPECT_EQ ( str . size (), 20U );
        EXPECT_EQ ( str, std :: string ( " -0b000010011010010 " ) );
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
