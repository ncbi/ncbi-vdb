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

#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <klib/report.h>

#include <atomic32.h>

#include "main-priv.h"

static atomic32_t hangup;
static atomic32_t quitting;

/* Hangup
 *  has the program received a SIGHUP
 */
rc_t Hangup ( void )
{
    if ( atomic32_read ( & hangup ) == 0 )
        return 0;
    LOGMSG ( klogInfo, "HANGUP...\n" );
    return RC ( rcExe, rcProcess, rcExecuting, rcProcess, rcIncomplete );
}

/* SignalNoHup
 *  tell the program to stay alive even after SIGHUP
 */
rc_t CC SignalNoHup ( void )
{   // not implemented
    return 0;
}

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

/* SetQuitting
 *  set the quitting flag (for internal use in this library)
 */
void SetQuitting()
{
    ReportSilence ();
    atomic32_inc ( & quitting );
}

rc_t
VdbInitialize()
{
    int ret = VdbInitializeSystem();
    return ret == 0 ? 0 : RC( rcExe, rcProcess, rcInitializing, rcLibrary, rcFailed );
}

void
VdbTerminate()
{
    VdbTerminateSystem();
}