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
#include <klib/extern.h>
#include "log-priv.h"
#include "writer-priv.h"
#include <klib/log.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/sort.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <os-native.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * LogLevel
 * defaults to the error level which is the lowest error level filtering
 * warning and informational messages
 */
static KLogLevel G_log_level = klogErr;
static rc_t G_log_last_rc = 0;

static KWrtHandler G_log_writer;
static KWrtHandler G_log_lib_writer;

static KFmtHandler G_log_formatter;
static KLogFmtFlags G_log_formatter_flags;
static KFmtHandler G_log_lib_formatter;
static KLogFmtFlags G_log_lib_formatter_flags;

/* LogLevelAdjust
 *  adjust process-global log level by an offset from current
 */
LIB_EXPORT void CC KLogLevelAdjust( int32_t adjust )
{
    int32_t lvl = KLogLevelGet();

    lvl += adjust;
    if( lvl < klogLevelMin ) {
        lvl = klogLevelMin;
    } else if( lvl > klogLevelMax ) {
        lvl = klogLevelMax;
    }
    G_log_level = lvl;
}

/* Get
 *  retrieve current process-global log level
 */
LIB_EXPORT KLogLevel CC KLogLevelGet(void)
{
    return G_log_level;
}

/* Set
 *  set process-global log level
 */
LIB_EXPORT rc_t CC KLogLevelSet(KLogLevel lvl)
{
    /* Don't allow an out of range set */
    if( (lvl < klogLevelMin) || (lvl > klogLevelMax) ) {
        return RC(rcRuntime, rcLog, rcUpdating, rcRange, rcInvalid);
    }
    G_log_level = lvl;
    return 0;
}

/*
 * These need to be kept in sync with type LogLevel
 */
static const char * logLevelParamStrings [] =
{
    "fatal",
    "sys",
    "int",
    "err",
    "warn",
    "info",
    "debug"
};

LIB_EXPORT const char ** CC KLogGetParamStrings ( void )
{
    return logLevelParamStrings;
}


/* LogLevelExplain
 *  inserts level description
 */
LIB_EXPORT rc_t CC KLogLevelExplain ( KLogLevel lvl, char *buffer, size_t bsize, size_t *num_writ )
{
    static const char undefined[] = "undefined";
    size_t size;
    uint32_t len;
    const char *t;

    /* catch unaccounted for or bad changes in log.h
     * if the asserts fail then the table above and the rest
     * of the function will fail */
    assert (klogLevelMin == 0);
    assert (klogLevelMin == klogFatal);
    assert ((klogFatal + 1) == klogSys);
    assert ((klogSys + 1) == klogInt);
    assert ((klogInt + 1) == klogErr);
    assert ((klogErr + 1) == klogWarn);
    assert ((klogWarn + 1) == klogInfo);
    assert ((klogInfo + 1) == klogDebug);
    assert ((klogDebug) == klogLevelMax);

    if ((lvl < klogLevelMin) || (lvl > klogLevelMax)) {
        t = undefined;
    } else {
        t = logLevelParamStrings[lvl];
    }

    len = string_measure (t, &size);
    if(len > bsize) {
        if (num_writ) {
            *num_writ = 0;
        }
        return RC ( rcRuntime, rcLog, rcLogging, rcBuffer, rcInsufficient );
    }
    if(num_writ) {
        *num_writ = len;
    }
    string_copy (buffer, bsize, t, len);
    return 0;
}

