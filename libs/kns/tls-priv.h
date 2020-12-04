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

#ifndef _h_libs_kns_tls_priv_
#define _h_libs_kns_tls_priv_

#ifndef _h_kns_tls_
#include <kns/tls.h>
#endif

#if ! defined ( MBEDTLS_CONFIG_FILE )
#include <mbedtls/config.h>
#else
#include MBEDTLS_CONFIG_FILE 
#endif

#include <mbedtls/net.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/aes.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>

#ifdef __cplusplus
extern "C" {
#endif

struct KConfig;

/*--------------------------------------------------------------------------
 * KTLSGlobals
 */
typedef struct KTLSGlobals KTLSGlobals;
struct KTLSGlobals
{
    mbedtls_x509_crt cacert;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    mbedtls_ssl_config config;

    bool allow_all_certs;

    bool safe_to_modify_ssl_config; /* needed for SetAllowAllCerts () */
};

/* Init
 */
rc_t KTLSGlobalsInit ( KTLSGlobals * tlsg, struct KConfig const * kfg );

/* Whack
 */
void KTLSGlobalsWhack ( KTLSGlobals * self );


#ifdef __cplusplus
}
#endif

#endif /* _h_libs_kns_tls_priv_ */
