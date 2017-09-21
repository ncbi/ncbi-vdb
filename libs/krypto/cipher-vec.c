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

#include <krypto/extern.h>
#include <klib/defs.h>

/* CIPHER_IMPL and BLOCKCIPHER_IMPL are defined in this header */
#include "ncbi-priv.h"
/* CIPHER_IMPL is used in this header */
#include "cipher-impl.h"
/* BLOCKCIPHER_IMPL is used in this header */
#include "blockcipher-impl.h"

#include "aes-ncbi-priv.h"
#include "null-ncbi-priv.h"
#include "cipher-priv.h"
#include "blockcipher-priv.h"

#include <krypto/cipher.h>
#include <krypto/ciphermgr.h>

#include <klib/defs.h>
#include <klib/refcount.h>
#include <klib/out.h>
#include <klib/rc.h>

#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


const char CMEMBER(ClassName)[] = CLASS_STRING(CIPHER_IMPL);



struct CIPHER_IMPL
{
    KCipher dad;
    const KBlockCipherVec * block_cipher;
};

typedef union CipherVec_u
{
    CipherVec   vec;
    CipherBlock block;
} CipherVec_u;

/*
 * Read a blocks worth of bytes into an AESState
 */
static __inline__ CipherVec CipherVecIn (const void * cvin)
{
#if USE_VEC_REG
    register CipherVec cv;

    __asm__ (
        "movdqu (%[a]),%[s]" : [s] "=x" (cv) : [a] "D" (cvin)
        );

    return cv;
#else
    CipherVec_u u;

    memmove (&u.block, cvin, sizeof (u));
    return u.vec;
#endif
}



/*
 * Write a block's worth of bytes out from an AESState
 */
static __inline__ void CipherVecOut (const CipherVec cv, void * cvout)
{
#if USE_VEC_REG
    register CipherVec rcv = cv;

    __asm__ (
        "movdqu %[s],(%[a])" : : [s] "x" (rcv), [a] "D" (cvout)
        );
#else
    CipherVec lcv = cv;
    memmove (cvout, &lcv, sizeof (lcv));
#endif
}


static rc_t CMEMBER(Destroy) (CIPHER_IMPL * self)
{
    rc_t rc = 0;

    if (self)
    {
        rc =  KCipherDestroy (&self->dad);
        free (self);
    }
    return rc;
}


static rc_t CMEMBER(SetEncryptKey)(CIPHER_IMPL * self,
                                  const char * user_key,
                                  uint32_t user_key_bits)
{
    rc_t rc = 0;

    assert (self);
    assert (user_key);
    assert (user_key_bits);

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcUpdating, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        self->block_cipher->v1.set_encrypt_key (self->dad.encrypt_key, user_key,
                                                user_key_bits);
        break;
    }
    return rc;
}


static rc_t CMEMBER(SetDecryptKey)(CIPHER_IMPL * self,
                                  const char * user_key,
                                  uint32_t user_key_bits)
{
    rc_t rc = 0;

    assert (self);
    assert (user_key);
    assert (user_key_bits);

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcUpdating, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        self->block_cipher->v1.set_decrypt_key (self->dad.decrypt_key, user_key,
                                                    user_key_bits);
        break;
    }
    return rc;
}


static
rc_t CMEMBER(SetEncryptIvec) (CIPHER_IMPL * self,
                             const void * ivec)
{
    assert (self);
    assert (ivec);

    memmove (self->dad.encrypt_ivec, ivec, self->dad.block_size);

    return 0;
}


static
rc_t CMEMBER(SetDecryptIvec) (CIPHER_IMPL * self,
                             const void * ivec)
{
    assert (self);
    assert (ivec);

    memmove (self->dad.decrypt_ivec, ivec, self->dad.block_size);

    return 0;
}


