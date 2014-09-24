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

#ifdef __cplusplus
extern "C" {
#endif

static __inline__
int uint8_lsbit ( uint8_t self )
{
    int i;
    for ( i = 0; ; ++ i )
    {
        if ( ( self & ( 1 << i ) ) != 0 )
            return i;
    }
    return -1;
}

static __inline__
int16_t uint16_lsbit ( uint16_t self )
{
    if ( self == 0 )
        return -1;
    if ( ( uint8_t ) self != 0 )
        return uint8_lsbit ( ( uint8_t ) self );
    return uint8_lsbit ( ( uint8_t ) ( self >> 8 ) ) + 8;
}

static __inline__
int32_t uint32_lsbit ( uint32_t self )
{
    if ( self == 0 )
        return -1;
    if ( ( uint16_t ) self != 0 )
        return uint16_lsbit ( ( uint16_t ) self );
    return uint16_lsbit ( ( uint16_t ) ( self >> 16 ) ) + 16;
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
    uint64_t lo = ( uint32_t ) self -> lo + ( uint32_t ) i -> lo;
    uint64_t hi = ( self -> lo >> 32 ) + ( i -> lo >> 32 ) + lo >> 32;
    self -> hi += i -> hi + ( uint32_t ) ( hi >> 32 );
    self -> lo = ( hi << 32 ) | ( uint32_t ) lo;
}

static __inline__
void int128_sub ( int128_t *self, const int128_t *i )
{
    int64_t lo = ( uint32_t ) self -> lo - ( uint32_t ) i -> lo;
    int64_t hi = ( uint32_t ) ( self -> lo >> 32 ) - ( uint32_t ) ( i -> lo >> 32 ) + lo >> 32;
    self -> hi -= i -> hi + ( hi >> 32 );
    self -> lo = ( hi << 32 ) | ( uint32_t ) lo;
}

static __inline__
void int128_sar ( int128_t *self, uint32_t i )
{
}

static __inline__
void int128_shl ( int128_t *self, uint32_t i )
{
}

static __inline__
void uint128_and ( uint128_t *self, const uint128_t *i )
{
}

static __inline__
void uint128_or ( uint128_t *self, const uint128_t *i )
{
}

static __inline__
void uint128_orlo ( uint128_t *self, uint64_t i )
{
    self -> lo |= i;
}

static __inline__
void uint128_xor ( uint128_t *self, const uint128_t *i )
{
}

static __inline__
void uint128_not ( uint128_t *self )
{
}

static __inline__
void uint128_shr ( uint128_t *self, uint32_t i )
{
}

static __inline__
void uint128_shl ( uint128_t *self, uint32_t i )
{
}

static __inline__
void uint128_bswap ( uint128_t *self )
{
}

static __inline__
void uint128_bswap_copy ( uint128_t *to, const uint128_t *from )
{
}


#ifdef __cplusplus
}
#endif

#endif /* _h_arch_impl_ */
