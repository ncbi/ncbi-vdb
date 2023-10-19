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

#pragma once

#include <kfs/directory.h>

#include <kfs/md5.h>

#include "colfmt.h"

#include "wcolidx0.h"
#include "wcolidx1.h"
#include "wcolidx2.h"

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KMD5SumFmt;


/*--------------------------------------------------------------------------
 * KWColumnIdx
 *  the index fork
 */
typedef struct KWColumnIdx KWColumnIdx;
struct KWColumnIdx
{
    /* first active id within db
       and first id on upper limit
       i.e. outside of db such that
       id_upper - id_first == num_ids */
    int64_t id_first;
    int64_t id_upper;

/*     struct KFile * f; */
    uint32_t vers;

    /* level 0 index */
    KWColumnIdx0 idx0;

    /* level 1 index */
    KWColumnIdx1 idx1;

    /* level 2 index */
    KWColumnIdx2 idx2;

    /* commit counter */
    uint32_t commit_count;
};

/* Create
 */
rc_t KWColumnIdxCreate ( KWColumnIdx *self,
    KDirectory *dir, struct KMD5SumFmt *md5, KCreateMode mode,
    uint64_t *data_eof, size_t pgsize, int32_t checksum );

/* Open
 */
rc_t KWColumnIdxOpenRead ( KWColumnIdx *self, const KDirectory *dir,
			  uint64_t *data_eof, size_t *pgsize,
			  int32_t *checksum );
rc_t KWColumnIdxOpenUpdate ( KWColumnIdx *self, KDirectory *dir,
    KMD5SumFmt *md5, uint64_t *data_eof, size_t *pgsize,
    int32_t *checksum );

/* Whack
 */
rc_t KWColumnIdxWhack ( KWColumnIdx *self,
    uint64_t data_eof, size_t pgsize, int32_t checksum );

/* Version
 */
rc_t KWColumnIdxVersion ( const KWColumnIdx *self, uint32_t *version );
#define KWColumnIdxVersion( self, version ) \
    KWColumnIdx1Version ( & ( self ) -> idx1, version )

/* ByteOrder
 */
rc_t KWColumnIdxByteOrder ( const KWColumnIdx *self, bool *reversed );
#define KWColumnIdxByteOrder( self, reversed ) \
    KWColumnIdx1ByteOrder ( & ( self ) -> idx1, reversed )

/* IdRange
 *  returns range of ids contained within
 */
rc_t KWColumnIdxIdRange ( const KWColumnIdx *self,
    int64_t *first, int64_t *last );

/* FindFirstRowId
 */
rc_t KWColumnIdxFindFirstRowId ( const KWColumnIdx * self,
    int64_t * found, int64_t start );

/* LocateBlob
 *  locate an existing blob
 */
rc_t KWColumnIdxLocateBlob ( const KWColumnIdx *self,
    KColBlobLoc *loc, int64_t first, int64_t last );

/* Commit
 *  writes a new blob location to idx0
 *  updates idx1 with header information
 *  this should be the final step in committing a write operation
 */
rc_t KWColumnIdxCommit ( KWColumnIdx *self, struct KMD5SumFmt *md5,
    const KColBlobLoc *loc, uint32_t commit_freq,
    uint64_t data_eof, size_t pgsize, int32_t checksum );

/* CommitDone
 *  finalizes a commit
 */
rc_t KWColumnIdxCommitDone ( KWColumnIdx * self );

/* Reindex
 */
rc_t KWColumnIdxReindex ( KWColumnIdx *self, struct KMD5SumFmt *md5,
    uint32_t commit_freq, uint64_t data_eof, size_t pgsize, int32_t checksum );


#ifdef __cplusplus
}
#endif
