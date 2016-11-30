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

#define KROWSET_IT KRowSetSimpleIterator

#include <kdb/extern.h>

#include <kdb/rowset.h>
#include <kdb/rowset-impl.h>
#include <kdb/table.h>
#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>
#include <klib/out.h>
#include <klib/sort.h>

#include <string.h>
#include <assert.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

/*--------------------------------------------------------------------------
 * forwards
 */
typedef struct KRowSetSimpleData KRowSetSimpleData;
typedef struct KRowSetSimpleIterator KRowSetSimpleIterator;

static
KRowSetSimpleIterator * CC KRowSetSimpleGetIterator ( const struct KRowSet * self, ctx_t ctx );

/*--------------------------------------------------------------------------
 * KRowSet
 */

struct KRowSetSimpleData
{
    uint64_t size;
    uint64_t num_rows;
    bool sorted;
    int64_t rows_array[1];
};

static
KRowSetSimpleData * KRowSetSimpleAllocateData ( ctx_t ctx, uint64_t size )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcAllocating );
    KRowSetSimpleData * data;

    if ( size <= 0 )
        INTERNAL_ERROR ( xcParamInvalid, "data size should be greater than zero" );
    else
    {
        data = malloc ( sizeof *data + (size - 1) * sizeof(int64_t) );
        if ( data == NULL )
            SYSTEM_ERROR ( xcNoMemory, "out of memory" );
        else
        {
            data -> size = size;
            data -> num_rows = 0;
            data -> sorted = true;
            return data;
        }
    }

    return NULL;
}

static
KRowSetSimpleData * GetRowSetSimpleData ( KRowSet *self, ctx_t ctx )
{
    if ( self == NULL || self -> data == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcAccessing );
        INTERNAL_ERROR ( xcSelfNull, "failed to get rowset data" );
        return NULL;
    }

    return self -> data;
}

static
void CC KRowSetSimpleDestroyData ( void *data, ctx_t ctx )
{
    free ( data );
}

static
bool CC KRowSetSimpleHasRowId ( const KRowSet * self, ctx_t ctx, int64_t row_id )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcSearching );
    const KRowSetSimpleData * data;

    TRY ( data = GetRowSetSimpleData ( (KRowSet *)self, ctx ) )
    {
        /* TBD - only want this algorithm when count > some threshold... */
        if ( data -> sorted )
        {
            uint64_t start, end, idx;

            start = 0;
            end = data -> num_rows;
            while (start < end)
            {
                idx = (start + end) / 2;
                if (row_id < data -> rows_array[idx])
                    end = idx;
                else if (row_id > data -> rows_array[idx])
                    start = idx + 1;
                else
                    return true;
            }
        }
        else
        {
            uint64_t i;
            for ( i = 0; i < data -> num_rows; ++i )
            {
                if ( data -> rows_array[i] == row_id )
                    return true;
            }
        }
    }
    return false;
}

static
void KRowSetSimpleDataSort ( KRowSetSimpleData * data )
{
    if ( data == NULL || data -> sorted || data -> num_rows <= 1 )
        return;

    ksort_int64_t ( data -> rows_array, data -> num_rows );
    data -> sorted = true;
}

static
void AppendRowId ( KRowSet *self, ctx_t ctx, int64_t row_id )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcInserting );
    KRowSetSimpleData * data;
    TRY ( data = GetRowSetSimpleData ( self, ctx ) )
    {
        if ( data -> size - data -> num_rows == 0 )
        {
            KRowSetSimpleData * old_data = data;
            uint64_t new_size;
            if ( data -> size == SIZE_MAX )
            {
                INTERNAL_ERROR ( xcIntegerOutOfBounds, "cannot allocate bigger simple container for row id insertion" );
                return;
            }
            if ( SIZE_MAX - data -> size > data -> size )
                new_size = data -> size * 2;
            else
                new_size = SIZE_MAX;

            TRY ( data = KRowSetSimpleAllocateData ( ctx, new_size ) )
            {
                memmove ( &data -> rows_array, &old_data -> rows_array, old_data -> num_rows * sizeof(int64_t) );
                data -> num_rows = old_data -> num_rows;
                data -> sorted = old_data -> sorted;
                self -> data = data;
                ON_FAIL ( KRowSetSimpleDestroyData ( old_data, ctx ) )
                    return;
            }
            else
                return;
        }
        if ( data -> sorted )
            data -> sorted = data -> num_rows == 0 || data -> rows_array[ data -> num_rows - 1 ] <= row_id;
        data -> rows_array[ data -> num_rows++ ] = row_id;
    }
}

