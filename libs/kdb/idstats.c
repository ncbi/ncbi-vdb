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

#include <kdb/extern.h>

#include "idstats-priv.h"

#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KIdStatsNode
 */
typedef struct KIdStatsNode KIdStatsNode;
struct KIdStatsNode
{
    BSTNode n;
    int64_t i_left, x_right;
};


/* Init
 */
static
void KIdStatsNodeInit ( KIdStatsNode *node, int64_t id, uint64_t count )
{
    if ( node != NULL )
    {
        node -> i_left = id;
        node -> x_right = id + count;
    }
}


/* Make
 */
static
KIdStatsNode *KIdStatsNodeMake ( int64_t id, uint64_t count )
{
    KIdStatsNode *node = malloc ( sizeof * node );
    KIdStatsNodeInit ( node, id, count );
    return node;
}

/* Whack
 */
static
void CC KIdStatsNodeWhack ( BSTNode *n, void *ignore )
{
    free ( n );
}

/* Cmp
 * Sort
 */
static
int64_t CC KIdStatsNodeCmp ( const void *item, const BSTNode *n )
{
    const KIdStatsNode *a = item;
    const KIdStatsNode *b = ( const KIdStatsNode* ) n;

    if ( a -> x_right < b -> i_left )
        return -1;
    if ( a -> i_left > b -> x_right )
        return 1;
    return 0;
}

static
int64_t CC KIdStatsNodeSort ( const BSTNode *item, const BSTNode *n )
{
    const KIdStatsNode *a = ( const KIdStatsNode* ) item;
    const KIdStatsNode *b = ( const KIdStatsNode* ) n;

    if ( a -> x_right < b -> i_left )
        return -1;
    if ( a -> i_left > b -> x_right )
        return 1;
    return 0;
}

/*--------------------------------------------------------------------------
 * KIdStats
 *  maintains statistics about text index mappings
 */

/* Init
 *  initialize the object
 */
void KIdStatsInit ( KIdStats *s )
{
    if ( s != NULL )
        memset ( s, 0, sizeof * s );
}

/* Whack
 *  tear down the object
 */
void KIdStatsWhack ( KIdStats *self )
{
    if ( self != NULL )
    {
        BSTreeWhack ( & self -> ids, KIdStatsNodeWhack, NULL );
        BSTreeInit ( & self -> ids );
    }
}

/* Insert
 *  add an entry representing 1 or more consecutive ids
 */
rc_t KIdStatsInsert ( KIdStats *self, int64_t id, uint64_t count )
{
    rc_t rc;
    KIdStatsNode *node, *existing;

    /* could be an assert - but here we go */
    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcValidating, rcParam, rcNull );

    /* if this is not the first entry */
    if ( self -> num_entries != 0 )
    {
        KIdStatsNode item;
        KIdStatsNodeInit ( & item, id, count );
        existing = ( KIdStatsNode* ) BSTreeFind ( & self -> ids, & item, KIdStatsNodeCmp );
        if ( existing != NULL )
        {
            /* min/max the range */
            if ( id < self -> i_min_id )
                self -> i_min_id = id;
            if ( ( int64_t ) ( id + count ) > self -> x_max_id )
                self -> x_max_id = id + count;
            ++ self -> num_entries;

            /* merge this insert with the returned node */
            if ( existing -> i_left > item . i_left )
            {
                self -> num_ids += existing -> i_left - item . i_left;
                existing -> i_left = item . i_left;

                /* merge node toward left */
                for ( node = ( KIdStatsNode* ) BSTNodePrev ( & existing -> n );
                      node != NULL;
                      node = ( KIdStatsNode* ) BSTNodePrev ( & existing -> n ) )
                {
                    /* exit loop if no intersection */
                    if ( existing -> i_left > node -> x_right )
                        break;

                    /* a hole will be closed */
                    assert ( self -> num_holes > 0 );
                    -- self -> num_holes;

                    /* remove overlaps from count and extend "existing" */
                    if ( existing -> i_left <= node -> i_left )
                        self -> num_ids -= node -> x_right - node -> i_left;
                    else
                    {
                        self -> num_ids -= node -> x_right - existing -> i_left;
                        existing -> i_left = node -> i_left;
                    }

                    /* drop old node */
                    BSTreeUnlink ( & self -> ids, & node -> n );
                    KIdStatsNodeWhack ( & node -> n, NULL );
                }
            }

            /* now merge right edge */
            if ( item . x_right > existing -> x_right )
            {
                self -> num_ids += item . x_right - existing -> x_right;
                existing -> x_right = item . x_right;

                /* merge node toward right */
                for ( node = ( KIdStatsNode* ) BSTNodeNext ( & existing -> n );
                      node != NULL;
                      node = ( KIdStatsNode* ) BSTNodeNext ( & existing -> n ) )
                {
                    /* exit loop if no intersection */
                    if ( existing -> x_right < node -> i_left )
                        break;

                    /* a hole will be closed */
                    assert ( self -> num_holes > 0 );
                    -- self -> num_holes;

                    /* remove overlaps from count and extend "existing" */
                    if ( existing -> x_right >= node -> x_right )
                        self -> num_ids -= node -> x_right - node -> i_left;
                    else
                    {
                        self -> num_ids -= existing -> x_right - node -> i_left;
                        existing -> x_right = node -> x_right;
                    }

                    /* drop old node */
                    BSTreeUnlink ( & self -> ids, & node -> n );
                    KIdStatsNodeWhack ( & node -> n, NULL );
                }
            }

            return 0;
        }
    }

    /* create the node */
    node = KIdStatsNodeMake ( id, count );
    if ( node == NULL )
        return RC ( rcDB, rcIndex, rcValidating, rcMemory, rcExhausted );

    /* insert it and be certain there are no collisions */
    rc = BSTreeInsertUnique ( & self -> ids, & node -> n,
        ( BSTNode** ) & existing, KIdStatsNodeSort );

    if ( rc != 0 )
        KIdStatsNodeWhack ( & node -> n, NULL );
    else if ( self -> num_entries == 0 )
    {
        /* initial insert */
        self -> i_min_id = id;
        self -> x_max_id = id + count;
        self -> num_entries = 1;
        self -> num_ids = count;
    }
    else
    {
        /* min/max the range */
        if ( id < self -> i_min_id )
            self -> i_min_id = id;
        if ( ( int64_t ) ( id + count ) > self -> x_max_id )
            self -> x_max_id = id + count;
        ++ self -> num_entries;
        self -> num_ids += count;

        /* the insert represents a hole */
        ++ self -> num_holes;
    }

    return rc;
}
