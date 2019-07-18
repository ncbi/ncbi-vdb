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

#include <klib/extern.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <va_copy.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <errno.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * String
 *  pseudo-intrinsic string
 */

/* StringCopy
 *  allocates a copy of a string
 */
LIB_EXPORT rc_t CC StringCopy ( const String **cpy, const String *str )
{
    if ( cpy != NULL )
    {
        if ( str != NULL )
        {
            size_t size = str -> size;
            String *s = malloc ( sizeof * s + str -> size + 1 );
            if ( s != NULL )
            {
                char *addr = ( char* ) ( s + 1 );
                StringInit ( s, addr, size, str -> len );
                memmove ( addr, str -> addr, size );
                addr [ size ] = 0;
                * cpy = s;
                return 0;
            }

            * cpy = NULL;
            return RC ( rcText, rcString, rcCopying, rcMemory, rcInsufficient );
        }

        * cpy = NULL;
    }
    return RC ( rcText, rcString, rcCopying, rcParam, rcNull );
}

/* StringConcat
 *  concatenate one string onto another
 */
LIB_EXPORT rc_t CC StringConcat ( const String **cat, const String *a, const String *b )
{
    if ( cat != NULL )
    {
        if ( a != NULL && b != NULL )
        {
            size_t size = a -> size + b -> size;
            String *s = malloc ( sizeof * s + size + 1 );
            if ( s != NULL )
            {
                char *addr = ( char* ) ( s + 1 );
                StringInit ( s, addr, size, a -> len + b -> len );
                memmove ( addr, a -> addr, a -> size );
                memmove ( & addr [ a -> size ], b -> addr, b -> size );
                addr [ size ] = 0;
                * cat = s;
                return 0;
            }

            * cat = NULL;
            return RC ( rcText, rcString, rcConcatenating, rcMemory, rcInsufficient );
        }

        * cat = NULL;
    }
    return RC ( rcText, rcString, rcConcatenating, rcParam, rcNull );
}

/* StringSubstr
 *  creates a substring of an existing one
 *  note that the substring is always a non-allocated copy
 *  and is dependent upon the lifetime of its source
 *
 *  returns "sub" if "idx" was valid
 *  or NULL otherwise
 *
 *  "len" may be 0 to indicate infinite length
 *  or may extend beyond end of source string.
 */
LIB_EXPORT String * CC StringSubstr ( const String *str, String *sub,
    uint32_t idx, uint32_t len )
{
    if ( str != NULL && sub != NULL && idx < str -> len )
    {
        const char *addr = string_idx ( str -> addr, str -> size, idx );
        if ( addr != NULL )
        {
            sub -> size = str -> size - ( size_t ) ( addr - str -> addr );
            sub -> len = str -> len - idx;
            sub -> addr = addr;
            if ( len > 0 && len < sub -> len )
            {
                const char *end = string_idx ( sub -> addr, sub -> size, len );
                if ( end != NULL )
                {
                    sub -> size = ( size_t ) ( end - sub -> addr );
                    sub -> len = len;
                }
            }
            return sub;
        }
    }
    return NULL;
}

/* StringTrim
 *  trims ascii white-space from both ends
 *  returns trimmed string in "trimmed"
 */
LIB_EXPORT String * CC StringTrim ( const String * str, String * trimmed )
{
    if ( trimmed != NULL )
    {
        if ( str == NULL )
            CONST_STRING ( trimmed, "" );
        else
        {
            const char * addr = str -> addr;
            size_t i, end, sz = str -> size;
            uint32_t len = str -> len;

            for ( end = sz; end > 0; -- end )
            {
                if ( ! isspace ( addr [ end - 1 ] ) )
                    break;
            }

            for ( i = 0; i < end; ++ i )
            {
                if ( ! isspace ( addr [ i ] ) )
                    break;
            }

            StringInit ( trimmed, & addr [ i ], end - i, len - ( i + sz - end ) );
        }
    }

    return trimmed;
}

/* StringHead
 *  access the first character
 *
 *  this is an efficient enough function to be included.
 *  the generic functions of accessing characters by index
 *  are apt to be extremely inefficient with UTF-8, and
 *  as such are not included.
 *
 *  returns EINVAL if the character is bad,
 *  or ENODATA if the string is empty
 */
