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
#include "../log-priv.h"
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdio.h>
#include <stdlib.h>

/* do not include windows.h, it is included already by os-native.h */
#include <os-native.h>

/* LogTimestamp
 *  generates a timestamp-string in GMT-format
 */
LIB_EXPORT rc_t CC LogTimestamp ( char *buffer, size_t bsize, size_t *num_writ )
{
    int len;
    SYSTEMTIME my_time;
    
    GetSystemTime( &my_time );

    /* make the timestamp */
    len = snprintf ( buffer, bsize,
                     "%04d-%02d-%02dT%02d:%02d:%02d",
                     my_time.wYear,
                     my_time.wMonth,
                     my_time.wDay,
                     my_time.wHour,
                     my_time.wMinute,
                     my_time.wSecond );
    if ( num_writ != NULL )
    {
        * num_writ = len;
    }

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
    SYSTEMTIME my_time;

    GetLocalTime( &my_time );

    /* make the timestamp */
    len = snprintf ( buffer, bsize,
                     "%04d-%02d-%02dT%02d:%02d:%02d",
                     my_time.wYear,
                     my_time.wMonth,
                     my_time.wDay,
                     my_time.wHour,
                     my_time.wMinute,
                     my_time.wSecond );
    if ( num_writ != NULL )
    {
        * num_writ = len;
    }

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
    DWORD my_process_id = GetCurrentProcessId();
    int len = snprintf ( buffer, bsize, "%u", my_process_id );
    * num_writ = len;
    if ( len < 0 || ( size_t ) len >= bsize )
    {
        if ( len < 0 )
            * num_writ = 0;
        return RC ( rcApp, rcLog, rcLogging, rcBuffer, rcInsufficient );
    }
    return 0;
}
