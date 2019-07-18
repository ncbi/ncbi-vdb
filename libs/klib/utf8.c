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
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <wctype.h>

/*--------------------------------------------------------------------------
 * raw text strings
 */

/* string_len
 *  length of string in characters
 */
LIB_EXPORT uint32_t CC string_len ( const char * str, size_t size )
{
    uint32_t len = 0;

    if ( str != NULL )
    {
        size_t i;
        for ( i = 0;; ++ len )
        {
            int c;
            size_t start;

            /* scan all ASCII characters */
            for ( start = i; i < size && str [ i ] > 0; ++ i )
                ( void ) 0;
            len += ( uint32_t ) ( i - start );
            if ( i >= size )
                break;

            /* handle a single UTF-8 character */
            c = ~ ( int ) str [ i ];
            if ( str [ i ] == 0 )
                break;

            /* str [ i ] was < 0, illegal */
            /*assert ( c > 0 && c <= 0x7F );*/
            if ( c <= 0 || c > 0x7F )
                break;

            /* if original code was 0b10xxxxxx, illegal */
            if ( c >= 0x40 )
                break;

            /* original code 0b110xxxxx = 2 byte */
            if ( c >= 0x20 )
                i += 2;
            /* original code 0b1110xxxx = 3 byte */
            else if ( c >= 0x10 )
                i += 3;
            /* original code 0b11110xxx = 4 byte */
            else if ( c >= 0x08 )
                i += 4;
            /* original code 0b111110xx = 5 byte */
            else if ( c >= 0x04 )
                i += 5;
            /* original code 0b1111110x = 6 byte */
            else if ( c >= 0x02 )
                i += 6;
            /* illegal code 0b1111111x */
            else
                break;
	    }
    }

    return len;
}

/* string_measure
 *  measures length of string in both characters and bytes
 */
LIB_EXPORT uint32_t CC string_measure ( const char * str, size_t * size )
{
    size_t i = 0;
    uint32_t len = 0;

    if ( str != NULL )
    {
        for ( ;; ++ len )
        {
            int c;
            size_t start;

            /* scan all ASCII characters */
            for ( start = i; str [ i ] > 0; ++ i )
                ( void ) 0;

            /* the character that stopped scan */
            c = ~ ( int ) str [ i ];

            /* the number of characters */
            len += ( uint32_t ) ( i - start );

            /* handle a single UTF-8 character */
            if ( str [ i ] == 0 )
                break;

            /* str [ i ] was < 0 */
            /*assert ( c > 0 && c <= 0x7F );*/
            if ( c <= 0 || c > 0x7F )
                break;

            /* if original code was 0b10xxxxxx, illegal */
            if ( c >= 0x40 )
                break;

            /* original code 0b110xxxxx = 2 byte */
            if ( c >= 0x20 )
                i += 2;
            /* original code 0b1110xxxx = 3 byte */
            else if ( c >= 0x10 )
                i += 3;
            /* original code 0b11110xxx = 4 byte */
            else if ( c >= 0x08 )
                i += 4;
            /* original code 0b111110xx = 5 byte */
            else if ( c >= 0x04 )
                i += 5;
            /* original code 0b1111110x = 6 byte */
            else if ( c >= 0x02 )
                i += 6;
            /* illegal code 0b1111111x */
            else
                break;
	    }
    }

    if ( size != NULL )
        * size = i;

    return len;
}

/* string_copy
 *  copies whole character text into a buffer
 *  terminates with null byte if possible
 *  returns the number of bytes copied
 */
