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

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KCOLUMN_IMPL
#define KCOLUMN_IMPL KColumnBase
#endif

struct KDBManager;
struct KTable;

/*--------------------------------------------------------------------------
 * KColumnBase, base structure for KCOLUMN_IMPL implementations
 */
typedef struct KColumnBase KColumnBase;

typedef struct KColumnBase_vt KColumnBase_vt;
struct KColumnBase_vt
{
    /* Public API */
    rc_t ( CC * whack )             ( KCOLUMN_IMPL *self );
    rc_t ( CC * addRef )            ( const KCOLUMN_IMPL *self );
    rc_t ( CC * release )           ( const KCOLUMN_IMPL *self );
    bool ( CC * locked )            ( const KCOLUMN_IMPL *self ); // not used
    rc_t ( CC * version )           ( const KCOLUMN_IMPL *self, uint32_t *version ); // not used
    rc_t ( CC * byteOrder )         ( const KCOLUMN_IMPL *self, bool *reversed ); // not used
    rc_t ( CC * idRange )           ( const KCOLUMN_IMPL *self, int64_t *first, uint64_t *count );
    rc_t ( CC * findFirstRowId )    ( const KCOLUMN_IMPL * self, int64_t * found, int64_t start );
    rc_t ( CC * openManagerRead )   ( const KCOLUMN_IMPL *self, struct KDBManager const **mgr );
    rc_t ( CC * openParentRead )    ( const KCOLUMN_IMPL *self, struct KTable const **tbl );

    // //TODO: write-side only; decide how to handle
    // rc_t ( CC * reindex )           ( KCOLUMN_IMPL *self );
    // rc_t ( CC * commitFreq )        ( KCOLUMN_IMPL *self, uint32_t *freq );
    // rc_t ( CC * setCommitFreq )     ( KCOLUMN_IMPL *self, uint32_t freq );
    // rc_t ( CC * openManagerUpdate ) ( KCOLUMN_IMPL *self, struct KDBManager **mgr );
    // rc_t ( CC * openParentUpdate )  ( KCOLUMN_IMPL *self, struct KTable **tbl );
};

// default implelentations where exist
extern rc_t KColumnBaseWhack ( KCOLUMN_IMPL *self );
extern rc_t CC KColumnBaseAddRef ( const KCOLUMN_IMPL *self );
extern rc_t CC KColumnBaseRelease ( const KCOLUMN_IMPL *self );

struct KColumnBase
{
    const KColumnBase_vt * vt;

    KRefcount refcount;

    // struct KTable const *tbl;
    // struct KDBManager const *mgr;
    // struct KDirectory const *dir;

    // KColumnIdx idx;
    // KColumnData df;

    // uint32_t csbytes;
    // int32_t checksum;
    // char path [ 1 ];
};


#ifdef __cplusplus
}
#endif

