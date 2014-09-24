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

#ifndef _h_strtol_
#define _h_strtol_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * strtoi32
 * strtoi32
 *  based upon actual usage
 */
#define strtoi32( str, endp, base ) \
    ( int32_t ) strtol ( str, endp, base )

#define strtou32( str, endp, base ) \
    ( uint32_t ) strtoul ( str, endp, base )


/*--------------------------------------------------------------------------
 * strtoi64
 * strtoi64
 *  based upon actual usage
 */
#define strtoi64( str, endp, base ) \
    _strtoi64 ( str, endp, base )

#define strtou64( str, endp, base ) \
    _strtoui64 ( str, endp, base )
    
#if 0
/*this implementation does not report overflow... */
static __inline
__int64 strtoi64( const char* str, char** endp, uint32_t base )
{
    int i = 0;
    __int64 ret_value = 0;

    if ( str != NULL && base != 1 && base <= 36 )
    {
        bool negate = false;

        for ( ; isspace( str [ i ] ); ++ i )
            ( void ) 0;

        switch ( str [ i ] )
        {
        case '-':
            negate = true;
        case '+':
            ++ i;
            break;
        }

        if ( base == 0 )
        {
            if ( str [ i ] != '0' )
                base = 10;
            else if ( tolower ( str [ i + 1 ] == 'x' ) )
            {
                base = 16;
                i += 2;
            }
            else
            {
                base = 8;
                i += 1;
            }
        }

        if ( base <= 10 )
        {
            for ( ; isdigit ( str [ i ] ); ++ i )
            {
                uint32_t digit = str [ i ] - '0';
                if ( digit >= base )
                    break;
                ret_value *= base;
                ret_value += digit;
            }
        }
        else
        {
            for ( ; ; ++ i )
            {
                if ( isdigit ( str [ i ] ) )
                {
                    ret_value *= base;
                    ret_value += str [ i ] - '0';
                }
                else if ( ! isalpha ( str [ i ] ) )
                    break;
                else
                {
                    uint32_t digit = toupper ( str [ i ] ) - 'A' + 10;
                    if ( digit >= base )
                        break;
                    ret_value *= base;
                    ret_value += digit;
                }
            }
        }

        if ( negate )
            ret_value = - ret_value;
    }

    if ( endp != NULL )
        * endp = (char *)str + i;

    return ret_value;
}

static __inline
unsigned __int64 strtou64( const char* str, char** endp, uint32_t base )
{
    return strtoi64( str, endp, base );
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _h_strtol_ */
