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
#include <kfs/file.h>
#include <kfs/md5.h>

#include "colfmt.h"

#include <klib/container.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KColWIdxBlock;
struct KColBlockLocInfo;


/*--------------------------------------------------------------------------
 * KWColumnIdx0
 *  level 0 index - event journaling
 */
typedef struct KWColumnIdx0 KWColumnIdx0;
struct KWColumnIdx0
{
    /* always append */
    uint64_t eof;

    /* the in-core indices */
    BSTree bst;
    size_t count;

    /* the on-disk indices */
    KFile *f;
    KMD5File *fmd5;
};


/* Create
 */
rc_t KWColumnIdx0Create_v1 ( KWColumnIdx0 *self, KDirectory *dir,
    struct KMD5SumFmt *md5, KCreateMode mode, bool bswap );
rc_t KWColumnIdx0Create ( KWColumnIdx0 *self, KDirectory *dir,
    uint32_t count, struct KMD5SumFmt *md5, KCreateMode mode, bool bswap );

/* Open
 */
rc_t KWColumnIdx0OpenRead_v1 ( KWColumnIdx0 *self,
    const KDirectory *dir, bool bswap );
rc_t KWColumnIdx0OpenRead ( KWColumnIdx0 *self,
    const KDirectory *dir, uint32_t count, bool bswap );

/* Whack
 */
void KWColumnIdx0Whack ( KWColumnIdx0 *self );

/* IdRange
 *  returns range of ids contained within
 */
bool KWColumnIdx0IdRange ( const KWColumnIdx0 *self,
    int64_t *first, int64_t *upper );

/* FindFirstRowId
 */
rc_t KWColumnIdx0FindFirstRowId ( const KWColumnIdx0 * self,
    int64_t * found, int64_t start );

/* LocateBlob
 *  locate an existing blob
 */
rc_t KWColumnIdx0LocateBlob ( const KWColumnIdx0 *self,
    KColBlobLoc *loc, int64_t first, int64_t upper );

/* Commit
 *  records an index location for addition or removal
 *  returns any prior value for rollback
 */
rc_t KWColumnIdx0Commit ( KWColumnIdx0 *self,
    const KColBlobLoc *loc, KColBlobLoc *prior, bool bswap );
rc_t KWColumnIdx0CommitDone ( KWColumnIdx0 *self );

/* Revert
 *  reverses effect of commit
 */
void KWColumnIdx0Revert ( KWColumnIdx0 *self,
    const KColBlobLoc *loc, const KColBlobLoc *prior );

/* DefineBlocks
 *  scans existing blob locators
 *  invokes handler with ranges of ids from similar blob entries
 */
rc_t KWColumnIdx0DefineBlocks ( const KWColumnIdx0 *self,
    rc_t ( * handle_range ) ( struct KColBlockLocInfo const *info, void *data ),
    void *data, size_t pgsize );

/* TranscribeBlocks
 *  writes all blocks within a range
 */
void KWColumnIdx0TranscribeBlocks ( const KWColumnIdx0 *self,
    int64_t first, int64_t upper, struct KColWIdxBlock *iblk );

/* Truncate
 *  whacks bt contents and truncates file
 */
void KWColumnIdx0Truncate ( KWColumnIdx0 *self );


#ifdef __cplusplus
}
#endif
