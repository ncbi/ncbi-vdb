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
#include <kdb/rowset-impl.h>
#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

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
KDB_EXTERN void CC KRowSetInit ( KRowSet *self, ctx_t ctx, const KRowSet_vt *vt,
    const char *classname, const char *name )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcInitializing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "KRowSetInit failed" );
    else if ( vt == NULL )
        INTERNAL_ERROR ( xcParamNull, "KRowSetInit failed: vt is NULL" );
    else
    {
        switch ( vt -> maj )
        {
        case 0:
            INTERNAL_ERROR ( xcInterfaceInvalid, "KRowSetInit failed" );
            return;
        case 1:
            switch ( vt -> min )
            {
            case 0:
#if _DEBUGGING
                if ( vt -> destroy_data == NULL         ||
                     vt -> add_row_id_range == NULL )
                {
                    INTERNAL_ERROR ( xcInterfaceNull, "KRowSetInit failed" );
                    return;
                }
#endif
                break;
            default:
                INTERNAL_ERROR ( xcInterfaceBadVersion, "KRowSetInit failed" );
                return;
            }
            break;

        default:
            INTERNAL_ERROR ( xcInterfaceBadVersion, "KRowSetInit failed" );
            return;
        }

        self -> vt = vt;
        KRefcountInit ( & self -> refcount, 1, classname, "init", name );
    }
}

static
void CC KRowSetDestroy ( KRowSet *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcDestroying );

    assert ( self -> vt != NULL );
    assert ( self -> vt -> destroy_data != NULL );

    ( * self -> vt -> destroy_data ) ( self -> data, ctx );

    free ( self );
}

/* AddRef
 * Release
 *  ignores NULL references
 */
KDB_EXTERN void CC KRowSetAddRef ( const KRowSet *self, ctx_t ctx )
{
    if ( self != NULL ) switch ( KRefcountAdd ( & self -> refcount, "KRowSet" ) )
    {
        case krefOkay:
            break;
        default:
        {
            FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcAttaching );
            INTERNAL_ERROR ( xcRefcountViolated, "error while incrementing refcount" );
        }
    }
}

KDB_EXTERN void CC KRowSetRelease ( const KRowSet *self, ctx_t ctx )
{
    if ( self != NULL ) switch ( KRefcountDrop ( & self -> refcount, "KRowSet" ) )
    {
        case krefOkay:
            break;
        case krefWhack:
            KRowSetDestroy ( ( KRowSet* ) self, ctx );
            break;
        default:
        {
            FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcReleasing );
            INTERNAL_ERROR ( xcRefcountViolated, "error while decrementing refcount" );
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
    if ( self == NULL || self -> vt == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcInserting );
        INTERNAL_ERROR ( xcSelfNull, "failed to insert rows into rowset" );
    }
    else
        self -> vt ->add_row_id_range ( self, ctx, row_id, count );
}

/* Visit
 *  execute a function on each row-id in set
 */
