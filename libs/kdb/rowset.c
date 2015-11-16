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
        rc = RC ( rcDB, rcRowSet, rcConstructing, rcParam, rcNull );
    else
    {
        rowset = calloc( 1, sizeof *rowset );
        if ( rowset == NULL )
            rc = RC ( rcDB, rcRowSet, rcConstructing, rcMemory, rcExhausted );
        else
        {
            KRefcountInit ( &rowset->refcount, 1, "KRowSet", "new", "" );

            *self = rowset;
            return 0;
        }
    }
    
    return rc;
}

/* Whack
 */

// pass KRowSet
static
void KRowSetLeafWhack ( RowSetLeaf * leaf, int depth )
{
#if CHECK_NODE_MARKS
	assert ( leaf->leaf_mark == LEAF_MARK );
#endif
	assert ( depth == 6 );

	free ( leaf );
    // remove from the DL list
}

static
void KRowSetNodeWhack ( RowSetNode * node, int depth )
{
	int i;

	assert ( depth == node->depth );

#if CHECK_NODE_MARKS
	assert ( node->node_mark == NODE_MARK );
#endif
	assert ( depth < 6 );

	for ( i = 0; i < 256; ++i )
	{
		assert ( (i & 0xFF) == i );

		if ( node->children[i] )
		{
			if ( depth + 1 < 6 ) {
				KRowSetNodeWhack ( node->children[i], depth + 1 );
			} else {
				KRowSetLeafWhack ( node->children[i], depth + 1 );
			}

		}
	}

	free ( node );
}

static
rc_t KRowSetWhack ( KRowSet *self )
{
	if ( self->root_node )
		KRowSetNodeWhack ( self->root_node, 0 );
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
rc_t KRowSetGetLeaf ( KRowSet * self, int64_t row_id, bool insert_when_needed, RowSetLeaf ** leaf_found )
{
	rc_t rc = 0;
	int i;
	RowSetNode* node;
	RowSetLeaf* leaf;
	uint8_t bt;
	bool smart_leaves_search;

	RowSetNode* inserting_subtree = NULL;
	RowSetNode** inserting_subtree_place;
	int inserting_subtree_at_depth;

	if ( self == NULL )
		return RC ( rcDB, rcRowSet, rcInserting, rcParam, rcNull );

	if ( leaf_found == NULL )
		return RC ( rcDB, rcRowSet, rcInserting, rcParam, rcNull );

	// we may change this approach later
	smart_leaves_search = self->number_leaves > 2;

	/* empty tree */
	if ( self->root_node == NULL )
	{
		RowSetNode* root = calloc ( 1, sizeof ( RowSetNode ) );
		if (root == NULL)
			return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );

#if CHECK_NODE_MARKS
		root->node_mark = NODE_MARK;
#endif
		root->depth = 0;

		self->root_node = root;
	}

	node = self->root_node;
	for ( i = 7; i > 2; --i )
	{
		bt = (row_id >> i * 8) & 0xFF;
		if ( node->children[bt] == NULL )
		{
			RowSetNode* new_node = calloc ( 1, sizeof ( RowSetNode ) );
			if (new_node == NULL)
			{
				rc = RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );
				break;
			}

#if CHECK_NODE_MARKS
			new_node->node_mark = NODE_MARK;
#endif
			new_node->depth = 8 - i;

			/* let's remember where we insert a subtree.
			   later if we fail, we will be able to cleanup */
			if ( inserting_subtree == NULL )
			{
				inserting_subtree = new_node;
				inserting_subtree_place = (RowSetNode **)&node->children[bt];
				inserting_subtree_at_depth = 8 - i;
			}

			node->children[bt] = new_node;
		}
		node = node->children[bt];
	}

	if ( rc == 0 )
	{
		/* just to allow "break" inside the block */
		do {
			bt = (row_id >> 16) & 0xFF;
			if ( node->children[bt] == NULL )
			{
				RowSetLeaf* new_leaf = calloc ( 1, sizeof ( RowSetLeaf ) );
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
		KRowSetNodeWhack ( inserting_subtree, inserting_subtree_at_depth );
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
		return RC ( rcDB, rcRowSet, rcAccessing, rcParam, rcNull );

	if ( num_rows == NULL )
			return RC ( rcDB, rcRowSet, rcAccessing, rcParam, rcNull );

	*num_rows = self->number_rows;
	return 0;
}

static
void KRowSetBrowseRowsLeafs ( RowSetLeaf * leaf, int depth, uint64_t row_id )
{
	int i;
	int j;

#if CHECK_NODE_MARKS
	assert ( leaf->leaf_mark == LEAF_MARK );
#endif
	assert ( leaf->leaf_id == row_id );
	assert ( depth == 6 );

	row_id <<= 16;

	for ( i = 0; i < LEAF_DATA_SZ_BT; ++i )
	{
		if ( leaf->data[i] )
		{
			uint64_t new_row_id = row_id | i << 3;
			for (j = 0; j < 8; ++j)
			{
				if ( leaf->data[i] & (1 << j) )
				{
					KOutMsg("Found row_id: %ld\n", (int64_t)(new_row_id | j));
				}
			}
		}
	}
}

static
void KRowSetBrowseRowsNodes ( RowSetNode * node, int depth, uint64_t row_id )
{
	int i;

#if CHECK_NODE_MARKS
	assert ( node->node_mark == NODE_MARK );
#endif
	assert ( node->depth == depth );
	assert ( depth < 6 );

	row_id <<= 8;

	for ( i = 0; i < 256; ++i )
	{
		assert ( (i & 0xFF) == i );

		if ( node->children[i] )
		{
			if ( depth + 1 < 6 )
				KRowSetBrowseRowsNodes ( node->children[i], depth + 1, row_id | i );
			else
				KRowSetBrowseRowsLeafs ( node->children[i], depth + 1, row_id | i );
		}
	}
}

KDB_EXTERN rc_t CC KRowSetPrintRows ( const KRowSet * self )
{
	if ( self == NULL )
		return RC ( rcDB, rcRowSet, rcAccessing, rcParam, rcNull );

	KOutMsg("Searching RowSet rows:\n");
	if ( self->root_node == NULL )
	{
		KOutMsg("Empty RowSet\n");
		return 0;
	}

	KRowSetBrowseRowsNodes ( self->root_node, 0, 0 );
	return 0;
}
