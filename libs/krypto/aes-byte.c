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

#include "aes-priv.h"
#include "cipher-priv.h"

#include <klib/rc.h>

#include <string.h>
#include <assert.h>


/* instantiate for class functions that use the VT */
#define BLOCKCIPHER_BYTE_IMPL KAESBlockCipherByte
#include "blockcipher-impl.h"

static const char KAESByteBlockCipherClassName[] = "KAESCipherByte";

static
void KAESBlockCipherByteDestroy (const KAESBlockCipherByte * self)
{
    /* we use a constant object */
}


static
uint32_t KAESBlockCipherByteBlockSize ()
{
    return AES_BLOCK_SIZE;
}


/* ----------------------------------------------------------------------
 * KeySize
 *   How large is the stored key for this cipher?  Not the user key used
 *   to create this key (key schedule)
 *
 *   This is needed by KCipher to know how large the KCipher objecr is at
 *   allocation and to know how much of a buffer each decryption/encryption is
 */
static
uint32_t KAESBlockCipherByteKeySize ()
{
    return  (uint32_t)sizeof (AES_KEY);
}


/* ----------------------------------------------------------------------
 * SetEncryptKey
 *   The KCipher calls this to have the block cipher build an encryption
 *   key in the KCipher object
 *
 */
static
rc_t KAESBlockCipherByteSetEncryptKey (void * encrypt_key,
                                       const char * user_key,
                                       uint32_t user_key_size)
{
    int iii;
    rc_t rc;

    assert (encrypt_key);
    assert (user_key);
    assert (user_key_size != 0);

    iii = AES_set_encrypt_key((const unsigned char *)user_key,
                              (int)user_key_size * 8,
                              encrypt_key);
    if (iii == 0)
        rc = 0;
    else
    {
        switch (iii)
        {
        default: /* not in the code when this was written */
            rc = RC (rcKrypto, rcCipher, rcUpdating, rcEncryptionKey, rcUnknown);
            break;

        case -1: /* bad parameters */
            rc = RC (rcKrypto, rcCipher, rcUpdating, rcParam, rcInvalid);
            break;

        case -2: /* bad bit count */
            rc = RC (rcKrypto, rcCipher, rcUpdating, rcParam, rcIncorrect);
            break;
        }
        memset (encrypt_key, 0, sizeof (AES_KEY));
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * SetDecryptKey
 *   The KCipher calls this to have the block cipher build an decryption
 *   key in the KCipher object
 *
 */
static
rc_t KAESBlockCipherByteSetDecryptKey (void * decrypt_key,
                                       const char * user_key,
                                       uint32_t user_key_size)
{
    int iii;
    rc_t rc;

    assert (decrypt_key);
    assert (user_key);
    assert (user_key_size != 0);

    iii = AES_set_decrypt_key((const unsigned char *)user_key, (int)user_key_size * 8,
                              decrypt_key);
    if (iii == 0)
        rc = 0;
    else
    {
        switch (iii)
        {
        default: /* not in the code when this was written */
            rc = RC (rcKrypto, rcCipher, rcUpdating, rcEncryptionKey, rcUnknown);
            break;

        case -1: /* bad parameters */
            rc = RC (rcKrypto, rcCipher, rcUpdating, rcParam, rcInvalid);
            break;

        case -2: /* bad bit count */
            rc = RC (rcKrypto, rcCipher, rcUpdating, rcParam, rcIncorrect);
            break;
        }
        memset (decrypt_key, 0, sizeof (AES_KEY));
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * Encrypt
 *
 *   Perform an encryption of a single block.  Chained modes and stream
 *   cipher modes will call this multiple times.
 *
 */
static
void KAESBlockCipherByteEncrypt (const CipherBlock in,
                                 CipherBlock out,
                                 const void * key)
{
    assert (in);
    assert (out);
    assert (key);

    AES_encrypt ((const uint8_t*)in, (uint8_t*)out, key);
}


/* ----------------------------------------------------------------------
 * Decrypt
 *
 *   Perform a decryption of a single block.  Chained modes and stream
 *   cipher modes will call this multiple times.
 *
 */
static
void KAESBlockCipherByteDecrypt (const CipherBlock in,
                                 CipherBlock out,
                                 const void * key)
{
    assert (in);
    assert (out);
    assert (key);

    AES_decrypt ((const uint8_t*)in, (uint8_t*)out, key);
}


static const struct
KBlockCipherByte_vt_v1 aes_vt = 
{
    /* .version */
    { 1, 0 },

    KAESBlockCipherByteDestroy,

    KAESBlockCipherByteBlockSize,
    KAESBlockCipherByteKeySize,

    KAESBlockCipherByteSetEncryptKey,
    KAESBlockCipherByteSetDecryptKey,

    KAESBlockCipherByteEncrypt,
    KAESBlockCipherByteDecrypt
};


rc_t KAESBlockCipherByteMake (KBlockCipherByte ** pobj)
{
    rc_t rc;

    if (pobj == NULL)
        rc = RC (rcKrypto, rcBlockCipher, rcConstructing, rcSelf, rcNull);

    else
    {
        rc = 0;
        *pobj = (KBlockCipherByte*)&aes_vt;
    }
    return rc;
}

/* EOF */

