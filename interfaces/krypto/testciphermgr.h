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
#ifndef _h_krypto_manager_test_
#define _h_krypto_manager_test_

#include <krypto/ciphermgr.h>

/* MakeCipher
 */
typedef int32_t kcipher_subtype;
enum
{
    ksubcipher_none = -1,
    ksubcipher_byte,
    ksubcipher_vec,
    ksubcipher_vecreg,
    ksubcipher_accelerated, /* for example AES-NI */ 
    ksubcipher_count
};

extern kcipher_subtype KCipherSubType;


KRYPTO_EXTERN
rc_t CC KCipherManagerTestMakeCipher (const KCipherManager * self,
                                      struct KCipher ** cipher,
                                      kcipher_type type,
                                      kcipher_subtype subtype);


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_manager_ */
