/*

  vdb3.kfc.except-tst


 */

#include "except.cpp"

#include "ktrace.cpp"
#include "memset_s.cpp"
#include "ascii.cpp"
#include "z128.cpp"
#include "fmt-base.cpp"
#include "xp.cpp"

#include <google/gtest.h>

#include <string>
#include <cstring>

namespace vdb3
{

    TEST ( ExceptionTest, constructor_destructor )
    {
        Exception ( XP ( XLOC ) );
    }

    TEST ( ExceptionTest, copy_op_constructor )
    {
        Exception x ( XP ( XLOC ) );
        Exception x1 ( x );
        x = x1;
    }

    TEST ( ExceptionTest, file_path )
    {
        Exception x ( XP ( XLOC ) );
        EXPECT_STREQ ( x . file () . zmsg, "except-tst.cpp" );
    }

    TEST ( ExceptionTest, lineno )
    {
        Exception x ( XP ( XLOC ) );
        EXPECT_EQ ( x . line (), ( unsigned int ) __LINE__ - 1U );
    }

    TEST ( ExceptionTest, function )
    {
        Exception x ( XP ( XLOC ) );
        EXPECT_STREQ ( x . function () . zmsg, __func__ );
    }

    TEST ( ExceptionTest, simple_what )
    {
        const char str [] = "simple what message";
        Exception x (
            XP ( XLOC )
            << str
            );
        const XMsg what = x . what ();
        EXPECT_EQ ( what . msg_size, sizeof str - 1 );
        EXPECT_STREQ ( what . zmsg, str );
    }

    TEST ( ExceptionTest, combined_what_cause_suggest )
    {
        const char pstr [] = "simple problem message";
        const char xstr [] = "simple context message";
        const char cstr [] = "simple cause message";
        const char sstr [] = "simple suggestion";

        Exception x (
            XP ( XLOC )
            << xprob
            << pstr
            << xctx
            << xstr
            << xcause
            << cstr
            << xsuggest
            << sstr
            );

        const XMsg prob = x . problem ();
        EXPECT_EQ ( prob . msg_size, sizeof pstr - 1 );
        EXPECT_STREQ ( prob . zmsg, pstr );

        const XMsg ctx = x . context ();
        EXPECT_EQ ( ctx . msg_size, sizeof xstr - 1 );
        EXPECT_STREQ ( ctx . zmsg, xstr );

        const XMsg cause = x . cause ();
        EXPECT_EQ ( cause . msg_size, sizeof cstr - 1 );
        EXPECT_STREQ ( cause . zmsg, cstr );

        const XMsg sug = x . suggestion ();
        EXPECT_EQ ( sug . msg_size, sizeof sstr - 1 );
        EXPECT_STREQ ( sug . zmsg, sstr );
    }

    TEST ( ExceptionTest, combined_standard_construct )
    {
        std :: string s ( "standard" );
        Exception x (
            XP ( XLOC )
            << "a more '"
            << s
            << "' scenario, size "
            << s . size ()
            << " ( "
            << hex
            << s . size ()
            << " ) [ "
            << binary
            << s . size ()
            << " ]"
            );

        const XMsg what = x . what ();
        EXPECT_EQ ( what . msg_size, 53U );
        EXPECT_STREQ ( what . zmsg, "a more 'standard' scenario, size 8 ( 0x8 ) [ 0b1000 ]" );
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
