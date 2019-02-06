/*==============================================================================
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

#include "json-priv.hpp"

#include <assert.h>
#include <ctype.h>
#include <errno.h>

//#include <codecvt>
//#include <locale>

#include <klib/text.h>
#include <strtol.h>

namespace ncbi
{
    /* static functions
     **********************************************************************************/

    // skip whitespace
    // return the position of the first not whitespace character or npos
    static
    bool skip_whitespace ( const JwtString & text, size_t & pos )
    {
        size_t count = text . size ();

        while ( pos < count )
        {
            if ( ! isspace ( text [ pos ] ) )
                break;

            ++ pos;
        }

        if ( pos >= count )
        {
            pos = JwtString::npos;
            return false;
        }

        return true;
    }

    /* hex_to_int
    *  where 'c' is known to be hex
    */
    static
    unsigned int
    hex_to_int ( char c )
    {
        int i = c - '0';
        if ( c > '9' )
        {
            if ( c < 'a' )
                i = c - 'A' + 10;
            else
                i = c - 'a' + 10;
        }

        if ( i < 0 || i > 16 )
        {
            throw JSONException ( __func__, __LINE__, "Invalid \\u escape sequence" );
        }
        return i;
    }

    static
    JwtString hex_to_utf8 ( const JwtString &text )
    {
        // size_t index;

        // try
        // {
            // unsigned int val = stoi ( text, &index, 16 );
            // if ( index != 4 )
            //     throw JSONException ( __func__, __LINE__, "Invalid \\u escape sequence" ); // test hit

//            std :: wstring_convert < std :: codecvt_utf8 < char32_t >, char32_t > conv;
//            JwtString utf8 ( conv . to_bytes ( val ) . data () );

//            return utf8;

            if ( text.size() != 4 )
                throw JSONException ( __func__, __LINE__, "Invalid \\u escape sequence" );

            /* treat 4-digit hex code as UTF16 */
            uint32_t u32 = hex_to_int ( text [ 0 ]);
            u32 <<= 4;
            u32 += hex_to_int ( text [ 1 ]);
            u32 <<= 4;
            u32 += hex_to_int ( text [ 2 ]);
            u32 <<= 4;
            u32 += hex_to_int ( text [ 3 ]);

            char utf8[5];
            int ch_len = utf32_utf8 ( utf8, utf8 + 4, u32 );
            assert ( ch_len > 0 && ch_len < 5 );
            utf8 [ ch_len ] = 0;
            return JwtString(utf8);
        // }
        // catch ( ... )
        // {
        //     throw JSONException ( __func__, __LINE__, "Invalid \\u escape sequence" ); // test hit
        // }
    }

    static
    void test_wellformed_utf8 ( const JwtString & text )
    {
        const char * cp = text . data ();
        size_t i, count = text . size ();
        for ( i = 0; i < count; )
        {
            /*

              all bytes where the MSB is 0 are ASCII,
              i.e. single-byte characters. The value '\0'
              is not accepted.

              it is convenient to use signed bytes to examine
              the string and detect MSB as negative values.

             */
            while ( cp [ i ] > 0 )
            {
                if ( ++ i == count )
                    return;
            }

            /*

              an extended UTF-8 formatted multi-byte Unicode character
              has a start byte with the high 2..6 bits set, which indicates
              the overall length of the character.

              for UTF-8, the follow are legal start bytes:
                0b110xxxxx = 2 byte character
                0b1110xxxx = 3 byte character
                0b11110xxx = 4 byte character
                0b111110xx = 5 byte character
                0b1111110x = 6 byte character

              it is convenient in C to invert the bits as follows:
                0b110xxxxx => 0b001xxxxx
                0b1110xxxx => 0b0001xxxx
                0b11110xxx => 0b00001xxx
                0b111110xx => 0b000001xx
                0b1111110x => 0b0000001x
              since this allows use of a builtin function to count
              the leading 0's, which tells us immediately the length
              of the character.

            */


            // get complement of signed start byte
            unsigned int leading = ~ cp [ i ];

            // disallow 0, which comes from 0b11111111
            // since UTF-8 start bytes require at least one 0,
            // and the builtin function cannot operate on it.
            if ( leading == 0 )
                throw JSONException ( __func__, __LINE__, "malformed UTF-8" );

            // the bit calculations rely upon knowing the word size
            assert ( sizeof leading == 4 );

            // determine the character length by the number of leading zeros
            // only interested in the lower byte, so disregard upper 24 bits
            int char_len = ( int ) __builtin_clz ( leading ) - 24;

            // legal extended UTF-8 characters are 2..6 bytes long
            if ( char_len < 2 || char_len > 6 )
                throw JSONException ( __func__, __LINE__, "malformed UTF-8" );

            // the string must actually be large enough to contain these
            if ( i + char_len > count )
                throw JSONException ( __func__, __LINE__, "malformed UTF-8" );

            // the remaining bytes of the character all MUST begin
            // with the pattern 0b10xxxxxx. we can examine these
            // while building the Unicode value into UTF-32 format
            unsigned int utf32 = ~ leading & ( 0x7FU >> char_len );
            for ( int j = 1; j < char_len; ++ j )
            {
                unsigned int ch = ( ( const unsigned char * ) cp ) [ i + j ];
                if ( ( ch & 0xC0 ) != 0x80 )
                    throw JSONException ( __func__, __LINE__, "malformed UTF-8" );
                utf32 = ( utf32 << 6 ) | ( ch & 0x3F );
            }

            // TBD - examine code for validity in Unicode

            // account for multi-byte character
            i += char_len;
        }
    }

    static
    void test_depth ( const JSONValue :: Limits & lim, unsigned int & depth )
    {
        if ( ++ depth > lim . recursion_depth )
            throw JSONException ( __func__, __LINE__, "parsing recursion exceeds maximum depth" );
    }

    /* JSONValue :: Limits
     **********************************************************************************/
    JSONValue :: Limits :: Limits ()
        : json_string_size ( 4 * 1024 * 1024 )
        , recursion_depth ( 32 )
        , numeral_length ( 256 )
        , string_size ( 64 * 1024 )
        , array_elem_count ( 4 * 1024 )
        , object_mbr_count ( 256 )
    {
    }

    JSONValue :: Limits JSONValue :: default_limits;

    /* JSONWrapper
     **********************************************************************************/
    JSONValue * JSONWrapper :: parse ( const JwtString & json, size_t & pos )
    {
        assert ( json [ pos ] == 'n' );

        if ( json . compare ( pos, sizeof "null" - 1, "null" ) == 0 )
            pos += sizeof "null" - 1;
        else
            throw JSONException ( __func__, __LINE__, "Expected keyword: 'null'") ; // test hit

        if ( pos < json . size () && isalnum ( json [ pos ] ) )
            throw JSONException ( __func__, __LINE__, "Expected keyword: 'null'" ); // test hit

        return JSONValue :: makeNull ();
    }

    /* JSONBoolean
     **********************************************************************************/
    JSONValue * JSONBoolean :: parse ( const JwtString &json, size_t & pos )
    {
        assert ( json [ pos ] == 'f' || json [ pos ] == 't' );

        bool tf;
        size_t start = pos;

        if ( json . compare ( start, sizeof "false" - 1, "false" ) == 0 )
        {
            tf = false;
            pos += sizeof "false" - 1;
        }
        else if ( json . compare ( start, sizeof "true" - 1, "true" ) == 0 )
        {
            tf = true;
            pos += sizeof "true" - 1;
        }
        else if ( json [ start ] == 'f' )
            throw JSONException ( __func__, __LINE__, "Expected keyword: 'false'" ); // test hit
        else
            throw JSONException ( __func__, __LINE__, "Expected keyword: 'true'" ); // test hit

        // if there was any extra characters following identification of a valid bool token
        if ( pos < json . size () && isalnum ( json [ pos ] ) )
        {
            if ( json [ start ] == 'f' )
                throw JSONException ( __func__, __LINE__, "Expected keyword: 'false'" ); // test hit
            else
                throw JSONException ( __func__, __LINE__, "Expected keyword: 'true'" ); // test hit
        }


        JSONValue *val = JSONValue :: makeBool ( tf );
        if ( val == nullptr )
            throw JSONException ( __func__, __LINE__, "Failed to make JSONValue" );

        return val;
    }

    /* JSONNumber
     **********************************************************************************/
    JSONValue * JSONNumber :: parse  ( const JSONValue :: Limits & lim, const JwtString &json, size_t & pos )
    {
        assert ( isdigit ( json [ pos ] ) || json [ pos ] == '-' );

        size_t start = pos;

        if ( json [ pos ] == '-' )
            ++ pos;

        if ( ! isdigit ( json [ pos ] ) )
            throw JSONException ( __func__, __LINE__, "Expected: digit" ); // test hit

        // check for 0
        if ( json [ pos ] == '0' )
            ++ pos;
        else
        {
            // just find the end of the number
            while ( isdigit ( json [ ++ pos ] ) )
                ;
        }

        bool is_float = false;
        switch ( json [ pos ] )
        {
            case '.':
            {
                // skip digits in search of float indicator
                while ( isdigit ( json [ ++ pos ] ) )
                    is_float = true;

                // must have at least one digit
                if ( ! is_float )
                    break; // we have an integer

                // if a character other than was [eE] found, break
                if ( toupper ( json [ pos ] ) != 'E' )
                    break;

                // no break - we have an [eE], fall through
            }
            case 'E':
            case 'e':
            {
                switch ( json [ ++ pos ] )
                {
                    case '+':
                    case '-':
                        ++ pos;
                        break;
                }

                while ( isdigit ( json [ pos ] ) )
                {
                    is_float = true;
                    ++ pos;
                }

                break;
            }
        }

        // check the number of total characters
        if ( pos - start > lim . numeral_length )
            throw JSONException ( __func__, __LINE__, "numeral length exceeds allowed limit" );

        // "pos" could potentially be a little beyond the end of
        // a legitimate number - let the conversion routines tell us
        JwtString num_str = json . substr ( start, pos - start );

        size_t num_len = 0;
        if ( ! is_float )
        {
            // try
            // {
            //     long long int num = ncbi :: stoll ( num_str, &num_len );
            //     pos = start + num_len;

            //     return JSONValue :: makeInteger ( num );
            // }
            // catch ( std :: out_of_range &e )
            // {
            //     // fall out
            // }

            char * endptr;
            errno = 0;
            int64_t value = strtoi64 ( num_str . c_str(), & endptr, 10 );
            if ( errno == 0 )
            {
                num_len = endptr - num_str . c_str ();
                pos = start + num_len;
                return JSONValue :: makeInteger ( value );
            }
            // fall out
        }

        // must be floating point
        ncbi :: stold ( num_str, &num_len );
        pos = start + num_len;

        if ( num_len > lim . numeral_length )
            throw JSONException ( __func__, __LINE__, "numeral size exceeds allowed limit" );

        JSONValue *val = JSONValue :: makeParsedNumber ( num_str . substr ( 0, num_len ) );
        if ( val == nullptr )
            throw JSONException ( __func__, __LINE__, "Failed to make JSONValue" );

        return val;
    }

    /* JSONString
     **********************************************************************************/
    JSONValue * JSONString :: parse  ( const JSONValue :: Limits & lim, const JwtString &json, size_t & pos )
    {
        assert ( json [ pos ] == '"' );

        JwtString str;

        // Find ending '"' or control characters
        size_t esc = json . find_first_of ( "\\\"", ++ pos );
        if ( esc == JwtString :: npos )
            throw JSONException ( __func__, __LINE__, "Invalid begin of string format" ); // test hit

        while ( 1 )
        {
            // add everything before the escape in
            // to the new string
            if ( str . size () + ( esc - pos ) > lim . string_size )
                throw JSONException ( __func__, __LINE__, "string size exceeds allowed limit" );

            str += json . substr ( pos, esc - pos );
            pos = esc;

            // found end of string
            if ( json [ pos ] != '\\' )
                break;

            // found '\'
            switch ( json [ ++ pos ] )
            {
                case '"':
                    str += '"';
                    break;
                case '\\':
                    str += '\\';
                    break;
                case '/':
                    str += '/';
                    break;
                case 'b':
                    str += '\b';
                    break;
                case 'f':
                    str += '\f';
                    break;
                case 'n':
                    str += '\n';
                    break;
                case 'r':
                    str += '\r';
                    break;
                case 't':
                    str += '\t';
                    break;
                case 'u':
                {
                    // start at the element after 'u'
#pragma warning "still need to deal with this properly"
                    JwtString unicode = json . substr ( pos + 1, 4 );
                    JwtString utf8 = hex_to_utf8 ( unicode );

                    str += utf8;
                    pos += 4;

                    break;
                }

                default:
                    throw JSONException ( __func__, __LINE__, "Invalid escape character" ); // test hit
            }

            // skip escaped character
            ++ pos;

            // Find ending '"' or control characters
            esc = json . find_first_of ( "\\\"", pos );
            if ( esc == JwtString :: npos )
                throw JSONException ( __func__, __LINE__, "Invalid end of string format" ); // test hit
        }

        assert ( esc == pos );
        assert ( json [ pos ] == '"' );

        // set pos to point to next token
        ++ pos;

        if ( str . size () > lim . string_size )
            throw JSONException ( __func__, __LINE__, "string size exceeds allowed limit" );

        // examine all characters for legal and well-formed UTF-8
        test_wellformed_utf8 ( str );

        JSONValue *val = JSONValue :: makeParsedString ( str );
        if ( val == nullptr )
            throw JSONException ( __func__, __LINE__, "Failed to make JSONValue" );

        return val;
    }

    JSONValue * JSONValue :: parse ( const Limits & lim, const JwtString & json, size_t & pos, unsigned int depth )
    {
        if ( skip_whitespace ( json, pos ) )
        {
            switch ( json [ pos ] )
            {
                case '{':
                    return JSONObject :: parse ( lim, json, pos, depth );
                case '[':
                    return JSONArray :: parse ( lim, json, pos, depth );
                case '"':
                    return JSONString :: parse ( lim, json, pos );
                case 'f':
                case 't':
                    return JSONBoolean :: parse ( json, pos );
                case '-':
                    return JSONNumber :: parse ( lim, json, pos );
                case 'n':
                    return JSONWrapper :: parse ( json, pos );
                default:
                    if ( isdigit ( json [ pos ] ) )
                        return JSONNumber :: parse ( lim, json, pos );

                    // garbage
                    throw JSONException ( __func__, __LINE__, "Invalid JSON format" ); // test hit
            }
        }

        return nullptr;
    }

    JSONValue * JSONValue :: makeNumber ( const JwtString & val )
    {
        size_t pos = 0;
        return JSONNumber :: parse ( default_limits, val, pos );
    }

    JSONValue * JSONValue :: makeString ( const JwtString & str )
    {
        if ( str . size () > default_limits . string_size )
            throw JSONException ( __func__, __LINE__, "string size exceeds allowed limit" );

        // examine all characters for legal and well-formed UTF-8
        test_wellformed_utf8 ( str );

        JSONValue *val = JSONValue :: makeParsedString ( str );
        if ( val == nullptr )
            throw JSONException ( __func__, __LINE__, "Failed to make JSONValue" );

        return val;
    }


    /* JSONArray
     **********************************************************************************/
    JSONArray * JSONArray :: parse ( const Limits & lim, const JwtString & json, size_t & pos, unsigned int depth )
    {
        assert ( json [ pos ] == '[' );

        JSONArray *array = new JSONArray ();
        try
        {
            while ( 1 )
            {
                // skip over '[' and any whitespace
                // json [ 0 ] is '[' or ','
                if ( ! skip_whitespace ( json, ++ pos ) )
                    throw JSONException ( __func__, __LINE__, "Expected: ']'" ); // test hit

                if ( json [ pos ] == ']' )
                    break;

                // use scope to invalidate value
                {
                    JSONValue *value = JSONValue :: parse ( lim, json, pos, depth );
                    if ( value == nullptr )
                        throw JSONException ( __func__, __LINE__, "Failed to create JSON object" );

                    array -> appendValue ( value );

                    if ( array -> count () > default_limits . array_elem_count )
                        throw JSONException ( __func__, __LINE__, "Array element count exceeds limit" );
                }

                // find and skip over ',' and skip any whitespace
                // exit loop if no ',' found
                if ( ! skip_whitespace ( json, pos ) || json [ pos ] != ',' )
                    break;
            }

            // must end on ']'
            if ( pos == JwtString :: npos || json [ pos ] != ']' )
                throw JSONException ( __func__, __LINE__, "Expected: ']'" ); // Test hit

            // skip over ']'
            ++ pos;
        }
        catch ( ... )
        {
            delete array;
            throw;
        }

        // JSONArray must be valid
        assert ( array != nullptr );
        return array;
    }

    /* JSONObject
     **********************************************************************************/

    // make an object from JSON source
    JSONObject * JSONObject :: parse ( const JwtString & json )
    {
        return parse ( default_limits, json );
    }

    JSONObject * JSONObject :: parse ( const JSONValue :: Limits & lim, const JwtString & json )
    {
        if ( json . empty () )
            throw JSONException ( __func__, __LINE__, "Empty JSON source" );

        if ( json . size () > lim . json_string_size )
            throw JSONException ( __func__, __LINE__, "JSON source exceeds allowed size limit" );

        size_t pos = 0;

        if ( ! skip_whitespace ( json, pos ) || json [ pos ] != '{' )
            throw JSONException ( __func__, __LINE__, "Expected: '{'" );

        JSONObject *obj = parse ( lim, json, pos, 0 );

        if ( pos < json . size () )
            throw JSONException ( __func__, __LINE__, "Trailing byes in JSON text" );

        return obj;
    }

    JSONObject * JSONObject :: parse ( const Limits & lim, const JwtString & json, size_t & pos, unsigned int depth )
    {
        test_depth ( lim, depth );

        assert ( json [ pos ] == '{' );

        JSONObject *obj = new JSONObject ();
        try
        {
            while ( 1 )
            {
                // skip over '{' and any whitespace
                // json [ pos ] is '{' or ',', start at json [ pos + 1 ]
                if ( ! skip_whitespace ( json, ++ pos ) )
                    throw JSONException ( __func__, __LINE__, "Expected: '}'" ); // test hit

                if ( json [ pos ] == '}' )
                    break;

                if ( json [ pos ] != '"' )
                    throw JSONException ( __func__, __LINE__, "Expected: 'name' " );

                JSONValue *name = JSONString :: parse ( lim, json, pos );
                if ( name == nullptr )
                    throw JSONException ( __func__, __LINE__, "Failed to create JSON object" );

                try
                {
                    // skip to ':'
                    if ( ! skip_whitespace ( json, pos ) || json [ pos ] != ':' )
                        throw JSONException ( __func__, __LINE__, "Expected: ':'" ); // test hit

                    // skip over ':'
                    ++ pos;

                    // get JSON value;
                    {
                        JSONValue *value = JSONValue :: parse ( lim, json, pos, depth );
                        try
                        {
                            if ( value == nullptr )
                                throw JSONException ( __func__, __LINE__, "Failed to create JSON object" );

                            obj -> addNameValuePair ( name -> toString(), value );
                        }
                        catch ( ... )
                        {
                            delete value;
                            throw;
                        }
                    }

                    if ( obj -> count () > default_limits . object_mbr_count )
                        throw JSONException ( __func__, __LINE__, "Array element count exceeds limit" );
                }
                catch ( ... )
                {
                    delete name;
                    throw;
                }

                delete name;

                // find and skip over ',' and skip any whitespace
                // exit loop if no ',' found
                if ( ! skip_whitespace ( json, pos ) || json [ pos ] != ',' )
                    break;
            }

            // must end on '}'
            if ( pos == JwtString :: npos || json [ pos ] != '}' )
                throw JSONException ( __func__, __LINE__, "Expected: '}'" ); // test hit

            // skip over '}'
            ++ pos;
        }
        catch ( ... )
        {
            delete obj;
            throw;
        }


        // JSONObject must be valid
        assert ( obj != nullptr );
        return obj;
    }


    /* JSON
     **********************************************************************************/

    // make an object from JSON source
    JSONValue * JSON :: parse ( const JwtString & json )
    {
        return parse ( JSONValue :: default_limits, json );
    }

    JSONValue * JSON :: parse ( const JSONValue :: Limits & lim, const JwtString & json )
    {
        if ( json . empty () )
            throw JSONException ( __func__, __LINE__, "Empty JSON source" );

        if ( json . size () > lim . json_string_size )
            throw JSONException ( __func__, __LINE__, "JSON source exceeds allowed size limit" );

        size_t pos = 0;

        if ( ! skip_whitespace ( json, pos ) )
            throw JSONException ( __func__, __LINE__, "Expected: '{' or '['" );

        JSONValue *val = nullptr;
        switch ( json [ pos ] )
        {
        case '{':
            val = JSONObject :: parse ( lim, json, pos, 0 );
            break;
        case '[':
            val = JSONArray :: parse ( lim, json, pos, 0 );
            break;
        default:
            throw JSONException ( __func__, __LINE__, "Expected: '{' or '['" );
        }

        if ( pos < json . size () )
        {
            delete val;
            throw JSONException ( __func__, __LINE__, "Trailing byes in JSON text" );
        }

        return val;
    }
}

