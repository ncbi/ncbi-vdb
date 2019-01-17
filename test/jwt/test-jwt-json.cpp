//
//  ncbi-json-ut.cpp
//  ncbi-oauth-test
//
//  Created by User on 7/5/18.
//  Copyright © 2018 NCBI. All rights reserved.
//

#include <ktst/unit_test.hpp>

#include <jwt/json.hpp>
#include <../libs/jwt/jwt-vector-impl.hpp>

#include <iostream>

TEST_SUITE(JwtJsonTestSuite);

using namespace std;

// googletest to ktst conversion macros
#define EXPECT_ANY_THROW    REQUIRE_THROW
#define EXPECT_STREQ(s1,s2) REQUIRE_EQ ( string ( s1 ), string ( s2 ) )
#define ASSERT_EQ           REQUIRE_EQ
#define ASSERT_TRUE         REQUIRE
#define ASSERT_FALSE(b)     REQUIRE ( ! ( b ) )
#define THROW_ON_TRUE(b)    THROW_ON_FALSE ( ! ( b ) )
#define EXPECT_NO_THROW(e)  (e)

using namespace ncbi;

class TestJSONValue : public JSONValue
{
public:
    static JSONValue * test_parse ( const std :: string & json, bool consume_all = true )
    {
        size_t pos = 0;
        if ( json . empty () )
            throw JSONException ( __func__, __LINE__, "Empty JSON source" );

        if ( json . size () > JSONValue :: default_limits . json_string_size )
            throw JSONException ( __func__, __LINE__, "JSON source exceeds allowed size limit" );

        JSONValue *val = parse ( default_limits, JwtString(json.data()), pos, 0 );

        if ( consume_all && pos < json . size () )
        {
            delete val;
            throw JSONException ( __func__, __LINE__, "Trailing byes in JSON text" ); // test hit
        }

        return val;
    }
};

/* JSON Construction - Whitebox
    *
    **********************************************************************************/
class JSONFixture_WhiteBox
{
public:
    enum JSONType { Object, Array, Value };

    JSONFixture_WhiteBox ()
    : jObj ( nullptr )
    {
    }

    ~JSONFixture_WhiteBox ()
    {
        delete jObj;
    }

    void make_throw ( JSONType type, const std :: string &json )
    {
        switch ( type )
        {
            case Object:
            case Array:
                EXPECT_ANY_THROW ( JSON :: parse ( JwtString ( json . data () ) ) );
                break;
            case Value:
                EXPECT_ANY_THROW ( TestJSONValue :: test_parse ( json ) );
                break;
        }
    }

    void make ( JSONType type, const std :: string &json, bool consume_all = true )
    {
        switch ( type )
        {
            case Object:
            case Array:
            {
                jObj = JSON :: parse ( JwtString ( json . data () ) );
                break;
            }
            case Value:
            {
                JSONValue *val = TestJSONValue :: test_parse ( json, consume_all );
                THROW_ON_FALSE ( val != nullptr );

                jObj = val;
                break;
            }
        }
    }

    void make_and_verify_eq ( JSONType type, const std :: string &json, const std :: string &expected, bool consume_all = true )
    {
        make ( type, json, consume_all );
        THROW_ON_FALSE ( string ( jObj -> toJSON() . data () ) == expected );
    }

protected:
    JSONValue *jObj;
};

/* Object
    * {}
    * { members }
    */
FIXTURE_TEST_CASE ( JSONObject_Empty, JSONFixture_WhiteBox )
{
    make_and_verify_eq ( Object , "{}", "{}" );
}
FIXTURE_TEST_CASE ( JSONObject_EmptyArray_wb, JSONFixture_WhiteBox )
{
    make_and_verify_eq ( Object , "{\"\":[]}", "{\"\":[]}" );
}
FIXTURE_TEST_CASE ( JSONObject_String_Member_wb, JSONFixture_WhiteBox )
{
    make_and_verify_eq ( Object , "{\"name\":\"value\"}", "{\"name\":\"value\"}" );
}

// JSONObject Exceptions
FIXTURE_TEST_CASE ( JSONObject_Throw_Empty, JSONFixture_WhiteBox )
{
    make_throw ( Object, "" );  // Empty JSON object
}
FIXTURE_TEST_CASE ( JSONObject_Throw_ExpecttRightBrace, JSONFixture_WhiteBox )
{
    make_throw ( Object, "{" ); // Expected '}'
}
FIXTURE_TEST_CASE ( JSONObject_Throw_ExpectLeftBrace, JSONFixture_WhiteBox )
{
    make_throw ( Object, "}" ); // Expected '{'
}
FIXTURE_TEST_CASE ( JSONObject_Throw_ExpectColon, JSONFixture_WhiteBox )
{
    make_throw ( Object, "{\"name\"\"value\"" ); // Expected ':'
}
FIXTURE_TEST_CASE ( JSONObject_Throw_ExpectRightBrace2, JSONFixture_WhiteBox )
{
    make_throw ( Object, "{\"name\":\"value\"" ); // Expected '}'
}
FIXTURE_TEST_CASE ( JSONObject_Throw_TrailingBytes, JSONFixture_WhiteBox )
{
    make_throw ( Object, "{\"name\":\"value\"}trailing" ); // Expected '}'
}

/* Array
    * []
    * [ elements ]
    */
FIXTURE_TEST_CASE ( JSONArray_Empty, JSONFixture_WhiteBox )
{
    make_and_verify_eq ( Array , "[]", "[]" );
}
FIXTURE_TEST_CASE ( JSONArray_String_Elems, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Array , "[\"name\",\"value\"]", "[\"name\",\"value\"]" );
}

