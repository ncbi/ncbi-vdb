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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
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
#if 0
    kt -> tzoff = ( int16_t ) ( t -> tm_gmtoff / 60 );
#else
    kt -> tzoff = 0;
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
        t = * localtime ( & unix_time );
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
        t = * gmtime ( & unix_time );
        KTimeMake ( kt, & t );
    }
    return kt;
}
