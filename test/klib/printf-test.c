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

#include <kapp/main.h>
#include <kapp/args.h>
#include <klib/printf.h>
#include <klib/symbol.h>
#include <klib/text.h>
#include <klib/time.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <kfg/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <va_copy.h>
#include <time.h>

static
rc_t test_vprintf ( const char *expected, const char *fmt, va_list args )
{
    rc_t rc;
    size_t num_writ;
    char buff [ 4096 ];

    rc = string_vprintf ( buff, sizeof buff, & num_writ, fmt, args );
    if ( rc != 0 )
    {
        /* report return code */
        pLogErr ( klogErr, rc, "string_vprintf returned non-zero rc with format '$(fmt)'", "fmt=%s", fmt );
    }
    else
    {
        size_t buff_size;

        /* verify results. they are:
           1. string itself
           2. number of characters in "num_writ"
        */

        if ( strcmp ( buff, expected ) != 0 )
        {
            /* set an appropriate return code */
            rc = RC ( rcExe, rcString, rcFormatting, rcData, rcIncorrect );

            /* report discrepancy */
            pLogErr ( klogErr, rc,
                      "string_vprintf result differs from expected:\n"
                      "  format   - '$(format)'\n"
                      "  expected - '$(expected)'\n"
                      "  observed - '$(observed)'"
                      , "format=%s,expected=%s,observed=%s"
                      , fmt
                      , expected
                      , buff
                );
        }

        buff_size = strlen ( buff );
        if ( buff_size != num_writ )
        {
            /* report discrepancy */
            pLogMsg ( klogErr,
                      "string_vprintf size differs from expected:\n"
                      "  expected - $(expected)\n"
                      "  observed - $(observed)"
                      , "expected=%zu,observed=%zu"
                      , buff_size
                      , num_writ
                );
        }
#if 1
        rc = 0;
#endif
    }

    return rc;
}

static
rc_t test_printf ( const char *expected, const char *fmt, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, fmt );
    rc = test_vprintf ( expected, fmt, args );
    va_end ( args );
    return rc;
}


/* NEW TESTING STRATEGY

    6a. size modifier
        a. NULL ( not given )
        b. 't' for 8-bit integer
        c. 'h' for 16 bit integer or 32-bit float
        d. 'l' for 64-bit integer or long double
        E. 'z' for size of size_t ( 32 or 64 bit depending upon architecture )
    6b. time modifier ( do this later )

    So, you'll want functions for each stage. The driving function will invoke
    the stages with parameters and values.
*/

static
rc_t final ( const char *flags, int32_t *field_width, int32_t *precision,
             char size_modifier, char storage_class, va_list args )
{
    rc_t rc;
    uint32_t i, j;
    va_list arg_copy;
    char stdcfmt [ 32 ], fmt [ 32 ], expected [ 4096 ];

    /* initialize counters */
    i = 0;

    /* build format string */
    fmt [ i ++ ] = '%';

    if ( flags != NULL )
        i += sprintf ( & fmt [ i ], "%s", flags );
    if ( field_width != NULL )
    {
        if ( field_width [ 0 ] == -1 )
            fmt [ i ++ ] = '*';
        else
            i += sprintf ( & fmt [ i ], "%u", field_width [ 0 ] );
    }
    if ( precision != NULL )
    {
        fmt [ i ++ ] = '.';

        if ( precision [ 0 ] == -1 )
            fmt [ i ++ ] = '*';
        else
            i += sprintf ( & fmt [ i ], "%u", precision [ 0 ]  );
    }
    fmt[i]=0;

    /* duplicate format string */
    strcpy ( stdcfmt, fmt );
    j = i;

    /* size modifiers */
    if ( size_modifier != 0 )
    {
        fmt [ i ++ ] = size_modifier;

        switch ( size_modifier )
        {
        case 't':
#if WINDOWS
            return 0; /* hh does not work as we expect on Windows */
#endif
            stdcfmt [ j ++ ] = 'h';
            stdcfmt [ j ++ ] = 'h';
            break;
        case 'z':
            if ( sizeof ( size_t ) != sizeof ( uint32_t ) )
                stdcfmt [ j ++ ] = 'l';
            break;
        case 'l':
            if ( sizeof ( long int ) == sizeof ( int32_t ) )
                stdcfmt [ j ++ ] = size_modifier;
            /* no break */
        default:
            stdcfmt [ j ++ ] = size_modifier;
        }
    }

    /* storage class/formatting */
    fmt [ i ++ ] = storage_class;
    stdcfmt [ j ++ ] = storage_class;

    /* terminate format strings with NUL */
    fmt [ i ] = 0;
    stdcfmt [ j ] = 0;

    /* generate expected string */
    va_copy ( arg_copy, args );
    vsnprintf ( expected, sizeof expected, stdcfmt, arg_copy );
    va_end ( arg_copy );

    /* execute test */
    va_copy ( arg_copy, args );
    rc = test_vprintf ( expected, fmt, arg_copy );
    va_end ( arg_copy );

    return rc;
}

