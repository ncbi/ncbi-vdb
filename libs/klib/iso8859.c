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
#include <assert.h>

/* iso8859_utf32
 *  converts 8-bit text to a single UTF32 character
 *  returns the number of 8-bit bytes consumed, such that:
 *    return > 0 means success
 *    return == 0 means insufficient input
 *    return < 0 means bad input
 */
LIB_EXPORT int CC iso8859_utf32 ( const uint32_t map [ 128 ],
    uint32_t *cp, const char *begin, const char *end )
{
    uint32_t ch;

    if ( begin == NULL || end == NULL )
        return -1;
    if ( begin >= end )
        return 0;

    ch = begin [ 0 ];
    if ( begin [ 0 ] < 0 )
    {
        ch = map [ ch & 0x7F ];
        if ( ch == 0 )
            return -1;
    }

    * cp = ch;
    return 1;
}

/* iso8859_string_size/len/measure
 *  measures UTF-16 strings
 */
LIB_EXPORT size_t CC iso8859_string_size ( const uint32_t map [ 128 ],
    const char *str )
{
    uint32_t i, ch;

    for ( ch = str [ i = 0 ]; ch != 0; ch = str [ ++ i ] )
    {
        if ( ( int ) ch < 0 && map [ ch & 0x7F ] == 0 )
            break;
    }

    return ( size_t ) i;
}

LIB_EXPORT uint32_t CC iso8859_string_len ( const uint32_t map [ 128 ],
    const char *str, size_t size )
{
    uint32_t i;

    for ( i = 0; i < ( uint32_t ) size; ++ i )
    {
        uint32_t ch = str [ i ];
        if ( str [ i ] < 0 && map [ ch & 0x7F ] == 0 )
            break;
    }

    return i;
}

LIB_EXPORT uint32_t CC iso8859_string_measure ( const uint32_t map [ 128 ],
    const char *str, size_t *size )
{
    uint32_t i, ch;

    for ( ch = str [ i = 0 ]; ch != 0; ch = str [ ++ i ] )
    {
        if ( ( int ) ch < 0 && map [ ch & 0x7F ] == 0 )
            break;
    }

    * size = i;
    return i;
}

/* conversion from ISO-8859-x to internal standard */
LIB_EXPORT uint32_t CC iso8859_cvt_string_len ( const uint32_t map [ 128 ],
    const char *src, size_t src_size, size_t *dst_size )
{
    size_t size;
    uint32_t i;

    for ( i = 0, size = 0; i < ( uint32_t ) src_size; ++ i )
    {
        uint32_t ch = src [ i ];
        if ( src [ i ] < 0 )
        {
            int ch_len;
            char ignore [ 8 ];

            ch = map [ ch & 0x7F ];
            if ( ch == 0 )
                break;

            ch_len = utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch );
            if ( ch_len <= 0 )
                break;

            size += ch_len;
        }
        else
        {
            ++ size;
        }
    }

    * dst_size = size;

    return i;
}

LIB_EXPORT uint32_t CC iso8859_cvt_string_measure ( const uint32_t map [ 128 ],
    const char *src, size_t *src_size, size_t *dst_size )
{
    size_t size;
    uint32_t i, ch;

    for ( ch = src [ i = 0 ], size = 0; ch != 0; ch = src [ ++ i ] )
    {
        if ( ( int ) ch < 0 )
        {
            int ch_len;
            char ignore [ 8 ];

            ch = map [ ch & 0x7F ];
            if ( ch == 0 )
                break;

            ch_len = utf32_utf8 ( ignore, & ignore [ sizeof ignore ], ch );
            if ( ch_len <= 0 )
                break;

            size += ch_len;
        }
        else
        {
            ++ size;
        }
    }

    * src_size = ( size_t ) i;
    * dst_size = size;

    return i;
}

LIB_EXPORT size_t CC iso8859_cvt_string_copy ( const uint32_t map [ 128 ],
    char *dst, size_t dst_size, const char *src, size_t src_size )
{
    char *begin = dst;
    char *dend = dst + dst_size;
    const char *send = src + src_size;

    while ( dst < dend && src < send )
    {
        int ch_len;
        uint32_t ch = * src ++;
        if ( ( int ) ch < 0 )
        {
            ch = map [ ch & 0x7F ];
            if ( ch == 0 )
                break;
        }
        ch_len = utf32_utf8 ( dst, dend, ch );
        if ( ch_len <= 0 )
            break;
        dst += ch_len;
    }

    if ( dst < dend )
        * dst = 0;
    return ( size_t ) ( dst - begin );
}
