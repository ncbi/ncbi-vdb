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

#ifndef _h_colidx_priv_
#define _h_colidx_priv_

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#include <kfs/md5.h>

#ifndef _h_colfmt_priv_
#include "colfmt-priv.h"
#endif

#ifndef _h_colidx0_priv_
#include "wcolidx0-priv.h"
#endif

#ifndef _h_colidx1_priv_
#include "wcolidx1-priv.h"
#endif

#ifndef _h_colidx2_priv_
#include "wcolidx2-priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KMD5SumFmt;


/*--------------------------------------------------------------------------
 * KColumnIdx
 *  the index fork
 */
typedef struct KColumnIdx KColumnIdx;
struct KColumnIdx
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
    KColumnIdx0 idx0;

    /* level 1 index */
    KColumnIdx1 idx1;

    /* level 2 index */
    KColumnIdx2 idx2;

    /* commit counter */
    uint32_t commit_count;
};

/* Create
 */
rc_t KColumnIdxCreate ( KColumnIdx *self,
    KDirectory *dir, struct KMD5SumFmt *md5, KCreateMode mode,
    uint64_t *data_eof, size_t pgsize, int32_t checksum );

/* Open
 */
rc_t KColumnIdxOpenRead ( KColumnIdx *self, const KDirectory *dir,
			  uint64_t *data_eof, size_t *pgsize, 
			  int32_t *checksum );
rc_t KColumnIdxOpenUpdate ( KColumnIdx *self, KDirectory *dir,
    KMD5SumFmt *md5, uint64_t *data_eof, size_t *pgsize,
    int32_t *checksum );

/* Whack
 */
rc_t KColumnIdxWhack ( KColumnIdx *self, 
    uint64_t data_eof, size_t pgsize, int32_t checksum );

/* Version
 */
rc_t KColumnIdxVersion ( const KColumnIdx *self, uint32_t *version );
#define KColumnIdxVersion( self, version ) \
    KColumnIdx1Version ( & ( self ) -> idx1, version )

/* ByteOrder
 */
rc_t KColumnIdxByteOrder ( const KColumnIdx *self, bool *reversed );
#define KColumnIdxByteOrder( self, reversed ) \
    KColumnIdx1ByteOrder ( & ( self ) -> idx1, reversed )

/* IdRange
 *  returns range of ids contained within
 */
rc_t KColumnIdxIdRange ( const KColumnIdx *self,
    int64_t *first, int64_t *last );

/* LocateBlob
 *  locate an existing blob
 */
rc_t KColumnIdxLocateBlob ( const KColumnIdx *self,
    KColBlobLoc *loc, int64_t first, int64_t last );

/* Commit
 *  writes a new blob location to idx0
 *  updates idx1 with header information
 *  this should be the final step in committing a write operation
 */
rc_t KColumnIdxCommit ( KColumnIdx *self, struct KMD5SumFmt *md5,
    const KColBlobLoc *loc, uint32_t commit_freq,
    uint64_t data_eof, size_t pgsize, int32_t checksum );

/* CommitDone
 *  finalizes a commit
 */
rc_t KColumnIdxCommitDone ( KColumnIdx * self );

/* Reindex
 */
rc_t KColumnIdxReindex ( KColumnIdx *self, struct KMD5SumFmt *md5,
    uint32_t commit_freq, uint64_t data_eof, size_t pgsize, int32_t checksum );


#ifdef __cplusplus
}
#endif

#endif /* _h_colidx_priv_ */