static
rc_t CMEMBER(SetEncryptCounterFunc) (CIPHER_IMPL * self,
                                    cipher_ctr_func func)
{
    assert (self);
    assert (func);

    self->dad.encrypt_counter_func = func;

    return 0;
}


static
rc_t CMEMBER(SetDecryptCounterFunc) (CIPHER_IMPL * self,
                                    cipher_ctr_func func)
{
    assert (self);
    assert (func);

    self->dad.decrypt_counter_func = func;

    return 0;
}


static __inline__ CipherVec CMEMBER(EncryptV1)(const CIPHER_IMPL * self,
                                              register CipherVec cv)
{
    return self->block_cipher->v1.encrypt (cv, self->dad.encrypt_key);
}


static __inline__ CipherVec  CMEMBER(DecryptV1)(const CIPHER_IMPL * self,
                                              register CipherVec cv)
{
    return self->block_cipher->v1.decrypt (cv, self->dad.decrypt_key);
}


static __inline__ rc_t CMEMBER(EncryptV1Int)(const CIPHER_IMPL * self,
                                            const void * in, void * out)
{
    CipherVec cv;

    cv = CipherVecIn(in);
    cv = CMEMBER(EncryptV1)(self, cv);
    CipherVecOut(cv, out);
    return 0;
}


static __inline__ rc_t CMEMBER(DecryptV1Int)(const CIPHER_IMPL * self,
                                            const void * in, void * out)
{
    CipherVec cv;

    cv = CipherVecIn(in);
    cv = CMEMBER(DecryptV1)(self, cv);
    CipherVecOut(cv, out);
    return 0;
}


static rc_t CMEMBER(Encrypt) (const CIPHER_IMPL * self, const void * in, void * out)
{
    rc_t rc;

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcEncoding, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        rc = CMEMBER(EncryptV1Int)(self, in, out);
        break;
    }

    return rc;
}


static
rc_t CMEMBER(Decrypt) (const CIPHER_IMPL * self, const void * in, void * out)
{
    rc_t rc = 0;

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcEncoding, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        rc = CMEMBER(DecryptV1Int)(self, in, out);
        break;
    }

    return rc;
}


/* ====================
 * longer runs of multiple blocks.
 *
 * The algorithms are well defined and standard in most cases
 *
 * These aremore or elss equivalent to class functions as they do not depend upon
 * the operation of the cipher and the algorithms are independent of anything about
 * the cipher other than its block size.
 *
 * PT: plain text block
 * CT: cipher text block
 * EK: encryption key
 * DK: decryption key (might be sthe same as EK)
 * ENC: encrypt cipher function on a block using a key
 * DEC: decrypt cipher function on a block using a key
 * IV: initialization vector - used as feedback for chaining
 * N:  number used once (nonce)
 * FB: feedback is the next IV in a chained/feedback mode
 */


/* -----
 * NOTE:
 * 'in' can be the same as 'out' but other overlaps are dangers as a block at a
 * time is written. The code does not look for overlaps at this point.
 */

/* ----------
 * Electronic Code Book - simple cipher with no chaining feedback  just iterate
 * simple encrypt/decrypt with the plain, text, cipher text and key/
 *
 * CT = ENC (PT,EK)
 * PT = DEC (CT,DK)
 */

/* -----
 * NOTE: currently an implmentation detail limits us to 8192 bit cipher block
 * size.  Changing MAX_BLOCK_SIZE in cipher.c can up that limit without 
 * causing any other compatibility issues. 
 *
 * Two local byte arrays are defined on the stack of 1024 bytes or 8192 bits.
 */
static
rc_t CMEMBER(EncryptEcb) (const CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
    const uint8_t * pin;
    uint8_t * pout;

    for ((pin = in), (pout = out);
         block_count --; 
         (pin += self->dad.block_size), (pout += self->dad.block_size))
    {
        CMEMBER(EncryptV1Int)(self, pin, pout);
    }
    return 0;
}


