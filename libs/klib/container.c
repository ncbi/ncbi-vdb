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

#include <klib/extern.h>
#include <klib/container.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * SLNode
 *  singly linked node
 */

#if 0
/* SLNodeFindNext
 *  find next element satisfying criteria
 */
LIB_EXPORT SLNode* CC SLNodeFindNext ( const SLNode *p, bool ( CC * f ) ( const SLNode *n ) )
{
    if ( p != NULL )
    {
        SLNode *n = p -> next;
        while ( n != NULL )
        {
            if ( ( * f ) ( n ) )
                return n;
            n = n -> next;
        }
    }
    return NULL;
}
#endif


/*--------------------------------------------------------------------------
 * SLList
 *  singly linked list
 */

/* SLListPushTail
 *  push a single node onto tail of list
 */
LIB_EXPORT void CC SLListPushTail ( SLList *sl, SLNode *n )
{
    if ( sl != NULL && n != NULL )
    {
        if ( sl -> tail == NULL )
            sl -> head = sl -> tail = n;
        else
        {
            sl -> tail -> next = n;
            sl -> tail = n;
        }
        n -> next = NULL;
    }
}

/* SLListPopHead
 *  pop a single node from head of list
 */
LIB_EXPORT SLNode* CC SLListPopHead ( SLList *sl )
{
    if ( sl != NULL )
    {
        SLNode *n = sl -> head;
        if ( n != NULL )
        {
            sl -> head = n -> next;
            if ( n -> next == NULL )
                sl -> tail = NULL;
        }
        return n;
    }
    return NULL;
}

/* SLListPopTail
 *  pop a single node from tail of list
 */
LIB_EXPORT SLNode* CC SLListPopTail ( SLList *sl )
{
    if ( sl != NULL )
    {
        SLNode *n = sl -> head;
        if ( n != NULL )
        {
            SLNode *tail = sl -> tail;
            if ( n == tail )
            {
                sl -> head = sl -> tail = NULL;
                return n;
            }
            while ( n -> next != tail )
                n = n -> next;
            sl -> tail = n;
            n -> next = NULL;
            return tail;
        }
    }
    return NULL;
}

/* SLListUnlink
 *  removes a designated node from list
 */
LIB_EXPORT void CC SLListUnlink ( SLList *sl, SLNode *n )
{
    if ( sl != NULL && n != NULL )
    {
        SLNode *p = sl -> head;
        if ( p == n )
        {
            sl -> head = p -> next;
            if ( p -> next == NULL )
                sl -> tail = NULL;
        }
        else while ( p != NULL )
        {
            if ( p -> next == n )
            {
                p -> next = n -> next;
                if ( n -> next == NULL )
                    sl -> tail = p;
                break;
            }
            p = p -> next;
        }
    }
}

/* SLListForEach
 *  executes a function on each list element
 */
LIB_EXPORT void CC SLListForEach ( const SLList *sl,
    void ( CC * f ) ( SLNode *n, void *data ), void *data )
{
    if ( sl != NULL )
    {
        SLNode *n = sl -> head;
        while ( n != NULL )
        {
            SLNode *next = n -> next;
            ( * f ) ( n, data );
            n = next;
        }
    }
}

/* SLListDoUntil
 *  executes a function on each element
 *  until the function returns true
 */
LIB_EXPORT bool CC SLListDoUntil ( const SLList *sl,
    bool ( CC * f ) ( SLNode *n, void *data ), void *data )
{
    if ( sl )
    {
        SLNode *n = sl -> head;
        while ( n != NULL )
        {
            SLNode *next = n -> next;
            if ( ( * f ) ( n, data ) )
                return true;
            n = next;
        }
    }

    return false;
}

/* SLListFindFirst
 *  find first element satisfying criteria
 */
LIB_EXPORT SLNode* CC SLListFindFirst ( const SLList *sl,
    bool ( CC * f ) ( const SLNode *n ) )
{
    if ( sl != NULL )
    {
        SLNode *n = sl -> head;
        while ( n != NULL )
        {
            SLNode *next = n -> next;
            if ( ( * f ) ( n ) )
                return n;
            n = next;
        }
    }
    return NULL;
}

/* SLListWhack
 *  pops elements from list and
 *  executes a user provided destructor
 */
LIB_EXPORT void CC SLListWhack ( SLList *sl, 
    void ( CC * whack ) ( SLNode *n, void *data ), void *data )
{
    if ( sl != NULL )
    {
        SLNode *n = sl -> head;
        sl -> head = sl -> tail = NULL;

        if ( whack != NULL )
        {
            while ( n != NULL )
            {
                SLNode *next = n -> next;
                ( * whack ) ( n, data );
                n = next;
            }
        }
    }
}


/*--------------------------------------------------------------------------
 * DLNode
 *  doubly linked node
 */

#if 0
/* DLNodeFindNext
 *  find next element satisfying criteria
 */
