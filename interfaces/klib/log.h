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

#ifndef _h_klib_log_
#define _h_klib_log_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_logfmt_
#include <klib/logfmt.h>
#endif

#ifndef _h_klib_writer_
#include <klib/writer.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * KLogLevel
 *  indicates the severity of message: emission of messages depends upon
 *  the setting of a global value that allows messages assigned higher priority
 * (lower numeric value) while filtering lower priority messages.
 *
 *  "fatal" - reported when about to abort
 *  "sys"   - an error occurred when interacting with OS
 *  "int"   - an internal logic error occurred
 *  "err"   - a user-level error occurred
 *  "warn"  - an unusual or undesired condition was detected
 *  "info"  - an event, state or condition is being reported
 *  "debug" - verbose output for use in debugging
 */
typedef uint32_t KLogLevel;
enum
{
    klogLevelMin = 0,
    klogFatal = klogLevelMin,
    klogSys,
    klogInt,
    klogErr,
    klogWarn,
    klogInfo,
    klogDebug,
    klogLevelMax = klogDebug
};

/* Get
 *  retrieve current process-global log level
 */
KLIB_EXTERN KLogLevel CC KLogLevelGet (void);

/* Set
 *  set process-global log level
 */
KLIB_EXTERN rc_t CC KLogLevelSet ( KLogLevel lvl );


/* KLogLevelAdjust
 *  adjust process-global log level by an offset from current
 */
KLIB_EXTERN void CC KLogLevelAdjust ( int32_t adjust );


/* KLogLevelExplain
 *  inserts level description
 *
 * buffer may not be NULL, num_writ may be NULL
 */
KLIB_EXTERN rc_t CC KLogLevelExplain ( KLogLevel lvl, char* buffer, size_t bsize, size_t* num_writ );

KLIB_EXTERN const char ** CC KLogGetParamStrings ( void );

/* LastErrorCode
 * LastErrorCodeReset
 *  return or clear the last reported rc_t with log level klogErr or above
 */
KLIB_EXTERN rc_t CC KLogLastErrorCode ( void );
KLIB_EXTERN void CC KLogLastErrorCodeReset ( void );

/* LastErrorCodeSet
 *  set the last reported rc_t with log level klogErr or above
 */
KLIB_EXTERN void CC KLogLastErrorCodeSet ( KLogLevel lvl, rc_t rc );

/* -----
 * Handlers for application and library writers.
 */
KLIB_EXTERN KWrtHandler* CC KLogHandlerGet (void);
KLIB_EXTERN KWrtHandler* CC KLogLibHandlerGet (void);

KLIB_EXTERN KWrtWriter CC KLogWriterGet (void);
KLIB_EXTERN KWrtWriter CC KLogLibWriterGet (void);
KLIB_EXTERN void* CC KLogDataGet (void);
KLIB_EXTERN void* CC KLogLibDataGet (void);


/* Handler
 *  sets output handler for standard output
 *
 *  "logger" [ IN ] and "self" [ IN, OPAQUE ] - callback function
 *  to handle log output
 */
KLIB_EXTERN rc_t CC KLogHandlerSet    (KWrtWriter writer, void * data);
KLIB_EXTERN rc_t CC KLogLibHandlerSet (KWrtWriter writer, void * data);

KLIB_EXTERN rc_t CC KLogHandlerSetStdOut    (void);
KLIB_EXTERN rc_t CC KLogLibHandlerSetStdOut (void);
KLIB_EXTERN rc_t CC KLogHandlerSetStdErr    (void);
KLIB_EXTERN rc_t CC KLogLibHandlerSetStdErr (void);

/* formatting */

typedef uint32_t KLogFmtFlags;
enum
{
    klogFmtTimestamp = 0x00000001,
    klogFmtSeverity = 0x00000002,
    klogFmtPid = 0x00000004,
    klogFmtAppName = 0x00000008,
    klogFmtAppVersion = 0x00000010,
    klogFmtMessage = 0x00000020, /* actual message  */
    klogFmtReason = 0x00000040, /* full RC message */
    klogFmtReasonShort = 0x00000080, /* object and state only */
    klogFmtRC = 0x00000100, /* rc enum textual values */
    klogFmtModule = 0x00000200, /* rc enum name for module */
    klogFmtTarget = 0x00000400, /* rc enum name for target */
    klogFmtContext = 0x00000800, /* rc enum name for context */
    klogFmtObject = 0x00001000, /* rc enum name for object */
    klogFmtState = 0x00002000, /* rc enum name for state */
    klogFmtModuleText = 0x00004000, /* rc enum text for module */
    klogFmtTargetText = 0x00008000, /* rc enum text for target */
    klogFmtContextText = 0x00010000, /* rc enum text for context */
    klogFmtObjectText = 0x00020000, /* rc enum text for object */
    klogFmtStateText = 0x00040000, /* rc enum text for state */
    klogFmtLocalTimestamp = 0x00080000 /* print time stamp in local time, if both bits set local takes over */
};

