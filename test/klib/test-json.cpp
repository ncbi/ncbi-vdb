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
* Unit tests for JSON scanner/parser
*/

#include <klib/json.h>

#include <ktst/unit_test.hpp>

#include <stdexcept>
#include <climits>
#include <cfloat>
#include <limits>

#include <klib/text.h>
#include <klib/namelist.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>

#define YYDEBUG 0
#include "../../libs/klib/json-lex.h"
#include "../../libs/klib/json-tokens.h"
extern "C" enum yytokentype JsonScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, JsonScanBlock* sb );

#include "../../libs/klib/json-priv.h"

using namespace std;

TEST_SUITE(KJsonTestSuite);

// Json scanner

static
int
Scan( const char * p_input, string * p_value = 0, bool p_debug = false )
{
    JsonScanBlock sb;
    JsonScan_yylex_init ( & sb, p_input, string_size ( p_input ) );
    JsonScan_set_debug ( & sb, p_debug );
    YYSTYPE lvalp;
    YYLTYPE llocp;
    int ret = JsonScan_yylex ( & lvalp, & llocp, & sb );
    if ( p_value != 0 )
    {
        * p_value = string ( lvalp . value, lvalp . value_len );
    }
    JsonScan_yylex_destroy ( & sb );
    return ret;
}

static
void
VerifyNumber( const char * p_input, bool p_debug = false )
{
    JsonScanBlock sb;
    JsonScan_yylex_init ( & sb, p_input, string_size ( p_input ) );
    JsonScan_set_debug ( & sb, p_debug );
    YYLTYPE llocp;

    YYSTYPE token;
    int type = JsonScan_yylex ( & token, & llocp, & sb );

    THROW_ON_FALSE ( jsonNUMBER == type );
    THROW_ON_FALSE ( jsonNUMBER == token . type );
    THROW_ON_FALSE ( string ( p_input ) == string ( token . value, token . value_len ) );

    JsonScan_yylex_destroy ( & sb );
}

TEST_CASE(KJson_Scan_Empty)             { REQUIRE_EQ ( (int)END_SOURCE, Scan ( "" ) ); }
TEST_CASE(KJson_Scan_Whitespace)        { REQUIRE_EQ ( (int)END_SOURCE, Scan ( " \t\r\n" ) ); }
TEST_CASE(KJson_Scan_LeftBrace)         { REQUIRE_EQ ( (int)'{', Scan ( "{" ) ); }
TEST_CASE(KJson_Scan_RightBrace)        { REQUIRE_EQ ( (int)'}', Scan ( "}" ) ); }
TEST_CASE(KJson_Scan_Equals)            { REQUIRE_EQ ( (int)'}', Scan ( "}" ) ); }
TEST_CASE(KJson_Scan_LeftSqBracket)     { REQUIRE_EQ ( (int)'[', Scan ( "[" ) ); }
TEST_CASE(KJson_Scan_RightSqBracket)    { REQUIRE_EQ ( (int)']', Scan ( "]" ) ); }
TEST_CASE(KJson_Scan_Colon)             { REQUIRE_EQ ( (int)':', Scan ( ":" ) ); }
TEST_CASE(KJson_Scan_Comma)             { REQUIRE_EQ ( (int)',', Scan ( "," ) ); }
TEST_CASE(KJson_Scan_True)              { REQUIRE_EQ ( (int)jsonTRUE, Scan ( "true" ) ); }
TEST_CASE(KJson_Scan_False)             { REQUIRE_EQ ( (int)jsonFALSE, Scan ( "false" ) ); }
TEST_CASE(KJson_Scan_Null)              { REQUIRE_EQ ( (int)jsonNULL, Scan ( "null" ) ); }
TEST_CASE(KJson_Scan_Unrecognized)      { REQUIRE_EQ ( (int)UNRECOGNIZED, Scan ( "%" ) ); }

