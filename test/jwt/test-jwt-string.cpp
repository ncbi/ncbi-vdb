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

/**
* Unit tests for JwtString
*/

#include <ktst/unit_test.hpp>

#include <climits>
#include <cfloat>

using namespace std;

#include <jwt/jwt-string.hpp>

using namespace ncbi;

TEST_SUITE(JwtStringTestSuite);

TEST_CASE ( JwtString_DefaultConstructor_Data )
{
    JwtString str;
    REQUIRE_EQ ( string(), string ( str.data() ) );
}
TEST_CASE ( JwtString_ConstructorCString )
{
    JwtString str("qq");
    REQUIRE_EQ ( string("qq"), string (str.data() ) );
}
TEST_CASE ( JwtString_size )
{
    JwtString str("qq");
    REQUIRE_EQ ( (size_t)2, str.size() );
}
TEST_CASE ( JwtString_ConstructorCString_NULL )
{
    //JwtString str(nullptr);
    //TODO: verify THROW
}
TEST_CASE ( JwtString_ConstructorCStringLen )
{
    JwtString str("qqq", 2);
    REQUIRE_EQ ( string("qq"), string (str.data() ) );
}
TEST_CASE ( JwtString_ConstructorCStringLen_NULL )
{
    //JwtString str(nullptr, 2);
    //TODO: verify THROW
}
TEST_CASE ( JwtString_ConstructorString )
{
    JwtString str1("qq");
    JwtString str2(str1);
    REQUIRE_EQ ( string("qq"), string (str2.data() ) );
}
TEST_CASE ( JwtString_ConstructorString_Empty )
{
    JwtString str1;
    JwtString str2(str1);
    REQUIRE_EQ ( string(), string (str2.data() ) );
    REQUIRE_EQ ( (size_t)0, str2.size() );
}
TEST_CASE ( JwtString_Empty_True )
{
    REQUIRE ( JwtString().empty() );
}
TEST_CASE ( JwtString_Empty_FALSE )
{
    REQUIRE ( ! JwtString("q").empty() );
}
TEST_CASE ( JwtString_c_str )
{
    JwtString str("qq");
    REQUIRE_EQ ( str.data(), str.c_str() );
}

TEST_CASE ( JwtString_find_first_of_Char_FromStart_Found )
{
    JwtString str("123345");
    REQUIRE_EQ ( (size_t)1, str.find_first_of('2') );
}
TEST_CASE ( JwtString_find_first_of_Char_FromStart_NotFound )
{
    JwtString str("123345");
    REQUIRE_EQ ( JwtString::npos, str.find_first_of('6') );
}
TEST_CASE ( JwtString_find_first_of_Char_FromMiddle_Found )
{
    JwtString str("123345");
    REQUIRE_EQ ( (size_t)3, str.find_first_of('3', 3) );
}
TEST_CASE ( JwtString_find_first_of_Char_FromMiddle_NotFound )
{
    JwtString str("123345");
    REQUIRE_EQ ( JwtString::npos, str.find_first_of('3', 4) );
}

TEST_CASE ( JwtString_find_first_of_String_FromStart_Found )
{
    JwtString str("123345");
    REQUIRE_EQ ( (size_t)1, str.find_first_of("2345") );
}
TEST_CASE ( JwtString_find_first_of_String_FromStart_NotFound )
{
    JwtString str("123345");
    REQUIRE_EQ ( JwtString::npos, str.find_first_of("678") );
}
TEST_CASE ( JwtString_find_first_of_String_FromMiddle_Found )
{
    JwtString str("123345");
    REQUIRE_EQ ( (size_t)3, str.find_first_of("123", 3) );
}
TEST_CASE ( JwtString_find_first_of_String_FromMiddle_NotFound )
{
    JwtString str("123345");
    REQUIRE_EQ ( JwtString::npos, str.find_first_of("12", 3) );
}

TEST_CASE ( JwtString_substr_copy )
{
    JwtString str("123345");
    REQUIRE_EQ ( string(str.data()), string(str.substr().data()) );
}
TEST_CASE ( JwtString_substr_middle )
{
    JwtString str("123345");
    REQUIRE_EQ ( string("345"), string(str.substr(3).data()) );
}
TEST_CASE ( JwtString_substr_middle_len )
{
    JwtString str("123345");
    REQUIRE_EQ ( string("34"), string(str.substr(3, 2).data()) );
}

TEST_CASE ( JwtString_clear )
{
    JwtString str("123345");
    str.clear();
    REQUIRE ( str.empty() );
}

