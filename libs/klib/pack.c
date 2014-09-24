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
#include <klib/pack.h>
#include <klib/rc.h>
#include <arch-impl.h>
#include <sysalloc.h>

#include <endian.h>
#include <byteswap.h>
#include <string.h>
#include <assert.h>

#if ! defined __LITTLE_ENDIAN && ! defined __BIG_ENDIAN
#error "only little and big endian architectures are supported"
#endif


/*--------------------------------------------------------------------------
 * pack
 */
#define MASK_SRC 0

/* READ_UNPACKED
 *  macro to read source elements into register
 *  casts source pointer to appropriate type
 *  performs byte swap if necessary to ensure that
 *    elements are in reverse order
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define READ_UNPACKED8( src, idx ) \
    ( ( const uint32_t* ) ( src ) ) [ idx ]
#else
#define READ_UNPACKED8( src, idx ) \
    bswap_32 ( ( ( const uint32_t* ) ( src ) ) [ idx ] )
#endif

#define READ_UNPACKED16( src, idx ) \
    ( ( const uint16_t* ) ( src ) ) [ idx ]

#define READ_UNPACKED32( src, idx ) \
    ( ( const uint32_t* ) ( src ) ) [ idx ]

#define READ_UNPACKED64( src, idx ) \
    ( ( const uint64_t* ) ( src ) ) [ idx ]

/* MASK
 *  macro to mask off portion of source register
 */
#if MASK_SRC

#define MASK8( in ) \
    ( in ) & src_mask
#define MASK16( in ) \
    ( in ) & src_mask
#define MASK32( in ) \
    ( in ) & src_mask
#define MASK64( in ) \
    ( in ) & src_mask

#else

#define MASK8( in ) \
    ( uint8_t ) ( in )
#define MASK16( in ) \
    ( uint16_t ) ( in )
#define MASK32( in ) \
    ( uint32_t ) ( in )
#define MASK64( in ) \
    ( uint64_t ) ( in )

#endif


/* WRITE_PACKED
 *  macro to write dest elements from accumulator
 *  casts dest pointer to appropriate type
 *  performs byte swap if necessary to ensure that
 *    elements are in stream order
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define WRITE_PACKED32( acc, dst, idx ) \
    ( ( uint32_t* ) ( dst ) ) [ idx ] = bswap_32 ( ( uint32_t ) ( acc ) )
#define WRITE_PACKED64( acc, dst, idx ) \
    ( ( uint64_t* ) ( dst ) ) [ idx ] = bswap_64 ( ( uint64_t ) ( acc ) )
#else
#define WRITE_PACKED32( acc, dst, idx ) \
    ( ( uint32_t* ) ( dst ) ) [ idx ] = ( uint32_t ) ( acc )
#define WRITE_PACKED64( acc, dst, idx ) \
    ( ( uint64_t* ) ( dst ) ) [ idx ] = ( uint64_t ) ( acc )
#endif

/* Pack8
 */
static
void Pack8 ( uint32_t packed, void *dst, const void *src, uint32_t count )
{
    /* accumulator */
    uint64_t acc;
    uint32_t abits;

    /* loop indices */
    uint32_t s, d, cnt;

#if MASK_SRC
    uint32_t src_mask = ( 1 << packed ) - 1;
#endif

    for ( acc = 0, abits = s = d = 0, cnt = count >> 2; s < cnt; ++ s )
    {
        /* get 4 bytes in reverse order, i.e.
           bits 0-7 are src[0], bits 8-15 are src[1], etc. */
        uint32_t in = READ_UNPACKED8 ( src, s );

        /* pack the bytes into our accumulator */
        acc <<= packed;
        acc |= MASK8 ( in );

        /* account for activity */
        abits += packed;
        in >>= 8;

        /* detect need to dump accumulator */
        if ( abits >= 32 )
        {
            abits -= 32;
            WRITE_PACKED32 ( acc >> abits, dst, d ++ );
        }

        /* repeat for remaining 3 source bytes */
        acc <<= packed;
        acc |= MASK8 ( in );
        abits += packed;
        in >>= 8;
        if ( abits >= 32 )
        {
            abits -= 32;
            WRITE_PACKED32 ( acc >> abits, dst, d ++ );
        }

        acc <<= packed;
        acc |= MASK8 ( in );
        abits += packed;
        in >>= 8;
        if ( abits >= 32 )
        {
            abits -= 32;
            WRITE_PACKED32 ( acc >> abits, dst, d ++ );
        }

        acc <<= packed;
        acc |= MASK8 ( in );
        abits += packed;
        in >>= 8;
        if ( abits >= 32 )
        {
            abits -= 32;
            WRITE_PACKED32 ( acc >> abits, dst, d ++ );
        }
    }

    /* handle remaining src bytes */
    for ( s <<= 2; s < count; ++ s )
    {
        acc <<= packed;
        acc |= MASK8 ( ( ( const uint8_t* ) src ) [ s ] );
        abits += packed;
        if ( abits >= 32 )
        {
            abits -= 32;
            WRITE_PACKED32 ( acc >> abits, dst, d ++ );
        }
    }

    /* handle remaining accumulator bits */
    if ( abits != 0 )
    {
        uint32_t out = bswap_32 ( ( uint32_t ) ( acc << ( 32 - abits ) ) );
        abits = ( abits + 7 ) >> 3;
        for ( d <<= 2; abits != 0; -- abits, out >>= 8, ++ d )
            ( ( uint8_t* ) dst ) [ d ] = ( uint8_t ) out;
    }
}

