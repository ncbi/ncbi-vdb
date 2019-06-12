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
#include "pbstree-priv.h"
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * PBSTree
 *  a flattened b-tree
 */

/* Make
 *  make a PBSTree structure
 *  the memory reference passed in is borrowed
 */
LIB_EXPORT rc_t CC PBSTreeMake ( PBSTree **ptp, const void *addr, size_t size, bool byteswap )
{
    rc_t rc;

    if ( ptp == NULL )
        rc = RC ( rcCont, rcTree, rcConstructing, rcParam, rcNull );
    else
    {
        if ( size == 0 )
            rc = RC ( rcCont, rcTree, rcConstructing, rcData, rcInvalid );
        else if ( addr == NULL )
            rc = RC ( rcCont, rcTree, rcConstructing, rcData, rcNull );
        else
        {
            PBSTree *pt = malloc ( sizeof * pt );
            if ( pt == NULL )
                rc = RC ( rcCont, rcTree, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = byteswap ?
                    PBSTreeMakeSwapped ( pt, addr, size ):
                    PBSTreeMakeNative ( pt, addr, size );
                if ( rc == 0 )
                {
                    * ptp = pt;
                    return 0;
                }

                free ( pt );
            }
        }

        * ptp = NULL;
    }

    return rc;
}


/* Init - PROTECTED
 *  initialize the parent class
 */
rc_t PBSTreeInit ( PBSTree *self, const PBSTree_vt *vt, const P_BSTree *pt )
{
    if ( self == NULL )
        return RC ( rcCont, rcTree, rcConstructing, rcSelf, rcNull );

    if ( vt == NULL )
        return RC ( rcCont, rcTree, rcConstructing, rcInterface, rcNull );
    if ( vt -> v1 . maj == 0 )
        return RC ( rcCont, rcTree, rcConstructing, rcInterface, rcInvalid );
    if ( vt -> v1 . maj > PBSTREE_LATEST )
        return RC ( rcFS, rcFile, rcConstructing, rcInterface, rcBadVersion );

    if ( pt == NULL )
        return RC ( rcCont, rcTree, rcConstructing, rcParam, rcNull );

#if _DEBUGGING
    if ( vt -> v1 . maj == 1 )
    {
        /* check v1.1 methods */
        if ( vt -> v1 . destroy == NULL ||
             vt -> v1 . count == NULL ||
             vt -> v1 . depth == NULL ||
             vt -> v1 . size == NULL ||
             vt -> v1 . get_node_data == NULL ||
             vt -> v1 . find == NULL ||
             vt -> v1 . for_each == NULL ||
             vt -> v1 . do_until == NULL )
        return RC ( rcCont, rcTree, rcConstructing, rcInterface, rcNull );
    }
#endif

    /* everything is okay */
    self -> vt = vt;
    self -> pt = pt;
    return 0;
}


/* Count
 *  returns number of elements in b-tree
 */
LIB_EXPORT uint32_t CC PBSTreeCount ( const PBSTree *self )
{
    if ( self != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . count ) ( self );
        }
    }
    return 0;
}

/* Depth
 *  returns number of layers in b-tree
 */
LIB_EXPORT uint32_t CC PBSTreeDepth ( const PBSTree *self )
{
    if ( self != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . depth ) ( self );
        }
    }

    return 0;
}

/* Size
 *  returns the size in bytes
 *  of the PBSTree image
 */
LIB_EXPORT size_t CC PBSTreeSize ( const PBSTree *self )
{
    if ( self != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . size ) ( self );
        }
    }

    return 0;
}


/* GetNodeData - INTERNAL
 *  finds node data boundaries
 */
rc_t PBSTreeGetNodeData ( const PBSTree *self,
    const void **addr, size_t *size, uint32_t id )
{
    assert ( self != NULL );
    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . get_node_data ) ( self, addr, size, id );
    }

    return RC ( rcCont, rcTree, rcAccessing, rcInterface, rcBadVersion );
}


