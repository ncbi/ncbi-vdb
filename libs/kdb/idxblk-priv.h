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

#ifndef _h_idxblk_priv_
#define _h_idxblk_priv_

#ifndef _h_colfmt_priv_
#include "colfmt-priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KColConstBlockMap
 *  maps an index or page section of block index
 */
typedef struct KColConstBlockMap KColConstBlockMap;
struct KColConstBlockMap
{
    /* types 1-3 have a header section */
    union
    {
        /* for assignment and manipulation */
        const void *p;
        const char *c;

        /* type 1 has a constant range/size */
        const uint32_t *span;

        /* type 2 has a predictable id/pos */
        const uint64_t *first;

        /* type 3 has a starting page and constant size */
        const struct
        {
            uint64_t pg;
            uint32_t sz;
        } *pred;

        /* for rewriting columns */
        uint32_t *last;

    } h;

    /* types 0 and 1 have explicit id/pos, where id is ordered */
    const uint64_t *d;

    /* types 0 and 2 have explicit range/size */
    const uint32_t *s;
};


/*--------------------------------------------------------------------------
 * KColIdxBlock
 *  level 2 index reading block
 *
 *  each index block has two sections
 */
typedef struct KColIdxBlock KColIdxBlock;
struct KColIdxBlock
{
    /* function for locating index */
    int ( * find ) ( const KColIdxBlock *self,
        const KColBlockLoc *bloc, uint32_t count,
        int64_t id, int64_t *first, uint32_t *span );

    /* function for locating blob */
    void ( * get ) ( const KColIdxBlock *self,
        const KColBlockLoc *bloc, uint32_t count,
        uint32_t idx, uint64_t *pos, uint32_t *sz );

    /* index section map */
    KColConstBlockMap id;

    /* page locator section map */
    KColConstBlockMap pg;
};

/* Init
 */
rc_t KColIdxBlockInit ( KColIdxBlock *self,
    const KColBlockLoc *bloc, size_t orig,
    const void *block, size_t block_size, bool bswap );

/* Find
 *  find zero-based index of entry into block
 *  that satisfies request
 *  returns -1 if not found
 */
int KColIdxBlockFind ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    int64_t id, int64_t *first, uint32_t *span );
#define KColIdxBlockFind( self, bloc, count, id, first, span ) \
    ( * ( self ) -> find ) ( self, bloc, count, id, first, span )

/* Get
 *  get blob location information
 */
void KColIdxBlockGet ( const KColIdxBlock *self,
    const KColBlockLoc *bloc, uint32_t count,
    uint32_t idx, uint64_t *pg, uint32_t *sz );
#define KColIdxBlockGet( self, bloc, count, idx, pg, sz ) \
    ( * ( self ) -> get ) ( self, bloc, count, idx, pg, sz )


#ifdef __cplusplus
}
#endif

#endif /* _h_idxblk_priv_ */
