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

#ifndef _hpp_vdb_cursor_
#define _hpp_vdb_cursor_

#ifndef _h_vdb_cursor_
#include <vdb/cursor.h>
#endif


/*--------------------------------------------------------------------------
 * VCursor
 *  a row cursor onto a VTable
 */
struct VCursor
{
    /* AddRef
     * Release
     *  all objects are reference counted
     *  NULL references are ignored
     */
    inline rc_t AddRef () const throw()
    { return VCursorAddRef ( this ); }

    inline rc_t Release () const throw()
    { return VCursorRelease ( this ); }


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
    inline rc_t AddColumn ( uint32_t *idx,
        const char *name, ... ) const throw()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VCursorVAddColumn ( this, idx, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t AddColumn ( uint32_t *idx,
        const char *name, va_list args ) const throw()
    { return VCursorVAddColumn ( this, idx, name, args ); }


    /* GetColumnIdx
     *  retrieve column index by name spec
     *
     *  "idx" [ OUT ] - return parameter for column index
     *
     *  "name" [ IN ] - NUL terminated column name spec.
     */
    inline rc_t GetColumnIdx ( uint32_t *idx,
        const char *name, ... ) const throw()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VCursorVGetColumnIdx ( this, idx, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t GetColumnIdx ( uint32_t *idx,
        const char *name, va_list args ) const throw()
    { return  VCursorVGetColumnIdx ( this, idx, name, args ); }


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
    inline rc_t Datatype ( uint32_t idx, VTypedecl *type, 
        VTypedesc *desc ) const throw()
    { return VCursorDatatype ( this, idx, type, desc ); }


    /* IdRange
     *  returns id range for column
     *
     *  "idx" [ IN, OPTIONAL ] - single column index or
     *  zero to indicate the range for all columns in cursor
     *
     *  "id" [ IN ] - page containing this row id is target
     *
     *  "first" [ OUT, NULL OKAY ] and "last" [ OUT, NULL OKAY ] -
     *  id range is returned in these output parameters, where
     *  at least ONE must be NOT-NULL
     */
    inline rc_t IdRange ( int64_t *first, uint64_t *count ) const throw()
    { return VCursorIdRange ( this, 0, first, count ); }

    inline rc_t IdRange ( uint32_t idx, int64_t *first,
        uint64_t *count ) const throw()
    { return VCursorIdRange ( this, idx, first, count ); }


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
    inline rc_t Open () const throw()
    { return VCursorOpen ( this ); }


    /* RowId
     *  report current row id
     * SetRowId
     *  seek to given row id
     */
    inline rc_t RowId ( int64_t *row_id ) const throw()
    { return VCursorRowId ( this, row_id ); }

    inline rc_t SetRowId ( int64_t row_id ) const throw()
    { return VCursorSetRowId ( this, row_id ); }


    /* OpenRow
     *  open currently closed row indicated by row id
     */
    inline rc_t OpenRow () const throw()
    { return VCursorOpenRow ( this ); }


    /* CommitRow
     *  commit row after writing
     *  prevents further writes
     */
    inline rc_t CommitRow () throw()
    { return VCursorCommitRow ( this ); }


    /* CloseRow
     *  balances OpenRow message
     *  if there are uncommitted modifications,
     *  discard all changes. otherwise,
     *  advance to next row
     */
    inline rc_t CloseRow () const throw()
    { return VCursorCloseRow ( this ); }


    /* FlushPage
     *  forces flush of all buffered page data
     *  fails if row is open
     *
     *  pages are normally auto-committed based upon
     *  size and column affinity
     */
    inline rc_t FlushPage () throw()
    { return VCursorFlushPage ( this ); }


    /* Read
     *  read entire single row of byte-aligned data into a buffer
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
    inline rc_t Read ( uint32_t col_idx, uint32_t elem_bits,
        void *buffer, uint32_t blen, uint32_t *row_len ) const throw()
    {
        return VCursorRead ( this, col_idx, elem_bits, buffer, blen,  row_len );
    }
    inline rc_t Read ( int64_t row_id, uint32_t col_idx, uint32_t elem_bits,
        void *buffer, uint32_t blen, uint32_t *row_len ) const throw()
    {
        return VCursorReadDirect ( this, row_id, col_idx, elem_bits, buffer, blen, row_len );
    }


    /* ReadBits
     *  read single row of potentially bit-aligned column data into a buffer
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
    inline rc_t ReadBits ( const VCursor *self, uint32_t col_idx,
        uint32_t elem_bits, uint32_t start, void *buffer, uint32_t boff,
        uint32_t blen, uint32_t *num_read, uint32_t *remaining ) const throw()
    {
        return VCursorReadBits ( this, col_idx, elem_bits, start,
            buffer, boff, blen, num_read, remaining );
    }
    inline rc_t ReadBits ( const VCursor *self, int64_t row_id, uint32_t col_idx,
        uint32_t elem_bits, uint32_t start, void *buffer, uint32_t boff,
        uint32_t blen, uint32_t *num_read, uint32_t *remaining ) const throw()
    {
        return VCursorReadBitsDirect ( this, row_id, col_idx, elem_bits, 
            start, buffer, boff, blen, num_read, remaining );
    }


    /* CellData
     *  access pointer to single cell of potentially bit-aligned column data
     *  can fail if row is dynamically generated
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
    inline rc_t CellData ( uint32_t col_idx, uint32_t *elem_bits,
        const void **base, uint32_t *boff, uint32_t *row_len ) const throw()
    {
        return VCursorCellData ( this, col_idx, elem_bits, base, boff, row_len );
    }
    inline rc_t CellData ( int64_t row_id, uint32_t col_idx, uint32_t *elem_bits,
        const void **base, uint32_t *boff, uint32_t *row_len ) const throw()
    {
        return VCursorCellDataDirect ( this, row_id, col_idx, elem_bits,
            base, boff, row_len );
    }

    /* Default
     *  give a default row value for column
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
    inline rc_t Default ( uint32_t col_idx, uint32_t elem_bits,
        const void *buffer, uint32_t boff, uint32_t row_len ) throw()
    {
        return VCursorDefault ( this, col_idx, elem_bits, buffer, boff, row_len );
    }


    /* Write
     *  append bit-aligned column data to row
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
    inline rc_t Write ( uint32_t col_idx, uint32_t elem_bits,
        const void *buffer, uint32_t boff, uint32_t count ) throw()
    {
        return VCursorWrite ( this, col_idx, elem_bits, buffer, boff, count );
    }


    /* Commit
     *  commit changes made to cursor
     *  fails if row is open
     */
    inline rc_t Commit () throw()
    { return VCursorCommit ( this ); }


    /* OpenParent
     *  duplicate reference to parent table
     *  NB - returned reference must be released
     */
    inline rc_t OpenParent ( const VTable **tbl ) const throw()
    { return VCursorOpenParentRead ( this, tbl ); }

    inline rc_t OpenParent ( VTable **tbl ) throw()
    { return VCursorOpenParentUpdate ( this, tbl ); }


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
    inline rc_t GetUserData ( void **data ) const throw()
    { return VCursorGetUserData ( this, data ); }

    inline rc_t SetUserData ( void *data,
        void ( CC * destroy ) ( void *data ) = 0 ) const throw()
    { return VCursorSetUserData ( this, data, destroy ); }


private:
    VCursor ();
    ~ VCursor ();
    VCursor ( const VCursor& );
    VCursor &operator = ( const VCursor& );
};

#endif // _hpp_vdb_cursor_
