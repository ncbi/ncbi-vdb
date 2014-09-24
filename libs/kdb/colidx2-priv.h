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

#ifndef _h_colidx2_priv_
#define _h_colidx2_priv_

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#ifndef _h_colfmt_priv_
#include "colfmt-priv.h"
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#include <klib/data-buffer.h>
#include "idxblk-priv.h"

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KColumnIdx2
 *  level 2 index
 */

typedef struct KColumnIdx2BlockCache
{
    /* single page cache */
    void *block;
    int64_t start_id;
    size_t count;
    KColIdxBlock iblk;
} KColumnIdx2BlockCache;

typedef struct KColumnIdx2 KColumnIdx2;
struct KColumnIdx2
{
    /* for adding new blocks */
    uint64_t eof;

    /* idx2 itself */
    struct KFile const *f;

    /* full caching mechanism */
    KDataBuffer cstorage;
    uint32_t	last;
};

/* Open
 */
rc_t KColumnIdx2OpenRead ( KColumnIdx2 *self,
    const KDirectory *dir, uint64_t eof );

/* Whack
 */
rc_t KColumnIdx2Whack ( KColumnIdx2 *self );

/* LocateBlob
 *  locate an existing blob
 */
rc_t KColumnIdx2LocateBlob ( const KColumnIdx2 *self,
    KColBlobLoc *loc, const KColBlockLoc *bloc,
    int64_t first, int64_t upper, bool bswap );


#ifdef __cplusplus
}
#endif

#endif /* _h_colidx2_priv_ */
