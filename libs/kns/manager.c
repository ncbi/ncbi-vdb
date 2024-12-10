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

#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */

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
#include <strtol.h> /* strtou64 */
#include <sysalloc.h>

#include <assert.h>

#include "../klib/int_checks-priv.h"

#if HAVE_GNU_LIBC_VERSION_H
#include <gnu/libc-version.h>
#endif
#if BSD || LINUX
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

static KLock *kns_manager_lock = NULL; /* Protects below */
static KDataBuffer kns_manager_user_agent;

static KDataBuffer kns_manager_guid;

/* thread-local values */
_Thread_local char kns_manager_user_agent_append [ 4096 ] = { 0 };
_Thread_local char kns_manager_clientip  [ KNSMANAGER_STRING_MAX ] = { 0 };
_Thread_local char kns_manager_sessionid [ KNSMANAGER_STRING_MAX ] = { 0 };
_Thread_local char kns_manager_pagehitid [ KNSMANAGER_STRING_MAX ] = { 0 };
_Thread_local char kns_manager_ua_suffix [ KNSMANAGER_STRING_MAX ] = { 0 };

quitting_t quitting;

static atomic_ptr_t kns_singleton;

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

#ifdef USE_SINGLETON
static bool SINGLETON = true;
#else
static bool SINGLETON = false;
#endif

bool KNSManagerUseSingleton(bool use) {
    bool b = SINGLETON;
    SINGLETON = use;
    return b;
}