TEST_CASE(KJson_Scan_EmptyString)
{
    REQUIRE_EQ ( (int)jsonSTRING, Scan ( "\"\"" ) );
}
TEST_CASE(KJson_Scan_String)
{
    string val;
    REQUIRE_EQ ( (int)jsonSTRING, Scan ( "\"qq\"", & val ) );
    REQUIRE_EQ ( string ( "qq" ), val );
}
TEST_CASE(KJson_Scan_StringEscapes)
{
    string val;
    REQUIRE_EQ ( (int)jsonSTRING, Scan ( "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"", & val ) );
    REQUIRE_EQ ( string ( "\\\" \\\\ \\/ \\b \\f \\n \\r \\t" ), val );
}
TEST_CASE(KJson_Scan_OneQuote)
{
    REQUIRE_EQ ( (int)jsonSTRING, Scan ( "\"\\\"\"" ) );
}
TEST_CASE(KJson_Scan_UTF8)
{
    string val;
    REQUIRE_EQ ( (int)jsonSTRING, Scan ( "\"какие-то странные символы\"", & val ) );
    REQUIRE_EQ ( string ( "какие-то странные символы" ), val );
    size_t size;
    REQUIRE_EQ ( 25u, string_measure ( val . c_str (), & size ) );
    REQUIRE_EQ ( (size_t)47, size );
}
TEST_CASE(KJson_Scan_Hex)
{
    string val;
    REQUIRE_EQ ( (int)jsonSTRING, Scan ( "\"\\uabcd\"", & val ) );
    REQUIRE_EQ ( string ( "\\uabcd" ), val );
}
TEST_CASE(KJson_Scan_HexUpper)
{
    string val;
    REQUIRE_EQ ( (int)jsonSTRING, Scan ( "\"\\uABCD\"", & val ) );
    REQUIRE_EQ ( string ( "\\uABCD" ), val );
}

TEST_CASE(KJson_Scan_Int)
{
    VerifyNumber ( "1234" );
}
TEST_CASE(KJson_Scan_Int_OneDigit)
{
    VerifyNumber ( "0" );
}
TEST_CASE(KJson_Scan_Int_Signed_Plus)
{
    VerifyNumber ( "+1234" );
}
TEST_CASE(KJson_Scan_Int_Signed_Mnius)
{
    VerifyNumber ( "-12340" );
}

TEST_CASE(KJson_Scan_Int_Frac)
{
    VerifyNumber ( "12.34" );
}

TEST_CASE(KJson_Scan_Int_Exp_e)
{
    VerifyNumber ( "12e34" );
}
TEST_CASE(KJson_Scan_Int_Exp_E)
{
    VerifyNumber ( "12E34" );
}
TEST_CASE(KJson_Scan_Int_Exp_e_Plus)
{
    VerifyNumber ( "12e+34" );
}
TEST_CASE(KJson_Scan_Int_Exp_E_Minus)
{
    VerifyNumber ( "12E-34" );
}

TEST_CASE(KJson_Scan_Int_Frac_Exp)
{
    VerifyNumber ( "12.34e+22" );
}

static
rc_t
Parse(  KJsonValue ** p_obj, const char * p_str )
{
    return KJsonValueMake ( p_obj, p_str, NULL, 0 );
}

static
string
ParseError(  KJsonValue ** p_obj, const char * p_str )
{
    char error[1024];
    if  ( KJsonValueMake ( p_obj, p_str, error, sizeof ( error ) ) == 0 )
    {
        return string ( " no error ");
    }
    return string ( error );
}

// Construction API

class JsonMake_Fixture
{
public:
    JsonMake_Fixture()
    :   m_val ( 0 ),
        m_obj ( 0 ),
        m_arr ( 0 )
    {
    }
    ~JsonMake_Fixture()
    {
        KJsonValueWhack( m_val );
        KJsonValueWhack( ( KJsonValue * ) KJsonObjectToValue ( m_obj ) );
        KJsonValueWhack( ( KJsonValue * ) KJsonArrayToValue ( m_arr ) );
    }

    KJsonValue *    m_val;
    KJsonObject *   m_obj;
    KJsonArray *    m_arr;
};

FIXTURE_TEST_CASE(Json_GetValueType_SefNull, JsonMake_Fixture)
{
    REQUIRE_EQ ( jsInvalid, KJsonGetValueType ( NULL ) );
}

FIXTURE_TEST_CASE(Json_MakeString, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeString ( & m_val, "qq", 2 ) );
    REQUIRE_EQ ( jsString, KJsonGetValueType ( m_val ) );
}
FIXTURE_TEST_CASE(Json_MakeString_Empty, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeString ( & m_val, "", 0 ) );
    REQUIRE_EQ ( jsString, KJsonGetValueType ( m_val ) );
}

FIXTURE_TEST_CASE(Json_MakeNumber, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeNumber ( & m_val, "4321", 4 ) );
    REQUIRE_EQ ( jsNumber, KJsonGetValueType ( m_val ) );
}

FIXTURE_TEST_CASE(Json_MakeBool, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeBool ( & m_val, true ) );
    REQUIRE_EQ ( jsBool, KJsonGetValueType ( m_val ) );
}

FIXTURE_TEST_CASE(Json_MakeNull, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeNull ( & m_val ) );
    REQUIRE_NOT_NULL ( m_val );
    REQUIRE_EQ ( jsNull, KJsonGetValueType ( m_val ) );
}

FIXTURE_TEST_CASE(Json_MakeObject, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeObject ( & m_obj ) );
    const KJsonValue * v = KJsonObjectToValue ( m_obj );
    REQUIRE_EQ ( jsObject, KJsonGetValueType ( v ) );
}

FIXTURE_TEST_CASE(Json_Object_AddMember, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeObject ( & m_obj ) );
    KJsonValue * v;
    REQUIRE_RC ( KJsonMakeNull ( & v ) );
    string name = "name";
    REQUIRE_RC ( KJsonObjectAddMember ( m_obj, name . c_str (), name . size (), v ) );
}

