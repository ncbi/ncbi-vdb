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

#ifndef _h_kdb_index_
#define _h_kdb_index_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KTable;
struct PBSTNode;
struct KDatabase;


/*--------------------------------------------------------------------------
 * KIdxType
 *  the type of index being dealt with
 */
typedef uint8_t KIdxType;
enum
{
    /* version 1 */

    kitText,          /* text string => id */
    kitU64,           /* uint64 (like file offset) to row id */

    kitProj = 128     /* reverse index flag, row id => key */
};


/*--------------------------------------------------------------------------
 * KIndex
 *  an object capable of mapping an object to integer oid
 */
typedef struct KIndex KIndex;


/* Addref
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
KDB_EXTERN rc_t CC KIndexAddRef ( const KIndex *self );
KDB_EXTERN rc_t CC KIndexRelease ( const KIndex *self );


/* CreateIndex
 * VCreateIndex
 *  create a new or open an existing index
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "type" [ IN ] - type of index to create
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */
KDB_EXTERN rc_t CC KDatabaseCreateIndex ( struct KDatabase *self, KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, ... );
KDB_EXTERN rc_t CC KTableCreateIndex ( struct KTable *self, KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, ... );

KDB_EXTERN rc_t CC KDatabaseVCreateIndex ( struct KDatabase *self, KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, va_list args );
KDB_EXTERN rc_t CC KTableVCreateIndex ( struct KTable *self, KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, va_list args );


/* OpenIndexRead
 * VOpenIndexRead
 *  open an index for read
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */
KDB_EXTERN rc_t CC KDatabaseOpenIndexRead ( struct KDatabase const *self,
    const KIndex **idx, const char *name, ... );
KDB_EXTERN rc_t CC KTableOpenIndexRead ( struct KTable const *self,
    const KIndex **idx, const char *name, ... );

KDB_EXTERN rc_t CC KDatabaseVOpenIndexRead ( struct KDatabase const *self,
    const KIndex **idx, const char *name, va_list args );
KDB_EXTERN rc_t CC KTableVOpenIndexRead ( struct KTable const *self,
    const KIndex **idx, const char *name, va_list args );


/* OpenIndexUpdate
 * VOpenIndexUpdate
 *  open an index for read/write
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */

KDB_EXTERN rc_t CC KDatabaseOpenIndexUpdate ( struct KDatabase *self,
    KIndex **idx, const char *name, ... );
KDB_EXTERN rc_t CC KTableOpenIndexUpdate ( struct KTable *self,
    KIndex **idx, const char *name, ... );

KDB_EXTERN rc_t CC KDatabaseVOpenIndexUpdate ( struct KDatabase *self,
    KIndex **idx, const char *name, va_list args );
KDB_EXTERN rc_t CC KTableVOpenIndexUpdate ( struct KTable *self,
    KIndex **idx, const char *name, va_list args );


/* Locked
 *  returns true if locked
 */
KDB_EXTERN bool CC KIndexLocked ( const KIndex *self );


/* Version
 *  returns the format version
 */
KDB_EXTERN rc_t CC KIndexVersion ( const KIndex *self, uint32_t *version );


/* Type
 *  returns the type of index
 */
KDB_EXTERN rc_t CC KIndexType ( const KIndex *self, KIdxType *type );


/* Commit
 *  ensure any changes are committed to disk
 */
KDB_EXTERN rc_t CC KIndexCommit ( KIndex *self );


/* CheckConsistency
 *  run a consistency check on the open index
 *
 *  "level" [ IN ] - a measure of rigor of the exercise:
 *    0 is the lightest
 *    1 will test all id mappings
 *    2 will perform key->id retrievals
 *    3 will perform id->key retrievals if a projection index exists
 *
 *  "start_id" [ OUT, NULL OKAY ] - returns the first id in index
 *
 *  "id_range" [ OUT, NULL OKAY ] - returns the range of ids from first to last
 *
 *  "num_keys" [ OUT, NULL OKAY ] - returns the number of key entries
 *
 *  "num_rows" [ OUT, NULL OKAY ] - returns the number of key->id mappings
 *
 *  "num_holes" [ OUT, NULL OKAY ] - returns the number of holes in the mapped id range
 */
