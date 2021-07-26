/*

  vdb3.kfc.fmt-base

 */

#include <vdb3/kfc/fmt-base.hpp>
#include <vdb3/kfc/ascii.hpp>

#include <ctype.h>

#if HAVE_QUADMATH
#include <quadmath.h>
#endif

namespace vdb3
{
    
    void FmtBase :: setRadix ( N32 _radix ) noexcept
    {
        // must be an integer in 2..36
        if ( _radix >= 2 && _radix <= 36 )
            radix = ( N16 ) _radix;
    }
    
    void FmtBase :: setMinIntegerDigits ( N32 count, ASCII fill ) noexcept
    {
        if ( count < 256 )
        {
            min_int_width = ( N16 ) count;
            have_precision = true;
            if ( fill > 0 && fill <= 127 && :: isprint ( fill ) )
                int_left_pad = fill;
        }
    }
    
    void FmtBase :: setMinFractionDigits ( N32 precision ) noexcept
    {
        if ( precision < 256 )
        {
            min_frac_width = ( N16 ) precision;
            have_precision = true;
        }
    }

    void FmtBase :: setFieldWidth ( N32 width ) noexcept
    {
        if ( width < 256 )
            min_field_width = width;
    }

    void FmtBase :: setAlignment ( FieldAlign align ) noexcept
    {
        switch ( align )
        {
        case left:
        case center:
        case right:
            field_alignment = align;
            have_alignment = true;
            break;
        }
    }
    
    void FmtBase :: putBool ( bool val )
    {
        writeField ( val ? ASCIIText ( "true", 4 ) : ASCIIText ( "false", 5 ), right );
    }

    void FmtBase :: putSLLInt ( signed long long int sval )
    {
        // split value into sign and magnitude
        unsigned long long val = sval;
        bool neg = false;
        if ( sval < 0 )
        {
            neg = true;
            val = - sval;
        }

        // declare a text buffer on the stack
        // this is enough space for a radix of 2
        ASCII digits [ sizeof val * 8 ];

        // perform conversion
        size_t i = sizeof digits;
        assert ( 2 <= radix && radix <= 36 );
        do
        {
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ val % radix ];
            val /= radix;
        }
        while ( val != 0 );

        // how many digits were generated
        size_t cur_width = sizeof digits - i;

        writeInt ( ASCIIText ( & digits [ i ], cur_width ), neg, true );
    }

    
#if HAVE_Z128
    void FmtBase :: putSInt128 ( signed __int128 sval )
    {
        // split value into sign and magnitude
        unsigned __int128  val = sval;
        bool neg = false;
        if ( sval < 0 )
        {
            neg = true;
            val = - sval;
        }

        // declare a text buffer on the stack
        // this is enough space for a radix of 2
        ASCII digits [ sizeof val * 8 ];

        // perform conversion
        size_t i = sizeof digits;
        assert ( 2 <= radix && radix <= 36 );
        do
        {
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ val % radix ];
            val /= radix;
        }
        while ( val != 0 );

        // how many digits were generated
        size_t cur_width = sizeof digits - i;

        writeInt ( ASCIIText ( & digits [ i ], cur_width ), neg, true );
    }
#else
    void FmtBase :: putSInt128 ( const Z128 & sval )
    {
        // split value into sign and magnitude
        Z128  val ( sval );
        bool neg = false;
        if ( sval < 0 )
        {
            neg = true;
            val . negate ();
        }

        // declare a text buffer on the stack
        // this is enough space for a radix of 2
        ASCII digits [ sizeof val * 8 ];

        // perform conversion
        size_t i = sizeof digits;
        assert ( 2 <= radix && radix <= 36 );
        do
        {
            Z64 d = val % radix;
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ d ];
            val /= radix;
        }
        while ( val != 0 );

        // how many digits were generated
        size_t cur_width = sizeof digits - i;

        writeInt ( ASCIIText ( & digits [ i ], cur_width ), neg, true );
    }
#endif
    
    void FmtBase :: putULLInt ( unsigned long long int val )
    {
        // declare a text buffer on the stack
        // this is enough space for a radix of 2
        ASCII digits [ sizeof val * 8 ];

        // perform conversion
        size_t i = sizeof digits;
        assert ( 2 <= radix && radix <= 36 );
        do
        {
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ val % radix ];
            val /= radix;
        }
        while ( val != 0 );

        // how many digits were generated
        size_t cur_width = sizeof digits - i;

        writeInt ( ASCIIText ( & digits [ i ], cur_width ), false, true );
    }
    