static
rc_t CMEMBER(DecryptEcb) (const CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
    const uint8_t * pin;
    uint8_t * pout;

    for ((pin = in), (pout = out);
         block_count --; 
         (pin += self->dad.block_size), (pout += self->dad.block_size))
    {
        CMEMBER(DecryptV1Int)(self, pin, pout);
    }

    return 0;
}


/* ----------
 * Cipher-Block Chaining
 * CT = (FB = ENC (PT^IV, EK))
 * PT = DEC ((FB = CT), DK)
 */
static
rc_t CMEMBER(EncryptCbc) (CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
    CipherVec cv;
    CipherVec ivec;
    const uint8_t * pin;
    uint8_t * pout;

#if 0
    ivec = *(const CipherVec*)self->dad.encrypt_ivec;
#else
    ivec = CipherVecIn (self->dad.encrypt_ivec);
#endif

    for ((pin = in), (pout = out);
         block_count --; 
         (pin += self->dad.block_size), (pout += self->dad.block_size))
    {
        cv = CipherVecIn (pin);
        cv ^= ivec;
        ivec = CMEMBER(EncryptV1)(self, cv);
        CipherVecOut (ivec, pout);
    }

#if 0
    *(CipherVec*)self->dad.encrypt_ivec = ivec;
#else
    CipherVecOut (ivec, self->dad.encrypt_ivec);
#endif

    return 0;
}    


static
rc_t CMEMBER(DecryptCbc) (CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
    CipherVec cv;
    CipherVec ivec;
    const uint8_t * pin;
    uint8_t * pout;

    ivec = CipherVecIn (self->dad.decrypt_ivec);

    for ((pin = in), (pout = out);
         block_count --; 
         (pin += self->dad.block_size), (pout += self->dad.block_size))
    {
        CipherVec temp;

        cv = CipherVecIn (pin);
#if 0
        temp = cv;
        temp = CMEMBER(DecryptV1)(self, temp);
#else
        temp = CMEMBER(DecryptV1)(self, cv);
#endif
        temp ^= ivec;
        CipherVecOut (temp, pout);
        ivec = cv;
    }

    *(CipherVec*)self->dad.decrypt_ivec = ivec;
    return 0;
}


/* ----------
 * Propagating cipher-block chaining
 * FB = PT ^ (CT = ENC ((PT^IV), EK))
 * FB = CT ^ (PT = DEC (CT,DK) ^ IV)
 */
static
rc_t CMEMBER(EncryptPcbc) (CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
#if 1
    CipherVec cv;
    CipherVec ivec;
    const uint8_t * pin;
    uint8_t * pout;

    ivec = *(const CipherVec*)self->dad.encrypt_ivec;

    for ((pin = in), (pout = out);
         block_count --; 
         (pin += self->dad.block_size), (pout += self->dad.block_size))
    {
        CipherVec temp;

        cv = CipherVecIn (pin);
        temp = cv ^ ivec;
        temp = CMEMBER(EncryptV1)(self, temp); 
        ivec = cv ^ temp;
        CipherVecOut (temp, pout);
    }

    *(CipherVec*)self->dad.encrypt_ivec = ivec;

    return 0;
#else
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
#endif
}


static
rc_t CMEMBER(DecryptPcbc) (CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
#if 1
    CipherVec cv;
    CipherVec ivec;
    const uint8_t * pin;
    uint8_t * pout;

    ivec = *(const CipherVec*)self->dad.encrypt_ivec;

    for ((pin = in), (pout = out);
         block_count --; 
         (pin += self->dad.block_size), (pout += self->dad.block_size))
    {
        CipherVec temp;

        cv = CipherVecIn (pin);
        temp = CMEMBER(DecryptV1)(self, cv);
        ivec ^= temp;
        CipherVecOut (temp, pout);
    }

    *(CipherVec*)self->dad.encrypt_ivec = ivec;

    return 0;
#else
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
#endif
}


