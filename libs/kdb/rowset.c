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

#include <kdb/rowset.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/out.h>
#include <klib/container.h>

#include <string.h>
#include <assert.h>

#if _DEBUGGING
#define CHECK_NODE_MARKS 1
#endif

#if CHECK_NODE_MARKS
#define NODE_MARK 55
#define LEAF_MARK 99
#endif

#define LEAF_DEPTH 6 // at which depth leaves are located
#define LEAF_DATA_SZ_BT 8192

/*--------------------------------------------------------------------------
 * KRowSet
 */
typedef struct RowSetLeaf RowSetLeaf;
typedef struct RowSetNode RowSetNode;
typedef struct RowSetNodeTransition RowSetNodeTransition;

// Holds bites of row_id followed by current byte (radix trie compression)
struct RowSetNodeTransition
{
    int8_t size;
    uint8_t data[5];
};

struct RowSetNode
{
    void* children[256];
    RowSetNodeTransition transitions[256];

#if CHECK_NODE_MARKS
    uint8_t node_mark;
#endif
};

struct RowSetLeaf
{
    DLNode dad;
    int64_t leaf_id; // this is basically a row_id >> 16
    uint8_t data[LEAF_DATA_SZ_BT];

#if CHECK_NODE_MARKS
    uint8_t leaf_mark;
#endif
};

struct KRowSet
{
    KRefcount refcount;
    RowSetNode * root_node;
    DLList leaf_nodes;

    size_t number_rows;
    size_t number_leaves;
};

KDB_EXTERN rc_t CC KCreateRowSet ( KRowSet ** self )
{
    rc_t rc;
    KRowSet * rowset;
    
    if ( self == NULL )
        rc = RC ( rcDB, rcRowSet, rcConstructing, rcSelf, rcNull );
    else
    {
        rowset = calloc( 1, sizeof *rowset );
        if ( rowset == NULL )
            rc = RC ( rcDB, rcRowSet, rcConstructing, rcMemory, rcExhausted );
        else
        {
            KRefcountInit ( &rowset->refcount, 1, "KRowSet", "new", "" );
            DLListInit ( &rowset->leaf_nodes );

            *self = rowset;
            return 0;
        }
    }
    
    return rc;
}

/* Whack
 */

/*
 * When data is NULL, it means that leaf is freeing during freeing the whole list
 * and DLListUnlink does not need to be called. Otherwise, we are removing a single leaf
 */
static
void KRowSetLeafWhack ( DLNode * node, void * data )
{
    RowSetLeaf * leaf = (RowSetLeaf *) node;
#if CHECK_NODE_MARKS
    assert ( leaf->leaf_mark == LEAF_MARK );
#endif

    if ( data != NULL )
    {
        DLList * leaf_nodes = data;
        DLListUnlink ( leaf_nodes, node );
    }

    free ( leaf );
}

/**
 * Recursively deallocates all trie nodes and leaves when "free_leaves" is true.
 *  Otherwise only deallocates nodes.
 */
static
void KRowSetNodeWhack ( KRowSet * self, RowSetNode * node, int depth, bool free_leaves )
{
    int i;

    assert ( self != NULL );
    assert ( depth < LEAF_DEPTH );

#if CHECK_NODE_MARKS
    assert ( node->node_mark == NODE_MARK );
#endif
    assert ( depth < LEAF_DEPTH );

    for ( i = 0; i < 256; ++i )
    {
        assert ( (i & 0xFF) == i );

        if ( node->children[i] )
        {
            int new_depth = depth + node->transitions[i].size + 1;
            assert ( new_depth <= LEAF_DEPTH );

            if ( new_depth < LEAF_DEPTH )
                KRowSetNodeWhack ( self, node->children[i], new_depth, free_leaves );
            else if ( free_leaves )
                KRowSetLeafWhack ( &((RowSetLeaf*) node->children[i])->dad, &self->leaf_nodes );
        }
    }

    free ( node );
}

/**
 * Whack
 *   Frees all leaves first and then walks through the trie
 *   and frees all remaining nodes
 */
