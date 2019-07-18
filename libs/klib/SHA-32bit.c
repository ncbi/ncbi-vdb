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

static const uint32_t H0_1[5] =
{
    0x67452301U,
    0xefcdab89U,
    0x98badcfeU,
    0x10325476U,
    0xc3d2e1f0U,
};

static const uint32_t K_1[4] =
{
    0x5a827999U,
    0x6ed9eba1U,
    0x8f1bbcdcU,
    0xca62c1d6U
};

static const uint32_t H0_2[8] =
{
    0x6a09e667U,
    0xbb67ae85U,
    0x3c6ef372U,
    0xa54ff53aU,
    0x510e527fU,
    0x9b05688cU,
    0x1f83d9abU,
    0x5be0cd19U,
};

static const uint32_t K_2[] =
{
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 
    0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U, 
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U, 
    0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U, 
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU, 
    0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, 
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 
    0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U, 
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U, 
    0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U, 
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 
    0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U, 
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, 
    0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U, 
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U, 
    0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U, 
};

#if 0 /* this is slower than pure C version */
#define ROL(X, N) uint32_rol ( X, N )
#define ROR(X, N) uint32_ror ( X, N )
#else
#define ROL(X, N) (((X) << (N)) | ((X) >> (32 - (N))))
#define ROR(X, N) ROL(X, 32-N)
#endif
#define SHR(X, N) ((X) >> (N))

#define Ch(X, Y, Z) (((X) & (Y))^((~(X))&(Z)))
#define Maj(X, Y, Z) (((X)&(Y))^((X)&(Z))^((Y)&(Z)))
#define Parity(X, Y, Z) ((X)^(Y)^(Z))
#define Sigma0(X) (ROR((X),  2)^ROR((X), 13)^ROR((X), 22))
#define Sigma1(X) (ROR((X),  6)^ROR((X), 11)^ROR((X), 25))
#define sigma0(X) (ROR((X),  7)^ROR((X), 18)^SHR((X),  3))
#define sigma1(X) (ROR((X), 17)^ROR((X), 19)^SHR((X), 10))

static void SHA1_stage(uint32_t Ho[8], const uint32_t Hi[8], uint32_t W[16])
{
    uint32_t a = Hi[0];
    uint32_t b = Hi[1];
    uint32_t c = Hi[2];
    uint32_t d = Hi[3];
    uint32_t e = Hi[4];
    unsigned t;
    
    for (t = 0; t != 16; ++t) {
        const uint32_t T = ROL(a, 5) + Ch(b, c, d) + e + K_1[0] + W[t];
        
        e = d; d = c; c = ROL(b, 30); b = a; a = T;
    }
    for ( ; t != 20; ++t) {
        const uint32_t T = ROL(a, 5) + Ch(b, c, d) + e + K_1[0] +
            (W[t%16] = ROL(W[(t-3)%16] ^ W[(t-8)%16] ^ W[(t-14)%16] ^ W[(t-16)%16], 1));
        
        e = d; d = c; c = ROL(b, 30); b = a; a = T;
    }
    for ( ; t != 40; ++t) {
        const uint32_t T = ROL(a, 5) + Parity(b, c, d) + e + K_1[1] +
            (W[t%16] = ROL(W[(t-3)%16] ^ W[(t-8)%16] ^ W[(t-14)%16] ^ W[(t-16)%16], 1));
        
        e = d; d = c; c = ROL(b, 30); b = a; a = T;
    }
    for ( ; t != 60; ++t) {
        const uint32_t T = ROL(a, 5) + Maj(b, c, d) + e + K_1[2] +
            (W[t%16] = ROL(W[(t-3)%16] ^ W[(t-8)%16] ^ W[(t-14)%16] ^ W[(t-16)%16], 1));
        
        e = d; d = c; c = ROL(b, 30); b = a; a = T;
    }
    for ( ; t != 80; ++t) {
        const uint32_t T = ROL(a, 5) + Parity(b, c, d) + e + K_1[3] +
            (W[t%16] = ROL(W[(t-3)%16] ^ W[(t-8)%16] ^ W[(t-14)%16] ^ W[(t-16)%16], 1));
        
        e = d; d = c; c = ROL(b, 30); b = a; a = T;
    }
    Ho[0] = Hi[0] + a;
    Ho[1] = Hi[1] + b;
    Ho[2] = Hi[2] + c;
    Ho[3] = Hi[3] + d;
    Ho[4] = Hi[4] + e;
}

