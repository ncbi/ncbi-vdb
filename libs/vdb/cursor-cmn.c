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

#include "cursor-struct.h"
#include "table-priv.h"
#include "column-priv.h"
#include "schema-parse.h"
#include "phys-priv.h"
#include "prod-priv.h"

#include <vdb/cursor.h>
#include <klib/rc.h>
#include <stdbool.h>

/******************* dispatch functions ***********************/
#define DISPATCH_VALUE(call, errVal)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return errVal;

#define DISPATCH(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return RC ( rcVDB, rcCursor, rcAccessing, rcSelf, rcNull );

LIB_EXPORT rc_t CC VCursorWhack ( const VCURSOR_IMPL *self )
{
    DISPATCH ( whack ( self ) );
}
LIB_EXPORT rc_t CC VCursorVAddColumn ( const VCursor *self, uint32_t *idx, const char *name, va_list args )
{
    DISPATCH ( vAddColumn ( self, idx, name, args ) );
}
LIB_EXPORT rc_t CC VCursorVGetColumnIdx ( const VCursor *self, uint32_t *idx, const char *name, va_list args )
{
    DISPATCH ( vGetColumnIdx ( self, idx, name, args ) );
}
LIB_EXPORT rc_t CC VCursorOpen ( const VCursor *self )
{
    DISPATCH ( open ( self ) );
}
LIB_EXPORT rc_t CC VCursorSetRowId ( const VCursor *self, int64_t row_id )
{
    DISPATCH ( setRowId ( self, row_id ) );
}
LIB_EXPORT rc_t CC VCursorOpenRow ( const VCursor *self )
{
    DISPATCH ( openRow ( self ) );
}
LIB_EXPORT rc_t CC VCursorWrite ( VCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count )
{
    DISPATCH ( write ( self, col_idx, elem_bits, buffer, boff, count ) );
}
LIB_EXPORT rc_t CC VCursorCommitRow ( VCursor *self )
{
    DISPATCH ( commitRow ( self ) );
}
LIB_EXPORT rc_t CC VCursorCloseRow ( const VCursor *self )
{
    DISPATCH ( closeRow ( self ) );
}
LIB_EXPORT rc_t CC VCursorRepeatRow ( VCursor *self, uint64_t count )
{
    DISPATCH ( repeatRow ( self, count ) );
}
LIB_EXPORT rc_t CC VCursorFlushPage ( VCursor *self )
{
    DISPATCH ( flushPage ( self ) );
}
LIB_EXPORT rc_t CC VCursorGetBlob ( const VCursor *self, const VBlob **blob, uint32_t col_idx )
{
    DISPATCH ( getBlob ( self, blob, col_idx ) );
}
LIB_EXPORT rc_t CC VCursorGetBlobDirect ( const VCursor *self,
    const VBlob **blob, int64_t row_id, uint32_t col_idx )
{
    DISPATCH ( getBlobDirect ( self, blob, row_id, col_idx ) );
}
LIB_EXPORT rc_t CC VCursorRead ( const VCursor *self, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len )
{
    DISPATCH ( read ( self, col_idx, elem_bits, buffer, blen, row_len ) );
}
LIB_EXPORT rc_t CC VCursorReadDirect ( const VCursor *self, int64_t row_id, uint32_t col_idx,
    uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len )
{
    DISPATCH ( readDirect ( self, row_id, col_idx, elem_bits, buffer, blen, row_len ) );
}
LIB_EXPORT rc_t CC VCursorReadBits ( const VCursor *self, uint32_t col_idx,
    uint32_t elem_bits, uint32_t start, void *buffer, uint32_t off,
    uint32_t blen, uint32_t *num_read, uint32_t *remaining )
{
    DISPATCH ( readBits ( self, col_idx, elem_bits, start, buffer, off, blen, num_read, remaining ) );
}
LIB_EXPORT rc_t CC VCursorReadBitsDirect ( const VCursor *self, int64_t row_id, uint32_t col_idx,
    uint32_t elem_bits, uint32_t start, void *buffer, uint32_t off,
    uint32_t blen, uint32_t *num_read, uint32_t *remaining )
{
    DISPATCH ( readBitsDirect ( self, row_id, col_idx, elem_bits, start, buffer, off, blen, num_read, remaining ) );
}
LIB_EXPORT rc_t CC VCursorCellData ( const VCursor *self, uint32_t col_idx,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len )
{
    DISPATCH ( cellData ( self, col_idx, elem_bits, base, boff, row_len ) );
}
LIB_EXPORT rc_t CC VCursorCellDataDirect ( const VCursor *self, int64_t row_id, uint32_t col_idx,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len )
{
    DISPATCH ( cellDataDirect ( self, row_id, col_idx, elem_bits, base, boff, row_len ) );
}
LIB_EXPORT rc_t CC VCursorDataPrefetch ( const VCursor *self, const int64_t *row_ids, uint32_t col_idx,uint32_t num_rows, int64_t min_valid_row_id, int64_t max_valid_row_id, bool continue_on_error )
{
    DISPATCH ( dataPrefetch ( self, row_ids, col_idx, num_rows, min_valid_row_id, max_valid_row_id, continue_on_error ) );
}
LIB_EXPORT rc_t CC VCursorDefault ( VCursor *self, uint32_t col_idx,
    bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len )
{
    DISPATCH ( _default ( self, col_idx, elem_bits, buffer, boff, row_len ) );
}
LIB_EXPORT rc_t CC VCursorCommit ( VCursor *self )
{
    DISPATCH ( commit ( self ) );
}
LIB_EXPORT rc_t CC VCursorOpenParentRead ( const VCursor *self, const VTable **tbl )
{
    DISPATCH ( openParentRead ( self, tbl ) );
}
LIB_EXPORT rc_t CC VCursorOpenParentUpdate ( VCursor *self, VTable **tbl )
{
    DISPATCH ( openParentUpdate ( self, tbl ) );
}

