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
#ifndef _h_kns_manager_
#define _h_kns_manager_

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
struct KConfig;

/*--------------------------------------------------------------------------
 * KNSManager
 *  manages the network system
 */
typedef struct KNSManager KNSManager;


/* Make
 *  create a manager instance
 */
KNS_EXTERN rc_t CC KNSManagerMake ( KNSManager **mgr );

/* MakeLocal
 *  create a manager instance without initializing singleton,
 *  for testing;
 *  or using multiple client mTLS certificated
 */
KNS_EXTERN rc_t CC KNSManagerMakeLocal ( struct KNSManager ** mgr,
    struct KConfig * kfg );



/* AddRef
 * Release
 *  ignores NULL references
 */
KNS_EXTERN rc_t CC KNSManagerAddRef ( const KNSManager *self );
KNS_EXTERN rc_t CC KNSManagerRelease ( const KNSManager *self );


/* SetVerbose
 *  set/clear verbosity flag of manager ( dflt is false )...
 *  the network-code has to request it
 */
KNS_EXTERN void CC KNSManagerSetVerbose ( KNSManager *self, bool verbosity );


/* IsVerbose
 *  request the verbosity flag of manager ( dflt is false )...
 */
KNS_EXTERN bool CC KNSManagerIsVerbose ( const KNSManager *self );


/* SetConnectionTimeouts
 *  sets default connect/read/write timeouts to supply to sockets
 *
 *  "connectMillis", "readMillis", "writeMillis" [ IN ] - when negative, infinite timeout
 *  when 0, return immediately, positive gives maximum wait time in sec/mS
 *  for connects, reads and writes respectively.
 */
KNS_EXTERN rc_t CC KNSManagerSetConnectionTimeouts ( KNSManager *self,
    int32_t connectMillis, int32_t readMillis, int32_t writeMillis );


/******************************************************************************/
/**************** API to manage HTTP File read retry behavior *****************/
/******************************************************************************/

/* SetRetryFailedReads
 *  manages retry layer on HttpFileRead
 *
 *  "retry" [ IN ] - true : turn on retry layer,
 *                   false: don't create retry layer.
 */
KNS_EXTERN rc_t CC KNSManagerSetRetryFailedReads ( KNSManager *self,
    bool retry );

/* GetRetryFailedReads
 *  returns whether or not retry layer on HttpFileRead is turned on
 */
KNS_EXTERN rc_t CC KNSManagerGetRetryFailedReads ( const KNSManager *self,
    bool *retry );


/* SetMaxReadRetryTime
 *  sets maximum time in HttpFileRead retry loop
 *
 *  "millis" [ IN ] - when negative, infinite timeout
 */
KNS_EXTERN rc_t CC KNSManagerSetMaxReadRetryTime ( KNSManager *self,
    int32_t millis );

/* GetMaxReadRetryTime
 *  returns maximum time in HttpFileRead retry loop
 *
 *  "millis" [ OUT ] - when negative, infinite timeout
 */
KNS_EXTERN rc_t CC KNSManagerGetMaxReadRetryTime ( const KNSManager *self,
    int32_t *millis );


/* SetMaxConnectRetryTime
 *  sets maximum time when opening HttpFile
 *
 *  "millis" [ IN ] - when negative, infinite timeout
 */
KNS_EXTERN rc_t CC KNSManagerSetMaxConnectRetryTime(KNSManager *self,
    int32_t millis);

/* GetMaxConnectRetryTime
 *  returns maximum time when opening HttpFile
 *
 *  "millis" [ OUT ] - when negative, infinite timeout
 */
KNS_EXTERN rc_t CC KNSManagerGetMaxConnectRetryTime(const KNSManager *self,
    int32_t *millis);


/* SetRetryFirstReads
 *  manages retry on the first HttpFileRead
 */
KNS_EXTERN rc_t CC KNSManagerSetRetryFirstReads ( KNSManager *self,
    bool retry );

/* GetRetryFirstReads
 *  returns whether or not retry on the first HttpFileRead is turned on
 */
KNS_EXTERN rc_t CC KNSManagerGetRetryFirstReads ( const KNSManager *self,
    bool *retry );


/* SetOwnCert
 *  sets own certificate and key for SSL handshake
 *
 * "own_cert" - buffer holding the own public certificate chain data
 *              in PEM or DER format
 * "pk_key"   - buffer holding the own private key in PEM or DER format
 */
KNS_EXTERN rc_t CC KNSManagerSetOwnCert(struct KNSManager * self,
    const char * own_cert, const char * pk_key);

/* GetOwnCert
 *  gets own certificate and key for SSL handshake
 *
 * "own_cert" - buffer holding the own public certificate chain data
 *              in PEM or DER format
 * "pk_key"   - buffer holding the own private key in PEM or DER format
 */
KNS_EXTERN rc_t CC KNSManagerGetOwnCert(const struct KNSManager * self,
    const char ** own_cert, const char ** pk_key);


/******************************************************************************/

/* Setters for https connections */

/* Note that the passed in strings must be NUL terminated, and
 * shorter than KNSMANAGER_STRING_MAX.
 * */
#define KNSMANAGER_STRING_MAX 128

/* Set/Get global UserAgent string for http connections
 */
KNS_EXTERN rc_t CC KNSManagerSetUserAgent ( KNSManager *self, const char * fmt, ... );
KNS_EXTERN rc_t CC KNSManagerGetUserAgent ( const char ** user_agent );

/* Set/Get thread-local UserAgent suffix string
 */
KNS_EXTERN rc_t CC KNSManagerSetUserAgentSuffix ( const char * suffix );
KNS_EXTERN rc_t CC KNSManagerGetUserAgentSuffix ( const char ** suffix );

/* Set thread-local logging attributes
 */
KNS_EXTERN rc_t CC KNSManagerSetClientIP  ( KNSManager *self, const char * clientip );
KNS_EXTERN rc_t CC KNSManagerSetSessionID ( KNSManager *self, const char * sessionid );
KNS_EXTERN rc_t CC KNSManagerSetPageHitID ( KNSManager *self, const char * pagehitid );

/* IP addresses are in network byte order */
/*
KNS_EXTERN rc_t CC KNSManagerSetClientIPv4  ( KNSManager *self, uint32_t client_ipv4_addr);
KNS_EXTERN rc_t CC KNSManagerSetClientIPv6  ( KNSManager *self, const uint16_t client_ipv6_addr[]);
*/

/******************************************************************************/

typedef rc_t(CC *quitting_t)(void);
KNS_EXTERN rc_t CC KNSManagerSetQuitting ( KNSManager *self,
    quitting_t quitting );
KNS_EXTERN quitting_t CC KNSManagerGetQuitting ( const KNSManager *self );

#ifdef __cplusplus
}
#endif

#endif /* _h_kns_manager_ */
