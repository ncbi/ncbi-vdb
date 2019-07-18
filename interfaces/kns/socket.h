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
#ifndef _h_kns_socket_
#define _h_kns_socket_

#ifndef _h_kns_extern_
#include <kns/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct timeout_t;
struct KStream;
struct KEndPoint;
struct KNSManager;

/*--------------------------------------------------------------------------
 * KSocket
 */
typedef struct KSocket KSocket;


/* MakeConnection
 *  create a connection-oriented stream
 * MakeTimedConnection
 *  create a connection-oriented stream having specific read/write timeouts
 * MakeRetryConnection
 *  create a connection-oriented stream before stated timeout, retrying as necessary
 * MakeRetryTimedConnection
 *  create a connection-oriented stream having specific read/write timeouts,
 *  before stated timeout, retrying as necessary
 *
 *  "conn" [ OUT ] - a stream for communication with the server
 *
 *  "retryTimeout" [ IN ] - the connect request should be repeated upon failure
 *   until this timeout expires.
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
KNS_EXTERN rc_t CC KNSManagerMakeConnection ( struct KNSManager const * self,
    struct KSocket ** conn, struct KEndPoint const * from, struct KEndPoint const * to );

KNS_EXTERN rc_t CC KNSManagerMakeTimedConnection ( struct KNSManager const * self,
    struct KSocket ** conn, int32_t readMillis, int32_t writeMillis,
    struct KEndPoint const * from, struct KEndPoint const * to );

KNS_EXTERN rc_t CC KNSManagerMakeRetryConnection ( struct KNSManager const * self,
    struct KSocket ** conn, struct timeout_t * retryTimeout,
    struct KEndPoint const * from, struct KEndPoint const * to );

KNS_EXTERN rc_t CC KNSManagerMakeRetryTimedConnection ( struct KNSManager const * self,
    struct KSocket ** conn, struct timeout_t * retryTimeout,
    int32_t readMillis, int32_t writeMillis,
    struct KEndPoint const * from, struct KEndPoint const * to );


/* AddRef
 * Release
 */
KNS_EXTERN rc_t CC KSocketAddRef ( const KSocket *self );
KNS_EXTERN rc_t CC KSocketRelease ( const KSocket *self );


/* GetStream
 */
KNS_EXTERN rc_t CC KSocketGetStream ( const KSocket * self, struct KStream ** s );


/* Query endpoints
 */
KNS_EXTERN rc_t CC KSocketGetRemoteEndpoint ( const KSocket * self, struct KEndPoint * ep );
KNS_EXTERN rc_t CC KSocketGetLocalEndpoint ( const KSocket * self, struct KEndPoint * ep );

/*--------------------------------------------------------------------------
 * KListener
 */
typedef struct KListener KListener;

/* MakeListener
 *  create a listener socket for accepting incoming connections
 *  enter listening state upon first use,
 *
 *  "ep" [ IN ] - a local endpoint
 *
 *  "listener" [ IN ] - a listener socket
 */
KNS_EXTERN rc_t CC KNSManagerMakeListener ( struct KNSManager const *self,
    KListener **listener, struct KEndPoint const * ep );


/* AddRef
 * Release
 */
KNS_EXTERN rc_t CC KListenerAddRef ( const KListener *self );
KNS_EXTERN rc_t CC KListenerRelease ( const KListener *self );


/* Accept
 *  wait for an incoming connection
 *
 *  "conn" [ OUT ] - a stream for communication with the client 
 */
KNS_EXTERN rc_t CC KListenerAccept ( KListener *self, struct KSocket **conn );


#ifdef __cplusplus
}
#endif

#endif /* _h_kns_socket_ */
