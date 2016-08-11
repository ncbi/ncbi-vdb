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
*/

#include <klib/extern.h>
#include <klib/status.h>
#include "writer-priv.h"
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <va_copy.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static KStsLevel G_sts_level = 0;

static KWrtHandler G_sts_handler;
static KWrtHandler G_sts_lib_handler;

static KFmtHandler G_sts_formatter;
static KStsFmtFlags G_sts_formatter_flags;
static KFmtHandler G_sts_lib_formatter;
static KStsFmtFlags G_sts_lib_formatter_flags;

static
rc_t CC KStsDefaultFormatter( void* self, KWrtHandler* writer,
                              size_t argc, const wrt_nvp_t args[],
                              size_t envc, const wrt_nvp_t envs[] )
{
    rc_t rc = 0;
    size_t num_writ, nsize;
    uint32_t mlen;
    char buffer[8192], *nbuffer;
    const char* msg, *mend;

    /* if writer is null than silence */
    if( writer == NULL || writer->writer == NULL ) {
        return rc;
    }
    msg = wrt_nvp_find_value(envc, envs, "message");
    if( msg != NULL ) {
        mend = msg + strlen(msg);
        /* strip trailing newlines */
        while( mend != msg && (*mend == '\n' || *mend == '\r') ) {
            --mend;
        }
        mlen = ( uint32_t ) ( mend - msg );
    } else {
        mlen = 0;
    }

    nbuffer = buffer;
    nsize = sizeof(buffer);
    do {
        rc = string_printf(nbuffer, nsize, & num_writ, "%s %s.%s: %.*s\n",
                                 wrt_nvp_find_value(envc, envs, "timestamp"),
                                 wrt_nvp_find_value(envc, envs, "app"),
                                 wrt_nvp_find_value(envc, envs, "version"),
                                 ( uint32_t ) mlen, msg);
        if( num_writ > nsize ) {
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

LIB_EXPORT rc_t CC KStsInit ( void )
{
    rc_t rc;

    G_sts_level = 0;
    rc = KStsHandlerSetStdOut();

    if ( rc == 0 )
        rc = KStsLibHandlerSetStdOut ();
#if 0
    if (rc == 0)
        rc = KStsLibHandlerSet(NULL, NULL);
#endif
    if (rc == 0)
        rc = KStsFmtHandlerSetDefault();

    if (rc == 0)
        rc = KStsLibFmtHandlerSetDefault();

    return rc;
}

static
rc_t CC sts_print(KFmtHandler* formatter, const KStsFmtFlags flags, KWrtHandler* writer, const char* msg, va_list args)
{
    rc_t rc = 0;
    char* nbuffer;
    size_t num_writ, remaining;

    uint32_t envc = 0;
    wrt_nvp_t envs[5];
    char ebuffer[2048];
    char mbuffer[2048];
    KFmtWriter fmtwrt;

    assert(formatter != NULL);
    assert(writer != NULL);

    fmtwrt = formatter->formatter;
    if( fmtwrt == NULL ) {
        fmtwrt = KStsDefaultFormatter;
    }
    if( fmtwrt == KStsDefaultFormatter && writer->writer == NULL ) {
        /* default formatting with NULL writer -> silence */
        return rc;
    }
    nbuffer = (char*)ebuffer;
    remaining = sizeof(ebuffer);
    do {
#define FIX_UP() if(rc){break;} remaining -= num_writ; nbuffer += num_writ
        if( flags & (kstsFmtTimestamp | kstsFmtLocalTimestamp) ) {
            if( flags & kstsFmtLocalTimestamp ) {
                rc = LogSimpleTimestamp(nbuffer, remaining, &num_writ);
            } else {
                rc = LogTimestamp(nbuffer, remaining, &num_writ);
            }
            nbuffer[num_writ++] = '\0';
            envs[envc].name = "timestamp";
            envs[envc++].value = nbuffer;
            FIX_UP();
        }
        if( flags & kstsFmtPid ) {
            rc = LogPID(nbuffer, remaining, &num_writ);
            nbuffer[num_writ++] = '\0';
            envs[envc].name = "pid";
            envs[envc++].value = nbuffer;
            FIX_UP();
        }
        if( flags & kstsFmtAppName ) {
            rc = LogAppName(nbuffer, remaining, &num_writ);
            nbuffer[num_writ++] = '\0';
            envs[envc].name = "app";
            envs[envc++].value = nbuffer;
            FIX_UP();
        }
        if( flags & kstsFmtAppVersion ) {
            rc = LogAppVersion(nbuffer, remaining, &num_writ);
            nbuffer[num_writ++] = '\0';
            envs[envc].name = "version";
            envs[envc++].value = nbuffer;
            FIX_UP();
        }
#undef FIX_UP
    } while(false);
    /* env must have one spare element for message added text below */
    if( rc == 0 && envc >= (sizeof(envs)/sizeof(envs[0])) ) {
        rc = RC(rcRuntime, rcLog, rcLogging, rcTable, rcInsufficient);
    }
    nbuffer = (char*)mbuffer;
    remaining = sizeof(mbuffer);

    if( rc == 0 ) {
        if( flags & kstsFmtMessage ) {
            if( msg == NULL || msg[0] == '\0' ) {
                msg = "empty status message";
            }
            do {
                va_list args_copy;
                va_copy(args_copy, args);
                rc = string_vprintf(nbuffer, remaining, &num_writ, msg, args_copy);
                va_end(args_copy);
                if( num_writ > remaining ) {
                    if(nbuffer != mbuffer) {
                        free(nbuffer);
                    }
                    nbuffer = malloc(remaining = num_writ);
                    if( nbuffer == NULL ) {
                        rc = RC(rcRuntime, rcLog, rcLogging, rcMemory, rcExhausted);
                    }
                } else {
                    if( rc == 0 ) {
                        envs[envc].name = "message";
                        envs[envc++].value = nbuffer;
                    }
                    break;
                }
            } while( rc == 0 );
        }
    }
    if( rc != 0 ) {
        /* print reason for failure */
        rc = string_printf((char*)mbuffer, sizeof(mbuffer), NULL, "status failure: %R in '%s'", rc, msg);
        envs[envc].name = "message";
        envs[envc++].value = mbuffer;
    }
    wrt_nvp_sort(envc, envs);
    rc = fmtwrt(formatter->data, writer, 0, NULL, envc, envs);
    if(nbuffer != mbuffer) {
        free(nbuffer);
    }
    return rc;
}

LIB_EXPORT rc_t CC KStsMsg(const char* msg, ...)
{
    rc_t rc;
    va_list args;

    va_start(args, msg);
    rc = sts_print(KStsFmtHandlerGet(), G_sts_formatter_flags, KStsHandlerGet(), msg, args);
    va_end(args);
    return rc;
}

LIB_EXPORT rc_t CC KStsLibMsg(const char* msg, ...)
{
    rc_t rc;
    va_list args;

    va_start(args, msg);
    rc = sts_print(KStsLibFmtHandlerGet(), G_sts_lib_formatter_flags, KStsLibHandlerGet(), msg, args);
    va_end(args);
    return rc;
}

LIB_EXPORT KStsLevel CC KStsLevelGet(void)
{
    return G_sts_level;
}

LIB_EXPORT void CC KStsLevelSet(KStsLevel lvl)
{
    G_sts_level = lvl;
}

LIB_EXPORT void CC KStsLevelAdjust(int32_t adjust)
{
    int32_t l = KStsLevelGet();
    l += adjust;
    KStsLevelSet( l < 0 ? 0 : l);
}

LIB_EXPORT rc_t CC KStsHandlerSetStdOut ( void )
{
    return KStsHandlerSet( KWrt_DefaultWriter, KWrt_DefaultWriterDataStdOut );
}

LIB_EXPORT rc_t CC KStsLibHandlerSetStdOut ( void )
{
    return KStsLibHandlerSet( KWrt_DefaultWriter, KWrt_DefaultWriterDataStdOut );
}

LIB_EXPORT rc_t CC KStsHandlerSetStdErr ( void )
{
    return KStsHandlerSet( KWrt_DefaultWriter, KWrt_DefaultWriterDataStdErr );
}

LIB_EXPORT rc_t CC KStsLibHandlerSetStdErr ( void )
{
    return KStsLibHandlerSet( KWrt_DefaultWriter, KWrt_DefaultWriterDataStdErr );
}

LIB_EXPORT rc_t CC KStsHandlerSet ( KWrtWriter writer, void * writer_data )
{
    G_sts_handler.writer = writer;
    G_sts_handler.data = writer_data;
    return 0;
}

LIB_EXPORT rc_t CC KStsLibHandlerSet ( KWrtWriter writer, void * writer_data )
{
    G_sts_lib_handler.writer = writer;
    G_sts_lib_handler.data = writer_data;
    return 0;
}

LIB_EXPORT void * CC KStsDataGet ( void )
{
    return ( KStsHandlerGet()->data );
}

LIB_EXPORT void * CC KStsLibDataGet ( void )
{
    return ( KStsLibHandlerGet()->data );
}

LIB_EXPORT KWrtWriter CC KStsWriterGet ( void )
{
    return ( KStsHandlerGet()->writer );
}

LIB_EXPORT KWrtWriter CC KStsLibWriterGet ( void )
{
    return ( KStsLibHandlerGet()->writer );
}

LIB_EXPORT KWrtHandler * CC KStsHandlerGet ( void )
{
    return ( &G_sts_handler );
}

LIB_EXPORT KWrtHandler * CC KStsLibHandlerGet ( void )
{
    return ( &G_sts_lib_handler );
}

static const uint32_t sts_dflt_opt = kstsFmtTimestamp | kstsFmtPid |
                                     kstsFmtMessage | kstsFmtAppName | kstsFmtAppVersion;

LIB_EXPORT rc_t CC KStsFmtHandlerSetDefault ( void )
{
    rc_t rc = KStsFmtHandlerSet(KStsDefaultFormatter, 0, NULL);
    if( rc == 0 ) {
        G_sts_formatter_flags = sts_dflt_opt;
    }
    return rc;
}

LIB_EXPORT rc_t CC KStsLibFmtHandlerSetDefault ( void )
{
    rc_t rc = KStsLibFmtHandlerSet(KStsDefaultFormatter, 0, NULL);
    if( rc == 0 ) {
        G_sts_lib_formatter_flags = sts_dflt_opt;
    }
    return rc;
}

LIB_EXPORT rc_t CC KStsFmtFlagsSet ( KStsFmtFlags flags )
{
    G_sts_formatter_flags |= flags;
    return 0;
}

LIB_EXPORT rc_t CC KStsLibFmtFlagsSet ( KStsFmtFlags flags )
{
    G_sts_lib_formatter_flags |= flags;
    return 0;
}

LIB_EXPORT rc_t CC KStsFmtHandlerSet ( KFmtWriter formatter, KStsFmtFlags flags, void* data )
{
    if( formatter == NULL ) {
        return RC(rcRuntime, rcLog, rcLogging, rcFunction, rcNull);
    }
    G_sts_formatter.formatter = formatter;
    G_sts_formatter.data = data;
    KStsFmtFlagsSet(flags);
    return 0;
}

LIB_EXPORT rc_t CC KStsLibFmtHandlerSet ( KFmtWriter formatter, KStsFmtFlags flags, void* data )
{
    if( formatter == NULL ) {
        return RC(rcRuntime, rcLog, rcLogging, rcFunction, rcNull);
    }
    G_sts_lib_formatter.formatter = formatter;
    G_sts_lib_formatter.data = data;
    KStsLibFmtFlagsSet(flags);
    return 0;
}

LIB_EXPORT void * CC KStsFmtDataGet ( void )
{
    return ( KStsFmtHandlerGet()->data );
}

LIB_EXPORT void * CC KStsLibFmtDataGet ( void )
{
    return ( KStsFmtHandlerGet()->data );
}

LIB_EXPORT KFmtWriter CC KStsFmtWriterGet ( void )
{
    return ( KStsFmtHandlerGet()->formatter ? KStsFmtHandlerGet()->formatter : KStsDefaultFormatter);
}

LIB_EXPORT KFmtWriter CC KStsLibFmtWriterGet ( void )
{
    return ( KStsLibFmtHandlerGet()->formatter ? KStsLibFmtHandlerGet()->formatter : KStsDefaultFormatter);
}

LIB_EXPORT KFmtHandler * CC KStsFmtHandlerGet ( void )
{
    return ( &G_sts_formatter );
}

LIB_EXPORT KFmtHandler * CC KStsLibFmtHandlerGet ( void )
{
    return ( &G_sts_lib_formatter );
}

LIB_EXPORT KStsFmtFlags CC KStsFmtFlagsGet ( void )
{
    return G_sts_formatter_flags;
}

LIB_EXPORT KStsFmtFlags CC KStsLibFmtFlagsGet ( void )
{
    return G_sts_lib_formatter_flags;
}
