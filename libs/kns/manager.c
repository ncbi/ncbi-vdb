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

#include <kns/extern.h>

#include <kfg/config.h>
#include <kfg/properties.h>

#include <klib/base64.h>
#include <klib/data-buffer.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/strings.h>
#include <klib/writer.h>

#include <kproc/lock.h>
#include <kproc/timeout.h>

#include <kns/http.h>
#include <kns/manager.h>
#include <kns/socket.h>

#include <cloud/manager.h>

#include <atomic.h> /* atomic_ptr_t */
#include <ctype.h>
#include <sysalloc.h>

#include <assert.h>

#if LINUX
#include <gnu/libc-version.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#include <stdio.h> /* fprintf */

#include "../klib/release-vers.h"
#include "http-priv.h"
#include "kns_manager-singleton.h" /* USE_SINGLETON */
#include "mgr-priv.h"
#include "stream-priv.h"
#include "sysmgr.h"

#ifndef MAX_CONN_LIMIT
#define MAX_CONN_LIMIT ( 60 * 1000 )
#endif

#ifndef MAX_CONN_READ_LIMIT
#define MAX_CONN_READ_LIMIT ( 10 * 60 * 1000 )
#endif

#ifndef MAX_CONN_WRITE_LIMIT
#define MAX_CONN_WRITE_LIMIT ( 10 * 60 * 1000 )
#endif

static KDataBuffer kns_manager_user_agent;
static KDataBuffer kns_manager_user_agent_append;

static KLock *kns_manager_lock = NULL; /* Protects below */
static KDataBuffer kns_manager_clientip;
static KDataBuffer kns_manager_sessionid;
static KDataBuffer kns_manager_pagehitid;
static KDataBuffer kns_manager_ua_suffix;
static KDataBuffer kns_manager_guid;

#if USE_SINGLETON
static atomic_ptr_t kns_singleton;
#endif

/*
#define RELEASE( type, obj ) do { rc_t rc2 = type##Release ( obj ); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while ( false )
*/

bool KNSManagerHttpProxyOnly ( const KNSManager *self )
{
    if ( self == NULL ) { return false; }

    return KNSProxiesHttpProxyOnly ( self->proxies );
}

struct KNSProxies *KNSManagerGetProxies ( const KNSManager *self, size_t *cnt )
{
    if ( self == NULL ) { return NULL; }
    size_t dummy;
    if ( cnt == NULL ) { cnt = &dummy; }
    return KNSProxiesGetHttpProxy ( self->proxies, cnt );
}


static rc_t KNSManagerWhack ( KNSManager *self )
{
    rc_t rc;

#if USE_SINGLETON
    KNSManager *our_mgr
        = atomic_test_and_set_ptr ( &kns_singleton, NULL, NULL );
    if ( self == our_mgr ) {
        if ( !self->notSingleton ) { return 0; }
        atomic_test_and_set_ptr ( &kns_singleton, NULL, self );
    }
#endif

    KNSProxiesWhack ( self->proxies );
    CloudMgrRelease ( self->cloud );

    rc = HttpRetrySpecsDestroy ( &self->retry_specs );

    KTLSGlobalsWhack ( &self->tlsg );

    free ( self );

    KNSManagerCleanup ();
    if ( kns_manager_lock ) {
        KLockRelease ( kns_manager_lock );
        kns_manager_lock = NULL;
    }
    if ( !rc ) rc = KDataBufferWhack ( &kns_manager_user_agent );
    if ( !rc ) rc = KDataBufferWhack ( &kns_manager_user_agent_append );
    if ( !rc ) rc = KDataBufferWhack ( &kns_manager_clientip );
    if ( !rc ) rc = KDataBufferWhack ( &kns_manager_sessionid );
    if ( !rc ) rc = KDataBufferWhack ( &kns_manager_pagehitid );
    if ( !rc ) rc = KDataBufferWhack ( &kns_manager_ua_suffix );
    if ( !rc ) rc = KDataBufferWhack ( &kns_manager_guid );

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerAddRef ( const KNSManager *self )
{
    if ( self != NULL ) {
        switch ( KRefcountAdd ( &self->refcount, "KNSManager" ) ) {
        case krefLimit:
            return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcExcessive );
        case krefNegative:
            return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
        }
    }
    return 0;
}


LIB_EXPORT rc_t CC KNSManagerRelease ( const KNSManager *self )
{
    if ( self != NULL ) {
        switch ( KRefcountDrop ( &self->refcount, "KNSManager" ) ) {
        case krefWhack: return KNSManagerWhack ( (KNSManager *)self );
        case krefNegative:
            return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
        }
    }
    return 0;
}

static rc_t CC KNSManagerMakeConfigImpl ( KNSManager **mgrp, KConfig *kfg );


