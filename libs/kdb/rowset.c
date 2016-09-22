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

#include <kdb/rowset.h>
#include <kdb/rowset-impl.h>
#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

KITFTOK_DEF ( KRowSet );
KITFTOK_DEF ( KRowSetIterator );

/* MakeRowSet
 *  may add others...
 */
KDB_EXTERN KRowSet * CC KTableMakeRowSet ( struct KTable const * self, ctx_t ctx )
{
    return KTableMakeRowSetSimple ( self, ctx );
}

/* Init
 *  initialize a newly allocated rowset object
 */
KDB_EXTERN void CC KRowSetInit ( KRowSet *self, ctx_t ctx, const KVTable *kvt,
    const char *classname )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcInitializing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "KRowSetInit failed" );
    else if ( kvt == NULL )
        INTERNAL_ERROR ( xcParamNull, "KRowSetInit failed: vt is NULL" );
    else
    {
        TRY ( KRefcountInit_v1 ( & self -> dad, ctx, kvt, classname ) )
        {
            const KRowSet_v1_vt *vt = KVTABLE_CAST ( kvt, ctx, KRowSet );
            if ( vt == NULL )
                INTERNAL_ERROR ( xcInterfaceIncorrect, "vtable does not appear to implement KRowSet" );
            else switch ( vt -> dad . min )
            {
            case 0:
#if _DEBUGGING
                if ( vt -> destroy_data == NULL         ||
                     vt -> add_row_id_range == NULL     ||
                     vt -> get_num_rows == NULL         ||
                     vt -> has_row_id == NULL           ||
                     vt -> get_iterator == NULL )
                {
                    INTERNAL_ERROR ( xcInterfaceInvalid, "null method pointer(s)" );
                    return;
                }
#endif
                break;
            default:
                INTERNAL_ERROR ( xcInterfaceInvalid, "rowset has an invalid version" );
            }

        }
    }
}

/* AddRowId
 *  add a single row to set
 *
 *  "row_id" [ IN ] - row-id to be added
 */
KDB_EXTERN void CC KRowSetAddRowId ( KRowSet * self, ctx_t ctx, int64_t row_id )
{
    KRowSetAddRowIdRange ( self, ctx, row_id, 1 );
}

/* AddRowIdRange
 *  adds row-ids within specified range
 *
 *  "row_id" [ IN ] and "count" [ IN ] - range of row-ids to be added
 */
KDB_EXTERN void CC KRowSetAddRowIdRange ( KRowSet * self, ctx_t ctx, int64_t row_id, uint64_t count )
{
    if ( self == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcInserting );
        INTERNAL_ERROR ( xcSelfNull, "failed to insert rows into rowset" );
    }
    else
    {
        const KRowSet_v1_vt * vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KRowSet );
        if ( vt == NULL )
        {
            FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcInserting );
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KRowSet interface" );
        }
        else
            vt -> add_row_id_range ( self, ctx, row_id, count );
    }
}

/* GetNumRowIds
 *  return the number of elements in set
 */
KDB_EXTERN uint64_t CC KRowSetGetNumRowIds ( const KRowSet * self, ctx_t ctx )
{
    if ( self == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcAccessing );
        INTERNAL_ERROR ( xcSelfNull, "failed to get number of rows in rowset" );
    }
    else
    {
        const KRowSet_v1_vt * vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KRowSet );
        if ( vt == NULL )
        {
            FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcAccessing );
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KRowSet interface" );
        }
        else
            return vt -> get_num_rows ( self, ctx );
    }

    return 0;
}

/* HasRowId
 *  checks if element is present in set
 */
KDB_EXTERN bool CC KRowSetHasRowId ( const KRowSet * self, ctx_t ctx, int64_t row_id )
{
    if ( self == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcAccessing );
        INTERNAL_ERROR ( xcSelfNull, "failed to find row in rowset" );
    }
    else
    {
        const KRowSet_v1_vt * vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KRowSet );
        if ( vt == NULL )
        {
            FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcAccessing );
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KRowSet interface" );
        }
        else
            return vt -> has_row_id ( self, ctx, row_id );
    }

    return false;
}

/* Visit
 *  execute a function on each row-id in set
 */
KDB_EXTERN void CC KRowSetVisit ( const KRowSet * self, ctx_t ctx,
    void ( CC * f ) ( ctx_t ctx, int64_t row_id, void * data ), void * data )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcAccessing );
    KRowSetIterator * it;

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to iterate over rowset" );
    else
    {
        TRY ( it = KRowSetMakeIterator ( self, ctx ) )
        {
            while ( !FAILED() && KRowSetIteratorIsValid ( it ) )
            {
                int64_t row_id;
                ON_FAIL ( row_id = KRowSetIteratorGetRowId ( it, ctx ) )
                    break;

                ON_FAIL ( f ( ctx, row_id, data ) )
                    break;

                KRowSetIteratorNext ( it, ctx );
            }

            KRowSetIteratorRelease ( it, ctx );
        }
    }
}

