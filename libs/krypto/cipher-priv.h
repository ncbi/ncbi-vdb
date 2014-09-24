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

#ifndef _h_krypto_cipher_priv_
#define _h_krypto_cipher_priv_

#include <krypto/extern.h>
#include <krypto/ciphermgr.h>
#include <krypto/cipher.h>

#include <klib/refcount.h>

/*
 * klib/rc.h ran out of room for new targets so we'll fake it for now
 * with a now unused target that will unfortunately leave some
 * confusing rc to english translations
 */
#define rcBlockCipher rcRng
#define rcCipher      rcRng

/*
 * for use within krypto and krypto testing
 */

enum e_aes_type
{
    kcipher_AES_core,     /* Rijndael aes_core.c portable */
    kcipher_AES_ncbi_sim, /* clean AES written for NCBI specifically from the  */
    kcipher_AES_ncbi_vec, /* xmm vector version version */
    kcipher_AES_ncbi_ni,  /* 64 bit AES-NI version */
    kcipher_AES_count
};

rc_t KCipherMake         (struct KCipher ** new_cipher,
                          kcipher_type type);
rc_t KCipherVecAesNiMake (struct KCipher ** new_cipher,
                          kcipher_type type);
rc_t KCipherVecRegMake   (struct KCipher ** new_cipher,
                          kcipher_type type);
rc_t KCipherVecMake      (struct KCipher ** new_cipher,
                          kcipher_type type);
rc_t KCipherByteMake     (struct KCipher ** new_cipher,
                          kcipher_type type);

/*
 * not for use outside krypto
 */
rc_t KCipherDestroy (struct KCipher * self);



#ifdef __cplusplus
}
#endif


#endif /* #ifndef _h_krypto_cipher_priv_ */