static rc_t CC KNSManagerMakeSingleton (
    KNSManager **mgrp, KConfig *aKfg, bool local )
{
    rc_t rc = 0;

    if ( mgrp == NULL ) {
        rc = RC ( rcNS, rcMgr, rcAllocating, rcParam, rcNull );
    } else {
        KConfig *kfg = aKfg;
        KNSManager *our_mgr = NULL;

        *mgrp = NULL;

#if USE_SINGLETON
        if ( !local ) {
            /* grab single-shot singleton */
            our_mgr = atomic_test_and_set_ptr ( &kns_singleton, NULL, NULL );
            if ( our_mgr != NULL ) {
                /* add a new reference and return */
                rc = KNSManagerAddRef ( our_mgr );
                if ( rc == 0 ) { *mgrp = our_mgr; }
                return rc;
            }
        }
#endif

        /* singleton was NULL. Make from scratch. */
        if ( kfg == NULL ) { rc = KConfigMake ( &kfg, NULL ); }

        if ( rc == 0 ) {
            if ( !kns_manager_lock ) {
                rc = KLockMake ( &kns_manager_lock );
                if ( rc ) { return rc; }
            }
            rc = KDataBufferMakeBytes ( &kns_manager_user_agent, 0 );
            if ( rc ) { return rc; }
            rc = KDataBufferPrintf ( &kns_manager_user_agent, "%s", "" );
            if ( rc ) { return rc; }

            rc = KDataBufferMakeBytes ( &kns_manager_user_agent_append, 0 );
            if ( rc ) { return rc; }
            rc = KDataBufferPrintf ( &kns_manager_user_agent_append, "%s", "" );
            if ( rc ) { return rc; }

            rc = KDataBufferMakeBytes ( &kns_manager_clientip, 0 );
            if ( rc ) { return rc; }
            rc = KDataBufferPrintf ( &kns_manager_clientip, "%s", "" );
            if ( rc ) { return rc; }

            rc = KDataBufferMakeBytes ( &kns_manager_sessionid, 0 );
            if ( rc ) { return rc; }
            rc = KDataBufferPrintf ( &kns_manager_sessionid, "%s", "" );
            if ( rc ) { return rc; }

            rc = KDataBufferMakeBytes ( &kns_manager_pagehitid, 0 );
            if ( rc ) { return rc; }
            rc = KDataBufferPrintf ( &kns_manager_pagehitid, "%s", "" );
            if ( rc ) { return rc; }

            rc = KDataBufferMakeBytes ( &kns_manager_ua_suffix, 0 );
            if ( rc ) { return rc; }
            rc = KDataBufferPrintf ( &kns_manager_ua_suffix, "%s", "" );
            if ( rc ) { return rc; }

            rc = KDataBufferMakeBytes ( &kns_manager_guid, 0 );
            if ( rc ) { return rc; }
            rc = KDataBufferPrintf ( &kns_manager_guid, "%s", "" );
            if ( rc ) { return rc; }

            rc = KNSManagerMakeConfigImpl ( &our_mgr, kfg );

            if ( aKfg == NULL ) { KConfigRelease ( kfg ); }

            if ( rc == 0 ) {
#if USE_SINGLETON
                if ( !local ) {
                    /* try to set single-shot ( set once, never reset ) */
                    KNSManager *new_mgr = atomic_test_and_set_ptr (
                        &kns_singleton, our_mgr, NULL );
                    if ( new_mgr != NULL ) {
                        /* somebody else got here first - drop our version */
                        assert ( our_mgr != new_mgr );
                        KNSManagerRelease ( our_mgr );

                        /* use the new manager,
                           just add a reference and return */
                        rc = KNSManagerAddRef ( new_mgr );
                        if ( rc == 0 ) { *mgrp = new_mgr; }
                        return rc;
                    }
                }
#endif
                /* return parameter */
                *mgrp = our_mgr;
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMake ( KNSManager **mgrp )
{
    return KNSManagerMakeSingleton ( mgrp, NULL, false );
}

LIB_EXPORT void KNSManagerSetVerbose ( KNSManager *self, bool verbosity )
{
    if ( self != NULL ) { self->verbose = verbosity; }
}


LIB_EXPORT bool KNSManagerIsVerbose ( const KNSManager *self )
{
    return ( self != NULL ) ? self->verbose : false;
}


/* MakeConnection
 *  create a connection-oriented stream
 *
 *  "conn" [ OUT ] - a stream for communication with the server
 *
 *  "from" [ IN ] - client endpoint
 *
 *  "to" [ IN ] - server endpoint
 *
 *  both endpoints have to be of type epIP; creates a TCP connection
 */
LIB_EXPORT rc_t CC KNSManagerMakeConnection ( const KNSManager *self,
    struct KSocket **conn, struct KEndPoint const *from,
    struct KEndPoint const *to )
{
    timeout_t tm;

    if ( self == NULL ) {
        if ( conn == NULL ) {
            return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
        }

        *conn = NULL;

        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    }

    TimeoutInit ( &tm, self->conn_timeout );

    return KNSManagerMakeRetryTimedConnection ( self, conn, &tm,
        self->conn_read_timeout, self->conn_write_timeout, from, to );
}
/* MakeTimedConnection
 *  create a connection-oriented stream
 *
 *  "conn" [ OUT ] - a stream for communication with the server
 *
 *  "retryTimeout" [ IN ] - if connection is refused, retry with 1ms intervals:
 * when negative, retry infinitely, when 0, do not retry, positive gives maximum
 * wait time in seconds
 *
 *  "readMillis" [ IN ] and "writeMillis" - when negative, infinite timeout
 *   when 0, return immediately, positive gives maximum wait time in mS
 *   for reads and writes respectively.
 *
 *  "from" [ IN ] - client endpoint
 *
 *  "to" [ IN ] - server endpoint
 *
 *  both endpoints have to be of type epIP; creates a TCP connection
 */
LIB_EXPORT rc_t CC KNSManagerMakeTimedConnection (
    struct KNSManager const *self, struct KSocket **conn, int32_t readMillis,
    int32_t writeMillis, struct KEndPoint const *from,
    struct KEndPoint const *to )
{
    timeout_t tm;

    if ( self == NULL ) {
        if ( conn == NULL ) {
            return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
        }

        *conn = NULL;

        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    }

    TimeoutInit ( &tm, self->conn_timeout );

    return KNSManagerMakeRetryTimedConnection (
        self, conn, &tm, readMillis, writeMillis, from, to );
}

/* MakeRetryConnection
 *  create a connection-oriented stream
 *
 *  "conn" [ OUT ] - a stream for communication with the server
 *
 *  "retryTimeout" [ IN ] - if connection is refused, retry with 1ms intervals:
 * when negative, retry infinitely, when 0, do not retry, positive gives maximum
 * wait time in seconds
 *
 *  "from" [ IN ] - client endpoint
 *
 *  "to" [ IN ] - server endpoint
 *
 *  both endpoints have to be of type epIP; creates a TCP connection
 */
LIB_EXPORT rc_t CC KNSManagerMakeRetryConnection (
    struct KNSManager const *self, struct KSocket **conn,
    timeout_t *retryTimeout, struct KEndPoint const *from,
    struct KEndPoint const *to )
{
    if ( self == NULL ) {
        if ( conn == NULL ) {
            return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
        }

        *conn = NULL;

        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    }

    return KNSManagerMakeRetryTimedConnection ( self, conn, retryTimeout,
        self->conn_read_timeout, self->conn_write_timeout, from, to );
}

/* SetConnectionTimeouts
 *  sets default connect/read/write timeouts to supply to sockets
 *
 *  "connectMillis", "readMillis", "writeMillis" [ IN ] - when negative,
 * infinite timeout when 0, return immediately, positive gives maximum wait time
 * in sec/mS for connects, reads and writes respectively.
 */
LIB_EXPORT rc_t CC KNSManagerSetConnectionTimeouts ( KNSManager *self,
    int32_t connectMillis, int32_t readMillis, int32_t writeMillis )
{
    if ( self == NULL ) {
        return RC ( rcNS, rcMgr, rcUpdating, rcSelf, rcNull );
    }

    /* limit values */
    if ( connectMillis < 0 || connectMillis > MAX_CONN_LIMIT ) {
        connectMillis = MAX_CONN_LIMIT;
    }

    if ( readMillis < 0 || readMillis > MAX_CONN_READ_LIMIT ) {
        readMillis = MAX_CONN_READ_LIMIT;
    }

    if ( writeMillis < 0 || writeMillis > MAX_CONN_WRITE_LIMIT ) {
        writeMillis = MAX_CONN_WRITE_LIMIT;
    }

    self->conn_timeout = connectMillis;
    self->conn_read_timeout = readMillis;
    self->conn_write_timeout = writeMillis;

    return 0;
}


/* SetHTTPTimeouts
 *  sets default read/write timeouts to supply to HTTP connections
 *
 *  "readMillis" [ IN ] and "writeMillis" - when negative, infinite timeout
 *  when 0, return immediately, positive gives maximum wait time in mS
 *  for reads and writes respectively.
 */
LIB_EXPORT rc_t CC KNSManagerSetHTTPTimeouts (
    KNSManager *self, int32_t readMillis, int32_t writeMillis )
{
    if ( self == NULL ) {
        return RC ( rcNS, rcMgr, rcUpdating, rcSelf, rcNull );
    }

    /* limit values */
    if ( readMillis < 0 || readMillis > MAX_HTTP_READ_LIMIT ) {
        readMillis = MAX_HTTP_READ_LIMIT;
    }

    if ( writeMillis < 0 || writeMillis > MAX_HTTP_WRITE_LIMIT ) {
        writeMillis = MAX_HTTP_WRITE_LIMIT;
    }

    self->http_read_timeout = readMillis;
    self->http_write_timeout = writeMillis;

    return 0;
}

/* GetHTTPProxyPath
 * DEPRECATED
 *  returns path to HTTP proxy server ( if set ) or NULL.
 *  return status is 0 if the path is valid, non-zero otherwise
 */
LIB_EXPORT rc_t CC KNSManagerGetHTTPProxyPath (
    const KNSManager *self, const String **proxy )
{
    fprintf ( stderr,
        "WARNING : KNSManagerGetHTTPProxyPath IS DEPRECATED AND SHOULD NOT BE "
        "USED\n" );
    return KNSProxiesGetHttpProxyPath ( self->proxies, proxy );
}


/* SetHTTPProxyPath
 *  sets a path to HTTP proxy server.
 *  a NULL path value removes all proxy settings.
 *
 *  the VPath passed in must still be released using VPathRelease,
 *  because KNSManager will attach a new reference to it.
 */
LIB_EXPORT rc_t CC KNSManagerSetHTTPProxyPath (
    KNSManager *self, const char *fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );
    rc = KNSManagerVSetHTTPProxyPath ( self, fmt, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerVSetHTTPProxyPath (
    KNSManager *self, const char *fmt, va_list args )
{
    if ( self == NULL ) {
        return RC ( rcNS, rcMgr, rcUpdating, rcSelf, rcNull );
    }

    return KNSProxiesVSetHTTPProxyPath ( self->proxies, fmt, args, true );
}

static rc_t KNSManagerHttpProxyInit ( KNSManager *self, const KConfig *kfg )
{
    assert ( self );
    self->proxies = KNSManagerKNSProxiesMake ( self, kfg );
    if ( self->proxies == NULL ) {
        return RC ( rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted );
    }

    return 0;
}

/* GetHTTPProxyEnabled
 *  returns true if a non-NULL proxy path exists and user wants to use it
 *  users indicate desire to use proxy through configuration
 *  or SetHTTPProxyEnabled
 */
LIB_EXPORT bool CC KNSManagerGetHTTPProxyEnabled ( const KNSManager *self )
{
    if ( self != NULL ) {
        return KNSProxiesGetHTTPProxyEnabled ( self->proxies );
    }

    return false;
}

/* SetHTTPProxyEnabled
 *  sets http-proxy enabled state to supplied value
 *  returns the prior value as a convenience
 */
LIB_EXPORT bool CC KNSManagerSetHTTPProxyEnabled (
    KNSManager *self, bool enabled )
{
    bool prior = false;
    if ( self != NULL ) {
        prior = KNSProxiesSetHTTPProxyEnabled ( self->proxies, enabled );
    }
    return prior;
}


static void KNSManagerSetNCBI_VDB_NET ( KNSManager *self, const KConfig *kfg )
{
    rc_t rc = 0;

    const KConfigNode *node = NULL;

    if ( self == NULL || kfg == NULL ) { return; }

    rc = KConfigOpenNodeRead ( kfg, &node, "/libs/kns/NCBI_VDB_NET" );
    if ( rc != 0 ) {
        self->NCBI_VDB_NETkfgValueSet = self->NCBI_VDB_NETkfgValue = false;
        return;
    }

    char buffer[1] = "";
    size_t num_read = 0;
    self->NCBI_VDB_NETkfgValueSet = true;
    KConfigNodeRead ( node, 0, buffer, sizeof buffer, &num_read, 0 );
    if ( num_read == 0 ) {
        self->NCBI_VDB_NETkfgValue = false;
    } else {
        switch ( buffer[0] ) {
        case '0':
        case 'f': /* false */ self->NCBI_VDB_NETkfgValue = false; break;
        default: self->NCBI_VDB_NETkfgValue = true; break;
        }
    }


    KConfigNodeRelease ( node );
    node = NULL;
}


/* VDB-DESIREMENTS:
1. to call *[s]/kfg/properties* to read configuration
2. to create a header file to keep constants (node names) */
static int32_t KNSManagerPrepareConnTimeout ( KConfig *kfg )
{
    int64_t result = 0;
    rc_t rc = KConfigReadI64 ( kfg, "/libs/kns/connect/timeout", &result );
    if ( rc != 0 || result < 0 ) { return MAX_CONN_LIMIT; }

    return result;
}
static int32_t KNSManagerPrepareConnReadTimeout ( KConfig *kfg )
{
    int64_t result = 0;
    rc_t rc = KConfigReadI64 ( kfg, "/libs/kns/connect/timeout/read", &result );
    if ( rc != 0 || result < 0 ) { return MAX_CONN_READ_LIMIT; }

    return result;
}
static int32_t KNSManagerPrepareConnWriteTimeout ( KConfig *kfg )
{
    int64_t result = 0;
    rc_t rc
        = KConfigReadI64 ( kfg, "/libs/kns/connect/timeout/write", &result );
    if ( rc != 0 || result < 0 ) { return MAX_CONN_WRITE_LIMIT; }

    return result;
}

static int32_t KNSManagerPrepareHttpReadTimeout ( KConfig *kfg )
{
    int64_t result = 0;
    rc_t rc = KConfigReadI64 ( kfg, "/http/timeout/read", &result );
    if ( rc != 0 || result < 0 ) { return MAX_HTTP_READ_LIMIT; }

    return result;
}
static int32_t KNSManagerPrepareHttpWriteTimeout ( KConfig *kfg )
{
    int64_t result = 0;
    rc_t rc = KConfigReadI64 ( kfg, "/http/timeout/write", &result );
    if ( rc != 0 || result < 0 ) { return MAX_HTTP_WRITE_LIMIT; }

    return result;
}

#if 0
static bool KNSManagerPrepareLogTlsErrors(KConfig* kfg) {
    const char * e = getenv("NCBI_VDB_TLS_LOG_ERR");
    if (e != NULL)
        if (e[0] == '\0')
            return true;
        else {
            if (e[0] == '0' ||
                e[0] == 'f') /* false */
            {
                return false;
            }
            else
                return true;
        }
    else {
        bool log = false;
        rc_t rc = KConfigReadBool(kfg, "/tls/NCBI_VDB_TLS_LOG_ERR", &log);
        if (rc != 0)
            return false;
        else
            return log;
    }
}

static int KNSManagerPrepareEmulateTldReadErrors(KConfig* kfg) {
    const char * e = getenv("NCBI_VDB_ERR_MBEDTLS_READ");
    if (e != NULL)
        return atoi(e);
    else {
        int64_t emult = 0;
        rc_t rc = KConfigReadI64(kfg, "/tls/NCBI_VDB_ERR_MBEDTLS_READ", &emult);
        if (rc != 0)
            return 0;
        else
            return emult;
    }
}
#endif

static bool KNSManagerPrepareResolveToCache ( KConfig *kfg )
{
    /* VResolverCache resolve to user's cache vs. cwd/AD */
    bool reslt = true;

    /* TODO: call ncbi-vdb/interfaces/kfg/properties.h for exact key name */
    rc_t rc
        = KConfigReadBool ( kfg, "/tools/prefetch/download_to_cache", &reslt );
    if ( rc == 0 ) { return reslt; }

    return true;
}

static bool KNSManagerPrepareAcceptAwsCharges ( KConfig *kfg )
{
    bool reslt = false;

    /* TODO: call ncbi-vdb/interfaces/kfg/properties.h for exact key name */
    rc_t rc = KConfigReadBool ( kfg, "/libs/cloud/accept_aws_charges", &reslt );
    if ( rc == 0 ) { return reslt; }

    return false;
}

static bool KNSManagerPrepareAcceptGcpCharges ( KConfig *kfg )
{
    bool reslt = false;

    /* TODO: call ncbi-vdb/interfaces/kfg/properties.h for exact key name */
    rc_t rc = KConfigReadBool ( kfg, "/libs/cloud/accept_gcp_charges", &reslt );
    if ( rc == 0 ) { return reslt; }

    return false;
}

static rc_t CC KNSManagerMakeConfigImpl ( KNSManager **mgrp, KConfig *kfg )
{
    rc_t rc;

    if ( mgrp == NULL || kfg == NULL ) {
        rc = RC ( rcNS, rcMgr, rcAllocating, rcParam, rcNull );
    } else {
        KNSManager *mgr = calloc ( 1, sizeof *mgr );
        if ( mgr == NULL ) {
            rc = RC ( rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted );
        } else {
            KRefcountInit ( &mgr->refcount, 1, "KNSManager", "init", "kns" );
            mgr->conn_timeout = KNSManagerPrepareConnTimeout ( kfg );
            mgr->conn_read_timeout = KNSManagerPrepareConnReadTimeout ( kfg );
            mgr->conn_write_timeout = KNSManagerPrepareConnWriteTimeout ( kfg );
            mgr->http_read_timeout = KNSManagerPrepareHttpReadTimeout ( kfg );
            mgr->http_write_timeout = KNSManagerPrepareHttpWriteTimeout ( kfg );
            mgr->maxTotalWaitForReliableURLs_ms = 10 * 60 * 1000; /* 10 min */
            mgr->maxNumberOfRetriesOnFailureForReliableURLs = 10;

            /*          mgr->logTlsErrors = KNSManagerPrepareLogTlsErrors(kfg);
                        mgr->emulateTlsReadErrors
                            = KNSManagerPrepareEmulateTldReadErrors(kfg); */

            mgr->resolveToCache = KNSManagerPrepareResolveToCache ( kfg );

            mgr->accept_aws_charges = KNSManagerPrepareAcceptAwsCharges ( kfg );
            mgr->accept_gcp_charges = KNSManagerPrepareAcceptGcpCharges ( kfg );

            if ( strlen ( kns_manager_guid.base ) == 0 ) {
                rc = KDataBufferResize ( &kns_manager_guid, 37 );
                size_t written = 0;
                KConfig_Get_GUID ( kfg, kns_manager_guid.base,
                    KDataBufferBytes ( &kns_manager_guid ), &written );
                assert ( written <= 37 );
            }

            rc = KNSManagerInit (); /* platform specific init in sysmgr.c ( in
                                       unix|win etc. subdir ) */
            if ( rc == 0 ) {
                /* the manager is not a proper singleton */
                if ( strlen ( kns_manager_user_agent.base ) == 0 ) {
                    ver_t version = RELEASE_VERS;
                    KNSManagerSetUserAgent (
                        mgr, PKGNAMESTR " ncbi-vdb.%V", version );
                }

                rc = HttpRetrySpecsInit ( &mgr->retry_specs, kfg );
                if ( rc == 0 ) {
                    rc = KTLSGlobalsInit ( &mgr->tlsg, kfg );

                    if ( rc == 0 ) {
                        rc = KNSManagerHttpProxyInit ( mgr, kfg );
                    }

                    if ( rc == 0 ) {
                        KNSManagerSetNCBI_VDB_NET ( mgr, kfg );

                        *mgrp = mgr;

                        /*
                        printf("KNSManager.conn_timeout(%d) = %d\n",
                        MAX_CONN_LIMIT, mgr->conn_timeout);
                        printf("KNSManager.conn_read_timeout(%d) = %d\n",
                        MAX_CONN_READ_LIMIT, mgr->conn_read_timeout);
                        printf("KNSManager.conn_write_timeout(%d) = %d\n",
                        MAX_CONN_WRITE_LIMIT, mgr->conn_write_timeout);
                        printf("KNSManager.http_read_timeout(%d) = %d\n",
                        MAX_HTTP_READ_LIMIT, mgr->http_read_timeout);
                        printf("KNSManager.http_write_timeout(%d) = %d\n",
                        MAX_HTTP_WRITE_LIMIT, mgr->http_write_timeout);
                        */

                        return 0;
                    }
                }
            }

            free ( mgr );
        }

        *mgrp = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMakeWithConfig ( KNSManager **mgrp, KConfig *kfg )
{
    return KNSManagerMakeSingleton ( mgrp, kfg, false );
}

LIB_EXPORT rc_t CC KNSManagerMakeLocal ( KNSManager **mgrp, KConfig *kfg )
{
    return KNSManagerMakeSingleton ( mgrp, kfg, true );
}

LIB_EXPORT rc_t CC KNSManagerMakeConfig ( KNSManager **mgrp, KConfig *kfg )
{
    return KNSManagerMakeLocal ( mgrp, kfg );
}

LIB_EXPORT rc_t CC KNSManagerSetUserAgent (
    KNSManager *self, const char *fmt, ... )
{
    /* 6/18/14 - don't check "self", since the current implementation
       is actually static. Later implementations will not be...
       Cast silences warning
       */
    (void)( self );

    rc_t rc = 0;
    if ( fmt == NULL ) {
        rc = RC ( rcNS, rcMgr, rcUpdating, rcParam, rcNull );
        return rc;
    }

    KDataBufferResize ( &kns_manager_user_agent, 0 );
    va_list args;
    va_start ( args, fmt );
    rc = KDataBufferVPrintf ( &kns_manager_user_agent, fmt, args );
    va_end ( args );

    return rc;
}


LIB_EXPORT rc_t CC KNSManagerGetUserAgent ( const char **user_agent )
{
    rc_t rc = 0;
    if ( user_agent == NULL ) {
        rc = RC ( rcNS, rcMgr, rcAccessing, rcParam, rcNull );
        return rc;
    }

    char cloudtrunc[64];
    const char *cloudid = getenv ( ENV_MAGIC_CE_TOKEN );
    if ( cloudid && strlen ( cloudid ) > 8 ) {
        /* AWS access keys should always begin with AKIA,
         * suffixes seems non-random */
        strncpy ( cloudtrunc, cloudid + 4, sizeof cloudtrunc );
        cloudtrunc[3] = '\0';
        assert ( strlen ( cloudtrunc ) < ( sizeof cloudtrunc - 1 ) );
    } else {
        strcpy ( cloudtrunc, "noc" );
    }

    const char *sessid = getenv ( ENV_VAR_SESSION_ID );
    if ( sessid == NULL ) { sessid = "nos"; }

    const char *libc_version = "";
#if LINUX
    libc_version = gnu_get_libc_version ();
#endif

    /* Sometimes called before KNSManagerMake */
    const char *guid = "nog";

    if ( ( KDataBufferBytes ( &kns_manager_guid ) == 0 )
        || ( strlen ( kns_manager_guid.base ) == 0 ) ) {
        KConfig *kfg = NULL;
        KConfigMake ( &kfg, NULL );
        size_t written = 0;

        rc = KDataBufferResize ( &kns_manager_guid, 37 );
        if ( rc ) {
            /* Some tests whack guid */
            rc = KDataBufferMakeBytes ( &kns_manager_guid, 37 );
            if ( rc ) { return rc; }
        }

        KConfig_Get_GUID ( kfg, kns_manager_guid.base,
            KDataBufferBytes ( &kns_manager_guid ), &written );
        assert ( written <= 37 );

        if ( kfg ) KConfigRelease ( kfg );
    }

    if ( strlen ( kns_manager_guid.base ) ) guid = kns_manager_guid.base;

    KDataBuffer phid;
    KDataBufferMakeBytes ( &phid, 0 );
    rc = KDataBufferPrintf (
        &phid, "%.3s%.4s%.3s,libc=%s", cloudtrunc, guid, sessid, libc_version );
    if ( rc ) { return rc; }

    if ( kns_manager_lock ) {
        rc_t rc = KLockAcquire ( kns_manager_lock );
        if ( rc ) { return rc; }
    }

    KDataBuffer sessids;
    KDataBufferMakeBytes ( &sessids, 0 );

    /* Some tests call before these are initialized */
    if ( KDataBufferBytes ( &kns_manager_clientip ) == 0 )
        KDataBufferPrintf ( &kns_manager_clientip, "%s", "" );
    if ( KDataBufferBytes ( &kns_manager_sessionid ) == 0 )
        KDataBufferPrintf ( &kns_manager_sessionid, "%s", "" );
    if ( KDataBufferBytes ( &kns_manager_pagehitid ) == 0 )
        KDataBufferPrintf ( &kns_manager_pagehitid, "%s", "" );
    if ( KDataBufferBytes ( &kns_manager_user_agent ) == 0 )
        KDataBufferPrintf ( &kns_manager_user_agent, "%s", "" );
    if ( KDataBufferBytes ( &kns_manager_ua_suffix ) == 0 )
        KDataBufferPrintf ( &kns_manager_ua_suffix, "%s", "" );

    /* If any are populated, emit all */
    if ( strlen ( kns_manager_clientip.base )
        || strlen ( kns_manager_sessionid.base )
        || strlen ( kns_manager_pagehitid.base ) ) {
        rc = KDataBufferPrintf ( &sessids, "cip=%s,sid=%s,pagehit=%s",
            kns_manager_clientip.base, kns_manager_sessionid.base,
            kns_manager_pagehitid.base );
    }

    if ( kns_manager_lock ) { KLockUnlock ( kns_manager_lock ); }

    if ( rc ) { return rc; }

    KDataBufferResize ( &kns_manager_user_agent_append, 0 );

    if ( sessids.base && strlen ( sessids.base ) ) {
        const String *b64;
        encodeBase64 ( &b64, sessids.base, strlen ( sessids.base ) );
        rc = KDataBufferPrintf ( &kns_manager_user_agent_append,
            "%s%s (phid=%s,%s)", kns_manager_user_agent.base,
            kns_manager_ua_suffix.base, phid.base, b64->addr );
        StringWhack ( b64 );
    } else {
        KDataBufferPrintf ( &kns_manager_user_agent_append, "%s%s (phid=%s)",
            kns_manager_user_agent.base, kns_manager_ua_suffix.base,
            phid.base );
    }

    KDataBufferWhack ( &phid );
    KDataBufferWhack ( &sessids );

    ( *user_agent ) = kns_manager_user_agent_append.base;
    return rc;
}


/******************************************************************************/

#define NCBI_VDB_NET                                                           \
    1 /* VDB-3399 : temporarily enable for internal testing                    \
       */

void KNSManagerSetLogNcbiVdbNetError ( KNSManager *self, bool set )
{
    if ( self ) { self->NCBI_VDB_NETnoLogError = !set; }
}

bool KNSManagerLogNcbiVdbNetError ( const KNSManager *self )
{
    if ( self == NULL ) {
#ifdef NCBI_VDB_NET
        return true;
#else
        return false;
#endif
    }
    if ( !self->logTlsErrors ) { return false; }

    if ( self->NCBI_VDB_NETnoLogError ) { return false; }
    const char *e = getenv ( "NCBI_VDB_NET" );
    if ( e != NULL ) {
        if ( e[0] == '0' || e[0] == 'f' ) /* false */
        {
            return false;
        }

        return true;
    }
    if ( self->NCBI_VDB_NETkfgValueSet ) { return self->NCBI_VDB_NETkfgValue; }


#ifdef NCBI_VDB_NET
    return true;
#else
    return false;
#endif
}

LIB_EXPORT rc_t CC KNSManagerSetAdCaching (
    struct KNSManager *self, bool enabled )
{
    if ( self != NULL ) { self->enabledResolveToAd = enabled; }
    return 0;
}
/*
LIB_EXPORT rc_t CC KNSManagerSetClientIPv4 (
    KNSManager *self, uint32_t client_ipv4_addr)
{
    if ( self == NULL )
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
    }

    char str[INET6_ADDRSTRLEN];
    struct in_addr ina;

    ina.s_addr=client_ipv4_addr;

    if (inet_ntop(AF_INET, &ina, str, sizeof str)==NULL)
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );

    if ( kns_manager_lock ) {
        rc_t rc = KLockAcquire ( kns_manager_lock );
        if ( rc ) { return rc; }
    }
    string_copy ( kns_manager_clientip, sizeof kns_manager_clientip, ipv4,
        strlen ( ipv4 ) );

    if ( kns_manager_lock ) { KLockUnlock ( kns_manager_lock ); }
    return 0;
}

LIB_EXPORT rc_t CC KNSManagerSetClientIPv6 (
    KNSManager *self, uint16_t client_ipv6_addr[])
{
    if ( self == NULL )
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
    }

    char str[INET6_ADDSTRLEN];
    struct in6_addr ina;

    memcpy(&ina.s6_addr,client_ipv6_addr, 16);
    ina.s_addr=client_ipv6_addr;

    if (inet_ntop(AF_INET6, &ina, str, sizeof str)==NULL)
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );

    if ( kns_manager_lock ) {
        rc_t rc = KLockAcquire ( kns_manager_lock );
        if ( rc ) { return rc; }
    }
    string_copy ( kns_manager_clientip, sizeof kns_manager_clientip, ipv4,
        strlen ( ipv4 ) );

    if ( kns_manager_lock ) { KLockUnlock ( kns_manager_lock ); }
    return 0;
}
*/
LIB_EXPORT rc_t CC KNSManagerSetUserAgentSuffix ( const char *suffix )
{
    if ( suffix == NULL ) {
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
    }

    KDataBufferResize ( &kns_manager_ua_suffix, 0 );
    return KDataBufferPrintf ( &kns_manager_ua_suffix, "%s", suffix );
}

LIB_EXPORT rc_t CC KNSManagerSetClientIP (
    KNSManager *self, const char *clientip )
{
    if ( self == NULL || clientip == NULL ) {
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
    }

    rc_t rc = 0;
    if ( kns_manager_lock ) {
        rc = KLockAcquire ( kns_manager_lock );
        if ( rc ) { return rc; }
    }

    KDataBufferResize ( &kns_manager_clientip, 0 );
    rc = KDataBufferPrintf ( &kns_manager_clientip, "%s", clientip );

    if ( kns_manager_lock ) { KLockUnlock ( kns_manager_lock ); }

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerSetSessionID (
    KNSManager *self, const char *sessionid )
{
    if ( self == NULL || sessionid == NULL ) {
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
    }

    rc_t rc = 0;
    if ( kns_manager_lock ) {
        rc = KLockAcquire ( kns_manager_lock );
        if ( rc ) { return rc; }
    }

    KDataBufferResize ( &kns_manager_sessionid, 0 );
    rc = KDataBufferPrintf ( &kns_manager_sessionid, "%s", sessionid );

    if ( kns_manager_lock ) { KLockUnlock ( kns_manager_lock ); }

    return rc;
}


LIB_EXPORT rc_t CC KNSManagerSetPageHitID (
    KNSManager *self, const char *pagehitid )
{
    if ( self == NULL || pagehitid == NULL ) {
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
    }

    rc_t rc = 0;
    if ( kns_manager_lock ) {
        rc = KLockAcquire ( kns_manager_lock );
        if ( rc ) { return rc; }
    }

    KDataBufferResize ( &kns_manager_pagehitid, 0 );
    rc = KDataBufferPrintf ( &kns_manager_pagehitid, "%s", pagehitid );

    if ( kns_manager_lock ) { KLockUnlock ( kns_manager_lock ); }

    return rc;
}