// JSONArray Exceptions
FIXTURE_TEST_CASE ( JSONArray_Throw_Empty, JSONFixture_WhiteBox )
{
    make_throw ( Array, "" );  // Empty JSON array
}
FIXTURE_TEST_CASE ( JSONArray_Throw_ExpectLeftBracket, JSONFixture_WhiteBox )
{
    make_throw ( Array, "]" ); // Expected '['
}
FIXTURE_TEST_CASE ( JSONArray_Throw_ExpectRightBracket, JSONFixture_WhiteBox )
{
    make_throw ( Array, "[" ); // Expected ']'
}
FIXTURE_TEST_CASE ( JSONArray_Throw_ExpectRightBracket2, JSONFixture_WhiteBox )
{
    make_throw ( Array, "[\"name\",\"name\"" ); // Expected ']'
}
FIXTURE_TEST_CASE ( JSONArray_Throw_TrailingBytes, JSONFixture_WhiteBox )
{
    make_throw ( Array, "[\"name\",\"name\"]trailing" ); // Expected ']'
}

/* Bool
    * true
    * false
    */
FIXTURE_TEST_CASE ( Bool_True, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "true", "true" );
}
FIXTURE_TEST_CASE ( Bool_False, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "false", "false" );
}

/* Integer
    * digit
    * digit1-9 digits
    * -digit
    * -digit1-9 digits
    */
FIXTURE_TEST_CASE ( Integer_Single, JSONFixture_WhiteBox )
{
    make_and_verify_eq ( Value , "0", "0" );
}
FIXTURE_TEST_CASE ( Integer_Multiple, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "12345", "12345" );
}
FIXTURE_TEST_CASE ( Integer_Single_Negative_zero, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "-0", "0" );
}
FIXTURE_TEST_CASE ( Integer_Single_Negative_one, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "-1", "-1" );
}
FIXTURE_TEST_CASE ( Integer_Multiple_Negative, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "-12345", "-12345" );
}

/* Floating point
    * int frac
    * int exp
    * int frac exp
    */
FIXTURE_TEST_CASE ( Float_Frac_zero, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "0.0", "0.0" );
}
    FIXTURE_TEST_CASE ( Float_Frac_onetwo, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "1.2", "1.2" );
}
FIXTURE_TEST_CASE ( Float_Frac_Precision, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "1234.56789", "1234.56789" );
}

FIXTURE_TEST_CASE ( Float_eE_nodigit, JSONFixture_WhiteBox )
{
    // invalid exp format, but construction should not fail
    // as it is the nature of parsers to consume tokens, not
    // entire strings - should return "0" and consumed
    // only one digit
    make_and_verify_eq ( Value , "0E", "0", false );
}

FIXTURE_TEST_CASE ( Float_eE_digit_lower, JSONFixture_WhiteBox )
{
    make ( Value , "0e0" );
}
FIXTURE_TEST_CASE ( Float_eE_digit_upper, JSONFixture_WhiteBox )
{
    make ( Value , "0E0" );
}
FIXTURE_TEST_CASE ( Float_eE_plus_digits_lower, JSONFixture_WhiteBox )
{
    make ( Value , "0e+0" );
}
FIXTURE_TEST_CASE ( Float_eE_plus_digits_upper, JSONFixture_WhiteBox )
{
    make ( Value , "0E+0" );
}
FIXTURE_TEST_CASE ( Float_eE_minus_digits_lower, JSONFixture_WhiteBox )
{
    make ( Value , "0e-0" );
}
FIXTURE_TEST_CASE ( Float_eE_minus_digits_upper, JSONFixture_WhiteBox )
{
    make ( Value , "0E-0" );
}

FIXTURE_TEST_CASE ( Float_Frac_Exp, JSONFixture_WhiteBox )
{
    make ( Value, "0.0e0" );
}

/* String
    * ""
    * " chars "
    */
FIXTURE_TEST_CASE ( String_Empty, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "\"\"", "\"\"" );
}
FIXTURE_TEST_CASE ( String_Char, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "\"a\"", "\"a\"" );
}
FIXTURE_TEST_CASE ( String_Chars, JSONFixture_WhiteBox )
{
    make_and_verify_eq( Value , "\"abc\"", "\"abc\"" );
}