LIB_EXPORT rc_t CC StringHead ( const String *str, uint32_t *ch )
{
    rc_t rc;
    if ( ch == NULL )
        rc = RC ( rcText, rcString, rcAccessing, rcParam, rcNull );
    else
    {
        if ( str == NULL )
            rc = RC ( rcText, rcString, rcAccessing, rcParam, rcNull );
        else
        {
            if ( str -> len == 0 )
                rc = RC ( rcText, rcString, rcAccessing, rcString, rcEmpty );
            else
            {
                int ch_len = utf8_utf32 ( ch, str -> addr, str -> addr + str -> size );
                if ( ch_len > 0 )
                    return 0;

                rc = RC ( rcText, rcString, rcAccessing, rcChar, rcInvalid );
            }
        }

        * ch = 0;
    }

    return rc;
}

/* StringPopHead
 *  remove and return the first character
 *
 *  returns EINVAL if the character is bad,
 *  or ENODATA if the string is empty
 */
LIB_EXPORT rc_t CC StringPopHead ( String *str, uint32_t *ch )
{
    rc_t rc;
    if ( ch == NULL )
        rc = RC ( rcText, rcChar, rcRemoving, rcParam, rcNull );
    else
    {
        if ( str == NULL )
            rc = RC ( rcText, rcChar, rcRemoving, rcParam, rcNull );
        else
        {
            if ( str -> len == 0 )
                rc = SILENT_RC ( rcText, rcChar, rcRemoving, rcString, rcEmpty );
            else
            {
                int ch_len = utf8_utf32 ( ch, str -> addr, str -> addr + str -> size );
                if ( ch_len > 0 )
                {
                    str -> addr += ch_len;
                    str -> size -= ch_len;
                    str -> len -= 1;
                    return 0;
                }

                rc = RC ( rcText, rcChar, rcRemoving, rcChar, rcInvalid );
            }
        }

        * ch = 0;
    }

    return rc;
}

/* StringCompare
 *  compare strings for relative ordering
 */
LIB_EXPORT int CC StringCompare ( const String *a, const String *b )
{
    int diff;
    uint32_t len;

    if ( a == b )
        return 0;
    if ( a == NULL )
        return -1;
    if ( b == NULL )
        return 1;

    len = a -> len;
    if ( b -> len < len )
        len = b -> len;

    diff = string_cmp ( a -> addr, a -> size, b -> addr, b -> size, len );
    if ( diff == 0 )
        diff = ( int ) a -> len - ( int ) b -> len;
    return diff;
}

/* StringCaseEqual
 *  compare strings for case-insensitive equality
 */
LIB_EXPORT bool CC StringCaseEqual ( const String *a, const String *b )
{
    uint32_t len;

    if ( a == b )
        return true;
    if ( a == NULL || b == NULL )
        return false;

    len = a -> len;
    if ( b -> len != len )
        return false;

    return strcase_cmp ( a -> addr, a -> size, b -> addr, b -> size, len ) == 0;
}

/* StringCaseCompare
 *  compare strings for relative case-insensitive ordering
 */
LIB_EXPORT int CC StringCaseCompare ( const String *a, const String *b )
{
    int diff;
    uint32_t len;

    if ( a == b )
        return 0;
    if ( a == NULL )
        return -1;
    if ( b == NULL )
        return 1;

    len = a -> len;
    if ( b -> len < len )
        len = b -> len;

    diff = strcase_cmp ( a -> addr, a -> size, b -> addr, b -> size, len );
    if ( diff == 0 )
        diff = ( int ) a -> len - ( int ) b -> len;
    return diff;
}

/* StringOrder
 *  compares strings as quickly as possible for
 *  deterministic ordering: first by length, then
 *  by binary (byte-wise) content.
 *
 *  performs more quickly than StringCompare for cases
 *  where only deterministic ordering is needed ( e.g. symbol table ).
 */
LIB_EXPORT int64_t CC StringOrder ( const String *a, const String *b )
{
    if ( a == b )
        return 0;
    if ( a == NULL )
        return -1;
    if ( b == NULL )
        return 1;

    if ( a -> size < b -> size )
        return -1;
    if ( a -> size > b -> size )
        return 1;
    return memcmp ( a -> addr, b -> addr, a -> size );
}

