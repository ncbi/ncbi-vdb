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

#ifndef _h_noarch_bitstr_
#define _h_noarch_bitstr_

#ifndef _h_bitstr_
#error "don't include <noarch/bitstr.h> directly - use <bitstr.h>"
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <byteswap.h>

#ifdef __cplusplus
extern "C" {
#endif

/* bitcpy
 *  copy a string of bits from source to dest
 *
 *  both source and dest may have non-byte aligned pointers
 *  the number of bits to copy need not be byte aligned
 *
 *  depending upon architecture and OS conventions, the word
 *  size may be adjusted to 1, 2, or 4 bytes, where the base
 *  pointers are always word aligned.
 *
 *  bits in memory are always treated as big-endian, meaning
 *  that on multi-byte fetches and stores, we perform byte-swapping
 *  if there are shifts or masks
 */
static __inline__
void bitcpy ( void *dbase, bitsz_t doff, const void *sbase, bitsz_t soff, bitsz_t sz )
{
    /* noop if sz == 0 */
    if ( sz != 0 )
    {
        /* loop counter and destination word count */
        size_t i, dcountz;

        /* left & right masks and working register */
        WRD lmask, rmask, reg;

        /* produce word-aligned pointers */
#if WRDSIZE == 8
        /* 1-4. all at once */
        WRD *dst = ( WRD* ) dbase + ( doff >> WRDSHIFT );
        const WRD *src = ( const WRD* ) sbase + ( soff >> WRDSHIFT );
#else
        /* 1. capture word alignment adjustment */
        size_t dadjust = ( size_t ) dbase & ( WRDSIZE / 8 - 1 );
        size_t sadjust = ( size_t ) sbase & ( WRDSIZE / 8 - 1 );

        /* 2. create word-aligned pointers */
        WRD *dst = ( WRD* ) ( ( size_t ) dbase - dadjust );
        const WRD *src = ( const WRD* ) ( ( size_t ) sbase - sadjust );

        /* 3. incorporate alignment adjustment into offset bits */
        doff += dadjust << 3;
        soff += sadjust << 3;

        /* 4. readjust pointers based upon offset */
        dst += doff >> WRDSHIFT;
        src += soff >> WRDSHIFT;
#endif
        /* 5. restate offsets */
        doff &= ( WRDSIZE - 1 );
        soff &= ( WRDSIZE - 1 );

        /* calculate number of words - 1 in dst */
        dcountz = ( doff + sz + ( WRDSIZE - 1 ) - WRDSIZE ) >> WRDSHIFT;

        /* calculate masks */
        lmask = rmask = ~ 0;
        lmask >>= doff;
        rmask >>= ( doff + sz ) & ( WRDSIZE - 1 );
        if ( ( WRD ) ( rmask + 1 ) == 0 )
            rmask = 0;

        /* prime register with masked dst [ 0 ] */
        reg = BSWAP ( dst [ 0 ] ) & ~ lmask;

        /* if source and destination are aligned */
        if ( doff == soff )
        {
            /* merge src [ 0 ] into reg through mask */
            reg |= BSWAP ( src [ 0 ] ) & lmask;

#if WRDSIZE > 8
            /* straight copies don't need byteswap                                                                                                                                                           
               other than on first and last words                                                                                                                                                            
               put first word back into little-endian                                                                                                                                                        
               for remainder of loop */
            if ( dcountz > 0 )
            {
                reg = BSWAP ( reg );
#endif
                /* aligned buffers have n:n word ratio */
                for ( i = 0; i < dcountz; )
                {
                    dst [ i ] = reg;
                    reg = src [ ++ i ];
                }

#if WRDSIZE > 8
                /* revert to big-endian */
                reg = BSWAP ( reg );
            }
#endif
        }

        /* shifting alignment  */
        else
        {
            /* source count may differ from dest count */
            size_t scountz = ( soff + sz + ( WRDSIZE - 1 ) - WRDSIZE ) >> WRDSHIFT;

            /* use double-word accumulator */
            ACC acc = BSWAP ( src [ 0 ] );

            /* shift amount */
            int shift = ( int ) doff - ( int ) soff;
            if ( shift > 0 )
            {
                /* take only valid bits in shifted initial src */
                reg |= ( WRD ) ( acc >> shift ) & lmask;

                /* because "shift" > 0, we know "dcountz" >= "scountz" */
                for ( acc <<= WRDSIZE, i = 0; i < scountz; acc <<= WRDSIZE )
                {
                    dst [ i ] = BSWAP ( reg );
                    ++ i;
                    acc |= BSWAP ( src [ i ] );
                    reg = ( WRD ) ( acc >> shift );
                }

                /* if "dcountz" > "scountz" */
                if ( i < dcountz )
                {
                    dst [ i ] = BSWAP ( reg );
                    reg = ( WRD ) ( acc >> shift );
                }
            }

            else
            {
                /* need single word read-ahead and right-shift */
                shift += WRDSIZE;

                /* because "shift" was < 0, we know "dcountz" <= "scountz" */
                for ( acc <<= WRDSIZE, i = 0; i < dcountz; acc <<= WRDSIZE )
                {
                    acc |= BSWAP ( src [ i + 1 ] );
                    reg |= ( WRD ) ( acc >> shift ) & lmask;
                    dst [ i ++ ] = BSWAP ( reg );
                    lmask = ~ 0;
                    reg = 0;
                }

                /* if "dcountz" < "scountz" */
                if ( i < scountz )
                    acc |= BSWAP ( src [ scountz ] );

                reg |= ( WRD ) ( acc >> shift ) & lmask;
            }
        }

        /* mask off unused bytes from src */
        reg &= ~ rmask;

        /* bring in saved bits from dst */
        reg |= BSWAP ( dst [ dcountz ] ) & rmask;

        /* write out last word */
        dst [ dcountz ] = BSWAP ( reg );
    }
}

/* bitcmp
 *  performs bitwise a - b, returning result as int
 *  result value has no meaning, only sign
 *  where < 0 means a < b, > 0 means a > b, and 0 means a == b
 *
 *  since the comparison produces a tri-state indicator of
 *  relative magnitude, the order of "a" and "b" is important.
 *  furthermore, the difference operator must be evaluated
 *  left to right, because the result indicates more than
 *  equality.
 *
 *  see bitcpy for general word alignment information
 */
static __inline__
int bitcmp ( const void *abase, bitsz_t aoff, const void *bbase, bitsz_t boff, bitsz_t sz )
{
    int diff = 0;

    if ( sz != 0 )
    {
        /* loop counter and left word count */
        size_t i, lcountz;

        /* left & right masks and working registers */
        WRD lmask, rmask, lreg, rreg;

        /* produce word-aligned pointers */
#if WRDSIZE == 8
        /* 1-4. all at once */
        const WRD *left = ( const WRD* ) abase + ( aoff >> WRDSHIFT );
        const WRD *right = ( const WRD* ) bbase + ( boff >> WRDSHIFT );
#else
        /* 1. capture word alignment adjustment */
        size_t aadjust = ( size_t ) abase & ( WRDSIZE / 8 - 1 );
        size_t badjust = ( size_t ) bbase & ( WRDSIZE / 8 - 1 );

        /* 2. create word-aligned pointers */
        const WRD *left = ( const WRD* ) ( ( size_t ) abase - aadjust );
        const WRD *right = ( const WRD* ) ( ( size_t ) bbase - badjust );

        /* 3. incorporate alignment adjustment into offset bits */
        aoff += aadjust << 3;
        boff += badjust << 3;

        /* 4. readjust pointers based upon offset */
        left += aoff >> WRDSHIFT;
        right += boff >> WRDSHIFT;
#endif
        /* 5. restate offsets */
        aoff &= ( WRDSIZE - 1 );
        boff &= ( WRDSIZE - 1 );

        /* calculate number of words - 1 in left
           since we know a-priori that "sz" > 0, we
           know that the left and right counts must be
           at least 1. our loops treat the last word
           specially, so calculate a loop counter that
           excludes the last word */
        lcountz = ( aoff + sz + ( WRDSIZE - 1 ) - WRDSIZE ) >> WRDSHIFT;

        /* calculate masks */
        lmask = rmask = ~ 0;
        lmask >>= aoff;
        rmask >>= ( aoff + sz ) & ( WRDSIZE - 1 );
        if ( ( WRD ) ( rmask + 1 ) == 0 )
            rmask = 0;

        /* significant bits from left [ 0 ] */
        lreg = BSWAP ( left [ 0 ] ) & lmask;

        /* if source and destination are aligned */
        if ( aoff == boff )
        {
            /* test against right bits through mask */
            rreg = BSWAP ( right [ 0 ] ) & lmask;

            /* produce a difference of all but the last
               aligned word, where initial word has been
               left-masked. the last word is tested below. */
            for ( i = 1; i <= lcountz; ++ i )
            {
                diff = ( int ) lreg - ( int ) rreg;
                if ( diff != 0 )
                    return diff;

                /* byte-swapping occurs on little-endian architectures */
                lreg = BSWAP ( left [ i ] );
                rreg = BSWAP ( right [ i ] );
            }

            /* fall out to end for masked comparison of last word */
        }

        /* shifting alignment */
        else
        {
            /* right count may differ from left count
               since alignments differ, the span of "sz"
               bits may hit a different number of words in
               the left array than in the right. */
            size_t rcountz = ( boff + sz + ( WRDSIZE - 1 ) - WRDSIZE ) >> WRDSHIFT;

            /* use double-word accumulator
               note that the extra bits get ignored */
            ACC acc = BSWAP ( right [ 0 ] );

            /* shift amount: positive if "b" needs to be right shifted.
               NOTE - since the comparison must be successively performed
               from left to right ( see above ), shifting is ALWAYS toward
               right, making for special handling when "shift" < 0 ( see below ) */
            int shift = ( int ) aoff - ( int ) boff;
            if ( shift > 0 )
            {
                /* initial word from right operand, aligned with left */
                rreg = ( WRD ) ( acc >> shift ) & lmask;

                /* "shift" > 0 means "lcountz" >= "rcountz" */
                for ( acc <<= WRDSIZE, i = 1; i <= rcountz; acc <<= WRDSIZE, ++ i )
                {
                    /* compare words at i-1 */
                    diff = ( int ) lreg - ( int ) rreg;
                    if ( diff != 0 )
                        return diff;

                    /* accumulate next word from right operand */
                    acc |= BSWAP ( right [ i ] );

                    /* bring in next word from left operand */
                    lreg = BSWAP ( left [ i ] );

                    /* produce aligned word from right operand */
                    rreg = ( WRD ) ( acc >> shift );
                }

                /* if there is one more word in left */
                if ( lcountz > rcountz )
                {
                    /* compare penultimate */
                    diff = ( int ) lreg - ( int ) rreg;
                    if ( diff != 0 )
                        return diff;

                    /* get last word in left */
                    lreg = BSWAP ( left [ lcountz ] );

                    /* last word from right is already in "acc" */
                    rreg = ( WRD ) ( acc >> shift );
                }

                /* fall out to end for masked comparison of last word */
            }

            else
            {
                /* since all shifts must be toward right ( due to left to right
                   comparison ), this alignment will require a pre-fetch from
                   right operand into accumulator, and adjusting the negative
                   shift amount to a positive right-shift. */
                shift += WRDSIZE;

                /* since "shift" was negative, we know "lcountz" <= "rcountz",
                   so use "lcountz" as loop limit. pre-shift "acc" as loop init */
                for ( acc <<= WRDSIZE, i = 1; i <= lcountz; acc <<= WRDSIZE, ++ i )
                {
                    /* accumulate next word from right operand */
                    acc |= BSWAP ( right [ i ] );

                    /* produce aligned word from right operand */
                    rreg = ( WRD ) ( acc >> shift ) & lmask;

                    /* now test against left */
                    diff = ( int ) lreg - ( int ) rreg;
                    if ( diff != 0 )
                        return diff;

                    /* bring in next word from left operand */
                    lreg = BSWAP ( left [ i ] );

                    /* no more left mask */
                    lmask = ~ 0;
                }

                /* if there is one more word in right */
                if ( lcountz < rcountz )
                    acc |= BSWAP ( right [ rcountz ] );

                /* produce "rreg" from "acc" */
                rreg = ( WRD ) ( acc >> shift ) & lmask;

                /* fall out to end for masked comparison of last word */
            }
        }

        /* mask off unused bytes from right */
        lreg &= ~ rmask;
        rreg &= ~ rmask;

        /* perform final comparison */
        diff = ( int ) lreg - ( int ) rreg;
    }

    return diff;
}

#ifdef __cplusplus
}
#endif

#endif /* _h_noarch_bitstr_ */
