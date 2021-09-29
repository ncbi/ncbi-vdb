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
#include <ncbi/secure/payload.hpp>

#include <assert.h>
#include <ctype.h>
#include <codecvt>
#include <locale>
#include <errno.h>

namespace ncbi
{
    static bool have_limits;
    JSON :: Limits JSON :: default_limits;

    String double_to_string ( long double val, unsigned int precision )
    {
        // TBD - come up with a more precise cutoff
        if ( precision > 40 )
            precision = 40;
        
        char buffer [ 1024 ];
        int len = std :: snprintf ( buffer, sizeof buffer, "%.*Lg", precision, val );
        if ( len < 0 )
        {
            int status = errno;
            throw JSONInternalError (
                XP ( XLOC )
                << "failed to convert long double to string - "
                << syserr ( status )
                );
        }
        else if ( ( size_t ) len >= sizeof buffer )
        {
            throw JSONInternalError (
                XP ( XLOC )
                << "failed to convert long double to string - "
                << "1024-byte buffer was insufficient"
                );
        }
        
        return String ( buffer, len );
    }
    
    // skip whitespace
    // adjust the cursor position until it reaches end or something other than whitespace
    // returns true if text found ( not end of text )
    static
    bool skip_whitespace ( String :: Iterator & curs )
    {
        for ( ; curs . isValid (); ++ curs )
        {
            if ( ! iswspace ( * curs ) )
                return true;
        }

        return false;
    }

    static
    unsigned int htoi ( UTF32 h )
    {
        assert ( iswxdigit ( h ) );
        if ( iswdigit ( h ) )
            return h - '0';
        return towupper ( h ) - 'A' + 10;
    }

    static
    unsigned int htoi ( UTF32 h1, UTF32 h2 )
    {
        return ( htoi ( h1 ) << 4 ) | htoi ( h2 );
    }

    static
    unsigned int htoi ( UTF32 h1, UTF32 h2, UTF32 h3, UTF32 h4 )
    {
        return ( htoi ( h1, h2 ) << 8 ) | htoi ( h3, h4 );
    }

    static
    String hex_to_utf8 ( const JSON :: Limits & lim, String :: Iterator & curs,
        const String :: Iterator & start )
    {
        assert ( start . isValid () );
        assert ( curs . isValid () );

        Payload pay ( 256 );
        UTF16 * buffer = ( UTF16 * ) pay . data ();
        count_t bend = pay . capacity () / sizeof buffer [ 0 ];
        count_t bidx = 0;

        unsigned int start_offset = ( unsigned int ) start . byteOffset ();
        unsigned int start_index = ( unsigned int ) start . charIndex ();

        while ( curs . isValid () )
        {
            size_t str_size = curs . byteOffset () - start_offset;
            if ( ( unsigned int ) str_size >= lim . string_size )
            {
                throw JSONLimitViolation (
                    XP ( XLOC )
                    << "string size ( "
                    << str_size
                    << " ) exceeds allowed limit ( "
                    << lim . string_size
                    << " )"
                    );
            }

            count_t str_length = curs . charIndex () - start_index;
            if ( ( unsigned int ) str_length >= lim . string_length )
            {
                throw JSONLimitViolation (
                    XP ( XLOC )
                    << "string length ( "
                    << str_length
                    << " ) exceeds allowed limit ( "
                    << lim . string_length
                    << " )"
                    );
            }

            auto save = curs;
            try
            {
                // look for escape
                if ( * curs != '\\' )
                    break;
                    
                // look for 'u'
                if ( * ++ curs != 'u' )
                {
                    -- curs;
                    break;
                }
            
                // no turning back now.
                // expect 4 hex digits
                UTF32 h1 = * ++ curs;
                if ( ! iswxdigit ( h1 ) )
                    throw MalformedJSON ( XP ( XLOC ) << "expected hex digit" );

                UTF32 h2 = * ++ curs;
                if ( ! iswxdigit ( h2 ) )
                    throw MalformedJSON ( XP ( XLOC ) << "expected hex digit" );

                UTF32 h3 = * ++ curs;
                if ( ! iswxdigit ( h3 ) )
                    throw MalformedJSON ( XP ( XLOC ) << "expected hex digit" );

                UTF32 h4 = * ++ curs;
                if ( ! iswxdigit ( h4 ) )
                    throw MalformedJSON ( XP ( XLOC ) << "expected hex digit" );

                ++ curs;

                // check buffer capacity
                if ( bidx == bend )
                {
                    pay . increaseCapacity ();
                    buffer = ( UTF16 * ) pay . data ();
                    bend = pay . capacity () / sizeof buffer [ 0 ];
                }

                // convert to a UTF-16 character
                buffer [ bidx ++ ] = htoi ( h1, h2, h3, h4 );

                // record size update
                pay . setSize ( bidx * sizeof buffer [ 0 ] );

            }
            catch ( BoundsException & x )
            {
                curs = save;
                throw MalformedJSON ( XP ( XLOC ) << "bad escape sequence" );
            }
            catch ( ... )
            {
                curs = save;
                throw;
            }
        }

        // convert UTF-16 to UTF-8
        return String ( buffer, bidx );
    }
    
