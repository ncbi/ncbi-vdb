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

#ifndef _h_krypto_cipher_
#define _h_krypto_cipher_

#include <krypto/extern.h>
#include <klib/defs.h>

typedef struct KCipher KCipher;

KRYPTO_EXTERN 
rc_t CC KCipherAddref (const KCipher * self);

KRYPTO_EXTERN rc_t CC KCipherRelease (const KCipher * self);

KRYPTO_EXTERN rc_t CC KCipherBlockSize (const KCipher * self, size_t * bytes);

KRYPTO_EXTERN rc_t CC KCipherSetEncryptKey (KCipher * self,
                                            const void * user_key,
                                            size_t user_key_size);

KRYPTO_EXTERN rc_t CC KCipherSetDecryptKey (KCipher * self,
                                            const void * user_key,
                                            size_t user_key_size);

/*
 * Set the ivec (Initialization vector or feedback) for the cipher
 * this is done automatically for the longer runs defined below.
 *
 * the size of ivec  must match KCipherBlockSize
 *
 * the ivec is copied into the cipher not used in place
 */
KRYPTO_EXTERN
 rc_t CC KCipherSetEncryptIVec (KCipher * self, const void * ivec);

KRYPTO_EXTERN
 rc_t CC KCipherSetDecryptIVec (KCipher * self, const void * ivec);


typedef void (*cipher_ctr_func)(void * ivec);

KRYPTO_EXTERN rc_t CC KCipherSetEncryptCtrFunc (KCipher * self, cipher_ctr_func func);

KRYPTO_EXTERN rc_t CC KCipherSetDecryptCtrFunc (KCipher * self, cipher_ctr_func func);

/*
 * 'in' can equal 'out'
 */
KRYPTO_EXTERN rc_t CC KCipherEncrypt (KCipher * self, const void * in, void * out);

KRYPTO_EXTERN rc_t CC KCipherDecrypt (KCipher * self, const void * in, void * out);


/* ====================
 * longer runs of multiple blocks.
 *
 * The algorithms are well defined and standard in most cases.
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
KRYPTO_EXTERN rc_t CC KCipherEncryptECB (KCipher * self, const void * in, void * out,
                                         uint32_t block_count);

KRYPTO_EXTERN rc_t CC KCipherDecryptECB (KCipher * self, const void * in, void * out,
                                         uint32_t block_count);

/* ----------
 * Cipher-Block Chaining
 * CT = (FB = ENC (PT^IV, EK))
 * PT = DEC ((FB = CT), DK)
 *
 */
KRYPTO_EXTERN rc_t CC KCipherEncryptCBC (KCipher * self, const void * in, void * out,
                                         uint32_t block_count);

KRYPTO_EXTERN rc_t CC KCipherDecryptCBC (KCipher * self, const void * in, void * out,
                                         uint32_t block_count);

/* ----------
 * Propagating cipher-block chaining
 * FB = PT ^ (CT = ENC ((PT^IV), EK))
 * FB = CT ^ (PT = DEC (CT,DK) ^ IV)
 */

/* not yet implemented */

/* ----------
 * Cipher Feedback
 * CT = (FB = PT) ^ ENC (IV, EK))
 * PT = (FB = CT) ^ ENC (IV, DK)
 *
 * NOTE the use of the encrypt function for decryption
 *
 * Not implemented as the openssl does something different
 */
KRYPTO_EXTERN
rc_t CC KCipherEncryptCFB (KCipher * self, const void * in, void * out,
                           uint32_t block_count);

KRYPTO_EXTERN
rc_t CC KCipherDecryptCFB (KCipher * self, const void * in, void * out,
                           uint32_t block_count);

KRYPTO_EXTERN
rc_t CC KCipherEncryptPCFB (KCipher * self, const void * in, void * out,
                            uint32_t block_count);

KRYPTO_EXTERN
rc_t CC KCipherDecryptPCFB (KCipher * self, const void * in, void * out,
                            uint32_t block_count);

/* ----------
 * Output Feedback
 * CT = PT ^ (FB = ENC (IV, EK))
 * PT = CT ^ (FB = ENC (IV, DK))
 *
 * NOTE the use of the encrypt function for decryption
 *
 * Not implemented as the openssl does something different
 */
KRYPTO_EXTERN
rc_t CC KCipherEncryptOFB (KCipher * self, const void * in, void * out,
                           uint32_t block_count);

KRYPTO_EXTERN
rc_t CC KCipherDecryptOFB (KCipher * self, const void * in, void * out,
                           uint32_t block_count);

/* ----------
 * Counter
 * IV is a nonce and not re-used as FB
 * CT = PT ^ ENC (N, EK)
 * PT = CT ^ ENC (N, DK)
 *
 * NOTE the use of the encrypt function for decryption
 *
 * nonce is a function that given an iv generates the next iv
 *
 */
KRYPTO_EXTERN
rc_t CC KCipherEncryptCTR (KCipher * self, const void * in,
                           void * out, uint32_t block_count);
KRYPTO_EXTERN
rc_t CC KCipherDecryptCTR (KCipher * self, const void * in,
                           void * out, uint32_t block_count);




#ifdef __cplusplus
}
#endif

#endif /* #ifndef _h_krypto_cipher_ */