LIB_EXPORT DLNode* CC DLNodeFindNext ( const DLNode *p, 
    bool ( CC * f ) ( const DLNode *n ) )
{
    if ( p != NULL )
    {
        DLNode *n = p -> next;
        while ( n != NULL )
        {
            if ( ( * f ) ( n ) )
                return n;
            n = n -> next;
        }
    }
    return NULL;
}

/* DLNodeFindPrev
 *  find previous element satisfying criteria
 */
LIB_EXPORT DLNode* CC DLNodeFindPrev ( const DLNode *p,
    bool ( CC * f ) ( const DLNode *n ) )
{
    if ( p != NULL )
    {
        DLNode *n = p -> prev;
        while ( n != NULL )
        {
            if ( ( * f ) ( n ) )
                return n;
            n = n -> prev;
        }
    }
    return NULL;
}
#endif


/*--------------------------------------------------------------------------
 * DLList
 *  doubly linked list
 */

/* DLListPushHead
 *  push a single node onto the head of list
 */
LIB_EXPORT void CC DLListPushHead ( DLList *dl, DLNode *n )
{
    if ( dl != NULL && n != NULL )
    {
        n -> prev = NULL;
        n -> next = dl -> head;
        if ( dl -> head == NULL )
            dl -> head = dl -> tail = n;
        else
        {
            dl -> head -> prev = n;
            dl -> head = n;
        }
    }
}

/* DLListPushTail
 *  push a single node onto the tail of list
 */
LIB_EXPORT void CC DLListPushTail ( DLList *dl, DLNode *n )
{
    if ( dl != NULL && n != NULL )
    {
        n -> next = NULL;
        n -> prev = dl -> tail;
        if ( dl -> tail == NULL )
            dl -> tail = dl -> head = n;
        else
        {
            dl -> tail -> next = n;
            dl -> tail = n;
        }
    }
}

/* DLListPopHead
 *  pop a single node from head of list
 */
LIB_EXPORT DLNode* CC DLListPopHead ( DLList *dl )
{
    if ( dl != NULL )
    {
        DLNode *n = dl -> head;
        if ( dl -> head != NULL )
        {
            dl -> head = n -> next;
            if ( n -> next == NULL )
                dl -> tail = NULL;
            else
                n -> next -> prev = NULL;
        }
        return n;
    }
    return NULL;
}

/* DLListPopTail
 *  pop a single node from tail of list
 */
LIB_EXPORT DLNode* CC DLListPopTail ( DLList *dl )
{
    if ( dl != NULL )
    {
        DLNode *n = dl -> tail;
        if ( dl -> tail != NULL )
        {
            dl -> tail = n -> prev;
            if ( n -> prev == NULL )
                dl -> head = NULL;
            else
                n -> prev -> next = NULL;
        }
        return n;
    }
    return NULL;
}

/* DLListPrependList
 *  pushes list contents onto the head of target
 */
LIB_EXPORT void CC DLListPrependList ( DLList *dl, DLList *l )
{
    if ( dl != NULL && l != NULL && l -> head != NULL )
    {
        if ( dl -> tail == NULL )
            * dl = * l;
        else
        {
            dl -> head -> prev = l -> tail;
            l -> tail -> next = dl -> head;
            dl -> head = l -> head;
        }

        l -> head = l -> tail = NULL;
    }
}

/* DLListAppendList
 *  pushes list contents onto the tail of target
 */
LIB_EXPORT void CC DLListAppendList ( DLList *dl, DLList *l )
{
    if ( dl != NULL && l != NULL && l -> head != NULL )
    {
        if ( dl -> tail == NULL )
            * dl = * l;
        else
        {
            dl -> tail -> next = l -> head;
            l -> head -> prev = dl -> tail;
            dl -> tail = l -> tail;
        }

        l -> head = l -> tail = NULL;
    }
}

/* DLListInsertNodeBefore
 *  inserts node "n" before "which" within list
 */
LIB_EXPORT void CC DLListInsertNodeBefore ( DLList *dl, DLNode *which, DLNode *n )
{
    if ( which != NULL && n != NULL )
    {
        /* take care of "n" */
        n -> next = which;
        n -> prev = which -> prev;

        /* link "which"'s prev to "n" */
        if ( which -> prev != NULL )
            which -> prev -> next = n;

        /* or if none, then perhaps head of list */
        else if ( dl != NULL && dl -> head == which )
            dl -> head = n;

        /* link "which" to "n" */
        which -> prev = n;
    }
}

/* DLListInsertNodeAfter
 *  inserts node "n" after "which" within list
 */
LIB_EXPORT void CC DLListInsertNodeAfter ( DLList *dl, DLNode *which, DLNode *n )
{
    if ( which != NULL && n != NULL )
    {
        /* take care of "n" */
        n -> prev = which;
        n -> next = which -> next;

        /* link "which"'s next to "n" */
        if ( which -> next != NULL )
            which -> next -> prev = n;

        /* or if none, then perhaps tail of list */
        else if ( dl != NULL && dl -> tail == which )
            dl -> tail = n;

        /* link "which" to "n" */
        which -> next = n;
    }
}

