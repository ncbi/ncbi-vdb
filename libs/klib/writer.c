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
#include <klib/out.h>
#include <klib/status.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/sort.h>
#include <sysalloc.h>
#include <atomic32.h>

#include "writer-priv.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <assert.h>

#include <os-native.h>

static char wrt_app[32];
static size_t wrt_app_length;
static char wrt_vers[16];
static size_t wrt_vers_length;

void* KWrt_DefaultWriterDataStdOut = NULL;
void* KWrt_DefaultWriterDataStdErr = NULL;

typedef struct RCCreateLoc RCCreateLoc;
struct RCCreateLoc
{
    const char *filename;
    const char *function;
    uint32_t lineno;
    rc_t rc;
};

static RCCreateLoc RC_loc_queue [ 3 ];
static atomic32_t RC_loc_reserve, RC_loc_written, RC_loc_read;
#define RC_LOC_QUEUE_SIZE ( sizeof RC_loc_queue / sizeof RC_loc_queue [ 0 ] )
#define RC_LOC_QUEUE_MASK ( RC_LOC_QUEUE_SIZE - 1 )
static bool reporting_unread = false;

/*
 *  "appname" [ IN ] - identity of executable, usually argv[0]
 *
 *  "vers" [ IN ] - 4-part version code: 0xMMmmrrrr, where
 *      MM = major release
 *      mm = minor release
 *    rrrr = bug-fix release
 */
LIB_EXPORT rc_t CC KWrtInit( const char* appname, uint32_t vers )
{
    rc_t rc;

    if ( appname == NULL )
        return RC ( rcRuntime, rcLog, rcConstructing, rcString, rcNull );
    if ( appname [ 0 ] == 0 )
        return RC ( rcRuntime, rcLog, rcConstructing, rcString, rcEmpty );

    do
    {
        const char* progname;
        const char* ext;
        size_t progname_z;

        /* find whichever is last \ or / */
        string_measure(appname, &progname_z);
        progname = string_rchr(appname, progname_z, '/');
        if( progname == NULL ) {
            progname = appname;
        } else {
            progname++;
            string_measure(progname, &progname_z);
        }
        appname = string_rchr(progname, progname_z, '\\');
        if( appname == NULL ) {
            appname = progname;
        } else {
            appname++;
        }
        string_measure(appname, &progname_z);

        ext = string_chr(appname, progname_z, '.');

        if( ext != NULL ) {
            wrt_app_length = ext - appname;
        } else {
            wrt_app_length = progname_z;
        }
        if ( wrt_app_length >= sizeof(wrt_app) ) {
            wrt_app_length = sizeof(wrt_app) - 1;
        }
        memmove(wrt_app, appname, wrt_app_length);
        wrt_app[wrt_app_length] = '\0';

        rc = string_printf ( wrt_vers, sizeof wrt_vers, & wrt_vers_length,
            "%.3V", vers );
        assert ( rc == 0 );

        rc = KWrtSysInit(&KWrt_DefaultWriterDataStdOut, &KWrt_DefaultWriterDataStdErr);
        if (rc) break;

        rc = KOutInit();
        if (rc) break;

        rc = KLogInit();
        if (rc) break;

        rc = KStsInit();
        if (rc) break;

        rc = KDbgInit();
    } while (0);

    return rc;
}

LIB_EXPORT size_t CC simple_write( int fd, const void * buf, size_t count )
{
    /* calls the platform-specific implementation ( $PLATFORM/syswriter.c ) */
    return sys_simple_write( fd, buf, count );
}

LIB_EXPORT int CC is_a_tty( int fd )
{
    /* calls the platform-specific implementation ( $PLATFORM/syswriter.c ) */
    return sys_is_a_tty( fd );
}

/*--------------------------------------------------------------------------
 * nvp - name/value pair
 */
static
int64_t CC wrt_nvp_cmp_func(const void *a, const void *b, void * ignored)
{
    int i = 0;
    const char *key = a;
    const char *name = ( ( const wrt_nvp_t* ) b ) -> name;

    while(key[i] == name[i]) {
        if( key[i] == '\0' || name[i] == '\0' ) {
            break;
        }
        ++i;
    }
    /* treat \0 or right-paren as key terminator */
    if( key[i] != 0 && key[i] != ')' ) {
        return (int64_t)key[i] - (int64_t)name[i];
    }
    return (int64_t)0 - (int64_t)name[i];
}

