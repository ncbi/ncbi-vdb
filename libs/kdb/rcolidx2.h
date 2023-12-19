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

#include "colfmt.h"

#include <klib/container.h>
#include <klib/data-buffer.h>

#include "ridxblk.h"

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KRColumnIdx2
 *  level 2 index
 */

typedef struct KRColumnIdx2BlockCache
{
    /* single page cache */
    void *block;
    int64_t start_id;
    size_t count;
    KColIdxBlock iblk;
} KRColumnIdx2BlockCache;

typedef struct KRColumnIdx2 KRColumnIdx2;
struct KRColumnIdx2
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
rc_t KRColumnIdx2OpenRead ( KRColumnIdx2 *self,
    const KDirectory *dir, uint64_t eof );

/* Whack
 */
rc_t KRColumnIdx2Whack ( KRColumnIdx2 *self );

/* LocateBlob
 *  locate an existing blob
 */
rc_t KRColumnIdx2LocateBlob ( const KRColumnIdx2 *self,
    KColBlobLoc *loc, const KColBlockLoc *bloc,
    int64_t first, int64_t upper, bool bswap );


#ifdef __cplusplus
}
#endif