static rc_t thousands_flag ( char *flags, uint32_t num_flags, int32_t *field_width, int32_t *precision,
                             char size_modifier, char storage_class, va_list args)
{
#if 1
    /* built-in sprintf can't handle this */
    return final ( flags, field_width, precision, size_modifier, storage_class, args );
#else
    rc_t rc;

    /* with nothing */
    rc = final ( flags, field_width, precision, size_modifier, storage_class, args );
    if ( rc != 0 )
        return rc;

    /* with thousands */
    flags [ num_flags ] = ',';
    flags [ num_flags + 1 ] = 0;
    rc = final ( flags, field_width, precision, size_modifier, storage_class, args );

    return rc;
#endif

}
static rc_t alternate_flag ( char *flags, uint32_t num_flags, int32_t *field_width, int32_t *precision,
                             char size_modifier, char storage_class, va_list args)
{
    rc_t rc;


    /* with nothing */
    rc = thousands_flag ( flags, num_flags, field_width, precision, size_modifier, storage_class, args );
    if ( rc != 0 )
        return rc;

    /* with alternate */
    flags [ num_flags ] = '#';
    flags [ num_flags + 1 ] = 0;
    rc = thousands_flag ( flags, num_flags + 1, field_width, precision, size_modifier, storage_class, args );

    return rc;
}

static rc_t alignment_flag ( char *flags, uint32_t num_flags, int32_t *field_width, int32_t *precision,
                             char size_modifier, char storage_class, va_list args)
{
    rc_t rc;

    /* with nothing */
    rc = alternate_flag ( flags, num_flags, field_width, precision, size_modifier, storage_class, args );
    if ( rc != 0 )
        return rc;

    /* with left-align */
    flags [ num_flags ] = '-';
    flags [ num_flags + 1 ] = 0;
    rc = alternate_flag ( flags, num_flags + 1, field_width, precision, size_modifier, storage_class, args );
    if ( rc != 0 )
        return rc;

    /* with zer-padding left-fill */
    flags [ num_flags ] = '0';
    flags [ num_flags + 1 ] = 0;
    rc = alternate_flag ( flags, num_flags + 1, field_width, precision, size_modifier, storage_class, args );
    if ( rc != 0 )
        return rc;

    /* with both left-align and zero-padded left fill */
    flags [ num_flags ] = '0';
    flags [ num_flags + 1 ] = '-';
    flags [ num_flags + 2 ] = 0;
    rc = alternate_flag ( flags, num_flags + 2, field_width, precision, size_modifier, storage_class, args );

    return rc;
}

static rc_t sign_flag ( int32_t *field_width, int32_t *precision,
                        char size_modifier, char storage_class, va_list args)
{
    rc_t rc;
    char flags [ 16 ];

    /* with nothing */
    flags [ 0 ] = 0;
    rc = alignment_flag ( flags, 0, field_width, precision, size_modifier, storage_class, args );
    if ( rc != 0 )
        return rc;

    /* with space */
    flags [ 0 ] = ' ';
    flags [ 1 ] = 0;
    rc = alignment_flag ( flags, 1, field_width, precision, size_modifier, storage_class, args );
    if ( rc != 0 )
        return rc;

    /* with forces + */
    flags [ 0 ] = '+';
    flags [ 1 ] = 0; 
    rc = alignment_flag ( flags, 1, field_width, precision, size_modifier, storage_class, args );
    if ( rc != 0 )
        return rc;

    /* with both */
    flags [ 0 ] = ' ';
    flags [ 1 ] = '+';
    flags [ 2 ] = 0;
    rc = alignment_flag ( flags, 2, field_width, precision, size_modifier, storage_class, args );

    return rc;
}

