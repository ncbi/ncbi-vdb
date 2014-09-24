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

#ifndef _h_klib_pbstree_
#define _h_klib_pbstree_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct BSTree;


/*--------------------------------------------------------------------------
 * PBSTNode
 *  identifies a node within persisted binary search tree
 *
 *  a BSTree will contain BSTNodes, which themselves are intrusive but
 *  irrelevant internal tree links plus an externally defined data
 *  structure, having both key and value, and supporting multiple
 *  keys per node. the internal links permit navigation from node
 *  to node that is not possible with the PBSTNode.
 *
 *  a PBSTree does not contain structured nodes, but stores linkage
 *  and ordering information separately from the externally defined
 *  data, and rather than using pointers, assigns integer ids to the
 *  nodes themselves. navigation by pointers would require allocations,
 *  which are unnecessary and expensive when traversing a read-only
 *  persisted image. navigation is therefore intrusive on an externally
 *  allocated node structure.
 */
typedef struct PBSTNode PBSTNode;
struct PBSTNode
{
    struct
    {
        const void *addr;
        size_t size;

    } data;

    const void *internal;
    uint32_t id;
};

/* PBSTNodeNext
 *  updates the structure
 *  returns next 1-based node id or 0 for NULL
 */
KLIB_EXTERN uint32_t CC PBSTNodeNext ( PBSTNode *self );

/* PBSTNodePrev
 *  updates the structure
 *  returns prev 1-based node id or 0 for NULL
 */
KLIB_EXTERN uint32_t CC PBSTNodePrev ( PBSTNode *self );

/* PBSTNodeFindNext
 *  find next element satisfying criteria
 *  fills out "n" if found
 *  returns 1-based node id or 0 for NULL
 */
KLIB_EXTERN uint32_t CC PBSTNodeFindNext ( PBSTNode *self,
     bool ( CC * f ) ( const PBSTNode *n ) );

/* PBSTNodeFindPrev
 *  find previous element satisfying criteria
 *  fills out "n" if found
 *  returns 1-based node id or 0 for NULL
 */
KLIB_EXTERN uint32_t CC PBSTNodeFindPrev ( PBSTNode *self,
    bool ( CC * f ) ( const PBSTNode *n ) );


/*--------------------------------------------------------------------------
 * PBSTree
 *  a flattened binary search tree
 *
 *  mimics read-only behavior of a BSTree
 */
typedef struct PBSTree PBSTree;

/* PBSTreeMake
 *  make a PBSTree structure
 *
 *  "mem" [ IN ] - constant memory image of persisted tree
 *  with a lifetime exceeding that of the PBSTree itself
 *
 *  "byteswap" [ IN ] - if true, the persisted image needs
 *  to be read with byteswapping
 */
KLIB_EXTERN rc_t CC PBSTreeMake ( PBSTree **pt, const void *addr, size_t size, bool byteswap );

/* PBSTreeCount
 *  returns number of elements in tree
 *  not included within the BSTree interface itself, but
 *  was included here due to the fact that it is constant
 *
 *  return value:
 *    integer value >= 0
 */
KLIB_EXTERN uint32_t CC PBSTreeCount ( const PBSTree *self );

/* PBSTreeDepth
 *  returns number of layers in tree
 *
 *  return value:
 *    integer value >= 0
 */
KLIB_EXTERN uint32_t CC PBSTreeDepth ( const PBSTree *self );

/* PBSTreeSize
 *  returns the size in bytes
 *  of the PBSTree image
 */
KLIB_EXTERN size_t CC PBSTreeSize ( const PBSTree *self );

/* PBSTreeGetNode
 *  gets a PBSTNode from an id
 *
 *  "node" [ OUT ] - return parameter for node
 *
 *  "id" [ IN ] - a 1-based integer node id
 *
 *  return values:
 *    EINVAL => an invalid parameter was passed
 *    ENOENT => id out of range
 */
KLIB_EXTERN rc_t CC PBSTreeGetNode ( const PBSTree *self, PBSTNode *node, uint32_t id );

