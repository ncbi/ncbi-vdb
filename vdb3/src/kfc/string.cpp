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

#include <kfc/string.hpp>
#include <kfc/caps.hpp>
#include <kfc/memmgr.hpp>
#include <kfc/except.hpp>
#include <kfc/callstk.hpp>
#include <kfc/rsrc.hpp>
#include <kfc/array.hpp>
#include <kfc/ptr.hpp>

#include <string.h>
#include <ctype.h>

// TEMPORARY
#include <stdio.h>

namespace vdb3
{

    /*------------------------------------------------------------------
     * String
     *  an immutable string object
     */


    // search for a character
    index_t String :: find_first ( UTF32 ch ) const
    {
        FUNC_ENTRY ();

        // if character and we are both ascii
        if ( ch < 128 && mem . size () == ascii_size )
            return mem . find_first ( ( U8 ) ch );

        // UTF-8 operation
        CONST_THROW ( xc_unimplemented_err, "UTF8" );
    }

    index_t String :: find_first ( UTF32 ch, index_t offset ) const
    {
        FUNC_ENTRY ();

        // if index is invalid
        if ( offset < 0 || ( count_t ) offset >= len )
            return -1;

        // if character and we are both ascii
        if ( ch < 128 && mem . size () == ascii_size )
            return mem . find_first ( ( U8 ) ch, offset );

        // UTF-8 operation
        CONST_THROW ( xc_unimplemented_err, "UTF8" );
    }

    index_t String :: find_first ( UTF32 ch, index_t offset, count_t _length ) const
    {
        FUNC_ENTRY ();

        // if index is invalid
        if ( offset < 0 || ( count_t ) offset >= len )
            return -1;

        // limit length
        if ( _length + offset > len )
            _length = len - offset;

        // if character and we are both ascii
        if ( ch < 128 && mem . size () == ascii_size )
            return mem . find_first ( ( U8 ) ch, offset, _length );

        // UTF-8 operation
        CONST_THROW ( xc_unimplemented_err, "UTF8" );
    }

    String String :: substr ( index_t offset ) const
    {
        FUNC_ENTRY ();

        // if index is invalid
        if ( offset < 0 || ( count_t ) offset >= len )
            return String ();

        // new length
        count_t _length = len - offset;

        // detect all-ASCII characters
        if ( mem . size () == ascii_size )
        {
            // can treat index as a byte offset
            Mem m = mem . subrange ( offset, _length );
            return String ( m, _length, _length );
        }

        // UTF-8 operation
        // TBD - may need to have friend access
        CONST_THROW ( xc_unimplemented_err, "UTF8" );
    }

    String String :: substr ( index_t offset, count_t _length ) const
    {
        FUNC_ENTRY ();

        // if index is invalid
        if ( offset < 0 || ( count_t ) offset >= len )
            return String ();

        // limit length
        if ( _length + offset > len )
            _length = len - offset;

        // detect all-ASCII characters
        if ( _length + offset <= ascii_size )
        {
            // can treat index as a byte offset
            Mem m = mem . subrange ( offset, _length );
            return String ( m, _length, _length );
        }

        // UTF-8 operation
        // TBD - may need to have friend access
        CONST_THROW ( xc_unimplemented_err, "UTF8" );
    }

    // split string at separator point
    // removes separator if width is given
    StringPair String :: split ( index_t offset ) const
    {
        FUNC_ENTRY ();

        if ( offset < 0 )
            return StringPair ( * this, String () );

        String left = substr ( 0, offset );
        String right = substr ( offset );

        return StringPair ( left, right );
    }

    StringPair String :: split ( index_t offset, count_t sep_width ) const
    {
        FUNC_ENTRY ();

        if ( offset < 0 )
            return StringPair ( * this, String () );

        String left = substr ( 0, offset );
        String right = substr ( offset + sep_width );
        return StringPair ( left, right );
    }

    String String :: toupper () const
    {
        FUNC_ENTRY ();

        if ( mem . size () == ascii_size )
        {
            StringBuffer copy ( rsrc -> mmgr . alloc ( ascii_size, false ) );
            copy . append ( * this );
            copy . toupper ();
            return copy . to_str ();
        }

        // UTF-8 operation
        // TBD - may need to have friend access
        CONST_THROW ( xc_unimplemented_err, "UTF8" );
    }

