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

#ifndef _h_klib_vlen_encode_
#define _h_klib_vlen_encode_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * This encoding is very similar to X.690's BER and/or Perl's pack('w').
 * It differs in that it handles signed values.
 * It uses bit 6 (of 7..0) of the first byte to store the sign.
 * The layout of bits from 7 to 0, from MSB to LSB is
 *  CSXXXXXX(CXXXXXXX)*
 * where:
 *  C is the continuation flag
 *  S is the sign flag, and
 *  X are the digits of the absolute value of the encoded number.
 * Thus a number x: |x| <
 *  0x40 is encoded in 1 byte,
 *  0x2000 in 2,
 *  0x80000000000000 (i.e. a 55 bit number) in 8, etc.
 * The worst case requires 10 bytes to encode.
 */

/*****************************************************************************
 * encode 1 value into buffer
 *
 * Parameters:
 *  dst, dsize: buffer to write into of length dsize, can be null, 0
 *  act_size: (optional) number of bytes needed to encode
 *  x: the value to encode
 */
KLIB_EXTERN rc_t CC vlen_encode1 ( void *dst, uint64_t dsize, uint64_t *act_size, int64_t x );

/*****************************************************************************
 * encode array into buffer
 *
 * Parameters:
 *  dst, dsize: buffer to write into of length dsize, can be null, 0
 *  act_size: (optional) number of bytes needed to encode
 *  x, xcount: the array of xcount elements to encode
 */
KLIB_EXTERN rc_t CC vlen_encode ( void *dst, uint64_t dsize, uint64_t *act_size, 
        const int64_t x[], uint32_t xcount );

/*****************************************************************************
 * decode 1 value from buffer
 *
 * Parameters:
 *  y: result
 *  src, ssize: buffer to read from of length ssize
 *  consumed: (optional) number of bytes used from src
 */
KLIB_EXTERN rc_t CC vlen_decode1 ( int64_t *y, const void *src,
        uint64_t ssize, uint64_t *consumed );

/*****************************************************************************
 * decode array from buffer
 *
 * Parameters:
 *  y, count: result array of ycount elements
 *  src, ssize: buffer to read from of length ssize
 *  consumed: (optional) number of bytes used from src
 */
KLIB_EXTERN rc_t CC vlen_decode ( int64_t *y, uint64_t ycount, const void *src,
        uint64_t ssize, uint64_t *consumed );


/*****************************************************************************
 * This encoding is very similar to X.690's BER and/or Perl's pack('w').
 * The layout of bits from 7 to 0, from MSB to LSB is
 *  CXXXXXXX(CXXXXXXX)*
 * where:
 *  C is the continuation flag
 *  X are the digits of the encoded number.
 * The worst case requires 10 bytes to encode.
 */

/*****************************************************************************
 * encode 1 value into buffer
 *
 * Parameters:
 *  dst, dsize: buffer to write into of length dsize, can be null, 0
 *  act_size: (optional) number of bytes needed to encode
 *  x: the value to encode
 */
KLIB_EXTERN rc_t CC vlen_encodeU1 ( void *dst, uint64_t dsize,
        uint64_t *act_size, uint64_t x );

/*****************************************************************************
 * encode array into buffer
 *
 * Parameters:
 *  dst, dsize: buffer to write into of length dsize, can be null, 0
 *  act_size: (optional) number of bytes needed to encode
 *  x, xcount: the array of xcount elements to encode
 */
KLIB_EXTERN rc_t CC vlen_encodeU ( void *dst, uint64_t dsize, uint64_t *act_size,
        const uint64_t x[], uint32_t xcount );

/*****************************************************************************
 * decode 1 value from buffer
 *
 * Parameters:
 *  y: result
 *  src, ssize: buffer to read from of length ssize
 *  consumed: (optional) number of bytes used from src
 */
KLIB_EXTERN rc_t CC vlen_decodeU1 ( uint64_t *y, const void *src, 
        uint64_t ssize, uint64_t *consumed );

/*****************************************************************************
 * decode array from buffer
 *
 * Parameters:
 *  y, count: result array of ycount elements
 *  src, ssize: buffer to read from of length ssize
 *  consumed: (optional) number of bytes used from src
 */
KLIB_EXTERN rc_t CC vlen_decodeU ( uint64_t *y, uint64_t ycount, const void *src, 
        uint64_t ssize, uint64_t *consumed );

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_vlen_encode_ */