/* DLListInsertListBefore
 *  inserts list "l" before "which" within list "dl"
 */
LIB_EXPORT void CC DLListInsertListBefore ( DLList *dl, DLNode *which, DLList *l )
{
    if ( which != NULL && l != NULL && l -> head != NULL )
    {
        /* take care of inserting list */
        l -> tail -> next = which;
        l -> head -> prev = which -> prev;

        /* link "which"'s prev to "l -> head" */
        if ( which -> prev != NULL )
            which -> prev -> next = l -> head;

        /* or if none, then perhaps head of list */
        else if ( dl != NULL && dl -> head == which )
            dl -> head = l -> head;

        /* link "which" to "l -> tail" */
        which -> prev = l -> tail;

        /* remove items from "l" */
        l -> head = l -> tail = NULL;
    }
}

/* DLListInsertListAfter
 *  inserts list "l" after "which" within list "dl"
 */
LIB_EXPORT void CC DLListInsertListAfter ( DLList *dl, DLNode *which, DLList *l )
{
    if ( which != NULL && l != NULL && l -> head != NULL )
    {
        /* take care of inserting list */
        l -> head -> prev = which;
        l -> tail -> next = which -> next;

        /* link "which"'s next to "l -> tail" */
        if ( which -> next != NULL )
            which -> next -> prev = l -> tail;

        /* or if none, then perhaps tail of list */
        else if ( dl != NULL && dl -> tail == which )
            dl -> head = l -> tail;

        /* link "which" to "l -> head" */
        which -> next = l -> head;

        /* remove items from "l" */
        l -> head = l -> tail = NULL;
    }
}

/* DLListUnlink
 *  removes a designated node from list
 */
LIB_EXPORT void CC DLListUnlink ( DLList *dl, DLNode *n )
{
    if ( n != NULL )
    {
        if ( n -> next == NULL )
        {
            if ( dl != NULL && dl -> tail == n )
            {
                if ( n -> prev == NULL )
                    dl -> head = dl -> tail = NULL;
                else
                {
                    n -> prev -> next = NULL;
                    dl -> tail = n -> prev;
                }
            }
            else
            {
                if ( n -> prev != NULL )
                    n -> prev -> next = NULL;
            }
        }
        else if ( n -> prev == NULL )
        {
            n -> next -> prev = NULL;
            if ( dl != NULL && dl -> head == n )
                dl -> head = n -> next;
        }
        else
        {
            n -> next -> prev = n -> prev;
            n -> prev -> next = n -> next;
        }
    }
}

/* DLListForEach
 *  executes a function on each list element
 */
LIB_EXPORT void CC DLListForEach ( const DLList *dl, bool reverse,
    void ( CC * f ) ( DLNode *n, void *data ), void *data )
{
    if ( dl != NULL )
    {
        DLNode *n, *next;
        if ( reverse )
        {
            n = dl -> tail;
            while ( n != NULL )
            {
                next = n -> prev;
                ( * f ) ( n, data );
                n = next;
            }
        }
        else
        {
            n = dl -> head;
            while ( n != NULL )
            {
                next = n -> next;
                ( * f ) ( n, data );
                n = next;
            }
        }
    }
}

/* DLListDoUntil
 *  executes a function on each element
 *  until the function returns 1
 */
LIB_EXPORT bool CC DLListDoUntil ( const DLList *dl, bool reverse,
    bool ( CC * f ) ( DLNode *n, void *data ), void *data )
{
    if ( dl != NULL )
    {
        DLNode *n, *next;
        if ( reverse )
        {
            n = dl -> tail;
            while ( n != NULL )
            {
                next = n -> prev;
                if ( ( * f ) ( n, data ) )
                    return true;
                n = next;
            }
        }
        else
        {
            n = dl -> head;
            while ( n != NULL )
            {
                next = n -> next;
                if ( ( * f ) ( n, data ) )
                    return true;
                n = next;
            }
        }
    }
    return false;
}

/* DLListFindFirst
 *  find first element satisfying criteria
 */
LIB_EXPORT DLNode* CC DLListFindFirst ( const DLList *dl,
    bool ( CC * f ) ( const DLNode *n ) )
{
    if ( dl != NULL )
    {
        DLNode *n = dl -> head;
        while ( n != NULL )
        {
            if ( ( * f ) ( n ) )
                return n;
            n = n -> next;
        }
    }
    return NULL;
}

/* DLListFindLast
 *  find last element satisfying criteria
 */
LIB_EXPORT DLNode* CC DLListFindLast ( const DLList *dl,
    bool ( CC * f ) ( const DLNode *n ) )
{
    if ( dl != NULL )
    {
        DLNode *n = dl -> tail;
        while ( n != NULL )
        {
            if ( ( * f ) ( n ) )
                return n;
            n = n -> prev;
        }
    }
    return NULL;
}