static rc_t KNSManagerWhack ( KNSManager *self )
{
    rc_t rc = 0;

    if ( SINGLETON ) {
        KNSManager *our_mgr
            = atomic_test_and_set_ptr ( &kns_singleton, NULL, NULL );
        if ( self == our_mgr ) {
            if ( !self->notSingleton ) { return 0; }
            atomic_test_and_set_ptr ( &kns_singleton, NULL, self );
        }
    }

    KNSProxiesWhack ( self->proxies );
    CloudMgrRelease ( self->cloud );

    rc = HttpRetrySpecsDestroy ( &self->retry_specs );

    KTLSGlobalsWhack ( &self->tlsg );

    free ( self->own_cert );
    free ( self->pk_key );

    memset ( self, 0, sizeof * self );

    free ( self );

    KNSManagerCleanup ();

    if ( kns_manager_lock != NULL ) {
        KLockRelease ( kns_manager_lock );
        kns_manager_lock = NULL;
    }

    if ( rc == 0 )
        rc = KDataBufferWhack ( &kns_manager_guid );
    if ( rc == 0 )
        rc = KDataBufferWhack ( &kns_manager_user_agent );

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

        if ( SINGLETON && !local ) {
            /* grab single-shot singleton */
            our_mgr = atomic_test_and_set_ptr ( &kns_singleton, NULL, NULL );
            if ( our_mgr != NULL ) {
                /* add a new reference and return */
                rc = KNSManagerAddRef ( our_mgr );
                if ( rc == 0 ) { *mgrp = our_mgr; }
                return rc;
            }
        }

        /* singleton was NULL. Make from scratch. */
        if ( kfg == NULL ) { rc = KConfigMake ( &kfg, NULL ); }

        if ( rc == 0 ) {
            if ( !kns_manager_lock ) {
                rc = KLockMake ( &kns_manager_lock );
                if ( rc ) { return rc; }
            }

            if (kns_manager_user_agent.base == NULL) {
                rc = KDataBufferMakeBytes ( &kns_manager_user_agent, 0 );
                if ( rc ) { return rc; }
                rc = KDataBufferPrintf ( &kns_manager_user_agent, "%s", "" );
                if ( rc ) { return rc; }
            }

            if (kns_manager_guid.base == NULL) {
                rc = KDataBufferMakeBytes ( &kns_manager_guid, 0 );
                if ( rc ) { return rc; }
                rc = KDataBufferPrintf ( &kns_manager_guid, "%s", "" );
                if ( rc ) { return rc; }
            }

            rc = KNSManagerMakeConfigImpl ( &our_mgr, kfg );

            if ( aKfg == NULL ) { KConfigRelease ( kfg ); }

            if ( rc == 0 ) {
                if ( SINGLETON && !local ) {
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
    timeout_t * ptm = NULL;

    if ( self == NULL ) {
        if ( conn == NULL ) {
            return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
        }

        *conn = NULL;

        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    }

    if (self->conn_timeout >= 0) {
        TimeoutInit ( &tm, self->conn_timeout );
        ptm = &tm;
    }

    return KNSManagerMakeRetryTimedConnection ( self, conn, ptm,
        self->conn_read_timeout, self->conn_write_timeout, from, to );
}

static rc_t KNSManagerMakeTimedConnectionInt ( struct KNSManager const *self,
    struct KSocket **conn, int32_t connectMillis, int32_t readMillis,
    int32_t writeMillis, struct KEndPoint const *from,
    struct KEndPoint const *to )
{
    timeout_t tm;
    timeout_t * ptm = NULL;

    if ( self == NULL ) {
        if ( conn == NULL ) {
            return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
        }

        *conn = NULL;

        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    }

    if (connectMillis >=0 ) {
        TimeoutInit ( &tm, connectMillis );
        ptm = &tm;
    }

    return KNSManagerMakeRetryTimedConnection (
        self, conn, ptm, readMillis, writeMillis, from, to );
}

/* MakeTimedConnection, MakeTimedConnectionExt
 *  create a connection-oriented stream
 *
 *  "conn" [ OUT ] - a stream for communication with the server
 *
 *  "retryTimeout" [ IN ] - if connection is refused, retry with 1ms intervals:
 * when negative, retry infinitely, when 0, do not retry, positive gives maximum
 * wait time in seconds
 *
 *  "connectMillis", "readMillis" and "writeMillis" [ IN ] - when negative,
 *   infinite timeout;
 *   when 0, return immediately; positive gives maximum wait time in mS
 *   for connection, reads and writes respectively.
 *
 *  "from" [ IN ] - client endpoint
 *
 *  "to" [ IN ] - server endpoint
 *
 *  both endpoints have to be of type epIP; creates a TCP connection
 */
/* Use connectMillis from KNSManager */
LIB_EXPORT rc_t CC KNSManagerMakeTimedConnection(
    struct KNSManager const *self, struct KSocket **conn, int32_t readMillis,
    int32_t writeMillis, struct KEndPoint const *from,
    struct KEndPoint const *to)
{
    if (self == NULL)
        return RC(rcNS, rcStream, rcConstructing, rcSelf, rcNull);

    return KNSManagerMakeTimedConnectionInt(
        self, conn, self->conn_timeout, readMillis, writeMillis, from, to);
}
/* connectMillis is specified */
LIB_EXPORT rc_t CC KNSManagerMakeTimedConnectionExt (
    struct KNSManager const *self, struct KSocket **conn, int32_t connectMillis,
    int32_t readMillis, int32_t writeMillis, struct KEndPoint const *from,
    struct KEndPoint const *to )
{
    return KNSManagerMakeTimedConnectionInt(
        self, conn, connectMillis, readMillis, writeMillis, from, to);
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
    if ( connectMillis > MAX_CONN_LIMIT ) {
        connectMillis = MAX_CONN_LIMIT;
    }

    if ( readMillis > MAX_CONN_READ_LIMIT ) {
        readMillis = MAX_CONN_READ_LIMIT;
    }

    if ( writeMillis > MAX_CONN_WRITE_LIMIT ) {
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

LIB_EXPORT rc_t CC KNSManagerSetOwnCert(KNSManager * self,
    const char * own_cert, const char * pk_key)
{
    if (self == NULL)
        return RC(rcNS, rcMgr, rcUpdating, rcSelf, rcNull);

    if (own_cert != NULL && pk_key == NULL)
        return RC(rcNS, rcMgr, rcUpdating, rcParam, rcNull);

    if (self->own_cert != NULL) {
        /* cannot clear certificate once set */
        if (own_cert == NULL)
            return RC(rcNS, rcMgr, rcClearing, rcDoc, rcExists);
        /* cannot update certificate once set */
        else
            return RC(rcNS, rcMgr, rcUpdating, rcDoc, rcExists);
    }

    assert(self->pk_key == NULL);

    if (own_cert != NULL) {
        self->own_cert = string_dup_measure(own_cert, NULL);
        if (self->own_cert == NULL)
            return RC(rcNS, rcMgr, rcUpdating, rcMemory, rcExhausted);
    }

    if (pk_key != NULL) {
        self->pk_key = string_dup_measure(pk_key, NULL);
        if (self->pk_key == NULL) {
            free(self->own_cert);
            self->own_cert = NULL;
            return RC(rcNS, rcMgr, rcUpdating, rcMemory, rcExhausted);
        }
    }

    if (self->own_cert != NULL) {
        assert(self->pk_key);
        return
            KTLSGlobalsSetupOwnCert(&self->tlsg, self->own_cert, self->pk_key);
    }
    else
        return 0;
}

static
bool OwnCertfromEnv(const char ** own_cert, const char ** pk_key) {
    char ** cert = (char**)own_cert;
    char ** key = (char**)pk_key;
    assert(cert && key);

#ifdef WINDOWS
    char* e = NULL;
    {
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & e, & buf_count, "VCBI_VDB_OWN_CERT" );
        if ( err || e == NULL )
            return false;
    }
#else
    char * e = getenv("VCBI_VDB_OWN_CERT");
    if (e == NULL)
        return false;
#endif /* WINDOWS */

    /* CAUTION: free( e ) must be called before exiting the function on WINDOWS */

    KDirectory * dir = NULL;
    rc_t rc = KDirectoryNativeDir(&dir);

    const KFile * file = NULL;
    size_t num_read = 0;
    uint64_t size = 0;

    if (rc == 0) {
        rc = KDirectoryOpenFileRead(dir, &file, "%s/own_cert", e);
        if (rc == 0)
            rc = KFileSize(file, &size);
        if (rc == 0)
            *cert = calloc(1, size + 1);
        if (rc == 0 && *cert == NULL)
        {
#ifdef WINDOWS
            free ( e );
            e = NULL;
#endif
            return false;
        }
        if (rc == 0)
            rc = KFileRead(file, 0, *cert, size + 1, &num_read);
        if (rc == 0)
            KFileRelease(file);
    }

    if (rc == 0) {
        rc = KDirectoryOpenFileRead(dir, &file, "%s/pk_key", e);
        if (rc == 0)
            rc = KFileSize(file, &size);
        if (rc == 0)
            *key = calloc(1, size + 1);
        if (rc == 0 && *key == NULL)
        {
#ifdef WINDOWS
            free ( e );
            e = NULL;
#endif
            return false;
        }
        if (rc == 0)
            rc = KFileRead(file, 0, *key, size + 1, &num_read);
        if (rc == 0)
            KFileRelease(file);
    }

    KDirectoryRelease(dir);
#ifdef WINDOWS
    free ( e );
#endif

    return rc == 0;
}

LIB_EXPORT rc_t CC KNSManagerGetOwnCert(const KNSManager * self,
    const char ** own_cert, const char ** pk_key)
{
    if (own_cert == NULL || pk_key == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcParam, rcNull);

    *own_cert = *pk_key = NULL;

    if (OwnCertfromEnv(own_cert, pk_key))
        return 0;

    if (self == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcSelf, rcNull);

    if (self->own_cert != NULL) {
        *own_cert = string_dup_measure(self->own_cert, NULL);
        if (*own_cert == NULL)
            return RC(rcNS, rcMgr, rcAccessing, rcMemory, rcExhausted);
    }

    if (self->pk_key != NULL) {
        *pk_key = string_dup_measure(self->pk_key, NULL);
        if (*pk_key == NULL) {
            free((char*)*own_cert);
            *own_cert = NULL;
            return RC(rcNS, rcMgr, rcAccessing, rcMemory, rcExhausted);
        }
    }

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
static int32_t KNSManagerLoadConnTimeout ( KConfig *kfg )
{
    int64_t result = 0;

    rc_t rc = KConfigReadI64 ( kfg, "/libs/kns/connect/timeout", &result );
    if ( rc != 0 )
        result = MAX_CONN_LIMIT;

    assert ( FITS_INTO_INT32 ( result ) );
    return (int32_t)result;
}
static int32_t KNSManagerLoadConnReadTimeout ( KConfig *kfg )
{
    int64_t result = 0;

    rc_t rc = KConfigReadI64 ( kfg, "/libs/kns/connect/timeout/read", &result );
    if ( rc != 0 )
        result = MAX_CONN_READ_LIMIT;

    assert ( FITS_INTO_INT32 ( result ) );
    return (int32_t)result;
}
static int32_t KNSManagerLoadConnWriteTimeout ( KConfig *kfg )
{
    int64_t result = 0;

    rc_t rc
        = KConfigReadI64 ( kfg, "/libs/kns/connect/timeout/write", &result );
    if ( rc != 0 )
        result = MAX_CONN_WRITE_LIMIT;

    assert ( FITS_INTO_INT32 ( result ) );
    return (int32_t)result;
}

static int32_t KNSManagerLoadHttpReadTimeout ( KConfig *kfg )
{
    int64_t result = 0; /* when negative - infinite timeout */

    rc_t rc = KConfigReadI64 ( kfg, "/http/timeout/read", &result );
    if ( rc != 0 )
        result = MAX_HTTP_READ_LIMIT;

    assert ( FITS_INTO_INT32 ( result ) );
    return (int32_t)result;
}
static int32_t KNSManagerLoadHttpWriteTimeout ( KConfig *kfg )
{
    int64_t result = 0; /* when negative - infinite timeout */

    rc_t rc = KConfigReadI64 ( kfg, "/http/timeout/write", &result );
    if ( rc != 0 )
        result = MAX_HTTP_WRITE_LIMIT;

    assert ( FITS_INTO_INT32 ( result ) );
    return (int32_t)result;
}

static
int32_t KNSManagerLoadTotalWaitForReliableURLs(const KConfig *kfg)
{
    rc_t rc = 0;

    int64_t result = 0;

#ifdef WINDOWS
    {
        char * str = NULL;
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & str, & buf_count, "NCBI_VDB_RELIABLE_WAIT" );
        if (!err && str != NULL) {
            char *end = NULL;
            result = strtou64(str, &end, 0);
            char chend = end[0];
            free(str);
            if (chend == 0)
            {
                assert ( FITS_INTO_INT32 ( result ) );
                return (int32_t)result;
            }
        }
    }
#else
    const char * str = getenv("NCBI_VDB_RELIABLE_WAIT");
    if (str != NULL) {
        char *end = NULL;
        result = strtou64(str, &end, 0);
        if (end[0] == 0)
        {
            assert ( FITS_INTO_INT32 ( result ) );
            return (int32_t)result;
        }
    }
#endif /* WINDOWS */

    rc = KConfigReadI64(kfg, "/http/reliable/wait", &result);
    if (rc != 0
        || result < 0)  /* no support for infinite wait in HTTP retry loop in */
    {     /* StableHttpFile: it might lead to infinite loop of reading errors */
        result = MAX_HTTP_TOTAL_READ_LIMIT;
    }

    assert ( FITS_INTO_INT32 ( result ) );
    return (int32_t)result;
}

static int32_t KNSManagerLoadTotalConnectWaitForReliableURLs(
    const KConfig *kfg)
{
    rc_t rc = 0;

    int64_t result = 0;

#ifdef WINDOWS
    {
        char * str = NULL;
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & str, & buf_count, "NCBI_VDB_RELIABLE_CONNECT_WAIT" );
        if (!err && str != NULL) {
            char *end = NULL;
            result = strtou64(str, &end, 0);
            char chend = end[0];
            free(str);
            if (chend == 0)
            {
                assert ( FITS_INTO_INT32 ( result ) );
                return (int32_t)result;
            }
        }
    }
#else
    const char * str = getenv("NCBI_VDB_RELIABLE_CONNECT_WAIT");
    if (str != NULL) {
        char *end = NULL;
        result = strtou64(str, &end, 0);
        if (end[0] == 0)
        {
            assert ( FITS_INTO_INT32 ( result ) );
            return (int32_t)result;
        }
    }
#endif

    rc = KConfigReadI64(kfg, "/http/reliable/connect/wait", &result);
    if (rc != 0)
        result = MAX_HTTP_TOTAL_CONNECT_LIMIT;

    assert ( FITS_INTO_INT32 ( result ) );
    return (int32_t)result;
}

static bool KNSManagerLoadRetryFirstRead(const KConfig *kfg) {
    rc_t rc = 0;

    bool result = 0;

#ifdef WINDOWS
    {
        char * str = NULL;
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & str, & buf_count, "NCBI_VDB_RELIABLE_RETRY_FIRST_READ" );
        if (!err && str != NULL)
        {
            char ch = str[0];
            free(str);
            if (ch != '\0') {
                switch (ch) {
                case 'f':
                    return false;
                case 't':
                    return true;
                default:
                    break;
                }
            }
        }
    }
#else
    const char * str = getenv("NCBI_VDB_RELIABLE_RETRY_FIRST_READ");
    if (str != NULL && str[0] != '\0') {
        switch (str[0]) {
        case 'f':
            return false;
        case 't':
            return true;
        default:
            break;
        }
    }
#endif

    rc = KConfigReadBool(kfg, "/http/reliable/retryFirstRead", &result);
    if (rc != 0)
        result = false;

    return result;
}

static bool KNSManagerLoadRetryFile(const KConfig *kfg) {
    rc_t rc = 0;

    bool result = 0;

#ifdef WINDOWS
    {
        char * str = NULL;
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & str, & buf_count, "NCBI_VDB_RELIABLE_RETRY_FILE" );
        if (!err && str != NULL)
        {
            char ch = str[0];
            free(str);
            if (ch != '\0') {
                switch (ch) {
                case 'f':
                    return false;
                case 't':
                    return true;
                default:
                    break;
                }
            }
        }
    }
#else
    const char * str = getenv("NCBI_VDB_RELIABLE_RETRY_FILE");
    if (str != NULL && str[0] != '\0') {
        switch (str[0]) {
        case 'f':
            return false;
        case 't':
            return true;
        default:
            break;
        }
    }
#endif

    rc = KConfigReadBool(kfg, "/http/reliable/retryFile", &result);
    if (rc != 0)
        result = true;

    return result;
}

static uint8_t KNSManagerLoadMaxNumberOfRetriesOnFailureForReliableURLs
(const KConfig *kfg)
{
    int64_t result = 0;
    rc_t rc = KConfigReadI64(kfg, "/http/reliable/retries", &result);
    if (rc != 0 || result < 0)
        result = 10;

    assert ( FITS_INTO_INT8 ( result ) );
    return (uint8_t)result;
}

#if 1
static uint64_t KNSManagerLoadLogTlsErrors(KConfig* kfg) {
#ifdef WINDOWS
    char * e = NULL;
    size_t buf_count = 0;
    errno_t err = _dupenv_s ( & e, & buf_count, "NCBI_VDB_TLS_LOG_ERR" );
    uint64_t ret = 0;
    if (!err && e != NULL)
    {
        if (e[0] == '\0')
            ret = 0;
        else {
            if (e[0] == '0' ||
                e[0] == 'f') /* false */
            {
                ret = 0;
            }
            else
            {
                int n = atoi(e);
                assert( n >= 0 );
                ret = (uint64_t)n;
            }
        }
        free(e);
        return ret;
    }

#else
    const char * e = getenv("NCBI_VDB_TLS_LOG_ERR");
    if (e != NULL)
        if (e[0] == '\0')
            return 0;
        else {
            if (e[0] == '0' ||
                e[0] == 'f') /* false */
            {
                return 0;
            }
            else
                return atoi(e);
        }
#endif
    else {
        uint64_t log = 0;
        rc_t rc = KConfigReadU64(kfg, "/tls/NCBI_VDB_TLS_LOG_ERR", &log);
        if (rc != 0)
            return 0;
        else
            return log;
    }
}
#endif
#if 0
static int KNSManagerLoadEmulateTldReadErrors(KConfig* kfg) {
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

static bool KNSManagerLoadResolveToCache ( KConfig *kfg )
{
    /* VResolverCache resolve to user's cache vs. cwd/AD */
    bool reslt = true;

    /* TODO: call ncbi-vdb/interfaces/kfg/properties.h for exact key name */
    rc_t rc
        = KConfigReadBool ( kfg, "/tools/prefetch/download_to_cache", &reslt );
    if ( rc == 0 ) { return reslt; }

    return true;
}

static bool KNSManagerLoadAcceptAwsCharges ( KConfig *kfg )
{
    bool reslt = false;

    /* TODO: call ncbi-vdb/interfaces/kfg/properties.h for exact key name */
    rc_t rc = KConfigReadBool ( kfg, "/libs/cloud/accept_aws_charges", &reslt );
    if ( rc == 0 ) { return reslt; }

    return false;
}

static bool KNSManagerLoadAcceptGcpCharges ( KConfig *kfg )
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

            mgr->conn_timeout = KNSManagerLoadConnTimeout ( kfg );
            mgr->conn_read_timeout = KNSManagerLoadConnReadTimeout ( kfg );
            mgr->conn_write_timeout = KNSManagerLoadConnWriteTimeout ( kfg );
            mgr->http_read_timeout = KNSManagerLoadHttpReadTimeout ( kfg );
            mgr->http_write_timeout = KNSManagerLoadHttpWriteTimeout ( kfg );

            mgr->maxTotalWaitForReliableURLs_ms =
                KNSManagerLoadTotalWaitForReliableURLs ( kfg );

            mgr->maxTotalConnectWaitForReliableURLs_ms =
                KNSManagerLoadTotalConnectWaitForReliableURLs ( kfg );

            mgr->maxNumberOfRetriesOnFailureForReliableURLs =
                KNSManagerLoadMaxNumberOfRetriesOnFailureForReliableURLs( kfg );

            mgr->retryFirstRead = KNSManagerLoadRetryFirstRead(kfg);
            mgr->retryFile = KNSManagerLoadRetryFile ( kfg );
            mgr->max_http_read_timeout = 60 * 1000; /* 1 minute */
            mgr->logTlsErrors = KNSManagerLoadLogTlsErrors(kfg); /*
                        mgr->emulateTlsReadErrors
                            = KNSManagerLoadEmulateTldReadErrors(kfg); */

            mgr->resolveToCache = KNSManagerLoadResolveToCache ( kfg );

            mgr->accept_aws_charges = KNSManagerLoadAcceptAwsCharges ( kfg );
            mgr->accept_gcp_charges = KNSManagerLoadAcceptGcpCharges ( kfg );

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
                    uint64_t logTlsErrors = KNSManagerLogNcbiVdbNetError(mgr);
                    rc = KTLSGlobalsInit ( &mgr->tlsg, kfg, logTlsErrors );

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

/* If fmt == NULL -> release kns_manager_user_agent */
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
        rc_t rc = KDataBufferWhack ( &kns_manager_user_agent );
        rc_t r2 = KDataBufferWhack ( &kns_manager_guid );
        if (rc == 0 && r2 != 0)
            rc = r2;
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

    const bool telemetry = true; /* always send telemetry bits */

    if ( user_agent == NULL ) {
        rc = RC ( rcNS, rcMgr, rcAccessing, rcParam, rcNull );
        return rc;
    }

    char cloudtrunc[64];

#ifdef WINDOWS
    {
        char *cloudid = NULL;
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & cloudid, & buf_count, ENV_MAGIC_CE_TOKEN );
        if ( !err && cloudid && strlen ( cloudid ) > 8 ) {
            /* AWS access keys should always begin with AKIA,
             * suffixes seems non-random */
            assert ( strlen ( cloudid ) - 4 < sizeof cloudtrunc );
            strncpy_s ( cloudtrunc, sizeof cloudtrunc, cloudid + 4, strlen ( cloudid ) - 4 );
            cloudtrunc[3] = '\0';
        } else {
            strncpy_s ( cloudtrunc, sizeof cloudtrunc, "noc", 3 );
        }
        if ( !err && cloudid )
            free ( cloudid );
    }
#else
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
#endif

#ifdef WINDOWS
    char sessid_buf[64];
    const char * sessid = NULL;
    {
        char *str = NULL;
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & str, & buf_count, ENV_VAR_SESSION_ID);
        if ( err || str == NULL )
        {
            sessid = "nos";
        }
        else
        {
            size_t len = strlen(str);
            assert ( len < sizeof sessid_buf );
            strncpy_s ( sessid_buf, sizeof sessid_buf, str, len );
            sessid = sessid_buf;
            free ( str );
        }
    }
