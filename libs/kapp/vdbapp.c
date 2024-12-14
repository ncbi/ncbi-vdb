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

#include <kapp/vdbapp.h>

#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <klib/report.h>
#include <klib/text.h>
#include <klib/ncbi-vdb-version.h>

#include <kns/manager.h>
#include <kproc/procmgr.h>

#include <atomic32.h>
#include <strtol.h>

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

    if ( vers == 0 )
    {   // by default, use the version # of the library
        vers = GetPackageVersion();
        SetKAppVersion( vers );
    }

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

        KNSManagerSetUserAgent ( kns, PKGNAMESTR " sra-toolkit %.*s.%.3V", ( uint32_t ) tool_size, tool, vers );
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


#ifdef WINDOWS
#pragma warning(disable:4127)
/*
to suppress the following condition warning:
sizeof i == sizeof ( int32_t ) and so on
*/
#endif

rc_t CC KAppCheckEnvironment ( bool require64Bits, uint64_t requireRamSize )
{
    rc_t rc;
    uint64_t totalRam;
#if _ARCH_BITS != 64
    if ( require64Bits )
    {
        rc = RC ( rcApp, rcNoTarg, rcInitializing, rcResources, rcUnsupported );
        LOGERR ( klogFatal, rc, "can only be run as 64-bit application" );
        return rc;
    }
#endif

    rc = KAppGetTotalRam ( & totalRam );
    if ( rc != 0 )
    {
        return rc;
    }

    if ( requireRamSize && totalRam < requireRamSize )
    {
        rc = RC ( rcApp, rcNoTarg, rcInitializing, rcResources, rcUnsupported );
        PLOGERR ( klogFatal, ( klogFatal, rc,  "there is not enough RAM in the system."
                                           " required size: $(REQUIRED) B, present: $(PRESENT) B"
                              , "REQUIRED=%lu,PRESENT=%lu"
                              , requireRamSize
                              , totalRam ) );
        return rc;
    }

    return 0;
}

/* AsciiToXXX
 *  replacement for atoi
 *  converts NUL terminated string in "arg" to integer
 *  invokes error handler if there is a format error in string
 *
 *  "arg" [ IN ] - NUL terminated textual representation of integer
 *  obeys standard conversion rules:
 *    starts with "0x" or "0X" - interpret as hex
 *    starts with '0' - interpret as octal
 *    otherwise - interpret as decimal
 *
 *  "handler_error" [ IN, NULL OKAY ] and "data" [ IN, OPAQUE ] -
 *  optional callback function to handle case where "arg" could not
 *  be processed in its entirety. default behavior is to log error
 *  using "logerr" and invoke "exit".
 */
static
void CC HandleAsciiToIntError ( const char *arg, void *ignore )
{
    rc_t rc;

    if ( arg == NULL )
        rc = RC ( rcApp, rcNumeral, rcConverting, rcString, rcNull );
    else if ( arg [ 0 ] == 0 )
        rc = RC ( rcApp, rcNumeral, rcConverting, rcString, rcEmpty );
    else
        rc = RC ( rcApp, rcNumeral, rcConverting, rcString, rcInvalid );

    LOGERR ( klogFatal, rc, "expected numeral" );
    exit ( 10 );
}

int32_t CC AsciiToI32 ( const char *arg,
    void ( CC * handle_error ) ( const char *arg, void *data ), void *data )
{
    if ( handle_error == NULL )
        handle_error = HandleAsciiToIntError;

    if ( arg != NULL && arg [ 0 ] != 0 )
    {
        char *end;
        long int i = strtol ( arg, & end, 0 );
        if ( end [ 0 ] == 0 )
        {
            if ( sizeof i == sizeof ( int32_t ) )
                return ( int32_t ) i;

            if ( ( ( i < 0 ) && ( ( ( uint64_t ) - i ) >> 32 ) == 0 ) ||
                 ( ( i > 0 ) && ( ( ( uint64_t ) i ) >> 32 ) == 0 ) )
                return ( int32_t ) i;
        }
    }

    ( * handle_error ) ( arg, data );
    return 0;
}

