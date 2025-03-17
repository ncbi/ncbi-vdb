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

#include "../main-priv.h"

#include <klib/rc.h>
#include <klib/text.h>

#include <WINDOWS.H>

/* SignalQuit
 *  tell the program to quit
 */
rc_t CC SignalQuit ( void )
{
    return RC ( rcExe, rcProcess, rcSignaling, rcNoObj, rcUnknown );
}

/* SignalHup
 *  send the program a SIGHUP
 */
rc_t CC SignalHup ( void )
{
    return RC ( rcExe, rcProcess, rcSignaling, rcNoObj, rcUnknown );
}

BOOL CC Our_HandlerRoutine(DWORD dwCtrlType)
{
    BOOL res = FALSE;
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT: SetQuitting();
        res = TRUE;
        break;
    }
    return res;
}

int
VdbInitializeSystem()
{
    /* must initialize COM... must initialize COM... */
    /* CoInitializeEx ( NULL, COINIT_MULTITHREADED ); */
    CoInitialize(NULL);

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)Our_HandlerRoutine, TRUE);

    return 0;
}

void
VdbTerminateSystem()
{
    /* balance the COM initialization */
    CoUninitialize();
}