#else
    const char *sessid = getenv ( ENV_VAR_SESSION_ID );
    if ( sessid == NULL ) { sessid = "nos"; }
#endif

    const char *libc_version = "";
#if HAVE_GNU_GET_LIBC_VERSION_F
    libc_version = gnu_get_libc_version ();
#endif

#ifdef WINDOWS
    char opt_bitmap_buf[64];
    const char * opt_bitmap = NULL;
    {
        char *str = NULL;
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & str, & buf_count, ENV_MAGIC_OPT_BITMAP);
        if ( err || str == NULL )
        {
            opt_bitmap = "nob";
        }
        else
        {
            size_t len = strlen(str);
            assert ( len < sizeof opt_bitmap_buf );
            strncpy_s (opt_bitmap_buf, sizeof opt_bitmap_buf, str, len );
            opt_bitmap = opt_bitmap_buf;
            free ( str );
        }
    }
#else
    const char *opt_bitmap=getenv(ENV_MAGIC_OPT_BITMAP); // VDB_OPT_BITMAP
    if (!opt_bitmap) opt_bitmap="nob";
#endif

    const char *guid = "nog";

    {
        KConfig *kfg = NULL;
        KConfigMake ( &kfg, NULL );

        /* Sometimes called before KNSManagerMake */
        if ( ( KDataBufferBytes ( &kns_manager_guid ) == 0 )
            || ( strlen ( kns_manager_guid.base ) == 0 ) )
        {
            size_t written = 0;

            rc = KDataBufferResize ( &kns_manager_guid, 37 );
            if ( rc != 0 ) {
                /* Some tests whack guid */
                rc = KDataBufferMakeBytes ( &kns_manager_guid, 37 );
                if ( rc != 0)
                { return rc; }
            }

            KConfig_Get_GUID ( kfg, kns_manager_guid.base,
                KDataBufferBytes ( &kns_manager_guid ), &written );
            assert ( written <= 37 );
        }

        if ( kfg != NULL )
            KConfigRelease ( kfg );
    }

    if ( strlen ( kns_manager_guid.base ) ) guid = kns_manager_guid.base;

    KDataBuffer phid;
    KDataBufferMakeBytes ( &phid, 0 );

    if ( telemetry )
      rc = KDataBufferPrintf (
        &phid, "%.3s%.4s%.3s,libc=%s,bmap=%s", cloudtrunc, guid, sessid, libc_version, opt_bitmap );
    else
      rc = KDataBufferPrintf (
        &phid, "%.3s%.4s%.3s,libc=%s"        , cloudtrunc, guid, sessid, libc_version             );
    if ( rc ) { return rc; }

    if ( kns_manager_lock ) {
        rc_t rc2 = KLockAcquire ( kns_manager_lock );
        if ( rc2 ) {
            KDataBufferWhack ( &phid );
            return rc2;
        }
    }

    /* Some tests call before these are initialized */
    if ( KDataBufferBytes ( &kns_manager_user_agent ) == 0 )
        KDataBufferPrintf ( &kns_manager_user_agent, "%s", "" );

    if ( kns_manager_lock ) { KLockUnlock ( kns_manager_lock ); }

    KDataBuffer sessids;
    KDataBufferMakeBytes ( &sessids, 0 );

    /* If any are populated, emit all */
    if ( strlen ( kns_manager_clientip )
        || strlen ( kns_manager_sessionid )
        || strlen ( kns_manager_pagehitid ) ) {
        rc = KDataBufferPrintf ( &sessids, "cip=%s,sid=%s,pagehit=%s",
            kns_manager_clientip, kns_manager_sessionid,
            kns_manager_pagehitid );
    }

    if ( rc ) {
        KDataBufferWhack ( &phid );
        KDataBufferWhack ( &sessids );
        return rc;
    }

    KDataBuffer platform;
    const char* env_platform_name = NULL;
    const char* env_platform_version = NULL;