// JSONValue Exceptions
FIXTURE_TEST_CASE ( JSONValue_Null_Throw_InvJSONFmt, JSONFixture_WhiteBox )
{
    make_throw ( Value, "a" );
}
FIXTURE_TEST_CASE ( JSONValue_Null_Throw_InvNullFmt_Missing, JSONFixture_WhiteBox )
{
    make_throw ( Value, "n" );
}
FIXTURE_TEST_CASE ( JSONValue_Null_Throw_InvNullFmt_Bad, JSONFixture_WhiteBox )
{
    make_throw ( Value, "nulll" );
}
FIXTURE_TEST_CASE ( JSONValue_String_Throw_InvBeginFormat, JSONFixture_WhiteBox )
{
    make_throw ( Value, "\"" ); // Invalid begin of string format
}
FIXTURE_TEST_CASE ( JSONValue_String_Throw_InvEscChar_Missing, JSONFixture_WhiteBox )
{
    make_throw ( Value, "\"\\" ); // Invalid escape character
}
FIXTURE_TEST_CASE ( JSONValue_String_Throw_InvEscChar_Bad, JSONFixture_WhiteBox )
{
    make_throw ( Value, "\"\\y" ); // Invalid escape character
}
FIXTURE_TEST_CASE ( JSONValue_String_Throw_InvUEscSeq_Missing, JSONFixture_WhiteBox )
{
    make_throw ( Value, "\"\\u" ); // Invalid \u escape sequence
}
FIXTURE_TEST_CASE ( JSONValue_String_Throw_InvUEscSeq_Short, JSONFixture_WhiteBox )
{
    make_throw ( Value, "\"\\uabc" ); // Invalid \u escape sequence
}
FIXTURE_TEST_CASE ( JSONValue_String_Throw_InvUEscSeq_Bad, JSONFixture_WhiteBox )
{
    make_throw ( Value, "\"\\uabcz" ); // Invalid \u escape sequence
}
FIXTURE_TEST_CASE ( JSONValue_String_Throw_InvEndFormat, JSONFixture_WhiteBox )
{
    make_throw ( Value, "\"\\u0061" ); // Invalid end of string format
}
FIXTURE_TEST_CASE ( JSONValue_String_Throw_Trailing, JSONFixture_WhiteBox )
{
    make_throw ( Value, "\"validtext\"trailing" );
}
FIXTURE_TEST_CASE ( JSONValue_Bool_Throw_True_Missing, JSONFixture_WhiteBox )
{
    make_throw ( Value, "t" );
}
FIXTURE_TEST_CASE ( JSONValue_Bool_Throw_True_Bad, JSONFixture_WhiteBox )
{
    make_throw ( Value, "truee" );
}
FIXTURE_TEST_CASE ( JSONValue_Bool_Throw_False_Missing, JSONFixture_WhiteBox )
{
    make_throw ( Value, "f" );
}
FIXTURE_TEST_CASE ( JSONValue_Bool_Throw_False_Bad, JSONFixture_WhiteBox )
{
    make_throw ( Value, "falsee" );
}
FIXTURE_TEST_CASE ( JSONValue_Integer_Throw_Negative_Missing, JSONFixture_WhiteBox )
{
    make_throw ( Value, "-" ); // Expected digit
}
FIXTURE_TEST_CASE ( JSONValue_Integer_Throw_Negative_Bad, JSONFixture_WhiteBox )
{
    make_throw ( Value, "-a" ); // Expected digit
}

/* JSON Construction - Blackbox
    *
    **********************************************************************************/
class JSONFixture_BlackBox
{
public:
    enum JSONType { Object, Array, Value };

    JSONFixture_BlackBox ()
    {
    }

    ~JSONFixture_BlackBox ()
    {
    }

    void make_and_verify_eq ( const std :: string &json, const std :: string &expected )
    {
        JSONValue * jVal = JSON :: parse ( JwtString ( json . data () ) );
        THROW_ON_FALSE ( std :: string ( jVal -> toJSON() . data () ) == expected );
        delete jVal;
    }

    void make_throw ( JSONType type, const std :: string &json, bool consume_all = true )
    {
        switch ( type )
        {
            case Object:
            case Array:
                EXPECT_ANY_THROW ( JSON :: parse ( JwtString ( json . data () ) ) );
                break;
            case Value:
                FAIL("make_throw: bad object type");
                break;
        }
    }
};


// Construction via JSON source
FIXTURE_TEST_CASE ( JSONObject_Emptym, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{}", "{}" );
}
FIXTURE_TEST_CASE ( JSONObject_Nested_Obj1, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"nested-obj\":{}}", "{\"nested-obj\":{}}" );
}
FIXTURE_TEST_CASE ( JSONObject_Nested_Obj2, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"nested-obj\":{\"nested-array\":[]}}",
                            "{\"nested-obj\":{\"nested-array\":[]}}" );
}
FIXTURE_TEST_CASE ( JSONObject_Nested_Obj3, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"nested-obj\":{\"nested-obj\":{}}}",
                            "{\"nested-obj\":{\"nested-obj\":{}}}" );
}
FIXTURE_TEST_CASE ( JSONObject_EmptyArray_bb, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"array\":[]}", "{\"array\":[]}" );
}
FIXTURE_TEST_CASE ( JSONObject_Bool_Member1, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"name\":true}", "{\"name\":true}" );
}
FIXTURE_TEST_CASE ( JSONObject_Bool_Member2, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"name\":false}", "{\"name\":false}" );
}
FIXTURE_TEST_CASE ( JSONObject_Number_Member1, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"name\":0}", "{\"name\":0}" );
}
FIXTURE_TEST_CASE ( JSONObject_Number_Member2, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"name\":1234567890}", "{\"name\":1234567890}" );
}
FIXTURE_TEST_CASE ( JSONObject_NegNumber_Member1, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"name\":-1}", "{\"name\":-1}" );
}
FIXTURE_TEST_CASE ( JSONObject_NegNumber_Member2, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"name\":-1234567890}", "{\"name\":-1234567890}" );
}
FIXTURE_TEST_CASE ( JSONObject_Float_Frac1, JSONFixture_BlackBox )
{
    make_and_verify_eq( "{\"name\":0.0}", "{\"name\":0.0}" );
}
FIXTURE_TEST_CASE ( JSONObject_Float_Frac2, JSONFixture_BlackBox )
{
    make_and_verify_eq( "{\"name\":123.456789}", "{\"name\":123.456789}" );
}
FIXTURE_TEST_CASE ( JSONObject_Float_eE_digit1, JSONFixture_BlackBox )
{
    make_and_verify_eq( "{\"name\":0e0}", "{\"name\":0e0}" );
}
FIXTURE_TEST_CASE ( JSONObject_Float_eE_digit2, JSONFixture_BlackBox )
{
    make_and_verify_eq( "{\"name\":0E0}", "{\"name\":0E0}" );
}
FIXTURE_TEST_CASE ( JSONObject_Float_eE_plus_digit, JSONFixture_BlackBox )
{
    make_and_verify_eq( "{\"name\":0e+0}", "{\"name\":0e+0}" );
}
FIXTURE_TEST_CASE ( JSONObject_Float_eE_minus_digit, JSONFixture_BlackBox )
{
    make_and_verify_eq( "{\"name\":0E-0}", "{\"name\":0E-0}" );
}
FIXTURE_TEST_CASE ( JSONObject_Float_Frac_Exp, JSONFixture_BlackBox )
{
    make_and_verify_eq( "{\"name\":0.0E0}", "{\"name\":0.0E0}" );
}
FIXTURE_TEST_CASE ( JSONObject_String_Member_bb, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"name\":\"value\"}", "{\"name\":\"value\"}" );
}
FIXTURE_TEST_CASE ( JSONObject_Member_Array, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"\":[true,false]}", "{\"\":[true,false]}" );
}