uint32_t CC AsciiToU32 ( const char *arg,
    void ( CC * handle_error ) ( const char *arg, void *data ), void *data )
{
    if ( handle_error == NULL )
        handle_error = HandleAsciiToIntError;

    if ( arg != NULL && arg [ 0 ] != 0 )
    {
        char *end;
        unsigned long int i = strtoul ( arg, & end, 0 );
        if ( end [ 0 ] == 0 )
        {
            if ( sizeof i == sizeof ( uint32_t ) )
                return ( uint32_t ) i;

            if ( ( ( ( uint64_t ) i ) >> 32 ) == 0 )
                return ( uint32_t ) i;
        }
    }

    ( * handle_error ) ( arg, data );
    return 0;
}

int64_t CC AsciiToI64 ( const char *arg,
    void ( CC * handle_error ) ( const char *arg, void *data ), void *data )
{
    if ( handle_error == NULL )
        handle_error = HandleAsciiToIntError;

    if ( arg != NULL && arg [ 0 ] != 0 )
    {
        char *end;
        int64_t i = strtoi64 ( arg, & end, 0 );

        if ( end [ 0 ] == 0 )
            return i;
    }

    ( * handle_error ) ( arg, data );
    return 0;
}

uint64_t CC AsciiToU64 ( const char *arg,
    void ( CC * handle_error ) ( const char *arg, void *data ), void *data )
{
    if ( handle_error == NULL )
        handle_error = HandleAsciiToIntError;

    if ( arg != NULL && arg [ 0 ] != 0 )
    {
        char *end;
        uint64_t i = strtou64 ( arg, & end, 0 );

        if ( end [ 0 ] == 0 )
            return i;
    }

    ( * handle_error ) ( arg, data );
    return 0;
}

/* KLogLevelParamStrings
 *  Used to compare against command line parameters
 *  These must match KLogLEvel enum in log.h
 */
static
void CC logLevelFromString ( const char * str, void *data )
{
    KLogLevel ix;
    const char ** paramStrings;

    paramStrings = KLogGetParamStrings();

    for ( ix = klogLevelMin; ix <= klogLevelMax; ++ix )
    {
        if ( strcmp ( str, paramStrings [ ( int ) ix ] ) == 0 )
        {
            * ( int32_t* ) data = ix;
            return;
        }
    }

    /* this RC should reflect an invalid string parameter to set the log level */
    PLOGERR ( klogFatal, ( klogFatal, RC ( rcApp, rcArgv, rcParsing, rcRange, rcInvalid ),
                           "log level '$(lvl)' is unrecognized", "lvl=%s", str ));
    exit ( 10 );
}

static
rc_t LogLevelAbsolute ( const char * string )
{
    int32_t 	new_level;
    int32_t	absolute_level = -1;	/* if this remains -1 then we didn't have a good symbolic string */

    /* parse as if integer value but fail over to symbolic strings */
    new_level = AsciiToU32 ( string, logLevelFromString, & absolute_level );

    return KLogLevelSet( ( KLogLevel ) ( ( absolute_level == -1 ) ? new_level : absolute_level ) );
}

static
rc_t LogLevelRelative ( const char * string )
{
    int32_t adjust = 0;
    int i;

    for ( i = 0; string [ i ] != 0; ++ i )
    {
        switch ( string [ i ] )
        {
        case '+':
            ++ adjust;
            break;

        case '-':
            -- adjust;
            break;

        default:
            return RC ( rcApp, rcArgv, rcParsing, rcToken, rcUnrecognized );
        }
    }
    KLogLevelAdjust(adjust);
    return 0;
}

rc_t CC NextLogLevelCommon ( const char * level_parameter )
{
    if ( level_parameter == NULL )
        return RC ( rcApp, rcArgv, rcParsing, rcString, rcNull );

    if ( ( level_parameter [ 0 ] == '+' ) || ( level_parameter [ 0 ] == '-' ) )
        return LogLevelRelative ( level_parameter );

    return LogLevelAbsolute ( level_parameter );
}
