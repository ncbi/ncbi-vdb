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

#ifndef _h_klib_container_
#define _h_klib_container_

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
 * SLNode
 *  singly linked node
 */
typedef struct SLNode SLNode;
struct SLNode
{
    SLNode *next;
};

/* SLNodeNext
 *  returns next node
 */
#define SLNodeNext( n ) \
    ( n ) -> next

#if 0
/* SLNodeFindNext
 *  find next element satisfying criteria
 */
KLIB_EXTERN SLNode* CC SLNodeFindNext ( const SLNode *n, bool ( CC * f ) ( const SLNode *n ) );
#endif


/*--------------------------------------------------------------------------
 * SLList
 *  singly linked list
 */
typedef struct SLList SLList;
struct SLList
{
    SLNode *head;
    SLNode *tail;
};


/* SLListInit
 *  initialize a singly linked list
 */
#define SLListInit( sl ) \
    ( void ) ( ( sl ) -> head = ( sl ) -> tail = NULL )

/* SLListHead
 *  returns list head
 */
#define SLListHead( sl ) \
    ( sl ) -> head

/* SLListTail
 *  returns list tail
 */
#define SLListTail( sl ) \
    ( sl ) -> tail

/* SLListPushHead
 *  push a single node onto head of list
 */
#define SLListPushHead( sl, n ) \
    ( void ) ( ( ( sl ) -> tail == NULL ? \
        ( void ) ( ( sl ) -> tail = ( n ) ) : ( void ) 0 ), \
        ( n ) -> next = ( sl ) -> head, ( sl ) -> head = ( n ) )

/* SLListPushTail
 *  push a single node onto tail of list
 */
KLIB_EXTERN void CC SLListPushTail ( SLList *sl, SLNode *n );

/* SLListPopHead
 *  pop a single node from head of list
 */
KLIB_EXTERN SLNode* CC SLListPopHead ( SLList *sl );

/* SLListPopTail
 *  pop a single node from tail of list
 */
KLIB_EXTERN SLNode* CC SLListPopTail ( SLList *sl );

/* SLListUnlink
 *  removes a designated node from list
 */
KLIB_EXTERN void CC SLListUnlink ( SLList *sl, SLNode *n );

/* SLListForEach
 *  executes a function on each list element
 */
KLIB_EXTERN void CC SLListForEach ( const SLList *sl,
    void ( CC * f ) ( SLNode *n, void *data ), void *data );

/* SLListDoUntil
 *  executes a function on each element
 *  until the function returns true
 */
KLIB_EXTERN bool CC SLListDoUntil ( const SLList *sl,
    bool ( CC * f ) ( SLNode *n, void *data ), void *data );

/* SLListFindFirst
 *  find first element satisfying criteria
 */
KLIB_EXTERN SLNode* CC SLListFindFirst ( const SLList *sl, bool ( CC * f ) ( const SLNode *n ) );

/* SLListWhack
 *  pops elements from list and
 *  executes a user provided destructor
 */
KLIB_EXTERN void CC SLListWhack ( SLList *sl, void ( CC * whack ) ( SLNode *n, void *data ), void *data );


/*--------------------------------------------------------------------------
 * DLNode
 *  doubly linked node
 */
typedef struct DLNode DLNode;
struct DLNode
{
    DLNode *next;
    DLNode *prev;
};

/* DLNodeNext
 *  returns next node
 */
#define DLNodeNext( n ) \
    ( n ) -> next

/* DLNodePrev
 *  returns prev node
 */
#define DLNodePrev( n ) \
    ( n ) -> prev

#if 0
/* DLNodeFindNext
 *  find next element satisfying criteria
 */
KLIB_EXTERN DLNode* CC DLNodeFindNext ( const DLNode *n, bool ( CC * f ) ( const DLNode *n ) );

/* DLNodeFindPrev
 *  find previous element satisfying criteria
 */
KLIB_EXTERN DLNode* CC DLNodeFindPrev ( const DLNode *n, bool ( CC * f ) ( const DLNode *n ) );
#endif

/*--------------------------------------------------------------------------
 * DLList
 *  doubly linked list
 */
typedef struct DLList DLList;
struct DLList
{
    DLNode *head;
    DLNode *tail;
};

/* DLListInit
 *  initialize a doubly linked list
 */
#define DLListInit( dl ) \
    ( void ) ( ( dl ) -> head = ( dl ) -> tail = NULL )