    String String :: trim () const
    {
        FUNC_ENTRY ();

        String trimmed = * this;

        if ( len == 0 )
            return trimmed;

        // access the character data
        Ptr < ascii > start = mem;
        Ptr < ascii > p = start;

        // trim from end
        bytes_t bytes = trimmed . mem . size ();
        for ( p += ( I64 ) ( U64 ) bytes - 1; isspace ( * p ); -- p )
        {
            -- trimmed . len;
            if ( trimmed . ascii_size == bytes )
                -- trimmed . ascii_size;
            if ( -- bytes == ( U64 ) 0 )
                return String ();
        }

        // trim from start
        for ( p = start; isspace ( * p ); ++ p )
        {
            -- trimmed . len;
            if ( trimmed . ascii_size != ( U64 ) 0 )
                -- trimmed . ascii_size;
            if ( -- bytes == ( U64 ) 0 )
                return String ();
        }

        index_t offset = p - start;
        Mem m = trimmed . mem . subrange ( offset, bytes );
        trimmed . mem = m;

        return trimmed;
    }

    bool String :: operator == ( const String & s ) const
    {
        FUNC_ENTRY ();

        if ( len == s . len && ascii_size == s . ascii_size )
            return mem == s . mem;

        return false;
    }

    bool String :: operator != ( const String & s ) const
    {
        FUNC_ENTRY ();

        if ( len == s . len && ascii_size == s . ascii_size )
            return mem != s . mem;

        return true;
    }

    String :: String ()
        : ascii_size ( 0 )
        , len ( 0 )
    {
    }

    String :: ~ String ()
    {
        ascii_size = 0;
        len = 0;
    }

    String :: String ( const String & s )
        : mem ( s . mem )
        , ascii_size ( s . ascii_size )
        , len ( s . len )
    {
    }

    void String :: operator = ( const String & s )
    {
        mem = s . mem;
        ascii_size = s . ascii_size;
        len = s . len;
    }

    String :: String ( const String & s, caps_t reduce )
        : mem ( s . mem, reduce )
        , ascii_size ( s . ascii_size )
        , len ( s . len )
    {
    }

    String :: String ( const Mem & m, const bytes_t & ascii_sz, count_t _len )
        : mem ( m, CAP_WRITE | CAP_RESIZE )
        , ascii_size ( ascii_sz )
        , len ( _len )
    {
        // ascii_sz <= _len <= sz
        assert ( ( ascii_sz <= _len ) && ( _len <= m . size () ) );
        // ascii_sz == m . size () || ( ascii_sz < _len < m . size () )
        assert ( ( ascii_sz == m . size () ) || ( ascii_sz < _len && _len < m . size () ) );
    }


    /*------------------------------------------------------------------
     * ConstString
     *  create a String from constant data
     */

    ConstString :: ConstString ( const char * text, size_t bytes )
        : String ( rsrc -> mmgr . make_const ( ( const void * ) text, bytes ), bytes, bytes )
    {
    }


    /*------------------------------------------------------------------
     * StringBuffer
     *  an editible string object
     */

    String StringBuffer :: to_str () const
    {
        FUNC_ENTRY ();

        Mem m = buffer . subrange ( 0, bytes );
        return String ( m, ascii_size, len );
    }

    index_t StringBuffer :: find_first ( UTF32 ch ) const
    {
        FUNC_ENTRY ();

        // if finding an ascii character ( i.e. byte )
        if ( ch < 128 )
        {
            // if string is 100% ascii, then byte offsets == char idx
            if ( ascii_size == bytes )
                return buffer . find_first ( ( U8 ) ch );
        }

        // UTF-8 operation
        // TBD - may need to have friend access
        CONST_THROW ( xc_unimplemented_err, "UTF8" );
    }

