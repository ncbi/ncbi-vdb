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

/*--------------------------------------------------------------------------
 * KRowSetIterator
 */

struct KRowSetIterator
{
    KRefcount refcount;
    const KRowSet * rowset;
    const KRowSetTreeLeaf * leaf; // will be NULL in empty iterator
    int64_t current_row_id;
    ptrdiff_t eol; // end of "list"; will be non-zero when there are no more rows and we moved after valid row-id sequence dimensions

    // only for leaf type RangeArray
    struct {
        int8_t current_range_index;
    } range_array;
};

static
void KRowSetIteratorStart ( KRowSetIterator * self )
{
    assert ( self != NULL );
    self->current_row_id = -1;
    self->leaf = KRowSetTreeGetFirstLeaf ( self->rowset );
    self->eol = self->leaf == NULL ? -1 : 0;
}

static
void KRowSetIteratorEnd ( KRowSetIterator * self )
{
    assert ( self != NULL );
    self->current_row_id = -1;
    self->leaf = KRowSetTreeGetLastLeaf ( self->rowset );
    self->eol = self->leaf == NULL ? 1 : 0;
}

static
bool CC KRowSetIteratorMove ( KRowSetIterator * iter, bool backward )
{
    int step = !backward ? 1 : -1;

    assert ( iter->rowset != NULL );

    if ( iter->eol != 0 )
    {
        iter->eol += step;
        if ( iter->eol == 0 )
        {
            if ( !backward )
                KRowSetIteratorStart ( iter );
            else
                KRowSetIteratorEnd ( iter );
        }
    }

    if ( iter->eol != 0 )
        return false;

    assert ( iter->leaf != NULL );

    while ( iter->eol == 0 )
    {
        int i;
        int j;
        int64_t last_row_id;
        int last_leaf_bt = -1;
        int64_t leaf_id;
        int64_t row_id;
        const KRowSetTreeLeaf * leaf = iter->leaf;

        last_row_id = iter->current_row_id;
        if ( last_row_id != -1 )
            last_leaf_bt = last_row_id & 0xFFFF;

        leaf_id = leaf->header.leaf_id;
        row_id = leaf_id << 16;

        switch ( leaf->header.type )
        {
        case LeafType_Bitmap:
        {
            int from_i;
            int from_j;
            int to_i;
            int to_j;


            if ( last_row_id != -1 && (last_row_id >> 16) == leaf_id )
            {
                assert ( last_leaf_bt != -1 );
                from_i = last_leaf_bt >> 3;
                from_j = (last_leaf_bt & 7) + step;
            }
            else
            {
                from_i = !backward ? 0 : LEAF_DATA_SZ_BT - 1;
                from_j = !backward ? 0 : 8 - 1;
            }

            if ( !backward )
            {
                to_i = LEAF_DATA_SZ_BT;
                to_j = 8;
            }
            else
            {
                to_i = -1;
                to_j = -1;
            }

            for ( i = from_i; i != to_i; i += step )
            {
                if ( leaf->data.bitmap[i] )
                {
                    int64_t new_row_id = row_id | i << 3;
                    for ( j = from_j; j != to_j; j += step )
                    {
                        if ( leaf->data.bitmap[i] & (1 << j) )
                        {
                            iter->current_row_id = (int64_t)( new_row_id | j );
                            return true;
                        }
                    }
                }
                from_j = !backward ? 0 : 8 - 1;
            }

            break;
        }
        case LeafType_ArrayRanges:
        {
            int from_i; // range index
            int from_j;
            int to_i;
            int to_j;
            int len = leaf->data.array_ranges.len;

            if ( last_row_id != -1 && (last_row_id >> 16) == leaf_id )
            {
                assert ( last_leaf_bt != -1 );
                assert ( iter->range_array.current_range_index != -1 );

                from_i = iter->range_array.current_range_index;
                from_j = last_leaf_bt + step;
            }
            else
            {
                from_i = !backward ? 0 : len - 1;
                from_j = !backward ? leaf->data.array_ranges.ranges[from_i].start : leaf->data.array_ranges.ranges[from_i].end;
            }

            if ( !backward )
            {
                to_i = len;
            }
            else
            {
                to_i = -1;
            }

            for ( i = from_i; i != to_i; i += step )
            {
                const struct KRowSetTreeLeafRowRange * range = &leaf->data.array_ranges.ranges[i];
                assert ( range->start <= range->end );
                to_j = (!backward ? range->end : range->start) + step;
                // single loop iteration just to check conditions
                for ( j = from_j; j != to_j; j += step )
                {
                    assert ( j == (j & 0xFFFF) );
                    iter->current_row_id = row_id | j;
                    iter->range_array.current_range_index = i;
                    return true;
                }
                from_j = !backward ? (range + step)->start : (range + step)->end;
            }
            break;
        }
        default:
            assert ( false );
        }

        if ( !backward )
            iter->leaf = KRowSetTreeLeafGetNext ( leaf );
        else
            iter->leaf = KRowSetTreeLeafGetPrev ( leaf );

        if ( iter->leaf == NULL )
        {
            iter->eol = step;
        }
    }

    return false;
}