FIXTURE_TEST_CASE(Json_MakeArray, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeArray ( & m_arr ) );
    const KJsonValue * v = KJsonArrayToValue ( m_arr );
    REQUIRE_EQ ( jsArray, KJsonGetValueType ( v ) );
}
//TODO: KJsonArrayAddElement

// KJson public API

class KJsonFixture : public JsonMake_Fixture
{
public:
    KJsonFixture()
    :   m_names ( 0 ),
        m_str ( 0 ),
        m_int64 ( 0 ),
        m_double ( 0.0 )
    {
        memset ( & m_buf, 0, sizeof m_buf );
    }

    ~KJsonFixture()
    {
        VNamelistRelease ( m_names );
        KDataBufferWhack ( & m_buf );
    }

    const KJsonValue * ParseValue( const char * p_input )
    {
        THROW_ON_RC ( Parse ( & m_val, p_input ) );
        THROW_ON_FALSE ( m_val != NULL);
        return m_val;
    }
    const KJsonObject * ParseObject( const char * p_input )
    {
        KJsonValue * val;
        THROW_ON_RC ( Parse ( & val, p_input ) );
        m_obj = ( KJsonObject * ) KJsonValueToObject ( val );
        THROW_ON_FALSE ( m_obj != NULL);
        return m_obj;
    }
    const KJsonArray * ParseArray( const char * p_input )
    {
        KJsonValue * val;
        THROW_ON_RC ( Parse ( & val, p_input ) );
        m_arr = ( KJsonArray * ) KJsonValueToArray ( val );
        THROW_ON_FALSE ( m_arr != NULL);
        return m_arr;
    }

    VNamelist *     m_names;
    const char *    m_str;
    int64_t         m_int64;
    double          m_double;
    bool            m_bool;
    KDataBuffer     m_buf;
};

// KJsonMake

FIXTURE_TEST_CASE(Json_Make_NullSelf, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonValueMake ( NULL, m_str, NULL, 0 ) );
}
FIXTURE_TEST_CASE(Json_Make_NullParam, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonValueMake ( & m_val, NULL, NULL, 0 ) );
}

// KJsonValueWhack

FIXTURE_TEST_CASE(Json_MakeWhack, KJsonFixture)
{
    KJsonValue * root;
    REQUIRE_RC ( KJsonValueMake ( & root, "{}", NULL, 0 ) );
    REQUIRE_NOT_NULL ( root );
    KJsonValueWhack ( root );
}

// parsing

FIXTURE_TEST_CASE(KJson_Parse_NullSelf, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( NULL, "{}" ) );
}
FIXTURE_TEST_CASE(KJson_Parse_EmptyInput, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_val, "" ) );
    REQUIRE_NULL ( m_obj );
}
FIXTURE_TEST_CASE(KJson_Parse_ExpectRightBrace, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_val, "{" ) );
}
FIXTURE_TEST_CASE(KJson_Parse_ExpectLeftBrace, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_val, "}" ) );
}
FIXTURE_TEST_CASE(KJson_Parse_ExpectColon, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_val, "{\"name\"\"value\"" ) );
}
FIXTURE_TEST_CASE(KJson_Parse_ExpectRightBrace2, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_val, "{\"name\":\"value\"" ) );
    REQUIRE_NULL ( m_obj );
}
FIXTURE_TEST_CASE(KJson_Parse_EmptyObject, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_val, " { } " ) );
    REQUIRE_NOT_NULL ( m_val );
}
FIXTURE_TEST_CASE(KJson_Parse_Object, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_val, "{\"name1\":\"value\", \"name2\":\":value\"}" ) );
    REQUIRE_NOT_NULL ( m_val );
}
FIXTURE_TEST_CASE(KJson_Parse_Object_SimilarNames, KJsonFixture)
{   // VDB-3660: { "xxA":..., "xx":...} fails
    REQUIRE_RC ( Parse ( & m_val, "{\"nameA\":\"value\", \"name\":\":value\"}" ) );
    REQUIRE_NOT_NULL ( m_val );
}
FIXTURE_TEST_CASE(KJson_Parse_Object_DuplicateName_ErrorReporting, KJsonFixture)
{
    string expectedStart = "line 1, col 33: RC("; // in debug build, this is followed by source code location
    string expectedEnd = "rcCont,rcTree,rcInserting,rcNode,rcExists)";
    string actual = ParseError ( & m_val, "{\"name\":\"value\", \"name\":\":value\"}" );
    REQUIRE_EQ ( expectedStart, actual . substr( 0, expectedStart . size () ) );
    REQUIRE_EQ ( expectedEnd, actual . substr( actual . size () - expectedEnd . size () ) );
    REQUIRE_NULL ( m_val );
}
FIXTURE_TEST_CASE(KJson_Parse_Object_BadUnicodeInName, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_val, "{\"\\uDFAA\":0}" ) );
}

