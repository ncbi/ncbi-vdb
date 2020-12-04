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

#ifndef _h_klib_hashfile_
#define _h_klib_hashfile_

#include <kfs/file.h>
#include <klib/defs.h>
#include <klib/extern.h>
#include <klib/rc.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KHashFile KHashFile;

/* Create a new KHashFile
 * Returns pointer to new hashfile, or NULL if error.
 * "self" [ OUT ] - Self
 * "hashfile" [ IN ] - If not NULL, where to spill data.
 */
KLIB_EXTERN rc_t KHashFileMake(KHashFile** self, struct KFile* hashfile);

/* Destroy hash file.
 */
KLIB_EXTERN void KHashFileDispose(KHashFile* self);

/* Return number of items in hash file. */
KLIB_EXTERN size_t KHashFileCount(const KHashFile* self);

/* Lookup key in hash file, return pointer to value if found or NULL if key
 * doesn't exist.
 * Keys will initially be searched via keyhash, and collisions dealt with via
 * memcmp (specialized for key_size==32 or 64).
 * "key" [ IN ] - Key to lookup.
 * "key_size" [ IN ] - Length of key.
 * "keyhash" [ IN ] - Hash of key.
 * "value" [ IN/OUT ] - Pointer to where value_bytes of value will be copied
 * if found. Can be NULL if only existence check needed.
 * "value_size" [ OUT ] - Length of value if found.
 * Returns true if key found.
 * *value will remain validly allocated until KHashFile is destroyed.
 */
KLIB_EXTERN bool KHashFileFind(const KHashFile* self, const void* key,
                               const size_t key_size, const uint64_t keyhash,
                               void* value, size_t* value_size);

/* Add or replace key/value pair
 * "key" [ IN ] - Key to insert.
 * "key_size" [ IN ] - Length of key.
 * "keyhash" [ IN ] - Hash of key to insert.
 * "value" [ IN ] - Value to insert. Ignored if value_size==0 (set)
 * "value_size" [ IN ] - Length of value.
 * NOTE: If null (\0) terminated strings are values, probably want
 * value_size=strlen(value)+1 so terminator copied into container.
 */
KLIB_EXTERN rc_t KHashFileAdd(KHashFile* self, const void* key,
                              const size_t key_size, const uint64_t keyhash,
                              const void* value, const size_t value_size);

/* Delete key/value pair
 * "key" [ IN ] - Key to delete.
 * "key_size" [ IN ] - Length of key.
 * "keyhash" [ IN ] - Hash of key to delete.
 * Returns true if key was present.
 */
KLIB_EXTERN bool KHashFileDelete(KHashFile* self, const void* key,
                                 const size_t key_size, uint64_t keyhash);

/* Make Iterator.
 * Keys added after IteratorMake may not appear.
 * Only one valid iterator per KHashFile at a time.
 */
KLIB_EXTERN void KHashFileIteratorMake(KHashFile* self);

/* Next key/value
 * "key" [ OUT ] - Next key
 * "key_size" [ OUT ] - Length of key
 * "value" [ OUT ] - Next value. Ignored if value_size==0 or NULL.
 * "value_size" [ OUT ] - Length of value.
 * Returns true if additional keys available
 */
KLIB_EXTERN bool KHashFileIteratorNext(KHashFile* self, void* key,
                                       size_t* key_size, void* value,
                                       size_t* value_size);

/* Reserve space for capacity elements */
KLIB_EXTERN rc_t KHashFileReserve(KHashFile* self, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_hashfile_ */
