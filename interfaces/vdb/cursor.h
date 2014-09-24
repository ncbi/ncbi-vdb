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

#ifndef _h_vdb_cursor_
#define _h_vdb_cursor_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_namelist_
#include <klib/namelist.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VBlob;
struct VTable;
struct VTypedesc;
struct VTypedecl;


/*--------------------------------------------------------------------------
 * KCreateMode
 *  mode definitions for cursor creation
 *
 *  NB - typedef'd as uint32_t in <klib/defs.h> to ensure uniformly
 *  predictable binary representation.
 */
enum
{
    kcmUpdate,          /* kcmOpen   */
    kcmReplace,         /* kcmInit   */
    kcmInsert           /* kcmCreate */
};

/*--------------------------------------------------------------------------
 * VCursor
 *  a row cursor onto a VTable
 */
typedef struct VCursor VCursor;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
VDB_EXTERN rc_t CC VCursorAddRef ( const VCursor *self );
VDB_EXTERN rc_t CC VCursorRelease ( const VCursor *self );


/* CreateCursor
 *  creates a cursor object onto table
 *  multiple read cursors are allowed
 *  only a single write cursor is allowed
 *
 *  "curs" [ OUT ] - return parameter for newly created cursor
 *
 *  "mode" [ IN ] - describes update behavior
 *    kcmUpdate   : allow inserts or updates
 *    kcmReplace  : replace all existing rows with newly written rows
 *    kcmInsert   : allow only inserts, i.e. new rows
 *
 *  NB - CreateCursorRead will be deprecated in future releases
 *  use CreateCachedCursorRead instead.
 */
VDB_EXTERN rc_t CC VTableCreateCursorRead ( struct VTable const *self, const VCursor **curs );
VDB_EXTERN rc_t CC VTableCreateCursorWrite ( struct VTable *self, VCursor **curs, KCreateMode mode );


/* CreateCachedCursorRead
 *  creates a read cursor object onto table with a cache limit in bytes
 *
 *  AVAILABILITY: version 2.1
 *
 *  "curs" [ OUT ] - return parameter for newly created cursor
 *
 *  "capacity" [ IN ] - the maximum bytes to cache on the cursor before
 *  dropping least recently used blobs
 */
VDB_EXTERN rc_t CC VTableCreateCachedCursorRead ( struct VTable const *self,
    const VCursor **curs, size_t capacity );


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
 *
 * NB - may return a non-zero status code of rcColumn, rcExists
 *  if the column was not added. the return "idx" will still
 *  be set properly and this does NOT indicate an error.
 */
VDB_EXTERN rc_t CC VCursorAddColumn ( const VCursor *self,
    uint32_t *idx, const char *name, ... );
VDB_EXTERN rc_t CC VCursorVAddColumn ( const VCursor *self,
    uint32_t *idx, const char *name, va_list args );


/* GetColumnIdx
 *  retrieve column index by name spec
 *
 *  "idx" [ OUT ] - return parameter for column index
 *
 *  "name" [ IN ] - NUL terminated column name spec.
 */
VDB_EXTERN rc_t CC VCursorGetColumnIdx ( const VCursor *self,
    uint32_t *idx, const char *name, ... );
VDB_EXTERN rc_t CC VCursorVGetColumnIdx ( const VCursor *self,
    uint32_t *idx, const char *name, va_list args );


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
VDB_EXTERN rc_t CC VCursorDatatype ( const VCursor *self, uint32_t idx,
    struct VTypedecl *type, struct VTypedesc *desc );


/* IdRange
 *  returns id range for column
 *
 *  "idx" [ IN, DEFAULT ZERO ] - single column index or
 *  zero to indicate the range for all columns in cursor
 *
 *  "id" [ IN ] - page containing this row id is target
 *
 *  "first" [ OUT, NULL OKAY ] and "count" [ OUT, NULL OKAY ] -
 *  id range is returned in these output parameters, where
 *  at least ONE must be NOT-NULL
 */
VDB_EXTERN rc_t CC VCursorIdRange ( const VCursor *self, uint32_t idx,
    int64_t *first, uint64_t *count );


/* Open
 *  open cursor, resolving schema for the set of opened columns
 *
 *  when cursor is created for read, its initial row id
 *  is set to first row available in any contained column.
 *
 *  when cursor is created for write, its initial row id
 *  is set for inserts ( appending ). when empty, initial
 *  row id is set to 1. otherwise, it is set to 1 beyond
 *  the last row available in any contained column.
 *
 *  NB - there is no corresponding "Close"
 *  use "Release" instead.
 */
VDB_EXTERN rc_t CC VCursorOpen ( const VCursor *self );


