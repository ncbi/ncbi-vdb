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

#ifndef _h_kdb_meta_
#define _h_kdb_meta_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KTable;
struct KColumn;
struct KDatabase;


/*--------------------------------------------------------------------------
 * KMetadata
 *  a versioned, hierarchical structure
 */
typedef struct KMetadata KMetadata;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
KDB_EXTERN rc_t CC KMetadataAddRef ( const KMetadata *self );
KDB_EXTERN rc_t CC KMetadataRelease ( const KMetadata *self );


/* OpenMetadataRead
 *  opens metadata for read
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
KDB_EXTERN rc_t CC KDatabaseOpenMetadataRead ( struct KDatabase const *self,
    const KMetadata **meta );
KDB_EXTERN rc_t CC KTableOpenMetadataRead ( struct KTable const *self,
    const KMetadata **meta );
KDB_EXTERN rc_t CC KColumnOpenMetadataRead ( struct KColumn const *self,
    const KMetadata **meta );

/* OpenMetadataUpdate
 *  open metadata for read/write
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
KDB_EXTERN rc_t CC KDatabaseOpenMetadataUpdate ( struct KDatabase *self,
    KMetadata **meta );
KDB_EXTERN rc_t CC KTableOpenMetadataUpdate ( struct KTable *self,
    KMetadata **meta );
KDB_EXTERN rc_t CC KColumnOpenMetadataUpdate ( struct KColumn *self,
    KMetadata **meta );


/* Version
 *  returns the metadata format version
 */
KDB_EXTERN rc_t CC KMetadataVersion ( const KMetadata *self, uint32_t *version );


/* ByteOrder
 *  indicates whether original byte order is reversed
 *  under current architecture.
 *
 *  the byte order of the column is established by
 *  the host architecture when created.
 *
 *  "reversed" [ OUT ] - if true, the original byte
 *  order is reversed with regard to host native byte order.
 */
KDB_EXTERN rc_t CC KMetadataByteOrder ( const KMetadata *self, bool *reversed );


/* Revision
 *  returns current revision number
 *  where 0 ( zero ) means tip
 */
KDB_EXTERN rc_t CC KMetadataRevision ( const KMetadata *self, uint32_t *revision );


/* MaxRevision
 *  returns the maximum revision available
 */
KDB_EXTERN rc_t CC KMetadataMaxRevision ( const KMetadata *self, uint32_t *revision );


/* Commit
 *  ensure any changes are committed to disk
 */
KDB_EXTERN rc_t CC KMetadataCommit ( KMetadata *self );


/* Freeze
 *  freezes current metadata revision
 *  further modification will begin on a copy
 */
KDB_EXTERN rc_t CC KMetadataFreeze ( KMetadata *self );


/* OpenRevision
 *  opens a read-only indexed revision of metadata
 */
KDB_EXTERN rc_t CC KMetadataOpenRevision ( const KMetadata *self,
    const KMetadata **meta, uint32_t revision );


/* GetSequence
 * SetSequence
 * NextSequence
 *  access a named sequence
 *
 *  "seq" [ IN ] - NUL terminated sequence name
 *
 *  "val" [ OUT ] - return parameter for sequence value
 *  "val" [ IN ] - new sequence value
 */
KDB_EXTERN rc_t CC KMetadataGetSequence ( const KMetadata *self,
    const char *seq, int64_t *val );
KDB_EXTERN rc_t CC KMetadataSetSequence ( KMetadata *self,
    const char *seq, int64_t val );
KDB_EXTERN rc_t CC KMetadataNextSequence ( KMetadata *self,
    const char *seq, int64_t *val );


/*--------------------------------------------------------------------------
 * KMDataNode
 *  a node with an optional value,
 *  optional attributes, and optional children
 *
 *  nodes are identified by path, relative to a starting node,
 *  where "/" serves as a path separator.
 */
typedef struct KMDataNode KMDataNode;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
KDB_EXTERN rc_t CC KMDataNodeAddRef ( const KMDataNode *self );
KDB_EXTERN rc_t CC KMDataNodeRelease ( const KMDataNode *self );


/* OpenNodeRead
 * VOpenNodeRead
 *  opens a metadata node
 *
 *  "node" [ OUT ] - return parameter for indicated metadata node
 *
 *  "path" [ IN, NULL OKAY ] - optional path for specifying named
 *  node within metadata hierarchy. paths will be interpreted as
 *  if they were file system paths, using '/' as separator. the
 *  special values NULL and "" are interpreted as "."
 */
KDB_EXTERN rc_t CC KMetadataOpenNodeRead ( const KMetadata *self,
    const KMDataNode **node, const char *path, ... );
KDB_EXTERN rc_t CC KMDataNodeOpenNodeRead ( const KMDataNode *self,
    const KMDataNode **node, const char *path, ... );

