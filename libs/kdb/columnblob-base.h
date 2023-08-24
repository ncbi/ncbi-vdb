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

#ifndef KCOLUMNBLOB_IMPL
#define KCOLUMNBLOB_IMPL KColumnBlobBase
#endif

#include <kdb/column.h>

/*--------------------------------------------------------------------------
 * KColumnBlobBase, base structure for KColumnBlob implementations
 */
typedef struct KColumnBlobBase KColumnBlobBase;

typedef struct KColumnBlobBase_vt KColumnBlobBase_vt;
struct KColumnBlobBase_vt
{
    /* Public API */
    rc_t ( CC * whack )             ( KCOLUMNBLOB_IMPL *self );
    rc_t ( CC * addRef )            ( const KCOLUMNBLOB_IMPL *self );
    rc_t ( CC * release )           ( const KCOLUMNBLOB_IMPL *self );
    rc_t ( CC * read )              ( const KCOLUMNBLOB_IMPL *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
    rc_t ( CC * readAll )           ( const KCOLUMNBLOB_IMPL * self, struct KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size );
    rc_t ( CC * validate )          ( const KCOLUMNBLOB_IMPL *self );
    rc_t ( CC * validateBuffer )    ( const KCOLUMNBLOB_IMPL * self, struct KDataBuffer const * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size );
    rc_t ( CC * idRange )           ( const KCOLUMNBLOB_IMPL *self, int64_t *first, uint32_t *count );

    // //TODO: write-side only; decide how to handle
    // rc_t ( CC * append ) ( KColumnBlob *self, const void *buffer, size_t size );
    // rc_t ( CC * assignRange ) ( KColumnBlob *self, int64_t first, uint32_t count );
    // rc_t ( CC * commit ) ( KColumnBlob *self );
};

// default implelentations where exist
extern rc_t CC KColumnBlobBaseWhack ( KCOLUMNBLOB_IMPL *self );
extern rc_t CC KColumnBlobBaseAddRef ( const KCOLUMNBLOB_IMPL *self );
extern rc_t CC KColumnBlobBaseRelease ( const KCOLUMNBLOB_IMPL *self );

struct KColumnBlobBase
{
    const KColumnBlobBase_vt * vt;

    atomic32_t refcount;

    // /* holds existing blob loc */
    // KColBlobLoc loc;
    // KColumnPageMap pmorig;

    // /* owning column */
    // const KColumn *col;

    // /* captured from idx1 for CRC32 validation */
    // bool bswap;
};

extern void KColumnBlobBaseInit( KCOLUMNBLOB_IMPL *self, const KColumnBlobBase_vt * vt );

#ifdef __cplusplus
}
#endif

