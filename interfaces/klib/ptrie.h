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

#ifndef _h_klib_ptrie_
#define _h_klib_ptrie_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_pbstree_
#include <klib/pbstree.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct Trie;
struct String;


/*--------------------------------------------------------------------------
 * PTNode
 *  a node within text tree
 *
 *  a Trie will contain TNodes, which themselves are intrusive but
 *  irrelevant internal tree links plus a key string plus an externally
 *  defined data structure, representing a value, and supporting multiple
 *  nodes per key. the internal links permit navigation from node
 *  to node that is not possible with the PTNode. furthermore, they
 *  provide direct access to the key string that is not normally stored
 *  verbatim within a persisted image.
 *
 *  a PTrie does not contain structured nodes, but stores linkage
 *  and ordering information separately from the externally defined
 *  data, and rather than using pointers, assigns integer ids to the
 *  nodes themselves. navigation by pointers would require allocations,
 *  which are unnecessary and expensive when traversing a read-only
 *  persisted image. navigation is therefore intrusive on an externally
 *  allocated node structure.
 */
typedef struct PTNode PTNode;
struct PTNode
{
    /* minimally value data
       may also contain key string information,
       either by reference or literal text */
    struct
    {
        const void *addr;
        size_t size;

    } data;

    /* used internally */
    const void *internal;
    uint32_t id;
};

/* MakeKey
 *  tries to make a key string from node
 *  will fail if key text was not embedded into image when created
 *
 *  "key" [ OUT ] - return parameter for a key string allocation
 *  that must be whacked with StringWhack when no longer needed.
 */
KLIB_EXTERN rc_t CC PTNodeMakeKey ( const PTNode *self, struct String const **key );


/*--------------------------------------------------------------------------
 * PTrie
 *  a persisted tree of text nodes
 *
 *  this is a collection of { key, value } pairs, where a many-value
 *  to one key paradigm is naturally supported, as in the b-tree.
 *
 *  the desired retrieval operations are:
 *   a) key -> id : value
 *   b) key -> { id : value, ... }
 *   c) RE -> id : value
 *   d) RE -> { id : value, ... }
 *   e) iteration across { id : value, ... }
 *   f) id : value -> key
 *
 *  the reverse retrieval operation is:
 *   a) id -> value
 *
 *  indexing of the id is performed externally, and should not be interpreted
 *  as a serial, integer value. it may be a byte offset or a combination of
 *  two integer values, as well as anything else.
 */
typedef struct PTrie PTrie;

/* Make
 *  make a persisted tree structure
 * MakeOrig - DEPRECATED
 *  includes code to handle original composite node encoding
 *
 *  "addr" [ IN ] and "size" [ IN ] - constant memory image of
 *  persisted text tree with a lifetime exceeding that of the
 *  PTrie itself
 *
 *  "byteswap" [ IN ] - true if persisted image must be byteswapped
 */
KLIB_EXTERN rc_t CC PTrieMake ( PTrie **tt,
    const void *addr, size_t size, bool byteswap );
KLIB_EXTERN rc_t CC PTrieMakeOrig ( PTrie **tt,
    const void *addr, size_t size, bool byteswap );

/* Count
 *  returns number of { id : value } pairs in text tree
 *  not included within the Trie interface itself, but
 *  was included here due to the fact that it is constant
 *
 *  return value:
 *    integer value >= 0
 */
KLIB_EXTERN uint32_t CC PTrieCount ( const PTrie *self );

/* Size
 *  returns the size in bytes
 *  of the PTrie image
 */
KLIB_EXTERN size_t CC PTrieSize ( const PTrie *self );

/* GetNode
 *  gets a PTNode from an id
 *
 *  "node" [ OUT ] - return parameter for node
 *
 *  "id" [ IN ] - a 1-based integer node id
 *
 *  return values:
 *    EINVAL => an invalid parameter was passed
 *    ENOENT => id out of range
 */
KLIB_EXTERN rc_t CC PTrieGetNode ( const PTrie *self, PTNode *node, uint32_t id );

/* Find
 * PTrieFindRE
 *  find a single { id, value } pair  within tree
 *
 *  "key" [ IN ] - an exact match text string
 *
 *  "re" [ IN ] - a regular expression string
 *
 *  "rtn" [ OUT ] - return parameter for node if found.
 *  its value is undefined unless the function returns success.
 *
 *  "custom_cmp" [ IN, NULL OKAY ] and "data" [ OPAQUE ] - optional
 *  comparison function
 *
 *  return value:
 *    0    => not found
 *    1..n => id of found pair
 */