/* DLListWhack
 *  pops elements from list and
 *  executes a user provided destructor
 */
LIB_EXPORT void CC DLListWhack ( DLList *dl,
    void ( CC * whack ) ( DLNode *n, void *data ), void *data )
{
    if ( dl != NULL )
    {
        DLNode *n = dl -> head;
        dl -> head = dl -> tail = NULL;

        if ( whack != NULL )
        {
            while ( n != NULL )
            {
                DLNode *next = n -> next;
                ( * whack ) ( n, data );
                n = next;
            }
        }
    }
}


/*--------------------------------------------------------------------------
 * BSTNode
 *  b-tree node
 */

#define LEFT 1
#define RIGHT 2

#define BALANCE( node ) \
    ( ( size_t ) ( node ) -> par & 3 )
#define ZERO_BALANCE( node ) \
    ( * ( size_t* ) & ( node ) -> par &= ~ ( size_t ) 3 )
#define CLR_BALANCE( node, bal ) \
    ( * ( size_t* ) & ( node ) -> par ^= ( bal ) )
#define SET_BALANCE( node, bal ) \
    ( * ( size_t* ) & ( node ) -> par |= ( bal ) )
#define LEFT_HEAVY( node ) \
    ( ( ( size_t ) ( node ) -> par & LEFT ) != 0 )
#define RIGHT_HEAVY( node ) \
    ( ( ( size_t ) ( node ) -> par & RIGHT ) != 0 )

#define PMASK 3
#define BBITS( node, bal ) ( bal )

#define PBITS( node ) \
    ( ( size_t ) ( node ) -> par & PMASK )
#define PARENT( node ) \
    ( BSTNode* ) ( ( size_t ) ( node ) -> par & ~ ( size_t ) PMASK )
#define SET_PARENT( node, p ) \
    ( ( node ) -> par = ( BSTNode* ) ( ( size_t ) ( p ) | PBITS ( node ) ) )
#define SET_PARBAL( node, p, bal ) \
    ( ( node ) -> par = ( BSTNode* ) ( ( size_t ) ( p ) | BBITS ( node, bal ) ) )


/* LeftMost
 *  returns the left-most child
 */
static
BSTNode* CC LeftMost ( BSTNode *q )
{
    if ( q != NULL )
    {
        BSTNode *p = q -> child [ 0 ];
        while ( p != NULL )
        {
            q = p;
            p = p -> child [ 0 ];
        }
    }
    return q;
}

/* RightMost
 *  returns the right-most child
 */
static
BSTNode* CC RightMost ( BSTNode *q )
{
    if ( q != NULL )
    {
        BSTNode *p = q -> child [ 1 ];
        while ( p != NULL )
        {
            q = p;
            p = p -> child [ 1 ];
        }
    }
    return q;
}

/* FirstNode
 *  the left-most node in tree
 */
#define FirstNode( bt ) \
    LeftMost ( ( bt ) -> root )

/* LastNode
 *  the right-most node in tree
 */
#define LastNode( bt ) \
    RightMost ( ( bt ) -> root )

/* BSTNodeNext
 *  returns next node
 */
LIB_EXPORT BSTNode* CC BSTNodeNext ( const BSTNode *n )
{
    BSTNode *p;

    if ( n == NULL )
        return NULL;

    p = n -> child [ 1 ];
    if ( p == 0 )
    {
        BSTNode *q = ( BSTNode* ) n;
        while ( 1 )
        {
            p = PARENT ( q );
            if ( p == NULL )
                return NULL;
            if ( p -> child [ 0 ] == q )
                return p;
            q = p;
        }
    }
    return LeftMost ( p );
}

/* BSTNodePrev
 *  returns prev node
 */
LIB_EXPORT BSTNode* CC BSTNodePrev ( const BSTNode *n )
{
    BSTNode *p = n -> child [ 0 ];
    if ( p == 0 )
    {
        BSTNode *q = ( BSTNode* ) n;
        while ( 1 )
        {
            p = PARENT ( q );
            if ( p == NULL )
                return NULL;
            if ( p -> child [ 1 ] == q )
                return p;
            q = p;
        }
    }
    return RightMost ( p );
}

/* BSTNodeParent
 *  returns a parent node if there, NULL otherwise
 */
LIB_EXPORT BSTNode* CC BSTNodeParent ( const BSTNode *n )
{
    if ( n != NULL )
        return PARENT ( n );
    return NULL;
}

/* BSTNodeFindNext
 *  find next element satisfying criteria
 */
LIB_EXPORT BSTNode* CC BSTNodeFindNext ( const BSTNode *p,
    bool ( CC * f ) ( const BSTNode *n ) )
{
    if ( p != NULL )
    {
        BSTNode *n = BSTNodeNext ( p );
        while ( n != NULL )
        {
            if ( ( * f ) ( n ) )
                return n;
            n = BSTNodeNext ( n );
        }
    }
    return NULL;
}

