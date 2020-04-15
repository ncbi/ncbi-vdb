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
 */

#ifndef _h_krypto_rsa_aes_hmac_
#define _h_krypto_rsa_aes_hmac_

#ifndef _h_krypto_extern_
#include <krypto/extern.h>
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
struct KDataBuffer;


/* RsaAesHmacEncrypt
 *  encrypt a block of data using a random AES key, which is itself
 *  encrypted using an RSA key, and generate a SHA-256 HMAC into
 *  the provided binary output buffer.
 *
 *  the output will be in pure binary form, lacking a header and
 *  any text-encoding such as base64 that might be needed for
 *  transmission.
 *
 *  "out" [ IN/OUT ] - pointer to a ZEROED KDataBuffer
 *  that will receive encrypted result. must be zeroed with
 *  "memset ( & out, 0, sizeof out );" or equivalent.
 *
 *  "in" [ IN ] and "in_bytes" [ IN ] - block of data to be encrypted
 *
 *  "zpek" [ IN ] - NUL-terminated RSA public encryption key
 *  in PEM format. MUST be NUL-terminated.
 */
KRYPTO_EXTERN rc_t CC RsaAesHmacEncrypt ( struct KDataBuffer * out,
    const void * in, size_t in_bytes, const char * zpek );


/* RsaAesHmacDecrypt
 *  decrypt a block of enciphered data produced by RsaAesHmacEncrypt().
 *
 *  "out" [ IN/OUT ] - pointer to a ZEROED KDataBuffer
 *  that will receive decrypted result. must be zeroed with
 *  "memset ( & out, 0, sizeof out );" or equivalent.
 *
 *  "in" [ IN ] and "in_bytes" [ IN ] - block of enciphered data to be decrypted
 *
 *  "zpdk" [ IN ] - a NUL-terminated FILE PATH to an
 *  RSA private decryption key in PEM format.
 *
 *  "zpwd" [ IN/OUT ] and "zpwd_size" [ IN ] - a mutable buffer containing
 *  a NUL-terminated password string used to decrypt the file designated by
 *  "zpdk". "zpwd_size" should be the total size of the BUFFER, not the
 *  size of the password itself. The buffer will be wiped after first use
 *  regardless of success or failure.
 */
KRYPTO_EXTERN rc_t CC RsaAesHmacDecrypt ( struct KDataBuffer * out,
    const void * in, size_t in_bytes, const char * zpdk,
    char * zpwd, size_t zpw_size );


/* EncryptForNCBI
 *  encrypts a block using a public RSA encryption key belonging to NCBI
 */
KRYPTO_EXTERN rc_t CC EncryptForNCBI ( struct KDataBuffer * out,
    const void * in, size_t in_bytes );


#ifdef __cplusplus
}
#endif

#endif /*  _h_krypto_rsa_aes_hmac_ */
