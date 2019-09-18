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
#ifndef _h_kns_mgr_priv_
#define _h_kns_mgr_priv_

#ifndef _h_kns_extern_
#include <kns/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct KStream;
struct KHttpFile;
struct KNSManager;
struct KFile;
struct KConfig;
struct KClientHttpRequest;

/************************** HTTP-retry-related stuff **************************/
struct HttpRetrySchedule;

struct HttpRetrySpecs
{
    struct HttpRetrySchedule** codes;
    uint8_t count;
};
typedef struct HttpRetrySpecs HttpRetrySpecs;

rc_t CC HttpRetrySpecsDestroy ( HttpRetrySpecs* self );

rc_t CC HttpRetrySpecsInit ( HttpRetrySpecs* self, struct KConfig* kfg);

bool HttpGetRetryCodes ( const HttpRetrySpecs* self, uint16_t code, uint8_t * max_retries, const uint16_t ** sleep_before_retry, bool * open_ended );

/* MakeConfig
 *  Create a manager instance using a custom configuration, for testing.
 *  Don't imitialize singleton.
 *  Same as KNSManagerMakeLocal for backward compatibility.
 */
KNS_EXTERN rc_t CC KNSManagerMakeConfig ( struct KNSManager **mgr, struct KConfig * kfg );

/* MakeLocal
 *  create a manager instance without initializing singleton, for testing
 */
KNS_EXTERN rc_t CC KNSManagerMakeLocal ( struct KNSManager ** mgr,
    struct KConfig * kfg );

/* MakeWithConfig
 *  Create a manager instance using a custom configuration, for testing.
 *  Initialize singleton.
 */
KNS_EXTERN rc_t CC KNSManagerMakeWithConfig ( struct KNSManager ** mgr,
    struct KConfig * kfg );

/* SetAdCaching
 *  Enable Caching to Accession as Directory in cwd
 */
KNS_EXTERN
rc_t CC KNSManagerSetAdCaching(struct KNSManager* self, bool enabled);


/** MakeReliableHttpFile, KNSManagerMakeReliableClientRequest:
 * Make HTTP file/request from a reliable URL:
 * we will try harder to recover upon any error
 * (make more retries)
 */
KNS_EXTERN rc_t CC KNSManagerMakeReliableHttpFile(
    struct KNSManager const *self, struct KFile const **file, struct KStream *conn, ver_t vers,
    bool reliable, bool need_env_token, bool payRequired,
    const char *url, ...);

KNS_EXTERN rc_t CC KNSManagerMakeReliableClientRequest (
    struct KNSManager const *self, struct KClientHttpRequest **req,
    ver_t version, struct KStream *conn, const char *url, ... );
KNS_EXTERN rc_t CC KNSManagerMakePaidHttpFile(struct KNSManager const *self,
    struct KFile const **file, struct KStream *conn, ver_t vers,
    bool payRequired, const char *url, ...);
KNS_EXTERN rc_t CC KNSManagerMakePaidReliableHttpFile(
    struct KNSManager const *self, struct KFile const **file,
    struct KStream *conn, ver_t vers, bool payRequired, const char *url, ...);

typedef struct {
    const char *url;

    const struct KNSManager * kns; /* used to retrieve HttpRetrySpecs */
    uint32_t last_sleep;
    uint32_t total_wait_ms;
    uint32_t max_total_wait_ms;

    uint32_t last_status;

    uint8_t max_retries;
    uint8_t retries_count;
} KHttpRetrier;

rc_t KHttpRetrierInit ( KHttpRetrier * self, const char * url, const struct KNSManager * kns );
bool KHttpRetrierWait ( KHttpRetrier * self, uint32_t status );
rc_t KHttpRetrierDestroy ( KHttpRetrier * self );

/*----------------------------------------------------------------------------*/

//typedef struct HttpProxy HttpProxy;
struct KNSProxies * KNSManagerGetProxies ( const struct KNSManager * self,
                                           size_t * cnt );

/* N.B.: DO NOT WHACK THE RETURNED http_proxy String !!! */
bool KNSProxiesGet ( struct KNSProxies * self, const String ** http_proxy,
    uint16_t * http_proxy_port, size_t * cnt, bool * last );

/* allow to have multiple comma-separated proxies in a spec */
#define MULTIPLE_PROXIES 1

/*--------------------------------------------------------------------------
 * URLBlock
 *  RFC 3986
 */
typedef enum
{
    st_NONE,
    st_HTTP,
    st_HTTPS,
    st_S3
} SchemeType;

typedef enum
{
    ct_NONE,
    ct_S3,
} CloudType;

typedef struct URLBlock URLBlock;
struct URLBlock
{
    String scheme;
    String host;
    String path; /* Path includes any parameter portion */
    String query;
    String fragment;

    uint32_t port;

    SchemeType scheme_type;
    CloudType cloud_type;
    bool tls;

    bool port_dflt;
};
extern void URLBlockInit ( URLBlock *self );
extern rc_t ParseUrl ( URLBlock * b, const char * url, size_t url_size );

#ifdef __cplusplus
}
#endif

#endif /* _h_kns_mgr_priv_ */