KLIB_EXTERN KFmtHandler* CC KLogFmtHandlerGet (void);
KLIB_EXTERN KFmtHandler* CC KLogLibFmtHandlerGet (void);

KLIB_EXTERN KLogFmtFlags CC KLogFmtFlagsGet ( void );
KLIB_EXTERN KLogFmtFlags CC KLogLibFmtFlagsGet ( void );

KLIB_EXTERN KFmtWriter CC KLogFmtWriterGet (void);
KLIB_EXTERN KFmtWriter CC KLogLibFmtWriterGet (void);
KLIB_EXTERN void* CC KLogFmtDataGet (void);
KLIB_EXTERN void* CC KLogLibFmtDataGet (void);

KLIB_EXTERN rc_t CC KLogFmtFlagsSet    (KLogFmtFlags flags);
KLIB_EXTERN rc_t CC KLogLibFmtFlagsSet (KLogFmtFlags flags);

KLIB_EXTERN rc_t CC KLogFmtHandlerSet    (KFmtWriter formatter, KLogFmtFlags flags, void* data);
KLIB_EXTERN rc_t CC KLogLibFmtHandlerSet (KFmtWriter formatter, KLogFmtFlags flags, void* data);

KLIB_EXTERN rc_t CC KLogFmtHandlerSetDefault(void);
KLIB_EXTERN rc_t CC KLogLibFmtHandlerSetDefault(void);

/*--------------------------------------------------------------------------
 * Log
 *  global logging
 *
 *  this logging API - like most others - attempts to remain isolated
 *  from any actual logging implementation, such that output may be
 *  routed in several ways.
 *
 *  the default handler
 *
 *  the path from unstructured text to formatted XML is difficult in that
 *  it requires parsing, and is therefore error prone.
 *
 *  the path from structured reports to XML as well as less or unstructured
 *  text is simple to implement, but generally more of a burden to use.
 *
 *  the challenge is therefore to present an API that makes structured
 *  logging less painful than it might be.
 */


/* Init
 *  initialize the logging module with executable identity and version,
 *  plus logging level. actual behavior is determined by the logging
 *  implementation.
 *
 */
KLIB_EXTERN rc_t CC KLogInit (void );

/* LogMsg
 *  makes an entry to the log file
 *
 *  "lvl" [ IN ] - severity of message
 *
 *  "msg" [ IN ] - NUL terminated string
 */
KLIB_EXTERN rc_t CC LogMsg ( KLogLevel lvl, const char *msg );
KLIB_EXTERN rc_t CC LogLibMsg ( KLogLevel lvl, const char *msg );

/* pLogMsg
 *  makes a parameterized entry to the log file
 *
 *  "lvl" [ IN ] - severity of message
 *
 *  "msg" [ IN ] - NUL terminated string with named parameters
 *
 *  "fmt" [ IN ] - NUL terminated format string, behaves according
 *  to standard printf-style formatting
 */
KLIB_EXTERN rc_t CC pLogMsg ( KLogLevel lvl, const char *msg, const char *fmt, ... );
KLIB_EXTERN rc_t CC vLogMsg ( KLogLevel lvl, const char *msg, const char *fmt, va_list args );
KLIB_EXTERN rc_t CC pLogLibMsg ( KLogLevel lvl, const char *msg, const char *fmt, ... );
KLIB_EXTERN rc_t CC vLogLibMsg ( KLogLevel lvl, const char *msg, const char *fmt, va_list args );


/* LogErr
 *  reports an error to log file
 *
 *  "lvl" [ IN ] - severity of message
 *
 *  "rc" [ IN ] - return code to be decoded
 *
 *  "msg" [ IN, NULL OKAY ] - optional NUL terminated message string
 */
KLIB_EXTERN rc_t CC LogErr ( KLogLevel lvl, rc_t rc, const char *msg );
KLIB_EXTERN rc_t CC LogLibErr ( KLogLevel lvl, rc_t rc, const char *msg );


/* pLogErr
 *  reports an error to log file
 *
 *  "lvl" [ IN ] - severity of message
 *
 *  "rc" [ IN ] - return code to be decoded
 *
 *  "msg" [ IN ] - NUL terminated message string with named parameters
 *
 *  "fmt" [ IN ] - NUL terminated format string, behaves according
 *  to standard printf-style formatting
 */
KLIB_EXTERN rc_t CC pLogErr ( KLogLevel lvl, rc_t rc, const char *msg, const char *fmt, ... );
KLIB_EXTERN rc_t CC vLogErr ( KLogLevel lvl, rc_t rc, const char *msg, const char *fmt, va_list args );
KLIB_EXTERN rc_t CC pLogLibErr ( KLogLevel lvl, rc_t rc, const char *msg, const char *fmt, ... );
KLIB_EXTERN rc_t CC vLogLibErr ( KLogLevel lvl, rc_t rc, const char *msg, const char *fmt, va_list args );