rc_t CC KLogDefaultFormatter( void* self, KWrtHandler* writer,
                              size_t argc, const wrt_nvp_t args[],
                              size_t envc, const wrt_nvp_t envs[] )
{
    rc_t rc = 0;
    size_t num_writ, nsize;
    uint64_t mlen;
    char buffer[8192], *nbuffer;
    const char* msg, *rc_msg;

    /* if writer is null than silence */
    if( writer == NULL || writer->writer == NULL ) {
        return rc;
    }
    msg = wrt_nvp_find_value(envc, envs, "message");
    rc_msg = wrt_nvp_find_value(envc, envs, "reason");
    if( msg != NULL ) {
        const char* mend = msg + string_size(msg);
        /* strip trailing newlines */
        while( mend != msg && (*mend == '\n' || *mend == '\r') ) {
            --mend;
        }
        mlen = mend - msg;
    } else {
        mlen = 0;
    }

    nbuffer = buffer;
    nsize = sizeof(buffer);
    do {
        rc = string_printf(nbuffer, nsize, & num_writ, "%s %s.%s %s: %s%s%.*s\n",
                                 wrt_nvp_find_value(envc, envs, "timestamp"),
                                 wrt_nvp_find_value(envc, envs, "app"),
                                 wrt_nvp_find_value(envc, envs, "version"),
                                 wrt_nvp_find_value(envc, envs, "severity"),
                                 rc_msg ? rc_msg : "", rc_msg ? " - " : "",
                                 ( uint32_t ) mlen, msg);
        if( num_writ > nsize )
        {
            assert ( nbuffer == buffer );
            nbuffer = malloc(nsize = num_writ + 2);
            if( nbuffer == NULL ) {
                rc = RC(rcRuntime, rcLog, rcLogging, rcMemory, rcExhausted);
                break;
            }
            continue;
        }
        /* replace newlines with spaces, excluding last one */
        for(nsize = 0; nsize < num_writ - 1; nsize++) {
            if( nbuffer[nsize] == '\n' || nbuffer[nsize] == '\r' ) {
                nbuffer[nsize] = ' ';
            }
        }
        break;
    } while(true);
    if( rc == 0 ) {
        rc = LogFlush(writer, nbuffer, num_writ);
    }
    if( nbuffer != buffer ) {
        free(nbuffer);
    }
    return rc;
}

/* Init
 *  initialize the logging module with executable identity and version,
 *  plus logging level. actual behavior is determined by the logging
 *  implementation.
 *
 *  "ident" [ IN ] - identity of executable
 *
 *  "vers" [ IN ] - 4-part version code: 0xMMmmrrrr, where
 *      MM = major release
 *      mm = minor release
 *    rrrr = bug-fix release
 *
 *  "lvl" [ IN ] - a LogLevel specifying cutoff point for logs, such
 *  that any log generated with a level above "lvl" will be ignored.
 */
LIB_EXPORT rc_t CC KLogInit ( void )
{
    rc_t rc;

    G_log_level = klogWarn;
    G_log_last_rc = 0;

    rc = KLogHandlerSetStdErr();

    if (rc == 0)
        rc = KLogLibHandlerSet (NULL, NULL);

    if (rc == 0)
        rc = KLogFmtHandlerSetDefault();

    if (rc == 0)
        rc = KLogLibFmtHandlerSetDefault();

    return rc;
}


/* LastErrorCode
 * LastErrorCodeReset
 *  return or clear the last reported rc_t with log level LogErr or above
 */
LIB_EXPORT rc_t CC KLogLastErrorCode ( void )
{
    return G_log_last_rc;
}

LIB_EXPORT void CC KLogLastErrorCodeReset ( void )
{
    G_log_last_rc = 0;
}

/* LastErrorCodeSet
 *  set the last reported rc_t with log level LogErr or above
 */
LIB_EXPORT void CC KLogLastErrorCodeSet ( KLogLevel lvl, rc_t rc )
{
    if ( lvl <= klogErr )
        G_log_last_rc = rc;
}

/* pLogMsg
 *  creates an info node with parameters
 *  <info time="" message="" ... />
 *
 *  "lvl" [ IN ] - severity of message
 *
 *  "msg" [ IN ] - NUL terminated string with named parameters
 *
 *  "fmt" [ IN ] - NUL terminated format string, behaves according
 *  to standard printf-style formatting
 */