/* DLListHead
 *  returns list head
 */
#define DLListHead( dl ) \
    ( dl ) -> head

/* DLListTail
 *  returns list tail
 */
#define DLListTail( dl ) \
    ( dl ) -> tail

/* DLListPushHead
 *  push a single node onto the head of list
 */
KLIB_EXTERN void CC DLListPushHead ( DLList *dl, DLNode *n );

/* DLListPushTail
 *  push a single node onto the tail of list
 */
KLIB_EXTERN void CC DLListPushTail ( DLList *dl, DLNode *n );

/* DLListPopHead
 *  pop a single node from head of list
 */
KLIB_EXTERN DLNode* CC DLListPopHead ( DLList *dl );

/* DLListPopTail
 *  pop a single node from tail of list
 */
KLIB_EXTERN DLNode* CC DLListPopTail ( DLList *dl );

/* DLListPrependList
 *  pushes list contents onto the head of target
 */
KLIB_EXTERN void CC DLListPrependList ( DLList *dl, DLList *l );

/* DLListAppendList
 *  pushes list contents onto the tail of target
 */
KLIB_EXTERN void CC DLListAppendList ( DLList *dl, DLList *l );

/* DLListInsertNodeBefore
 *  inserts node "n" before "which" within list
 */
KLIB_EXTERN void CC DLListInsertNodeBefore ( DLList *dl, DLNode *which, DLNode *n );

/* DLListInsertNodeAfter
 *  inserts node "n" after "which" within list
 */
KLIB_EXTERN void CC DLListInsertNodeAfter ( DLList *dl, DLNode *which, DLNode *n );

/* DLListInsertListBefore
 *  inserts list "l" before "which" within list "dl"
 */
KLIB_EXTERN void CC DLListInsertListBefore ( DLList *dl, DLNode *which, DLList *l );

/* DLListInsertListAfter
 *  inserts list "l" after "which" within list "dl"
 */
KLIB_EXTERN void CC DLListInsertListAfter ( DLList *dl, DLNode *which, DLList *l );

/* DLListUnlink
 *  removes a designated node from list
 */
KLIB_EXTERN void CC DLListUnlink ( DLList *dl, DLNode *n );

/* DLListForEach
 *  executes a function on each list element
 */
KLIB_EXTERN void CC DLListForEach ( const DLList *dl, bool reverse,
    void ( CC * f ) ( DLNode *n, void *data ), void *data );

/* DLListDoUntil
 *  executes a function on each element
 *  until the function returns true
 */
KLIB_EXTERN bool CC DLListDoUntil ( const DLList *dl, bool reverse,
    bool ( CC * f ) ( DLNode *n, void *data ), void *data );

/* DLListFindFirst
 *  find first element satisfying criteria
 */
KLIB_EXTERN DLNode* CC DLListFindFirst ( const DLList *dl, bool ( CC * f ) ( const DLNode *n ) );

/* DLListFindLast
 *  find last element satisfying criteria
 */
KLIB_EXTERN DLNode* CC DLListFindLast ( const DLList *dl, bool ( CC * f ) ( const DLNode *n ) );

/* DLListWhack
 *  pops elements from list and
 *  executes a user provided destructor
 */
KLIB_EXTERN void CC DLListWhack ( DLList *dl, void ( CC * whack ) ( DLNode *n, void *data ), void *data );


/*--------------------------------------------------------------------------
 * BSTNode
 *  binary search tree node
 */
typedef struct BSTNode BSTNode;
struct BSTNode
{
    BSTNode *par;
    BSTNode *child [ 2 ];
};

/* BSTNodeNext
 *  returns next node
 */
KLIB_EXTERN BSTNode* CC BSTNodeNext ( const BSTNode *n );

/* BSTNodePrev
 *  returns prev node
 */
KLIB_EXTERN BSTNode* CC BSTNodePrev ( const BSTNode *n );

/* BSTNodeParent
 *  returns a parent node if there, NULL otherwise
 */
KLIB_EXTERN BSTNode* CC BSTNodeParent ( const BSTNode *n );

/* BSTNodeFindNext
 *  find next element satisfying criteria
 */
KLIB_EXTERN BSTNode* CC BSTNodeFindNext ( const BSTNode *n, bool ( CC * f ) ( const BSTNode *n ) );

