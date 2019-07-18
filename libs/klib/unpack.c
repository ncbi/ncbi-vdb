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
 * unpack
 */

/* READ_PACKED
 *  macro to read source elements into register
 *  casts source pointer to appropriate type
 *  performs byte swap if necessary to ensure that
 *    elements are in reverse order
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define READ_PACKED32( src, idx ) \
    bswap_32 ( ( ( const uint32_t* ) ( src ) ) [ idx ] )
#define READ_PACKED64( src, idx ) \
    bswap_64 ( ( ( const uint64_t* ) ( src ) ) [ idx ] )
#else
#define READ_PACKED32( src, idx ) \
    ( ( const uint32_t* ) ( src ) ) [ idx ]
#define READ_PACKED64( src, idx ) \
    ( ( const uint64_t* ) ( src ) ) [ idx ]
#endif

/* WRITE_UNPACKED
 *  macro to write destination elements from register
 *  casts destination pointer to appropriate type
 *  performs byte swap if necessary to ensure that
 *    elements are in original stream order
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define WRITE_UNPACKED8( out, dst, idx ) \
    ( ( uint64_t* ) ( dst ) ) [ idx ] = ( out )
#else
#define WRITE_UNPACKED8( out, dst, idx ) \
    ( ( uint64_t* ) ( dst ) ) [ idx ] = bswap_64 ( out )
#endif
uint8_t unpack_8_from_2_arr[256][4]={
{0,0,0,0},{0,0,0,1},{0,0,0,2},{0,0,0,3},
{0,0,1,0},{0,0,1,1},{0,0,1,2},{0,0,1,3},
{0,0,2,0},{0,0,2,1},{0,0,2,2},{0,0,2,3},
{0,0,3,0},{0,0,3,1},{0,0,3,2},{0,0,3,3},
{0,1,0,0},{0,1,0,1},{0,1,0,2},{0,1,0,3},
{0,1,1,0},{0,1,1,1},{0,1,1,2},{0,1,1,3},
{0,1,2,0},{0,1,2,1},{0,1,2,2},{0,1,2,3},
{0,1,3,0},{0,1,3,1},{0,1,3,2},{0,1,3,3},
{0,2,0,0},{0,2,0,1},{0,2,0,2},{0,2,0,3},
{0,2,1,0},{0,2,1,1},{0,2,1,2},{0,2,1,3},
{0,2,2,0},{0,2,2,1},{0,2,2,2},{0,2,2,3},
{0,2,3,0},{0,2,3,1},{0,2,3,2},{0,2,3,3},
{0,3,0,0},{0,3,0,1},{0,3,0,2},{0,3,0,3},
{0,3,1,0},{0,3,1,1},{0,3,1,2},{0,3,1,3},
{0,3,2,0},{0,3,2,1},{0,3,2,2},{0,3,2,3},
{0,3,3,0},{0,3,3,1},{0,3,3,2},{0,3,3,3},
{1,0,0,0},{1,0,0,1},{1,0,0,2},{1,0,0,3},
{1,0,1,0},{1,0,1,1},{1,0,1,2},{1,0,1,3},
{1,0,2,0},{1,0,2,1},{1,0,2,2},{1,0,2,3},
{1,0,3,0},{1,0,3,1},{1,0,3,2},{1,0,3,3},
{1,1,0,0},{1,1,0,1},{1,1,0,2},{1,1,0,3},
{1,1,1,0},{1,1,1,1},{1,1,1,2},{1,1,1,3},
{1,1,2,0},{1,1,2,1},{1,1,2,2},{1,1,2,3},
{1,1,3,0},{1,1,3,1},{1,1,3,2},{1,1,3,3},
{1,2,0,0},{1,2,0,1},{1,2,0,2},{1,2,0,3},
{1,2,1,0},{1,2,1,1},{1,2,1,2},{1,2,1,3},
{1,2,2,0},{1,2,2,1},{1,2,2,2},{1,2,2,3},
{1,2,3,0},{1,2,3,1},{1,2,3,2},{1,2,3,3},
{1,3,0,0},{1,3,0,1},{1,3,0,2},{1,3,0,3},
{1,3,1,0},{1,3,1,1},{1,3,1,2},{1,3,1,3},
{1,3,2,0},{1,3,2,1},{1,3,2,2},{1,3,2,3},
{1,3,3,0},{1,3,3,1},{1,3,3,2},{1,3,3,3},
{2,0,0,0},{2,0,0,1},{2,0,0,2},{2,0,0,3},
{2,0,1,0},{2,0,1,1},{2,0,1,2},{2,0,1,3},
{2,0,2,0},{2,0,2,1},{2,0,2,2},{2,0,2,3},
{2,0,3,0},{2,0,3,1},{2,0,3,2},{2,0,3,3},
{2,1,0,0},{2,1,0,1},{2,1,0,2},{2,1,0,3},
{2,1,1,0},{2,1,1,1},{2,1,1,2},{2,1,1,3},
{2,1,2,0},{2,1,2,1},{2,1,2,2},{2,1,2,3},
{2,1,3,0},{2,1,3,1},{2,1,3,2},{2,1,3,3},
{2,2,0,0},{2,2,0,1},{2,2,0,2},{2,2,0,3},
{2,2,1,0},{2,2,1,1},{2,2,1,2},{2,2,1,3},
{2,2,2,0},{2,2,2,1},{2,2,2,2},{2,2,2,3},
{2,2,3,0},{2,2,3,1},{2,2,3,2},{2,2,3,3},
{2,3,0,0},{2,3,0,1},{2,3,0,2},{2,3,0,3},
{2,3,1,0},{2,3,1,1},{2,3,1,2},{2,3,1,3},
{2,3,2,0},{2,3,2,1},{2,3,2,2},{2,3,2,3},
{2,3,3,0},{2,3,3,1},{2,3,3,2},{2,3,3,3},
{3,0,0,0},{3,0,0,1},{3,0,0,2},{3,0,0,3},
{3,0,1,0},{3,0,1,1},{3,0,1,2},{3,0,1,3},
{3,0,2,0},{3,0,2,1},{3,0,2,2},{3,0,2,3},
{3,0,3,0},{3,0,3,1},{3,0,3,2},{3,0,3,3},
{3,1,0,0},{3,1,0,1},{3,1,0,2},{3,1,0,3},
{3,1,1,0},{3,1,1,1},{3,1,1,2},{3,1,1,3},
{3,1,2,0},{3,1,2,1},{3,1,2,2},{3,1,2,3},
{3,1,3,0},{3,1,3,1},{3,1,3,2},{3,1,3,3},
{3,2,0,0},{3,2,0,1},{3,2,0,2},{3,2,0,3},
{3,2,1,0},{3,2,1,1},{3,2,1,2},{3,2,1,3},
{3,2,2,0},{3,2,2,1},{3,2,2,2},{3,2,2,3},
{3,2,3,0},{3,2,3,1},{3,2,3,2},{3,2,3,3},
{3,3,0,0},{3,3,0,1},{3,3,0,2},{3,3,0,3},
{3,3,1,0},{3,3,1,1},{3,3,1,2},{3,3,1,3},
{3,3,2,0},{3,3,2,1},{3,3,2,2},{3,3,2,3},
{3,3,3,0},{3,3,3,1},{3,3,3,2},{3,3,3,3},
};
uint8_t unpack_8_from_1_arr[256][8]={
{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,1},{0,0,0,0,0,0,1,0},{0,0,0,0,0,0,1,1},
{0,0,0,0,0,1,0,0},{0,0,0,0,0,1,0,1},{0,0,0,0,0,1,1,0},{0,0,0,0,0,1,1,1},
{0,0,0,0,1,0,0,0},{0,0,0,0,1,0,0,1},{0,0,0,0,1,0,1,0},{0,0,0,0,1,0,1,1},
{0,0,0,0,1,1,0,0},{0,0,0,0,1,1,0,1},{0,0,0,0,1,1,1,0},{0,0,0,0,1,1,1,1},
{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,1},{0,0,0,1,0,0,1,0},{0,0,0,1,0,0,1,1},
{0,0,0,1,0,1,0,0},{0,0,0,1,0,1,0,1},{0,0,0,1,0,1,1,0},{0,0,0,1,0,1,1,1},
{0,0,0,1,1,0,0,0},{0,0,0,1,1,0,0,1},{0,0,0,1,1,0,1,0},{0,0,0,1,1,0,1,1},
{0,0,0,1,1,1,0,0},{0,0,0,1,1,1,0,1},{0,0,0,1,1,1,1,0},{0,0,0,1,1,1,1,1},
{0,0,1,0,0,0,0,0},{0,0,1,0,0,0,0,1},{0,0,1,0,0,0,1,0},{0,0,1,0,0,0,1,1},
{0,0,1,0,0,1,0,0},{0,0,1,0,0,1,0,1},{0,0,1,0,0,1,1,0},{0,0,1,0,0,1,1,1},
{0,0,1,0,1,0,0,0},{0,0,1,0,1,0,0,1},{0,0,1,0,1,0,1,0},{0,0,1,0,1,0,1,1},
{0,0,1,0,1,1,0,0},{0,0,1,0,1,1,0,1},{0,0,1,0,1,1,1,0},{0,0,1,0,1,1,1,1},
{0,0,1,1,0,0,0,0},{0,0,1,1,0,0,0,1},{0,0,1,1,0,0,1,0},{0,0,1,1,0,0,1,1},
{0,0,1,1,0,1,0,0},{0,0,1,1,0,1,0,1},{0,0,1,1,0,1,1,0},{0,0,1,1,0,1,1,1},
{0,0,1,1,1,0,0,0},{0,0,1,1,1,0,0,1},{0,0,1,1,1,0,1,0},{0,0,1,1,1,0,1,1},
{0,0,1,1,1,1,0,0},{0,0,1,1,1,1,0,1},{0,0,1,1,1,1,1,0},{0,0,1,1,1,1,1,1},
{0,1,0,0,0,0,0,0},{0,1,0,0,0,0,0,1},{0,1,0,0,0,0,1,0},{0,1,0,0,0,0,1,1},
{0,1,0,0,0,1,0,0},{0,1,0,0,0,1,0,1},{0,1,0,0,0,1,1,0},{0,1,0,0,0,1,1,1},
{0,1,0,0,1,0,0,0},{0,1,0,0,1,0,0,1},{0,1,0,0,1,0,1,0},{0,1,0,0,1,0,1,1},
{0,1,0,0,1,1,0,0},{0,1,0,0,1,1,0,1},{0,1,0,0,1,1,1,0},{0,1,0,0,1,1,1,1},
{0,1,0,1,0,0,0,0},{0,1,0,1,0,0,0,1},{0,1,0,1,0,0,1,0},{0,1,0,1,0,0,1,1},
{0,1,0,1,0,1,0,0},{0,1,0,1,0,1,0,1},{0,1,0,1,0,1,1,0},{0,1,0,1,0,1,1,1},
{0,1,0,1,1,0,0,0},{0,1,0,1,1,0,0,1},{0,1,0,1,1,0,1,0},{0,1,0,1,1,0,1,1},
{0,1,0,1,1,1,0,0},{0,1,0,1,1,1,0,1},{0,1,0,1,1,1,1,0},{0,1,0,1,1,1,1,1},
{0,1,1,0,0,0,0,0},{0,1,1,0,0,0,0,1},{0,1,1,0,0,0,1,0},{0,1,1,0,0,0,1,1},
{0,1,1,0,0,1,0,0},{0,1,1,0,0,1,0,1},{0,1,1,0,0,1,1,0},{0,1,1,0,0,1,1,1},
{0,1,1,0,1,0,0,0},{0,1,1,0,1,0,0,1},{0,1,1,0,1,0,1,0},{0,1,1,0,1,0,1,1},
{0,1,1,0,1,1,0,0},{0,1,1,0,1,1,0,1},{0,1,1,0,1,1,1,0},{0,1,1,0,1,1,1,1},
{0,1,1,1,0,0,0,0},{0,1,1,1,0,0,0,1},{0,1,1,1,0,0,1,0},{0,1,1,1,0,0,1,1},
{0,1,1,1,0,1,0,0},{0,1,1,1,0,1,0,1},{0,1,1,1,0,1,1,0},{0,1,1,1,0,1,1,1},
{0,1,1,1,1,0,0,0},{0,1,1,1,1,0,0,1},{0,1,1,1,1,0,1,0},{0,1,1,1,1,0,1,1},
{0,1,1,1,1,1,0,0},{0,1,1,1,1,1,0,1},{0,1,1,1,1,1,1,0},{0,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0},{1,0,0,0,0,0,0,1},{1,0,0,0,0,0,1,0},{1,0,0,0,0,0,1,1},
{1,0,0,0,0,1,0,0},{1,0,0,0,0,1,0,1},{1,0,0,0,0,1,1,0},{1,0,0,0,0,1,1,1},
{1,0,0,0,1,0,0,0},{1,0,0,0,1,0,0,1},{1,0,0,0,1,0,1,0},{1,0,0,0,1,0,1,1},
{1,0,0,0,1,1,0,0},{1,0,0,0,1,1,0,1},{1,0,0,0,1,1,1,0},{1,0,0,0,1,1,1,1},
{1,0,0,1,0,0,0,0},{1,0,0,1,0,0,0,1},{1,0,0,1,0,0,1,0},{1,0,0,1,0,0,1,1},
{1,0,0,1,0,1,0,0},{1,0,0,1,0,1,0,1},{1,0,0,1,0,1,1,0},{1,0,0,1,0,1,1,1},
{1,0,0,1,1,0,0,0},{1,0,0,1,1,0,0,1},{1,0,0,1,1,0,1,0},{1,0,0,1,1,0,1,1},
{1,0,0,1,1,1,0,0},{1,0,0,1,1,1,0,1},{1,0,0,1,1,1,1,0},{1,0,0,1,1,1,1,1},
{1,0,1,0,0,0,0,0},{1,0,1,0,0,0,0,1},{1,0,1,0,0,0,1,0},{1,0,1,0,0,0,1,1},
{1,0,1,0,0,1,0,0},{1,0,1,0,0,1,0,1},{1,0,1,0,0,1,1,0},{1,0,1,0,0,1,1,1},
{1,0,1,0,1,0,0,0},{1,0,1,0,1,0,0,1},{1,0,1,0,1,0,1,0},{1,0,1,0,1,0,1,1},
{1,0,1,0,1,1,0,0},{1,0,1,0,1,1,0,1},{1,0,1,0,1,1,1,0},{1,0,1,0,1,1,1,1},
{1,0,1,1,0,0,0,0},{1,0,1,1,0,0,0,1},{1,0,1,1,0,0,1,0},{1,0,1,1,0,0,1,1},
{1,0,1,1,0,1,0,0},{1,0,1,1,0,1,0,1},{1,0,1,1,0,1,1,0},{1,0,1,1,0,1,1,1},
{1,0,1,1,1,0,0,0},{1,0,1,1,1,0,0,1},{1,0,1,1,1,0,1,0},{1,0,1,1,1,0,1,1},
{1,0,1,1,1,1,0,0},{1,0,1,1,1,1,0,1},{1,0,1,1,1,1,1,0},{1,0,1,1,1,1,1,1},
{1,1,0,0,0,0,0,0},{1,1,0,0,0,0,0,1},{1,1,0,0,0,0,1,0},{1,1,0,0,0,0,1,1},
{1,1,0,0,0,1,0,0},{1,1,0,0,0,1,0,1},{1,1,0,0,0,1,1,0},{1,1,0,0,0,1,1,1},
{1,1,0,0,1,0,0,0},{1,1,0,0,1,0,0,1},{1,1,0,0,1,0,1,0},{1,1,0,0,1,0,1,1},
{1,1,0,0,1,1,0,0},{1,1,0,0,1,1,0,1},{1,1,0,0,1,1,1,0},{1,1,0,0,1,1,1,1},
{1,1,0,1,0,0,0,0},{1,1,0,1,0,0,0,1},{1,1,0,1,0,0,1,0},{1,1,0,1,0,0,1,1},
{1,1,0,1,0,1,0,0},{1,1,0,1,0,1,0,1},{1,1,0,1,0,1,1,0},{1,1,0,1,0,1,1,1},
{1,1,0,1,1,0,0,0},{1,1,0,1,1,0,0,1},{1,1,0,1,1,0,1,0},{1,1,0,1,1,0,1,1},
{1,1,0,1,1,1,0,0},{1,1,0,1,1,1,0,1},{1,1,0,1,1,1,1,0},{1,1,0,1,1,1,1,1},
{1,1,1,0,0,0,0,0},{1,1,1,0,0,0,0,1},{1,1,1,0,0,0,1,0},{1,1,1,0,0,0,1,1},
{1,1,1,0,0,1,0,0},{1,1,1,0,0,1,0,1},{1,1,1,0,0,1,1,0},{1,1,1,0,0,1,1,1},
{1,1,1,0,1,0,0,0},{1,1,1,0,1,0,0,1},{1,1,1,0,1,0,1,0},{1,1,1,0,1,0,1,1},
{1,1,1,0,1,1,0,0},{1,1,1,0,1,1,0,1},{1,1,1,0,1,1,1,0},{1,1,1,0,1,1,1,1},
{1,1,1,1,0,0,0,0},{1,1,1,1,0,0,0,1},{1,1,1,1,0,0,1,0},{1,1,1,1,0,0,1,1},
{1,1,1,1,0,1,0,0},{1,1,1,1,0,1,0,1},{1,1,1,1,0,1,1,0},{1,1,1,1,0,1,1,1},
{1,1,1,1,1,0,0,0},{1,1,1,1,1,0,0,1},{1,1,1,1,1,0,1,0},{1,1,1,1,1,0,1,1},
{1,1,1,1,1,1,0,0},{1,1,1,1,1,1,0,1},{1,1,1,1,1,1,1,0},{1,1,1,1,1,1,1,1},
};

static
void CC Unpack8From2(uint8_t *dst,const uint8_t *src,int32_t count)
{
	if(count > 0){
		int i;
		for(i=0;i<count/4;i++,dst+=4,src++){
			memmove(dst,unpack_8_from_2_arr[*src],4);
		}
		for(i=0;i< (count&3);i++){
			dst[i] = unpack_8_from_2_arr[*src][i];
		}
	}
}
static
void CC Unpack8From1(uint8_t *dst,const uint8_t *src,int32_t count)
{
	if(count > 0){
		int i;
		for(i=0;i<count/8;i++,dst+=8,src++){
			memmove(dst,unpack_8_from_1_arr[*src],8);
		}
		for(i=0;i< (count&7);i++){
			dst[i] = unpack_8_from_1_arr[*src][i];
		}
	}
}




/* Unpack8
 */