/* RowId
 *  report current row id
 * SetRowId
 *  seek to given row id
 */
VDB_EXTERN rc_t CC VCursorRowId ( const VCursor *self, int64_t *row_id );
VDB_EXTERN rc_t CC VCursorSetRowId ( const VCursor *self, int64_t row_id );


/* OpenRow
 *  open currently closed row indicated by row id
 */
VDB_EXTERN rc_t CC VCursorOpenRow ( const VCursor *self );

/* CommitRow
 *  commit row after writing
 *  prevents further writes
 */
VDB_EXTERN rc_t CC VCursorCommitRow ( VCursor *self );

/* RepeatRow
 *  repeats the current row by the count provided
 *  row must have been committed
 *
 *  AVAILABILITY: version 2.6
 *
 *  "count" [ IN ] - the number of times to repeat
 *  the current row.
 */
VDB_EXTERN rc_t CC VCursorRepeatRow ( VCursor *self, uint64_t count );

/* CloseRow
 *  balances OpenRow message
 *  if there are uncommitted modifications,
 *  discard all changes. otherwise,
 *  advance to next row
 */
VDB_EXTERN rc_t CC VCursorCloseRow ( const VCursor *self );


/* FlushPage
 *  forces flush of all buffered page data
 *  fails if row is open
 *
 *  pages are normally auto-committed based upon
 *  size and column affinity
 */
VDB_EXTERN rc_t CC VCursorFlushPage ( VCursor *self );


/* GetBlob
 *  retrieve a blob of data containing the current row id
 * GetBlobDirect
 *  retrieve a blob of data containing the requested row id
 *
 *  "blob" [ OUT ] - return parameter for a new reference
 *  to VBlob containing requested cell. NB - must be released
 *  via VBlobRelease when no longer needed.
 *
 *  "row_id" [ IN ] - allows ReadDirect random access to any cell
 *  in column
 *
 *  "col_idx" [ IN ] - index of column to be read, returned by "AddColumn"
 */
VDB_EXTERN rc_t CC VCursorGetBlob ( const VCursor *self,
    struct VBlob const **blob, uint32_t col_idx );
VDB_EXTERN rc_t CC VCursorGetBlobDirect ( const VCursor *self,
    struct VBlob const **blob, int64_t row_id, uint32_t col_idx );


/* Read
 *  read entire single row of byte-aligned data into a buffer
 * ReadDirect
 *  bypass the need to use SetRowId/OpenRow/CloseRow for addressing
 *
 *  "row_id" [ IN ] - allows ReadDirect random access to any cell
 *  in column
 *
 *  "col_idx" [ IN ] - index of column to be read, returned by "AddColumn"
 *
 *  "elem_bits" [ IN ] - expected element size in bits, required
 *  to be compatible with the actual element size, and be a multiple
 *  of 8 ( byte-aligned ). for non-byte-aligned data, see ReadBits
 *
 *  "buffer" [ OUT ] and "blen" [ IN ] - return buffer for row data
 *  where "blen" gives buffer capacity in elements. the total buffer
 *  size in bytes == ( "elem_bits" * "blen" + 7 ) / 8.
 *
 *  "row_len" [ OUT ] - return parameter for the number of elements
 *  in the requested row.
 *
 *  when the return code is 0, "row_len" will contain the number of
 *  elements read into buffer. if the return code indicates that the
 *  buffer is too small, "row_len" will give the required buffer length.
 */
