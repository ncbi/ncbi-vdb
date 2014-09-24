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

#ifndef _h_kapp_main_
#define _h_kapp_main_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_kapp_args_
#include <kapp/args.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KMain
 *  invoked by platform specific "main" entrypoint
 */

/* Quitting
 *  is the program supposed to exit
 */
rc_t CC Quitting ( void );

/* SignalQuit
 *  tell the program to quit
 */
rc_t CC SignalQuit ( void );

/* Hangup
 *  has the program received a SIGHUP
 */
rc_t CC Hangup ( void );

/* SignalHup
 *  send the program a SIGHUP
 */
rc_t CC SignalHup ( void );

/* SignalNoHup
 *  tell the program to stay alive even after SIGHUP
 */
rc_t CC SignalNoHup ( void );

/* Version  EXTERN
 *  return 4-part version code: 0xMMmmrrrr, where
 *      MM = major release
 *      mm = minor release
 *    rrrr = bug-fix release
 */
ver_t CC KAppVersion ( void );
    
/* KMain - EXTERN
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
rc_t CC KMain ( int argc, char *argv [] );


/* Usage - EXTERN
 *  This function is called when the command line argument
 *  handling sees -? -h or --help
 */
rc_t CC Usage ( struct Args const * args );


/* Version - EXTERN
 *  Obsolete: formerly called when the command line option handler
 *  saw -V or --version
 */
rc_t CC Version ( struct Args const * args );


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
int32_t CC AsciiToI32 ( const char *arg,
    void ( CC * handle_error ) ( const char *arg, void *data ), void *data );
uint32_t CC AsciiToU32 ( const char *arg,
    void ( CC * handle_error ) ( const char *arg, void *data ), void *data );
int64_t CC AsciiToI64 ( const char *arg,
    void ( CC * handle_error ) ( const char *arg, void *data ), void *data );
uint64_t CC AsciiToU64 ( const char *arg,
    void ( CC * handle_error ) ( const char *arg, void *data ), void *data );


/* NextLogLevel
 * NextLogLevelh
 *  these match NextArg and NextArgh but take the parameter and use it
 *  to set the Log Level.
 *
 * legal values for the parameter are:
 * 1. a sequence of + or - characters that each bump the current log level
 *    up or down one.
 * 2. an integer with a decimal value from 0 to 13 (octal and hex with the 
 *    same range are accepted.
 * 3. fatal, err, warn, info, debug1, debug3, debug3, debug4, debug5, debug6
 *    debug7, debug8, debug9, debug10
 */
void CC NextLogLevel ( const char **argp, int *ip, int argc, char *argv [],
    const char* ( CC * handle_null ) ( void *data ), void *data );
void CC NextLogLevelh ( int *ip, int argc, char *argv [],
    const char* ( CC * handle_null ) ( void *data ), void *data );

#define LogLevelSet( S ) \
    NextLogLevelCommon ( S )

rc_t CC NextLogLevelCommon ( const char * level_parameter );

#ifdef __cplusplus
}
#endif

#endif /* _h_kapp_main_ */
