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

#include <klib/report.h>
#include <klib/rc.h>
#include <klib/log.h>

#include <signal.h>
#include <errno.h>

#if ! _DEBUGGING && ! defined CATCH_SIGSEGV
#define CATCH_SIGSEGV 1
#endif

#if _DEBUGGING && ! defined PRODUCE_CORE
#define PRODUCE_CORE 1
#endif

/* SignalQuit
 *  tell the program to quit
 */
rc_t SignalQuit ( void )
{
    ReportSilence ();

    if ( kill ( 0, SIGTERM ) != 0 ) switch ( errno )
    {
    case EINVAL:
        return RC ( rcExe, rcProcess, rcSignaling, rcMessage, rcInvalid );
    case EPERM:
        return RC ( rcExe, rcProcess, rcSignaling, rcMessage, rcUnauthorized );
    default:
        return RC ( rcExe, rcProcess, rcSignaling, rcNoObj, rcUnknown );
    }
    return 0;
}

rc_t SignalHup ( void )
{
    if ( kill ( 0, SIGHUP ) != 0 ) switch ( errno )
    {
    case EINVAL:
        return RC ( rcExe, rcProcess, rcSignaling, rcMessage, rcInvalid );
    case EPERM:
        return RC ( rcExe, rcProcess, rcSignaling, rcMessage, rcUnauthorized );
    default:
        return RC ( rcExe, rcProcess, rcSignaling, rcNoObj, rcUnknown );
    }
    return 0;
}

/* SigQuitHandler
 */
static
void SigQuitHandler ( int sig )
{
    const char *msg;

    SetQuitting();

    switch ( sig )
    {
    case SIGINT:
        msg = "^C";
        break;
    case SIGQUIT:
        msg = "QUIT";
        break;
    case SIGTERM:
        msg = "TERM";
        break;
    default:
        PLOGMSG ( klogWarn, ( klogWarn, "SIGNAL - $(sig)\n", "sig=%d", sig ));
        return;
    }

    PLOGMSG ( klogInfo, ( klogInfo, "SIGNAL - $(sig)", "sig=%s", msg ));
}

/* SigCoreHandler
 */
#if ! PRODUCE_CORE
static
void SigCoreHandler ( int sig )
{
    PLOGMSG ( klogFatal, ( klogFatal, "SIGNAL - $(sig)\n", "sig=%d", sig ));
    exit ( 1 );
}
#endif

/* SigSegvHandler
 */
#if CATCH_SIGSEGV
static
void SigSegvHandler ( int sig )
{
    ( ( void ) sig );
    PLOGMSG ( klogFatal, ( klogFatal, "SIGNAL - $(sig)\n", "sig=Segmentation fault" ));
#if PRODUCE_CORE
    abort ();
#endif
    exit ( 1 );
}
#endif

static struct
{
    void ( * handler ) ( int );
    int sig;
} sigs [] =
{
    { SigQuitHandler, SIGINT },
#if CATCH_SIGSEGV
    { SigSegvHandler, SIGSEGV },
#endif
    { SigQuitHandler, SIGTERM }
}
#if ! PRODUCE_CORE
, core_sigs [] =
{
#if ! CATCH_SIGSEGV
    { SigCoreHandler, SIGSEGV },
#endif
    { SigCoreHandler, SIGQUIT },
    { SigCoreHandler, SIGILL },
    { SigCoreHandler, SIGABRT },
    { SigCoreHandler, SIGFPE }
}
#endif
;

struct sigaction sig_saves [ sizeof sigs / sizeof sigs [ 0 ] ];
#if ! PRODUCE_CORE
struct sigaction core_sig_saves [ sizeof core_sigs / sizeof core_sigs [ 0 ] ];
#endif

int
VdbInitializeSystem()
{
    int i, status;

    /* install signal handlers */
    for ( i = 0; i < sizeof sigs / sizeof sigs [ 0 ]; ++ i )
    {
        struct sigaction act;
        memset ( & act, 0, sizeof act );
        act . sa_handler = sigs [ i ] . handler;
        act . sa_flags = SA_RESETHAND;

        status = sigaction ( sigs [ i ] . sig, & act, & sig_saves [ i ] );
        if ( status < 0 )
        {
            PLOGMSG ( klogFatal, ( klogFatal,
                     "failed to install handler for signal $(sig) - $(msg)"
                     , "sig=%d,msg='%s'"
                     , sigs [ i ] . sig
                     , strerror ( errno )
                          ));
            return 2;
        }
    }

    /* install signal handlers to prevent generating core files */
#if ! PRODUCE_CORE
    for ( i = 0; i < sizeof core_sigs / sizeof core_sigs [ 0 ]; ++ i )
    {
        struct sigaction act;
        memset ( & act, 0, sizeof act );
        act . sa_handler = core_sigs [ i ] . handler;

        status = sigaction ( core_sigs [ i ] . sig, & act, & core_sig_saves [ i ] );
        if ( status < 0 )
        {
            PLOGMSG ( klogFatal, ( klogFatal,
                     "failed to install handler for signal $(sig) - $(msg)"
                     , "sig=%d,msg='%s'"
                     , core_sigs [ i ] . sig
                     , strerror ( errno )
                          ));
            return 4;
        }
    }
#endif
    return 0;
}

void
VdbTerminateSystem()
{
    /* remove handlers, for what it's worth */
    for ( size_t i = 0; i < sizeof sigs / sizeof sigs [ 0 ]; ++ i )
        sigaction ( sigs [ i ] . sig, & sig_saves [ i ], NULL );
}