/* PBSTreeFind
 *  find an object within tree
 *
 *  "rtn" [ OUT ] - return parameter for node if found. its value
 *  is undefined unless the function returns success.
 *
 *  "item" [ IN ] - item to be matched against a node. will be
 *  supplied as the first parameter to the comparison function.
 *
 *  "cmp" [ IN ] - function that evaluates "item" against each internal
 *  node for relative position, returning 0 for match, negative when
 *  "item" is to left of node, and positive otherwise.
 *
 *  return value:
 *    0    => not found
 *    1..n => internal id of node, also recorded within "rtn"
 */
KLIB_EXTERN uint32_t CC PBSTreeFind ( const PBSTree *self, PBSTNode *rtn,
    const void *item, int ( CC * cmp ) ( const void *item, const PBSTNode *n , void * data), void * data );

/* PBSTreeForEach
 *  executes a function on each tree element
 *
 *  "reverse" [ IN ] - if true, traverse from last to first element,
 *  if false, traverse in the normal forward direction.
 *
 *  "f" [ IN ] and "data" [ IN ] - callback function for evaluating each
 *  node within the tree. the passed out node structure is itself fully
 *  modifiable.
 */
KLIB_EXTERN void CC PBSTreeForEach ( const PBSTree *self, bool reverse,
    void ( CC * f ) ( PBSTNode *n, void *data ), void *data );

/* PBSTreeDoUntil
 *  executes a function on each element
 *  until the function returns true
 *
 *  "reverse" [ IN ] - if true, traverse from last to first element,
 *  if false, traverse in the normal forward direction.
 *
 *  "f" [ IN ] and "data" [ IN ] - callback function for evaluating each
 *  node within the tree. the passed out node structure is itself fully
 *  modifiable. the function returns "true" to halt iteration.
 *
 *  return values:
 *    the last value returned by "f" or false if never invoked
 */
KLIB_EXTERN bool CC PBSTreeDoUntil ( const PBSTree *self, bool reverse,
    bool ( CC * f ) ( PBSTNode *n, void *data ), void *data );

/* PBSTreeWhack
 *  whacks PBSTree object
 *  the constant memory image used to create the PBSTree may now be released
 */
KLIB_EXTERN void CC PBSTreeWhack ( PBSTree *self );



/*--------------------------------------------------------------------------
 * persistence functions
 */

/* PTWriteFunc
 *  a generic streaming function
 */
typedef rc_t ( CC * PTWriteFunc )
    ( void *param, const void *buffer, size_t bytes, size_t *num_writ );

/* PTAuxFunc
 *  a function to measure or write auxiliary node data
 *  where "node" is a BSTNode or TTNode.
 *
 *  when "write" is NULL, the number of bytes that would
 *  be written is returned in "num_writ".
 */
typedef rc_t ( CC * PTAuxFunc )
    ( void *param, const void *node, size_t *num_writ,
      PTWriteFunc write, void *write_param );


/*--------------------------------------------------------------------------
 * BSTree
 */

/* BSTreePersist
 *  write a binary search tree to some storage location
 *
 *  the tree is persisted by making between one and three passes
 *  over its nodes, see description of "write" parameter.
 *
 *  the first pass examines internal tree structure and invokes
 *  a user-supplied function to determine overall size.
 *
 *  the second pass persists the internal structure in a packed
 *  format, using the user-supplied generic "write" function.
 *
 *  the third pass invokes another user-supplied function to write
 *  auxiliary node data to output.
 *
 *  "num_writ" [ OUT, NULL OKAY ] -  returns parameter for the number
 *  of bytes written as a result of persisting the tree. this will
 *  be the actual bytes written regardless of return status.
 *
 *  "write" [ IN, NULL OKAY ]  and "write_param" [ IN ] -  a generic
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
KLIB_EXTERN rc_t CC BSTreePersist ( struct BSTree const *self, size_t *num_writ,
    PTWriteFunc write, void *write_param, PTAuxFunc aux, void *aux_param );

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_pbstree_ */