/* StringOrderNoNullCheck
 *  avoids tests for NULL strings
 */
LIB_EXPORT int64_t CC StringOrderNoNullCheck ( const String *a, const String *b )
{
    if ( a -> size < b -> size )
        return -1;
    if ( a -> size > b -> size )
        return 1;
    return memcmp ( a -> addr, b -> addr, a -> size );
}

/* StringMatch
 *  creates a substring of "a" in "match"
 *  for all of the sequential matching characters between "a" and "b"
 *  starting from character [ 0 ].
 *
 *  returns the number of characters that match.
 */
LIB_EXPORT uint32_t CC StringMatch ( String *match, const String *a, const String *b )
{
    if ( a == NULL || b == NULL )
    {
        CONST_STRING ( match, "" );
        return 0;
    }

    if ( a == b )
    {
        * match = * a;
        return a -> len;
    }

    if ( match == NULL )
    {
        return string_match ( a -> addr, a -> size,
            b -> addr, b -> size, (uint32_t)b -> size, NULL );
    }

    match -> addr = a -> addr;
    return match -> len = string_match ( a -> addr, a -> size,
        b -> addr, b -> size, (uint32_t)b -> size, & match -> size );
}

/* StringMatchExtend
 *  extends a substring of "a" in "match"
 *  for all of the sequential matching characters between "a" and "b"
 *  starting from character [ match -> len ].
 *
 *  returns the number of matching characters that were extended.
 */
LIB_EXPORT uint32_t CC StringMatchExtend ( String *match, const String *a, const String *b )
{
    size_t msize;
    uint32_t len;

    assert ( match != NULL );
    if ( match -> len == 0 )
        return StringMatch ( match, a, b );

    assert ( a != NULL );
    assert ( match -> addr == a -> addr );
    assert ( match -> len <= a -> len );

    if ( b == NULL || match -> len == a -> len || match -> len >= b -> len )
        return 0;

    msize = match -> size;
    len = string_match ( a -> addr + msize, a -> size - msize,
        b -> addr + msize, b -> size - msize, (uint32_t)b -> size, & msize );

    match -> len += len;
    match -> size += msize;
    return len;
}

/* StringCopyUTF...
 *  creates a String from UTF16 or UTF32 UNICODE input
 *  wchar_t is one or the other, depending upon OS and compiler.
 */
LIB_EXPORT rc_t CC StringCopyUTF16 ( const String **cpy, const uint16_t *text, size_t bytes )
{
    if ( cpy != NULL )
    {
        if ( text != NULL || bytes == 0 )
        {
            size_t size;
            uint32_t len = utf16_cvt_string_len ( text, bytes, & size );
            String *str = ( String* ) malloc ( sizeof * str + 1 + size );
            if ( ( * cpy = str ) == NULL )
                return RC ( rcText, rcString, rcCopying, rcMemory, rcInsufficient );
            StringInit ( str, ( char* ) ( str + 1 ), size, len );
            str -> size = utf16_cvt_string_copy ( ( char* ) str -> addr, size, text, bytes );
            return 0;
        }

        * cpy = NULL;
    }
    return RC ( rcText, rcString, rcCopying, rcParam, rcNull );
}

LIB_EXPORT rc_t CC StringCopyUTF32 ( const String **cpy, const uint32_t *text, size_t bytes )
{
    if ( cpy != NULL )
    {
        if ( text != NULL || bytes == 0 )
        {
            size_t size;
            uint32_t len = utf32_cvt_string_len ( text, bytes, & size );
            String *str = ( String* ) malloc ( sizeof * str + 1 + size );
            if ( ( * cpy = str ) == NULL )
                return RC ( rcText, rcString, rcCopying, rcMemory, rcInsufficient );
            StringInit ( str, ( char* ) ( str + 1 ), size, len );
            str -> size = utf32_cvt_string_copy ( ( char* ) str -> addr, size, text, bytes );
            return 0;
        }

        * cpy = NULL;
    }
    return RC ( rcText, rcString, rcCopying, rcParam, rcNull );
}