/* BSTNodeFindPrev
 *  find previous element satisfying criteria
 */
LIB_EXPORT BSTNode* CC BSTNodeFindPrev ( const BSTNode *p,
    bool ( CC * f ) ( const BSTNode *n ) )
{
    if ( p != NULL )
    {
        BSTNode *n = BSTNodePrev ( p );
        while ( n != NULL )
        {
            if ( ( * f ) ( n ) )
                return n;
            n = BSTNodePrev ( n );
        }
    }
    return NULL;
}


/*--------------------------------------------------------------------------
 * BSTree
 *  b-tree
 */

/* BSTreeDepth
 *  returns number of layers in b-tree
 *
 *  if "exact" is 1, then the maximum
 *  depth is returned. otherwise, the depth of
 *  an arbitrary leaf node is returned
 */
LIB_EXPORT uint32_t CC BSTreeDepth ( const BSTree *bt, bool exact )
{
    BSTNode *p;
    uint32_t depth;

    if ( bt == NULL || bt -> root == NULL )
        return 0;

    depth = 1;

    if ( exact )
    {
        for ( p = FirstNode ( bt ); p != NULL; p = BSTNodeNext ( p ) )
        {
            BSTNode *q;
            unsigned int ndepth;

            if ( p -> child [ 0 ] != NULL || p -> child [ 1 ] != NULL )
                continue;

            for ( ndepth = 1, q = PARENT ( p ); q != NULL; q = PARENT ( q ) )
                ++ ndepth;

            if ( ndepth > depth )
                depth = ndepth;
        }
    }
    else
    {
        for ( p = bt -> root;; ++ depth )
        {
            if ( p -> child [ 0 ] != NULL )
                p = p -> child [ 0 ];
            else if ( p -> child [ 1 ] != NULL )
                p = p -> child [ 1 ];
            else
                break;
        }
    }

    return depth;
}

/* BSTreeFirst
 *  returns first node
 */
LIB_EXPORT BSTNode* CC BSTreeFirst ( const BSTree *bt )
{
    if ( bt == NULL )
        return NULL;
    return FirstNode ( bt );
}

/* BSTreeLast
 *  returns last node
 */
LIB_EXPORT BSTNode* CC BSTreeLast ( const BSTree *bt )
{
    if ( bt == NULL )
        return NULL;
    return LastNode ( bt );
}

/* BSTreeFind
 *  find an object within tree
 *  "cmp" function returns equivalent of "item" - "n"
 */
LIB_EXPORT BSTNode* CC BSTreeFind ( const BSTree *bt, const void *item,
    int ( CC * cmp ) ( const void *item, const BSTNode *n ) )
{
    if ( bt != NULL )
    {
        BSTNode *n = bt -> root;
        while ( n != NULL )
        {
            int diff = ( * cmp ) ( item, n );
            if ( diff == 0 )
                return n;
            n = n -> child [ diff > 0 ];
        }
    }
    return NULL;
}

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
 */
static
BSTNode* CC RotateRightAtY ( BSTNode *y, BSTNode *x )
{
    BSTNode *w = x;
    BSTNode *z = x -> child [ 1 ];
    y -> child [ 0 ] = z;
    x -> child [ 1 ] = y;
    x -> par = PARENT ( y );
    y -> par = x;

    /* patch parent link */
    if ( z != 0 )
        SET_PARENT ( z, y );

    return w;
}

static
BSTNode* CC RotateLeftAtY ( BSTNode *y, BSTNode *x )
{
    BSTNode *w = x;
    BSTNode *z = x -> child [ 0 ];
    y -> child [ 1 ] = z;
    x -> child [ 0 ] = y;
    x -> par = PARENT ( y );
    y -> par = x;

    /* patch parent link */
    if ( z != 0 )
        SET_PARENT ( z, y );

    return w;
}

static
BSTNode* CC RotateLeftAtXRightAtY ( BSTNode *y, BSTNode *x )
{
    BSTNode *w = x -> child [ 1 ];
    BSTNode *z = w -> child [ 0 ];
    x -> child [ 1 ] = z;
    if ( z != 0 )
        SET_PARENT ( z, x );
    z = w -> child [ 1 ];
    w -> child [ 0 ] = x;
    y -> child [ 0 ] = z;
    w -> child [ 1 ] = y;
        
    switch ( BALANCE ( w ) )
    {
    case 0:
        w -> par = PARENT ( y );
        x -> par = w;
        y -> par = w;
        break;
    case LEFT:
        w -> par = PARENT ( y );
        x -> par = w;
        SET_PARBAL ( y, w, RIGHT );
        break;
    case RIGHT:
        w -> par = PARENT ( y );
        SET_PARBAL ( x, w, LEFT );
        y -> par = w;
        break;
    }

    /* patch parent link */
    if ( z != 0 )
        SET_PARENT ( z, y );

    return w;
}

