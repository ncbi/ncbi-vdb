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

struct RowSetNode
{
	void* children[256];
	uint8_t depth;
#if CHECK_NODE_MARKS
	uint8_t node_mark;
#endif
};

struct RowSetLeaf
{
    DLNode dad;
	int64_t leaf_id; /* this is basically a row_id >> 16 */
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
	assert ( depth == node->depth );

#if CHECK_NODE_MARKS
	assert ( node->node_mark == NODE_MARK );
#endif
	assert ( depth < LEAF_DEPTH );

	for ( i = 0; i < 256; ++i )
	{
		assert ( (i & 0xFF) == i );

		if ( node->children[i] )
		{
			if ( depth + 1 < LEAF_DEPTH ) {
				KRowSetNodeWhack ( self, node->children[i], depth + 1, free_leaves );
			} else if ( free_leaves ) {
				KRowSetLeafWhack ( &((RowSetLeaf*) node->children[i])->dad, &self->leaf_nodes );
			}
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
void KRowSetFindNearestLeaf ( KRowSet * self, RowSetNode ** nodes_stack, RowSetLeaf * leaf, RowSetLeaf ** result, bool * result_left )
{
	bool smart_leaves_search;

	assert ( self != NULL );
	assert ( nodes_stack != NULL );
	assert ( leaf != NULL );
	assert ( result != NULL );
	assert ( result_left != NULL );

	*result = NULL;

	// we may change this approach later
	smart_leaves_search = self->number_leaves > 2;

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
			int j;
			int j_max;
			uint8_t bt = (leaf_id >> 8 * (LEAF_DEPTH - i - 1)) & 0xFF;
			RowSetNode * current_node = nodes_stack[i];
			void * nearest_subtree = NULL;
			bool nearest_subtre_left;
			assert ( current_node != NULL );
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
					break;
				}

				if ( bt - j >= 0 && current_node->children[bt - j] != NULL )
				{
					nearest_subtre_left = true;
					nearest_subtree = current_node->children[bt - j];
					break;
				}
			}

			if ( nearest_subtree != NULL )
			{
				for ( j = i; j < LEAF_DEPTH - 1; ++j )
				{
					int search_i;
					int search_start = nearest_subtre_left ? 255 : 0;
					int search_stop = nearest_subtre_left ? 0 : 255;
					int search_step = nearest_subtre_left ? -1 : 1;
					void * nearest_subtree_next = NULL;

#if CHECK_NODE_MARKS
					assert ( ((RowSetNode *) nearest_subtree)->node_mark == NODE_MARK );
#endif

					for ( search_i = search_start; search_step > 0 ? search_i <= search_stop : search_i >= search_stop; search_i += search_step )
					{
						if ( ((RowSetNode *) nearest_subtree)->children[search_i] != NULL )
						{
							nearest_subtree_next = ((RowSetNode *) nearest_subtree)->children[search_i];
							break;
						}
					}

					assert ( nearest_subtree_next != NULL );
					nearest_subtree = nearest_subtree_next;
				}

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
rc_t KRowSetGetLeaf ( KRowSet * self, int64_t row_id, bool insert_when_needed, RowSetLeaf ** leaf_found )
{
	rc_t rc = 0;
	int i;
	int depth;
	RowSetNode* node;
	RowSetLeaf* leaf;
	uint8_t bt;
	RowSetNode * nodes_stack[LEAF_DEPTH];

	RowSetNode* inserting_subtree = NULL;
	RowSetNode** inserting_subtree_place;
	int inserting_subtree_at_depth;

	if ( self == NULL )
		return RC ( rcDB, rcRowSet, rcSelecting, rcSelf, rcNull );

	if ( leaf_found == NULL )
		return RC ( rcDB, rcRowSet, rcSelecting, rcParam, rcNull );

	/* empty tree */
	if ( self->root_node == NULL )
	{
		RowSetNode* root;
		if ( !insert_when_needed )
			return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

		root = calloc ( 1, sizeof ( RowSetNode ) );
		if (root == NULL)
			return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );

#if CHECK_NODE_MARKS
		root->node_mark = NODE_MARK;
#endif
		root->depth = 0;

		self->root_node = root;
	}

	depth = 0;
	node = self->root_node;
	nodes_stack[depth] = node;
	for ( i = 7; i > 8 - LEAF_DEPTH; --i )
	{
		++depth;
		bt = (row_id >> i * 8) & 0xFF;
		if ( node->children[bt] == NULL )
		{
			RowSetNode* new_node;
			if ( !insert_when_needed )
				return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

			new_node = calloc ( 1, sizeof ( RowSetNode ) );
			if (new_node == NULL)
			{
				rc = RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );
				break;
			}

#if CHECK_NODE_MARKS
			new_node->node_mark = NODE_MARK;
#endif
			new_node->depth = depth;

			/* let's remember where we insert a subtree.
			   later if we fail, we will be able to cleanup */
			if ( inserting_subtree == NULL )
			{
				inserting_subtree = new_node;
				inserting_subtree_place = (RowSetNode **)&node->children[bt];
				inserting_subtree_at_depth = depth;
			}

			node->children[bt] = new_node;
		}
		node = node->children[bt];
		assert ( depth < sizeof nodes_stack / sizeof nodes_stack[0] );
		nodes_stack[depth] = node;
	}

	if ( rc == 0 )
	{
		/* just to allow "break" inside the block */
		do {
			++depth;
			bt = (row_id >> 16) & 0xFF;
			if ( node->children[bt] == NULL )
			{
				RowSetLeaf * nearest_leaf;
				bool nearest_leaf_left;
				RowSetLeaf * new_leaf;

				if ( !insert_when_needed )
					return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

				new_leaf = calloc ( 1, sizeof ( RowSetLeaf ) );
				if (new_leaf == NULL)
				{
					rc = RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );
					break;
				}
#if CHECK_NODE_MARKS
				new_leaf->leaf_mark = LEAF_MARK;
#endif
				new_leaf->leaf_id = row_id >> 16;

				node->children[bt] = new_leaf;

				KRowSetFindNearestLeaf ( self, nodes_stack, new_leaf, &nearest_leaf, &nearest_leaf_left );
				if ( nearest_leaf == NULL )
					DLListPushTail ( &self->leaf_nodes, &new_leaf->dad );
				else if (nearest_leaf_left)
					DLListInsertNodeAfter ( &self->leaf_nodes, &nearest_leaf->dad, &new_leaf->dad );
				else
					DLListInsertNodeBefore ( &self->leaf_nodes, &nearest_leaf->dad, &new_leaf->dad );

				++self->number_leaves;
			}
			leaf = node->children[bt];

			*leaf_found = leaf;
		}
		while(false);
	}

	if ( rc != 0 && inserting_subtree != NULL )
	{
		*inserting_subtree_place = NULL;
		KRowSetNodeWhack ( self, inserting_subtree, inserting_subtree_at_depth, true );
	}

	return rc;
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