#ifdef WINDOWS
    char env_platform_name_buf[64], env_platform_version_buf[64];
    {
        char * str = NULL;
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & str, & buf_count, ENV_MAGIC_PLATFORM_NAME );
        if ( !err && str != NULL )
        {
            assert ( strlen(str) < sizeof(env_platform_name_buf) );
            strncpy_s ( env_platform_name_buf, sizeof env_platform_name_buf, str, strlen(str) );
            env_platform_name = env_platform_name_buf;
            free ( str );
        }
    }
    {
        char * str = NULL;
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & str, & buf_count, ENV_MAGIC_PLATFORM_VERSION );
        if ( !err && str != NULL )
        {
            assert ( strlen(str) < sizeof(env_platform_version_buf) );
            strncpy_s ( env_platform_version_buf, sizeof env_platform_version_buf, str, strlen(str) );
            env_platform_version = env_platform_version_buf;
            free ( str );
        }
    }
#else
    env_platform_name = getenv(ENV_MAGIC_PLATFORM_NAME);
    env_platform_version = getenv(ENV_MAGIC_PLATFORM_VERSION);
#endif

    KDataBufferMakeBytes(&platform, 0);
    if (env_platform_name)
    {
        if (env_platform_version)
        {
            rc=KDataBufferPrintf(&platform," via %s %s",
                                 env_platform_name,
                                 env_platform_version);
            if (rc)
            {
                KDataBufferWhack ( &phid );
                KDataBufferWhack ( &sessids );
                KDataBufferWhack ( &platform );
                return rc;
            }
        }
        else{
            rc=KDataBufferPrintf(&platform," via %s", env_platform_name);
            if (rc)
            {
                KDataBufferWhack ( &phid );
                KDataBufferWhack ( &sessids );
                KDataBufferWhack ( &platform );
                return rc;
            }
        }
    }
    else
    {
            rc=KDataBufferPrintf(&platform,"%s","");
            if (rc)
            {
                KDataBufferWhack ( &phid );
                KDataBufferWhack ( &sessids );
                KDataBufferWhack ( &platform );
                return rc;
            }
    }

    if ( sessids.base && strlen ( sessids.base ) ) {
        const String *b64;
        encodeBase64 ( &b64, sessids.base, strlen ( sessids.base ) );
        rc = string_printf ( kns_manager_user_agent_append, sizeof kns_manager_user_agent_append, NULL,
            "%s%s%s (phid=%s,%s)",
            kns_manager_user_agent.base, kns_manager_ua_suffix, platform.base,
            phid.base, b64->addr );
        StringWhack ( b64 );
    } else {
        rc = string_printf ( kns_manager_user_agent_append, sizeof kns_manager_user_agent_append, NULL,
            "%s%s%s (phid=%s)",
            kns_manager_user_agent.base, kns_manager_ua_suffix, platform.base,
            phid.base );
    }

    KDataBufferWhack ( &phid );
    KDataBufferWhack ( &sessids );
    KDataBufferWhack ( &platform );

    ( *user_agent ) = kns_manager_user_agent_append;
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