static
rc_t KRowSetWhack ( KRowSet * self )
{
    assert ( self != NULL );

    if ( self->root_node )
    {
        KRowSetNodeWhack ( self, self->root_node, 0, false );
        DLListWhack ( &self->leaf_nodes, KRowSetLeafWhack, NULL );
    }
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  ignores NULL references
 */
KDB_EXTERN rc_t CC KRowSetAddRef ( const KRowSet *self )
{
    if ( self != NULL ) switch ( KRefcountAdd ( & self -> refcount, "KRowSet" ) )
    {
        case krefOkay:
            break;
        default:
            return RC ( rcDB, rcRowSet, rcAttaching, rcConstraint, rcViolated );
    }
    return 0;
}

KDB_EXTERN rc_t CC KRowSetRelease ( const KRowSet *self )
{
    if ( self != NULL ) switch ( KRefcountDrop ( & self -> refcount, "KRowSet" ) )
    {
        case krefOkay:
            break;
        case krefWhack:
            return KRowSetWhack ( ( KRowSet* ) self );
        default:
            return RC ( rcDB, rcRowSet, rcReleasing, rcConstraint, rcViolated );
    }
    return 0;
}

/**
 * Get byte of "leaf_id" which represents node's child at "depth"
 */
static
inline uint8_t KRowSetGetLeafIdByte ( int64_t leaf_id, int depth )
{
    return leaf_id >> (LEAF_DEPTH - depth - 1) * 8 & 0xFF;
}

/**
 * Searches a nearest neighbor of just inserted "leaf",
 *  by walking through nodes_stack from the most deep node to the root one.
 *
 * Is used to inserter newly created "leaf" to a linked list.
 */
static
void KRowSetFindNearestLeaf ( KRowSet * self, RowSetNode * nodes_stack[], int nodes_depth[], RowSetLeaf * leaf, RowSetLeaf ** result, bool * result_left )
{
    assert ( self != NULL );
    assert ( nodes_stack != NULL );
    assert ( leaf != NULL );
    assert ( result != NULL );
    assert ( result_left != NULL );

    *result = NULL;

    if ( self->number_leaves == 0 )
        return;

    if ( self->number_leaves == 1 )
    {
        *result = (RowSetLeaf *) DLListHead ( &self->leaf_nodes );
        *result_left = (*result)->leaf_id < leaf->leaf_id;
        return;
    }

    {
        int i;
        int64_t leaf_id = leaf->leaf_id;
        RowSetLeaf * nearest_leaf = NULL;
        bool nearest_leaf_left;
        for ( i = LEAF_DEPTH - 1; i >= 0; --i )
        {
            int depth;
            int j;
            int j_max;
            uint8_t bt;
            RowSetNode * current_node = nodes_stack[i];
            void * nearest_subtree = NULL;
            bool nearest_subtre_left;
            int nearest_subtree_depth;

            // determine nodes stack depth, since some of the nodes may have been compressed
            if ( current_node == NULL )
                continue;

            depth = nodes_depth[i];
            bt = KRowSetGetLeafIdByte ( leaf_id, depth );

            assert ( current_node->children[bt] != NULL );

            if ( 255 - bt > bt )
                j_max = 255 - bt;
            else
                j_max = bt;

            for (j = 1; j <= j_max; ++j)
            {
                if ( bt + j <= 255 && current_node->children[bt + j] != NULL )
                {
                    nearest_subtre_left = false;
                    nearest_subtree = current_node->children[bt + j];
                    nearest_subtree_depth = depth + 1 + current_node->transitions[bt + j].size;
                    break;
                }

                if ( bt - j >= 0 && current_node->children[bt - j] != NULL )
                {
                    nearest_subtre_left = true;
                    nearest_subtree = current_node->children[bt - j];
                    nearest_subtree_depth = depth + 1 + current_node->transitions[bt - j].size;
                    break;
                }
            }

            if ( nearest_subtree != NULL )
            {
                for ( j = nearest_subtree_depth; j < LEAF_DEPTH; ++j )
                {
                    int search_i;
                    int search_start = nearest_subtre_left ? 255 : 0;
                    int search_stop = nearest_subtre_left ? 0 : 255;
                    int search_step = nearest_subtre_left ? -1 : 1;
                    void * nearest_subtree_next = NULL;
                    RowSetNodeTransition * nearest_subtree_next_tr;

#if CHECK_NODE_MARKS
                    assert ( ((RowSetNode *) nearest_subtree)->node_mark == NODE_MARK );
#endif

                    for ( search_i = search_start; search_step > 0 ? search_i <= search_stop : search_i >= search_stop; search_i += search_step )
                    {
                        if ( ((RowSetNode *) nearest_subtree)->children[search_i] != NULL )
                        {
                            nearest_subtree_next = ((RowSetNode *) nearest_subtree)->children[search_i];
                            nearest_subtree_next_tr = &((RowSetNode *) nearest_subtree)->transitions[search_i];
                            break;
                        }
                    }

                    assert ( nearest_subtree_next != NULL );
                    nearest_subtree = nearest_subtree_next;
                    j += nearest_subtree_next_tr->size;
                }

                assert ( j == LEAF_DEPTH );

                nearest_leaf = nearest_subtree;
                nearest_leaf_left = nearest_subtre_left;

                break;
            }
        }

        // empty tree is handled by previous "if"s
        assert ( nearest_leaf != NULL );

#if CHECK_NODE_MARKS
        assert ( ((RowSetLeaf *) nearest_leaf)->leaf_mark == LEAF_MARK );
#endif
        *result = nearest_leaf;
        *result_left = nearest_leaf_left;
    }

    return;
}

/**
 * Checks if node's child pointed by "node_depth"'s "leaf_id" byte matches "leaf_id".
 *  It is possible that transition uses more than one byte of "leaf_id".
 *  In such case, we have to make sure that those bytes match.
 */
static
bool KRowSetNodeIsTransitionMatches ( int64_t leaf_id, int node_depth, const RowSetNodeTransition * tr, int * size_matched )
{
    int matched;

    assert ( node_depth + tr->size < LEAF_DEPTH );

    for ( matched = 0; matched < tr->size; ++matched )
    {
        if ( KRowSetGetLeafIdByte ( leaf_id, matched + node_depth + 1 ) != tr->data[matched] )
            break;
    }

    if ( size_matched != NULL )
        *size_matched = matched;

    return matched == tr->size;
}

/**
 * Assigns a new child to a "node".
 *  It will note create any extra node's even if child_depth - node_depth > 1
 *  (because of radix trie compression)
 */
static
void KRowSetNodeSetChild ( RowSetNode * node, void * child, int64_t leaf_id, int node_depth, int child_depth )
{
    uint8_t bt = KRowSetGetLeafIdByte ( leaf_id, node_depth );
    int i;

    node->children[bt] = child;
    node->transitions[bt].size = child_depth - 1 - node_depth;

    for ( i = node_depth; i < child_depth - 1; ++i )
    {
        node->transitions[bt].data[i - node_depth] = KRowSetGetLeafIdByte ( leaf_id, i + 1 );
    }
}

/**
 * Splits the node, by inserting "allocated_interm_node" between "node" and it's child
 *  Corrects transitions after insertion.
 *
 *  NB - To ease error handling and deallocation of resources in case of errors,
 *       "allocated_interm_node" has to be allocated before calling this function.
 */
static
void KRowSetSplitNode ( RowSetNode * node, int node_depth, RowSetNode * allocated_interm_node, int interm_node_depth, int64_t leaf_id )
{
    uint8_t bt = KRowSetGetLeafIdByte ( leaf_id, node_depth );
    uint8_t interm_bt;
    void * child = node->children[bt];
    int first_tr_size = interm_node_depth - node_depth - 1;

#if CHECK_NODE_MARKS
    allocated_interm_node->node_mark = NODE_MARK;
#endif

    assert ( allocated_interm_node != NULL );
    assert ( node->children[bt] != NULL );

    interm_bt = node->transitions[bt].data[first_tr_size];
    allocated_interm_node->transitions[interm_bt].size = node->transitions[bt].size - first_tr_size - 1;
    for ( int i = 0; i < allocated_interm_node->transitions[interm_bt].size; ++i )
    {
        allocated_interm_node->transitions[interm_bt].data[i] = node->transitions[bt].data[first_tr_size + i + 1];
    }
    allocated_interm_node->children[interm_bt] = child;

    KRowSetNodeSetChild ( node, allocated_interm_node, leaf_id, node_depth, interm_node_depth );
}

/**
 * Sets "allocated_leaf" members and inserts it as a child to a previously found "node" that fits leaf_id.
 *  Also inserts "allocated_leaf" to leaves linked list.
 *
 *  NB - To ease error handling and deallocation of resources in case of errors,
 *       "allocated_leaf" has to be allocated before calling this function.
 */
static
void KRowSetInsertLeaf ( KRowSet * self, int64_t leaf_id, RowSetNode * node, int node_depth, RowSetNode * nodes_stack[], int nodes_depth[], RowSetLeaf * allocated_leaf )
{
    RowSetLeaf * nearest_leaf;
    bool nearest_leaf_left;
    uint8_t bt;
    bt = KRowSetGetLeafIdByte ( leaf_id, node_depth );

    assert ( allocated_leaf != NULL );
    assert ( node->children[bt] == NULL );

#if CHECK_NODE_MARKS
    allocated_leaf->leaf_mark = LEAF_MARK;
#endif
    allocated_leaf->leaf_id = leaf_id;

    KRowSetNodeSetChild ( node, allocated_leaf, leaf_id, node_depth, LEAF_DEPTH );
    node->children[bt] = allocated_leaf;

    KRowSetFindNearestLeaf ( self, nodes_stack, nodes_depth, allocated_leaf, &nearest_leaf, &nearest_leaf_left );
    if ( nearest_leaf == NULL )
        DLListPushTail ( &self->leaf_nodes, &allocated_leaf->dad );
    else if (nearest_leaf_left)
        DLListInsertNodeAfter ( &self->leaf_nodes, &nearest_leaf->dad, &allocated_leaf->dad );
    else
        DLListInsertNodeBefore ( &self->leaf_nodes, &nearest_leaf->dad, &allocated_leaf->dad );

    ++self->number_leaves;
    assert ( KRowSetNodeIsTransitionMatches ( leaf_id, node_depth, &node->transitions[bt], NULL ) );
}

/**
 * Searches for a leaf by "row_id".
 *  Returns rcNotFound when leaf is not in the tree and "insert_when_needed" is false,
 *  otherwise inserts a new leaf and returns it in "leaf_found".
 *
 *  NB - This function preallocates all required resources
 * 	     and only after that changes the RowSet data structure.
 *
 * 	     Using this approach we can simply return from the function,
 * 	     since all the resources are being allocated once.
 */
static
rc_t KRowSetGetLeaf ( KRowSet * self, int64_t row_id, bool insert_when_needed, RowSetLeaf ** leaf_found )
{
    int64_t leaf_id = row_id >> 16;
    RowSetNode * node;
    RowSetLeaf * new_leaf;
    uint8_t bt;
    int depth = 0;
    int nodes_stack_size = 0;
    RowSetNode * nodes_stack[LEAF_DEPTH] = { NULL }; // initialize array with NULLs
    int nodes_depth[LEAF_DEPTH];

    if ( self == NULL )
        return RC ( rcDB, rcRowSet, rcSelecting, rcSelf, rcNull );

    if ( leaf_found == NULL )
        return RC ( rcDB, rcRowSet, rcSelecting, rcParam, rcNull );

    // empty tree
    if ( self->root_node == NULL )
    {
        RowSetNode * root;
        if ( !insert_when_needed )
            return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

        root = calloc ( 1, sizeof ( RowSetNode ) );
        if (root == NULL)
            return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );

        // pre-allocate leaf here
        new_leaf = calloc ( 1, sizeof ( RowSetLeaf ) );
        if (new_leaf == NULL)
        {
            free ( root );
            return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );
        }

#if CHECK_NODE_MARKS
        root->node_mark = NODE_MARK;
#endif

        self->root_node = root;

        node = self->root_node;
        nodes_stack[nodes_stack_size] = node;
        nodes_depth[nodes_stack_size++] = depth;

        KRowSetInsertLeaf ( self, leaf_id, node, depth, nodes_stack, nodes_depth, new_leaf );
        *leaf_found = new_leaf;
        return 0;
    }

    node = self->root_node;
    nodes_stack[nodes_stack_size] = node;
    nodes_depth[nodes_stack_size++] = depth;
