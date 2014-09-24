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

#ifndef _h_klib_trie_
#define _h_klib_trie_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * TNode
 *  a node within trie
 */
typedef struct TNode TNode;
struct TNode
{
    BSTNode n;
    String key;
};

/* TNodeMake
 *  creates a TNode of variable size
 *
 *  "n" will contain the returned node
 *
 *  "size" must be at least the size of a TNode
 */
KLIB_EXTERN rc_t CC TNodeMake ( TNode **n, size_t size );

/* TNodeWhack
 *  performs final whacking of node
 */
KLIB_EXTERN void CC TNodeWhack ( TNode *self );


/*--------------------------------------------------------------------------
 * Trie
 *  a tree of text nodes
 *
 *  this is a collection of { key, value } pairs, where a many-value
 *  to one key paradigm is naturally supported, as in the b-tree.
 *
 *  the desired insertion operations are:
 *   a) key -> value
 *   b) key -> { value, ... }
 *
 *  the desired retrieval operations are:
 *   a) key -> value
 *   b) key -> { value, ... }
 *   c) RE -> key : value
 *   d) RE -> { key : value, ... }
 *   e) iteration across { key : value, ... }
 *
 *  the reverse retrieval operation is externally managed by virtue of
 *  externalization of the TNode, which bundles together key and value.
 *  any external mechanism allowing access to the TNode will perform a
 *  reverse lookup.
 */
typedef struct Trie Trie;
struct Trie
{
    /* root node in the tree */
    struct TTrans *root;

    /* forward and reverse UTF-32 character maps */
    const uint16_t *map;
    const uint32_t *rmap;

    /* range of acceptable UTF-32 input characters */
    uint32_t first_char;
    uint32_t last_char;

    /* width of transition array */
    uint16_t width;

    /* limit to the number of values */
    uint16_t limit;

    /* automatically expand character set */
    uint8_t cs_expand;

#if _DEBUGGING
    /* post-validate tree after every insert */
    uint8_t validate;

    uint8_t align1 [ 2 ];
#else
    uint8_t align1 [ 3 ];
#endif
};

/* TrieInit
 *  initialize a text-tree structure
 *
 *  "accept" [ IN ] -  a character-set string where each character
 *  represents itself. the only exception is the '-' character,
 *  which - when sandwiched between two other characters - is
 *  interpreted en tot as a range expression. an example would be
 *  "-0-9" is identical to "-0123456789"
 *
 *  "limit" [ IN ] - sets the number of values to be accumulated in
 *  any container. if this limit would be exceeded by any insertion,
 *  the node is expanded and its values are re-distributed among
 *  its children.
 *
 *  "cs_expand" [ IN ] - if true, then any valid characters observed
 *  during insertions are automatically added to the accept character
 *  set. doing so will ensure that there are no ambiguous transitions.
 *
 *  returns status codes:
 *    EINVAL => an invalid parameter
 *    ENOMEM => failed to allocate internal structures
 */
KLIB_EXTERN rc_t CC TrieInit ( Trie *tt, const char *accept,
    uint32_t limit, bool cs_expand );

/* TrieInsert
 *  inserts an item into tree
 *
 *  "item" [ IN ] -  a TNode forming the { key, value } pair.
 *
 *  returns status codes:
 *    EINVAL => an invalid parameter
 *    ENOMEM => failed to allocate internal structures
 */
KLIB_EXTERN rc_t CC TrieInsert ( Trie *self, TNode *item );

/* TrieInsertUnique
 *  behaves like TrieInsert if the key does not already have an
 *  associated value. fails otherwise, returning the existing node.
 *
 *  "item" [ IN ] - { key, value } pair
 *
 *  "exist" [ OUT, NULL OKAY ] - return parameter for existing node
 *  if insert failed due to unique constraint. NULL otherwise.
 *
 *  returns status codes:
 *    EINVAL => an invalid parameter
 *    EEXIST => the key is already bound to a value
 *    ENOMEM => failed to allocate internal structures
 */
KLIB_EXTERN rc_t CC TrieInsertUnique ( Trie *self, TNode *item, TNode **exist );

/* TrieUnlink
 *  remove an object from the tree
 *
 *  "item" [ IN ] - an actual TNode presumed to be held within tree.
 *
 *  return value:
 *    true  => node belonged to tree and was removed
 *    false => node does not belong to tree
 */
KLIB_EXTERN bool CC TrieUnlink ( Trie *self, TNode *item );

/* TrieValidate
 *  run validation check on tree structure
 *
 *  return status codes:
 *    ?
 *
 * TEMPORARY
 */
#if 0
KLIB_EXTERN int CC TrieValidate ( const Trie *self );
#endif

/* TrieFind
 * TrieFindRE
 *  find a single object within tree
 *
 *  "key" [ IN ] - an exact match text string
 *
 *  "re" [ IN ] - a regular expression string
 *
 *  return value:
 *    NULL    => no match
 *    TNode* => arbitrarily chosen matching node
 */
KLIB_EXTERN TNode* CC TrieFind ( const Trie *self, const String *key );
#if 0
KLIB_EXTERN TNode* CC TrieFindRE ( const Trie *self, const String *re );
#endif

/* TrieFindAll
 * TrieFindAllRE
 *  find multiple objects within tree
 *
 *  "key" [ IN ] - an exact match text string
 *
 *  "re" [ IN ] - a regular expression string
 *
 *  "buffer" [ OUT ] and "capacity" [ IN ] - a user-supplied
 *  array of TNode* with a capacity of "capacity" elements.
 *  if successful, the entire found set will be returned unordered within.
 *
 *  "num_found" [ OUT ] - indicates the size of the found set,
 *  regardless of return value, such that if the supplied
 *  buffer were to be too small, the required size is returned.
 *
 *  returns status codes:
 *    EINVAL  => an invalid parameter
 *    ENOENT  => the found set was empty
 *    ENOBUFS => the found set was too large
 */
KLIB_EXTERN rc_t CC TrieFindAll ( const Trie *self, const String *key,
    TNode *buffer [], uint32_t capacity, uint32_t *num_found );
#if 0
KLIB_EXTERN rc_t CC TrieFindAllRE ( const Trie *self, const String *re,
    TNode *buffer [], uint32_t capacity, uint32_t *num_found );
#endif

/* TrieForEach
 *  executes a function on each tree element
 *
 *  "f" [ IN ] and "data" [ IN ] - iteration callback function for
 *  examining each TNode in the tree
 */
KLIB_EXTERN void CC TrieForEach ( const Trie *self,
    void ( CC * f ) ( TNode *n, void *data ), void *data );

/* TrieDoUntil
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
KLIB_EXTERN bool CC TrieDoUntil ( const Trie *self,
    bool ( CC * f ) ( TNode *n, void *data ), void *data );

/* TrieExplore
 *  executes a function on each element in the path of the key
 *  from furthest node to closest node
 *  until the function returns true or end of chain
 */
KLIB_EXTERN bool CC TrieExplore ( const Trie *self, const String *key,
    bool ( CC * f ) ( TNode *n, void *data ), void *data );

/* TrieWhack
 *  tears down internal structure
 *
 *  "whack" [ IN, NULL OKAY ] and "data" [ IN ] - optional function
 *  to be invoked on TNode objects contained within. if NULL, the
 *  nodes will be whacked via "TNodeWhack"
 */
KLIB_EXTERN void CC TrieWhack ( Trie *self,
    void ( CC * whack ) ( TNode *n, void *data ), void *data );

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_trie_ */
