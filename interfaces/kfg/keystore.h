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

#ifndef _h_kfg_keystore_
#define _h_kfg_keystore_

#ifndef _h_kfg_extern_
#include <kfg/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct KFile;
struct KConfig;
struct String;

/*--------------------------------------------------------------------------
 * KEncryptionKey
 *  Encryption key, reference-counted and kept in non-swappable memory
 */
typedef struct KEncryptionKey KEncryptionKey;

KFG_EXTERN rc_t CC KEncryptionKeyAddRef ( struct KEncryptionKey *self );
KFG_EXTERN rc_t CC KEncryptionKeyRelease ( struct KEncryptionKey *self );


/*--------------------------------------------------------------------------
 * KKeyStore
 *  Encryption key storage interface
 */
typedef struct KKeyStore KKeyStore;

 /* all strings are NUL-terminated */
 
/* AddRef
 */
KFG_EXTERN rc_t CC KKeyStoreAddRef ( const KKeyStore *self );
/* Release
 */
KFG_EXTERN rc_t CC KKeyStoreRelease ( const KKeyStore *self );

/* 
 * KKeyStoreSetTemporaryKeyFromFile
 *
 *  set a temporarily encryption key 
 *  the key will only be maintained for this instance of KKeyStore (use for externally supplied keys).
 * 
 *  enc_file[ IN, NULL OK ] - open KFile containing the key (characters after the first \n are ignored)
 *  if NULL is specified, forget the previously set temporary key
 */
KFG_EXTERN rc_t CC KKeyStoreSetTemporaryKeyFromFile(KKeyStore* self, const struct KFile* enc_file);

/* KKeyStoreGetKey
 * KKeyStoreGetKeyByProjectId
 *  returns the encryption key associated with the specified object
 * 
 *  obj_key [ IN, NULL OK ] - NUL-terminated key of the object in question. The key can be an accession name, or a file system path.
 *      If NULL, will return the current protected repository's key,
 *      or, if projectId is passed, will return project's repository's key,
 *      or, if not within a protected repository, the global key if available
 *      from environment or configuration 
 *  enc_key [ OUT ] 
 */
KFG_EXTERN rc_t CC KKeyStoreGetKey(const KKeyStore* self, const char* obj_key, KEncryptionKey** enc_key);

KFG_EXTERN rc_t CC KKeyStoreGetKeyByProjectId(const KKeyStore* self,
    const char* obj_key, KEncryptionKey** enc_key, uint32_t projectId);

/*
 * Registering bindings between numeric object Ids and object names
 */                                        
 
/* KKeyStoreSetBindingsFile
 *  Specifies location of a bindings file used by this instance of KeyStore. 
 *  If never set or set to NULL, uses default location ($NCBI_HOME/objid.mapping)
 * 
 * self [ IN ] - KKeyStore object
 * path [ IN, NUL-TERMINATED, NULL OK ] - pathname to the bindings file. If NULL, reset to default location ($NCBI_HOME/objid.mapping)
 */
KFG_EXTERN rc_t CC KKeyStoreSetBindingsFile(struct KKeyStore* self, const char* path);

/* KKeyStoreGetBindingsFile
 *  Returns location of a bindings file used by this instance of KeyStore. 
 * 
 * self [ IN ] - KKeyStore object
 * returns pathname to the bindings file. If NULL, default location will be used when needed
 */
KFG_EXTERN const char* KKeyStoreGetBindingsFile(const struct KKeyStore* self);
 
/* KKeyStoreRegisterObject
 *  registers a binding between an object Id and an object name 
 * 
 * self [ IN ] - KKeyStore object
 * oid [ IN ] - object id
 * obj [ IN ] - object's name (any characters except '\r' and '\n' )
 */
KFG_EXTERN rc_t CC KKeyStoreRegisterObject(struct KKeyStore* self, uint32_t oid, const struct String* name);

/* KKeyStoreGetObjectId
 *  look up an object id by an object name 
 * 
 * self [ IN ] - KKeyStore object
 * obj [ IN ] - object's name 
 * oid [ OUT ] - object id
 */
KFG_EXTERN rc_t CC VKKeyStoreGetObjectId(const struct KKeyStore* self, const struct String* name, uint32_t* oid);

/* KKeyStoreGetObjectName
 *  look up an object name by an object id
 * 
 * self [ IN ] - KKeyStore object
 * obj [ IN ] - object's name 
 * oid [ OUT ] - object id
 */
KFG_EXTERN rc_t CC KKeyStoreGetObjectName(const struct KKeyStore* self, uint32_t oid, const struct String** name);


/* KKeyStoreMake
 * Creates a non-keyring implementation, which uses a combination of location-based and global encryption keys.
 * 
 * self [ OUT ]
 * kfg [ IN, NULL OK ] - optional config object to use
 */
KFG_EXTERN rc_t CC KKeyStoreMake(KKeyStore** self, struct KConfig* kfg);

#ifdef __cplusplus
}
#endif

#endif /* _h_kfg_keystore_ */
