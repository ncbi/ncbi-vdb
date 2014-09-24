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
#ifndef _h_krypto_blockcipher_impl_
#define _h_krypto_blockcipher_impl_

/* 
 * NOTE:  Currently this is being written to support cipher blocks of
 * exactly 128 bits.  It will have to be amended if alternative block
 * sizes are need for other cipher blocks
 */

#include <krypto/extern.h>
#include <klib/defs.h>

/*
 * we need to know whether we are supporting vectors as 
 * XMM type registers or as structs
 */
#include "ncbi-priv.h"


#ifndef USE_VEC
#error "USE_VEC most be defined true or false"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CIPHER_BLOCK_MAX (16)

typedef char CipherBlock [CIPHER_BLOCK_MAX];

#if USEVEC
#include <v128.h>
typedef v128_u8_t CipherVec;
#else
typedef struct CipherVec
{
    CipherBlock block;
} CipherVec;
#endif

typedef struct KBlockCipher_vt KBlockCipher_vt;
struct KBlockCipher_vt
{
    uint32_t maj;
    uint32_t min;
};

typedef uint32_t BlockCipherType;
enum block_cipher_type
{
    blockcipher_type_byte,
    blockcipher_type_vec
};


typedef struct KBlockCipherByte_vt_v1 KBlockCipherByte_vt_v1;
typedef union KBlockCipherByte KBlockCipherByte;

#ifndef BLOCKCIPHER_BYTE_IMPL
#define BLOCKCIPHER_BYTE_IMPL KBlockCipherByte
#else
typedef union KBlockCipherByte BLOCKCIPHER_BYTE_IMPL;
#endif


struct KBlockCipherByte_vt_v1
{
    KBlockCipher_vt version;

    /* version == 1.x */

    /* start minor version == 0 */
    void        (* destroy         )(const BLOCKCIPHER_BYTE_IMPL * self);

    uint32_t    (* block_size      )();

    uint32_t    (* key_size        )();

    rc_t        (* set_encrypt_key )(void * encrypt_key,
                                     const char * user_key,
                                     uint32_t user_key_bits);

    rc_t        (* set_decrypt_key )(void * decrypt_key,
                                     const char * user_key,
                                     uint32_t user_key_bits);

    void        (* encrypt         )(const CipherBlock in,
                                     CipherBlock out,
                                     const void * encrypt_key);

    void        (* decrypt         )(const CipherBlock in,
                                     CipherBlock out,
                                     const void * encrypt_key);

    /* end minor version == 0 */

};

union KBlockCipherByte
{
    KBlockCipher_vt        version;
    KBlockCipherByte_vt_v1 v1;
};


typedef struct KBlockCipherVec_vt_v1 KBlockCipherVec_vt_v1;
typedef union KBlockCipherVec KBlockCipherVec;

#ifndef BLOCKCIPHER_VEC_IMPL
#define BLOCKCIPHER_VEC_IMPL KBlockCipherVec
#else
typedef union KBlockCipherVec BLOCKCIPHER_VEC_IMPL;
#endif


struct KBlockCipherVec_vt_v1
{
    KBlockCipher_vt version;

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
    KBlockCipher_vt       version;
    KBlockCipherVec_vt_v1 v1;
};


#ifdef __cplusplus
}
#endif



#endif /* _h_krypto_blockcipher_impl_ */

