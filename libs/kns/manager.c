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
#include "libkns.vers.h"
#include "mgr-priv.h"
#include "stream-priv.h"
#include "sysmgr.h"

#include <kfg/config.h>

#include <klib/misc.h> /* GetHttpTries */
#include <klib/printf.h>
#include <klib/refcount.h>
#include <klib/rc.h>

#include <kns/manager.h>
#include <kns/socket.h>

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

static
rc_t KNSManagerWhack ( KNSManager * self )
{
    rc_t rc;
    KConfigRelease ( self -> kfg );
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
                    ver_t version = LIBKNS_VERS;
                    KNSManagerSetUserAgent ( mgr, PKGNAMESTR " ncbi-vdb.%V", version );
                }

                rc = KConfigAddRef ( kfg );
                if ( rc == 0 )
                {
                    mgr -> kfg = kfg;
                    rc = HttpRetrySpecsInit ( & mgr -> retry_specs, mgr -> kfg );
                    if ( rc == 0 )
                    {
                        * mgrp = mgr;
                        return 0;
                    }
                    KConfigRelease ( kfg );
                }
            }

            free ( mgr );
        }

        * mgrp = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KNSManagerMake ( KNSManager **mgrp )
{
    KConfig* kfg;
    rc_t rc = KConfigMake(&kfg, NULL);
    if ( rc == 0 )
    {
        rc_t rc2;
        rc = KNSManagerMakeConfig ( mgrp, kfg );
        rc2 = KConfigRelease ( kfg );
        if ( rc == 0 )
        {   
            rc = rc2;
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
KNS_EXTERN rc_t CC KNSManagerMakeTimedConnection ( struct KNSManager const * self,
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
KNS_EXTERN rc_t CC KNSManagerMakeRetryConnection ( struct KNSManager const * self,
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


rc_t CC KNSManagerGetUserAgent ( const char ** user_agent )
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

/*TODO:remove*/
uint32_t KNSManagerGetNumberOfRetriesOnFailure(const KNSManager *self) {
    if (self == NULL) {
        return 0;
    }
    return self->maxNumberOfRetriesOnFailure;
}
uint32_t KNSManagerGetNumberOfRetriesOnFailureForReliableURLs
    (const KNSManager *self)
{
    if (self == NULL) {
        return 0;
    }
    return self->maxNumberOfRetriesOnFailureForReliableURLs;
}
uint32_t KNSManagerGetLogFailuresNumber(const KNSManager *self) {
    if (self == NULL) {
        return 0;
    }
    return self->logFailures;
}
uint32_t KNSManagerGetTestFailuresNumber(const KNSManager *self) {
    if (self == NULL) {
        return 0;
    }
    return self->testFailuresNumber;
}