// Addon Tests
FIXTURE_TEST_CASE ( JSONObject_Addon1, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"modDateA\": \"\",\"modDate\": 0}" ,
                            "{\"modDate\":0,\"modDateA\":\"\"}" );
}
FIXTURE_TEST_CASE ( JSONObject_Addon2, JSONFixture_BlackBox )
{
    make_and_verify_eq ( "{\"modDate\": \"\",\"modDateA\": 0}" ,
                        "{\"modDate\":\"\",\"modDateA\":0}" );
}
FIXTURE_TEST_CASE ( JSONObject_Addon3, JSONFixture_BlackBox )
{
    make_throw ( Object, "{\"modDate\": \"\",\"modDate\": \"\"}" );
}

/* JSONValue Construction - Method
    *
    **********************************************************************************/
class JSONFixture_JSONValue_Interface
{
public:
    enum Type { jvt_null, jvt_bool, jvt_int, jvt_double,
        jvt_num, jvt_string, jvt_array, jvt_obj };

    JSONFixture_JSONValue_Interface ()
    : jObj ( nullptr )
    {
    }

    ~JSONFixture_JSONValue_Interface ()
    {
        delete jObj;
    }

    void assert_is_of_type ( const Type type )
    {
        switch ( type )
        {
        case jvt_null:
            THROW_ON_FALSE ( jObj -> isNull () );
            THROW_ON_TRUE ( jObj -> isBool () );
            THROW_ON_TRUE( jObj -> isInteger () );
            THROW_ON_TRUE( jObj -> isNumber () );
            THROW_ON_TRUE( jObj -> isString () );
            THROW_ON_TRUE( jObj -> isArray () );
            THROW_ON_TRUE( jObj -> isObject () );
            break;
        case jvt_bool:
            THROW_ON_FALSE ( jObj -> isBool () );
            THROW_ON_TRUE( jObj -> isNull () );
            THROW_ON_TRUE( jObj -> isInteger () );
            THROW_ON_TRUE( jObj -> isNumber () );
            THROW_ON_TRUE( jObj -> isString () );
            THROW_ON_TRUE( jObj -> isArray () );
            THROW_ON_TRUE( jObj -> isObject () );
            break;
        case jvt_int:
            THROW_ON_FALSE ( jObj -> isInteger () );
            THROW_ON_TRUE( jObj -> isBool () );
            THROW_ON_TRUE( jObj -> isNull () );
            THROW_ON_TRUE( jObj -> isNumber () );
            THROW_ON_TRUE( jObj -> isString () );
            THROW_ON_TRUE( jObj -> isArray () );
            THROW_ON_TRUE( jObj -> isObject () );
            break;
        case jvt_double:
            THROW_ON_FALSE ( jObj -> isNumber () );
            THROW_ON_TRUE( jObj -> isBool () );
            THROW_ON_TRUE( jObj -> isInteger () );
            THROW_ON_TRUE( jObj -> isNull () );
            THROW_ON_TRUE( jObj -> isString () );
            THROW_ON_TRUE( jObj -> isArray () );
            THROW_ON_TRUE( jObj -> isObject () );
            break;
        case jvt_num:
            THROW_ON_FALSE ( jObj -> isNumber () );
            THROW_ON_TRUE( jObj -> isBool () );
            THROW_ON_TRUE( jObj -> isInteger () );
            THROW_ON_TRUE( jObj -> isNull () );
            THROW_ON_TRUE( jObj -> isString () );
            THROW_ON_TRUE( jObj -> isArray () );
            THROW_ON_TRUE( jObj -> isObject () );
            break;
        case jvt_string:
            THROW_ON_FALSE ( jObj -> isString () );
            THROW_ON_TRUE( jObj -> isBool () );
            THROW_ON_TRUE( jObj -> isInteger () );
            THROW_ON_TRUE( jObj -> isNumber () );
            THROW_ON_TRUE( jObj -> isNull () );
            THROW_ON_TRUE( jObj -> isArray () );
            THROW_ON_TRUE( jObj -> isObject () );
            break;
        case jvt_array:
            THROW_ON_FALSE ( jObj -> isArray () );
            THROW_ON_TRUE( jObj -> isBool () );
            THROW_ON_TRUE( jObj -> isInteger () );
            THROW_ON_TRUE( jObj -> isNumber () );
            THROW_ON_TRUE( jObj -> isString () );
            THROW_ON_TRUE( jObj -> isNull () );
            THROW_ON_TRUE( jObj -> isObject () );
            break;
    case jvt_obj:
            THROW_ON_FALSE ( jObj -> isObject () );
            THROW_ON_TRUE( jObj -> isBool () );
            THROW_ON_TRUE( jObj -> isInteger () );
            THROW_ON_TRUE( jObj -> isNumber () );
            THROW_ON_TRUE( jObj -> isString () );
            THROW_ON_TRUE( jObj -> isArray () );
            THROW_ON_TRUE( jObj -> isNull () );
            break;
        }
    }

