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

#ifndef _h_klib_hashtable_
#define _h_klib_hashtable_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KHashTable KHashTable;

typedef enum hashkey_type { raw, cstr } hashkey_type;
/* NB: Not thread safe */

/* Create a new KHashTable.
 * Returns pointer to new table, or NULL if error.
 * "self" [ OUT ] - Self
 * "key_size" [ IN ] - Number of bytes keys require
 * "value_size" [ IN ] - Can be 0 (set). Number of bytes values require.
 * "capacity" [ IN ] - Hint about initial capacity.
 *                            If 0, use 16 as default
 * "max_load_factor" [ IN ] - When hash table exceeds this, grow.
 *                            If 0, use 0.6 as default
 * "key_type" [ IN ] - If raw, use memcmp and memcpy to compare/copy keys.
 *                     If cstr, strcmp(*key) will be used to compare keys and
 *                     only pointer to key will be placed in container.
 *                     Caller still responsible for key's memory allocation.
 */
KLIB_EXTERN rc_t KHashTableMake(KHashTable** self, size_t key_size,
                                size_t value_size, size_t capacity,
                                double max_load_factor,
                                hashkey_type key_type);

/* Destroy hash table and optionally elements.
 * If keywhack/valuewhack !=NULL invoke for each element in table.
 * "keywhack" [ IN ] - If not NULL, call to destroy each key.
 * "valuewhack" [ IN ] - "" value. Ignored if value_size==0 (set).
 * TODO: keywhack and valuewhack not implemented
 */
KLIB_EXTERN void KHashTableDispose(KHashTable* self,
                                   void (*keywhack)(void* item, void* data),
                                   void (*valuewhack)(void* item, void* data),
                                   void* data);

/* Return number of items in hash table. */
KLIB_EXTERN size_t KHashTableCount(const KHashTable* self);

/* Lookup key in hash table, return pointer to value if found or NULL if key
 * doesn't exist.
 * Keys will initially be searched via keyhash, and colliisions dealt with via
 * memcmp (specialized for key_size==32 or 64).
 * "key" [ IN ] - Key to lookup.
 * "keyhash" [ IN ] - Hash of key.
 * "value" [ IN/OUT ] - Pointer to where value_bytes of value will be copied
 * if found. Can be NULL if only existence check needed.
 * Returns true if key found.
 */
KLIB_EXTERN bool KHashTableFind(const KHashTable* self, const void* key,
                                uint64_t keyhash, void* value);

/* Add or replace key/value pair
 * "key" [ IN ] - Key to insert.
 * "keyhash" [ IN ] - Hash of key to insert.
 * "value" [ IN ] - Value to insert. Ignored if value_size==0 (set)
 */
KLIB_EXTERN rc_t KHashTableAdd(KHashTable* self, const void* key,
                               uint64_t keyhash, const void* value);

/* Delete key/value pair
 * "key" [ IN ] - Key to delete.
 * "keyhash" [ IN ] - Hash of key to delete.
 * Returns true if key was present.
 */
KLIB_EXTERN bool KHashTableDelete(KHashTable* self, const void* key,
                                  uint64_t keyhash);

/* Make Iterator.
 * Will become invalid after any insertions. Only one valid iterator per
 * KHashTable at a time.
 */
KLIB_EXTERN void KHashTableIteratorMake(KHashTable* self);

/* Next key/value
 * "key" [ OUT ] - Next key
 * "value" [ OUT ] - Next value. Ignored if value_size==0 or NULL.
 * Returns true if additional keys available
 */
KLIB_EXTERN bool KHashTableIteratorNext(KHashTable* self, void* key,
                                        void* value);

/* Returns current load factor (# buckets / # items) */
/* KLIB_EXTERN double KHashTableGetLoadFactor(const KHashTable* self); */

/* Reserve space for capacity elements */
KLIB_EXTERN rc_t KHashTableReserve(KHashTable* self, size_t capacity);

/* Hash function */
KLIB_EXTERN uint64_t KHash(const char* s, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_hashtable_ */