LIB_EXPORT size_t CC string_copy ( char *dst, size_t dst_size, const char *src, size_t src_size )
{
    size_t i;
    char * dend;
    const char * send;

    if ( dst == NULL || src == NULL )
        return 0;

    if ( dst_size < src_size )
        src_size = dst_size;

    dend = dst + dst_size;
    send = src + src_size;

    for ( i = 0; i < src_size; )
    {
        uint32_t ch;
        int len1, len2;

        /* optimistic copy of ASCII data */
        for ( ; i < src_size && src [ i ] > 0; ++ i )
            dst [ i ] = src [ i ];
        if ( i == src_size )
            break;

        /* read a ( hopefully complete ) UNICODE character ( detect NUL ) */
        len1 = utf8_utf32 ( & ch, & src [ i ], send );
        if ( len1 <= 0 || ch == 0 )
            break;

        /* write the UNICODE character in UTF-8 */
        len2 = utf32_utf8 ( & dst [ i ], dend, ch );
        if ( len2 <= 0 )
            break;

        /* should have been identical number of bytes */
        if ( len1 != len2 )
            break;

        /* advance over the UTF-8 character */
        i += len1;
    }

    if ( i < dst_size )
        dst [ i ] = 0;

    return i;
}

LIB_EXPORT size_t CC old_string_copy ( char *dst, size_t dst_size, const char *src, size_t src_size )
{
    const char *send;
    char *dend, *begin;

    assert ( dst != NULL && src != NULL );

    if ( dst_size < src_size )
        src_size = dst_size;

    begin = dst;
    dend = dst + dst_size;
    send = src + src_size;

    while ( src < send )
    {
        uint32_t ch;

        /* get a complete source character */
        int len = utf8_utf32 ( & ch, src, send );
        if ( len <= 0 )
            break;
        src += len;

        /* write it to the destination */
        len = utf32_utf8 ( dst, dend, ch );
        if ( len <= 0 )
            break;
        dst += len;
    }

    if ( dst < dend )
        * dst = 0;

    return ( size_t ) ( dst - begin );
}


/* string_copy_measure
 *  copies whole character text into a buffer
 *  terminates with null byte if possible
 *  returns the number of bytes copied
 */
LIB_EXPORT size_t CC string_copy_measure ( char *dst, size_t dst_size, const char *src )
{
    size_t i;
    char * dend;

    if ( dst == NULL || src == NULL )
        return 0;

    dend = dst + dst_size;

    for ( i = 0;; )
    {
        uint32_t ch;
        int len1, len2;

        /* optimistic copy of ASCII data ( NUL terminated ) */
        for ( ; i < dst_size && src [ i ] > 0; ++ i )
            dst [ i ] = src [ i ];
        if ( i == dst_size || src [ i ] == 0 )
            break;

        /* read a ( hopefully complete ) UNICODE character */
        len1 = utf8_utf32 ( & ch, & src [ i ], & src [ i + 6 ] );
        if ( len1 <= 0 )
            break;

        /* write the UNICODE character in UTF-8 */
        len2 = utf32_utf8 ( & dst [ i ], dend, ch );
        if ( len2 <= 0 )
            break;

        /* should have been identical number of bytes */
        if ( len1 != len2 )
            break;

        /* advance over the UTF-8 character */
        i += len1;
    }

    if ( i < dst_size )
        dst [ i ] = 0;

    return i;
}

LIB_EXPORT size_t CC old_string_copy_measure ( char *dst, size_t dst_size, const char *src )
{
    char *dend, *begin;

    assert ( dst != NULL && src != NULL );

    begin = dst;
    dend = dst + dst_size;

    while ( dst < dend && src [ 0 ] != 0 )
    {
        uint32_t ch;

        /* get a complete source character */
        int len = utf8_utf32 ( & ch, src, src + 6 );
        if ( len <= 0 )
            break;
        src += len;

        /* write it to the destination */
        len = utf32_utf8 ( dst, dend, ch );
        if ( len <= 0 )
            break;
        dst += len;
    }

    if ( dst < dend )
        * dst = 0;

    return ( size_t ) ( dst - begin );
}

/* tolower_copy
 *  copies whole character text in lower-case
 *  terminates with null byte if possible
 *  returns the number of bytes copied
 */
