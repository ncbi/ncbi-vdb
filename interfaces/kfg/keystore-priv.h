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

#ifndef _h_kfg_keystore_priv_
#define _h_kfg_keystore_priv_

#ifndef _h_kfg_keystore_
#include <kfg/keystore.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 KEncryptionKey
 */
 struct KEncryptionKey
 {
    KRefcount   refcount;
    String      value; /* 0-terminator is not included */
 };

KFG_EXTERN rc_t CC KEncryptionKeyMake(const char* value, KEncryptionKey** key);
 
/*--------------------------------------------------------------------------
 KKeyStore 
 */
#ifndef KKEYSTORE_IMPL
    #define KKEYSTORE_IMPL struct KKeyStore
#endif

typedef struct KKeyStore_vt_v1_struct {
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    rc_t ( *destroy   )                 ( KKEYSTORE_IMPL* self );
    rc_t ( *getKey )                    ( const KKEYSTORE_IMPL* self, const char* obj_key, KEncryptionKey** enc_key);
    /* end minor version == 0 */

} KKeyStore_vt_v1;

typedef union KKeyStore_vt {
    KKeyStore_vt_v1* v1;
} KKeyStore_vt;

/* KKeyStoreSetConfig
 * Specify the Config object used by the KeyStore object. If the object is not specified, one will be created 
 * when required using KConfigMake
 *
 * kfg [ IN, NULL OK ] if NULL, forget the current kfg object
 */
KFG_EXTERN rc_t CC KKeyStoreSetConfig(struct KKeyStore* self, const struct KConfig* kfg);

#ifdef __cplusplus
}
#endif

#endif /* _h_kfg_keystore_priv_ */