/* StringWhack
 *  deallocates a string
 *  ignores strings not allocated by this library
 */
LIB_EXPORT void CC StringWhack ( const String* self )
{
    free ( ( void* ) self );
}


/* StringToInt
 *  simple string conversion functions
 */
LIB_EXPORT int64_t StringToI64 ( const String * self, rc_t * optional_rc )
{
    if ( self != NULL )
        return string_to_I64 ( self -> addr, self -> size, optional_rc );

    if ( optional_rc != NULL )
        * optional_rc = RC ( rcText, rcString, rcEvaluating, rcSelf, rcNull );

    return 0;
}

LIB_EXPORT uint64_t StringToU64 ( const String * self, rc_t * optional_rc )
{
    if ( self != NULL )
        return string_to_U64 ( self -> addr, self -> size, optional_rc );

    if ( optional_rc != NULL )
        * optional_rc = RC ( rcText, rcString, rcEvaluating, rcSelf, rcNull );

    return 0;
}


/*--------------------------------------------------------------------------
 * raw text strings
 */

/* string_size
 *  length of string in bytes
 */
LIB_EXPORT size_t CC string_size ( const char *str )
{
    if ( str == NULL )
        return 0;
    return strlen ( str );
}

/* string_dup
 *  replaces the broken C library strndup
 *  creates a NUL-terminated malloc'd string
 */
LIB_EXPORT char * CC string_dup ( const char *str, size_t size )
{
    char *dst;
    if ( str == NULL )
        dst = NULL;
    else
    {
        dst = malloc ( size + 1 );
        if ( dst != NULL )
            string_copy ( dst, size + 1, str, size );
    }
    return dst;
}

/* string_dup_measure
 *  replaces the broken C library strdup
 *  creates a NUL-terminated malloc'd string
 *  returns size of string unless "size" is NULL
 */
LIB_EXPORT char * CC string_dup_measure ( const char *str, size_t *size )
{
    size_t bytes = string_size ( str );
    if ( size != NULL )
        * size = bytes;
    return string_dup ( str, bytes );
}

/* string_hash
 *  hashes a string
 */
LIB_EXPORT uint32_t CC string_hash ( const char *str, size_t size )
{
    size_t i;
    uint32_t hash;

    assert ( str != NULL );

    if ( str == NULL )
        return 0;

    for ( hash = 0, i = 0; i < size; ++ i )
    {
        uint32_t ch = ( ( const unsigned char* )  str ) [ i ];
        hash = ( ( hash << 1 ) - ( hash >> 16 ) ) ^ ch;
    }
    return hash ^ ( hash >> 16 );
}

/* string_to_int
 *  simple string conversion functions
 *
 *  these functions are defined to consume the entire string.
 *  leading spaces are tolerated, repeated signs are accepted for signed conversion,
 *  decimal and hex encodings are accepted for unsigned conversion,
 *  decimal only for signed conversion.
 *
 *  "optional_rc" [ OUT, NULL OKAY ] - if non-null, user is interested
 *  in error conditions. if the parameter is present, the string must be
 *  completely consumed without overflow.
 *
 *  optional return values ( with { GetRCObject ( rc ), GetRCState ( rc ) }:
 *   0                            : no error
 *   { rcRange, rcExcessive }     : integer overflow
 *   { rcTransfer, rcIncomplete } : extra characters remain in string
 *   { rcData, rcInsufficient }   : no numeric text was found
 *
 *  return values - regardless of "optional_rc":
 *    val             : when no error
 *    val             : on incomplete transfer
 *    +/- max int64_t : when signed overflow occurs ( StringToI64 only )
 *    max uint64_t    : when unsigned overflow occurs ( StringToU64 only )
 *    0               : when no input text is found
 */
