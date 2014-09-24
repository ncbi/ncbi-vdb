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

#ifndef _h_krypto_aes_ncbi_priv_libs_
#define _h_krypto_aes_ncbi_priv_libs_

#include <klib/defs.h>
#include <v128.h>

#include "ncbi-priv.h"
#include "cipher-impl.h"
#include "blockcipher-impl.h"

#ifdef  __cplusplus
extern "C" {
#endif


/* ======================================================================
 * FIPS-197 2.1
 */
#define AES_BLOCK_BITS (128)
#define AES_BLOCK_BYTES (AES_BLOCK_BITS/8)


/* ======================================================================
 * FIPS-197 3,4 - 3.5
 *
 * FIPS-197 defines Nk as number of 32-bit words in the user key
 */
#define AES_Nk_128 (4)
#define AES_Nk_192 (6)
#define AES_Nk_256 (8)


/*
 * FIPS-197 defines Nb as number of 32-bit words in the cipher block
 *
 * There is much odd naming in the document from the perspective of a software
 * engineer.
 */
#define AES_Nb (4)
#define AES_Nb_128 (AES_Nb)
#define AES_Nb_192 (AES_Nb)
#define AES_Nb_256 (AES_Nb)


/* 
 * FIPS-197 defines Nr as the number of rounds for a cipher of a given original 
 * user key length.
 *
 * Another odd naming as it is really the zero based highest round offset
 * as 1 first round + Nr-1 middle rounds + 1 last round are performed for
 * each encrypt or decrypt block.  There are thus Nr + 1 round keys needed
 * in the encryption and decryption key schedules
 */
#define AES_Nr_128 (10)
#define AES_Nr_192 (12)
#define AES_Nr_256 (14)
#define AES_Nr_MAX (15)

/* Most useful for testingg against Appendix A-C of the FIPS-197 document */
#define DEBUG_OBJECT(msg) DBGMSG(DBG_AES,DBG_FLAG(DBG_AES_OBJECT), msg)
#define DEBUG_KEYEXP(msg) DBGMSG(DBG_AES,DBG_FLAG(DBG_AES_KEYEXP), msg)
#define DEBUG_CIPHER(msg) DBGMSG(DBG_AES,DBG_FLAG(DBG_AES_CIPHER), msg)
#define DEBUG_INVKEYEXP(msg) DBGMSG(DBG_AES,DBG_FLAG(DBG_AES_INVKEYEXP), msg)
#define DEBUG_INVCIPHER(msg) DBGMSG(DBG_AES,DBG_FLAG(DBG_AES_INVCIPHER), msg)

#define DEBUG_CIPHER_MVECTOR(M,v)                                   \
    DBGMSG(DBG_AES,DBG_FLAG(DBG_AES_CIPHER),                        \
           ("%s:\t%0.8x %0.8x %0.8x %0.8x\n",M,                     \
            v.columns[0],v.columns[1],v.columns[2],v.columns[3]))
#define DEBUG_INVCIPHER_MVECTOR(M,v)                                \
    DBGMSG(DBG_AES,DBG_FLAG(DBG_AES_INVCIPHER),                     \
           ("%s:\t%0.8x %0.8x %0.8x %0.8x\n",M,                     \
            v.columns[0],v.columns[1],v.columns[2],v.columns[3]))

#if _DEBUGGING
#if 1
#define DEBUG_CIPHER_VECTOR(M,V)                        \
    {                                                   \
        CipherVec_AES_u v; v.state = V;                 \
        DBGMSG(DBG_AES,DBG_FLAG(DBG_AES_CIPHER),        \
               ("%s:\t%0.8x %0.8x %0.8x %0.8x\n", M,    \
                bswap_32(v.columns[0]),                 \
                bswap_32(v.columns[1]),                 \
                bswap_32(v.columns[2]),                 \
                bswap_32(v.columns[3])));               \
            }
#define DEBUG_INVCIPHER_VECTOR(M,V)                     \
    {                                                   \
        CipherVec_AES_u v; v.state = V;                 \
        DBGMSG(DBG_AES,DBG_FLAG(DBG_AES_INVCIPHER),     \
               ("%s:\t%0.8x %0.8x %0.8x %0.8x\n", M,    \
                bswap_32(v.columns[0]),                 \
                bswap_32(v.columns[1]),                 \
                bswap_32(v.columns[2]),                 \
                bswap_32(v.columns[3])));               \
            }
#else
#define DEBUG_CIPHER_VECTOR(M,v)                              \
    KOutMsg ( "%s:\t%0.8x %0.8x %0.8x %0.8x\n", M,            \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[0]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[1]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[2]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[3]))
#define DEBUG_INVCIPHER_VECTOR(M,v)                           \
    KOutMsg ( "%s:\t%0.8x %0.8x %0.8x %0.8x\n", M,            \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[0]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[1]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[2]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[3]))
#endif
#define DDEBUG_CIPHER_VECTOR(M,v)                             \
    KOutMsg ( "%s:\t%0.8x %0.8x %0.8x %0.8x\n", M,            \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[0]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[1]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[2]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[3]))
#define DDEBUG_INVCIPHER_VECTOR(M,v)                          \
    KOutMsg ( "%s:\t%0.8x %0.8x %0.8x %0.8x\n", M,            \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[0]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[1]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[2]), \
              bswap_32(((CipherVec_AES_u*)&(v))->columns[3]))

#else
#define DEBUG_CIPHER_VECTOR(M,V)
#define DEBUG_INVCIPHER_VECTOR(M,V)
#endif

/* -----
 * 8 and 32 bit unsigned integers
 */
typedef uint8_t  AESByte;
typedef uint32_t AESWord;
typedef union AESColumn AESColumn;
union AESColumn
{
    AESWord word;
    AESByte bytes [sizeof(AESWord)];
};


/* -----
 * 128 byte block in various forms
 */
typedef AESByte AESBlock [AES_Nb*sizeof(AESWord)];

typedef v128_u8_t  CipherVecByte;
typedef v128_u32_t CipherVecWord;


typedef struct AESKeySchedule  AESKeySchedule;
struct AESKeySchedule
{
    CipherVec round_keys [AES_Nr_MAX];
    uint32_t number_of_rounds;
};

/* ======================================================================
 * FIPS-197 3,4 - 3.5
 *
 * An AES State is the same size as the cipher block size.
 * They are eaither 4 columns of 32 bits or 16 individual bytes.
 * The columns can be accessed by their individual bytes s0
 * there are 4X32, 4X4X8 or 16X8 bits in a state.
 */
typedef union
{
    CipherVec   state;
    uint64_t    u64     [AES_Nb/2];
    AESWord     columns [AES_Nb];
    AESByte     bytes   [AES_Nb * sizeof (AESWord)];
    AESByte     grid    [AES_Nb][sizeof (AESWord)];
} CipherVec_AES_u;


typedef union
{
    CipherVec   state;
    v128_u32_t columns;
} CipherVec_uu __attribute__ ((aligned(16)));

/* 
 * KCipher with 128 bit alignment.
 */
typedef struct CipherAes CipherAes;
struct CipherAes
{
    KCipher dad;
    AESKeySchedule e_key;
    AESKeySchedule d_key;
    CipherVec    e_ivec;
    CipherVec    d_ivec;
};


#ifdef  __cplusplus
}
#endif

#endif /* #ifndef _h_krypto_aes_ncbi_priv_libs_ */


