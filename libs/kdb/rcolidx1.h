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

#ifdef __cplusplus
extern "C" {
#endif


#ifndef LAZY_LOAD_COLUMN_IDX1
#define LAZY_LOAD_COLUMN_IDX1 0
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KColIdxBlock;
struct KColBlockLocInfo;


/*--------------------------------------------------------------------------
 * KRColumnIdx1
 *  level 1 index
 */
typedef struct KRColumnIdx1 KRColumnIdx1;
struct KRColumnIdx1
{
#if USE_BSTREE_IN_COLUMN_IDX1
    /* tree of level-2 block locators */
    BSTree bst;
#else
    /***** binary searched array with state *****/
    uint64_t last_found;
    const KColBlockLoc *data;
#endif
    struct KFile const *f;
    struct KFile const *fidx;
    uint32_t count;
    uint32_t vers;
#if LAZY_LOAD_COLUMN_IDX1
    uint32_t load_off;
    rc_t load_rc;
#endif
    bool bswap;
    bool loaded;
    uint8_t align [ sizeof ( size_t ) - 2 ];
};

/* Open
 */
rc_t KRColumnIdx1OpenRead ( KRColumnIdx1 *self, const KDirectory *dir,
    uint64_t *data_eof, uint32_t *idx0_count, uint64_t *idx2_eof,
    size_t *pgsize, int32_t *checksum );

/* Whack
 */
rc_t KRColumnIdx1Whack ( KRColumnIdx1 *self );

/* Version
 */
rc_t KRColumnIdx1Version ( const KRColumnIdx1 *self, uint32_t *version );
#define KRColumnIdx1Version( self, version ) \
    ( * ( version ) = ( self ) -> vers, 0 )

/* ByteOrder
 */
rc_t KRColumnIdx1ByteOrder ( const KRColumnIdx1 *self, bool *reversed );
#define KRColumnIdx1ByteOrder( self, reversed ) \
    ( * ( reversed ) = ( self ) -> bswap, 0 )

/* IdRange
 *  returns range of ids contained within
 */
bool KRColumnIdx1IdRange ( const KRColumnIdx1 *self,
    int64_t *first, int64_t *upper );

/* LocateFirstRowIdBlob
 */
rc_t KRColumnIdx1LocateFirstRowIdBlob ( const KRColumnIdx1 * self,
    KColBlockLoc * bloc, int64_t start );

/* LocateBlock
 *  locates an idx2 block by range
 */
rc_t KRColumnIdx1LocateBlock ( const KRColumnIdx1 *self,
    KColBlockLoc *bloc, int64_t first, int64_t upper );


#ifdef __cplusplus
}
#endif