static
int64_t CC wrt_nvp_sort_func(const void *a, const void *b, void * ignored)
{
    const wrt_nvp_t *left = a;
    const wrt_nvp_t *right = b;
    return strcmp ( left -> name, right -> name );
}

LIB_EXPORT void CC wrt_nvp_sort( size_t argc, wrt_nvp_t argv[])
{
    if( argc > 1 ) {
        ksort(argv, argc, sizeof(argv[0]), wrt_nvp_sort_func, NULL);
    }
}

LIB_EXPORT const wrt_nvp_t* CC wrt_nvp_find( size_t argc, const wrt_nvp_t argv[], const char* key )
{
    if( argc > 0 ) {
        return kbsearch(key, argv, argc, sizeof(argv[0]), wrt_nvp_cmp_func, NULL);
    }
    return NULL;
}

LIB_EXPORT const char* CC wrt_nvp_find_value( size_t argc, const wrt_nvp_t argv[], const char* key )
{
    if( argc > 0 ) {
        const wrt_nvp_t* n = (const wrt_nvp_t*)kbsearch(key, argv, argc, sizeof(argv[0]), wrt_nvp_cmp_func, NULL);
        if( n != NULL ) {
            return n->value;
        }
    }
    return NULL;
}

static
rc_t RCLiteral ( rc_t self, char *buffer, size_t bsize, size_t *num_writ )
{
#if ! _DEBUGGING && RECORD_RC_FILE_LINE
    ( void ) GetRCLineno ();
#endif
    return string_printf ( buffer, bsize, num_writ
#if _DEBUGGING
        , "rc = %s:%u:$s:%u.%u.%u.%u.%u"
        , GetRCFilename(), GetRCLineno (), GetRCFunction ()
#else
        , "rc = %u.%u.%u.%u.%u"
#endif
        , ( uint32_t ) GetRCModule ( self )
        , ( uint32_t ) GetRCTarget ( self )
        , ( uint32_t ) GetRCContext ( self )
        , ( uint32_t ) GetRCObject ( self )
        , ( uint32_t ) GetRCState ( self )
    );
}

LIB_EXPORT rc_t CC RCExplain ( rc_t rc, char *buffer, size_t bsize, size_t *num_writ )
{
    return RCExplain2
        ( rc, buffer, bsize, num_writ, eRCExOpt_CompleteMsg );
}

LIB_EXPORT rc_t CC RCExplain2 ( rc_t rc, char *buffer, size_t bsize, size_t *num_writ,
                                enum ERCExplain2Options options )
{
    bool noMessageIfNoError =
        (options == eRCExOpt_NoMessageIfNoError || 
         options == eRCExOpt_ObjAndStateOnlyIfError);
    int len;
    size_t total = 0;

    const char *mod = GetRCModuleText ( GetRCModule ( rc ) );
    const char *targ = GetRCTargetText ( GetRCTarget ( rc ) );
    const char *ctx = GetRCContextText ( GetRCContext ( rc ) );
    const char *obj = GetRCObjectText ( GetRCObject ( rc ) );
    const char *state = GetRCStateText ( GetRCState ( rc ) );

    assert( buffer && num_writ );

    *num_writ = 0;
    if( rc == 0 && noMessageIfNoError ) {
        buffer[0] = '\0';
        return 0;
    }