    static
    void test_depth ( const JSON :: Limits & lim, unsigned int & depth )
    {
        if ( ++ depth > lim . recursion_depth )
        {
            throw JSONLimitViolation (
                XP ( XLOC )
                << "parsing recursion depth ( "
                << depth
                << " ) exceeds maximum depth ( "
                << lim . recursion_depth
                << " )"
                );
        }
    }

    JSONValueRef JSON :: parse ( const String & json )
    {
        // parse with default limits
        initLimits ();
        return parse ( default_limits, json );
    }

    JSONValueRef JSON :: parse ( const Limits & lim, const String & json )
    {
        // refuse empty JSON
        if ( json . isEmpty () )
            throw MalformedJSON ( XP ( XLOC ) << "Empty JSON source" );

        // check up front if it is huge
        if ( json . size () > ( size_t ) lim . json_string_size )
        {
            throw JSONLimitViolation (
                XP ( XLOC )
                << "JSON source size ( "
                << json . size ()
                << " ) exceeds allowed limit ( "
                << lim . json_string_size
                << " )"
                );
        }

        // walk across string with iterator
        auto curs = json . makeIterator ();

        // all whitespace is as good as empty
        if ( ! skip_whitespace ( curs ) )
        {
            throw MalformedJSON (
                XP ( XLOC )
                << "Expected: '{' or '[' at position "
                << curs . charIndex ()
                );
        }

        // return value
        JSONValueRef val;
        switch ( * curs )
        {
        case '{':
            val = parseObject ( lim, json, curs, 0 ) . release ();
            break;
        case '[':
            val = parseArray ( lim, json, curs, 0 ) . release ();
            break;
        case '"':
            val = parseString ( lim, json, curs );
            break;
        case 'f':
        case 't':
            val = parseBoolean ( curs );
            break;
        case '-':
            val = parseNumber ( lim, json, curs );
            break;
        case 'n':
            val = parseNull ( curs );
            break;
        default:
            if ( iswdigit ( * curs ) )
            {
                val = parseNumber ( lim, json, curs );
                break;
            }

            throw MalformedJSON (
                XP ( XLOC )
                << "Expected: '{' or '[' at position "
                << curs . charIndex ()
                );
        }

        // reject if any trailing characters are left
        if ( skip_whitespace ( curs ) )
        {
            throw MalformedJSON (
                XP ( XLOC )
                << "Trailing bytes in JSON text at position "
                << curs . charIndex ()
                );
        }

        return val;
    }

    JSONArrayRef JSON :: parseArray ( const String & json )
    {
        // parse with default limits
        initLimits ();
        return parseArray ( default_limits, json );
    }

