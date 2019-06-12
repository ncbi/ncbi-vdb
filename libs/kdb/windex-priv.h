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

#ifndef _h_index_priv_
#define _h_index_priv_

#ifndef _h_index_cmn_
#include "index-cmn.h"
#endif

#ifndef _h_klib_trie_
#include <klib/trie.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct BSTNode;
struct KDirectory;


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
 * KTrieIdxNode_v1
 */
typedef struct KTrieIdxNode_v1 KTrieIdxNode_v1;
struct KTrieIdxNode_v1
{
    TNode n;
    uint32_t id;
    char key [ 1 ];
};

/*--------------------------------------------------------------------------
 * KTrieIndex_v1
 */
struct KTrieIndex_v1
{
    KPTrieIndex_v1 pt;
    Trie key2id;
    KTrieIdxNode_v1 **id2node;
    uint32_t first;
    uint32_t last;
    uint32_t len;
};

/* insert string into trie, mapping to 32 bit id */
rc_t KTrieIndexInsert_v1 ( KTrieIndex_v1 *self,
    bool proj, const char *key, uint32_t id );

/* drop string from trie and all mappings */
rc_t KTrieIndexDelete_v1 ( KTrieIndex_v1 *self,
    bool proj, const char *key );

/* persist index to file */
rc_t KTrieIndexPersist_v1 ( const KTrieIndex_v1 *self,
    bool proj, struct KDirectory *dir, const char *path, bool use_md5 );


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
 * KTrieIdxNode_v2_s1
 *  strategy 1 - store only start id, derive range from proj index
 */
typedef struct KTrieIdxNode_v2_s1 KTrieIdxNode_v2_s1;
struct KTrieIdxNode_v2_s1
{
    TNode n;
    int64_t start_id;
    char key [ 1 ];
};

/*--------------------------------------------------------------------------
 * KTrieIdxNode_v2_s2
 *  strategy 2 - store complete range when not using proj index
 */
typedef struct KTrieIdxNode_v2_s2 KTrieIdxNode_v2_s2;
struct KTrieIdxNode_v2_s2
{
    TNode n;
    int64_t start_id;
    uint32_t span;
    char key [ 1 ];
};

/*--------------------------------------------------------------------------
 * KTrieIndex_v2
 */
struct KTrieIndex_v2
{
    int64_t first, last;
    KPTrieIndex_v2 pt;
    Trie key2id;
    KTrieIdxNode_v2_s1 **ord2node;
    uint32_t count;
    uint32_t max_span;
};

/* cause persisted tree to be loaded into trie */
rc_t KTrieIndexAttach_v2 ( KTrieIndex_v2 *self, bool proj );

/* insert string into trie, mapping to 64 bit id */
rc_t KTrieIndexInsert_v2 ( KTrieIndex_v2 *self,
    bool proj, const char *key, int64_t id );

/* drop string from trie and all mappings */
rc_t KTrieIndexDelete_v2 ( KTrieIndex_v2 *self,
    bool proj, const char *key );

/* persist index to file */
rc_t KTrieIndexPersist_v2 ( const KTrieIndex_v2 *self,
    bool proj, struct KDirectory *dir, const char *path, bool use_md5 );


/*--------------------------------------------------------------------------
 * KU64Index_v3
 */
struct KU64Index_v3
{
    BSTree tree;
    rc_t rc;
};

rc_t KU64IndexInsert_v3(KU64Index_v3* self, bool unique, uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty);
rc_t KU64IndexDelete_v3(KU64Index_v3* self, uint64_t key);

rc_t KU64IndexPersist_v3(KU64Index_v3* self, bool proj, struct KDirectory *dir, const char *path, bool use_md5);



/*--------------------------------------------------------------------------
 * KIndex
 *  represents an index
 */

/* Cmp
 * Sort
 */
int KIndexCmp ( const void *item, struct BSTNode const *n );
int KIndexSort ( struct BSTNode const *item, struct BSTNode const *n );


#ifdef __cplusplus
}
#endif

#endif /* _h_index_priv_ */