#if CHECK_NODE_MARKS
        assert ( node->node_mark == NODE_MARK );
#endif

    for ( ; depth < LEAF_DEPTH; )
    {
        int tr_size_matched;
        bt = KRowSetGetLeafIdByte ( leaf_id, depth );
        // no child at a given transition, let's insert leaf here
        if ( node->children[bt] == NULL )
        {
            if ( !insert_when_needed )
                return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

            // pre-allocate leaf here
            new_leaf = calloc ( 1, sizeof ( RowSetLeaf ) );
            if (new_leaf == NULL)
                return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );

            KRowSetInsertLeaf ( self, leaf_id, node, depth, nodes_stack, nodes_depth, new_leaf );
            *leaf_found = new_leaf;
            return 0;
        }

        // transition does not match, split the node
        if ( !KRowSetNodeIsTransitionMatches ( leaf_id, depth, &node->transitions[bt], &tr_size_matched ) )
        {
            RowSetNode * interm_node;
            int interm_node_depth = depth + 1 + tr_size_matched;

            if ( !insert_when_needed )
                return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

            // pre-allocate node and leaf here
            interm_node = calloc ( 1, sizeof ( RowSetNode ) );
            new_leaf = calloc ( 1, sizeof ( RowSetLeaf ) );
            if (interm_node == NULL || new_leaf == NULL)
            {
                free ( interm_node );
                return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );
            }

            KRowSetSplitNode ( node, depth, interm_node, interm_node_depth, leaf_id );

            node = interm_node;
            depth = interm_node_depth;
