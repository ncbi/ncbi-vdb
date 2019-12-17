/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#include <klib/extern.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/writer.h>
#include <klib/symbol.h>
#include <klib/time.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>

#include "writer-priv.h"

#include <os-native.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <stdio.h>
#include <assert.h>
#include <va_copy.h>

/* the number of PrintFmt, PrintArg and String
   elements to allocate in function-local storage */
#define LOCAL_FMT_COUNT 64


/* for testing purposes - temporarily define this guy */
#define STDC_COMPATIBILITY 1

/***********************************
 * OLD PRINTF CODE IS NOW INACTIVE *
 ***********************************/
#define USE_OLD_PRINTF 0

/***********************************
 * OUR OLD BUDDY                   *
 ***********************************/
#if WINDOWS
#undef snprintf
#define snprintf _snprintf
#endif

/**************************************
 * NAMING TRICKS TO HAVE BOTH ON HAND *
 **************************************/
#if USE_OLD_PRINTF
#define old_string_vprintf string_vprintf
#define old_string_printf string_printf
#define old_vkfprintf vkfprintf
#define old_kfprintf kfprintf
#else
#define new_string_vprintf string_vprintf
#define new_string_printf string_printf
#define new_vkfprintf vkfprintf
#define new_kfprintf kfprintf
#endif

/************************************
 * BEHAVIORS SENSITIVE TO STDC MODE *
 ************************************/
#if STDC_COMPATIBILITY
#define NULL_STRING_TEXT "(null)"
#else
#define NULL_STRING_TEXT "NULL"
#endif
#define DOUSE_NUM_PREFIX_IF_ZERO              STDC_COMPATIBILITY
#define KEEP_OCT_PREFIX_IF_ZERO               STDC_COMPATIBILITY
#define OCTAL_PREFIX_COUNTS_TOWARD_PRECISION  STDC_COMPATIBILITY
#define HEX_PREFIX_FOLLOWS_CASE               STDC_COMPATIBILITY
/* Present in 2.3.3 (from SLES 9.3), absent in 2.5 (from CentOS 5.6) */
#define EMULATE_SMALLINT_EXTENSION_BUG      ( STDC_COMPATIBILITY && __GLIBC__ != 0 && (__GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 5) ) )
#define ZERO_PAD_ONLY_NUMBERS               ( !STDC_COMPATIBILITY || __GLIBC__ != 0 )

#define USE_LIB_FLOAT 1

static
size_t string_flush ( char *dst, size_t bsize, const KWrtHandler *flush, rc_t *rc, size_t *total )
{
    size_t num_writ, flushed;
    for ( flushed = 0; flushed < bsize; flushed += num_writ )
    {
        * rc = ( * flush -> writer ) ( flush -> data, dst, bsize, & num_writ );
        if ( * rc != 0 )
            break;
        if ( num_writ == 0 )
            break;
    }

    if ( * rc == 0 )
    {
        * total += flushed;
        if ( flushed < bsize )
            memmove ( dst, dst + flushed, bsize - flushed );
    }

    return bsize - flushed;
}

static
rc_t CC string_flush_printf ( char *dst, size_t bsize,
    const KWrtHandler *flush, size_t *num_writ, const char *fmt, ... );

