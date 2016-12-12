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

#ifndef _h_kns_stream_
#define _h_kns_stream_

#ifndef _h_kns_extern_
#include <kns/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct timeout_t;


/*--------------------------------------------------------------------------
 * KStream
 *  the stream is defined to have no concept of size,
 *  and to not support any form of random access
 */
typedef struct KStream KStream;


/* AddRef
 * Release
 *  ignores NULL references
 */
KNS_EXTERN rc_t CC KStreamAddRef ( const KStream *self );
KNS_EXTERN rc_t CC KStreamRelease ( const KStream *self );


/* Read
 * TimedRead
 *  read data from stream
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - return parameter giving number of bytes
 *  actually read. when returned value is zero and return code is
 *  also zero, interpreted as end of stream.
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
KNS_EXTERN rc_t CC KStreamRead ( const KStream *self,
    void *buffer, size_t bsize, size_t *num_read );
KNS_EXTERN rc_t CC KStreamTimedRead ( const KStream *self,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm );

/* ReadAll
 * TimedReadAll
 *  read from stream until "bsize" bytes have been retrieved
 *  or until end-of-input
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - return parameter giving number of bytes
 *  actually read. when returned value is zero and return code is
 *  also zero, interpreted as end of stream.
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
KNS_EXTERN rc_t CC KStreamReadAll ( const KStream *self,
    void *buffer, size_t bsize, size_t *num_read );
KNS_EXTERN rc_t CC KStreamTimedReadAll ( const KStream *self,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm );

/* ReadExactly
 * TimedReadExactly
 *  read from stream until "bytes" have been retrieved
 *  or return incomplete transfer
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
KNS_EXTERN rc_t CC KStreamReadExactly ( const KStream *self,
    void *buffer, size_t bytes );
KNS_EXTERN rc_t CC KStreamTimedReadExactly ( const KStream *self,
    void *buffer, size_t bytes, struct timeout_t *tm );


/* Write
 * TimedWrite
 *  send data to stream
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
KNS_EXTERN rc_t CC KStreamWrite ( KStream *self,
    const void *buffer, size_t size, size_t *num_writ );
KNS_EXTERN rc_t CC KStreamTimedWrite ( KStream *self,
    const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm );

/* WriteAll
 * TimedWriteAll
 *  write to stream until "size" bytes have been transferred
 *  or until no further progress can be made
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
KNS_EXTERN rc_t CC KStreamWriteAll ( KStream *self,
    const void *buffer, size_t size, size_t *num_writ );
KNS_EXTERN rc_t CC KStreamTimedWriteAll ( KStream *self,
    const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm );

/* WriteExactly
 * TimedWriteExactly
 *  write to stream until "bytes" have been transferred
 *  or return incomplete transfer error
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
KNS_EXTERN rc_t CC KStreamWriteExactly ( KStream *self,
    const void *buffer, size_t bytes );
KNS_EXTERN rc_t CC KStreamTimedWriteExactly ( KStream *self,
    const void *buffer, size_t bytes, struct timeout_t *tm );


/* MakeStdIn
 *  creates a read-only stream on stdin
 */
KNS_EXTERN rc_t CC KStreamMakeStdIn ( const KStream **std_in );

/* MakeStdOut
 * MakeStdErr
 *  creates a write-only stream on stdout or stderr
 */
KNS_EXTERN rc_t CC KStreamMakeStdOut ( KStream **std_out );
KNS_EXTERN rc_t CC KStreamMakeStdErr ( KStream **std_err );


/* MakeBuffered
 *  makes a one or two-way stream buffer
 *  either "in" or "out" may be NULL, but not both
 *  if neither are NULL, then the stream is two-way
 *
 *  each non-NULL stream will get a fixed-sized buffer
 *  of the size indicated, or default value if size == 0
 */
KNS_EXTERN rc_t CC KStreamMakeBuffered ( KStream ** buffered,
    const KStream * in, KStream * out, size_t bufer_size );


/* MakeFromBuffer
 *  makes "stream" from provided "buffer" of "size" bytes
 */
KNS_EXTERN rc_t CC KStreamMakeFromBuffer ( KStream ** stream,
    const char * buffer, size_t size );


#ifdef __cplusplus
}
#endif

#endif /* _h_kns_stream_ */