FIXTURE_TEST_CASE(KJson_ParseArray_ExpectRightBracket, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_val, "[}" ) );
}
FIXTURE_TEST_CASE(KJson_ParseArray_ExpectRightBracket2, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_val, "[\"name\",\"name\"" ) );
}
FIXTURE_TEST_CASE(KJson_ParseArray_ExpectLeftBracket, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_val, "\"name\",\"name\"]" ) );
}
FIXTURE_TEST_CASE(KJson_ParseArray_Empty, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_val, "[]" ) );
}
FIXTURE_TEST_CASE(KJson_ParseArray_String_Elems, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_val, "[\"name\",\"name\"]" ) );
}

FIXTURE_TEST_CASE(KJson_Parse_Leak, KJsonFixture)
{
    // this overflows bison's stack if it is originally allocated at the default size (YYINITDEPTH=200);
    // they do reallocate it but then do not deallocate, which causes a leak.
    // Now, YYINITDEPTH is set to 10000 in json-grammar.y.
    REQUIRE_RC ( Parse ( & m_val, "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]" ) );
}

FIXTURE_TEST_CASE(KJson_Parse_DistinguishTruncatedInput, KJsonFixture)
{
    rc_t rc = Parse ( & m_val, "{" );
    REQUIRE_EQ ( rcIncomplete, GetRCState ( rc ) );
}

// KJsonGetString

FIXTURE_TEST_CASE(KJsonGetString_NulLSelf, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetString ( NULL, & m_str ) );
}
FIXTURE_TEST_CASE(KJsonGetString_NullParam, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetString ( ParseValue ( "\"abcd\"" ), NULL ) );
}
FIXTURE_TEST_CASE(KJsonGetString_WrongType, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetString ( ParseValue ( "null" ),  & m_str ) );
}
FIXTURE_TEST_CASE(KJsonGetString_Simple, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetString ( ParseValue ( "\"abcd\"" ),  & m_str ) );
    REQUIRE_EQ ( string ( "abcd" ), string ( m_str ) );
}

FIXTURE_TEST_CASE(KJsonGetString_Escapes, KJsonFixture)
{
    const KJsonValue * node = ParseValue ( "\" \\\" \\\\ \\/ \\b \\f \\n \\r \\t \"" );
    REQUIRE_RC ( KJsonGetString ( node, & m_str ) );
    REQUIRE_EQ ( string ( " \" \\ / \b \f \n \r \t " ), string ( m_str ) );
}

FIXTURE_TEST_CASE(KJsonGetString_Utf16_to_Utf8, KJsonFixture)
{
    // странные using UTF-16
    const KJsonValue * node = ParseValue ( "\"\\u0441\\u0442\\u0440\\u0430\\u043d\\u043d\\u044b\\u0435\"" );
    REQUIRE_RC ( KJsonGetString ( node, & m_str ) );
    REQUIRE_EQ ( string ( "странные" ), string ( m_str ) );
}
FIXTURE_TEST_CASE(KJsonGetString_Utf16_SurrogatePair_to_Utf8, KJsonFixture)
{
    const KJsonValue * node = ParseValue ( "\"\\udbff\\udfff\"" ); // U+10FFFF
    REQUIRE_RC ( KJsonGetString ( node, & m_str ) );
    const char * expected =  "\xf4\x8f\xbf\xbf"; // U+10FFFF in UTF-8
    REQUIRE_EQ ( 1u, string_len ( expected, 4 ) );
    REQUIRE_EQ ( string ( expected ), string ( m_str ) );
}
FIXTURE_TEST_CASE(KJsonGetString_Utf16_BadSurrogatePair_1, KJsonFixture)
{   // A lone surrogate character is invalid in UTF-16
    REQUIRE_RC_FAIL ( Parse ( & m_val, "{\"\" : \"\\udbff\" }" ) );
}
FIXTURE_TEST_CASE(KJsonGetString_Utf16_BadSurrogatePair_2, KJsonFixture)
{   // A lone surrogate character is invalid in UTF-16
    REQUIRE_RC_FAIL ( Parse ( & m_val, "{\"\" : \"\\udbff123456\" }" ) );
}
FIXTURE_TEST_CASE(KJsonGetString_Utf16_BadSurrogatePair_3, KJsonFixture)
{   // A lone surrogate character is invalid in UTF-16
    REQUIRE_RC_FAIL ( Parse ( & m_val, "{\"\" : \"\\udbff\\t\" }" ) );
}
FIXTURE_TEST_CASE(KJsonGetString_Utf16_BadSurrogatePair_4, KJsonFixture)
{   // A lone surrogate character is invalid in UTF-16
    REQUIRE_RC_FAIL ( Parse ( & m_val, "{\"\" : \"\\udbff\\u123\" }" ) );
}
FIXTURE_TEST_CASE(KJsonGetString_Utf16_BadSurrogatePair_5, KJsonFixture)
{   // Low surrogate out of range DC00-DFFF
    REQUIRE_RC_FAIL ( Parse ( & m_val, "{\"\" : \"\\udbff\\uCC00\" }" ) );
}
FIXTURE_TEST_CASE(KJsonGetString_Utf16_BadSurrogatePair_6, KJsonFixture)
{   // Low surrogate out of range DC00-DFFF
    REQUIRE_RC_FAIL ( Parse ( & m_val, "{\"\" : \"\\udbff\\uE000\" }" ) );
}

