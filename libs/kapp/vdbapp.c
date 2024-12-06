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

#if ! NO_KRSRC
#include <kfc/except.h>
//#include <kfc/rsrc.h>
#include <kfc/rsrc-global.h>
#include <kfc/ctx.h>
#endif

#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <klib/report.h>
#include <klib/text.h>

#include <kns/manager.h>
#include <kproc/procmgr.h>

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

static KNSManager * kns = NULL;
#if ! NO_KRSRC
static KCtx local_ctx, * ctx = & local_ctx;
#else
void CC atexit_task ( void )
{
    KProcMgrWhack ();
}
#endif

rc_t
VdbInitialize( int argc, char *argv [], ver_t vers )
{
    int ret = VdbInitializeSystem();
    if ( ret != 0 )
    {
        return RC( rcExe, rcProcess, rcInitializing, rcLibrary, rcFailed );
    }

#if NO_KRSRC
    int status;
#else
    DECLARE_FUNC_LOC ( rcExe, rcProcess, rcExecuting );
    //UNUSED(s_func_loc);
#endif

    rc_t rc = 0;

    /* initialize error reporting */
    ReportInit ( argc, argv, vers );

#if NO_KRSRC
    /* initialize cleanup tasks */
    status = atexit ( atexit_task );
    if ( status != 0 )
        return SILENT_RC ( rcApp, rcNoTarg, rcInitializing, rcFunction, rcNotAvailable );

    /* initialize proc mgr */
    rc = KProcMgrInit ();
    if ( rc != 0 )
        return rc;

    kns = NULL;
#else
    ON_FAIL ( KRsrcGlobalInit ( & local_ctx, & s_func_loc, false ) )
    {
        assert ( ctx -> rc != 0 );
        return ctx -> rc;
    }

    kns = ctx -> rsrc -> kns;
#endif

    /* initialize the default User-Agent in the kns-manager to default value - using "vers" and argv[0] above strrchr '/' */
    {
        const char * tool = argv[ 0 ];
        size_t tool_size = string_size ( tool );

        const char * sep = string_rchr ( tool, tool_size, '/' );
        if ( sep ++ == NULL )
            sep = tool;
        else
            tool_size -= sep - tool;

        sep = string_chr ( tool = sep, tool_size, '.' );
        if ( sep != NULL )
            tool_size = sep - tool;

        KNSManagerSetUserAgent ( kns, PKGNAMESTR " sra-toolkit %.*s.%V", ( uint32_t ) tool_size, tool, vers );
    }

    KNSManagerSetQuitting ( kns, Quitting );

    /* initialize logging */
    rc = KWrtInit(argv[0], vers);
    if ( rc == 0 )
        rc = KLogLibHandlerSetStdErr ();
    if ( rc == 0 )
        rc = KStsLibHandlerSetStdOut ();

#if KFG_COMMON_CREATION
    if ( rc == 0 )
    {
        KConfig *kfg;
        rc = KConfigMake ( & kfg );
    }
#endif

    return rc;
}

void
VdbTerminate( rc_t rc )
{
    {
        rc_t r2 = 0;
        if ( kns != NULL )
            r2 = KNSManagerRelease ( kns );
        else
            r2 = KNSManagerSetUserAgent ( kns, NULL );
        if ( rc == 0 && r2 != 0 )
            rc = r2;
        kns = NULL;
    }

#if KFG_COMMON_CREATION
    KConfigRelease ( kfg );
#endif

    /* finalize error reporting */
    ReportSilence ();
    ReportFinalize ( rc );

#if ! NO_KRSRC
    KRsrcGlobalWhack ( ctx );
#endif

    VdbTerminateSystem();
}