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

#include <klib/printf.h>
#include <klib/rc.h>

#include <kproc/timeout.h>

#include <kns/manager.h>
#include <kns/socket.h>
#include <kns/http.h>

#include <atomic.h> /* atomic_ptr_t */
#include <sysalloc.h>

#include <assert.h>

#include <stdio.h> /* fprintf */

#include "../klib/release-vers.h"
#include "cloud.h" /* KNSManagerMakeCloud */
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

static char kns_manager_user_agent [ 128 ] = "ncbi-vdb";

#if USE_SINGLETON
static atomic_ptr_t kns_singleton;
#endif

#define RELEASE( type, obj ) do { rc_t rc2 = type##Release ( obj ); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while ( false )

bool KNSManagerHttpProxyOnly ( const KNSManager * self ) {
    if ( self == NULL )
        return false;
    else
        return KNSProxiesHttpProxyOnly ( self -> proxies );
}

struct KNSProxies * KNSManagerGetProxies ( const KNSManager * self,
                                           size_t * cnt )
{
    if ( self == NULL)
        return NULL;
    else {
        size_t dummy;
        if ( cnt == NULL )
            cnt = & dummy;
        return KNSProxiesGetHttpProxy ( self -> proxies, cnt );
    }
}


static
rc_t KNSManagerWhack ( KNSManager * self )
{
    rc_t rc;

#if USE_SINGLETON
    KNSManager * our_mgr = atomic_test_and_set_ptr ( & kns_singleton, NULL, NULL );
    if ( self == our_mgr ) {
        if ( ! self -> notSingleton )
            return 0;
        else
            atomic_test_and_set_ptr ( & kns_singleton, NULL, self );
    }
#endif

    KNSProxiesWhack ( self -> proxies );
    CloudRelease(self->cloud);

    if ( self -> aws_access_key_id != NULL )
        StringWhack ( self -> aws_access_key_id );

    if ( self -> aws_secret_access_key != NULL )
        StringWhack ( self -> aws_secret_access_key );

    if ( self -> aws_region != NULL )
        StringWhack ( self -> aws_region );

    if ( self -> aws_output != NULL )
        StringWhack ( self -> aws_output );
    
    rc = HttpRetrySpecsDestroy ( & self -> retry_specs );

    KTLSGlobalsWhack ( & self -> tlsg );

    free ( self );

    KNSManagerCleanup ();

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerAddRef ( const KNSManager *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd( &self->refcount, "KNSManager" ) )
        {
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
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KNSManager" ) )
        {
        case krefWhack:
            return KNSManagerWhack ( ( KNSManager * ) self );
        case krefNegative:
            return RC ( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
        }
    }
    return 0;
}

static
void KNSManagerLoadAWS ( struct KNSManager *self, const KConfig * kfg )
{
    rc_t rc;

    const KConfigNode *aws_node;

    if ( self == NULL )
        return;

    rc = KConfigOpenNodeRead ( kfg, &aws_node, "AWS" );
    if ( rc == 0 )
    {
        do
        {
            String *access_key_id = NULL, *secret_access_key = NULL, *region = NULL, *output = NULL;
            const KConfigNode *access_key_id_node, *secret_access_key_node, *region_node, *output_node;

            rc = KConfigNodeOpenNodeRead ( aws_node, &access_key_id_node, "aws_access_key_id" );
            if ( rc == 0 )
            {
                rc = KConfigNodeReadString ( access_key_id_node, &access_key_id );

                KConfigNodeRelease ( access_key_id_node );

                if( rc != 0 )
                    break;
            }


            rc = KConfigNodeOpenNodeRead ( aws_node, &secret_access_key_node, "aws_secret_access_key" );
            if ( rc == 0 )
            {
                rc = KConfigNodeReadString ( secret_access_key_node, &secret_access_key );

                KConfigNodeRelease ( secret_access_key_node );

                if ( rc != 0 )
                    break;
            }
        
            rc = KConfigNodeOpenNodeRead ( aws_node, &region_node, "region" );
            if ( rc == 0 )
            {
                rc = KConfigNodeReadString ( region_node, &region );

                KConfigNodeRelease ( region_node );

                if ( rc != 0 )
                    break;
            }

            rc = KConfigNodeOpenNodeRead ( aws_node, &output_node, "output" );
            if ( rc == 0 )
            {
                rc = KConfigNodeReadString ( output_node, &output );

                KConfigNodeRelease ( output_node );
                
                if ( rc != 0 )
                    break;
            }

            self -> aws_access_key_id = access_key_id;
            self -> aws_secret_access_key = secret_access_key;
            self -> aws_region = region;
            self -> aws_output = output;

        } while ( 0 );

        KConfigNodeRelease ( aws_node );
    }
}

LIB_EXPORT rc_t CC KNSManagerMake ( KNSManager ** mgrp )
{
    rc_t rc;

    if ( mgrp == NULL )
        rc = RC ( rcNS, rcMgr, rcAllocating, rcParam, rcNull );
    else
    {
        KConfig * kfg;
        KNSManager * our_mgr;

        * mgrp = NULL;

#if USE_SINGLETON
        /* grab single-shot singleton */
        our_mgr = atomic_test_and_set_ptr ( & kns_singleton, NULL, NULL );
        if ( our_mgr != NULL )
        {
            /* add a new reference and return */
            rc = KNSManagerAddRef ( our_mgr );
            if ( rc == 0 )
                * mgrp = our_mgr;
            return rc;
        }
#endif

        /* singleton was NULL. make from scratch. */
        rc = KConfigMake ( & kfg, NULL );
        if ( rc == 0 )
        {
            rc = KNSManagerMakeConfig ( & our_mgr, kfg );
            KConfigRelease ( kfg );

            if ( rc == 0 )
            {
#if USE_SINGLETON
                /* try to set single-shot ( set once, never reset ) */
                KNSManager * new_mgr = atomic_test_and_set_ptr ( & kns_singleton, our_mgr, NULL );
                if ( new_mgr != NULL )
                {
                    /* somebody else got here first - drop our version */
                    assert ( our_mgr != new_mgr );
                    KNSManagerRelease ( our_mgr );

                    /* use the new manager, just add a reference and return */
                    rc = KNSManagerAddRef ( new_mgr );
                    if ( rc == 0 )
                        * mgrp = new_mgr;
                    return rc;
                }
#endif

                /* return parameter */
                * mgrp = our_mgr;
            }
        }
    }

    return rc;
}

LIB_EXPORT void KNSManagerSetVerbose ( KNSManager *self, bool verbosity )
{
    if ( self != NULL )
        self -> verbose = verbosity;
}


LIB_EXPORT bool KNSManagerIsVerbose ( const KNSManager *self )
{
    return ( self != NULL ) ? self -> verbose : false;
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
LIB_EXPORT rc_t CC KNSManagerMakeConnection ( const KNSManager * self,
    struct KSocket **conn, struct KEndPoint const *from, struct KEndPoint const *to )
{
    timeout_t tm;

    if ( self == NULL )
    {
        if ( conn == NULL )
            return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );

        * conn = NULL;

        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    }

    TimeoutInit ( & tm, self -> conn_timeout );

    return KNSManagerMakeRetryTimedConnection ( self, conn, 
        & tm, self -> conn_read_timeout, self -> conn_write_timeout, from, to );
}
/* MakeTimedConnection
 *  create a connection-oriented stream
 *
 *  "conn" [ OUT ] - a stream for communication with the server
 *
 *  "retryTimeout" [ IN ] - if connection is refused, retry with 1ms intervals: when negative, retry infinitely,
 *   when 0, do not retry, positive gives maximum wait time in seconds 
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
LIB_EXPORT rc_t CC KNSManagerMakeTimedConnection ( struct KNSManager const * self,
    struct KSocket **conn, int32_t readMillis, int32_t writeMillis,
    struct KEndPoint const *from, struct KEndPoint const *to )
{
    timeout_t tm;

    if ( self == NULL )
    {
        if ( conn == NULL )
            return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );

        * conn = NULL;

        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    }

    TimeoutInit ( & tm, self -> conn_timeout );

    return KNSManagerMakeRetryTimedConnection ( self, conn, 
        & tm, readMillis, writeMillis, from, to );
}    
    
/* MakeRetryConnection
 *  create a connection-oriented stream
 *
 *  "conn" [ OUT ] - a stream for communication with the server
 *
 *  "retryTimeout" [ IN ] - if connection is refused, retry with 1ms intervals: when negative, retry infinitely,
 *   when 0, do not retry, positive gives maximum wait time in seconds 
 *
 *  "from" [ IN ] - client endpoint
 *
 *  "to" [ IN ] - server endpoint 
 *
 *  both endpoints have to be of type epIP; creates a TCP connection
 */    
LIB_EXPORT rc_t CC KNSManagerMakeRetryConnection ( struct KNSManager const * self,
    struct KSocket ** conn, timeout_t * retryTimeout,
    struct KEndPoint const * from, struct KEndPoint const * to )
{
    if ( self == NULL )
    {
        if ( conn == NULL )
            return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );

        * conn = NULL;

        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    }

    return KNSManagerMakeRetryTimedConnection ( self, conn, 
        retryTimeout, self -> conn_read_timeout, self -> conn_write_timeout, from, to );
}    

