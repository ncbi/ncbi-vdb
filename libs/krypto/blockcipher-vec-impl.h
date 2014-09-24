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
#error "no direct includes"

#ifndef _h_krypto_blockcipher_vec_impl_
#define _h_krypto_blockcipher_vec_impl_

/* 
 * NOTE:  Currently this is being written to support cipher blocks of
 * exactly 128 bits.  It will have to be amended if alternative block
 * sizes are need for other cipher blocks
 */

#include <krypto/extern.h>
#include <klib/defs.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * CIPHER
 *  
 */
#define CIPHER_BLOCK_MAX (16)

#ifdef GNUC
#include <v128.h>
typedef v128_u8_t CipherVec;
#else
typedef struct CipherVec CipherVec;
#endif


typedef union KBlockCipherVec KBlockCipherVec;

#ifndef BLOCKCIPHER_VEC_IMPL
#define BLOCKCIPHER_VEC_IMPL KBlockCipherVec
#else
typedef union KBlockCipherVec BLOCKCIPHER_VEC_IMPL;
#endif

typedef struct KBlockCipherVec_vt KBlockCipherVec_vt;
struct KBlockCipherVec_vt
{
    uint32_t maj;
    uint32_t min;
};


typedef struct KBlockCipherVec_vt_v1 KBlockCipherVec_vt_v1;
struct KBlockCipherVec_vt_v1
{
    KBlockCipherVec_vt version;

    /* version == 1.x */

    /* start minor version == 0 */
    void        (* destroy         )(const BLOCKCIPHER_VEC_IMPL * self);

    uint32_t    (* block_size      )();

    uint32_t    (* key_size        )();

    rc_t        (* set_encrypt_key )(void * encrypt_key,
                                     const char * user_key,
                                     uint32_t user_key_size);

    rc_t        (* set_decrypt_key )(void * decrypt_key,
                                     const char * user_key,
                                     uint32_t user_key_size);

    CipherVec   (* encrypt         )(CipherVec block,
                                     const void * encrypt_key);

    CipherVec   (* decrypt         )(CipherVec block,
                                     const void * decrypt_key);

    /* end minor version == 0 */

};

union KBlockCipherVec
{
    KBlockCipherVec_vt    version;
    KBlockCipherVec_vt_v1 v1;
};


#ifdef __cplusplus
}
#endif

#endif /* _h_krypto_blockcipher_vec_impl_ */

