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

#ifdef __cplusplus
extern "C" {
#endif

struct String;
struct KConfig;
struct HttpRetrySpecs;

struct KNSManager
{
    KRefcount refcount;
    
    struct KConfig * kfg;
    struct String const * http_proxy;

    struct String const *aws_access_key_id;
    struct String const *aws_secret_access_key;
    struct String const *aws_region;
    struct String const *aws_output;
    
    struct HttpRetrySpecs retry_specs;
    
    int32_t conn_timeout;
    int32_t conn_read_timeout;
    int32_t conn_write_timeout;
    int32_t http_read_timeout;
    int32_t http_write_timeout;
    
    uint32_t maxTotalWaitForReliableURLs_ms;

    uint16_t http_proxy_port;

    uint8_t  maxNumberOfRetriesOnFailureForReliableURLs;

    bool http_proxy_enabled; /* TBD - does this need to be static today? */
    bool verbose;
};

/*
    private getter for the user-agent...
    currently "STATIC", i.e. doesn't take "self" param
*/
rc_t CC KNSManagerGetUserAgent ( const char ** user_agent );

/* test */
struct KStream;
void KStreamForceSocketClose ( struct KStream const * self );

#ifdef __cplusplus
}
#endif

#endif /* _h_libs_kns_mgr_priv_ */
