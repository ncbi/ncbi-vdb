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

#ifndef _h_arch_impl_
#define _h_arch_impl_

#include <stdint.h>

/* Intel versions from 11.0 through 13.0 mangle certain shift
 * statements in inline assembly. */
#if defined(__INTEL_COMPILER)  &&  __INTEL_COMPILER_BUILD_DATE >= 20090131 \
    &&  __INTEL_COMPILER_BUILD_DATE < 20130607 
#  define HAVE_ICC_SHIFT_BUG 1 
#endif

#ifndef USE_GCC_BUILTIN
#define USE_GCC_BUILTIN 1
#endif

#if USE_GCC_BUILTIN
#include <strings.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

static __inline__
int16_t uint16_lsbit ( uint16_t self )
{
    return ( int16_t ) __builtin_ffs ( self ) - 1;
}

static __inline__
int32_t uint32_lsbit ( uint32_t self )
{
    return __builtin_ffs ( self ) - 1;
}

static __inline__
int32_t uint64_lsbit ( uint64_t self )
{
    return __builtin_ffsll ( self ) - 1;
}

static __inline__
int16_t uint16_msbit ( uint16_t self )
{
    if ( self != 0 )
        return ( int16_t ) 31 - __builtin_clz ( ( uint32_t ) self );
    return -1;
}

static __inline__
int32_t uint32_msbit ( uint32_t self )
{
    if ( self != 0 )
        return 31 - __builtin_clz ( self );
    return -1;
}

static __inline__
int32_t uint64_msbit ( uint64_t self )
{
    if ( self != 0 )
        return 63 - __builtin_clzll ( self );
    return -1;
}

typedef struct int128_t int128_t;
struct int128_t
{
    uint64_t lo;
    int64_t hi;
};

static __inline__
int64_t int128_hi ( const int128_t *self )
{
    return self -> hi;
}

static __inline__
uint64_t int128_lo ( const int128_t *self )
{
    return self -> lo;
}

static __inline__
void int128_sethi ( int128_t *self, int64_t i )
{
    self -> hi = i;
}

static __inline__
void int128_setlo ( int128_t *self, uint64_t i )
{
    self -> lo = i;
}

typedef struct uint128_t uint128_t;
struct uint128_t
{
    uint64_t lo;
    uint64_t hi;
};

static __inline__
uint64_t uint128_hi ( const uint128_t *self )
{
    return self -> hi;
}

static __inline__
uint64_t uint128_lo ( const uint128_t *self )
{
    return self -> lo;
}

static __inline__
void uint128_sethi ( uint128_t *self, uint64_t i )
{
    self -> hi = i;
}

static __inline__
void uint128_setlo ( uint128_t *self, uint64_t i )
{
    self -> lo = i;
}

static __inline__
void int128_add ( int128_t *self, const int128_t *i )
{
    __asm__ __volatile__
    (
        "mov (%%rsi), %%rax;"
        "mov 8(%%rsi), %%rcx;"
        "add %%rax, (%%rdi);"
        "adc %%rcx, 8(%%rdi);"
        :
        : "D" ( self ), "S" ( i )
        : "%rax", "%rcx"
    );
}

static __inline__
void int128_sub ( int128_t *self, const int128_t *i )
{
    __asm__ __volatile__
    (
        "mov (%%rsi), %%rax;"
        "mov 8(%%rsi), %%rcx;"
        "sub %%rax, (%%rdi);"
        "sbb %%rcx, 8(%%rdi);"
        :
        : "D" ( self ), "S" ( i )
        : "%rax", "%rcx"
    );
}

static __inline__
void int128_sar ( int128_t *self, uint32_t i )
{
    __asm__ __volatile__
    (
        "mov %%esi, %%ecx;"
        "mov 8(%%rdi), %%rax;"
        "shrd %%cl, %%rax, (%%rdi);"
        "sar %%cl, %%rax;"
        "mov %%rax, 8(%%rdi);"
        :
        : "D" ( self ), "S" ( i )
        :  "%rax", "%rcx"
    );
}

static __inline__
void int128_shl ( int128_t *self, uint32_t i )
{
    __asm__ __volatile__
    (
        "mov %%esi, %%ecx;"
        "mov (%%rdi), %%rax;"
        "shld %%cl, %%rax, 8(%%rdi);"
        "shl %%cl, %%rax;"
        "mov %%rax, (%%rdi);"
        :
        : "D" ( self ), "S" ( i )
        : "%rax", "%rcx"
    );
}

static __inline__
void uint128_and ( uint128_t *self, const uint128_t *i )
{
    __asm__ __volatile__
    (
        "mov (%%rsi), %%rax;"
        "mov 8(%%rsi), %%rcx;"
        "and %%rax, (%%rdi);"
        "and %%rcx, 8(%%rdi);"
        :
        : "D" ( self ), "S" ( i )
        :"%rax", "%rcx"
    );
}

