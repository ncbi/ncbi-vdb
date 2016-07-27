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

#include "http-priv.h"
#include "mgr-priv.h"
#include "stream-priv.h"
#include "sysmgr.h"
#include "../klib/release-vers.h"

#include <kfg/config.h>

#include <klib/printf.h>
#include <klib/refcount.h>
#include <klib/rc.h>

#include <kns/manager.h>
#include <kns/socket.h>
#include <kns/http.h>

#include <vfs/manager.h>
#include <vfs/path.h>

#include <atomic.h>
#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX_CONN_LIMIT
#define MAX_CONN_LIMIT ( 10 * 60 )
#endif

#ifndef MAX_CONN_READ_LIMIT
#define MAX_CONN_READ_LIMIT ( 10 * 60 * 1000 )
#endif

#ifndef MAX_CONN_WRITE_LIMIT
#define MAX_CONN_WRITE_LIMIT ( 10 * 60 * 1000 )
#endif

static char kns_manager_user_agent [ 128 ] = "ncbi-vdb";

#define USE_SINGLETON 1

#if USE_SINGLETON
static atomic_ptr_t kns_singleton;
#endif

static
rc_t KNSManagerWhack ( KNSManager * self )
{
    rc_t rc;

#if USE_SINGLETON
    KNSManager * our_mgr = atomic_test_and_set_ptr ( & kns_singleton, NULL, NULL );
    if ( self == our_mgr )
        return 0;
#endif

    if ( self -> http_proxy != NULL )
        StringWhack ( self -> http_proxy );

    if ( self -> aws_access_key_id != NULL )
        StringWhack ( self -> aws_access_key_id );

    if ( self -> aws_secret_access_key != NULL )
        StringWhack ( self -> aws_secret_access_key );

    if ( self -> aws_region != NULL )
        StringWhack ( self -> aws_region );

    if ( self -> aws_output != NULL )
        StringWhack ( self -> aws_output );
    
    rc = HttpRetrySpecsDestroy ( & self -> retry_specs );

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
void KNSManagerHttpProxyInit ( KNSManager * self, KConfig * kfg )
{
    const KConfigNode * proxy;
    rc_t rc = KConfigOpenNodeRead ( kfg, & proxy, "http/proxy" );
    if ( rc == 0 )
    {
        const KConfigNode * proxy_path;
        rc = KConfigNodeOpenNodeRead ( proxy, & proxy_path, "path" );
        if ( rc == 0 )
        {
            String * path;
            rc = KConfigNodeReadString ( proxy_path, & path );
            if ( rc == 0 )
            {
                rc = KNSManagerSetHTTPProxyPath ( self, "%S", path );
                if ( rc == 0 )
                {
                    const KConfigNode * proxy_enabled;
                    rc = KConfigNodeOpenNodeRead ( proxy, & proxy_enabled, "enabled" );
                    if ( rc == 0 )
                    {
                        rc = KConfigNodeReadBool ( proxy_enabled, & self -> http_proxy_enabled );
                        KConfigNodeRelease ( proxy_enabled );
                    }
                    else if ( GetRCState ( rc ) == rcNotFound )
                    {
                        rc = 0;
                    }

                    if ( rc != 0 )
                    {
                        KNSManagerSetHTTPProxyPath ( self, NULL );
                        assert ( self -> http_proxy_enabled == false );
                    }
                }

                StringWhack ( path );
            }

            KConfigNodeRelease ( proxy_path );
        }

        KConfigNodeRelease ( proxy );
    }
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

LIB_EXPORT rc_t CC KNSManagerMakeConfig ( KNSManager **mgrp, KConfig* kfg )
{
    rc_t rc;

    if ( mgrp == NULL )
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
            mgr -> http_read_timeout = MAX_HTTP_READ_LIMIT;
            mgr -> http_write_timeout = MAX_HTTP_WRITE_LIMIT;
            mgr -> maxTotalWaitForReliableURLs_ms = 10 * 60 * 1000; /* 10 min */
            mgr -> maxNumberOfRetriesOnFailureForReliableURLs = 10;
            mgr -> verbose = false;

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
                    KNSManagerLoadAWS ( mgr, kfg );
                    KNSManagerHttpProxyInit ( mgr, kfg );
                    * mgrp = mgr;
                    return 0;
                }
            }

            free ( mgr );
        }

        * mgrp = NULL;
    }

    return rc;
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
    if ( self == NULL )
    {
        if ( conn == NULL )
            return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );

        * conn = NULL;

        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    }

    return KNSManagerMakeRetryTimedConnection ( self, conn, 
        self -> conn_timeout, self -> conn_read_timeout, self -> conn_write_timeout, from, to );
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
    if ( self == NULL )
    {
        if ( conn == NULL )
            return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );

        * conn = NULL;

        return RC ( rcNS, rcStream, rcConstructing, rcSelf, rcNull );
    }

    return KNSManagerMakeRetryTimedConnection ( self, conn, 
        self -> conn_timeout, readMillis, writeMillis, from, to );
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
    struct KSocket **conn, int32_t retryTimeout, struct KEndPoint const *from, struct KEndPoint const *to )
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
    int32_t connectSecs, int32_t readMillis, int32_t writeMillis )
{
    if ( self == NULL )
        return RC ( rcNS, rcMgr, rcUpdating, rcSelf, rcNull );

    /* limit values */
    if ( connectSecs < 0 )
        connectSecs = -1;
    else if ( connectSecs > MAX_CONN_LIMIT )
        connectSecs = MAX_CONN_LIMIT;
        
    if ( readMillis < 0 )
        readMillis = -1;
    else if ( readMillis > MAX_CONN_READ_LIMIT )
        readMillis = MAX_CONN_READ_LIMIT;

    if ( writeMillis < 0 )
        writeMillis = -1;
    else if ( writeMillis > MAX_CONN_WRITE_LIMIT )
        writeMillis = MAX_CONN_WRITE_LIMIT;

    self -> conn_timeout = connectSecs;
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
    if ( readMillis < 0 )
        readMillis = -1;
    else if ( readMillis > MAX_HTTP_READ_LIMIT )
        readMillis = MAX_HTTP_READ_LIMIT;

    if ( writeMillis < 0 )
        writeMillis = -1;
    else if ( writeMillis > MAX_HTTP_WRITE_LIMIT )
        writeMillis = MAX_HTTP_WRITE_LIMIT;

    self -> http_read_timeout = readMillis;
    self -> http_write_timeout = writeMillis;

    return 0;
}

