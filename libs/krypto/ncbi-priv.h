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
#ifndef _h_krypto_ncbi_priv_
#define _h_krypto_ncbi_priv_

#include <klib/defs.h>

#ifdef _GNUC_
#include <v128.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#if defined (USEVEC)
#if defined (USEAESNI)

# if ! defined (USEVECREG)
/*
 * AES_NI && ! VECREG
 * Illegal
 */
#  error "Bad combination of defines"

# else

/*
 * AESNI && VECREG
 * use AES-NI, and SSSE3 and lower
 */

# define USE_AES_NI             1
# define USE_VEC_REG            1
# define USE_VEC                1
# define CMEMBER(name)          KCipherVecAesNi##name
# define CIPHER_IMPL            KCipherVecAesNi
# define AESBCMEMBER(name)      KAESBlockCipherVecAesNi##name
# define AESBCIPHER_IMPL        KAESBlockCipherVecAesNi
# define NULLBCMEMBER(name)     KNullBlockCipherVecAesNi##name
# define NULLBCIPHER_IMPL       KNullBlockCipherVecAesNi
# define BCIPHER_VALUE          CipherVec

# endif
#elif defined (USEVECREG)

# define USE_AES_NI             0
# define USE_VEC_REG            1
# define USE_VEC                1
# define CMEMBER(name)          KCipherVecReg##name
# define CIPHER_IMPL            KCipherVecReg
# define AESBCMEMBER(name)      KAESBlockCipherVecReg##name
# define AESBCIPHER_IMPL        KAESBlockCipherVecReg
# define NULLBCMEMBER(name)     KNullBlockCipherVecReg##name
# define NULLBCIPHER_IMPL       KNullBlockCipherVecReg
# define BCIPHER_VALUE          CipherVec

#else

# define USE_AES_NI             0
# define USE_VEC_REG            0
# define USE_VEC                1
# define CMEMBER(name)          KCipherVec##name
# define CIPHER_IMPL            KCipherVec
# define AESBCMEMBER(name)      KAESBlockCipherVec##name
# define AESBCIPHER_IMPL        KAESBlockCipherVec
# define NULLBCMEMBER(name)     KNullBlockCipherVec##name
# define NULLBCIPHER_IMPL       KNullBlockCipherVec
# define BCIPHER_VALUE          CipherVec

#endif

#else /* ! defined (USEVEC) */


# define USE_AES_NI             0
# define USE_VEC_REG            0
# define USE_VEC                0
# define MEMBER(name)           KCipherByte##name
# define CMEMBER(name)          KCipherByte##name
# define CIPHER_IMPL            KCipherByte
# define AESBCMEMBER(name)      KAESBlockCipherByte##name
# define AESBCIPHER_IMPL        KAESBlockCipherByte
# define NULLBCMEMBER(name)     KNullBlockCipherByte##name
# define NULLBCIPHER_IMPL       KNullBlockCipherByte
# define BCIPHER_VALUE          CipherBlock

#endif /* defined (USEVEC) */

#define CLASS_STRING(name)  #name



#ifdef  __cplusplus
}
#endif

#endif /* #ifndef _h_krypto_ncbi_priv_ */


