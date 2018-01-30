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
#include <klib/checksum.h>
#include <arch-impl.h>

#include <endian.h>
#include <byteswap.h>

#include <stdint.h>
#include <string.h>

#if _ARCH_BITS == 32
#define UL( x ) x ## ULL
#else
#define UL( x ) x ## UL
#endif

static const uint64_t H0_384[8] =
{
    UL(0xcbbb9d5dc1059ed8),
    UL(0x629a292a367cd507),
    UL(0x9159015a3070dd17),
    UL(0x152fecd8f70e5939),
    UL(0x67332667ffc00b31),
    UL(0x8eb44a8768581511),
    UL(0xdb0c2e0d64f98fa7),
    UL(0x47b5481dbefa4fa4),
};

static const uint64_t H0_512[8] =
{
    UL(0x6a09e667f3bcc908),
    UL(0xbb67ae8584caa73b),
    UL(0x3c6ef372fe94f82b),
    UL(0xa54ff53a5f1d36f1),
    UL(0x510e527fade682d1),
    UL(0x9b05688c2b3e6c1f),
    UL(0x1f83d9abfb41bd6b),
    UL(0x5be0cd19137e2179),
};

static const uint64_t K[] =
{
    UL(0x428a2f98d728ae22), UL(0x7137449123ef65cd), UL(0xb5c0fbcfec4d3b2f), UL(0xe9b5dba58189dbbc), 
    UL(0x3956c25bf348b538), UL(0x59f111f1b605d019), UL(0x923f82a4af194f9b), UL(0xab1c5ed5da6d8118), 
    UL(0xd807aa98a3030242), UL(0x12835b0145706fbe), UL(0x243185be4ee4b28c), UL(0x550c7dc3d5ffb4e2), 
    UL(0x72be5d74f27b896f), UL(0x80deb1fe3b1696b1), UL(0x9bdc06a725c71235), UL(0xc19bf174cf692694), 
    UL(0xe49b69c19ef14ad2), UL(0xefbe4786384f25e3), UL(0x0fc19dc68b8cd5b5), UL(0x240ca1cc77ac9c65), 
    UL(0x2de92c6f592b0275), UL(0x4a7484aa6ea6e483), UL(0x5cb0a9dcbd41fbd4), UL(0x76f988da831153b5), 
    UL(0x983e5152ee66dfab), UL(0xa831c66d2db43210), UL(0xb00327c898fb213f), UL(0xbf597fc7beef0ee4), 
    UL(0xc6e00bf33da88fc2), UL(0xd5a79147930aa725), UL(0x06ca6351e003826f), UL(0x142929670a0e6e70), 
    UL(0x27b70a8546d22ffc), UL(0x2e1b21385c26c926), UL(0x4d2c6dfc5ac42aed), UL(0x53380d139d95b3df), 
    UL(0x650a73548baf63de), UL(0x766a0abb3c77b2a8), UL(0x81c2c92e47edaee6), UL(0x92722c851482353b), 
    UL(0xa2bfe8a14cf10364), UL(0xa81a664bbc423001), UL(0xc24b8b70d0f89791), UL(0xc76c51a30654be30), 
    UL(0xd192e819d6ef5218), UL(0xd69906245565a910), UL(0xf40e35855771202a), UL(0x106aa07032bbd1b8), 
    UL(0x19a4c116b8d2d0c8), UL(0x1e376c085141ab53), UL(0x2748774cdf8eeb99), UL(0x34b0bcb5e19b48a8), 
    UL(0x391c0cb3c5c95a63), UL(0x4ed8aa4ae3418acb), UL(0x5b9cca4f7763e373), UL(0x682e6ff3d6b2b8a3), 
    UL(0x748f82ee5defb2fc), UL(0x78a5636f43172f60), UL(0x84c87814a1f0ab72), UL(0x8cc702081a6439ec), 
    UL(0x90befffa23631e28), UL(0xa4506cebde82bde9), UL(0xbef9a3f7b2c67915), UL(0xc67178f2e372532b), 
    UL(0xca273eceea26619c), UL(0xd186b8c721c0c207), UL(0xeada7dd6cde0eb1e), UL(0xf57d4f7fee6ed178), 
    UL(0x06f067aa72176fba), UL(0x0a637dc5a2c898a6), UL(0x113f9804bef90dae), UL(0x1b710b35131c471b), 
    UL(0x28db77f523047d84), UL(0x32caab7b40c72493), UL(0x3c9ebe0a15c9bebc), UL(0x431d67c49c100d4c), 
    UL(0x4cc5d4becb3e42b6), UL(0x597f299cfc657e2a), UL(0x5fcb6fab3ad6faec), UL(0x6c44198c4a475817), 
};

#undef UL

#define ROL(X, N) uint64_rol ( X, N )
#define ROR(X, N) uint64_ror ( X, N )
#define SHR(X, N) ((X) >> (N))

#define Ch(X, Y, Z) (((X) & (Y))^((~(X))&(Z)))
#define Maj(X, Y, Z) (((X)&(Y))^((X)&(Z))^((Y)&(Z)))
#define Parity(X, Y, Z) ((X)^(Y)^(Z))
#define Sigma0(X) (ROR((X), 28)^ROR((X), 34)^ROR((X), 39))
#define Sigma1(X) (ROR((X), 14)^ROR((X), 18)^ROR((X), 41))
#define sigma0(X) (ROR((X),  1)^ROR((X),  8)^SHR((X), 7))
#define sigma1(X) (ROR((X), 19)^ROR((X), 61)^SHR((X), 6))