LIB_EXPORT rc_t CC pLogMsg ( KLogLevel lvl, const char *msg, const char *fmt, ... )
{
    rc_t rc;
    va_list args;

    if ( lvl > KLogLevelGet() )
        return 0;

    va_start ( args, fmt );
    rc = vLogMsg ( lvl, msg, fmt, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC pLogLibMsg ( KLogLevel lvl, const char *msg, const char *fmt, ... )
{
    rc_t rc;
    va_list args;

    if ( lvl > KLogLevelGet() )
        return 0;

    va_start ( args, fmt );
    rc = vLogLibMsg ( lvl, msg, fmt, args );
    va_end ( args );

    return rc;
}

/* logsubstituteparams
 *  perform param substitution by name
 */
static
rc_t logsubstituteparams ( const char* msg, uint32_t argc, const wrt_nvp_t argv[],
                           uint32_t envc, const wrt_nvp_t envs[],
                           char* buffer, size_t bsize, size_t* num_writ )
{
    rc_t rc;
    size_t i, sz;
    for ( rc = 0, i = 0, sz = 0; msg [ i ] != 0; ++ sz, ++ i )
    {
        /* copy msg character */
        if ( sz < bsize )
            buffer [ sz ] = msg [ i ];

        /* detect substitution param */
        if ( msg [ i ] == '$' && msg [ i + 1 ] == '(' )
        {
            const char *value;
            size_t size;

            /* find param by name */
            const wrt_nvp_t *arg = wrt_nvp_find(argc, argv, &msg[ i + 2 ]);
            if( arg == NULL ) {
                arg = wrt_nvp_find(envc, envs, &msg[ i + 2 ]);
                if( arg == NULL ) {
                    rc = RC ( rcRuntime, rcLog, rcInserting, rcParam, rcNotFound );
                    break;
                }
            }
            /* substitute param value */
            for(value = arg->value; *value != 0 && sz < bsize; value++, sz++) {
                    buffer[sz] = *value;
            }
            /* compensate for outer loop's increment */
            --sz;
            /* advance past param token */
            i += string_measure(arg->name, &size) + 2;
            assert( msg[i] == ')' );
        }
    }

    * num_writ = sz;

    if ( rc != 0 )
        return rc;

    if ( sz >= bsize )
        return RC ( rcRuntime, rcLog, rcInserting, rcBuffer, rcInsufficient );

    buffer [ sz ] = 0;
    return 0;
}


/* logappendpmsg
 *  appends a message and parameters
 */
static
int logmatchname ( const char *fmt, char *pdata )
{
    int i;
    for ( i = 0; fmt [ i ] == pdata [ i ]; ++ i )
    {
        switch ( fmt [ i ] )
        {
        case '=':
            if ( i > 0 )
            {
                pdata [ i ] = 0;
                return i;
            }
        case 0:
        case ',':
            return -1;
        }
    }
    return 0;
}

static
rc_t logtokenizeparams ( const char* fmt, char* pdata,
                         uint32_t* argcp, wrt_nvp_t argv[], uint32_t arg_max )
{
    /* now split into parameters */
    uint32_t argc;
    int len = logmatchname ( fmt, pdata );
    if ( len ++ <= 0 )
        return RC ( rcRuntime, rcLog, rcTokenizing, rcParam, rcInvalid );

    argv [ 0 ] . name = pdata;
    argv [ 0 ] . value = & pdata [ len ];
    for ( argc = 1, fmt += len; ; ++ argc )
    {
        /* find end of formal param fmt */
        char *end = strchr ( fmt, ',' );
        if ( end == NULL )
            break;
        fmt = end + 1;

        /* find end of param value by
           finding beginning of next name */
        end = strchr ( argv [ argc - 1 ] . value, ',' );
        while ( end != NULL )
        {
            len = logmatchname ( fmt, end + 1 );
            if ( len > 0 )
                break;
            if ( len < 0 )
                return RC ( rcRuntime, rcLog, rcTokenizing, rcParam, rcInvalid );
            end = strchr ( end + 1, ',' );
        }

        /* detect badly formed parameter value or va_list */
        if ( end ++ == NULL )
            return RC ( rcRuntime, rcLog, rcTokenizing, rcParam, rcInvalid );

        /* detect too many parameters */
        if ( argc == arg_max )
            return RC ( rcRuntime, rcLog, rcTokenizing, rcRange, rcExcessive );

        /* account for NUL */
        ++ len;

        /* NUL terminate value */
        end [ -1 ] = 0;

        /* record param name and value addresses */
        argv [ argc ] . name = end;
        argv [ argc ] . value = end + len;

        /* advance over formal param name */
        fmt += len;
    }

    /* if multiple parameters, order by name */
    wrt_nvp_sort(argc, argv);
    * argcp = argc;
    return 0;
}

static
rc_t prep_v_args( uint32_t* argc, wrt_nvp_t argv[], size_t max_argc,
                  char* pbuffer, size_t pbsize, const char* fmt, va_list args )
{
    size_t num_writ = 0;
    rc_t rc = string_vprintf ( pbuffer, pbsize, & num_writ, fmt, args );
    if ( rc == SILENT_RC ( rcText, rcString, rcConverting, rcBuffer,
                           rcInsufficient ) )
    {
        size_t pos = num_writ;
        char truncated [] = "... [ truncated ]";
        size_t required = num_writ + sizeof truncated;
        if ( required > pbsize ) {
            assert ( pbsize > sizeof truncated );
            pos = pbsize - sizeof truncated;
        }
        {
            size_t c = string_copy_measure ( pbuffer + pos, pbsize, truncated );
            assert ( c + 1 == sizeof truncated );
            rc = 0;
        }
    }
    if ( rc == 0 )
    {
        /* tokenize the parameters into name/value pairs */
        rc = logtokenizeparams(fmt, pbuffer, argc, argv, ( uint32_t ) max_argc);
    }
    return rc;
}

static
rc_t log_print( KFmtHandler* formatter, const KLogFmtFlags flags, KWrtHandler* writer,
                KLogLevel lvl, bool use_rc, rc_t status,
                const char* msg, const char* fmt, va_list args )
{
    rc_t rc = 0;
    char* nbuffer;
    size_t num_writ, remaining;

    uint32_t envc = 0;
    wrt_nvp_t envs[20];
    char ebuffer[2048];
    uint32_t argc = 0;
    wrt_nvp_t argv[32];
    char pbuffer[4096];
    char abuffer[4096];
    KFmtWriter fmtwrt;

    assert(formatter != NULL);
    assert(writer != NULL);

    fmtwrt = formatter->formatter;
    if( fmtwrt == NULL ) {
        fmtwrt = KLogDefaultFormatter;
    }
    if( fmtwrt == KLogDefaultFormatter && writer->writer == NULL ) {
        /* default formatting with NULL writer -> silence */
        return rc;
    }
    nbuffer = (char*)ebuffer;
    remaining = sizeof(ebuffer);
    do {
#define FIX_UP() if(rc){break;} remaining -= num_writ; nbuffer += num_writ
        if( flags & (klogFmtTimestamp | klogFmtLocalTimestamp) ) {
            if( flags & klogFmtLocalTimestamp ) {
                rc = LogSimpleTimestamp(nbuffer, remaining, &num_writ);
            } else {
                rc = LogTimestamp(nbuffer, remaining, &num_writ);
            }
            nbuffer[num_writ++] = '\0';
            envs[envc].name = "timestamp";
            envs[envc++].value = nbuffer;
            FIX_UP();
        }
        if( flags & klogFmtSeverity ) {
            rc = KLogLevelExplain(lvl, nbuffer, remaining, &num_writ);
            nbuffer[num_writ++] = '\0';
            envs[envc].name = "severity";
            envs[envc++].value = nbuffer;
            FIX_UP();
        }
        if( flags & klogFmtPid ) {
            rc = LogPID(nbuffer, remaining, &num_writ);
            nbuffer[num_writ++] = '\0';
            envs[envc].name = "pid";
            envs[envc++].value = nbuffer;
            FIX_UP();
        }
        if( flags & klogFmtAppName ) {
            rc = LogAppName(nbuffer, remaining, &num_writ);
            nbuffer[num_writ++] = '\0';
            envs[envc].name = "app";
            envs[envc++].value = nbuffer;
            FIX_UP();
        }
        if( flags & klogFmtAppVersion ) {
            rc = LogAppVersion(nbuffer, remaining, &num_writ);
            nbuffer[num_writ++] = '\0';
            envs[envc].name = "version";
            envs[envc++].value = nbuffer;
            FIX_UP();
        }
        if( use_rc ) {
            if( flags & klogFmtReason ) {
                rc = RCExplain(status, nbuffer, remaining, &num_writ);
                nbuffer[num_writ++] = '\0';
                envs[envc].name = "reason";
                envs[envc++].value = nbuffer;
                FIX_UP();
            }
            if( flags & klogFmtReasonShort ) {
                rc = RCExplain2(status, nbuffer, remaining, &num_writ, eRCExOpt_ObjAndStateOnlyIfError);
                nbuffer[num_writ++] = '\0';
                envs[envc].name = "reason_short";
                envs[envc++].value = nbuffer;
                FIX_UP();
            }
            if( flags & klogFmtRC ) {
                rc = string_printf(nbuffer, remaining, & num_writ, "%R", status);
                envs[envc].name = "rc";
                envs[envc++].value = nbuffer;
                FIX_UP();
            }
            if( flags & klogFmtModule ) {
                envs[envc].name = "module";
                envs[envc++].value = GetRCModuleIdxText(GetRCModule(status));
            }
            if( flags & klogFmtTarget ) {
                envs[envc].name = "target";
                envs[envc++].value = GetRCTargetIdxText(GetRCTarget(status));
            }
            if( flags & klogFmtContext ) {
                envs[envc].name = "context";
                envs[envc++].value = GetRCContextIdxText(GetRCContext(status));
            }
            if( flags & klogFmtObject ) {
                envs[envc].name = "object";
                envs[envc++].value = GetRCObjectIdxText(GetRCObject(status));
            }
            if( flags & klogFmtState ) {
                envs[envc].name = "state";
                envs[envc++].value = GetRCStateIdxText(GetRCState(status));
            }
            if( flags & klogFmtModuleText ) {
                envs[envc].name = "module-text";
                envs[envc++].value = GetRCModuleText(GetRCModule(status));
            }
            if( flags & klogFmtTargetText ) {
                envs[envc].name = "target-text";
                envs[envc++].value = GetRCTargetText(GetRCTarget(status));
            }
            if( flags & klogFmtContextText ) {
                envs[envc].name = "context-text";
                envs[envc++].value = GetRCContextText(GetRCContext(status));
            }
            if( flags & klogFmtObjectText ) {
                envs[envc].name = "object-text";
                envs[envc++].value = GetRCObjectText(GetRCObject(status));
            }
            if( flags & klogFmtStateText ) {
                envs[envc].name = "state-text";
                envs[envc++].value = GetRCStateText(GetRCState(status));
            }
        }
#undef FIX_UP
    } while(false);
    /* env must have one spare element for message added text below */
    if( rc == 0 && envc >= (sizeof(envs)/sizeof(envs[0])) ) {
        rc = RC(rcRuntime, rcLog, rcLogging, rcTable, rcInsufficient);
    }
    nbuffer = (char*)abuffer;
    remaining = sizeof(abuffer);
    if( rc == 0 ) {
        if( fmt != NULL ) {
            rc = prep_v_args(&argc, argv, sizeof(argv)/sizeof(argv[0]) - 1, pbuffer, sizeof(pbuffer), fmt, args);
        }
        if( rc == 0 && (flags & klogFmtMessage) ) {
            int retries = 0;
            if( msg == NULL || msg[0] == '\0' ) {
                msg = "empty log message";
            }
            do {
                /* substitute parameters into message */
                wrt_nvp_sort(envc, envs);
                rc = logsubstituteparams(msg, argc, argv, envc, envs, nbuffer, remaining - 1, &num_writ);
                if( GetRCState(rc) == rcInsufficient ) {
                    if(nbuffer != abuffer) {
                        free(nbuffer);
                    }
                    remaining += num_writ > remaining ? num_writ : remaining;
                    nbuffer = malloc(remaining);
                    if( nbuffer == NULL ) {
                        rc = RC(rcRuntime, rcLog, rcLogging, rcMemory, rcExhausted);
                    }
                } else {
                    if( rc == 0 ) {
                        nbuffer[num_writ] = '\0';
                        envs[envc].name = "message";
                        envs[envc++].value = nbuffer;
                    }
                    break;
                }
                if ( retries ++ > 9 ) /* something is wrong: too many retries */
                    break;
            } while(rc != 0);
        }
    }
    if( rc != 0 ) {
        /* print reason for failure */
        string_printf((char*)abuffer, sizeof(abuffer), &num_writ, "log failure: %R in '%s'", rc, msg);
        envs[envc].name = "message";
        envs[envc++].value = abuffer;
    }

    wrt_nvp_sort(envc, envs);

    {
        rc_t rc2 = fmtwrt(formatter->data, writer, argc, argv, envc, envs);
        if( (rc2 == 0) && use_rc ) {
            KLogLastErrorCodeSet(lvl, status);
        }
        if(nbuffer != abuffer) {
            free(nbuffer);
        }
        return rc != 0 ? rc : rc2;
    }
}

/* LogMsg
 *  makes an entry to the log file
 *
 *  "lvl" [ IN ] - severity of message
 *
 *  "msg" [ IN ] - NUL terminated string
 */

LIB_EXPORT rc_t CC LogMsg ( KLogLevel lvl, const char *msg )
{
    if ( lvl > KLogLevelGet() )
        return 0;

    return log_print(KLogFmtHandlerGet(), G_log_formatter_flags,
                     KLogHandlerGet(), lvl, false, 0, msg, NULL, NULL );
}

LIB_EXPORT rc_t CC LogLibMsg ( KLogLevel lvl, const char *msg )
{
    if ( lvl > KLogLevelGet() )
        return 0;

    return log_print(KLogLibFmtHandlerGet(), G_log_lib_formatter_flags,
                     KLogLibHandlerGet(), lvl, false, 0, msg, NULL, NULL );
}


/* LogErr
 *  reports an error to log file
 *
 *  "lvl" [ IN ] - severity of message
 *
 *  "status" [ IN ] - return code to be decoded
 *
 *  "msg" [ IN, NULL OKAY ] - optional NUL terminated message string
 */
LIB_EXPORT rc_t CC LogErr ( KLogLevel lvl, rc_t status, const char *msg )
{
    if ( lvl > KLogLevelGet() )
        return 0;

    return log_print(KLogFmtHandlerGet(), G_log_formatter_flags,
                     KLogHandlerGet(), lvl, true, status, msg, NULL, NULL );
}

LIB_EXPORT rc_t CC LogLibErr ( KLogLevel lvl, rc_t status, const char *msg )
{
    if ( lvl > KLogLevelGet() )
        return 0;

    return log_print(KLogLibFmtHandlerGet(), G_log_lib_formatter_flags,
                     KLogLibHandlerGet(), lvl, true, status, msg, NULL, NULL );
}

/* vLogMsg
 *  makes a parameterized entry to the log file
 *
 *  "lvl" [ IN ] - severity of message
 *
 *  "msg" [ IN ] - NUL terminated string with named parameters
 *
 *  "fmt" [ IN ] - NUL terminated format string, behaves according
 *  to standard printf-style formatting
 */
LIB_EXPORT rc_t CC vLogMsg ( KLogLevel lvl, const char *msg, const char *fmt, va_list args )
{
    if ( lvl > KLogLevelGet() )
        return 0;

    return log_print(KLogFmtHandlerGet(), G_log_formatter_flags,
                     KLogHandlerGet(), lvl, false, 0, msg, fmt, args );
}

LIB_EXPORT rc_t CC vLogLibMsg ( KLogLevel lvl, const char *msg, const char *fmt, va_list args )
{
    if ( lvl > KLogLevelGet() )
        return 0;

    return log_print(KLogLibFmtHandlerGet(), G_log_lib_formatter_flags,
                     KLogLibHandlerGet(), lvl, false, 0, msg, fmt, args );
}

/* vLogErr
 *  reports an error to log file
 *
 *  "lvl" [ IN ] - severity of message
 *
 *  "status" [ IN ] - return code to be decoded
 *
 *  "msg" [ IN ] - NUL terminated message string with named parameters
 *
 *  "fmt" [ IN ] - NUL terminated format string, behaves according
 *  to standard printf-style formatting
 */
LIB_EXPORT rc_t CC vLogErr ( KLogLevel lvl, rc_t status, const char *msg, const char *fmt, va_list args )
{
    if ( lvl > KLogLevelGet() )
        return 0;

    return log_print(KLogFmtHandlerGet(), G_log_formatter_flags,
                     KLogHandlerGet(), lvl, true, status, msg, fmt, args );
}

LIB_EXPORT rc_t CC vLogLibErr ( KLogLevel lvl, rc_t status, const char *msg, const char *fmt, va_list args )
{
    if ( lvl > KLogLevelGet() )
        return 0;
    return log_print(KLogLibFmtHandlerGet(), G_log_lib_formatter_flags,
                     KLogLibHandlerGet(), lvl, true, status, msg, fmt, args );
}

/* pLogErr
 *  reports an error to log file
 *
 *  "lvl" [ IN ] - severity of message
 *
 *  "status" [ IN ] - return code to be decoded
 *
 *  "msg" [ IN ] - NUL terminated message string with named parameters
 *
 *  "fmt" [ IN ] - NUL terminated format string, behaves according
 *  to standard printf-style formatting
 */
LIB_EXPORT rc_t CC pLogErr ( KLogLevel lvl, rc_t status, const char *msg, const char *fmt, ... )
{
    rc_t rc;
    va_list args;

    if ( lvl > KLogLevelGet() )
        return 0;

    va_start ( args, fmt );
    rc = vLogErr ( lvl, status, msg, fmt, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC pLogLibErr ( KLogLevel lvl, rc_t status, const char *msg, const char *fmt, ... )
{
    rc_t rc;
    va_list args;

    if ( lvl > KLogLevelGet() )
        return 0;

    va_start ( args, fmt );
    rc = vLogLibErr ( lvl, status, msg, fmt, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KLogHandlerSetStdOut ( void )
{
    return KLogHandlerSet(KWrt_DefaultWriter, KWrt_DefaultWriterDataStdOut);
}

LIB_EXPORT rc_t CC KLogLibHandlerSetStdOut ( void )
{
    return KLogLibHandlerSet(KWrt_DefaultWriter, KWrt_DefaultWriterDataStdOut);
}

LIB_EXPORT rc_t CC KLogHandlerSetStdErr ( void )
{
    return KLogHandlerSet(KWrt_DefaultWriter, KWrt_DefaultWriterDataStdErr);
}

LIB_EXPORT rc_t CC KLogLibHandlerSetStdErr ( void )
{
    return KLogLibHandlerSet(KWrt_DefaultWriter, KWrt_DefaultWriterDataStdErr);
}

LIB_EXPORT rc_t CC KLogHandlerSet ( KWrtWriter writer, void * data )
{
    G_log_writer.writer = writer;
    G_log_writer.data = data;
    return 0;
}

LIB_EXPORT rc_t CC KLogLibHandlerSet ( KWrtWriter writer, void * data )
{
    G_log_lib_writer.writer = writer;
    G_log_lib_writer.data = data;
    return 0;
}

LIB_EXPORT void * CC KLogDataGet ( void )
{
    return ( KLogHandlerGet()->data );
}

LIB_EXPORT void * CC KLogLibDataGet ( void )
{
    return ( KLogLibHandlerGet()->data );
}

LIB_EXPORT KWrtWriter CC KLogWriterGet ( void )
{
    return ( KLogHandlerGet()->writer );
}

LIB_EXPORT KWrtWriter CC KLogLibWriterGet ( void )
{
    return ( KLogLibHandlerGet()->writer );
}

LIB_EXPORT KWrtHandler * CC KLogHandlerGet ( void )
{
    return ( &G_log_writer );
}

LIB_EXPORT KWrtHandler * CC KLogLibHandlerGet ( void )
{
    return ( &G_log_lib_writer );
}

static const uint32_t log_dflt_opt = klogFmtTimestamp | klogFmtSeverity | klogFmtPid |
                klogFmtMessage | klogFmtAppName | klogFmtAppVersion | klogFmtReason;

LIB_EXPORT rc_t CC KLogFmtHandlerSetDefault ( void )
{
    rc_t rc = KLogFmtHandlerSet(KLogDefaultFormatter, 0, NULL);
    if( rc == 0 ) {
        G_log_formatter_flags = log_dflt_opt;
    }
    return rc;
}

LIB_EXPORT rc_t CC KLogLibFmtHandlerSetDefault ( void )
{
    rc_t rc = KLogLibFmtHandlerSet(KLogDefaultFormatter, 0, NULL);
    if( rc == 0 ) {
        G_log_lib_formatter_flags = log_dflt_opt;
    }
    return rc;
}

LIB_EXPORT rc_t CC KLogFmtFlagsSet ( KLogFmtFlags flags )
{
    G_log_formatter_flags |= flags;
    return 0;
}

LIB_EXPORT rc_t CC KLogLibFmtFlagsSet ( KLogFmtFlags flags )
{
    G_log_lib_formatter_flags |= flags;
    return 0;
}

LIB_EXPORT rc_t CC KLogFmtHandlerSet ( KFmtWriter formatter, KLogFmtFlags flags, void* data )
{
    if( formatter == NULL ) {
        return RC(rcRuntime, rcLog, rcLogging, rcFunction, rcNull);
    }
    G_log_formatter.formatter = formatter;
    G_log_formatter.data = data;
    KLogFmtFlagsSet(flags);
    return 0;
}

LIB_EXPORT rc_t CC KLogLibFmtHandlerSet ( KFmtWriter formatter, KLogFmtFlags flags, void* data )
{
    if( formatter == NULL ) {
        return RC(rcRuntime, rcLog, rcLogging, rcFunction, rcNull);
    }
    G_log_lib_formatter.formatter = formatter;
    G_log_lib_formatter.data = data;
    KLogLibFmtFlagsSet(flags);
    return 0;
}

LIB_EXPORT void * CC KLogFmtDataGet ( void )
{
    return ( KLogFmtHandlerGet()->data );
}

LIB_EXPORT void * CC KLogLibFmtDataGet ( void )
{
    return ( KLogFmtHandlerGet()->data );
}

LIB_EXPORT KFmtWriter CC KLogFmtWriterGet ( void )
{
    return ( KLogFmtHandlerGet()->formatter ? KLogFmtHandlerGet()->formatter : KLogDefaultFormatter );
}

LIB_EXPORT KFmtWriter CC KLogLibFmtWriterGet ( void )
{
    return ( KLogLibFmtHandlerGet()->formatter ? KLogLibFmtHandlerGet()->formatter : KLogDefaultFormatter );
}

LIB_EXPORT KFmtHandler * CC KLogFmtHandlerGet ( void )
{
    return ( &G_log_formatter );
}

LIB_EXPORT KFmtHandler * CC KLogLibFmtHandlerGet ( void )
{
    return ( &G_log_lib_formatter );
}

LIB_EXPORT KLogFmtFlags CC KLogFmtFlagsGet ( void )
{
    return G_log_formatter_flags;
}

LIB_EXPORT KLogFmtFlags CC KLogLibFmtFlagsGet ( void )
{
    return G_log_lib_formatter_flags;
}