TEST_CASE ( JwtString_compare_string_less )
{
    JwtString str("023345");
    REQUIRE_GT ( 0, str.compare("123345") );
}
TEST_CASE ( JwtString_compare_string_shorter )
{
    JwtString str("2345");
    REQUIRE_GT ( 0, str.compare("234567") );
}
TEST_CASE ( JwtString_compare_string_eq )
{
    JwtString str("123345");
    REQUIRE_EQ ( 0, str.compare("123345") );
}
TEST_CASE ( JwtString_compare_string_greater )
{
    JwtString str("124345");
    REQUIRE_LT ( 0, str.compare("123345") );
}
TEST_CASE ( JwtString_compare_string_pos_less )
{
    JwtString str("12345");
    REQUIRE_GT ( 0, str.compare(1, 6, "345") );
}
TEST_CASE ( JwtString_compare_string_pos_size_less )
{
    JwtString str("12345");
    REQUIRE_GT ( 0, str.compare(1, 4, "234567", 7) );
}
TEST_CASE ( JwtString_compare_string_pos_size_eq )
{
    JwtString str("12345");
    REQUIRE_EQ ( 0, str.compare(1, 4, "234567", 4) );
}

TEST_CASE ( JwtString_compare_JwtString_pos_len_subpos_len_less )
{
    JwtString str1("012345");
    JwtString str2("123456");
    REQUIRE_GT ( 0, str1.compare(1, 4, str2, 1, 4) );
}
TEST_CASE ( JwtString_compare_JwtString_pos_len_subpos_len_wqs )
{
    JwtString str1("012345");
    JwtString str2("123456");
    REQUIRE_EQ ( 0, str1.compare(2, 4, str2, 1, 4) );
}

TEST_CASE ( JwtString_find_Found )
{
    JwtString str("012345");
    REQUIRE_EQ ( (size_t)2, str.find("23") );
}
TEST_CASE ( JwtString_find_NotFound )
{
    JwtString str("012345");
    REQUIRE_EQ ( JwtString::npos, str.find("32") );
}
TEST_CASE ( JwtString_find_pos_Found )
{
    JwtString str("01234523");
    REQUIRE_EQ ( (size_t)6, str.find("23", 3) );
}
TEST_CASE ( JwtString_find_pos_NotFound )
{
    JwtString str("012345");
    REQUIRE_EQ ( JwtString::npos, str.find("23", 3) );
}

TEST_CASE ( JwtString_Increment_JwtString )
{
    JwtString str1("012345");
    JwtString str2("678");
    str1+=str2;
    REQUIRE_EQ ( string("012345678"), string(str1.data()) );
}
TEST_CASE ( JwtString_Increment_Char )
{
    JwtString str("012345");
    str+='6';
    REQUIRE_EQ ( string("0123456"), string(str.data()) );
}
TEST_CASE ( JwtString_Increment_At )
{
    JwtString str("012345");
    REQUIRE_EQ ( '1', str[1] );
}

TEST_CASE ( JwtString_Less )
{
    JwtString str1("012345");
    JwtString str2("678");
    REQUIRE ( str1 < str2 );
}
TEST_CASE ( JwtString_Greater )
{
    JwtString str1("012345");
    JwtString str2("678");
    REQUIRE ( ! ( str2 < str1 ) );
}

TEST_CASE ( JwtString_Plus_string )
{
    JwtString str("12345");
    REQUIRE_EQ ( string("12345678"), string ( ( str + "678" ) . data() ) );
}
TEST_CASE ( JwtString_Plus_char )
{
    JwtString str("12345");
    REQUIRE_EQ ( string("123456"), string ( ( str + '6' ) . data() ) );
}
TEST_CASE ( JwtString_Plus_JwtString )
{
    JwtString str1("12345");
    JwtString str2("678");
    REQUIRE_EQ ( string("12345678"), string ( ( str1 + str2 ) . data() ) );
}

TEST_CASE ( JwtString_Stoi )
{
    JwtString str("12345");
    REQUIRE_EQ ( 12345, ncbi::stoi(str) );
}

TEST_CASE ( JwtString_Stoll )
{
    JwtString str("9223372036854775807 ");
    REQUIRE_EQ ( LLONG_MAX , ncbi::stoll(str) );
}

TEST_CASE ( JwtString_Stold )
{
    JwtString str("3.14 ");
    REQUIRE_CLOSE ( (long double)3.14 , ncbi::stold(str), DBL_EPSILON );
}

TEST_CASE ( JwtString_to_string_Unsigned )
{
    REQUIRE_EQ ( string("123") , string(ncbi::to_string(123u).data()) );
}
TEST_CASE ( JwtString_to_string_LongLong )
{
    REQUIRE_EQ ( string("123") , string(ncbi::to_string(123ll).data()) );
}

//////////////////////////////////////////// Main

extern "C"
{

#include <kapp/args.h>

ver_t CC KAppVersion ( void )
{
return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
return 0;
}

rc_t CC Usage ( const Args * args )
{
return 0;
}

const char UsageDefaultName[] = "test-jwt-string";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=JwtStringTestSuite(argc, argv);
    return rc;
}

}
