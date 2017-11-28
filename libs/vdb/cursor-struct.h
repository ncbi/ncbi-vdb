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
struct KNamelist;
struct KDlset;
struct VTable;
struct VCtxId;
struct VSchema;
struct SColumn;
struct VColumn;
struct VPhysical;

typedef struct VCursor_vt VCursor_vt;
struct VCursor_vt
{
    rc_t ( CC * addRef ) ( const VCursor *self );
    rc_t ( CC * release ) ( const VCursor *self );
    rc_t ( CC * vAddColumn ) ( const VCursor *self, uint32_t *idx, const char *name, va_list args );
    rc_t ( CC * vGetColumnIdx ) ( const VCursor *self, uint32_t *idx, const char *name, va_list args );
    rc_t ( CC * datatype ) ( const VCursor *self, uint32_t idx, struct VTypedecl *type, struct VTypedesc *desc );
    rc_t ( CC * open ) ( const VCursor *self );
    rc_t ( CC * idRange ) ( const VCursor *self, uint32_t idx, int64_t *first, uint64_t *count );
    rc_t ( CC * rowId ) ( const VCursor *self, int64_t *row_id );
    rc_t ( CC * setRowId ) ( const VCursor *self, int64_t row_id );
    rc_t ( CC * findNextRowId ) ( const VCursor *self, uint32_t idx, int64_t * next );
    rc_t ( CC * findNextRowIdDirect ) ( const VCursor *self, uint32_t idx, int64_t start_id, int64_t * next );
    rc_t ( CC * openRow ) ( const VCursor *self );
    rc_t ( CC * write ) ( VCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count );
    rc_t ( CC * commitRow ) ( VCursor *self );
    rc_t ( CC * closeRow ) ( const VCursor *self );
    rc_t ( CC * repeatRow ) ( VCursor *self, uint64_t count );
    rc_t ( CC * flushPage ) ( VCursor *self );
    rc_t ( CC * getBlob ) ( const VCursor *self, struct VBlob const **blob, uint32_t col_idx );
    rc_t ( CC * getBlobDirect ) ( const VCursor *self, struct VBlob const **blob, int64_t row_id, uint32_t col_idx );
    rc_t ( CC * read ) ( const VCursor *self, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
    rc_t ( CC * readDirect ) ( const VCursor *self, int64_t row_id, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
    rc_t ( CC * readBits ) ( const VCursor *self, uint32_t col_idx, uint32_t elem_bits, uint32_t start, void *buffer, uint32_t boff, uint32_t blen, uint32_t *num_read, uint32_t *remaining );
    rc_t ( CC * readBitsDirect ) ( const VCursor *self, int64_t row_id, uint32_t col_idx, uint32_t elem_bits, uint32_t start, void *buffer, uint32_t boff, uint32_t blen, uint32_t *num_read, uint32_t *remaining );
    rc_t ( CC * cellData ) ( const VCursor *self, uint32_t col_idx, uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
    rc_t ( CC * cellDataDirect ) ( const VCursor *self, int64_t row_id, uint32_t col_idx, uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
    rc_t ( CC * dataPrefetch ) ( const VCursor * self, const int64_t * row_ids, uint32_t col_idx, uint32_t num_rows, int64_t min_valid_row_id, int64_t max_valid_row_id, bool );
    rc_t ( CC * _default ) ( VCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len );
    rc_t ( CC * commit ) ( VCursor *self );
    rc_t ( CC * openParentRead ) ( const VCursor *self, struct VTable const **tbl );
    rc_t ( CC * openParentUpdate ) ( VCursor *self, struct VTable **tbl );
    rc_t ( CC * getUserData ) ( const VCursor *self, void **data );
    rc_t ( CC * setUserData ) ( const VCursor *self, void *data, void ( CC * destroy ) ( void *data ) );
};

struct VCursor
{
    VCursor_vt * vt;

    /* row id */
    int64_t row_id;

    /* half-closed page range */
    int64_t start_id, end_id;

    /* starting id for flush */
    volatile int64_t flush_id;

    /* attached reference to table */
    struct VTable KONST *tbl;

    /* cursor-specific schema and table */
    struct VSchema SKONST *schema;
    struct STable SKONST *stbl;

    /* background flush thread objects */
    int64_t launch_cnt;
    struct KThread *flush_thread;
    struct KLock *flush_lock;
    struct KCondition *flush_cond;

    /* background pagemap conversion objects */
    struct KThread *pagemap_thread;
    PageMapProcessRequest pmpr;

    /* user data */
    void *user;
    void ( CC * user_whack ) ( void *data );

    /* external named cursor parameters */
    BSTree named_params;

    /* linked cursors */
    BSTree linked_cursors;

    /* read-only blob cache */
    VBlobMRUCache *blob_mru_cache;

    /* external row of VColumn* by ord ( owned ) */
    Vector row;

    Vector v_cache_curs;
    Vector v_cache_cidx;
    /** trying to prevent forward prefetch on rows which are cached ***/
    bool    cache_col_active;
    int64_t cache_empty_start; /** first rowid where cache is detected to be empty **/
    int64_t cache_empty_end;   /** last  rowid  **/

    /* column objects by cid ( not-owned ) */
    VCursorCache col;

    /* physical columns by cid ( owned ) */
    VCursorCache phys;
    uint32_t phys_cnt;

    /* productions by cid ( not-owned ) */
    VCursorCache prod;

    /* intermediate productions ( owned ) */
    Vector owned;

    /* trigger productions ( not-owned ) */
    Vector trig;

    KRefcount refcount;

    volatile uint32_t flush_cnt;

    /* foreground state */
    uint8_t state;

    /* flush_state */
    volatile uint8_t flush_state;

    bool read_only;

    /* support for sradb-v1 API */
    bool permit_add_column;
    bool permit_post_open_add;
    /* support suspension of schema-declared triggers **/
    bool suspend_triggers;
    /* cursor used in sub-selects */
    bool is_sub_cursor;
    /* cursor for VDB columns located in separate db.tbl ***/
    const struct VCursor* cache_curs;
};

/* VTableCursor methods */

/* "constructor", provides a concrete VCursor_vt. Defined in cursor.c and wcursor.c */
rc_t VCursorMake ( VCursor **cursp, const struct VTable *tbl );

rc_t VTableCursorMake ( VCursor **cursp, const struct VTable *tbl, VCursor_vt *vt );

/* methods shared between cursor.c and wcursor.c */
rc_t VTableCursorAddRef ( const VCursor *self );
rc_t VTableCursorRelease ( const VCursor *self );
rc_t VTableCursorVAddColumn ( const VCursor *cself, uint32_t *idx, const char *name, va_list args );
rc_t VTableCursorVGetColumnIdx ( const VCursor *self, uint32_t *idx, const char *name, va_list args );
rc_t VTableCursorDatatype ( const VCursor *self, uint32_t idx, struct VTypedecl *type, struct VTypedesc *desc );
rc_t VTableCursorIdRange ( const VCursor *self, uint32_t idx, int64_t *first, uint64_t *count );
rc_t VTableCursorRowId ( const VCursor *self, int64_t *id );
rc_t VTableCursorFindNextRowId ( const VCursor *self, uint32_t idx, int64_t *next );
rc_t VTableCursorFindNextRowIdDirect ( const VCursor *self, uint32_t idx, int64_t start_id, int64_t *next );
rc_t VTableCursorGetBlob ( const VCursor *self, const VBlob **blob, uint32_t col_idx );
rc_t VTableCursorGetBlobDirect ( const VCursor *self, const VBlob **blob, int64_t row_id, uint32_t col_idx );
rc_t VTableCursorRead ( const VCursor *self, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
rc_t VTableCursorReadDirect ( const VCursor *self, int64_t row_id, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
rc_t VTableCursorReadBits ( const VCursor *self, uint32_t col_idx, uint32_t elem_bits, uint32_t start, void *buffer, uint32_t off, uint32_t blen, uint32_t *num_read, uint32_t *remaining );
rc_t VTableCursorReadBitsDirect ( const VCursor *self, int64_t row_id, uint32_t col_idx, uint32_t elem_bits, uint32_t start, void *buffer, uint32_t off, uint32_t blen, uint32_t *num_read, uint32_t *remaining );
rc_t VTableCursorCellData ( const VCursor *self, uint32_t col_idx, uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
rc_t VTableCursorCellDataDirect ( const VCursor *self, int64_t row_id, uint32_t col_idx,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
rc_t VTableCursorDataPrefetch( const VCursor *cself, const int64_t *row_ids, uint32_t col_idx, uint32_t num_rows, int64_t min_valid_row_id, int64_t max_valid_row_id, bool continue_on_error );
rc_t VTableCursorOpenParentRead ( const VCursor *self, const struct VTable **tbl );
rc_t VTableCursorGetUserData ( const VCursor *self, void **data );
rc_t VTableCursorSetUserData ( const VCursor *cself, void *data, void ( CC * destroy ) ( void *data ) );

#ifdef __cplusplus
}
#endif

#endif /* _h_cursor_struct_ */
