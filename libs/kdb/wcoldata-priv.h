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

#ifndef _h_coldata_priv_
#define _h_coldata_priv_

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#ifndef _h_klib_checksum_
#include <klib/checksum.h>
#endif

#include <kfs/file.h>
#include <kfs/md5.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KMD5SumFmt;
typedef union KColumnPageMap KColumnPageMap;


/*--------------------------------------------------------------------------
 * KColumnData
 *  data fork
 *
 *  only handling append-mode today
 *
 *  kept 64-bit aligned
 */
typedef struct KColumnData KColumnData;
struct KColumnData
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
#define KColumnDataDefaultPageSize( reuse_pages ) \
    ( ( reuse_pages ) ? 4096 : 1 )

/* Create
 */
rc_t KColumnDataCreate ( KColumnData *self, KDirectory *dir,
    KMD5SumFmt *md5, KCreateMode mode, uint64_t eof, size_t pgsize );

/* Open
 */
rc_t KColumnDataOpenRead ( KColumnData *self,
    const KDirectory *dir, uint64_t eof, size_t pgsize );
rc_t KColumnDataOpenUpdate ( KColumnData *self, KDirectory *dir,
    KMD5SumFmt *md5, uint64_t eof, size_t pgsize );

/* Whack
 */
rc_t KColumnDataWhack ( KColumnData *self );

/* Read
 *  reads from the data fork using a blob map
 */
rc_t KColumnDataRead ( const KColumnData *self, const KColumnPageMap *pm,
    size_t offset, void *buffer, size_t bsize, size_t *num_read );

/* Write
 *  writes to the data fork using a blob map
 */
rc_t  KColumnDataWrite ( KColumnData *self, KColumnPageMap *pm,
    size_t offset, const void *buffer, size_t bytes, size_t *num_writ );

/* Commit
 *  keeps changes indicated by page map and blob size
 */
rc_t KColumnDataCommit ( KColumnData *self,
    const KColumnPageMap *pm, size_t bytes );

/* CommitDone
 *  finalizes a commit
 */
rc_t KColumnDataCommitDone ( KColumnData * self );

/* Free
 *  frees pages from a map
 */
rc_t KColumnDataFree ( KColumnData *self,
    const KColumnPageMap *pm, size_t bytes );


/*--------------------------------------------------------------------------
 * KColumnPageMap
 *  map of pages involved in column blob
 */
union KColumnPageMap
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
rc_t KColumnPageMapCreate (  KColumnPageMap *self, KColumnData *cd );

/* Open
 *  opens an blob by raw page id and size
 *
 *  "pm" [ OUT ] - modifiable parameter for blob page map
 *
 *  "pg" [ IN ] and "sz" [ IN ] - identifies pages of data fork included
 *  within the blob.
 */
rc_t KColumnPageMapOpen ( KColumnPageMap *pm,
    KColumnData *cd, uint64_t pg, size_t sz );

/* Whack
 *  disposes of memory in the case of a page array
 */
void KColumnPageMapWhack ( KColumnPageMap *self, const KColumnData *cd );

/* Id
 *  captures id of initial page
 */
rc_t KColumnPageMapId ( const KColumnPageMap *self,
    const KColumnData *cd, uint64_t *pg );


#ifdef __cplusplus
}
#endif

#endif /* _h_coldata_priv_ */
