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

#include<klib/printf.h> /* string_printf */
#include <klib/rc.h> /* SILENT_RC */
#include <klib/time.h>

/* do not include windows.h, it is included already by os-native.h */
#include <os-native.h>
#include <time.h>


/*--------------------------------------------------------------------------
 * KTime_t
 *  64 bit time_t
 */

#if _ARCH_BITS == 32
#define UNIX_EPOCH_IN_WIN       116444736000000000ULL
#else
#define UNIX_EPOCH_IN_WIN       116444736000000000UL
#endif
#define UNIX_TIME_UNITS_IN_WIN  10000000
#define MS_TIME_UNITS_IN_WIN  	10000

/* KTime2FILETIME
 *  convert from Unix to Windows
 */
static
const FILETIME *KTime2FILETIME ( KTime_t ts, FILETIME *ft )
{
    uint64_t win_time = ( ts * UNIX_TIME_UNITS_IN_WIN ) + UNIX_EPOCH_IN_WIN;
    ft -> dwLowDateTime = ( DWORD ) win_time;
    ft -> dwHighDateTime = win_time >> 32;
    return ft;
}

/* FILETIME2KTime
 */
static
KTime_t FILETIME2KTime ( const FILETIME *ft )
{
    uint64_t win_time = ft -> dwLowDateTime + ( ( int64_t ) ft -> dwHighDateTime << 32 );
    return ( KTime_t ) ( win_time - UNIX_EPOCH_IN_WIN ) / UNIX_TIME_UNITS_IN_WIN;
}

/* FILETIME2KTimeMs
 */
static
KTimeMs_t FILETIME2KTimeMs ( const FILETIME *ft )
{
    uint64_t win_time = ft -> dwLowDateTime + ( ( int64_t ) ft -> dwHighDateTime << 32 );
    return ( KTimeMs_t ) ( win_time - UNIX_EPOCH_IN_WIN ) / MS_TIME_UNITS_IN_WIN;
}


/* Stamp
 *  current timestamp
 */
LIB_EXPORT KTime_t CC KTimeStamp ( void )
{
    FILETIME ft;
    GetSystemTimeAsFileTime ( & ft );
    return FILETIME2KTime ( & ft );
}

LIB_EXPORT KTimeMs_t CC KTimeMsStamp ( void )
{
    FILETIME ft;
    GetSystemTimeAsFileTime ( & ft );
    return FILETIME2KTimeMs ( & ft );
}

/*--------------------------------------------------------------------------
 * SYSTEMTIME
 */

static
int SYSTEMTIME_compare ( const SYSTEMTIME *a, const SYSTEMTIME *b )
{
    int diff = a -> wMonth - b -> wMonth;
    if ( diff == 0 )
    {
        diff = a -> wDay - b -> wDay;
        if ( diff == 0 )
        {
            diff = a -> wHour - b -> wHour;
            if ( diff == 0 )
            {
                diff = a -> wMinute - b -> wMinute;
                if ( diff == 0 )
                    diff = a -> wSecond - b -> wSecond;
            }
        }
    }
    return diff;
}

static
void SYSTEMTIME_from_half_baked_SYSTEMTIME ( const SYSTEMTIME *half_baked, SYSTEMTIME *proper, WORD year )
{
    FILETIME ft;

    * proper = * half_baked;

    /* fix some stuff */
    proper -> wYear = year;
    proper -> wMonth = half_baked -> wMonth;
    proper -> wDayOfWeek = 0; /* ignored */
    proper -> wDay = 1;
    proper -> wHour = half_baked -> wHour;
    proper -> wMinute = 0;
    proper -> wSecond = 0;
    proper -> wMilliseconds = 0;

    /* convert it to FILETIME and back, just to get the proper day of week
       if there's a better way to do it, go ahead.
       by now, my lunch is too difficult to keep down... */
    SystemTimeToFileTime ( proper, & ft );
    FileTimeToSystemTime ( & ft, proper );

    /* now, move ahead to the day of week */
    proper -> wDay += half_baked -> wDayOfWeek - proper -> wDayOfWeek;
    if ( half_baked -> wDayOfWeek < proper -> wDayOfWeek )
        proper -> wDay += 7;
    proper -> wDayOfWeek = half_baked -> wDayOfWeek;

    /* now find the occurrence of the weekday */
    if ( half_baked -> wDay > 1 )
        proper -> wDay += ( half_baked -> wDay - 1 ) * 7;
}

/*--------------------------------------------------------------------------
 * KTime
 *  simple time structure
 */


/* Make
 *  make KTime from struct tm
 */
static
void KTimeMake ( KTime *kt, const SYSTEMTIME *st )
{
    kt -> year = st -> wYear;
    kt -> month = st -> wMonth - 1;
    kt -> day = st -> wDay - 1;
    kt -> weekday = st -> wDayOfWeek;
    kt -> hour = ( uint8_t ) st -> wHour;
    kt -> minute = ( uint8_t ) st -> wMinute;
    kt -> second = ( uint8_t ) st -> wSecond;
}


