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

#include <klib/text.h>
#include <klib/namelist.h>

#define YYDEBUG 1
#include "../../libs/klib/json-lex.h"
#include "../../libs/klib/json-tokens.h"
extern "C" enum yytokentype JsonScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, JsonScanBlock* sb );

#include "../../libs/klib/json-priv.h"

extern int Json_debug;

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
TEST_CASE(KJson_Scan_Unicode)
{
    string val;
    REQUIRE_EQ ( (int)jsonSTRING, Scan ( "\"какие-то странные символы\"", & val ) );
    REQUIRE_EQ ( string ( "какие-то странные символы" ), val );
}
TEST_CASE(KJson_Scan_Hex)
{
    string val;
    REQUIRE_EQ ( (int)jsonSTRING, Scan ( "\"\\u1234\"", & val ) );
    REQUIRE_EQ ( string ( "\\u1234" ), val );
}

TEST_CASE(KJson_Scan_Int)
{
    string val;
    REQUIRE_EQ ( (int)jsonNUMBER, Scan ( "1234", &val ) );
    REQUIRE_EQ ( string ( "1234" ), val );
}
TEST_CASE(KJson_Scan_Int_OneDigit)
{
    string val;
    REQUIRE_EQ ( (int)jsonNUMBER, Scan ( "0", &val ) );
    REQUIRE_EQ ( string ( "0" ), val );
}
TEST_CASE(KJson_Scan_Int_Signed_Plus)
{
    string val;
    REQUIRE_EQ ( (int)jsonNUMBER, Scan ( "+1234", &val ) );
    REQUIRE_EQ ( string ( "+1234" ), val );
}
TEST_CASE(KJson_Scan_Int_Signed_Mnius)
{
    string val;
    REQUIRE_EQ ( (int)jsonNUMBER, Scan ( "-1234", &val ) );
    REQUIRE_EQ ( string ( "-1234" ), val );
}

TEST_CASE(KJson_Scan_Int_Frac)
{
    string val;
    REQUIRE_EQ ( (int)jsonNUMBER, Scan ( "12.34", &val ) );
    REQUIRE_EQ ( string ( "12.34" ), val );
}

TEST_CASE(KJson_Scan_Int_Exp_e)
{
    string val;
    REQUIRE_EQ ( (int)jsonNUMBER, Scan ( "12e34", &val ) );
    REQUIRE_EQ ( string ( "12e34" ), val );
}
TEST_CASE(KJson_Scan_Int_Exp_E)
{
    string val;
    REQUIRE_EQ ( (int)jsonNUMBER, Scan ( "12E34", &val ) );
    REQUIRE_EQ ( string ( "12E34" ), val );
}
TEST_CASE(KJson_Scan_Int_Exp_e_Plus)
{
    string val;
    REQUIRE_EQ ( (int)jsonNUMBER, Scan ( "12e+34", &val ) );
    REQUIRE_EQ ( string ( "12e+34" ), val );
}
TEST_CASE(KJson_Scan_Int_Exp_E_Minus)
{
    string val;
    REQUIRE_EQ ( (int)jsonNUMBER, Scan ( "12E-34", &val ) );
    REQUIRE_EQ ( string ( "12E-34" ), val );
}

TEST_CASE(KJson_Scan_Int_Frac_Exp)
{
    string val;
    REQUIRE_EQ ( (int)jsonNUMBER, Scan ( "12.34e+22", &val ) );
    REQUIRE_EQ ( string ( "12.34e+22" ), val );
}

//TODO: conversion of escapes into UTF8
//TODO: conversion of numbers into int64_t and float

static
rc_t
Parse(  KJsonObject ** p_obj, const char * p_str )
{
    return KJsonMake ( p_obj, p_str, NULL, 0 );
}

static
string
ParseError(  KJsonObject ** p_obj, const char * p_str )
{
    char error[1024];
    if  ( KJsonMake ( p_obj, p_str, error, sizeof ( error ) ) == 0 )
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
        KJsonWhack( m_obj );
        KJsonValueWhack( ( KJsonValue * ) KJsonArrayToValue ( m_arr ) );
    }

    KJsonValue *    m_val;
    KJsonObject *   m_obj;
    KJsonArray *    m_arr;
};

FIXTURE_TEST_CASE(Json_MakeNull, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeNull ( & m_val ) );
    REQUIRE_NOT_NULL ( m_val );
}

FIXTURE_TEST_CASE(Json_MakeString, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeString ( & m_val, "qq", 2 ) );
    REQUIRE_NOT_NULL ( m_val );
}
FIXTURE_TEST_CASE(Json_MakeString_Empty, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeString ( & m_val, "", 0 ) );
    REQUIRE_NOT_NULL ( m_val );
}