    JSONArrayRef JSON :: parseArray ( const Limits & lim, const String & json )
    {
        // refuse empty JSON
        if ( json . isEmpty () )
            throw MalformedJSON ( XP ( XLOC ) << "Empty JSON source" );

        // check up front if it is huge
        if ( json . size () > ( size_t ) lim . json_string_size )
        {
            throw JSONLimitViolation (
                XP ( XLOC )
                << "JSON source size ( "
                << json . size ()
                << " ) exceeds allowed limit ( "
                << lim . json_string_size
                << " )"
                );
        }

        // walk across string with iterator
        auto curs = json . makeIterator ();

        // all whitespace is as good as empty
        if ( ! skip_whitespace ( curs ) )
        {
            throw MalformedJSON (
                XP ( XLOC )
                << "Expected: '[' at position "
                << curs . charIndex ()
                );
        }

        // return array
        JSONArrayRef array;
        switch ( * curs )
        {
        case '[':
            array = parseArray ( lim, json, curs, 0 );
            break;
        default:
            throw MalformedJSON (
                XP ( XLOC )
                << "Expected: '[' at position "
                << curs . charIndex ()
                );
        }

        // reject if any trailing characters are left
        if ( skip_whitespace ( curs ) )
        {
            throw MalformedJSON (
                XP ( XLOC )
                << "Trailing bytes in JSON text at position "
                << curs . charIndex ()
                );
        }

        return array;
    }

    JSONObjectRef JSON :: parseObject ( const String & json )
    {
        // parse with default limits
        initLimits ();
        return parseObject ( default_limits, json );
    }

    JSONObjectRef JSON :: parseObject ( const Limits & lim, const String & json )
    {
        // refuse empty JSON
        if ( json . isEmpty () )
            throw MalformedJSON ( XP ( XLOC ) << "Empty JSON source" );

        // check up front if it is huge
        if ( json . size () > ( size_t ) lim . json_string_size )
        {
            throw JSONLimitViolation (
                XP ( XLOC )
                << "JSON source size ( "
                << json . size ()
                << " ) exceeds allowed limit ( "
                << lim . json_string_size
                << " )"
                );
        }

        // walk across string with iterator
        auto curs = json . makeIterator ();

        // all whitespace is as good as empty
        if ( ! skip_whitespace ( curs ) )
        {
            throw MalformedJSON (
                XP ( XLOC )
                << "Expected: '{' at position "
                << curs . charIndex ()
                );
        }

        // return object
        JSONObjectRef obj;
        switch ( * curs )
        {
        case '{':
            obj = parseObject ( lim, json, curs, 0 );
            break;
        default:
            throw MalformedJSON (
                XP ( XLOC )
                << "Expected: '{' at position "
                << curs . charIndex ()
                );
        }

        // reject if any trailing characters are left
        if ( skip_whitespace ( curs ) )
        {
            throw MalformedJSON (
                XP ( XLOC )
                << "Trailing bytes in JSON text at position "
                << curs . charIndex ()
                );
        }

        return obj;
    }

    JSONValueRef JSON :: makeNull ()
    {
        // create a wrapper with no value
        return JSONValueRef ( new JSONWrapper ( jvt_null ) );
    }

    JSONValueRef JSON :: makeBoolean ( bool val )
    {
        // create a wrapper with a Boolean value
        return JSONValueRef ( new JSONWrapper ( jvt_bool, new JSONBoolean ( val ) ) );
    }

    JSONValueRef JSON :: makeNumber ( const String & val )
    {
        // parse a number from a string using default limits
        initLimits ();
        auto curs = val . makeIterator ();
        return parseNumber ( default_limits, val, curs );
    }

    JSONValueRef JSON :: makeInteger ( long long int val )
    {
        // make an integer from binary value
        return JSONValueRef ( new JSONWrapper ( jvt_int, new JSONInteger ( val ) ) );
    }

    JSONValueRef JSON :: makeDouble ( long double val, unsigned int precision )
    {
        // make a textual number from binary value and precision
        return makeParsedNumber ( double_to_string ( val, precision ) );
    }