    void set_types ( const Type type )
    {
        switch ( type )
        {
            case jvt_null:
                THROW_ON_FALSE ( ( jObj -> setBool ( true ) ) . isBool () );
                THROW_ON_FALSE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                THROW_ON_FALSE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                THROW_ON_FALSE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                THROW_ON_FALSE ( ( jObj -> setString ( "string" ) ) . isString () );
                THROW_ON_FALSE ( ( jObj -> setNull () ) . isNull () );
                break;
            case jvt_bool:
                THROW_ON_FALSE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                THROW_ON_FALSE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                THROW_ON_FALSE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                THROW_ON_FALSE ( ( jObj -> setString ( "string" ) ) . isString () );
                THROW_ON_FALSE ( ( jObj -> setNull () ) . isNull () );
                THROW_ON_FALSE ( ( jObj -> setBool ( true ) ) . isBool () );
                break;
            case jvt_int:
                THROW_ON_FALSE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                THROW_ON_FALSE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                THROW_ON_FALSE ( ( jObj -> setString ( "string" ) ) . isString () );
                THROW_ON_FALSE ( ( jObj -> setNull () ) . isNull () );
                THROW_ON_FALSE ( ( jObj -> setBool ( true ) ) . isBool () );
                THROW_ON_FALSE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                break;
            case jvt_double:
                THROW_ON_FALSE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                THROW_ON_FALSE ( ( jObj -> setString ( "string" ) ) . isString () );
                THROW_ON_FALSE ( ( jObj -> setNull () ) . isNull () );
                THROW_ON_FALSE ( ( jObj -> setBool ( true ) ) . isBool () );
                THROW_ON_FALSE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                THROW_ON_FALSE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                break;
            case jvt_num:
                THROW_ON_FALSE ( ( jObj -> setString ( "string" ) ) . isString () );
                THROW_ON_FALSE ( ( jObj -> setNull () ) . isNull () );
                THROW_ON_FALSE ( ( jObj -> setBool ( true ) ) . isBool () );
                THROW_ON_FALSE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                THROW_ON_FALSE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                THROW_ON_FALSE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                break;
            case jvt_string:
                THROW_ON_FALSE ( ( jObj -> setNull () ) . isNull () );
                THROW_ON_FALSE ( ( jObj -> setBool ( true ) ) . isBool () );
                THROW_ON_FALSE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                THROW_ON_FALSE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                THROW_ON_FALSE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                THROW_ON_FALSE ( ( jObj -> setString ( "string" ) ) . isString () );
                break;
            case jvt_array:
                EXPECT_ANY_THROW ( jObj -> setNull () );
                EXPECT_ANY_THROW ( jObj -> setBool ( true ) );
                EXPECT_ANY_THROW ( jObj -> setInteger ( 123 ) );
                EXPECT_ANY_THROW ( jObj -> setDouble ( 123.456789, 10 ) );
                EXPECT_ANY_THROW ( jObj -> setNumber ( "123.456789" ) );
                EXPECT_ANY_THROW ( jObj -> setString ( "string" ) );
                break;
            case jvt_obj:
                EXPECT_ANY_THROW ( jObj -> setNull () );
                EXPECT_ANY_THROW ( jObj -> setBool ( true ) );
                EXPECT_ANY_THROW ( jObj -> setInteger ( 123 ) );
                EXPECT_ANY_THROW ( jObj -> setDouble ( 123.456789, 10 ) );
                EXPECT_ANY_THROW ( jObj -> setNumber ( "123.456789" ) );
                EXPECT_ANY_THROW ( jObj -> setString ( "string" ) );
                break;
        }
    }