    index_t StringBuffer :: find_first ( UTF32 ch, index_t starting_pos ) const
    {
        FUNC_ENTRY ();

        // if finding an ascii character ( i.e. byte )
        if ( ch < 128 )
        {
            // if string is 100% ascii, then byte offsets == char idx
            if ( ascii_size == bytes )
                return buffer . find_first ( ( U8 ) ch, starting_pos );
        }

        // UTF-8 operation
        // TBD - may need to have friend access
        CONST_THROW ( xc_unimplemented_err, "UTF8" );
    }

    StringBuffer & StringBuffer :: assign ( const String & s )
    {
        FUNC_ENTRY ();

        // become empty
        bytes = 0;
        ascii_size = 0;
        len = 0;

        // append
        return append ( s );
    }

    StringBuffer & StringBuffer :: assign ( const StringBuffer & s )
    {
        FUNC_ENTRY ();

        // become empty
        bytes = 0;
        ascii_size = 0;
        len = 0;

        // append
        return append ( s );
    }

    StringBuffer & StringBuffer :: assign ( const char * fmt, ... )
    {
        FUNC_ENTRY ();

        // become empty
        bytes = 0;
        ascii_size = 0;
        len = 0;

        // append
        va_list args;
        va_start ( args, fmt );
        vappend ( fmt, args );
        va_end ( args );

        return * this;
    }

    StringBuffer & StringBuffer :: vassign ( const char * fmt, va_list args )
    {
        FUNC_ENTRY ();

        // become empty
        bytes = 0;
        ascii_size = 0;
        len = 0;

        // append
        return vappend ( fmt, args );
    }

    StringBuffer & StringBuffer :: append ( const String & s )
    {
        FUNC_ENTRY ();

        bytes_t amount = s . size ();
        resize ( bytes + amount );

        bytes += buffer . copy ( amount, bytes, s . mem, 0 );
        if ( ascii_size == len )
            ascii_size += s . ascii_size;
        len += s . len;

        return * this;
    }

    StringBuffer & StringBuffer :: append ( const StringBuffer & s )
    {
        FUNC_ENTRY ();

        size_t amount = s . size ();
        resize ( bytes + amount );

        bytes += buffer . copy ( amount, bytes, s . buffer, 0 );
        if ( ascii_size == len )
            ascii_size += s . ascii_size;
        len += s . len;

        return * this;
    }

    StringBuffer & StringBuffer :: append ( const char * fmt, ... )
    {
        FUNC_ENTRY ();

        va_list args;
        va_start ( args, fmt );
        vappend ( fmt, args );
        va_end ( args );

        return * this;
    }

    StringBuffer & StringBuffer :: vappend ( const char * fmt, va_list args )
    {
        // no harm, no foul
        if ( fmt == 0 )
            return * this;

        FUNC_ENTRY ();

        // loop over format
        for ( size_t i = 0;; ++ i )
        {
            // search for '%'
            size_t start;
            bool end = true;
            for ( start = i; fmt [ i ] != 0; ++ i )
            {
                if ( fmt [ i ] == '%' )
                {
                    end = false;
                    break;
                }
            }

            // append whatever literal string was given
            if ( start < i )
            {
                ConstString s ( & fmt [ start ], i - start );
                append ( s );
            }

            // detect end of fmt string
            if ( end )
                break;

            // handle escaped percent sign
            if ( fmt [ ++ i ] == '%' )
            {
                append ( '%' );
            }
            else if ( isalpha ( fmt [ i ] ) )
            {
                // handle simple substitution printf item
                i = fappend_simple ( fmt, args, i );
            }
            else
            {
                // handle longer format printf item
                i = fappend_long ( fmt, args, i );
            }
        }

        return * this;
    }

    StringBuffer & StringBuffer :: append ( UTF32 ch )
    {
        return append ( ch, 1 );
    }

    StringBuffer & StringBuffer :: append ( UTF32 ch, count_t repeat )
    {
        FUNC_ENTRY ();

        if ( ch >= 128 )
        {
            // UTF-8 operation
            // TBD - may need to have friend access
            CONST_THROW ( xc_unimplemented_err, "UTF8" );
        }
        else if ( ch != 0 )
        {
            resize ( bytes + repeat );
            bytes += buffer . fill ( repeat, ( I64 ) ( U64 ) bytes, ( U8 ) ch );
            if ( ascii_size == len )
                ascii_size += repeat;
            len += repeat;
        }

        return * this;
    }

