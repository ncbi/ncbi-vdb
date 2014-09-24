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

#include <stdint.h>
#include <float.h>
#include <math.h>

#if ENCODING
static unsigned split_and_pack_f32(const float src[], uint32_t n, unsigned m, uint8_t *pexp) {
    const uint32_t mask = ~(0x007FFFFF >> (m - 1));
    uint32_t i; /* src & exponent index */
    uint32_t j; /* mantissa byte offset */
    uint32_t breg = 0;
    int inreg = 0;
    uint8_t *pman = pexp + n;
    
    for (i = 0, j = 0; i != n; ++i) {
        float f = src[i];
        uint32_t man;
        int exp;
        int sgn = 0;
        
        if (f == 0.0) {
            exp = 0;
            man = 0;
        }
        else {
            if (f < 0.0) {
                sgn = 1;
                f = -f;
            }
            man = (uint32_t)ldexpf(frexpf(f, &exp), 24);
            man &= mask;
#if ROUND_TO_NEAREST
            {
                float fl = ldexpf(man, exp - 24);
                float fh = ldexpf(man + (mask & ~(mask << 1)), exp - 24);
                
                if (fh - f < f - fl) {
                    man = (uint32_t)ldexpf(frexpf(fh, &exp), 24);
                    man &= mask;
                }
            }
#endif
            exp += 127;
            man &= 0x7FFFFF;
            if (sgn)
                man |= 0x800000;
            man >>= (24 - m);
        }
        pexp[i] = exp;
        if (inreg + m < 32) {
            breg = (breg << m) | man;
            inreg += m;
        }
        else {
            const int n = 32 - inreg;
            inreg = m - n;
            breg = (breg << n) | (man >> inreg);
            
            pman[j + 0] = breg >> 24;
            pman[j + 1] = breg >> 16;
            pman[j + 2] = breg >>  8;
            pman[j + 3] = breg >>  0;
            j += 4;
            
            breg = man;
        }
    }
    breg <<= 32 - inreg;
    while (inreg > 0) {
        pman[j] = breg >> 24;
        breg <<= 8;
        ++j;
        inreg -= 8;
    }
    return j;
}
#endif /* ENCODING */

#if DECODING
static void unpack_and_join_f32(const uint8_t src[], uint32_t n, unsigned m, float dst[]) {
    uint32_t i;
    uint32_t j;
    uint32_t breg;
    uint32_t inreg;
    
    for (i = 0, j = n, breg = 0, inreg = 0; i != n; ) {
        if (inreg >= m) {
            const uint32_t man = (((breg >> (inreg - m)) << (24 - m)) & 0x00FFFFFF);
            const int exp = src[i];
            
            if (exp != 0 || man != 0) {
                const float f = ldexpf(man | 0x800000, (exp - 127) - 24);
                
                if ((man & 0x800000) != 0)
                    dst[i] = -f;
                else
                    dst[i] = f;
            }
            else
                dst[i] = 0.0;
            inreg -= m;
            ++i;
        }
        else {
            breg = (breg << 8) | src[j];
            inreg += 8;
            ++j;
        }
    }
}
#endif /* DECODING */