static
BSTNode* CC RotateRightAtXLeftAtY ( BSTNode *y, BSTNode *x )
{
    BSTNode *w = x -> child [ 0 ];
    BSTNode *z = w -> child [ 1 ];
    x -> child [ 0 ] = z;
    if ( z != 0 )
        SET_PARENT ( z, x );
    z = w -> child [ 0 ];
    w -> child [ 1 ] = x;
    y -> child [ 1 ] = z;
    w -> child [ 0 ] = y;
        
    switch ( BALANCE ( w ) )
    {
    case 0:
        w -> par = PARENT ( y );
        x -> par = w;
        y -> par = w;
        break;
    case LEFT:
        w -> par = PARENT ( y );
        SET_PARBAL ( x, w, RIGHT );
        y -> par = w;
        break;
    case RIGHT:
        w -> par = PARENT ( y );
        x -> par = w;
        SET_PARBAL ( y, w, LEFT );
        break;
    }

    /* patch parent link */
    if ( z != 0 )
        SET_PARENT ( z, y );

    return w;
}

static
BSTNode* CC RebalanceLeft ( BSTNode *y, BSTNode *x )
{
    /* detect child balance */
    if ( LEFT_HEAVY ( x ) )
        return RotateRightAtY ( y, x );

    /* child is right heavy */
    return RotateLeftAtXRightAtY ( y, x );
}

static
BSTNode* CC RebalanceRight ( BSTNode *y, BSTNode *x )
{
    /* detect child balance */
    if ( RIGHT_HEAVY ( x ) )
        return RotateLeftAtY ( y, x );

    /* left heavy */
    return RotateRightAtXLeftAtY ( y, x );
}


static
void CC RebalanceAfterInsert ( BSTNode **root, BSTNode *y, BSTNode *x )
{
    BSTNode *w, *z;

    /* detect left insertion */
    if ( y -> child [ 0 ] == x )
    {
        /* if y was right-heavy, done */
        if ( RIGHT_HEAVY ( y ) )
        {
            CLR_BALANCE ( y, RIGHT );
            return;
        }

        /* rebalance left insertion */
        w = RebalanceLeft ( y, x );
    }

    /* right insertion */
    else
    {
        /* if y was left-heavy, done */
        if ( LEFT_HEAVY ( y ) )
        {
            CLR_BALANCE ( y, LEFT );
            return;
        }

        /* rebalance right insertion */
        w = RebalanceRight ( y, x );
    }

    /* fix parent to child */
    assert ( BALANCE ( w ) == 0 );
    z = w -> par;
    if ( z == 0 )
        * root = w;
    else
        z -> child [ z -> child [ 1 ] == y ] = w;
}

LIB_EXPORT rc_t CC BSTreeInsert ( BSTree *bt, BSTNode *n,
    int ( CC * sort ) ( const BSTNode *n, const BSTNode *p ) )
{
    if ( bt != NULL && n != NULL )
    {
        int diff;

        BSTNode *p = bt -> root;
        BSTNode *q = NULL;
        BSTNode *y = NULL;

        while ( p != NULL )
        {
            diff = ( * sort ) ( n, p );
            q = p;
            if ( BALANCE ( p ) != 0 )
                y = p;
            p = p -> child [ diff > 0 ];
        }

        n -> par = q;
        n -> child [ 0 ] = n -> child [ 1 ] = NULL;

        if ( q == NULL )
            bt -> root = n;
        else
        {
            q -> child [ diff > 0 ] = n;

            /* run a trace-back */
            for ( p = n; q != y; )
            {
                /* this is safe because q has 0 balance */
                BSTNode *z = q -> par;
                if ( q -> child [ 0 ] == p )
                    SET_BALANCE ( q, LEFT );
                else
                    SET_BALANCE ( q, RIGHT );

                p = q;
                q = z;
            }

            /* rebalance */
            if ( q != NULL )
                RebalanceAfterInsert ( & bt -> root, q, p );
        }
    }

    /* never fails in this implementation */
    return 0;
}

/* BSTreeInsertUnique
 *  insert an object within tree, but only if unique
 *  "sort" function returns equivalent of "item" - "n"
 *  returns non-NULL "n" upon match or NULL on success
 */
