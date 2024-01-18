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

#include <klib/refcount.h>

#include <kdb/index.h>

#include <stdarg.h>

#ifndef KINDEX_IMPL
#define KINDEX_IMPL KIndex
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * KIndex
 *   base structure for KIndex implementations
 */
typedef struct KIndex KIndex;

typedef struct KIndex_vt KIndex_vt;
struct KIndex_vt
{
    /* Public API */
    rc_t ( CC * whack )             ( KINDEX_IMPL *self );
    rc_t ( CC * addRef )            ( const KINDEX_IMPL *self );
    rc_t ( CC * release )           ( const KINDEX_IMPL *self );
    bool ( CC * locked )            ( const KINDEX_IMPL *self );
    rc_t ( CC * version )           ( const KINDEX_IMPL *self, uint32_t *version );
    rc_t ( CC * type )              ( const KINDEX_IMPL *self, KIdxType *type );
    rc_t ( CC * consistencyCheck )  ( const KINDEX_IMPL *self, uint32_t level,
                                        int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
                                        uint64_t *num_rows, uint64_t *num_holes );
    rc_t ( CC * findText )          ( const KINDEX_IMPL *self, const char *key, int64_t *start_id, uint64_t *id_count,
                                        int ( CC * custom_cmp ) ( const void *item, struct PBSTNode const *n, void *data ),
                                        void *data );
    rc_t ( CC * findAllText )       ( const KINDEX_IMPL *self, const char *key, rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, void *data ), void *data );
    rc_t ( CC * projectText )       ( const KINDEX_IMPL *self, int64_t id, int64_t *start_id, uint64_t *id_count, char *key, size_t kmax, size_t *actsize );
    rc_t ( CC * projectAllText )    ( const KINDEX_IMPL *self, int64_t id, rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, const char *key, void *data ), void *data );
    rc_t ( CC * findU64 )           ( const KINDEX_IMPL *self, uint64_t offset, uint64_t *key, uint64_t *key_size, int64_t *start_id, uint64_t *id_count );
    rc_t ( CC * findAllU64 )        ( const KINDEX_IMPL *self, uint64_t offset,
                                        rc_t ( CC * f ) ( uint64_t key, uint64_t key_size, int64_t start_id, uint64_t id_count, void *data ),
                                        void *data);
    void ( CC * setMaxRowId )       ( const KINDEX_IMPL *cself, int64_t max_row_id );
};

struct KIndex
{
    const KIndex_vt * vt;

    KRefcount refcount;
};

// destructor
LIB_EXPORT rc_t CC KIndexWhack ( KIndex *self );

// default implelentations where exist
extern rc_t CC KIndexBaseWhack ( KINDEX_IMPL *self );
extern rc_t CC KIndexBaseAddRef ( const KINDEX_IMPL *self );
extern rc_t CC KIndexBaseRelease ( const KINDEX_IMPL *self );

// write-side only API
// KDB_EXTERN rc_t CC KIndexCommit ( KIndex *self );
// KDB_EXTERN rc_t CC KIndexInsertText ( KIndex *self, bool unique, const char *key, int64_t id );
// KDB_EXTERN rc_t CC KIndexDeleteText ( KIndex *self, const char *key );
// KDB_EXTERN rc_t CC KIndexInsertU64 ( KIndex *self, bool unique, uint64_t key, uint64_t key_size, int64_t start_id, uint64_t id_count );
// KDB_EXTERN rc_t CC KIndexDeleteU64 ( KIndex *self, uint64_t key );

#ifdef __cplusplus
}
#endif

