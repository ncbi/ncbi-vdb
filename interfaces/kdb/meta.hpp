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

#ifndef _hpp_kdb_meta_
#define _hpp_kdb_meta_

#ifndef _h_kdb_meta_
#include <kdb/meta.h>
#endif


/*--------------------------------------------------------------------------
 * KMetadata
 *  a versioned, hierarchical structure
 */
struct KMetadata
{
    /* AddRef
     * Release
     *  all objects are reference counted
     *  NULL references are ignored
     */
    inline rc_t AddRef () const throw()
    { return KMetadataAddRef ( this ); }

    inline rc_t Release () const  throw()
    { return KMetadataRelease ( this ); }


    /* OpenNodeRead
     *  opens a metadata node
     *
     *  "node" [ OUT ] - return parameter for indicated metadata node
     *
     *  "path" [ IN, NULL OKAY ] - optional path for specifying named
     *  node within metadata hierarchy. paths will be interpreted as
     *  if they were file system paths, using '/' as separator. the
     *  special values NULL and "" are interpreted as "."
     */
    inline rc_t OpenNodeRead ( const KMDataNode **node, 
        const char *path, ... ) const throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = KMetadataVOpenNodeRead ( this, node, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenNodeRead ( const KMDataNode **node, 
        const char *path, va_list args ) const throw()
    { return KMetadataVOpenNodeRead ( this, node, path, args ); }


    /* OpenNodeUpdate
     *  opens a metadata node
     *
     *  "node" [ OUT ] - return parameter for indicated metadata node
     *
     *  "path" [ IN, NULL OKAY ] - optional path for specifying named
     *  node within metadata hierarchy. paths will be interpreted as
     *  if they were file system paths, using '/' as separator. the
     *  special values NULL and "" are interpreted as "."
     */
    inline rc_t OpenNodeUpdate ( KMDataNode **node, 
        const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = KMetadataVOpenNodeUpdate ( this, node, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenNodeUpdate ( KMDataNode **node, 
        const char *path, va_list args ) throw()
    { return KMetadataVOpenNodeUpdate ( this, node, path, args ); }


    /* Version
     *  returns the metadata format version
     */
    inline rc_t Version ( uint32_t *version ) const  throw()
    { return KMetadataVersion ( this, version ); }


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
    inline rc_t ByteOrder ( bool *reversed ) const throw()
    { return KMetadataByteOrder ( this, reversed ); }


    /* Revision
     *  returns current revision number
     *  where 0 ( zero ) means tip
     */
    inline rc_t Revision ( uint32_t *revision ) const throw()
    { return KMetadataRevision ( this, revision ); }


    /* MaxRevision
     *  returns the maximum revision available
     */
    inline rc_t MaxRevision ( uint32_t *revision ) const throw()
    { return KMetadataMaxRevision ( this, revision ); }


    /* Commit
     *  ensure any changes are committed to disk
     */
    inline rc_t Commit () throw()
    { return KMetadataCommit ( this ); }


    /* Freeze
     *  freezes current metadata revision
     *  further modification will begin on a copy
     */
    inline rc_t Freeze () throw()
    { return KMetadataFreeze ( this ); }


    /* OpenRevision
     *  opens a read-only indexed revision of metadata
     */
    inline rc_t OpenRevision ( const KMetadata **meta, uint32_t revision ) const throw()
    { return KMetadataOpenRevision ( this, meta, revision ); }


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
    inline rc_t GetSequence ( const char *seq, int64_t *val ) const throw()
    { return KMetadataGetSequence ( this, seq, val ); }

    inline rc_t SetSequence ( const char *seq, int64_t val ) throw()
    { return KMetadataSetSequence ( this, seq, val ); }

    inline rc_t NextSequence ( const char *seq, int64_t *val ) throw()
    { return KMetadataNextSequence ( this, seq, val ); }

private:
    KMetadata ();
    ~ KMetadata ();
    KMetadata ( const KMetadata& );
    KMetadata &operator = ( const KMetadata& );

};


/*--------------------------------------------------------------------------
 * KMDataNode
 *  a node with an optional value,
 *  optional attributes, and optional children
 *
 *  nodes are identified by path, relative to a starting node,
 *  where "/" serves as a path separator.
 */
struct KMDataNode
{
    /* AddRef
     * Release
     *  ignores NULL references
     */
    inline rc_t AddRef () const  throw()
    { return KMDataNodeAddRef ( this ); }

    inline rc_t Release () const  throw()
    { return KMDataNodeRelease ( this ); }


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
    inline rc_t OpenNodeRead ( const KMDataNode **node,
        const char *path, ... ) const throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = KMDataNodeVOpenNodeRead ( this, node, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenNodeRead ( const KMDataNode **node,
        const char *path, va_list args ) const throw()
    { return KMDataNodeVOpenNodeRead ( this, node, path, args ); }


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
    inline rc_t OpenNodeUpdate ( KMDataNode **node,
        const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = KMDataNodeVOpenNodeUpdate ( this, node, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenNodeUpdate ( KMDataNode **node,
        const char *path, va_list args ) throw()
    { return KMDataNodeVOpenNodeUpdate ( this, node, path, args ); }


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
     inline rc_t ByteOrder ( bool *reversed ) const throw()
     { return KMDataNodeByteOrder ( this, reversed ); }


    /* Write
     *  write a node value or attribute
     *  overwrites anything already there
     *
     *  "buffer" [ IN ] and "size" [ IN ] - new value data
     */
     inline rc_t Write ( const void *buffer, size_t size ) throw()
     { return KMDataNodeWrite ( this, buffer, size ); }


     /* Append
      *  append data to value
      *
      *  "buffer" [ IN ] and "size" [ IN ] - value data to be appended
      */
     inline rc_t Append ( const void *buffer, size_t size ) throw()
     { return KMDataNodeAppend ( this, buffer, size ); }


    /* Read ( formatted )
     *  reads as integer or float value in native byte order
     *
     *  "bXX" [ OUT ] - return parameter for numeric value
     */
     inline rc_t ReadB8 ( void *b8 ) const throw()
     { return KMDataNodeReadB8 ( this, b8 ); }
     inline rc_t ReadB16 ( void *b16 ) const throw()
     { return KMDataNodeReadB16 ( this, b16 ); }
     inline rc_t ReadB32 ( void *b32 ) const throw()
     { return KMDataNodeReadB32 ( this, b32 ); }
     inline rc_t ReadB64 ( void *b64 ) const throw()
     { return KMDataNodeReadB64 ( this, b64 ); }
     inline rc_t ReadB128 ( void *b128 ) const throw()
     { return KMDataNodeReadB128 ( this, b128 ); }


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
     inline rc_t Read ( size_t offset, void *buffer, size_t bsize,
        size_t *num_read, size_t *remaining ) const throw()
     { return KMDataNodeRead ( this, offset, buffer, bsize, num_read, remaining ); }


    /* Read ( formatted )
     *  reads as integer or float value in native byte order
     *  casts smaller-sized values to desired size, e.g.
     *    uint32_t to uint64_t
     *
     *  "i" [ OUT ] - return parameter for signed integer
     *  "u" [ OUT ] - return parameter for unsigned integer
     *  "f" [ OUT ] - return parameter for double float
     */
     inline rc_t Read ( int16_t *value ) const  throw()
    { return KMDataNodeReadAsI16 ( this, value ); }
     inline rc_t Read ( uint16_t *value ) const  throw()
    { return KMDataNodeReadAsU16 ( this, value ); }

     inline rc_t Read ( int32_t *value ) const  throw()
    { return KMDataNodeReadAsI32 ( this, value ); }
     inline rc_t Read ( uint32_t *value ) const  throw()
    { return KMDataNodeReadAsU32 ( this, value ); }

    inline rc_t Read ( int64_t *value ) const  throw()
    { return KMDataNodeReadAsI64 ( this, value ); }
    inline rc_t Read ( uint64_t *value ) const  throw()
    { return KMDataNodeReadAsU64 ( this, value ); }

    inline rc_t Read ( double *value ) const  throw()
    { return KMDataNodeReadAsF64 ( this, value ); }


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
    inline rc_t Read ( char *buffer, size_t bsize, size_t *size ) const  throw()
    { return KMDataNodeReadCString ( this, buffer, bsize, size ); }


    /* Write ( formatted )
     *  writes integer or float value in metadata byte order
     *
     *  "bXX" [ IN ] - numeric value
     */
     inline rc_t WriteB8 ( const void *b8 ) throw()
     { return KMDataNodeWriteB8 ( this, b8 ); }
     inline rc_t WriteB16 ( const void *b16 ) throw()
     { return KMDataNodeWriteB16 ( this, b16 ); }
     inline rc_t WriteB32 ( const void *b32 ) throw()
     { return KMDataNodeWriteB32 ( this, b32 ); }
     inline rc_t WriteB64 ( const void *b64 ) throw()
     { return KMDataNodeWriteB64 ( this, b64 ); }
     inline rc_t WriteB128 ( const void *b128 ) throw()
     { return KMDataNodeWriteB128 ( this, b128 ); }


    /* Write ( formatted )
     *  writes string
     *
     *  "str" [ IN ] - NUL terminated string.
     */
     inline rc_t WriteCString ( const char *str ) throw()
     { return KMDataNodeWriteCString ( this, str ); }


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
     inline rc_t ReadAttr ( const char *name, char *buffer, 
        size_t bsize, size_t *size ) const throw()
     { return KMDataNodeReadAttr ( this, name, buffer, bsize, size ); }


    /* WriteAttr
     *  writes NUL-terminated string
     *
     *  "name" [ IN ] - NUL terminated attribute name
     *
     *  "value" [ IN ] - NUL terminated attribute value
     */
    inline rc_t WriteAttr ( const char *name, const char *value )  throw()
    { return KMDataNodeWriteAttr ( this, name, value ); }


    /* ReadAttrAs ( formatted )
     *  reads as integer or float value in native byte order
     *  casts smaller-sized values to desired size, e.g.
     *    uint32_t to uint64_t
     *
     *  "i" [ OUT ] - return parameter for signed integer
     *  "u" [ OUT ] - return parameter for unsigned integer
     *  "f" [ OUT ] - return parameter for double float
     */
     inline rc_t ReadAttrAsI16 ( const char *attr, int16_t *i ) const throw()
     { return KMDataNodeReadAttrAsI16 ( this, attr, i ); }
     inline rc_t ReadAttrAsU16 ( const char *attr, uint16_t *i ) const throw()
     { return KMDataNodeReadAttrAsU16 ( this, attr, i ); }
     inline rc_t ReadAttrAsI32 ( const char *attr, int32_t *i ) const throw()
     { return KMDataNodeReadAttrAsI32 ( this, attr, i ); }
     inline rc_t ReadAttrAsU32 ( const char *attr, uint32_t *i ) const throw()
     { return KMDataNodeReadAttrAsU32 ( this, attr, i ); }
     inline rc_t ReadAttrAsI64 ( const char *attr, int64_t *i ) const throw()
     { return KMDataNodeReadAttrAsI64 ( this, attr, i ); }
     inline rc_t ReadAttrAsU64 ( const char *attr, uint64_t *i ) const throw()
     { return KMDataNodeReadAttrAsU64 ( this, attr, i ); }
     inline rc_t ReadAttrAsF64 ( const char *attr, double *f ) const throw()
     { return KMDataNodeReadAttrAsF64 ( this, attr, f ); }


     /* Drop
     *  drop some or all node content
     */
     inline rc_t DropAll () throw()
     { return KMDataNodeDropAll ( this ); }
     inline rc_t DropAttr ( const char *attr ) throw()
     { return KMDataNodeDropAttr ( this, attr ); }
     inline rc_t DropChild ( const char *path, ... ) throw()
     {
        va_list args;
        va_start ( args, path );
        rc_t rc = KMDataNodeVDropChild ( this, path, args );
        va_end ( args );
        return rc;
     }
     inline rc_t DropChild ( const char *path, va_list args ) throw()
     { return KMDataNodeVDropChild ( this, path, args ); }


    /* Rename
     *  renames a contained object
     *
     *  "from" [ IN ] - NUL terminated string in UTF-8
     *  giving simple name of existing attr
     *
     *  "to" [ IN ] - NUL terminated string in UTF-8
     *  giving new simple attr name
     */
    inline rc_t RenameAttr ( const char *from, const char *to ) throw()
    { return KMDataNodeRenameAttr ( this, from, to ); }
    inline rc_t RenameChild ( const char *from, const char *to ) throw()
    { return KMDataNodeRenameChild ( this, from, to ); }

private:
    KMDataNode ();
    ~ KMDataNode ();
    KMDataNode ( const KMDataNode& );
    KMDataNode &operator = ( const KMDataNode& );
};


#endif // _hpp_kdb_meta_
