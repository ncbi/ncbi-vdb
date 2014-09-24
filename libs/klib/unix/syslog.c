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
#include "log-priv.h"
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <errno.h>


/* LogTimestamp
 *  generates a timestamp string
 */

LIB_EXPORT rc_t CC LogTimestamp ( char *buffer, size_t bsize, size_t *num_writ )
{
    int len;
    static time_t last_time = 0;
    static struct tm cal;
    
    /* get current time */
    time_t t = time ( 0 );
    
    /* initialize time on first run */
    if ( ! last_time )
    {
        last_time = t;
        gmtime_r ( & last_time, & cal );
    }
    
    /* or update if time has passed */
    else if ( t != last_time )
    {
        /* update every 5 minutes or so */
        time_t dt = t - last_time;
        last_time = t;
        if ( dt >= 300 )
            gmtime_r ( & last_time, & cal );
	
        /* otherwise, just update the struct manually */
        else
        {
            /* advance seconds */
            dt += cal . tm_sec;
            cal . tm_sec = ( int ) ( dt % 60 );
	    
            /* detect a rolled-over minute */
            if ( ( dt /= 60 ) != 0 )
            {
                /* advance minutes */
                dt += cal . tm_min;
                cal . tm_min = ( int ) ( dt % 60 );
		
                /* detect a rolled-over hour */
                if ( ( dt /= 60 ) != 0 )
                {
                    /* roll-over of an hour - refetch */
                    gmtime_r ( & last_time, & cal );
                }
            }
        }
    }
    
    /* make the timestamp */
    len = snprintf ( buffer, bsize,
                     "%04d-%02d-%02dT%02d:%02d:%02d"
                     , cal . tm_year + 1900
                     , cal . tm_mon + 1
                     , cal . tm_mday
                     , cal . tm_hour
                     , cal . tm_min
                     , cal . tm_sec
        );

    if ( num_writ != NULL )
        * num_writ = len;

    if ( len < 0 || ( size_t ) len >= bsize )
    {
        if ( len < 0 && num_writ != NULL )
            * num_writ = 0;
        return RC ( rcApp, rcLog, rcLogging, rcBuffer, rcInsufficient );
    }

    return 0;
}

/* LogSimpleTimestamp
 *  generates a local timestamp string without time zone
 */
LIB_EXPORT rc_t CC LogSimpleTimestamp ( char *buffer, size_t bsize, size_t *num_writ )
{
    int len;
    static time_t last_time = 0;
    static struct tm cal;
    
    /* get current time */
    time_t t = time ( 0 );
    
    /* initialize time on first run */
    if ( ! last_time )
    {
        last_time = t;
        localtime_r ( & last_time, & cal );
    }
    
    /* or update if time has passed */
    else if ( t != last_time )
    {
        /* update every 5 minutes or so */
        time_t dt = t - last_time;
        last_time = t;
        if ( dt >= 300 )
            localtime_r ( & last_time, & cal );

        /* otherwise, just update the struct manually */
        else
        {
            /* advance seconds */
            dt += cal . tm_sec;
            cal . tm_sec = ( int ) ( dt % 60 );

            /* detect a rolled-over minute */
            if ( ( dt /= 60 ) != 0 )
            {
                /* advance minutes */
                dt += cal . tm_min;
                cal . tm_min = ( int ) ( dt % 60 );

                /* detect a rolled-over hour */
                if ( ( dt /= 60 ) != 0 )
                {
                    /* roll-over of an hour - refetch */
                    localtime_r ( & last_time, & cal );
                }
            }
        }
    }
    
    /* make the timestamp */
    len = snprintf ( buffer, bsize,
                     "%04d-%02d-%02dT%02d:%02d:%02d"
                     , cal . tm_year + 1900
                     , cal . tm_mon + 1
                     , cal . tm_mday
                     , cal . tm_hour
                     , cal . tm_min
                     , cal . tm_sec
        );
    if ( num_writ != NULL )
        * num_writ = len;

    if ( len < 0 || ( size_t ) len >= bsize )
    {
        if ( len < 0 && num_writ != NULL )
            * num_writ = 0;
        return RC ( rcApp, rcLog, rcLogging, rcBuffer, rcInsufficient );
    }

    return 0;
}

/* LogPID
 *  generates a process id
 */
LIB_EXPORT rc_t CC LogPID ( char *buffer, size_t bsize, size_t *num_writ )
{
    /* pid_t is signed not unsigned int */
    int len = snprintf ( buffer, bsize, "%d", getpid () );
    * num_writ = len;
    if ( len < 0 || ( size_t ) len >= bsize )
    {
        if ( len < 0 )
            * num_writ = 0;
        return RC ( rcApp, rcLog, rcLogging, rcBuffer, rcInsufficient );
    }
    return 0;
}