    void StringBuffer :: reset ()
    {
        bytes = 0;
        ascii_size = 0;
        len = 0;
    }

    void StringBuffer :: trim ()
    {
        if ( bytes == ( U64 ) 0 )
            return;

        FUNC_ENTRY ();

        // access the character data
        Ptr < ascii > start = buffer;
        Ptr < ascii > p = start;

        // trim from end
        for ( p += ( I64 ) ( U64 ) bytes - 1; isspace ( * p ); -- p )
        {
            -- len;
            if ( ascii_size == bytes )
                -- ascii_size;
            if ( -- bytes == ( U64 ) 0 )
                return;
        }

        // trim from start
        for ( p = start; isspace ( * p ); ++ p )
        {
            -- len;
            if ( ascii_size != ( U64 ) 0 )
                -- ascii_size;
            if ( -- bytes == ( U64 ) 0 )
                return;
        }

        // if start was moved
        index_t offset = p - start;
        if ( offset > ( I64 ) 0 )
        {
            {
                // create offset to start
                Mem m = buffer . subrange ( offset );
                buffer = m;
            }

            // resize to force discard
            buffer . resize ( ( ( U64 ) bytes + 4095 ) & ~ ( U64 ) 4095, true );
        }
    }

    bool StringBuffer :: trim_eoln ()
    {
        FUNC_ENTRY ();

        bool trimmed = false;

        if ( bytes != ( U64 ) 0 )
        {
            // access character data
            Ptr < ascii > p = buffer;

            // must end with '\n'
            if ( p [ ( U64 ) bytes - 1 ] == '\n' )
            {
                // going to trim
                trimmed = true;

                // detect CRLF
                if ( ( U64 ) bytes >= 2 && p [ ( U64 ) bytes - 2 ] == '\r' )
                {
                    len -= ( U64 ) 2;
                    if ( ascii_size == bytes )
                        ascii_size -= ( U64 ) 2;
                    bytes -= ( U64 ) 2;
                }
                else
                {
                    len -= ( U64 ) 1;
                    if ( ascii_size == bytes )
                        ascii_size -= ( U64 ) 1;
                    bytes -= ( U64 ) 1;
                }
            }
        }

        return trimmed;
    }

    void StringBuffer :: toupper ()
    {
        FUNC_ENTRY ();

        // UTF-8 operation
        if ( bytes != ascii_size )
            CONST_THROW ( xc_unimplemented_err, "UTF8" );

        Array < char > a = buffer;
        for ( count_t i = 0; i < len; ++ i )
        {
            char ch = a [ i ];
            a [ i ] = :: toupper ( ch );
        }
    }

    StringBuffer :: StringBuffer ()
        : buffer ( rsrc -> mmgr . alloc ( 0, false ) )
        , bytes ( 0 )
        , ascii_size ( 0 )
        , len ( 0 )
    {
    }

    StringBuffer :: ~ StringBuffer ()
    {
        reset ();
    }

    StringBuffer :: StringBuffer ( const Mem & m )
        : buffer ( m )
        , bytes ( 0 )
        , ascii_size ( 0 )
        , len ( 0 )
    {
    }

    StringBuffer :: StringBuffer ( const char * fmt, ... )
        : buffer ( rsrc -> mmgr . alloc ( 0, false ) )
        , bytes ( 0 )
        , ascii_size ( 0 )
        , len ( 0 )
    {
        FUNC_ENTRY ();

        va_list args;
        va_start ( args, fmt );
        vappend ( fmt, args );
        va_end ( args );
    }

    StringBuffer :: StringBuffer ( const char * fmt, va_list args )
        : buffer ( rsrc -> mmgr . alloc ( 0, false ) )
        , bytes ( 0 )
        , ascii_size ( 0 )
        , len ( 0 )
    {
        FUNC_ENTRY ();

        vappend ( fmt, args );
    }