KLIB_EXTERN uint32_t CC PTrieFind ( const PTrie *self, struct String const *key, PTNode *rtn,
    int ( CC * custom_cmp ) ( const void *item, const PBSTNode *n ,void *data), void * data );

#if 0
KLIB_EXTERN uint32_t CC PTrieFindRE ( const PTrie *self, struct String const *re, PTNode *rtn );
#endif

/* FindAll
 * PTrieFindAllRE
 *  find multiple objects within tree
 *
 *  "key" [ IN ] - an exact match text string
 *
 *  "re" [ IN ] - a regular expression string
 *
 *  "buffer" [ OUT ] and "capacity" [ IN ] - a user-supplied
 *  array of PTNode with a capacity of "capacity" elements.
 *  if successful, the entire found set will be returned unordered within.
 *
 *  "num_found" [ OUT ] - indicates the size of the found set,
 *  regardless of return value, such that if the supplied
 *  buffer were to be too small, the required size is returned.
 *
 *  "custom_cmp" [ IN, NULL OKAY ] and "data" [ OPAQUE ] - optional
 *  comparison function
 *
 *  returns status codes:
 *    EINVAL  => an invalid parameter
 *    ENOENT  => the found set was empty
 *    ENOBUFS => the found set was too large
 */
#if 0
KLIB_EXTERN rc_t CC PTrieFindAll ( const PTrie *self, struct String const *key,
    PTNode buffer [], uint32_t capacity, uint32_t *num_found,
    int ( CC * custom_cmp ) ( const void *item, const PBSTNode *n, void *data ), void *data );
KLIB_EXTERN rc_t CC PTrieFindAllRE ( const PTrie *self, struct String const *re,
    PTNode buffer [], uint32_t capacity, uint32_t *num_found );
#endif

/* ForEach
 *  executes a function on each tree element
 *
 *  "f" [ IN ] and "data" [ IN ] - iteration callback function for
 *  examining each TNode in the tree
 */
KLIB_EXTERN void CC PTrieForEach ( const PTrie *self,
    void ( CC * f ) ( PTNode *n, void *data ), void *data );

/* DoUntil
 *  executes a function on each tree element
 *  until the function returns true
 *
 *  "f" [ IN ] and "data" [ IN ] - iteration callback function for
 *  examining each TNode in the tree. the function returns "true"
 *  to halt iteration.
 *
 *  return values:
 *    the last value returned by "f" or false if never invoked
 */
KLIB_EXTERN bool CC PTrieDoUntil ( const PTrie *self,
    bool ( CC * f ) ( PTNode *n, void *data ), void *data );

/* Whack
 *  tears down internal structure
 */
KLIB_EXTERN void CC PTrieWhack ( PTrie *self );


/*--------------------------------------------------------------------------
 * Trie
 */

/* Persist
 *  much like BSTreePersist but operates on a Trie
 *
 *  "num_writ" [ OUT, NULL OKAY ] -  returns parameter for the number
 *  of bytes written as a result of persisting the b-tree. this will
 *  be the actual bytes written regardless of return status.
 *
 *  "ext_keys" [ IN ] - if true, does not store key data internally.
 *  key text gets encoded into the tree structure in all cases. when
 *  stored internally, any key text not represented by the tree will
 *  be explicitly written by this function. otherwise, the caller will
 *  be expected to store the text as desired. while internal storage
 *  will probably be more efficient, it will cause this function to
 *  fail if the Trie was built with ambiguous key transitions, i.e.
 *  if initialized with "cs_expand" false and keys were added having
 *  characters not included within the "accept" character set. this
 *  is because the tree channels all unrecognized characters through
 *  a single code, making their recovery impossible without being
 *  stored externally.
 *
 *  "write" [ IN, NULL OKAY ] and "write_param" [ IN ] -  a generic
 *  output streaming function used for all operations. if NULL, then
 *  the function will exit after its first pass with the number of
 *  bytes required in "num_writ".
 *
 *  "aux" [ IN ] and "aux_param" [ IN ] - a specialized function for
 *  streaming auxiliary node data to output using the supplied "write"
 *  function. it is invoked during the first pass with a NULL write
 *  function for gathering size data, and during the third pass with
 *  a non-NULL write function.
 */
KLIB_EXTERN rc_t CC TriePersist ( struct Trie const *self, size_t *num_writ, bool ext_keys,
    PTWriteFunc write, void *write_param, PTAuxFunc aux, void *aux_param );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_ptrie_ */
