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

#ifndef _h_libs_kns_mgr_priv_
#define _h_libs_kns_mgr_priv_

#ifndef _h_kns_extern_
#include <kns/extern.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef _h_kns_mgr_priv_
#include <kns/kns-mgr-priv.h>
#endif

#ifndef _h_libs_kns_tls_priv_
#include "tls-priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct String;
struct KConfig;
struct HttpRetrySpecs;
struct KNSProxies;
struct CloudMgr;

struct KNSManager
{
    struct HttpRetrySpecs retry_specs;

    KTLSGlobals tlsg;

    KRefcount refcount;

    int32_t conn_timeout;
    int32_t conn_read_timeout;
    int32_t conn_write_timeout;
    int32_t http_read_timeout;
    int32_t http_write_timeout;

    uint32_t maxTotalWaitForReliableURLs_ms;

    uint8_t  maxNumberOfRetriesOnFailureForReliableURLs;

    struct KNSProxies * proxies;

    struct CloudMgr * cloud;

    bool verbose;
    bool logTlsErrors;

    /* VResolverCache try to resolve to user's cache before cwd/AD */
    bool resolveToCache;

    /* VResolverCache ia allowed to resolve to user's cache vs. cwd/AD */
    bool enabledResolveToAd;

    bool accept_aws_charges;
    bool accept_gcp_charges;

    bool NCBI_VDB_NETnoLogError;
    bool NCBI_VDB_NETkfgValueSet;
    bool NCBI_VDB_NETkfgValue;

    bool notSingleton;
};

bool KNSManagerLogNcbiVdbNetError ( const struct KNSManager * self );
void KNSManagerSetLogNcbiVdbNetError(struct KNSManager * self, bool set);

/* returns true when we should not try direct internet connection
 * when HttpProxies are set */
bool KNSManagerHttpProxyOnly ( const struct KNSManager * self );

rc_t KNSManagerGetCloudLocation(const struct KNSManager * self,
    char * buffer, size_t bsize, size_t * num_read, size_t * remaining);

/******************************** KNSProxies **********************************/

struct KNSProxies * KNSManagerKNSProxiesMake ( struct KNSManager * mgr,
                                               const struct KConfig * kfg );

rc_t KNSProxiesWhack ( struct KNSProxies * self );

bool KNSProxiesGetHTTPProxyEnabled ( const struct KNSProxies * self );

bool KNSProxiesSetHTTPProxyEnabled ( struct KNSProxies * self, bool enabled );

bool KNSProxiesHttpProxyOnly ( const struct KNSProxies * self );

rc_t KNSProxiesVSetHTTPProxyPath ( struct KNSProxies * self,
    const char * fmt, va_list args, bool clear );

struct KNSProxies * KNSProxiesGetHttpProxy ( struct KNSProxies * self,
                                             size_t * cnt );

/* DEPRECATED */
rc_t KNSProxiesGetHttpProxyPath ( const struct KNSProxies* self,
                                  const String ** proxy );


/************************************ test ************************************/
struct KStream;
void KStreamForceSocketClose ( struct KStream const * self );

#ifdef __cplusplus
}
#endif

#endif /* _h_libs_kns_mgr_priv_ */
