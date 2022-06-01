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

#ifndef _h_blob_priv_itf_
#define _h_blob_priv_itf_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_vdb_blob_
#include <vdb/blob.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef _h_klib_data_buffer_
#include <klib/data-buffer.h>
#endif

#ifndef _h_vdb_xform_
#include <vdb/xform.h>
#endif

#if _DEBUGGING
#define VBLOG_HAS_NAME 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct PageMap;
struct BlobHeaders;
struct VBlobPageMapCache;

/*--------------------------------------------------------------------------
 * VBlob
 */
struct VBlob
{
    int64_t start_id;
    int64_t stop_id;

    struct PageMap *pm;
    struct BlobHeaders *headers;
    struct VBlobPageMapCache *spmc; /* cache for split */
    KDataBuffer data;
    KRefcount refcount;

/*    uint32_t row_count; */ /* == stop_id + 1 - start_id */
    bool no_cache;
    VByteOrder byte_order;
#if VBLOG_HAS_NAME
    const char name[32];
#endif
};

#define BlobBufferBits(BLOB) ((uint64_t)KDataBufferBits(&(BLOB)->data))
#define BlobBufferBytes(BLOB) ((size_t)(KDataBufferBytes(&(BLOB)->data)))
#define BlobRowCount(BLOB) ((uint32_t)( (uint64_t)((BLOB)->stop_id - (BLOB)->start_id) == UINT64_MAX ? UINT32_MAX : (BLOB)->stop_id + 1 - (BLOB)->start_id ))

#ifdef __cplusplus
}
#endif
#endif /* _h_blob_priv_ */