LIB_EXPORT size_t CC tolower_copy ( char *dst, size_t dst_size, const char *src, size_t src_size )
{
    size_t i;
    char * dend;
    const char * send;

    if ( dst == NULL || src == NULL )
        return 0;

    if ( dst_size < src_size )
        src_size = dst_size;

    dend = dst + dst_size;
    send = src + src_size;

    for ( i = 0; i < src_size; )
    {
        uint32_t ch;
        int len1, len2;

        /* optimistic copy of ASCII data */
        for ( ; i < src_size && src [ i ] > 0; ++ i )
            dst [ i ] = ( char ) tolower ( src [ i ] );
        if ( i == src_size )
            break;

        /* read a ( hopefully complete ) UNICODE character ( detect NUL ) */
        len1 = utf8_utf32 ( & ch, & src [ i ], send );
        if ( len1 <= 0 || ch == 0 )
            break;

        /* lower case it */
        ch = towlower ( ( wint_t ) ch );

        /* write the UNICODE character in UTF-8 */
        len2 = utf32_utf8 ( & dst [ i ], dend, ch );
        if ( len2 <= 0 )
            break;

        /* should have been identical number of bytes */
        if ( len1 != len2 )
            break;

        /* advance over the UTF-8 character */
        i += len1;
    }

    if ( i < dst_size )
        dst [ i ] = 0;

    return i;
}


LIB_EXPORT size_t CC old_tolower_copy ( char *dst, size_t dst_size, const char *src, size_t src_size )
{
    const char *send;
    char *dend, *begin;

    assert ( dst != NULL && src != NULL );

    if ( dst_size < src_size )
        src_size = dst_size;

    begin = dst;
    dend = dst + dst_size;
    send = src + src_size;

    while ( src < send )
    {
        uint32_t ch;

        /* get a complete source character */
        int len = utf8_utf32 ( & ch, src, send );
        if ( len <= 0 )
            break;
        src += len;

        /* lower case it */
        ch = towlower ( ( wint_t ) ch );

        /* write it to the destination */
        len = utf32_utf8 ( dst, dend, ch );
        if ( len <= 0 )
            break;
        dst += len;
    }

    if ( dst < dend )
        * dst = 0;

    return ( size_t ) ( dst - begin );
}


/* toupper_copy
 *  copies whole character text in upper-case
 *  terminates with null byte if possible
 *  returns the number of bytes copied
 */
LIB_EXPORT size_t CC toupper_copy ( char *dst, size_t dst_size, const char *src, size_t src_size )
{
    size_t i;
    char * dend;
    const char * send;

    if ( dst == NULL || src == NULL )
        return 0;

    if ( dst_size < src_size )
        src_size = dst_size;

    dend = dst + dst_size;
    send = src + src_size;

    for ( i = 0; i < src_size; )
    {
        uint32_t ch;
        int len1, len2;

        /* optimistic copy of ASCII data */
        for ( ; i < src_size && src [ i ] > 0; ++ i )
            dst [ i ] = ( char ) toupper ( src [ i ] );
        if ( i == src_size )
            break;

        /* read a ( hopefully complete ) UNICODE character ( detect NUL ) */
        len1 = utf8_utf32 ( & ch, & src [ i ], send );
        if ( len1 <= 0 || ch == 0 )
            break;

        /* upper case it */
        ch = towupper ( ( wint_t ) ch );

        /* write the UNICODE character in UTF-8 */
        len2 = utf32_utf8 ( & dst [ i ], dend, ch );
        if ( len2 <= 0 )
            break;

        /* should have been identical number of bytes */
        if ( len1 != len2 )
            break;

        /* advance over the UTF-8 character */
        i += len1;
    }

    if ( i < dst_size )
        dst [ i ] = 0;

    return i;
}

LIB_EXPORT size_t CC old_toupper_copy ( char *dst, size_t dst_size, const char *src, size_t src_size )
{
    const char *send;
    char *dend, *begin;

    assert ( dst != NULL && src != NULL );

    if ( dst_size < src_size )
        src_size = dst_size;

    begin = dst;
    dend = dst + dst_size;
    send = src + src_size;

    while ( src < send )
    {
        uint32_t ch;

        /* get a complete source character */
        int len = utf8_utf32 ( & ch, src, send );
        if ( len <= 0 )
            break;
        src += len;

        /* upper case it */
        ch = towupper ( ( wint_t ) ch );

        /* write it to the destination */
        len = utf32_utf8 ( dst, dend, ch );
        if ( len <= 0 )
            break;
        dst += len;
    }

    if ( dst < dend )
        * dst = 0;

    return ( size_t ) ( dst - begin );
}