static __inline__
void uint128_or ( uint128_t *self, const uint128_t *i )
{
    __asm__ __volatile__
    (
        "mov (%%rsi), %%rax;"
        "mov 8(%%rsi), %%rcx;"
        "or %%rax, (%%rdi);"
        "or %%rcx, 8(%%rdi);"
        :
        : "D" ( self ), "S" ( i )
        :"%rax", "%rcx"
    );
}

static __inline__
void uint128_orlo ( uint128_t *self, uint64_t i )
{
    self -> lo |= i;
}

static __inline__
void uint128_xor ( uint128_t *self, const uint128_t *i )
{
    __asm__ __volatile__
    (
        "mov (%%rsi), %%rax;"
        "mov 8(%%rsi), %%rcx;"
        "xor %%rax, (%%rdi);"
        "xor %%rcx, 8(%%rdi);"
        :
        : "D" ( self ), "S" ( i )
        :"%rax", "%rcx"
    );
}

static __inline__
void uint128_not ( uint128_t *self )
{
    __asm__ __volatile__
    (
        "mov (%%rdi), %%rax;"
        "mov 8(%%rdi), %%rcx;"
        "not %%rax;"
        "not %%rcx;"
        "mov %%rax, (%%rdi);"
        "mov %%rcx, 8(%%rdi);"
        :
        : "D" ( self )
        : "%rax", "%rcx"
    );
}

static __inline__
void uint128_shr ( uint128_t *self, uint32_t i )
{
#ifdef HAVE_ICC_SHIFT_BUG
    if (i > 0) {
        self->lo >>= i;
        self->lo  |= self->hi << (64 - i);
        self->hi >>= i;
    }
#else
    __asm__ __volatile__
    (
        "mov %%esi, %%ecx;"
        "mov 8(%%rdi), %%rax;"
        "shrd %%cl, %%rax, (%%rdi);"
        "shr %%cl, %%rax;"
        "mov %%rax, 8(%%rdi);"
        :
        : "D" ( self ), "S" ( i )
        : "%rax", "%rcx"
    );
#endif
}

static __inline__
void uint128_shl ( uint128_t *self, uint32_t i )
{
    __asm__ __volatile__
    (
        "mov %%esi, %%ecx;"
        "mov (%%rdi), %%rax;"
        "shld %%cl, %%rax, 8(%%rdi);"
        "shl %%cl, %%rax;"
        "mov %%rax, (%%rdi);"
        :
        : "D" ( self ), "S" ( i )
        : "%rax", "%rcx"
    );
}

static __inline__
void uint128_bswap ( uint128_t *self )
{
    __asm__ __volatile__
    (
        "mov (%%rdi), %%rax;"
        "mov 8(%%rdi), %%rcx;"
        "bswap %%rax;"
        "bswap %%rcx;"
        "mov %%rax, 8(%%rdi);"
        "mov %%rcx, (%%rdi);"
        :
        : "D" ( self )
        : "%rax", "%rcx"
    );
}

static __inline__
void uint128_bswap_copy ( uint128_t *to, const uint128_t *from )
{
    __asm__ __volatile__
    (
        "mov (%%rsi), %%rax;"
        "mov 8(%%rsi), %%rcx;"
        "bswap %%rax;"
        "bswap %%rcx;"
        "mov %%rax, 8(%%rdi);"
        "mov %%rcx, (%%rdi);"
        :
        : "D" ( to ), "S" ( from )
        : "%rax", "%rcx"
    );
}

static __inline__
uint32_t uint32_rol ( uint32_t val, uint8_t bits )
{
    uint32_t rtn;
    __asm__ __volatile__
    (
        "rol %%cl, %%eax;"
        : "=a" ( rtn )
        : "a" ( val ), "c" ( bits )
    );
    return rtn;
}

static __inline__
uint32_t uint32_ror ( uint32_t val, uint8_t bits )
{
    uint32_t rtn;
    __asm__ __volatile__
    (
        "ror %%cl, %%eax;"
        : "=a" ( rtn )
        : "a" ( val ), "c" ( bits )
    );
    return rtn;
}

static __inline__
uint64_t uint64_rol ( uint64_t val, uint8_t bits )
{
    uint64_t rtn;
    __asm__ __volatile__
    (
        "rol %%cl, %%rax;"
        : "=a" ( rtn )
        : "a" ( val ), "c" ( bits )
    );
    return rtn;
}

static __inline__
uint64_t uint64_ror ( uint64_t val, uint8_t bits )
{
    uint64_t rtn;
    __asm__ __volatile__
    (
        "ror %%cl, %%rax;"
        : "=a" ( rtn )
        : "a" ( val ), "c" ( bits )
    );
    return rtn;
}


#ifdef __cplusplus
}
#endif

#endif /* _h_arch_impl_ */
