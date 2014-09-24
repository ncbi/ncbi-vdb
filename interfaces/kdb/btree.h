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

#ifndef _h_kdb_btree_
#define _h_kdb_btree_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * defines
 */

/* KEY2ID
 *  the original behavior of btree was to perform key=>value mapping
 *  by converting to a key=>id mapping, no values are stored in the b-tree
 *  page file, allowing for an external storage
 */
#ifndef BTREE_KEY2ID
#define BTREE_KEY2ID 1
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;
struct KDataBuffer;


/*--------------------------------------------------------------------------
 * KBTreeCompareFunc
 */
typedef int ( CC * KBTreeCompareFunc )
    ( const void *a, size_t asize, const void *b, size_t bsize );


#if ! BTREE_KEY2ID
/*--------------------------------------------------------------------------
 * KBTreeValue
 *  gives access to a value within a data page
 */
typedef struct KBTreeValue KBTreeValue;
struct KBTreeValue
{
    uint64_t data [ 3 ];
};

/* Whack
 *  destroys object for further use
 *  may write modifications to disk ( see "write_through" below )
 */
KDB_EXTERN rc_t CC KBTreeValueWhack ( KBTreeValue *self );


/* AccessRead
 * AccessUpdate
 *  gain access to value address
 *  update access marks data page as modified
 *
 *  "mem" [ OUT ] - pointer to value
 *
 *  "bytes" [ OUT, NULL OKAY ] - size of value memory
 */
KDB_EXTERN rc_t CC KBTreeValueAccessRead ( const KBTreeValue *self,
    const void **mem, size_t *bytes );
KDB_EXTERN rc_t CC KBTreeValueAccessUpdate ( KBTreeValue *self,
    void **mem, size_t *bytes );

#endif /* BTREE_KEY2ID */

/*--------------------------------------------------------------------------
 * KBTree
 *  this implementation is an extremely simplified structure
 *  meant to provide the ability to create an index for temporary use
 */
typedef struct KBTree KBTree;


/* KBTreeKeyType
 *  describes the type of key to be used
 */
typedef uint8_t KBTreeKeyType;
enum
{
    kbtOpaqueKey,               /* key is a ( void*, size_t ) pair  */
#if 0
    kbtNulTermCharKey,          /* key is a NUL-terminated char []  */
    kbtStringKey,               /* key is a String*                 */
    kbtInt8Key,                 /* key is an int8_t                 */
    kbtInt16Key,                /* key is an int16_t                */
    kbtInt32Key,                /* key is an int32_t                */
    kbtInt64Key,                /* key is an int64_t                */
    kbtUint8Key,                /* key is a uint8_t                 */
    kbtUint16Key,               /* key is a uint16_t                */
    kbtUint32Key,               /* key is a uint32_t                */
    kbtUint64Key,               /* key is a uint64_t                */
    kbtFloatKey,                /* key is a float                   */
    kbtDoubleKey,               /* key is a double                  */
#endif
    kbtPacked2naKey,            /* key is in packed 2na             */
    kbtLastDefined
};


/* MakeRead
 * MakeUpdate
 *  make a b-tree object backed by supplied KFile
 *
 *  "backing" [ IN ] - open file with appropriate permissions:
 *   read is required in all cases, and write is required for update.
 *   NB - a reference will be attached to this file.
 *
 *  "climit" [ IN ] - cache limit in bytes. the internal cache will
 *   retain UP TO ( but not exceeding ) the limit specified. a value
 *   of 0 ( zero ) will disable caching.
 *
 *  "write_through" [ IN ] - if true, causes flushing of modified page
 *   after its value is released
 *
 *  "type" [ IN ] - describes the key type ( see above )
 *
 *  "key_chunk_size" [ IN ] - the "chunking" ( alignment ) factor for
 *   storing keys, rounded up to the nearest power of 2.
 *
 *  "value_chunk_size" [ IN ] - chunking factor for values
 *   ( see "key_chunk_size" )
 *
 *  "min_key_size" [ IN ] and "max_key_size" [ IN ] - specifies the allowed
 *   opaque key sizes. min == max implies fixed size. ignored for well
 *   known fixed size key types.
 *
 *  "id_size" [ IN ] - size of id in bytes, from 1 to 8.
 *
 *  "min_value_size" [ IN ] and "max_value_size" [ IN ] - specifies the allowed
 *   value sizes. min == max implies fixed size.
 *
 *  "cmp" [ IN, NULL OKAY ] - optional comparison callback function for opaque keys.
 *   specific key types will use internal comparison functions. for opaque keys, a
 *   NULL function pointer will cause ordering by size and binary comparison.
 */
KDB_EXTERN rc_t CC KBTreeMakeRead ( const KBTree **bt,
    struct KFile const *backing, size_t climit, KBTreeCompareFunc cmp );
#if BTREE_KEY2ID
KDB_EXTERN rc_t CC KBTreeMakeUpdate ( KBTree **bt, struct KFile *backing,
    size_t climit, bool write_through, KBTreeKeyType type,
    size_t min_key_size, size_t max_key_size, size_t id_size,
    KBTreeCompareFunc cmp );
#else
KDB_EXTERN rc_t CC KBTreeMakeUpdate ( KBTree **bt, struct KFile *backing,
    size_t climit, bool write_through, KBTreeKeyType type,
    size_t key_chunk_size, size_t value_chunk_size,
    size_t min_key_size, size_t max_key_size,
    size_t min_value_size, size_t max_value_size,
    KBTreeCompareFunc cmp );
#endif


/* AddRef
 * Release
 *  ignores NULL references
 */