#if CHECK_NODE_MARKS
            assert ( node->node_mark == NODE_MARK );
#endif
            assert ( depth < sizeof nodes_stack / sizeof nodes_stack[0] );
            nodes_stack[nodes_stack_size] = node;
            nodes_depth[nodes_stack_size++] = depth;

            KRowSetInsertLeaf ( self, leaf_id, node, depth, nodes_stack, nodes_depth, new_leaf );
            *leaf_found = new_leaf;
            return 0;

        }

        // follow the transition, check if transition leads to a leaf
        depth += 1 + tr_size_matched;
        if ( depth == LEAF_DEPTH )
        {
            *leaf_found = node->children[bt];
            return 0;
        }

        node = node->children[bt];
#if CHECK_NODE_MARKS
        assert ( node->node_mark == NODE_MARK );
#endif
        assert ( depth < sizeof nodes_stack / sizeof nodes_stack[0] );
        nodes_stack[nodes_stack_size] = node;
        nodes_depth[nodes_stack_size++] = depth;
    }

    // "for" loop iterates up to a maximum possible depth,
    // so it must come to a leaf
    assert ( false );
    return 0;
}

KDB_EXTERN rc_t CC KRowSetInsertRow ( KRowSet * self, int64_t row_id )
{
    rc_t rc;
    RowSetLeaf * leaf;
    uint16_t bt;

    rc = KRowSetGetLeaf ( self, row_id, true, &leaf );
    if ( rc != 0 )
        return rc;

    bt = row_id & 0xFFFF;
    if ( ( leaf->data[bt >> 3] & (1 << (bt & 7)) ) != 0 )
        return RC ( rcDB, rcRowSet, rcInserting, rcId, rcDuplicate );

    leaf->data[bt >> 3] |= 1 << (bt & 7);
    ++self->number_rows;

    return 0;
}