/* Local
 *  populate "kt" from "ts" in local time zone
 */
LIB_EXPORT const KTime* CC KTimeLocal ( KTime *kt, KTime_t ts )
{
    if ( kt != NULL )
    {
        DWORD tz_id;
        FILETIME ft;
        SYSTEMTIME gst, lst;
        TIME_ZONE_INFORMATION tz;

        /* generate windows time in 100nS units */
        KTime2FILETIME ( ts, & ft );

        /* generate a system time - almost what we need,
           except it's GMT and has no associated time zone */
        FileTimeToSystemTime ( & ft, & gst );

        /* assume we're NOT in DST */
        kt -> dst = false;

        /* get local timezone information */
        tz_id = GetTimeZoneInformation ( & tz );
        switch ( tz_id )
        {
        case TIME_ZONE_ID_STANDARD:
        case TIME_ZONE_ID_DAYLIGHT:

            /* convert GMT time to local time with tz info */
            SystemTimeToTzSpecificLocalTime ( & tz, & gst, & lst );
            KTimeMake ( kt, & lst );

            /* our gentle brothers and sisters in Redmond never
               cease to amaze... it's very nice - handy, even -
               to know that the system is "currently" operating
               in one mode or another, but that tells us nothing
               about the timestamp we're trying to interpret.

               to discover whether the timestamp we're converting
               is within daylight savings time, we can compare against
               the two railpost SYSTEMTIME entries, but then there's
               no telling whether we're in or out since the calendar
               is circular! aside from having to perform a multi-part
               comparison, we'll come to different conclusions depending
               upon whether the hemisphere is northern or southern!

               to disambiguate, we can use tz_id to detect hemisphere,
               and then know what's going on. Wow.

               also, it's not clear to the author whether the returned
               structures in tz will be proper or hacked, since the
               MSDN descriptions only describe how to hack them for
               input, but not how they will look on output. */

            if ( tz . StandardDate . wMonth == 0 || tz . DaylightDate . wMonth == 0 )
                kt -> tzoff = - ( int16_t ) tz . Bias;
            else
            {
                bool south = tz_id == TIME_ZONE_ID_DAYLIGHT;

                SYSTEMTIME cst, dst, std;
                GetSystemTime ( & cst );

                /* fill out proper structures, since those in tz are bad... */
                SYSTEMTIME_from_half_baked_SYSTEMTIME ( & tz . DaylightDate, & dst, cst . wYear );
                SYSTEMTIME_from_half_baked_SYSTEMTIME ( & tz . StandardDate, & std, cst . wYear );

                /* perform northern test for DST */
                if ( SYSTEMTIME_compare ( & lst, & dst ) >= 0 && SYSTEMTIME_compare ( & lst, & std ) < 0 )
                    kt -> dst = true;

                /* test to see which hemisphere */
                south ^= ( SYSTEMTIME_compare ( & cst, & dst ) >= 0 && SYSTEMTIME_compare ( & cst, & std ) < 0 );

                /* correct for southern hemisphere */
                kt -> dst ^= south;

                /* set the timezone offset */
                kt -> tzoff = - ( int16_t ) ( tz . Bias +
                    kt -> dst ? tz . DaylightBias : tz . StandardBias );
            }
            break;

        default:

            /* failed - use GMT instead */
            KTimeMake ( kt, & gst );
            kt -> tzoff = 0;
        }
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
        FILETIME ft;
        SYSTEMTIME gst;

        /* generate windows time in 100nS units */
        KTime2FILETIME ( ts, & ft );

        /* generate a system time */
        FileTimeToSystemTime ( & ft, & gst );

	/* fill out GMT time structure */
        KTimeMake ( kt, & gst );
        kt -> tzoff = 0;
        kt -> dst = false;
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
#if USE_WINDOWS_NATIVE
        FILETIME ft;
        SYSTEMTIME st;
        
        st . wYear = self -> year;
        st . wMonth = self -> month + 1;
        st . wDay = self -> day + 1;
        st . wDayOfWeek = self -> weekday;
        st . wHour = self -> hour;
        st . wMinute = self -> minute;
        st . wSecond = self -> second;

#if 0        
        kt -> year = st -> wYear;
        kt -> month = st -> wMonth - 1;
        kt -> day = st -> wDay - 1;
        kt -> weekday = st -> wDayOfWeek;
        kt -> hour = ( uint8_t ) st -> wHour;
        kt -> minute = ( uint8_t ) st -> wMinute;
        kt -> second = ( uint8_t ) st -> wSecond;
#endif

        SystemTimeToFileTime ( & st, & ft );
#error "TBD - convert ft to seconds"

#else /* USE_WINDOWS_NATIVE */
        struct tm t;

        assert ( self -> year >= 1900 );
        t . tm_year = self -> year - 1900;
        t . tm_mon = self -> month;
        t . tm_mday = self -> day;
        t . tm_wday = self -> weekday;
        t . tm_hour = self -> hour;
        t . tm_min = self -> minute;
        t . tm_sec = self -> second;
        t . tm_isdst = self -> dst;

        ts = _mkgmtime( &t );
#endif /* USE_WINDOWS_NATIVE */ 
    }

    return ts;
}