static void SHA2_stage(uint32_t Ho[8], const uint32_t Hi[8], uint32_t W[16])
{
    uint32_t a = Hi[0];
    uint32_t b = Hi[1];
    uint32_t c = Hi[2];
    uint32_t d = Hi[3];
    uint32_t e = Hi[4];
    uint32_t f = Hi[5];
    uint32_t g = Hi[6];
    uint32_t h = Hi[7];
    unsigned t;
    
    for (t = 0; t != 16; ++t) {
        const uint32_t T1 = h + Sigma1(e) + Ch(e, f, g) + K_2[t] + W[t];
        const uint32_t T2 = Sigma0(a) + Maj(a, b, c);
        
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }
    for ( ; t != 64; ++t) {
        const uint32_t T1 = h + Sigma1(e) + Ch(e, f, g) + K_2[t] +
            (W[t%16] = sigma1(W[(t-2)%16])+W[(t-7)%16]+sigma0(W[(t-15)%16])+W[(t-16)%16]);
        const uint32_t T2 = Sigma0(a) + Maj(a, b, c);
        
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

static __inline__ void SHA_32b_Init(struct SHA32bitState *ctx, int which)
{
    if (which == 0)
        memmove(ctx->H, H0_1, sizeof(H0_1));
    else
        memmove(ctx->H, H0_2, sizeof(H0_2));
    ctx->len = 0;
    ctx->cur = 0;
}

static __inline__ void SHA_32b_Append(struct SHA32bitState *ctx, int which, const uint8_t data[], size_t length)
{
    unsigned i;
    unsigned j;
    unsigned n;
    
    ctx->len += length;
    for (i = 0; i != length; i += n) {
        n = length - i > 64 ? 64 : (unsigned)(length - i);
        if (ctx->cur + n > 64)
            n = 64 - ctx->cur;
        memmove(&ctx->W[ctx->cur], &data[i], n);
        ctx->cur += n;
        if (ctx->cur == 64) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            uint32_t W[16];
            
            for (j = 0; j != 16; ++j)
                W[j] = bswap_32(((const uint32_t *)ctx->W)[j]);
            
            (which ? SHA2_stage : SHA1_stage)(ctx->H, ctx->H, W);
#else
            (which ? SHA2_stage : SHA1_stage)(ctx->H, ctx->H, ctx->W);
#endif
            ctx->cur = 0;
        }
    }
}

static __inline__ void SHA_32b_Finish(const struct SHA32bitState *ctx, int which, uint32_t rslt[8])
{
    unsigned j;
    unsigned const n = (ctx->cur + 3) >> 2;
    uint32_t W[32];
    
    memset(W, 0, sizeof(W));
    for (j = 0; j != n; ++j) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        W[j] = bswap_32(((const uint32_t *)ctx->W)[j]);
#else
        W[j] = ((const uint32_t *)ctx->W)[j];
#endif
    }
    if (ctx->cur & 3) {
        --j;
        W[j] &= (~((uint32_t)0)) << ((4 - (ctx->cur & 3)) * 8);
        W[j] |= ((uint32_t)0x80U) << ((3 - (ctx->cur & 3)) * 8);
    }
    else
        W[j] = ((uint32_t)0x80U) << 24;
    if (j < 14) {
        W[14] = (uint32_t)(ctx->len >> 29);
        W[15] = (uint32_t)(ctx->len <<  3);
        (which ? SHA2_stage : SHA1_stage)(rslt, ctx->H, W);
    }
    else {
        W[30] = (uint32_t)(ctx->len >> 29);
        W[31] = (uint32_t)(ctx->len <<  3);
        (which ? SHA2_stage : SHA1_stage)(rslt, ctx->H, W);
        (which ? SHA2_stage : SHA1_stage)(rslt, rslt, W + 16);
    }
}

void CC SHA1StateInit(SHA1State *ctx)
{
    SHA_32b_Init(ctx, 0);
}

void CC SHA1StateAppend(SHA1State *ctx, const void *data, size_t length)
{
    SHA_32b_Append(ctx, 0, data, length);
}

void CC SHA1StateFinish(SHA1State *ctx, uint8_t hash[20])
{
    uint32_t H[8];

    SHA_32b_Finish(ctx, 0, H);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    {
        H[0] = bswap_32(H[0]);
        H[1] = bswap_32(H[1]);
        H[2] = bswap_32(H[2]);
        H[3] = bswap_32(H[3]);
        H[4] = bswap_32(H[4]);
    }
#endif
    memmove(hash, H, 20);
}

void CC SHA256StateInit(SHA256State *ctx)
{
    SHA_32b_Init(ctx, 1);
}

void CC SHA256StateAppend(SHA256State *ctx, const void *data, size_t length)
{
    SHA_32b_Append(ctx, 1, data, length);
}

void CC SHA256StateFinish(SHA256State *ctx, uint8_t hash[32])
{
    uint32_t H[8];

    SHA_32b_Finish(ctx, 1, H);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    {
        H[0] = bswap_32(H[0]);
        H[1] = bswap_32(H[1]);
        H[2] = bswap_32(H[2]);
        H[3] = bswap_32(H[3]);
        H[4] = bswap_32(H[4]);
        H[5] = bswap_32(H[5]);
        H[6] = bswap_32(H[6]);
        H[7] = bswap_32(H[7]);
    }
#endif
    memmove(hash, H, 32);
}