uint64_t KNSManagerLogNcbiVdbNetError ( const KNSManager *self )
{
    if ( self == NULL ) {
#ifdef NCBI_VDB_NET
        return 1;
#else
        return 0;
#endif
    }
    if ( self->logTlsErrors == 0 )
        return 0;

    if ( self->NCBI_VDB_NETnoLogError ) { return 0; }

#ifdef WINDOWS
    {
        char * e = NULL;
        size_t buf_count = 0;
        errno_t err = _dupenv_s ( & e, & buf_count, "NCBI_VDB_NET" );
        if ( !err && e != NULL )
        {
            char ch = e[0];
            free ( e );
            if (ch == '\0' || ch == '0' || ch == 'f') /* false */
            {
                return 0;
            }
            return 1;
        }
    }
#else
    const char *e = getenv ( "NCBI_VDB_NET" );
    if ( e != NULL ) {
        if ( e[0] == '\0' || e[0] == '0' || e[0] == 'f' ) /* false */
        {
            return 0;
        }

        return 1;
    }
#endif
    if ( self->NCBI_VDB_NETkfgValueSet ) { return self->NCBI_VDB_NETkfgValue; }

    return self->logTlsErrors;

#if 0
#ifdef NCBI_VDB_NET
    return true;
#else
    return false;
#endif
#endif
}