/* SetConnectionTimeouts
 *  sets default connect/read/write timeouts to supply to sockets
 *
 *  "connectMillis", "readMillis", "writeMillis" [ IN ] - when negative, infinite timeout
 *  when 0, return immediately, positive gives maximum wait time in sec/mS
 *  for connects, reads and writes respectively.
 */
LIB_EXPORT rc_t CC KNSManagerSetConnectionTimeouts ( KNSManager *self,
    int32_t connectMillis, int32_t readMillis, int32_t writeMillis )
{
    if ( self == NULL )
        return RC ( rcNS, rcMgr, rcUpdating, rcSelf, rcNull );

    /* limit values */
    if ( connectMillis < 0 || connectMillis > MAX_CONN_LIMIT )
        connectMillis = MAX_CONN_LIMIT;
        
    if ( readMillis < 0 || readMillis > MAX_CONN_READ_LIMIT )
        readMillis = MAX_CONN_READ_LIMIT;

    if ( writeMillis < 0 || writeMillis > MAX_CONN_WRITE_LIMIT )
        writeMillis = MAX_CONN_WRITE_LIMIT;

    self -> conn_timeout = connectMillis;
    self -> conn_read_timeout = readMillis;
    self -> conn_write_timeout = writeMillis;

    return 0;
}