// KJsonGetNumber

FIXTURE_TEST_CASE(KJsonGetNumber_NullSelf, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetNumber ( NULL, & m_int64 ) );
}
FIXTURE_TEST_CASE(KJsonGetNumber_NullParam, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetNumber ( ParseValue ( "0" ), NULL ) );
}

FIXTURE_TEST_CASE(KJsonGetNumber_WrongType, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetNumber ( ParseValue ( "null" ), & m_int64 ) );
}

FIXTURE_TEST_CASE(KJsonGetNumber_NotAnInteger, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetNumber ( ParseValue ( "0.1" ), & m_int64 ) );
}
FIXTURE_TEST_CASE(KJsonGetNumber_Unsigned, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetNumber ( ParseValue ( "9223372036854775807" ), & m_int64 ) );
    REQUIRE_EQ ( (int64_t)LLONG_MAX, m_int64 );
}
FIXTURE_TEST_CASE(KJsonGetNumber_UnsignedOverflow, KJsonFixture)
{   // LLONG_MAX + 1
    REQUIRE_RC_FAIL ( KJsonGetNumber ( ParseValue ( "9223372036854775808" ), & m_int64 ) );
}
FIXTURE_TEST_CASE(KJsonGetNumber_Signed, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetNumber ( ParseValue ( "-9223372036854775808" ), & m_int64 ) );
    REQUIRE_EQ ( (int64_t)LLONG_MIN, m_int64 );
}
FIXTURE_TEST_CASE(KJsonGetNumber_SignedUnderflow, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetNumber ( ParseValue ( "-9223372036854775809" ), & m_int64 ) );
}

// KJsonGetDouble

FIXTURE_TEST_CASE(KJsonGetDouble_NullSelf, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetDouble ( NULL, & m_double ) );
}
FIXTURE_TEST_CASE(KJsonGetDouble_NullParam, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetDouble ( ParseValue ( "0" ), NULL ) );
}
FIXTURE_TEST_CASE(KJsonGetDouble_WrongType, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetDouble ( ParseValue ( "null" ), & m_double ) );
}
FIXTURE_TEST_CASE(KJsonGetNumber_Integer, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetDouble ( ParseValue ( "1" ), & m_double ) );
    REQUIRE_EQ ( 1.0, m_double );
}
FIXTURE_TEST_CASE(KJsonGetNumber_Frac, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetDouble ( ParseValue ( "1.0" ), & m_double ) );
    REQUIRE_EQ ( 1.0, m_double );
}
FIXTURE_TEST_CASE(KJsonGetNumber_Exp_1, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetDouble ( ParseValue ( "1e1" ), & m_double ) );
    REQUIRE_EQ ( 1e1, m_double );
}
FIXTURE_TEST_CASE(KJsonGetNumber_Exp_2, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetDouble ( ParseValue ( "1E1" ), & m_double ) );
    REQUIRE_EQ ( 1e1, m_double );
}
FIXTURE_TEST_CASE(KJsonGetNumber_Exp_3, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetDouble ( ParseValue ( "1e+1" ), & m_double ) );
    REQUIRE_EQ ( 1e+1, m_double );
}
FIXTURE_TEST_CASE(KJsonGetNumber_Exp_4, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetDouble ( ParseValue ( "1e-1" ), & m_double ) );
    REQUIRE_EQ ( 1e-1, m_double );
}
FIXTURE_TEST_CASE(KJsonGetNumber_FracExp, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetDouble ( ParseValue ( "1.2e3" ), & m_double ) );
    REQUIRE_EQ ( 1.2e3, m_double );
}
FIXTURE_TEST_CASE(KJsonGetNumber_Overflow, KJsonFixture)
{   // DBL_MAX + a bit
    REQUIRE_RC_FAIL ( KJsonGetDouble ( ParseValue ( "1.8e+308" ), & m_double ) );
}
FIXTURE_TEST_CASE(KJsonGetNumber_NegOverflow, KJsonFixture)
{   // -DBL_MAX - a bit
    REQUIRE_RC_FAIL ( KJsonGetDouble ( ParseValue ( "-1.8e+308" ), & m_double ) );
}
FIXTURE_TEST_CASE(KJsonGetNumber_Underflow, KJsonFixture)
{   // DBL_MIN - a bit
    REQUIRE_RC_FAIL ( KJsonGetDouble ( ParseValue ( "2.1e-324" ), & m_double ) );
}