/* conditional wrapper */
#ifdef _LIBRARY

/*
 * Usage:
 *  LOGMSG (logWarn, (logWarn, "Something happened"));
 *
 * But we can't HAVE a pony...
 */
#define LOGMSG(lvl,msg)         \
    ((((unsigned)lvl) <= KLogLevelGet()) ? LogLibMsg (lvl, msg) : (rc_t)0)

/*
 * fmt is  two fmt strings plus parameters 
 * usage resembles
 *   PLOGMSG (logWarn, (logWarn, "message with $(PARAM1) and $(PARAM2)", "PARAM1=%s,PARAM2=%d", "parameter1", int_var));
 */
#define PLOGMSG(lvl,msg)        \
    ((((unsigned)lvl) <= KLogLevelGet()) ? pLogLibMsg msg : (rc_t)0)

/*
 * fmt is  two fmt strings plus parameters 
 * usage resembles
 *   VLOGMSG (logWarn, (logWarn, "message with $(PARAM1) and $(PARAM2)", "PARAM1=%s,PARAM2=%d", args));
 */
#define VLOGMSG(lvl,msg)        \
    ((((unsigned)lvl) <= KLogLevelGet()) ? vLogLibMsg msg : (rc_t)0)

/*
 * Usage:
 *  LOGMSG (logWarn, rc, "Something wicked this way comes");
 */
#define LOGERR(lvl,rc,msg)        \
    ((((unsigned)lvl) <= KLogLevelGet()) ? LogLibErr (lvl,rc,msg) : (rc_t)0)

/*
 * fmt is  two fmt strings plus parameters 
 * usage resembles
 *   PLOGERR (logWarn, (logWarn, rc, "message with $(PARAM1) and $(PARAM2)", "PARAM1=%s,PARAM2=%d", "parameter1", int_var));
 */
#define PLOGERR(lvl,msg)        \
    ((((unsigned)lvl) <= KLogLevelGet()) ? pLogLibErr msg : (rc_t)0)

/*
 * fmt is  two fmt strings plus parameters 
 * usage resembles
 *   VLOGERR (logWarn, (logWarn, rc, "message with $(PARAM1) and $(PARAM2)", "PARAM1=%s,PARAM2=%d", args));
 */
#define VLOGERR(lvl,msg)        \
    ((((unsigned)lvl) <= KLogLevelGet()) ? vLogLibErr msg : (rc_t)0)

#else

/*
 * Usage:
 *  LOGMSG (logWarn, "Something happened");
 */
#define LOGMSG(lvl,msg)         \
    ((((unsigned)lvl) <= KLogLevelGet()) ? LogMsg (lvl,msg) : (rc_t)0)

/*
 * fmt is  two fmt strings plus parameters 
 * usage resembles
 *   PLOGMSG (logWarn, (logWarn, "message with $(PARAM1) and $(PARAM2)", "PARAM1=%s,PARAM2=%d", "parameter1", int_var));
 */
#define PLOGMSG(lvl,msg)        \
    ((((unsigned)lvl) <= KLogLevelGet()) ? pLogMsg msg : (rc_t)0)

/*
 * fmt is  two fmt strings plus parameters 
 * usage resembles
 *   VLOGMSG (logWarn, (logWarn, "message with $(PARAM1) and $(PARAM2)", "PARAM1=%s,PARAM2=%d", args));
 */
#define VLOGMSG(lvl,msg)        \
    ((((unsigned)lvl) <= KLogLevelGet()) ? vLogMsg msg : (rc_t)0)

/*
 * Usage:
 *  LOGMSG (logWarn, rc, "Something wicked this way comes");
 */
#define LOGERR(lvl,rc,msg)        \
    ((((unsigned)lvl) <= KLogLevelGet()) ? LogErr (lvl,rc,msg) : (rc_t)0)

/*
 * fmt is  two fmt strings plus parameters 
 * usage resembles
 *   PLOGERR (logWarn, (logWarn, rc, "message with $(PARAM1) and $(PARAM2)", "PARAM1=%s,PARAM2=%d", "parameter1", int_var));
 */
#define PLOGERR(lvl,msg)        \
    ((((unsigned)lvl) <= KLogLevelGet()) ? pLogErr msg : (rc_t)0)

/*
 * fmt is  two fmt strings plus parameters 
 * usage resembles
 *   VLOGERR (logWarn, (logWarn, rc, "message with $(PARAM1) and $(PARAM2)", "PARAM1=%s,PARAM2=%d", args));
 */
#define VLOGERR(lvl,msg)        \
    ((((unsigned)lvl) <= KLogLevelGet()) ? vLogErr msg : (rc_t)0)

#endif

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_log_ */
