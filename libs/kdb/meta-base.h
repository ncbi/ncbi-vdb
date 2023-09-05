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

#ifndef KMETA_IMPL
#define KMETA_IMPL KMetadataBase
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
typedef struct KMDataNode KMDataNode;

/*--------------------------------------------------------------------------
 * KMetadataBase
 *   base structure for KMetadata implementations
 */
typedef struct KMetadataBase KMetadataBase;

typedef struct KMetadata_vt KMetadata_vt;
struct KMetadata_vt
{
    /* Public API */
    rc_t ( CC * whack )                 ( KMETA_IMPL *self );
    rc_t ( CC * addRef )                ( const KMETA_IMPL *self );
    rc_t ( CC * release )               ( const KMETA_IMPL *self );
    rc_t ( CC * version )               ( const KMETA_IMPL *self, uint32_t *version );
};

// KDB_EXTERN rc_t CC KMetadataByteOrder ( const KMetadata *self, bool *reversed );
// KDB_EXTERN rc_t CC KMetadataRevision ( const KMetadata *self, uint32_t *revision );
// KDB_EXTERN rc_t CC KMetadataMaxRevision ( const KMetadata *self, uint32_t *revision );
// KDB_EXTERN rc_t CC KMetadataOpenRevision ( const KMetadata *self, const KMetadata **meta, uint32_t revision );
// KDB_EXTERN rc_t CC KMetadataGetSequence ( const KMetadata *self,const char *seq, int64_t *val );
// KDB_EXTERN rc_t CC KMetadataSetSequence ( KMetadata *self,const char *seq, int64_t val );
// KDB_EXTERN rc_t CC KMetadataNextSequence ( KMetadata *self,const char *seq, int64_t *val );
// KDB_EXTERN rc_t CC KMetadataVOpenNodeRead ( const KMetadata *self,const KMDataNode **node, const char *path, va_list args );
// KDB_EXTERN rc_t CC KMDataNodeVOpenNodeRead ( const KMDataNode *self,const KMDataNode **node, const char *path, va_list args );

// // move to Database/Table/Column
// KDB_EXTERN rc_t CC KDatabaseOpenMetadataRead ( struct KDatabase const *self, const KMetadata **meta );
// KDB_EXTERN rc_t CC KTableOpenMetadataRead ( struct KTable const *self, const KMetadata **meta );
// KDB_EXTERN rc_t CC KColumnOpenMetadataRead ( struct KColumn const *self, const KMetadata **meta );

// // KMDataNode
// KDB_EXTERN rc_t CC KMDataNodeAddRef ( const KMDataNode *self );
// KDB_EXTERN rc_t CC KMDataNodeRelease ( const KMDataNode *self );
// KDB_EXTERN rc_t CC KMDataNodeByteOrder ( const KMDataNode *self, bool *reversed );
// KDB_EXTERN rc_t CC KMDataNodeRead ( const KMDataNode *self,size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
// KDB_EXTERN rc_t CC KMDataNodeReadB8 ( const KMDataNode *self, void *b8 );
// KDB_EXTERN rc_t CC KMDataNodeReadB16 ( const KMDataNode *self, void *b16 );
// KDB_EXTERN rc_t CC KMDataNodeReadB32 ( const KMDataNode *self, void *b32 );
// KDB_EXTERN rc_t CC KMDataNodeReadB64 ( const KMDataNode *self, void *b64 );
// KDB_EXTERN rc_t CC KMDataNodeReadB128 ( const KMDataNode *self, void *b128 );
// KDB_EXTERN rc_t CC KMDataNodeReadAsI16 ( const KMDataNode *self, int16_t *i );
// KDB_EXTERN rc_t CC KMDataNodeReadAsU16 ( const KMDataNode *self, uint16_t *u );
// KDB_EXTERN rc_t CC KMDataNodeReadAsI32 ( const KMDataNode *self, int32_t *i );
// KDB_EXTERN rc_t CC KMDataNodeReadAsU32 ( const KMDataNode *self, uint32_t *u );
// KDB_EXTERN rc_t CC KMDataNodeReadAsI64 ( const KMDataNode *self, int64_t *i );
// KDB_EXTERN rc_t CC KMDataNodeReadAsU64 ( const KMDataNode *self, uint64_t *u );
// KDB_EXTERN rc_t CC KMDataNodeReadAsF64 ( const KMDataNode *self, double *f );
// KDB_EXTERN rc_t CC KMDataNodeReadCString ( const KMDataNode *self,char *buffer, size_t bsize, size_t *size );
// KDB_EXTERN rc_t CC KMDataNodeReadAttr ( const KMDataNode *self, const char *name,char *buffer, size_t bsize, size_t *size );
// KDB_EXTERN rc_t CC KMDataNodeReadAttrAsI16 ( const KMDataNode *self, const char *attr, int16_t *i );
// KDB_EXTERN rc_t CC KMDataNodeReadAttrAsU16 ( const KMDataNode *self, const char *attr, uint16_t *u );
// KDB_EXTERN rc_t CC KMDataNodeReadAttrAsI32 ( const KMDataNode *self, const char *attr, int32_t *i );
// KDB_EXTERN rc_t CC KMDataNodeReadAttrAsU32 ( const KMDataNode *self, const char *attr, uint32_t *u );
// KDB_EXTERN rc_t CC KMDataNodeReadAttrAsI64 ( const KMDataNode *self, const char *attr, int64_t *i );
// KDB_EXTERN rc_t CC KMDataNodeReadAttrAsU64 ( const KMDataNode *self, const char *attr, uint64_t *u );
// KDB_EXTERN rc_t CC KMDataNodeReadAttrAsF64 ( const KMDataNode *self, const char *attr, double *f );
// KDB_EXTERN rc_t CC KMDataNodeCompare( const KMDataNode *self, KMDataNode const *other, bool *equal );