/* string_cmp
 *  performs a safe strncmp
 *
 *  "max_chars" limits the extent of the comparison
 *  to not exceed supplied value, i.e. the number of
 *  characters actually compared will be the minimum
 *  of asize, bsize and max_chars.
 *
 *  if either string size ( or both ) < max_chars and
 *  all compared characters match, then the result will
 *  be a comparison of asize against bsize.
 */
LIB_EXPORT int CC string_cmp ( const char *a, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars )
{
    size_t i, sz;
    uint32_t num_chars;
    const char *aend, *bend;

    if ( max_chars == 0 )
        return 0;

    if ( b == NULL )
        return a != NULL;
    if ( a == NULL )
        return -1;

    sz = asize;
    if ( asize > bsize )
        sz = bsize;

    aend = a + asize;
    bend = b + bsize;
    num_chars = 0;

    for ( i = 0; i < sz; )
    {
        int len1, len2;
        uint32_t ach, bch;

        /* loop to process ASCII characters */
        for ( ; i < sz; ++ i )
        {
            /* detect UTF-8 character */
            if ( a [ i ] < 0 || b [ i ] < 0 )
                break;

            /* detect different or NUL character */
            if ( a [ i ] != b [ i ] || a [ i ] == 0 )
                return a [ i ] - b [ i ];

            /* if char count is sufficient, we're done */
            if ( ++ num_chars == max_chars )
                return 0;
        }

        /* read a character from a */
        len1 = utf8_utf32 ( & ach, & a [ i ], aend );
        if ( len1 <= 0 )
        {
            asize = i;

            len2 = utf8_utf32 ( & bch, & b [ i ], bend );
            if ( len2 <= 0 )
                bsize = i;

            break;
        }

        /* read a character from b */
        len2 = utf8_utf32 ( & bch, & b [ i ], bend );
        if ( len2 <= 0 )
        {
            bsize = i;
            break;
        }

        /* compare characters */
        if ( ach != bch )
        {
            if ( ach < bch )
                return -1;
            return 1;
        }

        /* if char count is sufficient, we're done */
        if ( ++ num_chars == max_chars )
            return 0;

        /* adjust the pointers */
        if ( len1 == len2 )
            i += len1;
        else
        {
            sz -= i;
            a += i + len1;
            b += i + len2;
            i = 0;
        }
    }

    /* one or both reached end < max_chars */
    if ( asize < bsize )
        return -1;

    return asize > bsize;
}

LIB_EXPORT int CC old_string_cmp ( const char *a, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars )
{
    uint32_t num_chars;
    const char *aend, *bend;

    assert ( a != NULL && b != NULL );

    if ( max_chars == 0 )
        return 0;

    aend = a + asize;
    bend = b + bsize;
    num_chars = 0;

    while ( a < aend && b < bend )
    {
        uint32_t ach, bch;

        /* read a character from a */
        int len = utf8_utf32 ( & ach, a, aend );
        if ( len <= 0 )
        {
            asize -= ( size_t ) ( aend - a );
            break;
        }
        a += len;

        /* read a character from b */
        len = utf8_utf32 ( & bch, b, bend );
        if ( len <= 0 )
        {
            bsize -= ( size_t ) ( bend - b );
            break;
        }
        b += len;

        /* compare characters */
        if ( ach != bch )
        {
            if ( ach < bch )
                return -1;
            return 1;
        }

        /* if char count is sufficient, we're done */
        if ( ++ num_chars == max_chars )
            return 0;
    }

    /* one or both reached end < max_chars */
    if ( asize < bsize )
        return -1;
    return asize > bsize;
}

/* strcase_cmp
 *  like string_cmp except case insensitive
 */
