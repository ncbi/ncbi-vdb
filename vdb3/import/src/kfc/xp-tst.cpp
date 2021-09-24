/*

  vdb3.kfc.xp-tst


 */

#include "xp.cpp"

#include "ktrace.cpp"
#include "ascii.cpp"
#include "z128.cpp"
#include "fmt-base.cpp"

#include <google/gtest.h>

#include <string>
#include <cstring>

namespace vdb3
{

    TEST ( XPTest, constructor_destructor )
    {
        XP xp ( XLOC );
    }

    TEST ( XPTest, bool_op )
    {
        bool val = true;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, short_int_op )
    {
        short int val = -1234;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, unsigned_short_int_op )
    {
        unsigned short int val = 1234;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, int_op )
    {
        int val = -123456;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, unsigned_int_op )
    {
        unsigned int val = 123456;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, long_int_op )
    {
        long int val = -12345678L;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, unsigned_long_int_op )
    {
        unsigned long int val = 12345678UL;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, long_long_int_op )
    {
        long long int val = -1234567890987654321LL;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, unsigned_long_long_int_op )
    {
        unsigned long long int val = 1234567890987654321ULL;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, float_op )
    {
        float val = 1.23456;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, double_op )
    {
        double val = 1.234567890987;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, long_double_op )
    {
        long double val = 1.23456789098765432123456789;
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, func_op )
    {
        XP xp ( XLOC );
        xp << xprob << xctx << xcause << xsuggest;
    }

    TEST ( XPTest, std_string_op )
    {
        std :: string val ( "bing bang boom" );
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, XMsg_op )
    {
        const ASCII str [] = "bing bang boom";
        XMsg val;
        val . msg_size = sizeof str - 1;
        memmove ( val . zmsg, str, sizeof str );
        XP xp ( XLOC );
        xp << val;
    }

    TEST ( XPTest, putChar_ASCII )
    {
        ASCII ch = 'A';
        XP xp ( XLOC );
        xp << ASCIIChar ( ch );
    }

    TEST ( XPTest, putChar_UTF32 )
    {
        UTF32 ch = 252; // 'ü'
        XP xp ( XLOC );
        xp << ASCIIChar ( ch );
    }

    TEST ( XPTest, putUTF8_zascii )
    {
        const UTF8 * str = "really ascii";
        XP xp ( XLOC );
        xp << ASCIIText ( str );
    }

    TEST ( XPTest, putUTF8_ascii_sz )
    {
        const UTF8 str [] = "really ascii";
        XP xp ( XLOC );
        xp << ASCIIText ( str, sizeof str );
    }

    TEST ( XPTest, putUTF8_zutf8 )
    {
        // this tests nothing...
        // XP is no longer interested in UTF8
        const UTF8 * str = "Einige Bücher";
        XP xp ( XLOC );
        xp << ASCIIText ( str );
    }

    TEST ( XPTest, putUTF8_utf8_sz )
    {
        const UTF8 * str = "Einige Bücher";
        XP xp ( XLOC );
        xp << ASCIIText ( str, :: strlen ( str ) );
    }

    TEST ( XPTest, putPtr )
    {
        const UTF8 * str = "Einige Bücher";
        XP xp ( XLOC );
        xp << Ptr ( str );
    }

    TEST ( XPTest, setRadix )
    {
        XP xp ( XLOC );
        for ( unsigned int r = 0; r < 64; ++ r )
        {
            xp . setRadix ( r );
            xp << 1234;
        }
    }

    TEST ( XPTest, sysError )
    {
        XP xp ( XLOC );
        xp << syserr ( 1 );         // EPERM
    }

    TEST ( XPTest, global_char_op )
    {
        char val = 'n';
        XP xp ( XLOC );
        xp << val << '\n';
    }

    TEST ( XPTest, global_UTF32_char_op )
    {
        // this tests nothing; UTF-8 to ASCII
        UTF32 val = 252;
        XP xp ( XLOC );
        xp << ASCIIChar ( val ) << '\n';
    }

    TEST ( XPTest, global_UTF8_zstr_op )
    {
        XP xp ( XLOC );
        xp << "Einige Bücher" << '\n';
    }

    TEST ( XPTest, putUTF8_xwhat_xcause_xsuggest )
    {
        XP xp ( XLOC );
        xp
            << xprob
            << "this goes to the problem msg"
            << xctx
            << "this goes to the context msg"
            << xcause
            << "and this goes to the cause msg"
            << xsuggest
            << "and finally this goes to the suggestion"
            ;
    }

    TEST ( XPTest, global_radix_ops )
    {
        XP xp ( XLOC );
        xp
            << binary
            << 1234
            << octal
            << 1234
            << decimal
            << 1234
            << hex
            << 1234
            << radix ( 36 )
            << 1234
            << '\n'
            ;
    }

    TEST ( XPTest, syserr_function )
    {
        for ( int err = 0; err < 200; ++ err )
            std :: cout << "errno = " << err << ": " << syserr ( err ) << '\n';
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
