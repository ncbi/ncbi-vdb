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
LIB_EXPORT uint32_t CC string_len ( const char *str, size_t size )
{
    const char *end;
    uint32_t len = 0;

    assert ( str != NULL );
    end = str + size;

    while ( str < end )
    {
        int c;

        while ( str [ 0 ] > 0 )
        {
            ++ len;
            if ( ++ str == end )
                return len;
        }

        c = str [ 0 ];
        if ( str [ 0 ] == 0 )
            break;
	
        c = ~ c;
        if ( c >= 0x40 )
            break;
	
        if ( c >= 0x20 )
            str += 2;
        else if ( c >= 0x10 )
            str += 3;
        else if ( c >= 0x08 )
            str += 4;
        else if ( c >= 0x04 )
            str += 5;
        else if ( c >= 0x02 )
            str += 6;
        else
            break;
	
        ++ len;
    }
    return len;
}

/* string_measure
 *  measures length of string in both characters and bytes
 */
LIB_EXPORT uint32_t CC string_measure ( const char *str, size_t *size )
{
    const char *begin;
    uint32_t len = 0;

    assert ( str != NULL );

    begin = str;

    while ( 1 )
    {
        int c;

        while ( str [ 0 ] > 0 )
        {
            ++ len;
            ++ str;
        }

        c = str [ 0 ];
        if ( str [ 0 ] == 0 )
            break;
	
        c = ~ c;
        if ( c >= 0x40 )
            break;
	
        if ( c >= 0x20 )
            str += 2;
        else if ( c >= 0x10 )
            str += 3;
        else if ( c >= 0x08 )
            str += 4;
        else if ( c >= 0x04 )
            str += 5;
        else if ( c >= 0x02 )
            str += 6;
        else
            break;
	
        ++ len;
    }

    if ( size != NULL )
        * size = ( size_t ) ( str - begin );
    return len;
}

/* string_copy
 *  copies whole character text into a buffer
 *  terminates with null byte if possible
 *  returns the number of bytes copied
 */
LIB_EXPORT size_t CC string_copy ( char *dst, size_t dst_size, const char *src, size_t src_size )
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
