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

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
typedef struct KMDataNode KMDataNode;

/*--------------------------------------------------------------------------
 * KMetadata
 *   base structure for KMetadata implementations
 */
typedef struct KMetadata KMetadata;

typedef struct KMetadata_vt KMetadata_vt;
struct KMetadata_vt
{
    /* Public API */
    rc_t ( CC * whack )         ( KMetadata *self );
    rc_t ( CC * addRef )        ( const KMetadata *self );
    rc_t ( CC * release )       ( const KMetadata *self );
    rc_t ( CC * version )       ( const KMetadata *self, uint32_t *version );
    rc_t ( CC * byteOrder )     ( const KMetadata *self, bool *reversed );
    rc_t ( CC * revision )      ( const KMetadata *self, uint32_t *revision );
    rc_t ( CC * maxRevision )   ( const KMetadata *self, uint32_t *revision );
    rc_t ( CC * openRevision )  ( const KMetadata *self, const KMetadata **meta, uint32_t revision );
    rc_t ( CC * getSequence )   ( const KMetadata *self,const char *seq, int64_t *val );
    rc_t ( CC * vOpenNodeRead ) ( const KMetadata *self,const KMDataNode **node, const char *path, va_list args );
};

struct KMetadata
{
    const KMetadata_vt * vt;

    KRefcount refcount;
};

// default implelentations where exist
extern rc_t CC KMetadataBaseWhack ( KMetadata *self );
extern rc_t CC KMetadataBaseAddRef ( const KMetadata *self );
extern rc_t CC KMetadataBaseRelease ( const KMetadata *self );

/* Attach
 * Sever
 *  like AddRef/Release, except called internally
 *  indicates that a child object is letting go...
 */
KMetadata *KMetadataAttach ( const KMetadata *self );
rc_t KMetadataSever ( const KMetadata *self );

#ifdef __cplusplus
}
#endif