/* apply field width */
static rc_t do_field_width ( int32_t *field_width, int32_t *precision,
                             char size_modifier, char storage_class, va_list args)
{
    rc_t rc;
    uint32_t i; 

    rc = sign_flag ( NULL, precision, size_modifier, storage_class, args );

    for ( i = 0 ; field_width [ i ] != -2 && rc == 0 ; ++ i )
        rc = sign_flag ( & field_width [ i ], precision, size_modifier, storage_class, args );

    return rc;
}

/* apply precision */
static rc_t do_precision ( int32_t *field_width, int32_t *precision,
                           char size_modifier, char storage_class, va_list args)
{
    rc_t rc;
    uint32_t i;

    rc = do_field_width ( field_width, NULL, size_modifier, storage_class, args );

    for ( i = 0 ; precision [ i ] != -2 && rc == 0; ++i )
       rc = do_field_width ( field_width, & precision [ i ], size_modifier, storage_class, args );

    return rc;
}


static rc_t do_size_modifier ( int32_t *field_width, int32_t *precision,
    const char *size_modifier, char storage_class, va_list args )
{
    rc_t rc = 0;
    uint32_t i;

    for ( i = 0 ; size_modifier [ i ] != 0 && rc == 0; ++ i )
    {
        char modifier = size_modifier [ i ];
        if ( modifier == ' ' )
            modifier = 0;
        rc = do_precision ( field_width, precision, modifier, storage_class, args );
    }

    return rc;
}


static rc_t do_storage_class ( int32_t *field_width, int32_t *precision,
                               const char *size_modifier, const char *storage_class, va_list args )
{
    rc_t rc = 0;
    uint32_t i;

    for ( i = 0 ; storage_class [ i ] != 0 && rc == 0; ++ i )
        rc = do_size_modifier ( field_width, precision, size_modifier, storage_class [ i ], args );

    return rc;
}


static rc_t make_initial_test (  int32_t *field_width, int32_t *precision,
                                 const char *size_modifier, const char *storage_class, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, storage_class );

    rc = do_storage_class ( field_width, precision, size_modifier, storage_class, args );
 
    va_end ( args );
    return rc;
}