/* GetNode
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
LIB_EXPORT rc_t CC PBSTreeGetNode ( const PBSTree *self, PBSTNode *node, uint32_t id )
{
    rc_t rc;
    if ( node == NULL )
        rc = RC ( rcCont, rcTree, rcAccessing, rcNode, rcNull );
    else
    {
        if ( self == NULL)
            rc = RC ( rcCont, rcTree, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = PBSTreeGetNodeData ( self,
                & node -> data . addr, & node -> data . size, id );
            if ( rc == 0 )
            {
                node -> internal = self;
                node -> id = id;
                return 0;
            }
        }

        node -> data . addr = node -> internal = NULL;
        node -> data . size = 0;
        node -> id = 0;
    }
    return rc;
}

/* PBSTreeFind
 *  find an object within tree
 *  "cmp" function returns equivalent of "item" - "n"
 */
LIB_EXPORT uint32_t CC PBSTreeFind ( const PBSTree *self, PBSTNode *n, const void *item,
    int ( CC * cmp ) ( const void *item, const PBSTNode *n, void *data ), void *data )
{
    PBSTNode buffer;

    if ( n == NULL )
        n = & buffer;

    n -> internal = self;
    if ( self != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . find ) ( self, n, item, cmp, data );
        }
    }

    n -> data . addr = NULL;
    n -> data . size = 0;
    return n -> id = 0;
}

/* PBSTreeForEach
 *  executes a function on each tree element
 */
LIB_EXPORT void CC PBSTreeForEach ( const PBSTree *self, bool reverse,
    void ( CC * f ) ( PBSTNode *n, void *data ), void *data )
{
    if ( self != NULL && f != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            ( * self -> vt -> v1 . for_each ) ( self, reverse, f, data );
            break;
        }
    }
}

/* PBSTreeDoUntil
 *  executes a function on each element
 *  until the function returns true
 */
LIB_EXPORT bool CC PBSTreeDoUntil ( const PBSTree *self, bool reverse,
    bool ( CC * f ) ( PBSTNode *n, void *data ), void *data )
{
    if ( self != NULL && f != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . do_until ) ( self, reverse, f, data );
        }
    }

    return false;
}

/* PBSTreeWhack
 *  whacks PBSTree object
 *  does not free memory reference
 */
LIB_EXPORT void CC PBSTreeWhack ( PBSTree *self )
{
    if ( self != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            ( * self -> vt -> v1 . destroy ) ( self );
            break;
        }
    }
}


/*--------------------------------------------------------------------------
 * PBSTNode
 *  identifies a node within persisted b-tree
 */

/* Next
 *  returns next node id
 */
LIB_EXPORT uint32_t CC PBSTNodeNext ( PBSTNode *n )
{
    if ( n == NULL )
        return 0;

    if ( PBSTreeGetNodeData ( n -> internal,
             & n -> data . addr, & n -> data . size, n -> id + 1 ) )
        return 0;

    return ++ n -> id;
}

/* Prev
 *  returns prev node id
 */
LIB_EXPORT uint32_t CC PBSTNodePrev ( PBSTNode *n )
{
    if ( n == NULL )
        return 0;

    if ( PBSTreeGetNodeData ( n -> internal,
             & n -> data . addr, & n -> data . size, n -> id - 1 ) )
        return 0;

    return -- n -> id;
}

/* FindNext
 *  find next element satisfying criteria
 */
LIB_EXPORT uint32_t CC PBSTNodeFindNext ( PBSTNode *n,
    bool ( CC * f ) ( const PBSTNode *n ) )
{
    PBSTNode save;

    if ( n == NULL || f == NULL )
        return 0;

    save = * n;

    while ( PBSTNodeNext ( n ) )
    {
        if ( ( * f ) ( n ) )
            return n -> id;
    }

    * n = save;
    return 0;
}

/* FindPrev
 *  find previous element satisfying criteria
 */
LIB_EXPORT uint32_t CC PBSTNodeFindPrev ( PBSTNode *n, 
    bool ( CC * f ) ( const PBSTNode *n ) )
{
    PBSTNode save;

    if ( n == NULL || f == NULL )
        return 0;

    save = * n;

    while ( PBSTNodePrev ( n ) )
    {
        if ( ( * f ) ( n ) )
            return n -> id;
    }

    * n = save;
    return 0;
}