/* BSTNodeFindPrev
 *  find previous element satisfying criteria
 */
KLIB_EXTERN BSTNode* CC BSTNodeFindPrev ( const BSTNode *n, bool ( CC * f ) ( const BSTNode *n ) );


/*--------------------------------------------------------------------------
 * BSTree
 *  binary search tree
 */
typedef struct BSTree BSTree;
struct BSTree
{
    BSTNode *root;
};

/* BSTreeInit
 *  initialize tree
 */
#define BSTreeInit( bt ) \
    ( void ) ( ( bt ) -> root = NULL )

/* BSTreeDepth
 *  returns number of layers in tree
 *
 *  if "exact" is true, then the maximum
 *  depth is returned. otherwise, the depth of
 *  an arbitrary leaf node is returned
 */
KLIB_EXTERN uint32_t CC BSTreeDepth ( const BSTree *bt, bool exact );

/* BSTreeFirst
 *  returns first node
 */
KLIB_EXTERN BSTNode* CC BSTreeFirst ( const BSTree *bt );

/* BSTreeLast
 *  returns last node
 */
KLIB_EXTERN BSTNode* CC BSTreeLast ( const BSTree *bt );

/* BSTreeFind
 *  find an object within tree
 *  "cmp" function returns equivalent of "item" - "n"
 */
KLIB_EXTERN BSTNode* CC BSTreeFind ( const BSTree *bt, const void *item,
    int ( CC * cmp ) ( const void *item, const BSTNode *n ) );

/* BSTreeInsert
 *  insert an object within tree, even if duplicate
 *  "sort" function returns equivalent of "item" - "n"
 *
 *  the treatment of order for items reported as identical
 *  i.e. sort function returns zero when they are compared,
 *  is undefined.
 *
 *  the current implementation treats '<=' as '<' such
 *  that all inserts are converted to a '<' or '>' comparison,
 *  but this should not be relied upon.
 *
 *  returns 0 if insert succeeded or an OS error code otherwise.
 */
KLIB_EXTERN rc_t CC BSTreeInsert ( BSTree *bt, BSTNode *item,
    int ( CC * sort ) ( const BSTNode *item, const BSTNode *n ) );

/* BSTreeInsertUnique
 *  insert an object within tree, but only if unique.
 *  "sort" function returns equivalent of "item" - "n"
 *
 *  returns 0 if insertion succeeded. or an OS error code otherwise.
 *  if error code is EEXIST, the existing object is returned in "exist".
 */
KLIB_EXTERN rc_t CC BSTreeInsertUnique ( BSTree *bt, BSTNode *item, BSTNode **exist,
    int ( CC * sort ) ( const BSTNode *item, const BSTNode *n ) );

/* BSTreeResort
 *  an optimized removal and re-insertion of
 *  all contained elements using another function
 *
 *  the treatment of order for items reported as identical
 *  i.e. sort function returns zero when they are compared,
 *  is undefined.
 *
 *  the current implementation treats '<=' as '<' such
 *  that all inserts are converted to a '<' or '>' comparison,
 *  but this should not be relied upon.
 */
KLIB_EXTERN void CC BSTreeResort ( BSTree *bt,
    int ( CC * resort ) ( const BSTNode *item, const BSTNode *n ) );

/* BSTreeUnlink
 *  removes a node from tree
 *
 *  returns true if node was removed from tree
 *  false if it could not be removed, e.g. was not in tree
 */
KLIB_EXTERN bool CC BSTreeUnlink ( BSTree *bt, BSTNode *n );

/* BSTreeForEach
 *  executes a function on each tree element
 */
KLIB_EXTERN void CC BSTreeForEach ( const BSTree *bt, bool reverse,
    void ( CC * f ) ( BSTNode *n, void *data ), void *data );

/* BSTreeDoUntil
 *  executes a function on each element
 *  until the function returns true
 *
 *  return values:
 *    false unless the function returns true
 */
KLIB_EXTERN bool CC BSTreeDoUntil ( const BSTree *bt, bool reverse,
    bool ( CC * f ) ( BSTNode *n, void *data ), void *data );

/* BSTreeWhack
 *  removes nodes from tree and
 *  executes a user provided destructor
 */
KLIB_EXTERN void CC BSTreeWhack ( BSTree *bt, void ( CC * whack ) ( BSTNode *n, void *data ), void *data );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_container_ */
