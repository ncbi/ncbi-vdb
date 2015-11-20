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
			bt = (leaf_id >> 8 * (LEAF_DEPTH - depth - 1)) & 0xFF;

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

static
inline bool KRowSetNodeIsTransitionMatches ( int64_t leaf_id, int node_depth, const RowSetNodeTransition * tr, int * size_matched )
{
	int matched;

	assert ( node_depth + tr->size < LEAF_DEPTH );

	for ( matched = 0; matched < tr->size; ++matched )
	{
		if ( (leaf_id >> (LEAF_DEPTH - matched - node_depth - 2) * 8 & 0xFF) != tr->data[matched] )
			break;
	}

	if ( size_matched != NULL )
		*size_matched = matched;

	return matched == tr->size;
}

static
void KRowSetNodeSetChild ( RowSetNode * node, void * child, int64_t leaf_id, int node_depth, int child_depth )
{
	uint8_t bt = (leaf_id >> (LEAF_DEPTH - 1 - node_depth) * 8) & 0xFF;
	int i;

	node->children[bt] = child;
	node->transitions[bt].size = child_depth - 1 - node_depth;

	for ( i = node_depth; i < child_depth - 1; ++i )
	{
		node->transitions[bt].data[i - node_depth] = leaf_id >> (LEAF_DEPTH - i - 2) * 8 & 0xFF;
	}
}

static
rc_t KRowSetGetLeaf ( KRowSet * self, int64_t row_id, bool insert_when_needed, RowSetLeaf ** leaf_found )
{
	rc_t rc = 0;
	int depth;
	RowSetNode * node;
	RowSetLeaf * leaf;
	RowSetLeaf * new_leaf = NULL;
	int64_t leaf_id = row_id >> 16;
	uint8_t bt;
	int nodes_stack_size = 0;
	RowSetNode * nodes_stack[LEAF_DEPTH];
	int nodes_depth[LEAF_DEPTH];

	if ( self == NULL )
		return RC ( rcDB, rcRowSet, rcSelecting, rcSelf, rcNull );

	if ( leaf_found == NULL )
		return RC ( rcDB, rcRowSet, rcSelecting, rcParam, rcNull );

	depth = 0;
	memset ( nodes_stack, 0, sizeof nodes_stack );

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
		bt = (leaf_id >> (LEAF_DEPTH - depth - 1) * 8) & 0xFF;
		if ( node->children[bt] == NULL )
		{
			if ( !insert_when_needed )
				return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

			// pre-allocate leaf here if it wasn't allocated before and exit "for" loop
			if ( new_leaf == NULL )
				new_leaf = calloc ( 1, sizeof ( RowSetLeaf ) );
			if (new_leaf == NULL)
				return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );
			break;
		}

		if ( KRowSetNodeIsTransitionMatches ( leaf_id, depth, &node->transitions[bt], &tr_size_matched ) )
		{
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
		else
		{
			RowSetNode * interm_node;
			RowSetNode * old_node;
			int old_depth;
			uint8_t old_bt;

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

#if CHECK_NODE_MARKS
			interm_node->node_mark = NODE_MARK;
#endif

			old_node = node->children[bt];
			old_depth = depth + 1 + node->transitions[bt].size;

			// TODO: move this to a separate function
			old_bt = node->transitions[bt].data[tr_size_matched];
			interm_node->transitions[old_bt].size = node->transitions[bt].size - tr_size_matched - 1;
			for ( int i = 0; i < interm_node->transitions[old_bt].size; ++i )
			{
				interm_node->transitions[old_bt].data[i] = node->transitions[bt].data[tr_size_matched + i + 1];
			}
			interm_node->children[old_bt] = old_node;

			KRowSetNodeSetChild ( node, interm_node, leaf_id, depth, depth + 1 + tr_size_matched );


			node = interm_node;
			depth = depth + 1 + tr_size_matched;
#if CHECK_NODE_MARKS
			assert ( node->node_mark == NODE_MARK );
#endif
			assert ( depth < sizeof nodes_stack / sizeof nodes_stack[0] );
			nodes_stack[nodes_stack_size] = node;
			nodes_depth[nodes_stack_size++] = depth;

			break;
		}
	}

	if ( rc != 0 )
		return rc;

	bt = (leaf_id >> (LEAF_DEPTH - depth - 1) * 8) & 0xFF;

	assert ( new_leaf != NULL );
	assert ( node->children[bt] == NULL );

	{
		RowSetLeaf * nearest_leaf;
		bool nearest_leaf_left;

#if CHECK_NODE_MARKS
		new_leaf->leaf_mark = LEAF_MARK;
#endif
		new_leaf->leaf_id = leaf_id;

		KRowSetNodeSetChild ( node, new_leaf, leaf_id, depth, LEAF_DEPTH );
		node->children[bt] = new_leaf;

		KRowSetFindNearestLeaf ( self, nodes_stack, nodes_depth, new_leaf, &nearest_leaf, &nearest_leaf_left );
		if ( nearest_leaf == NULL )
			DLListPushTail ( &self->leaf_nodes, &new_leaf->dad );
		else if (nearest_leaf_left)
			DLListInsertNodeAfter ( &self->leaf_nodes, &nearest_leaf->dad, &new_leaf->dad );
		else
			DLListInsertNodeBefore ( &self->leaf_nodes, &nearest_leaf->dad, &new_leaf->dad );

		++self->number_leaves;
	}
	assert ( KRowSetNodeIsTransitionMatches ( leaf_id, depth, &node->transitions[bt], NULL ) );
	leaf = node->children[bt];

	*leaf_found = leaf;

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

static
void KRowSetWalkLeafRows ( DLNode *n, void *data )
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
