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

#include "ncbi-priv.h"
#include "cipher-priv.h"
#include "blockcipher-priv.h"

#include <klib/rc.h>


#include <byteswap.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sysalloc.h>

#include "blockcipher-impl.h"

#if USE_VEC
#include <cpuid.h>
#endif
#if USE_VECREG
#include <v128.h>
#endif


#if USE_VEC
#define BLOCKCIPHER_IMPL BLOCKCIPHER_VEC_IMPL
#else
#define BLOCKCIPHER_IMPL BLOCKCIPHER_BYTE_IMPL
#endif

typedef struct NULL_CLASS NULL_CLASS;

static const char NULLBCMEMBER(aes_ncbi_name) [] = CLASS_STRING(AES_CLASS);


/* ----------------------------------------------------------------------
 * Destroy
 *   How large is the stored key for this cipher?  Not the user key used
 *   to create this key (key schedule)
 *
 *   This is needed by KCipher to know how large the KCipher objecr is at
 *   allocation and to know how much of a buffer each decryption/encryption is
 */
static
void NULLBCMEMBER(Destroy)    (const BLOCKCIPHER_IMPL * self)
{
    /* we use a constant object so do nothing */
}


/* ----------------------------------------------------------------------
 * BlockSize
 */
static
uint32_t NULLBCMEMBER(BlockSize) ()
{
    return 16;
}


/* ----------------------------------------------------------------------
 * KeySize
 *   How large is the stored key for this cipher?  Not the user key used
 *   to create this key (key schedule)
 *
 *   This is needed by KCipher to know how large the KCipher objecr is at
 *   allocation and to know how much of a buffer each decryption/encryption is
 */
static uint32_t NULLBCMEMBER(KeySize) ()
{
    return 0;
}


/* ----------------------------------------------------------------------
 * SetEncryptKey
 *   The KCipher calls this to have the block cipher build an encryption
 *   key in the KCipher object
 *
 */
static rc_t NULLBCMEMBER(SetEncryptKey) (void * encrypt_key, const char * user_key,
                                         uint32_t user_key_size)
{
    return 0;
}


/* ----------------------------------------------------------------------
 * SetDecryptKey
 *   The KCipher calls this to have the block cipher build an decryption
 *   key in the KCipher object
 *
 */
static rc_t NULLBCMEMBER(SetDecryptKey) (void * decrypt_key,
                                     const char * user_key,
                                     uint32_t user_key_size)
{
    return 0;
}


/* ----------------------------------------------------------------------
 * Encrypt
 *
 *   Perform an encryption of a single block.  Chained modes and stream
 *   cipher modes will call this multiple times.
 *
 */
#if USE_VEC
static CipherVec NULLBCMEMBER(Encrypt) (CipherVec state,
                                        const void * encrypt_key)
{
    return state;
}
#else
static void NULLBCMEMBER(Encrypt) (const CipherBlock in,
                                   CipherBlock * out,
                                   const void * encrypt_key)
{
}
#endif


/* ----------------------------------------------------------------------
 * Decrypt
 *
 *   Perform a decryption of a single block.  Chained modes and stream
 *   cipher modes will call this multiple times.
 */
#if USE_VEC
static CipherVec NULLBCMEMBER(Decrypt) (CipherVec state,
                                        const void * decrypt_key)
{
    return state;
}
#else
static void NULLBCMEMBER(Decrypt) (const CipherBlock in,
                                   CipherBlock * out,
                                   const void * encrypt_key)
{
}
#endif


/* ----------------------------------------------------------------------
 * MakeProcessorSupport
 *
 * Run-time check for support of this particular AES implementation by
 * checking the flags given via the cpuid instruction.
 */
static __inline__
bool NULLBCMEMBER(ProcessorSupport) ()
{
    return true;
}


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
 * name      ASCIZ c-string the name of this class
 */
static const
#if USE_VEC
KBlockCipherVec_vt_v1
#else
KBlockCipherByte_vt_v1
#endif
NULLBCMEMBER(_vt_) = {
    { 1, 1 },

    NULLBCMEMBER(Destroy),
    NULLBCMEMBER(BlockSize),
    NULLBCMEMBER(KeySize),
    NULLBCMEMBER(SetEncryptKey),
    NULLBCMEMBER(SetDecryptKey),
    NULLBCMEMBER(Encrypt),
    NULLBCMEMBER(Decrypt)
};


/* ----------------------------------------------------------------------
 * Make
 *
 * Create a new Null Block Cipher object.
 * The processor is checked to see if this particular version is supported on
 * this particular CPU.
 */
rc_t NULLBCMEMBER(Make) (KBlockCipher ** new_obj)
{
    rc_t rc;

    /* Check parameter first */
    if (new_obj == NULL)
        rc = RC (rcKrypto, rcBlockCipher, rcConstructing, rcSelf, rcNull);

    else
    {
        *new_obj = NULL;

        /* check for processor support of this flavor */
        if ( ! NULLBCMEMBER(ProcessorSupport)())
            rc = RC (rcKrypto, rcBlockCipher, rcConstructing, rcFunction, rcUnsupported);

        else
        {
            rc = 0;
            *new_obj = (KBlockCipher*)&(NULLBCMEMBER(_vt_));
        }
    }
    return rc;
}

/* end of file lib/krypto/aes-ncbi.c */