#if 0

// trying to switch to using KJson's parser

    JSONValue * JSONValue::fromKJson( const struct KJsonValue * )
    {
        JSONValue * ret = makeNull();
        return ret;
    }

    JSONArray * JSONArray::fromKJson( const struct KJsonArray * )
    {   // move the contents of KJsonValue to JSONArray
        JSONArray * ret = make ();
        return ret;
    }


    JSONObject * JSONObject::fromKJson( const struct KJsonObject * p_node )
    {   // copy the contents of KJsonObject to JSONObject
        JSONObject * ret = make ();
        VNamelist * names;
        rc_t rc = VNamelist :: Make ( & names, );
        rc_t rc = KJsonObjectGetNames ( p_node, & names );
        if ( rc != 0 )
        {
            throw JSONException ( __func__, __LINE__, "KJsonObjectGetNames failed" ) ;
        }


        return ret;
    }

    /* JSON
     **********************************************************************************/

    // make an object from JSON source
    JSONValue * JSON :: parse ( const JwtString & json )
    {
        return parse ( JSONValue :: default_limits, json );
    }

    JSONValue * JSON :: parse ( const JSONValue :: Limits & lim, const JwtString & json )
    {
        if ( json . empty () )
            throw JSONException ( __func__, __LINE__, "Empty JSON source" );

        if ( json . size () > lim . json_string_size )
            throw JSONException ( __func__, __LINE__, "JSON source exceeds allowed size limit" );

        // size_t pos = 0;

        KJsonValue * root;
        char error[1024];
        rc_t rc = KJsonValueMake ( & root, json.c_str(), error, sizeof ( error ) );
        if ( rc != 0 )
        {
            throw JSONException ( __func__, __LINE__, error );
        }

        //TODO: report trailing ws from bison-generated parser
        // if ( pos < json . size () )
        //     throw JSONException ( __func__, __LINE__, "Trailing byes in JSON text" );

        assert( root != nullptr );
        JSONValue * ret;
        switch ( KJsonGetValueType( root ) )
        {
        case jsObject:
            ret = JSONObject::fromKJson ( KJsonValueToObject ( root ) );
            break;
        case jsArray:
            ret = JSONArray::fromKJson ( KJsonValueToArray ( root ) ) ;
            break;
        default:
            throw JSONException ( __func__, __LINE__, "Invalid type of Json object" );
        }

        KJsonValueWhack ( root );
        return ret;
    }
}
#endif