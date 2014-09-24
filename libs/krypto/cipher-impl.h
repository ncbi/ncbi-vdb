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

#ifndef _h_krypto_cipher_impl_
#define _h_krypto_cipher_impl_

#include <krypto/extern.h>
#include <klib/defs.h>
#include <klib/refcount.h>
#include <krypto/cipher.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef CIPHER_IMPL
#define CIPHER_IMPL KCipher
#else
struct CIPHER_IMPL;
typedef struct CIPHER_IMPL CIPHER_IMPL;
#endif

typedef struct KCipher_vt KCipher_vt;
struct KCipher_vt
{
    uint32_t maj;
    uint32_t min;
};


typedef struct KCipher_vt_v1 KCipher_vt_v1;
struct KCipher_vt_v1
{
    KCipher_vt version;

    /* version == 1.x */
    /* start minor version == 0 */
    rc_t ( CC * destroy             )(CIPHER_IMPL * self);

    rc_t ( CC * set_encrypt_key     )(CIPHER_IMPL * self,
                                      const char * user_key,
                                      uint32_t user_key_bits);

    rc_t ( CC * set_decrypt_key     )(CIPHER_IMPL * self,
                                      const char * user_key,
                                      uint32_t user_key_bits);

    rc_t ( CC * set_encrypt_ivec    )(CIPHER_IMPL * self,
                                      const void * ivec);

    rc_t ( CC * set_decrypt_ivec    )(CIPHER_IMPL * self,
                                      const void * ivec);

    rc_t ( CC * set_encrypt_ctr_func)(CIPHER_IMPL * self,
                                      cipher_ctr_func enc_ctr_func);

    rc_t ( CC * set_decrypt_ctr_func)(CIPHER_IMPL * self,
                                      cipher_ctr_func enc_ctr_func);

    /* single block encryption */
    rc_t ( CC * encrypt             )(const CIPHER_IMPL * self,
                                      const void * in,
                                      void * out);
    rc_t ( CC * decrypt             )(const CIPHER_IMPL * self,
                                      const void * in,
                                      void * out);

    /* electronic code book */
    rc_t ( CC * encrypt_ecb         )(const CIPHER_IMPL * self,
                                      const void * in,
                                      void * out,
                                      uint32_t block_count);

    rc_t ( CC * decrypt_ecb         )(const CIPHER_IMPL * self,
                                      const void * in,
                                      void * out,
                                      uint32_t block_count);

    /* cipher-block chaining */
    rc_t ( CC * encrypt_cbc         )(CIPHER_IMPL * self,
                                      const void * in,
                                      void * out,
                                      uint32_t block_count);
    rc_t ( CC * decrypt_cbc         )(CIPHER_IMPL * self,
                                      const void * in,
                                      void * out,
                                      uint32_t block_count);

    /* propagating cipher-block chaining */
    rc_t ( CC * encrypt_pcbc        )(CIPHER_IMPL * self, const void * in,
                                      void * out, uint32_t block_count);
    rc_t ( CC * decrypt_pcbc        )(CIPHER_IMPL * self, const void * in,
                                      void * out, uint32_t block_count);

    /* cipher feedback */
    rc_t ( CC * encrypt_cfb         )(CIPHER_IMPL * self, const void * in,
                                      void * out, uint32_t block_count);
    rc_t ( CC * decrypt_cfb         )(CIPHER_IMPL * self, const void * in,
                                      void * out, uint32_t block_count);

    /* output feedback */
    rc_t ( CC * encrypt_ofb         )(CIPHER_IMPL * self, const void * in,
                                      void * out, uint32_t block_count);
    rc_t ( CC * decrypt_ofb         )(CIPHER_IMPL * self, const void * in,
                                      void * out, uint32_t block_count);

    /* Counter */
    rc_t ( CC * encrypt_ctr         )(CIPHER_IMPL * self, const void * in,
                                      void * out, uint32_t block_count);
    rc_t ( CC * decrypt_ctr         )(CIPHER_IMPL * self, const void * in,
                                      void * out, uint32_t block_count);

    /* end minor version == 0 */

    /* ANY NEW ENTRIES MUST BE REFLECTED IN libs/krypto/cipher.c
       BY BOTH THE CORRESPONDING MESSAGE DISPATCH FUNCTION(s) AND
       VTABLE VALIDITY CHECKS IN CIPHERInit */
};

typedef union KCipher_ptr KCipher_ptr;
union KCipher_ptr
{
    const KCipher_vt    * version;
    const KCipher_vt_v1 * v1;
};


struct KCipher
{
    KRefcount refcount;
    uint32_t block_size;
    KCipher_ptr vt;
    const char * name;
    void * encrypt_key;
    void * decrypt_key;
    void * encrypt_ivec;
    void * decrypt_ivec;
    cipher_ctr_func encrypt_counter_func;
    cipher_ctr_func decrypt_counter_func;
};

void KCipherInit (KCipher * self, uint32_t block_size,
                  const KCipher_vt * vt, const char * name);


#ifdef __cplusplus
}
#endif

#endif /* _h_krypto_cipher_impl_ */