static
void CC KRowSetSimpleAddRowIdRange ( KRowSet *self, ctx_t ctx, int64_t start_row_id, uint64_t count )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcInserting );
    uint64_t i;

    if ( (uint64_t)INT64_MAX - start_row_id < count )
    {
        USER_ERROR ( xcIntegerOutOfBounds, "bad row id range" );
        return;
    }


    for ( i = 0; i < count; ++i )
    {
        int64_t row_id = i + start_row_id;
        bool row_exists;
        TRY ( row_exists = KRowSetSimpleHasRowId ( self, ctx, row_id ) )
        {
            if ( row_exists )
                continue;

            ON_FAIL ( AppendRowId ( self, ctx, row_id ) )
                break;
        }
    }
}

static
uint64_t CC KRowSetSimpleGetNumRowIds ( const KRowSet * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcAccessing );
    const KRowSetSimpleData * data;
    TRY ( data = GetRowSetSimpleData ( (KRowSet *) self, ctx ) )
    {
        return data -> num_rows;
    }

    return 0;
}

static
void CC KRowSetSimpleDestroy ( KRefcount_v1 *self_refcount, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcDestroying );

    KRowSet * self = (KRowSet *)self_refcount;

    const KRowSet_v1_vt * vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KRowSet );
    if ( vt == NULL )
        INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KRowSet interface" );
    else
    {
        KTableRelease ( self -> table );
        self -> table = NULL;

        assert ( vt != NULL );
        assert ( vt -> destroy_data != NULL );

        ( * vt -> destroy_data ) ( self -> data, ctx );
        self -> data = NULL;
    }
}

static KRefcount_v1_vt vtKRowSetSimpleRefCount =
{
    KVTABLE_INITIALIZER ( KRowSetSimpleRefCount, KRefcount_v1, 0, NULL ),
    KRowSetSimpleDestroy,
    NULL
};

static KRowSet_v1_vt vtKRowSetSimple =
{
    KVTABLE_INITIALIZER ( KRowSetSimple, KRowSet, 0, &vtKRowSetSimpleRefCount.dad ),

    KRowSetSimpleDestroyData,
    KRowSetSimpleAddRowIdRange,
    KRowSetSimpleGetNumRowIds,
    KRowSetSimpleHasRowId,
    KRowSetSimpleGetIterator
};

KDB_EXTERN KRowSet * CC KTableMakeRowSetSimple ( struct KTable const * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcConstructing );
    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to construct rowset: table is NULL" );
    else
    {
        KRowSet *r;

        r = calloc ( 1, sizeof *r );
        if ( r == NULL )
            SYSTEM_ERROR ( xcNoMemory, "out of memory" );
        else
        {
            TRY ( KRowSetInit ( r, ctx, &vtKRowSetSimple.dad, "KRowSetSimple" ) )
            {
                TRY ( r -> data = KRowSetSimpleAllocateData ( ctx, 16 ) )
                {
                    rc_t rc;
                    r -> table = self;
                    rc = KTableAddRef ( r -> table );
                    if ( rc != 0 )
                        INTERNAL_ERROR ( xcUnexpected, "unknown result from KTableAddRef: rc = %R", rc );
                    else
                        return r;
                }
            }
            free ( r );
        }
    }

    return NULL;
}

/*--------------------------------------------------------------------------
 * KRowSetSimpleIterator
 */

struct KRowSetSimpleIterator
{
    KRowSetIterator dad;
    const KRowSet * rowset;
    const KRowSetSimpleData * rowset_data;
    uint64_t array_index;
};