    JSONValueRef JSON :: makeString ( const String & str )
    {
        initLimits ();

        // check string size limit
        if ( str . size () > default_limits . string_size )
        {
            throw JSONLimitViolation (
                XP ( XLOC )
                << "string size ( "
                << str . size ()
                << " ) exceeds allowed limit ( "
                << default_limits . string_size
                << " )"
                );
        }

        // check string length limit
        if ( str . length () > default_limits . string_length )
        {
            throw JSONLimitViolation (
                XP ( XLOC )
                << "string length ( "
                << str . length ()
                << " ) exceeds allowed limit ( "
                << default_limits . string_length
                << " )"
                );
        }
        
        // make a string from outside
        return makeParsedString ( str );
    }

    JSONArrayRef JSON :: makeArray ()
    {
        // make a new empty array
        return JSONArrayRef ( new JSONArray () );
    }

    JSONObjectRef JSON :: makeObject ()
    {
        // make a new empty object
        return JSONObjectRef ( new JSONObject () );
    }

    void JSON :: initLimits ()
    {
        if ( ! have_limits )
            default_limits = Limits ();
    }
    
    JSONValueRef JSON :: parse ( const Limits & lim, const String & json,
        String :: Iterator & curs, unsigned int depth )
    {
        // parse potentially empty text
        if ( skip_whitespace ( curs ) )
        {
            switch ( * curs )
            {
                case '{':
                    return parseObject ( lim, json, curs, depth ) . release ();
                case '[':
                    return parseArray ( lim, json, curs, depth ) . release ();
                case '"':
                    return parseString ( lim, json, curs );
                case 'f':
                case 't':
                    return parseBoolean ( curs );
                case '-':
                    return parseNumber ( lim, json, curs );
                case 'n':
                    return parseNull ( curs );
                default:
                    if ( iswdigit ( * curs ) )
                        return parseNumber ( lim, json, curs );

                    // garbage
                    throw MalformedJSON (
                        XP ( XLOC )
                        << "Invalid JSON format at position "
                        << curs . charIndex ()
                        );
            }
        }

        // this is where we need to check returns for nullptr
        return JSONValueRef ( nullptr );
    }

    JSONValueRef JSON :: parseNull ( String :: Iterator & curs )
    {
        count_t pos = curs . charIndex ();

        do
        {
            try
            {
                // spell 'n' 'u' 'l' 'l' one character at a time
                assert ( * curs == 'n' );
                if ( * ++ curs != 'u' )
                    break;
                if ( * ++ curs != 'l' )
                    break;
                if ( * ++ curs != 'l' )
                    break;
                ++ curs;
                
                // ensure that it doesn't appear to continue
                if ( curs . isValid () && iswalnum ( * curs ) )
                    break;

                // make a null element
                return makeNull ();
            }
            catch ( ... )
            {
            }

        }
        while ( false );

        // bad JSON
        throw MalformedJSON (
            XP ( XLOC )
            << "Expected keyword: 'null' at position "
            << pos
            ) ;
    }

    JSONValueRef JSON :: parseBoolean ( String :: Iterator & curs )
    {
        count_t pos = curs . charIndex ();

        bool which = false;
        bool known = false;
        do
        {
            try
            {
                if ( * curs == 't' )
                {
                    which = known = true;
                    if ( * ++ curs != 'r' )
                        break;
                    if ( * ++ curs != 'u' )
                        break;
                    if ( * ++ curs != 'e' )
                        break;
                }
                else
                {
                    known = true;
                    assert ( * curs == 'f' );
                    if ( * ++ curs != 'a' )
                        break;
                    if ( * ++ curs != 'l' )
                        break;
                    if ( * ++ curs != 's' )
                        break;
                    if ( * ++ curs != 'e' )
                        break;
                }

                ++ curs;
                
                // ensure that it doesn't appear to continue
                if ( curs . isValid () && iswalnum ( * curs ) )
                    break;

                return makeBoolean ( which );
            }
            catch ( ... )
            {
            }

        }
        while ( false );

        // bad JSON
        throw MalformedJSON (
            XP ( XLOC )
            << "Expected keyword: '"
            << ( known ? ( which ? "true" : "false" ) : "true' or 'false" )
            << "' at position "
            << pos
            ) ;
    }

