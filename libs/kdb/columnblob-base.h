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

#include <kdb/column.h>

/*--------------------------------------------------------------------------
 * KColumnBlobBase, base structure for KColumnBlob read-side implementations
 */
typedef struct KColumnBlob KColumnBlob;

typedef struct KColumnBlob_vt KColumnBlob_vt;
struct KColumnBlob_vt
{
    /* Public read-side API */
    rc_t ( CC * whack )             ( KColumnBlob * self );
    rc_t ( CC * addRef )            ( const KColumnBlob * self );
    rc_t ( CC * release )           ( const KColumnBlob * self );
    rc_t ( CC * read )              ( const KColumnBlob * self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
    rc_t ( CC * readAll )           ( const KColumnBlob * self, struct KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size );
    rc_t ( CC * validate )          ( const KColumnBlob * self );
    rc_t ( CC * validateBuffer )    ( const KColumnBlob * self, struct KDataBuffer const * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size );
    rc_t ( CC * idRange )           ( const KColumnBlob * self, int64_t *first, uint32_t *count );
};

// default implelentations where exist
extern rc_t CC KColumnBlobBaseWhack ( KColumnBlob *self );
extern rc_t CC KColumnBlobBaseAddRef ( const KColumnBlob *self );
extern rc_t CC KColumnBlobBaseRelease ( const KColumnBlob *self );

struct KColumnBlob
{
    const KColumnBlob_vt * vt;

    atomic32_t refcount;
};

extern void KColumnBlobBaseInit( KColumnBlob *self, const KColumnBlob_vt * vt );

#ifdef __cplusplus
}
#endif