KDB_EXTERN rc_t CC KBTreeAddRef ( const KBTree *self );
KDB_EXTERN rc_t CC KBTreeRelease ( const KBTree *self );


/* DropBacking
 *  used immediately prior to releasing
 *  prevents modified pages from being flushed to disk
 *  renders object nearly useless
 */
KDB_EXTERN rc_t CC KBTreeDropBacking ( KBTree *self );


/* Size
 *  returns size in bytes of file and cache
 *
 *  "lsize" [ OUT, NULL OKAY ] - return parameter for logical size
 *
 *  "fsize" [ OUT, NULL OKAY ] - return parameter for file size
 *
 *  "csize" [ OUT, NULL OKAY ] - return parameter for cache size
 */
KDB_EXTERN rc_t CC KBTreeSize ( const KBTree *self,
    uint64_t *lsize, uint64_t *fsize, size_t *csize );


/* Find
 *  searches for a match
 *
 *  "val" [ OUT ] - return parameter for value found
 *   accessed via KBTreeValueAccess* described above
 *   must be balanced with a call to KBTreeValueWhack.
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */
#if BTREE_KEY2ID
KDB_EXTERN rc_t CC KBTreeFind ( const KBTree *self, uint64_t *id,
    const void *key, size_t key_size );
#else
KDB_EXTERN rc_t CC KBTreeFind ( const KBTree *self, KBTreeValue *val,
    const void *key, size_t key_size );
#endif


/* FindOne
 *  searches for a match
 *  where "match" is defined as an exact match
 *  of the shorter ( key, entry ) against the
 *  corresponding prefix of the longer.
 *
 *  "id" [ OUT ] - return id of matching entry if found
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 *
 *  "match_type" [ IN ] - choose search algorithm
 *
 *  "remainder" [ OUT ] - returns the number of bytes
 *  left unmatched by comparison between the key and entry.
 *  values:
 *    = 0    - exact match
 *    < 0    - key is shorter than entry
 *    > 0    - key is longer than entry
 */
#if BTREE_KEY2ID && 1
typedef uint32_t KBTreeMatchType;
enum
{
    kbtMatchFirst = 1,       /* choose first matching entry       */
    kbtMatchRandom           /* choose a matching entry at random */
};

typedef struct KBTreeMatchResult KBTreeMatchResult;
struct KBTreeMatchResult
{
    uint64_t id;
    int64_t remainder;
};

KDB_EXTERN rc_t CC KBTreeFindOne ( const KBTree *self,
    KBTreeMatchResult *found, KBTreeMatchType match_type,
    const void *key, size_t key_size );
#endif


/* FindAll
 *  searches for all matches
 *  where "match" is defined as an exact match
 *  of the shorter ( key, entry ) against the
 *  corresponding prefix of the longer.
 *
 *  "ids" [ IN/OUT ] - array of uint64_t gets dynamically
 *  resized based upon the number of matches
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 *
 *  "remainder" [ OUT ] - returns the number of bytes
 *  left unmatched by comparison between the key and entry.
 *  values:
 *    = 0    - exact match
 *    < 0    - key is shorter than entry
 *    > 0    - key is longer than entry
 */
#if BTREE_KEY2ID && 1
KDB_EXTERN rc_t CC KBTreeFindAll ( const KBTree *self,
    struct KDataBuffer *found_set,
    const void *key, size_t key_size );
#endif


/* FindCustom
 *  searches for a match using a custom function
 *
 *  "val" [ OUT ] - return parameter for value found
 *   accessed via KBTreeValueAccess* described above
 *   must be balanced with a call to KBTreeValueWhack.
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */
#if BTREE_KEY2ID && 0
KDB_EXTERN rc_t CC KBTreeFindCustom ( const KBTree *self, uint64_t *id,
    const void *key, size_t key_size, KBTreeCompareFunc custom_cmp );
#endif


/* Entry
 *  searches for a match or creates a new entry
 *
 *  "val" [ OUT ] - return parameter for value found
 *   accessed via KBTreeValueAccess* described above
 *   must be balanced with a call to KBTreeValueWhack.
 *
 *  "was_inserted" [ OUT ] - if true, the returned value was the result of an
 *   insertion and can be guaranteed to be all 0 bits. otherwise, the returned
 *   value will be whatever was there previously.
 *
 *  "alloc_size" [ IN ] - the number of value bytes to allocate upon insertion,
 *   i.e. if the key was not found. this value must agree with the limits
 *   specified in Make ( see above ).
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key
 */
#if BTREE_KEY2ID
KDB_EXTERN rc_t CC KBTreeEntry ( KBTree *self, uint64_t *id,
    bool *was_inserted, const void *key, size_t key_size );
#else
KDB_EXTERN rc_t CC KBTreeEntry ( KBTree *self, KBTreeValue *val,
    bool *was_inserted, size_t alloc_size,
    const void *key, size_t key_size );
#endif


/* ForEach
 *  executes a function on each tree element
 *
 *  "reverse" [ IN ] - if true, iterate in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - callback function
 */
#if BTREE_KEY2ID
KDB_EXTERN rc_t CC KBTreeForEach ( const KBTree *self, bool reverse,
    void ( CC * f ) ( const void *key, size_t key_size, uint64_t id, void *data ), void *data );
#else
KDB_EXTERN rc_t CC KBTreeForEach ( const KBTree *self, bool reverse,
    void ( CC * f ) ( const void *key, size_t key_size, KBTreeValue *val, void *data ), void *data );
#endif


#ifdef __cplusplus
}
#endif

#endif /*  _h_kdb_btree_ */
