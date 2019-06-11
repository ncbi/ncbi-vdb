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
        "push %%ebx;"
        "mov (%%edx), %%eax;"
        "mov 4(%%edx), %%ebx;"
        "mov 8(%%edx), %%esi;"
        "mov 12(%%edx), %%edi;"
        "add %%eax, (%%ecx);"
        "adc %%ebx, 4(%%ecx);"
        "adc %%esi, 8(%%ecx);"
        "adc %%edi, 12(%%ecx);"
        "pop %%ebx;"
        :
        : "c" ( self ), "d" ( i )
        : "%eax", "%esi", "%edi"
    );
}

static __inline__
void int128_sub ( int128_t *self, const int128_t *i )
{
    __asm__ __volatile__
    (
        "push %%ebx;"
        "mov (%%edx), %%eax;"
        "mov 4(%%edx), %%ebx;"
        "mov 8(%%edx), %%esi;"
        "mov 12(%%edx), %%edi;"
        "sub %%eax, (%%ecx);"
        "sbb %%ebx, 4(%%ecx);"
        "sbb %%esi, 8(%%ecx);"
        "sbb %%edi, 12(%%ecx);"
        "pop %%ebx;"
        :
        : "c" ( self ), "d" ( i )
        : "%eax", "%esi", "%edi"
    );
}

static __inline__
void int128_sar ( int128_t *self, uint32_t i )
{
    __asm__ __volatile__
    (
        "mov 4(%%edx), %%eax;"
        "shrd %%cl, %%eax, (%%edx);"
        "mov 8(%%edx), %%eax;"
        "shrd %%cl, %%eax, 4(%%edx);"
        "mov 12(%%edx), %%eax;"
        "shrd %%cl, %%eax, 8(%%edx);"
        "sar %%cl, %%eax;"
        "mov %%eax, 12(%%edx);"
        :
        : "d" ( self ), "c" ( i )
        :  "%eax"
    );
}

static __inline__
void int128_shl ( int128_t *self, uint32_t i )
{
    __asm__ __volatile__
    (
        "mov 8(%%edx), %%eax;"
        "shld %%cl, %%eax, 12(%%edx);"
        "mov 4(%%edx), %%eax;"
        "shld %%cl, %%eax, 8(%%edx);"
        "mov (%%edx), %%eax;"
        "shld %%cl, %%eax, 4(%%edx);"
        "shl %%cl, %%eax;"
        "mov %%eax, (%%edx);"
        :
        : "d" ( self ), "c" ( i )
        :  "%eax"
    );
}

static __inline__
void uint128_and ( uint128_t *self, const uint128_t *i )
{
    __asm__ __volatile__
    (
        "push %%ebx;"
        "mov (%%edx), %%eax;"
        "mov 4(%%edx), %%ebx;"
        "mov 8(%%edx), %%esi;"
        "mov 12(%%edx), %%edi;"
        "and %%eax, (%%ecx);"
        "and %%ebx, 4(%%ecx);"
        "and %%esi, 8(%%ecx);"
        "and %%edi, 12(%%ecx);"
        "pop %%ebx;"
        :
        : "c" ( self ), "d" ( i )
        : "%eax", "%esi", "%edi"
    );
}

static __inline__
void uint128_or ( uint128_t *self, const uint128_t *i )
{
    __asm__ __volatile__
    (
        "push %%ebx;"
        "mov (%%edx), %%eax;"
        "mov 4(%%edx), %%ebx;"
        "mov 8(%%edx), %%esi;"
        "mov 12(%%edx), %%edi;"
        "or %%eax, (%%ecx);"
        "or %%ebx, 4(%%ecx);"
        "or %%esi, 8(%%ecx);"
        "or %%edi, 12(%%ecx);"
        "pop %%ebx;"
        :
        : "c" ( self ), "d" ( i )
        : "%eax", "%esi", "%edi"
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
        "push %%ebx;"
        "mov (%%edx), %%eax;"
        "mov 4(%%edx), %%ebx;"
        "mov 8(%%edx), %%esi;"
        "mov 12(%%edx), %%edi;"
        "xor %%eax, (%%ecx);"
        "xor %%ebx, 4(%%ecx);"
        "xor %%esi, 8(%%ecx);"
        "xor %%edi, 12(%%ecx);"
        "pop %%ebx;"
        :
        : "c" ( self ), "d" ( i )
        : "%eax", "%esi", "%edi"
    );
}