    void to_types ( const Type type, const void *cmp = nullptr )
    {
        switch ( type )
        {
            case jvt_null:
                EXPECT_ANY_THROW  ( jObj -> toBool () );
                EXPECT_ANY_THROW  ( jObj -> toInteger () );
                EXPECT_ANY_THROW  ( jObj -> toNumber() );
                THROW_ON_FALSE  ( string ( jObj -> toString () . data () ) == "null" );
                EXPECT_ANY_THROW  ( jObj -> toArray () );
                EXPECT_ANY_THROW  ( jObj -> toObject () );
                break;
            case jvt_bool:
                THROW_ON_FALSE  ( jObj -> toBool () );
                EXPECT_ANY_THROW  ( jObj -> toInteger () );
                EXPECT_ANY_THROW  ( jObj -> toNumber() );
                THROW_ON_FALSE  ( string ( jObj -> toString () . data () ) == ( const char * ) cmp );
                EXPECT_ANY_THROW  ( jObj -> toArray () );
                EXPECT_ANY_THROW  ( jObj -> toObject () );
                break;
            case jvt_int:
                EXPECT_ANY_THROW  ( jObj -> toBool () );
                THROW_ON_FALSE  ( jObj -> toInteger () == * ( long long int * ) cmp  );
                THROW_ON_FALSE  ( string ( jObj -> toNumber () . data () ) ==
                                std :: to_string ( * ( long long int * ) cmp ) );
                THROW_ON_FALSE  ( string ( jObj -> toString () . data () ) ==
                                std :: to_string ( * ( long long int * ) cmp ) );
                EXPECT_ANY_THROW  ( jObj -> toArray () );
                EXPECT_ANY_THROW  ( jObj -> toObject () );
                break;
            case jvt_double:
                EXPECT_ANY_THROW  ( jObj -> toBool () );
                EXPECT_ANY_THROW  ( jObj -> toInteger () );
                THROW_ON_FALSE  ( string ( jObj -> toNumber () . data () ) ==
                                std :: to_string ( * ( long double * ) cmp ) );
                THROW_ON_FALSE  ( string ( jObj -> toString () . data () ) ==
                                std :: to_string ( * ( long double * ) cmp ) );
                EXPECT_ANY_THROW  ( jObj -> toArray () );
                EXPECT_ANY_THROW  ( jObj -> toObject () );
                break;
            case jvt_num:
                EXPECT_ANY_THROW  ( jObj -> toBool () );
                EXPECT_ANY_THROW  ( jObj -> toInteger () );
                THROW_ON_FALSE  ( string ( jObj -> toNumber () . data () ) == ( const char * ) cmp );
                THROW_ON_FALSE  ( string ( jObj -> toString () . data () ) == ( const char * ) cmp );
                EXPECT_ANY_THROW  ( jObj -> toArray () );
                EXPECT_ANY_THROW  ( jObj -> toObject () );
                break;
            case jvt_string:
                EXPECT_ANY_THROW  ( jObj -> toBool () );
                EXPECT_ANY_THROW  ( jObj -> toInteger () );
                EXPECT_ANY_THROW  ( jObj -> toNumber () );
                THROW_ON_FALSE  ( string ( jObj -> toString () . data () ) == ( const char * ) cmp );
                EXPECT_ANY_THROW  ( jObj -> toArray () );
                EXPECT_ANY_THROW  ( jObj -> toObject () );
                break;
            case jvt_array:
                EXPECT_ANY_THROW  ( jObj -> toBool () );
                EXPECT_ANY_THROW  ( jObj -> toInteger () );
                EXPECT_ANY_THROW  ( jObj -> toNumber() );
                EXPECT_ANY_THROW  ( jObj -> toString () );
                EXPECT_NO_THROW  ( jObj -> toArray () );
                EXPECT_ANY_THROW  ( jObj -> toObject () );
                break;
            case jvt_obj:
                EXPECT_ANY_THROW  ( jObj -> toBool () );
                EXPECT_ANY_THROW  ( jObj -> toInteger () );
                EXPECT_ANY_THROW  ( jObj -> toNumber() );
                EXPECT_ANY_THROW  ( jObj -> toString () );
                EXPECT_ANY_THROW  ( jObj -> toArray () );
                EXPECT_NO_THROW  ( jObj -> toObject () );
                break;
        }
    }

protected:
    size_t pos;
    JSONValue *jObj;
};

// JSONObject
FIXTURE_TEST_CASE ( t_null, JSONFixture_JSONValue_Interface )
{
    Type type = jvt_null;
    jObj = JSONValue :: makeNull ();
    ASSERT_TRUE ( jObj != nullptr );
    assert_is_of_type ( type );
    set_types ( type );
    to_types ( type );
    JSONValue *clone = jObj -> clone ();
    ASSERT_TRUE( clone -> isNull () );
    delete clone;
}
FIXTURE_TEST_CASE ( t_bool, JSONFixture_JSONValue_Interface )
{
    Type type = jvt_bool;
    const char *val = "true";
    jObj = JSONValue :: makeBool ( true );
    ASSERT_TRUE ( jObj != nullptr );
    assert_is_of_type ( type );
    set_types ( type );
    to_types ( type, val );
    JSONValue *clone = jObj -> clone ();
    ASSERT_TRUE( clone -> isBool () );
    delete clone;
}
FIXTURE_TEST_CASE ( t_integer, JSONFixture_JSONValue_Interface )
{
    Type type = jvt_int;
    long long int val = 123;
    jObj = JSONValue :: makeInteger ( val );
    ASSERT_TRUE ( jObj != nullptr );
    assert_is_of_type ( type );
    set_types ( type );
    to_types ( type, &val );
    JSONValue *clone = jObj -> clone ();
    ASSERT_TRUE( clone -> isInteger () );
    delete clone;
}
FIXTURE_TEST_CASE ( t_double, JSONFixture_JSONValue_Interface )
{
    Type type = jvt_double;
    long double val = 123.456789;
    jObj = JSONValue :: makeDouble ( val, 10 );
    ASSERT_TRUE ( jObj != nullptr );
    assert_is_of_type ( type );
    set_types ( type );
    to_types ( type, &val );
    JSONValue *clone = jObj -> clone ();
    ASSERT_TRUE( clone -> isNumber () );
    delete clone;
}
FIXTURE_TEST_CASE ( t_number, JSONFixture_JSONValue_Interface )
{
    Type type = jvt_num;
    const char * val = "123.456789";
    jObj = JSONValue :: makeNumber ( val );
    ASSERT_TRUE ( jObj != nullptr );
    assert_is_of_type ( type );
    set_types ( type );
    to_types ( type, val );
    JSONValue *clone = jObj -> clone ();
    ASSERT_TRUE( clone -> isNumber () );
    delete clone;
}
FIXTURE_TEST_CASE ( t_string, JSONFixture_JSONValue_Interface )
{
    Type type = jvt_string;
    const char * val = "string";
    jObj = JSONValue :: makeString ( val );
    ASSERT_TRUE ( jObj != nullptr );
    assert_is_of_type ( type );
    set_types ( type );
    to_types ( type, val );
    JSONValue *clone = jObj -> clone ();
    ASSERT_TRUE( clone -> isString () );
    delete clone;
}
FIXTURE_TEST_CASE ( t_array, JSONFixture_JSONValue_Interface )
{
    Type type = jvt_array;
    jObj = JSONArray :: make ();
    ASSERT_TRUE ( jObj != nullptr );
    assert_is_of_type ( type );
    set_types ( type );
    to_types ( type );
}
FIXTURE_TEST_CASE ( t_obj, JSONFixture_JSONValue_Interface )
{
    Type type = jvt_obj;
    jObj = JSONObject :: make ();
    ASSERT_TRUE ( jObj != nullptr );
    assert_is_of_type ( type );
    set_types ( type );
    to_types ( type );
}
/* JSONObject Construction - Method
    *
    **********************************************************************************/
