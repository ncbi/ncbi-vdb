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

#include "coldata.h"

#include <kfs/directory.h>
#include <klib/checksum.h>

#include <kfs/file.h>
#include <kfs/md5.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KMD5SumFmt;
typedef union KWColumnPageMap KWColumnPageMap;


/*--------------------------------------------------------------------------
 * KWColumnData
 *  data fork
 *
 *  only handling append-mode today
 *
 *  kept 64-bit aligned
 */
typedef struct KWColumnData KWColumnData;
struct KWColumnData
{
    /* cached end of data fork */
    uint64_t eof;

    /* data fork itself */
    KFile *f;
    KMD5File *fmd5;

    /* page size */
    size_t pgsize;

};

/* DefaultPageSize
 *  static method
 */
#define KWColumnDataDefaultPageSize( reuse_pages ) \
    ( ( reuse_pages ) ? 4096 : 1 )

/* Create
 */
rc_t KWColumnDataCreate ( KWColumnData *self, KDirectory *dir,
    KMD5SumFmt *md5, KCreateMode mode, uint64_t eof, size_t pgsize );

/* Open
 */
rc_t KWColumnDataOpenRead ( KWColumnData *self,
    const KDirectory *dir, uint64_t eof, size_t pgsize );
rc_t KWColumnDataOpenUpdate ( KWColumnData *self, KDirectory *dir,
    KMD5SumFmt *md5, uint64_t eof, size_t pgsize );

/* Whack
 */
rc_t KWColumnDataWhack ( KWColumnData *self );

/* Write
 *  writes to the data fork using a blob map
 */
rc_t  KWColumnDataWrite ( KWColumnData *self, KWColumnPageMap *pm,
    size_t offset, const void *buffer, size_t bytes, size_t *num_writ );

/* Commit
 *  keeps changes indicated by page map and blob size
 */
rc_t KWColumnDataCommit ( KWColumnData *self,
    const KWColumnPageMap *pm, size_t bytes );

/* CommitDone
 *  finalizes a commit
 */
rc_t KWColumnDataCommitDone ( KWColumnData * self );

/* Free
 *  frees pages from a map
 */
rc_t KWColumnDataFree ( KWColumnData *self,
    const KWColumnPageMap *pm, size_t bytes );


/*--------------------------------------------------------------------------
 * KWColumnPageMap
 *  map of pages involved in column blob
 */
union KWColumnPageMap
{
    /* for non-paged data forks, a single page id
       describes the start of the blob, where the
       remainder is sequential */
    uint64_t pg;
};

/* Create
 *  creates a new page map using the first available page id
 *  obtains first free data fork page
 */
rc_t KWColumnPageMapCreate (  KWColumnPageMap *self, KWColumnData *cd );

/* Open
 *  opens an blob by raw page id and size
 *
 *  "pm" [ OUT ] - modifiable parameter for blob page map
 *
 *  "pg" [ IN ] and "sz" [ IN ] - identifies pages of data fork included
 *  within the blob.
 */
rc_t KWColumnPageMapOpen ( KWColumnPageMap *pm,
    KWColumnData *cd, uint64_t pg, size_t sz );

/* Id
 *  captures id of initial page
 */
rc_t KWColumnPageMapId ( const KWColumnPageMap *self,
    const KWColumnData *cd, uint64_t *pg );


#ifdef __cplusplus
}
#endif