// KJsonGetString on numbers
FIXTURE_TEST_CASE(KJsonGetString_Double, KJsonFixture)
{
    REQUIRE_RC ( KJsonGetString ( ParseValue ( "2.1e-324" ), & m_str ) );
    REQUIRE_EQ ( string ( "2.1e-324" ), string ( m_str ) );
}

// KJsonGetBool

FIXTURE_TEST_CASE(KJsonGetBool_NullSelf, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetBool ( NULL, & m_bool ) );
}
FIXTURE_TEST_CASE(KJsonGetBool_NullParam, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetBool ( ParseValue ( "false" ), NULL ) );
}
FIXTURE_TEST_CASE(KJsonGetBool_WrongType, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonGetBool ( ParseValue ( "null" ), & m_bool ) );
}
FIXTURE_TEST_CASE(KJsonGetDouble_True, KJsonFixture)
{
    m_bool = false;
    REQUIRE_RC ( KJsonGetBool ( ParseValue ( "true" ), & m_bool ) );
    REQUIRE ( m_bool );
}
FIXTURE_TEST_CASE(KJsonGetDouble_False, KJsonFixture)
{
    m_bool = true;
    REQUIRE_RC ( KJsonGetBool ( ParseValue ( "false" ), & m_bool ) );
    REQUIRE ( ! m_bool );
}

// KJsonObjectToValue

FIXTURE_TEST_CASE(Json_ObjectToValue_NullSelf, KJsonFixture)
{
    REQUIRE_NULL ( KJsonObjectToValue ( NULL ) );
}
FIXTURE_TEST_CASE(Json_ObjectToValue, KJsonFixture)
{
    REQUIRE_RC ( KJsonMakeObject ( & m_obj ) );
    REQUIRE_NOT_NULL ( KJsonObjectToValue ( m_obj ) );
}

// KJsonValueToObject

FIXTURE_TEST_CASE(Json_ValueToObject_NullSelf, KJsonFixture)
{
    REQUIRE_NULL ( KJsonValueToObject ( NULL ) );
}
FIXTURE_TEST_CASE(Json_ValueToObject_NotAnObject, KJsonFixture)
{
    REQUIRE_RC ( KJsonMakeNull ( & m_val ) );
    REQUIRE_NULL ( KJsonValueToObject ( m_val ) );
}
FIXTURE_TEST_CASE(Json_ValueToObject, KJsonFixture)
{
    REQUIRE_RC ( KJsonMakeObject ( & m_obj ) );
    const KJsonValue * v = KJsonObjectToValue ( m_obj );
    REQUIRE_NOT_NULL ( KJsonValueToObject ( v ) );
}

// KJsonObjectGetNames

FIXTURE_TEST_CASE(KJsonObject_GetNames_NullSelf, KJsonFixture)
{
    REQUIRE_RC ( VNamelistMake ( & m_names, 1 ) );
    REQUIRE_RC_FAIL ( KJsonObjectGetNames ( NULL, m_names ) );
}
FIXTURE_TEST_CASE(KJsonObject_GetNames_NullParam, KJsonFixture)
{
    ParseObject ( "{}" );
    REQUIRE_RC_FAIL ( KJsonObjectGetNames ( m_obj, NULL ) );
}
FIXTURE_TEST_CASE(KJsonObject_GetNames_Empty, KJsonFixture)
{
    ParseObject ( "{}" );
    REQUIRE_RC ( VNamelistMake ( & m_names, 1 ) );
    REQUIRE_RC ( KJsonObjectGetNames ( m_obj, m_names ) );
    uint32_t count;
    REQUIRE_RC ( VNameListCount ( m_names, & count ) );
    REQUIRE_EQUAL ( 0u, count );
}
FIXTURE_TEST_CASE(KJsonObject_GetNames, KJsonFixture)
{
    ParseObject ( "{\"c\":null, \"a\":null, \"b\":null}" );
    REQUIRE_RC ( VNamelistMake ( & m_names, 1 ) );
    REQUIRE_RC ( KJsonObjectGetNames ( m_obj, m_names ) );
    uint32_t count;
    REQUIRE_RC ( VNameListCount ( m_names, & count ) );
    REQUIRE_EQUAL ( 3u, count );
    // m_names are sorted
    const char * name;
    REQUIRE_RC ( VNameListGet ( m_names, 0, & name ) );
    REQUIRE_EQUAL ( string ( "a" ), string ( name ) );
    REQUIRE_RC ( VNameListGet ( m_names, 1, & name ) );
    REQUIRE_EQUAL ( string ( "b" ), string ( name ) );
    REQUIRE_RC ( VNameListGet ( m_names, 2, & name ) );
    REQUIRE_EQUAL ( string ( "c" ), string ( name ) );
}

// KJsonObjectGetMember