/* Pack16
 */
static
void Pack16 ( uint32_t packed, void *dst, const void *src, uint32_t count )
{
    /* accumulator */
    uint64_t acc;
    uint32_t abits;

    /* loop indices */
    uint32_t s, d;

#if MASK_SRC
    uint32_t src_mask = ( 1 << packed ) - 1;
#endif

    for ( acc = 0, abits = d = s = 0; s < count; ++ s )
    {
        /* get 2 bytes in native order */
        uint32_t in = READ_UNPACKED16 ( src, s );

        /* pack the bytes into our accumulator */
        acc <<= packed;
        acc |= MASK16 ( in );

        /* account for activity */
        abits += packed;

        /* detect need to dump accumulator */
        if ( abits >= 32 )
        {
            abits -= 32;
            WRITE_PACKED32 ( acc >> abits, dst, d ++ );
        }
    }

    /* handle remaining accumulator bits */
    if ( abits != 0 )
    {
        uint32_t out = bswap_32 ( ( uint32_t ) ( acc << ( 32 - abits ) ) );
        abits = ( abits + 7 ) >> 3;
        for ( d <<= 2; abits != 0; -- abits, out >>= 8, ++ d )
            ( ( uint8_t* ) dst ) [ d ] = ( uint8_t ) out;
    }
}

/* Pack32
 */
static
void Pack32 ( uint32_t packed, void *dst, const void *src, uint32_t count )
{
    /* accumulator */
    uint64_t acc;
    uint32_t abits;

    /* loop indices */
    uint32_t s, d;

#if MASK_SRC
    uint32_t src_mask = ( 1 << packed ) - 1;
#endif

    for ( acc = 0, abits = d = s = 0; s < count; ++ s )
    {
        /* get 4 bytes in native order */
        uint32_t in = READ_UNPACKED32 ( src, s );

        /* pack the bytes into our accumulator */
        acc <<= packed;
        acc |= MASK32 ( in );

        /* account for activity */
        abits += packed;

        /* detect need to dump accumulator */
        if ( abits >= 32 )
        {
            abits -= 32;
            WRITE_PACKED32 ( acc >> abits, dst, d ++ );
        }
    }

    /* handle remaining accumulator bits */
    if ( abits != 0 )
    {
        uint32_t out = bswap_32 ( ( uint32_t ) ( acc << ( 32 - abits ) ) );
        abits = ( abits + 7 ) >> 3;
        for ( d <<= 2; abits != 0; -- abits, out >>= 8, ++ d )
            ( ( uint8_t* ) dst ) [ d ] = ( uint8_t ) out;
    }
}

/* Pack64a
 *  source is 64 bits, packed <= 32 bits
 */
static
void Pack64a ( uint32_t packed, void *dst, const void *src, uint32_t count )
{
    /* accumulator */
    uint64_t acc;
    uint32_t abits;

    /* loop indices */
    uint32_t s, d;

#if MASK_SRC
    uint64_t src_mask = ( 1 << packed ) - 1;
#endif

    for ( acc = 0, abits = d = s = 0; s < count; ++ s )
    {
        /* get 8 bytes in native order */
        uint64_t in = READ_UNPACKED64 ( src, s );

        /* pack the bytes into our accumulator */
        acc <<= packed;
        acc |= MASK32 ( in );

        /* account for activity */
        abits += packed;

        /* detect need to dump accumulator */
        if ( abits >= 32 )
        {
            abits -= 32;
            WRITE_PACKED32 ( acc >> abits, dst, d ++ );
        }
    }

    /* handle remaining accumulator bits */
    if ( abits != 0 )
    {
        uint32_t out = bswap_32 ( ( uint32_t ) ( acc << ( 32 - abits ) ) );
        abits = ( abits + 7 ) >> 3;
        for ( d <<= 2; abits != 0; -- abits, out >>= 8, ++ d )
            ( ( uint8_t* ) dst ) [ d ] = ( uint8_t ) out;
    }
}

/* Pack64b
 *  source is 64 bits, packed > 32 bits
 */