    JSONValueRef JSON :: parseNumber ( const Limits & lim,
        const String & json, String :: Iterator & curs )
    {
        assert ( iswdigit ( * curs ) || * curs == '-' );

        // record starting position within string
        count_t start = curs . charIndex ();

        // skip over negation
        if ( * curs == '-' )
            ++ curs;

        if ( ! curs . isValid () || ! iswdigit ( * curs ) )
        {
            throw MalformedJSON (
                XP ( XLOC )
                << "Expected: digit at position "
                << curs . charIndex ()
                );
        }

        // check for 0
        if ( * curs == '0' )
            ++ curs;
        else
        {
            // we know from the tests above that val is 1..9
            assert ( * curs >= '1' && * curs <= '9' );
            ++ curs;

            // just find the end of the number
            while ( curs . isValid () && iswdigit ( * curs ) )
                ++ curs;
        }

        // declare a cursor to peek ahead
        auto peek = curs;

        bool is_float = false;
        if ( peek . isValid () )
        {
            switch ( * peek )
            {
            case '.':
            {
                // skip digits in search of float indicator
                ++ peek;
                while ( peek . isValid () && iswdigit ( * peek ) )
                {
                    ++ peek;
                    is_float = true;
                }

                // must have at least one digit
                if ( ! is_float )
                    break; // we have an integer

                // if a character other than was [eE] found, break
                if ( peek . isValid () && towupper ( * peek ) != 'E' )
                    break;

                // no break - we have an [eE], fall through
            }
            case 'E':
            case 'e':
            {
                ++ peek;
                if ( peek . isValid () )
                {
                    switch ( * peek )
                    {
                    case '+':
                    case '-':
                        ++ peek;
                        break;
                    }
                }

                while ( peek . isValid () && iswdigit ( * peek ) )
                {
                    ++ peek;
                    is_float = true;
                }

                break;
            }}
        }

        // update curs if we found floating point
        if ( is_float )
            curs = peek;

        // check the number of total characters
        count_t num_length = curs . charIndex () - start;
        if ( num_length > lim . numeral_length )
        {
            throw JSONLimitViolation (
                XP ( XLOC )
                << "numeral length ( "
                << num_length
                << " ) exceeds allowed limit ( "
                << lim . numeral_length
                << " )"
                );
        }

        // this is the numeric string
        String num_str = json . subString ( start, curs . charIndex () - start );
        if ( ! is_float )
        {
            try
            {
                // try to convert it to a binary integer
                long long int num = decToLongLongInteger ( num_str );
                return makeInteger ( num );
            }
            catch ( OverflowException & x )
            {
                // too big - fall out
            }
        }

        // keep it as a string
        return makeParsedNumber ( num_str );
    }