#if HAVE_Z128
    void FmtBase :: putUInt128 ( unsigned __int128 val )
    {
        // declare a text buffer on the stack
        // this is enough space for a radix of 2
        ASCII digits [ sizeof val * 8 ];

        // perform conversion
        size_t i = sizeof digits;
        assert ( 2 <= radix && radix <= 36 );
        do
        {
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ val % radix ];
            val /= radix;
        }
        while ( val != 0 );

        // how many digits were generated
        size_t cur_width = sizeof digits - i;

        writeInt ( ASCIIText ( & digits [ i ], cur_width ), false, true );
    }
#else
    void FmtBase :: putUInt128 ( const N128 & nval )
    {
        N128 val ( nval );
        
        // declare a text buffer on the stack
        // this is enough space for a radix of 2
        ASCII digits [ sizeof val * 8 ];

        // perform conversion
        size_t i = sizeof digits;
        assert ( 2 <= radix && radix <= 36 );
        do
        {
            N64 d = val % radix;
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ d ];
            val /= radix;
        }
        while ( val != 0 );

        // how many digits were generated
        size_t cur_width = sizeof digits - i;

        writeInt ( ASCIIText ( & digits [ i ], cur_width ), false, true );
    }
#endif
    
    void FmtBase :: putD ( double val )
    {
        // just really couldn't care enough to scrape together
        // a decent floating point formatting function...
        // so rely upon stdclib
        int cur_width = 0;
        ASCII digits [ 1024 ];

        if ( ! have_precision )
        {
            // this is pretty universally expected to be 6 fractional digits
            cur_width = :: snprintf ( digits, sizeof digits, "%lf", val );
        }
        else
        {
            // assemble overall numeral width
            bool sign = val < 0;
            int total_width = sign + min_int_width + min_frac_width + 1;
            if ( int_left_pad == '0' )
            {
                cur_width = :: snprintf ( digits, sizeof digits, "%0*.*lf"
                                          , total_width, min_frac_width, val );
            }
            else
            {
                cur_width = :: snprintf ( digits, sizeof digits, "%*.*lf"
                                          , total_width, min_frac_width, val );
            }
        }

        if ( cur_width < 0 || ( size_t ) cur_width > sizeof digits )
            writeField ( ASCIIText ( "<bad-value>" ), right );
        else
            writeField ( ASCIIText ( digits, cur_width ), right );
    }
    
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128 && HAVE_QUADMATH
    void FmtBase :: putF128 ( __float128 val )
    {
        int cur_width = 0;
        ASCII digits [ 4096 ];

        if ( ! have_precision )
            cur_width = :: quadmath_snprintf ( digits, sizeof digits, "%f", val );
        else
        {
            bool sign = val < 0;
            int total_width = sign + min_int_width + min_frac_width + 1;
            if ( int_left_pad == '0' )
            {
                cur_width = :: quadmath_snprintf ( digits, sizeof digits, "%0*.*f"
                                                   , total_width, min_frac_width, val );
            }
            else
            {
                cur_width = :: quadmath_snprintf ( digits, sizeof digits, "%*.*f"
                                                   , total_width, min_frac_width, val );
            }
        }
        if ( cur_width < 0 || ( size_t ) cur_width > sizeof digits )
            writeField ( ASCIIText ( "<bad-value>" ), right );
        else
            writeField ( ASCIIText ( digits, cur_width ), right );
    }
#else
    void FmtBase :: putLD ( long double val )
    {
        int cur_width = 0;
        ASCII digits [ 4096 ];

        if ( ! have_precision )
            cur_width = :: snprintf ( digits, sizeof digits, "%Lf", val );
        else
        {
            bool sign = val < 0;
            int total_width = sign + min_int_width + min_frac_width + 1;
            if ( int_left_pad == '0' )
            {
                cur_width = :: snprintf ( digits, sizeof digits, "%0*.*Lf"
                                          , total_width, min_frac_width, val );
            }
            else
            {
                cur_width = :: snprintf ( digits, sizeof digits, "%*.*Lf"
                                          , total_width, min_frac_width, val );
            }
        }
        if ( cur_width < 0 || ( size_t ) cur_width > sizeof digits )
            writeField ( ASCIIText ( "<bad-value>" ), right );
        else
            writeField ( ASCIIText ( digits, cur_width ), right );
    }
