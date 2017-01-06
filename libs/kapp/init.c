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

#include <kapp/extern.h>
#include "main-priv.h"
#include <sysalloc.h>
#include <kapp/main.h>
#include <kfg/config.h>
#include <kproc/procmgr.h>
#include <klib/report.h>
#include <klib/writer.h>
#include <klib/log.h>
#include <klib/text.h>
#include <klib/status.h>
#include <klib/rc.h>
#include <kns/manager.h>

#include <strtol.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

static void setupUserAgent(KNSManager *kns, char const *tool, ver_t vers)
{
    char const *sep;
    char const *end = tool + string_size(tool);

    /* strip path */
    sep = string_rchr(tool, end - tool, '/');
    if (sep)
        tool = sep + 1;

    /* strip version or extension */
    sep = string_chr(tool, end - tool, '.');
    if (sep)
        end = sep;
    
    KNSManagerSetUserAgent(kns ? kns : ((KNSManager *)(intptr_t)-1), "%s sra-toolkit %.*s.%V", PKGNAMESTR, (uint32_t)(end - tool), tool, vers);
}

static rc_t setupLogging(char const *tool, ver_t vers)
{
    rc_t rc = KWrtInit(tool, vers);
    if ( rc == 0 )
        rc = KLogLibHandlerSetStdErr ();
    if ( rc == 0 )
        rc = KStsLibHandlerSetStdOut ();
    return rc;
}

/* KMane
 *  executable entrypoint "main" is implemented by
 *  an OS-specific wrapper that takes care of establishing
 *  signal handlers, logging, etc.
 *
 *  in turn, OS-specific "main" will invoke "KMain" as
 *  platform independent main entrypoint.
 *
 *  "argc" [ IN ] - the number of textual parameters in "argv"
 *  should never be < 0, but has been left as a signed int
 *  for reasons of tradition.
 *
 *  "argv" [ IN ] - array of NUL terminated strings expected
 *  to be in the shell-native character set: ASCII or UTF-8
 *  element 0 is expected to be executable identity or path.
 */

static KConfig *kfg;

#if NO_KRSRC
static
void CC atexit_task ( void )
{
    KProcMgrWhack ();
}

static rc_t setup(int argc, char *argv[], ver_t vers)
{
    rc_t rc;
    int status;
    
    /* initialize error reporting */
    ReportInit ( argc, argv, vers );
    
    /* initialize cleanup tasks */
    status = atexit ( atexit_task );
    if ( status != 0 )
        return SILENT_RC ( rcApp, rcNoTarg, rcInitializing, rcFunction, rcNotAvailable );
    
    /* initialize proc mgr */
    rc = KProcMgrInit ();
    if ( rc != 0 )
        return rc;
    
    setupUserAgent(NULL, argv[0], vers);
    rc = setupLogging(argv[0], vers);
    
#if KFG_COMMON_CREATION
    if ( rc == 0 )
    {
        rc = KConfigMake ( &kfg );
    }
#endif
    return rc;
}

static void teardown(rc_t rc)
{
    KConfigRelease ( kfg );

    /* finalize error reporting */
    ReportSilence ();
    ReportFinalize ( rc );
}

#else
#include <kfc/except.h>
#include <kfc/rsrc.h>
#include <kfc/rsrc-global.h>
#include <kfc/ctx.h>

static KCtx kctx;

static rc_t setup(int argc, char *argv[], ver_t vers)
{
    rc_t rc;
    KCtx *ctx = &kctx;
    DECLARE_FUNC_LOC ( rcExe, rcProcess, rcExecuting );
    
    ON_FAIL ( KRsrcGlobalInit ( & kctx, & s_func_loc, false ) )
    {
        assert ( ctx -> rc != 0 );
        return ctx -> rc;
    }
    
    /* initialize error reporting */
    ReportInit ( argc, argv, vers );
    
    setupUserAgent(ctx->rsrc->kns, argv[0], vers);
    rc = setupLogging(argv[0], vers);
    
#if KFG_COMMON_CREATION
    if ( rc == 0 )
    {
        rc = KConfigMake ( &kfg );
    }
#endif
    return rc;
}

static void teardown(rc_t rc)
{
    KConfigRelease ( kfg );
    
    /* finalize error reporting */
    ReportSilence ();
    ReportFinalize ( rc );
    
    KRsrcGlobalWhack ( &kctx );
}
#endif

rc_t KAppSetup(int argc, char *argv[], ver_t vers)
{
    return setup(argc, argv, vers);
}

void KAppTeardown(rc_t rc)
{
    teardown(rc);
}
