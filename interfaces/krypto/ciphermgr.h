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
#ifndef _h_krypto_manager_
#define _h_krypto_manager_

#ifndef _h_krypto_extern_
#include <krypto/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif


#ifndef _h_kfs_defs_
#include <kfs/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* enabled/disable a set of in-house clean;y written aes ciphers. some optimize
 * for certain c processors */

#define USE_NCBI_AES 1


/*--------------------------------------------------------------------------
 * KCipherManager
 */
struct KCipher;

typedef struct KCipherManager KCipherManager;

/* AddRef
 * Release
 *  ignores NULL references
 */
KRYPTO_EXTERN
rc_t CC KCipherManagerAddRef (const KCipherManager *self);

KRYPTO_EXTERN
rc_t CC KCipherManagerRelease (const KCipherManager *self);


/* MakeCipher
 */
typedef uint32_t kcipher_type;
enum
{
    kcipher_null, /* no encryption = just a copy */
    kcipher_AES,
    kcipher_count
};


KRYPTO_EXTERN
rc_t CC KCipherManagerMakeCipher (const KCipherManager * self,
                                  struct KCipher ** cipher,
                                  kcipher_type type);

/* Make
 */
KRYPTO_EXTERN
rc_t CC KCipherManagerMake (KCipherManager ** pmanager);




#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_manager_ */
