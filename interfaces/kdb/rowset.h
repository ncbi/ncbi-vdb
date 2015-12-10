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

#ifndef _h_kdb_rowset_
#define _h_kdb_rowset_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KTable;


/*--------------------------------------------------------------------------
 * KRowSet
 *  a subset of matching rows
 */
typedef struct KRowSet KRowSet;


/* MakeRowSet
 *  may add others...
 */
KDB_EXTERN rc_t CC KTableMakeRowSet ( struct KTable const * self, KRowSet ** rowset );


/* AddRef
 * Release
 *  ignores NULL references
 */
KDB_EXTERN rc_t CC KRowSetAddRef ( const KRowSet * self );
KDB_EXTERN rc_t CC KRowSetRelease ( const KRowSet * self );


/* AddRowId
 *  add a single row to set
 *
 *  "row_id" [ IN ] - row-id to be added
 *
 *  "optional_inserted" [ OUT, NULL OKAY ] - returns true if row-id
 *  was actually added, false otherwise.
 */
KDB_EXTERN rc_t CC KRowSetAddRowId ( KRowSet * self, int64_t row_id,
    bool * optional_inserted );


/* AddRowIdRange
 *  adds row-ids within specified range
 *
 *  "row_id" [ IN ] and "count" [ IN ] - range of row-ids to be added
 *
 *  "optional_inserted" [ OUT, NULL OKAY ] - returns the number of ids
 *  actually added. this can be from 0.."count" depending upon whether the
 *  row-id(s) already exist.
 */
KDB_EXTERN rc_t CC KRowSetAddRowIdRange ( KRowSet * self, int64_t row_id,
    uint64_t count, uint64_t * optional_inserted );


/* GetNumRowIds
 *  return the number of elements in set
 */
KDB_EXTERN rc_t CC KRowSetGetNumRowIds ( const KRowSet * self, uint64_t * num_rows );


/* Visit
 *  execute a function on each row-id in set
 */
KDB_EXTERN rc_t CC KRowSetVisit ( const KRowSet * self, bool reverse,
    void ( CC * f ) ( int64_t row_id, void * data ), void * data );


/*--------------------------------------------------------------------------
 * KRowSetIterator
 *  an iterator over rowset
 */
typedef struct KRowSetIterator KRowSetIterator;


/* MakeIterator
 *  create an iterator on set
 *  initially set to first row-id in set
 */
KDB_EXTERN rc_t CC KRowSetMakeIterator ( const KRowSet * self, KRowSetIterator ** iter );


/* AddRef
 * Release
 *  ignores NULL references
 */
KDB_EXTERN rc_t CC KRowSetIteratorAddRef ( const KRowSetIterator * self );
KDB_EXTERN rc_t CC KRowSetIteratorRelease ( const KRowSetIterator * self );


/* First
 * Last
 *  rewind iterator to first or last item in set
 */
KDB_EXTERN rc_t CC KRowSetIteratorFirst ( KRowSetIterator * self );
KDB_EXTERN rc_t CC KRowSetIteratorLast ( KRowSetIterator * self );


/* Next
 *  advance iterator to next row-id

 *  advance to first row-id on initial invocation
 *  advance to next row-id subsequently
 *  returns rcDone if no more row-ids are available.
 */
KDB_EXTERN rc_t CC KRowSetIteratorNext ( KRowSetIterator * self );


/* Prev
 *  advance iterator to previous row-id

 *  advance to last row-id on initial invocation
 *  advance to prev row-id subsequently
 *  returns rcDone if no more row-ids are available.
 */
KDB_EXTERN rc_t CC KRowSetIteratorPrev ( KRowSetIterator * self );


/*IsValid
 * check if iterator points to a valid row
 *
 * returns false when iterator points somewhere outside of a row set
 */
KDB_EXTERN bool CC KRowSetIteratorIsValid ( const KRowSetIterator * self );

/* RowId
 *  report current row id
 */
KDB_EXTERN rc_t CC KRowSetIteratorRowId ( const KRowSetIterator * self, int64_t * row_id );


#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_rowset_ */