KDB_EXTERN rc_t CC KMetadataVOpenNodeRead ( const KMetadata *self,
    const KMDataNode **node, const char *path, va_list args );
KDB_EXTERN rc_t CC KMDataNodeVOpenNodeRead ( const KMDataNode *self,
    const KMDataNode **node, const char *path, va_list args );


/* OpenNodeUpdate
 * VOpenNodeUpdate
 *  opens a metadata node
 *
 *  "node" [ OUT ] - return parameter for indicated metadata node
 *
 *  "path" [ IN, NULL OKAY ] - optional path for specifying named
 *  node within metadata hierarchy. paths will be interpreted as
 *  if they were file system paths, using '/' as separator. the
 *  special values NULL and "" are interpreted as "."
 */
KDB_EXTERN rc_t CC KMetadataOpenNodeUpdate ( KMetadata *self,
    KMDataNode **node, const char *path, ... );
KDB_EXTERN rc_t CC KMDataNodeOpenNodeUpdate ( KMDataNode *self,
    KMDataNode **node, const char *path, ... );

KDB_EXTERN rc_t CC KMetadataVOpenNodeUpdate ( KMetadata *self,
    KMDataNode **node, const char *path, va_list args );
KDB_EXTERN rc_t CC KMDataNodeVOpenNodeUpdate ( KMDataNode *self,
    KMDataNode **node, const char *path, va_list args );


/* ByteOrder
 *  indicates whether original byte order is reversed
 *  under current architecture.
 *
 *  the byte order of the column is established by
 *  the host architecture when created.
 *
 *  "reversed" [ OUT ] - if true, the original byte
 *  order is reversed with regard to host native byte order.
 */
KDB_EXTERN rc_t CC KMDataNodeByteOrder ( const KMDataNode *self, bool *reversed );


/* Read
 *  read a node value
 *
 *  "offset" [ IN ] - initial offset into metadata
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - number of bytes actually read
 *
 *  "remaining" [ OUT, NULL OKAY ] - optional return parameter for
 *  the number of bytes remaining to be read.
 *  specifically, "offset" + "num_read" + "remaining" == sizeof node data
 */
KDB_EXTERN rc_t CC KMDataNodeRead ( const KMDataNode *self,
    size_t offset, void *buffer, size_t bsize,
    size_t *num_read, size_t *remaining );


/* Write
 *  write a node value or attribute
 *  overwrites anything already there
 *
 *  "buffer" [ IN ] and "size" [ IN ] - new value data
 */
KDB_EXTERN rc_t CC KMDataNodeWrite ( KMDataNode *self, const void *buffer, size_t size );


/* Append
 *  append data to value
 *
 *  "buffer" [ IN ] and "size" [ IN ] - value data to be appended
 */
KDB_EXTERN rc_t CC KMDataNodeAppend ( KMDataNode *self, const void *buffer, size_t size );


/* Read ( formatted )
 *  reads as integer or float value in native byte order
 *
 *  "bXX" [ OUT ] - return parameter for numeric value
 */
KDB_EXTERN rc_t CC KMDataNodeReadB8 ( const KMDataNode *self, void *b8 );
KDB_EXTERN rc_t CC KMDataNodeReadB16 ( const KMDataNode *self, void *b16 );
KDB_EXTERN rc_t CC KMDataNodeReadB32 ( const KMDataNode *self, void *b32 );
KDB_EXTERN rc_t CC KMDataNodeReadB64 ( const KMDataNode *self, void *b64 );
KDB_EXTERN rc_t CC KMDataNodeReadB128 ( const KMDataNode *self, void *b128 );


/* ReadAs ( formatted )
 *  reads as integer or float value in native byte order
 *  casts smaller-sized values to desired size, e.g.
 *    uint32_t to uint64_t
 *
 *  "i" [ OUT ] - return parameter for signed integer
 *  "u" [ OUT ] - return parameter for unsigned integer
 *  "f" [ OUT ] - return parameter for double float
 */
KDB_EXTERN rc_t CC KMDataNodeReadAsI16 ( const KMDataNode *self, int16_t *i );
KDB_EXTERN rc_t CC KMDataNodeReadAsU16 ( const KMDataNode *self, uint16_t *u );
KDB_EXTERN rc_t CC KMDataNodeReadAsI32 ( const KMDataNode *self, int32_t *i );
KDB_EXTERN rc_t CC KMDataNodeReadAsU32 ( const KMDataNode *self, uint32_t *u );
KDB_EXTERN rc_t CC KMDataNodeReadAsI64 ( const KMDataNode *self, int64_t *i );
KDB_EXTERN rc_t CC KMDataNodeReadAsU64 ( const KMDataNode *self, uint64_t *u );
KDB_EXTERN rc_t CC KMDataNodeReadAsF64 ( const KMDataNode *self, double *f );


