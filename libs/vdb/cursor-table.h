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

struct VCursor;
typedef struct VCursor VTableCursor; /* TODO: break out VCursor with vt only; promote data members shared between table and view cursors */

#define VCURSOR_IMPL VTableCursor

#include "cursor-struct.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * functions defined for table-based cursors only
 */

rc_t VCursorMakeFromTable ( VTableCursor **cursp, const struct VTable *tbl );

/* "constructor", provides a concrete VCursor_vt. Defined in cursor.c and wcursor.c */
rc_t VTableCursorMake ( VTableCursor **cursp, const struct VTable *tbl, VCursor_vt *vt );

/* implementations of methods in VCursor_vt shared between cursor.c and wcursor.c */

rc_t VTableCursorAddRef ( const VTableCursor *self );
rc_t VTableCursorRelease ( const VTableCursor *self );
rc_t VTableCursorVAddColumn ( const VTableCursor *cself, uint32_t *idx, const char *name, va_list args );
rc_t VTableCursorVGetColumnIdx ( const VTableCursor *self, uint32_t *idx, const char *name, va_list args );
rc_t VTableCursorDatatype ( const VTableCursor *self, uint32_t idx, struct VTypedecl *type, struct VTypedesc *desc );
rc_t VTableCursorIdRange ( const VTableCursor *self, uint32_t idx, int64_t *first, uint64_t *count );
rc_t VTableCursorRowId ( const VTableCursor *self, int64_t *id );
rc_t VTableCursorFindNextRowId ( const VTableCursor *self, uint32_t idx, int64_t *next );
rc_t VTableCursorFindNextRowIdDirect ( const VTableCursor *self, uint32_t idx, int64_t start_id, int64_t *next );
rc_t VTableCursorGetBlob ( const VTableCursor *self, const VBlob **blob, uint32_t col_idx );
rc_t VTableCursorGetBlobDirect ( const VTableCursor *self, const VBlob **blob, int64_t row_id, uint32_t col_idx );
rc_t VTableCursorRead ( const VTableCursor *self, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
rc_t VTableCursorReadDirect ( const VTableCursor *self, int64_t row_id, uint32_t col_idx, uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
rc_t VTableCursorReadBits ( const VTableCursor *self, uint32_t col_idx, uint32_t elem_bits, uint32_t start, void *buffer, uint32_t off, uint32_t blen, uint32_t *num_read, uint32_t *remaining );
rc_t VTableCursorReadBitsDirect ( const VTableCursor *self, int64_t row_id, uint32_t col_idx, uint32_t elem_bits, uint32_t start, void *buffer, uint32_t off, uint32_t blen, uint32_t *num_read, uint32_t *remaining );
rc_t VTableCursorCellData ( const VTableCursor *self, uint32_t col_idx, uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
rc_t VTableCursorCellDataDirect ( const VTableCursor *self, int64_t row_id, uint32_t col_idx,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
rc_t VTableCursorDataPrefetch( const VTableCursor *cself, const int64_t *row_ids, uint32_t col_idx, uint32_t num_rows, int64_t min_valid_row_id, int64_t max_valid_row_id, bool continue_on_error );
rc_t VTableCursorOpenParentRead ( const VTableCursor *self, const struct VTable **tbl );
rc_t VTableCursorGetUserData ( const VTableCursor *self, void **data );
rc_t VTableCursorSetUserData ( const VTableCursor *cself, void *data, void ( CC * destroy ) ( void *data ) );

VCursorCache * VTableCursorColumns ( VTableCursor * self );
VCursorCache * VTableCursorPhysicalColumns ( VTableCursor * self );
Vector * VTableCursorGetRow ( VTableCursor * self );

/* functions shared between cursor.c and wcursor.c */

rc_t VTableCreateCursorWriteInt ( struct VTable *self, VTableCursor **cursp, KCreateMode mode, bool create_thread );

/* Whack
 * Destroy
 */
rc_t VCursorWhack ( VTableCursor *self );
rc_t VCursorDestroy ( VTableCursor *self );

/* SupplementSchema
 *  scan table for physical column names
 *  create transparent yet incomplete (untyped) columns for unknown names
 *  create incomplete (untyped) physical columns for forwarded names
 *  repeat process on static columns, except create complete (fully typed) objects
 */
rc_t VCursorSupplementSchema ( VTableCursor const *self );

/* SetRowIdRead - PRIVATE
 *  seek to given row id
 *
 *  "row_id" [ IN ] - row id to select
 */
rc_t VCursorSetRowIdRead ( VTableCursor *self, int64_t row_id );

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
rc_t VCursorListReadableColumns ( VTableCursor *self, BSTree *columns );

/* ListWritableColumns
 *  walks list of SPhysicals and trigger SProductions
 *  attempts to resolve all write rules
 *  records any SColumn that can be reached
 *  populates BTree with VColumnRef objects
 */
rc_t VCursorListWritableColumns ( VTableCursor *self, BSTree *columns );
rc_t VCursorListSeededWritableColumns ( VTableCursor *self, BSTree *columns, struct KNamelist const *seed );

/* PostOpenAdd
 *  handle opening of a column after the cursor is opened
 */
rc_t VCursorPostOpenAdd ( VTableCursor *self, struct VColumn *col );
rc_t VCursorPostOpenAddRead ( VTableCursor *self, struct VColumn *col );

/* OpenRowRead
 * CloseRowRead
 */
rc_t VCursorOpenRowRead ( VTableCursor *self );
rc_t VCursorCloseRowRead ( VTableCursor *self );

/* Open
 */
rc_t VCursorOpenRead ( VTableCursor *self, struct KDlset const *libs );

Vector * VTableCursorGetRow ( VTableCursor * self );

const struct VTable * VTableCursorGetTable ( const VTableCursor * self );

bool VTableCursorIsReadOnly ( const VTableCursor * self );
const struct VSchema * VTableCursorGetSchema ( const VTableCursor * self );

VBlobMRUCache * VTableCursorGetBlobMruCache ( VTableCursor * self );
uint32_t VTableCursorIncrementPhysicalProductionCount ( VTableCursor * self );

const struct KSymbol * VTableCursorFindOverride ( const VTableCursor *self, const struct VCtxId *cid );

#ifdef __cplusplus
}
#endif

#endif /* _h_cursor_table_priv_ */
