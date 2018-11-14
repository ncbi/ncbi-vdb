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

#ifndef _h_kfs_chunk_reader_
#define _h_kfs_chunk_reader_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KChunkReader
 *  an interface for driving chunked responses from a single read
 */
typedef struct KChunkReader KChunkReader;

/* AddRef
 * Release
 *  ignores NULL references
 */
KFS_EXTERN rc_t CC KChunkReaderAddRef ( const KChunkReader * self );
KFS_EXTERN rc_t CC KChunkReaderRelease ( const KChunkReader * self );

/* BufferSize
 *  returns 0 if size is not known or not known to be consistent
 *  returns > 0 if size is known and consistent
 *  returns 0 if "self" is NULL
 */
KFS_EXTERN size_t CC KChunkReaderBufferSize ( const KChunkReader * self );

/* NextBuffer
 *  returns a pointer to and size of the next available buffer for chunk
 *
 *  "buf" [ OUT ] - pointer to chunk buffer
 *
 *  "size" [ OUT ] - size of chunk buffer
 */
KFS_EXTERN rc_t CC KChunkReaderNextBuffer ( KChunkReader * self, void ** buf, size_t * size );

/* ConsumeChunk
 *  delivered to consumer to process the chunk as desired,
 *  e.g. to write to a file or display to a monitor
 *
 *  "pos" [ IN ] - the position of the chunk within the stream or object
 *
 *  "buf" [ IN ] - pointer to buffer
 *
 *  "bytes [ IN ] - the number of valid bytes in buffer; <= size
 */
KFS_EXTERN rc_t CC KChunkReaderConsumeChunk ( KChunkReader * self,
    uint64_t pos, const void * buf, size_t size );

/* ReturnBuffer
 *  allows reader to recycle or free the chunk buffer
 *
 *  "buf" [ IN ] - pointer to chunk buffer
 *
 *  "size" [ IN ] - size of chunk buffer
 */
KFS_EXTERN rc_t CC KChunkReaderReturnBuffer ( KChunkReader * self, void * buf, size_t size );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_chunk_reader_ */