static
rc_t string_flush_vprintf ( char *dst, size_t bsize, const KWrtHandler *flush,
    size_t *num_writ, const char *fmt, va_list args )
{
    rc_t rc;
    size_t sidx, didx, total, sz;

    uint32_t *np;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
    double f64;
#if ! USE_LIB_FLOAT
    uint64_t frac;
    double ftmp;
    uint32_t exponent;
#endif
    const char *cp, *end;
    const String *str;
    const KSymbol *sym;
    const KTime *tm;

    char buff [ 256 ];
    char use_sign, padding;
    const char *hex_digits;
    uint32_t i, j, len, digits, min_field_width, max_field_width, precision;
    bool left_align, comma_separate, have_precision, byte_size, half_size, long_size;
    bool alternate, date_time_zone;
#if ! USE_LIB_FLOAT
    bool trim_trailing_zeros;
#endif

    if ( fmt == NULL )
    {
        rc = RC ( rcText, rcString, rcFormatting, rcParam, rcNull );
        if ( num_writ != NULL )
            * num_writ = 0;
        return rc;
    }

    if ( fmt [ 0 ] == 0 )
    {
        rc = RC ( rcText, rcString, rcFormatting, rcParam, rcEmpty );
        if ( num_writ != NULL )
            * num_writ = 0;
        return rc;
    }

    if ( dst == NULL )
    {
        flush = NULL;
        bsize = 0;
    }

    for ( rc = 0, sidx = didx = total = 0; fmt [ sidx ] != 0 && rc == 0; ++ sidx )
    {
        /* flush buffer */
        if ( didx == bsize && flush != NULL )
        {
            didx = string_flush ( dst, didx, flush, & rc, & total );
            if ( rc != 0 )
                break;
        }

        /* copy until ending NUL or '%' */
        if ( fmt [ sidx ] != '%' )
        {
            if ( didx < bsize )
                dst [ didx ] = fmt [ sidx ];
            ++ didx;
            continue;
        }

        /* process escaped '%' */
        if ( fmt [ ++ sidx ] == '%' )
        {
            if ( didx < bsize )
                dst [ didx ] = '%';
            ++ didx;
            continue;
        }

        digits = 0;

        /* process flags */
        use_sign = 0;
        left_align = comma_separate = alternate = false;
        padding = ' ';

        while ( 1 )
        {
            switch ( fmt [ sidx ] )
            {
            case '-':
                left_align = true;
                padding = ' ';
                ++ sidx;
                continue;
            case '+':
                use_sign = '+';
                ++ sidx;
                continue;
            case ' ':
                if ( use_sign != '+' )
                    use_sign = ' ';
                ++ sidx;
                continue;
            case '0':
                if ( ! left_align )
                    padding = '0';
                ++ sidx;
                continue;
            case ',':
            case '\'':
                comma_separate = true;
                ++ sidx;
                continue;
            case '#':
                alternate = true;
                ++ sidx;
                continue;
            }

            break;
        }

        /* field width */
        min_field_width = 0;
        if ( fmt [ sidx ] == '*' )
        {
            min_field_width = va_arg ( args, unsigned int );
            ++ sidx;
        }
        else while ( isdigit ( fmt [ sidx ] ) )
        {
            min_field_width *= 10;
            min_field_width += fmt [ sidx ++ ] - '0';
        }

        /* precision */
        precision = 0;
        have_precision = false;
        if ( fmt [ sidx ] == '.' )
        {
            padding = ' ';
            have_precision = true;
            if ( fmt [ ++ sidx ] == '*' ) {
                precision = va_arg ( args, unsigned int );
                sidx++;
            } else for ( ; isdigit ( fmt [ sidx ] ); ++ sidx )
            {
                precision *= 10;
                precision += fmt [ sidx ] - '0';
            }
        }

        /* storage size */
        byte_size = half_size = long_size = date_time_zone = false;
        switch ( fmt [ sidx ] )
        {
        case 't':
            byte_size = true;
            ++ sidx;
            break;
        case 'h':
            /* this should not be necessary */
            if ( fmt [  ++ sidx ] != 'h' )
                half_size = true;
            else
            {
                byte_size = true;
                ++ sidx;
            }
            break;
        case 'l':
            long_size = true;
            ++ sidx;
            break;
        case 'z':
            date_time_zone = true;
            if ( sizeof ( size_t ) == sizeof ( uint64_t ) )
                long_size = true;
            ++ sidx;
            break;
        }

        /* format */
#if ! USE_LIB_FLOAT
        trim_trailing_zeros = false;
#endif
        switch ( fmt [  sidx  ] )
        {
        case 'd':
        case 'i':

            /* retrieve argument as signed integer */
            if ( byte_size )
                i64 = ( int8_t ) va_arg ( args, int );
            else if ( half_size )
                i64 = ( int16_t ) va_arg ( args, int );
            else if ( long_size )
                i64 = va_arg ( args, int64_t );
            else
                i64 = va_arg ( args, int32_t );

#if ! STDC_COMPATIBILITY
            /* detect zero */
            if ( i64 == 0 )
                use_sign = 0;
            else
#endif
            /* detect negative */
            if ( i64 < 0 )
            {
                use_sign = '-';
                i64 = - i64;
            }

            i = sizeof buff;

#if ! USE_LIB_FLOAT
        make_signed_integer:
#endif
            /* convert to numeral */
            if ( comma_separate ) for ( -- i, j = 0; ; -- i, ++ j )
            {
                if ( j == 3 )
                {
                    buff [ -- i ] = ',';
                    j = 0;
                }
                buff [ i ] =  ( i64 % 10 ) + '0';
                ++ digits;
                if ( ( i64 /= 10 ) == 0 )
                    break;
            }
            else for ( -- i, digits = 0; ; -- i )
            {
                buff [ i ] =  ( i64 % 10 ) + '0';
                ++ digits;
                if ( ( i64 /= 10 ) == 0 )
                    break;
            }

        insert_integer:

            /* the actual length */
            len = sizeof buff - i;

            /* test for buffer flush */
            if ( flush != NULL && didx < bsize )
            {
                max_field_width = len;
                if ( len < precision )
                    max_field_width = precision;
                max_field_width += ( use_sign != 0 );
                if ( max_field_width < min_field_width )
                    max_field_width = min_field_width;
                if ( didx + max_field_width > bsize )
                {
                    didx = string_flush ( dst, didx, flush, & rc, & total );
                    if ( rc != 0 )
                        break;
                }
            }

            /* insert left-aligned */
            if ( left_align )
            {
                /* sign */
                if ( use_sign != 0 )
                {
                    if ( didx < bsize )
                        dst [ didx ] = use_sign;
                    ++ didx;
                    ++ len;
                }

                /* precision */
                for ( ; digits < precision; ++ didx, ++ len, ++ digits )
                {
                    if ( didx < bsize )
                        dst [ didx ] = '0';
                }

                /* value */
                for ( ; i < sizeof buff; ++ didx, ++ i )
                {
                    if ( didx < bsize )
                        dst [ didx ] = buff [ i ];
                }

                /* padding */
                for ( ; len < min_field_width; ++ didx, ++ len )
                {
                    if ( didx < bsize )
                        dst [ didx ] = ' ';
                }
            }
            /* insert right-aligned */
            else
            {
                /* remove sign from field width */
                if ( use_sign != 0 && min_field_width != 0 )
                    -- min_field_width;

                /* zero padding means issue sign first */
                if ( use_sign != 0 && padding == '0' )
                {
                    if ( didx < bsize )
                        dst [ didx ] = use_sign;
                    ++ didx;
                    use_sign = 0;
                }

                /* adjust length for precision */
                if ( precision > digits )
                    len += precision - digits;

                /* apply padding */
                for ( ; len < min_field_width; ++ didx, -- min_field_width )
                {
                    if ( didx < bsize )
                        dst [ didx ] = padding;
                }

                /* sign */
                if ( use_sign != 0 )
                {
                    if ( didx < bsize )
                        dst [ didx ] = use_sign;
                    ++ didx;
                }

                /* precision */
                for ( ; digits < precision; ++ didx, ++ digits )
                {
                    if ( didx < bsize )
                        dst [ didx ] = '0';
                }

                /* value */
                for ( ; i < sizeof buff; ++ didx, ++ i )
                {
                    if ( didx < bsize )
                        dst [ didx ] = buff [ i ];
                }
            }

            break;

        case 'u':

            /* retrieve argument as unsigned integer */
            if ( byte_size )
                u64 = ( uint8_t ) va_arg ( args, int );
            else if ( half_size )
                u64 = ( uint16_t ) va_arg ( args, int );
            else if ( long_size )
                u64 = va_arg ( args, uint64_t );
            else
                u64 = va_arg ( args, uint32_t );

            /* no sign */
            use_sign = 0;

            i = sizeof buff - 1;

            /* convert to numeral */
            if ( comma_separate ) for ( j = digits = 0; ; -- i, ++ j )
            {
                if ( j == 3 )
                {
                    buff [ -- i ] = ',';
                    j = 0;
                }
                buff [ i ] =  ( u64 % 10 ) + '0';
                ++ digits;
                if ( ( u64 /= 10 ) == 0 )
                    break;
            }
            else for ( digits = 0; ; -- i )
            {
                buff [ i ] =  ( u64 % 10 ) + '0';
                ++ digits;
                if ( ( u64 /= 10 ) == 0 )
                    break;
            }

            goto insert_integer;

        case 'p':

            /* retrieve argument as unsigned integer */
            if ( sizeof ( void* ) == sizeof ( uint32_t ) )
                u64 = va_arg ( args, uint32_t );
            else
                u64 = va_arg ( args, uint64_t );

            goto make_hex_integer;

        case 'x':
        case 'X':

            /* retrieve argument as unsigned integer */
            if ( byte_size )
                u64 = ( uint8_t ) va_arg ( args, int );
            else if ( half_size )
                u64 = ( uint16_t ) va_arg ( args, int );
            else if ( long_size )
                u64 = va_arg ( args, uint64_t );
            else
                u64 = va_arg ( args, uint32_t );

        make_hex_integer:

            /* choose numeric case */
            hex_digits = ( fmt [ sidx ] == 'x' ) ?
                "0123456789abcdefx" : "0123456789ABCDEFX";

            /* no sign */
            use_sign = 0;

            i = sizeof buff - 1;

            /* convert to numeral */
            if ( comma_separate ) for ( j = digits = 0; ; -- i, ++ j )
            {
                if ( j == 3 )
                {
                    buff [ -- i ] = ',';
                    j = 0;
                }
                buff [ i ] =  hex_digits [ u64 & 15 ];
                ++ digits;
                if ( ( u64 >>= 4 ) == 0 )
                    break;
            }
            else for ( digits = 0; ; -- i )
            {
                buff [ i ] =  hex_digits [ u64 & 15 ];
                ++ digits;
                if ( ( u64 >>= 4 ) == 0 )
                    break;
            }

            if ( alternate )
            {
                buff [ -- i ] = hex_digits [ 16 ];
                buff [ -- i ] = '0';
            }

            goto insert_integer;

        case 'o':

            /* retrieve argument as unsigned integer */
            if ( byte_size )
                u64 = ( uint8_t ) va_arg ( args, int );
            else if ( half_size )
                u64 = ( uint16_t ) va_arg ( args, int );
            else if ( long_size )
                u64 = va_arg ( args, uint64_t );
            else
                u64 = va_arg ( args, uint32_t );

            /* no sign */
            use_sign = 0;

            i = sizeof buff - 1;

            /* convert to numeral */
            if ( comma_separate ) for ( j = digits = 0; ; -- i, ++ j )
            {
                if ( j == 3 )
                {
                    buff [ -- i ] = ',';
                    j = 0;
                }
                buff [ i ] =  ( u64 & 7 ) + '0';
                ++ digits;
                if ( ( u64 >>= 3 ) == 0 )
                    break;
            }
            else for ( digits = 0; ; -- i )
            {
                buff [ i ] =  ( u64 & 7 ) + '0';
                ++ digits;
                if ( ( u64 >>= 3 ) == 0 )
                    break;
            }

            if ( alternate )
            {
                buff [ -- i ] = '0';
#if STDC_COMPATIBILITY
                ++ digits;
#endif
            }

            goto insert_integer;

        case 'b':

            /* retrieve argument as unsigned integer */
            if ( byte_size )
                u64 = ( uint8_t ) va_arg ( args, int );
            else if ( half_size )
                u64 = ( uint16_t ) va_arg ( args, int );
            else if ( long_size )
                u64 = va_arg ( args, uint64_t );
            else
                u64 = va_arg ( args, uint32_t );

            /* no sign */
            use_sign = 0;

            i = sizeof buff - 1;

            /* convert to numeral */
            if ( comma_separate ) for ( j = digits = 0; ; -- i, ++ j )
            {
                if ( j == 3 )
                {
                    buff [ -- i ] = ',';
                    j = 0;
                }
                buff [ i ] =  ( u64 & 1 ) + '0';
                ++ digits;
                if ( ( u64 >>= 1 ) == 0 )
                    break;
            }
            else for ( i = sizeof buff - 1; ; -- i )
            {
                buff [ i ] =  ( u64 & 1 ) + '0';
                ++ digits;
                if ( ( u64 >>= 1 ) == 0 )
                    break;
            }

            if ( alternate )
            {
                buff [ -- i ] = 'b';
                buff [ -- i ] = '0';
            }

            goto insert_integer;

#if USE_LIB_FLOAT
        case 'g':
        case 'e':
        case 'f':
        {
            char subfmt [ 16 ];
            char * psubfmt = subfmt;

            *psubfmt++ = '%';

            if (alternate)
                *psubfmt++ = '#';

            if (use_sign)
                *psubfmt++ = use_sign;

            if (left_align)
                *psubfmt++ = '-';
            else if (padding == '0')
                *psubfmt++ = '0';

            *psubfmt++ = '*';
            *psubfmt++ = '.';
            *psubfmt++ = '*';
            *psubfmt++ = fmt [  sidx  ];
            *psubfmt = '\0';
            
            /* retrieve argument as double or long double */
        
            if ( long_size )
                f64 = ( double ) va_arg ( args, long double );
            else
                f64 = va_arg ( args, double );

            if ( ! have_precision )
                precision = 6;
            else if ( precision > 20 )
                precision = 20;

            i = snprintf (buff, sizeof buff, subfmt, min_field_width, precision, f64);

            if ( i >= sizeof buff )
            {
                i = sizeof buff - 1;
                buff [ i ] = '\0';
            }
            min_field_width = 0;
            have_precision = false;

            cp = buff;
            goto make_nul_term_string;
        }
#else
        case 'g':
            trim_trailing_zeros = true;
        case 'e':

#define HANDLE_NAN() \
            switch (fpclassify (f64))                   \
            {                                           \
            case FP_NAN:                                \
                cp = "nan";                             \
                goto make_nul_term_string;              \
            case FP_INFINITE:                           \
                cp = (f64 < 0) ? "-inf" : "inf";        \
                goto make_nul_term_string;              \
            }


            /* retrieve argument as double or long double */
        
            if ( long_size )
                f64 = ( double ) va_arg ( args, long double );
            else
                f64 = va_arg ( args, double );

            HANDLE_NAN();

            if ( f64 < 0 )
            {
                use_sign = '-';
                f64 = - f64;
            }

            exponent = 0;
            buff [ sizeof buff - 3 ] = '+';
            buff [ sizeof buff - 4 ] = 'e';

            i = len = sizeof buff;

            ftmp = f64;

            if ( f64 >= 10.0 )
            {
                for ( ftmp = f64 / 10, exponent = 1; ftmp >= 10.0; ++ exponent )
                    ftmp /= 10;

                if ( exponent < precision && fmt [ sidx ] == 'g' )
                    goto make_normal_float;
            }
            else if ( f64 < 1.0 && f64 > 0.0 )
            {
                buff [ sizeof buff - 3 ] = '-';
                for ( ftmp = f64 * 10, exponent = 1; ftmp < 1.0; ++ exponent )
                    ftmp *= 10;

                if ( exponent <= 4 && fmt [ sidx ] == 'g' )
                    goto make_normal_float;
            }
            else if ( fmt [ sidx ] == 'g' )
            {
                goto make_normal_float;
            }

            /* just for safety */
            exponent %= 100;

            trim_trailing_zeros = false;

            f64 = ftmp;

            buff [ sizeof buff - 1 ] = ( exponent % 10 ) + '0';
            buff [ sizeof buff - 2 ] = ( exponent / 10 ) + '0';

            i = len = sizeof buff - 4;
            goto make_normal_float;

        case 'f':

            /* retrieve argument as double or long double */
            if ( long_size )
                f64 = ( double ) va_arg ( args, long double );
            else
                f64 = va_arg ( args, double );

            HANDLE_NAN();

            if ( f64 < 0 )
            {
                use_sign = '-';
                f64 = - f64;
            }

            i = len = sizeof buff;

        make_normal_float:

            if ( ! have_precision )
                precision = 6;
            else if ( precision > 20 )
                precision = 20;

            ftemp = 0.5;
            for (j = 0; j < precision ; ++j)
                ftemp /= 10;

            f64 += ftemp;


            /* save off integral portion */
            i64 = ( int64_t ) f64;

            /* convert to fraction */
            f64 = f64 - i64;

            /* promote by precision */
            for ( j = 0; j < precision; ++ j )
                f64 *= 10;

            for ( frac = ( uint64_t ) f64; i + precision > len; frac /= 10 )
                buff [ -- i ] = frac % 10 + '0';

            if ( trim_trailing_zeros )
            {
                for ( j = len; -- j >= i; -- precision )
                {
                    if ( buff [ j ] != '0' )
                        break;
                }

                memmove ( & buff [ len - precision ], & buff [ i ], precision );
                i = len - precision;
            }

            if ( precision != 0 || alternate )
                buff [ -- i ] = '.';

            goto make_signed_integer;
#endif
        case 'c':

            /* retrieve as an int */
            u32 = va_arg ( args, unsigned int );
            if ( u32 < 128 )
                buff [ i = sizeof buff - 1 ] = ( char ) u32;
            else
            {
                int dbytes = utf32_utf8 ( buff, & buff [ sizeof buff ], u32 );
                if ( dbytes <= 0 )
                    buff [ i = sizeof buff - 1 ] = '?';
                else
                    memmove ( & buff [ i = sizeof buff - dbytes ], buff, dbytes );
            }

            /* precision makes no sense, but no error */
            precision = 0;

            /* sign makes no sense */
            use_sign = 0;

#if ZERO_PAD_ONLY_NUMBERS
            /* padding is always with spaces */
            padding = ' ';
#endif

            /* other than that, we can treat it as an integer */
            goto insert_integer;

        case 's':

            /* retrieve as a NUL terminated ( or precision limited ) string */
            cp = va_arg ( args, const char* );
            if ( cp == NULL )
                cp = "NULL";

        make_nul_term_string:

            /* in our case, no precision field means unlimited */
            if ( ! have_precision )
                precision = -1;

#if ZERO_PAD_ONLY_NUMBERS
            /* padding is always with spaces */
            padding = ' ';
#endif
            /* test for buffer flush */
            if ( flush != NULL && didx < bsize )
            {
                max_field_width = 0;
                if ( have_precision )
                    max_field_width = precision;
                if ( max_field_width < min_field_width )
                    max_field_width = min_field_width;
                if ( didx + max_field_width > bsize )
                {
                    didx = string_flush ( dst, didx, flush, & rc, & total );
                    if ( rc != 0 )
                        break;
                }
            }

            /* if right aligning with a minimum field width, measure string */
            if ( ! left_align && min_field_width != 0 )
            {
                for ( len = 0; cp [ len ] != 0 && len < precision; ++ len )
                    ( void ) 0;

                for ( ; len < min_field_width; ++ didx, ++ len )
                {
                    if ( didx < bsize )
                        dst [ didx ] = padding;
                }
            }

            /* copy string */
            for ( i = 0; i < precision && cp [ i ] != 0; ++ didx, ++ i )
            {
                if ( flush != NULL && didx == bsize )
                {
                    didx = string_flush ( dst, didx, flush, & rc, & total );
                    if ( rc != 0 )
                        break;
                }

                if ( didx < bsize )
                    dst [ didx ] = cp [ i ];
            }

            if ( rc != 0 )
                break;

            /* apply right padding */
            if ( left_align ) for ( ; i < min_field_width; ++ didx, ++ i )
            {
                if ( flush != NULL && didx == bsize )
                {
                    didx = string_flush ( dst, didx, flush, & rc, & total );
                    if ( rc != 0 )
                        break;
                }

                if ( didx < bsize )
                    dst [ didx ] = ' ';
            }
            break;

            /* String object */
        case 'S':

            /* retrieve as a NUL terminated ( or precision limited ) string */
            str = va_arg ( args, const String* );
            if ( str == NULL )
            {
                cp = "NULL";
                goto make_nul_term_string;
            }

        make_String:

            /* in our case, no precision field means unlimited */
            if ( ! have_precision )
                precision = -1;

            /* test for buffer flush */
            if ( flush != NULL && didx < bsize )
            {
                /* buffer is measured in bytes, while printing
                   widths are measured in characters... */
                max_field_width = ( uint32_t ) str -> size;
                if ( str -> len < min_field_width )
                    max_field_width += min_field_width - str -> len;
                if ( didx + max_field_width > bsize )
                {
                    didx = string_flush ( dst, didx, flush, & rc, & total );
                    if ( rc != 0 )
                        break;
                }
            }

            /* if right aligning with a minimum field width, measure string */
            if ( ! left_align && min_field_width != 0 )
            {
                len = str -> len;
                if ( len > precision )
                    len = precision;

                for ( ; len < min_field_width; ++ didx, ++ len )
                {
                    if ( didx < bsize )
                        dst [ didx ] = ' ';
                }
            }

            cp = str -> addr;
            end = cp + str -> size;

            /* copy string */
            for ( i = 0; i < str -> len && i < precision; ++ i )
            {
                uint32_t ch;
                int sbytes = utf8_utf32 ( & ch, cp, end );
                if ( sbytes <= 0 )
                {
                    if ( sbytes == 0 )
                        rc = RC ( rcText, rcString, rcFormatting, rcData, rcInsufficient );
                    else
                        rc = RC ( rcText, rcString, rcFormatting, rcData, rcCorrupt );
                    break;
                }
                cp += sbytes;

                if ( didx < bsize )
                {
                    int dbytes = utf32_utf8 ( dst + didx, dst + bsize, ch );
                    if ( dbytes > 0 )
                    {
                        didx += dbytes;
                        continue;
                    }
                    if ( dbytes < 0 )
                    {
                        rc = RC ( rcText, rcString, rcFormatting, rcData, rcCorrupt );
                        break;
                    }
                }

                didx += sbytes;
            }

            /* apply right padding */
            if ( left_align ) for ( ; i < min_field_width; ++ didx, ++ i )
            {
                if ( didx < bsize )
                    dst [ didx ] = ' ';
            }
            break;

            /* version number */
        case 'V':

            u32 = va_arg ( args, uint32_t );

            if ( ! have_precision )
            {
                if ( ( u32 & 0xFFFF ) != 0 )
                    precision = 3;
                else if ( ( u32 & 0xFF0000 ) != 0 )
                    precision = 2;
                else
                    precision = 1;
            }

            switch ( precision )
            {
            case 0:
                cp = ""; break;
            case 1:
                cp ="%u"; break;
            case 2:
                cp ="%u.%u"; break;
            default:
                cp ="%u.%u.%u";
            }

            have_precision = false;
            precision = 0;

            rc = string_printf ( buff, sizeof buff, & sz,
                                 cp,
                                 VersionGetMajor ( u32 ),
                                 VersionGetMinor ( u32 ),
                                 VersionGetRelease ( u32 ) );
            if ( rc != 0 )
                break;

            use_sign = 0;
            padding = ' ';
            memmove ( & buff [ i = ( uint32_t ) ( sizeof buff - sz ) ], buff, sz );
            goto insert_integer;

        case 'R':

            rc = va_arg ( args, rc_t );
            sz = KWrtFmt_rc_t ( buff, sizeof buff, alternate ? "#" : "", rc );
            rc = 0; /* reset back to ok */
            assert ( sz < sizeof buff );

            use_sign = 0;
            padding = ' ';
            memmove ( & buff [ i = ( uint32_t ) ( sizeof buff - sz ) ], buff, sz );
            goto insert_integer;

        case 'N':

            /* THIS IS WRONG - FIELD WIDTH AND FRIENDS WILL NOT BE USED */
            sym = va_arg ( args, const KSymbol* );

            if ( sym -> dad != NULL )
            {
                if ( flush != NULL )
                    didx = string_flush ( dst, didx, flush, & rc, & total );

                sz = 0;
                rc = string_flush_printf ( & dst [ didx ], bsize - didx, flush, & sz, "%N:", sym -> dad );
                if ( rc != 0 )
                    break;

                didx += ( uint32_t ) sz;
                if ( flush != NULL )
                    didx = 0;
            }

            str = & sym -> name;
            goto make_String;

        case 'T': /* KTime */

            tm = va_arg ( args, const KTime* );

            sz = 0;

            /* LEGEND
             *  modifier 'h' means do date only
             *  modifier 'l' means date and time
             *  modifier 'z' means date, time and timezone
             *  no modifier means time
             *  precision affects time
             *  leading zero affects time
             */
            if ( date_time_zone || long_size || half_size )
            {
                static char const *months [ 12 ] =
                    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
                static char const *weekdays [ 7 ] =
                    { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
                rc = string_printf ( buff, sizeof buff, & sz, "%s %s %u %u"
                                     , weekdays [ tm -> weekday ]
                                     , months [ tm -> month ]
                                     , tm -> day + 1
                                     , tm -> year
                    );
                if ( rc != 0 )
                    break;
            }

            if ( ! half_size )
            {
                if ( sz != 0 )
                    buff [ sz ++ ] = ' ';

                u64 = sz;

                rc = string_printf ( & buff [ sz ], sizeof buff - sz, & sz,
                                     padding == '0' ? "%02u:%02u:%02u %cM" : "%u:%02u:%02u %cM"
                                     , ( tm -> hour + 11 ) % 12 + 1
                                     , tm -> minute
                                     , tm -> second
                                     , ( tm -> hour < 12 ) ? 'A' : 'P'
                    );
                if ( rc != 0 )
                    break;

                u64 += sz;

                if ( date_time_zone )
                {
                    rc = string_printf ( & buff [ u64 ], sizeof buff - ( size_t ) u64, & sz,
                                         " %+02d", tm -> tzoff / 60 );
                    if ( rc != 0 )
                        break;

                    u64 += sz;
                }

                sz = ( size_t ) u64;
            }

            padding = ' ';
            memmove ( & buff [ i = ( uint32_t ) ( sizeof buff - sz ) ], buff, sz );
            cp = buff;
            goto make_nul_term_string;

        case '!': /* operating system error code: e.g. errno or GetLastError() */
            u32 = va_arg ( args, uint32_t ); /* fetching it unsigned but it can be signed */

            sz = KWrtFmt_error_code ( buff, sizeof buff - 1, u32 );
            assert ( sz < sizeof buff );
            buff [ sz ] = '\0';

            padding  = ' ';
            memmove ( & buff [ i = ( uint32_t ) ( sizeof buff - sz ) ], buff, sz );
            cp = buff;
            goto make_nul_term_string;

        case 'n':

            np = va_arg ( args, uint32_t* );
            if ( np != NULL )
                * np = ( uint32_t ) ( total + didx );
            break;

        case 0:
            -- sidx;
            break;

        default:
            rc = RC ( rcText, rcString, rcFormatting, rcFormat, rcUnrecognized );
        }
    }

    if ( num_writ != NULL )
        * num_writ = total + didx;

    if ( flush != NULL )
    {
        if ( rc == 0 && didx != 0 )
        {
            didx = string_flush ( dst, didx, flush, & rc, & total );
            if ( rc == 0 && didx != 0 )
                rc = RC ( rcRuntime, rcString, rcFormatting, rcTransfer, rcIncomplete );
        }
    }
    else if ( didx < bsize )
        dst [ didx ] = 0;
    else if ( rc == 0 )
        rc = RC ( rcText, rcString, rcFormatting, rcBuffer, rcInsufficient );

    return rc;
}

static
rc_t CC string_flush_printf ( char *dst, size_t bsize,
    const KWrtHandler *flush, size_t *num_writ, const char *fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = string_flush_vprintf ( dst, bsize, flush, num_writ, fmt, args );

    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC old_string_vprintf ( char *dst, size_t bsize,
    size_t *num_writ, const char *fmt, va_list args )
{
    return string_flush_vprintf ( dst, bsize, NULL, num_writ, fmt, args );
}

LIB_EXPORT rc_t CC old_string_printf ( char *dst, size_t bsize, size_t *num_writ, const char *fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = string_flush_vprintf ( dst, bsize, NULL, num_writ, fmt, args );

    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC old_vkfprintf ( const KWrtHandler *out,
    size_t *num_writ, const char * fmt, va_list args )
{
    rc_t rc;

    if ( out == NULL )
    {
        rc = RC ( rcRuntime, rcString, rcFormatting, rcFile, rcNull );
        if ( num_writ != NULL )
            * num_writ = 0;
    }
    else
    {
        char buff [ 4096 ];
        rc = string_flush_vprintf ( buff, sizeof buff, out, num_writ, fmt, args );
        if ( rc != 0 )
            rc = ResetRCContext ( rc, rcRuntime, rcString, rcFormatting );
    }

    return rc;
}

LIB_EXPORT rc_t CC old_kfprintf ( const KWrtHandler *out,
    size_t *num_writ, const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = old_vkfprintf ( out, num_writ, fmt, args );
        
    va_end ( args );

    return rc;
}

/****************************
 * NEW PRINTF IS NOW ACTIVE *
 ****************************/

/* create_overflow
 *  create a buffer so large that it cannot overflow again
 *  copy in existing structures
 *  map self-referential pointers
 */
static
rc_t create_overflow ( KDataBuffer *overflow, const char *fmt_str,
    String **strp, PrintFmt **fmtp, PrintArg **argp,
    uint32_t str_idx, uint32_t fmt_idx, uint32_t arg_idx )
{
    /* measure the format string and infer that
       we can never have more format items than
       characters in the string... */
    size_t fmt_str_size = strlen ( fmt_str );

    /* our size will create a format and argument
       for every byte. this should be a total overkill. */
    size_t buff_size = ( sizeof ** strp + sizeof ** fmtp + sizeof **argp ) * fmt_str_size;

    /* make the buffer */
    rc_t rc = KDataBufferMakeBytes ( overflow, buff_size );
    if ( rc == 0 )
    {
        /* capture pointers to stack structures */
        const String *str = * strp;
        const PrintFmt *fmt = * fmtp;
        const PrintArg *args = * argp;

        /* destination pointers */
        String *dstr = overflow -> base;
        PrintFmt *dfmt = ( void* ) ( dstr + fmt_str_size );
        PrintArg *dargs = ( void* ) ( dfmt + fmt_str_size );

        /* copy existing data */
        memmove ( dstr, str, str_idx * sizeof * dstr );
        memmove ( dfmt, fmt, fmt_idx * sizeof * dfmt );
        memmove ( dargs, args, arg_idx * sizeof * dargs );

        /* if there are any pointers into "str", relocate them */
        if ( str_idx != 0 )
        {
            uint32_t i, j;
            for ( i = j = 0; i < fmt_idx && j < str_idx; ++ i )
            {
                switch ( dfmt [ i ] . type )
                {
                case sptString:
                case sptUCS2String:
                case sptUTF32String:
                    if ( dargs [ i ] . S == & str [ j ] )
                        dargs [ i ] . S = & dstr [ j ++ ];
                    break;
                }
            }
        }

        /* replace pointers */
        * strp = dstr;
        * fmtp = dfmt;
        * argp = dargs;
   }

    return rc;
}


/* parse_format_string
 *  parse format string and args into structured format
 */
static
rc_t parse_format_string ( const char *fmt_str, va_list vargs,
    String **strp, PrintFmt **fmtp, PrintArg **argp, KDataBuffer *overflow )
{
    rc_t rc;
    uint32_t i, str_idx, fmt_idx, arg_idx;

    PrintFmt *fmt = * fmtp;
    PrintArg *args = * argp;

    if ( fmt_str == NULL )
        return RC ( rcText, rcString, rcFormatting, rcParam, rcNull );

    /* loop over format string */
    for ( rc = 0, i = str_idx = fmt_idx = arg_idx = 0; fmt_str [ i ] != 0; ++ i )
    {
        bool alternate, numeric;
        char size_modifier, time_modifier;
        bool has_precision, has_index, infinite_first;

        /* loop to gather literal portions */
        uint32_t start;
        for ( start = i; ; ++ i )
        {
            /* run until we hit start of substitution token
               or until we hit end of format string */
            if ( fmt_str [ i ] != 0 && fmt_str [ i ] != '%' )
                continue;

            /* detect a literal string */
            if ( i != start )
            {
                /* expand into overflow */
                if ( fmt_idx == LOCAL_FMT_COUNT )
                {
                    rc = create_overflow ( overflow, fmt_str,
                        strp, fmtp, argp, str_idx, fmt_idx, arg_idx );
                    if ( rc != 0 )
                        return rc;

                    fmt = * fmtp;
                    args = * argp;
                }

                /* create a text-literal format */
                memset ( & fmt [ fmt_idx ], 0, sizeof fmt [ 0 ] );
                fmt [ fmt_idx ] . u . l . text = & fmt_str [ start ];
                fmt [ fmt_idx ] . u . l . size = i - start;
                fmt [ fmt_idx ] . fmt = spfText;
                fmt [ fmt_idx ] . type = sptLiteral;

                /* increment counter */
                ++ fmt_idx;
            }

            /* detect escape sequence */
            if ( fmt_str [ i ] == 0 || fmt_str [ i + 1 ] != '%' )
                break;

            /* skip over escape */
            start = ++ i;
        }

        /* done when NUL byte is seen */
        if ( fmt_str [ i ] == 0 )
            break;

        /* detect overflow */
        if ( fmt_idx == LOCAL_FMT_COUNT )
        {
            rc = create_overflow ( overflow, fmt_str,
                strp, fmtp, argp, str_idx, fmt_idx, arg_idx );
            if ( rc != 0 )
                return rc;

            fmt = * fmtp;
            args = * argp;
        }

        /* initial format
         *  thousands_separate    = false
         *  add_prefix            = false
         *  force_decimal_point   = false
         *  leave_trailing_zeros  = false
         *  print_time            = false
         *  print_date            = false
         *  print_weekday         = false
         *  print_timezone        = false
         *  hour_24               = false
         *  sign                  = 0
         *  left_fill             = space
         */
        memset ( & fmt [ fmt_idx ], 0, sizeof fmt [ 0 ] );
        fmt [ fmt_idx ] . left_fill = ' ';

        /* scan flags */
        alternate = false;
        while ( 1 )
        {
            switch ( fmt_str [ ++ i ] )
            {
                /* plus and space modify application of sign
                   to signed integer and floating point conversions.
                   plus overrides space. */
            case ' ':
                if ( fmt [ fmt_idx ] . sign == 0 )
            case '+':
                    fmt [ fmt_idx ] . sign = fmt_str [ i ];
                continue;

                /* dash indicates left-alignment. indicate this
                   by setting "left_fill" to NUL. */
            case '-':
                fmt [ fmt_idx ] . left_fill = 0;
                continue;

                /* zero indicates an alternate left-fill for
                   numeric conversions. the zero is inserted before
                   any sign character in { '+', '-' or ' ' }.
                   since "left_fill" is also used to indicate
                   alignment, only store when right aligning. */
            case '0':
                if ( fmt [ fmt_idx ] . left_fill != 0 )
                    fmt [ fmt_idx ] . left_fill = '0';
                continue;

                /* hash indicates that the formatter should use an
                   "alternate" approach. that approach is specific
                   to the format. */
            case '#':
                alternate = true;
                continue;

                /* comma ( or apostrophe outside of US ) indicates
                   that the integer portion of a numeral should use
                   a comma as a thousands separator for legibility. */
            case ',':
            case '\'':
                fmt [ fmt_idx ] . thousands_separate = 1;
                continue;
            }

            /* we've hit a non-flag character */
            break;
        }

        /* minimum field width */
        if ( isdigit ( fmt_str [ i ] ) )
        {
            /* literal */
            fmt [ fmt_idx ] . u . f . min_field_width = fmt_str [ i ] - '0';
            while ( isdigit ( fmt_str [ ++ i ] ) )
            {
                fmt [ fmt_idx ] . u . f . min_field_width *= 10;
                fmt [ fmt_idx ] . u . f . min_field_width += fmt_str [ i ] - '0';
            }
        }
        else if ( fmt_str [ i ] == '*' )
        {
            /* external - we populate the structure directly
               rather than marking the value as external */
            fmt [ fmt_idx ] . u . f . min_field_width = va_arg ( vargs, uint32_t );
            ++ i;
        }

        /* precision */
        has_precision = false;
        if ( fmt_str [ i ] == '.' )
        {
            /* a single dot implies a precision value of 0 */
            has_precision = true;

            if ( isdigit ( fmt_str [ ++ i ] ) )
            {
                /* a literal precision */
                fmt [ fmt_idx ] . u . f . precision = fmt_str [ i ] - '0';
                while ( isdigit ( fmt_str [ ++ i ] ) )
                {
                    fmt [ fmt_idx ] . u . f . precision *= 10;
                    fmt [ fmt_idx ] . u . f . precision += fmt_str [ i ] - '0';
                }
            }
            else if ( fmt_str [ i ] == '*' )
            {
                /* external - again populate directly */
                fmt [ fmt_idx ] . u . f . precision = va_arg ( vargs, uint32_t );
                ++ i;
            }
            else if ( fmt_str [ i ] == '-' )
            {
                /* eat a negative precision - treat as 0 */
                while ( isdigit ( fmt_str [ ++ i ] ) )
                    ( void ) 0;
            }
        }

        /* index - given when parameter is a vector */
        has_index = infinite_first = false;
        if ( fmt_str [ i ] == ':' )
        {
            bool has_start, has_len, has_end, end_is_stop;
            has_start = has_len = has_end = end_is_stop = false;

            /* parameter is taken as a vector,
               with a default index starting at 0 */
            has_index = true;

            if ( isdigit ( fmt_str [ ++ i ] ) )
            {
                /* literal index */
                fmt [ fmt_idx ] . u . f . start_idx = fmt_str [ i ] - '0';
                while ( isdigit ( fmt_str [ ++ i ] ) )
                {
                    fmt [ fmt_idx ] . u . f . start_idx *= 10;
                    fmt [ fmt_idx ] . u . f . start_idx += fmt_str [ i ] - '0';
                }
                has_start = true;
            }
            else switch ( fmt_str [ i ] )
            {
            case '*':
                /* external index, captured directly */
                fmt [ fmt_idx ] . u . f . start_idx = va_arg ( vargs, uint32_t );
                ++ i;
                has_start = true;
                break;
            case '$':
                ++ i;
                has_start = infinite_first = true;
                break;
            }

            /* detect range */
            switch ( fmt_str [ i ] )
            {
                /* given as start-stop */
            case '-':
                end_is_stop = true;

                /* given as start/len */
            case '/':

                has_len = true;

                if ( isdigit ( fmt_str [ ++ i ] ) )
                {
                    /* literal selection length or end */
                    fmt [ fmt_idx ] . u . f . select_len = fmt_str [ i ] - '0';
                    while ( isdigit ( fmt_str [ ++ i ] ) )
                    {
                        fmt [ fmt_idx ] . u . f . select_len *= 10;
                        fmt [ fmt_idx ] . u . f . select_len += fmt_str [ i ] - '0';
                    }
                    has_end = true;
                }
                else switch ( fmt_str [ i ] )
                {
                case '*':
                    /* external selection length or end */
                    fmt [ fmt_idx ] . u . f . select_len = va_arg ( vargs, uint32_t );
                    ++ i;
                    has_end = true;
                    break;
                case '$':
                    ++ i;
                    end_is_stop = false;
                    break;
                case '-':
                    /* negatives are garbage */
                    while ( isdigit ( fmt_str [ ++ i ] ) )
                        ( void ) 0;
                    break;
                default:
                    end_is_stop = false;
                }
                break;
            }

            /* massage values */
            if ( infinite_first )
            {
                /* having an infinite first forces length of 1 */
                fmt [ fmt_idx ] . u . f . select_len = 1;
            }
            else if ( has_len )
            {
                /* if either a number or an external value was given */
                if ( has_end && end_is_stop )
                {
                    /* end <= start is translated to length 1 */
                    if ( fmt [ fmt_idx ] . u . f . select_len <= fmt [ fmt_idx ] . u . f . start_idx )
                        fmt [ fmt_idx ] . u . f . select_len = 1;

                    /* end > start is translated to stated length */
                    else
                        fmt [ fmt_idx ] . u . f . select_len -= fmt [ fmt_idx ] . u . f . start_idx - 1;
                }
            }
            else if ( has_start )
            {
                /* with no end specified, selection length is 1 */
                fmt [ fmt_idx ] . u . f . select_len = 1;
            }
        }

        /* size - one of the greatest leg-irons in the C legacy,
           guess at the parameter size so that va_arg can extract
           it properly. rather than using predictable parameter sizes,
           they are fuzzy. we have ( mostly ) kept them for tradition...
           if no modifier is given, 32-bit is assumed for integer and
           64-bit for floating point. */
        size_modifier = time_modifier = 0;
        switch ( fmt_str [ i ] )
        {
            /* "Tiny" modifier - like "hh" in C format */
        case 't':
            /* "Half" modifier - same as C format */
        case 'h':
            /* "Long" modifier - means 64-bit for integers, otherwise like C */
        case 'l':
            size_modifier = time_modifier = fmt_str [ i ++ ];
            break;
            /* "siZe" modifier - whatever the size of size_t is */
        case 'z':
            ++ i;
            time_modifier = 'z';
            if ( sizeof ( size_t ) == sizeof ( uint64_t ) )
                size_modifier = 'l';
            break;
        }

        /* output format
           describes the formatting to apply on output
           if precision has not been set, give it a default value */
        numeric = false;
        fmt [ fmt_idx ] . pointer_arg = has_index;
        switch ( fmt_str [ i ] )
        {
            /* decimal signed integer */
        case 'd':
        case 'i':
            fmt [ fmt_idx ] . radix = 10;
            fmt [ fmt_idx ] . fmt = spfSignedInt;
            numeric = true;
            if ( ! has_precision )
                fmt [ fmt_idx ] . u . f . precision = 1;
            else if ( fmt [ fmt_idx ] . left_fill == '0' )
                fmt [ fmt_idx ] . left_fill = ' ';
            break;

            /* decimal unsigned integer */
        case 'u':
            fmt [ fmt_idx ] . radix = 10;
        unsigned_int:
            fmt [ fmt_idx ] . fmt = spfUnsigned;
            fmt [ fmt_idx ] . sign = 0;
            numeric = true;
            if ( ! has_precision )
                fmt [ fmt_idx ] . u . f . precision = 1;
            else if ( fmt [ fmt_idx ] . left_fill == '0' )
                fmt [ fmt_idx ] . left_fill = ' ';
            break;

            /* hex unsigned integer */
        case 'x':
            fmt [ fmt_idx ] . add_prefix = alternate;
            fmt [ fmt_idx ] . radix = 16;
            goto unsigned_int;

            /* upper-case hex unsigned integer or pointer value */
        case 'X':
        case 'p':
            fmt [ fmt_idx ] . upper_case_num = 1;
            fmt [ fmt_idx ] . add_prefix = alternate;
            fmt [ fmt_idx ] . radix = 16;
            goto unsigned_int;

            /* octal unsigned integer */
        case 'o':
            fmt [ fmt_idx ] . add_prefix = alternate;
            fmt [ fmt_idx ] . radix = 8;
            goto unsigned_int;

            /* binary unsigned integer */
        case 'b':
            fmt [ fmt_idx ] . add_prefix = alternate;
            fmt [ fmt_idx ] . radix = 2;
            goto unsigned_int;

            /* decimal signed floating point */
        case 'f':
            fmt [ fmt_idx ] . fmt = spfStdFloat;
        fmt_float:
            fmt [ fmt_idx ] . radix = 10;
            fmt [ fmt_idx ] . force_decimal_point = alternate;
            numeric = true;
            if ( ! has_precision )
                fmt [ fmt_idx ] . u . f . precision = 6;
            break;

            /* scientific notation floating point */
        case 'e':
            fmt [ fmt_idx ] . fmt = spfSciFloat;
            goto fmt_float;

            /* "general" floating point */
        case 'g':
            fmt [ fmt_idx ] . leave_trailing_zeros = alternate;
            fmt [ fmt_idx ] . fmt = spfGenFloat;
            goto fmt_float;

            /* character data */
        case 's':
        case 'S':
            fmt [ fmt_idx ] . pointer_arg = 1;
            if ( ! has_precision )
            /* no break */
        case 'c':
                fmt [ fmt_idx ] . u . f . precision = -1;
            fmt [ fmt_idx ] . fmt = spfText;
            break;
            
            /* KSymbol* */
        case 'N':
            fmt [ fmt_idx ] . pointer_arg = 1;
            fmt [ fmt_idx ] . fmt = spfSymbol;
            break;

            /* ver_t */
        case 'V':
            size_modifier = ( sizeof ( ver_t ) == sizeof ( uint32_t ) ) ? 0 : 'l';
            fmt [ fmt_idx ] . min_vers_components = ! has_precision;
            fmt [ fmt_idx ] . fmt = spfVersion;
            break;

            /* rc_t */
        case 'R':
            size_modifier = ( sizeof ( rc_t ) == sizeof ( uint32_t ) ) ? 0 : 'l';
            fmt [ fmt_idx ] . explain_rc = alternate;
            fmt [ fmt_idx ] . fmt = spfRC;
            break;

            /* KTime* */
        case 'T':
            fmt [ fmt_idx ] . pointer_arg = 1;
            fmt [ fmt_idx ] . fmt = spfTime;
            numeric = true;

            /* here, the "size" modifier is interpreted
               as a selector for the output format. perhaps
               it could have been a precision thing, but... */
            switch ( time_modifier )
            {
                /* by itself, 'T' implies print the time */
            case 0:
                fmt [ fmt_idx ] . print_time = 1;
                break;

                /* "half" 'T' means the date */
            case 'h':
                fmt [ fmt_idx ] . print_date = 1;
                break;

                /* "zone" 'T' means long date with time zone */
            case 'z':
                fmt [ fmt_idx ] . print_timezone = 1;
                /* no break */

                /* "long" 'T' means date and time */
            case 'l':
                fmt [ fmt_idx ] . print_time = 1;
                fmt [ fmt_idx ] . print_date = 1;
                fmt [ fmt_idx ] . print_weekday = 1;
                break;
            }
            break;

            /* OS-specific error code:
               int on Unix-like systems
               DWORD on Windows */
        case '!':
            size_modifier = 0;
            fmt [ fmt_idx ] . fmt = spfOSErr;
            break;

#if SUPPORT_PERCENT_N
            /* awful out parameter giving characters printed so far */
        case 'n':
            fmt [ fmt_idx ] . fmt = spfNone;
            break;
#endif
        default:
            return RC ( rcText, rcString, rcFormatting, rcFormat, rcUnrecognized );
        }

#if ZERO_PAD_ONLY_NUMBERS
        /* handle zero padding for non-numeric cases */
        if ( ! numeric && fmt [ fmt_idx ] . left_fill == '0' )
            fmt [ fmt_idx ] . left_fill = ' ';
#endif

        /* argument format */
        if ( has_index )
        {
            /* all arguments MUST be vectors. */
            args [ arg_idx ] . p = va_arg ( vargs, const void* );

            /* discriminate on vector type */
            switch ( fmt_str [ i ] )
            {
                /* signed integer */
            case 'd':
            case 'i':
            case '!':

                if ( infinite_first )
                    return RC ( rcText, rcString, rcFormatting, rcIndex, rcIncorrect );

                /* select from 4 possibilities */
                switch ( size_modifier )
                {
                case 0:
                    fmt [ fmt_idx ] . type = sptSignedInt32Vect;
                    break;
                case 'l':
                    fmt [ fmt_idx ] . type = sptSignedInt64Vect;
                    break;
                case 'h':
                    fmt [ fmt_idx ] . type = sptSignedInt16Vect;
                    break;
                case 't':
                    fmt [ fmt_idx ] . type = sptSignedInt8Vect;
                    break;
                }
                break;

                /* pointer value - cast to 32 or 64-bit unsigned integer */
            case 'p':
                size_modifier = ( sizeof ( void* ) == sizeof ( uint64_t ) ) ? 'l' : 0;
                /* no break */

                /* unsigned integer */
            case 'u':
            case 'x':
            case 'X':
            case 'o':
            case 'b':
            case 'V':
            case 'R':

                if ( infinite_first )
                    return RC ( rcText, rcString, rcFormatting, rcIndex, rcIncorrect );

                switch ( size_modifier )
                {
                case 0:
                    fmt [ fmt_idx ] . type = sptUnsignedInt32Vect;
                    break;
                case 'l':
                    fmt [ fmt_idx ] . type = sptUnsignedInt64Vect;
                    break;
                case 'h':
                    fmt [ fmt_idx ] . type = sptUnsignedInt16Vect;
                    break;
                case 't':
                    fmt [ fmt_idx ] . type = sptUnsignedInt8Vect;
                    break;
                }
                break;

                /* floating point */
            case 'e':
            case 'f':
            case 'g':

                if ( infinite_first )
                    return RC ( rcText, rcString, rcFormatting, rcIndex, rcIncorrect );

                switch ( size_modifier )
                {
                case 0:
                    fmt [ fmt_idx ] . type = sptFloat64Vect;
                    break;
                case 'h':
                    fmt [ fmt_idx ] . type = sptFloat32Vect;
                    break;
                case 'l':
                    fmt [ fmt_idx ] . type = sptFloatLongVect;
                    break;
                }
                break;

                /* character array */
            case 'c':

                /* can't determine last */
                if ( infinite_first )
                    return RC ( rcText, rcString, rcFormatting, rcIndex, rcIncorrect );

                /* length must be bounded */
                if ( fmt [ fmt_idx ] . u . f . select_len == 0 )
                    fmt [ fmt_idx ] . u . f . select_len = 1;

                fmt [ fmt_idx ] . type = sptNulTermString;
                break;

            case 's':
#if STDC_COMPATIBILITY
                if ( args [ arg_idx ] . p == NULL ) {
                    args [ arg_idx ] . p = NULL_STRING_TEXT;
                }
#endif
                /* NUL-terminated string issues */
                if ( args [ arg_idx ] . p != NULL && infinite_first )
                {
                    const char *cp = args [ arg_idx ] . p;

                    /* handle selection of last element */
                    size_t len = strlen ( cp );
                    if ( len != 0 )
                    {
                        fmt [ fmt_idx ] . u . f . start_idx = len - 1;
                        fmt [ fmt_idx ] . u . f . select_len = 1;
                    }
                }

                fmt [ fmt_idx ] . type = sptNulTermString;
                break;

                /* character string */
            case 'S':

                /* if the index starts with last element */
                if ( args [ arg_idx ] . p != NULL && infinite_first )
                {
                    const String *s = args [ arg_idx ] . p;
                    if ( s -> size != 0 )
                    {
                        fmt [ fmt_idx ] . u . f . start_idx = s -> size - 1;
                        fmt [ fmt_idx ] . u . f . select_len = 1;
                    }
                }

                fmt [ fmt_idx ] . type = sptString;
                break;

                /* no other type supports vectors */
            default:
                return RC ( rcText, rcString, rcFormatting, rcFormat, rcIncorrect );
            }
        }
        else
        {
            /* scalar or string arguments */
            switch ( fmt_str [ i ] )
            {
                /* signed integer */
            case 'd':
            case 'i':
            case '!':
                fmt [ fmt_idx ] . type = sptSignedInt;
                switch ( size_modifier )
                {
                case 0:
                    args [ arg_idx ] . d = va_arg ( vargs, int32_t );
                    break;
                case 'l':
                    args [ arg_idx ] . d = va_arg ( vargs, int64_t );
                    break;
#if EMULATE_SMALLINT_EXTENSION_BUG
                case 'h':
                case 't':
                    args [ arg_idx ] . d = va_arg ( vargs, int );
                    break;
#else
                case 'h':
                    args [ arg_idx ] . d = ( int16_t ) va_arg ( vargs, int );
                    break;
                case 't':
                    args [ arg_idx ] . d = ( int8_t ) va_arg ( vargs, int );
                    break;
#endif
                }
                break;

                /* pointer cast to uint64_t */
            case 'p':
                size_modifier = ( sizeof ( void* ) == sizeof ( uint64_t ) ) ? 'l' : 0;
                /* no break */

                /* unsigned integer */
            case 'u':
            case 'x':
            case 'X':
            case 'o':
            case 'b':
            case 'V':
            case 'R':
                fmt [ fmt_idx ] . type = sptUnsignedInt;
                switch ( size_modifier )
                {
                case 0:
                    args [ arg_idx ] . u = va_arg ( vargs, uint32_t );
                    break;
                case 'l':
                    args [ arg_idx ] . u = va_arg ( vargs, uint64_t );
                    break;
                case 'h':
                    args [ arg_idx ] . u = ( uint16_t ) va_arg ( vargs, unsigned int );
                    break;
                case 't':
                    args [ arg_idx ] . u = ( uint8_t ) va_arg ( vargs, unsigned int );
                    break;
                }
                break;

                /* floating point */
            case 'e':
            case 'f':
            case 'g':
                fmt [ fmt_idx ] . type = sptFloat;
                switch ( size_modifier )
                {
                case 0:
                case 'h':
                    args [ arg_idx ] . f = va_arg ( vargs, double );
                    break;
                case 'l':
                    args [ arg_idx ] . f = ( double ) va_arg ( vargs, long double );
                    break;
                default:
                    return RC ( rcText, rcString, rcFormatting, rcFormat, rcIncorrect );
                }
                break;

                /* single character */
            case 'c':
                fmt [ fmt_idx ] . type = sptChar;
                args [ arg_idx ] . c = va_arg ( vargs, uint32_t );
                break;

                /* NUL-terminated string */
            case 's':
                fmt [ fmt_idx ] . type = sptNulTermString;
                args [ arg_idx ] . s = va_arg ( vargs, const char* );
                break;

                /* String object */
            case 'S':
                fmt [ fmt_idx ] . type = sptString;
                args [ arg_idx ] . S = va_arg ( vargs, const String* );
                break;

                /* KSymbol or KTime object */
            case 'N':
            case 'T':
                fmt [ fmt_idx ] . type = sptPointer;
                args [ arg_idx ] . p = va_arg ( vargs, const void* );
                break;

#if SUPPORT_PERCENT_N
            case 'n':
                fmt [ fmt_idx ] . type = sptBytesPrinted;
                args [ arg_idx ] . n = va_arg ( vargs, uint32_t* );
                break;
#endif
            }
        }

        /* account for format and argument */
        ++ fmt_idx;
        ++ arg_idx;
    }

    /* record final fmt */
    if ( rc == 0 )
    {
        if ( fmt_idx == LOCAL_FMT_COUNT )
        {
            rc = create_overflow ( overflow, fmt_str,
                strp, fmtp, argp, str_idx, fmt_idx, arg_idx );
            if ( rc != 0 )
                return rc;

            fmt = * fmtp;
        }

        memset ( & fmt [ fmt_idx ], 0, sizeof fmt [ 0 ] );
    }

    return rc;
}


/* KBufferedWrtHandler
 *  combines buffer and possibly NULL handler
 *  handles flushing, counting, etc.
 */
typedef struct KBufferedWrtHandler KBufferedWrtHandler;
struct KBufferedWrtHandler
{
    const KWrtHandler *handler;
    char *buff;
    size_t bsize;
    size_t cur;
    size_t flushed;
    bool overflow;
};


/* flush_buffer
 *  writes data to handler
 *  or NUL-terminates buffer
 */
static
rc_t flush_buffer ( KBufferedWrtHandler *out )
{
    const KWrtHandler *flush = out -> handler;
    if ( flush != NULL  && flush -> writer != NULL )
    {
        rc_t rc = 0;
        size_t num_writ, flushed;

/*        assert ( out -> cur != 0 ); */

        for ( flushed = 0; flushed < out -> cur; flushed += num_writ )
        {
            rc = ( * flush -> writer ) ( flush -> data,
                & out -> buff [ flushed ], out -> cur - flushed, & num_writ );
            if ( rc != 0 )
                break;
            if ( num_writ == 0 )
                break;
        }

        if ( flushed == 0 )
            return rc;

        out -> flushed += flushed;
        if ( flushed < out -> cur )
            memmove ( out -> buff, & out -> buff [ flushed ], out -> cur - flushed );
        out -> cur -= flushed;

        return 0;
    }

    /* NB - header file states that space for NUL byte
       is REQUIRED, so this test will declare overflow
       whenever there is not space for the NUL termination */
    if ( out -> cur >= out -> bsize )
        out -> overflow = true;
    else
        out -> buff [ out -> cur ] = 0;

    return 0;
}

static
rc_t write_buffer ( KBufferedWrtHandler *out, const char *data, size_t bytes )
{
    size_t total, num_writ;
    for ( total = 0; total < bytes; out -> cur += num_writ, total += num_writ )
    {
        if ( ! out -> overflow && out -> cur == out -> bsize )
        {
            rc_t rc = flush_buffer ( out );
            if ( rc != 0 )
                return rc;
        }
        if ( out -> overflow )
            num_writ = bytes - total;
        else
        {
            num_writ = out -> bsize - out -> cur;
            if ( total + num_writ > bytes )
                num_writ = bytes - total;
            memmove ( & out -> buff [ out -> cur ], & data [ total ], num_writ );
        }
    }
    return 0;
}


/* print_padding
 *  issue padding characters
 */
static
rc_t print_padding ( KBufferedWrtHandler *out, size_t count, char pad_char )
{
    size_t total, num_writ;
    for ( total = 0; total < count; out -> cur += num_writ, total += num_writ )
    {
        if ( ! out -> overflow && out -> cur == out -> bsize )
        {
            rc_t rc = flush_buffer ( out );
            if ( rc != 0 )
                return rc;
        }
        if ( out -> overflow )
            num_writ = count - total;
        else
        {
            size_t i = out -> cur;
            size_t lim = out -> cur + count - total;
            if ( lim > out -> bsize )
                lim = out -> bsize;
            for ( num_writ = lim - i; i < lim; ++ i )
                out -> buff [ i ] = pad_char;
        }
    }
    return 0;
}

/* print_nul_term_string
 */
static
rc_t print_nul_term_string ( KBufferedWrtHandler *out, String *S, size_t precision )
{
    size_t total, num_writ;
    const char *s = S -> addr;

    for ( S -> len = 0, total = 0; total < precision && s [ 0 ] != 0;
          s += num_writ, out -> cur += num_writ, total += num_writ )
    {
        char *buff;
        size_t lim;

        if ( ! out -> overflow && out -> cur == out -> bsize )
        {
            rc_t rc = flush_buffer ( out );
            if ( rc != 0 )
                return rc;
        }

        if ( out -> overflow )
        {
            lim = precision - total;

            for ( num_writ = 0; num_writ < lim; ++ num_writ )
            {
                if ( s [ num_writ ] == 0 )
                    break;

                /* count characters as well as bytes */
                if ( ( s [ num_writ ] & 0xC0 ) != 0x80 )
                    ++ S -> len;
            }
        }
        else
        {
            buff = out -> buff + out -> cur;
            lim = out -> bsize - out -> cur;

            if ( total + lim > precision )
                lim = precision - total;

            for ( num_writ = 0; num_writ < lim; ++ num_writ )
            {
                if ( s [ num_writ ] == 0 )
                    break;

                /* count characters as well as bytes */
                if ( ( ( buff [ num_writ ] = s [ num_writ ] ) & 0xC0 ) != 0x80 )
                    ++ S -> len;
            }
        }
    }

    S -> size = total;

    return 0;
}

/* print_string
 */
static
rc_t print_string ( KBufferedWrtHandler *out, const String *s, size_t precision )
{
    /* TBD - this is only correct for ASCII */
    if ( s -> size < precision )
        precision = s -> size;

    return write_buffer ( out, s -> addr, precision );
}

static
rc_t print_symbol ( KBufferedWrtHandler *out, const KSymbol *sym )
{
    if ( sym -> dad != NULL )
    {
        rc_t rc = print_symbol ( out, sym -> dad );
        if ( rc == 0 )
            rc = print_padding ( out, 1, ':' );
        if ( rc != 0 )
            return rc;
    }
    return print_string ( out, & sym -> name, sym -> name . size );
}

static
rc_t print_literal ( KBufferedWrtHandler *out, const char *text, size_t size )
{
    return write_buffer ( out, text, size );
}


/* structured_print_engine
 *  prints into a buffer
 *  if an output handler is provided, the buffer will be flushed to it as needed
 *  returns the number of BYTES written, but not the number of characters.
 */
static const char* alnum_set [ 2 ] [ 2 ] =
{
    /* "normal" alpha-numeric ordering for rendering numerals */
    {
        "0123456789abcdefghijklmnopqrstuvwxyz",
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    },
    /* "abnormal" alpha-numeric ordering used by SFF format */
    {
        "abcdefghijklmnopqrstuvwxyz0123456789",
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    }
};

static
rc_t structured_print_engine ( KBufferedWrtHandler *out,
    const PrintFmt *fmt, const PrintArg *args )
{
    rc_t rc;
    uint32_t fmt_idx, arg_idx;

    /* declare these here to stifle the compiler
       they should most naturally be declared within the loop,
       but since they are conditionally initialized by the switch,
       the compiler warns they may be used before initialization.
       rather than quiet the complaint within the loop, they have
       been manually hoisted outside and initialized once. */
    uint32_t i = 0;
    int64_t i64 = 0;
    uint64_t u64 = 0;
    double f64 = 0;
    const void *p = NULL;

    /* most of the arguments are going to be pulled into "S" */
    String S;
    StringInit ( & S, NULL, 0, 0 );

    /* mark no overflow */
    out -> overflow = false;

    /* loop across all format structures */
    for ( rc = 0, out -> cur = out -> flushed = 0, fmt_idx = arg_idx = 0;
          fmt [ fmt_idx ] . type != sptTerm; ++ fmt_idx )
    {
        /* copy entry - we're going to modify it */
        PrintFmt f = fmt [ fmt_idx ];

        /* ENGINE PARAMETERS */

        /* padding to left side of field */
        uint32_t left_pad;

        /* sign character if not NUL */
        char sign_char;

        /* prefix for unsigned int */
        char prefix [ 4 ];
        uint32_t prefix_len;

        /* true if performing comma separation */
        bool comma_sep;

        /* zero-fill, from either integer precision or min-field-width */
        uint32_t zero_fill;

        /* length of integer portion of numeral, prefix contribution */
        uint32_t int_len = 0, prefix_contribution;

        /* string index, from index and text precision */
        bool text_index;
        uint64_t text_start, text_lim;

        /* END ENGINE PARAMETERS */

        int cvt_len;
        uint32_t c, *n;

        const KTime *tm;
        const KSymbol *N;

        char text [ 1024 ];
        uint32_t dst_len;

        char ffmt [ 16 ];
        const char *to_numeral, *cfmt;

        /* detect literal */
        if ( f . type == sptLiteral )
        {
            rc = print_literal ( out, f . u . l . text, f . u . l . size );
            if ( rc != 0 )
                break;

            continue;
        }

        /* gather external parameters */
        if ( f . ext_field_width )
             f . u . f . min_field_width = args [ arg_idx ++ ] . u;
        if ( f . ext_precision )
            f . u . f . precision = args [ arg_idx ++ ] . u;
        if ( f . ext_start_index )
            f . u . f . start_idx = args [ arg_idx ++ ] . u;
        if ( f . ext_select_len )
            f . u . f . select_len = args [ arg_idx ++ ] . u;
        else if ( f . ext_stop_index )
        {
            f . u . f . select_len = args [ arg_idx ++ ] . u;
            if ( f . u . f . select_len <= f . u . f . start_idx )
                f . u . f . select_len = 1;
            else
                f . u . f . select_len -= f . u . f . start_idx - 1;
        }

        /* early initialization */
        text_index = false;
        text_start = 0;
        text_lim = -1;

        /* vector parameters may have NULL pointers under certain
           circumstances, e.g. pre-calculated format with out-of-bounds
           index coordinates, when this is detectable. */
        if ( f . pointer_arg && args [ arg_idx ] . p == NULL )
        {
            if ( f . fmt == spfText )
            {
                CONST_STRING ( & S, NULL_STRING_TEXT );
                f . type_cast = 0;
                f . type = sptString;
                if ( f . u . f . precision < S . len )
#if STDC_COMPATIBILITY  &&  !defined(__GLIBC__)
                    S . size = f . u . f . precision;
                    S . len = (uint32_t) f . u . f . precision;
#else
                    StringInit ( & S, "", 0, 0 );
#endif                    
            }
            else
            {
                memset ( & f, 0, sizeof f );
                f . u . f . precision = 1;
                f . radix = 10;
                f . fmt = spfUnsigned;
                f . type = sptUnsignedInt;
                u64 = 0;
            }
        }

        /* get source */
        else switch ( f . type )
        {
            /* scalar or vector signed integer */
        case sptSignedInt:
            i64 = args [ arg_idx ] . d;
            break;
        case sptSignedInt8Vect:
            i64 = args [ arg_idx ] . d8 [ f . u . f . start_idx ];
            break;
        case sptSignedInt16Vect:
            i64 = args [ arg_idx ] . d16 [ f . u . f . start_idx ];
            break;
        case sptSignedInt32Vect:
            i64 = args [ arg_idx ] . d32 [ f . u . f . start_idx ];
            break;
        case sptSignedInt64Vect:
            i64 = args [ arg_idx ] . d64 [ f . u . f . start_idx ];
            break;

            /* scalar or vector unsigned integer */
        case sptUnsignedInt:
            u64 = args [ arg_idx ] . u;
            break;
        case sptUnsignedInt8Vect:
            u64 = args [ arg_idx ] . u8 [ f . u . f . start_idx ];
            break;
        case sptUnsignedInt16Vect:
            u64 = args [ arg_idx ] . u16 [ f . u . f . start_idx ];
            break;
        case sptUnsignedInt32Vect:
            u64 = args [ arg_idx ] . u32 [ f . u . f . start_idx ];
            break;
        case sptUnsignedInt64Vect:
            u64 = args [ arg_idx ] . u64 [ f . u . f . start_idx ];
            break;

            /* scalar or vector float */
        case sptFloat:
            f64 = args [ arg_idx ] . f;
            break;
        case sptFloat32Vect:
            f64 = args [ arg_idx ] . f32 [ f . u . f . start_idx ];
            break;
        case sptFloat64Vect:
            f64 = args [ arg_idx ] . f64 [ f . u . f . start_idx ];
            break;
        case sptFloatLongVect:
            f64 = ( double ) args [ arg_idx ] . flong [ f . u . f . start_idx ];
            break;

            /* utf-8 character */
        case sptChar:
            c = args [ arg_idx ] . c;
            cvt_len = utf32_utf8 ( text, & text [ sizeof text ], c );
            if ( cvt_len < 0 )
            {
                text [ 0 ] = '?';
                cvt_len = 1;
            }
            StringInit ( & S, text, cvt_len, 1 );
            break;

            /* NUL-terminated string */
        case sptNulTermString:

            /* special initialization to flag size/length unknown */
            StringInit ( & S, args [ arg_idx ] . s, 0, -1 );

            /* IF THE STRING IS INDEXED OR MAY NEED LEFT ALIGNMENT */
            if ( f . u . f . start_idx != 0 || f . u . f . select_len != 0 ||
                 ( f . u . f . min_field_width != 0 && f . left_fill ) )
            {
                /* precision limits upper bounds
                   if the index range is more restrictive than precision,
                   take this as precision for text case */
                if ( f . u . f . select_len != 0 && f . u . f . select_len < f . u . f . precision )
                    f . u . f . precision = f . u . f . select_len;

                /* scan to start location */
                for ( S . len = 0; S . len < f . u . f . start_idx; ++ S . size )
                {
                    /* if end of NUL-terminated string is reached before start,
                       then create EMPTY string */
                    if ( S . addr [ S . size ] == 0 )
                    {
                        f . u . f . start_idx = 0;
                        f . u . f . select_len = 0;
                        f . u . f . precision = 0;
                        break;
                    }
                    /* walk across utf-8 multi-byte character */
                    if ( ( S . addr [ S . size ] & 0xC0 ) != 0x80 )
                        ++ S . len;
                }

                /* create sub-string
                   limit scan to required precision for efficiency */
                for ( S . addr += S . size, S . size = 0, S . len = 0;
                      S . len < f . u . f . precision; ++ S . size )
                {
                    if ( S . addr [ S . size ] == 0 )
                        break;
                    if ( ( S . addr [ S . size ] & 0xC0 ) != 0x80 )
                        ++ S . len;
                }
            }

            /* precision limits text length */
            if ( text_lim > f . u . f . precision )
                text_lim = f . u . f . precision;

            break;

            /* String object */
        case sptString:

            /* the String may be indexed */
            if ( f . u . f . start_idx != 0 || f . u . f . select_len != 0 )
            {
                text_index = true;
                text_start = f . u . f . start_idx;
                if ( f . u . f . select_len != 0 )
                    text_lim = f . u . f . select_len;
            }

            /* precision limits text length */
            if ( text_lim > f . u . f . precision )
                text_lim = f . u . f . precision;

            S = * args [ arg_idx ] . S;
            if ( S . addr == NULL && S . size == 0 && S . len == 0 )
                S . addr = "";
            break;

            /* UCS-2 or UTF-32 String object */
        case sptUCS2String:
        case sptUTF32String:
            return RC ( rcText, rcString, rcConverting, rcType, rcUnsupported );

            /* an object pointer */
        case sptPointer:
            p = args [ arg_idx ] . p;
            break;

        case sptRowId:
            i64 = args [ arg_idx ] . d;
            break;
        case sptRowLen:
            u64 = args [ arg_idx ] . u;
            break;

#if SUPPORT_PERCENT_N
        case sptBytesPrinted:
            n = args [ arg_idx ] . n;
            if ( n != NULL )
                * n = (uint32_t) ( out -> cur + out -> flushed );
            ++ arg_idx;
            continue;
#endif
        default:
            return RC ( rcText, rcString, rcConverting, rcType, rcUnrecognized );
        }

        if ( f . type_cast ) switch ( f . type )
        {
        case sptSignedInt:
        case sptSignedInt8Vect:
        case sptSignedInt16Vect:
        case sptSignedInt32Vect:
        case sptSignedInt64Vect:
            f64 = (double) i64;
            break;

        case sptUnsignedInt:
        case sptUnsignedInt8Vect:
        case sptUnsignedInt16Vect:
        case sptUnsignedInt32Vect:
        case sptUnsignedInt64Vect:
            f64 = (double) u64;
            break;

        case sptFloat:
        case sptFloat32Vect:
        case sptFloat64Vect:
        case sptFloatLongVect:
            i64 = (int64_t) f64;
            break;

        case sptChar:
        case sptNulTermString:
        case sptString:
            break;

            /* UCS-2 or UTF-32 String object */
        case sptUCS2String:
        case sptUTF32String:
        case sptPointer:
            return RC ( rcText, rcString, rcConverting, rcType, rcUnsupported );

        case sptRowId:
            f64 = (double) i64;
            break;
        case sptRowLen:
            f64 = (double) u64;
            break;

#if SUPPORT_PERCENT_N
        case sptBytesPrinted:
            break;
#endif
        default:
            return RC ( rcText, rcString, rcConverting, rcType, rcUnrecognized );
        }

        /* advance arg count */
        ++ arg_idx;

        /* initialize engine params */
        sign_char = 0;
        comma_sep = false;
        left_pad = prefix_len = prefix_contribution = zero_fill = 0;

        /* format */
        switch ( f . fmt )
        {
        case spfNone:
        case spfText:
            break;

        case spfSignedInt:
            if ( i64 < 0 )
            {
                sign_char = '-';
                u64 = - i64;
            }
            else
            {
                sign_char = f . sign;
                u64 = i64;
            }
            f . add_prefix = 0;
            /* no break */
        case spfUnsigned:
            to_numeral = alnum_set [ f . reverse_alnum ] [ f . upper_case_num ];

            if ( f . radix > 36 )
                f . radix = 10;

            i = sizeof text;
#if _DEBUGGING
            text [ -- i ] = 0;
#endif
            /* record if we are comma separating below */
            comma_sep = f . thousands_separate;

#if DOUSE_NUM_PREFIX_IF_ZERO
            if ( u64 == 0 && f . add_prefix )
            {
#if KEEP_OCT_PREFIX_IF_ZERO
                if ( f . u . f . precision == 0 && f . radix == 8 )
                    f . u . f . precision = 1;
#endif
                f . add_prefix = 0;
            }
#endif
            /* special case for value 0 and precision 0 */
            if ( u64 != 0 || f . u . f . precision != 0 )
            {
                /* convert numeral */
                do
                    text [ -- i ] = to_numeral [ u64 % f . radix ];
                while ( ( u64 /= f . radix ) != 0 );

                /* add prefix to unsigned numeral */
                if ( f . add_prefix )
                {
                    switch ( f . radix )
                    {
                    case 2:
                        memmove ( prefix, "0b", prefix_len = 2 );
                        break;
                    case 8:
                        memmove ( prefix, "0", prefix_len = prefix_contribution = 1 );
#if OCTAL_PREFIX_COUNTS_TOWARD_PRECISION
                        if ( f . add_prefix && f . u . f . precision != 0 )
                            -- f . u . f . precision;
#endif
                        break;
                    case 16:
#if HEX_PREFIX_FOLLOWS_CASE
                        if ( to_numeral [ 10 ] == 'A' )
                            memmove ( prefix, "0X", prefix_len = 2 );
                        else
#endif
                            memmove ( prefix, "0x", prefix_len = 2 );
                        break;
                    }
                }
            }

            /* determine length in characters, also size in bytes */
#if _DEBUGGING
            int_len = sizeof text - i - 1;
#else
            int_len = sizeof text - i;
#endif
            /* create text string */
            StringInit ( & S, & text [ i ], int_len, int_len );

            /* zero-fill amount */
            if ( f . u . f . precision > ( uint64_t ) int_len )
                zero_fill = ( uint32_t ) ( f . u . f . precision - ( uint64_t ) int_len );
            else if ( f . left_fill == '0' )
            {
                /* the known characters associated with integer */
                dst_len = int_len + prefix_len + ( sign_char != 0 );
                if ( comma_sep && int_len != 0 )
                    dst_len += ( int_len + prefix_contribution - 1 ) / 3;
                if ( ( uint64_t ) dst_len < f . u . f . min_field_width )
                {
                    /* determine the numeric width, including zero padding */
                    dst_len = ( uint32_t ) f . u . f . min_field_width -
                        prefix_len + prefix_contribution - ( sign_char != 0 );
                    if ( comma_sep && int_len != 0 )
                    {
                        /* desired numeric-only portion of field is "dst_len"
                           it will be extended not only by some number of '0's,
                           but will be sprinkled with commas.

                           if "x" were our number of numerals, then:
                             x + ( x - 1 ) / 3 = dst_len
                           with allowances for integer math. solving for x:
                             x = ( 3 * dst_len + 3 ) / 4

                           for example: given int_len = 1, value = 0:
                             dst_len =  1 : x =  1
                             dst_len =  2 : x =  2
                             dst_len =  3 : x =  3
                             dst_len =  4 : x =  3 [ requires special compensation ]
                             dst_len =  5 : x =  4
                             dst_len =  6 : x =  5
                             dst_len =  7 : x =  6
                             dst_len =  8 : x =  6 [ requires special compensation ]
                                . . .
                             dst_len = 19 : x = 15
                             dst_len = 20 : x = 15
                             dst_len = 21 : x = 16
                             dst_len = 22 : x = 17
                        */

                        /* apply compensation for impossible width */
                        if ( ( dst_len & 3 ) == 0 )
                            ++ left_pad;

                        /* dst_len now becomes the number of numerals */
                        dst_len = ( dst_len * 3 + 3 ) >> 2;
                    }

                    /* number of zeros to add */
                    zero_fill = dst_len - int_len;
                    prefix_len -= prefix_contribution;
                }
            }

            break;

            /* TEMPORARY - cheat by calling through to runtime sprintf
               this needs to be implemented to handle thousands separation */
        case spfStdFloat:
            c = 'f';
        fmt_float:
            sign_char = f . sign;
            if ( f64 < 0 )
            {
                sign_char = '-';
                f64 = - f64;
            }

            ffmt [ i = 0 ] = '%';
            if ( f . force_decimal_point )
                ffmt [ ++ i ] = '#';
            if ( f . u . f . precision > 20 )
                f . u . f . precision = 20;
            sprintf ( & ffmt [ ++ i ], ".%u%c"
                      , ( uint32_t ) f . u . f . precision
                      , ( char ) c );
            cvt_len = snprintf ( text, sizeof text, ffmt, f64 );
            assert ( cvt_len >= 0 && ( size_t ) cvt_len < sizeof text );
            StringInit ( & S, text, cvt_len, cvt_len );

            /* record if we are comma separating below and capture length of integer portion */
            int_len = 0;
            comma_sep = f . thousands_separate;
            /* if ( comma_sep ) */
            {
                for ( ; isdigit ( text [ int_len ] ); ++ int_len )
                    ( void ) 0;
            }

            /* zero-fill amount */
            if ( f . left_fill == '0' )
            {
                /* the known characters associated with numeral */
                dst_len = cvt_len + ( sign_char != 0 );
                if ( comma_sep && int_len != 0 )
                    dst_len += ( int_len - 1 ) / 3;
                if ( ( uint64_t ) dst_len < f . u . f . min_field_width )
                {
                    /* determine the integer width, including zero padding */
                    dst_len = ( uint32_t ) f . u . f . min_field_width -
                        cvt_len + int_len - ( sign_char != 0 );
                    if ( comma_sep && int_len != 0 )
                    {
                        /* SEE COMMENTS IN INTEGER SECTION */

                        /* apply compensation for impossible width */
                        if ( ( dst_len & 3 ) == 0 )
                            ++ left_pad;

                        /* dst_len now becomes the number of numerals */
                        dst_len = ( dst_len * 3 + 3 ) >> 2;
                    }

                    /* number of zeros to add */
                    zero_fill = dst_len - int_len;
                }
            }
            break;

        case spfGenFloat:
            c = 'g';
            goto fmt_float;

        case spfSciFloat:
            c = 'e';
            goto fmt_float;

            /* version */
        case spfVersion:

            if ( f . min_vers_components )
            {
                if ( ( u64 & 0xFFFF ) != 0 )
                    f . u . f . precision = 3;
                else if ( ( u64 & 0xFF0000 ) != 0 )
                    f . u . f . precision = 2;
                else
                    f . u . f . precision = 1;
            }

            switch ( f . u . f . precision )
            {
            case 0:
                cfmt = "";
                break;
            case 1:
                cfmt = "%u";
                break;
            case 2:
                cfmt = "%u.%u";
                break;
            default:
                cfmt = "%u.%u.%u";
            }

            dst_len = sprintf ( text, cfmt
                                , VersionGetMajor ( ( uint32_t ) u64 )
                                , VersionGetMinor ( ( uint32_t ) u64 )
                                , VersionGetRelease ( ( uint32_t ) u64 )
                );
            StringInit ( & S, text, dst_len, dst_len );
            f . u . f . precision = dst_len;
            break;

        case spfSymbol:

            /* calculate total size of symbol */
            for ( N = p, S = N -> name, N = N -> dad; N != NULL; N = N -> dad )
            {
                S . len += N -> name . len + 1;
                S . size += N -> name . size + 1;
            }

            /* mark need to recheck format */
            S . addr = NULL;
            break;


        case spfTime:

            tm = p;

            dst_len = 0;
            if ( f . print_date )
            {
                static char const *months [ 12 ] =
                    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
                if ( f . print_weekday )
                {
                    static char const *weekdays [ 7 ] =
                        { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
                    dst_len = sprintf ( text, "%s %s %u %u"
                                        , weekdays [ tm -> weekday ]
                                        , months [ tm -> month ]
                                        , tm -> day + 1
                                        , tm -> year
                        );
                }
                else
                {
                    dst_len = sprintf ( text, "%s %u %u"
                                        , months [ tm -> month ]
                                        , tm -> day + 1
                                        , tm -> year
                        );
                }
            }

            if ( f . print_time )
            {
                if ( dst_len != 0 )
                    text [ dst_len ++ ] = ' ';

                if ( f . hour_24 )
                {
                    dst_len += sprintf ( & text [ dst_len ]
                                         , f . left_fill == '0' ? "%02u:%02u:%02u" : "%u:%02u:%02u"
                                         , tm -> hour
                                         , tm -> minute
                                         , tm -> second
                        );
                }
                else
                {
                    dst_len += sprintf ( & text [ dst_len ]
                                         , f . left_fill == '0' ? "%02u:%02u:%02u %cM" : "%u:%02u:%02u %cM"
                                         , ( tm -> hour + 11 ) % 12 + 1
                                         , tm -> minute
                                         , tm -> second
                                         , ( tm -> hour < 12 ) ? 'A' : 'P'
                        );
                }

                if ( f . print_timezone )
                {
                    dst_len += sprintf ( & text [ dst_len ]
                                         , " %+02d"
                                         , tm -> tzoff / 60
                        );
                }
            }

            StringInit ( & S, text, dst_len, dst_len );
            break;

        case spfRC:
            dst_len = (uint32_t) KWrtFmt_rc_t ( text, sizeof text, f . explain_rc ? "#" : "", ( rc_t ) u64 );
            StringInit ( & S, text, dst_len, dst_len );
            break;

        case spfOSErr:
            dst_len = (uint32_t) KWrtFmt_error_code ( text, sizeof text, ( int ) i64 );
            StringInit ( & S, text, dst_len, dst_len );
            break;

        default:
            return RC ( rcText, rcString, rcConverting, rcFormat, rcUnrecognized );
        }

        /* at this point, we have text in "S".
           if the text pointer is NULL, it will need to be further processed.
           if the text size is 0 but the length is not, then we may need to measure string.
        */

        /* apply index limitation */
        if ( text_index )
        {
            assert ( S . addr != NULL );
            assert ( S . size != 0 || S . len == 0 );
            if ( StringSubstr ( & S, & S, ( uint32_t ) text_start, ( uint32_t ) text_lim ) == NULL )
                StringInit ( & S, "", 0, 0 );
        }

        /* determine overall width of substitution so far */
        dst_len = left_pad + ( sign_char != 0 ) + prefix_len + zero_fill + S . len;

        /* if there are comma insertions, get those, too */
        if ( comma_sep && ( int_len != 0 || zero_fill != 0 ) )
            dst_len += ( int_len + zero_fill - 1 ) / 3;

        /* calculate remaining left padding */
        if ( f . left_fill != 0 && f . u . f . min_field_width > dst_len )
        {
            assert ( S . size != 0 || S . len == 0 );
            left_pad += ( uint32_t ) ( f . u . f . min_field_width ) - dst_len;
            dst_len = ( uint32_t ) f . u . f . min_field_width;
        }

        /* left padding */
        if ( left_pad != 0 )
        {
            rc = print_padding ( out, left_pad, f . left_fill );
            if ( rc != 0 )
                return rc;
        }

        /* output sign or prefix */
        if ( sign_char != 0 )
        {
            rc = print_padding ( out, 1, sign_char );
            if ( rc != 0 )
                return rc;
        }
        else if ( prefix_len != 0 )
        {
            rc = write_buffer ( out, prefix, prefix_len );
            if ( rc != 0 )
                return rc;
        }

        /* output comma-separated numeral */
        if ( comma_sep && ( zero_fill != 0 || int_len != 0 ) )
        {
            uint32_t pos = zero_fill + int_len;
            uint32_t chunk = ( pos - 1 ) % 3 + 1;

            for ( i = 0, pos -= chunk; chunk > 0; -- chunk )
            {
                if ( zero_fill != 0 )
                {
                    rc = print_padding ( out, 1, '0' );
                    -- zero_fill;
                }
                else
                {
                    rc = print_padding ( out, 1, S . addr [ i ] );
                    ++ i;
                }
                if ( rc != 0 )
                    return rc;
            }

            assert ( pos % 3 == 0 );
            for ( pos /= 3; pos != 0; -- pos )
            {
                rc = print_padding ( out, 1, ',' );
                if ( rc != 0 )
                    return rc;

                if ( zero_fill != 0 )
                {
                    if ( zero_fill >= 3 )
                    {
                        rc = print_padding ( out, 3, '0' );
                        zero_fill -= 3;
                    }
                    else
                    {
                        rc = print_padding ( out, zero_fill, '0' );
                        if ( rc == 0 )
                            rc = write_buffer ( out, S . addr, 3 - zero_fill );
                        i += 3 - zero_fill;
                        zero_fill = 0;
                    }
                }
                else
                {
                    rc = write_buffer ( out, & S . addr [ i ], 3 );
                    i += 3;
                }

                if ( rc != 0 )
                    return rc;
            }

            rc = write_buffer ( out, & S . addr [ i ], S . len - i );
            if ( rc != 0 )
                return rc;
        }

        /* output numeral with zero fill */
        else if ( zero_fill != 0 )
        {
            rc = print_padding ( out, zero_fill, '0' );
            if ( rc == 0 )
                rc = print_string ( out, & S, text_lim );
        }

        /* output NUL-terminated string */
        else if ( S . size == 0 && S . len != 0 )
        {
            assert ( f . left_fill == 0 || f . u . f . min_field_width == 0 );
            rc = print_nul_term_string ( out, & S, text_lim );
            dst_len = S . len;
        }

        /* output KSymbol */
        else if ( S . addr == NULL )
        {
            rc = print_symbol ( out, p );
        }

        /* output anything else in a String */
        else
        {
            rc = print_string ( out, & S, text_lim );
        }

        /* recover from error */
        if ( rc != 0 )
            return rc;

        /* apply right padding */
        if ( f . u . f . min_field_width > dst_len )
        {
            rc = print_padding ( out, f . u . f . min_field_width - dst_len, ' ' );
            if ( rc != 0 )
                return rc;
        }
    }

    rc = flush_buffer ( out );

    if ( rc == 0 && out -> overflow )
        rc = RC ( rcText, rcString, rcConverting, rcBuffer, rcInsufficient );

    return rc;
}


/* structured_printf
 *  uses constant format descriptors and argument block
 *  prints to "out" handler
 */
LIB_EXPORT rc_t CC structured_printf ( const KWrtHandler *handler,
    size_t *num_writ, const PrintFmt *fmt, const PrintArg *args )
{
    if ( handler != NULL )
    {
        rc_t rc;
        char buff [ 16 * 1024 ];

        KBufferedWrtHandler out;
        out . handler = handler;
        out . buff = buff;
        out . bsize = sizeof buff;

        rc = structured_print_engine ( & out, fmt, args );

        if ( num_writ != NULL )
            * num_writ = out . cur + out . flushed;

        return rc;
    }

    if ( num_writ != NULL )
        * num_writ = 0;

    return RC ( rcRuntime, rcString, rcConverting, rcFile, rcNull );
}


/* structured_sprintf
 *  uses constant format descriptors and argument block
 *  prints to UTF-8 character buffer "dst"
 */
LIB_EXPORT rc_t CC structured_sprintf ( char *dst, size_t bsize,
    size_t *num_writ, const PrintFmt *fmt, const PrintArg *args )
{
    if ( dst != NULL || bsize == 0 )
    {
        rc_t rc;

        KBufferedWrtHandler out;
        out . handler = NULL;
        out . buff = dst;
        out . bsize = bsize;

        rc = structured_print_engine ( & out, fmt, args );

        if ( num_writ != NULL )
            * num_writ = out . cur + out . flushed;

        return rc;
    }

    if ( num_writ != NULL )
        * num_writ = 0;

    return RC ( rcRuntime, rcString, rcConverting, rcBuffer, rcNull );
}


/* string_printf
 *  provides a facility similar to snprintf
 *
 *  "dst" [ OUT ] and "bsize" [ IN ] - output buffer for string
 *  will be NUL-terminated if possible
 *
 *  "num_writ" [ OUT, NULL OKAY ] - returns the number of non-NUL bytes
 *  written to "dst" or the required "bsize" to complete successfully,
 *  not including the NUL termination.
 *
 *  "fmt" [ IN ] and "args" [ IN, OPTIONAL ] - data to write
 *
 *  returns 0 if all bytes were successfully written and a NUL-byte was
 *  written into the buffer.
 *
 *  returns rcBuffer, rcInsufficient if the buffer was too small. in this
 *  case, it is possible that the only missing byte would be the NUL
 *  termination, and the output string may still be usable since "num_writ"
 *  indicates the actual number of text bytes.
 */
LIB_EXPORT rc_t CC new_string_vprintf ( char *dst, size_t bsize,
    size_t *num_writ, const char *fmt_str, va_list vargs )
{
    rc_t rc;

    String str [ 64 ], * strp = str;
    PrintFmt fmt [ 64 ], * fmtp = fmt;
    PrintArg args [ 64 ], * argp = args;

    KDataBuffer overflow;

    size_t dummy;
    if ( num_writ == NULL )
        num_writ = & dummy;

    * num_writ = 0;

    rc = parse_format_string ( fmt_str, vargs,
        & strp, & fmtp, & argp, & overflow );

    if ( rc == 0 )
        rc = structured_sprintf ( dst, bsize, num_writ, fmtp, argp );

    if ( strp != str )
        KDataBufferWhack ( & overflow );

    return rc;
}

LIB_EXPORT rc_t CC new_string_printf ( char *dst, size_t bsize,
    size_t *num_writ, const char *fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = new_string_vprintf ( dst, bsize, num_writ, fmt, args );

    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC new_vkfprintf ( const KWrtHandler *out,
    size_t *num_writ, const char *fmt_str, va_list vargs )
{
    rc_t rc;

    String str [ 64 ], * strp = str;
    PrintFmt fmt [ 64 ], * fmtp = fmt;
    PrintArg args [ 64 ], * argp = args;

    KDataBuffer overflow;

    size_t dummy;
    if ( num_writ == NULL )
        num_writ = & dummy;

    * num_writ = 0;

    rc = parse_format_string ( fmt_str, vargs,
        & strp, & fmtp, & argp, & overflow );

    if ( rc == 0 )
        rc = structured_printf ( out, num_writ, fmtp, argp );

    if ( strp != str )
        KDataBufferWhack ( & overflow );

    return rc;
}

LIB_EXPORT rc_t CC new_kfprintf ( const KWrtHandler *out,
    size_t *num_writ, const char *fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = new_vkfprintf ( out, num_writ, fmt, args );
        
    va_end ( args );

    return rc;
}

#define USE_SMARTER_BUFFER_PRINT 1
#if USE_SMARTER_BUFFER_PRINT
static rc_t KDataBufferWriter(void *vself, char const *content, size_t size, size_t *num_writ)
{
    KDataBuffer *const self = vself;
    size_t const orig_size = (size_t)self->elem_count; /* includes nul */
    rc_t rc = KDataBufferResize(self, orig_size + size);
    if (rc == 0) {
        char *const base = self->base;
        
        memmove(base + orig_size - 1 /* overwrite nul */, content, size);
        *num_writ = size;

        base[orig_size + size - 1] = '\0'; /* add nul */
    }
    return rc;
}
#endif

LIB_EXPORT rc_t CC KDataBufferVPrintf ( KDataBuffer * buf, const char * fmt, va_list args )
{
    rc_t rc;

    if ( buf == NULL )
        rc = RC ( rcText, rcString, rcFormatting, rcBuffer, rcNull );
    else if ( fmt == NULL )
        rc = RC ( rcText, rcString, rcFormatting, rcParam, rcNull );
    else if ( fmt [ 0 ] == 0 )
        rc = RC ( rcText, rcString, rcFormatting, rcParam, rcEmpty );
    else
    {
#if USE_SMARTER_BUFFER_PRINT
        KWrtHandler handler;
        uint64_t const orig_size = buf->elem_count;
        
        handler.writer = KDataBufferWriter;
        handler.data = buf;
        
        if (orig_size == 0) {
            if (buf->elem_bits == 0)
                buf->elem_bits = 8;
            if (buf->elem_bits == 8) {
                rc = KDataBufferResize(buf, 1);
                if (rc)
                    return rc;
                ((char *)buf->base)[0] = '\0';
            }
        }
        if (buf->elem_bits != 8)
            return RC ( rcText, rcString, rcFormatting, rcParam, rcIncorrect );

        /* nul terminator is required */
        if (((char const *)buf->base)[buf->elem_count - 1] != '\0')
            return RC ( rcText, rcString, rcFormatting, rcParam, rcIncorrect );

        rc = vkfprintf(&handler, NULL, fmt, args);
        if (rc)
            (void)KDataBufferResize(buf, orig_size);
#else
        size_t bsize;
        char * buffer;
        size_t content;
        size_t num_writ;
        uint64_t orig_size;

        /* the C library ruins a va_list upon use
           in case we ever need to use it a second time,
           make a copy first */
        va_list args_copy;

        /* begin to calculate content and bsize */
        content = ( size_t ) ( orig_size = buf -> elem_count );

        /* check for an empty buffer */
        if ( content == 0 )
        {
            /* detect buffers initialized by memset to zero */
            if ( buf -> elem_bits == 0 )
                buf -> elem_bits = 8;

            /* size to 4K */
            rc = KDataBufferResize ( buf, bsize = 4096 );
            if ( rc != 0 )
            {
                buf -> elem_count = 0;
                return rc;
            }
        }
        else if ( buf -> elem_bits != 8 )
        {
            return RC ( rcText, rcString, rcFormatting, rcParam, rcIncorrect );
        }
        else
        {
            /* recover actual size of buffer, assuming 4K increments */
            bsize = ( content + 4095 ) & ~ ( size_t ) 4095;

            /* discount NUL byte */
            content -= 1;
        }

        /* prepare for a second attempt after a resize */
        va_copy ( args_copy, args );
            
        /* nothing has yet been written, buffer pointer is stable */
        num_writ = 0;
        buffer = buf -> base;

        /* try to print into the buffer and ALWAYS leave room for a NUL byte */
        rc = string_vprintf ( & buffer [ content ], bsize - 1 - content, & num_writ, fmt, args );

        /* the error we are EXPECTING and can fix is where the buffer was too small */
        if ( GetRCState ( rc ) == rcInsufficient && GetRCObject ( rc ) == rcBuffer )
        {
            /* calculate a new needed size */
            size_t new_size = ( content + num_writ + 4095 + 1 ) & ~ ( size_t ) 4095;

            /* mark that nothing has yet been successfully written */
            num_writ = 0;

            /* resize the buffer */
            rc = KDataBufferResize ( buf, new_size );
            if ( rc == 0 )
            {
                /* assume reallocation took place */
                buffer = buf -> base;
                bsize = new_size;

                /* try again with the newly sized buffer */
                rc = string_vprintf ( & buffer [ content ], bsize - 1 - content, & num_writ, fmt, args_copy );

                /* any error means nothing was printed */
                if ( rc != 0 )
                    num_writ = 0;
            }
        }

        /* destroy copy */
        va_end ( args_copy );

        /* NUL terminate even if redundant */
        assert ( content + num_writ < bsize );
        assert ( num_writ + 1 == bsize || rc != 0 || buffer [ content + num_writ ] == 0 );
        buffer [ content + num_writ ] = 0;
    
        /* size down to bsize + NUL */
        if ( rc != 0 )
            KDataBufferResize ( buf, orig_size );
        else
            KDataBufferResize ( buf, content + num_writ + 1 );
#endif
    }

    return rc;
}

/* forward to KDataBufferVPrintf */
LIB_EXPORT rc_t CC KDataBufferPrintf ( KDataBuffer * buf, const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );
    rc = KDataBufferVPrintf ( buf, fmt, args );
    va_end ( args );

    return rc;
}
