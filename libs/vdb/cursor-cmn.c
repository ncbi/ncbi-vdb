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

#define KONST const
#define SKONST
 #include "cursor-struct.h"
#include "table-priv.h"
#undef KONST
#undef SKONST

#include <vdb/cursor.h>
#include <klib/rc.h>
#include <stdbool.h>

/* dispatch functions */
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

LIB_EXPORT rc_t CC VCursorAddRef ( const VCursor *self )
{
    DISPATCH_VALUE ( addRef ( self ), 0 ); /* here, NULL is OK */
}

LIB_EXPORT rc_t CC VCursorRelease ( const VCursor *self )
{
    DISPATCH_VALUE ( release ( self ), 0 ); /* here, NULL is OK */
}

LIB_EXPORT rc_t CC VCursorVAddColumn ( const VCursor *self, uint32_t *idx, const char *name, va_list args )
{
    DISPATCH ( vAddColumn ( self, idx, name, args ) );
}
LIB_EXPORT rc_t CC VCursorVGetColumnIdx ( const VCursor *self, uint32_t *idx, const char *name, va_list args )
{
    DISPATCH ( vGetColumnIdx ( self, idx, name, args ) );
}
LIB_EXPORT rc_t CC VCursorDatatype ( const VCursor *self, uint32_t idx, struct VTypedecl *type, struct VTypedesc *desc )
{
    DISPATCH ( datatype ( self, idx, type, desc ) );
}
LIB_EXPORT rc_t CC VCursorOpen ( const VCursor *self )
{
    DISPATCH ( open ( self ) );
}
LIB_EXPORT rc_t CC VCursorIdRange ( const VCursor *self, uint32_t idx, int64_t *first, uint64_t *count )
{
    DISPATCH ( idRange ( self, idx, first, count ) );
}
LIB_EXPORT rc_t CC VCursorRowId ( const VCursor *self, int64_t *id )
{
    DISPATCH ( rowId ( self, id ) );
}
LIB_EXPORT rc_t CC VCursorSetRowId ( const VCursor *self, int64_t row_id )
{
    DISPATCH ( setRowId ( self, row_id ) );
}
LIB_EXPORT rc_t CC VCursorFindNextRowId ( const VCursor *self, uint32_t idx, int64_t *next )
{
    DISPATCH ( findNextRowId ( self, idx, next ) );
}
LIB_EXPORT rc_t CC VCursorFindNextRowIdDirect ( const VCursor *self, uint32_t idx, int64_t start_id, int64_t *next )
{
    DISPATCH ( findNextRowIdDirect ( self, idx, start_id, next ) );
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
LIB_EXPORT rc_t CC VCursorGetUserData ( const VCursor *self, void **data )
{
    DISPATCH ( getUserData ( self, data ) );
}
LIB_EXPORT rc_t CC VCursorSetUserData ( const VCursor *self, void *data, void ( CC * destroy ) ( void *data ) )
{
    DISPATCH ( setUserData ( self, data, destroy ) );
}

/* private API dispatch */
VCursorCache * VCursorColumns ( struct VCursor * self )
{
    DISPATCH_VALUE ( columns ( self ), NULL );
}
rc_t VCursorMakeColumn ( struct VCursor *self, struct VColumn **col, struct SColumn const *scol, Vector *cx_bind )
{
    DISPATCH ( makeColumn ( self, col, scol, cx_bind ) );
}
VCursorCache * VCursorPhysicalColumns ( VCursor * self )
{
    DISPATCH_VALUE ( physicalColumns ( self ), NULL );
}
Vector * VCursorGetRow ( struct VCursor * self )
{
    DISPATCH_VALUE ( getRow ( self ), NULL );
}
const VTable * VCursorGetTable ( const struct VCursor * self )
{
    DISPATCH_VALUE ( getTable ( self ), NULL );
}
bool VCursorIsReadOnly ( const struct VCursor * self )
{
    DISPATCH_VALUE ( isReadOnly ( self ), true );
}
struct VSchema const * VCursorGetSchema ( struct VCursor const *self )
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
rc_t CC VCursorPermitPostOpenAdd ( struct VCursor const *self )
{
    DISPATCH ( permitPostOpenAdd ( self ) );
}
rc_t CC VCursorSuspendTriggers ( struct VCursor const *self )
{
    DISPATCH ( suspendTriggers ( self ) );
}
rc_t CC VCursorPageIdRange ( struct VCursor const *self, uint32_t idx, int64_t id, int64_t *first, int64_t *last )
{
    DISPATCH ( pageIdRange ( self, idx, id, first, last ) );
}
rc_t CC VCursorIsStaticColumn ( struct VCursor const *self, uint32_t col_idx, bool *is_static )
{
    DISPATCH ( isStaticColumn ( self, col_idx, is_static ) );
}
rc_t CC VCursorLinkedCursorGet( const struct VCursor * self,const char * tbl, struct VCursor const ** curs)
{
    DISPATCH ( linkedCursorGet ( self, tbl, curs ) );
}
rc_t CC VCursorLinkedCursorSet( const struct VCursor * self,const char * tbl, struct VCursor const * curs)
{
    DISPATCH ( linkedCursorSet ( self, tbl, curs ) );
}
uint64_t CC VCursorSetCacheCapacity ( struct VCursor * self,uint64_t capacity )
{
    DISPATCH_VALUE ( setCacheCapacity ( self, capacity ), 0 );
}
uint64_t CC VCursorGetCacheCapacity ( const struct VCursor * self )
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
void VCursorCacheWhack ( VCursorCache *self,
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
void *VCursorCacheGet ( const VCursorCache *self, const VCtxId *cid )
{
    const Vector *ctx = ( const void* ) VectorGet ( & self -> cache, cid -> ctx );
    return VectorGet ( ctx, cid -> id );
}

/* Set
 *  set object by cid
 */
rc_t VCursorCacheSet ( VCursorCache *self, const VCtxId *cid, const void *item )
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