    size_t StringBuffer :: fappend_simple ( const char * fmt, va_list args, size_t i )
    {
        FUNC_ENTRY ();

        I64 i64;
        U64 u64;
        //F64 f64;
        UTF32 ch;
        U32 j, base;
        bool neg = false;
        const String * str;
        ascii num [ 256 ];

        const ascii * chmap = "0123456789abcdefghijklmnopqrstuvwxyz";

        // simple argument, no formatting
        switch ( fmt [ i ] )
        {
        case 'u':
            u64 = va_arg ( args, U32 );

        cvt_decimal_int:
            base = 10;

        cvt_integer:
            j = sizeof num;
            if ( u64 == 0 )
                num [ -- j ] = '0';
            else do
            {
                num [ -- j ] = chmap [ u64 % base ];
                u64 /= base;
            }
            while ( u64 != 0 );

            if ( neg )
                num [ -- j ] = '-';

            {
                ConstString nstr ( & num [ j ], sizeof num - j );
                append ( nstr );
            }
            break;

        case 'l':
            switch ( fmt [ ++ i ] )
            {
            case 'u':
                u64 = va_arg ( args, U64 );
                goto cvt_decimal_int;
                
            case 'd':
            case 'i':
                i64 = va_arg ( args, I64 );
                if ( i64 >= 0 )
                {
                    u64 = i64;
                    goto cvt_decimal_int;
                }
                u64 = -i64;
                neg = true;
                goto cvt_decimal_int;

            case 'X':
                chmap = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            case 'x':
                u64 = va_arg ( args, U64 );
                base = 16;
                goto cvt_integer;

            case 'o':
                u64 = va_arg ( args, U64 );
                base = 8;
                goto cvt_integer;

            case 'b':
                u64 = va_arg ( args, U64 );
                base = 2;
                goto cvt_integer;

            default:
                THROW ( xc_bad_fmt_err, "expected one of [udiXxob] after length modifier 'l' but found '%c'", fmt [ i ] );
            }
            break;

        case 'z':
            switch ( fmt [ ++ i ] )
            {
            case 'u':
                u64 = va_arg ( args, size_t );
                goto cvt_decimal_int;

            case 'X':
                chmap = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            case 'x':
                u64 = va_arg ( args, size_t );
                base = 16;
                goto cvt_integer;

            case 'o':
                u64 = va_arg ( args, size_t );
                base = 8;
                goto cvt_integer;

            case 'b':
                u64 = va_arg ( args, size_t );
                base = 2;
                goto cvt_integer;

            default:
                THROW ( xc_bad_fmt_err, "expected one of [uXxob] after length modifier 'z' but found '%c'", fmt [ i ] );
            }
            break;

        case 'd':
        case 'i':
            i64 = va_arg ( args, I32 );
            if ( i64 >= 0 )
            {
                u64 = i64;
                goto cvt_decimal_int;
            }
            u64 = -i64;
            neg = true;
            goto cvt_decimal_int;

        case 'X':
            chmap = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        case 'x':
            u64 = va_arg ( args, U32 );
            base = 16;
            goto cvt_integer;

        case 'o':
            u64 = va_arg ( args, U32 );
            base = 8;
            goto cvt_integer;
            
        case 'b':
            u64 = va_arg ( args, U32 );
            base = 2;
            goto cvt_integer;

        case 's':
            str = va_arg ( args, const String * );
            if ( str == 0 )
                append ( CONST_STRING ( "(null)" ) );
            else
                append ( * str );
            break;

        case 'c':
            ch = va_arg ( args, UTF32 );
            append ( ch );
            break;

        case 'f':
        case 'e':
        case 'g':
        {
            // TBD
            char fmt2 [ 2 ];
            fmt2 [ 0 ] = fmt [ i ];
            fmt2 [ 1 ] = 0;
            j = vsnprintf ( num, sizeof num, fmt2, args );
            ConstString fstr ( num, j );
            append ( fstr );
            break;
        }

        case 'h':
            switch ( fmt [ ++ i ] )
            {
            case 'u':
                u64 = ( U16 ) va_arg ( args, U32 );
                goto cvt_decimal_int;

            case 'd':
            case 'i':
                i64 = ( I16 ) va_arg ( args, I32 );
                if ( i64 >= 0 )
                {
                    u64 = i64;
                    goto cvt_decimal_int;
                }
                u64 = -i64;
                neg = true;
                goto cvt_decimal_int;
                
            case 'X':
                chmap = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            case 'x':
                u64 = ( U16 ) va_arg ( args, U32 );
                base = 16;
                goto cvt_integer;

            case 'o':
                u64 = ( U16 ) va_arg ( args, U32 );
                base = 8;
                goto cvt_integer;

            case 'b':
                u64 = ( U16 ) va_arg ( args, U32 );
                base = 2;
                goto cvt_integer;

            default:
                THROW ( xc_bad_fmt_err, "expected one of [udiXxob] after length modifier 'h' but found '%c'", fmt [ i ] );
            }
            break;

        case 't':
            switch ( fmt [ ++ i ] )
            {
            case 'u':
                u64 = ( U8 ) va_arg ( args, U32 );
                goto cvt_decimal_int;
                
            case 'd':
            case 'i':
                i64 = ( I8 ) va_arg ( args, I32 );
                if ( i64 >= 0 )
                {
                    u64 = i64;
                    goto cvt_decimal_int;
                }
                u64 = -i64;
                neg = true;
                goto cvt_decimal_int;

            case 'X':
                chmap = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            case 'x':
                u64 = ( U8 ) va_arg ( args, U32 );
                base = 16;
                goto cvt_integer;

            case 'o':
                u64 = ( U8 ) va_arg ( args, U32 );
                base = 8;
                goto cvt_integer;

            case 'b':
                u64 = ( U8 ) va_arg ( args, U32 );
                base = 2;
                goto cvt_integer;
                
            default:
                THROW ( xc_bad_fmt_err, "expected one of [udiXxob] after length modifier 't' but found '%c'", fmt [ i ] );
            }
            break;
        }

        return i;
    }

