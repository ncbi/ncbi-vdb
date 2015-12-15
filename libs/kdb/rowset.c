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
#include "rowset-priv.h"
#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/out.h>

#include <string.h>
#include <assert.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

/*--------------------------------------------------------------------------
 * KRowSet
 */

struct KRowSet
{
    KRefcount refcount;
    KRowSetTreeNode * root_node;
    DLList leaf_nodes;

    uint64_t number_rows;
    size_t number_leaves;
};

KDB_EXTERN rc_t CC KTableMakeRowSet ( struct KTable const * self, KRowSet ** rowset )
{
    rc_t rc;
    KRowSet * self_rowset;
    
    if ( rowset == NULL )
        rc = RC ( rcDB, rcRowSet, rcConstructing, rcSelf, rcNull );
    else
    {
        self_rowset = calloc ( 1, sizeof *self_rowset );
        if ( self_rowset == NULL )
            rc = RC ( rcDB, rcRowSet, rcConstructing, rcMemory, rcExhausted );
        else
        {
            KRefcountInit ( &self_rowset->refcount, 1, "KRowSet", "new", "" );
            DLListInit ( &self_rowset->leaf_nodes );

            *rowset = self_rowset;
            return 0;
        }
    }
    
    return rc;
}

/**
 * Allocates and initializes single leaf
 */
static
rc_t KRowSetAllocateLeaf ( KRowSetTreeLeafType type, int64_t leaf_id, KRowSetTreeLeaf ** leaf )
{
    KRowSetTreeLeaf * allocated_leaf;
    size_t size = sizeof allocated_leaf->header;

    assert ( leaf != NULL );

    switch ( type )
    {
    case LeafType_Bitmap:
        size += sizeof allocated_leaf->data.bitmap;
        break;
    case LeafType_ArrayRanges:
        size += sizeof allocated_leaf->data.array_ranges;
        break;
    default:
        assert ( false );
    }

    allocated_leaf = calloc ( 1, size );
    if ( allocated_leaf == NULL )
        return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );

#if CHECK_NODE_MARKS
    allocated_leaf->header.leaf_mark = LEAF_MARK;
#endif
    allocated_leaf->header.type = type;
    allocated_leaf->header.leaf_id = leaf_id;

    *leaf = allocated_leaf;
    return 0;
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
    KRowSetTreeLeaf * leaf = (KRowSetTreeLeaf *) node;