LIB_EXPORT int CC strcase_cmp ( const char *a, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars )
{
    size_t i, sz;
    uint32_t num_chars;
    const char *aend, *bend;

    if ( max_chars == 0 )
        return 0;

    if ( b == NULL )
        return a != NULL;
    if ( a == NULL )
        return -1;

    sz = asize;
    if ( asize > bsize )
        sz = bsize;

    aend = a + asize;
    bend = b + bsize;
    num_chars = 0;

    for ( i = 0; i < sz; )
    {
        int len1, len2;
        uint32_t ach, bch;

        /* loop to process ASCII characters */
        for ( ; i < sz; ++ i )
        {
            /* detect UTF-8 character */
            if ( a [ i ] < 0 || b [ i ] < 0 )
                break;

            /* detect different or NUL character */
            if ( a [ i ] != b [ i ] || a [ i ] == 0 )
            {
                ach = tolower ( a [ i ] );
                bch = tolower ( b [ i ] );
                if ( ach != bch || ach == 0 )
                    return ach - bch;
            }

            /* if char count is sufficient, we're done */
            if ( ++ num_chars == max_chars )
                return 0;
        }

        /* read a character from a */
        len1 = utf8_utf32 ( & ach, & a [ i ], aend );
        if ( len1 <= 0 )
        {
            asize = i;

            len2 = utf8_utf32 ( & bch, & b [ i ], bend );
            if ( len2 <= 0 )
                bsize = i;

            break;
        }

        /* read a character from b */
        len2 = utf8_utf32 ( & bch, & b [ i ], bend );
        if ( len2 <= 0 )
        {
            bsize = i;
            break;
        }

        /* compare characters */
        if ( ach != bch )
        {
            /* only go lower case if they differ */
            ach = towlower ( ( wint_t ) ach );
            bch = towlower ( ( wint_t ) bch );

            if ( ach != bch )
            {
                if ( ach < bch )
                    return -1;
                return 1;
            }
        }

        /* if char count is sufficient, we're done */
        if ( ++ num_chars == max_chars )
            return 0;

        /* adjust the pointers */
        if ( len1 == len2 )
            i += len1;
        else
        {
            sz -= i;
            a += i + len1;
            b += i + len2;
            i = 0;
        }
    }

    /* one or both reached end < max_chars */
    if ( asize < bsize )
        return -1;
    return asize > bsize;
}

LIB_EXPORT int CC old_strcase_cmp ( const char *a, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars )
{
    uint32_t num_chars;
    const char *aend, *bend;

    assert ( a != NULL && b != NULL );

    if ( max_chars == 0 )
        return 0;

    aend = a + asize;
    bend = b + bsize;
    num_chars = 0;

    while ( a < aend && b < bend )
    {
        uint32_t ach, bch;

        /* read a character from a */
        int len = utf8_utf32 ( & ach, a, aend );
        if ( len <= 0 )
        {
            asize -= ( size_t ) ( aend - a );
            break;
        }
        a += len;

        /* read a character from b */
        len = utf8_utf32 ( & bch, b, bend );
        if ( len <= 0 )
        {
            bsize -= ( size_t ) ( bend - b );
            break;
        }
        b += len;

        /* compare characters with case */
        if ( ach != bch )
        {
            /* only go lower case if they differ */
            ach = towlower ( ( wint_t ) ach );
            bch = towlower ( ( wint_t ) bch );

            if ( ach != bch )
            {
                if ( ach < bch )
                    return -1;
                return 1;
            }
        }

        /* if char count is sufficient, we're done */
        if ( ++ num_chars == max_chars )
            return 0;
    }

    /* one or both reached end < max_chars */
    if ( asize < bsize )
        return -1;
    return asize > bsize;
}

/* string_match
 *  returns the number of matching characters
 *
 *  "max_chars" limits the extent of the comparison
 *  to not exceed supplied value, i.e. the number of
 *  characters actually compared will be the minimum
 *  of asize, bsize and max_chars.
 *
 *  "msize" will be set to the size of the matched string
 *  if not NULL
 */