/* Read ( formatted )
 *  reads as C-string
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - output buffer for
 *  NUL terminated string.
 *
 *  "size" [ OUT ] - return parameter giving size of string
 *  not including NUL byte. the size is set both upon success
 *  and insufficient buffer space error.
 */
KDB_EXTERN rc_t CC KMDataNodeReadCString ( const KMDataNode *self,
    char *buffer, size_t bsize, size_t *size );


/* Write ( formatted )
 *  writes integer or float value in metadata byte order
 *
 *  "bXX" [ IN ] - numeric value
 */
KDB_EXTERN rc_t CC KMDataNodeWriteB8 ( KMDataNode *self, const void *b8 );
KDB_EXTERN rc_t CC KMDataNodeWriteB16 ( KMDataNode *self, const void *b16 );
KDB_EXTERN rc_t CC KMDataNodeWriteB32 ( KMDataNode *self, const void *b32 );
KDB_EXTERN rc_t CC KMDataNodeWriteB64 ( KMDataNode *self, const void *b64 );
KDB_EXTERN rc_t CC KMDataNodeWriteB128 ( KMDataNode *self, const void *b128 );


/* Write ( formatted )
 *  writes string
 *
 *  "str" [ IN ] - NUL terminated string.
 */
KDB_EXTERN rc_t CC KMDataNodeWriteCString ( KMDataNode *self, const char *str );


/* ReadAttr
 *  reads as NUL-terminated string
 *
 *  "name" [ IN ] - NUL terminated attribute name
 *
 *  "buffer" [ OUT ] and "bsize" - return parameter for attribute value
 *
 *  "size" [ OUT ] - return parameter giving size of string
 *  not including NUL byte. the size is set both upon success
 *  and insufficient buffer space error.
 */
KDB_EXTERN rc_t CC KMDataNodeReadAttr ( const KMDataNode *self, const char *name,
    char *buffer, size_t bsize, size_t *size );


/* WriteAttr
 *  writes NUL-terminated string
 *
 *  "name" [ IN ] - NUL terminated attribute name
 *
 *  "value" [ IN ] - NUL terminated attribute value
 */
KDB_EXTERN rc_t CC KMDataNodeWriteAttr ( KMDataNode *self,
    const char *name, const char *value );


/* ReadAttrAs ( formatted )
 *  reads as integer or float value in native byte order
 *  casts smaller-sized values to desired size, e.g.
 *    uint32_t to uint64_t
 *
 *  "i" [ OUT ] - return parameter for signed integer
 *  "u" [ OUT ] - return parameter for unsigned integer
 *  "f" [ OUT ] - return parameter for double float
 */
KDB_EXTERN rc_t CC KMDataNodeReadAttrAsI16 ( const KMDataNode *self, const char *attr, int16_t *i );
KDB_EXTERN rc_t CC KMDataNodeReadAttrAsU16 ( const KMDataNode *self, const char *attr, uint16_t *u );
KDB_EXTERN rc_t CC KMDataNodeReadAttrAsI32 ( const KMDataNode *self, const char *attr, int32_t *i );
KDB_EXTERN rc_t CC KMDataNodeReadAttrAsU32 ( const KMDataNode *self, const char *attr, uint32_t *u );
KDB_EXTERN rc_t CC KMDataNodeReadAttrAsI64 ( const KMDataNode *self, const char *attr, int64_t *i );
KDB_EXTERN rc_t CC KMDataNodeReadAttrAsU64 ( const KMDataNode *self, const char *attr, uint64_t *u );
KDB_EXTERN rc_t CC KMDataNodeReadAttrAsF64 ( const KMDataNode *self, const char *attr, double *f );


/* Drop
 * VDrop
 *  drop some or all node content
 */
KDB_EXTERN rc_t CC KMDataNodeDropAll ( KMDataNode *self );
KDB_EXTERN rc_t CC KMDataNodeDropAttr ( KMDataNode *self, const char *attr );
KDB_EXTERN rc_t CC KMDataNodeDropChild ( KMDataNode *self, const char *path, ... );
KDB_EXTERN rc_t CC KMDataNodeVDropChild ( KMDataNode *self, const char *path, va_list args );


/* Rename
 *  renames a contained object
 *
 *  "from" [ IN ] - NUL terminated string in UTF-8
 *  giving simple name of existing attr
 *
 *  "to" [ IN ] - NUL terminated string in UTF-8
 *  giving new simple attr name
 */
KDB_EXTERN rc_t CC KMDataNodeRenameAttr ( KMDataNode *self, const char *from, const char *to );
KDB_EXTERN rc_t CC KMDataNodeRenameChild ( KMDataNode *self, const char *from, const char *to );


#ifdef __cplusplus
}
#endif

#endif /*  _h_kdb_meta_ */
