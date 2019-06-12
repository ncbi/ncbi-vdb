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

#ifndef _h_index_cmn_
#define _h_index_cmn_

#ifndef _h_kdbfmt_priv_
#include "kdbfmt-priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define KDBINDEXVERS 4
#define V2FIND_RETURNS_SPAN 1

/*--------------------------------------------------------------------------
 * forwards
 */
struct KMMap;
struct PTrie;
struct BSTNode;
struct PBSTNode;
struct KIndex;



/*--------------------------------------------------------------------------
 * KIndexFileHeader
 */

/* v1, v2
 *  just normal simple kdb header
 */
typedef struct KDBHdr KIndexFileHeader_v1, KIndexFileHeader_v2, KIndexFileHeader_v1_v2;

/* v3, v4
 *  indicates type
 */
typedef struct KIndexFileHeader_v3 KIndexFileHeader_v3, KIndexFileHeader_v4, KIndexFileHeader_v3_v4;
struct KIndexFileHeader_v3
{
    KIndexFileHeader_v1_v2 h;
    uint32_t index_type;
    uint32_t reserved1;
};


/*--------------------------------------------------------------------------
 * V1
 *  version 1 of the trie index was hard-coded to enforce uniqueness of
 *  both the string, and a 32-bit id. furthermore, the id was unfortunately
 *  assumed to occupy a mostly contiguous space, such that the projection
 *  was always implemented as an array of ptrie node ids where the id was
 *  used to index the array.
 *
 *  the introduction of highly sparse ids led to deprecation of this
 *  implementation. see version 2 for further information.
 */


/*--------------------------------------------------------------------------
 * KPTrieIndex_v1
 *  persisted keymap
 */
typedef struct KPTrieIndex_v1 KPTrieIndex_v1;
struct KPTrieIndex_v1
{
    struct KMMap const *mm;
    struct PTrie *key2id;
    const uint32_t *id2node;
    uint32_t first;
    uint32_t last;
    bool byteswap;
};

/* initialize an index from file */
rc_t KPTrieIndexInit_v1 ( KPTrieIndex_v1 *self, struct KMMap const *mm, bool byteswap );

/* whackitywhack */
void KPTrieIndexWhack_v1 ( KPTrieIndex_v1 *self );

/* consistency check */
rc_t KPTrieIndexCheckConsistency_v1 ( const KPTrieIndex_v1 *self,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes,
    struct KIndex const *outer, bool key2id, bool id2key );


/*--------------------------------------------------------------------------
 * KTrieIndex_v1
 */
typedef struct KTrieIndex_v1 KTrieIndex_v1;

/* initialize an index from file - can be NULL */
rc_t KTrieIndexOpen_v1 ( KTrieIndex_v1 *self, struct KMMap const *mm, bool byteswap );

/* whack whack */
void KTrieIndexWhack_v1 ( KTrieIndex_v1 *self );

/* map key to id ( was Key2Id ) */
rc_t KTrieIndexFind_v1 ( const KTrieIndex_v1 *self,
    const char *key, uint32_t *id,
    int ( CC * custom_cmp ) ( const void *item, struct PBSTNode const *n, void *data ),
    void *data );

/* projection index id to key-string ( was Id2Key ) */
rc_t KTrieIndexProject_v1 ( const KTrieIndex_v1 *self,
    uint32_t id, char *key_buff, size_t buff_size, size_t *actsize );

/* consistency check */
rc_t KTrieIndexCheckConsistency_v1 ( const KTrieIndex_v1 *self,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes,
    struct KIndex const *outer, bool key2id, bool id2key );


/*--------------------------------------------------------------------------
 * V2
 *  version 2 of the trie index was introduced to handle sparse ids,
 *  and to recognize that ids may be 64 bits and/or negative.
 *
 *  v2 introduces strategy identifiers to handle various cases.
 *
 *  CONSTRAINTS
 *  - both key and id are unique ( version 1 )
 *  - key is not unique, but must map to a contiguous range of ids,
 *    while ids are unique ( the main use case for SRA )
 *
 *  INSERTION
 *  - ids are observed in increasing order
 *
 *  PROJECTION
 *  - id range is contiguous or nearly so ( can use single array )
 *  - id range is sparse
 *
 *  the implementation may be extended by adding new strategies,
 *  but the moment the implementation supports 1 to many mappings,
 *  inserted with ids in increasing order, and an unique constraint
 *  on the ids themselves.
 *
 *  the general case for v2 is "key -> id range", where "id range -> key"
 *  is via contiguous array if avg ( id range ) <= 2, and via sparse
 *  array otherwise.
 *
 *  for the key -> id mappings, this means that the in-core node
 *  either retains an id range ( when not using a projection index ),
 *  or it retains a start-id only, since its range can be determined
 *  from the projection index.
 *
 *  for id -> key mappings, the id is first converted to an ordinal, and
 *  the ordinal to a node. when ids are contiguous, id -> ordinal is simply
 *  derived by subtracting the initial start id. when sparse, an id -> ord
 *  array is used in a binary search to produce the ordinal.
 *
 *  ids are assumed to be 64 bit, stored as id - start id, and packed to
 *  a minimum number of bits to represent the id. ptrie node ids are still
 *  32-bit entities.
 */