static
void Pack64b ( uint32_t packed, void *dst, const void *src, uint32_t count )
{
    /* accumulator */
    uint128_t acc;
    uint32_t abits;

    /* loop indices */
    uint32_t s, d;

#if MASK_SRC
    uint64_t src_mask = ( ( uint64_t ) 1U << packed ) - 1;
#endif

    for ( abits = d = s = 0; s < count; ++ s )
    {
        /* get 8 bytes in native order */
        uint64_t in = READ_UNPACKED64 ( src, s );

        /* pack the bytes into our accumulator */
        uint128_shl ( & acc, packed );
        uint128_orlo ( & acc, MASK64 ( in ) );

        /* account for activity */
        abits += packed;

        /* detect need to dump accumulator */
        if ( abits >= 64 )
        {
            uint64_t save = uint128_lo ( & acc );
            abits -= 64;
            uint128_shr ( & acc, abits );
            WRITE_PACKED64 ( uint128_lo ( & acc ), dst, d ++ );
            uint128_setlo ( & acc, save );
        }
    }

    /* handle remaining accumulator bits */
    if ( abits != 0 )
    {
        uint64_t out;
        uint128_shl ( & acc, 64 - abits );
        out = bswap_64 ( uint128_lo ( & acc ) );
        abits = ( abits + 7 ) & ~ 7;
        for ( d <<= 3, out >>= 64 - abits; abits != 0; abits -= 8, out >>= 8, ++ d )
            ( ( uint8_t* ) dst ) [ d ] = ( uint8_t ) out;
    }
}

/* Pack
 *  accepts a series of unpacked source bits
 *  produces a series of packed destination bits by eliminating MSB
 *
 *  "unpacked" [ IN ] - original element size in bits
 *  must be in 2^^[3..6], i.e. { 8, 16, 32, 64 }
 *
 *  "packed" [ IN ] - packed element size in bits
 *  must be <= "unpacked"
 *
 *  "src" [ IN ] and "ssize" [ IN ] - source buffer
 *  with size given in bytes
 *
 *  "consumed" [ OUT, NULL OKAY ] - number of source bytes
 *  consumed. if NULL, then all source bytes MUST be consumed
 *  or an error will be generated.
 *
 *  "dst" [ OUT ] and "dst_off" [ IN ] - destination
 *  buffer bit address
 *
 *  "dsize" [ IN ] - size of "dst" in bits, not including "dst_off"
 *
 *  "psize" [ OUT ] - resultant packed size in bits
 *
 * NB - the implementation may allow packing in place
 */
LIB_EXPORT rc_t CC Pack ( uint32_t unpacked, uint32_t packed,
    const void *src, size_t ssize, size_t *consumed,
    void *dst, bitsz_t dst_off, bitsz_t dsize, bitsz_t *psize )
{
    /* prepare for failure */
    if ( consumed != NULL )
        * consumed = 0;

    /* validate output param */
    if ( psize == NULL )
        return RC ( rcXF, rcBuffer, rcPacking, rcParam, rcNull );
    * psize = 0;

    /* require power of 2 with value between 8 and 64 */
    if ( ( ( unpacked - 1 ) & unpacked ) != 0 ||
         unpacked < 8 || unpacked > 64 )
        return RC ( rcXF, rcBuffer, rcPacking, rcParam, rcInvalid );

    /* validate remaining params */
    if ( unpacked < packed )
        return RC ( rcXF, rcBuffer, rcPacking, rcParam, rcInvalid );

    if ( packed == 0 )
        return RC ( rcXF, rcBuffer, rcPacking, rcParam, rcInvalid );

    if ( ssize == 0 )
        return 0;

    if ( src == NULL )
        return RC ( rcXF, rcBuffer, rcPacking, rcParam, rcNull );

    /* detect a byte size so large it can't be converted to bits */
    if ( ( ssize & ( ( size_t ) 7 << ( sizeof ssize * 8 - 3 ) ) ) != 0 )
        return RC ( rcXF, rcBuffer, rcPacking, rcBuffer, rcExcessive );

    /* required destination buffer size */
    * psize = ( ( ssize << 3 ) / unpacked ) * packed;

    if ( dsize < * psize )
    {
        if ( consumed == NULL )
            return RC ( rcXF, rcBuffer, rcPacking, rcBuffer, rcInsufficient );

        ssize = ( dsize / packed ) * ( unpacked >> 3 );
        * psize = ( ( ssize << 3 ) / unpacked ) * packed;
    }

    if ( dst == NULL )
    {
        * psize = 0;
        return RC ( rcXF, rcBuffer, rcPacking, rcParam, rcNull );
    }

    /* all parameters are validated */
    dst = & ( ( char* ) dst ) [ dst_off >> 3 ];
    dst_off &= 7;

    if ( consumed != NULL )
        * consumed = ssize;

    if ( unpacked == 8 && packed == 8 && dst_off == 0 )
    {
        if ( ( const void* ) dst != src )
            memcpy ( dst, src, ssize );
        return 0;
    }

    /* TBD - enable packing into existing buffers */
    if ( dst_off != 0 )
        return RC ( rcXF, rcBuffer, rcPacking, rcOffset, rcUnsupported );

    switch ( unpacked )
    {
    case 8:
        Pack8 ( packed, dst, src, ( uint32_t ) ssize );
        break;
    case 16:
        Pack16 ( packed, dst, src, ( uint32_t ) ( ssize >> 1 ) );
        break;
    case 32:
        Pack32 ( packed, dst, src, ( uint32_t ) ( ssize >> 2 ) );
        break;
    case 64:
        if ( packed > 32 )
            Pack64b ( packed, dst, src, ( uint32_t ) ( ssize >> 3 ) );
        else
            Pack64a ( packed, dst, src, ( uint32_t ) ( ssize >> 3 ) );
        break;
    }

    return 0;
}
