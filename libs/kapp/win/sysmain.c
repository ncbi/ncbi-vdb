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

#define UNICODE 1
#define _UNICODE 1

#include "../main-priv.h"
#include <sysalloc.h>
#include <kapp/main.h>
#include <klib/log.h>
#include <klib/status.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/report.h> 
#include <atomic32.h>

/* #define _WIN32_WINNT 0x0500 */
/* commented out: 10/21/2010 by wolfgang
   reason: Kurt introduced in sysdll.c a new functionality
   which requires a newer windows-version 
   (i realized it as compiler parameter in build/Makefile.vc++) */

#include <WINDOWS.H>
#include <OBJBASE.H>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <os-native.h>

/*--------------------------------------------------------------------------
 * Main
 */

static bool no_hup;
static atomic32_t hangup;
static atomic32_t quitting;

/* Quitting
 *  is the program supposed to exit
 */
rc_t CC Quitting ( void )
{
    if ( atomic32_read ( & quitting ) == 0 )
        return 0;
    LOGMSG ( klogInfo, "EXITING..." );
    return RC ( rcExe, rcProcess, rcExecuting, rcProcess, rcCanceled );
}

/* SignalQuit
 *  tell the program to quit
 */
rc_t CC SignalQuit ( void )
{
    return RC ( rcExe, rcProcess, rcSignaling, rcNoObj, rcUnknown );
}

/* Hangup
 *  has the program received a SIGHUP
 */
rc_t CC Hangup ( void )
{
    if ( atomic32_read ( & hangup ) == 0 )
        return 0;
    LOGMSG ( klogInfo, "HANGUP...\n" );
    return RC ( rcExe, rcProcess, rcExecuting, rcProcess, rcIncomplete );
}

/* SignalHup
 *  send the program a SIGHUP
 */
rc_t CC SignalHup ( void )
{
    return RC ( rcExe, rcProcess, rcSignaling, rcNoObj, rcUnknown );
}

/* SignalNoHup
 *  tell the program to stay alive even after SIGHUP
 */
rc_t CC SignalNoHup ( void )
{
    no_hup = true;
    return 0;
}


BOOL CC Our_HandlerRoutine( DWORD dwCtrlType )
{
    BOOL res = FALSE;
    switch( dwCtrlType )
    {
    case CTRL_C_EVENT : ReportSilence ();
                        atomic32_inc ( & quitting );
                        res = TRUE;
                        break;
    }
    return res;
}


/* main
 *  Windows specific main entrypoint
 */
static
int main2 ( int argc, char *argv [] )
{
    rc_t rc;

    SetConsoleCtrlHandler( ( PHANDLER_ROUTINE ) Our_HandlerRoutine, TRUE );

    /* run this guy */
    rc = KMane ( argc, argv );

    return ( rc == 0 ) ? 0 : 3;
}

static
char *rewrite_arg ( const wchar_t *arg )
{
    char *utf8;
    bool has_drive = false;
    size_t i, src_size, dst_size;
    DWORD len;

    /* detect drive or full path */
    wchar_t rewrit [ MAX_PATH ];
    if ( arg [ 0 ] < 128 )
    {
        bool rewrite = false;

        /* look for non-drive path */
        if ( arg [ 0 ] == '\\' || arg [ 0 ] == '/' )
        {
            /* full path - not network */
            if ( arg [ 1 ] != '\\' && arg [ 1 ] != '/' )
            {
                /* check for cygdrive */
                if ( memcmp( arg, L"/cygdrive/", sizeof L"/cygdrive/" - sizeof L"" ) == 0 )
                    arg += sizeof "/cygdrive" - 1;
                else
                    rewrite = true;
             
            }
            
        }
        /* look for drive path */
        else if ( isalpha ( arg [ 0 ] ) && arg [ 1 ] == ':' )
        {
            has_drive = true;

            /* look for drive relative */
            if ( arg [ 2 ] != '\\' && arg [ 2 ] != '/' )
                rewrite = true;
        }
        if ( rewrite )
        {
            /* incomplete path */
            len = GetFullPathNameW ( arg, sizeof rewrit / sizeof rewrit [ 0 ], rewrit, NULL );
            if ( len == 0 || len >= MAX_PATH )
            {
                /* complain */
                return NULL;
            }
            arg = rewrit;

            has_drive = ( isalpha ( arg [ 0 ] ) && arg [ 1 ] == ':' );
        }
    }

    /* this point, we should only have normal arguments,
       or network/full/drive-full/relative paths */

    /* measure the string */
    len = wchar_cvt_string_measure ( arg, & src_size, & dst_size );

    /* allocate a UTF-8 buffer */
    utf8 = malloc ( dst_size + 1 );
    if ( utf8 != NULL )
    {
        /* normal arguments get no offsets */
        uint32_t offset = 0;

        /* check for need to convert drive */
        if ( has_drive )
        {
            /* convert to pseudo mount point */
            utf8 [ 0 ] = '/';
            utf8 [ 1 ] = ( char ) arg [ 0 ];
            offset = 2;
        }

        /* copy the wide argument to utf8 */
        wchar_cvt_string_copy ( & utf8 [ offset ], dst_size - offset + 1,
            & arg [ offset ], src_size - offset * sizeof * arg );

        /* terminate the string */
        utf8 [ dst_size ] = 0;

        /* map all backslashes to fwdslashes */
        for ( i = 0; i < dst_size; ++ i )
        {
            if ( utf8 [ i ] == '\\' )
                utf8 [ i ] = '/';
        }
    }

    return utf8;
}


int __cdecl wmain ( int argc, wchar_t *wargv [], wchar_t *envp [] )
{
    char **argv;
    int i, status;

    /* must initialize COM... must initialize COM... */
    /* CoInitializeEx ( NULL, COINIT_MULTITHREADED ); */
    CoInitialize(NULL);

    /* create a copy of args */
    argv = calloc ( argc + 1, sizeof * argv );
    if ( argv == NULL )
        status = 5;
    else
    {
        /* convert wchar_t arguments to UTF-8
           rewriting anything that looks like a path */
        for ( i = 0; i < argc; ++ i )
        {
            argv [ i ] = rewrite_arg ( wargv [ i ] );
            if ( argv [ i ] == NULL )
                break;
        }

        /* perform normal main operations on UTF-8 with POSIX-style paths */
        if ( i == argc )
            status = main2 ( argc, argv );

        /* tear down argv */
        while ( -- i >= 0 )
            free ( argv [ i ] );
        free ( argv );
    }

    /* balance the COM initialization */
    CoUninitialize ();

    return status;
}