LIB_EXPORT rc_t CC KNSManagerSetAdCaching (
    struct KNSManager *self, bool enabled )
{
    if ( self != NULL )
        self->enabledResolveToAd = enabled;
    return 0;
}

LIB_EXPORT rc_t CC KNSManagerGetAdCaching(
    const KNSManager* self, bool * enabled)
{
    assert(self && enabled);
    *enabled = self->enabledResolveToAd;
    return 0;
}

LIB_EXPORT rc_t CC KNSManagerGetResolveToCache(
    const KNSManager* self, bool * resolveToCache)
{
    assert(self && resolveToCache);
    *resolveToCache = self->resolveToCache;
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
    size_t len = strlen( suffix );
    if ( len >= KNSMANAGER_STRING_MAX )
    {
        return RC ( rcNS, rcMgr, rcAttaching, rcParam, rcTooLong );
    }

    string_copy ( kns_manager_ua_suffix, sizeof kns_manager_ua_suffix, suffix, len );
    return 0;
}

LIB_EXPORT rc_t CC KNSManagerGetUserAgentSuffix ( const char ** suffix )
{
    if ( suffix == NULL ) {
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
    }
    *suffix = kns_manager_ua_suffix;
    return 0;
}

LIB_EXPORT rc_t CC KNSManagerSetClientIP (
    KNSManager *self, const char *clientip )
{
    if ( self == NULL || clientip == NULL ) {
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
    }
    size_t len = strlen( clientip );
    if ( len >= KNSMANAGER_STRING_MAX )
    {
        return RC ( rcNS, rcMgr, rcAttaching, rcParam, rcTooLong );
    }

    string_copy ( kns_manager_clientip, sizeof kns_manager_clientip, clientip, len );
    return 0;
}

