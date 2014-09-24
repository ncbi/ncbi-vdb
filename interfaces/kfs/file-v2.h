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

#ifndef _h_kfs_file_v2_
#define _h_kfs_file_v2_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_kfc_refcount_
#include <kfc/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct timeout_t;


/*--------------------------------------------------------------------------
 * KFile_v2
 *  a file is normally created with a KDirectory
 *  optionally, the standard i/o files may be created directly
 */
typedef struct KFile_v2 KFile_v2;

/* Duplicate
 * Release
 *  ignores NULL references
 */
static __inline__
KFile_v2 * KFileDuplicate_v2 ( const KFile_v2 * self, ctx_t ctx, caps_t rm )
{
    return ( KFile_v2 * ) KRefcountDuplicate_v1 ( TO_REFCOUNT_V1 ( self ), ctx, rm );
}

static __inline__
void KFileRelease_v2 ( const KFile_v2 * self, ctx_t ctx )
{
    KRefcountRelease_v1 ( TO_REFCOUNT_V1 ( self ), ctx );
}


/* RandomAccess
 *  ALMOST by definition, the file is random access
 *
 *  certain file types will refuse random access
 *  these include FIFO and socket based files, but also
 *  wrappers that require serial access ( e.g. compression )
 *
 *  returns 0 if random access, error code otherwise
 */
KFS_EXTERN bool CC KFileRandomAccess_v2 ( const KFile_v2 *self, ctx_t ctx );


/* Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
KFS_EXTERN uint32_t CC KFileType_v2 ( const KFile_v2 *self, ctx_t ctx );


/* Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
KFS_EXTERN uint64_t CC KFileSize_v2 ( const KFile_v2 *self, ctx_t ctx );


/* SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
KFS_EXTERN void CC KFileSetSize_v2 ( KFile_v2 *self, ctx_t ctx, uint64_t size );


/* Read
 * TimedRead
 *  read file from known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - return parameter giving number of bytes
 *  actually read. when returned value is zero and return code is
 *  also zero, interpreted as end of file.
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
KFS_EXTERN size_t CC KFileRead_v2 ( const KFile_v2 *self, ctx_t ctx,
    uint64_t pos, void *buffer, size_t bsize );
KFS_EXTERN size_t CC KFileTimedRead_v2 ( const KFile_v2 *self, ctx_t ctx,
    uint64_t pos, void *buffer, size_t bsize, struct timeout_t *tm );

/* ReadAll
 * TimedReadAll
 *  read from file until "bsize" bytes have been retrieved
 *  or until end-of-input
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - return parameter giving number of bytes
 *  actually read. when returned value is zero and return code is
 *  also zero, interpreted as end of file.
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
KFS_EXTERN size_t CC KFileReadAll_v2 ( const KFile_v2 *self, ctx_t ctx,
    uint64_t pos, void *buffer, size_t bsize );
KFS_EXTERN size_t CC KFileTimedReadAll_v2 ( const KFile_v2 *self, ctx_t ctx,
    uint64_t pos, void *buffer, size_t bsize, struct timeout_t *tm );

/* ReadExactly
 * TimedReadExactly
 *  read from file until "bytes" have been retrieved
 *  or return incomplete transfer error
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bytes" [ IN ] - return buffer for read
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
KFS_EXTERN void CC KFileReadExactly_v2 ( const KFile_v2 *self, ctx_t ctx,
    uint64_t pos, void *buffer, size_t bytes );
KFS_EXTERN void CC KFileTimedReadExactly_v2 ( const KFile_v2 *self, ctx_t ctx,
    uint64_t pos, void *buffer, size_t bytes, struct timeout_t *tm );

/* Write
 * TimedWrite
 *  write file at known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed writes. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
KFS_EXTERN size_t CC KFileWrite_v2 ( KFile_v2 *self, ctx_t ctx,
    uint64_t pos, const void *buffer, size_t size );
KFS_EXTERN size_t CC KFileTimedWrite_v2 ( KFile_v2 *self, ctx_t ctx,
    uint64_t pos, const void *buffer, size_t size, struct timeout_t *tm );

/* WriteAll
 * TimedWriteAll
 *  write to file until "size" bytes have been transferred
 *  or until no further progress can be made
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed writes. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
KFS_EXTERN size_t CC KFileWriteAll_v2 ( KFile_v2 *self, ctx_t ctx,
    uint64_t pos, const void *buffer, size_t size );
KFS_EXTERN size_t CC KFileTimedWriteAll_v2 ( KFile_v2 *self, ctx_t ctx,
    uint64_t pos, const void *buffer, size_t size, struct timeout_t *tm );

/* WriteExactly
 * TimedWriteExactly
 *  write to file until "bytes" have been transferred
 *  or return incomplete transfer error
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ IN ] and "bytes" [ IN ] - data to be written
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed writes. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
KFS_EXTERN void CC KFileWriteExactly_v2 ( KFile_v2 *self, ctx_t ctx,
    uint64_t pos, const void *buffer, size_t bytes );
KFS_EXTERN void CC KFileTimedWriteExactly_v2 ( KFile_v2 *self, ctx_t ctx,
    uint64_t pos, const void *buffer, size_t bytes, struct timeout_t *tm );

/* MakeStdIn
 *  creates a read-only file on stdin
 */
KFS_EXTERN const KFile_v2 * CC KFileMakeStdIn_v2 ( ctx_t ctx );

/* MakeStdOut
 * MakeStdErr
 *  creates a write-only file on stdout or stderr
 */
KFS_EXTERN const KFile_v2 * CC KFileMakeStdOut_v2 ( ctx_t ctx );
KFS_EXTERN const KFile_v2 * CC KFileMakeStdErr_v2 ( ctx_t ctx );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_file_v2_ */
