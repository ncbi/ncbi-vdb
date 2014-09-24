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

#include <krypto/cipher.h>
#include <krypto/ciphermgr.h>
#include <krypto/testciphermgr.h>
#include "cipher-priv.h"
#include "cipher-impl.h"

#include <klib/rc.h>


static const char KCipherClassName[] = "KCipherClassName";

/* ----------------------------------------------------------------------
 * Init
 *
 *   Initialize the fields of this object.  The derived class will call this
 *   during it's initialization.
 *
 * self      object to initialze
 * vt        the virtual table of the derived class
 * mgr       the cipher manager that is the construction factory block cipher
 *           objects hold references to the manager while the manager merely
 *           points at the block ciphers when all block ciphers are destroyed
 *           the manager loses its references and it too can be destroyed if not
 *           held elsewhere
 * name      ASCIZ c-string the name of this class; pointer is expected to remain valid
 *           for the life of the object
 */
void KCipherInit (KCipher * self,
                  uint32_t block_size,
                  const KCipher_vt * vt,
                  const char * name)
{
    KRefcountInit (&self->refcount, 1, KCipherClassName, "init", name);
    self->block_size = block_size;
    self->vt.version = vt;
    self->name = name;

    /* the remaining fields have to be initialized in the derived object init */
}


/* ----------------------------------------------------------------------
 * Destroy
 *   base class destruction called during the derived class destruction
 *
 * self is the cipher object
 *
 * memory release is the repsonsibility of the derived class destructor
 */
rc_t KCipherDestroy (KCipher * self)
{
    if (self)
    {
        KRefcountWhack (&self->refcount, self->name);
    }
    return 0;
}

/* ----------------------------------------------------------------------
 * AddRef
 *   add a new owner to this class.  This will mean another instance of 
 *   KCipher used this Block Cipher
 */
KRYPTO_EXTERN
rc_t CC KCipherAddRef (const KCipher * self)
{
    if (self)
    {
        switch (KRefcountAdd (&self->refcount, KCipherClassName))
        {
        case krefLimit:
            return RC (rcKrypto, rcCipher, rcAttaching, rcRange, rcExcessive);
        }
    }
    return 0;
}


/* ----------------------------------------------------------------------
 * Release
 *   
 */
static __inline__ rc_t KCipherWhack (KCipher * self)
{
    switch (self->vt.version->maj)
    {
    case 1:
        return self->vt.v1->destroy (self);
    }
    return RC (rcKrypto, rcCipher, rcDestroying, rcInterface, rcBadVersion);
}