FIXTURE_TEST_CASE(Json_ObjectToValue_NullSelf, JsonMake_Fixture)
{
    REQUIRE_NULL ( KJsonObjectToValue ( NULL ) );
}
FIXTURE_TEST_CASE(Json_ObjectToValue, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeObject ( & m_obj ) );
    REQUIRE_NOT_NULL ( KJsonObjectToValue ( m_obj ) );
}

FIXTURE_TEST_CASE(Json_ValueToObject_NullSelf, JsonMake_Fixture)
{
    REQUIRE_NULL ( KJsonValueToObject ( NULL ) );
}
FIXTURE_TEST_CASE(Json_ValueToObject_NotAnObject, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeNull ( & m_val ) );
    REQUIRE_NULL ( KJsonValueToObject ( m_val ) );
}
FIXTURE_TEST_CASE(Json_ValueToObject, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeObject ( & m_obj ) );
    const KJsonValue * v = KJsonObjectToValue ( m_obj );
    REQUIRE_NOT_NULL ( KJsonValueToObject ( v ) );
}

FIXTURE_TEST_CASE(Json_MakeObject, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeObject ( & m_obj ) );
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
}
FIXTURE_TEST_CASE(Json_ArrayToValue_NullSelf, JsonMake_Fixture)
{
    REQUIRE_NULL ( KJsonArrayToValue ( NULL ) );
}
FIXTURE_TEST_CASE(Json_ArrayToValue, JsonMake_Fixture)
{
    REQUIRE_RC ( KJsonMakeArray ( & m_arr ) );
    REQUIRE_NOT_NULL ( KJsonArrayToValue ( m_arr ) );
}
FIXTURE_TEST_CASE(Json_ValueToArray_NullSelf, JsonMake_Fixture)
{
    REQUIRE_NULL ( KJsonValueToArray ( NULL ) );
}


// KJson public API

class KJsonFixture
{
public:
    KJsonFixture()
    :   m_root ( 0 ),
        m_input ( "" )
    {
    }

    ~KJsonFixture()
    {
        KJsonWhack ( m_root );
    }

    KJsonObject *   m_root;
    const char *    m_input;
};

// KJsonMake

FIXTURE_TEST_CASE(Json_Make_NullSelf, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonMake ( NULL, m_input, NULL, 0 ) );
}
FIXTURE_TEST_CASE(Json_Make_NullParam, KJsonFixture)
{
    REQUIRE_RC_FAIL ( KJsonMake ( & m_root, NULL, NULL, 0 ) );
}

FIXTURE_TEST_CASE(Json_MakeWhack, KJsonFixture)
{
    KJsonObject * root;  /* the root is always an object */
    KJsonMake ( & root, "{}", NULL, 0 );
    REQUIRE_NOT_NULL ( root );
    KJsonWhack ( root );
}

FIXTURE_TEST_CASE(KJson_Parse_NullSelf, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( NULL, "{}" ) );
}
FIXTURE_TEST_CASE(KJson_Parse_EmptyInput, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_root, "" ) );
    REQUIRE_NULL ( m_root );
}
FIXTURE_TEST_CASE(KJson_Parse_ExpectRightBrace, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_root, "{" ) );
}
FIXTURE_TEST_CASE(KJson_Parse_ExpectLeftBrace, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_root, "}" ) );
}
FIXTURE_TEST_CASE(KJson_Parse_ExpectColon, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_root, "{\"name\"\"value\"" ) );
}
FIXTURE_TEST_CASE(KJson_Parse_ExpectRightBrace2, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_root, "{\"name\":\"value\"" ) );
    REQUIRE_NULL ( m_root );
    /*TODO: remove leak from bison*/
}
FIXTURE_TEST_CASE(KJson_Parse_EmptyObject, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, " { } " ) );
    REQUIRE_NOT_NULL ( m_root );
}
FIXTURE_TEST_CASE(KJson_Parse_Object, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{\"name1\":\"value\", \"name2\":\":value\"}" ) );
    REQUIRE_NOT_NULL ( m_root );
}
FIXTURE_TEST_CASE(KJson_Parse_Object_DuplicateName, KJsonFixture)
{
    string expected = "line 1, col 33: RC(libs/klib/json.c:172:KJsonMake rcCont,rcTree,rcInserting,rcNode,rcExists)";
    REQUIRE_EQ ( expected, ParseError ( & m_root, "{\"name\":\"value\", \"name\":\":value\"}" ) );
    REQUIRE_NULL ( m_root );
    /*TODO: remove leak from bison*/
}

FIXTURE_TEST_CASE(KJson_ParseArray_ExpectRightBracket, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_root, "{\"\":[}" ) );
}
FIXTURE_TEST_CASE(KJson_ParseArray_ExpectRightBracket2, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_root, "{\"\":[\"name\",\"name\"}" ) );
}
FIXTURE_TEST_CASE(KJson_ParseArray_ExpectLeftBracket, KJsonFixture)
{
    REQUIRE_RC_FAIL ( Parse ( & m_root, "{\"\":\"name\",\"name\"]}" ) );
}
FIXTURE_TEST_CASE(KJson_ParseArray_Empty, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{\"\":[]}" ) );
}
FIXTURE_TEST_CASE(KJson_ParseArray_String_Elems, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{\"\":[\"name\",\"name\"]}" ) );
}