KDB_EXTERN rc_t CC KIndexConsistencyCheck ( const KIndex *self, uint32_t level,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes );


/* Insert
 *  creates a mapping from key to id
 *  and potentially from id to key if supported
 *
 *  "unique" [ IN ] - if true, key must be unique
 *
 *  "key" [ IN ] - NUL terminated string for text
 *
 *  "id" [ IN ] - id
 */
KDB_EXTERN rc_t CC KIndexInsertText ( KIndex *self, bool unique,
    const char *key, int64_t id );

/* Delete
 *  deletes all mappings from key
 */
KDB_EXTERN rc_t CC KIndexDeleteText ( KIndex *self, const char *key );

/* Find
 *  finds a single mapping from key
 *
 *  "key" [ IN ] - NUL terminated string to be found
 *
 *  "start_id" [ OUT ] - starting id of found range
 *
 *  "id_count [ OUT, NULL OKAY ] - the number of contiguous
 *  row ids in found range
 *
 *  "custom_cmp" [ IN, NULL OKAY ] and "data" [ IN, OPAQUE ]
 *  optional comparison function for search
 */
KDB_EXTERN rc_t CC KIndexFindText ( const KIndex *self,
    const char *key, int64_t *start_id, uint64_t *id_count,
    int ( CC * custom_cmp ) ( const void *item,
        struct PBSTNode const *n, void *data ),
    void *data );

/* FindAll
 *  finds all mappings from key
 */
KDB_EXTERN rc_t CC KIndexFindAllText ( const KIndex *self, const char *key,
    rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, void *data ),
    void *data );

/* Project
 *  finds key(s) mapping to value/id if supported
 *
 *  "id" [ IN ] - row id to be located
 *
 *  "start_id [ OUT, NULL OKAY ] - the first id of found range
 *
 *  "id_count [ OUT, NULL OKAY ] - the number of contiguous
 *  row ids in found range
 *
 *  "key" [ OUT ] and "kmax" [ IN ] - return buffer
 *  for NUL terminated index text
 *
 *  "actsize" [ OUT, NULL OKAY ] - returns key size in bytes,
 *   excluding NUL termination
 *
 *  returns rcBuffer, rcInsufficient
 *  if kmax <= strlen ( key )
 */
KDB_EXTERN rc_t CC KIndexProjectText ( const KIndex *self,
    int64_t id, int64_t *start_id, uint64_t *id_count,
    char *key, size_t kmax, size_t *actsize );


/* ProjectAll
 *  finds key(s) mapping to value/id if supported
 */
KDB_EXTERN rc_t CC KIndexProjectAllText ( const KIndex *self, int64_t id,
    rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, const char *key, void *data ),
    void *data );


/* InsertU64
 *  creates a mapping from an (file) offset range to an id range
 *
 *  "unique" [ IN ] - if true, key ranges cannot overlap
 *
 *  "key" [ IN ] - offset (in file)
 *
 *  "key_size" [ IN ] - size of chunk (in file)
 *
 *  "start_id" [ IN ] - starting id 
 *
 *  "id_count" [ IN ] - number of ids in chunk
 */
KDB_EXTERN rc_t CC KIndexInsertU64 ( KIndex *self, bool unique,
    uint64_t key, uint64_t key_size, int64_t start_id, uint64_t id_count );

/* DeleteU64
 *  deletes all mappings from key
 */
KDB_EXTERN rc_t CC KIndexDeleteU64 ( KIndex *self, uint64_t key );

/* FindU64
 *  finds a FIRST chunk by offset with in file
 */
KDB_EXTERN rc_t CC KIndexFindU64 ( const KIndex *self, uint64_t offset,
    uint64_t *key, uint64_t *key_size, int64_t *start_id, uint64_t *id_count );

/* FindAllU64
 *  Iterate through all chunks with an offset and call f() for each range
 */
KDB_EXTERN rc_t CC KIndexFindAllU64 ( const KIndex *self, uint64_t offset,
    rc_t ( CC * f ) ( uint64_t key, uint64_t key_size,
        int64_t start_id, uint64_t id_count, void *data ),
    void *data);


#ifdef __cplusplus
}
#endif

#endif /*  _h_kdb_index_ */
