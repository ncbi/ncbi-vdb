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

#ifndef _h_unix_native_
#include "../unix/unix-native.h"
#endif

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * strdup - declared unless _ANSI_SOURCE is defined - redeclare anyway
 * strndup - implemented inline here
 */
char *strdup ( const char *str );

#if !defined(__MAC_10_6) || (__MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_6)
static __inline__
char *strndup ( const char *str, size_t n )
{
    char *dupstr;

    const char *end = ( const char* ) memchr ( str, 0, n );
    if ( end != NULL )
        n = end - str;

    dupstr = (char*)malloc ( n + 1 );
    if ( dupstr != NULL )
    {
        memmove ( dupstr, str, n );
        dupstr [ n ] = 0;
    }

    return dupstr;
}
#endif

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

/*--------------------------------------------------------------------------
 * memchr - implemented inline here
 */
static __inline__
void *memrchr ( const void *s, int c, size_t n )
{
    size_t i;
    const char *cp = (const char*)s;
    for ( i = n; i > 0; )
    {
        if ( ( int ) cp [ -- i ] == c )
            return ( void* ) & cp [ i ];
    }
    return NULL;
}


/*--------------------------------------------------------------------------
 * strtoll - declared unless _ANSI_SOURCE is defined - redeclare anyway
 * strtoul - older includes were not ready for c99
 *
 *  NB - the define __DARWIN_NO_LONG_LONG will be true for
 *       cases when long long would be int64_t.
 */
#if __DARWIN_NO_LONG_LONG
int64_t strtoll ( const char *s, char **end, int base );
uint64_t strtoull ( const char *s, char **end, int base );
#endif

#ifdef __cplusplus
}
#endif

#endif /* _h_os_native_ */
