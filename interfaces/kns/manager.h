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


/*--------------------------------------------------------------------------
 * KNSManager
 *  manages the network system
 */
typedef struct KNSManager KNSManager;


/* Make
 *  create a manager instance
 */
KNS_EXTERN rc_t CC KNSManagerMake ( KNSManager **mgr );


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


/* Set/Get UserAgent
 *  for http connections
 */
KNS_EXTERN rc_t CC KNSManagerSetUserAgent ( KNSManager *self, const char * fmt, ... );
KNS_EXTERN rc_t CC KNSManagerGetUserAgent ( const char ** user_agent );

KNS_EXTERN rc_t CC KNSManagerSetUserAgentSuffix ( const char * suffix );
/* Setters for https connections */

/* IP addresses are in network byte order */
/*
KNS_EXTERN rc_t CC KNSManagerSetClientIPv4  ( KNSManager *self, uint32_t client_ipv4_addr);
KNS_EXTERN rc_t CC KNSManagerSetClientIPv6  ( KNSManager *self, const uint16_t client_ipv6_addr[]);
*/

/* Note that the passed in strings must be NUL terminated, and
 * should be reasonably short (< 100 characters).
 * */
KNS_EXTERN rc_t CC KNSManagerSetClientIP  ( KNSManager *self, const char * clientip );
KNS_EXTERN rc_t CC KNSManagerSetSessionID ( KNSManager *self, const char * sessionid );
KNS_EXTERN rc_t CC KNSManagerSetPageHitID ( KNSManager *self, const char * pagehitid );


#ifdef __cplusplus
}
#endif

#endif /* _h_kns_manager_ */