/*--------------------------------------------------------------------------
 * KPTrieIndex_v2
 *  persisted keymap
 */
typedef struct KPTrieIndex_v2 KPTrieIndex_v2;
struct KPTrieIndex_v2
{
    int64_t first, last, maxid;
    struct KMMap const *mm;
    struct PTrie *key2id;
    const uint32_t *ord2node;
    union
    {
        const uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
        const int64_t *v64;
    } id2ord;
    uint32_t count;
    uint8_t variant;
    uint8_t id_bits;
    uint8_t span_bits;
    uint8_t byteswap;
};


/* initialize an index from file */
rc_t KPTrieIndexInit_v2 ( KPTrieIndex_v2 *self, struct KMMap const *mm, bool byteswap );
rc_t KPTrieIndexInit_v3_v4 ( KPTrieIndex_v2 *self, struct KMMap const *mm, bool byteswap, bool ptorig );

/* whackitywhack */
void KPTrieIndexWhack_v2 ( KPTrieIndex_v2 *self );

/* map a row id to ord */
uint32_t KPTrieIndexID2Ord_v2 ( const KPTrieIndex_v2 *self, int64_t id );

/* consistency check */
rc_t KPTrieIndexCheckConsistency_v2 ( const KPTrieIndex_v2 *self,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes,
    struct KIndex const *outer, bool key2id, bool id2key, bool all_ids, bool convertFromV1 );


/*--------------------------------------------------------------------------
 * KTrieIndex_v2
 */
typedef struct KTrieIndex_v2 KTrieIndex_v2;

/* initialize an index from file */
rc_t KTrieIndexOpen_v2 ( KTrieIndex_v2 *self, struct KMMap const *mm, bool byteswap );

/* whack whack */
void KTrieIndexWhack_v2 ( KTrieIndex_v2 *self );

/* map key to id range */
rc_t KTrieIndexFind_v2 ( const KTrieIndex_v2 *self,
    const char *key, int64_t *start_id,
#if V2FIND_RETURNS_SPAN
    uint32_t *span,
#endif
    int ( CC * custom_cmp ) ( const void *item, struct PBSTNode const *n, void *data ),
    void * data,
    bool convertFromV1);

/* projection index id to key-string */
#if V2FIND_RETURNS_SPAN
rc_t KTrieIndexProject_v2 ( const KTrieIndex_v2 *self,
    int64_t id, int64_t *start_id, uint32_t *span,
    char *key_buff, size_t buff_size, size_t *actsize );
#else
rc_t KTrieIndexProject_v2 ( const KTrieIndex_v2 *self,
    int64_t id, char *key_buff, size_t buff_size, size_t *actsize );
#endif

/* consistency check */
rc_t KTrieIndexCheckConsistency_v2 ( const KTrieIndex_v2 *self,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes,
    struct KIndex const *outer, bool key2id, bool id2key, bool all_ids, bool convertFromV1 );


/*--------------------------------------------------------------------------
 * KU64Index_v3
 */
typedef struct KU64Index_v3 KU64Index_v3;

rc_t KU64IndexOpen_v3 ( KU64Index_v3 *self, struct KMMap const *mm, bool byteswap );
rc_t KU64IndexWhack_v3 ( KU64Index_v3 *self );

rc_t KU64IndexFind_v3 ( const KU64Index_v3 *self, uint64_t offset,
    uint64_t *key, uint64_t *key_size, int64_t *id, uint64_t *id_qty );

rc_t KU64IndexFindAll_v3 ( const KU64Index_v3 *self, uint64_t offset, 
    rc_t ( CC * f ) ( uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data ),
    void* data );

#ifdef __cplusplus
}
#endif

#endif /* _h_index_priv_ */