LIB_EXPORT rc_t CC KNSManagerSetSessionID (
    KNSManager *self, const char *sessionid )
{
    if ( self == NULL || sessionid == NULL ) {
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
    }
    size_t len = strlen( sessionid );
    if ( len >= KNSMANAGER_STRING_MAX )
    {
        return RC ( rcNS, rcMgr, rcAttaching, rcParam, rcTooLong );
    }

    string_copy ( kns_manager_sessionid, sizeof kns_manager_sessionid, sessionid, len );
    return 0;
}


LIB_EXPORT rc_t CC KNSManagerSetPageHitID (
    KNSManager *self, const char *pagehitid )
{
    if ( self == NULL || pagehitid == NULL ) {
        return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
    }
    size_t len = strlen( pagehitid );
    if ( len >= KNSMANAGER_STRING_MAX )
    {
        return RC ( rcNS, rcMgr, rcAttaching, rcParam, rcTooLong );
    }

    string_copy ( kns_manager_pagehitid, sizeof kns_manager_pagehitid, pagehitid, len );
    return 0;
}


LIB_EXPORT
rc_t CC KNSManagerSetQuitting(KNSManager *self, quitting_t aQuitting)
{
    quitting = aQuitting;
    return 0;
}

LIB_EXPORT
quitting_t CC KNSManagerGetQuitting(const KNSManager *self)
{
    return quitting;
}