/* Intersect
 *  performs an intersection between two sets and returns the result
 */
KDB_EXTERN KRowSet * CC KRowSetIntersect ( ctx_t ctx, const KRowSet * a, const KRowSet * b )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcProcessing );
    KRowSet * result;

    if ( a -> table != b -> table )
        USER_ERROR ( xcParamIncorrect, "cannot intersect rowsets from different tables" );
    else
    {
        TRY ( result = KTableMakeRowSet ( a -> table, ctx ) )
        {
            KRowSetIterator * a_it;
            KRowSetIterator * b_it;
            TRY ( a_it = KRowSetMakeIterator ( a, ctx ) )
            {
                TRY ( b_it = KRowSetMakeIterator ( b, ctx ) )
                {
                    while ( !FAILED() && KRowSetIteratorIsValid ( a_it ) && KRowSetIteratorIsValid ( b_it ) )
                    {
                        int64_t a_row_id;
                        int64_t b_row_id;
                        ON_FAIL ( a_row_id = KRowSetIteratorGetRowId ( a_it, ctx ) )
                            break;

                        ON_FAIL ( b_row_id = KRowSetIteratorGetRowId ( b_it, ctx ) )
                            break;

                        if ( a_row_id < b_row_id )
                            KRowSetIteratorNext ( a_it, ctx );
                        else if ( a_row_id > b_row_id )
                            KRowSetIteratorNext ( b_it, ctx );
                        else
                        {
                            TRY ( KRowSetAddRowId ( result, ctx, a_row_id ) )
                            {
                                TRY ( KRowSetIteratorNext ( a_it, ctx ) )
                                {
                                    KRowSetIteratorNext ( b_it, ctx );
                                }
                            }
                        }
                    }
                    KRowSetIteratorRelease ( b_it, ctx );
                }
                KRowSetIteratorRelease ( a_it, ctx );
            }
            if ( !FAILED() )
                return result;

            KRowSetRelease ( result, ctx );
        }
    }

    return NULL;
}

/* Union
 *  performs a union between two sets and returns the result
 */
KDB_EXTERN KRowSet * CC KRowSetUnion ( ctx_t ctx, const KRowSet * a, const KRowSet * b )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcProcessing );
    KRowSet * result;

    if ( a -> table != b -> table )
        USER_ERROR ( xcParamIncorrect, "cannot intersect rowsets from different tables" );
    else
    {
        TRY ( result = KTableMakeRowSet ( a -> table, ctx ) )
        {
            KRowSetIterator * a_it;
            KRowSetIterator * b_it;
            TRY ( a_it = KRowSetMakeIterator ( a, ctx ) )
            {
                TRY ( b_it = KRowSetMakeIterator ( b, ctx ) )
                {
                    while ( !FAILED() && KRowSetIteratorIsValid ( a_it ) && KRowSetIteratorIsValid ( b_it ) )
                    {
                        int64_t a_row_id;
                        int64_t b_row_id;
                        ON_FAIL ( a_row_id = KRowSetIteratorGetRowId ( a_it, ctx ) )
                            break;

                        ON_FAIL ( b_row_id = KRowSetIteratorGetRowId ( b_it, ctx ) )
                            break;

                        if ( a_row_id < b_row_id )
                        {
                            TRY ( KRowSetAddRowId ( result, ctx, a_row_id ) )
                            {
                                KRowSetIteratorNext ( a_it, ctx );
                            }
                        }
                        else if ( a_row_id > b_row_id )
                        {
                            TRY ( KRowSetAddRowId ( result, ctx, b_row_id ) )
                            {
                                KRowSetIteratorNext ( b_it, ctx );
                            }
                        }
                        else
                        {
                            TRY ( KRowSetAddRowId ( result, ctx, a_row_id ) )
                            {
                                TRY ( KRowSetIteratorNext ( a_it, ctx ) )
                                {
                                    KRowSetIteratorNext ( b_it, ctx );
                                }
                            }
                        }
                    }
                    while ( !FAILED() && KRowSetIteratorIsValid ( a_it ) )
                    {
                        int64_t a_row_id;
                        TRY ( a_row_id = KRowSetIteratorGetRowId ( a_it, ctx ) )
                        {
                            TRY ( KRowSetAddRowId ( result, ctx, a_row_id ) )
                            {
                                KRowSetIteratorNext ( a_it, ctx );
                            }
                        }
                    }
                    while ( !FAILED() && KRowSetIteratorIsValid ( b_it ) )
                    {
                        int64_t b_row_id;
                        TRY ( b_row_id = KRowSetIteratorGetRowId ( b_it, ctx ) )
                        {
                            TRY ( KRowSetAddRowId ( result, ctx, b_row_id ) )
                            {
                                KRowSetIteratorNext ( b_it, ctx );
                            }
                        }
                    }
                    KRowSetIteratorRelease ( b_it, ctx );
                }
                KRowSetIteratorRelease ( a_it, ctx );
            }
            if ( !FAILED() )
                return result;

            KRowSetRelease ( result, ctx );
        }
    }

    return NULL;
}