static
void CC Unpack8 ( uint32_t packed, uint32_t count, void *dst,
    const void *src, bitsz_t src_off, bitsz_t ssize )
{
    uint64_t acc;
    uint64_t out;
    uint32_t abits;
    uint64_t src_mask;
    /* keep track of trailing bits */
    uint32_t discard = (uint32_t)( ( 0 - ( ssize += src_off ) ) & 7 );


    if(src_off == 0){
	switch(packed){
	 case 1:
		Unpack8From1(dst,src,count);
		return;
	 case 2:
		Unpack8From2(dst,src,count);
		return;
	}
    }
	


    /* convert to bytes */
    ssize = ( ssize + 7 ) >> 3;

    /* first, get any stray source bytes */
    for ( abits = 0, acc = 0; ( ssize & 3 ) != 0; abits += 8 )
    {
        acc <<= 8;
        acc |= ( ( const uint8_t* ) src ) [ -- ssize ];
    }

    /* only reading 4 bytes at a time now */
    ssize >>= 2;

    /* if source size was even multiple of 4 bytes */
    if ( abits == 0 )
    {
        assert ( ssize != 0 );
        acc = READ_PACKED32 ( src, -- ssize );
        abits = 32;
    }
    /* bytes were accumulated in backward order */
    else if ( abits != 8 )
    {
        acc = bswap_32 ( ( uint32_t ) ( acc << ( 32 - abits ) ) );
    }

    /* discard alignment bits */
    if ( discard != 0 )
    {
        acc >>= discard;
        abits -= discard;
    }

    /* create source mask */
    src_mask = ( ( uint64_t ) 1U << packed ) - 1;

    /* write stray output bytes */
    for ( ; ( count & 7 ) != 0; abits -= packed, acc >>= packed )
    {
        /* make sure there are enough source bits */
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }

        /* write directly to destination */
        ( ( uint8_t* ) dst ) [ -- count ] = ( uint8_t ) ( acc & src_mask );
    }

    /* accumulate into unpack register */
    for ( count >>= 3; count != 0; abits -= packed, acc >>= packed )
    {
        /* bring in enough source bits */
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }

        /* get byte in reverse order */
        out = acc & src_mask;

        /* drop packed bits */
        abits -= packed;
        acc >>= packed;

        /* same for remaining output bytes */
        out <<= 8;
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }
        out |= acc & src_mask;
        abits -= packed;
        acc >>= packed;

        out <<= 8;
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }
        out |= acc & src_mask;
        abits -= packed;
        acc >>= packed;

        out <<= 8;
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }
        out |= acc & src_mask;
        abits -= packed;
        acc >>= packed;

        out <<= 8;
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }
        out |= acc & src_mask;
        abits -= packed;
        acc >>= packed;

        out <<= 8;
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }
        out |= acc & src_mask;
        abits -= packed;
        acc >>= packed;

        out <<= 8;
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }
        out |= acc & src_mask;
        abits -= packed;
        acc >>= packed;

        out <<= 8;
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }
        out |= acc & src_mask;

        /* write output */
        WRITE_UNPACKED8 ( out, dst, -- count );
    }

    /* should have written everything */
    assert ( abits == 0 );
    assert ( ssize == 0 );
}


