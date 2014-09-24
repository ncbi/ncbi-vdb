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

#ifndef _h_trie_priv_
#define _h_trie_priv_

#ifndef _h_klib_trie_
#include <klib/trie.h>
#endif


/*--------------------------------------------------------------------------
 * TTrans
 *  a text transition node
 */
typedef struct TTrans TTrans;
struct TTrans
{
    /* child nodes */
    TTrans **child;

    /* values associated with the node */
    BSTree vals;

    /* the depth occupied within tree */
    uint32_t depth;

    /* number of transitions in array */
    uint16_t tcnt;

    /* number of values in b-tree */
    uint16_t vcnt;
};

/* TTransBacktrace
 *  describes a TTrans
 *  with backtrace pointer to parent and transition index from parent
 */
typedef struct TTransBacktrace TTransBacktrace;
struct TTransBacktrace
{
    const TTransBacktrace *par;
    const TTrans *trans;
    uint32_t idx;
};

/* TTransForEach
 * TTransForEachBT
 */
void TTransForEach ( const TTrans *trans, uint32_t width,
    void ( CC * f ) ( const TTransBacktrace *bt, uint32_t width, void *data ), void *data );
void TTransForEachBT ( const TTransBacktrace *bt, uint32_t width,
    void ( CC * f ) ( const TTransBacktrace *bt, uint32_t width, void *data ), void *data );

/* TTransDoUntil
 * TTransDoUntilBT
 */
bool TTransDoUntil ( const TTrans *trans, uint32_t width,
    bool ( CC * f ) ( const TTransBacktrace *bt, uint32_t width, void *data ), void *data );
bool TTransDoUntilBT ( const TTransBacktrace *bt, uint32_t width,
    bool ( CC * f ) ( const TTransBacktrace *bt, uint32_t width, void *data ), void *data );

/* TTransValidate
 *  recursively checks structure
 */
int TTransValidate ( const TTrans *trans, const Trie *tt );


/*--------------------------------------------------------------------------
 * TTSearchNode
 *  this is a complicated guy.
 *
 *  if the only values allowed in a TTrans were terminal values,
 *  then the natural structure would imply a TTrans node for every
 *  path and a child array to hold children. the tree would be
 *  simple to navigate; searching would return TTrans nodes.
 *
 *  however, the structure abandons child arrays when the count
 *  is low enough, and allows non-terminal values in the value
 *  tree. a string may still terminate on a TTrans, but necessarily
 *  there will be strings that terminate in space.
 *
 *  to handle this, as well as to address non-intrusive list
 *  functionality, this search node is able to do too many things.
 *
 *  1) it allows a poor version of a child array to be added
 *     to an existing TTrans.
 *
 *  2) it defines a fake TTrans object with its own depth
 *     and value list.
 *
 *  3) the fake TTrans can itself be exploded to contain
 *     further fake TTrans objects
 *
 *  4) it functions as a non-intrusive reference to a TTNode
 *     for inclusion into temporary child lists.
 */
enum
{
    snTrans,
    snKeynode,
    snTransEx,
    snFakeTrans,
    snFakeTransEx
};

typedef struct TTSearchNode TTSearchNode;
struct TTSearchNode
{
    union
    {
        /* snTrans, snTransEx */
        struct
        {
            const TTrans *ref;
            DLList vals;
            DLList child;

        } trans;

        /* snKeynode */
        struct
        {
            DLNode n;
            const TNode *ref;

        } keynode;

        /* snFakeTrans, snFakeTransEx */
        struct
        {
            DLNode n;
            DLList vals;
            DLList child;
            uint32_t depth;
            uint32_t idx;

        } fake;

    } u;

    uint32_t refcount;
    uint32_t variant;
};

/* TTSearchNodeMakeTrans
 *  creates a reference node
 */
rc_t TTSearchNodeMakeTrans ( TTSearchNode **s, const TTrans *trans );

/* TTSearchNodeExplodeTrans
 *  explodes a simple TTrans to split values into term and non-term
 *  may create fake trans objects within "child"
 */
rc_t TTSearchNodeExplodeTrans ( TTSearchNode *s, const Trie *tt );

/* TTSearchNodeExplodeFakeTrans
 *  explodes a fake trans to split values into term and non-term
 *  may create fake trans objects within "child"
 */
rc_t TTSearchNodeExplodeFakeTrans ( TTSearchNode *s, const Trie *tt );

/* TTSearchNodeRelease
 *  releases a reference, and potentially the object
 */
void TTSearchNodeRelease ( TTSearchNode *s );

/* TTSearchNodeReuseTrans
 *  if a node has no other references,
 *  destructs and re-initializes the node for reuse.
 *
 *  otherwise, a new node is created.
 */
rc_t TTSearchNodeReuseTrans ( TTSearchNode **s, const TTrans *trans );


/*--------------------------------------------------------------------------
 * TTSearchNodeRef
 */
typedef struct TTSearchNodeRef TTSearchNodeRef;
struct TTSearchNodeRef
{
    DLNode n;
    TTSearchNode *s;
};

/* TTSearchNodeRefMake
 */
rc_t TTSearchNodeRefMake ( TTSearchNodeRef **ref, TTSearchNode *s );

/* TTSearchNodeRefWhack
 * TTSearchNodeRefWhackAll
 */
void TTSearchNodeRefWhack ( TTSearchNodeRef *p );
void TTSearchNodeRefWhackAll ( DLNode *n, void *ignore );


/*--------------------------------------------------------------------------
 * Trie
 */

/* TrieMapChar
 *  maps a character to an index
 */
uint32_t TrieMapChar ( const Trie *tt, uint32_t ch );

/* TrieNextIdx
 *  pops a character from key string
 *  maps it to an index
 */
rc_t TrieNextIdx ( const Trie *tt, String *key, uint32_t *idx );


#endif /* _h_trie_priv_ */