static
void CC KRowSetSimpleIteratorDestroy ( KRefcount_v1 *self_refcount, ctx_t ctx )
{
    KRowSetSimpleIterator * self = (KRowSetSimpleIterator *)self_refcount;
    if ( self == NULL || self -> rowset == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcIterator, rcDestroying );
        INTERNAL_ERROR ( xcSelfNull, "failed to destroy rowset iterator" );
    }
    else
    {
        KRowSetRelease ( self -> rowset, ctx );
    }
}

static
bool CC KRowSetSimpleIteratorNext ( struct KRowSetSimpleIterator * self, ctx_t ctx )
{
    if ( self == NULL || self -> rowset_data == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcIterator, rcPositioning );
        INTERNAL_ERROR ( xcSelfNull, "failed to move rowset iterator" );
    }
    else if ( self -> array_index == self -> rowset_data -> num_rows )
    {
        FUNC_ENTRY ( ctx, rcDB, rcIterator, rcPositioning );
        INTERNAL_ERROR ( xcIteratorExhausted, "failed to move rowset iterator - no more elements" );
    }
    else
        return ++self -> array_index < self -> rowset_data -> num_rows;

    return false;
}

static
bool CC KRowSetSimpleIteratorIsValid ( const struct KRowSetSimpleIterator * self )
{
    if (self == NULL || self -> rowset_data == NULL || self -> rowset == NULL || self -> rowset_data != self -> rowset -> data )
        return false;

    return self -> array_index < self -> rowset_data -> num_rows;
}

static
int64_t CC KRowSetSimpleIteratorGetRowId ( const struct KRowSetSimpleIterator * self, ctx_t ctx )
{
    if ( !KRowSetSimpleIteratorIsValid ( self ) )
    {
        FUNC_ENTRY ( ctx, rcDB, rcIterator, rcAccessing );
        if (self == NULL || self -> rowset_data == NULL || self -> rowset == NULL )
            INTERNAL_ERROR ( self == NULL ? xcSelfNull : xcSelfInvalid, "cannot get row id from iterator" );
        else if ( self -> rowset_data != self -> rowset -> data )
            USER_ERROR ( xcSelfInvalid, "usage of iterator after modifying rowset" );
        else
            USER_ERROR ( xcSelfInvalid, "usage of iterator beyond rowset range" );
        return -1;
    }
    return self -> rowset_data -> rows_array[self -> array_index];
}

static KRefcount_v1_vt vtKRowSetIteratorSimpleRefCount =
{
    KVTABLE_INITIALIZER ( KRowSetIteratorSimpleRefCount, KRefcount_v1, 0, NULL ),
    KRowSetSimpleIteratorDestroy,
    NULL
};

static KRowSetIterator_v1_vt vtKRowSetIteratorSimple =
{
    KVTABLE_INITIALIZER ( KRowSetIteratorSimple, KRowSetIterator, 0, &vtKRowSetIteratorSimpleRefCount.dad ),

    KRowSetSimpleIteratorNext,
    KRowSetSimpleIteratorIsValid,
    KRowSetSimpleIteratorGetRowId
};

static
KRowSetSimpleIterator * CC KRowSetSimpleGetIterator ( const struct KRowSet * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcDB, rcIterator, rcConstructing );

    if ( self == NULL || self -> data == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to get rowset data" );
    else
    {
        KRowSetSimpleIterator *r;
        r = calloc ( 1, sizeof *r );
        if ( r == NULL )
            SYSTEM_ERROR ( xcNoMemory, "out of memory" );
        else
        {
            TRY ( KRowSetIteratorInit ( &r -> dad, ctx, &vtKRowSetIteratorSimple.dad, "KRowSetIteratorSimple" ) )
            {
                KRowSetSimpleDataSort ( self -> data ); /* make sure rowset is sorted */
                r -> rowset_data = self -> data;
                r -> array_index = 0;
                TRY ( r -> rowset = KRowSetDuplicate ( self, ctx, 0 ) )
                {
                    return r;
                }
            }
            free ( r );
        }
    }

    return NULL;
}