/* private API dispatch */
rc_t VCursorMakeColumn ( struct VCursor *self, struct VColumn **col, struct SColumn const *scol, Vector *cx_bind )
{
    DISPATCH ( makeColumn ( self, col, scol, cx_bind ) );
}
const VTable * VCursorGetTable ( const struct VCursor * self )
{
    DISPATCH_VALUE ( getTable ( self ), NULL );
}
bool VCursorIsReadOnly ( const struct VCursor * self )
{
    DISPATCH_VALUE ( isReadOnly ( self ), true );
}
LIB_EXPORT struct VSchema const * VCursorGetSchema(struct VCursor const *self)
{
    DISPATCH_VALUE ( getSchema ( self ), NULL );
}
VBlobMRUCache * VCursorGetBlobMruCache ( struct VCursor * self )
{
    DISPATCH_VALUE ( getBlobMruCache ( self ), NULL );
}
uint32_t VCursorIncrementPhysicalProductionCount ( struct VCursor * self )
{
    DISPATCH_VALUE ( incrementPhysicalProductionCount ( self ), 0 );
}
const struct KSymbol * VCursorFindOverride ( const VCursor *self, const struct VCtxId *cid )
{
    DISPATCH_VALUE ( findOverride ( self, cid ), NULL );
}
LIB_EXPORT rc_t CC VCursorPermitPostOpenAdd(struct VCursor const *self)
{
    DISPATCH ( permitPostOpenAdd ( self ) );
}
LIB_EXPORT rc_t CC VCursorSuspendTriggers(struct VCursor const *self)
{
    DISPATCH ( suspendTriggers ( self ) );
}
LIB_EXPORT rc_t CC VCursorLinkedCursorGet(const struct VCursor * self, const char * tbl, struct VCursor const ** curs)
{
    DISPATCH ( linkedCursorGet ( self, tbl, curs ) );
}
LIB_EXPORT rc_t CC VCursorLinkedCursorSet(const struct VCursor * self, const char * tbl, struct VCursor const * curs)
{
    DISPATCH ( linkedCursorSet ( self, tbl, curs ) );
}
LIB_EXPORT uint64_t CC VCursorSetCacheCapacity(struct VCursor * self, uint64_t capacity)
{
    DISPATCH_VALUE ( setCacheCapacity ( self, capacity ), 0 );
}
LIB_EXPORT uint64_t CC VCursorGetCacheCapacity(const struct VCursor * self)
{
    DISPATCH_VALUE ( getCacheCapacity ( self ), 0 );
}
rc_t VCursorLaunchPagemapThread ( struct VCursor *self )
{
    DISPATCH ( launchPagemapThread ( self ) );
}
const PageMapProcessRequest* VCursorPageMapProcessRequest ( const struct VCursor *self )
{
    DISPATCH_VALUE ( pageMapProcessRequest ( self ), NULL );
}
bool VCursorCacheActive ( const struct VCursor * self, int64_t * cache_empty_end )
{
    DISPATCH_VALUE ( cacheActive ( self, cache_empty_end ), false );
}
rc_t VCursorInstallTrigger ( struct VCursor * self, struct VProduction * prod )
{
    DISPATCH ( installTrigger ( self, prod ) );
}