/* Unpack16
 */
static
void CC Unpack16 ( uint32_t packed, uint32_t count, void *dst,
    const void *src, bitsz_t src_off, bitsz_t ssize )
{
    uint64_t acc;
    uint32_t abits;

    uint64_t src_mask;
    uint32_t discard = (uint32_t)( ( 0 - ( ssize += src_off ) ) & 7 );

    /* convert to bytes */
    ssize = ( ssize + 7 ) >> 3;

    /* first, get any stray source bytes */
    for ( abits = 0, acc = 0; ( ssize & 3 ) != 0; abits += 8 )
    {
        acc <<= 8;
        acc |= ( ( const uint8_t* ) src ) [ -- ssize ];
    }

    /* only reading 4 bytes at a time now */
    ssize >>= 2;

    /* if source size was even multiple of 4 bytes */
    if ( abits == 0 )
    {
        assert ( ssize != 0 );
        acc = READ_PACKED32 ( src, -- ssize );
        abits = 32;
    }
    /* bytes were accumulated in backward order */
    else if ( abits != 8 )
    {
        acc = bswap_32 ( ( uint32_t ) ( acc << ( 32 - abits ) ) );
    }


    /* discard alignment bits */
    if ( discard != 0 )
    {
        assert ( discard < 8 );
        acc >>= discard;
        abits -= discard;
    }

    /* create write mask */
    src_mask = ( ( uint64_t ) 1U << packed ) - 1;

    /* write output */
    for ( ; count != 0; abits -= packed, acc >>= packed )
    {
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }

        ( ( uint16_t* ) dst ) [ -- count ] = ( uint16_t ) ( acc & src_mask );
    }

    /* should have written everything */
    assert ( abits == 0 );
    assert ( ssize == 0 );
}


