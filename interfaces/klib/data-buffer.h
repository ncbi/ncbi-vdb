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

#ifndef _h_klib_data_buffer
#define _h_klib_data_buffer

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KDataBuffer
 *  simple, open structure to reference an opaque data buffer
 *
 *  "base" - pointer to first byte of buffer
 *
 *  "elem_bits" - size of buffer element datatype in bits
 *
 *  "elem_count" - size of buffer in elements
 *
 *  "bit_offset" [ DEFAULT ZERO ] - offset from "base"
 *  to first bit of buffer, always 0 when "elem_bits" % 8 == 0
 *  bits are left-packed, i.e.:
 *
 *    bit_offset | starting bit
 *   ============+=============
 *            0  |  7
 *            1  |  6
 *            2  |  5
 *              ...
 *            6  |  1
 *            7  |  0
 *
 * NB - there may be limits to the total number of bits within the buffer
 *      that may be determined by architecture or runtime environment.
 */
typedef struct KDataBuffer KDataBuffer;
struct KDataBuffer
{
    const void *ignore;
    void *base;
    uint64_t elem_bits;
    uint64_t elem_count;
    uint8_t bit_offset;
};


/* Bits
 *  return buffer size in bits
 */
#define KDataBufferBits( self ) \
    ((((bitsz_t)((const KDataBuffer *)(self))->elem_bits) * ((const KDataBuffer *)(self))->elem_count))

/* Bytes
 *  returns buffer size in bytes
 */
#define KDataBufferBytes( self ) \
   ((size_t)((KDataBufferBits(self) + 7) >> 3))


/* Make
 *  create a new empty buffer
 *
 *  "buffer" [ OUT ] - pointer to structure to initialize
 *
 *  "elem_bits" [ IN ] - the number of bits in each element
 *
 *  "elem_capacity" [ IN ] - the minimum number of elements to be allocated
 */
KLIB_EXTERN rc_t CC KDataBufferMake ( KDataBuffer *buffer,
    uint64_t elem_bits, uint64_t elem_capacity );


/* MakeBytes
 * MakeBits
 *  create a new empty buffer with default element size
 */
#define KDataBufferMakeBytes( buffer, bytes ) \
    KDataBufferMake ( buffer, 8, bytes )
#define KDataBufferMakeBits( buffer, bits ) \
    KDataBufferMake ( buffer, 1, bits )

/* Sub
 *  create a sub-range reference to an existing buffer
 *
 *  "sub" [ OUT ] - pointer to subrange structure
 *
 *  "start" [ IN ] - element offset of subrange from start of "self".
 *  if given start >= self->elem_count, the resultant subrange
 *  will have an element count of 0.
 *
 *  "count" [ IN, DEFAULT UINT64_MAX ] - number of elements of subrange.
 *  when given count exceeds buffer size, the actual count is calculated to be
 *  all remaining count in "self" from "start". otherwise, the
 *  requested count will be limited to the actual count available
 *  in "self".
 */
KLIB_EXTERN rc_t CC KDataBufferSub ( const KDataBuffer *self,
    KDataBuffer *sub, uint64_t start, uint64_t count );


/* MakeWritable
 *  make a writable copy of the buffer, copying contents if needed.
 *
 *  "writable" [ OUT ] - pointer to the structure to initialize
 *
 * The usage pattern is:
 *   KDataBuffer buffer;
 * ...
 * initialize buffer
 * do stuff which might make it shared
 *   (like pass it to a function that might retain it)
 * ...
 *   KDataBuffer writable;
 *   rc_t rc = KDataBufferMakeWritable(&buffer, &writable);
 *
 *   if (rc == 0) {
 *       / * until you whack the old one KDataBufferWritable(&writable) might be false! * /
 *       KDataBufferWhack(&buffer);
 *       buffer = writable;
 * ...
 * do whatever you want with buffer because it is now writable (not shared)
 * ...
 *   }
 *   KDataBufferWhack(&buffer);
 */
KLIB_EXTERN rc_t CC KDataBufferMakeWritable ( const KDataBuffer *self, KDataBuffer *writable );


/* Whack
 *  release memory associated with a buffer.
 */
KLIB_EXTERN rc_t CC KDataBufferWhack ( KDataBuffer *self );


/* Resize
 *  make a buffer bigger or smaller.
 *  can fail if not enough memory.
 *  can fail if not writable.
 *
 *  "new_count" [ IN ] - new number of elements
 */
KLIB_EXTERN rc_t CC KDataBufferResize ( KDataBuffer *self, uint64_t new_count );


/* Cast
 *  create a new data-buffer with a different element size
 *  won't increase the total number of bits
 *
 *  "cast" [ OUT ] - newly typed buffer
 *
 *  "new_elem_bits" [ IN ] - new element size in bits
 *
 *  "can_shrink" [ IN ] - when true allow
 *  KDataBufferBits ( cast ) < KDataBufferBits ( self )
 */
KLIB_EXTERN rc_t CC KDataBufferCast ( const KDataBuffer *self,
    KDataBuffer *cast, uint64_t new_elem_bits, bool can_shrink );


/* Writable
 *  returns true if buffer is valid and writable
 */
KLIB_EXTERN bool CC KDataBufferWritable ( const KDataBuffer *self );


/* CheckIntegrity
 *  performs some level of integrity checking on buffer structure
 */
KLIB_EXTERN rc_t CC KDataBufferCheckIntegrity ( const KDataBuffer *self );


/* Wipe
 *  overwrite allocated memory
 */
KLIB_EXTERN rc_t CC KDataBufferWipe ( KDataBuffer * self );


/* WipeNWhack
 *  wipes and releases memory associated with a buffer.
 */
KLIB_EXTERN rc_t CC KDataBufferWipeNWhack ( KDataBuffer * self );


/* WipeResize
 *  make a buffer bigger or smaller.
 *  can fail if not enough memory.
 *  can fail if not writable.
 *  wipes memory before release or reallocation
 *
 *  "new_count" [ IN ] - new number of elements
 */
KLIB_EXTERN rc_t CC KDataBufferWipeResize ( KDataBuffer * self, uint64_t new_count );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_data_buffer_ */
