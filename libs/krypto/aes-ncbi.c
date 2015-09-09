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
 */


#include <krypto/extern.h>

#include "ncbi-priv.h"
#include "aes-ncbi-priv.h"
#include "cipher-priv.h"
#include "blockcipher-priv.h"

#include <klib/debug.h>
#include <klib/out.h>
#include <klib/rc.h>


#include <byteswap.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sysalloc.h>

#if USE_VEC
#include <cpuid.h>
#endif

#if USE_VECREG
#include <v128.h>
#endif

/*
 * This implements the AES cipher as defined by FIPS-197 from NIST
 *  
 * Notes on implmentation:
 *
 * NOTE:
 * Byte ordering in AES is not critical but unllike the original implmentation
 * bytes are not re-ordered to a big endian order upon reading them in and this
 * code will expect to run on an Intel/AMD type processor and might have issues
 * on some big endian processors.
 *
 * NOTE:
 * Many functions are written as static inline functions to
 * facilitate GCC type assembly language optimization on various processors
 *
 * NOTE:
 * This implements the Equivalent Inverse Cipher not the Inverse Cipher.  In
 * general functions will match those defined in the FIPS-197 document where they
 * do not get optimized away for Intel/AMD XMM registers and AES-NI instructions.
 *
 * NOTE:
 * C-99 types are used for specific sized types.  An unaddornded unsigned type
 * is used for many index type variables to allow the compiler choice in
 * size where size does not matter very much.  As these index variables do
 * not go above 16 any size would do but the compiler can choose the size most
 * suited for array subscripting.
 */

static const
CipherVec vec_00 =  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const
CipherVec vec_01 =  { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                      0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };

static const
CipherVec vec_0F =  { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
                      0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F };

static const
CipherVec vec_10 =  { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                      0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };



/* ======================================================================
 * These functions implment the Advanced Encryption Standard AES as defined
 * in the FIPS (Federal Information Processing Standars Publication 197
 * Of Nevember 26, 2001.
 */

/* ======================================================================
 * FIPS-197 5.1
 */

/*
 * Read a blocks worth of bytes into an CipherVec
 *
 * With vector registers it is a single instruction, without them it
 * is a memmove call
 */
static __inline__ 
CipherVec AESBCMEMBER(StateIn) (const void * ain)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(StateIn) (const void * ain)
{
#if USE_VEC_REG

    register CipherVec vec;
    __asm__ (
        "movdqu (%[a]),%[s]" : [s] "=x" (vec) : [a] "D" (ain)
        );
    return vec;

#else

    CipherVec_AES_u u;
    memmove (&u.bytes, ain, sizeof (u));
    return u.state;

#endif
}


/*
 * Write a block's worth of bytes out from an CipherVec
 *
 * With vector registers it is a single instruction, without them it
 * is a memmove call
 */
static __inline__
void AESBCMEMBER(StateOut) (const CipherVec vec, void * rout)
    __attribute__ ((always_inline));

static __inline__
void AESBCMEMBER(StateOut) (const CipherVec vec, void * rout)
{
#if USE_VEC_REG

    register CipherVec rvec = vec;
    __asm__ (
        "movdqu %[s],(%[a])" : : [s] "x" (rvec), [a] "D" (rout)
        );

#else

    memmove (rout, &vec, sizeof (vec));

#endif
}


static __inline__ 
CipherVec AESBCMEMBER(VecXor) (CipherVec v1, CipherVec v2)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(VecXor) (CipherVec v1, CipherVec v2)
{
#if USE_VEC

    return v1 ^ v2;

#else

    CipherVec_AES_u u1, u2; /* state and key not countries :) */
    unsigned ix;

    u1.state = v1;
    u2.state = v2;

    for (ix = 0; ix < AES_Nb; ++ix)
        u1.columns[ix] ^= u2.columns[ix];

    return u1.state;

#endif
}


static __inline__ 
CipherVec AESBCMEMBER(VecAnd) (CipherVec v1, CipherVec v2)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(VecAnd) (CipherVec v1, CipherVec v2)
{
#if USE_VEC

    return v1 & v2;

#else

    CipherVec_AES_u u1, u2; /* state and key not countries :) */
    unsigned ix;

    u1.state = v1;
    u2.state = v2;

    for (ix = 0; ix < AES_Nb; ++ix)
        u1.columns[ix] &= u2.columns[ix];

    return u1.state;

#endif
}


static __inline__ 
CipherVec AESBCMEMBER(VecOr) (CipherVec v1, CipherVec v2)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(VecOr) (CipherVec v1, CipherVec v2)
{
#if USE_VEC

    return v1 | v2;

#else

    CipherVec_AES_u u1, u2; /* state and key not countries :) */
    unsigned ix;

    u1.state = v1;
    u2.state = v2;

    for (ix = 0; ix < AES_Nb; ++ix)
        u1.columns[ix] |= u2.columns[ix];

    return u1.state;

#endif
}


static __inline__ 
CipherVec AESBCMEMBER(VecAdd) (CipherVec v1, CipherVec v2)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(VecAdd) (CipherVec v1, CipherVec v2)
{
#if USE_VEC

    return v1 + v2;

#else

    CipherVec_AES_u u1, u2; /* state and key not countries :) */
    unsigned ix;

    u1.state = v1;
    u2.state = v2;

    for (ix = 0; ix < AES_Nb; ++ix)
        u1.columns[ix] += u2.columns[ix];

    return u1.state;

#endif
}


static __inline__ 
CipherVec AESBCMEMBER(VecSub) (CipherVec v1, CipherVec v2)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(VecSub) (CipherVec v1, CipherVec v2)
{
#if USE_VEC

    return v1 - v2;

#else

    CipherVec_AES_u u1, u2; /* state and key not countries :) */
    unsigned ix;

    u1.state = v1;
    u2.state = v2;

    for (ix = 0; ix < AES_Nb; ++ix)
        u1.columns[ix] -= u2.columns[ix];

    return u1.state;

#endif
}


static __inline__ 
CipherVec AESBCMEMBER(VecMul) (CipherVec v1, CipherVec v2)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(VecMul) (CipherVec v1, CipherVec v2)
{
#if USE_VEC

    return v1 * v2;

#else

    CipherVec_AES_u u1, u2; /* state and key not countries :) */
    unsigned ix;

    u1.state = v1;
    u2.state = v2;

    for (ix = 0; ix < AES_Nb; ++ix)
        u1.columns[ix] *= u2.columns[ix];

    return u1.state;

#endif
}


static __inline__ 
CipherVec AESBCMEMBER(VecDiv) (CipherVec v1, CipherVec v2)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(VecDiv) (CipherVec v1, CipherVec v2)
{
#if USE_VEC

    return v1 / v2;

#else

    CipherVec_AES_u u1, u2; /* state and key not countries :) */
    unsigned ix;

    u1.state = v1;
    u2.state = v2;

    for (ix = 0; ix < AES_Nb; ++ix)
        u1.columns[ix] /= u2.columns[ix];

    return u1.state;

#endif
}

static __inline__ 
CipherVec AESBCMEMBER(PackShuffleBytes) (CipherVec dest, CipherVec mask)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(PackShuffleBytes) (CipherVec dest, CipherVec mask)
{
#if USE_VEC_REG

    return op_PSHUFB128 (dest, mask);

#else

    CipherVec_AES_u out, in, gate;
    unsigned ix;

    in.state = dest;
    gate.state = mask;

    for (ix = 0; ix < sizeof dest; ++ix)
    {
        if (gate.bytes[ix] & 0x80)
            out.bytes[ix] = 0;
        else
            out.bytes[ix] = in.bytes[gate.bytes[ix] & 0x0F];
    }

    return out.state;

#endif
}


