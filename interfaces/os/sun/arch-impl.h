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
#include <byteswap.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __inline__
#define __inline__ inline
#endif

static __inline__
int16_t uint16_lsbit ( uint16_t self )
{
    int rtn;
    for ( rtn = 0; rtn < 16; ++ rtn )
    {
        if ( ( self & ( 1 << rtn) ) != 0 )
            return ( int16_t ) rtn;
    }
    return -1;
}

static __inline__
int32_t uint32_lsbit ( uint32_t self )
{
    int rtn;
    for ( rtn = 0; rtn < 32; ++ rtn )
    {
        if ( ( self & ( 1 << rtn) ) != 0 )
            return ( int32_t ) rtn;
    }
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
    int carry = ( ( int64_t ) self -> lo < 0 ) && ( ( int64_t ) i -> lo < 0 );
    self -> hi += i -> hi;
    self -> lo += i -> lo;
    self -> hi += carry;
}

static __inline__
void int128_sub ( int128_t *self, const int128_t *i )
{
    int carry = self -> lo < i -> lo;
    self -> hi -= i -> hi;
    self -> lo -= i -> lo;
    self -> hi -= carry;
}

static __inline__
void int128_sar ( int128_t *self, uint32_t i )
{
    if ( ( i &= 0x7F ) != 0 )
    {
        int64_t shifted = self -> hi;

        if ( i >= 64 )
        {
            self -> hi >>= 63;
            self -> lo = shifted >> ( i - 64 );
        }
        else
        {
            self -> lo >>= i;
            shifted <<= 64 - i;
            self -> hi >>= i;
            self -> lo |= shifted;
        }
    }
}

static __inline__
void int128_shl ( int128_t *self, uint32_t i )
{
    if ( ( i &= 0x7F ) != 0 )
    {
        uint64_t shifted = self -> lo;

        if ( i >= 64 )
        {
            self -> lo = 0;
            self -> hi = ( int64_t ) ( shifted << ( i - 64 ) );
        }
        else
        {
            self -> hi <<= i;
            shifted >>= 64 - i;
            self -> lo <<= i;
            self -> hi |= ( int64_t ) shifted;
        }
    }
}

static __inline__
void uint128_and ( uint128_t *self, const uint128_t *i )
{
    self -> hi &= i -> hi;
    self -> lo &= i -> lo;
}

static __inline__
void uint128_or ( uint128_t *self, const uint128_t *i )
{
    self -> hi |= i -> hi;
    self -> lo |= i -> lo;
}

static __inline__
void uint128_orlo ( uint128_t *self, uint64_t i )
{
    self -> lo |= i;
}

static __inline__
void uint128_xor ( uint128_t *self, const uint128_t *i )
{
    self -> hi ^= i -> hi;
    self -> lo ^= i -> lo;
}

static __inline__
void uint128_not ( uint128_t *self )
{
    self -> hi = ~ self -> hi;
    self -> lo = ~ self -> lo;
}

static __inline__
void uint128_shr ( uint128_t *self, uint32_t i )
{
    if ( ( i &= 0x7F ) != 0 )
    {
        uint64_t shifted = self -> hi;

        if ( i >= 64 )
        {
            self -> hi >>= 63;
            self -> lo = shifted >> ( i - 64 );
        }
        else
        {
            self -> lo >>= i;
            shifted <<= 64 - i;
            self -> hi >>= i;
            self -> lo |= shifted;
        }
    }
}

static __inline__
void uint128_shl ( uint128_t *self, uint32_t i )
{
    if ( ( i &= 0x7F ) != 0 )
    {
        uint64_t shifted = self -> lo;

        if ( i >= 64 )
        {
            self -> lo = 0;
            self -> hi = shifted << ( i - 64 );
        }
        else
        {
            self -> hi <<= i;
            shifted >>= 64 - i;
            self -> lo <<= i;
            self -> hi |= shifted;
        }
    }
}

static __inline__
void uint128_bswap ( uint128_t *self )
{
    uint64_t tmp = bswap_64 ( self -> lo );
    self -> lo = bswap_64 ( self -> hi );
    self -> hi = tmp;
}

static __inline__
void uint128_bswap_copy ( uint128_t *to, const uint128_t *from )
{
    uint64_t tmp = bswap_64 ( from -> lo );
    to -> lo = bswap_64 ( from -> hi );
    to -> hi = tmp;
}

#ifdef __cplusplus
}
#endif

#endif /* _h_arch_impl_ */