    /* English'ish formatting */
#if _DEBUGGING
    {
        const char *function = GetRCFunction ();
        if ( function != NULL )
        {
            len = snprintf(buffer + total, bsize - total, "%s:%u:%s: ", GetRCFilename(), GetRCLineno (), function );
            if( len < 0 || ( total + len ) >= bsize ) {
                return RCLiteral ( rc, buffer, bsize, num_writ );
            }
            total += len;
        }
    }
#elif RECORD_RC_FILE_LINE
    ( void ) GetRCLineno ();
#endif
    if( obj != NULL ) {
        len = snprintf(buffer + total, bsize - total, "%s", obj);
        if( len < 0 || ( total + len ) >= bsize ) {
            return RCLiteral ( rc, buffer, bsize, num_writ );
        }
        total += len;
    }
    if( state != NULL ) {
        len = snprintf(buffer + total, bsize - total, "%s%s", total ? " " : "", state);
        if( len < 0 || ( total + len ) >= bsize ) {
            return RCLiteral ( rc, buffer, bsize, num_writ );
        }
        total += len;
    }
    if( rc != 0 && options == eRCExOpt_CompleteMsg ) {
        if( ctx != NULL ) {
            len = snprintf ( buffer + total, bsize - total, "%swhile %s", total ? " " : "", ctx );
            if ( len < 0 || ( total + len ) >= bsize ) {
                return RCLiteral ( rc, buffer, bsize, num_writ );
            }
            total += len;
            if( targ != NULL ) {
                len = snprintf ( buffer + total, bsize - total, "%s%s", total ? " " : "", targ );
                if( len < 0 || ( total + len ) >= bsize ) {
                    return RCLiteral ( rc, buffer, bsize, num_writ );
                }
                total += len;
            }
        } else if( targ != NULL ) {
            len = snprintf ( buffer + total,
                bsize - total, "%swhile acting upon %s", total ? " " : "", targ );
            if( len < 0 || ( total + len ) >= bsize ) {
                return RCLiteral ( rc, buffer, bsize, num_writ );
            }
            total += len;
        }
    }
    if( mod != NULL && options == eRCExOpt_CompleteMsg ) {
        len = snprintf(buffer + total, bsize - total, "%swithin %s module", total ? " " : "", mod);
        if( len < 0 || ( total + len ) >= bsize ) {
            return RCLiteral ( rc, buffer, bsize, num_writ );
        }
        total += len;
    }
    *num_writ = total;
    return 0;
}

/*--------------------------------------------------------------------------
 * RC
 */


/* GetRCModuleText
 */
const char *GetRCModuleText ( enum RCModule mod )
{
    if ( ( int ) mod < 0 || ( int ) mod >= ( int ) rcLastModule_v1_2 )
        return "<INVALID-MODULE>";
    return gRCModule_str [ ( int ) mod ];
}

/* GetRCModuleIdxText
 */
const char *GetRCModuleIdxText ( enum RCModule mod )
{
    if ( ( int ) mod < 0 || ( int ) mod >= ( int ) rcLastModule_v1_2 )
        return "<INVALID-MODULE>";
    return gRCModuleIdx_str [ ( int ) mod ];
}

/* GetRCTargetText
 */
const char *GetRCTargetText ( enum RCTarget targ )
{
    if ( ( int ) targ < 0 || ( int ) targ >= ( int ) rcLastTarget_v1_2 )
        return "<INVALID-TARGET>";
    return gRCTarget_str [ ( int ) targ ];
}

/* GetRCTargetIdxText
 */
const char *GetRCTargetIdxText ( enum RCTarget targ )
{
    if ( ( int ) targ < 0 || ( int ) targ >= ( int ) rcLastTarget_v1_2 )
        return "<INVALID-TARGET>";
    return gRCTargetIdx_str [ ( int ) targ ];
}

/* GetRCContextText
 */
const char *GetRCContextText ( enum RCContext ctx )
{
    if ( ( int ) ctx < 0 || ( int ) ctx >= ( int ) rcLastContext_v1_1 )
        return "<INVALID-CONTEXT>";
    return gRCContext_str [ ( int ) ctx ];
}

/* GetRCContextIdxText
 */
const char *GetRCContextIdxText ( enum RCContext ctx )
{
    if ( ( int ) ctx < 0 || ( int ) ctx >= ( int ) rcLastContext_v1_1 )
        return "<INVALID-CONTEXT>";
    return gRCContextIdx_str [ ( int ) ctx ];
}

/* GetRCObjectText
 */
const char *GetRCObjectText ( int obj )
{
    if ( ( int ) obj < 0 || ( int ) obj >= ( int ) rcLastObject_v1_2 )
        return "<INVALID-OBJECT>";
    if ( ( int ) obj < ( int ) rcLastTarget_v1_1 )
        return gRCTarget_str [ ( int ) obj ];
    return gRCObject_str [ ( int ) obj - ( int ) ( rcLastTarget_v1_1 - 1 ) ];
}

/* GetRCObjectIdxText
 */
