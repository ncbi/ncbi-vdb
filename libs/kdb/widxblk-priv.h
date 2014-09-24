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

#ifndef _h_widxblk_priv_
#define _h_widxblk_priv_

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

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


/*--------------------------------------------------------------------------
 * KColBlockMap
 *  maps an index or page section of block index
 */
typedef struct KColBlockMap KColBlockMap;
struct KColBlockMap
{
    void ( * trans ) ( KColBlockMap *self, uint32_t idx,
        uint64_t first, uint32_t span );

    union
    {
        /* for assignment and manipulation */
        void *p;
        char *c;

        /* type 1 has a constant range/size */
        uint32_t *span;

        /* type 2 has a predictable id/pos */
        uint64_t *first;

        /* type 3 has a predictable id/pos and constant range/size */
        struct
        {
            uint64_t pg;
            uint32_t sz;
        } *pred;

    } h;

    /* types 0 and 1 have explicit id/pos, where id is ordered */
    uint64_t *d;

    /* type 2 has explicit range/size */
    uint32_t *s;
};

/* SetEntry
 *  sets an entry
 */
void KColBlockMapSetEntry ( KColBlockMap *self,
    uint32_t slot, uint64_t first, uint32_t span );
#define KColBlockMapSetEntry( self, slot, first, span ) \
    ( * ( self ) -> trans ) ( self, slot, first, span )


/*--------------------------------------------------------------------------
 * KColBlockLocInfo
 *  extensions for defining prospective idx2 blocks
 */
typedef struct KColBlockLocInfo KColBlockLocInfo;
struct KColBlockLocInfo
{
    /* the range of ids in a block */
    int64_t start_id;
    int64_t end_id;

    /* the range of data pages */
    uint64_t start_pg;
    uint64_t end_pg;

    /* the size of the last blob in block
       which is the size of them all in the
       case of pg_type == 2 or 3 */
    uint32_t size;

    /* the number of entries in block */
    uint32_t count;

    /* the types */
    uint16_t id_type;
    uint16_t pg_type;
};

/* Merge
 *  calculates the cost of merging two blocks
 *  returns the cost in bytes where a benefit is negative
 *  merges right into left
 */
int64_t KColBlockLocInfoMerge ( KColBlockLocInfo *left,
    const KColBlockLocInfo *right );


/*--------------------------------------------------------------------------
 * KColWIdxBlock
 *  level 2 index building block
 *
 *  each index block has two sections
 */
typedef struct KColWIdxBlock KColWIdxBlock;
struct KColWIdxBlock
{
    KColBlockMap id;
    KColBlockMap pg;

    uint32_t count;
    uint32_t size;

    /* for assigning */
    uint32_t idx;
};

/* Init
 */
rc_t KColWIdxBlockInit ( KColWIdxBlock *iblk, const KColBlockLocInfo *info );

/* Whack
 */
void KColWIdxBlockWhack ( KColWIdxBlock *self );

/* SetEntry
 *  sets index and page location information
 */
void KColWIdxBlockSetEntry ( KColWIdxBlock *self,
    int64_t start_id, uint32_t span,
    uint64_t pg, uint32_t sz );
#if 0
#define KColWIdxBlockSetEntry( self, start_id, span, pos, sz ) \
    ( KColBlockMapSetEntry ( & ( self ) -> id, ( self ) -> idx, start_id, span ), \
      KColBlockMapSetEntry ( & ( self ) -> pg, ( self ) -> idx, pos, sz ), \
      ( void ) ++ ( self ) -> idx )
#endif

/* Compress
 *  compress the block
 *  update the BlockLoc
 */
rc_t KColWIdxBlockCompress ( KColWIdxBlock *self, bool bswap,
    const KColBlockLocInfo *info, KColBlockLoc *bloc, size_t *to_write );

/* PersistPtr
 *  access the pointer for persisting to disk
 */
const void *KColWIdxBlockPersistPtr ( const KColWIdxBlock *self,
    const KColBlockLoc *bloc );


#ifdef __cplusplus
}
#endif

#endif /* _h_colidx2_priv_ */
