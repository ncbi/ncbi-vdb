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

/*
 * KCipherByte is a byte oriented KCipher that can work on all processors
 * and be built with all compilers
 */
#include <krypto/extern.h>
#include <klib/defs.h>
#include <krypto/cipher.h>

#include "ncbi-priv.h"

#include "blockcipher-impl.h"

#include "cipher-impl.h"
#include "cipher-priv.h"
#include "blockcipher-priv.h"

#include <klib/rc.h>

#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


const char MEMBER(ClassName)[] = CLASS_STRING(CIPHER_IMPL);

/* 
 * KCipher with unaligned data.  Byte alignment.
 */
struct KCipherByte
{
    KCipher dad;

    const KBlockCipherByte * block_cipher;
};


static rc_t MEMBER(Destroy) (KCipherByte * self)
{
    rc_t rc = 0;

    if (self)
    {
        rc =  KCipherDestroy (&self->dad);
        free (self);
    }
    return rc;
}


static rc_t MEMBER(SetEncryptKey) (KCipherByte * self,
                                   const char * user_key,
                                   uint32_t user_key_bits)
{
    rc_t rc = 0;

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcUpdating, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        rc = self->block_cipher->v1.set_encrypt_key (self->dad.encrypt_key, user_key,
                                                     user_key_bits);
        break;
    }
    return rc;
}


static rc_t MEMBER(SetDecryptKey) (KCipherByte * self,
                                   const char * user_key,
                                   uint32_t user_key_bits)
{
    rc_t rc = 0;

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcDestroying, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        rc = self->block_cipher->v1.set_decrypt_key (self->dad.decrypt_key, user_key,
                                                     user_key_bits);
        break;
    }
    return rc;
}


static rc_t MEMBER(SetEncryptIvec)(KCipherByte * self,
                                   const void * ivec)
{
    assert (self);
    assert (ivec);

    memmove (self->dad.encrypt_ivec, ivec, self->dad.block_size);

    return 0;
}


static rc_t MEMBER(SetDecryptIvec)(KCipherByte * self,
                                   const void * ivec)
{
    assert (self);
    assert (ivec);

    memmove (self->dad.decrypt_ivec, ivec, self->dad.block_size);

    return 0;
}


static rc_t MEMBER(SetEncryptCounterFunc)(KCipherByte * self,
                                          cipher_ctr_func func)
{
    assert (self);
    assert (func);

    self->dad.encrypt_counter_func = func;

    return 0;
}


static rc_t MEMBER(SetDecryptCounterFunc)(KCipherByte * self,
                                          cipher_ctr_func func)
{
    assert (self);
    assert (func);

    self->dad.decrypt_counter_func = func;

    return 0;
}


static rc_t MEMBER(Encrypt)(const KCipherByte * self,
                            const void * in, void * out)
{
    rc_t rc = 0;

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcEncoding, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        self->block_cipher->v1.encrypt (in, out, self->dad.encrypt_key);
        break;
    }
    return rc;
}


static rc_t MEMBER(Decrypt)(const KCipherByte * self,
                            const void * in, void * out)
{
    rc_t rc = 0;

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcEncoding, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        self->block_cipher->v1.decrypt (in, out, self->dad.decrypt_key);
        break;
    }
    return rc;
}


static rc_t MEMBER(EncryptEcb)(const KCipherByte * self,
                               const void * in, void * out,
                               uint32_t block_count)
{
    rc_t rc = 0;
    const char * pin = in;
    char * pout = out;

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcEncoding, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        for (; block_count; --block_count)
        {
            self->block_cipher->v1.encrypt (pin, pout, self->dad.encrypt_key);

            pin += self->dad.block_size;
            pout += self->dad.block_size;
        }
        break;
    }
    return rc;
}


static rc_t MEMBER(DecryptEcb)(const KCipherByte * self,
                               const void * in, void * out,
                               uint32_t block_count)
{
    rc_t rc = 0;
    const char * pin = in;
    char * pout = out;

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcEncoding, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        for (; block_count; --block_count)
        {
            self->block_cipher->v1.decrypt (pin, pout, self->dad.decrypt_key);

            pin += self->dad.block_size;
            pout += self->dad.block_size;
        }
        break;
    }
    return rc;
}


static rc_t MEMBER(EncryptCbc)(KCipherByte * self,
                               const void * in, void * out,
                               uint32_t block_count)
{
    rc_t rc = 0;
    const char * pin = in;
    char * pout = out;

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcEncoding, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        if (block_count)
        {
            CipherBlock block;
            unsigned ix;

            memmove (block, self->dad.encrypt_ivec, self->dad.block_size);

            do
            {
                for (ix = 0; ix < self->dad.block_size; ++ix)
                    block[ix] ^= pin[ix];

                self->block_cipher->v1.encrypt (block, block,
                                                self->dad.encrypt_key);

                memmove (pout, block, self->dad.block_size);

                pin += self->dad.block_size;
                pout += self->dad.block_size;

            } while (--block_count);

            memmove (self->dad.encrypt_ivec, block, self->dad.block_size);
        }
        break;
    }
    return rc;
}


