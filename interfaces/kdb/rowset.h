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
 * KRowSet
 *  a subset of matching rows
 */
typedef struct KRowSet KRowSet;
    
KDB_EXTERN rc_t CC KCreateRowSet ( KRowSet ** self );

/* AddRef
 * Release
 *  ignores NULL references
 */
KDB_EXTERN rc_t CC KRowSetAddRef ( const KRowSet * self );
KDB_EXTERN rc_t CC KRowSetRelease ( const KRowSet * self );
    
KDB_EXTERN rc_t CC KRowSetInsertRowRange ( KRowSet * self, int64_t row_id, uint64_t count, uint64_t * inserted );

#define KRowSetInsertRow( SELF, ROW_ID ) KRowSetInsertRowRange ( (SELF), (ROW_ID), 1, NULL )

KDB_EXTERN rc_t CC KRowSetGetNumRows ( const KRowSet * self, uint64_t * num_rows );

KDB_EXTERN rc_t CC KRowSetWalkRows ( const KRowSet * self, bool reverse,
		void ( CC * f ) ( int64_t row_id, void * data ), void * data );

/*--------------------------------------------------------------------------
 * KRowSetIterator
 *  an iterator over rowset
 */
typedef struct KRowSetIterator KRowSetIterator;

KDB_EXTERN rc_t CC KRowSetCreateIterator ( const KRowSet * self, bool reverse, KRowSetIterator ** iter );

KDB_EXTERN rc_t CC KRowSetIteratorAddRef ( const KRowSetIterator * iter );
KDB_EXTERN rc_t CC KRowSetIteratorRelease ( const KRowSetIterator * iter );

KDB_EXTERN bool CC KRowSetIteratorNext ( KRowSetIterator * iter );

KDB_EXTERN int64_t CC KRowSetIteratorRowId ( const KRowSetIterator * iter );
    
#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_rowset_ */