static void SHA_stage(uint64_t Ho[8], const uint64_t Hi[8], uint64_t W[16])
{
    uint64_t a = Hi[0];
    uint64_t b = Hi[1];
    uint64_t c = Hi[2];
    uint64_t d = Hi[3];
    uint64_t e = Hi[4];
    uint64_t f = Hi[5];
    uint64_t g = Hi[6];
    uint64_t h = Hi[7];
    unsigned t;
    
    for (t = 0; t != 16; ++t) {
        const uint64_t T1 = h + Sigma1(e) + Ch(e, f, g) + K[t] + W[t];
        const uint64_t T2 = Sigma0(a) + Maj(a, b, c);
        
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }
    for ( ; t != 80; ++t) {
        const uint64_t T1 = h + Sigma1(e) + Ch(e, f, g) + K[t] +
            (W[t%16] = sigma1(W[(t-2)%16])+W[(t-7)%16]+sigma0(W[(t-15)%16])+W[(t-16)%16]);
        const uint64_t T2 = Sigma0(a) + Maj(a, b, c);
        
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }
    Ho[0] = Hi[0] + a;
    Ho[1] = Hi[1] + b;
    Ho[2] = Hi[2] + c;
    Ho[3] = Hi[3] + d;
    Ho[4] = Hi[4] + e;
    Ho[5] = Hi[5] + f;
    Ho[6] = Hi[6] + g;
    Ho[7] = Hi[7] + h;
}

static __inline__ void SHA_64b_Init(struct SHA64bitState *ctx, int which)
{
    if (which == 0)
        memmove(ctx->H, H0_384, sizeof(H0_384));
    else
        memmove(ctx->H, H0_512, sizeof(H0_512));
    ctx->len = 0;
    ctx->cur = 0;
}

static __inline__ void SHA_64b_Append(struct SHA64bitState *ctx, const uint8_t data[], size_t length)
{
    unsigned i;
    unsigned j;
    unsigned n;
    
    ctx->len += length;
    for (i = 0; i != length; i += n) {
        n = length - i > 128 ? 128 : (unsigned)(length - i);
        if (ctx->cur + n > 128)
            n = 128 - ctx->cur;
        memmove(&ctx->W[ctx->cur], &data[i], n);
        ctx->cur += n;
        if (ctx->cur == 128) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            uint64_t W[16];
            
            for (j = 0; j != 16; ++j)
                W[j] = bswap_64(((const uint64_t *)ctx->W)[j]);
            SHA_stage(ctx->H, ctx->H, W);
#else
            SHA_stage(ctx->H, ctx->H, ctx->W);
#endif
            ctx->cur = 0;
        }
    }
}


static __inline__ void SHA_64b_Finish(const struct SHA64bitState *ctx, uint64_t rslt[8])
{
    unsigned j;
    unsigned const n = (ctx->cur + 7) >> 3;
    uint64_t W[32];
    
    memset(W, 0, sizeof(W));
    for (j = 0; j != n; ++j) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        W[j] = bswap_64(((const uint64_t *)ctx->W)[j]);
#else
        W[j] = ((const uint64_t *)ctx->W)[j];
#endif
    }
    if (ctx->cur & 7) {
        --j;
        W[j] &= (~((uint64_t)0)) << ((8 - (ctx->cur & 7)) * 8);
        W[j] |= ((uint64_t)0x80U) << ((7 - (ctx->cur & 7)) * 8);
    }
    else
        W[j] = ((uint64_t)0x80U) << 56;
    if (j < 14) {
        W[14] = ctx->len >> 61;
        W[15] = ctx->len <<  3;
        SHA_stage(rslt, ctx->H, W);
    }
    else {
        W[30] = ctx->len >> 61;
        W[31] = ctx->len <<  3;
        SHA_stage(rslt, ctx->H, W);
        SHA_stage(rslt, rslt, W + 16);
    }
}

void CC SHA384StateInit(SHA384State *ctx)
{
    SHA_64b_Init(ctx, 0);
}

void CC SHA384StateAppend(SHA384State *ctx, const void *data, size_t length)
{
    SHA_64b_Append(ctx, data, length);
}

void CC SHA384StateFinish(SHA384State *ctx, uint8_t hash[48])
{
    uint64_t H[8];

    SHA_64b_Finish(ctx, H);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    {
        H[0] = bswap_64(H[0]);
        H[1] = bswap_64(H[1]);
        H[2] = bswap_64(H[2]);
        H[3] = bswap_64(H[3]);
        H[4] = bswap_64(H[4]);
        H[5] = bswap_64(H[5]);
    }
#endif
    memmove(hash, H, 48);
}

void CC SHA512StateInit(SHA512State *ctx)
{
    SHA_64b_Init(ctx, 1);
}

void CC SHA512StateAppend(SHA512State *ctx, const void *data, size_t length)
{
    SHA_64b_Append(ctx, data, length);
}

void CC SHA512StateFinish(SHA512State *ctx, uint8_t hash[64])
{
    uint64_t H[8];

    SHA_64b_Finish(ctx, H);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    {
        H[0] = bswap_64(H[0]);
        H[1] = bswap_64(H[1]);
        H[2] = bswap_64(H[2]);
        H[3] = bswap_64(H[3]);
        H[4] = bswap_64(H[4]);
        H[5] = bswap_64(H[5]);
        H[6] = bswap_64(H[6]);
        H[7] = bswap_64(H[7]);
    }
#endif
    memmove(hash, H, 64);
}
