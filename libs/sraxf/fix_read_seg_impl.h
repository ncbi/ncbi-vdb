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

/* READ_SEG was engineered to be capable of holding
   any sort of read description, including out-of-order,
   non-contiguous, gapped or overlapped segments.

   This property was never used, and READ_SEG is by now
   obsolete, so we can count on it being used as a sort
   of READ_LEN where the starts represent an integration
   of the lengths, i.e. all segments are in order and
   contiguous. */
#define READ_SEGS_ALWAYS_CONTIGUOUS 1

static void fix_read_seg_impl(const uint16_t *rs_src, uint32_t nreads,
    uint32_t spot_len, uint32_t * rs_dst)
{
    const uint32_t nreads2 = nreads + nreads;
        uint32_t i, total;

        assert(rs_src && rs_dst);

#if READ_SEGS_ALWAYS_CONTIGUOUS
        /* first, check the stated bases within the read-seg */
        for ( i = 1, total = 0; i < nreads2; i += 2 )
            total += rs_src [ i ];

        /* if there are no bases reported, just divide the
           spot_len by the number of reads and rewrite */
        if ( total == 0 )
        {
            /* if there is a single read, fix it */
            if ( nreads == 1 )
            {
                rs_dst [ 0 ] = 0;
                rs_dst [ 1 ] = spot_len;
            }
            else
            {
                double fstart = 0;
                double flen = ( double ) spot_len / nreads;
                for ( i = total = 0; i < nreads2; fstart += flen, i += 2 )
                {
                    long int start = lround ( fstart );
                    long int end = lround ( fstart + flen );
                    if ( start >= ( long int ) spot_len )
                    {
                        rs_dst [ i ] = spot_len;
                        rs_dst [ i + 1 ] = 0;
                    }
                    else
                    {
                        if ( end > ( long int ) spot_len )
                            end = ( long int ) spot_len - start;
                        else
                            end -= start;

                        rs_dst [ i ] = ( uint32_t ) start;
                        rs_dst [ i + 1 ] = ( uint32_t ) end;
                    }

                    total += rs_dst [ i + 1 ];
                }

                assert ( total == spot_len );
            }
        }

        /* have the right number of bases - just rewrite starts */
        else if ( total == spot_len )
        {
            for ( i = total = 0; i < nreads2; i += 2 )
            {
                rs_dst [ i ] = total;
                total += ( rs_dst [ i + 1 ] = rs_src [ i + 1 ] );
            }
        }

        /* have too many bases - trust all up to limit */
        else if ( total > spot_len )
        {
            for ( i = total = 0; i < nreads2; i += 2 )
            {
                rs_dst [ i ] = total;
                total += ( rs_dst [ i + 1 ] = rs_src [ i + 1 ] );
                if ( total > spot_len )
                {
                    total = spot_len;
                    rs_dst [ i + 1 ] = total - rs_dst [ i ];
                }
            }
        }

        /* too few bases - trust all but last */
        else
        {
            for ( i = 0; i < nreads2; i += 2 )
                rs_dst [ i + 1 ] = rs_src [ i + 1 ];

            for ( ; total < spot_len && i != 0; )
            {
                uint32_t remaining = spot_len - total;
                i -= 2;
                rs_dst [ i + 1 ] += remaining;
                total += remaining;
            }

            for ( i = 0, total = 0; i < nreads2; i += 2 )
            {
                rs_dst [ i ] = total;
                total += rs_dst [ i + 1 ];
            }
        }
#else
        /* loop increments by 2
           where the read start is even, read length odd */
        for ( i = total = 0; i < nreads2; i += 2 )
        {
            /* is start greater than max? */
            if ( rs_src [ i ] >= spot_len )
            {
                /* clip start to end */
                rs_dst [ i ] = spot_len;
                /* zero length read */
                rs_dst [ i + 1 ] = 0;
            }
            /* is end greater than max? */
            else if ( rs_src [ i ] + rs_src [ i + 1 ] > spot_len )
            {
                /* keep start */
                rs_dst [ i ] = rs_src [ i ];
                /* clip length to end */
                rs_dst [ i + 1 ] = spot_len - rs_src [ i ];
            }
            else
            {
                /* copy existing segment */
                rs_dst [ i ]   = rs_src [ i ];
                rs_dst [ i + 1 ] = rs_src [ i + 1 ];
            }

            /* sum total length */
            total += rs_src [ i + 1 ];
        }
#endif
}
