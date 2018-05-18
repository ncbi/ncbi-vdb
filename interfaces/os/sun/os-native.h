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

#ifndef _h_os_native_
#define _h_os_native_

#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

#ifndef _h_unix_native_
#include "../unix/unix-native.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __inline__
#define __inline__ inline
#endif

/*--------------------------------------------------------------------------
 * index
 *  we should get rid of this entirely
 */
#define index strchr


#ifndef strndupa /* Solaris 11 adds some of these */
/*--------------------------------------------------------------------------
 * strndup - implemented inline here
 */
static __inline__
char *strndup ( const char *str, size_t n )
{
    char *dupstr;

    const char *end = ( const char* ) memchr ( str, 0, n );
    if ( end != NULL )
        n = end - str;

    dupstr = ( char* ) malloc ( n + 1 );
    if ( dupstr != NULL )
    {
        memmove ( dupstr, str, n );
        dupstr [ n ] = 0;
    }

    return dupstr;
}

/*--------------------------------------------------------------------------
 * strchrnul - implemented inline here
 */
static __inline__
char *strchrnul ( const char *str, int c )
{
    int i;
    for ( i = 0; str [ i ] != 0 && str [ i ] != c; ++i )
        ( void ) 0;
    return & ( ( char* ) str ) [ i ];
}
#endif

/*--------------------------------------------------------------------------
 * memchr - implemented inline here
 */
static __inline__
void *memrchr ( const void *s, int c, size_t n )
{
    size_t i;
    const char *cp = ( const char* ) s;
    for ( i = n; i > 0; )
    {
        if ( ( int ) cp [ -- i ] == c )
            return ( void* ) & cp [ i ];
    }
    return NULL;
}

#ifndef strndupa
static __inline__
char *strsep ( char **stringp, const char *delim )
{
    char *s, *tok, c, delim_char;
    const char *p_delim;

    if ( ( s = *stringp ) == NULL )
        return NULL;

    for ( tok = s; ; )
    {
        c = *s++;
	p_delim = delim;
	do
	{
	    if ( ( delim_char = *p_delim++ ) == c )
	    {
	        if ( c == 0 )
		    s = NULL;
		else
		  s[-1] = 0;
		*stringp = s;
		return ( tok );
	    }
	} while ( delim_char != 0 );
    }
}

static __inline__
const char *strcasestr (const char *s1, const char *s2)
{
    unsigned char c2 = tolower((unsigned char) *s2);
    size_t l1 = strlen(s1), l2 = strlen(s2);
    
    if (l2 == 0) {
        return s1;
    }

    while (l1 >= l2) {
        if (tolower((unsigned char) *s1) == c2
            &&  (l2 == 1  ||  strncasecmp(s1 + 1, s2 + 1, l2 - 1) == 0)) {
            return s1;
        }
        ++s1;
        --l1;
    }

    return NULL;
}
#endif

static __inline__
time_t timegm ( struct tm *gmt )
{
    /* XXX - extend past 2099.  (2100 isn't a leap year.) */
    static const int days_so_far [ 12 ] =
    {
        0,  31,  59,  90, 120, 151,
        181, 212, 243, 273, 304, 334
    };
    int days = ( ( gmt -> tm_year - 70 ) * 365 +
                 ( ( gmt -> tm_year - 68 ) >> 2 ) +
                 days_so_far [ gmt -> tm_mon ] + gmt -> tm_mday
        );
    if ( gmt -> tm_year % 4 == 0 && gmt -> tm_mon < 2 )
        -- days;

    return ( ( days * 24 + gmt -> tm_hour ) * 60 + gmt -> tm_min ) * 60 + gmt -> tm_sec;
}

#ifdef __cplusplus
}
#endif

#endif /* _h_os_native_ */