static rc_t MEMBER(DecryptCbc)(KCipherByte * self,
                               const void * in, void * out,
                               uint32_t block_count)
{
    rc_t rc = 0;
    const char * pin = in;
    char * pout = out;

    switch (self->block_cipher->version.maj)
    {
    default:
        rc = RC (rcKrypto, rcCipher, rcEncoding, rcBlockCipher, rcBadVersion);
        break;

    case 1:
        if (block_count)
        {
            CipherBlock ivec;
            CipherBlock block;
            unsigned ix;

            memmove (ivec, self->dad.decrypt_ivec, self->dad.block_size);

            do
            {
                self->block_cipher->v1.decrypt (pin, block,
                                                self->dad.decrypt_key);

                for (ix = 0; ix < self->dad.block_size; ++ix)
                    pout[ix] = block[ix] ^ ivec[ix];

                memmove (ivec, pin, self->dad.block_size); 

                pin += self->dad.block_size;
                pout += self->dad.block_size;

            } while (--block_count);

            memmove (self->dad.decrypt_ivec, ivec, self->dad.block_size);
        }
        break;
    }
    return rc;
}


static rc_t MEMBER(EncryptPcbc)(KCipherByte * self,
                                const void * in, void * out,
                                uint32_t block_count)
{
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
}


static rc_t MEMBER(DecryptPcbc)(KCipherByte * self,
                                const void * in, void * out,
                                uint32_t block_count)
{
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
}


static rc_t MEMBER(EncryptCfb)(KCipherByte * self,
                               const void * in, void * out,
                               uint32_t block_count)
{
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
}


static rc_t MEMBER(DecryptCfb)(KCipherByte * self,
                               const void * in, void * out,
                               uint32_t block_count)
{
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
}


static rc_t MEMBER(EncryptOfb)(KCipherByte * self,
                               const void * in, void * out,
                               uint32_t block_count)
{
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
}


static rc_t MEMBER(DecryptOfb)(KCipherByte * self,
                               const void * in, void * out,
                               uint32_t block_count)
{
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
}


static rc_t MEMBER(EncryptCtr)(KCipherByte * self,
                               const void * in, void * out,
                               uint32_t block_count)
{
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
}


static rc_t MEMBER(DecryptCtr)(KCipherByte * self,
                               const void * in, void * out,
                               uint32_t block_count)
{
    return RC (rcKrypto, rcCipher, rcEncoding, rcFunction, rcUnsupported);
}


static struct KCipher_vt_v1 MEMBER(_vt_v1) =
{
    { 1, 0 },

    MEMBER(Destroy),
    MEMBER(SetEncryptKey),
    MEMBER(SetDecryptKey),
    MEMBER(SetEncryptIvec),
    MEMBER(SetDecryptIvec),
    MEMBER(SetEncryptCounterFunc),
    MEMBER(SetDecryptCounterFunc),
    MEMBER(Encrypt),
    MEMBER(Decrypt),
    MEMBER(EncryptEcb),
    MEMBER(DecryptEcb),
    MEMBER(EncryptCbc),
    MEMBER(DecryptCbc),
    MEMBER(EncryptPcbc),
    MEMBER(DecryptPcbc),
    MEMBER(EncryptCfb),
    MEMBER(DecryptCfb),
    MEMBER(EncryptOfb),
    MEMBER(DecryptOfb),
    MEMBER(EncryptCtr),
    MEMBER(DecryptCtr)
};


static __inline__ rc_t MEMBER(Alloc)(KCipherByte ** pobj,
                                     const KBlockCipher * block_cipher)
{
    size_t block_size = block_cipher->byte.v1.block_size();
    size_t key_size = block_cipher->byte.v1.key_size();
    rc_t rc = 0;

    *pobj = calloc (1, sizeof (**pobj) + 2 * key_size + 2 * block_size);
    if (*pobj == NULL)
        rc = RC (rcKrypto, rcCipher, rcConstructing, rcMemory, rcExhausted);

    return rc;
}


static __inline__ rc_t MEMBER(Init)(KCipherByte * self,
                                    const KBlockCipher * block_cipher)
{
    KCipher * dad = &self->dad;
    size_t block_size = block_cipher->byte.v1.block_size();
    size_t key_size = block_cipher->byte.v1.key_size();

    KCipherInit (dad, (uint32_t) block_size,
                 (const KCipher_vt*)&MEMBER(_vt_v1),
                 MEMBER(ClassName));

    dad->encrypt_key = (uint8_t *)(self + 1);
    dad->decrypt_key = ((uint8_t *)dad->encrypt_key) + key_size;
    dad->encrypt_ivec = ((uint8_t *)dad->decrypt_key) + key_size;
    dad->decrypt_ivec = ((uint8_t *)dad->encrypt_ivec) + block_size;

    self->block_cipher = &block_cipher->byte;

    return 0;
}


rc_t MEMBER(Make) (KCipher ** new_obj, kcipher_type type)
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
        KCipherByte * obj;

        rc = MEMBER(Alloc)(&obj, block_cipher);
        if (rc == 0)
        {
            rc = MEMBER(Init)(obj, block_cipher);
            if (rc == 0)
                *new_obj = &obj->dad;
            else
                free (obj);
        }
    }
    return rc;
}

/* EOF */