class JSONFixture_JSONObject_Interface
{
public:
    JSONFixture_JSONObject_Interface ()
    : jObj ( nullptr )
    {
    }

    ~JSONFixture_JSONObject_Interface ()
    {
        delete jObj;
    }

    void make_empty ()
    {
        jObj = JSONObject :: make ();
        THROW_ON_FALSE ( jObj != nullptr );
    }

protected:
    size_t pos;
    JSONObject *jObj;
};

// JSONObject
FIXTURE_TEST_CASE ( isObject, JSONFixture_JSONObject_Interface )
{
    make_empty();
    ASSERT_TRUE ( jObj -> isObject () );
}
FIXTURE_TEST_CASE ( clone_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    JSONValue *obj = jObj -> clone ();
    ASSERT_TRUE ( obj -> isObject () );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj -> toJSON() . c_str() );
    delete obj;
}
FIXTURE_TEST_CASE ( isEmpty_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    ASSERT_TRUE( jObj -> isEmpty() );
}
FIXTURE_TEST_CASE ( exists_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    ASSERT_FALSE( jObj -> exists ( "name" ) );
}
FIXTURE_TEST_CASE ( count_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    ASSERT_EQ ( jObj -> count (), 0ul );
}
FIXTURE_TEST_CASE ( getNames, JSONFixture_JSONObject_Interface )
{
    make_empty();
    ASSERT_TRUE ( jObj -> getNames () . empty () );
}
FIXTURE_TEST_CASE ( setValue_Null_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeNull () );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":null}" );
}
FIXTURE_TEST_CASE ( setValue_Bool_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeBool ( true ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":true}" );
}
FIXTURE_TEST_CASE ( setValue_Integer_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeInteger( 123 ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":123}" );
}
FIXTURE_TEST_CASE ( setValue_Double_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeDouble( 123.456789, 10 ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":123.456789}" );
}
FIXTURE_TEST_CASE ( setValue_Number_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeNumber( "123.456789" ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":123.456789}" );
}
FIXTURE_TEST_CASE ( setValue_String_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":\"value\"}" );
}
FIXTURE_TEST_CASE ( setValue_Object_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "obj", JSONObject :: make () );
    jObj -> getValue ( "obj" ) . toObject ()
    . setValue ( "name", JSONValue :: makeString ( "value" ) );
    jObj -> getValue ( "obj" ) . toObject ()
    . setValue ( "number", JSONValue :: makeInteger ( 2 ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"obj\":{\"name\":\"value\",\"number\":2}}" );
}
FIXTURE_TEST_CASE ( setValue_Array_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "array", JSONArray :: make () );
    jObj -> getValue ( "array" ) . toArray ()
    . appendValue ( JSONValue :: makeString ( "first" ) );
    jObj -> getValue ( "array" ) . toArray ()
    . appendValue ( JSONValue :: makeInteger ( 2 ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"array\":[\"first\",2]}" );
}
FIXTURE_TEST_CASE ( setFinalValue_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setFinalValue ( "name", JSONValue :: makeString ( "value" ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":\"value\"}" );
}
FIXTURE_TEST_CASE ( getValue_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
    JSONValue &val = jObj -> getValue ( "name" );
    EXPECT_STREQ ( val . toJSON() . c_str(), "\"value\"" );
}
FIXTURE_TEST_CASE ( getConstValue_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
    const JSONObject *cObj = jObj;
    const JSONValue &val =  cObj -> getValue ( "name" );
    EXPECT_STREQ ( val . toJSON() . c_str(), "\"value\"" );
}
FIXTURE_TEST_CASE ( removeValue_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
    jObj -> removeValue ( "name" );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{}" );
}
FIXTURE_TEST_CASE ( removeFinalValue_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setFinalValue ( "name", JSONValue :: makeString ( "value" ) );
    jObj -> removeValue ( "name" );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":\"value\"}" );
}
FIXTURE_TEST_CASE ( operator_equals_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
    JSONObject * obj = JSONObject :: make ();
    * obj = * jObj;
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj -> toJSON() . c_str () );
    delete obj;
}

FIXTURE_TEST_CASE ( copy_constructor_obj, JSONFixture_JSONObject_Interface )
{
    make_empty();
    jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
    JSONObject obj ( *jObj );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj . toJSON() . c_str () );
}

/* JSONArray Construction - Method
    *
    **********************************************************************************/
class JSONFixture_JSONArray_Interface
{
public:
    JSONFixture_JSONArray_Interface ()
    : jObj ( nullptr )
    {
    }

    ~JSONFixture_JSONArray_Interface ()
    {
        delete jObj;
    }