LIB_EXPORT int64_t string_to_I64 ( const char * text, size_t bytes, rc_t * optional_rc )
{
    rc_t rc = 0;

    if ( text == NULL )
        rc = RC ( rcText, rcString, rcEvaluating, rcParam, rcNull );
    else
    {
        int64_t val;

        size_t i, start;
        uint8_t negate = 0;

        /* allow white space */
        for ( i = 0; i < bytes; ++ i )
        {
            if ( ! isspace ( text [ i ] ) )
                break;
        }

        /* allow sign */
        for ( ; i < bytes; ++ i )
        {
            switch ( text [ i ] )
            {
            case '-':
                negate ^= 1;
                continue;
            case '+':
                continue;
            }
            break;
        }

        start = i;
        for ( val = 0; i < bytes; ++ i )
        {
            uint8_t digit;

            /* HACK ALERT: by declaring this variable "volatile",
               we prevent gcc-4.8.1 and friends from trying to optimize
               this loop, and producing bad code ( see below ).
               gcc-4.9.1 does not exhibit this behavior.
            */
            volatile int64_t x = 0;

            if ( ! isdigit ( text [ i ] ) )
                break;

            /* want to bring this digit into number */
            digit = text [ i ] - '0';

            /* detect overflow on multiplication
               The gcc optimization replaced a signed int-max
               with an unsigned int-max, destroying the test.
             */
            if ( val > INT64_MAX / 10 )
            {
                rc = RC ( rcText, rcString, rcEvaluating, rcRange, rcExcessive );
                val = INT64_MAX;
                break;
            }

            val *= 10;
            assert ( val >= 0 );

            /* detect overflow on addition */
            x = val + digit - negate;
            if ( x < 0 )
            {
                rc = RC ( rcText, rcString, rcEvaluating, rcRange, rcExcessive );
                val = INT64_MAX;
                break;
            }

            val += digit;
        }

        if ( negate )
            val = ( rc != 0 ) ? INT64_MIN : - val;

        if ( start != i )
        {
            if ( optional_rc != NULL )
            {
                if ( rc == 0 && i != bytes )
                    rc = RC ( rcText, rcString, rcParsing, rcTransfer, rcIncomplete );

                * optional_rc = rc;
            }

            return val;
        }

        /* no digits were converted */
        rc = RC ( rcText, rcString, rcParsing, rcData, rcInsufficient );
    }

    if ( optional_rc != NULL )
        * optional_rc = rc;

    return 0;
}

LIB_EXPORT uint64_t string_to_U64 ( const char * text, size_t bytes, rc_t * optional_rc )
{
    rc_t rc = 0;

    if ( text == NULL )
        rc = RC ( rcText, rcString, rcEvaluating, rcParam, rcNull );
    else
    {
        uint64_t val;
        size_t i, start;

        /* allow white space */
        for ( i = 0; i < bytes; ++ i )
        {
            if ( ! isspace ( text [ i ] ) )
                break;
        }

        /* detect hex */
        if ( bytes - i >= 3 && text [ i ] == '0' && tolower ( text [ i + 1 ] ) == 'x' )
        {
            start = i += 2;
            for ( val = 0; i < bytes; ++ i )
            {
                uint8_t xdigit;

                if ( ! isxdigit ( text [ i ] ) )
                    break;

                /* want to bring this digit into number */
                xdigit = isdigit ( text [ i ] ) ?
                    text [ i ] - '0' : tolower ( text [ i ] ) - 'a' + 10;

                /* detect overflow */
                if ( i - start > 16 )
                {
                    rc = RC ( rcText, rcString, rcEvaluating, rcRange, rcExcessive );
                    val = UINT64_MAX;
                    break;
                }

                val = ( val << 4 ) | xdigit;
            }
        }
        else
        {
            start = i;
            for ( val = 0; i < bytes; ++ i )
            {
                uint8_t digit;

                if ( ! isdigit ( text [ i ] ) )
                    break;

                /* want to bring this digit into number */
                digit = text [ i ] - '0';

                /* detect overflow on multiplication */
                if ( val > UINT64_MAX / 10 )
                {
                    rc = RC ( rcText, rcString, rcEvaluating, rcRange, rcExcessive );
                    val = UINT64_MAX;
                    break;
                }

                val *= 10;

                /* detect overflow on addition */
                if ( val > UINT64_MAX - digit )
                {
                    rc = RC ( rcText, rcString, rcEvaluating, rcRange, rcExcessive );
                    val = UINT64_MAX;
                    break;
                }

                val += digit;
            }
        }

        if ( start != i )
        {
            if ( optional_rc != NULL )
            {
                if ( rc == 0 && i != bytes )
                    rc = RC ( rcText, rcString, rcEvaluating, rcTransfer, rcIncomplete );

                * optional_rc = rc;
            }

            return val;
        }

        /* no digits were converted */
        rc = RC ( rcText, rcString, rcParsing, rcData, rcInsufficient );
    }

    if ( optional_rc != NULL )
        * optional_rc = rc;

    return 0;
}