// // move to KTable
// KDB_EXTERN rc_t CC KTableMetaCopy( struct KTable *self, const struct KTable *src, const char * path, bool src_node_has_to_exist );
// KDB_EXTERN rc_t CC KTableMetaCompare ( const struct KTable *self, const struct KTable *other, const char * path, bool * equal );

struct KMetadataBase
{
    KMetadata_vt * vt;

    KRefcount refcount;
};

// default implelentations where exist
extern rc_t CC KMetadataBaseWhack ( KMETA_IMPL *self );
extern rc_t CC KMetadataBaseAddRef ( const KMETA_IMPL *self );
extern rc_t CC KMetadataBaseRelease ( const KMETA_IMPL *self );

/* Attach
 * Sever
 *  like AddRef/Release, except called internally
 *  indicates that a child object is letting go...
 */
KMETA_IMPL *KMetadataAttach ( const KMETA_IMPL *self );
rc_t KMetadataSever ( const KMETA_IMPL *self );

// write side only public API

// KDB_EXTERN rc_t CC KDatabaseOpenMetadataUpdate ( struct KDatabase *self, KMetadata **meta );
// KDB_EXTERN rc_t CC KTableOpenMetadataUpdate ( struct KTable *self, KMetadata **meta );
// KDB_EXTERN rc_t CC KColumnOpenMetadataUpdate ( struct KColumn *self, KMetadata **meta );

// KDB_EXTERN rc_t CC KMetadataCommit ( KMetadata *self );
// KDB_EXTERN rc_t CC KMetadataFreeze ( KMetadata *self );

// KDB_EXTERN rc_t CC KMetadataOpenNodeUpdate ( KMetadata *self,KMDataNode **node, const char *path, ... );
// KDB_EXTERN rc_t CC KMDataNodeOpenNodeUpdate ( KMDataNode *self,KMDataNode **node, const char *path, ... );

// KDB_EXTERN rc_t CC KMetadataVOpenNodeUpdate ( KMetadata *self,KMDataNode **node, const char *path, va_list args );
// KDB_EXTERN rc_t CC KMDataNodeVOpenNodeUpdate ( KMDataNode *self,KMDataNode **node, const char *path, va_list args );


// KDB_EXTERN rc_t CC KMDataNodeWrite ( KMDataNode *self, const void *buffer, size_t size );
// KDB_EXTERN rc_t CC KMDataNodeAppend ( KMDataNode *self, const void *buffer, size_t size );

// KDB_EXTERN rc_t CC KMDataNodeWriteB8 ( KMDataNode *self, const void *b8 );
// KDB_EXTERN rc_t CC KMDataNodeWriteB16 ( KMDataNode *self, const void *b16 );
// KDB_EXTERN rc_t CC KMDataNodeWriteB32 ( KMDataNode *self, const void *b32 );
// KDB_EXTERN rc_t CC KMDataNodeWriteB64 ( KMDataNode *self, const void *b64 );
// KDB_EXTERN rc_t CC KMDataNodeWriteB128 ( KMDataNode *self, const void *b128 );

// KDB_EXTERN rc_t CC KMDataNodeWriteCString ( KMDataNode *self, const char *str );

// KDB_EXTERN rc_t CC KMDataNodeWriteAttr ( KMDataNode *self,
//     const char *name, const char *value );

// KDB_EXTERN rc_t CC KMDataNodeDropAll ( KMDataNode *self );
// KDB_EXTERN rc_t CC KMDataNodeDropAttr ( KMDataNode *self, const char *attr );
// KDB_EXTERN rc_t CC KMDataNodeDropChild ( KMDataNode *self, const char *path, ... );
// KDB_EXTERN rc_t CC KMDataNodeVDropChild ( KMDataNode *self, const char *path, va_list args );

// KDB_EXTERN rc_t CC KMDataNodeRenameAttr ( KMDataNode *self, const char *from, const char *to );
// KDB_EXTERN rc_t CC KMDataNodeRenameChild ( KMDataNode *self, const char *from, const char *to );

// KDB_EXTERN rc_t CC KMDataNodeCopy( KMDataNode *self, KMDataNode const *source );


#ifdef __cplusplus
}
#endif