    size_t StringBuffer :: fappend_long ( const char * fmt, va_list args, size_t i )
    {
        FUNC_ENTRY ();

        I64 i64;
        U64 u64;
        F64 f64;
        UTF32 ch;
        U32 j, k, base;
        const String * str;
        ascii num [ 256 ];

        const ascii * chmap = "0123456789abcdefghijklmnopqrstuvwxyz";

        // perform a full format
        ascii use_sign = 0;
        ascii padding = ' ';
        U64 digits = 0;
        bool left_align = false;
        bool comma_separate = false;
        bool alternate = false;

        // FLAGS
        while ( 1 )
        {
            switch ( fmt [ i ] )
            {
            case '-':
                left_align = true;
                padding = ' ';
                ++ i;
                continue;
            case '+':
                use_sign = '+';
                ++ i;
                continue;
            case ' ':
                if ( use_sign != '+' )
                    use_sign = ' ';
                ++ i;
                continue;
            case '0':
                if ( ! left_align )
                    padding = '0';
                ++ i;
                continue;
            case ',':
            case '\'':
                comma_separate = true;
                ++ i;
                continue;
            case '#':
                alternate = true;
                ++ i;
                continue;
            }

            break;
        }

        // zero padding is weird if comma-separating
        if ( comma_separate )
            padding = ' ';

        // FIELD WIDTH
        U64 min_field_width = 0;
        if ( fmt [ i ] == '*' )
        {
            min_field_width = va_arg ( args, size_t );
            ++ i;
        }
        else for ( ; isdigit ( fmt [ i ] ); ++ i )
        {
            min_field_width *= 10;
            min_field_width += fmt [ i ] - '0';
        }

        // PRECISION
        U64 precision = 0;
        bool have_precision = false;
        if ( fmt [ i ] == '.' )
        {
            padding = ' ';
            have_precision = true;
            if ( fmt [ i ] == '*' )
            {
                precision = va_arg ( args, size_t );
                ++ i;
            }
            else for ( ; isdigit ( fmt [ i ] ); ++ i )
            {
                precision *= 10;
                precision += fmt [ i ] - '0';
            }
        }

        // STORAGE SIZE
        U8 arg_size = sizeof ( U32 );
        bool date_time_zone = false;
        switch ( fmt [ i ] )
        {
        case 't':
            arg_size = sizeof ( U8 );
            ++ i;
            break;
        case 'h':
            arg_size = sizeof ( U16 );
            ++ i;
            break;
        case 'l':
            arg_size = sizeof ( U64 );
            ++ i;
            break;
        case 'z':
            date_time_zone = true;
            arg_size = sizeof ( size_t );
            ++ i;
            break;
        }

        // ARGUMENT
        switch ( fmt [ i ] )
        {
        case 'u':
        case 'X':
        case 'x':
        case 'o':
        case 'b':
            use_sign = 0;
            switch ( arg_size )
            {
            case 1:
                u64 = ( U8 ) va_arg ( args, U32 );
                break;
            case 2:
                u64 = ( U16 ) va_arg ( args, U32 );
                break;
            case 4:
                u64 = va_arg ( args, U32 );
                break;
            case 8:
                u64 = va_arg ( args, U64 );
                break;
            }
            break;

        case 'd':
        case 'i':
            switch ( arg_size )
            {
            case 1:
                i64 = ( I8 ) va_arg ( args, I32 );
                break;
            case 2:
                i64 = ( I16 ) va_arg ( args, I32 );
                break;
            case 4:
                i64 = va_arg ( args, I32 );
                break;
            case 8:
                i64 = va_arg ( args, I64 );
                break;
            }

            if ( i64 < 0 )
            {
                u64 = - i64;
                use_sign = '-';
            }
            else
            {
                if ( i64 == 0 )
                    use_sign = 0;
                u64 = i64;
            }
            break;
        }

        // FORMAT
        U32 comma_mag = 3;
        ascii comma = ',';
        //bool floating_comma = false;
        U64 field_width, total_field_width;
        switch ( fmt [ i ] )
        {
        case 'u':
        case 'd':
        case 'i':
            base = 10;

        l_cvt_integer:

            j = sizeof num;
            if ( u64 == 0 )
            {
                num [ -- j ] = '0';
                digits = 1;
            }
            else if ( comma_separate )
            {
                for ( k = 0, digits = 0; u64 != 0; ++ digits, ++ k )
                {
                    if ( k == comma_mag )
                    {
                        num [ -- j ] = comma;
                        k = 0;
                    }
                    num [ -- j ] = chmap [ u64 % base ];
                    u64 /= base;
                }
                for ( ; digits < precision && j != 0; ++ digits, ++ k )
                {
                    if ( k == comma_mag )
                    {
                        num [ -- j ] = comma;
                        k = 0;
                        if ( j == 0 )
                            break;
                    }
                    num [ -- j ] = 0;
                }
            }
            else for ( digits = 0; u64 != 0; ++ digits )
            {
                num [ -- j ] = chmap [ u64 % base ];
                u64 /= base;
            }

            field_width = sizeof num - j;

        insert_num:

            total_field_width = field_width + ( use_sign != 0 );

            if ( left_align )
            {
                if ( use_sign != 0 )
                    append ( use_sign );

                append ( ConstString ( & num [ j ], field_width ) );
 
                if ( total_field_width < min_field_width )
                    append ( ' ', min_field_width - total_field_width );
            }
            else
            {
                // check sign and padding
                if ( use_sign != 0 && padding == '0' )
                {
                    append ( use_sign );
                    use_sign = 0;
                }

                // apply padding
                if ( total_field_width < min_field_width )
                    append ( padding, min_field_width - total_field_width );

                // sign again
                if ( use_sign != 0 )
                    append ( use_sign );

                // now the numeral
                append ( ConstString ( & num [ j ], field_width ) );
            }

            break;


        case 'X':
            chmap = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        case 'x':
            base = 16;
            comma_mag = 4;
            goto l_cvt_integer;
        case 'o':
            base = 8;
            goto l_cvt_integer;
        case 'b':
            base = 2;
            comma_mag = 8;
            goto l_cvt_integer;

        case 's':
        {
            String null_str = CONST_STRING ( "(null)" );
            str = va_arg ( args, const String * );
            if ( str == 0 )
                str = & null_str;

            field_width = str -> length ();
            if ( field_width >= min_field_width )
            {
                append ( * str );
            }
            else if ( left_align )
            {
                append ( * str );
                append ( ' ', min_field_width - field_width );
            }
            else
            {
                append ( ' ', min_field_width - field_width );
                append ( * str );
            }

            break;
        }

        case 'c':
            ch = va_arg ( args, UTF32 );

            field_width = 1;
            if ( field_width >= min_field_width )
            {
                append ( ch );
            }
            else if ( left_align )
            {
                append ( ch );
                append ( ' ', min_field_width - field_width );
            }
            else
            {
                append ( ' ', min_field_width - field_width );
                append ( ch );
            }
            break;

        case 'f':
        case 'e':
        case 'g':
        {
            f64 = va_arg ( args, F64 );
            if ( f64 < 0 )
            {
                use_sign = '-';
                f64 = - f64;
            }

            char fmt2 [ 16 ];
            j = 0;
            if ( alternate )
                fmt2 [ j ++ ] = '#';
            if ( min_field_width != 0 )
                j += snprintf ( & fmt2 [ j ], sizeof fmt2 - j, "%u", ( U32 ) min_field_width );
            if ( have_precision )
            {
                fmt2 [ j ++ ] = '.';
                if ( precision != 0 )
                    j += snprintf ( & fmt2 [ j ], sizeof fmt2 - j, "%u", ( U32 ) precision );
            }
            fmt2 [ j ++ ] = fmt [ i ];
            fmt2 [ j ] = 0;

            field_width = snprintf ( num, sizeof num, fmt2, f64 );
            j = 0;
            goto insert_num;
        }

        default:
            // TBD - extract UTF32 character
            THROW ( xc_bad_fmt_err, "expected one of [udiXxobscfeg] but found '%c'", fmt [ i ] );
        }

        return i;
    }