/* MakeIterator
 *  create an iterator on set
 *  initially set to first row-id in set
 */
KDB_EXTERN KRowSetIterator * CC KRowSetMakeIterator ( const KRowSet * self, ctx_t ctx )
{
    if ( self == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcIterator, rcCreating );
        INTERNAL_ERROR ( xcSelfNull, "failed to create rowset iterator" );
    }
    else
    {
        const KRowSet_v1_vt * vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KRowSet );
        if ( vt == NULL )
        {
            FUNC_ENTRY ( ctx, rcDB, rcIterator, rcCreating );
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KRowSet interface" );
        }
        else
            return vt -> get_iterator ( self, ctx );
    }

    return NULL;
}

/* Init
 *  initialize a newly allocated rowset iterator object
 */
KDB_EXTERN void CC KRowSetIteratorInit ( KRowSetIterator *self, ctx_t ctx, const KVTable *kvt,
    const char *classname )
{
    FUNC_ENTRY ( ctx, rcDB, rcIterator, rcInitializing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "KRowSetIteratorInit failed" );
    else if ( kvt == NULL )
        INTERNAL_ERROR ( xcParamNull, "KRowSetIteratorInit failed: vt is NULL" );
    else
    {
        TRY ( KRefcountInit_v1 ( & self -> dad, ctx, kvt, classname ) )
        {
            const KRowSetIterator_v1_vt *vt = KVTABLE_CAST ( kvt, ctx, KRowSetIterator );
            if ( vt == NULL )
                INTERNAL_ERROR ( xcInterfaceIncorrect, "vtable does not appear to implement KRowSetIterator" );
            else switch ( vt -> dad . min )
            {
            case 0:
#if _DEBUGGING
                if ( vt -> next == NULL            ||
                     vt -> is_valid == NULL        ||
                     vt -> get_row_id == NULL )
                {
                    INTERNAL_ERROR ( xcInterfaceNull, "KRowSetIteratorInit failed" );
                    return;
                }
#endif
                break;
            default:
                INTERNAL_ERROR ( xcInterfaceBadVersion, "rowset iterator has an invalid version" );
            }

        }
    }
}

/* Next
 *  advance iterator to next row-id

 *  advance to first row-id on initial invocation
 *  advance to next row-id subsequently
 *  returns rcDone if no more row-ids are available.
 */
KDB_EXTERN bool CC KRowSetIteratorNext ( KRowSetIterator * self, ctx_t ctx )
{

    if ( self == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcIterator, rcPositioning );
        INTERNAL_ERROR ( xcSelfNull, "failed to move rowset iterator" );
    }
    else
    {
        const KRowSetIterator_v1_vt * vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KRowSetIterator );
        if ( vt == NULL )
        {
            FUNC_ENTRY ( ctx, rcDB, rcIterator, rcPositioning );
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KRowSetIterator interface" );
        }
        else
            return vt -> next ( self, ctx );
    }

    return false;
}

/*IsValid
 * check if iterator points to a valid row
 *
 * returns false when iterator points somewhere outside of a row set
 */
KDB_EXTERN bool CC KRowSetIteratorIsValid ( const KRowSetIterator * self )
{
    if ( self != NULL )
    {
        HYBRID_FUNC_ENTRY ( rcDB, rcIterator, rcAccessing );
        const KRowSetIterator_v1_vt * vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KRowSetIterator );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KRowSetIterator interface" );
        else
            return vt -> is_valid ( self );
    }

    return false;
}

/* RowId
 *  report current row id
 */
KDB_EXTERN int64_t CC KRowSetIteratorGetRowId ( const KRowSetIterator * self, ctx_t ctx )
{
    if ( self == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcIterator, rcAccessing );
        INTERNAL_ERROR ( xcSelfNull, "failed to access rowset iterator" );
    }
    else
    {
        const KRowSetIterator_v1_vt * vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KRowSetIterator );
        if ( vt == NULL )
        {
            FUNC_ENTRY ( ctx, rcDB, rcIterator, rcAccessing );
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KRowSetIterator interface" );
        }
        else
            return vt -> get_row_id ( self, ctx );
    }

    return -1;
}