KRYPTO_EXTERN
rc_t CC KCipherRelease (const KCipher * self)
{
    if ( self != NULL )
    {
        switch (KRefcountDrop ( & self -> refcount, KCipherClassName))
        {
        case krefWhack:
            return KCipherWhack ((KCipher *)self);

        case krefNegative:
            return RC ( rcKrypto, rcCipher, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* ----------------------------------------------------------------------
 * Some Cipher Methods are really pass through methods to the block cipher
 *   
 */
KRYPTO_EXTERN rc_t CC KCipherBlockSize (const KCipher * self, size_t * bytes)
{
    if (self == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcSelf, rcNull);

    if (bytes == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcParam, rcNull);

    *bytes = self->block_size;

    return 0;
}


/* ----------------------------------------------------------------------
 * 
 *   
 */
KRYPTO_EXTERN rc_t CC KCipherSetEncryptKey (KCipher * self, const void * user_key, size_t user_key_size)
{
    if (self == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcSelf, rcNull);

    if ((user_key == NULL)||(user_key_size == 0))
        return RC (rcKrypto, rcCipher, rcAccessing, rcParam, rcNull);

    switch (self->vt.version->maj)
    {
    case 1:
        return self->vt.v1->set_encrypt_key (self, user_key, (uint32_t) user_key_size);
    }
    return RC (rcKrypto, rcCipher, rcUpdating, rcInterface, rcBadVersion);
}


/* ----------------------------------------------------------------------
 * 
 *   
 */
KRYPTO_EXTERN rc_t CC KCipherSetDecryptKey (KCipher * self, const void * user_key, size_t user_key_size)
{
    if (self == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcSelf, rcNull);

    if ((user_key == NULL)||(user_key_size == 0))
        return RC (rcKrypto, rcCipher, rcAccessing, rcParam, rcNull);

    switch (self->vt.version->maj)
    {
    case 1:
        return self->vt.v1->set_decrypt_key (self, user_key, (uint32_t) user_key_size);
    }
    return RC (rcKrypto, rcCipher, rcUpdating, rcInterface, rcBadVersion);
}



/* ----------------------------------------------------------------------
 *  
 *
 * Set the ivec (Initialization vector or feedback) for the cipher
 *
 * the size of ivec  must match KCipherBlockSize
 *
 * the ivec is copied into the cipher not used in place
 */
KRYPTO_EXTERN rc_t CC KCipherSetEncryptIVec (KCipher * self, const void * ivec)
{
    if (self == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcSelf, rcNull);

    if (ivec == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcParam, rcNull);

    switch (self->vt.version->maj)
    {
    case 1:
        return self->vt.v1->set_encrypt_ivec (self, ivec);
    }
    return RC (rcKrypto, rcCipher, rcUpdating, rcInterface, rcBadVersion);
}


/* ----------------------------------------------------------------------
 * 
 *   
 */
KRYPTO_EXTERN rc_t CC KCipherSetDecryptIVec (KCipher * self, const void * ivec)
{
    if (self == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcSelf, rcNull);

    if (ivec == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcParam, rcNull);

    switch (self->vt.version->maj)
    {
    case 1:
        return self->vt.v1->set_decrypt_ivec (self, ivec);
    }
    return RC (rcKrypto, rcCipher, rcUpdating, rcInterface, rcBadVersion);
}


/* ----------------------------------------------------------------------
 * 
 *   
 */
KRYPTO_EXTERN rc_t CC KCipherSetEncryptCtrFunc (KCipher * self, cipher_ctr_func func)
{
    if (self == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcSelf, rcNull);

    if (func == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcParam, rcNull);

    switch (self->vt.version->maj)
    {
    case 1:
        return self->vt.v1->set_encrypt_ctr_func (self, func);
    }
    return RC (rcKrypto, rcCipher, rcUpdating, rcInterface, rcBadVersion);
}


/* ----------------------------------------------------------------------
 * 
 *   
 */
KRYPTO_EXTERN rc_t CC KCipherSetDecryptCtrFunc (KCipher * self, cipher_ctr_func func)
{
    if (self == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcSelf, rcNull);

    if (func == NULL)
        return RC (rcKrypto, rcCipher, rcAccessing, rcParam, rcNull);

    switch (self->vt.version->maj)
    {
    case 1:
        return self->vt.v1->set_decrypt_ctr_func (self, func);
    }
    return RC (rcKrypto, rcCipher, rcUpdating, rcInterface, rcBadVersion);
}


KRYPTO_EXTERN rc_t CC KCipherEncrypt (KCipher * self, const void * in, void * out)
{
    if (self == NULL)
        return RC (rcKrypto, rcCipher, rcEncoding, rcSelf, rcNull);

    if ((in == NULL) || (out == NULL))
        return RC (rcKrypto, rcCipher, rcEncoding, rcParam, rcNull);

    switch (self->vt.version->maj)
    {
    case 1:
        return self->vt.v1->encrypt (self, in, out);
    }
    return RC (rcKrypto, rcCipher, rcAccessing, rcInterface, rcBadVersion);
}


KRYPTO_EXTERN rc_t CC KCipherDecrypt (KCipher * self, const void * in, void * out)
{
    if (self == NULL)
        return RC (rcKrypto, rcCipher, rcEncoding, rcSelf, rcNull);

    if ((in == NULL) || (out == NULL))
        return RC (rcKrypto, rcCipher, rcEncoding, rcParam, rcNull);

    switch (self->vt.version->maj)
    {
    case 1:
        return self->vt.v1->decrypt (self, in, out);
    }
    return RC (rcKrypto, rcCipher, rcAccessing, rcInterface, rcBadVersion);
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
 *
 * NOTE: currently an implementation detail limits us to 8192 bit cipher block
 * size.  Changing MAX_BLOCK_SIZE in cipher.c can up that limit without 
 * causing any other compatibility issues. 
 *
 * Two local byte arrays are defined on the stack of 1024 bytes or 8192 bits.
 */
/*
 * NOTE: if in and out overlap incorrectly this will fail
 */
#define BLOCK_FUNC_PASTE(A,B) A##B
#define BLOCK_FUNC(MODE,METHOD) \
    KRYPTO_EXTERN rc_t CC BLOCK_FUNC_PASTE(KCipher,MODE)                \
        (KCipher * self, const void * in, void * out,                   \
         uint32_t block_count)                                          \
    {                                                                   \
        rc_t rc;                                                        \
                                                                        \
        if (self == NULL)                                               \
            rc = RC (rcKrypto, rcCipher, rcEncoding, rcSelf, rcNull);   \
                                                                        \
        else if (in == NULL)                                            \
            rc = RC (rcKrypto, rcCipher, rcEncoding, rcParam, rcNull);  \
                                                                        \
        else if (out == NULL)                                           \
            rc = RC (rcKrypto, rcCipher, rcEncoding, rcParam, rcNull);  \
                                                                        \
        else if (block_count == 0)                                      \
            rc = 0;                                                     \
                                                                        \
        else                                                            \
        {                                                               \
            switch (self->vt.version->maj)                              \
            {                                                           \
            case 1:                                                     \
                return self->vt.v1->METHOD(self, in, out, block_count); \
            }                                                           \
            rc = RC (rcKrypto, rcCipher, rcAccessing, rcInterface,      \
                     rcBadVersion);                                     \
        }                                                               \
                                                                        \
        return rc;                                                      \
    }

/* ----------
 * Electronic Code Book - simple cipher with no chaining feedback  just iterate
 * simple encrypt/decrypt with the plain, text, cipher text and key/
 *
 * CT = ENC (PT,EK)
 * PT = DEC (CT,DK)
 */
BLOCK_FUNC(EncryptECB,encrypt_ecb)
BLOCK_FUNC(DecryptECB,decrypt_ecb)

/* ----------
 * Cipher-Block Chaining
 * CT = (FB = ENC (PT^IV, EK))
 * PT = DEC ((FB = CT), DK)
 */
BLOCK_FUNC(EncryptCBC,encrypt_cbc)
BLOCK_FUNC(DecryptCBC,decrypt_cbc)

/* ----------
 * Propagating cipher-block chaining
 * FB = PT ^ (CT = ENC ((PT^IV), EK))
 * FB = CT ^ (PT = DEC (CT,DK) ^ IV)
 */
BLOCK_FUNC(EncryptPCBC,encrypt_pcbc)
BLOCK_FUNC(DecryptPCBC,decrypt_pcbc)

/* ----------
 * Cipher Feedback
 * CT = (FB = PT) ^ ENC (IV, EK))
 * PT = (FB = CT) ^ DEC (IV, DK)
 */
BLOCK_FUNC(EncryptCFB,encrypt_cfb)
BLOCK_FUNC(DecryptCFB,decrypt_cfb)

/* ----------
 * Output Feedback
 * CT = PT ^ (FB = ENC (IV, EK))
 * PT = CT ^ (FB = DEC (IV, DK))
 */
BLOCK_FUNC(EncryptOFB,encrypt_ofb)
BLOCK_FUNC(DecryptOFB,decrypt_ofb)

/* Counter
 * IV is a nonce and not re-used as FB
 * CT = PT ^ ENC (N, EK)
 * PT = CT ^ ENC (N, DK) 
 *
 * The enc_ctr_func (or dec_ctr_func) will be called to update the ivec before
 * to be the nonce (number used once) before each call to the block cipher 
 * encrypt function.  The encrypt function is used for decryption mode as well
 */
BLOCK_FUNC(EncryptCTR,encrypt_ctr)
BLOCK_FUNC(DecryptCTR,decrypt_ctr)


#ifndef USE_SLOW_ONES
#define USE_SLOW_ONES _DEBUGGING
#endif

#include <stdio.h>
rc_t KCipherMakeInt (KCipher ** new_cipher, kcipher_type type)
{
    rc_t rc;

    *new_cipher = NULL;

#ifndef WIN32
    rc = KCipherVecAesNiMake (new_cipher, type);
    /*printf("KCipherVecAesNiMake %s (%u) \n", rc ? "FAILED" : "succeeded", rc);*/
    if (rc)
    {
        if (GetRCState(rc) == rcUnsupported)
        {
#if USE_SLOW_ONES
            rc = KCipherVecRegMake (new_cipher, type);
            /*printf("KCipherVecRegMake %s (%u) \n", rc ? "FAILED" : "succeeded", rc);*/
            if (rc)
            {
                if (GetRCState(rc) == rcUnsupported)
                {
                    rc = KCipherVecMake (new_cipher, type);
                    /*printf("KCipherVecMake %s (%u) \n", rc ? "FAILED" : "succeeded", rc);*/
                    if (GetRCState(rc) == rcUnsupported)
                    {
#endif
#endif
                        rc = KCipherByteMake (new_cipher, type);
                        /*printf("KCipherByteMake %s (%u) \n", rc ? "FAILED" : "succeeded", rc);*/
#ifndef WIN32
#if USE_SLOW_ONES
                    }
                }
            }
#endif
        }
    }
#endif
    return rc;
}

kcipher_subtype KCipherSubType = ksubcipher_none;

rc_t KCipherMake (KCipher ** new_cipher, kcipher_type type)
{
    rc_t rc;
    if (new_cipher == NULL)
        rc = RC (rcKrypto, rcCipher, rcConstructing, rcSelf, rcNull);

    else
    {
        switch (type)
        {
        case kcipher_null:
        case kcipher_AES:
            switch (KCipherSubType)
            {
            case ksubcipher_byte:
                rc = KCipherByteMake (new_cipher, type);
                break;
#ifdef USEVEC
            case ksubcipher_vec:
                rc = KCipherVecMake (new_cipher, type);
                break;
#endif
#ifdef USEVECREG
            case ksubcipher_vecreg:
                rc = KCipherVecRegMake (new_cipher, type);
                break;
#endif
#ifdef USEAESNI
            case ksubcipher_accelerated:
                rc = KCipherVecAesNiMake (new_cipher, type);
                break;
#endif
            default:
                rc = KCipherMakeInt (new_cipher, type);
                break;
            }
            break;
        default:
            rc = RC (rcKrypto, rcCipher, rcConstructing, rcBlockCipher, rcInvalid);
            break;
        }
    }
    return rc;
}






/* EOF */
