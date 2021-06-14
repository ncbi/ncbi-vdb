/*

  vdb3.cmn.string-tst

 */

#include <vdb3/kfc/rsrc-mem.hpp>

#include "string.cpp"
#include "str-iter.cpp"

#include "buffer.cpp"
#include "text-utf8.cpp"
#include "serial.cpp"
#include "dserial.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{
    RsrcMem rsrc;

    /*=================================================*
     *                   StringTest                    *
     *=================================================*/

    TEST ( StringTest, constructor_destructor )
    {
        String str;
    }

    TEST ( StringTest, constructor_from_manifest_cstring_constant )
    {
        String str1 ( "a manifest string" );
        String str2 = "another manifest string";
    }

    TEST ( StringTest, constructor_from_manifest_string_constant_and_size )
    {
        const UTF8 utf8 [] = "a manifest string";
        String str ( CTextLiteral ( utf8, sizeof utf8 - 1 ) );
        EXPECT_EQ ( str . size (), sizeof utf8 - 1 );
        EXPECT_EQ ( str . count (), sizeof utf8 - 1 );
        EXPECT_EQ ( str . length (), sizeof utf8 - 1 );
    }

    TEST ( StringTest, constructor_from_manifest_unicode_cstring_constant )
    {
        const UTF8 utf8 [] = "Einige Bücher";
        String str = utf8;
        EXPECT_EQ ( str . size (), sizeof utf8 - 1 );
        EXPECT_EQ ( str . count (), sizeof utf8 - 2 );
        EXPECT_EQ ( str . length (), sizeof utf8 - 2 );
    }

    TEST ( StringTest, constructor_from_stl_ascii_string )
    {
        std :: string ascii ( "a manifest string" );
        String str ( rsrc, ascii );
        EXPECT_EQ ( str . size (), ascii . size () );
        EXPECT_EQ ( str . count (), ascii . size () );
        EXPECT_EQ ( str . length (), ascii . size () );
    }

    TEST ( StringTest, constructor_from_stl_unicode_string )
    {
        std :: string utf8 ( "Einige Bücher" );
        String str ( rsrc, utf8 );
        EXPECT_EQ ( str . size (), utf8 . size () );
        EXPECT_EQ ( str . count (), utf8 . size () - 1 );
        EXPECT_EQ ( str . length (), utf8 . size () - 1 );
    }

    TEST ( StringTest, constructor_from_stl_ASCII_string_with_control_char )
    {
        // NB - the "\x07" escape sequence is allowed to extend
        // to as many hex digits as it wants (great), and collides
        // with the 'd' of "doo" and essentially creates a different
        // character.
        std :: string utf8 ( "zippity\t\x07" "doo\ndah!" );
        String str ( rsrc, utf8 );
        EXPECT_EQ ( str . size (), utf8 . size () );
        EXPECT_EQ ( str . count (), utf8 . size () );
        EXPECT_EQ ( str . length (), utf8 . size () );
        EXPECT_EQ ( str [ 7 ], ( UTF32 ) '\t' );
        EXPECT_EQ ( str [ 8 ], ( UTF32 ) 7 );
        EXPECT_EQ ( str [ 9 ], ( UTF32 ) 'd' );
    }

    TEST ( StringTest, constructor_from_ascii_utf16 )
    {
        const UTF8 utf8 [] = "a manifest string";
        UTF16 utf16 [ sizeof utf8 ];
        for ( size_t i = 0; i < sizeof utf8; ++ i )
            utf16 [ i ] = utf8 [ i ];
        String str ( rsrc, utf16 );
        EXPECT_EQ ( str . size (), sizeof utf8 - 1 );
        EXPECT_EQ ( str . count (), sizeof utf8 - 1 );
        EXPECT_EQ ( str . length (), sizeof utf8 - 1 );
    }

    TEST ( StringTest, constructor_from_unicode_utf16 )
    {
        const char utf8 [] = "Einige Bücher";

        size_t i, offset;
        UTF32 utf32 [ sizeof utf8 ];
        for ( i = offset = 0; offset < sizeof utf8 - 1; ++ i )
        {
            UniChar ch = utf8_to_utf32 ( utf8, sizeof utf8 - 1, offset );
            utf32 [ i ] = ch . ch;
            offset += ch . size;
        }
        utf32 [ i ] = 0;
        
        UTF16 utf16 [ sizeof utf8 ];
        for ( i = offset = 0; utf32 [ i ] != 0; ++ offset, ++ i )
        {
            if ( utf32 [ i ] < 0x10000 )
                utf16 [ offset ] = utf32 [ i ];
            else
            {
                UTF32 ch = utf32 [ i ] - 0x10000;
                utf16 [ offset + 0 ] = ( ch >> 10 ) + 0xD800;
                utf16 [ offset + 1 ] = ( ch & 0x3FF ) + 0xDC00;
                ++ offset;
            }
        }
        utf16 [ offset ] = 0;

        String str ( rsrc, utf16 );
        EXPECT_EQ ( str . size (), sizeof utf8 - 1 );
        EXPECT_EQ ( str . count (), sizeof utf8 - 2 );
        EXPECT_EQ ( str . length (), sizeof utf8 - 2 );
    }

    TEST ( StringTest, constructor_from_unicode_utf32 )
    {
        const char utf8 [] = "Einige Bücher";

        size_t i, offset;
        UTF32 utf32 [ sizeof utf8 ];
        for ( i = offset = 0; offset < sizeof utf8 - 1; ++ i )
        {
            UniChar ch = utf8_to_utf32 ( utf8, sizeof utf8 - 1, offset );
            utf32 [ i ] = ch . ch;
            offset += ch . size;
        }
        utf32 [ i ] = 0;

        String str ( rsrc, utf32 );
        EXPECT_EQ ( str . size (), sizeof utf8 - 1 );
        EXPECT_EQ ( str . count (), sizeof utf8 - 2 );
        EXPECT_EQ ( str . length (), sizeof utf8 - 2 );
    }

    TEST ( StringTest, copy_constructor_op )
    {
        String str1 = "abc";
        String str2 ( str1 );
        str1 = str2;
    }

    TEST ( StringTest, predicates )
    {
        String str1;
        EXPECT_EQ ( str1 . isEmpty (), true );
        EXPECT_EQ ( str1 . isAscii (), true );

        String str2 = "abc";
        EXPECT_EQ ( str2 . isEmpty (), false );
        EXPECT_EQ ( str2 . isAscii (), true );

        String str3 = "¿dónde están las uvas?";
        EXPECT_EQ ( str3 . isEmpty (), false );
        EXPECT_EQ ( str3 . isAscii (), false );
    }

    TEST ( StringTest, properties )
    {
        String str1;
        EXPECT_EQ ( str1 . size (), 0U );
        EXPECT_EQ ( str1 . count (), 0U );

        std :: string ascii = "abc";
        String str2 ( rsrc, ascii );
        EXPECT_EQ ( str2 . size (), ascii . size () );
        EXPECT_EQ ( str2 . count (), ascii . size () );

        std :: string utf8 = "¿dónde están las uvas?";
        String str3 ( rsrc, utf8 );
        EXPECT_EQ ( str3 . size (), utf8 . size () );
        EXPECT_EQ ( str3 . count (), utf8 . size () - 3 );
    }

    TEST ( StringTest, accessors )
    {
        String str1;
        EXPECT_EQ ( str1 . data () . addr (), nullptr );
        EXPECT_ANY_THROW ( str1 . getChar ( 0 ) );
        EXPECT_ANY_THROW ( str1 . getChar ( 1 ) );

        std :: string ascii = "abc";
        String str2 ( rsrc, ascii );
        EXPECT_NE ( str2 . data () . addr (), nullptr );
        EXPECT_EQ ( str2 . getChar ( 0 ), ( UTF32 ) 'a' );
        EXPECT_EQ ( str2 . getChar ( 1 ), ( UTF32 ) 'b' );
        EXPECT_EQ ( str2 . getChar ( 2 ), ( UTF32 ) 'c' );
        EXPECT_ANY_THROW ( str2 . getChar ( 3 ) );

        std :: string utf8 = "¿dónde están las uvas?";
        String str3 ( rsrc, utf8 );
        EXPECT_NE ( str3 . data () . addr (), nullptr );
        EXPECT_EQ ( str3 . getChar (  0 ), ( UTF32 ) 0xBF );
        EXPECT_EQ ( str3 . getChar (  1 ), ( UTF32 )  'd' );
        EXPECT_EQ ( str3 . getChar (  2 ), ( UTF32 ) 0xF3 );
        EXPECT_EQ ( str3 . getChar (  3 ), ( UTF32 )  'n' );
        EXPECT_EQ ( str3 . getChar (  4 ), ( UTF32 )  'd' );
        EXPECT_EQ ( str3 . getChar (  5 ), ( UTF32 )  'e' );
        EXPECT_EQ ( str3 . getChar (  6 ), ( UTF32 )  ' ' );
        EXPECT_EQ ( str3 . getChar (  7 ), ( UTF32 )  'e' );
        EXPECT_EQ ( str3 . getChar (  8 ), ( UTF32 )  's' );
        EXPECT_EQ ( str3 . getChar (  9 ), ( UTF32 )  't' );
        EXPECT_EQ ( str3 . getChar ( 10 ), ( UTF32 ) 0xE1 );
        EXPECT_EQ ( str3 . getChar ( 11 ), ( UTF32 )  'n' );
        EXPECT_EQ ( str3 . getChar ( 12 ), ( UTF32 )  ' ' );
        EXPECT_EQ ( str3 . getChar ( 13 ), ( UTF32 )  'l' );
        EXPECT_EQ ( str3 . getChar ( 14 ), ( UTF32 )  'a' );
        EXPECT_EQ ( str3 . getChar ( 15 ), ( UTF32 )  's' );
        EXPECT_EQ ( str3 . getChar ( 16 ), ( UTF32 )  ' ' );
        EXPECT_EQ ( str3 . getChar ( 17 ), ( UTF32 )  'u' );
        EXPECT_EQ ( str3 . getChar ( 18 ), ( UTF32 )  'v' );
        EXPECT_EQ ( str3 . getChar ( 19 ), ( UTF32 )  'a' );
        EXPECT_EQ ( str3 . getChar ( 20 ), ( UTF32 )  's' );
        EXPECT_EQ ( str3 . getChar ( 21 ), ( UTF32 )  '?' );
        EXPECT_ANY_THROW ( str3 . getChar ( 22 ) );

        // perform a slight shuffle to ensure lack of stateful behavior
        EXPECT_EQ ( str3 . getChar ( 14 ), ( UTF32 )  'a' );
        EXPECT_EQ ( str3 . getChar (  1 ), ( UTF32 )  'd' );
        EXPECT_EQ ( str3 . getChar (  2 ), ( UTF32 ) 0xF3 );
        EXPECT_EQ ( str3 . getChar (  3 ), ( UTF32 )  'n' );
        EXPECT_EQ ( str3 . getChar (  4 ), ( UTF32 )  'd' );
        EXPECT_EQ ( str3 . getChar (  5 ), ( UTF32 )  'e' );
        EXPECT_EQ ( str3 . getChar ( 17 ), ( UTF32 )  'u' );
        EXPECT_EQ ( str3 . getChar (  6 ), ( UTF32 )  ' ' );
        EXPECT_EQ ( str3 . getChar (  0 ), ( UTF32 ) 0xBF );
        EXPECT_EQ ( str3 . getChar (  7 ), ( UTF32 )  'e' );
        EXPECT_EQ ( str3 . getChar (  8 ), ( UTF32 )  's' );
        EXPECT_EQ ( str3 . getChar (  9 ), ( UTF32 )  't' );
        EXPECT_EQ ( str3 . getChar ( 10 ), ( UTF32 ) 0xE1 );
        EXPECT_EQ ( str3 . getChar ( 11 ), ( UTF32 )  'n' );
        EXPECT_EQ ( str3 . getChar ( 12 ), ( UTF32 )  ' ' );

        // TBD - 3 and 4 byte UTF-8 characters
    }

    TEST ( StringTest, iterator )
    {
        std :: string utf8 = "¿dónde están las uvas?";
        String str ( rsrc, utf8 );
        StringIterator it = str . makeIterator ();

        EXPECT_EQ ( it . isValid (), true );
        EXPECT_EQ ( it . pos () . index (), 0 );
        EXPECT_EQ ( it . pos () . offset (), 0U );
        EXPECT_EQ ( * it, ( UTF32 ) 0xBF );

        ++ it;
        EXPECT_EQ ( it . isValid (), true );
        EXPECT_EQ ( it . pos () . index (), 1 );
        EXPECT_EQ ( it . pos () . offset (), 2U );
        EXPECT_EQ ( * it, ( UTF32 ) 'd' );

        ++ it;
        EXPECT_EQ ( it . isValid (), true );
        EXPECT_EQ ( it . pos () . index (), 2 );
        EXPECT_EQ ( it . pos () . offset (), 3U );
        EXPECT_EQ ( * it, ( UTF32 ) 0xF3 );

        ++ it;
        EXPECT_EQ ( it . isValid (), true );
        EXPECT_EQ ( it . pos () . index (), 3 );
        EXPECT_EQ ( it . pos () . offset (), 5U );
        EXPECT_EQ ( * it, ( UTF32 ) 'n' );

        ++ it;
        EXPECT_EQ ( it . isValid (), true );
        EXPECT_EQ ( it . pos () . index (), 4 );
        EXPECT_EQ ( it . pos () . offset (), 6U );
        EXPECT_EQ ( * it, ( UTF32 ) 'd' );

        ++ it; //  5
        ++ it; //  6
        ++ it; //  7
        ++ it; //  8
        ++ it; //  9

        ++ it;
        EXPECT_EQ ( it . isValid (), true );
        EXPECT_EQ ( it . pos () . index (), 10 );
        EXPECT_EQ ( it . pos () . offset (), 12U );
        EXPECT_EQ ( * it, ( UTF32 ) 0xE1 );

        ++ it;
        EXPECT_EQ ( it . isValid (), true );
        EXPECT_EQ ( it . pos () . index (), 11 );
        EXPECT_EQ ( it . pos () . offset (), 14U );
        EXPECT_EQ ( * it, ( UTF32 ) 'n' );

        ++ it; // 12
        ++ it; // 13
        ++ it; // 14
        ++ it; // 15
        ++ it; // 16
        ++ it; // 17
        ++ it; // 18
        ++ it; // 19
        ++ it; // 20

        ++ it;
        EXPECT_EQ ( it . isValid (), true );
        EXPECT_EQ ( it . pos () . index (), 21 );
        EXPECT_EQ ( it . pos () . offset (), 24U );
        EXPECT_EQ ( * it, ( UTF32 ) '?' );

        ++ it;
        EXPECT_EQ ( it . isValid (), false );
        EXPECT_EQ ( it . pos () . index (), 22 );
        EXPECT_EQ ( it . pos () . offset (), 25U );
        EXPECT_ANY_THROW ( * it );

        -- it; // 21
        -- it; // 20
        -- it; // 19
        -- it; // 18
        -- it; // 17
        -- it; // 16

        -- it;
        EXPECT_EQ ( it . isValid (), true );
        EXPECT_EQ ( it . pos () . index (), 15 );
        EXPECT_EQ ( it . pos () . offset (), 18U );
        EXPECT_EQ ( * it, ( UTF32 ) 's' );
    }

    /*=================================================*
     *                NULTermStringTest                *
     *=================================================*/

    TEST ( NULTermStringTest, test_c_str )
    {
        std :: string utf8 = "¿dónde están las uvas?";
        String str ( rsrc, utf8 );
        NULTermString zstr ( rsrc, str );
        EXPECT_STREQ ( utf8 . c_str (), zstr . c_str () );
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