/* Unpack32
 */
static
void CC Unpack32 ( uint32_t packed, uint32_t count, void *dst,
    const void *src, bitsz_t src_off, bitsz_t ssize )
{
    uint64_t acc;
    uint32_t abits;

    uint64_t src_mask;
    uint32_t discard = (uint32_t)( ( 0 - ( ssize += src_off ) ) & 7 );

    /* convert to bytes */
    ssize = ( ssize + 7 ) >> 3;

    /* first, get any stray source bytes */
    for ( abits = 0, acc = 0; ( ssize & 3 ) != 0; abits += 8 )
    {
        acc <<= 8;
        acc |= ( ( const uint8_t* ) src ) [ -- ssize ];
    }

    /* only reading 4 bytes at a time now */
    ssize >>= 2;

    /* if source size was even multiple of 4 bytes */
    if ( abits == 0 )
    {
        assert ( ssize != 0 );
        acc = READ_PACKED32 ( src, -- ssize );
        abits = 32;
    }
    /* bytes were accumulated in backward order */
    else if ( abits != 8 )
    {
        acc = bswap_32 ( ( uint32_t ) ( acc << ( 32 - abits ) ) );
    }

    /* discard alignment bits */
    if ( discard != 0 )
    {
        assert ( discard < 8 );
        acc >>= discard;
        abits -= discard;
    }

    /* create write mask */
    src_mask = ( ( uint64_t ) 1U << packed ) - 1;

    /* write output */
    for ( ; count != 0; abits -= packed, acc >>= packed )
    {
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }

        ( ( uint32_t* ) dst ) [ -- count ] = ( uint32_t ) ( acc & src_mask );
    }

    /* should have written everything */
    assert ( abits == 0 );
    assert ( ssize == 0 );
}