LIB_EXPORT uint32_t CC string_match ( const char *a_orig, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars, size_t *msize )
{
    uint32_t i;
    const char *a, *aend, *bend;

    assert ( a_orig != NULL && b != NULL );

    a = a_orig;
    aend = a_orig + asize;
    bend = b + bsize;

    for ( i = 0; i < max_chars && a < aend && b < bend; ++ i )
    {
        int lena, lenb;
        uint32_t ach, bch;

        /* read a character from a */
        lena = utf8_utf32 ( & ach, a, aend );
        if ( lena <= 0 )
            break;

        /* read a character from b */
        lenb = utf8_utf32 ( & bch, b, bend );
        if ( lenb <= 0 )
            break;

        /* compare characters */
        if ( ach != bch )
            break;

        a += lena;
        b += lenb;
    }

    if ( msize != NULL )
        * msize = ( size_t ) ( a - a_orig );
    return i;
}

/* strcase_match
 *  like string_match except case insensitive
 */
LIB_EXPORT uint32_t CC strcase_match ( const char *a_orig, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars, size_t *msize )
{
    uint32_t i;
    const char *a, *aend, *bend;

    assert ( a_orig != NULL && b != NULL );

    a = a_orig;
    aend = a_orig + asize;
    bend = b + bsize;

    for ( i = 0; i < max_chars && a < aend && b < bend; ++ i )
    {
        int lena, lenb;
        uint32_t ach, bch;

        /* read a character from a */
        lena = utf8_utf32 ( & ach, a, aend );
        if ( lena <= 0 )
            break;

        /* read a character from b */
        lenb = utf8_utf32 ( & bch, b, bend );
        if ( lenb <= 0 )
            break;

        /* compare characters */
        if ( ach != bch )
        {
            /* only go lower case if they differ */
            ach = towlower ( ( wint_t ) ach );
            bch = towlower ( ( wint_t ) bch );

            if ( ach != bch )
                break;
        }

        a += lena;
        b += lenb;
    }

    if ( msize != NULL )
        * msize = ( size_t ) ( a - a_orig );
    return i;
}

/* string_chr
 *  performs a safe strchr
 *  "ch" is in UTF32
 */
LIB_EXPORT char * CC string_chr ( const char *str, size_t size, uint32_t ch )
{
    size_t i;

    if ( str == NULL || size == 0 )
        return NULL;

    if ( ch < 128 )
    {
        /* looking for an ASCII character */
        for ( i = 0; i < size; ++ i )
        {
            /* perform direct ASCII match */
            if ( str [ i ] == ( char ) ch )
                return ( char * ) & str [ i ];
        }
    }
    else
    {
        int len;
        uint32_t c;
        const char *end = str + size;

        for ( i = 0; i < size; )
        {
            /* skip over ASCII */
            for ( ; i < size && str [ i ] > 0; ++ i )
                ( void ) 0;
            if ( i == size )
                break;

            /* read UTF-8 */
            len = utf8_utf32 ( & c, & str [ i ], end );
            if ( len <= 0 )
                break;
            if ( c == ch )
                return ( char* ) & str [ i ];
            i += len;
        }
    }
    return NULL;
}

LIB_EXPORT char * CC old_string_chr ( const char *str, size_t size, uint32_t ch )
{
    const char *end;

    if ( str == NULL )
        return NULL;

    end = str + size;
    while ( str < end )
    {
        uint32_t c;
        int len = utf8_utf32 ( & c, str, end );
        if ( len <= 0 )
            break;
        if ( c == ch )
            return ( char* ) str;
        str += len;
    }
    return NULL;
}

/* string_rchr
 *  performs a safe strrchr
 */