/* utf8_utf32
 *  converts UTF8 text to a single UTF32 character
 *  returns the number of UTF8 bytes consumed, such that:
 *    return > 0 means success
 *    return == 0 means insufficient input
 *    return < 0 means bad input or bad argument
 */
LIB_EXPORT int CC utf8_utf32 ( uint32_t *dst, const char *begin, const char *end )
{
    int c;
    uint32_t ch;
    const char *src, *stop;

    if ( dst == NULL || begin == NULL || end == NULL )
        return -1;

    if ( begin == end )
        return 0;

    /* non-negative bytes are ASCII-7 */
    c = begin [ 0 ];
    if ( begin [ 0 ] >= 0 )
    {
        dst [ 0 ] = c;
        return 1;
    }
	
    /* the leftmost 24 bits are set
       the rightmost 8 can look like:
       110xxxxx == 2 byte character
       1110xxxx == 3 byte character
       11110xxx == 4 byte character
       111110xx == 5 byte character
       1111110x == 6 byte character
    */
	
    src = begin;
	
    /* invert bits to look at range */
    ch = c;
    c = ~ c;
	
    /* illegal range */
    if ( c >= 0x40 )
        return -1;
	
    /* 2 byte */
    else if ( c >= 0x20 )
    {
        ch &= 0x1F;
        stop = src + 2;
    }
	
    /* 3 byte */
    else if ( c >= 0x10 )
    {
        ch &= 0xF;
        stop = src + 3;
    }
	
    /* 4 byte */
    else if ( c >= 8 )
    {
        ch &= 7;
        stop = src + 4;
    }
	
    /* 5 byte */
    else if ( c >= 4 )
    {
        ch &= 3;
        stop = src + 5;
    }
	
    /* illegal */
    else if ( c < 2 )
        return -1;
    
    /* 6 byte */
    else
    {
        ch &= 1;
        stop = src + 6;
    }
    
    /* must have sufficient input */
    if ( stop > end )
        return 0;
	
    /* complete the character */
    while ( ++ src != stop )
    {
        c = src [ 0 ] & 0x7F;
        if ( src [ 0 ] >= 0 || c >= 0x40 )
            return -1;
        ch = ( ch << 6 ) | c;
    }
	
    /* record the character */
    dst [ 0 ] = ch;
	
    /* return the bytes consumed */
    return ( int ) ( src - begin );
}

/* utf32_utf8
 *  converts a single UTF32 character to UTF8 text
 *  returns the number of UTF8 bytes generated, such that:
 *    return > 0 means success
 *    return == 0 means insufficient output
 *    return < 0 means bad character or bad argument
 */
LIB_EXPORT int CC utf32_utf8 ( char *begin, char *end, uint32_t ch )
{
    int len;
    char *dst;
    uint32_t mask;

    if ( begin == NULL || end == NULL )
        return -1;
    if ( begin >= end )
        return 0;

    if ( ch < 128 )
    {
        begin [ 0 ] = ( char ) ch;
        return 1;
    }

    /* 2 byte */
    if ( ch < 0x00000800 )
    {
        /* 110xxxxx */
        mask = 0xC0U;
        len = 2;
    }
	
    /* 3 byte */
    else if ( ch < 0x00010000 )
    {
        /* 1110xxxx */
        mask = 0xE0U;
        len = 3;
    }

    /* 4 byte */
    else if ( ch < 0x00200000 )
    {
        /* 11110xxx */
        mask = 0xF0U;
        len = 4;
    }
	
    /* 5 byte */
    else if ( ch < 0x04000000 )
    {
        /* 111110xx */
        mask = 0xF8U;
        len = 5;
    }
	
    /* 6 byte */
    else
    {
        /* 1111110x */
        mask = 0xFCU;
        len = 6;
    }
	
    dst = begin + len;
    if ( dst > end )
        return 0;
    
    while ( -- dst > begin )
    {
        /* 10xxxxxx */ /* too many casts to suit different compilers */
        dst [ 0 ] = ( char ) (( char ) 0x80 | ( ( char ) ch & ( char ) 0x3F ));
        ch >>= 6;
    }
    
    dst [ 0 ] = ( char ) ( mask | ch );

    return len;
}

