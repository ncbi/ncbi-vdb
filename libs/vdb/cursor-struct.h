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

#ifndef _h_cursor_struct_
#define _h_cursor_struct_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_vector_
#include <klib/vector.h>
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#include <vdb/cursor.h>
#include "cursor-priv.h"

#ifndef KONST
#define KONST
#endif

#ifndef SKONST
#define SKONST const
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KLock;
struct KCondition;
struct KThread;
struct KSymbol;
struct SColumn;
struct VTable;
struct VCtxId;
struct VSchema;
struct VColumn;

#ifndef VCURSOR_IMPL
#define VCURSOR_IMPL VCursor
#endif

typedef struct VCursor_vt VCursor_vt;
struct VCursor_vt
{
    /* Public API */
    rc_t ( CC * whack ) ( const VCURSOR_IMPL *self );
    rc_t ( CC * vAddColumn ) ( const VCURSOR_IMPL *self, uint32_t *idx, const char *name, va_list args );
    rc_t ( CC * vGetColumnIdx ) ( const VCURSOR_IMPL *self, uint32_t *idx, const char *name, va_list args );
    rc_t ( CC * open ) ( const VCURSOR_IMPL *self );
    rc_t ( CC * setRowId ) ( const VCURSOR_IMPL *self, int64_t row_id );
    rc_t ( CC * openRow ) ( const VCURSOR_IMPL *self );
    rc_t ( CC * write ) ( VCURSOR_IMPL *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count );
    rc_t ( CC * commitRow ) ( VCURSOR_IMPL *self );
    rc_t ( CC * closeRow ) ( const VCURSOR_IMPL *self );
    rc_t ( CC * repeatRow ) ( VCURSOR_IMPL *self, uint64_t count );
    rc_t ( CC * flushPage ) ( VCURSOR_IMPL *self );
    rc_t ( CC * getBlob ) ( const VCURSOR_IMPL *self, struct VBlob const **blob, uint32_t col_idx );
    rc_t ( CC * getBlobDirect ) ( const VCURSOR_IMPL *self, struct VBlob const **blob, int64_t row_id, uint32_t col_idx );
    rc_t ( CC * read ) ( const VCURSOR_IMPL *self, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
    rc_t ( CC * readDirect ) ( const VCURSOR_IMPL *self, int64_t row_id, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
    rc_t ( CC * readBits ) ( const VCURSOR_IMPL *self, uint32_t col_idx, uint32_t elem_bits, uint32_t start, void *buffer, uint32_t boff, uint32_t blen, uint32_t *num_read, uint32_t *remaining );
    rc_t ( CC * readBitsDirect ) ( const VCURSOR_IMPL *self, int64_t row_id, uint32_t col_idx, uint32_t elem_bits, uint32_t start, void *buffer, uint32_t boff, uint32_t blen, uint32_t *num_read, uint32_t *remaining );
    rc_t ( CC * cellData ) ( const VCURSOR_IMPL *self, uint32_t col_idx, uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
    rc_t ( CC * cellDataDirect ) ( const VCURSOR_IMPL *self, int64_t row_id, uint32_t col_idx, uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
    rc_t ( CC * dataPrefetch ) ( const VCURSOR_IMPL * self, const int64_t * row_ids, uint32_t col_idx, uint32_t num_rows, int64_t min_valid_row_id, int64_t max_valid_row_id, bool );
    rc_t ( CC * _default ) ( VCURSOR_IMPL *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len );
    rc_t ( CC * commit ) ( VCURSOR_IMPL *self );
    rc_t ( CC * openParentRead ) ( const VCURSOR_IMPL *self, struct VTable const **tbl );
    rc_t ( CC * openParentUpdate ) ( VCURSOR_IMPL *self, struct VTable **tbl );

    /* Private API, deined in interfaces/vdb/vdb-priv.h */
    rc_t ( CC * permitPostOpenAdd ) ( const VCURSOR_IMPL * self );
    rc_t ( CC * suspendTriggers ) ( const VCURSOR_IMPL * self );
    struct VSchema const * ( * getSchema ) ( const VCURSOR_IMPL * self);
    rc_t ( CC * linkedCursorGet ) ( const VCURSOR_IMPL *cself,const char *tbl, struct VCursor const **curs);
    rc_t ( CC * linkedCursorSet ) ( const VCURSOR_IMPL *cself,const char *tbl, struct VCursor const *curs);
    uint64_t ( CC * setCacheCapacity ) ( VCURSOR_IMPL *self,uint64_t capacity);
    uint64_t ( CC * getCacheCapacity ) ( const VCURSOR_IMPL *self);

    /* libvdb-internal API, deined in cursor-priv.h */
    rc_t ( * makeColumn ) ( VCURSOR_IMPL *self, struct VColumn **col, const struct SColumn *scol, Vector *cx_bind );
    const struct VTable * ( * getTable ) ( const VCURSOR_IMPL * self );
    bool ( * isReadOnly ) ( const VCURSOR_IMPL * self );
    VBlobMRUCache * ( * getBlobMruCache ) ( VCURSOR_IMPL * self );
    uint32_t ( * incrementPhysicalProductionCount ) ( VCURSOR_IMPL * self );
    const struct KSymbol * ( * findOverride ) ( const VCURSOR_IMPL *self, const struct VCtxId *cid );
    rc_t ( * launchPagemapThread ) ( VCURSOR_IMPL *self );
    const PageMapProcessRequest * ( * pageMapProcessRequest ) ( const VCURSOR_IMPL *self );
    bool ( * cacheActive ) ( const VCURSOR_IMPL * self, int64_t * cache_empty_end );
    rc_t ( * installTrigger ) ( VCURSOR_IMPL * self, struct VProduction * prod );
};

struct VCursor
{
    VCursor_vt * vt;

    /* row id */
    int64_t row_id;

    /* half-closed page range */
    int64_t start_id;
    int64_t end_id;

    /* user data */
    void *user;
    void ( CC * user_whack ) ( void *data );

    /* external row of VColumn* by ord ( owned ) */
    Vector row;

    /* column objects by cid ( not-owned ) */
    VCursorCache col;

    /* physical columns by cid ( owned ) */
    VCursorCache phys;

    /* productions by cid ( not-owned ) */
    VCursorCache prod;

    /* intermediate productions ( owned ) */
    Vector owned;

    KRefcount refcount;

    /* foreground state */
    uint8_t state;
};

#ifdef __cplusplus
}
#endif

#endif /* _h_cursor_struct_ */