KDB_EXTERN rc_t CC KRowSetGetNumRows ( const KRowSet * self, size_t * num_rows )
{
    if ( self == NULL )
        return RC ( rcDB, rcRowSet, rcAccessing, rcSelf, rcNull );

    if ( num_rows == NULL )
            return RC ( rcDB, rcRowSet, rcAccessing, rcParam, rcNull );

    *num_rows = self->number_rows;
    return 0;
}

typedef struct {
    bool reverse;
    void ( CC * cb ) ( int64_t row_id, void * data );
    void * data;
} KRowSetWalkRowParams;

/**
 * Walks through each leaf's rows and calls "cb" for each found row
 */
static
void KRowSetWalkLeafRows ( DLNode * n, void * data )
{
    RowSetLeaf * leaf = (RowSetLeaf *) n;
    KRowSetWalkRowParams * walk_params = data;

    int i;
    int j;
    int64_t row_id = leaf->leaf_id;

#if CHECK_NODE_MARKS
    assert ( leaf->leaf_mark == LEAF_MARK );
#endif
    assert ( walk_params != NULL );

    if ( walk_params == NULL || walk_params->cb == NULL )
        return;

    row_id <<= 16;

    if (!walk_params->reverse)
    {

        for ( i = 0; i < LEAF_DATA_SZ_BT; ++i )
        {
            if ( leaf->data[i] )
            {
                uint64_t new_row_id = row_id | i << 3;
                for (j = 0; j < 8; ++j)
                {
                    if ( leaf->data[i] & (1 << j) )
                    {
                        walk_params->cb ( (int64_t)(new_row_id | j), walk_params->data );
                    }
                }
            }
        }

    }
    else
    {

        for ( i = LEAF_DATA_SZ_BT - 1; i >= 0 ; --i )
        {
            if ( leaf->data[i] )
            {
                uint64_t new_row_id = row_id | i << 3;
                for (j = 7; j >= 0; --j)
                {
                    if ( leaf->data[i] & (1 << j) )
                    {
                        walk_params->cb ( (int64_t)(new_row_id | j), walk_params->data );
                    }
                }
            }
        }

    }
}

KDB_EXTERN rc_t CC KRowSetWalkRows ( const KRowSet * self, bool reverse,
        void ( CC * cb ) ( int64_t row_id, void * data ), void * data )
{
    KRowSetWalkRowParams walk_params;

    if ( self == NULL )
        return RC ( rcDB, rcRowSet, rcAccessing, rcSelf, rcNull );

    walk_params.reverse = reverse;
    walk_params.cb = cb;
    walk_params.data = data;
    DLListForEach ( &self->leaf_nodes, reverse, KRowSetWalkLeafRows, &walk_params );

    return 0;
}