/* SetHTTPTimeouts
 *  sets default read/write timeouts to supply to HTTP connections
 *
 *  "readMillis" [ IN ] and "writeMillis" - when negative, infinite timeout
 *  when 0, return immediately, positive gives maximum wait time in mS
 *  for reads and writes respectively.
 */
LIB_EXPORT rc_t CC KNSManagerSetHTTPTimeouts ( KNSManager *self,
    int32_t readMillis, int32_t writeMillis )
{
    if ( self == NULL )
        return RC ( rcNS, rcMgr, rcUpdating, rcSelf, rcNull );

    /* limit values */
    if ( readMillis < 0 || readMillis > MAX_HTTP_READ_LIMIT )
        readMillis = MAX_HTTP_READ_LIMIT;

    if ( writeMillis < 0 || writeMillis > MAX_HTTP_WRITE_LIMIT )
        writeMillis = MAX_HTTP_WRITE_LIMIT;

    self -> http_read_timeout = readMillis;
    self -> http_write_timeout = writeMillis;

    return 0;
}

/* GetHTTPProxyPath
 * DEPRECATED
 *  returns path to HTTP proxy server ( if set ) or NULL.
 *  return status is 0 if the path is valid, non-zero otherwise
 */
LIB_EXPORT rc_t CC KNSManagerGetHTTPProxyPath ( const KNSManager * self, const String ** proxy )
{
    fprintf ( stderr,
"WARNING : KNSManagerGetHTTPProxyPath IS DEPRECATED AND SHOULD NOT BE USED\n" );
    return KNSProxiesGetHttpProxyPath ( self -> proxies, proxy );
}


/* SetHTTPProxyPath
 *  sets a path to HTTP proxy server.
 *  a NULL path value removes all proxy settings.
 *
 *  the VPath passed in must still be released using VPathRelease,
 *  because KNSManager will attach a new reference to it.
 */