static __inline__
void uint128_not ( uint128_t *self )
{
    __asm__ __volatile__
    (
        "notl (%%ecx);"
        "notl 4(%%ecx);"
        "notl 8(%%ecx);"
        "notl 12(%%ecx);"
        :
        : "c" ( self )
    );
}

static __inline__
void uint128_shr ( uint128_t *self, uint32_t i )
{
    __asm__ __volatile__
    (
        "mov 4(%%edx), %%eax;"
        "shrd %%cl, %%eax, (%%edx);"
        "mov 8(%%edx), %%eax;"
        "shrd %%cl, %%eax, 4(%%edx);"
        "mov 12(%%edx), %%eax;"
        "shrd %%cl, %%eax, 8(%%edx);"
        "shr %%cl, %%eax;"
        "mov %%eax, 12(%%edx);"
        :
        : "d" ( self ), "c" ( i )
        :  "%eax"
    );
}

static __inline__
void uint128_shl ( uint128_t *self, uint32_t i )
{
    __asm__ __volatile__
    (
        "mov 8(%%edx), %%eax;"
        "shld %%cl, %%eax, 12(%%edx);"
        "mov 4(%%edx), %%eax;"
        "shld %%cl, %%eax, 8(%%edx);"
        "mov (%%edx), %%eax;"
        "shld %%cl, %%eax, 4(%%edx);"
        "shl %%cl, %%eax;"
        "mov %%eax, (%%edx);"
        :
        : "d" ( self ), "c" ( i )
        :  "%eax"
    );
}

static __inline__
void uint128_bswap ( uint128_t *self )
{
    __asm__ __volatile__
    (
        "mov (%%ecx), %%eax;"
        "mov 12(%%ecx), %%edx;"
        "bswap %%eax;"
        "bswap %%edx;"
        "mov %%eax, 12(%%ecx);"
        "mov %%edx, (%%ecx);"
        "mov 4(%%ecx), %%eax;"
        "mov 8(%%ecx), %%edx;"
        "bswap %%eax;"
        "bswap %%edx;"
        "mov %%eax, 8(%%ecx);"
        "mov %%edx, 4(%%ecx);"
        :
        : "c" ( self )
        :  "%eax", "%edx"
    );
}

static __inline__
void uint128_bswap_copy ( uint128_t *to, const uint128_t *from )
{
    __asm__ __volatile__
    (
        "push %%ebx;"
        "mov (%%edx), %%eax;"
        "mov 12(%%edx), %%ebx;"
        "bswap %%eax;"
        "bswap %%ebx;"
        "mov %%eax, 12(%%ecx);"
        "mov %%ebx, (%%ecx);"
        "mov 4(%%edx), %%eax;"
        "mov 8(%%edx), %%ebx;"
        "bswap %%eax;"
        "bswap %%ebx;"
        "mov %%eax, 8(%%ecx);"
        "mov %%ebx, 4(%%ecx);"
        "pop %%ebx;"
        :
        : "c" ( to ), "d" ( from )
        :  "%eax"
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
    rtn = ( val << bits ) | ( val >> ( 64 - bits ) );
    return rtn;
}

static __inline__
uint64_t uint64_ror ( uint64_t val, uint8_t bits )
{
    uint64_t rtn;
    rtn = ( val >> bits ) | ( val << ( 64 - bits ) );
    return rtn;
}


#ifdef __cplusplus
}
#endif

#endif /* _h_arch_impl_ */
