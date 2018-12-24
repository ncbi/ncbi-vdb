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
#include <klib/time.h>
#include <klib/rc.h> /* RC */

#include <unistd.h>
#include <stdlib.h>
#include <string.h> /* memset */
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>


/*--------------------------------------------------------------------------
 * KTime_t
 *  64 bit time_t
 */


/* Stamp
 *  current timestamp
 */
LIB_EXPORT KTime_t CC KTimeStamp ( void )
{
    return time ( NULL );
}

LIB_EXPORT KTimeMs_t CC KTimeMsStamp ( void )
{
    struct timeval tm;
    gettimeofday( &tm, NULL );
    return ( ( tm.tv_sec * 1000 ) + ( tm.tv_usec / 1000 ) );
}

/*--------------------------------------------------------------------------
 * KTime
 *  simple time structure
 */


/* Make
 *  make KTime from struct tm
 */
static
void KTimeMake ( KTime *kt, struct tm const *t )
{
    kt -> year = t -> tm_year + 1900;
    kt -> month = t -> tm_mon;
    kt -> day = t -> tm_mday - 1;
    kt -> weekday = t -> tm_wday;
#if !defined(__SunOS)  &&  !defined(__sun__)
    kt -> tzoff = ( int16_t ) ( t -> tm_gmtoff / 60 );
#endif
    kt -> hour = ( uint8_t ) t -> tm_hour;
    kt -> minute = ( uint8_t ) t -> tm_min;
    kt -> second = ( uint8_t ) t -> tm_sec;
    kt -> dst = t -> tm_isdst != 0;
}


/* Local
 *  populate "kt" from "ts" in local time zone
 */
LIB_EXPORT const KTime* CC KTimeLocal ( KTime *kt, KTime_t ts )
{
    if ( kt != NULL )
    {
        struct tm t;
        time_t unix_time = ( time_t ) ts;
        localtime_r ( & unix_time, & t );
        KTimeMake ( kt, & t );
    }
    return kt;
}


/* Global
 *  populate "kt" from "ts" in GMT
 */
LIB_EXPORT const KTime* CC KTimeGlobal ( KTime *kt, KTime_t ts )
{
    if ( kt != NULL )
    {
        struct tm t;
        time_t unix_time = ( time_t ) ts;
        gmtime_r ( & unix_time, & t );
        KTimeMake ( kt, & t );
    }
    return kt;
}


/* MakeTime
 *  make a KTime_t from KTime
 */
LIB_EXPORT KTime_t CC KTimeMakeTime ( const KTime *self )
{
    KTime_t ts = 0;

    if ( self != NULL )
    {
        struct tm t;

        assert ( self -> year >= 1900 ); // TODO
        t . tm_year = self -> year - 1900;
        t . tm_mon = self -> month;
        t . tm_mday = self -> day + 1;
        t . tm_wday = self -> weekday;
#if !defined(__SunOS)  &&  !defined(__sun__)
        t . tm_gmtoff = self -> tzoff * 60; 
#endif
        t . tm_hour = self -> hour;
        t . tm_min = self -> minute;
        t . tm_sec = self -> second;
        t . tm_isdst = self -> dst;

        ts = mktime ( &t );
        ts -= timezone;
    }

    return ts;
}


LIB_EXPORT const KTime* CC KTimeFromIso8601 ( KTime *kt, const char * s,
    size_t size )
{
    struct tm t;

    const char * c = NULL;

    if ( kt == NULL || s == NULL )
        return NULL;

    memset ( & t, 0, sizeof t );

    if ( size == 19 )
        c = strptime ( s, "%Y-%m-%dT%H:%M:%S", & t );
    else if ( size == 20 )
        c = strptime ( s, "%Y-%m-%dT%H:%M:%S%z", & t );
    else
        return NULL;

    if ( c != NULL && c - s != size )
        return NULL;

    memset ( kt, 0, sizeof * kt );
    KTimeMake ( kt, & t );

    return kt;
}


/* Iso8601
 *  populate "s" from "ks" according to ISO-8601:
 *         YYYY-MM-DDThh:mm:ssTZD
 */
KLIB_EXTERN size_t CC KTimeIso8601 ( KTime_t ts, char * s, size_t size ) {
    const KTime * r = NULL;
    KTime now;

    time_t unix_time = ( time_t ) ts;
    struct tm t;

    if ( ts == 0 || s == NULL || size == 0 )
        return 0;

    r = KTimeGlobal ( & now, ts );
    if ( r == NULL )
        return 0;

    gmtime_r ( & unix_time, & t );
    return strftime ( s, size, "%FT%TZ", & t );
}


/* Iso8601
*  populate "s" from "ks" according to RFC 2616:
*         Sun Nov 6 08:49:37 1994 +0000 ; ANSI C's asctime() format
*
* https://www.ietf.org/rfc/rfc2616.txt 3.3.1 Full Date
*
* https://www.ietf.org/rfc/rfc2822.txt 3.3. Date and Time Specification
*     The form "+0000" SHOULD be used to indicate a time zone at Universal Time.
*/
KLIB_EXTERN size_t CC KTimeRfc2616(KTime_t ts, char * s, size_t size) {
    const KTime * r = NULL;
    KTime now;

    time_t unix_time = (time_t)ts;
    struct tm t;

    if (ts == 0 || s == NULL || size == 0)
        return 0;

    r = KTimeGlobal(&now, ts);
    if (r == NULL)
        return 0;

    gmtime_r(&unix_time, &t);
    return strftime(s, size, "%a, %d %b %Y %H:%M:%S +0000", &t);
}


LIB_EXPORT rc_t CC KSleepMs(uint32_t milliseconds) {
    struct timespec time;

    time.tv_sec = (milliseconds / 1000);
    time.tv_nsec = (milliseconds % 1000) * 1000 * 1000;

    if ( nanosleep ( & time, NULL ) != 0 )
    {
        switch ( errno )
        {
        case EINTR:
            return SILENT_RC(rcRuntime, rcTimeout, rcWaiting, rcTimeout, rcInterrupted);
        default:
            return RC(rcRuntime, rcTimeout, rcWaiting, rcParam, rcInvalid);
        }
    }

    return 0;
}