    JSONValueRef JSON :: parseString ( const Limits & lim,
        const String & json, String :: Iterator & curs )
    {
        assert ( * curs == '"' );

        // accumulate text here
        StringBuffer sb;

        // mark the start of the string
        String :: Iterator start ( ++ curs );

        // a look-ahead
        String :: Iterator delim = curs;

        // Find ending '"' or escaped characters
        if ( ! delim . findFirstOf ( "\\\"" ) )
            throw MalformedJSON ( XP ( XLOC ) << "unterminated string" );

        while ( 1 )
        {
            // add everything before the delimiter to the new string
            size_t proj_size = sb . size () + ( delim . byteOffset () - curs . byteOffset () );
            if ( proj_size > lim . string_size )
            {
                throw JSONLimitViolation (
                    XP ( XLOC )
                    << "string size ( "
                    << proj_size
                    << " ) exceeds allowed limit ( "
                    << lim . string_size
                    << " )"
                    );
            }

            count_t proj_len = sb . length () + ( delim - curs );
            if ( proj_len > lim . string_length )
            {
                throw JSONLimitViolation (
                    XP ( XLOC )
                    << "string length ( "
                    << proj_len
                    << " ) exceeds allowed limit ( "
                    << lim . string_length
                    << " )"
                    );
            }

            sb += json . subString ( curs . charIndex (), delim - curs );
            curs = delim;

            // found end of string
            if ( * curs != '\\' )
                break;

            // found '\'
            bool advance = true;
            switch ( * ++ curs )
            {
                case '"':
                    sb += '"';
                    break;
                case '\\':
                    sb += '\\';
                    break;
                case '/':
                    sb += '/';
                    break;
                case 'b':
                    sb += '\b';
                    break;
                case 'f':
                    sb += '\f';
                    break;
                case 'n':
                    sb += '\n';
                    break;
                case 'r':
                    sb += '\r';
                    break;
                case 't':
                    sb += '\t';
                    break;
                case 'u':
                {
                    // back up to escape for regular pattern
                    -- curs;
                    String utf8 = hex_to_utf8 ( lim, curs, start );
                    sb += utf8;

                    // prepare for skip ahead below
                    advance = false;
                    break;
                }

                default:
                    throw MalformedJSON ( XP ( XLOC ) << "Invalid escape character" ); // test hit
            }

            // skip escaped character
            if ( advance )
                ++ curs;

            // Find ending '"' or control characters
            if ( ! ( delim = curs ) . findFirstOf ( "\\\"" ) )
                throw MalformedJSON ( XP ( XLOC ) << "unterminated string" );
        }

        // being here should mean that we had a break above
        // on the line looking for a backslash
        assert ( delim == curs );

        // because "esc" is identical to "curs"
        // and because "esc" found either a backslash or quote,
        // and because backslash kept us in the loop, we know
        // that the current character must be a closing quote.
        assert ( * curs == '"' );

        // set pos to point to next token
        ++ curs;

        if ( sb . size () > lim . string_size )
        {
            throw JSONLimitViolation (
                XP ( XLOC )
                << "string size ( "
                << sb . size ()
                << " ) exceeds allowed limit ( "
                << lim . string_size
                << " )"
                );
        }
        if ( sb . length () > lim . string_length )
        {
            throw JSONLimitViolation (
                XP ( XLOC )
                << "string length ( "
                << sb . length ()
                << " ) exceeds allowed limit ( "
                << lim . string_length
                << " )"
                );
        }
        
        return makeParsedString ( sb . stealString () );
    }

    JSONArrayRef JSON :: parseArray ( const Limits & lim, const String & json,
        String :: Iterator & curs, unsigned int depth )
    {
        assert ( * curs == '[' );

        JSONArrayRef array ( new JSONArray () );
        while ( 1 )
        {
            // skip over '[' and any whitespace
            // * curs is known to be  '[' or ','
            if ( ! skip_whitespace ( ++ curs ) )
                throw MalformedJSON ( XP ( XLOC ) << "Expected: ']'" );

            // allow an empty array
            if ( * curs == ']' )
                break;

            // use scope to invalidate value
            {
                JSONValueRef value = parse ( lim, json, curs, depth );
                if ( value == nullptr )
                    throw MalformedJSON ( XP ( XLOC ) << "Excpected: ',' or ']'" );

                array -> appendValue ( value );

                if ( array -> count () > lim . array_elem_count )
                {
                    throw JSONLimitViolation (
                        XP ( XLOC )
                        << "Array element count ( "
                        << array -> count ()
                        << " ) exceeds limit ( "
                        << lim . array_elem_count
                        << " )"
                        );
                }
            }

            // find and skip over ',' and skip any whitespace
            // exit loop if no ',' found
            if ( ! skip_whitespace ( curs ) || * curs != ',' )
                break;
        }

        // must end on ']'
        if ( ! curs . isValid () || * curs != ']' )
            throw MalformedJSON ( XP ( XLOC ) << "Excpected: ']'" );

        // skip over ']'
        ++ curs;

        // JSONArray must be valid
        assert ( array != nullptr );
        return array;
    }
        