#if CHECK_NODE_MARKS
    assert ( leaf->header.leaf_mark == LEAF_MARK );
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
void KRowSetNodeWhack ( KRowSet * self, KRowSetTreeNode * node, int depth, bool free_leaves )
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
                KRowSetLeafWhack ( &((KRowSetTreeLeaf*) node->children[i])->header.dad, &self->leaf_nodes );
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
void KRowSetFindNearestLeaf ( KRowSet * self, KRowSetTreeNode * nodes_stack[], int nodes_depth[], int nodes_stack_size, KRowSetTreeLeaf * leaf, KRowSetTreeLeaf ** result, bool * result_left )
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
        *result = (KRowSetTreeLeaf *) DLListHead ( &self->leaf_nodes );
        *result_left = (*result)->header.leaf_id < leaf->header.leaf_id;
        return;
    }

    {
        int i;
        int64_t leaf_id = leaf->header.leaf_id;
        KRowSetTreeLeaf * nearest_leaf = NULL;
        bool nearest_leaf_left;
        for ( i = nodes_stack_size - 1; i >= 0; --i )
        {
            int depth;
            int j;
            int j_max;
            uint8_t bt;
            KRowSetTreeNode * current_node = nodes_stack[i];
            void * nearest_subtree = NULL;
            bool nearest_subtre_left;
            int nearest_subtree_depth;

            assert ( current_node != NULL );

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
                    KRowSetTreeNodeTransition * nearest_subtree_next_tr;

#if CHECK_NODE_MARKS
                    assert ( ((KRowSetTreeNode *) nearest_subtree)->node_mark == NODE_MARK );
#endif

                    for ( search_i = search_start; search_step > 0 ? search_i <= search_stop : search_i >= search_stop; search_i += search_step )
                    {
                        if ( ((KRowSetTreeNode *) nearest_subtree)->children[search_i] != NULL )
                        {
                            nearest_subtree_next = ((KRowSetTreeNode *) nearest_subtree)->children[search_i];
                            nearest_subtree_next_tr = &((KRowSetTreeNode *) nearest_subtree)->transitions[search_i];
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
        assert ( ((KRowSetTreeLeaf *) nearest_leaf)->header.leaf_mark == LEAF_MARK );
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
bool KRowSetNodeIsTransitionMatches ( int64_t leaf_id, int node_depth, const KRowSetTreeNodeTransition * tr, int * size_matched )
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
void KRowSetNodeSetChild ( KRowSetTreeNode * node, void * child, int64_t leaf_id, int node_depth, int child_depth )
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
void KRowSetSplitNode ( KRowSetTreeNode * node, int node_depth, KRowSetTreeNode * allocated_interm_node, int interm_node_depth, int64_t leaf_id )
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
 * Collapses node, by linking (parent) "node" to "collapse_node"'s the only child
 *  and deallocates "collapse_node"
 */
static
void KRowSetCollapseNode ( KRowSetTreeNode * node, int node_depth, KRowSetTreeNode * collapse_node, int collapse_node_depth, uint8_t collapse_node_bt, int64_t leaf_id )
{
    int i;
    uint8_t node_bt = KRowSetGetLeafIdByte ( leaf_id, node_depth );

#if CHECK_NODE_MARKS
    node->node_mark = NODE_MARK;
    collapse_node->node_mark = NODE_MARK;
#endif
    assert ( node->children[node_bt] == collapse_node );
    assert ( collapse_node->children[collapse_node_bt] != NULL );

    node->children[node_bt] = collapse_node->children[collapse_node_bt];
    node->transitions[node_bt].data[node->transitions[node_bt].size++] = collapse_node_bt;
    for ( i = 0; i < collapse_node->transitions[collapse_node_bt].size; ++i )
    {
        node->transitions[node_bt].data[node->transitions[node_bt].size++] = collapse_node->transitions[collapse_node_bt].data[i];
    }

    free ( collapse_node );
}

/**
 * Sets "allocated_leaf" members and inserts it as a child to a previously found "node" that fits leaf_id.
 *  Also inserts "allocated_leaf" to leaves linked list.
 *
 *  NB - To ease error handling and deallocation of resources in case of errors,
 *       "allocated_leaf" has to be allocated before calling this function.
 */
static
void KRowSetInsertLeaf ( KRowSet * self, int64_t leaf_id, KRowSetTreeNode * node, int node_depth, KRowSetTreeNode * nodes_stack[], int nodes_depth[], int nodes_stack_size, KRowSetTreeLeaf * allocated_leaf )
{
    KRowSetTreeLeaf * nearest_leaf;
    bool nearest_leaf_left;
    uint8_t bt;
    bt = KRowSetGetLeafIdByte ( leaf_id, node_depth );

    assert ( allocated_leaf != NULL );
    assert ( node->children[bt] == NULL );

    KRowSetNodeSetChild ( node, allocated_leaf, leaf_id, node_depth, LEAF_DEPTH );
    node->children[bt] = allocated_leaf;

    KRowSetFindNearestLeaf ( self, nodes_stack, nodes_depth, nodes_stack_size, allocated_leaf, &nearest_leaf, &nearest_leaf_left );
    if ( nearest_leaf == NULL )
        DLListPushTail ( &self->leaf_nodes, &allocated_leaf->header.dad );
    else if (nearest_leaf_left)
        DLListInsertNodeAfter ( &self->leaf_nodes, &nearest_leaf->header.dad, &allocated_leaf->header.dad );
    else
        DLListInsertNodeBefore ( &self->leaf_nodes, &nearest_leaf->header.dad, &allocated_leaf->header.dad );

    ++self->number_leaves;
    assert ( KRowSetNodeIsTransitionMatches ( leaf_id, node_depth, &node->transitions[bt], NULL ) );
}

/**
 * Searches for a leaf by its id.
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
rc_t KRowSetGetLeaf ( KRowSet * self, int64_t leaf_id, bool insert_when_needed, KRowSetTreeLeaf ** leaf_found, KRowSetTreeNode * nodes_stack[], int nodes_depth[], int * nodes_stack_size )
{
    rc_t rc;
    KRowSetTreeNode * node;
    KRowSetTreeLeaf * new_leaf;
    uint8_t bt;
    int depth = 0;
    int nodes_stack_size_int;
    KRowSetTreeNode * nodes_stack_int[LEAF_DEPTH];
    int nodes_depth_int[LEAF_DEPTH];

    if ( self == NULL )
        return RC ( rcDB, rcRowSet, rcSelecting, rcSelf, rcNull );

    if ( leaf_found == NULL )
        return RC ( rcDB, rcRowSet, rcSelecting, rcParam, rcNull );

    if ( nodes_stack == NULL && nodes_depth == NULL && nodes_stack_size == NULL )
    {
        nodes_stack = nodes_stack_int;
        nodes_depth = nodes_depth_int;
        nodes_stack_size = &nodes_stack_size_int;
    }
    else if ( nodes_stack == NULL || nodes_depth == NULL || nodes_stack_size == NULL )
        return RC ( rcDB, rcRowSet, rcSelecting, rcParam, rcNull );

    *nodes_stack_size = 0;

    // empty tree
    if ( self->root_node == NULL )
    {
        KRowSetTreeNode * root;
        if ( !insert_when_needed )
            return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

        root = calloc ( 1, sizeof ( KRowSetTreeNode ) );
        if (root == NULL)
            return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );

        // pre-allocate leaf here
        rc = KRowSetAllocateLeaf ( LeafType_ArrayRanges, leaf_id, &new_leaf );
        if ( rc != 0 )
        {
            free ( root );
            return rc;
        }

#if CHECK_NODE_MARKS
        root->node_mark = NODE_MARK;
#endif

        self->root_node = root;

        node = self->root_node;
        nodes_stack[*nodes_stack_size] = node;
        nodes_depth[(*nodes_stack_size)++] = depth;

        KRowSetInsertLeaf ( self, leaf_id, node, depth, nodes_stack, nodes_depth, *nodes_stack_size, new_leaf );
        *leaf_found = new_leaf;
        return 0;
    }

    node = self->root_node;
    nodes_stack[*nodes_stack_size] = node;
    nodes_depth[(*nodes_stack_size)++] = depth;
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
            rc = KRowSetAllocateLeaf ( LeafType_ArrayRanges, leaf_id, &new_leaf );
            if ( rc != 0 )
                return rc;

            KRowSetInsertLeaf ( self, leaf_id, node, depth, nodes_stack, nodes_depth, *nodes_stack_size, new_leaf );
            *leaf_found = new_leaf;
            return 0;
        }

        // transition does not match, split the node
        if ( !KRowSetNodeIsTransitionMatches ( leaf_id, depth, &node->transitions[bt], &tr_size_matched ) )
        {
            KRowSetTreeNode * interm_node;
            int interm_node_depth = depth + 1 + tr_size_matched;

            if ( !insert_when_needed )
                return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

            // pre-allocate node and leaf here
            interm_node = calloc ( 1, sizeof ( KRowSetTreeNode ) );
            if ( interm_node == NULL )
                return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );

            rc = KRowSetAllocateLeaf ( LeafType_ArrayRanges, leaf_id, &new_leaf );
            if ( rc != 0 )
            {
                free ( interm_node );
                return rc;
            }

            KRowSetSplitNode ( node, depth, interm_node, interm_node_depth, leaf_id );

            node = interm_node;
            depth = interm_node_depth;
#if CHECK_NODE_MARKS
            assert ( node->node_mark == NODE_MARK );
#endif
            assert ( depth < LEAF_DEPTH );
            nodes_stack[*nodes_stack_size] = node;
            nodes_depth[(*nodes_stack_size)++] = depth;

            KRowSetInsertLeaf ( self, leaf_id, node, depth, nodes_stack, nodes_depth, *nodes_stack_size, new_leaf );
            *leaf_found = new_leaf;
            return 0;

        }

        // check if transition leads to a leaf or follow the transition
        if ( depth + 1 + tr_size_matched == LEAF_DEPTH )
        {
            *leaf_found = node->children[bt];
            return 0;
        }

        node = node->children[bt];
        depth += 1 + tr_size_matched;
#if CHECK_NODE_MARKS
        assert ( node->node_mark == NODE_MARK );
#endif
        assert ( depth < LEAF_DEPTH );
        nodes_stack[*nodes_stack_size] = node;
        nodes_depth[(*nodes_stack_size)++] = depth;
    }

    // "for" loop iterates up to a maximum possible depth,
    // so it must come to a leaf
    assert ( false );
    return 0;
}

/**
 * Transforms leaf from LeafType_ArrayRanges to LeafType_Bitmap
 */
static
rc_t KRowSetTreeLeafTransform ( KRowSet * self, KRowSetTreeLeaf ** p_leaf, KRowSetTreeNode * parent_node, int parent_depth )
{
    rc_t rc;
    int i;
    int j;
    int len;
    uint8_t parent_bt;
    uint16_t leaf_bt;
    KRowSetTreeLeaf * leaf;
    KRowSetTreeLeaf * new_leaf;

    assert ( p_leaf != NULL );
    leaf = *p_leaf;
    assert ( leaf != NULL );

    parent_bt = KRowSetGetLeafIdByte ( leaf->header.leaf_id, parent_depth );
    assert ( KRowSetNodeIsTransitionMatches ( leaf->header.leaf_id, parent_depth, &parent_node->transitions[parent_bt], NULL ) );

    rc = KRowSetAllocateLeaf( LeafType_Bitmap, leaf->header.leaf_id, &new_leaf );
    if ( rc != 0 )
        return rc;

    // copy rows to a new leaf
    len = leaf->data.array_ranges.len;
    for ( i = 0; i < len; ++i )
    {
        struct KRowSetTreeLeafRowRange * range = &leaf->data.array_ranges.ranges[i];
        for ( j = range->start; j <= range->end; ++j )
        {
            leaf_bt = j;
            new_leaf->data.bitmap[leaf_bt >> 3] |= 1 << (leaf_bt & 7);
        }
    }
    new_leaf->header.leaf_rows = leaf->header.leaf_rows;

    DLListInsertNodeBefore ( &self->leaf_nodes, &leaf->header.dad, &new_leaf->header.dad );
    KRowSetLeafWhack ( &leaf->header.dad, &self->leaf_nodes );
    leaf = new_leaf;
    parent_node->children[parent_bt] = leaf;
    *p_leaf = leaf;

    return 0;
}

static
int KRowSetBitmapByteRows ( uint8_t bt )
{
    static int NIBBLE_LOOKUP[] = {
        0, 1, 1, 2, 1, 2, 2, 3,
        1, 2, 2, 3, 2, 3, 3, 4
    };
    return NIBBLE_LOOKUP[bt & 0x0F] + NIBBLE_LOOKUP[bt >> 4];
}


/**
 * NB- it may reallocate leaf in case when leaf is transformed,
 *     so leaf pointer may change as a result of this function
 */
static
rc_t KRowSetTreeLeafAddRows ( KRowSet * self, KRowSetTreeLeaf ** p_leaf, uint16_t leaf_row_start, uint16_t leaf_row_end, KRowSetTreeNode * parent_node, int parent_depth, int * rows_inserted )
{
    KRowSetTreeLeaf * leaf = *p_leaf;
    int i;
    uint16_t leaf_row;

    assert ( rows_inserted != NULL );
    assert ( leaf_row_end >= leaf_row_start );

    *rows_inserted = 0;

    switch ( leaf->header.type )
    {
    case LeafType_Bitmap:
    {
        uint8_t* mem_ptr;
        size_t mem_size;

        // try to align to the nearest 8 rows from left
        if ( (leaf_row_start | 7) <  leaf_row_end )
        {
            for ( i = leaf_row_start; i <= (leaf_row_start | 7); ++i )
            {
                leaf_row = i;
                if ( ( leaf->data.bitmap[leaf_row >> 3] & (1 << (leaf_row & 7)) ) != 0 )
                    continue;

                leaf->data.bitmap[leaf_row >> 3] |= 1 << (leaf_row & 7);
                ++leaf->header.leaf_rows;
                ++(*rows_inserted);
            }

            leaf_row_start = (leaf_row_start | 7) + 1;
        }

        // if we are left aligned and still have more than 8 rows,
        // then use memset to set multiple rows at once
        if ( (leaf_row_start & 7) == 0 && leaf_row_end + 1 - leaf_row_start >= 8 )
        {
            int range_rows_prev = 0;

            mem_ptr = &leaf->data.bitmap[leaf_row_start >> 3];
            mem_size = (leaf_row_end + 1 - leaf_row_start) / 8;

            assert ( mem_size > 0 );

            for ( i = 0; i < mem_size; ++i )
            {
                range_rows_prev += KRowSetBitmapByteRows ( mem_ptr[i] );
            }

            memset ( mem_ptr, 0xFF, mem_size );
            leaf_row_start += mem_size * 8;
            *rows_inserted += mem_size * 8 - range_rows_prev;
            leaf->header.leaf_rows += mem_size * 8 - range_rows_prev;
        }

        // insert all remaining rows
        {
            for ( i = leaf_row_start; i <= leaf_row_end; ++i )
            {
                leaf_row = i;
                if ( ( leaf->data.bitmap[leaf_row >> 3] & (1 << (leaf_row & 7)) ) != 0 )
                    continue;

                leaf->data.bitmap[leaf_row >> 3] |= 1 << (leaf_row & 7);
                ++leaf->header.leaf_rows;
                ++(*rows_inserted);
            }
            return 0;
        }

        break;
    }
    case LeafType_ArrayRanges:
    {
        int j;
        int insert_i;
        int len = leaf->data.array_ranges.len;
        int max_len = 8;
        for ( i = 0; i < len; ++i )
        {
            struct KRowSetTreeLeafRowRange * range = &leaf->data.array_ranges.ranges[i];

            // we can just increase existing range to the left
            if ( range->start > 0 && leaf_row_end >= range->start - 1 && leaf_row_start < range->start )
            {
                if ( i == 0 || (range - 1)->end < leaf_row_start )
                {
                    *rows_inserted += range->start - leaf_row_start;
                    leaf->header.leaf_rows += range->start - leaf_row_start;
                    range->start = leaf_row_start;
                }
            }

            // or to the right
            // but lets first check next range, maybe we can just merge them both
            if ( leaf_row_start <= range->end + 1 && leaf_row_end > range->end )
            {
                if ( i + 1 < len && leaf_row_end >= (range+1)->start - 1 )
                {
                    // just merge two ranges
                    *rows_inserted += (range+1)->start - range->end - 1;

                    range->end = (range+1)->end;

                    --len;
                    --leaf->data.array_ranges.len;
                    for ( j = i + 1; j < len; --j )
                    {
                        leaf->data.array_ranges.ranges[j] = leaf->data.array_ranges.ranges[j + 1];
                    }
                    --i;
                    continue;
                }

                leaf->header.leaf_rows += leaf_row_end - range->end;
                *rows_inserted += leaf_row_end - range->end;
                range->end = leaf_row_end;
            }

            // not found - insert new range
            if ( range->start > leaf_row_end && ( i == 0 || (range - 1)->end < leaf_row_start ) )
            {
                break;
            }
        }

        // check if we are done
        if ( i > 0 && leaf->data.array_ranges.ranges[i-1].end >= leaf_row_end )
            break;

        // came here because we need to insert a new range
        // (or transform a leaf to a bitmap)
        insert_i = i;

        if ( len == max_len )
        {
            rc_t rc;
            rc = KRowSetTreeLeafTransform ( self, p_leaf, parent_node, parent_depth );
            if ( rc != 0 )
                return rc;

            return KRowSetTreeLeafAddRows ( self, p_leaf, leaf_row_start, leaf_row_end, parent_node, parent_depth, rows_inserted );
        }

        ++len;
        ++leaf->data.array_ranges.len;
        for ( i = len - 1; i > insert_i; --i )
        {
            leaf->data.array_ranges.ranges[i] = leaf->data.array_ranges.ranges[i - 1];
        }

        leaf->data.array_ranges.ranges[insert_i].start = leaf_row_start;
        leaf->data.array_ranges.ranges[insert_i].end = leaf_row_end;
        leaf->header.leaf_rows += leaf_row_end - leaf_row_start + 1;
        *rows_inserted += leaf_row_end - leaf_row_start + 1;

        break;
    }
    default:
        assert ( false );
    }

    return 0;
}

KDB_EXTERN rc_t CC KRowSetAddRowIdRange ( KRowSet * self, int64_t row_id_start,
    uint64_t row_id_count, uint64_t * optional_inserted )
{
    rc_t rc = 0;
    KRowSetTreeLeaf * leaf;
    int64_t current_range_start = row_id_start;
    uint64_t total_inserted = 0;
    uint16_t leaf_row_start;
    uint16_t leaf_row_end;
    int leaf_row_count;
    int leaf_rows_inserted;

    int nodes_stack_size;
    KRowSetTreeNode * nodes_stack[LEAF_DEPTH];
    int nodes_depth[LEAF_DEPTH];

    if ( row_id_start < 0 || row_id_count <= 0 )
        return RC ( rcDB, rcRowSet, rcInserting, rcParam, rcInvalid );

    if ( row_id_start + row_id_count < row_id_start )
        return RC ( rcDB, rcRowSet, rcInserting, rcParam, rcOutofrange );

    if ( optional_inserted != NULL )
        *optional_inserted = 0;

    while ( rc == 0 && current_range_start < row_id_start + row_id_count )
    {
        leaf_row_start = current_range_start & ROW_LEAF_MAX;
        leaf_row_count = ROW_LEAF_MAX - leaf_row_start + 1;
        if ( current_range_start + leaf_row_count > row_id_start + row_id_count )
            leaf_row_count = row_id_start + row_id_count - current_range_start;

        assert ( leaf_row_start + leaf_row_count - 1 <= ROW_LEAF_MAX );
        leaf_row_end = leaf_row_start + leaf_row_count - 1;

        rc = KRowSetGetLeaf ( self, current_range_start >> 16, true, &leaf, nodes_stack, nodes_depth, &nodes_stack_size );
        if ( rc == 0 )
        {
            rc = KRowSetTreeLeafAddRows ( self, &leaf, leaf_row_start, leaf_row_end, nodes_stack[nodes_stack_size-1], nodes_depth[nodes_stack_size-1], &leaf_rows_inserted );
        }

        if ( rc == 0 )
            assert ( leaf_row_count >= leaf_rows_inserted );
        else
            assert ( leaf_row_count > leaf_rows_inserted );

        current_range_start += leaf_row_count;
        total_inserted += leaf_rows_inserted;
    }

    self->number_rows += total_inserted;
    if ( optional_inserted != NULL )
        *optional_inserted = total_inserted;

    return rc;
}

KDB_EXTERN rc_t CC KRowSetAddRowId ( KRowSet * self, int64_t row_id, bool * optional_inserted )
{
    rc_t rc;
    uint64_t num_inserted;

    if ( optional_inserted != NULL )
        *optional_inserted = false;

    rc = KRowSetAddRowIdRange ( self, row_id, 1, &num_inserted );
    if ( rc != 0 )
        return rc;

    if ( optional_inserted != NULL )
        *optional_inserted = num_inserted > 0;

    return 0;
}

KDB_EXTERN rc_t CC KRowSetGetNumRowIds ( const KRowSet * self, uint64_t * num_rows )
{
    if ( self == NULL )
        return RC ( rcDB, rcRowSet, rcAccessing, rcSelf, rcNull );

    if ( num_rows == NULL )
        return RC ( rcDB, rcRowSet, rcAccessing, rcParam, rcNull );

    *num_rows = self->number_rows;
    return 0;
}

KDB_EXTERN rc_t CC KRowSetVisit ( const KRowSet * self, bool reverse,
    void ( CC * cb ) ( int64_t row_id, void * data ), void * data )
{
    rc_t rc;
    KRowSetIterator * it;

    if ( self == NULL )
        return RC ( rcDB, rcRowSet, rcAccessing, rcSelf, rcNull );

    if ( self->number_rows == 0 )
        return 0;

    rc = KRowSetMakeIterator ( self, &it );
    if ( rc != 0 )
        return rc;

    if ( !reverse )
        rc = KRowSetIteratorFirst ( it );
    else
        rc = KRowSetIteratorLast ( it );

    while ( rc == 0 )
    {
        int64_t row_id;
        rc = KRowSetIteratorRowId ( it, &row_id );
        if ( rc != 0 )
            break;

        cb ( row_id, data );

        if ( !reverse )
            rc = KRowSetIteratorNext ( it );
        else
            rc = KRowSetIteratorPrev ( it );

        if ( GetRCState ( rc ) == rcDone )
        {
            rc = 0;
            break;
        }
    }

    KRowSetIteratorRelease ( it );

    return rc;
}

/**
 * Tries to intersect array-range leaf (self) with bitmap leaf (other).
 * Returns true if succeeds to produce array-range result and stores it in target_ranges/target_rows
 */
static
bool KRowSetIntersectRowRangeAndBitmapLeaves ( KRowSet * self, const KRowSetTreeLeaf * self_leaf, const KRowSetTreeLeaf * other_leaf, struct KRowSetTreeLeafArrayRanges * target_ranges, int * target_rows )
{
    const struct KRowSetTreeLeafRowRange * self_range;
    const struct KRowSetTreeLeafRowRange * self_end_range;

    struct KRowSetTreeLeafRowRange * target_range;
    const struct KRowSetTreeLeafRowRange * target_first_range;
    const struct KRowSetTreeLeafRowRange * target_end_range;

    assert ( self_leaf != NULL );
    assert ( other_leaf != NULL );
    assert ( target_ranges != NULL );
    assert ( target_rows != NULL );

#if CHECK_NODE_MARKS
    assert ( self_leaf->header.leaf_mark == LEAF_MARK );
    assert ( other_leaf->header.leaf_mark == LEAF_MARK );
#endif

    assert ( self_leaf->header.type == LeafType_ArrayRanges );
    assert ( other_leaf->header.type == LeafType_Bitmap );

    self_range = &self_leaf->data.array_ranges.ranges[0];
    self_end_range = &self_leaf->data.array_ranges.ranges[self_leaf->data.array_ranges.len];

    target_range = &target_ranges->ranges[0];
    target_first_range = &target_ranges->ranges[0];
    target_end_range = &target_ranges->ranges[8];

    target_ranges->len = 0;
    *target_rows = 0;

    int i;
    for ( i = 0; i < LEAF_DATA_SZ_BT; ++i )
    {
        uint16_t bt_first_row = i << 3;
        uint16_t bt_last_row = bt_first_row | 7;
        int j;

        if ( other_leaf->data.bitmap[i] == 0 )
            continue;

        while ( self_range != self_end_range && bt_first_row > self_range->end )
            ++self_range;

        if ( self_range == self_end_range || bt_first_row > self_range->end )
        {
            return true;
        }

        if ( bt_last_row < self_range->start )
        {
            i = (self_range->start >> 3) - 1;
            continue;
        }

        for ( j = 0; j < 8; ++j )
        {
            uint16_t bt_row = bt_first_row + j;
            if ( (other_leaf->data.bitmap[i] & (1 << j)) != 0 && bt_row <= self_range->end && bt_row >= self_range->start )
            {
                if ( target_range != target_first_range && (target_range-1)->end == bt_row - 1 )
                {
                    ++(target_range-1)->end;
                    ++(*target_rows);
                    continue;
                }

                if ( target_range == target_end_range )
                {
                    return false;
                }

                target_range->start = target_range->end = bt_row;
                ++(*target_rows);
                ++target_range;
                ++target_ranges->len;
            }
        }
    }


    return true;
}

/**
 * Tries to intersect array-range leaf (self) with array-range leaf (other).
 * Returns true if succeeds to produce array-range result and stores it in target_ranges/target_rows
 */
static
bool KRowSetIntersectRowRangeLeaves ( KRowSet * self, const KRowSetTreeLeaf * self_leaf, const KRowSetTreeLeaf * other_leaf, struct KRowSetTreeLeafArrayRanges * target_ranges, int * target_rows )
{
    const struct KRowSetTreeLeafRowRange * self_range;
    const struct KRowSetTreeLeafRowRange * self_end_range;
    const struct KRowSetTreeLeafRowRange * other_range;
    const struct KRowSetTreeLeafRowRange * other_end_range;

    struct KRowSetTreeLeafRowRange * target_range;
    const struct KRowSetTreeLeafRowRange * target_end_range;

    assert ( self_leaf != NULL );
    assert ( other_leaf != NULL );
    assert ( target_ranges != NULL );
    assert ( target_rows != NULL );

#if CHECK_NODE_MARKS
    assert ( self_leaf->header.leaf_mark == LEAF_MARK );
    assert ( other_leaf->header.leaf_mark == LEAF_MARK );
#endif

    assert ( self_leaf->header.type == LeafType_ArrayRanges );
    assert ( other_leaf->header.type == LeafType_ArrayRanges );

    self_range = &self_leaf->data.array_ranges.ranges[0];
    self_end_range = &self_leaf->data.array_ranges.ranges[self_leaf->data.array_ranges.len];
    other_range = &other_leaf->data.array_ranges.ranges[0];
    other_end_range = &other_leaf->data.array_ranges.ranges[other_leaf->data.array_ranges.len];

    target_range = &target_ranges->ranges[0];
    target_end_range = &target_ranges->ranges[8];

    target_ranges->len = 0;
    *target_rows = 0;

    while ( self_range != self_end_range && other_range != other_end_range )
    {
        if ( self_range->end < other_range->start )
        {
            ++self_range;
            continue;
        }

        if ( other_range->end < self_range->start )
        {
            ++other_range;
            continue;
        }

        if ( target_range == target_end_range )
            return false;

        target_range->start = MAX ( self_range->start, other_range->start );
        target_range->end = MIN ( self_range->end, other_range->end );
        *target_rows += target_range->end - target_range->start + 1;
        ++target_range;
        ++target_ranges->len;

        if ( self_range->end < other_range->end )
            ++self_range;
        else if ( other_range->end < self_range->end )
            ++other_range;
        else
        {
            ++self_range;
            ++other_range;
        }
    }

    return true;
}

/**
 * Performs mutable intersection of self_leaf with other_leaf.
 *  Result of intersection is saved to self_leaf.
 *
 * NB - it checks types of self and other leaves and performs intersection depending on types.
 */
static
rc_t KRowSetOpAndIntersectLeaf ( KRowSet * self, KRowSetTreeLeaf ** self_leaf_p, const KRowSetTreeLeaf * other_leaf )
{
    KRowSetTreeLeaf * self_leaf;
    int64_t leaf_id;

    assert ( self_leaf_p != NULL );
    self_leaf = *self_leaf_p;
    assert ( self_leaf != NULL );
    assert ( other_leaf != NULL );

#if CHECK_NODE_MARKS
    assert ( self_leaf->header.leaf_mark == LEAF_MARK );
    assert ( other_leaf->header.leaf_mark == LEAF_MARK );
#endif

    leaf_id = self_leaf->header.leaf_id;
    assert ( leaf_id == other_leaf->header.leaf_id );

    switch ( self_leaf->header.type )
    {
    case LeafType_Bitmap:
        if ( other_leaf->header.type == LeafType_Bitmap )
        {
            int i;
            for ( i = 0; i < LEAF_DATA_SZ_BT; ++i )
            {
                int rows_before = KRowSetBitmapByteRows ( self_leaf->data.bitmap[i] );
                int rows_after;
                self_leaf->data.bitmap[i] &= other_leaf->data.bitmap[i];
                rows_after = KRowSetBitmapByteRows ( self_leaf->data.bitmap[i] );
                self_leaf->header.leaf_rows -= rows_before - rows_after;
                self->number_rows -= rows_before - rows_after;
            }
            return 0;

        }

        if ( other_leaf->header.type == LeafType_ArrayRanges )
        {
            int i;
            const struct KRowSetTreeLeafRowRange * other_range = &other_leaf->data.array_ranges.ranges[0];
            const struct KRowSetTreeLeafRowRange * other_end_range = &other_leaf->data.array_ranges.ranges[other_leaf->data.array_ranges.len];
            for ( i = 0; i < LEAF_DATA_SZ_BT; ++i )
            {
                uint16_t bt_first_row = i << 3;
                uint16_t bt_last_row = bt_first_row | 7;
                int j;

                if ( self_leaf->data.bitmap[i] == 0 )
                    continue;

                while ( other_range != other_end_range && bt_first_row > other_range->end )
                    ++other_range;

                if ( other_range == other_end_range || bt_first_row > other_range->end )
                {
                    for ( ; i < LEAF_DATA_SZ_BT; ++i )
                    {
                        int bt_rows = KRowSetBitmapByteRows ( self_leaf->data.bitmap[i] );
                        self_leaf->data.bitmap[i] = 0;
                        self_leaf->header.leaf_rows -= bt_rows;
                        self->number_rows -= bt_rows;
                    }
                    break;
                }

                if ( bt_last_row < other_range->start )
                {
                    int bt_rows = KRowSetBitmapByteRows ( self_leaf->data.bitmap[i] );
                    self_leaf->data.bitmap[i] = 0;
                    self_leaf->header.leaf_rows -= bt_rows;
                    self->number_rows -= bt_rows;
                    continue;
                }

                for ( j = 0; j < 8; ++j )
                {
                    uint16_t bt_row = bt_first_row + j;
                    if ( (self_leaf->data.bitmap[i] & (1 << j))== 0 )
                        continue;

                    if ( bt_row > other_range->end || bt_row < other_range->start )
                    {
                        self_leaf->data.bitmap[i] &= ~(1 << j);
                        --self_leaf->header.leaf_rows;
                        --self->number_rows;
                    }
                }
            }
            return 0;
        }

        assert ( false );
        break;
    case LeafType_ArrayRanges:
    {
        struct KRowSetTreeLeafArrayRanges result_ranges;
        int result_rows;
        bool can_produce_ranges;
        if ( other_leaf->header.type == LeafType_Bitmap )
        {
            can_produce_ranges = KRowSetIntersectRowRangeAndBitmapLeaves ( self, self_leaf, other_leaf, &result_ranges, &result_rows );
        }
        else if ( other_leaf->header.type == LeafType_ArrayRanges )
        {
            can_produce_ranges = KRowSetIntersectRowRangeLeaves ( self, self_leaf, other_leaf, &result_ranges, &result_rows );
        }
        else
            assert ( false );

        if ( can_produce_ranges )
        {
            int i;
            self_leaf->data.array_ranges.len = result_ranges.len;
            for ( i = 0; i < result_ranges.len; ++i )
            {
                assert ( result_ranges.ranges[i].start <= result_ranges.ranges[i].end );
                self_leaf->data.array_ranges.ranges[i] = result_ranges.ranges[i];
            }
            self->number_rows += result_rows - self_leaf->header.leaf_rows;
            self_leaf->header.leaf_rows = result_rows;
            return 0;
        }

        // transform array range leaf into bitmap and perform operation once again
        {
            rc_t rc;

            int nodes_stack_size;
            KRowSetTreeNode * nodes_stack[LEAF_DEPTH];
            int nodes_depth[LEAF_DEPTH];
            KRowSetTreeLeaf * search_leaf;

            rc = KRowSetGetLeaf ( self, self_leaf->header.leaf_id, false, &search_leaf, nodes_stack, nodes_depth, &nodes_stack_size );
            if ( GetRCObject ( rc ) == rcItem && GetRCState ( rc ) == rcNotFound )
                return RC ( rcDB, rcRowSet, rcUpdating, rcItem, rcInconsistent );

            if ( rc != 0 )
                return rc;

            if ( search_leaf != self_leaf )
                return RC ( rcDB, rcRowSet, rcUpdating, rcItem, rcInconsistent );

            rc = KRowSetTreeLeafTransform ( self, self_leaf_p, nodes_stack[nodes_stack_size-1], nodes_depth[nodes_stack_size-1] );
            if ( rc != 0 )
                return rc;

            self_leaf = *self_leaf_p;

            return KRowSetOpAndIntersectLeaf ( self, self_leaf_p, other_leaf );
        }
        break;
    }
    default:
        assert ( false );
    }

    assert ( false );
    return 0;
}

/**
 * Removes a leaf (and its rows) from the tree and deallocates/collapses intermediate nodes when necessary
 */
static
rc_t KRowSetRemoveLeaf ( KRowSet * self, KRowSetTreeLeaf * leaf )
{
    rc_t rc;
    int i;
    int64_t leaf_id;

    int nodes_stack_size;
    KRowSetTreeNode * nodes_stack[LEAF_DEPTH];
    int nodes_depth[LEAF_DEPTH];
    KRowSetTreeLeaf * search_leaf;

    assert ( leaf != NULL );
#if CHECK_NODE_MARKS
        assert ( leaf->header.leaf_mark == LEAF_MARK );
#endif
    leaf_id = leaf->header.leaf_id;

    rc = KRowSetGetLeaf ( self, leaf->header.leaf_id, false, &search_leaf, nodes_stack, nodes_depth, &nodes_stack_size );
    if ( GetRCObject ( rc ) == rcItem && GetRCState ( rc ) == rcNotFound )
        return RC ( rcDB, rcRowSet, rcUpdating, rcItem, rcInconsistent );

    if ( rc != 0 )
        return rc;

    if ( search_leaf != leaf )
        return RC ( rcDB, rcRowSet, rcUpdating, rcItem, rcInconsistent );

    for ( i = nodes_stack_size - 1; i >= 0; --i )
    {
        int number_children;
        int j;
        int found_j; // this will be used in case when there is only a single child exists
        uint8_t bt;

        KRowSetTreeNode * node = nodes_stack[i];
        assert ( node != NULL );
#if CHECK_NODE_MARKS
        assert ( node->node_mark == NODE_MARK );
#endif

        bt = KRowSetGetLeafIdByte ( leaf_id, nodes_depth[i] );

        assert ( node->children[bt] != NULL );

        node->children[bt] = NULL;
        node->transitions[bt].size = 0;

        number_children = 0;
        found_j = -1;
        for ( j = 0; j < 256; ++j )
        {
            if ( node->children[j] != NULL )
            {
                ++number_children;
                found_j = j;
            }
        }

        if ( number_children == 0 )
        {
            assert ( i != 0 || node == self->root_node );
            KRowSetNodeWhack ( self, node, nodes_depth[i], false );
            if ( i == 0 )
                self->root_node = NULL;
            continue;
        }

        // it has only a single child, need to collapse
        if ( number_children == 1 && i > 0 )
            KRowSetCollapseNode ( nodes_stack[i-1], nodes_depth[i-1], node, nodes_depth[i], found_j, leaf_id );

        break;
    }

    self->number_rows -= leaf->header.leaf_rows;
    --self->number_leaves;
    KRowSetLeafWhack ( (DLNode *) leaf, &self->leaf_nodes );

    return 0;
}

KDB_EXTERN rc_t CC KRowSetOpAnd ( KRowSet * self, const KRowSet * other )
{
    rc_t rc;
    KRowSetTreeLeaf * self_leaf;
    const KRowSetTreeLeaf * other_leaf;

    if ( self == NULL )
        return RC ( rcDB, rcRowSet, rcUpdating, rcSelf, rcNull );

    if ( other == NULL )
        return RC ( rcDB, rcRowSet, rcUpdating, rcParam, rcNull );

    self_leaf = (KRowSetTreeLeaf *) KRowSetTreeGetFirstLeaf ( self );
    other_leaf = KRowSetTreeGetFirstLeaf ( other );

    while ( self_leaf != NULL )
    {
        KRowSetTreeLeaf * next_leaf;
        if ( other_leaf == NULL || other_leaf->header.leaf_id > self_leaf->header.leaf_id )
        {
            next_leaf = (KRowSetTreeLeaf *) KRowSetTreeLeafGetNext ( self_leaf );
            KRowSetRemoveLeaf ( self, self_leaf );
            self_leaf = next_leaf;
            continue;
        }

        if ( other_leaf->header.leaf_id < self_leaf->header.leaf_id )
        {
            other_leaf = KRowSetTreeLeafGetNext ( other_leaf );
            continue;
        }

        // leafs are aligned
        assert ( other_leaf->header.leaf_id == self_leaf->header.leaf_id );
        rc = KRowSetOpAndIntersectLeaf ( self, &self_leaf, other_leaf );
        if ( rc != 0 )
            return rc;

        other_leaf = KRowSetTreeLeafGetNext ( other_leaf );

        next_leaf = (KRowSetTreeLeaf *) KRowSetTreeLeafGetNext ( self_leaf );
        if ( self_leaf->header.leaf_rows == 0 )
            KRowSetRemoveLeaf ( self, self_leaf );
        self_leaf = next_leaf;
    }

    return 0;
}

const KRowSetTreeLeaf * KRowSetTreeGetFirstLeaf ( const KRowSet * self )
{
    assert ( self != NULL );
    return (const KRowSetTreeLeaf * )DLListHead ( &self->leaf_nodes );
}

const KRowSetTreeLeaf * KRowSetTreeGetLastLeaf ( const KRowSet * self )
{
    assert ( self != NULL );
    return (const KRowSetTreeLeaf * )DLListTail ( &self->leaf_nodes );
}

const KRowSetTreeLeaf * KRowSetTreeLeafGetNext ( const KRowSetTreeLeaf * leaf )
{
    assert ( leaf != NULL );
    return (const KRowSetTreeLeaf * ) DLNodeNext ( &leaf->header.dad );
}

const KRowSetTreeLeaf * KRowSetTreeLeafGetPrev ( const KRowSetTreeLeaf * leaf )
{
    assert ( leaf != NULL );
    return (const KRowSetTreeLeaf * ) DLNodePrev ( &leaf->header.dad );
}