KDB_EXTERN void CC KRowSetVisit ( const KRowSet * self, ctx_t ctx, bool reverse,
    void ( CC * f ) ( int64_t row_id, void * data ), void * data )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcAccessing );
    KRowSetIterator * it;

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to iterate over rowset" );
    else if ( reverse )
        INTERNAL_ERROR ( xcFunctionUnsupported, "failed to iterate over rowset in reverse order" );
    else
    {
        TRY ( it = KRowSetMakeIterator ( self, ctx ) )
        {
            while ( !FAILED() && KRowSetIteratorIsValid ( it ) )
            {
                int64_t row_id;
                ON_FAIL ( row_id = KRowSetIteratorGetRowId ( it, ctx ) )
                    break;

                f ( row_id, data );

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

    TRY ( result = KTableMakeRowSet ( NULL, ctx ) )
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

    return NULL;
}

/* Union
 *  performs a union between two sets and returns the result
 */
KDB_EXTERN KRowSet * CC KRowSetUnion ( ctx_t ctx, const KRowSet * a, const KRowSet * b )
{
    FUNC_ENTRY ( ctx, rcDB, rcRowSet, rcProcessing );
    KRowSet * result;

    TRY ( result = KTableMakeRowSet ( NULL, ctx ) )
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

    return NULL;
}

/* MakeIterator
 *  create an iterator on set
 *  initially set to first row-id in set
 */
KDB_EXTERN KRowSetIterator * CC KRowSetMakeIterator ( const KRowSet * self, ctx_t ctx )
{
    if ( self == NULL || self -> vt == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcIterator, rcCreating );
        INTERNAL_ERROR ( xcSelfNull, "failed to create rowset iterator" );
    }
    else
        return self -> vt -> get_iterator ( self, ctx );

    return NULL;
}

/* Init
 *  initialize a newly allocated rowset iterator object
 */
KDB_EXTERN void CC KRowSetIteratorInit ( KRowSetIterator *self, ctx_t ctx, const KRowSetIterator_vt *vt,
    const char *classname, const char *name )
{
    FUNC_ENTRY ( ctx, rcDB, rcIterator, rcInitializing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "KRowSetIteratorInit failed" );
    else if ( vt == NULL )
        INTERNAL_ERROR ( xcParamNull, "KRowSetIteratorInit failed: vt is NULL" );
    else
    {
        switch ( vt -> maj )
        {
        case 0:
            INTERNAL_ERROR ( xcInterfaceInvalid, "KRowSetIteratorInit failed" );
            return;
        case 1:
            switch ( vt -> min )
            {
            case 0:
#if _DEBUGGING
                if ( vt -> destroy == NULL         ||
                     vt -> next == NULL            ||
                     vt -> is_valid == NULL        ||
                     vt -> get_row_id == NULL )
                {
                    INTERNAL_ERROR ( xcInterfaceNull, "KRowSetIteratorInit failed" );
                    return;
                }
#endif
                break;
            default:
                INTERNAL_ERROR ( xcInterfaceBadVersion, "KRowSetIteratorInit failed" );
                return;
            }
            break;

        default:
            INTERNAL_ERROR ( xcInterfaceBadVersion, "KRowSetIteratorInit failed" );
            return;
        }

        self -> vt = vt;
        KRefcountInit ( & self -> refcount, 1, classname, "init", name );
    }
}

static
void CC KRowSetIteratorDestroy ( KRowSetIterator *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcDB, rcIterator, rcDestroying );

    assert ( self -> vt != NULL );
    assert ( self -> vt -> destroy != NULL );

    ( * self -> vt -> destroy ) ( self, ctx );

    free ( self );
}

/* AddRef
 * Release
 *  ignores NULL references
 */
KDB_EXTERN void CC KRowSetIteratorAddRef ( const KRowSetIterator *self, ctx_t ctx )
{
    if ( self != NULL ) switch ( KRefcountAdd ( & self -> refcount, "KRowSetIterator" ) )
    {
        case krefOkay:
            break;
        default:
        {
            FUNC_ENTRY ( ctx, rcDB, rcIterator, rcAttaching );
            INTERNAL_ERROR ( xcRefcountViolated, "error while incrementing refcount" );
        }
    }
}

KDB_EXTERN void CC KRowSetIteratorRelease ( const KRowSetIterator *self, ctx_t ctx )
{
    if ( self != NULL ) switch ( KRefcountDrop ( & self -> refcount, "KRowSetIterator" ) )
    {
        case krefOkay:
            break;
        case krefWhack:
            KRowSetIteratorDestroy ( ( KRowSetIterator* ) self, ctx );
            break;
        default:
        {
            FUNC_ENTRY ( ctx, rcDB, rcIterator, rcReleasing );
            INTERNAL_ERROR ( xcRefcountViolated, "error while decrementing refcount" );
        }
    }
}

/* Next
 *  advance iterator to next row-id

 *  advance to first row-id on initial invocation
 *  advance to next row-id subsequently
 *  returns rcDone if no more row-ids are available.
 */
KDB_EXTERN void CC KRowSetIteratorNext ( KRowSetIterator * self, ctx_t ctx )
{
    if ( self == NULL || self -> vt == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcIterator, rcPositioning );
        INTERNAL_ERROR ( xcSelfNull, "failed to move rowset iterator" );
    }
    else
        self -> vt -> next ( self, ctx );
}

/*IsValid
 * check if iterator points to a valid row
 *
 * returns false when iterator points somewhere outside of a row set
 */
KDB_EXTERN bool CC KRowSetIteratorIsValid ( const KRowSetIterator * self )
{
    if ( self == NULL || self -> vt == NULL )
    {
        return false;
    }
    else
        return self -> vt -> is_valid ( self );
}

/* RowId
 *  report current row id
 */
KDB_EXTERN int64_t CC KRowSetIteratorGetRowId ( const KRowSetIterator * self, ctx_t ctx )
{
    if ( self == NULL || self -> vt == NULL )
    {
        FUNC_ENTRY ( ctx, rcDB, rcIterator, rcAccessing );
        INTERNAL_ERROR ( xcSelfNull, "failed to access rowset iterator" );
    }
    else
        return self -> vt -> get_row_id ( self, ctx );

    return -1;
}