const char *GetRCObjectIdxText ( int obj )
{
    if ( ( int ) obj < 0 || ( int ) obj >= ( int ) rcLastObject_v1_2 )
        return "<INVALID-OBJECT>";
    if ( ( int ) obj < ( int ) rcLastTarget_v1_1 )
        return gRCTargetIdx_str [ ( int ) obj ];
    return gRCObjectIdx_str [ ( int ) obj - ( int ) ( rcLastTarget_v1_1 - 1 ) ];
}

/* GetRCStateText
 */
const char *GetRCStateText ( enum RCState state )
{
    if ( ( int ) state < 0 || ( int ) state >= ( int ) rcLastState_v1_1 )
        return "<INVALID-STATE>";
    return gRCState_str [ ( int ) state ];
}

/* GetRCStateIdxText
 */
const char *GetRCStateIdxText ( enum RCState state )
{
    if ( ( int ) state < 0 || ( int ) state >= ( int ) rcLastState_v1_1 )
        return "<INVALID-STATE>";
    return gRCStateIdx_str [ ( int ) state ];
}

static
uint32_t read_rc_loc_head ( void )
{
    int32_t idx = atomic32_read ( & RC_loc_written );
    if ( ! reporting_unread )
    {
        atomic32_set ( & RC_loc_read, idx );
    }
    return idx;
}

static
const char *get_rc_filename ( uint32_t idx )
{
    const char *p = RC_loc_queue [ idx & RC_LOC_QUEUE_MASK ] . filename;
    if( p != NULL )
    {
        int i;
        const char *sep;
        const char *RC_filename = p;
#if WINDOWS
        static char win_rc_filename [ 4096 ];
        size_t w, len = string_copy_measure ( win_rc_filename, sizeof win_rc_filename - 1, p );
        if ( len >= 2 && isalpha ( win_rc_filename [ 0 ] ) && win_rc_filename [ 1 ] == ':' )
        {
            win_rc_filename [ 1 ] = win_rc_filename [ 0 ];
            win_rc_filename [ 0 ] = '/';
        }
        for ( w = 0; w < len; ++ w )
        {
            if ( win_rc_filename [ w ] == '\\' )
                win_rc_filename [ w ] = '/';
        }
        p = RC_filename = win_rc_filename;
#endif
        if ( (p = strstr(RC_filename, "/interfaces/")) != NULL ||
             (p = strstr(RC_filename, "/libs/")) != NULL ||
             (p = strstr(RC_filename, "/services/")) != NULL ||
             (p = strstr(RC_filename, "/tools/")) != NULL ||
             (p = strstr(RC_filename, "/asm-trace/")) != NULL )
        {
            return p + 1;
        }

        for ( i = 0, sep = strrchr ( p = RC_filename, '/' ); sep != NULL && i < 3; ++ i )
        {
            p = sep + 1;
            sep = string_rchr ( RC_filename, sep - RC_filename, '/' );
        }
    }

    return p;
}

LIB_EXPORT const char * CC GetRCFilename ( void )
{
    return get_rc_filename ( read_rc_loc_head () );
}

/* InsertSpace
 *  inserts a division after current text
 *
 *  "spacer" [ IN, NULL OKAY ] - optional characters to insert
 */
LIB_EXPORT rc_t CC LogInsertSpace(const char *spacer, char *buffer, size_t bsize, size_t *num_writ)
{
    int len;

    if ( spacer == NULL )
    {
        if ( bsize < 2 )
            return RC ( rcRuntime, rcLog, rcLogging, rcBuffer, rcInsufficient );
        buffer [ 0 ] = ' ';
        buffer [ 1 ] = 0;
        * num_writ = 1;
        return 0;
    }

    len = snprintf ( buffer, bsize, "%s", spacer );

    * num_writ = len;

    if ( len < 0 || (size_t)len >= bsize )
    {
        if ( len < 0 )
            * num_writ = 0;
        return RC ( rcRuntime, rcLog, rcLogging, rcBuffer, rcInsufficient );
    }

    return 0;
}

LIB_EXPORT rc_t CC LogAppName(char *buffer, size_t bsize, size_t *num_writ)
{
    if( wrt_app_length > bsize ) {
        return RC(rcRuntime, rcLog, rcLogging, rcBuffer, rcInsufficient);
    }
    memmove(buffer, wrt_app, wrt_app_length);
    *num_writ = wrt_app_length;
    return 0;
}

