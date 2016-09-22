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

#ifndef _h_kfc_refcount_
#include <kfc/refcount.h>
#endif

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
KDB_EXTERN KRowSet * CC KTableMakeRowSet ( struct KTable const * self, ctx_t ctx );


/* Duplicate
 * Release
 *  ignores NULL references
 */
static __inline__
KRowSet * KRowSetDuplicate ( const KRowSet * self, ctx_t ctx, caps_t rm )
{
    return ( KRowSet * ) KRefcountDuplicate_v1 ( TO_REFCOUNT_V1 ( self ), ctx, rm );
}

static __inline__
void KRowSetRelease ( const KRowSet * self, ctx_t ctx )
{
    KRefcountRelease_v1 ( TO_REFCOUNT_V1 ( self ), ctx );
}

/* AddRowId
 *  add a single row to set
 *
 *  "row_id" [ IN ] - row-id to be added
 */
KDB_EXTERN void CC KRowSetAddRowId ( KRowSet * self, ctx_t ctx, int64_t row_id );


/* AddRowIdRange
 *  adds row-ids within specified range
 *
 *  "row_id" [ IN ] and "count" [ IN ] - range of row-ids to be added
 */
KDB_EXTERN void CC KRowSetAddRowIdRange ( KRowSet * self, ctx_t ctx, int64_t row_id, uint64_t count );

/* needed (? maybe ?)
 *  add an array of row-ids in random order
 */


/* GetNumRowIds
 *  return the number of elements in set
 */
KDB_EXTERN uint64_t CC KRowSetGetNumRowIds ( const KRowSet * self, ctx_t ctx );

/* HasRowId
 *  checks if element is present in set
 */
KDB_EXTERN bool CC KRowSetHasRowId ( const KRowSet * self, ctx_t ctx, int64_t row_id );

/* Visit
 *  execute a function on each row-id in set
 */
KDB_EXTERN void CC KRowSetVisit ( const KRowSet * self, ctx_t ctx,
    void ( CC * f ) ( ctx_t ctx, int64_t row_id, void * data ), void * data );

/* Intersect
 *  performs an intersection between two sets and returns the result
 */
KDB_EXTERN KRowSet * CC KRowSetIntersect ( ctx_t ctx, const KRowSet * a, const KRowSet * b );

/* Union
 *  performs a union between two sets and returns the result
 */
KDB_EXTERN KRowSet * CC KRowSetUnion ( ctx_t ctx, const KRowSet * a, const KRowSet * b );

/*--------------------------------------------------------------------------
 * KRowSetIterator
 *  an iterator over rowset
 */
typedef struct KRowSetIterator KRowSetIterator;


/* MakeIterator
 *  create an iterator on set
 *  initially set to first row-id in set
 *  iterator always goes over rows in sorted order
 *  after creating an iterator, any modifications of rowset will result in undefined behavior
 *    when accessing the iterator (except IteratorRelease function)
 */
KDB_EXTERN KRowSetIterator * CC KRowSetMakeIterator ( const KRowSet * self, ctx_t ctx );


/* Duplicate
 * Release
 *  ignores NULL references
 */
static __inline__
KRowSet * KRowSetIteratorDuplicate ( const KRowSetIterator * self, ctx_t ctx, caps_t rm )
{
    return ( KRowSet * ) KRefcountDuplicate_v1 ( TO_REFCOUNT_V1 ( self ), ctx, rm );
}

static __inline__
void KRowSetIteratorRelease ( const KRowSetIterator * self, ctx_t ctx )
{
    KRefcountRelease_v1 ( TO_REFCOUNT_V1 ( self ), ctx );
}

/* Next
 *  advance iterator to next row-id

 *  advance to first row-id on initial invocation
 *  advance to next row-id subsequently
 *  returns false if no more row-ids are available.
 */
KDB_EXTERN bool CC KRowSetIteratorNext ( KRowSetIterator * self, ctx_t ctx );

/*IsValid
 * check if iterator points to a valid row
 *
 * returns false when iterator points somewhere outside of a row set
 */
KDB_EXTERN bool CC KRowSetIteratorIsValid ( const KRowSetIterator * self );

/* GetRowId
 *  report current row id
 */
KDB_EXTERN int64_t CC KRowSetIteratorGetRowId ( const KRowSetIterator * self, ctx_t ctx );


#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_rowset_ */
