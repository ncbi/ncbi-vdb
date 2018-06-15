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
#ifndef _h_kns_tls_
#define _h_kns_tls_

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
struct String;
struct KSocket;
struct KStream;
struct KNSManager;


/*--------------------------------------------------------------------------
 * KTLSStream
 *  a stream encrypted with TLS
 */
typedef struct KTLSStream KTLSStream;


/* MakeTLSStream
 *  create a TLS wrapper upon an existing KSocket
 *
 *  data read from or written to TLS stream is in plaintext
 *  data from to or wrapped stream is in ciphertext
 *
 *  "plaintext" [ OUT ] - return parameter for TLS stream
 *
 *  "ciphertext" [ IN ] - existing stream of ciphertext
 *
 *  "host" [ IN ] - host dns name
 */
KNS_EXTERN rc_t CC KNSManagerMakeTLSStream ( struct KNSManager const * self,
    KTLSStream ** plaintext, struct KSocket const * ciphertext,
    struct String const * host );

    
/* Set/Get AllowAllCerts
 *  modify behavior of TLS certificate validation
 */
KNS_EXTERN rc_t CC KNSManagerSetAllowAllCerts ( struct KNSManager *self, bool allow_all_certs );
KNS_EXTERN rc_t CC KNSManagerGetAllowAllCerts ( struct KNSManager const *self, bool * allow_all_certs );


/* AddRef
 * Release
 *  ignores NULL references
 */
KNS_EXTERN rc_t CC KTLSStreamAddRef ( const KTLSStream * self );
KNS_EXTERN rc_t CC KTLSStreamRelease ( const KTLSStream * self );


/* VerifyCACert
 *  noop if "AllowAllCerts" is true
 */
KNS_EXTERN rc_t CC KTLSStreamVerifyCACert ( const KTLSStream * self );


/* GetStream
 *  reference-counted cast operation
 *  creates a reference to a KStream from a KTLSStream
 */
KNS_EXTERN rc_t CC KTLSStreamGetStream ( const KTLSStream * self,
    struct KStream ** strm );


#ifdef __cplusplus
}
#endif

#endif /* _h_kns_tls_ */