static
rc_t run ( const char *progname )
{
    rc_t rc = 0;
    int32_t i;

    int32_t field_width [ ] = { 2, 5, 9, 10, -2 };
    int32_t precision [ ] = { 1, 3, 8, 12, -2 };
    char c [ ] = { "aA!@0{;>" };

    int32_t ext_value [ ] = { -1, -2 };
    int32_t randValue, randValue_2, randValue_3;
    double randValue_f;


    for ( i = 0 ; i < 8 ; ++ i )
    {
        /* create random number */
        srand ( time ( NULL ) );
        
        
        /* signed integer */
        if ( rc == 0 )
        {
            randValue = rand ();
            randValue_2 = rand () % 10;
            randValue_3 = rand () % 5;
            
            rc = make_initial_test ( field_width, precision, " ht", "di", randValue );
            if ( rc == 0 )
                rc = make_initial_test ( ext_value, precision, " ht", "di", randValue_2, randValue );
            if ( rc == 0 )
                rc = make_initial_test ( field_width, ext_value, " ht", "di", randValue_3, randValue );
            
            if ( rc == 0 )
                rc = make_initial_test ( field_width, precision, "l", "di", ( int64_t ) randValue );
            if ( rc == 0 )
                rc = make_initial_test ( ext_value, precision, "l", "di", randValue_2, ( int64_t ) randValue );
            if ( rc == 0 )
                rc = make_initial_test ( field_width, ext_value, "l", "di", randValue_3, ( int64_t ) randValue );
        }
        
        /* unsigned integer */
        if ( rc == 0 )
        {
            rc = make_initial_test ( field_width, precision, " ht", "uxXo", randValue );
            if ( rc == 0 )
                rc = make_initial_test ( ext_value, precision, " ht", "uxXo", randValue_2, randValue );
            if ( rc == 0 )
                rc = make_initial_test ( field_width, ext_value, " ht", "uxXo", randValue_3, randValue );
            
            if ( rc == 0 )
                rc = make_initial_test ( field_width, precision, "l", "uxXo", ( uint64_t ) randValue );
            if ( rc == 0 )
                rc = make_initial_test ( ext_value, precision, "l", "uxXo", randValue_2, ( uint64_t )randValue );
            if ( rc == 0 )
                rc = make_initial_test ( field_width, ext_value, "l", "uxXo", randValue_3, ( uint64_t ) randValue );
            
            if ( rc == 0 )
                rc = make_initial_test ( field_width, precision, "z", "uxXo", ( size_t ) randValue );
            if ( rc == 0 )
                rc = make_initial_test ( ext_value, precision, "z", "uxXo", randValue_2, ( size_t ) randValue );
            if ( rc == 0 )
                rc = make_initial_test ( field_width, ext_value, "z", "uxXo", randValue_3, ( size_t ) randValue );
        }
        
        /* float */
        if ( rc == 0 )
        {
            
            randValue_f = ( double ) randValue / ( ( randValue % 100 ) + 1 );
            
            /*** could use some floating point random numbers here */
#if 0            
            rc = make_initial_test ( field_width, precision, " ", "feg", randValue );
            if ( rc == 0 )
                rc = make_initial_test ( ext_value, precision, " ", "feg", randValue_2, randValue );
            if ( rc == 0 )
                rc = make_initial_test ( field_width, ext_value, " ", "feg", randValue_3, randValue );
#endif            

            if ( rc == 0 )
                rc = make_initial_test ( field_width, precision, " ", "feg", randValue_f );
            if ( rc == 0 )
                rc = make_initial_test ( ext_value, precision, " ", "feg", randValue_2, randValue_f );
            if ( rc == 0 )
                rc = make_initial_test ( field_width, ext_value, " ", "feg", randValue_3, randValue_f );
        }
        
        /* character */
        if ( rc == 0 )
        {
            rc = make_initial_test ( field_width, precision, " ", "c", c [ i ] );
            if ( rc == 0 )
                rc = test_printf ( "I like 1 embedded % character", "I like %u embedded %% character", 1 );
        }
        
        /* text string */
        if ( rc == 0 )
        {
            rc = make_initial_test ( field_width, precision, " ", "s", "Kurt is having a fit" );
#if !defined(__SunOS)  &&  !defined(__sun__)
            /* Solaris printf doesn't cope with NULLs */
#if 0
            /* The standard says this result is undefined, we shouldn't test for it, it is not consistent */
            rc = make_initial_test ( field_width, precision, " ", "s", NULL );
#endif
#endif            
            rc = make_initial_test ( field_width, precision, " ", "s", "" );
            rc = make_initial_test ( field_width, precision, " ", "s", "OK" );
            rc = make_initial_test ( field_width, precision, " ", "s", "1234567890" );
            rc = make_initial_test ( field_width, precision, " ", "s", "\"`~!@#$%^&*()-_=+[]{}|\\';:?/.,<>" );
        }
        
    } 
  
    /* hand written tests */

    {
    
        int8_t t [ ] = { -128, -67, 0, 4, 56, 100, 127 };
        int16_t h  [ ] = { -32768, -2546, -398, -89, 0, 34, 123, 5736, 32767 };
        int32_t v [ ] = { -2147483648, -45287957, -100001, 45, 0, 106, 7234, 546963874, 2147483647 };

        /*** naked integer literals have type "int" in C, meaning they
             can't be more than 32 bits. By adding "L" to the end of the
             literal numeral, the compiler will read them as "long int",
             which is in fact 64 bits on this machine. on a 32-bit machine,
             you need type "long long int".

             you can make use of a pre-processor symbol to do this properly - I'll do it below. 
             */
        int64_t l [ ] = { INT64_C(-9223372036854775807) - INT64_C(1), INT64_C(-67283678467328376), INT64_C(-2837640198), INT64_C(0),  INT64_C(187267509872), INT64_C(9223372036854775807) }; 

        /* d, i */

        /* 8 bit */
        test_printf ( "-000128", "%07:0td", t );
        test_printf ( "  -67"  , "%5:1td", t );
        test_printf ( "0"      , "%:2td", t );
        test_printf ( "4    "  , "%-5:3td", t );
        test_printf ( " 56"    , "% .1:4td", t );
        test_printf ( "100 "   , "%-4.2:5td", t );
        test_printf ( "127  "  , "%-05:6td", t );
        /* 16 bit */
        test_printf ( "-32768"    , "%.2:0hd", h );
        test_printf ( "-2546  "   , "%-07:1hd", h );
        test_printf ( "-398"      , "% :2hd", h );
        test_printf ( "-0089"     , "%05:3hd", h );
        test_printf ( "0000"      , "%04:4hd", h );
        test_printf ( "+34"       , "%+:5hd", h );
        test_printf ( "+0123"     , "% +05:6hd", h );
        test_printf ( "5736 "     , "%-05:7hd", h );
        test_printf ( "     32767", "%10:8hd", h );
        /* 32 bit */
        test_printf ( "-2,147,483,648", "%,:0d", v );
        test_printf ( "-0045287957"   , "%011:1d", v );
        test_printf ( "-100001 "      , "%-8:2d", v );
        test_printf ( "45"            , "%0:3d", v );
        test_printf ( "0"             , "%,:4d", v );
        test_printf ( "106"           , "%:5d", v );
        test_printf ( "0,007,234"     , "%,09:6d", v );
        test_printf ( "546963874"     , "%.3:7d", v );
        test_printf ( "2147483647"    , "%10:8d", v );
        /* 64 bit */
        test_printf ( "-9223372036854775808"    , "%:0ld", l );
        test_printf ( "-67,283,678,467,328,376" , "%,:1ld", l );
        test_printf ( "-2837640198         "    , "%-20:2ld", l );
        test_printf ( "+0"                      , "%+:3ld", l );
        test_printf ( "00000000187267509872"    , "%020:4ld", l );
        test_printf ( "9223372036854775807"     , "%.2:5ld", l );

    } 


    {
        /* uxXo */

       uint8_t u_t [ ] = { 0, 128, 255};
       uint16_t u_h [ ] = { 0, 128, 5378, 65535};
       uint32_t u_v [ ] = { 0, 847, 7859, 376859, 86742874, 4294967295 };
       uint64_t u_l [ ] = { 0, 178, 178364, 1783940987, 17836479208762, UINT64_C(18446744073709551615) };


       /* 8 bit */
       test_printf ( "    0", "%5:0tu", u_t );
       test_printf ( "80"   , "%:1tx", u_t );
       test_printf ( "0x80" , "%#:1tx", u_t );
       test_printf ( "377"  , "%:2to", u_t );
       test_printf ( "0377" , "%#:2to", u_t );
       /* 16 bit */
       test_printf ( "0"        , "%#:0hX", u_h );
       test_printf ( "00128"    , "%05:1hu", u_h );
       test_printf ( "0x1502"   , "%-#3:2hx", u_h );
       test_printf ( "000177777", "%#09:3ho", u_h );
       /* 32 bit */
       test_printf ( "00000"          , "%05:0u", u_v );
       test_printf ( "34f"            , "%:1x", u_v );
       test_printf ( "0X1EB3"         , "%#6:2X", u_v );
       test_printf ( "1340033"        , "%:3o", u_v );
       test_printf ( "86742874       ", "%-#15:4u", u_v );
       test_printf ( "ffffffff"       , "%0:5x", u_v );
       /* 64 bit */
       test_printf ( "0    "               , "%-5:0lX", u_l );
       test_printf ( "0262 "               , "%#-5:1lo", u_l );
       test_printf ( "178364"              , "%.4:2lu", u_l );
       test_printf ( "0x006a54c77b"        , "%#012:3lx", u_l );
       test_printf ( "1038E101DD3A"        , "%.5:4lX", u_l );
       test_printf ( "18446744073709551615", "%:5lu", u_l );

    }


    {
        /* float */ 

        float f [ ] = { -2.1474836, -45.287957, -10000.1, 0.45, 0, 1.06 };
        double lf [ ] = { -9223372036854775808.0, -28.37640198 };
        
        /* 32 bit */
        test_printf ( "  -2.15"             , "%7.2:0hf", f );
        test_printf ( "-00045.288"          , "%010.3:1hf" , f );
        test_printf ( "-10000.099609"       , "%:2hf", f );
        test_printf ( "0"                   , "%.0:3hf", f );
        test_printf ( "0.000"               , "%-5.3:4hf", f );
        test_printf ( "1.060000    "        , "%-012:5hf", f );
        /* 64 bit */
        test_printf ( "-9223372036854775808.000000" , "%:0f", lf );
        test_printf ( "-28.37640198000000069101", "%2.20:1f", lf );
    }



    /* s */
    test_printf ( "There are too many tests" , "%:s"    , "There are too many tests" );
    test_printf ( "a"                        , "%:6s"   , "There are too many tests" );

    test_printf ( "There"                    , "%:/5s"  , "There are too many tests" );
    test_printf ( "too m"                    , "%:*/5s" , 10, "There are too many tests" );
    test_printf ( "There"                    , "%:/*s"  , 5,"There are too many tests" );
    test_printf ( " too many tests"          , "%:9/16s", "There are too many tests" );
    test_printf ( "s"                        , "%:$/5s" , "There are too many tests" );
    test_printf ( "tests"                    , "%:19/$s", "There are too many tests" );

    test_printf ( "There"                    , "%:0-4s", "There are too many tests" );
    test_printf ( "There are too many tests" , "%:-s"  , "There are too many tests" );
    test_printf ( "too many tests"           , "%:10-s", "There are too many tests" );
    test_printf ( "There are"                , "%:-8s" , "There are too many tests" );

    test_printf ( "e are too many tests"     , "%:*-s" , 4 , "There are too many tests" );
    test_printf ( "e are too "               , "%:4-*s", 13, "There are too many tests" );
    test_printf ( "There are t"              , "%:-*s" , 10, "There are too many tests" );
    test_printf ( "ere a"                    , "%:*-6s", 2 , "There are too many tests" );

    test_printf ( "s"                        , "%:$s"  , "There are too many tests" );
    test_printf ( "s"                        , "%:$-s" , "There are too many tests" );
    test_printf ( "s"                        , "%:$-2s", "There are too many tests" );
    test_printf ( "There are too many tests" , "%:-$s" , "There are too many tests" );
    test_printf ( "re are too many tests"    , "%:3-$s", "There are too many tests" );

    /* with field width, precision, and flags */
    test_printf ( "There are too many tests" , "%5:s"       , "There are too many tests" );
    test_printf ( "                   a"     , "%20:6s"     , "There are too many tests" );
    test_printf ( "There     "               , "%-10:/5s"   , "There are too many tests" );
    test_printf ( "     too m"               , "%*:*/5s"    , 10, 10, "There are too many tests" );
    test_printf ( "The"                      , "%.3:/*s"    , 5,"There are too many tests" );
    test_printf ( " too"                     , "%*.4:9/16s" , 2,  "There are too many tests" );

    test_printf ( "There"                          , "%-:0-4s", "There are too many tests" );
    test_printf ( "There are too many tests      " , "%-30:-s", "There are too many tests" );
    test_printf ( "too       "                     , "%-10.4:10-s" , "There are too many tests" );
    test_printf ( "There"                          , "%.*:-8s"  , 5, "There are too many tests" );
    test_printf ( ""                               , "%s", "" );

    {
        String S;
        CONST_STRING ( & S, "My Bonnie lies over the ocean" );
        test_printf ( "My Bo", "%:0-4S", & S );
    }

    {
        String str1, str2, str3;
        KSymbol *sym1, *sym2, *sym3;

        CONST_STRING ( & str1, "outer" );
        CONST_STRING ( & str2, "inner" );
        CONST_STRING ( & str3, "leaf" );

        KSymbolMake ( & sym1, & str1, 0, NULL );
        KSymbolMake ( & sym2, & str2, 0, NULL );
        KSymbolMake ( & sym3, & str3, 0, NULL );

        sym2 -> dad = sym1;
        sym3 -> dad = sym2;

        test_printf ( "outer:inner:leaf", "%N", sym3 );
        test_printf ( "    outer:inner:leaf", "%20N", sym3 );
        test_printf ( "outer:inner:leaf    ", "%-20N", sym3 );
        test_printf ( "outer:inner:leaf    ", "%-20.2N", sym3 );

        KSymbolWhack ( & sym3 -> n, NULL );
        KSymbolWhack ( & sym2 -> n, NULL );
        KSymbolWhack ( & sym1 -> n, NULL );
    }

    test_printf ( "version 1", "version %V", 0x1000000 );
    test_printf ( "version ", "version %.V", 0x1000000 );
    test_printf ( "version     1", "version %5V", 0x1000000 );
    test_printf ( "version 1.2", "version %V", 0x1020000 );
    test_printf ( "version   1.2", "version %5V", 0x1020000 );
    test_printf ( "version 1.2.3", "version %V", 0x1020003 );
    test_printf ( "version 1.2.3", "version %5V", 0x1020003 );
    test_printf ( "version 1.2.3 ", "version %-6V", 0x1020003 );
    test_printf ( "version 1.0", "version %.2V", 0x1000000 );
    test_printf ( "version 1.0.0", "version %.3V", 0x1000000 );
    test_printf ( "version 1.0.0", "version %.4V", 0x1000000 );
    test_printf ( "version 1.0", "version %#.2V", 0x1000000 );

    /* RC can't be tested due to embedded filename and lineno */
#if 0
    rc = RC ( rcExe, rcString, rcFormatting, rcData, rcNoErr );
    test_printf ( "?", "%#R", rc );
#endif

    {
        KTime t;
        t . year = 2011;
        t . month = 9;
        t . day = 28;
        t . weekday = 5;
        t . tzoff = -5 * 60;
        t . hour = 15;
        t . minute = 2;
        t . second = 16;
        t . dst = true;

        test_printf ( "3:02:16 PM", "%T", & t );
        test_printf ( "03:02:16 PM", "%0T", & t );
        test_printf ( "Fri Oct 29 2011 3:02:16 PM", "%lT", & t );
        test_printf ( "Fri Oct 29 2011 3:02:16 PM -5", "%zT", & t );
        test_printf ( "Oct 29 2011", "%hT", & t );
    }
    { /* insufficient buffer; here we have to bypass test_printf since we need a custom buffer size */
        char buff[10];
        size_t num_writ;
        rc = string_printf ( buff, 1, &num_writ, "%s", "0123456789" );
	    if ( rc == 0 )
	    {
    	    pLogErr ( klogErr, rc, "string_vprintf returned zero rc with insufficient buffer", "");
            rc = -1;
	    }
        else
        {
            rc = 0;
        }        
    }

#if LINUX
    test_printf ( "Success", "%!", 0 );
    test_printf ( "Operation not permitted", "%!", 1 );
#endif

    return rc;
    
}