LIB_EXPORT rc_t CC LogAppVersion(char *buffer, size_t bsize, size_t *num_writ)
{
    if( wrt_vers_length > bsize ) {
        return RC(rcRuntime, rcLog, rcLogging, rcBuffer, rcInsufficient);
    }
    memmove(buffer, wrt_vers, wrt_vers_length);
    *num_writ = wrt_vers_length;
    return 0;
}

LIB_EXPORT rc_t CC LogFlush ( const KWrtHandler* handler, const char *buffer, const size_t bsize)
{
    rc_t rc = 0;
    size_t num_written;
    size_t remaining;

    assert(handler != NULL);
    assert(buffer != NULL);

    for(remaining = bsize; rc == 0 && remaining > 0; remaining -= num_written, buffer += num_written) {
        rc = handler->writer(handler->data, buffer, remaining, &num_written);
    }
    return rc;
}

static
const char *get_rc_function ( uint32_t idx )
{
    return RC_loc_queue [ idx & RC_LOC_QUEUE_MASK ] . function;
}

LIB_EXPORT const char * CC GetRCFunction ( void )
{
    return get_rc_function ( read_rc_loc_head () );
}

static
uint32_t get_rc_lineno ( uint32_t idx )
{
    return RC_loc_queue [ idx & RC_LOC_QUEUE_MASK ] . lineno;
}

LIB_EXPORT uint32_t CC GetRCLineno ( void )
{
    return get_rc_lineno ( read_rc_loc_head () );
}

static
rc_t get_rc_code ( uint32_t idx )
{
    return RC_loc_queue [ idx & RC_LOC_QUEUE_MASK ] . rc;
}

LIB_EXPORT rc_t CC SetRCFileFuncLine ( rc_t rc, const char *filename, const char *funcname, uint32_t lineno )
{
    /* the limit based upon last guy successfully written */
    int32_t lim = atomic32_read ( & RC_loc_written ) + RC_LOC_QUEUE_SIZE;

    /* try to reserve a slot for writing */
    int32_t rsrv = atomic32_read_and_add_lt ( & RC_loc_reserve, 1, lim ) + 1;

    /* see if we got the reservation */
    if ( rsrv <= lim )
    {
        uint32_t idx = rsrv & RC_LOC_QUEUE_MASK;
        RC_loc_queue [ idx ] . filename = filename;
        RC_loc_queue [ idx ] . function = funcname;
        RC_loc_queue [ idx ] . lineno = lineno;
        RC_loc_queue [ idx ] . rc = rc;
        /* TBD - proper release sequence */
        atomic32_set ( & RC_loc_written, rsrv );
    }

    return rc;
}

/* GetUnreadRCInfo
 *  expected to be called after all threads are quiet
 */
LIB_EXPORT bool CC GetUnreadRCInfo ( rc_t *rc, const char **filename, const char **funcname, uint32_t *lineno )
{
    int32_t last_writ;

    reporting_unread = true;

    /* these are not atomic, but the ordering is important */
    last_writ = atomic32_read ( & RC_loc_written );
    if ( last_writ > 0 )
    {
        /* this arrangement attempts to make the access to
           RC_loc_read dependent upon access to RC_loc_written */
        int32_t last_read = atomic32_read ( & RC_loc_read );
        if ( last_read < last_writ )
        {
            /* check reserved slots */
            int32_t rsrv = atomic32_read ( & RC_loc_reserve );

            /* adjust last read */
            if ( last_writ - last_read > RC_LOC_QUEUE_SIZE )
                last_read = last_writ - RC_LOC_QUEUE_SIZE;

            /* any reserved rows must be considered overwritten */
            last_read += rsrv - last_writ;

            /* these are the rows we can report */
            if ( last_read < last_writ )
            {
                int32_t idx = last_read + 1;
                atomic32_set ( & RC_loc_read, idx );

                if ( filename != NULL )
                    * filename = get_rc_filename ( idx );
                if ( funcname != NULL )
                    * funcname = get_rc_function ( idx );
                if ( lineno != NULL )
                    * lineno = get_rc_lineno ( idx );
                if ( rc != NULL )
                    * rc = get_rc_code ( idx );

                return true;
            }
        }
    }
    reporting_unread = false;
    return false;
}