/* Unpack64a
 *  packed size <= 32
 */
static
void CC Unpack64a ( uint32_t packed, uint32_t count, void *dst,
    const void *src, bitsz_t src_off, bitsz_t ssize )
{
    uint64_t acc;
    uint32_t abits;

    uint64_t src_mask;
    uint32_t discard = (uint32_t)( ( 0 - ( ssize += src_off ) ) & 7 );

    /* convert to bytes */
    ssize = ( ssize + 7 ) >> 3;

    /* first, get any stray source bytes */
    for ( abits = 0, acc = 0; ( ssize & 3 ) != 0; abits += 8 )
    {
        acc <<= 8;
        acc |= ( ( const uint8_t* ) src ) [ -- ssize ];
    }

    /* only reading 4 bytes at a time now */
    ssize >>= 2;

    /* if source size was even multiple of 4 bytes */
    if ( abits == 0 )
    {
        assert ( ssize != 0 );
        acc = READ_PACKED32 ( src, -- ssize );
        abits = 32;
    }
    /* bytes were accumulated in backward order */
    else if ( abits != 8 )
    {
        acc = bswap_32 ( ( uint32_t ) ( acc << ( 32 - abits ) ) );
    }

    /* discard alignment bits */
    if ( discard != 0 )
    {
        assert ( discard < 8 );
        acc >>= discard;
        abits -= discard;
    }

    /* create write mask */
    src_mask = ( ( uint64_t ) 1U << packed ) - 1;

    /* write output */
    for ( ; count != 0; abits -= packed, acc >>= packed )
    {
        if ( abits < packed )
        {
            assert ( ssize != 0 );
            acc |= ( uint64_t ) READ_PACKED32 ( src, -- ssize ) << abits;
            abits += 32;
            assert ( abits >= packed );
        }

        ( ( uint64_t* ) dst ) [ -- count ] = acc & src_mask;
    }

    /* should have written everything */
    assert ( abits == 0 );
    assert ( ssize == 0 );
}