/*--------------------------------------------------------------------------
 * VCursorCache
 */

/* Whack
 */
void
VCursorCacheWhack ( VCursorCache *self,
    void ( CC * whack ) ( void *item, void *data ), void *data )
{
    uint32_t i = VectorStart ( & self -> cache );
    uint32_t end = VectorLength ( & self -> cache );
    for ( end += i; i < end; ++ i )
    {
        Vector *ctx;
        rc_t rc = VectorSwap ( & self -> cache, i, NULL, ( void** ) & ctx );
        if ( rc == 0 && ctx != NULL )
        {
            VectorWhack ( ctx, whack, data );
            free ( ctx );
        }
    }

    VectorWhack ( & self -> cache, NULL, NULL );
}


/* Get
 *  retrieve object by cid
 */
void *
VCursorCacheGet ( const VCursorCache *self, const VCtxId *cid )
{
    const Vector *ctx = ( const void* ) VectorGet ( & self -> cache, cid -> ctx );
    return VectorGet ( ctx, cid -> id );
}

/* Set
 *  set object by cid
 */
rc_t
VCursorCacheSet ( VCursorCache *self, const VCtxId *cid, const void *item )
{
    Vector *ctx = VectorGet ( & self -> cache, cid -> ctx );
    if ( ctx == NULL )
    {
        rc_t rc;

        ctx = malloc ( sizeof * ctx );
        if ( ctx == NULL )
            return RC ( rcVDB, rcVector, rcConstructing, rcMemory, rcExhausted );

        rc = VectorSet ( & self -> cache, cid -> ctx, ctx );
        if ( rc != 0 )
        {
            free ( ctx );
            return rc;
        }

        VectorInit ( ctx, 0, 16 );
    }

    return VectorSet ( ctx, cid -> id, item );
}

/* Swap
 *  swap object by cid
 */
rc_t VCursorCacheSwap ( VCursorCache *self, const VCtxId *cid, const void *item, void **prior )
{
    Vector *ctx = VectorGet ( & self -> cache, cid -> ctx );
    if ( ctx == NULL )
    {
        * prior = NULL;
        return VCursorCacheSet ( self, cid, item );
    }
    return VectorSwap ( ctx, cid -> id, item, prior );
}

/* implementations common to all descendants of VCursor */

static void CC VCursorVColumnWhack_checked( void *item, void *data )
{
    if ( item != NULL )
        VColumnWhack( item, data );
}