FIXTURE_TEST_CASE(KJsonObject_GetMember_NullSelf, KJsonFixture)
{
    REQUIRE_NULL ( KJsonObjectGetMember ( NULL, "a") );
}
FIXTURE_TEST_CASE(KJsonObject_GetValue_NullParam, KJsonFixture)
{
    ParseObject ( "{}" );
    REQUIRE_NULL ( KJsonObjectGetMember ( m_obj, NULL ) );
}
FIXTURE_TEST_CASE(KJsonObject_GetMember_EmptyObject, KJsonFixture)
{
    ParseObject ( "{}" );
    REQUIRE_NULL ( KJsonObjectGetMember ( m_obj, "a" ) );
}
FIXTURE_TEST_CASE(KJsonObject_GetMember_NotFound, KJsonFixture)
{
    ParseObject ( "{\"c\":null, \"a\":null, \"b\":null}" );
    REQUIRE_NULL ( KJsonObjectGetMember ( m_obj, "aa" ) );
}

FIXTURE_TEST_CASE(KJsonObject_GetMember_Found, KJsonFixture)
{
    ParseObject ( "{\"c\":null, \"a\":null, \"b\":{}}" );
    const KJsonValue * v = KJsonObjectGetMember ( m_obj, "b" );
    REQUIRE_NOT_NULL ( v );
    const KJsonObject * obj = KJsonValueToObject ( v );
    REQUIRE_NOT_NULL ( obj );
    REQUIRE_RC ( VNamelistMake ( & m_names, 1 ) );
    REQUIRE_RC ( KJsonObjectGetNames ( obj, m_names ) );
    uint32_t count;
    REQUIRE_RC ( VNameListCount ( m_names, & count ) );
    REQUIRE_EQ ( 0u, count );
}

FIXTURE_TEST_CASE(KJsonObject_QuoteAsMemberName, KJsonFixture)
{
    ParseObject ( "{\"\\\"\":1}" );
    const KJsonValue * v = KJsonObjectGetMember ( m_obj, "\"" );
    REQUIRE_NOT_NULL ( v );
}

// KJsonArrayToValue

FIXTURE_TEST_CASE(Json_ArrayToValue_NullSelf, KJsonFixture)
{
    REQUIRE_NULL ( KJsonArrayToValue ( NULL ) );
}
FIXTURE_TEST_CASE(Json_ArrayToValue, KJsonFixture)
{
    REQUIRE_RC ( KJsonMakeArray ( & m_arr ) );
    REQUIRE_NOT_NULL ( KJsonArrayToValue ( m_arr ) );
}

// KJsonValueToArray

FIXTURE_TEST_CASE(Json_ValueToArray_NullSelf, KJsonFixture)
{
    REQUIRE_NULL ( KJsonValueToArray ( NULL ) );
}
FIXTURE_TEST_CASE(Json_ValueToArray_NotAnArray, KJsonFixture)
{
    REQUIRE_RC ( KJsonMakeNull ( & m_val ) );
    REQUIRE_NULL ( KJsonValueToArray ( m_val ) );
}
FIXTURE_TEST_CASE(Json_ValueToArray, KJsonFixture)
{
    REQUIRE_RC ( KJsonMakeArray ( & m_arr ) );
    const KJsonValue * v = KJsonArrayToValue ( m_arr );
    REQUIRE ( KJsonValueToArray ( v ) );
}

// KJsonArrayGetLength

FIXTURE_TEST_CASE(KJsonArray_GetLength_NullSelf, KJsonFixture)
{
    REQUIRE_EQ ( 0u, KJsonArrayGetLength ( NULL ) );
}
FIXTURE_TEST_CASE(KJsonArray_GetLength_Empty, KJsonFixture)
{
    ParseArray ( "[]" );
    REQUIRE_EQ ( 0u, KJsonArrayGetLength ( m_arr ) );
}
FIXTURE_TEST_CASE(KJsonArray_GetLength, KJsonFixture)
{
    ParseArray ( "[1,2.3e4, null,false , \"\" , {} ]" );
    REQUIRE_EQ ( 6u, KJsonArrayGetLength ( m_arr ) );
}

// KJsonArrayGetElement

FIXTURE_TEST_CASE(KJsonArray_GetElement_NullSelf, KJsonFixture)
{
    REQUIRE_NULL ( KJsonArrayGetElement ( NULL, 0 ) );
}
FIXTURE_TEST_CASE(KJsonArray_GetElement_IndexOutOfRange, KJsonFixture)
{
    ParseArray ( "[1,2.3e4, null,false , \"\" , {} ]" );
    REQUIRE_NULL ( KJsonArrayGetElement ( m_arr, 6 ) );
}
FIXTURE_TEST_CASE(KJsonArray_GetElement_First, KJsonFixture)
{
    ParseArray ( "[1,2.3e4, null,false , \"\" , {} ]" );
    const KJsonValue * val = KJsonArrayGetElement ( m_arr, 0 );
    REQUIRE_EQ ( jsNumber, KJsonGetValueType ( val ) );
}
FIXTURE_TEST_CASE(KJsonArray_GetElement_Last, KJsonFixture)
{
    ParseArray ( "[1,2.3e4, null,false , \"\" , {} ]" );
    const KJsonValue * val = KJsonArrayGetElement ( m_arr, 5 );
    REQUIRE_EQ ( jsObject, KJsonGetValueType ( val ) );
}

