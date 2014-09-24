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

#ifndef _h_kfs_key_
#define _h_kfs_key_

#ifndef _h_krypto_extern_
#include <krypto/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KKey KKey;


typedef uint32_t KKeyType;
enum eKKeyType
{
    kkeyNone,
    kkeyAES128,
    kkeyAES192,
    kkeyAES256,
    kkeyTypeCount
};

#define KKeyTypeDefault (kkeyAES128)

struct KKey
{
    KKeyType type;
    uint8_t  text [32]; /* might be more, might be less */
};

enum eKKeySize
{
    kkeyMinimumSize = 6
};

KRYPTO_EXTERN_DATA KKey KKeyEncrypt;
KRYPTO_EXTERN_DATA KKey KKeyDecrypt;

/* WGA ncryption passwords are only significant to 32 characters */
#define WGA_MAX_PASSWORD (32)
KRYPTO_EXTERN_DATA char WGAEncryptPassword[WGA_MAX_PASSWORD];
KRYPTO_EXTERN_DATA char WGADecryptPassword[WGA_MAX_PASSWORD];

typedef char KryptoKeyMapNCBI [32*2];
typedef char KryptoKeyMapWGA  [2 + 32*2];
typedef char KryptoKeyMap [(2*sizeof(KryptoKeyMapNCBI))+(2*sizeof(KryptoKeyMapWGA))+1];
enum
{
    KryptoKeyMapEncrypt = 0,
    KryptoKeyMapDecrypt = sizeof(KryptoKeyMapNCBI),
    KryptoKeyMapWGAEncrypt = KryptoKeyMapDecrypt + sizeof(KryptoKeyMapNCBI),
    KryptoKeyMapWGADecrypt = KryptoKeyMapWGAEncrypt + sizeof(KryptoKeyMapWGA),
    KryptoKeyMapNul = KryptoKeyMapWGADecrypt + sizeof(KryptoKeyMapWGA)
};

KRYPTO_EXTERN  rc_t CC KKeyInitRead (KKey * self, KKeyType key_type, 
                                     const void * password, size_t password_size);

KRYPTO_EXTERN rc_t CC KKeyInitUpdate (KKey * key, KKeyType key_type, 
                                      const void * password, size_t password_size);


#define FAIL_SHORT_PASSWORD (false)



#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_key_ */