rc_t VCursorWhackInt ( const VCursor * p_self )
{
    VCursor * self = ( VCursor * ) p_self;
    KRefcountWhack ( & self -> refcount, "VCursor" );

    if ( self -> user_whack != NULL )
    {
        ( * self -> user_whack ) ( self -> user );
    }
    VCursorCacheWhack ( & self -> col, NULL, NULL );
    VCursorCacheWhack ( & self -> phys, VPhysicalWhack, NULL );
    VCursorCacheWhack ( & self -> prod, NULL, NULL );
    VectorWhack ( & self -> owned, VProductionWhack, NULL );
    VectorWhack ( & self -> row, VCursorVColumnWhack_checked, NULL );

    free ( self );

    return 0;
}

LIB_EXPORT
rc_t CC
VCursorAddRef ( const VCursor * p_self )
{
    if ( p_self != NULL )
    {
        switch ( KRefcountAdd ( & p_self -> refcount, "VCursor" ) )
        {
        case krefLimit:
            return RC ( rcVDB, rcCursor, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT
rc_t CC
VCursorRelease ( const VCursor * p_self )
{
    if ( p_self != NULL )
    {
        switch ( KRefcountDrop ( & p_self -> refcount, "VCursor" ) )
        {
        case krefWhack:
            return VCursorWhack ( p_self );
        case krefNegative:
            return RC ( rcVDB, rcCursor, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* RowId
 *  return id from row cursor within open blob
 *
 *  "id" [ OUT ] - current row id
 */

LIB_EXPORT
rc_t CC
VCursorRowId ( const VCursor * p_self, int64_t * p_id )
{
    if ( p_self == NULL )
    {
        return RC ( rcVDB, rcCursor, rcAccessing, rcSelf, rcNull );
    }
    if ( p_id == NULL )
    {
        return RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    * p_id = p_self -> row_id;
    return 0;
}

/* AddColumn
 *  add a column to an unopened cursor
 *
 *  "idx" [ OUT ] - return parameter for column index
 *
 *  "name" [ IN ] - NUL terminated column name spec.
 *  to identify a column by name, provide the column name
 *  by itself. if there are multiple types available under
 *  that name, the default type for that column will be
 *  selected. to select a specific type, the name may
 *  be cast to that type using a cast expression, e.g.
 *    "( type ) name"
 *  the special name "*" may be added to a read cursor.
 */
LIB_EXPORT
rc_t CC VCursorAddColumn ( const VCursor * p_self, uint32_t * p_idx, const char * p_name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, p_name );
    rc = VCursorVAddColumn ( p_self, p_idx, p_name, args );
    va_end ( args );

    return rc;
}

rc_t
VCursorGetColidx ( const VCursor *          p_self,
                   const SColumn *          p_scol,
                   const SNameOverload *    p_name,
                   uint32_t                 p_type,
                   uint32_t *               p_idx )
{
    rc_t rc;

    if ( p_scol == NULL || p_type != eColumn )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcNotFound );
    }
    else
    {
        /* if the column-spec gave us the exact column, return it */
        VColumn *col = VCursorCacheGet ( & p_self -> col, & p_scol -> cid );
        if ( col != NULL )
        {
            * p_idx = col -> ord;
            return 0;
        }

        /* prepare for failure */
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcNotFound );

        /* there should be a name overload object */
        if ( p_name != NULL )
        {
            uint32_t count, i = VectorStart ( & p_name -> items );
            uint32_t end = VectorLength ( & p_name -> items );
            for ( end += i, count = 0; i < end; ++ i )
            {
                p_scol = ( const void* ) VectorGet ( & p_name -> items, i );
                if ( p_scol != NULL )
                {
                    col = VCursorCacheGet ( & p_self -> col, & p_scol -> cid );
                    if ( col != NULL )
                    {
                        * p_idx = col -> ord;
                        ++ count;
                    }
                }
            }

            if ( count == 1 )
            {
                return 0;
            }
            if ( count != 0 )
            {
                return RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcAmbiguous );
            }
        }
    }

    return rc;
}

LIB_EXPORT
rc_t CC
VCursorGetColumnIdx ( const VCursor *self, uint32_t *idx, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VCursorVGetColumnIdx ( self, idx, name, args );
    va_end ( args );

    return rc;
}


/* Datatype
 *  returns typedecl and/or typedef for column data
 *
 *  "idx" [ IN ] - column index
 *
 *  "type" [ OUT, NULL OKAY ] - returns the column type declaration
 *
 *  "def" [ OUT, NULL OKAY ] - returns the definition of the type
 *  returned in "type_decl"
 *
 * NB - one of "type" and "def" must be non-NULL
 */
LIB_EXPORT
rc_t CC
VCursorDatatype ( const VCursor *  p_self,
                  uint32_t              p_idx,
                  struct VTypedecl *    p_type,
                  struct VTypedesc *    p_desc )
{
    rc_t rc;

    if ( p_type == NULL && p_desc == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        const VColumn * vcol = ( const VColumn* ) VectorGet ( & p_self -> row, p_idx );
        if ( vcol != NULL )
        {
            return VColumnDatatype ( vcol, p_type, p_desc );
        }
        else
        {
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcNotFound );
            if ( p_type != NULL )
            {
                memset ( p_type, 0, sizeof * p_type );
            }
            if ( p_desc != NULL )
            {
                memset ( p_desc, 0, sizeof * p_desc );
            }
        }
    }

    return rc;
}

/* IdRange
 *  returns id range for column
 *
 *  "idx" [ IN ] - column index
 *
 *  "id" [ IN ] - page containing this row id is target
 *
 *  "first" [ OUT, NULL OKAY ] and "last" [ OUT, NULL OKAY ] -
 *  id range is returned in these output parameters, where
 *  at least ONE must be NOT-NULL
 */
typedef struct VCursorIdRangeData VCursorIdRangeData;
struct VCursorIdRangeData
{
    int64_t first, last;
    rc_t rc;
};

static
bool CC
column_id_range ( void *item, void *data )
{
    if ( ( size_t ) item > 8 )
    {
        int64_t first, last;
        VCursorIdRangeData *pb = data;

        rc_t rc = VColumnIdRange ( ( const void* ) item, & first, & last );

        if ( GetRCState ( rc ) == rcEmpty )
            return false;

        if ( ( pb -> rc = rc ) != 0 )
            return true;

        if ( first < pb -> first )
            pb -> first = first;
        if ( last > pb -> last )
            pb -> last = last;
    }

    return false;
}

rc_t CC
VCursorIdRange ( const VCursor * p_self,
                 uint32_t        p_idx,
                 int64_t *       p_first,
                 uint64_t *      p_count )
{
    rc_t rc;
    VCursor * self = ( VCursor * ) p_self;

    if ( p_first == NULL && p_count == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        int64_t dummy;
        uint64_t dummy_count;

        if ( p_first == NULL )
        {
            p_first = & dummy;
        }
        else if ( p_count == NULL )
        {
            p_count = & dummy_count;
        }
        if ( self -> state < vcReady )
        {
            if ( self -> state == vcFailed )
            {
                rc = RC ( rcVDB, rcCursor, rcAccessing, rcCursor, rcInvalid );
            }
            else
            {
                rc = RC ( rcVDB, rcCursor, rcAccessing, rcCursor, rcNotOpen );
            }
        }
        else if ( p_idx == 0 )
        {
            VCursorIdRangeData pb;

            pb . first = INT64_MAX;
            pb . last = INT64_MIN;
            pb . rc = SILENT_RC ( rcVDB, rcCursor, rcAccessing, rcRange, rcEmpty );

            if ( ! VectorDoUntil ( & self -> row, false, column_id_range, & pb ) )
            {
                * p_first = pb . first;
                * p_count = pb . last >= pb . first ? pb . last + 1 - pb . first : 0;
                return pb . rc;
            }

            rc = pb . rc;
        }
        else
        {
            const VColumn *vcol = ( const VColumn* ) VectorGet ( & self -> row, p_idx );
            if ( vcol == NULL )
            {
                rc = RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcNotFound );
            }
            else
            {
                int64_t last;

                rc = VColumnIdRange ( vcol, p_first, & last );
                if  (rc == 0 )
                {
                    * p_count = last + 1 - * p_first;
                }
                return rc;
            }
        }

        * p_first = 0;
        * p_count = 0;
    }

    return rc;
}


/* SetRowIdRead - PRIVATE
 *  seek to given row id
 *
 *  "row_id" [ IN ] - row id to select
 */
rc_t
VCursorSetRowIdRead ( VCursor *self, int64_t row_id )
{
    assert ( self != NULL );

    self -> row_id = row_id;
    return 0;
}


/* FindNextRowId
 *  finds the id of the next row having valid ( non-null ) cell data
 *  "*next" may contain any row id > "VCursorRowId()".
 *  returns rcNotFound if none can be located.
 *
 *  "idx" [ IN, DEFAULT ZERO ] - single column index or
 *  zero to indicate the range for all columns in cursor.
 *  in the latter case ( idx == 0 ), we find the first row having
 *  ANY valid cell data.
 *
 *  "next" [ OUT ] - return parameter for next valid row id
 *
 * FindNextRowIdDirect
 *  "start_id" [ IN ] - starting id for search, such that "*next"
 *  could contain any id >= start_id upon successful return
 */
rc_t VCursorRowFindNextRowId ( const Vector * self, uint32_t idx, int64_t start_id, int64_t * next )
{
    uint32_t i;
    int64_t best = INT64_MAX;
    rc_t rc = SILENT_RC ( rcVDB, rcCursor, rcSelecting, rcCursor, rcEmpty );

    uint32_t start, end;

    /* for walking across the open columns */
    assert ( self != NULL );
    start = VectorStart ( self );
    end = start + VectorLength ( self );

    /* if using a specific column, ensure the index is proper */
    if ( idx != 0 )
    {
        if ( idx < start || idx >= end )
            return RC ( rcVDB, rcCursor, rcSelecting, rcId, rcInvalid );

        /* set the range to be just this column */
        start = idx;
        end = idx + 1;
    }

    /* walk across all columns */
    for ( i = start; i < end; ++ i )
    {
        /* retrieve the column */
        const VColumn * vcol = ( const VColumn * ) VectorGet ( self, i );

        /* could assert that vcol != NULL, because it should never be so.
           but the purpose of this function is not so much to insist on
           this property but to discover the next row id */
        if ( vcol != NULL )
        {
            /* assume the column is physical */
            bool is_static = false;
            KColumn * kcol = NULL;
            rc = VColumnGetKColumn ( vcol, & kcol, & is_static );
            if ( kcol != NULL && rc == 0 )
            {
                /* we have a physical column - ask kdb what the next id is */
                rc = KColumnFindFirstRowId ( kcol, next, start_id );
                KColumnRelease ( kcol );
                if ( rc == 0 )
                {
                    /* there can be no better than the supplied id */
                    if ( * next == start_id )
                        break;

                    /* record the best of all columns */
                    if ( * next < best )
                        best = * next;
                }

                /* if this column has no more ids, try next column */
                else if ( GetRCState ( rc ) != rcNotFound )
                    break;
            }
            else if ( is_static )
            {
                /* we have a static column, meaning a contiguous range of row ids */
                int64_t sfirst, slast;
                rc = VColumnIdRange ( vcol, & sfirst, & slast );
                if ( rc != 0 )
                    break;
                if ( sfirst >= slast || start_id > slast )
                {
                    rc = RC ( rcVDB, rcCursor, rcSelecting, rcRow, rcNotFound );
                    break;
                }
                if ( start_id >= sfirst )
                {
                    * next = start_id;
                    rc = 0;
                    break;
                }
                if ( sfirst < best )
                    best = sfirst;

            }

            * next = best;
        }
    }

    return rc;
}

rc_t
VCursorFindNextRowId ( const VCursor *self, uint32_t idx, int64_t *next )
{
    rc_t rc = 0;

    if ( next == NULL )
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    else
    {
        return VCursorRowFindNextRowId ( & self -> row, idx, self -> row_id + 1, next );
    }

    return rc;
}

rc_t
VCursorFindNextRowIdDirect ( const VCursor *self, uint32_t idx, int64_t start_id, int64_t *next )
{
    rc_t rc = 0;

    if ( next == NULL )
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    else
    {
        return VCursorRowFindNextRowId ( & self -> row, idx, start_id, next );
    }

    return rc;
}

/* OpenRowRead
 * CloseRowRead
 */
rc_t
VCursorOpenRowRead ( VCursor *self )
{
    assert ( self != NULL );

    self -> state = vcRowOpen;
    return 0;
}

rc_t
VCursorCloseRowRead ( VCursor *self )
{
    assert ( self != NULL );

    ++ self -> row_id;
    self -> state = vcReady;
    return 0;
}

/* GetUserData
 *  store/retrieve an opaque pointer to user data
 *
 *  "data" [ OUT ] - return parameter for getting data
 */
rc_t
VCursorGetUserData ( const VCursor *self, void **data )
{
    rc_t rc;

    if ( data == NULL )
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    else
    {
        * data = self -> user;
        return 0;
    }

    return rc;
}

/* SetUserData
 *  store/retrieve an opaque pointer to user data
 *
 *  "data" [ IN ] - parameter for setting data
 *
 *  "destroy" [ IN, NULL OKAY ] - optional destructor param
 *  invoked from destructor of "self"
 */
rc_t
VCursorSetUserData ( const VCursor *cself, void *data, void ( CC * destroy ) ( void *data ) )
{
    VCursor *self = ( VCursor* ) cself;

    self -> user = data;
    self -> user_whack = destroy;

    return 0;
}

rc_t
VCursorPageIdRange ( struct VCursor const *self, uint32_t idx, int64_t id, int64_t *first, int64_t *last )
{
    rc_t rc;

    if ( first == NULL && last == NULL )
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    else
    {
        int64_t dummy;
        if ( first == NULL )
            first = & dummy;
        else if ( last == NULL )
            last = & dummy;

        if ( self == NULL )
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcSelf, rcNull );
        else
        {
            const VColumn *vcol = ( const VColumn* ) VectorGet ( & self -> row, idx );
            if ( vcol == NULL )
                rc = RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcNotFound );
            else
                return VColumnPageIdRange ( vcol, id, first, last );
        }

        * first = * last = 0;
    }

    return rc;
}

/* IsStaticColumn
 *  answers question: "does this column have the same value for every cell?"
 */
LIB_EXPORT
rc_t CC
VCursorIsStaticColumn ( const VCursor *self, uint32_t col_idx, bool *is_static )
{
    rc_t rc;

    if ( is_static == NULL )
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcSelf, rcNull );
        else
        {
            uint32_t start = VectorStart ( & self -> row );
            uint32_t end = start + VectorLength ( & self -> row );
            if ( col_idx < start || col_idx >= end )
                rc = RC ( rcVDB, rcCursor, rcSelecting, rcId, rcInvalid );
            else
            {
                VColumn *col = VectorGet ( & self -> row, col_idx );
                return VColumnIsStatic ( col, is_static );
            }
        }

        * is_static = false;
    }

    return rc;
}

VCursorCache *
VCursorColumns ( VCursor * self )
{
    assert ( self != NULL );
    return & self -> col;
}

VCursorCache *
VCursorPhysicalColumns ( struct VCursor * self )
{
    assert ( self != NULL );
    return & self -> phys;
}

Vector *
VCursorGetRow ( struct VCursor * self )
{
    assert ( self != NULL );
    return & self -> row;
}

