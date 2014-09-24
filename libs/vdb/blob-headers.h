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

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

struct KDataBuffer;
typedef struct BlobHeaders BlobHeaders;

/* add ref and release */
rc_t BlobHeadersAddRef( const BlobHeaders *self );
rc_t BlobHeadersRelease( const BlobHeaders *self );

/* constructors */

/* create a new header stack */
rc_t BlobHeadersCreate( BlobHeaders **lhs );

/* add a new header frame to the stack */
rc_t BlobHeadersCreateChild( BlobHeaders *self, BlobHeaders **child );

/* create header stack from serialization */
rc_t BlobHeadersCreateFromData( BlobHeaders **lhs, const uint8_t *src, uint64_t ssize );

/* move down the header stack
 * you will need to AddRef the result pointer
 * before releasing the argument pointer
 */
const BlobHeaders *BlobHeadersGetNextFrame( const BlobHeaders *cself );

/* this is a convenience function to
 * pop the current frame off the stack
 * returns the new current frame
 *  get next frame
 *  addref next frame
 *  release current frame
 *  return next frame
 *
 * The idea is to do
 *  headers = BlobHeadersPop(headers);
 */
const BlobHeaders *BlobHeadersPop( const BlobHeaders *cself );

/* a worst-case estimate of how many bytes
 * the serialization will take
 */
uint32_t BlobHeadersGetMaxBufferSize( const BlobHeaders *self );

/* serialize the whole stack
 *
 * Params:
 *  self: (in) top of the stack
 *  buffer: (in/out) the data buffer to write into
 *  offset: (in) number of bytes into buffer to start writing at
 */
rc_t BlobHeadersSerialize( BlobHeaders *self, struct KDataBuffer *buffer, uint64_t offset, uint64_t *sz );

/* get the header in the current frame */
/* for read */
struct VBlobHeader *BlobHeadersGetHeader( const BlobHeaders *self );
struct VBlobHeader *BlobHeadersGetHdrWrite( BlobHeaders *self );
struct VBlobHeader *BlobHeadersCreateDummyHeader( uint8_t version, uint32_t fmt, uint8_t flags, uint64_t size );


/* VBlobHeader stuff */
rc_t VBlobHeaderAddRef ( struct VBlobHeader const *self );

#if NOT_DEFINED_IN_VDB_XFORM_H
uint8_t VBlobHeaderVersion( const struct VBlobHeader *self );
void VBlobHeaderSetVersion( struct VBlobHeader *self, uint8_t version );
#endif

uint8_t VBlobHeaderFlags( const struct VBlobHeader *self );
void VBlobHeaderSetFlags( struct VBlobHeader *self, uint8_t flags );

uint32_t VBlobHeaderFormatId( const struct VBlobHeader *self );
void VBlobHeaderSetFormat( struct VBlobHeader *self, uint32_t fmt );

uint64_t VBlobHeaderSourceSize( const struct VBlobHeader *self );
void VBlobHeaderSetSourceSize( struct VBlobHeader *self, uint64_t ssize );


/* Replace
 *  replace contents of "self" with the contents of "src"
 *
 *  after running multiple processes in parallel, one header must
 *  be chosen to associate with the output blob. use this function
 *  to set ...
 */
rc_t VBlobHeaderReplace ( struct VBlobHeader *self, struct VBlobHeader const *src );
