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

#ifndef _h_cursor_table_priv_
#define _h_cursor_table_priv_

#ifdef __cplusplus
extern "C" {
#endif

struct VTableCursor;
typedef struct VTableCursor VTableCursor;

#define VCURSOR_IMPL VTableCursor
#include "cursor-struct.h"

struct VTableCursor
{
    struct VCursor_vt * vt;

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

/*--------------------------------------------------------------------------
 * functions defined for table-based cursors only
 */

rc_t VCursorMakeFromTable ( VCURSOR_IMPL **cursp, const struct VTable *tbl );

/* "constructor", provides a concrete VCursor_vt. Defined in cursor.c and wcursor.c */
rc_t VTableCursorMake ( VCURSOR_IMPL **cursp, const struct VTable *tbl, VCursor_vt *vt );

/* implementations of methods in VCursor_vt shared between cursor.c and wcursor.c */

rc_t VTableCursorAddRef ( const VCURSOR_IMPL *self );
rc_t VTableCursorRelease ( const VCURSOR_IMPL *self );
rc_t VTableCursorVAddColumn ( const VCURSOR_IMPL *cself, uint32_t *idx, const char *name, va_list args );
rc_t VTableCursorVGetColumnIdx ( const VCURSOR_IMPL *self, uint32_t *idx, const char *name, va_list args );
rc_t VTableCursorDatatype ( const VCURSOR_IMPL *self, uint32_t idx, struct VTypedecl *type, struct VTypedesc *desc );
rc_t VTableCursorIdRange ( const VCURSOR_IMPL *self, uint32_t idx, int64_t *first, uint64_t *count );
rc_t VTableCursorRowId ( const VCURSOR_IMPL *self, int64_t *id );
rc_t VTableCursorFindNextRowId ( const VCURSOR_IMPL *self, uint32_t idx, int64_t *next );
rc_t VTableCursorFindNextRowIdDirect ( const VCURSOR_IMPL *self, uint32_t idx, int64_t start_id, int64_t *next );
rc_t VTableCursorGetBlob ( const VCURSOR_IMPL *self, const VBlob **blob, uint32_t col_idx );
rc_t VTableCursorGetBlobDirect ( const VCURSOR_IMPL *self, const VBlob **blob, int64_t row_id, uint32_t col_idx );
rc_t VTableCursorRead ( const VCURSOR_IMPL *self, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
rc_t VTableCursorReadDirect ( const VCURSOR_IMPL *self, int64_t row_id, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
rc_t VTableCursorReadBits ( const VCURSOR_IMPL *self, uint32_t col_idx, uint32_t elem_bits, uint32_t start, void *buffer, uint32_t off, uint32_t blen, uint32_t *num_read, uint32_t *remaining );
rc_t VTableCursorReadBitsDirect ( const VCURSOR_IMPL *self, int64_t row_id, uint32_t col_idx, uint32_t elem_bits, uint32_t start, void *buffer, uint32_t off, uint32_t blen, uint32_t *num_read, uint32_t *remaining );
rc_t VTableCursorCellData ( const VCURSOR_IMPL *self, uint32_t col_idx, uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
rc_t VTableCursorCellDataDirect ( const VCURSOR_IMPL *self, int64_t row_id, uint32_t col_idx,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
rc_t VTableCursorDataPrefetch( const VCURSOR_IMPL *cself, const int64_t *row_ids, uint32_t col_idx, uint32_t num_rows, int64_t min_valid_row_id, int64_t max_valid_row_id, bool continue_on_error );
rc_t VTableCursorOpenParentRead ( const VCURSOR_IMPL *self, const struct VTable **tbl );
rc_t VTableCursorGetUserData ( const VCURSOR_IMPL *self, void **data );
rc_t VTableCursorSetUserData ( const VCURSOR_IMPL *cself, void *data, void ( CC * destroy ) ( void *data ) );

VCursorCache * VTableCursorColumns ( VCURSOR_IMPL * self );
VCursorCache * VTableCursorPhysicalColumns ( VCURSOR_IMPL * self );
Vector * VTableCursorGetRow ( VCURSOR_IMPL * self );

/* functions shared between cursor.c and wcursor.c */

rc_t VTableCreateCursorWriteInt ( struct VTable *self, VCURSOR_IMPL **cursp, KCreateMode mode, bool create_thread );

/* Whack
 * Destroy
 */
rc_t VCursorWhack ( VCURSOR_IMPL *self );
rc_t VCursorDestroy ( VCURSOR_IMPL *self );

/* SupplementSchema
 *  scan table for physical column names
 *  create transparent yet incomplete (untyped) columns for unknown names
 *  create incomplete (untyped) physical columns for forwarded names
 *  repeat process on static columns, except create complete (fully typed) objects
 */
rc_t VCursorSupplementSchema ( VCURSOR_IMPL const *self );

/* SetRowIdRead - PRIVATE
 *  seek to given row id
 *
 *  "row_id" [ IN ] - row id to select
 */
rc_t VCursorSetRowIdRead ( VCURSOR_IMPL *self, int64_t row_id );

/**
*** VTableCreateCursorReadInternal is only visible in vdb and needed for schema resolutions
****/
rc_t  VTableCreateCursorReadInternal(const struct VTable *self, const VTableCursor **cursp);


/* ListReadableColumns
 *  performs an insert of '*' to cursor
 *  attempts to resolve all read rules
 *  records all SColumns that successfully resolved
 *  populates BTree with VColumnRef objects
 */
rc_t VCursorListReadableColumns ( VCURSOR_IMPL *self, BSTree *columns );

/* ListWritableColumns
 *  walks list of SPhysicals and trigger SProductions
 *  attempts to resolve all write rules
 *  records any SColumn that can be reached
 *  populates BTree with VColumnRef objects
 */
rc_t VCursorListWritableColumns ( VCURSOR_IMPL *self, BSTree *columns );
rc_t VCursorListSeededWritableColumns ( VCURSOR_IMPL *self, BSTree *columns, struct KNamelist const *seed );

/* PostOpenAdd
 *  handle opening of a column after the cursor is opened
 */
rc_t VCursorPostOpenAdd ( VCURSOR_IMPL *self, struct VColumn *col );
rc_t VCursorPostOpenAddRead ( VCURSOR_IMPL *self, struct VColumn *col );

/* OpenRowRead
 * CloseRowRead
 */
rc_t VCursorOpenRowRead ( VCURSOR_IMPL *self );
rc_t VCursorCloseRowRead ( VCURSOR_IMPL *self );

/* Open
 */
rc_t VCursorOpenRead ( VCURSOR_IMPL *self, struct KDlset const *libs );

Vector * VTableCursorGetRow ( VCURSOR_IMPL * self );

const struct VTable * VTableCursorGetTable ( const VCURSOR_IMPL * self );

bool VTableCursorIsReadOnly ( const VCURSOR_IMPL * self );
const struct VSchema * VTableCursorGetSchema ( const VCURSOR_IMPL * self );

VBlobMRUCache * VTableCursorGetBlobMruCache ( VCURSOR_IMPL * self );
uint32_t VTableCursorIncrementPhysicalProductionCount ( VCURSOR_IMPL * self );

const struct KSymbol * VTableCursorFindOverride ( const VCURSOR_IMPL *self, const struct VCtxId *cid );

rc_t CC VTableCursorPermitPostOpenAdd ( struct VCURSOR_IMPL const *self );

rc_t CC VTableCursorSuspendTriggers ( struct VCURSOR_IMPL const *self );

struct VSchema const* CC VTableCursorGetSchema ( struct VCURSOR_IMPL const *self);

rc_t CC VTableCursorPageIdRange ( struct VCURSOR_IMPL const *self,
    uint32_t idx, int64_t id, int64_t *first, int64_t *last );

rc_t CC VTableCursorIsStaticColumn ( struct VCURSOR_IMPL const *self,
    uint32_t col_idx, bool *is_static );

rc_t CC VTableCursorLinkedCursorGet(const struct VCURSOR_IMPL *cself,const char *tbl, struct VCursor const **curs);
rc_t CC VTableCursorLinkedCursorSet(const struct VCURSOR_IMPL *cself,const char *tbl, struct VCursor const *curs);

uint64_t CC VTableCursorSetCacheCapacity(struct VCURSOR_IMPL *self,uint64_t capacity);
uint64_t CC VTableCursorGetCacheCapacity(const struct VCURSOR_IMPL *self);

/** pagemap supporting thread **/
rc_t VTableCursorLaunchPagemapThread ( struct VCURSOR_IMPL *self );
const PageMapProcessRequest* VTableCursorPageMapProcessRequest ( const struct VCURSOR_IMPL *self );
rc_t VTableCursorTerminatePagemapThread(struct VCURSOR_IMPL *self);

bool VTableCursorCacheActive ( const struct VCURSOR_IMPL * self, int64_t * cache_empty_end );

#ifdef __cplusplus
}
#endif

#endif /* _h_cursor_table_priv_ */