KDB_EXTERN rc_t CC KRowSetMakeIterator ( const KRowSet * self, KRowSetIterator ** iter )
{
    rc_t rc;
    KRowSetIterator * it;
    uint64_t num_rows;

    if ( self == NULL )
        return RC ( rcDB, rcIterator, rcConstructing, rcSelf, rcNull );

    if ( iter == NULL )
        return RC ( rcDB, rcIterator, rcConstructing, rcParam, rcNull );

    rc = KRowSetGetNumRowIds ( self, &num_rows );
    if ( rc != 0 )
        return rc;

    if ( num_rows == 0 )
        return RC ( rcDB, rcIterator, rcConstructing, rcItem, rcNotFound );

    it = calloc ( 1, sizeof *it );
    if ( it == NULL )
        rc = RC ( rcDB, rcIterator, rcConstructing, rcMemory, rcExhausted );
    else
    {
        KRefcountInit ( &it->refcount, 1, "KRowSetIterator", "new", "" );

        rc = KRowSetAddRef ( self );
        if ( rc == 0 )
        {
            it->rowset = self;
            KRowSetIteratorFirst ( it );
            assert ( KRowSetIteratorIsValid ( it ) );

            *iter = it;
            return 0;
        }

        free ( it );
    }

    return rc;
}

/**
 * Whack
 */
static
rc_t KRowSetIteratorWhack ( KRowSetIterator * iter )
{
    assert ( iter != NULL );

    KRowSetRelease ( iter->rowset );
    iter->rowset = NULL;
    free ( iter );

    return 0;
}

KDB_EXTERN rc_t CC KRowSetIteratorAddRef ( const KRowSetIterator * iter )
{
    if ( iter != NULL ) switch ( KRefcountAdd ( & iter -> refcount, "KRowSetIterator" ) )
    {
        case krefOkay:
            break;
        default:
            return RC ( rcDB, rcRowSet, rcAttaching, rcConstraint, rcViolated );
    }
    return 0;
}
KDB_EXTERN rc_t CC KRowSetIteratorRelease ( const KRowSetIterator * iter )
{
    if ( iter != NULL ) switch ( KRefcountDrop ( & iter -> refcount, "KRowSetIterator" ) )
    {
        case krefOkay:
            break;
        case krefWhack:
            return KRowSetIteratorWhack ( ( KRowSetIterator* ) iter );
        default:
            return RC ( rcDB, rcRowSet, rcReleasing, rcConstraint, rcViolated );
    }
    return 0;
}

KDB_EXTERN rc_t CC KRowSetIteratorFirst ( KRowSetIterator * self )
{
    if ( self == NULL )
        return RC ( rcDB, rcIterator, rcPositioning, rcSelf, rcNull );

    KRowSetIteratorStart ( self );
    KRowSetIteratorNext ( self );

    return 0;
}

KDB_EXTERN rc_t CC KRowSetIteratorLast ( KRowSetIterator * self )
{
    if ( self == NULL )
        return RC ( rcDB, rcIterator, rcPositioning, rcSelf, rcNull );

    KRowSetIteratorEnd ( self );
    KRowSetIteratorPrev ( self );

    return 0;
}

KDB_EXTERN rc_t CC KRowSetIteratorNext ( KRowSetIterator * self )
{
    bool moved;
    if ( self == NULL )
        return RC ( rcDB, rcIterator, rcPositioning, rcSelf, rcNull );

    moved = KRowSetIteratorMove ( self, false );

    if ( !moved )
        return RC ( rcDB, rcIterator, rcPositioning, rcItem, rcDone );

    return 0;
}

KDB_EXTERN rc_t CC KRowSetIteratorPrev ( KRowSetIterator * self )
{
    bool moved;

    if ( self == NULL )
        return RC ( rcDB, rcIterator, rcPositioning, rcSelf, rcNull );

    moved = KRowSetIteratorMove ( self, true );

    if ( !moved )
        return RC ( rcDB, rcIterator, rcPositioning, rcItem, rcDone );

    return 0;
}

KDB_EXTERN bool CC KRowSetIteratorIsValid ( const KRowSetIterator * self )
{
    return self != NULL && !self->eol;
}

KDB_EXTERN rc_t CC KRowSetIteratorRowId ( const KRowSetIterator * self, int64_t * row_id )
{
    if ( self == NULL )
        return RC ( rcDB, rcIterator, rcAccessing, rcSelf, rcNull );

    if ( row_id == NULL )
        return RC ( rcDB, rcIterator, rcAccessing, rcParam, rcNull );

    if ( !KRowSetIteratorIsValid ( self ) )
        return RC ( rcDB, rcIterator, rcAccessing, rcSelf, rcInvalid );

    assert ( self->current_row_id != -1 );

    *row_id = self->current_row_id;
    return 0;
}