#endif
    
    void FmtBase :: putPtr ( const Ptr & pval )
    {
        // assign pointer to a size_t
        size_t val = ( size_t ) pval . addr ();
        
        // declare a text buffer on the stack
        // this is enough space for a radix of 16
        // plus a 2 byte prefix
        ASCII digits [ sizeof val * 2 + 2 ];

        // perform conversion
        size_t i = sizeof digits;
        do
        {
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ val & 0xF ];
            val >>= 4;
        }
        while ( i > 2 );

        // prefix as hex
        digits [ 0 ] = '0';
        digits [ 1 ] = 'x';

        writeField ( ASCIIText ( digits, sizeof digits ), right );
    }


    void FmtBase :: writeField ( const TextRgn & val, FieldAlign dflt )
    {
        // the total field width
        count_t total_width = val . count ();

        // determine total padding
        count_t padding = 0;
        if ( total_width < ( count_t ) min_field_width )
            padding = ( count_t ) min_field_width - total_width;

        // determine alignment
        FieldAlign alignment = have_alignment ? field_alignment : dflt;

        // issue left padding
        if ( padding != 0 )
        {
            switch ( alignment )
            {
            case left:
                break;
            case center:
                fill ( ' ', padding - padding / 2 );
                padding /= 2;
                break;
            case right:
                fill ( ' ', padding );
                padding = 0;
                break;
            }
        }

        // issue text
        write ( val );

        // issue right padding
        if ( padding != 0 )
        {
            switch ( alignment )
            {
            case left:
            case center:
                fill ( ' ', padding );
                break;
            case right:
                break;
            }
        }

        // reset defaults
        reset ();
    }
    
    void FmtBase :: writeInt ( const TextRgn & num, bool neg, bool pre )
    {
        // if left-padding with zeros, consider them to be part of number
        count_t zero_fill = 0;
        if ( int_left_pad == '0' && num . count () < ( count_t ) min_int_width )
            zero_fill = ( count_t ) min_int_width - num . count ();
        
        // radix prefix
        const ASCII * p = "";
        if ( pre )
        {
            switch ( radix )
            {
            case 2:
                p = "0b"; break;
            case 8:
                p = "0o"; break;
            case 16:
                p = "0x"; break;
            }
        }
        ASCIIText prefix ( p, ( p [ 0 ] != 0 ) ? 2 : 0 );

        // the number of digits in numeric representation
        count_t total_width = num . count () + zero_fill + prefix . count () + neg;

        // if left-padding with other than zero, calculate "zero_fill" as padding
        if ( int_left_pad != '0' && total_width < ( count_t ) min_int_width )
        {
            zero_fill = ( count_t ) min_int_width - total_width;
            total_width = min_int_width;
        }

        // determine total padding
        count_t padding = 0;
        if ( total_width < ( count_t ) min_field_width )
            padding = ( count_t ) min_field_width - total_width;

        // determine alignment
        FieldAlign alignment = have_alignment ? field_alignment : right;

        // issue left padding
        if ( padding != 0 )
        {
            // there is a case where the numeral left fill is also space
            count_t space_fill = 0;
            if ( zero_fill != 0 && int_left_pad == ' ' )
            {
                space_fill = zero_fill;
                zero_fill = 0;
            }
            
            switch ( alignment )
            {
            case left:
                if ( space_fill != 0 )
                    fill ( ' ', space_fill );
                break;
            case center:
                fill ( ' ', space_fill + padding - padding / 2 );
                padding /= 2;
                break;
            case right:
                fill ( ' ', space_fill + padding );
                padding = 0;
                break;
            }
        }

        // handle left-zero padding
        if ( zero_fill != 0 && int_left_pad != '0' )
        {
            // handle case where somebody set the fill to '-'
            if ( int_left_pad != '-' )
                fill ( int_left_pad, zero_fill );
            else
            {
                if ( zero_fill > 1 )
                    fill ( int_left_pad, zero_fill - 1 );
                fill ( ' ', 1 );
            }
        }

        // issue a sign
        if ( neg )
            write ( ASCIIChar ( '-' ) );

        // issue prefix
        if ( prefix . count () != 0 )
            write ( prefix );

        // handle left-zero padding
        if ( zero_fill != 0 && int_left_pad == '0' )
            fill ( int_left_pad, zero_fill );

        // issue number
        write ( num );

        // issue right padding
        if ( padding != 0 )
        {
            switch ( alignment )
            {
            case left:
            case center:
                fill ( ' ', padding );
                break;
            case right:
                break;
            }
        }

        // reset defaults
        reset ();
    }
    
    FmtBase :: FmtBase () noexcept
    {
        reset ();
    }
    
    FmtBase :: ~ FmtBase () noexcept
    {
        reset ();
    }

    void FmtBase :: reset () noexcept
    {
        radix = 10;
        min_int_width = 0;
        min_frac_width = 0;
        min_field_width = 0;
        int_left_pad = ' ';
        field_alignment = left;
        have_precision = false;
        have_alignment = false;
    }

}