/* ----------
 * Cipher Feedback
 * CT = (FB = PT) ^ ENC (IV, EK))
 * PT = (FB = CT) ^ DEC (IV, DK)
 */
static
rc_t CMEMBER(EncryptCfb) (CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
#if 1
    CipherVec cv;
    CipherVec ivec;
    const uint8_t * pin;
    uint8_t * pout;

    ivec = *(const CipherVec*)self->dad.encrypt_ivec;

    for ((pin = in), (pout = out);
         block_count --; 
         (pin += self->dad.block_size), (pout += self->dad.block_size))
    {
        ivec = CMEMBER(EncryptV1)(self, ivec); 
        cv = CipherVecIn (pin);
        ivec ^= cv;
        CipherVecOut (ivec, pout);
    }

    *(CipherVec*)self->dad.encrypt_ivec = ivec;

    return 0;
#else
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
#endif
}


static
rc_t CMEMBER(DecryptCfb) (CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
#if 1
    CipherVec cv;
    CipherVec ivec;
    const uint8_t * pin;
    uint8_t * pout;

    ivec = *(const CipherVec*)self->dad.encrypt_ivec;

    for ((pin = in), (pout = out);
         block_count --; 
         (pin += self->dad.block_size), (pout += self->dad.block_size))
    {
        cv = CMEMBER(EncryptV1)(self, ivec); 
        ivec = CipherVecIn (pin);
        cv ^= ivec;
        CipherVecOut (cv, pout);
    }

    *(CipherVec*)self->dad.encrypt_ivec = ivec;

    return 0;
#else
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
#endif
}


/* ----------
 * Output Feedback
 * CT = PT ^ (FB = ENC (IV, EK))
 * PT = CT ^ (FB = DEC (IV, DK))
 */
static
rc_t CMEMBER(EncryptOfb) (CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
#if 1
    CipherVec cv;
    CipherVec ivec;
    const uint8_t * pin;
    uint8_t * pout;

    ivec = *(const CipherVec*)self->dad.encrypt_ivec;

    for ((pin = in), (pout = out);
         block_count --; 
         (pin += self->dad.block_size), (pout += self->dad.block_size))
    {
        ivec = CMEMBER(EncryptV1)(self, ivec); 
        cv = CipherVecIn (pin);
        cv ^= ivec;
        CipherVecOut (cv, pout);
    }

    *(CipherVec*)self->dad.encrypt_ivec = ivec;

    return 0;
#else
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
#endif
}


static
rc_t CMEMBER(DecryptOfb) (CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
    return CMEMBER(EncryptOfb)(self, in, out, block_count);
}


/* Counter
 * IV is a nonce and not re-used as FB
 * CT = PT ^ ENC (N, EK)
 * PT = CT ^ ENC (N, DK) 
 * Note decrypt is encrypt.
 * nonce is a function that given an iv generates the next iv
 */
static
rc_t CMEMBER(EncryptCtr) (CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
}


static
rc_t CMEMBER(DecryptCtr) (CIPHER_IMPL * self, const void * in, void * out, uint32_t block_count)
{
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
}

static
struct KCipher_vt_v1 CMEMBER(_vt_v1) =
{
    { 1, 0 },

    CMEMBER(Destroy),
    CMEMBER(SetEncryptKey),
    CMEMBER(SetDecryptKey),
    CMEMBER(SetEncryptIvec),
    CMEMBER(SetDecryptIvec),
    CMEMBER(SetEncryptCounterFunc),
    CMEMBER(SetDecryptCounterFunc),
    CMEMBER(Encrypt),
    CMEMBER(Decrypt),
    CMEMBER(EncryptEcb),
    CMEMBER(DecryptEcb),
    CMEMBER(EncryptCbc),
    CMEMBER(DecryptCbc),
    CMEMBER(EncryptPcbc),
    CMEMBER(DecryptPcbc),
    CMEMBER(EncryptCfb),
    CMEMBER(DecryptCfb),
    CMEMBER(EncryptOfb),
    CMEMBER(DecryptOfb),
    CMEMBER(EncryptCtr),
    CMEMBER(DecryptCtr)
};