    void make_empty ()
    {
        jObj = JSONArray :: make ();
        THROW_ON_FALSE ( jObj != nullptr );
    }

protected:
    size_t pos;
    JSONArray *jObj;
};

// JSONObject
FIXTURE_TEST_CASE ( isArray, JSONFixture_JSONArray_Interface )
{
    make_empty();
    ASSERT_TRUE ( jObj -> isArray () );
}
FIXTURE_TEST_CASE ( clone_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    JSONValue *obj = jObj -> clone ();
    ASSERT_TRUE ( obj -> isArray () );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj -> toJSON() . c_str() );
    delete obj;
}
FIXTURE_TEST_CASE ( isEmpty_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    ASSERT_TRUE( jObj -> isEmpty() );
}
FIXTURE_TEST_CASE ( exists_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    ASSERT_FALSE( jObj -> exists ( 3 ) );
}
FIXTURE_TEST_CASE ( count_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    ASSERT_EQ ( jObj -> count (), 0lu );
}
FIXTURE_TEST_CASE ( appendValue, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> appendValue ( JSONValue :: makeNull () );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[null]" );
}
FIXTURE_TEST_CASE ( setValue_Null_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeNull () );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[null]" );
}
FIXTURE_TEST_CASE ( setValue_Bool_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeBool ( true ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[true]" );
}
FIXTURE_TEST_CASE ( setValue_Integer_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeInteger( 123 ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[123]" );
}
FIXTURE_TEST_CASE ( setValue_Double_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeDouble( 123.456789, 10 ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[123.456789]" );
}
FIXTURE_TEST_CASE ( setValue_Number_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeNumber( "123.456789" ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[123.456789]" );
}
FIXTURE_TEST_CASE ( setValue_String_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[\"value\"]" );
}
FIXTURE_TEST_CASE ( setValue_Object_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONObject :: make () );
    jObj -> getValue ( 0 ) . toObject ()
    . setValue ( "name", JSONValue :: makeString ( "value" ) );
    jObj -> getValue ( 0 ) . toObject ()
    . setValue ( "number", JSONValue :: makeInteger ( 2 ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[{\"name\":\"value\",\"number\":2}]" );
}
FIXTURE_TEST_CASE ( setValue_Array_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONArray :: make () );
    jObj -> getValue ( 0 ) . toArray ()
    . appendValue ( JSONValue :: makeString ( "first" ) );
    jObj -> getValue ( 0 ) . toArray ()
    . appendValue ( JSONValue :: makeInteger ( 2 ) );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[[\"first\",2]]" );
}
FIXTURE_TEST_CASE ( getValue_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
    JSONValue &val = jObj -> getValue ( 0 );
    EXPECT_STREQ ( val . toJSON() . c_str(), "\"value\"" );
}
FIXTURE_TEST_CASE ( getConstValue_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
    const JSONArray *cObj = jObj;
    const JSONValue &val =  cObj -> getValue ( 0 );
    EXPECT_STREQ ( val . toJSON() . c_str(), "\"value\"" );
}
FIXTURE_TEST_CASE ( removeValue_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
    delete jObj -> removeValue ( 0 );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[]" );
}
FIXTURE_TEST_CASE ( operator_equals_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
    JSONArray obj = *jObj;
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj . toJSON() . c_str () );
}

FIXTURE_TEST_CASE ( copy_constructor_arr, JSONFixture_JSONArray_Interface )
{
    make_empty();
    jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
    JSONArray obj ( *jObj );
    EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj . toJSON() . c_str () );
}

/* Fuzzing
    *
    **********************************************************************************/
class JSONFixture_Fuzzing
{
public:
    JSONFixture_Fuzzing ()
    : jObj ( nullptr )
    {
    }

    ~JSONFixture_Fuzzing ()
    {
        delete jObj;
    }

    void run_crash_file ( const std :: string & name)
    {
        std :: string path = "crash-files/" + name;
        FILE *file = fopen ( path . c_str (), "rb" );
        if ( file != nullptr )
        {
            try
            {
                fseek ( file, 0, SEEK_END );
                long fSize = ftell ( file );
                rewind ( file );

                char *buff = new char [ fSize ];
                try
                {
                    size_t num_read = fread ( buff, 1, fSize, file );
                    if ( num_read == ( size_t ) fSize )
                    {
                        EXPECT_ANY_THROW ( delete JSON :: parse ( JwtString ( buff, num_read ) ) );
                    }
                }
                catch ( ... )
                {
                    delete [] buff;
                    throw;
                }

                delete [] buff;
            }
            catch ( ... )
            {
                fclose ( file );
                throw;
            }
            fclose ( file );
        }
    }

protected:
    size_t pos;
    JSONObject *jObj;
};

FIXTURE_TEST_CASE ( test1, JSONFixture_Fuzzing )
{
    run_crash_file ( "oom-019001c7b22ae7889a8cf8e09def61728fc8cbdd" );
}

FIXTURE_TEST_CASE ( test2, JSONFixture_Fuzzing ) // fuzz crash but test doesnt
{
}

FIXTURE_TEST_CASE ( test3, JSONFixture_Fuzzing )
{
}

FIXTURE_TEST_CASE ( test4, JSONFixture_Fuzzing ) // fuzz crash but test doesnt
{
}

//////////////////////////////////////////// Main

extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

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

const char UsageDefaultName[] = "test-jwt";

rc_t CC KMain ( int argc, char *argv [] )
{
KConfigDisableUserSettings();
rc_t rc=JwtJsonTestSuite(argc, argv);
return rc;
}

}
