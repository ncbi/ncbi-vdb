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

#include <klib/extern.h>
#include <klib/vlen-encode.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <byteswap.h>
#include <endian.h>
#include <string.h>

LIB_EXPORT rc_t CC vlen_encode1(void *Dst, uint64_t dsize, uint64_t *psize, int64_t X) {
    int sgn = 0;
    uint64_t x;
    uint8_t *dst = Dst;
    
    if (dst == NULL)
        dsize = 0;
    
    if (X < 0) {
        sgn = 0x40;
        x = -X;
    }
    else
        x = X;
    /* 6 -> 8 */
    if (x < 0x40) {
        if (psize)
            *psize = 1;
        if (dsize < 1)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = (uint8_t)sgn | ( (uint8_t)x & 0x3F );
        return 0;
    }
    /* 13 -> 16 */
    if (x < 0x2000) {
        if (psize)
            *psize = 2;
        if (dsize < 2)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | (uint8_t)sgn | ( (uint8_t)( x >> 7 ) & 0x3F );
        dst[1] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 20 -> 24 */
    if (x < 0x100000) {
        if (psize)
            *psize = 3;
        if (dsize < 3)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | (uint8_t)sgn | ( (uint8_t)( x >> 14 ) & 0x3F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[2] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 27 -> 32 */
    if (x < 0x8000000) {
        if (psize)
            *psize = 4;
        if (dsize < 4)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | (uint8_t)sgn | ( (uint8_t)( x >> 21 ) & 0x3F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[3] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 34 -> 40 */
    if (x < ( ( uint64_t ) 0x4 << 32 ) ) {
        if (psize)
            *psize = 5;
        if (dsize < 5)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | (uint8_t)sgn | ( (uint8_t)( x >> 28 ) & 0x3F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[3] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[4] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 41 -> 48 */
    if (x < ( ( uint64_t ) 0x200 << 32 ) ) {
        if (psize)
            *psize = 6;
        if (dsize < 6)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | (uint8_t)sgn | ( (uint8_t)( x >> 35 ) & 0x3F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[3] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[4] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[5] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 48 -> 56 */
    if (x < ( ( uint64_t ) 0x10000 << 32 ) ) {
        if (psize)
            *psize = 7;
        if (dsize < 7)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | (uint8_t)sgn | ( (uint8_t)( x >> 42 ) & 0x3F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 35 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
        dst[3] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[4] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[5] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[6] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 55 -> 64 */
    if (x < ( ( uint64_t ) 0x800000 << 32 ) ) {
        if (psize)
            *psize = 8;
        if (dsize < 8)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | (uint8_t)sgn | ( (uint8_t)( x >> 49 ) & 0x3F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 42 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 35 ) & 0x7F );
        dst[3] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
        dst[4] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[5] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[6] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[7] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 62 -> 72 */
    if (x < ( ( uint64_t ) 0x40000000 << 32 ) ) {
        if (psize)
            *psize = 9;
        if (dsize < 9)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | (uint8_t)sgn | ( (uint8_t)( x >> 56 ) & 0x3F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 49 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 42 ) & 0x7F );
        dst[3] = 0x80 | ( (uint8_t)( x >> 35 ) & 0x7F );
        dst[4] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
        dst[5] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[6] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[7] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[8] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 64 -> 80 */
    if (psize)
        *psize = 10;
    if (dsize < 10)
        return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
    dst[0] = 0x80 | (uint8_t)sgn | ( (uint8_t)( x >> 63 ) & 0x3F );
    dst[1] = 0x80 | ( (uint8_t)( x >> 56 ) & 0x7F );
    dst[2] = 0x80 | ( (uint8_t)( x >> 49 ) & 0x7F );
    dst[3] = 0x80 | ( (uint8_t)( x >> 42 ) & 0x7F );
    dst[4] = 0x80 | ( (uint8_t)( x >> 35 ) & 0x7F );
    dst[5] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
    dst[6] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
    dst[7] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
    dst[8] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
    dst[9] = ( (uint8_t)x & 0x7F );
    return 0;
}

LIB_EXPORT rc_t CC vlen_decode1 ( int64_t *dst, const void *Src, uint64_t ssize, uint64_t *consumed ) {
    const uint8_t *src = Src;
    int x;
    int i;
    int sgn;
    int64_t y;

    if (dst == NULL || src == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcNull);
    if (ssize == 0)
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcInvalid);
    if (ssize > 10)
        ssize = 10;
    
    x = src[0];
    sgn = x & 0x40;
    y = x & 0x3F;
    i = 1;
    if ((x & 0x80) != 0) {
        if (i == ssize)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
        x = src[i];
        y = (y << 7) | (x & 0x7F);
        ++i;
        if ((x & 0x80) != 0) {
            if (i == ssize)
                return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
            x = src[i];
            y = (y << 7) | (x & 0x7F);
            ++i;
            if ((x & 0x80) != 0) {
                if (i == ssize)
                    return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
                x = src[i];
                y = (y << 7) | (x & 0x7F);
                ++i;
                if ((x & 0x80) != 0) {
                    if (i == ssize)
                        return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
                    x = src[i];
                    y = (y << 7) | (x & 0x7F);
                    ++i;
                    if ((x & 0x80) != 0) {
                        if (i == ssize)
                            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
                        x = src[i];
                        y = (y << 7) | (x & 0x7F);
                        ++i;
                        if ((x & 0x80) != 0) {
                            if (i == ssize)
                                return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
                            x = src[i];
                            y = (y << 7) | (x & 0x7F);
                            ++i;
                            if ((x & 0x80) != 0) {
                                if (i == ssize)
                                    return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
                                x = src[i];
                                y = (y << 7) | (x & 0x7F);
                                ++i;
                                if ((x & 0x80) != 0) {
                                    if (i == ssize)
                                        return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
                                    x = src[i];
                                    y = (y << 7) | (x & 0x7F);
                                    ++i;
                                    if ((x & 0x80) != 0) {
                                        if (i == ssize)
                                            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
                                        x = src[i];
                                        y = (y << 7) | (x & 0x7F);
                                        ++i;
                                        if ((x & 0x80) != 0)
                                            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    *dst = sgn ? -y : y;
    if (consumed)
        *consumed = i;
    return 0;
}

LIB_EXPORT rc_t CC vlen_encode ( void *Dst, uint64_t dsize, uint64_t *act_size,
    const int64_t x[], uint32_t xcount ) {
    uint32_t i;
    uint64_t n;
    uint64_t m = 0;
    rc_t rc = 0;
    uint8_t *dst = Dst;
    
    if (dst != NULL) {
        for (i = 0, rc = 0; rc == 0 && i != xcount; ++i) {
            rc = vlen_encode1(dst, dsize, &n, x[i]);
            dst += n;
            dsize -= n;
            m += n;
        }
        if (rc == 0)
            goto DONE;
    }
    else
        i = 0;
    for (; i != xcount; ++i) {
        vlen_encode1(0, 0, &n, x[i]);
        m += n;
    }
DONE:
    if (act_size)
        *act_size = m;
    return rc;
}

LIB_EXPORT rc_t CC vlen_decode ( int64_t *Y, uint64_t ycount, 
    const void *Src, uint64_t ssize, uint64_t *consumed ) {
    const uint8_t *src = Src;
    uint64_t i, j;
    
    if (Y == NULL || Src == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcNull);
    if (ssize == 0)
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcInvalid);
    if (ssize < ycount)
        return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
    
    for (i = 0, j = 0; j != ycount && i + 10 < ssize; ++j) {
        int64_t y;
        int sgn;
#define XTYPE_SIZE 64
#if XTYPE_SIZE == 64
#define XTYPE uint64_t
#elif XTYPE_SIZE == 32
#define XTYPE uint32_t
#else
#endif
        XTYPE x;
        
        memmove (& x, src + i, sizeof x); /* may be misaligned */        
#if __BYTE_ORDER == __BIG_ENDIAN
#if XTYPE_SIZE == 32
        x = bswap_32(x);
#elif XTYPE_SIZE == 64
        x = bswap_64(x);
#endif
#endif
        sgn = (int)x & 0x40;
        y = x & 0x3F;
        ++i;
        if ((x & 0x80) != 0) {
            x >>= 8;
            y = (y << 7) | (x & 0x7F);
            ++i;
            if ((x & 0x80) != 0) {
                x >>= 8;
                y = (y << 7) | (x & 0x7F);
                ++i;
                if ((x & 0x80) != 0) {
                    x >>= 8;
                    y = (y << 7) | (x & 0x7F);
                    ++i;
                    if ((x & 0x80) != 0) {
#if XTYPE_SIZE == 32
                        x = *(const XTYPE *)(src + i);
#if __BYTE_ORDER == __BIG_ENDIAN
                        x = bswap_32(x);
#endif
#elif XTYPE_SIZE == 64
                        x >>= 8;
#endif
                        y = (y << 7) | (x & 0x7F);
                        ++i;
                        if ((x & 0x80) != 0) {
                            x >>= 8;
                            y = (y << 7) | (x & 0x7F);
                            ++i;
                            if ((x & 0x80) != 0) {
                                x >>= 8;
                                y = (y << 7) | (x & 0x7F);
                                ++i;
                                if ((x & 0x80) != 0) {
                                    x >>= 8;
                                    y = (y << 7) | (x & 0x7F);
                                    ++i;
                                    if ((x & 0x80) != 0) {
                                        x = src[i];
                                        y = (y << 7) | (x & 0x7F);
                                        ++i;
                                        if ((x & 0x80) != 0) {
                                            x = src[i];
                                            y = (y << 7) | (x & 0x7F);
                                            ++i;
                                            if ((x & 0x80) != 0)
                                                return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        Y[j] = sgn ? -y : y;
    }
    for ( ; j != ycount; ++j) {
        uint64_t n;
        rc_t rc = vlen_decode1(Y + j, src + i, ssize - i, &n);
        if (rc)
            return rc;
        i += n;
    }
    if (consumed)
        *consumed = i;
    return 0;
}

LIB_EXPORT rc_t CC vlen_encodeU1 ( void *Dst, uint64_t dsize,
    uint64_t *psize, uint64_t X ) {
    uint64_t x = X;
    uint8_t *dst = Dst;
    
    if (dst == NULL)
        dsize = 0;
    
    /* 7 -> 8 */
    if (x < 0x80) {
        if (psize)
            *psize = 1;
        if (dsize < 1)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = (uint8_t)x & 0x7F;
        return 0;
    }
    /* 14 -> 16 */
    if (x < 0x4000) {
        if (psize)
            *psize = 2;
        if (dsize < 2)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F);
        dst[1] = (uint8_t)x & 0x7F;
        return 0;
    }
    /* 21 -> 24 */
    if (x < 0x200000) {
        if (psize)
            *psize = 3;
        if (dsize < 3)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[2] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 28 -> 32 */
    if (x < 0x10000000) {
        if (psize)
            *psize = 4;
        if (dsize < 4)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[3] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 35 -> 40 */
    if (x < ( ( uint64_t ) 0x8 << 32 ) ) {
        if (psize)
            *psize = 5;
        if (dsize < 5)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[3] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[4] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 42 -> 48 */
    if (x < ( ( uint64_t ) 0x400 << 32 ) ) {
        if (psize)
            *psize = 6;
        if (dsize < 6)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | ( (uint8_t)( x >> 35 ) & 0x7F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[3] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[4] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[5] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 49 -> 56 */
    if (x < ( ( uint64_t ) 0x20000 << 32 ) ) {
        if (psize)
            *psize = 7;
        if (dsize < 7)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | ( (uint8_t)( x >> 42 ) & 0x7F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 35 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
        dst[3] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[4] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[5] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[6] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 56 -> 64 */
    if (x < ( ( uint64_t ) 0x1000000 << 32 ) ) {
        if (psize)
            *psize = 8;
        if (dsize < 8)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | ( (uint8_t)( x >> 49 ) & 0x7F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 42 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 35 ) & 0x7F );
        dst[3] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
        dst[4] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[5] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[6] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[7] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 63 -> 72 */
    if (x < ( ( uint64_t ) 0x80000000 << 32 ) ) {
        if (psize)
            *psize = 9;
        if (dsize < 9)
            return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
        dst[0] = 0x80 | ( (uint8_t)( x >> 56 ) & 0x7F );
        dst[1] = 0x80 | ( (uint8_t)( x >> 49 ) & 0x7F );
        dst[2] = 0x80 | ( (uint8_t)( x >> 42 ) & 0x7F );
        dst[3] = 0x80 | ( (uint8_t)( x >> 35 ) & 0x7F );
        dst[4] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
        dst[5] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
        dst[6] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
        dst[7] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
        dst[8] = ( (uint8_t)x & 0x7F );
        return 0;
    }
    /* 64 -> 80 */
    if (psize)
        *psize = 10;
    if (dsize < 10)
        return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
    dst[0] = 0x80 | ( (uint8_t)( x >> 63 ) & 0x7F );
    dst[1] = 0x80 | ( (uint8_t)( x >> 56 ) & 0x7F );
    dst[2] = 0x80 | ( (uint8_t)( x >> 49 ) & 0x7F );
    dst[3] = 0x80 | ( (uint8_t)( x >> 42 ) & 0x7F );
    dst[4] = 0x80 | ( (uint8_t)( x >> 35 ) & 0x7F );
    dst[5] = 0x80 | ( (uint8_t)( x >> 28 ) & 0x7F );
    dst[6] = 0x80 | ( (uint8_t)( x >> 21 ) & 0x7F );
    dst[7] = 0x80 | ( (uint8_t)( x >> 14 ) & 0x7F );
    dst[8] = 0x80 | ( (uint8_t)( x >> 7 ) & 0x7F );
    dst[9] = ( (uint8_t)x & 0x7F );
    return 0;
}

static
rc_t vlen_decodeU1_imp ( uint64_t *dst, const void *Src,
    uint64_t ssize, uint64_t *consumed ) {
    const uint8_t *src = Src;
    uint64_t y;
    int c;
    
    if (ssize > 10)
        ssize = 10;
    
    c = src[0];
    if ((c & 0x80) == 0) {
        *dst = c;
        *consumed = 1;
        return 0;
    }
    if (ssize != 1) {
        y = c & 0x7F;
        c = src[1];
        if ((c & 0x80) == 0) {
            *dst = (y << 7) | c;
            *consumed = 2;
            return 0;
        }
        if (ssize != 2) {
            y = (y << 7) | (c & 0x7F);
            c = src[2];
            if ((c & 0x80) == 0) {
                *dst = (y << 7) | c;
                *consumed = 3;
                return 0;
            }
            if (ssize != 3) {
                y = (y << 7) | (c & 0x7F);
                c = src[3];
                if ((c & 0x80) == 0) {
                    *dst = (y << 7) | c;
                    *consumed = 4;
                    return 0;
                }
                if (ssize != 4) {
                    y = (y << 7) | (c & 0x7F);
                    c = src[4];
                    if ((c & 0x80) == 0) {
                        *dst = (y << 7) | c;
                        *consumed = 5;
                        return 0;
                    }
                    if (ssize != 5) {
                        y = (y << 7) | (c & 0x7F);
                        c = src[5];
                        if ((c & 0x80) == 0) {
                            *dst = (y << 7) | c;
                            *consumed = 6;
                            return 0;
                        }
                        if (ssize != 6) {
                            y = (y << 7) | (c & 0x7F);
                            c = src[6];
                            if ((c & 0x80) == 0) {
                                *dst = (y << 7) | c;
                                *consumed = 7;
                                return 0;
                            }
                            if (ssize != 7) {
                                y = (y << 7) | (c & 0x7F);
                                c = src[7];
                                if ((c & 0x80) == 0) {
                                    *dst = (y << 7) | c;
                                    *consumed = 8;
                                    return 0;
                                }
                                if (ssize != 8) {
                                    y = (y << 7) | (c & 0x7F);
                                    c = src[8];
                                    if ((c & 0x80) == 0) {
                                        *dst = (y << 7) | c;
                                        *consumed = 9;
                                        return 0;
                                    }
                                    if (ssize != 9) {
                                        y = (y << 7) | (c & 0x7F);
                                        c = src[9];
                                        if ((c & 0x80) == 0) {
                                            *dst = (y << 7) | c;
                                            *consumed = 10;
                                            return 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
                                            
    return RC(rcRuntime, rcFunction, rcExecuting, rcData, rcInvalid);
}

LIB_EXPORT rc_t CC vlen_decodeU1( uint64_t *dst, const void *Src,
    uint64_t ssize, uint64_t *consumed ) {
    if (dst == NULL || Src == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcNull);
    if (ssize == 0)
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcInvalid);
    
    return vlen_decodeU1_imp(dst, Src, ssize, consumed);
}

LIB_EXPORT rc_t CC vlen_encodeU( void *Dst, uint64_t dsize,
    uint64_t *act_size, const uint64_t x[], uint32_t xcount ) {
    uint32_t i;
    uint64_t n;
    uint64_t m = 0;
    rc_t rc = 0;
    uint8_t *dst = Dst;
    
    if (dst != NULL) {
        for (i = 0, rc = 0; rc == 0 && i != xcount; ++i) {
            rc = vlen_encodeU1(dst, dsize, &n, x[i]);
            dst += n;
            dsize -= n;
            m += n;
        }
        if (rc == 0)
            goto DONE;
    }
    else
        i = 0;
    for (; i != xcount; ++i) {
        vlen_encodeU1(0, 0, &n, x[i]);
        m += n;
    }
DONE:
    if (act_size)
        *act_size = m;
    return rc;
}

LIB_EXPORT rc_t CC vlen_decodeU ( uint64_t Y[], uint64_t ycount,
    const void *Src, uint64_t ssize, uint64_t *consumed ) {
    const uint8_t *src = Src;
    uint64_t i, j;
    
    if (Y == NULL || Src == NULL)
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcNull);
    if (ssize == 0)
        return RC(rcXF, rcFunction, rcExecuting, rcParam, rcInvalid);
    if (ssize < ycount)
        return RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
    
    for (j = i = 0; j != ycount; ++j) {
        uint64_t n;
        rc_t rc = vlen_decodeU1_imp(Y + j, src + i, ssize - i, &n);
        if (rc)
            return rc;
        i += n;
    }
    if (consumed)
        *consumed = i;
    return 0;
}

#if 0
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void print ( const unsigned char *buf, int n ) {
    int i;
    for (i = 0; i != n; ++i)
        printf("%02X ", buf[i]);
    puts("");
}

#define N 128
void test( void ) {
    unsigned char buf[N * 10];
    uint32_t i;
    uint64_t y[N], x[N];
    unsigned s[8];
    uint64_t bsz;
    uint64_t temp;
    uint32_t total;
    uint64_t min = 640;
    uint64_t max = 0;
    int j;
    
    for (i = 0; i != 8; ++i) {
        sranddev();
        s[i] = random();
    }
    for (j = 0, total = 0; j != 1024 * 1024; ++j) {
        for (i = 0; i != N; ++i)
            x[i] = random();
        vlen_encodeU(buf, 1024, &bsz, x, N);
        total += bsz;
        if (max < bsz)
            max = bsz;
        if (min > bsz)
            min = bsz;
        vlen_decodeU(y, N, buf, bsz, &temp);
        assert(temp == bsz);
        assert(memcmp(y, x, sizeof(y)) == 0);
    }
    for (j = 0, total = 0; j != 1024 * 1024; ++j) {
        for (i = 0; i != N; ++i)
            x[i] = random();
        vlen_encode(buf, 1024, &bsz, x, N);
        total += bsz;
        if (max < bsz)
            max = bsz;
        if (min > bsz)
            min = bsz;
        vlen_decode(y, N, buf, bsz, &temp);
        assert(temp == bsz);
        assert(memcmp(y, x, sizeof(y)) == 0);
    }
    printf("min: %u\tmax: %u\tavg: %f\n", (int)min, (int)max, ((float)total) / j);
}
#endif