static __inline__
rc_t CMEMBER(AllocAes)(CIPHER_IMPL ** pobj,
                       const KBlockCipher * block_cipher)
{
    CipherAes * obj;

    obj = calloc (1, sizeof (*obj));
    *pobj = (CIPHER_IMPL*)obj;

    return (obj ? 0 : RC (rcKrypto, rcCipher, rcConstructing, rcMemory, rcExhausted));
}


static __inline__
rc_t CMEMBER(InitAes) (CIPHER_IMPL * self,
                      const KBlockCipher * block_cipher)
{
    KCipher * dad = &self->dad;
    CipherAes * aes = (struct CipherAes*)self;

    KCipherInit (dad, sizeof (CipherVec),
                 (const KCipher_vt*)&CMEMBER(_vt_v1),
                 CMEMBER(ClassName));

    dad->encrypt_key = &aes->e_key;
    dad->decrypt_key = &aes->d_key;
    dad->encrypt_ivec = &aes->e_ivec;
    dad->decrypt_ivec = &aes->d_ivec;

    self->block_cipher = &block_cipher->vec;

    return 0;
}


static __inline__
rc_t CMEMBER(AllocNull)(CIPHER_IMPL ** pobj,
                        const KBlockCipher * block_cipher)
{
    CipherNull * obj;

    obj = calloc (1, sizeof (*obj));
    *pobj = (CIPHER_IMPL*)obj;

    return (obj ? 0 : RC (rcKrypto, rcCipher, rcConstructing, rcMemory, rcExhausted));
}


static __inline__
rc_t CMEMBER(InitNull) (CIPHER_IMPL * self,
                        const KBlockCipher * block_cipher)
{
    KCipher * dad = &self->dad;
    CipherNull * null = (CipherNull*)self;

    KCipherInit (dad, sizeof (CipherVec),
                 (const KCipher_vt*)&CMEMBER(_vt_v1),
                 CMEMBER(ClassName));

    dad->encrypt_key = &null->e_ivec;
    dad->decrypt_key = &null->d_ivec;
    dad->encrypt_ivec = &null->e_ivec;
    dad->decrypt_ivec = &null->d_ivec;

    self->block_cipher = &block_cipher->vec;

    return 0;
}


rc_t CMEMBER(Make) (KCipher ** new_obj, kcipher_type type)
{
    rc_t rc;
    KBlockCipher * block_cipher;

    if (new_obj == NULL)
        return RC (rcKrypto, rcCipher, rcConstructing, rcSelf, rcNull);

    *new_obj = NULL;

    switch (type)
    {
    default:
        return RC (rcKrypto, rcCipher, rcConstructing, rcParam, rcInvalid);

    case kcipher_null:
        rc = NULLBCMEMBER(Make)(&block_cipher);
        break;

    case kcipher_AES:
        rc = AESBCMEMBER(Make)(&block_cipher);
        break;
    }

    if (rc == 0)
    {
        CIPHER_IMPL * obj;

        switch (type)
        {
        case kcipher_null:
            rc = CMEMBER(AllocNull) (&obj, block_cipher);
            if (rc == 0)
            {
                rc = CMEMBER(InitNull) (obj, block_cipher);
                if (rc == 0)
                    *new_obj = (KCipher*)obj;
                else
                    free (obj);
            }
            break;
        case kcipher_AES:
            rc = CMEMBER(AllocAes) (&obj, block_cipher);
            if (rc == 0)
            {
                rc = CMEMBER(InitAes) (obj, block_cipher);
                if (rc == 0)
                    *new_obj = (KCipher*)obj;
                else
                    free (obj);
            }
        default:
            /* can't really get here */
            break;
        }
    }
    return rc;
}


/* EOF */