    JSONObjectRef JSON :: parseObject ( const Limits & lim, const String & json,
        String :: Iterator & curs, unsigned int depth )
    {
        test_depth ( lim, depth );

        assert ( * curs == '{' );

        JSONObjectRef obj ( new JSONObject () );
        while ( 1 )
        {
            // skip over '{' and any whitespace
            // json [ pos ] is '{' or ',', start at json [ pos + 1 ]
            if ( ! skip_whitespace ( ++ curs ) )
                throw MalformedJSON ( XP ( XLOC ) << "Expected: '}'" );

            if ( * curs == '}' )
                break;

            if ( * curs != '"' )
                throw MalformedJSON ( XP ( XLOC ) << "Expected: \"<name>\" " );

            JSONValueRef name = parseString ( lim, json, curs );

            // skip to ':'
            if ( ! skip_whitespace ( curs ) || * curs != ':' )
                throw MalformedJSON ( XP ( XLOC ) << "Expected: ':'" );
                
            // skip over ':'
            ++ curs;
                
            // get JSON value;
            {
                JSONValueRef value = parse ( lim, json, curs, depth );
                if ( value == nullptr )
                    throw MalformedJSON ( XP ( XLOC ) << "Expected: ',' or '}'" );
                            
                obj -> addValue ( name -> toString (), value );
            }
                
            if ( obj -> count () > lim . object_mbr_count )
            {
                throw JSONLimitViolation (
                    XP ( XLOC )
                    << "Array element count ( "
                    << obj -> count ()
                    << " ) exceeds limit ( "
                    << lim . object_mbr_count
                    << " )"
                    );
            }

            // find and skip over ',' and skip any whitespace
            // exit loop if no ',' found
            if ( ! skip_whitespace ( curs ) || * curs != ',' )
                break;
        }

        // must end on '}'
        if ( ! curs . isValid () || * curs != '}' )
            throw MalformedJSON ( XP ( XLOC ) << "Expected: '}'" ); // test hit

        // skip over '}'
        ++ curs;

        // JSONObject must be valid
        assert ( obj != nullptr );
        return obj;
    }

    JSONValueRef JSON :: makeParsedNumber ( const String & val )
    {
        // numeric string is already validated,
        // so just create the value and wrapper
        return JSONValueRef ( new JSONWrapper ( jvt_num, new JSONNumber ( val ) ) );
    }

    JSONValueRef JSON :: makeParsedString ( const String & val )
    {
        // string is already valid/transformed
        // just create a value and wrapper
        return JSONValueRef ( new JSONWrapper ( jvt_str, new JSONString ( val ) ) );
    }

    JSONValueRef JSON :: test_parse ( const String & json, bool consume_all )
    {
        if ( json . isEmpty () )
            throw MalformedJSON ( XP ( XLOC ) << "Empty JSON source" );

        initLimits ();

        if ( json . size () > default_limits . json_string_size )
            throw JSONLimitViolation ( XP ( XLOC ) << "JSON source exceeds allowed size limit" );

        auto curs = json . makeIterator ();
        JSONValueRef val = parse ( default_limits, json, curs, 0 );

        if ( consume_all && skip_whitespace ( curs ) )
            throw MalformedJSON ( XP ( XLOC ) << "Trailing byes in JSON text" ); // test hit

        return val;
    }

    JSON :: Limits :: Limits ()
        : json_string_size ( 4 * 1024 * 1024 )
        , recursion_depth ( 32 )
        , numeral_length ( 256 )
        , string_size ( 64 * 1024 )
        , string_length ( 64 * 1024 )
        , array_elem_count ( 4 * 1024 )
        , object_mbr_count ( 256 )
    {
        have_limits = true;
    }
}