/* Version  EXTERN
 *  return 4-part version code: 0xMMmmrrrr, where
 *      MM = major release
 *      mm = minor release 
 *    rrrr = bug-fix release
 */
ver_t CC KAppVersion ( void )
{
    return 0;
}


/* Usage
 *  This function is called when the command line argument
 *  handling sees -? -h or --help
 */
rc_t CC UsageSummary ( const char *progname )
{
    return KOutMsg (
        "\n"
        "Usage:\n"
        "  %s [Options]\n"
        "\n"
        "Summary:\n"
        "  Simple test of printf.\n"
        , progname );
}

const char UsageDefaultName[] = "time-test";

rc_t CC Usage ( const Args *args )
{
    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;
    rc_t rc;

    if (args == NULL)
        rc = RC (rcApp, rcArgv, rcAccessing, rcSelf, rcNull);
    else
        rc = ArgsProgram (args, &fullpath, &progname);

    UsageSummary (progname);

    KOutMsg ("Options:\n");

    HelpOptionsStandard();

    HelpVersion (fullpath, KAppVersion());

    return rc;
}

    
/* KMain
 */
rc_t CC KMain ( int argc, char *argv [] )
{
    Args *args;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0 );
    if ( rc == 0 )
    {
        KConfigDisableUserSettings();
        rc = run ( argv [ 0 ] );
        ArgsWhack ( args );
    }

    return rc;
}