/*TODO:
rc_t CC KJsonToString ( const KJsonObject * root, char * error, size_t error_size );

bool CC KJsonIsString ( const KJsonValue * value );
bool CC KJsonIsNumber ( const KJsonValue * value );
bool CC KJsonIsObject ( const KJsonValue * value );
bool CC KJsonIsArray ( const KJsonValue * value );
bool CC KJsonIsTrue ( const KJsonValue * value );
bool CC KJsonIsFalse ( const KJsonValue * value );
bool CC KJsonIsNull ( const KJsonValue * value );

rc_t CC KJsonGetString ( const KJsonValue * node, const char ** value );
rc_t CC KJsonGetNumber ( const KJsonObject * node, int64_t * value );
rc_t CC KJsonGetDouble ( const KJsonObject * node, double * value );

*/

FIXTURE_TEST_CASE(KJsonObject_GetNames_NullSelf, KJsonFixture)
{
    VNamelist * names;
    REQUIRE_RC ( VNamelistMake ( & names, 1 ) );
    REQUIRE_RC_FAIL ( KJsonObjectGetNames ( NULL, names ) );
    REQUIRE_RC ( VNamelistRelease ( names ) );
}
FIXTURE_TEST_CASE(KJsonObject_GetNames_NullParam, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{}" ) );
    REQUIRE_RC_FAIL ( KJsonObjectGetNames ( m_root, NULL ) );
}
FIXTURE_TEST_CASE(KJsonObject_GetNames_Empty, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{}" ) );
    VNamelist * names;
    REQUIRE_RC ( VNamelistMake ( & names, 1 ) );
    REQUIRE_RC ( KJsonObjectGetNames ( m_root, names ) );
    uint32_t count;
    REQUIRE_RC ( VNameListCount ( names, & count ) );
    REQUIRE_EQUAL ( 0u, count );
    REQUIRE_RC ( VNamelistRelease ( names ) );
}
FIXTURE_TEST_CASE(KJsonObject_GetNames, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{\"c\":null, \"a\":null, \"b\":null}" ) );
    VNamelist * names;
    REQUIRE_RC ( VNamelistMake ( & names, 1 ) );
    REQUIRE_RC ( KJsonObjectGetNames ( m_root, names ) );
    uint32_t count;
    REQUIRE_RC ( VNameListCount ( names, & count ) );
    REQUIRE_EQUAL ( 3u, count );
    // names are sorted
    const char * name;
    REQUIRE_RC ( VNameListGet ( names, 0, & name ) );
    REQUIRE_EQUAL ( string ( "a" ), string ( name ) );
    REQUIRE_RC ( VNameListGet ( names, 1, & name ) );
    REQUIRE_EQUAL ( string ( "b" ), string ( name ) );
    REQUIRE_RC ( VNameListGet ( names, 2, & name ) );
    REQUIRE_EQUAL ( string ( "c" ), string ( name ) );

    REQUIRE_RC ( VNamelistRelease ( names ) );
}

FIXTURE_TEST_CASE(KJsonObject_GetValue_NullSelf, KJsonFixture)
{
    REQUIRE_NULL ( KJsonObjectGetValue ( NULL, "a") );
}
FIXTURE_TEST_CASE(KJsonObject_GetValue_NullParam, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{}" ) );
    REQUIRE_NULL ( KJsonObjectGetValue ( m_root, NULL ) );
}
FIXTURE_TEST_CASE(KJsonObject_GetValue_EmptyObject, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{}" ) );
    REQUIRE_NULL ( KJsonObjectGetValue ( m_root, "a" ) );
}
FIXTURE_TEST_CASE(KJsonObject_GetValue_NotFound, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{\"c\":null, \"a\":null, \"b\":null}" ) );
    REQUIRE_NULL ( KJsonObjectGetValue ( m_root, "aa" ) );
}

FIXTURE_TEST_CASE(KJsonObject_GetValue_Found, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{\"c\":null, \"a\":null, \"b\":{}}" ) );
    const KJsonValue * v = KJsonObjectGetValue ( m_root, "b" );
    REQUIRE_NOT_NULL ( v );
    REQUIRE ( KJsonIsObject ( v ) );
}

/*TODO:
uint32_t CC KJsonArrayLength ( const KJsonArray * node );
*/

FIXTURE_TEST_CASE(KJsonArray_GetElement, KJsonFixture)
{
    REQUIRE_RC ( Parse ( & m_root, "{\"a\":[]}" ) );
    const KJsonValue * v = KJsonObjectGetValue ( m_root, "a" );
    REQUIRE_NOT_NULL ( v );
}


//TODO:error reporting (message, location)

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