LIB_EXPORT rc_t CC BSTreeInsertUnique ( BSTree *bt, BSTNode *n, BSTNode **exist,
    int ( CC * sort ) ( const BSTNode *n, const BSTNode *p ) )
{
    if ( bt != NULL && n != NULL )
    {
        int diff;

        BSTNode *p = bt -> root;
        BSTNode *q = NULL;
        BSTNode *y = NULL;

        while ( p != NULL )
        {
            diff = ( * sort ) ( n, p );

            if ( diff == 0 )
            {
                /* fail to insert */
                if ( exist != NULL )
                    * exist = p;
                return RC ( rcCont, rcTree, rcInserting, rcNode, rcExists );
            }

            q = p;
            if ( BALANCE ( p ) != 0 )
                y = p;
            p = p -> child [ diff > 0 ];
        }

        n -> par = q;
        n -> child [ 0 ] = n -> child [ 1 ] = NULL;

        if ( q == NULL )
            bt -> root = n;
        else
        {
            q -> child [ diff > 0 ] = n;

            /* run a trace-back */
            for ( p = n; q != y; )
            {
                /* this is safe because q has 0 balance */
                BSTNode *z = q -> par;
                if ( q -> child [ 0 ] == p )
                    SET_BALANCE ( q, LEFT );
                else
                    SET_BALANCE ( q, RIGHT );

                p = q;
                q = z;
            }

            /* rebalance */
            if ( q != NULL )
                RebalanceAfterInsert ( & bt -> root, q, p );
        }
    }

    /* only fails on existing item in this implementation */
    return 0;
}

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
LIB_EXPORT void CC BSTreeResort ( BSTree *bt,
    int ( CC * resort ) ( const BSTNode *item, const BSTNode *n ) )
{
    if ( bt != NULL )
    {
        BSTNode *p = bt -> root;
        bt -> root = NULL;

        while ( p != NULL )
        {
            BSTNode *q = p -> child [ 0 ];
            if ( q == 0 )
            {
                q = p -> child [ 1 ];
                BSTreeInsert ( bt, p, resort );
            }
            else
            {
                p -> child [ 0 ] = q -> child [ 1 ];
                q -> child [ 1 ] = p;
            }
            p = q;
        }
    }
}

/* BSTreeUnlink
 *  removes a node from tree
 */
static
void CC RebalanceAfterUnlink ( BSTNode **root, BSTNode *q, int dir )
{
    while ( q != 0 )
    {
        BSTNode *w, *x, *y = q;
        q = PARENT ( q );

        if ( ! dir )
        {
            if ( q && q -> child [ 1 ] == y )
                dir = 1;

            /* simulate an increment of balance */
            switch ( BALANCE ( y ) )
            {
            case 0:
                SET_BALANCE ( y, RIGHT );
                return;
            case LEFT:
                CLR_BALANCE ( y, LEFT );
                break;
            case RIGHT:
                /* y has just become ++ */
                x = y -> child [ 1 ];
                if ( LEFT_HEAVY ( x ) )
                {
                    w = RotateRightAtXLeftAtY ( y, x );
                    if ( q == 0 )
                        * root = w;
                    else
                        q -> child [ dir ] = w;
                }
                else
                {
                    w = y -> child [ 1 ] = x -> child [ 0 ];
                    x -> child [ 0 ] = y;
                    SET_PARENT ( x, q );
                    SET_PARENT ( y, x );
                    if ( w != 0 )
                        SET_PARENT ( w, y );
                    if ( q == 0 )
                        * root = x;
                    else
                        q -> child [ dir ] = x;
                    if ( BALANCE ( x ) == 0 )
                    {
                        SET_BALANCE ( x, LEFT );
                        SET_PARBAL ( y, x, RIGHT );
                        return;
                    }
                    ZERO_BALANCE ( x );
                    ZERO_BALANCE ( y );
                    /* y = x; */
                }
                break;
            }
        }

        /* symmetric case */
        else
        {
            if ( q && q -> child [ 0 ] == y )
                dir = 0;

            switch ( BALANCE ( y ) )
            {
            case 0:
                SET_BALANCE ( y, LEFT );
                return;
            case LEFT:
                /* y has just become -- */
                x = y -> child [ 0 ];
                if ( RIGHT_HEAVY ( x ) )
                {
                    w = RotateLeftAtXRightAtY ( y, x );
                    if ( q == 0 )
                        * root = w;
                    else
                        q -> child [ dir ] = w;
                }
                else
                {
                    w = x -> child [ 1 ];
                    y -> child [ 0 ] = w;
                    x -> child [ 1 ] = y;
                    SET_PARENT ( x, q );
                    SET_PARENT ( y, x );
                    if ( w != 0 )
                        SET_PARENT ( w, y );
                    if ( q == 0 )
                        * root = x;
                    else
                        q -> child [ dir ] = x;
                    if ( BALANCE ( x ) == 0 )
                    {
                        SET_BALANCE ( x, RIGHT );
                        SET_PARBAL ( y, x, LEFT );
                        return;
                    }
                    ZERO_BALANCE ( x );
                    ZERO_BALANCE ( y );
                    /* y = x; */
                }
                break;
            case RIGHT:
                CLR_BALANCE ( y, RIGHT );
                break;
            }
        }
    }
}