static __inline__ 
CipherVec AESBCMEMBER(VecSLLBI) (CipherVec v, const int k)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(VecSLLBI) (CipherVec v, const int k)
{
#if USE_VEC_REG
    static const CipherVec sl_mask [9] = 
        {
            { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
              0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
            { 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
              0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE },
            { 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
              0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC },
            { 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
              0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8 },
            { 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
              0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0 },
            { 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
              0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0 },
            { 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
              0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0 },
            { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
              0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 },
            { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
        };
    CipherVec vv = v;

    assert (k <= 8);
    

/*     DEBUG_CIPHER_VECTOR ("mask", sl_mask[k]); */
/*     DEBUG_CIPHER_VECTOR ("v 1", vv); */
    vv = op_PSLLDI128 (vv, k);
/*     DEBUG_CIPHER_VECTOR ("v 2", vv); */
    vv = AESBCMEMBER(VecAnd) (vv, sl_mask[k]);
/*     DEBUG_CIPHER_VECTOR ("v 3", vv); */
    return vv;

#else

    CipherVec_AES_u vv; /* state and key not countries :) */
    unsigned ix;

    vv.state = v;

    for (ix = 0; ix < sizeof (v); ++ix)
    {
        KOutMsg ("SLLBI %d %x ", k, vv.bytes[ix]);
        vv.bytes[ix] <<= k;
        KOutMsg ("%x\n", vv.bytes[ix]);
    }
    return vv.state;

#endif
}


static __inline__ 
CipherVec AESBCMEMBER(VecSRLBI) (CipherVec v, const int k)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(VecSRLBI) (CipherVec v, const int k)
{
#if USE_VEC_REG
    static const CipherVec sr_mask [9] = 
        {
            { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
              0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
            { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
              0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F },
            { 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
              0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F },
            { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
              0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },
            { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
              0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
            { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
              0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 },
            { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
              0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 },
            { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
              0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
            { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
        };

    v = op_PSRLDI128 (v, k);
    v = AESBCMEMBER(VecAnd) (v, sr_mask[(k<8)?k:8]);
    return v;

#else

    CipherVec_AES_u vv; /* state and key not countries :) */
    unsigned ix;

    vv.state = v;

    for (ix = 0; ix < sizeof (v); ++ix)
    {
        KOutMsg ("SRLBI %d %x ", k, vv.bytes[ix]);
        vv.bytes[ix] >>= k;
        KOutMsg ("%x\n", vv.bytes[ix]);
    }
    return vv.state;

#endif
}


static __inline__
CipherVec AESBCMEMBER(SetColumn) (CipherVec state, AESWord column, const int which)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(SetColumn) (CipherVec state, AESWord column, const int which)
{
#if USE_VEC_REG
    state = op_PINSRUD (state, column, which);
#else
    CipherVec_AES_u u;
    u.state = state;
    u.columns [which] = column;
    state = u.state;
#endif
    return state;
}


/* ======================================================================
 * FIPS-197 5.1.4
 *
 * The AddRoundKey transformation is a sinple exclusive or of all 128 bits
 * in the current block against a round key.  This operation is used in all
 * Cipher and EqInvCipher rounds
 *
 * With vector types it is a single operation that with vector registers is
 * a single op-code.
 */
static __inline__ 
CipherVec AESBCMEMBER(AddRoundKey) (CipherVec state, CipherVec round_key)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(AddRoundKey) (CipherVec state, CipherVec round_key)
{
    return AESBCMEMBER(VecXor) (state, round_key);
}


/* ======================================================================
 * FIPS-197 5.1.2
 *
 * FIPS 197 describes the state in column major format
 * while C and assembly programmers tend to think in row major
 * The byte order is 0,1,2,3,,...F as shown in the first diagram.
 *
 *  ShiftRows() is defined as leaving this state
 * +---+---+---+---+     +---+---+---+---+
 * | 0 | 4 | 8 | C |     | 0 | 4 | 8 | C |
 * +---+---+---+---+     +---+---+---+---+
 * | 1 | 5 | 9 | D |     | 5 | 9 | D | 1 |
 * +---+---+---+---+     +---+---+---+---+
 * | 2 | 6 | A | E |     | A | E | 2 | 6 |
 * +---+---+---+---+     +---+---+---+---+
 * | 3 | 7 | B | F |     | F | 3 | 7 | B |
 * +---+---+---+---+     +---+---+---+---+
 */
static __inline__
CipherVec AESBCMEMBER(ShiftRows) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(ShiftRows) (CipherVec state)
{
static const
    CipherVec ShiftRowTable
    /*   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
    = {  0,  5, 10, 15,  4,  9, 14,  3,  8, 13,  2,  7, 12,  1,  6, 11 };

    state = AESBCMEMBER(PackShuffleBytes) (state, ShiftRowTable);
    return state;
}


/* ======================================================================
 * FIPS-197 5.3.1
 *
 * InvShiftRows() is defined as leaving this state
 * +---+---+---+---+     +---+---+---+---+
 * | 0 | 4 | 8 | C |     | 0 | 4 | 8 | C |
 * +---+---+---+---+     +---+---+---+---+
 * | 1 | 5 | 9 | D |     | D | 1 | 5 | 9 |
 * +---+---+---+---+     +---+---+---+---+
 * | 2 | 6 | A | E |     | A | E | 2 | 6 |
 * +---+---+---+---+     +---+---+---+---+
 * | 3 | 7 | B | F |     | 7 | B | F | 3 |
 * +---+---+---+---+     +---+---+---+---+
 */
static __inline__
CipherVec AESBCMEMBER(InvShiftRows) (CipherVec state)
    __attribute__ ((always_inline));


static __inline__
CipherVec AESBCMEMBER(InvShiftRows) (CipherVec state)
{
    static const CipherVecByte InvShiftRowTable
        /*   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
        = {  0, 13, 10,  7,  4,  1, 14, 11,  8,  5,  2, 15, 12,  9,  6,  3 };

    state = AESBCMEMBER(PackShuffleBytes) (state, InvShiftRowTable);
    return state;
}


/* ======================================================================
 * +---+---+---+---+     +---+---+---+---+
 * | 0 | 4 | 8 | C |     | 3 | 7 | B | F |
 * +---+---+---+---+     +---+---+---+---+
 * | 1 | 5 | 9 | D |     | 0 | 4 | 8 | C |
 * +---+---+---+---+     +---+---+---+---+
 * | 2 | 6 | A | E |     | 1 | 5 | 9 | D |
 * +---+---+---+---+     +---+---+---+---+
 * | 3 | 7 | B | F |     | 2 | 6 | A | E |
 * +---+---+---+---+     +---+---+---+---+
 */
static __inline__
CipherVec AESBCMEMBER(RotBytesRight) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(RotBytesRight) (CipherVec state)
{
static const
    CipherVec ShiftRowTable
    /*   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
    = {  3,  0,  1,  2,  7,  4,  5,  6, 11,  8,  9, 10, 15, 12, 13, 14 };

    state = AESBCMEMBER(PackShuffleBytes) (state, ShiftRowTable);
    return state;
}


/* ======================================================================
 * +---+---+---+---+     +---+---+---+---+
 * | 0 | 4 | 8 | C |     | 1 | 5 | B | D |
 * +---+---+---+---+     +---+---+---+---+
 * | 1 | 5 | 9 | D |     | 2 | 6 | 8 | E |
 * +---+---+---+---+     +---+---+---+---+
 * | 2 | 6 | A | E |     | 3 | 7 | 9 | F |
 * +---+---+---+---+     +---+---+---+---+
 * | 3 | 7 | B | F |     | 0 | 4 | 8 | C |
 * +---+---+---+---+     +---+---+---+---+
 */
static __inline__
CipherVec AESBCMEMBER(RotBytesLeft) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(RotBytesLeft) (CipherVec state)
{
    static const CipherVec ShiftRowTable
        /*   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
        = {  1,  2,  3,  0,  5,  6,  7,  4,  9, 10, 11,  8, 13, 14, 15, 12 };

    state = AESBCMEMBER(PackShuffleBytes) (state, ShiftRowTable);
    return state;
}


/* ======================================================================
 * FIPS-197 5.1.1
 */


/*
 * SubBytes replaces each byte in a state with a specific byte value from
 * A Rijndael Substitution box
 */
static const 
AESByte AESBCMEMBER(RijndaelSBox)[256] = 
{
/*        -0    -1    -2    -3    -4    -5    -6    -7    -8    -9    -A    -B    -C    -D    -E    -F */
/* 0- */ 0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
/* 1- */ 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
/* 2- */ 0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
/* 3- */ 0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
/* 4- */ 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
/* 5- */ 0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
/* 6- */ 0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
/* 7- */ 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
/* 8- */ 0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
/* 9- */ 0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
/* A- */ 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
/* B- */ 0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
/* C- */ 0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
/* D- */ 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
/* E- */ 0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
/* F- */ 0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};


#if 0
static __inline__
CipherVec AESBCMEMBER(SubBytes) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(SubBytes) (CipherVec state)
{
    CipherVec_AES_u u;
    unsigned ix;

    u.state = state;

    for (ix = 0; ix < sizeof (u.bytes); ++ix)
    {
        u.bytes[ix] = AESBCMEMBER(RijndaelSBox)[u.bytes[ix]];
    }

    return u.state;
}
#endif

/*
 * SubWord performs the same operation as SubBytes but on
 * the four bytes of a column and not a whole state
 */
static __inline__
AESWord AESBCMEMBER(SubWord) (AESWord w)
    __attribute__ ((always_inline));

static __inline__
AESWord AESBCMEMBER(SubWord) (AESWord w)
{
    AESColumn col;
    unsigned ix;

    col.word = w;
    for (ix = 0; ix < sizeof (col); ++ix)
        col.bytes[ix] = AESBCMEMBER(RijndaelSBox)[(unsigned)col.bytes[ix]];
    return col.word;
}

#if 0
/*
 * InvSubBytes replaces each byte in a state with a specific byte value from
 * an Inversze Rijndael Substitution box.  That is InvSubBytes undoes SubBytes
 */
static const 
uint8_t AESBCMEMBER(RijndaelInvSBox)[256] = 
{
/*        -0    -1    -2    -3    -4    -5    -6    -7    -8    -9    -A    -B    -C    -D    -E    -F */
/* 0- */ 0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
/* 1- */ 0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
/* 2- */ 0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
/* 3- */ 0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
/* 4- */ 0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
/* 5- */ 0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
/* 6- */ 0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
/* 7- */ 0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
/* 8- */ 0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
/* 9- */ 0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
/* A- */ 0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
/* B- */ 0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
/* C- */ 0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
/* D- */ 0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
/* E- */ 0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
/* F- */ 0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};


static __inline__
CipherVec AESBCMEMBER(InvSubBytes) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(InvSubBytes) (CipherVec state)
{
    CipherVec_AES_u u;
    unsigned ix;

    u.state = state;
    for (ix = 0; ix < sizeof (u.bytes); ++ix)
        u.bytes[ix] = AESBCMEMBER(RijndaelInvSBox)[u.bytes[ix]];

    return u.state;
}
#endif

static const
CipherVec AESBCMEMBER(RijndaelSBoxV)[16] = 
{
/*     -0    -1    -2    -3    -4    -5    -6    -7    -8    -9    -A    -B    -C    -D    -E    -F */
    { 0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76 },
    { 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0 },
    { 0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15 },
    { 0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75 },
    { 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84 },
    { 0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF },
    { 0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8 },
    { 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2 },
    { 0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73 },
    { 0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB },
    { 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79 },
    { 0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08 },
    { 0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A },
    { 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E },
    { 0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF },
    { 0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16 }
};

static const
CipherVec AESBCMEMBER(RijndaelInvSBoxV)[16] = 
{
/*     -0    -1    -2    -3    -4    -5    -6    -7    -8    -9    -A    -B    -C    -D    -E    -F */
    { 0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB },
    { 0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB },
    { 0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E },
    { 0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25 },
    { 0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92 },
    { 0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84 },
    { 0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06 },
    { 0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B },
    { 0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73 },
    { 0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E },
    { 0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B },
    { 0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4 },
    { 0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F },
    { 0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF },
    { 0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61 },
    { 0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D }
};


static const
CipherVec lo_filter = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static __inline__
CipherVec AESBCMEMBER(SubBytesInt) (CipherVec state, const CipherVec box [16])
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(SubBytesInt) (CipherVec state, const CipherVec box [16])
{
    CipherVec lo, hi, tmp, out;
    unsigned ix;

    out = vec_00;

/*     DEBUG_CIPHER_VECTOR ("state", state); */
    lo = AESBCMEMBER(VecAnd) (state, vec_0F);
/*     DEBUG_CIPHER_VECTOR ("lo", lo); */
#if USE_VEC_REG
    hi = op_PSRLDI128 (state, 4);
/*     DEBUG_CIPHER_VECTOR ("hi", hi); */
    hi = AESBCMEMBER(VecAnd) (hi, vec_0F);
/*     DEBUG_CIPHER_VECTOR ("hi", hi); */
#else
    hi = AESBCMEMBER(VecDiv) (state, vec_10);
/*     DEBUG_CIPHER_VECTOR ("hi", hi); */
#endif

    for (ix = 0; ix < sizeof (state); ++ix)
    {
        tmp = AESBCMEMBER(PackShuffleBytes) (lo_filter, hi);
/*         DEBUG_CIPHER_VECTOR ("tmp", tmp); */
        tmp = AESBCMEMBER(VecOr) (lo, tmp);
/*         DEBUG_CIPHER_VECTOR ("tmp", tmp); */
        tmp = AESBCMEMBER(PackShuffleBytes) (box[ix], tmp);
/*         DEBUG_CIPHER_VECTOR ("tmp", tmp); */
        out = AESBCMEMBER(VecOr) (out, tmp);
/*         DEBUG_CIPHER_VECTOR ("out", out); */
        hi  = AESBCMEMBER(VecSub) (hi, vec_01);
/*         DEBUG_CIPHER_VECTOR ("hi", hi); */
        hi  = AESBCMEMBER(VecAnd) (hi, vec_0F);
/*         DEBUG_CIPHER_VECTOR ("hi", hi); */
    }
/*     DEBUG_CIPHER_VECTOR ("out", out); */
    return out;
}


static __inline__
CipherVec AESBCMEMBER(SubBytes) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(SubBytes) (CipherVec state)
{
    return AESBCMEMBER(SubBytesInt) (state, AESBCMEMBER(RijndaelSBoxV));
}


static __inline__
CipherVec AESBCMEMBER(InvSubBytes) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(InvSubBytes) (CipherVec state)
{
    return AESBCMEMBER(SubBytesInt) (state, AESBCMEMBER(RijndaelInvSBoxV));
}


/* ======================================================================
 * FIPS-197 5.1.3
 */

/*
 * there are no psllb or psrlb instructions so shift words not bytes
 * and then cut off bits that would have been zeroed if there were
 * such instructions
 *
 * 
 */
static const CipherVec FF_tab = 
{ 0x00, 0x1B, 0x36, 0x2D, 0x6C, 0x77, 0x5A, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static __inline__
CipherVec AESBCMEMBER(FF_mul) (register CipherVec state, const int bits)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(FF_mul) (register CipherVec state, const int bits)
{
    CipherVec sl, sr;

    assert (bits <= 8);

    DEBUG_CIPHER_VECTOR ("state", state);
    sr = AESBCMEMBER(VecSRLBI) (state, 8 - bits);
    DEBUG_CIPHER_VECTOR ("sr", sr);
    sl = AESBCMEMBER(VecSLLBI) (state, bits);
    DEBUG_CIPHER_VECTOR ("sl", sl);
    sr = AESBCMEMBER(PackShuffleBytes) (FF_tab, sr);
    return AESBCMEMBER(VecXor)(sl, sr);
}


static __inline__
CipherVec AESBCMEMBER(FF_mul_02) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(FF_mul_02) (register CipherVec state)
{
    DEBUG_CIPHER_VECTOR ("state", state);

    state =  AESBCMEMBER(FF_mul) (state, 1);
    DEBUG_CIPHER_VECTOR ("state", state);

    return state;
}


static __inline__
CipherVec AESBCMEMBER(FF_mul_04) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(FF_mul_04) (CipherVec state)
{
    DEBUG_CIPHER_VECTOR ("state", state);

    state =  AESBCMEMBER(FF_mul) (state, 2);
    DEBUG_CIPHER_VECTOR ("state", state);

    return state;
}


static __inline__
CipherVec AESBCMEMBER(FF_mul_08) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(FF_mul_08) (CipherVec state)
{
    DEBUG_CIPHER_VECTOR ("state", state);

    state =  AESBCMEMBER(FF_mul) (state, 3);
    DEBUG_CIPHER_VECTOR ("state", state);

    return state;
}


static AESByte AESBCMEMBER(FFtable)[] = { 0x00, 0x1b, 0x36, 0x2d, 0x6c, 0x77, 0x5a, 0x41 };


static __inline__ 
AESByte AESBCMEMBER(FFtab) (uint8_t x)
    __attribute__ ((always_inline));

static __inline__ 
AESByte AESBCMEMBER(FFtab) (uint8_t x)
{
    return AESBCMEMBER(FFtable)[x];
}


static __inline__ 
AESByte AESBCMEMBER(FFmul02) (AESByte b)
    __attribute__ ((always_inline));

static __inline__ 
AESByte AESBCMEMBER(FFmul02) (AESByte b)
{
    return (b << 1) ^ AESBCMEMBER(FFtab)(b >> 7);
}


static __inline__ 
AESByte AESBCMEMBER(FFmul04) (AESByte b)
    __attribute__ ((always_inline));

static __inline__ 
AESByte AESBCMEMBER(FFmul04) (AESByte b)
{
    return (b << 2) ^ AESBCMEMBER(FFtab)(b >> 6);
}


static __inline__ 
AESByte AESBCMEMBER(FFmul08) (AESByte b)
    __attribute__ ((always_inline));

static __inline__ 
AESByte AESBCMEMBER(FFmul08) (AESByte b)
{
    return (b << 3) ^ AESBCMEMBER(FFtab)(b >> 5);
}


static __inline__
CipherVec AESBCMEMBER(MixColumns) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(MixColumns) (CipherVec state)
{
    register CipherVec c0, c1, c2, c3;
    
    c1 = AESBCMEMBER(RotBytesLeft) (state);
    c2 = AESBCMEMBER(RotBytesLeft) (c1);
    c3 = AESBCMEMBER(RotBytesLeft) (c2);
    c0 = AESBCMEMBER(VecXor) (state, c1);


#if USE_VEC_REG
    c0 = AESBCMEMBER(FF_mul_02) (c0);
#else
    {
        CipherVec_AES_u tmp;
        unsigned ix;

        tmp.state = c0;
    
        for (ix = 0; ix < sizeof tmp; ++ix)
            tmp.bytes[ix] = AESBCMEMBER(FFmul02)(tmp.bytes[ix]);
        c0 = tmp.state;
    }
#endif
    c1 = AESBCMEMBER(VecXor) (c1, c2);
    c1 = AESBCMEMBER(VecXor) (c1, c3);
    c1 = AESBCMEMBER(VecXor) (c1, c0);

    return c1;
/* obsoleted 
    CipherVec_AES_u in, out;
    unsigned ix;

    in.state = state;

    for (ix = 0; ix < sizeof (out.bytes); ix += 4)
    {
        unsigned const s0 = in.bytes[ix + 0];
        unsigned const s1 = in.bytes[ix + 1];
        unsigned const s2 = in.bytes[ix + 2];
        unsigned const s3 = in.bytes[ix + 3];

        out.bytes[ix + 0] =      s1 ^ s2 ^ s3 ^ AESBCMEMBER(FFmul02)(s0 ^ s1);
        out.bytes[ix + 1] = s0      ^ s2 ^ s3 ^ AESBCMEMBER(FFmul02)(s1 ^ s2);
        out.bytes[ix + 2] = s0 ^ s1      ^ s3 ^ AESBCMEMBER(FFmul02)(s2 ^ s3);
        out.bytes[ix + 3] = s0 ^ s1 ^ s2      ^ AESBCMEMBER(FFmul02)(s3 ^ s0);
    }
    return out.state;
*/
}

static __inline__
CipherVec AESBCMEMBER(InvMixColumns) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(InvMixColumns) (CipherVec state)
{
#if USE_AES_NI
    state = op_AESIMC (state);
#else
    register CipherVec r1, r2, r3, f2, f4, f8;

    r1 = AESBCMEMBER(RotBytesLeft) (state);
    r2 = AESBCMEMBER(RotBytesLeft) (r1);
    r3 = AESBCMEMBER(RotBytesLeft) (r2);        

    f4 = AESBCMEMBER(VecXor) (state, r2);
    f2 = AESBCMEMBER(VecXor) (state, r1);
    f8 = AESBCMEMBER(VecXor) (r2, r3);
    state = AESBCMEMBER(VecXor) (f8, r1);
    f8 = AESBCMEMBER(VecXor) (f8, f2);


#if USE_VEC_REG
    f2 = AESBCMEMBER(FF_mul) (f2,1);
    f4 = AESBCMEMBER(FF_mul) (f4,2);
    f8 = AESBCMEMBER(FF_mul) (f8,3);
#else
    {
        CipherVec_AES_u fu2, fu4, fu8;
        unsigned ix;

        fu2.state = f2;
        fu4.state = f4;
        fu8.state = f8;

        for (ix = 0; ix < sizeof fu2; ++ix)
        {
            fu2.bytes[ix] = AESBCMEMBER(FFmul02)(fu2.bytes[ix]);
            fu4.bytes[ix] = AESBCMEMBER(FFmul04)(fu4.bytes[ix]);
            fu8.bytes[ix] = AESBCMEMBER(FFmul08)(fu8.bytes[ix]);
        }
        f2 = fu2.state;
        f4 = fu4.state;
        f8 = fu8.state;
    }
#endif

    state = AESBCMEMBER(VecXor) (state, f2);
    state = AESBCMEMBER(VecXor) (state, f4);
    state = AESBCMEMBER(VecXor) (state, f8);

#endif

    return state;
/*
    CipherVec_AES_u in, out;
    unsigned ix;

    in.state = state;

    for (ix = 0; ix < sizeof (out.bytes); ix += AES_Nb)
    {
        AESWord s0 = in.bytes[ix + 0];
        AESWord s1 = in.bytes[ix + 1];
        AESWord s2 = in.bytes[ix + 2];
        AESWord s3 = in.bytes[ix + 3];
        AESWord p, q;

        q =     AESBCMEMBER(FFmul08)(s0 ^ s1 ^ s2 ^ s3);
        p = q ^ AESBCMEMBER(FFmul04)(s0 ^ s2);
        q = q ^ AESBCMEMBER(FFmul04)(s1 ^ s3);

        out.bytes[ix + 0] =      s1 ^ s2 ^ s3 ^ AESBCMEMBER(FFmul02)(s0 ^ s1) ^ p;
        out.bytes[ix + 1] = s0      ^ s2 ^ s3 ^ AESBCMEMBER(FFmul02)(s1 ^ s2) ^ q;
        out.bytes[ix + 2] = s0 ^ s1      ^ s3 ^ AESBCMEMBER(FFmul02)(s2 ^ s3) ^ p;
        out.bytes[ix + 3] = s0 ^ s1 ^ s2      ^ AESBCMEMBER(FFmul02)(s3 ^ s0) ^ q;
    }

    return out.state;
*/
}


/* ======================================================================
 * FIPS-197 5.2
 */
static const
AESWord AESBCMEMBER(Rcon)[] = {
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x0000001B, 0x00000036
};


static __inline__
AESWord AESBCMEMBER(RotWord) (AESWord w)
    __attribute__ ((always_inline));

static __inline__
AESWord AESBCMEMBER(RotWord) (AESWord w)
{
    return (w >> 8) | (w << 24);
}


static __inline__
AESWord AESBCMEMBER(GetWord) (const AESByte * pointer)
    __attribute__ ((always_inline));

static __inline__
AESWord AESBCMEMBER(GetWord) (const AESByte * pointer)
{
    /*
     * not portable to architectures which require aligned multibyte reads
     * but as this only runs on Intel at this point it is fine for now
     */
    return *(AESWord*)pointer;
}



static __inline__
void AESBCMEMBER(KeyExpansion) (CipherVec * r, const AESByte * key, unsigned Nr, unsigned Nk)
{
    AESWord * w = (AESWord*)r;
    const AESWord * pRcon;
    unsigned ix, limit;
    AESWord temp;


    DEBUG_KEYEXP(("%s (w (%p), key (%p), Nr (%u) Nk (%u))\n", __func__, w, key, Nr, Nk));

    for (ix = 0; ix < Nk; ++ix)
    {
        DEBUG_KEYEXP(("i (dec): %u\n", ix));
        temp = AESBCMEMBER(GetWord) (key+(ix*4));
        w[ix] = temp;
        DEBUG_KEYEXP(("kw[%i]: %0.8x\n", ix, temp));
    }

    limit = AES_Nb * (Nr + 1);
    pRcon = AESBCMEMBER(Rcon);
    DEBUG_KEYEXP(("Nk (%u) Nr (%u) limit (%u)\n", Nk, Nr, limit));
        
    for (; ix < limit; ++ix)
    {

        DEBUG_KEYEXP(("i (dec): %u %u\n", ix, limit));

        /* temp should already be set */
        DEBUG_KEYEXP(("temp: %0.8x\n", temp));
        if ((ix % Nk) == 0)
        {
            temp = AESBCMEMBER(RotWord)(temp);
            DEBUG_KEYEXP(("after RotWord: %0.8x \n", temp));
            temp = AESBCMEMBER(SubWord) (temp);
            DEBUG_KEYEXP(("after SubWord: %0.8x \n", temp));
            DEBUG_KEYEXP(("Rcon [i/Nk]: %0.8x\n", *pRcon));
            temp ^= *pRcon++;
            DEBUG_KEYEXP(("after XOR with Rcon: %0.8x \n", temp));
        }
        else if ((Nk > 6) && ((ix % Nk) == 4))
        {
            temp = AESBCMEMBER(SubWord) (temp);
            DEBUG_KEYEXP(("after SubWord: %0.8x \n", temp));
        }
        DEBUG_KEYEXP(("w[i-Nk]: %0.8x\n", w[ix-Nk]));
        temp ^= w [ix - Nk];
        DEBUG_KEYEXP(("kw[ix]: %0.8x\n", temp));
        w [ix] = temp;
    }
}


static __inline__
CipherVec AESBCMEMBER(StateShiftLeft32) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(StateShiftLeft32) (CipherVec state)
{
#if (USE_VEC_REG)
    static const CipherVec mask = { 0xFF, 0xFF, 0xFF, 0xFF, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    return AESBCMEMBER(PackShuffleBytes) (state, mask);
#else
    CipherVec_AES_u temp;

    temp.state = state;
    temp.columns[3] = temp.columns[2];
    temp.columns[2] = temp.columns[1];
    temp.columns[1] = temp.columns[0];
    temp.columns[0] = 0;
    return temp.state;
#endif
}


static __inline__
CipherVec AESBCMEMBER(StateShiftRight64) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(StateShiftRight64) (CipherVec state)
{
#if (USE_VEC_REG)
    static const CipherVec mask = { 8, 9, 10, 11, 12, 13, 14, 15, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    return AESBCMEMBER(PackShuffleBytes) (state, mask);
#else
    CipherVec_AES_u temp;

    temp.state = state;
    temp.columns[0] = temp.columns[2];
    temp.columns[1] = temp.columns[3];
    temp.columns[2] = 0;
    temp.columns[3] = 0;
    return temp.state;
#endif
}


static __inline__
CipherVec AESBCMEMBER(StateShiftLeft64) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(StateShiftLeft64) (CipherVec state)
{
#if (USE_VEC_REG)
    static const CipherVec mask = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 1, 2, 3, 4, 5, 6, 7 };
    return AESBCMEMBER(PackShuffleBytes) (state, mask);
#else
    CipherVec_AES_u temp;

    temp.state = state;
    temp.columns[3] = temp.columns[2];
    temp.columns[2] = temp.columns[1];
    temp.columns[1] = temp.columns[0];
    temp.columns[0] = 0;
    return temp.state;
#endif
}


/*
 * incoming vector in words
 *     W0:W1:W2:W3
 *
 * output vector in words
 *     W3:W3:W3:W3
 */
static __inline__
CipherVec AESBCMEMBER(StateDupColumn3) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(StateDupColumn3) (CipherVec state)
{
#if (USE_VEC_REG)
    return op_PSHUFD128 (state,0xFF);
#else
    CipherVec_AES_u temp;

    temp.state = state;
    temp.columns[2] = temp.columns[3];
    temp.columns[1] = temp.columns[3];
    temp.columns[0] = temp.columns[3];
    return temp.state;
#endif
}


/*
 * incoming vector in words
 *     W0:W1:W2:W3
 *
 * output vector in words
 *     W2:W2:W2:W2
 */
static __inline__
CipherVec AESBCMEMBER(StateDupColumn2) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(StateDupColumn2) (CipherVec state)
{
#if (USE_VEC_REG)
    return op_PSHUFD128 (state,0xAA);
#else
    CipherVec_AES_u temp;

    temp.state = state;
    temp.columns[3] = temp.columns[2];
    temp.columns[1] = temp.columns[2];
    temp.columns[0] = temp.columns[2];
    return temp.state;
#endif
}


/*
 * incoming vector in words
 *     W0:W1:W2:W3
 *
 * output vector in words
 *     W2:W2:W2:W2
 */
static __inline__
CipherVec AESBCMEMBER(StateDupColumn1) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(StateDupColumn1) (CipherVec state)
{
#if (USE_VEC_REG)
    return op_PSHUFD128 (state,0x55);
#else
    CipherVec_AES_u temp;

    temp.state = state;
    temp.columns[3] = temp.columns[1];
    temp.columns[2] = temp.columns[1];
    temp.columns[0] = temp.columns[1];
    return temp.state;
#endif
}


/*
 * incoming vector in words
 *     W0:W1:W2:W3
 *
 * output vector in words
 *     W0:W0:W0:W0
 */
static __inline__
CipherVec AESBCMEMBER(StateDupColumn0) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(StateDupColumn0) (CipherVec state)
{
#if (USE_VEC_REG)
    return op_PSHUFD128 (state,0x00);
#else
    CipherVec_AES_u temp;

    temp.state = state;
    temp.columns[3] = temp.columns[0];
    temp.columns[2] = temp.columns[0];
    temp.columns[1] = temp.columns[0];
    return temp.state;
#endif
}


/*
 * incoming vector in words
 *     W0:W1:W2:W3
 *     X0:X1:X2:X3
 *
 * output vector in words
 *     W0:W1:X0:X1
 */
static __inline__
CipherVec AESBCMEMBER(StateMerge1) (CipherVec w, CipherVec x)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(StateMerge1) (CipherVec w, CipherVec x)
{
#if USE_VEC_REG
    return op_SHUFPD (w, x, 0x00);
#else
    CipherVec_AES_u ww;
    CipherVec_AES_u xx;

    ww.state = w;
    xx.state = x;

    ww.columns[2] = xx.columns[0];
    ww.columns[3] = xx.columns[1];

    return ww.state;
#endif
}


/*
 * incoming vector in words
 *     W0:W1:W2:W3
 *     X0:X1:X2:X3
 *
 * output vector in words
 *     X2:X3:W0:W1
 */
static __inline__
CipherVec AESBCMEMBER(StateMerge2) (CipherVec w, CipherVec x)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(StateMerge2) (CipherVec w, CipherVec x)
{
#if USE_VEC_REG
    return op_SHUFPD (w, x, 0x01);
#else
    CipherVec_AES_u ww;
    CipherVec_AES_u xx;

    ww.state = w;
    xx.state = x;

    ww.columns[0] = ww.columns[2];
    ww.columns[1] = ww.columns[3];
    ww.columns[2] = xx.columns[0];
    ww.columns[3] = xx.columns[1];

    return ww.state;
#endif
}


/*
 * incoming vector in words
 *     W0:W1:W2:W3
 *     X0:X1:X2:X3
 *
 * output vector in words
 *     X2:X3:W0:W1
 */
static __inline__
CipherVec AESBCMEMBER(StateMerge3) (CipherVec v0, CipherVec v1)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(StateMerge3) (CipherVec v0, CipherVec v1)
{
#if USE_VEC_REG
    return op_SHUFPD (v0, v1, 0x02);
#else
    CipherVec_AES_u t0;
    CipherVec_AES_u t1;

    t0.state = v0;
    t1.state = v1;

    t0.columns[2] = t0.columns[0];
    t0.columns[3] = t0.columns[1];
    t0.columns[0] = t1.columns[2];
    t0.columns[1] = t1.columns[3];

    return t0.state;
#endif
}


/*
 * incoming vector in words
 *     W0:W1:W2:W3
 *
 * output vector in words
 *     W0:W0^W1:W0^W1^W2:W0^W1^W2^W3
 */
static __inline__
CipherVec AESBCMEMBER(ShiftXorColumns) (CipherVec state)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(ShiftXorColumns) (CipherVec state)
{
#if USE_VEC
    register CipherVec shift = state;

    state ^= shift = AESBCMEMBER(StateShiftLeft32) (shift);
    state ^= shift = AESBCMEMBER(StateShiftLeft32) (shift);
    state ^= shift = AESBCMEMBER(StateShiftLeft32) (shift);

    return state;
#else
    CipherVec_AES_u temp;

    temp.state = state;

    temp.columns[1] ^= temp.columns[0];
    temp.columns[2] ^= temp.columns[1];
    temp.columns[3] ^= temp.columns[2];

    return temp.state;
#endif
}


static __inline__ 
CipherVec AESBCMEMBER(AesKeyGenAssist) (register CipherVec state, const int rcon)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(AesKeyGenAssist) (register CipherVec state, const int rcon)
{
#if (USE_AES_NI)
    return op_AESKEYGENASSIST (state, rcon);
#else
    CipherVec_AES_u in;
    CipherVec_AES_u out;
    AESWord rconw;

    rconw = (AESWord)(AESByte)rcon;
    in.state = state;

    out.columns[0] = AESBCMEMBER(SubWord)(in.columns[1]);
    out.columns[1] = AESBCMEMBER(RotWord)(out.columns[0]) ^ rconw;
    out.columns[2] = AESBCMEMBER(SubWord)(in.columns[3]);
    out.columns[3] = AESBCMEMBER(RotWord)(out.columns[2]) ^ rconw;

    return out.state;
#endif
}


static __inline__ 
CipherVec AESBCMEMBER(AesKeyGenAssistColumn1) (register CipherVec state, const int rcon)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(AesKeyGenAssistColumn1) (register CipherVec state, const int rcon)
{
#if (USE_AES_NI)
    register CipherVec tmp = state;
    tmp = op_AESKEYGENASSIST (tmp, rcon);
    return  AESBCMEMBER(StateDupColumn1) (tmp);
#else
    CipherVec_AES_u u;
    AESWord rconw;
    AESWord temp;

    rconw = (AESWord)(AESByte)rcon;
    u.state = state;
    temp  = AESBCMEMBER(SubWord)(u.columns[1]);
    temp  = AESBCMEMBER(RotWord)(temp);
    temp ^= rconw;

    u.columns [0] = u.columns [1] = u.columns [2] = u.columns [3] = temp;
    return u.state;
#endif
}


static __inline__ 
CipherVec AESBCMEMBER(AesKeyGenAssistColumn2) (register CipherVec state)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(AesKeyGenAssistColumn2) (register CipherVec state)
{
#if (USE_AES_NI)
    register CipherVec tmp = state;
    tmp = op_AESKEYGENASSIST (tmp, 0);
    return  AESBCMEMBER(StateDupColumn2) (tmp);
#else
    CipherVec_AES_u u;
    AESWord temp;

    u.state = state;
    temp  = AESBCMEMBER(SubWord)(u.columns[3]);

    u.columns [0] = u.columns [1] = u.columns [2] = u.columns [3] = temp;
    return u.state;
#endif
}


static __inline__ 
CipherVec AESBCMEMBER(AesKeyGenAssistColumn3) (register CipherVec state, const int rcon)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(AesKeyGenAssistColumn3) (register CipherVec state, const int rcon)
{
#if (USE_AES_NI)
    register CipherVec tmp = state;
    tmp = op_AESKEYGENASSIST (tmp, rcon);
    return  AESBCMEMBER(StateDupColumn3) (tmp);

#else
    register CipherVec vrcon;
    register CipherVec tmp;

    vrcon = AESBCMEMBER(SetColumn) (vec_00, (AESWord)rcon, 3);
    tmp = AESBCMEMBER(SubBytes) (state);
    DEBUG_CIPHER_VECTOR ("SubBytes", tmp);
    tmp = AESBCMEMBER(RotBytesLeft) (tmp);
    DEBUG_CIPHER_VECTOR ("RotBytesLeft", tmp);
    tmp = AESBCMEMBER(VecXor)(tmp, vrcon);
    DEBUG_CIPHER_VECTOR ("VecXor", tmp);
    tmp = AESBCMEMBER(StateDupColumn3) (tmp);
    DEBUG_CIPHER_VECTOR ("StateDupColumn3", tmp);
    return tmp;

#endif
/*
#elif USE_VEC && 1
    register CipherVec tmp;
    CipherVec_AES_u u;
    
    u.columns [3] = rcon;

    tmp = AESBCMEMBER(SubBytes) (state);
    DEBUG_CIPHER_VECTOR ("SubBytes", tmp);
    tmp = AESBCMEMBER(RotBytesLeft) (tmp);
    DEBUG_CIPHER_VECTOR ("RotBytesLeft", tmp);
    tmp = AESBCMEMBER(VecXor)(tmp, u.state);
    DEBUG_CIPHER_VECTOR ("VecXor", tmp);
    tmp = AESBCMEMBER(StateDupColumn3) (tmp);
    DEBUG_CIPHER_VECTOR ("StateDupColumn3", tmp);
    return tmp;

#else
    CipherVec_AES_u u;
    AESWord rconw;
    AESWord temp;

    rconw = (AESWord)(AESByte)rcon;
    u.state = state;
    temp  = AESBCMEMBER(SubWord)(u.columns[3]);
    temp  = AESBCMEMBER(RotWord)(temp);
    temp ^= rconw;

    u.columns [0] = u.columns [1] = u.columns [2] = u.columns [3] = temp;
    return u.state;
#endif
*/
}


#if USE_VEC
static __inline__
void AESBCMEMBER(KeyExpansion128) (CipherVec * round_keys, const AESByte * key)
{
    CipherVec state;
    CipherVec kgastate;

    state = AESBCMEMBER(StateIn) (key);
    round_keys[0] = state;

#define KE128(rcon,offset) \
    kgastate = AESBCMEMBER(AesKeyGenAssistColumn3) (state, rcon); \
    state = AESBCMEMBER(ShiftXorColumns) (state); \
    state ^= kgastate; \
    round_keys[offset] = state

    KE128(0x01,1);
    KE128(0x02,2);
    KE128(0x04,3);
    KE128(0x08,4);
    KE128(0x10,5);
    KE128(0x20,6);
    KE128(0x40,7);
    KE128(0x80,8);
    KE128(0x1B,9);
    KE128(0x36,10);
}
#else
static __inline__
void AESBCMEMBER(KeyExpansion128) (CipherVec * w, const AESByte * key)
{
    OUTMSG(("\n\n\n%s: Not my code\n\n\n",__func__));
    AESBCMEMBER(KeyExpansion) (w, key, AES_Nr_128, AES_Nk_128);
}
#endif


#if USE_VEC
static __inline__
void AESBCMEMBER(KeyExpansion192) (CipherVec * round_keys, const AESByte * key)
{
    /* work on three blocks at a time which is two Nr worth
     * we'll build it with 6 words in v0:v1 and 6 words in w0:w1
     * but move them them into v0:v1:v2 before writing them to
     * the key schedule
     */

    register CipherVec v0, v1, v2, v3, v4;
    register CipherVec k;          /* we'll only need one of the key gen assist blocks at a time */

/* pattern 0 */
    v0  = AESBCMEMBER(StateIn) (key);              /* k0    : k1    : k2     : k3           */
    v1  = AESBCMEMBER(StateIn) (key+8);            /* k2    : k3    : k4     : k5           */
    v1  = AESBCMEMBER(StateShiftRight64) (v1);     /* k4    : k5    : 0      : 0            */

    round_keys[0] = v0; /* K0:K1:K2:K3 */

/* pattern 1 */
/* start the working pattern with 6 ready words in v0:v1 */

    k   = AESBCMEMBER(AesKeyGenAssistColumn1)
        (k = v1, 0x01);                            /* F(k5) : F(k5) : F(k5)  : F(k5)        */
    v3  = AESBCMEMBER(ShiftXorColumns) (v0);       /* k0    : k0^k1 :k0^k1^k2:k0^k1^k2^k3   */
    v3 ^= k;                                       /* k6    : k7    : k8     : k9           */
    v4  = AESBCMEMBER(ShiftXorColumns) (v1);       /* k4    : k4^k5 : k4^k5  : k4^k5        */
    k   = AESBCMEMBER(StateDupColumn3) (k = v3);   /* k9    : k9    : k9     : k9           */
    v4 ^= k;                                       /* k10   : k11   : k10    : k11          */
    v1  = AESBCMEMBER(StateMerge1) (v1,v3);        /* k4    : k5    : k6     : k7           */
    round_keys[1] = v1; /* K4:K5:K6:K7 */

    v2  = AESBCMEMBER(StateMerge2) (v3,v4);        /* k8    : k9    : k10    : k11          */
    round_keys[2] = v2; /* K8:K9:K10:K11 */

/* pattern 2 */
    v0  = AESBCMEMBER(ShiftXorColumns) (v3);       /* k6    : k6^k7 :k6^k7^k8:k6^k7^k8^k9   */
    v1  = AESBCMEMBER(ShiftXorColumns) (v4);       /* k10   :k10^k11: k10^k11: k10^k11      */
    k   = AESBCMEMBER(AesKeyGenAssistColumn3)
        (k = v2,0x02);                             /* F(k11): F(k11): F(k11) : F(k11)       */
    v0 ^= k;                                       /* k12   : k13   : k14    : k15          */
    round_keys[3] = v0; /* K12:K13:K14:K15 */
    k   = AESBCMEMBER(StateDupColumn3) (k = v0);   /* k15   : k15   : k15    : k15          */
    v1 ^= k;                                       /* k16   : k17   : k16    : k17          */


/* pattern 1 */
/* start the working pattern with 6 ready words in v0:v1 */

    k   = AESBCMEMBER(AesKeyGenAssistColumn1)
        (k = v1,0x04);                             /* F(k17): F(k17): F(k17) : F(k17)       */
    v3  = AESBCMEMBER(ShiftXorColumns) (v0);       /* k12   :k12^k13:k12^k13^k14:k12^k13^k14^k15 */
    v3 ^= k;                                       /* k18   : k19   : k20    : k21          */
    v4  = AESBCMEMBER(ShiftXorColumns) (v1);       /* k16   :k16^k17: k17    : 0            */
    k   = AESBCMEMBER(StateDupColumn3) (k = v3);   /* k21   : k21   : k21    : k21          */
    v4 ^= k;                                       /* k22   : k23   : x      : x            */
    v1  = AESBCMEMBER(StateMerge1) (v1,v3);        /* k16   : k17   : k18    : k19          */
    round_keys[4] = v1; /* K16:K17:K18:K19 */

    v2  = AESBCMEMBER(StateMerge2) (v3,v4);        /* k20   : k21   : k22    : k23          */
    round_keys[5] = v2; /* K20:K21:K22:K23 */

/* pattern 2 */
    v0  = AESBCMEMBER(ShiftXorColumns) (v3);       /* k18   :k18^k19:k18^k19^k20:k18^k19^k20^k21 */
    v1  = AESBCMEMBER(ShiftXorColumns) (v4);       /* k22   :k22^k23: k23    : 0            */
    k   = AESBCMEMBER(AesKeyGenAssistColumn3)
        (k = v2,0x08);                             /* F(k23): F(k23): F(k23) : F(k23)       */
    v0 ^= k;                                       /* k24   : k25   : k26    : k27          */
    k   = AESBCMEMBER(StateDupColumn3) (k = v0);   /* k27   : k27   : k27    : k27          */
    v1 ^= k;                                       /* k28   : k29   : x      : x            */

    round_keys[6] = v0; /* K24:K25:K26:K27 */

/* pattern 1 4-15 = 28-39 */
/* start the working pattern with 6 ready words in v0:v1 */

    k   = AESBCMEMBER(AesKeyGenAssistColumn1)
        (k = v1,0x10);                             /* F(k29): F(k29): F(k29) : F(k29)       */
    v3  = AESBCMEMBER(ShiftXorColumns) (v0);       /* k24   :k24^k25:k24^k25^k26:k24^k25^k26^k27 */
    v3 ^= k;                                       /* k30   : k31   : k32    : k33          */
    v4  = AESBCMEMBER(ShiftXorColumns) (v1);       /* k26   :k28^k29: k29    : 0            */
    k   = AESBCMEMBER(StateDupColumn3) (k = v3);   /* k33   : k33   : k33    : k33          */
    v4 ^= k;                                       /* k34   : k35   : x      : x            */
    v1  = AESBCMEMBER(StateMerge1) (v1,v3);        /* k28   : k29   : k30    : k31          */

    round_keys[7] = v1; /* K28:K29:K30:K31 */

    v2  = AESBCMEMBER(StateMerge2) (v3,v4);        /* k32   : k33   : k34    : k35          */
    round_keys[8] = v2; /* K32:K33:K34:K35 */
/* pattern 2 */
    v0  = AESBCMEMBER(ShiftXorColumns) (v3);       /* k30   :k30^k31:k30^k31^k32:k30^k31^k32^k33 */
    v1  = AESBCMEMBER(ShiftXorColumns) (v4);       /* k26   :k26^k27: k27    : 0            */
    k   = AESBCMEMBER(AesKeyGenAssistColumn3)
        (k = v2,0x20);                             /* F(k35): F(k35): F(k35) : F(k35)       */
    v0 ^= k;                                       /* k36   : k37   : k38    : k39          */
    k   = AESBCMEMBER(StateDupColumn3) (v0);       /* k39   : k39   : k39    : k39          */
    v1 ^= k;                                       /* k40   : k41   : x      : x            */

    round_keys[9] = v0; /* K36:K37:K38:K39 */

/* pattern 1 4-15 = 40-55 but we only need through 51 so we change from pattern 2 to pattern 3 */
/* start the working pattern with 6 ready words in v0:v1 */

    k   = AESBCMEMBER(AesKeyGenAssistColumn1)
        (k = v1,0x40);                             /* F(k41): F(k41): F(k41) : F(k41)       */
    v3  = AESBCMEMBER(ShiftXorColumns) (v0);       /* k36   :k36^k37:k36^k37^k38:k36^k37^k38^k39 */
    v4  = AESBCMEMBER(ShiftXorColumns) (v1);       /* k40   :k40^k41: x   5  : x            */
    v3 ^= k;                                       /* k42   : k43   : k44    : k45          */
    k   = AESBCMEMBER(StateDupColumn3) (k = v3);   /* k45   : k45   : k45    : k45          */
    v4 ^= k;                                       /* k46   : k47   : x      : x            */
    v1  = AESBCMEMBER(StateMerge1) (v1,v3);        /* k40   : k41   : k42    : k43          */
    round_keys[10] = v1; /* K40:K41:K42:K43 */
    v2  = AESBCMEMBER(StateMerge2) (v3,v4);        /* k44   : k45   : k46    : k47          */
    round_keys[11] = v2; /* K44:K45:K46:K47 */

/* pattern 3 */
    v0  = AESBCMEMBER(ShiftXorColumns) (v3);       /* k42   :k42^k43:k42^k43^k44:k42^k43^k44^k45  */
    k   = AESBCMEMBER(AesKeyGenAssistColumn3)
                                       (v2,0x80);  /* F(k47): F(k47): F(k47) : F(k47)       */
    v0 ^= k;                                       /* k48   : k49   : k50    : k51          */
    round_keys[12] = v0;                           /* K48:K49:K50:K51 */
}
#else
static __inline__
void AESBCMEMBER(KeyExpansion192) (CipherVec * w, const AESByte * key)
{
    AESBCMEMBER(KeyExpansion) (w, key, AES_Nr_192, AES_Nk_192);
}
#endif




/* ----- */

#if USE_VEC || 1
static __inline__
void AESBCMEMBER(KeyExpansion256) (CipherVec * round_keys, const AESByte * key)
{
    CipherVec v0, v1;
    CipherVec k;

    v0 = AESBCMEMBER (StateIn) (key);
    v1 = AESBCMEMBER (StateIn) (key + sizeof v0);

    round_keys[0] = v0; /* k0  : k1  : k2  : k3  */
    round_keys[1] = v1; /* k4  : k5  : k6  : k7  */

#define KE256_1(offset,rcon)                            \
    k   = AESBCMEMBER(AesKeyGenAssistColumn3)(v1,rcon); \
    v0  = AESBCMEMBER(ShiftXorColumns) (v0);            \
    v0  = AESBCMEMBER(VecXor) (v0, k);                  \
    round_keys[offset] = v0

#define KE256_2(offset) \
    k   = AESBCMEMBER(AesKeyGenAssistColumn2)(v0);      \
    v1  = AESBCMEMBER(ShiftXorColumns) (v1);            \
    v1  = AESBCMEMBER(VecXor) (v1, k);                  \
    round_keys[offset] = v1

    KE256_1(2,0x01);    /* k8  : k9  : k10 : k11 */
    KE256_2(3);         /* k12 : k13 : k14 : k15 */
    KE256_1(4,0x02);    /* k16 : k17 : k18 : k19 */
    KE256_2(5);         /* k20 : k21 : k22 : k23 */
    KE256_1(6,0x04);    /* k24 : k25 : k26 : k27 */
    KE256_2(7);         /* k28 : k29 : k30 : k31 */
    KE256_1(8,0x08);    /* k32 : k33 : k34 : k35 */
    KE256_2(9);         /* k36 : k37 : k38 : k39 */
    KE256_1(10,0x10);   /* k40 : k41 : k42 : k43 */
    KE256_2(11);        /* k44 : k45 : k46 : k47 */
    KE256_1(12,0x20);   /* k48 : k49 : k50 : k51 */
    KE256_2(13);        /* k52 : k53 : k54 : k55 */
    KE256_1(14,0x40);   /* k56 : k57 : k58 : k59 */
}
#else
static __inline__
void AESBCMEMBER(KeyExpansion256) (CipherVec * w, const AESByte * key)
{
    AESBCMEMBER(KeyExpansion) (w, key, AES_Nr_256, AES_Nk_256);
}
#endif


static __inline__
void AESBCMEMBER(EqInvKeyExpansion) (CipherVec * r, const CipherVec * dr, unsigned Nr)
{
    unsigned ix, jx;

    jx = Nr;
    ix = 0;
    r[ix++] = dr[jx--];
    while (jx)
        r[ix++] = AESBCMEMBER(InvMixColumns)(dr[jx--]);
    r[ix] = dr[jx];
}


static __inline__
void AESBCMEMBER(EqInvKeyExpansion128) (CipherVec * w, const CipherVec * kw)
{
    AESBCMEMBER(EqInvKeyExpansion) (w, kw, AES_Nr_128);
}


static __inline__
void AESBCMEMBER(EqInvKeyExpansion192) (CipherVec * w, const CipherVec * kw)
{
    AESBCMEMBER(EqInvKeyExpansion) (w, kw, AES_Nr_192);
}


static __inline__
void AESBCMEMBER(EqInvKeyExpansion256) (CipherVec * w, const CipherVec * kw)
{
    AESBCMEMBER(EqInvKeyExpansion) (w, kw, AES_Nr_256);
}


/*
 * Cipher First Round
 */
static __inline__ 
CipherVec AESBCMEMBER(FirstRound) (CipherVec state, const CipherVec round_keys)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(FirstRound) (CipherVec state, const CipherVec round_keys)
{
    DEBUG_CIPHER_VECTOR("Start of Round", state);

    DEBUG_CIPHER_VECTOR("Round Key", round_keys);

    state = AESBCMEMBER(AddRoundKey) (state, round_keys);

    DEBUG_CIPHER_VECTOR("End 1st Round", state);

    return state;
}


static __inline__ 
CipherVec AESBCMEMBER(MiddleRound) (register CipherVec state,
                                    register const CipherVec round_keys)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(MiddleRound) (register CipherVec state,
                                    register const CipherVec round_keys)
{
#if _DEBUGGING
    CipherVec mstate;
    mstate = state;

    DEBUG_CIPHER_VECTOR("Start of Round", mstate);

#endif

#if USE_AES_NI
    state = op_AESENC (state, round_keys);
#else
    state = AESBCMEMBER(SubBytes) (state);

    DEBUG_CIPHER_VECTOR("After SubBytes", state);

    state = AESBCMEMBER(ShiftRows) (state);

    DEBUG_CIPHER_VECTOR("After ShiftRow", state);

    state = AESBCMEMBER(MixColumns) (state);

    /* ommited n on purpose */
    DEBUG_CIPHER_VECTOR("After MixColum", state);

    state = AESBCMEMBER(AddRoundKey) (state, round_keys);
#endif
    DEBUG_CIPHER_VECTOR("Round Key", round_keys);
    DEBUG_CIPHER_VECTOR("End of Round", state);

    return state;
}


static __inline__ 
CipherVec AESBCMEMBER(LastRound) (register CipherVec state, register CipherVec round_keys)
    __attribute__ ((always_inline));

static __inline__ 
CipherVec AESBCMEMBER(LastRound) (register CipherVec state, register CipherVec round_keys)
{
    DEBUG_CIPHER_VECTOR("Start of Round", state);

#if USE_AES_NI
    state = op_AESENCLAST (state, round_keys);
#else
    state = AESBCMEMBER(SubBytes) (state);

    DEBUG_CIPHER_VECTOR("After SubBytes", state);

    state = AESBCMEMBER(ShiftRows) (state);

    DEBUG_CIPHER_VECTOR("After ShiftRow", state);

    state = AESBCMEMBER(AddRoundKey) (state, round_keys);
#endif
    DEBUG_CIPHER_VECTOR("Round Key", round_keys);
    DEBUG_CIPHER_VECTOR("End of Round", state);

    return state;
}


/* ----------------------------------------------------------------------
 * Cipher
 */
/*
 * 
This implementation as stated
 * above for simplifaction broke out the three types of rounds into
 * first round, middle rounds, and last round.  This approach was taken
 * for ease of coding for optimization using smaller bytes of code
 * but inlining of the functions for efficiency
 */
static __inline__ 
CipherVec AESBCMEMBER(Cipher) (CipherVec state, const CipherVec * key,
                            unsigned Nr)
{
    unsigned ix = 0;

    state = AESBCMEMBER(FirstRound) (state, key[ix++]);

    for ( ; ix < Nr; ++ix)
        state = AESBCMEMBER(MiddleRound) (state, key[ix] );

    return AESBCMEMBER(LastRound) (state, key[ix]);
}


/* 
 * Section 5.1 of FIPS 197
 *
 *   This set of routines implements our structured wrapping of the 
 *   FIPS-197 Section 5.3.5 Equivalent Inverse Cipher.  In particular
 *   figure 15 Pseudo Code for the Equivalent Inverse Cipher.
 *
 *   The FIPS pseudocode has a single function which we have broken out 
 *   into three kinds of rounds - first round, middle rounds and last
 *   round as we have also done with the cipher rountine.
 *
 *   The EqInvCipher operates on whole cipher blocks so the Byte and Block
 *   code is the same here until subroutines of the round functions are
 *   called.  
 */

/* 
 * First round of equivalent inverse cipher is merely an XOR against the
 * round key.
 */
static __inline__
CipherVec AESBCMEMBER(EqInvFirstRound) (CipherVec state, CipherVec round_keys)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(EqInvFirstRound) (CipherVec state, CipherVec round_keys)
{
    DEBUG_INVCIPHER_VECTOR("Start of Round", state);

    state =  AESBCMEMBER(AddRoundKey) (state, round_keys);

    DEBUG_INVCIPHER_VECTOR("Round Key", round_keys);
    DEBUG_INVCIPHER_VECTOR("End of Round", state);
    return state;
}


/*
 * middle rounds of the equivalent inverse cipher perform four steps
 *   InvSubBytes (inverse substitute bytes
 *   InvShiftRows (inverse shift rows
 *   InvMixColumns (inverse mix columns)
 *   AddRoundKey   (xor against the round key)
 *
 * the AES-NI instruction set from until implments all four steps with 
 * a single instruction
 */

static __inline__
CipherVec AESBCMEMBER(EqInvMiddleRound) (register CipherVec state,
                                         register CipherVec round_keys)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(EqInvMiddleRound) (register CipherVec state,
                                         register CipherVec round_keys)
{
#if _DEBUGGING
    CipherVec mstate;
    CipherVec mround_keys;

    mstate = state;
    mround_keys = round_keys;
#endif

    DEBUG_INVCIPHER_VECTOR("Start of Round", mstate);

    /* 
     * AES-NI implmentation
     */
#if USE_AES_NI

    state = op_AESDEC (state, round_keys);

#else /* #if USE_AES_NI */

    /*
     * All other implementations
     */
    state = AESBCMEMBER(InvSubBytes) (state);

# if _DEBUGGING
    mstate = state;

    DEBUG_INVCIPHER_VECTOR("After InvSubBy", mstate);
# endif

    state = AESBCMEMBER(InvShiftRows) (state);

# if _DEBUGGING
    mstate = state;

    DEBUG_INVCIPHER_VECTOR("After InvShift", mstate);
# endif

    state = AESBCMEMBER(InvMixColumns) (state);

# if _DEBUGGING
    mstate = state;

    DEBUG_INVCIPHER_VECTOR("After InvMixCo", mstate);
# endif

    state = AESBCMEMBER(AddRoundKey) (state, round_keys);
#endif

#if _DEBUGGING
    mstate = state;

    DEBUG_INVCIPHER_VECTOR("Round Key", mround_keys);

    DEBUG_INVCIPHER_VECTOR("End of Round", mstate);
#endif /* #if USE_AES_NI */

    return state;
}


/*
 * The final round of the equivalent inverse cipher is the same as 
 * the middle rounds with the ommission of the InvMixColumns step.
 */
static __inline__
CipherVec AESBCMEMBER(EqInvLastRound) (register CipherVec state,
                                       register CipherVec round_keys)
    __attribute__ ((always_inline));

static __inline__
CipherVec AESBCMEMBER(EqInvLastRound) (register CipherVec state,
                                       register CipherVec round_keys)
{
    DEBUG_INVCIPHER_VECTOR("Start of Round", state);

#if USE_AES_NI
    state = op_AESDECLAST (state, round_keys);
#else

    state = AESBCMEMBER(InvSubBytes) (state);

    DEBUG_INVCIPHER_VECTOR("After InvSubBy", state);

    state = AESBCMEMBER(InvShiftRows) (state);

    DEBUG_INVCIPHER_VECTOR("After InvShift", state);

    state = AESBCMEMBER(AddRoundKey) (state, round_keys);
#endif
    DEBUG_INVCIPHER_VECTOR("Round Key", round_keys);
    DEBUG_INVCIPHER_VECTOR("End of Round", state);

    return state;
}


/*
 * The EqInvCipher functions (is defined in Figure 15 'Pseudo Code for
 * for the Equivalent Inverse Cipher.  This implementation as stated
 * above for simplifaction broke out the three types of rounds into
 * first round, middle rounds, and last round.  This approach was taken
 * for ease of coding for optimization using smaller bytes of code
 * but inlining of the functions for efficiency
 */
static /*__inline__*/
CipherVec AESBCMEMBER(EqInvCipher) (CipherVec state, const CipherVec * key,
                                    unsigned Nr)
{
    unsigned ix = 0;

    state = AESBCMEMBER(EqInvFirstRound) (state, key[ix++]);

    for ( ; ix < Nr; ++ix)
        state = AESBCMEMBER(EqInvMiddleRound) (state, key[ix] );

    return AESBCMEMBER(EqInvLastRound) (state, key[ix]);
}


/* ======================================================================
 * This section of the file is the use of the cipher defined above within
 * our BlockCipherObject.
 *
 * These are the functions that will be directly referenced in the vector
 * table for the block cipher.  These are not specifically derived from
 * the FIPS-197 document.
 */
#include "blockcipher-impl.h"

typedef struct AES_CLASS AES_CLASS;

static const char AESBCMEMBER(aes_ncbi_name) [] = CLASS_STRING(AES_CLASS);


/* ----------------------------------------------------------------------
 * Destroy
 *   How large is the stored key for this cipher?  Not the user key used
 *   to create this key (key schedule)
 *
 *   This is needed by KCipher to know how large the KCipher objecr is at
 *   allocation and to know how much of a buffer each decryption/encryption is
 */
static
void AESBCMEMBER(Destroy) (const BLOCKCIPHER_VEC_IMPL * self)
{
    /* we use a constant object so do nothing */
}


/* ----------------------------------------------------------------------
 * BlockSize
 */
static
uint32_t AESBCMEMBER(BlockSize) ()
{
    return (uint32_t) sizeof (AESBlock);
}


/* ----------------------------------------------------------------------
 * KeySize
 *   How large is the stored key for this cipher?  Not the user key used
 *   to create this key (key schedule)
 *
 *   This is needed by KCipher to know how large the KCipher objecr is at
 *   allocation and to know how much of a buffer each decryption/encryption is
 */
static
uint32_t AESBCMEMBER(KeySize) ()
{
    return (uint32_t) sizeof (AESKeySchedule);
}


/* ----------------------------------------------------------------------
 * SetEncryptKey
 *   The KCipher calls this to have the block cipher build an encryption
 *   key in the KCipher object
 *
 */
static
rc_t AESBCMEMBER(SetEncryptKey) (void * encrypt_key, const char * user_key,
                                 uint32_t user_key_size)
{
    rc_t rc;
    AESKeySchedule * key;

    rc = 0;
    key = encrypt_key;

    switch (user_key_size)
    {
    case AES_Nk_128 * sizeof (AESWord):
        key->number_of_rounds = AES_Nr_128;
        AESBCMEMBER(KeyExpansion128) (key->round_keys, (AESByte*)user_key);
        break;

    case AES_Nk_192 * sizeof (AESWord):
        key->number_of_rounds = AES_Nr_192;
        AESBCMEMBER(KeyExpansion192) (key->round_keys, (AESByte*)user_key);
        break;

    case AES_Nk_256 * sizeof (AESWord):
        key->number_of_rounds = AES_Nr_256;
        AESBCMEMBER(KeyExpansion256) (key->round_keys, (AESByte*)user_key);
        break;

    default:
        key->number_of_rounds = 0;
        memset (key, 0, sizeof (*key));
        rc = RC (rcKrypto, rcBlockCipher, rcUpdating, rcParam, rcInvalid);
        break;
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * SetDecryptKey
 *   The KCipher calls this to have the block cipher build an decryption
 *   key in the KCipher object
 *
 */
static
rc_t AESBCMEMBER(SetDecryptKey) (void * decrypt_key,
                                 const char * user_key,
                                 uint32_t user_key_size)
{
    AESKeySchedule encrypt_key;
    rc_t rc;

    rc = AESBCMEMBER(SetEncryptKey) (&encrypt_key, user_key, user_key_size);
    if (rc == 0)
    {
        AESKeySchedule * key;

        key = decrypt_key;

        switch (key->number_of_rounds = encrypt_key.number_of_rounds)
        {
        case AES_Nr_128:
            AESBCMEMBER(EqInvKeyExpansion128) (key->round_keys,
                                               encrypt_key.round_keys);
            break;

        case AES_Nr_192:
            AESBCMEMBER(EqInvKeyExpansion192) (key->round_keys,
                                               encrypt_key.round_keys);
            break;

        case AES_Nr_256:
            AESBCMEMBER(EqInvKeyExpansion256) (key->round_keys,
                                               encrypt_key.round_keys);
            break;
        }
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * Encrypt
 *
 *   Perform an encryption of a single block.  Chained modes and stream
 *   cipher modes will call this multiple times.
 *
 */
static
CipherVec AESBCMEMBER(Encrypt) (CipherVec state,
                                const void * encrypt_key)
{
    const AESKeySchedule * key = encrypt_key;
    assert (key);

    switch (key->number_of_rounds)
    {
    default:
        state = vec_00;
        break;

    case AES_Nr_128:
        state = AESBCMEMBER(Cipher) (state, key->round_keys, AES_Nr_128);
        break;

    case AES_Nr_192:
        state = AESBCMEMBER(Cipher) (state, key->round_keys, AES_Nr_192);
        break;

    case AES_Nr_256:
        state = AESBCMEMBER(Cipher) (state, key->round_keys, AES_Nr_256);
        break;
    }

    return state;
}


/* ----------------------------------------------------------------------
 * Decrypt
 *
 *   Perform a decryption of a single block.  Chained modes and stream
 *   cipher modes will call this multiple times.
 */
static
CipherVec AESBCMEMBER(Decrypt) (CipherVec state,
                                const void * decrypt_key)
{
    const AESKeySchedule * key = decrypt_key;

    assert (key);

    switch (key->number_of_rounds)
    {
    default:
        state = vec_00; /* for an illegal cipher key schedule just write zeroes */
        break;

    case AES_Nr_128:
        state = AESBCMEMBER(EqInvCipher) (state, key->round_keys,
                                          AES_Nr_128);
        break;

    case AES_Nr_192:
        state = AESBCMEMBER(EqInvCipher) (state, key->round_keys,
                                          AES_Nr_192);
        break;

    case AES_Nr_256:
        state = AESBCMEMBER(EqInvCipher) (state, key->round_keys,
                                          AES_Nr_256);
        break;
    }

    return state;
}


/* ----------------------------------------------------------------------
 * MakeProcessorSupport
 *
 * Run-time check for support of this particular AES implmentation by
 * checking the flags given via the cpuid instruction.
 */
static __inline__
bool AESBCMEMBER(ProcessorSupport) ()
{
    uint32_t level = 1, a, b, c, d;
    bool ret = true;

    __get_cpuid (level, &a, &b, &c, &d);

    DEBUG_OBJECT(("%s: level %x a %x, b %x c %x d %x\n",__func__, level, a,b,c,d));

#if USE_AES_NI
    /*
     * Only one flag tells whether the AES-NI instructions
     * are implmented in this CPU
     */
    if ( ! (c & bit_AES))
    {
        DEBUG_OBJECT(("%s: failed no AES\n",__func__));
        ret = false;
    } else
#endif

#if USE_VEC_REG
    /*
     * Several flags indicate different levels of SSE
     * instruction implmentation
     *
     * we require SSE4.1
     *
     * that dates from 2006 so is most likely good enough
     */
        /* if ( ! (c & (bit_SSE4_2))
    {
        DEBUG_OBJECT(("%s: failed no SSSE3\n",__func__));
        ret = false;
    }

    else */
        if ( ! (c & bit_SSE4_1))
    {
        DEBUG_OBJECT(("%s: failed no SSSE3\n",__func__));
        ret = false;
    }

    else if ( ! (c & bit_SSSE3))
    {
        DEBUG_OBJECT(("%s: failed no SSSE3\n",__func__));
        ret = false;
    }

    else if ( ! (c & bit_SSE3))
    {
        DEBUG_OBJECT(("%s: failed no SSE3\n",__func__));
        ret = false;
    }

    else if ( ! (d & bit_SSE2))
    {
        DEBUG_OBJECT(("%s: failed no SSE2\n",__func__));
        ret = false;
    }

    else if ( ! (d & bit_SSE))
    {
        DEBUG_OBJECT(("%s: failed no SSE\n",__func__));
        ret = false;
    }
#endif
    DEBUG_OBJECT(("%s: passed\n",__func__));
    return ret;
}


/* ----------------------------------------------------------------------
 * Init
 *
 *   Initialize the fields of this object.  The derived class will call this
 *   during it's initialization.
 *
 * self      object to initialze
 * vt        the virtual table of the derived class
 * mgr       the cipher manager that is the construction factory block cipher
 *           objects hold references to the manager while the manager merely
 *           points at the block ciphers when all block ciphers are destroyed
 *           the manager loses its references and it too can be destroyed if not
 *           held elsewhere
 * name      ASCIZ c-string the name of this class
 */
static const
KBlockCipherVec_vt_v1 AESBCMEMBER(_vt_) = 
{
    { 1, 1 },

    AESBCMEMBER(Destroy),
    AESBCMEMBER(BlockSize),
    AESBCMEMBER(KeySize),
    AESBCMEMBER(SetEncryptKey),
    AESBCMEMBER(SetDecryptKey),
    AESBCMEMBER(Encrypt),
    AESBCMEMBER(Decrypt)
};


/* ----------------------------------------------------------------------
 * Make
 *
 * Create a new AES Block Cipher object.
 * The processor is checked to see if this particular version is supported on
 * this particular CPU.
 */
rc_t AESBCMEMBER(Make) (KBlockCipher ** new_obj)
{
    rc_t rc;

    DEBUG_OBJECT(("%s: %p\n",__func__,new_obj));

    /* Check parameter first */
    if (new_obj == NULL)
        rc = RC (rcKrypto, rcBlockCipher, rcConstructing, rcSelf, rcNull);

    else
    {
        *new_obj = NULL;

        /* check for processor support of this flavor */
        if ( ! AESBCMEMBER(ProcessorSupport)())
            rc = RC (rcKrypto, rcBlockCipher, rcConstructing, rcFunction, rcUnsupported);

        else
        {
            rc = 0;
            *new_obj = (KBlockCipher*)&(AESBCMEMBER(_vt_));
        }
    }
    return rc;
}

/* end of file lib/krypto/aes-ncbi.c */