/******************************************************************************/
/**************** API to manage HTTP File read retry behavior *****************/
/******************************************************************************/

/* SetRetryFailedReads
 *  manages retry layer on HttpFileRead
 *
 *  "retry" [ IN ] - true : turn on retry layer,
 *                   false: don't create retry layer.
 */
KNS_EXTERN rc_t CC KNSManagerSetRetryFailedReads(KNSManager *self,
    bool retry)
{
    if (self == NULL)
        return RC(rcNS, rcMgr, rcUpdating, rcSelf, rcNull);
    else {
        self->retryFile = retry;
        return 0;
    }
}

/* GetRetryFailedReads
 *  returns whether or not retry layer on HttpFileRead is turned on
 */
LIB_EXPORT rc_t CC KNSManagerGetRetryFailedReads(const KNSManager *self,
    bool *retry)
{
    if (self == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcSelf, rcNull);
    else if (retry == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcParam, rcNull);
    else {
        *retry = self->retryFile;
        return 0;
    }
}

/* SetMaxConnectRetryTime
 *  sets maximum time when opening HttpFile
 *
 *  "millis" - when negative, infinite timeout
 */
LIB_EXPORT rc_t CC KNSManagerSetMaxConnectRetryTime(KNSManager *self,
    int32_t millis)
{
    if (self == NULL)
        return RC(rcNS, rcMgr, rcUpdating, rcSelf, rcNull);
    else {
        if (millis < 0)
            self->maxTotalConnectWaitForReliableURLs_ms = ~0;
        else
            self->maxTotalConnectWaitForReliableURLs_ms = millis;

        return 0;
    }
}

/* SetMaxReadRetryTime
 *  sets maximum time in HttpFileRead retry loop
 *
 *  "millis" - when negative, infinite timeout
 */
LIB_EXPORT rc_t CC KNSManagerSetMaxReadRetryTime(KNSManager *self,
    int32_t millis)
{
    if (self == NULL)
        return RC(rcNS, rcMgr, rcUpdating, rcSelf, rcNull);
    else {
        if (millis < 0)
            self->maxTotalWaitForReliableURLs_ms = ~0;
        else
            self->maxTotalWaitForReliableURLs_ms = millis;

        return 0;
    }
}

/* GetMaxReadRetryTime
 *  returns maximum time in HttpFileRead retry loop
 */
LIB_EXPORT rc_t CC KNSManagerGetMaxReadRetryTime(const KNSManager *self,
    int32_t *millis)
{
    if (self == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcSelf, rcNull);
    else if (millis == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcParam, rcNull);
    else {
        if (self->maxTotalWaitForReliableURLs_ms == ~0)
            *millis = -1;
        else
            *millis = self->maxTotalWaitForReliableURLs_ms;

        return 0;
    }
}


/* GetMaxConnectRetryTime
 *  returns maximum time when opening HttpFile
 */
LIB_EXPORT rc_t CC KNSManagerGetMaxConnectRetryTime(const KNSManager *self,
    int32_t *millis)
{
    if (self == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcSelf, rcNull);
    else if (millis == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcParam, rcNull);
    else {
        if (self->maxTotalConnectWaitForReliableURLs_ms == ~0)
            *millis = -1;
        else
            *millis = self->maxTotalConnectWaitForReliableURLs_ms;

        return 0;
    }
}


/* SetRetryFirstReads
 *  manages retry on the first HttpFileRead
 */
LIB_EXPORT rc_t CC KNSManagerSetRetryFirstReads(KNSManager *self,
    bool retry)
{
    if (self == NULL)
        return RC(rcNS, rcMgr, rcUpdating, rcSelf, rcNull);
    else {
        self->retryFirstRead = retry;
        return 0;
    }
}

/* GetRetryFirstReads
 *  returns whether or not retry on the first HttpFileRead is turned on
 */
LIB_EXPORT rc_t CC KNSManagerGetRetryFirstReads(const KNSManager *self,
    bool *retry)
{
    if (self == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcSelf, rcNull);
    else if (retry == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcParam, rcNull);
    else {
        *retry = self->retryFirstRead;
        return 0;
    }
}

/******************************************************************************/