static
void CC BTUnlink ( BSTNode **root, BSTNode *p, int dir )
{
    BSTNode *q = PARENT ( p );
    BSTNode *l, *r = p -> child [ 1 ];
    if ( r == 0 )
    {
      /* no right child - simple unlink */
        l = p -> child [ 0 ];
        if ( q == 0 )
            * root = l;
        else
            q -> child [ dir ] = l;
        if ( l != 0 )
            SET_PARENT ( l, q );
    }
    else
    {
      /* have a right child - check its left */
        l = r -> child [ 0 ];
        if ( l == 0 )
        {
            l = p -> child [ 0 ];
            r -> child [ 0 ] = l;

            /* take not only p's parent ( q )
            // but its balance as well */
            r -> par = p -> par;

            if ( q == 0 )
                * root = r;
            else
                q -> child [ dir ] = r;

            if ( l != 0 )
                SET_PARENT ( l, r );

            /* artificially reset for following */
            q = r;
            dir = 1;
        }

        /* involves some work */
        else
        {
	  /* find smallest subsequent item */
            r = l -> child [ 0 ];
            while ( r != 0 )
            {
                l = r;
                r = l -> child [ 0 ];
            }

            /* unlink it */
            r = PARENT ( l );
            r -> child [ 0 ] = l -> child [ 1 ];

            /* take over doomed node */
            l -> child [ 0 ] = p -> child [ 0 ];
            l -> child [ 1 ] = p -> child [ 1 ];

            /* take not only p's parent ( q )
            // but its balance as well */
            l -> par = p -> par;

            /* new king pin */
            if ( q == 0 )
                * root = l;
            else
                q -> child [ dir ] = l;

            /* update parent links */
            q = l -> child [ 0 ];
            if ( q != 0 )
                SET_PARENT ( q, l );
            q = l -> child [ 1 ];
            SET_PARENT ( q, l );
            q = r -> child [ 0 ];
            if ( q != 0 )
                SET_PARENT ( q, r );

            q = r;
            dir = 0;
        }
    }

    /* now - rebalance what we've undone */
    if ( q != 0 )
        RebalanceAfterUnlink ( root, q, dir );
}

static
bool CC BSTreeContains ( const BSTNode *root, const BSTNode *n )
{
    while ( n != NULL )
    {
        if ( n == root )
            return true;
        n = PARENT ( n );
    }
    return false;
}

LIB_EXPORT bool CC BSTreeUnlink ( BSTree *bt, BSTNode *n )
{
    if ( bt != NULL && BSTreeContains ( bt -> root, n ) )
    {
        int dir = 0;
        BSTNode *q = PARENT ( n );
        if ( q != 0 )
        {
            assert ( q -> child [ 0 ] == n || q -> child [ 1 ] == n );
            dir = q -> child [ 1 ] == n;
        }
        BTUnlink ( & bt -> root, n, ( int ) dir );
        return true;
    }
    return false;
}

/* BSTreeForEach
 *  executes a function on each tree element
 */
LIB_EXPORT void CC BSTreeForEach ( const BSTree *bt, bool reverse,
    void ( CC * f ) ( BSTNode *n, void *data ), void *data )
{
    if ( bt != NULL )
    {
        BSTNode *n, *next;
        if ( reverse )
        {
            n = LastNode ( bt );
            while ( n != NULL )
            {
                next = BSTNodePrev ( n );
                ( * f ) ( n, data );
                n = next;
            }
        }
        else
        {
            n = FirstNode ( bt );
            while ( n != NULL )
            {
                next = BSTNodeNext ( n );
                ( * f ) ( n, data );
                n = next;
            }
        }
    }
}

/* BSTreeDoUntil
 *  executes a function on each element
 *  until the function returns 1
 */
LIB_EXPORT bool CC BSTreeDoUntil ( const BSTree *bt, bool reverse,
    bool ( CC * f ) ( BSTNode *n, void *data ), void *data )
{
    if ( bt != NULL )
    {
        BSTNode *n, *next;
        if ( reverse )
        {
            n = LastNode ( bt );
            while ( n != NULL )
            {
                next = BSTNodePrev ( n );
                if ( ( * f ) ( n, data ) )
                    return true;
                n = next;
            }
        }
        else
        {
            n = FirstNode ( bt );
            while ( n != NULL )
            {
                next = BSTNodeNext ( n );
                if ( ( * f ) ( n, data ) )
                    return true;
                n = next;
            }
        }
    }
    return false;
}

/* BSTreeWhack
 *  removes nodes from tree and
 *  executes a user provided destructor
 */
LIB_EXPORT void CC BSTreeWhack ( BSTree *bt,
    void ( CC * whack ) ( BSTNode *n, void *data ), void *data )
{
    if ( bt != NULL )
    {
        BSTNode *p = bt -> root;
        bt -> root = NULL;

        if ( whack != NULL )
        {
            while ( p != NULL )
            {
                BSTNode *q = p -> child [ 0 ];
                if ( q == 0 )
                {
                    q = p -> child [ 1 ];
                    ( * whack ) ( p, data );
                }
                else
                {
                    p -> child [ 0 ] = q -> child [ 1 ];
                    q -> child [ 1 ] = p;
                }
                p = q;
            }
        }
    }
}