/* YYYY-MM-DDThh:mm:ssTZD */
LIB_EXPORT size_t CC KTimeIso8601 ( KTime_t ts, char * s, size_t size )
{
    rc_t rc = 0;
    size_t num_writ = 0;

    const KTime * r = NULL;
    KTime ktime;

    time_t unix_time = ( time_t ) ts;
    struct tm t;

    if ( ts == 0 || s == NULL || size < 19 )
        return 0;

    r = KTimeGlobal ( & ktime, ts );
    if ( r == NULL )
        return 0;

    rc = string_printf ( s, size, & num_writ, "%04d-%02d-%02dT%02d:%02d:%02dZ",
        ktime . year, ktime . month + 1, ktime . day + 1,
        ktime . hour, ktime . minute, ktime . second );
    if ( rc == 0 )
        return num_writ;
    else if ( rc ==
          SILENT_RC ( rcText, rcString, rcConverting, rcBuffer, rcInsufficient )
        && num_writ == size )
    {
        return num_writ;
    }
    return 0;
}

static const char * month(uint16_t month) {
    switch (month) {
    case 0: return "Jan";
    case 1: return "Feb";
    case 2: return "Mar";
    case 3: return "Apr";
    case 4: return "May";
    case 5: return "Jun";
    case 6: return "Jul";
    case 7: return "Aug";
    case 8: return "Sep";
    case 9: return "Oct";
    case 10: return "Nov";
    case 11: return "Dec";
    default: return "";
    }
}

static const char * weekday(uint16_t weekday) {
    switch (weekday) {
    case 0: return "Sun";
    case 1: return "Mon";
    case 2: return "Tue";
    case 3: return "Wed";
    case 4: return "Thu";
    case 5: return "Fri";
    case 6: return "Sat";
    default: return "";
    }
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
    rc_t rc = 0;
    size_t num_writ = 0;

    const KTime * r = NULL;
    KTime ktime;

    time_t unix_time = (time_t)ts;
    struct tm t;

    if (ts == 0 || s == NULL || size < 19)
        return 0;

    r = KTimeGlobal(&ktime, ts);
    if (r == NULL)
        return 0;

    rc = string_printf(s, size, &num_writ,
        "%s, %02d %s %04d 02d:%02d:%02d +0000",
        weekday(ktime.weekday), ktime.day + 1, month(ktime.month), ktime.year,
        ktime.hour, ktime.minute, ktime.second);
    if (rc == 0)
        return num_writ;
    else if (rc ==
        SILENT_RC(rcText, rcString, rcConverting, rcBuffer, rcInsufficient)
        && num_writ == size)
    {
        return num_writ;
    }

    return 0;
}

LIB_EXPORT const KTime* CC KTimeFromIso8601 ( KTime *kt, const char * s,
                                              size_t size )
{
    int i = 0;
    int tmp = 0;
    char c = 0;

    if ( kt == NULL || s == NULL || size < 18 || size > 20 )
        return NULL;

    memset ( kt, 0, sizeof * kt );

    for ( i = 0, tmp = 0; i < 4; ++ i ) {
        char c = s [ i ];
        if ( ! isdigit ( c ) )
            return NULL;
        tmp = tmp * 10 + c - '0';
    }
    kt -> year = tmp;

    if ( s [ i ] != '-' )
        return NULL;

    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = c - '0';
    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = tmp * 10 + c - '0';
    if ( tmp == 0 || tmp > 12 )
        return NULL;
    kt -> month = tmp - 1;

    c = s [ ++ i ];
    if ( c != '-' )
        return NULL;

    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = c - '0';
    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = tmp * 10 + c - '0';
    if ( tmp == 0 || tmp > 31 )
        return NULL;
    kt -> day = tmp;

    c = s [ ++ i ];
    if ( c != 'T' )
        return NULL;

    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = c - '0';
    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = tmp * 10 + c - '0';
    if ( tmp > 23 )
        return NULL;
    kt -> hour = tmp;

    c = s [ ++ i ];
    if ( c != ':' )
        return NULL;

    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = c - '0';
    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = tmp * 10 + c - '0';
    if ( tmp > 59 )
        return NULL;
    kt -> minute = tmp;

    c = s [ ++ i ];
    if ( c != ':' )
        return NULL;

    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = c - '0';
    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = tmp * 10 + c - '0';
    if ( tmp > 59 )
        return NULL;
    kt -> second = tmp;

    if ( size > 19 ) {
        c = s [ ++ i ];
        if ( c != 'Z' )
            return NULL;
    }

    return kt;
}


LIB_EXPORT rc_t CC KSleepMs(uint32_t milliseconds) 
{
    Sleep ( milliseconds );
    return 0;
}
