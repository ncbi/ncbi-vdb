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
#include <vdb/extern.h>
#include <vdb/xform.h>
#include <arch-impl.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>
#include <sysalloc.h>
#include <time.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#if 1
static struct {
	uint8_t num_bits;
	uint8_t val[4];
} map[16] = {

#if WE_SECRETLY_MEANT_4NA_BY_2NA
#define A 1
#define C 2
#define G 4
#define T 8
#else
#define A 0
#define C 1
#define G 2
#define T 3
#endif
/*0000*/{4,{A,C,G,T}}, 
/*0001*/{1,{A}},
/*0010*/{1,{C}},
/*0011*/{2,{A,C}},
/*0100*/{1,{G}},
/*0101*/{2,{A,G}},
/*0110*/{2,{C,G}},
/*0111*/{3,{A,C,G}},
/*1000*/{1,{T}},
/*1001*/{2,{A,T}},
/*1010*/{2,{C,T}},
/*1011*/{3,{A,C,T}},
/*1100*/{2,{G,T}},
/*1101*/{3,{A,G,T}},
/*1110*/{3,{C,G,T}},
/*1111*/{4,{A,C,G,T}}
#undef A
#undef C
#undef G
#undef T
};

static uint8_t randomize_ambiguity(uint8_t base, unsigned *seed, bool *reseed) {
    assert(base < 16);
    
    if (map[base].num_bits == 1) {
        base = map[base].val[0];
        *seed = (*seed << 2) | base;
        *reseed = true;
        return base;
    }
    else {        
        unsigned int rval;
        unsigned int num_bits = map [ base ] . num_bits;
        
        if (*reseed)
        {
            *reseed = false;
            srand(*seed);
        }
        rval = ( unsigned int ) ( ( ( uint64_t ) rand () * num_bits ) / RAND_MAX );

        if ( rval >= num_bits)
            return map[base].val[rval % num_bits];

        return map[base].val[rval];
    }
}
#else
typedef uint8_t dna4na_t;

static const bool isExact[16]
  = { 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 };

static const struct { dna4na_t val[12]; } possibleBase[16] = {
    {{ 1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8 }}, /* 0000 1 2 4 8 */
    {{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }}, /* 0001 1 */
    {{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 }}, /* 0010 2 */
    {{ 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2 }}, /* 0011 1 2 */
    {{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 }}, /* 0100 4 */
    {{ 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4 }}, /* 0101 1 4 */
    {{ 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4 }}, /* 0110 2 4 */
    {{ 1, 2, 4, 1, 2, 4, 1, 2, 4, 1, 2, 4 }}, /* 0111 1 2 4 */
    {{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 }}, /* 1000 8 */
    {{ 1, 8, 1, 8, 1, 8, 1, 8, 1, 8, 1, 8 }}, /* 1001 1 8 */
    {{ 2, 8, 2, 8, 2, 8, 2, 8, 2, 8, 2, 8 }}, /* 1010 2 8 */
    {{ 1, 2, 8, 1, 2, 8, 1, 2, 8, 1, 2, 8 }}, /* 1011 1 2 8 */
    {{ 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8 }}, /* 1100 4 8 */
    {{ 1, 4, 8, 1, 4, 8, 1, 4, 8, 1, 4, 8 }}, /* 1101 1 4 8 */
    {{ 2, 4, 8, 2, 4, 8, 2, 4, 8, 2, 4, 8 }}, /* 1110 2 4 8 */
    {{ 1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8 }}  /* 1111 1 2 4 8 */
};

static uint8_t randomize_ambiguity(uint8_t base, unsigned *seed, bool *reseed) {
    assert(base < 16);
    
    if (isExact[base]) {
        typedef uint8_t dna2na_t;
        
        static const dna2na_t map4na2na[16]
        = { 0, 0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0 };
        
        *seed = (*seed << 2) | map4na2na[base];
        *reseed = true;
        return base;
    }
    else {
        unsigned rval;
        
        if (*reseed) {
            *reseed = false;
            srand(*seed);
        }
        rval = ((uint8_t)(rand() / (RAND_MAX >> 8))) % 12;
        assert(rval < 12);
        return possibleBase[base].val[rval];
    }
}
#endif


static
rc_t CC rand_4na_2na_impl  ( void *data,
    const VXformInfo *info, int64_t row_id, VRowResult *rslt,
    uint32_t argc, const VRowData argv [] )
{
    unsigned seed = time( NULL ) + row_id;
    bool reseed = true;
    uint8_t *dst;
    const uint8_t *src = argv [ 0 ] . u . data . base;
    rc_t	rc;
    KDataBuffer *dst_buf = rslt -> data;
    
    src += argv[0].u.data.first_elem;
    rslt->elem_count = argv[0].u.data.elem_count;
    assert(dst_buf -> elem_bits == 8 );
#if 0
    rslt->no_cache = true; /** do not cache randomized data **/
#endif
    rc = KDataBufferResize ( dst_buf, rslt->elem_count );
    if( rc == 0 ){
	    int i,last;
	    dst = dst_buf -> base;

	    for ( last = rslt -> elem_count; last >  0 && src[last-1] == 15; last --){} /** find run of Ns from the right **/
	    if  ( rslt -> elem_count - last < 5) last = rslt -> elem_count; /*** discard not so large run of Ns ***/
	    
	    for ( i = 0; i < last ; ++ i )
		dst[i] = randomize_ambiguity(src[i], &seed, &reseed);
	    for ( ; i < rslt -> elem_count; ++ i ) /** large run of Ns from the right is set to As, not randomized  **/
		dst[i] = 0;
    }
    return rc;
}


VTRANSFACT_IMPL ( INSDC_SEQ_rand_4na_2na, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> self = NULL;
    rslt -> whack = NULL;
    rslt -> u . ndf = rand_4na_2na_impl;
    rslt -> variant = vftNonDetRow;

    return 0;
}