/* utf16_string_size/len/measure
 *  measures UTF-16 strings
 */
LIB_EXPORT size_t CC utf16_string_size ( const uint16_t *str )
{
    uint32_t i, ch;

    for ( ch = str [ i = 0 ]; ch != 0 ; ch = str [ ++ i ] )
    {
        char ignore [ 8 ];
        if ( utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch ) <= 0 )
            break;
    }

    return i * sizeof * str;
}

LIB_EXPORT uint32_t CC utf16_string_len ( const uint16_t *str, size_t size )
{
    uint32_t i, str_len = ( uint32_t ) ( size >> 1 );

    for ( i = 0; i < str_len; ++ i )
    {
        char ignore [ 8 ];
        uint32_t ch = str [ i ];
        if ( utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch ) <= 0 )
            break;
    }

    return i;
}

LIB_EXPORT uint32_t CC utf16_string_measure ( const uint16_t *str, size_t *size )
{
    uint32_t i, ch;

    for ( ch = str [ i = 0 ]; ch != 0 ; ch = str [ ++ i ] )
    {
        char ignore [ 8 ];
        if ( utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch ) <= 0 )
            break;
    }

    * size = i * sizeof * str;

    return i;
}

/* utf32_string_size/len/measure
 */
LIB_EXPORT size_t CC utf32_string_size ( const uint32_t *str )
{
    uint32_t i, ch;

    for ( ch = str [ i = 0 ]; ch != 0 ; ch = str [ ++ i ] )
    {
        char ignore [ 8 ];
        if ( utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch ) <= 0 )
            break;
    }

    return i * sizeof * str;
}

LIB_EXPORT uint32_t CC utf32_string_len ( const uint32_t *str, size_t size )
{
    uint32_t i, str_len = ( uint32_t ) ( size >> 2 );

    for ( i = 0; i < str_len; ++ i )
    {
        char ignore [ 8 ];
        uint32_t ch = str [ i ];
        if ( utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch ) <= 0 )
            break;
    }

    return i;
}

LIB_EXPORT uint32_t CC utf32_string_measure ( const uint32_t *str, size_t *size )
{
    uint32_t i, ch;

    for ( ch = str [ i = 0 ]; ch != 0 ; ch = str [ ++ i ] )
    {
        char ignore [ 8 ];
        if ( utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch ) <= 0 )
            break;
    }

    * size = i * sizeof * str;

    return i;
}

/* whcar_string_size/len/measure
 *  measures whcar_t strings
 */
LIB_EXPORT size_t CC wchar_string_size ( const wchar_t *str )
{
    uint32_t i, ch;

    for ( ch = str [ i = 0 ]; ch != 0 ; ch = str [ ++ i ] )
    {
        char ignore [ 8 ];
        if ( utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch ) <= 0 )
            break;
    }

    return i * sizeof * str;
}

LIB_EXPORT uint32_t CC wchar_string_len ( const wchar_t *str, size_t size )
{
    uint32_t i, str_len = ( uint32_t ) ( size / sizeof * str );

    for ( i = 0; i < str_len; ++ i )
    {
        char ignore [ 8 ];
        uint32_t ch = str [ i ];
        if ( utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch ) <= 0 )
            break;
    }

    return i;
}

LIB_EXPORT uint32_t CC wchar_string_measure ( const wchar_t *str, size_t *size )
{
    uint32_t i, ch;

    for ( ch = str [ i = 0 ]; ch != 0 ; ch = str [ ++ i ] )
    {
        char ignore [ 8 ];
        if ( utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch ) <= 0 )
            break;
    }

    * size = i * sizeof * str;

    return i;
}