// KJsonToJsonString
FIXTURE_TEST_CASE(KJson_ToJsonString_NullSelf, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonToJsonString ( NULL, & m_buf, 0, false ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_NullParam, KJsonFixture)
{
    ParseValue ( "{}" );
    REQUIRE_RC_FAIL ( KJsonToJsonString ( m_val, NULL, 0, false ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_Empty, KJsonFixture)
{
    ParseValue ( "{}" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 1024, false ) );
    REQUIRE_EQ ( string ( "{}" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_Reallocation, KJsonFixture)
{
    ParseValue ( "{}" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 1, false ) ); // p_increment == 1 will cause buffer reallocation
    REQUIRE_EQ ( string ( "{}" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_String, KJsonFixture)
{
    ParseValue ( "\"characters\"" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 0, false ) );
    REQUIRE_EQ ( string ( "\"characters\"" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_String_Escapes, KJsonFixture)
{   // all ASCII control characters that do not have Json escapes are represented as \\u00xx
    ParseValue ( "\" \\\" \\\\ \\/ \\b \\n \\r \\t ы \a \"" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 0, false ) );
    REQUIRE_EQ ( string ( "\" \\\" \\\\ \\/ \\b \\n \\r \\t ы \\\\u0007 \"" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_Number, KJsonFixture)
{
    ParseValue ( "123.45e-1" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 0, false ) );
    REQUIRE_EQ ( string ( "123.45e-1" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_BooleanMember, KJsonFixture)
{
    ParseValue ( "{ \"a\" : true }" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 0, false ) );
    REQUIRE_EQ ( string ( "{\"a\":true}" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_NullMember, KJsonFixture)
{
    ParseValue ( "{ \"a\" : null }" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 0, false ) );
    REQUIRE_EQ ( string ( "{\"a\":null}" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_ObjectMember, KJsonFixture)
{
    ParseValue ( "{ \"a\" : { \"aa\" : null, \"ab\" : 123 } }" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 0, false ) );
    REQUIRE_EQ ( string ( "{\"a\":{\"aa\":null,\"ab\":123}}" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_ArrayMember_Empty, KJsonFixture)
{
    ParseValue ( "{ \"a\" : [ ] }" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 0, false ) );
    REQUIRE_EQ ( string ( "{\"a\":[]}" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_ArrayMember, KJsonFixture)
{
    ParseValue ( "{ \"a\" : [ 1, null, false, [ {}, \"str\" ] ] }" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 0, false ) );
    REQUIRE_EQ ( string ( "{\"a\":[1,null,false,[{},\"str\"]]}" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_MultipleMembers, KJsonFixture)
{
    ParseValue ( "{ \"a\" : \"str\", \"b\" : 123, \"c\" : false, \"d\" : null, \"e\" : {}, \"f\" : [] }" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 0, false ) );
    REQUIRE_EQ ( string ( "{\"a\":\"str\",\"b\":123,\"c\":false,\"d\":null,\"e\":{},\"f\":[]}" ), string ( ( const char *) m_buf . base ) );
}
FIXTURE_TEST_CASE(KJson_ToJsonString_PrettyPrint, KJsonFixture)
{
    ParseValue ( "{ \"a\" : \"str\", \"b\" : 123, \"c\" : false, \"d\" : null, \"e\" : {\"a\":true,\"b\":false }, \"f\" : [ [ 1, 2 ], 3], \"g\":{\"a\":true}, \"h\":[1] }" );
    REQUIRE_RC ( KJsonToJsonString ( m_val, & m_buf, 0, true ) );
    REQUIRE_EQ ( string (
        "{\n"
        "\t\"a\" : \"str\",\n"
        "\t\"b\" : 123,\n"
        "\t\"c\" : false,\n"
        "\t\"d\" : null,\n"
        "\t\"e\" : {\n"
        "\t\t\"a\" : true,\n"
        "\t\t\"b\" : false\n"
        "\t},\n"
        "\t\"f\" : [\n"
        "\t\t[\n"
        "\t\t\t1,\n"
        "\t\t\t2\n"
        "\t\t],\n"
        "\t\t3\n"
        "\t],\n"
        "\t\"g\" : {\n"
        "\t\t\"a\" : true\n"
        "\t},\n"
        "\t\"h\" : [\n"
        "\t\t1\n"
        "\t]\n"
        "}" ),
        string ( ( const char *) m_buf . base ) );
}

//////////////////////////////////////////////////// Main
extern "C"
{
#ifdef WINDOWS
#define main wmain
#endif
int main ( int argc, char *argv [] )
{
    rc_t rc=KJsonTestSuite(argc, argv);
    return rc;
}

}