LIB_EXPORT char * CC string_rchr ( const char *str, size_t size, uint32_t ch )
{
    int64_t i;

    if ( str == NULL || size == 0 )
        return NULL;

    if ( ch < 128 )
    {
        /* looking for an ASCII character */
        for ( i = ( int64_t ) size - 1; i >= 0; -- i )
        {
            /* perform direct ASCII match */
            if ( str [ i ] == ( char ) ch )
                return ( char * ) & str [ i ];
        }
    }
    else
    {
        int len;
        uint32_t c;
        const char *end;

        for ( i = ( int64_t ) size - 1; i >= 0; -- i )
        {
            /* skip over ASCII */
            for ( ; i >= 0 && str [ i ] > 0; -- i )
                ( void ) 0;
            if ( i < 0 )
                break;

            /* back over UTF-8 */
            for ( end = & str [ i + 1 ]; i >= 0 && ( str [ i ] & 0xC0 ) == 0x80; -- i )
                ( void ) 0;
            if ( i < 0 )
                break;

            /* read UTF-8 */
            len = utf8_utf32 ( & c, & str [ i ], end );
            if ( len <= 0 || & str [ i + len ] != end )
                break;
            if ( c == ch )
                return ( char* ) & str [ i ];
        }
    }
    return NULL;
}

LIB_EXPORT char * CC old_string_rchr ( const char *str, size_t size, uint32_t ch )
{
    const char *end;

    if ( str == NULL )
        return NULL;

    end = str + size;
    while ( end > str )
    {
        int len;
        uint32_t c;
        const char *p = end - 1;

        /* back up to find beginning of character */
        while ( p > str && ( * p & 0xC0 ) == 0x80 )
            -- p;

        len = utf8_utf32 ( & c, p, end );
        if ( len <= 0 )
            break;
        if ( c == ch )
            return ( char* ) p;
        end = p;
    }
    return NULL;
}

/* string_idx
 *  seek an indexed character
 */
LIB_EXPORT char * CC string_idx ( const char *str, size_t size, uint32_t idx )
{
    uint32_t i;
    const char *end;

    if ( str == NULL )
        return NULL;

    end = str + size;
    for ( i = 0; str < end; ++ i )
    {
        uint32_t c;
        int len = utf8_utf32 ( & c, str, end );
        if ( len <= 0 )
            break;
        if ( i == idx )
            return ( char* ) str;
        str += len;
    }
    return NULL;
}

/* conversion from UTF-16 to internal standard */
LIB_EXPORT uint32_t CC utf16_cvt_string_len ( const uint16_t *src,
    size_t src_size, size_t *dst_size )
{
    size_t size;
    uint32_t i, src_len = ( uint32_t ) ( src_size >> 1 );

    for ( size = 0, i = 0; i < src_len; ++ i )
    {
        uint32_t ch = src [ i ];

        char ignore [ 8 ];
        int ch_len = utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch );
        if ( ch_len <= 0 )
            break;
        size += ch_len;
    }

    * dst_size = size;
    return i;
}

LIB_EXPORT uint32_t CC utf16_cvt_string_measure ( const uint16_t *src,
    size_t *src_size, size_t *dst_size )
{
    size_t size;
    uint32_t i, ch;

    for ( size = 0, ch = src [ i = 0 ]; ch != 0; ch = src [ ++ i ] )
    {
        char ignore [ 8 ];
        int ch_len = utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch );
        if ( ch_len <= 0 )
            break;
        size += ch_len;
    }

    * src_size = i * sizeof * src;
    * dst_size = size;
    return i;
}

LIB_EXPORT size_t CC utf16_cvt_string_copy ( char *dst, size_t dst_size,
    const uint16_t *src, size_t src_size )
{
    char *begin = dst;
    char *dend = dst + dst_size;
    const uint16_t *send = ( const uint16_t* ) ( ( const char* ) src + src_size );

    while ( dst < dend && src < send )
    {
        uint32_t ch = * src ++;
        int ch_len = utf32_utf8 ( dst, dend, ch );
        if ( ch_len <= 0 )
            break;
        dst += ch_len;
    }

    if ( dst < dend )
        * dst = 0;
    return ( size_t ) ( dst - begin );
}