LIB_EXPORT rc_t CC KNSManagerSetHTTPProxyPath ( KNSManager * self, const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );
    rc = KNSManagerVSetHTTPProxyPath ( self, fmt, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerVSetHTTPProxyPath ( KNSManager * self,
    const char * fmt, va_list args )
{
    if ( self == NULL )
        return RC ( rcNS, rcMgr, rcUpdating, rcSelf, rcNull );
    else
        return KNSProxiesVSetHTTPProxyPath (self -> proxies,
                                            fmt, args, true);
}

static
rc_t KNSManagerHttpProxyInit ( KNSManager * self, const KConfig * kfg )
{
    assert ( self );
    self -> proxies = KNSManagerKNSProxiesMake ( self, kfg );
    if ( self -> proxies == NULL )
        return RC ( rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted );
    else
        return 0;
}

/* GetHTTPProxyEnabled
 *  returns true if a non-NULL proxy path exists and user wants to use it
 *  users indicate desire to use proxy through configuration
 *  or SetHTTPProxyEnabled
 */
LIB_EXPORT bool CC KNSManagerGetHTTPProxyEnabled ( const KNSManager * self )
{
    if ( self != NULL )
        return KNSProxiesGetHTTPProxyEnabled ( self -> proxies );
    else
        return false;
}

/* SetHTTPProxyEnabled
 *  sets http-proxy enabled state to supplied value
 *  returns the prior value as a convenience
 */
LIB_EXPORT bool CC KNSManagerSetHTTPProxyEnabled ( KNSManager * self, bool enabled )
{
    bool prior = false;
    if ( self != NULL )
       prior = KNSProxiesSetHTTPProxyEnabled ( self -> proxies, enabled );
    return prior;
}


static void KNSManagerSetNCBI_VDB_NET ( KNSManager * self, const KConfig * kfg )
{
    rc_t rc = 0;

    const KConfigNode * node = NULL;

    if ( self == NULL || kfg == NULL )
        return;

    rc = KConfigOpenNodeRead ( kfg, & node, "/libs/kns/NCBI_VDB_NET" );
    if ( rc != 0 ) {
        self -> NCBI_VDB_NETkfgValueSet = self -> NCBI_VDB_NETkfgValue = false;
        return;
    }
    else {
        char buffer [ 1 ] = "";
        size_t num_read = 0;
        self -> NCBI_VDB_NETkfgValueSet = true;
        rc = KConfigNodeRead ( node, 0, buffer, sizeof buffer, & num_read, 0 );
        if ( num_read == 0 )
            self -> NCBI_VDB_NETkfgValue = false;
        else switch ( buffer [ 0 ] ) {
            case '0':
            case 'f': /* false */
                self -> NCBI_VDB_NETkfgValue = false;
                break;
            default:
                self -> NCBI_VDB_NETkfgValue = true;
                break;
        }
    }

    KConfigNodeRelease ( node );
    node = NULL;
} 


static int32_t KNSManagerPrepareHttpReadTimeout(KConfig* kfg) {
    int64_t result = 0;
    rc_t rc = KConfigReadI64(kfg, "/http/timeout/read", &result);
    if (rc == 0)
        return result;
    else
        return MAX_HTTP_READ_LIMIT;
}

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

LIB_EXPORT rc_t CC KNSManagerMakeConfig ( KNSManager **mgrp, KConfig* kfg )
{
    rc_t rc;

    if ( mgrp == NULL || kfg == NULL)
        rc = RC ( rcNS, rcMgr, rcAllocating, rcParam, rcNull );
    else
    {
        KNSManager * mgr = calloc ( 1, sizeof * mgr );
        if ( mgr == NULL )
            rc = RC ( rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted );
        else
        {
            KRefcountInit ( & mgr -> refcount, 1, "KNSManager", "init", "kns" );
            mgr -> conn_timeout = MAX_CONN_LIMIT;
            mgr -> conn_read_timeout = MAX_CONN_READ_LIMIT;
            mgr -> conn_write_timeout = MAX_CONN_WRITE_LIMIT;
            mgr -> http_read_timeout = KNSManagerPrepareHttpReadTimeout(kfg);
            mgr -> http_write_timeout = MAX_HTTP_WRITE_LIMIT;
            mgr -> maxTotalWaitForReliableURLs_ms = 10 * 60 * 1000; /* 10 min */
            mgr -> maxNumberOfRetriesOnFailureForReliableURLs = 10;

            mgr->logTlsErrors = KNSManagerPrepareLogTlsErrors(kfg);

            rc = KNSManagerInit (); /* platform specific init in sysmgr.c ( in unix|win etc. subdir ) */
            if ( rc == 0 )
            {
                /* the manager is not a proper singleton */
                if ( kns_manager_user_agent [ 0 ] == 0 )
                {
                    ver_t version = RELEASE_VERS;
                    KNSManagerSetUserAgent ( mgr, PKGNAMESTR " ncbi-vdb.%V", version );
                }

                rc = HttpRetrySpecsInit ( & mgr -> retry_specs, kfg );
                if ( rc == 0 )
                {
                    rc = KTLSGlobalsInit ( & mgr -> tlsg, kfg );

                    if ( rc == 0 )
                        rc = KNSManagerHttpProxyInit ( mgr, kfg );

                    if ( rc == 0 )
                    {
                        KNSManagerLoadAWS ( mgr, kfg );
                        KNSManagerSetNCBI_VDB_NET ( mgr, kfg );

                        * mgrp = mgr;

                        return 0;
                    }
                }
            }

            free ( mgr );
        }

        * mgrp = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC KNSManagerSetUserAgent ( KNSManager * self, const char * fmt, ... )
{
    /* 6/18/14 - don't check "self", since the current implementation
       is actually static. Later implementations will not be... */

    rc_t rc = 0;
    if ( fmt == NULL )
        rc = RC( rcNS, rcMgr, rcUpdating, rcParam, rcNull );
    else
    {
        size_t bytes;
        char scratch [ sizeof kns_manager_user_agent ];

        va_list args;
        va_start ( args, fmt );
        rc = string_vprintf ( scratch, sizeof scratch, & bytes, fmt, args );
        va_end ( args );

        if ( rc == 0 )
            string_copy ( kns_manager_user_agent, sizeof kns_manager_user_agent, scratch, bytes );
    }
    return rc;
}


LIB_EXPORT rc_t CC KNSManagerGetUserAgent ( const char ** user_agent )
{
    rc_t rc = 0;
    if ( user_agent == NULL )
        rc = RC( rcNS, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        ( *user_agent ) = kns_manager_user_agent;
    }
    return rc;
}


/******************************************************************************/

#define NCBI_VDB_NET 1 /* VDB-3399 : temporarily enable for internal testing */

void KNSManagerSetLogNcbiVdbNetError(KNSManager * self, bool set) {
    if (self)
        self->NCBI_VDB_NETnoLogError = ! set;
}

bool KNSManagerLogNcbiVdbNetError ( const KNSManager * self ) {
    if ( self == NULL )
#ifdef NCBI_VDB_NET
    return true;
#else
    return false;
#endif
    else {
        if (!self->logTlsErrors)
            return false;

        if (self->NCBI_VDB_NETnoLogError)
            return false;
        else {
            const char * e = getenv("NCBI_VDB_NET");
            if (e != NULL) {
                if (e[0] == '0' ||
                    e[0] == 'f') /* false */
                {
                    return false;
                }
                else
                    return true;
            }
            else {
                if (self->NCBI_VDB_NETkfgValueSet)
                    return self->NCBI_VDB_NETkfgValue;
            }

#ifdef NCBI_VDB_NET
            return true;
#else
            return false;
#endif
        }
    }
}

rc_t KNSManagerGetCloudLocation(const KNSManager * cself,
    char * buffer, size_t bsize, size_t * num_read, size_t * remaining)
{
    KNSManager * self = (KNSManager *)cself;

    rc_t rc = 0;

    if (buffer == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcParam, rcNull);

    if (self == NULL)
        return RC(rcNS, rcMgr, rcAccessing, rcParam, rcNull);

    if (self->cloud == NULL)
        rc = KNSManagerMakeCloud(self, &self->cloud);

    if (rc == 0) {
        const char * location = NULL;

        size_t dummy = 0;

        if (num_read == NULL)
            num_read = &dummy;
        if (remaining == NULL)
            remaining = &dummy;

        assert(self->cloud);
        location = CloudGetLocation(self->cloud);

        if (location == NULL) {
            if (bsize > 0)
                buffer[0] = '\0';
            *num_read = *remaining = 0;
        }
        else {
            size_t s = string_copy_measure(buffer, bsize, location);
            if (s <= bsize) {
                *num_read = s;
                *remaining = 0;
            }
            else {
                *num_read = bsize;
                *remaining = s - bsize;
            }
        }
    }

    return rc;
}
