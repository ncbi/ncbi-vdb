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

#ifndef _h_colidx1_priv_
#define _h_colidx1_priv_

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#include <kfs/md5.h>

#ifndef _h_colfmt_priv_
#include "colfmt-priv.h"
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KColIdxBlock;
struct KColBlockLocInfo;


/*--------------------------------------------------------------------------
 * KColumnIdx1
 *  level 1 index
 */
typedef struct KColumnIdx1 KColumnIdx1;
struct KColumnIdx1
{
    /* tree of level-2 block locators */
    BSTree bst;
    struct KFile *f;
    struct KFile *fidx;
    struct KMD5File *fmd5;
    struct KMD5File *fidxmd5;

    uint32_t count;
    uint32_t vers;
    /* might have to switch to bit flags if more needed */
    bool bswap;
    bool use_md5;
#define CONVERT_ON_SAVE_NONE	0
#define CONVERT_ON_SAVE_V1	1
    uint8_t convert;
    uint8_t align [ sizeof ( size_t ) - 3 ];
};

/* Create
 */
rc_t KColumnIdx1Create ( KColumnIdx1 *self,
    KDirectory *dir, KMD5SumFmt *md5, KCreateMode mode,
    uint64_t *data_eof, uint32_t *idx0_count, uint64_t *idx2_eof,
    size_t pgsize, int32_t checksum );

/* Open
 */
rc_t KColumnIdx1OpenRead ( KColumnIdx1 *self, const KDirectory *dir,
    uint64_t *data_eof, uint32_t *idx0_count, uint64_t *idx2_eof,
    size_t *pgsize, int32_t *checksum );
rc_t KColumnIdx1OpenUpdate ( KColumnIdx1 *self, KDirectory *dir,
    KMD5SumFmt *md5, uint64_t *data_eof, uint32_t *idx0_count,
    uint64_t *idx2_eof, size_t *pgsize, int32_t *checksum );

/* Whack
 */
rc_t KColumnIdx1Whack ( KColumnIdx1 *self );

/* Version
 */
rc_t KColumnIdx1Version ( const KColumnIdx1 *self, uint32_t *version );
#define KColumnIdx1Version( self, version ) \
    ( * ( version ) = ( uint32_t ) ( self ) -> vers, 0 )

/* ByteOrder
 */
rc_t KColumnIdx1ByteOrder ( const KColumnIdx1 *self, bool *reversed );
#define KColumnIdx1ByteOrder( self, reversed ) \
    ( * ( reversed ) = ( self ) -> bswap, 0 )

/* IdRange
 *  returns range of ids contained within
 */
bool KColumnIdx1IdRange ( const KColumnIdx1 *self,
    int64_t *first, int64_t *upper );

/* LocateBlock
 *  locates an idx2 block by range
 */
rc_t KColumnIdx1LocateBlock ( const KColumnIdx1 *self,
    KColBlockLoc *bloc, int64_t first, int64_t upper );

/* WriteHeader
 */
rc_t KColumnIdx1WriteHeader ( KColumnIdx1 *self,
    uint64_t data_eof, uint32_t idx0_count, uint64_t idx2_eof,
    size_t pgsize, int32_t checksum );

/* Commit
 *  records a block location
 */
rc_t KColumnIdx1Commit ( KColumnIdx1 *self, const KColBlockLoc *bloc );
rc_t KColumnIdx1CommitDone ( KColumnIdx1 *self );

/* Revert
 *  reverses effect of commit
 */
bool KColumnIdx1Revert ( KColumnIdx1 *self, int64_t start_id, uint32_t id_range );


#ifdef __cplusplus
}
#endif

#endif /* _h_colidx1_priv_ */