/* Unpack64b
 *  packed size > 32
 */
static
void CC Unpack64b ( uint32_t packed, uint32_t count, void *dst,
    const void *src, bitsz_t src_off, bitsz_t ssize )
{
    uint128_t acc;
    uint32_t abits;

    uint64_t src_mask;
    uint32_t discard = (uint32_t)( ( 0 - ( ssize += src_off ) ) & 7 );

    /* convert to bytes */
    ssize = ( ssize + 7 ) >> 3;

    /* probably still don't support native 128 bit */
    uint128_sethi ( & acc, 0 );
    uint128_setlo ( & acc, 0 );

    /* first, get any stray source bytes */
    for ( abits = 0; ( ssize & 7 ) != 0; abits += 8 )
    {
        uint128_shl ( & acc, 8 );
        uint128_orlo ( & acc, ( ( const uint8_t* ) src ) [ -- ssize ] );
    }

    /* only reading 8 bytes at a time now */
    ssize >>= 3;

    /* if source size was even multiple of 4 bytes */
    if ( abits == 0 )
    {
        assert ( ssize != 0 );
        uint128_setlo ( & acc, READ_PACKED64 ( src, -- ssize ) );
        abits = 64;
    }
    /* bytes were accumulated in backward order */
    else if ( abits != 8 )
    {
        uint128_setlo ( & acc, bswap_64 ( uint128_lo ( & acc ) << ( 64 - abits ) ) );
    }

    /* discard alignment bits */
    if ( discard != 0 )
    {
        assert ( discard < 8 );
        uint128_shr ( & acc, discard );
        abits -= discard;
    }

    /* create write mask */
    src_mask = ( ( uint64_t ) 1U << packed ) - 1;

    /* write output */
    for ( ; count != 0; abits -= packed, uint128_shr ( & acc, packed ) )
    {
        if ( abits < packed )
        {
            uint128_t in;
            assert ( ssize != 0 );
            uint128_sethi ( & in, 0 );
            uint128_setlo ( & in, READ_PACKED64 ( src, -- ssize ) );
            uint128_shl ( & in, abits );
            uint128_or ( & acc, & in );
            abits += 64;
            assert ( abits >= packed );
        }

        ( ( uint64_t* ) dst ) [ -- count ] = uint128_lo ( & acc ) & src_mask;
    }

    /* should have written everything */
    assert ( abits == 0 );
    assert ( ssize == 0 );
}


