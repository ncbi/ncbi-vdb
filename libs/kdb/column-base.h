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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KCOLUMN_IMPL
#define KCOLUMN_IMPL KColumn
#endif

struct KDBManager;
struct KTable;
struct KMetadata;
struct KColumnBlob;

/*--------------------------------------------------------------------------
 * KColumnBase, base structure for KColumn implementations
 */
typedef struct KColumn KColumn;

typedef struct KColumn_vt KColumn_vt;
struct KColumn_vt
{
    /* Public API */
    rc_t ( CC * whack )             ( KCOLUMN_IMPL * self );
    rc_t ( CC * addRef )            ( const KCOLUMN_IMPL * self );
    rc_t ( CC * release )           ( const KCOLUMN_IMPL * self );
    bool ( CC * locked )            ( const KCOLUMN_IMPL * self ); // not used
    rc_t ( CC * version )           ( const KCOLUMN_IMPL * self, uint32_t * version ); // not used
    rc_t ( CC * byteOrder )         ( const KCOLUMN_IMPL * self, bool * reversed ); // not used
    rc_t ( CC * idRange )           ( const KCOLUMN_IMPL * self, int64_t * first, uint64_t * count );
    rc_t ( CC * findFirstRowId )    ( const KCOLUMN_IMPL * self, int64_t * found, int64_t start );
    rc_t ( CC * openManagerRead )   ( const KCOLUMN_IMPL * self, struct KDBManager const ** mgr );
    rc_t ( CC * openParentRead )    ( const KCOLUMN_IMPL * self, struct KTable const ** tbl );
    rc_t ( CC * openMetadataRead )  ( const KCOLUMN_IMPL * self, const struct KMetadata ** meta );
    rc_t ( CC * openBlobRead )      ( const KCOLUMN_IMPL * self, const struct KColumnBlob **blobp, int64_t id );
};

// default implelentations where exist
extern rc_t KColumnBaseWhack ( KCOLUMN_IMPL *self );
extern rc_t CC KColumnBaseAddRef ( const KCOLUMN_IMPL *self );
extern rc_t CC KColumnBaseRelease ( const KCOLUMN_IMPL *self );

struct KColumn
{
    const KColumn_vt * vt;

    KRefcount refcount;
};

/* Attach
 * Sever
 *  like Release, except called internally
 *  indicates that a child object is letting go...
 */
extern KCOLUMN_IMPL *KColumnAttach ( const KCOLUMN_IMPL *self );
extern rc_t KColumnSever ( const KCOLUMN_IMPL *self );

#ifdef __cplusplus
}
#endif