    void StringBuffer :: resize ( const bytes_t & new_size )
    {
        if ( new_size > buffer . size () )
        {
            FUNC_ENTRY ();

            U64 rounded_size = ( ( U64 ) new_size + 4095 ) & ~ ( U64 ) 4095;
            buffer . resize ( rounded_size, true );
        }
    }

    /*
     * NULTermString
     *  create a string that is NUL-terminated,
     *  and sports a cast to const char *
     *  for use with native OS
     */

    // supports nasty cast to NUL-terminated string
    NULTermString :: operator const char * () const
    {
        // attempt access to the raw memory
        const Ptr < char > p = rgn;

        // the memory block must not be empty
        // or addressing the array will fail
        if ( len != 0 )
        {
            assert ( rgn . size () != ( U64 ) 0 );
            return & p [ 0 ];
        }

        // for empty strings, just return an empty C string
        return "";
    }

    NULTermString :: NULTermString ( const String & str )
        : String ( StringBuffer ( "%sz", & str ) . toString () )
    {
        // the current value is a copy of input
        // with an additional character added - space for NUL
        bytes_t cur_size = rgn . size ();
        if ( ascii_size == cur_size )
            -- ascii_size;
        -- len;
        -- cur_size;

        // change last character to NUL
        // access mem as a read-only array
        // because capabilities have been reduced
        Ptr < ascii > ptr = rgn;
        const ascii * p = & ptr [ cur_size ];

        // having stolen a pointer,
        // use C to hack our own system
        * ( ascii * ) p = 0;

        // drop size of memory without losing
        // the guaranteed trailing NUL byte
        Region r = rgn . subRegion ( 0, cur_size );

        // forget the initial copy
        rgn = r;
    }


    void NULTermString :: operator = ( const String & str )
    {
        // copy the string with an additional character
        {
            StringBuffer buf ( "%sz", & str );
            String :: operator = ( buf . toString () );
        }

        // see constructor above for comments
        bytes_t cur_size = rgn . size ();
        if ( ascii_size == cur_size )
            -- ascii_size;
        -- len;
        -- cur_size;

        Ptr < ascii > a = rgn;
        const ascii * p = & a [ cur_size ];
        * ( U8 * ) p = 0;

        Region r = rgn . subRegion ( 0, cur_size );
        rgn = r;
    }

}
