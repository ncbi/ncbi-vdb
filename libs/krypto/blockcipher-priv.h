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

#ifndef _h_krypto_blockcipher_priv_
#define _h_krypto_blockcipher_priv_

#include <krypto/extern.h>
#include "cipher-impl.h"

#include "blockcipher-impl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================
 */
/* typedef struct KBlockCipher KBlockCipher; */
/*     typedef union KBlockCipher_vt KBlockCipher_vt; */

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
/* rc_t KBlockCipherInit (KBlockCipher * self, */
/*                        const KBlockCipher_vt * vt, */
/*                        const char * name); */


typedef union KBlockCipher KBlockCipher;
union KBlockCipher
{
    KBlockCipherByte byte;
    KBlockCipherVec  vec;
};


rc_t KAESBlockCipherVecAesNiMake  (KBlockCipher ** vec_cipher);
rc_t KAESBlockCipherVecRegMake    (KBlockCipher ** vec_cipher);
rc_t KAESBlockCipherVecMake       (KBlockCipher ** vec_cipher);
rc_t KAESBlockCipherByteMake      (KBlockCipher ** byte_cipher);

rc_t KNullBlockCipherVecAesNiMake (KBlockCipher ** vec_cipher);
rc_t KNullBlockCipherVecRegMake   (KBlockCipher ** vec_cipher);
rc_t KNullBlockCipherVecMake      (KBlockCipher ** vec_cipher);
rc_t KNullBlockCipherByteMake     (KBlockCipher ** byte_cipher);


/* KRYPTO_EXTERN rc_t CC KBlockCipherAlloc (KBlockCipher ** obj, size_t z); */


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _h_krypto_blockcipher_priv_ */
