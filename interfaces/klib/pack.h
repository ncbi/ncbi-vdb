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

#ifndef _h_klib_pack_
#define _h_klib_pack_

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
 * bit-packing operations
 *  packed bits have leading zeros eliminated
 *  unpacked bits have no more information, but may occupy more storage
 *
 *  packed bits are stored in big-endian byte order and big-bit-endian
 *  bit order. unpacked bits are in architecture native order.
 *
 *  little-endian architectures place the least significant information
 *  in the left-most addresses. Intel architecture still has bit ordering
 *  with LSB to the right. the only way to view byte and bit order as
 *  coherent in this architecture is in 2 dimensions, such that bit ordering
 *  within a byte is along one dimension and the bytes are another.
 *
 *  viewing a stream of bits is by definition uni-dimensional, and thus
 *  this code treats bit streams in big-bit-endian order, with the most
 *  significant bit of the most significant byte to the left, and each
 *  bit to the right has decreasing significance.
 *
 *  packed bits are left-aligned, such that an 8=>2 packing of the bytes
 *  [ 1, 2, 3 ] will yield a single byte of 0b01101100 ( 0x6C ).
 *
 *  a pack or unpack operation with identical bit sizes ( e.g. 16=>16 )
 *  will act like a memcpy on big-endian architectures and a byte-swap
 *  on little-endian architectures.
 */


/* Pack
 *  accepts a series of unpacked source bits
 *  produces a series of packed destination bits by eliminating MSB
 *
 *  "unpacked" [ IN ] - original element size in bits
 *  must be in 2^^[3..6], i.e. { 8, 16, 32, 64 }
 *
 *  "packed" [ IN ] - packed element size in bits
 *  must be <= "unpacked"
 *
 *  "src" [ IN ] and "ssize" [ IN ] - source buffer
 *  with size given in bytes
 *
 *  "consumed" [ OUT, NULL OKAY ] - number of source bytes
 *  consumed. if NULL, then all source bytes MUST be consumed
 *  or an error will be generated.
 *
 *  "dst" [ OUT ] and "dst_off" [ IN ] - destination
 *  buffer bit address
 *
 *  "dsize" [ IN ] - size of "dst" in bits, not including "dst_off"
 *
 *  "psize" [ OUT ] - resultant packed size in bits
 *
 * NB - the implementation may allow packing in place
 */
KLIB_EXTERN rc_t CC Pack ( uint32_t unpacked, uint32_t packed,
    const void *src, size_t ssize, size_t *consumed,
    void *dst, bitsz_t dst_off, bitsz_t dsize, bitsz_t *psize );


/* Unpack
 *  accepts a series of packed source bits
 *  produces a series of unpacked destination bits by left-padding zeros
 *
 *  "packed" [ IN ] - packed element size in bits
 *
 *  "unpacked [ IN ] - original element size in bits
 *  must be >= "packed" and
 *  must be in 2^^[3..6], i.e. { 8, 16, 32, 64 }
 *
 *  "src" [ IN ] and "src_off" [ IN ] - source buffer
 *  bit address
 *
 *  "ssize" [ IN ] - number of bits in "src", not including "src_off"
 *
 *  "consumed" [ OUT, NULL OKAY ] - number of source bits
 *  consumed. if NULL, then all source bits MUST be consumed
 *  or an error will be generated.
 *
 *  "dst" [ OUT ] and "dsize" [ IN ] - destination buffer
 *  where size is given in bytes
 *
 *  "usize" [ OUT ] - resultant unpacked size in bytes
 *
 * NB - "src" and "dst" may have same address since
 *  the implementation unpacks from right to left, smaller to larger
 */
KLIB_EXTERN rc_t CC Unpack ( uint32_t packed, uint32_t unpacked,
    const void *src, bitsz_t src_off, bitsz_t ssize, bitsz_t *consumed,
    void *dst, size_t dsize, size_t *usize );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_pack_ */