VDB_EXTERN rc_t CC VCursorRead ( const VCursor *self, uint32_t col_idx,
    uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
VDB_EXTERN rc_t CC VCursorReadDirect ( const VCursor *self, int64_t row_id, uint32_t col_idx,
    uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );


/* ReadBits
 *  read single row of potentially bit-aligned column data into a buffer
 * ReadBitsDirect
 *  bypass the need to use SetRowId/OpenRow/CloseRow for addressing
 *
 *  "col_idx" [ IN ] - index of column to be read, returned by "AddColumn"
 *
 *  "elem_bits" [ IN ] - expected element size in bits, required to be
 *  compatible with the actual element size, and may ( or may not ) be
 *  a multiple of 8 ( byte aligned ).
 *
 *  "start" [ IN ] - zero-based starting index to first element,
 *  valid from 0 .. row_len - 1
 *
 *  "buffer" [ IN ], "boff" [ IN ] and "blen" [ IN ] -
 *  return buffer for row data, where "boff" is in BITS
 *  and "blen" is in ELEMENTS.
 *
 *  "num_read" [ OUT ] - return parameter for the number of elements
 *  read, which is <= "blen"
 *
 *  "remaining" [ OUT, NULL OKAY ] - optional return parameter for
 *  the number of elements remaining to be read. specifically,
 *  "start" + "num_read" + "remaining" == row length, assuming that
 *  "start" <= row length.
 */
VDB_EXTERN rc_t CC VCursorReadBits ( const VCursor *self, uint32_t col_idx,
    uint32_t elem_bits, uint32_t start, void *buffer, uint32_t boff,
    uint32_t blen, uint32_t *num_read, uint32_t *remaining );
VDB_EXTERN rc_t CC VCursorReadBitsDirect ( const VCursor *self, int64_t row_id, uint32_t col_idx,
    uint32_t elem_bits, uint32_t start, void *buffer, uint32_t boff,
    uint32_t blen, uint32_t *num_read, uint32_t *remaining );


/* CellData
 *  access pointer to single cell of potentially bit-aligned column data
 * CellDataDirect
 *  bypass the need to use SetRowId/OpenRow/CloseRow for addressing
 *
 *  "col_idx" [ IN ] - index of column to be read, returned by "AddColumn"
 *
 *  "elem_bits" [ OUT, NULL OKAY ] - optional return parameter for
 *  element size in bits
 *
 *  "base" [ OUT ] and "boff" [ OUT, NULL OKAY ] -
 *  compound return parameter for pointer to row starting bit
 *  where "boff" is in BITS
 *
 *  "row_len" [ OUT, NULL OKAY ] - the number of elements in cell
 */
VDB_EXTERN rc_t CC VCursorCellData ( const VCursor *self, uint32_t col_idx,
    uint32_t *elem_bits, const void **base, uint32_t *boff,
    uint32_t *row_len );
VDB_EXTERN rc_t CC VCursorCellDataDirect ( const VCursor *self, int64_t row_id,
    uint32_t col_idx, uint32_t *elem_bits, const void **base,
    uint32_t *boff, uint32_t *row_len );


/* Default
 *  give a default row value for cell
 *  TBD - document full cell data, not append
 *
 *  "col_idx" [ IN ] - index of column to be read, returned by "AddColumn"
 *
 *  "elem_bits" [ IN ] - stated element size in bits, required
 *  to be compatible with the actual element size
 *
 *  "buffer" [ IN ] and "boff" [ IN ] - compound pointer and offset
 *  to start of default row data where "boff" is in BITS
 *
 *  "row_len" [ IN ] - the number of elements in default row
 */
VDB_EXTERN rc_t CC VCursorDefault ( VCursor *self, uint32_t col_idx,
    bitsz_t elem_bits, const void *buffer, bitsz_t boff,
    uint64_t row_len );


/* Write
 *  append bit-aligned column data to cell
 *
 *  "col_idx" [ IN ] - index of column to be read, returned by "AddColumn"
 *
 *  "elem_bits" [ IN ] - stated element size in bits, required
 *  to be compatible with the actual element size
 *
 *  "buffer" [ IN ] and "boff" [ IN ] - compound pointer and offset
 *  to start of default row data where "boff" is in BITS
 *
 *  "count" [ IN ] - the number of elements to append
 */
VDB_EXTERN rc_t CC VCursorWrite ( VCursor *self, uint32_t col_idx,
    bitsz_t elem_bits, const void *buffer, bitsz_t boff,
    uint64_t count );


/* Commit
 *  commit changes made to cursor
 *  fails if row is open
 */
VDB_EXTERN rc_t CC VCursorCommit ( VCursor *self );


/* OpenParent
 *  duplicate reference to parent table
 *  NB - returned reference must be released
 */
VDB_EXTERN rc_t CC VCursorOpenParentRead ( const VCursor *self, struct VTable const **tbl );
VDB_EXTERN rc_t CC VCursorOpenParentUpdate ( VCursor *self, struct VTable **tbl );


/* GetUserData
 * SetUserData
 *  store/retrieve an opaque pointer to user data
 *
 *  "data" [ OUT ] - return parameter for getting data
 *  "data" [ IN ] - parameter for setting data
 *
 *  "destroy" [ IN, NULL OKAY ] - optional destructor param
 *  invoked from destructor of "self"
 */
VDB_EXTERN rc_t CC VCursorGetUserData ( const VCursor *self, void **data );
VDB_EXTERN rc_t CC VCursorSetUserData ( const VCursor *self,
    void *data, void ( CC * destroy ) ( void *data ) );


VDB_EXTERN rc_t CC VCursorLinkedCursorGet(const VCursor *cself,const char *tbl,VCursor const **curs);
VDB_EXTERN rc_t CC VCursorLinkedCursorSet(const VCursor *cself,const char *tbl,VCursor const *curs);



#ifdef __cplusplus
}
#endif

#endif /*  _h_vdb_cursor_ */