/* GetHTTPProxyPath
 *  returns path to HTTP proxy server ( if set ) or NULL.
 *  return status is 0 if the path is valid, non-zero otherwise
 */
LIB_EXPORT rc_t CC KNSManagerGetHTTPProxyPath ( const KNSManager * self, const String ** proxy )
{
    rc_t rc = 0;

    if ( proxy == NULL )
        rc = RC ( rcNS, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcNS, rcMgr, rcAccessing, rcSelf, rcNull );
        else if ( self -> http_proxy != NULL )
        {
            return StringCopy ( proxy, self -> http_proxy );
        }

        * proxy = NULL;
    }

    return rc;
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

LIB_EXPORT rc_t CC KNSManagerVSetHTTPProxyPath ( KNSManager * self, const char * fmt, va_list args )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC ( rcNS, rcMgr, rcUpdating, rcSelf, rcNull );
    else
    {
        uint16_t proxy_port = 0;
        const String * proxy = NULL;

        if ( fmt != NULL && fmt [ 0 ] != 0 )
        {
            size_t psize;
            char path [ 4096 ];
            rc = string_vprintf ( path, sizeof path, & psize, fmt, args );
            if ( rc == 0 && psize != 0 )
            {
                char * colon = string_rchr ( path, psize, ':' );
                if ( colon != NULL )
                {
                    char * end;
                    const char * port_spec = colon + 1;
                    /* it is true that some day we might read symbolic port names... */
                    long port_num = strtol ( port_spec, & end, 10 );
                    if ( port_num <= 0 || port_num >= 0x10000 || end [ 0 ] != 0 )
                        rc = RC ( rcNS, rcMgr, rcUpdating, rcPath, rcInvalid );
                    else
                    {
                        proxy_port = ( uint64_t ) port_num;
                        psize = colon - path;
                    }
                }

                if ( rc == 0 )
                {
                    String tmp;
                    StringInit ( & tmp, path, psize, string_len ( path, psize ) );
                    rc = StringCopy ( & proxy, & tmp );
                }
            }
        }

        if ( rc == 0 )
        {
            if ( self -> http_proxy != NULL )
            {
                StringWhack ( self -> http_proxy );
                self -> http_proxy_port = 0;
            }

            self -> http_proxy = proxy;
            self -> http_proxy_enabled = ( proxy != NULL );
            self -> http_proxy_port = proxy_port;
        }
    }

    return rc;
}


/* GetHTTPProxyEnabled
 *  returns true if a non-NULL proxy path exists and user wants to use it
 *  users indicate desire to use proxy through configuration or SetHTTPProxyEnabled
 */
LIB_EXPORT bool CC KNSManagerGetHTTPProxyEnabled ( const KNSManager * self )
{
    if ( self != NULL )
        return self -> http_proxy_enabled;

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
    {
        prior = self -> http_proxy_enabled;
        self -> http_proxy_enabled = enabled;
    }
    return prior;
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