/* Unpack
 *  accepts a series of packed source bits
 *  produces a series of unpacked destination bits by left-padding zeros
 *
 *  "packed" [ IN ] - packed element size in bits
 *
 *  "unpacked [ IN ] - original element size in bits
 *  must be >= "packed" and
 *  must be in 2^^[3..6], i.e. { 8, 16, 32, 64 }
 *
 *  "src" [ IN ] and "src_off" [ IN ] - source buffer
 *  bit address
 *
 *  "ssize" [ IN ] - number of bits in "src", not including "src_off"
 *
 *  "consumed" [ OUT, NULL OKAY ] - number of source bits
 *  consumed. if NULL, then all source bits MUST be consumed
 *  or an error will be generated.
 *
 *  "dst" [ OUT ] and "dsize" [ IN ] - destination buffer
 *  where size is given in bytes
 *
 *  "usize" [ OUT ] - resultant unpacked size in bytes
 *
 * NB - "src" and "dst" may have same address since
 *  the implementation unpacks from right to left, smaller to larger
 */
LIB_EXPORT rc_t CC Unpack ( uint32_t packed, uint32_t unpacked,
    const void *src, bitsz_t src_off, bitsz_t ssize, bitsz_t *consumed,
    void *dst, size_t dsize, size_t *usize )
{
    uint32_t count;

    /* prepare for failure */
    if ( consumed != NULL )
        * consumed = 0;

    /* validate output param */
    if ( usize == NULL )
        return RC ( rcXF, rcBuffer, rcUnpacking, rcParam, rcNull );
    * usize = 0;

    /* require power of 2 with value between 8 and 64 */
    if ( ( ( unpacked - 1 ) & unpacked ) != 0 ||
         unpacked < 8 || unpacked > 64 )
        return RC ( rcXF, rcBuffer, rcUnpacking, rcParam, rcInvalid );

    /* validate remaining params */
    if ( unpacked < packed || packed == 0 )
        return RC ( rcXF, rcBuffer, rcUnpacking, rcParam, rcInvalid );

    if ( ssize == 0 )
        return 0;

    if ( src == NULL )
        return RC ( rcXF, rcBuffer, rcUnpacking, rcParam, rcNull );

    src = & ( ( const char* ) src ) [ src_off >> 3 ];
    src_off &= 7;

    /* required destination buffer size */
    count = ( uint32_t ) ( ssize / packed );
    if ( count == 0 )
        return 0;
    * usize = ( (size_t)count * unpacked ) >> 3;

    if ( dsize < * usize )
    {
        if ( consumed == NULL )
            return RC ( rcXF, rcBuffer, rcUnpacking, rcBuffer, rcInsufficient );

        count = (uint32_t)( dsize / ( unpacked >> 3 ) );
        ssize = (size_t)count * packed;
        * usize = ( (size_t)count * unpacked ) >> 3;
    }

    if ( dst == NULL )
    {
        * usize = 0;
        return RC ( rcXF, rcBuffer, rcUnpacking, rcParam, rcNull );
    }

    /* all parameters are validated */
    if ( unpacked == 8 && packed == 8 && src_off == 0 )
    {
        if ( ( const void* ) dst != src )
            memmove ( dst, src, ssize >> 3 );
        return 0;
    }

    /* TBD - enable unpacking from offsets */
    if ( src_off != 0 )
        return RC ( rcXF, rcBuffer, rcUnpacking, rcOffset, rcUnsupported );

    switch ( unpacked )
    {
    case 8:
        Unpack8 ( packed, count, dst, src, src_off, ssize );
        break;
    case 16:
        Unpack16 ( packed, count, dst, src, src_off, ssize );
        break;
    case 32:
        Unpack32 ( packed, count, dst, src, src_off, ssize );
        break;
    case 64:
        if ( packed > 32 )
            Unpack64b ( packed, count, dst, src, src_off, ssize );
        else
            Unpack64a ( packed, count, dst, src, src_off, ssize );
        break;
    }

    return 0;
}