/* conversion from UTF-32 to internal standard */
LIB_EXPORT uint32_t CC utf32_cvt_string_len ( const uint32_t *src,
    size_t src_size, size_t *dst_size )
{
    size_t size;
    uint32_t i, src_len = ( uint32_t ) ( src_size >> 2 );

    for ( size = 0, i = 0; i < src_len; ++ i )
    {
        uint32_t ch = src [ i ];

        char ignore [ 8 ];
        int ch_len = utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch );
        if ( ch_len <= 0 )
            break;
        size += ch_len;
    }

    * dst_size = size;
    return i;
}

LIB_EXPORT uint32_t CC utf32_cvt_string_measure ( const uint32_t *src,
    size_t *src_size, size_t *dst_size )
{
    size_t size;
    uint32_t i, ch;

    for ( size = 0, ch = src [ i = 0 ]; ch != 0; ch = src [ ++ i ] )
    {
        char ignore [ 8 ];
        int ch_len = utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch );
        if ( ch_len <= 0 )
            break;
        size += ch_len;
    }

    * src_size = i * sizeof * src;
    * dst_size = size;
    return i;
}

LIB_EXPORT size_t CC utf32_cvt_string_copy ( char *dst, size_t dst_size,
    const uint32_t *src, size_t src_size )
{
    char *begin = dst;
    char *dend = dst + dst_size;
    const uint32_t *send = ( const uint32_t* ) ( ( const char* ) src + src_size );

    while ( dst < dend && src < send )
    {
        uint32_t ch = * src ++;
        int ch_len = utf32_utf8 ( dst, dend, ch );
        if ( ch_len <= 0 )
            break;
        dst += ch_len;
    }

    if ( dst < dend )
        * dst = 0;
    return ( size_t ) ( dst - begin );
}

/* conversion from wchar_t to internal standard */
LIB_EXPORT uint32_t CC wchar_cvt_string_len ( const wchar_t *src,
    size_t src_size, size_t *dst_size )
{
    size_t size;
    uint32_t i, src_len = ( uint32_t ) ( src_size / sizeof * src );

    for ( size = 0, i = 0; i < src_len; ++ i )
    {
        uint32_t ch = src [ i ];

        char ignore [ 8 ];
        int ch_len = utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch );
        if ( ch_len <= 0 )
            break;
        size += ch_len;
    }

    * dst_size = size;
    return i;
}

LIB_EXPORT uint32_t CC wchar_cvt_string_measure ( const wchar_t *src,
    size_t *src_size, size_t *dst_size )
{
    size_t size;
    uint32_t i, ch;

    for ( size = 0, ch = src [ i = 0 ]; ch != 0; ch = src [ ++ i ] )
    {
        char ignore [ 8 ];
        int ch_len = utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch );
        if ( ch_len <= 0 )
            break;
        size += ch_len;
    }

    * src_size = i * sizeof * src;
    * dst_size = size;
    return i;
}

LIB_EXPORT size_t CC wchar_cvt_string_copy ( char *dst, size_t dst_size,
    const wchar_t *src, size_t src_size )
{
    char *begin = dst;
    char *dend = dst + dst_size;
    const wchar_t *send = ( const wchar_t* ) ( ( const char* ) src + src_size );

    while ( dst < dend && src < send )
    {
        uint32_t ch = * src ++;
        int ch_len = utf32_utf8 ( dst, dend, ch );
        if ( ch_len <= 0 )
            break;
        dst += ch_len;
    }

    if ( dst < dend )
        * dst = 0;

    return ( size_t ) ( dst - begin );
}

LIB_EXPORT size_t CC string_cvt_wchar_copy ( wchar_t *dst, size_t dst_size,
    const char *src, size_t src_size )
{
    uint32_t len, blen = ( uint32_t ) ( dst_size / sizeof *dst );
    const char *send = ( const char* ) src + src_size;

    for ( len = 0; len < blen && src < send; ++ len )
    {
        uint32_t ch;
        int consumed = utf8_utf32 ( &ch, src, send );
        if ( consumed <= 0 )
            break;
        dst [ len ] = ( wchar_t ) ch;
        src += consumed;
    }

    if ( len < blen )
        dst [ len ] = 0;

    return ( size_t ) ( len );
}
