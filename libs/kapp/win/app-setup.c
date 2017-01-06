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

static wchar_t const *stripCigDrive(wchar_t const *const arg)
{
    static wchar_t const cigdrive[] = L"/cigdrive/";
    int offset;
    
    for (offset = 0; offset < 10; ++offset) {
        if (arg[offset] != cigdrive[offset])
            return arg;
    }
    return arg + 9;
}

/* if dst is NULL, no conversion is done, but everything else is */
static int convertArg(char dst[], wchar_t const warg[])
{
    size_t src_size = 0, dst_size = 0;
    wchar_t const *const arg = stripCigDrive(warg);
    wchar_t dummy[1];
    DWORD len = GetFullPathNameW(arg, 0, dummy, NULL);
    wchar_t *full = malloc(len);
    if (len > 0 && full != NULL) {
        GetFullPathNameW(arg, len, full, NULL);
        wchar_cvt_string_measure(full, &src_size, &dst_size);
        if (dst) {
            wchar_cvt_string_copy(dst, dst_size + 1, full, src_size);
            
            /* convert drive letter "X:" to "/X" */
            if (dst[1] == ':') {
                dst[1] = arg[0];
                dst[0] = '/';
            }
        }
        free(full);
    }
    else {
        /* something is unrecoverably wrong */
        abort();
    }
    if (dst) {
        int i;
        
        /* convert backslashes */
        for (i = 0; i < dst_size; ++i) {
            if (dst[i] == '\\')
                dst[i] = '/';
        }
        dst[dst_size] = '\0';
    }
    return (int)dst_size + 1;
}

/* total bytes needed to convert argv to utf-8 */
static int measureArgs(int argc, wchar_t *argv[])
{
    int i;
    int totalLen = 0;
    
    for (i = 0; i < argc; ++i) {
        totalLen += convertArg(NULL, argv[i]);
    }
    return totalLen;
}

rc_t CC KAppRun(int argc, char *argv[], ver_t vers, rc_t (CC *)(int argc, char *argv[]) umain)
{
    rc_t rc = 0;
    wchar_t **wargv = (wchar_t **)&argv[0];
    
    CoInitialize(NULL);
    
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)Our_HandlerRoutine, TRUE);
    {
        char **const argv = malloc((argc + 1) * sizeof(char *) + measureArgs(argc, wargv));
        char *const strings = (char *)(argv + argc + 1);
        if (argv) {
            int j;
            int i;
            int len = 0;
            
            for (i = j = 0; i < argc; ++i, j += len) {
                argv[i] = strings + j;
                len = convertArg(argv[i], wargv[i]);
            }
            argv[argc] = NULL;
        }
        else {
            /* something is unrecoverably wrong */
            abort();
        }
        rc = KAppSetup(argc, argv, vers);
        if (rc == 0) {
            rc = umain(argc, argv);
            KAppTeardown(rc);
        }
        free(argv);
    }
    CoUninitialize();
    return rc;
}
