/*

  vdb3.kfc.rgn

 */

#include <vdb3/kfc/rgn.hpp>
#include <vdb3/kfc/ktrace.hpp>
#include <vdb3/kfc/except.hpp>

#ifndef BRUTE_FORCE_SIZE_LIMIT
#define BRUTE_FORCE_SIZE_LIMIT 128
#endif

namespace vdb3
{

    const size_t brute_force_size_limit = BRUTE_FORCE_SIZE_LIMIT;
    
    /*=====================================================*
     *                       UTILITY                       *
     *=====================================================*/

    static
    size_t brute_force_fwd_find ( const byte_t * haystack, size_t hsize,
                                  const byte_t * needle, size_t nsize )
    {
        assert ( nsize <= hsize );
        assert ( nsize != 0 );

#if DEBUG
        size_t find_cost = 0;
#endif
        
        // just mount a dumb scan
        size_t lim = hsize - nsize + 1;
        for ( size_t i = 0; i < lim; ++ i )
        {
            // compare start of needle against start of current alignment
            KTRACE ( TRACE_PRG, "testing alignment ( 0, %zu ) offset 0: 0x%02x = 0x%02x ( '%c' = '%c' )"
                    , i
                    , needle [ 0 ]
                    , haystack [ i ]
                    , ( :: isprint ( needle [ 0 ] ) ? ( char ) needle [ 0 ] : '?' )
                    , ( :: isprint ( haystack [ i ] ) ? ( char ) haystack [ i ] : '?' )
                );
#if DEBUG
            ++ find_cost;
#endif
            if ( needle [ 0 ] == haystack [ i ] )
            {
                // compare entire needle at current alignment
                for ( size_t j = 1; ; ++ j )
                {
                    // end of needle with no mismatch
                    if ( j == nsize )
                    {
                        KTRACE ( TRACE_PRG, "regions align at ( 0, %zu )", i );
#if DEBUG
                        KTRACE ( TRACE_QA, "total cost of comparisons = %zu", find_cost );
#endif
                        return i;
                    }

                    // a mismatch causes realignment
                    KTRACE ( TRACE_PRG, "testing alignment ( 0, %zu ) offset %zu: 0x%02x = 0x%02x ( '%c' = '%c' )"
                            , i
                            , j
                            , needle [ j ]
                            , haystack [ i + j ]
                            , ( :: isprint ( needle [ j ] ) ? ( char ) needle [ j ] : '?' )
                            , ( :: isprint ( haystack [ i + j ] ) ? ( char ) haystack [ i + j ] : '?' )
                        );
#if DEBUG
                    ++ find_cost;
#endif
                    if ( needle [ j ] != haystack [ i + j ] )
                        break;
                }
            }
        }

        // not found
        KTRACE ( TRACE_PRG, "regions do not align" );
#if DEBUG
        KTRACE ( TRACE_QA, "total cost of comparisons = %zu", find_cost );
#endif
        return hsize;
    }

    static
    size_t brute_force_rev_find ( const byte_t * haystack, size_t hsize,
                                  const byte_t * needle, size_t nsize )
    {
        assert ( nsize <= hsize );
        assert ( nsize != 0 );

#if DEBUG
        size_t find_cost = 0;
#endif
        
        // just mount a dumb scan
        for ( size_t i = hsize - nsize + 1; i != 0; )
        {
            -- i;
            
            // compare start of needle against start of current alignment
            // compare start of needle against start of current alignment
            KTRACE ( TRACE_PRG, "testing alignment ( 0, %zu ) offset 0: 0x%02x = 0x%02x ( '%c' = '%c' )"
                    , i
                    , needle [ 0 ]
                    , haystack [ i ]
                    , ( :: isprint ( needle [ 0 ] ) ? ( char ) needle [ 0 ] : '?' )
                    , ( :: isprint ( haystack [ i ] ) ? ( char ) haystack [ i ] : '?' )
                );
#if DEBUG
            ++ find_cost;
#endif
            if ( needle [ 0 ] == haystack [ i ] )
            {
                // compare entire needle at current alignment
                for ( size_t j = 1; ; ++ j )
                {
                    // end of needle with no mismatch
                    if ( j == nsize )
                    {
                        KTRACE ( TRACE_PRG, "regions align at ( 0, %zu )", i );
#if DEBUG
                        KTRACE ( TRACE_QA, "total cost of comparisons = %zu", find_cost );
#endif
                        return i;
                    }

                    // a mismatch causes realignment
                    KTRACE ( TRACE_PRG, "testing alignment ( 0, %zu ) offset %zu: 0x%02x = 0x%02x ( '%c' = '%c' )"
                            , i
                            , j
                            , needle [ j ]
                            , haystack [ i + j ]
                            , ( :: isprint ( needle [ j ] ) ? ( char ) needle [ j ] : '?' )
                            , ( :: isprint ( haystack [ i + j ] ) ? ( char ) haystack [ i + j ] : '?' )
                        );
#if DEBUG
                    ++ find_cost;
#endif
                    if ( needle [ j ] != haystack [ i + j ] )
                        break;
                }
            }
        }

        // not found
        KTRACE ( TRACE_PRG, "regions do not align" );
#if DEBUG
        KTRACE ( TRACE_QA, "total cost of comparisons = %zu", find_cost );
#endif
        return hsize;
    }

    static
    size_t brute_force_first_of ( const byte_t * haystack, size_t hsize,
                                  const byte_t * cset, size_t ccard )
    {
        assert ( hsize != 0 );
        assert ( ccard != 0 );
        
        // just mount a dumb scan
        for ( size_t i = 0; i < hsize; ++ i )
        {
            for ( size_t j = 0; j < ccard; ++ j )
            {
                if ( haystack [ i ] == cset [ j ] )
                    return i;
            }
        }

        // not found
        return hsize;
    }

    static
    size_t brute_force_last_of ( const byte_t * haystack, size_t hsize,
                                 const byte_t * cset, size_t ccard )
    {
        assert ( hsize != 0 );
        assert ( ccard != 0 );
        
        // just mount a dumb scan
        for ( size_t i = hsize; i != 0; )
        {
            -- i;
            for ( size_t j = 0; j < ccard; ++ j )
            {
                if ( haystack [ i ] == cset [ j ] )
                    return i;
            }
        }

        // not found
        return hsize;
    }

    template < class T > static
    size_t rodarmer_fwd_find ( const byte_t * haystack, size_t hsize,
                               const byte_t * needle, size_t nsize )
    {
        /* I'm sure somebody invented this and published back when I was a boy
         * but I'm just reinventing it now. Let's see how close I come to standard...
         */

        /* observation 1:
         *  At any given alignment A = ( 0, i ) where i is an offset in haystack,
         *  a find means that all pairings ( j, i+j ) match for j = 0..nsize-1.
         *  Since they all have to match, there is no reason why we would start with j=0. 
         *  in particular, 0 is far to the left and we are trying to make progress
         *  to the right. Better to start with j=nsize-1.
         *
         * observation 2:
         *  Naive comparison learns/retains a single bit of information from
         *  comparing ( j, i+j ), which is match-vs-mismatch. But if the value
         *  of the byte at i+j gives further information in the case of a mismatch,
         *  allowing us to ask whether this value occurs anywhere within the needle
         *  and if so, where? If it is not in the needle at all, then there is no benefit
         *  to be had by shifting the alignment one byte at a time, since we know the
         *  value at that particular address in the haystack will never match any byte
         *  within the needle. Better in this case to skip ahead by nsize, and all
         *  achieved with comparison of a single byte.
         *
         * observation 3:
         *  A mismatch generates either a new alignment or a not-found verdict. The
         *  new alignment will either be a shift by x ( where 0 < x < nsize ) to align
         *  with a known match, or it will shift by nsize because no bytes within
         *  needle match at this location. A match, on the other hand, will NOT realign,
         *  but will shift the focus toward needle[0] until a complete match is had or
         *  some mismatch occurs. A mismatch within the interior of the needle causes
         *  a realignment ( or not-found ), but becomes slightly more complicated.
         */

        // should have been checked below
        assert ( nsize != 0 );
        assert ( hsize > nsize );

#if DEBUG
        size_t find_cost = nsize;
#endif

        // prepare a table

#if DEBUG
        const T max_loc = ~ ( T ) 0;
        assert ( ( size_t ) max_loc >= nsize );
#endif

        /* function
         *  The function we'd like maps ( val, j ) => ( adj )
         *
         *  The primary driver has j = nsize-1, and says for all val in 0..255
         *  determine the adjustment to the current alignment, where the functional
         *  value of 0 indicates a match, and non-zero results in a realignment to
         *  either the nearest j that produces a match, i.e. the smallest adj such that
         *  ( val, j-adj ) => 0, OR the adjustment will be the smaller of
         *  ( nsize, max ( tbl ) ). If the table uses bytes to represent adj, then the
         *  maximum adjustment is 255, which would not be optimal in cases where nsize
         *  > 255.
         *
         *  A question arises about the dimensionality of the table to represent
         *  j != nsize-1. The theoretical function would indicate a pre-calculated adj
         *  for all ( val, j ) pairs, but the table size would necessarily grow quite
         *  large as nsize increases. I can imagine two approaches, where one is to put
         *  a limit on the dimension for j ( e.g. 16 ), and another is to use a single
         *  dimensional table that doesn't represent the full function, but only records
         *  last occurrence of a value within the needle.
         *    ( val ) => ( loc ) such that when j = loc, it's a match,
         *    and when j > loc, it gives a calculated adj of j-loc.
         *
         *  Finally, there is the question of the number of bits to use for offset or
         *  location information. This can be made programmatic. The size of the entries
         *  affects the overhead for preparing the table and also the cache performance.
         */
        T last_loc [ 16 ] [ 256 ];

        // only 1 level has been initialized
        // it tells how deep the table has to be in order to handle repeated values
        // if there are so many repeats that the table gets overflowed, it becomes useless
        N32 k, last_loc_levels = 0;

        // fill out table with value locations from needle
        size_t i, j;
        for ( j = nsize; j != 0; -- j )
        {
            // the value
            byte_t val = needle [ j - 1 ];
            for ( k = 0; k < sizeof last_loc / sizeof last_loc [ 0 ]; ++ k )
            {
                // if the table level has not been initialized,
                // zero it out now
                if ( k == last_loc_levels )
                {
                    KTRACE ( TRACE_PRG, "initializing last_loc [ %u ] with cost of %u", k, sizeof last_loc [ k ] );
                    :: memset ( & last_loc [ k ], 0, sizeof last_loc [ k ] );
                    ++ last_loc_levels;
#if DEBUG
                    find_cost += sizeof last_loc [ 0 ];
#endif
                }

                // if there is no entry for this value
                KTRACE ( TRACE_PRG, "checking last_loc [ %u ] [ %u ]", k, val );
                if ( last_loc [ k ] [ val ] == 0 )
                {
                    // record it as a 1-based offset
                    KTRACE ( TRACE_PRG, "setting last_loc [ %u ] [ %u ] to %zu", k, val, j );
                    last_loc [ k ] [ val ] = ( T ) j;
                    break;
                }
            }
        }
        
#if DEBUG
        size_t init_cost = find_cost;        
        KTRACE ( TRACE_QA, "assumed initialization overhead = %zu", find_cost );
#endif

        // move toward right across haystack
        for ( i = nsize - 1; i < hsize; )
        {
            // see if this matches ANYWHERE within the needle
            KTRACE ( TRACE_PRG, "testing alignment ( 0, %zu ) offset %zu: 0x%02x = 0x%02x ( '%c' = '%c' )"
                    , i - ( nsize - 1 )
                    , nsize - 1
                    , needle [ nsize - 1 ]
                    , haystack [ i ]
                    , ( :: isprint ( needle [ nsize - 1 ] ) ? ( char ) needle [ nsize - 1 ] : '?' )
                    , ( :: isprint ( haystack [ i ] ) ? ( char ) haystack [ i ] : '?' )
                );
#if DEBUG
            ++ find_cost;
#endif
            size_t loc = last_loc [ 0 ] [ haystack [ i ] ];
            if ( loc != 0 )
            {
                // if the match is at some other position, realign
                if ( loc != nsize )
                {
                    // there is a tiny amount of fruit here
                    // because by now we know of a match on the interior
                    // if remembering this would save cycles, it might
                    // be worth the trouble, but chances are that it will
                    // cost more than it will save. in any event, it is
                    // information lost now.
                    assert ( loc < nsize );
                    KTRACE ( TRACE_GEEK, "nearest byte found at offset %zu - advancing alignment from table by %zu"
                            , loc - 1
                            , nsize - loc
                        );
                    i += nsize - loc;
                realign:
                    continue;
                }

                KTRACE ( TRACE_GEEK, "bytes match at offset %zu", loc - 1 );

                // we have a match of the last byte in the needle
                // and in particular, the last match was at i-j+1
                for ( j = 1; j < nsize; ++ j )
                {
                    // subtraction in unsigned territory
                    assert ( i >= j );
                    KTRACE ( TRACE_PRG, "testing alignment ( 0, %zu ) offset %zu: 0x%02x = 0x%02x ( '%c' = '%c' )"
                            , i - ( nsize - 1 )
                            , nsize - j - 1
                            , needle [ nsize - j - 1 ]
                            , haystack [ i - j ]
                            , ( :: isprint ( needle [ nsize - j - 1 ] ) ? ( char ) needle [ nsize - j - 1 ] : '?' )
                            , ( :: isprint ( haystack [ i - j ] ) ? ( char ) haystack [ i - j ] : '?' )
                        );
#if DEBUG
                    ++ find_cost;
#endif
                    byte_t val = haystack [ i - j ];
                    loc = last_loc [ 0 ] [ val ];

                    // if there was no match here, need to look at how to realign
                    if ( loc == 0 )
                    {
                        // we know that the previous best match was at nsize-j
                        // and while we don't know how many times the value at
                        // nsize-j repeats, we know the alignment cannot contain
                        // this location. so shift just past the mismatch and go.
                        KTRACE ( TRACE_GEEK, "impossible value at - advancing alignment from table by %zu"
                                , nsize - j
                            );
                        i += nsize - j;
                        goto realign;
                    }

                    // the location at current table level may be to right
                    // quicker check for mismatch is to compare values
                    if ( needle [ nsize - 1 - j ] != val )
                    {
                        // create 1-based "j"
                        j = nsize - j;
                        assert ( j > 1 );
                        
                        // we're interested in finding a loc < current position
                        if ( loc < j )
                        {
                            KTRACE ( TRACE_GEEK, "nearest byte found at offset %zu - advancing alignment from table by %zu"
                                    , loc - 1
                                    , j - loc
                                );
                            i += j - loc;
                            goto realign;
                        }

                        // search table depth
                        for ( k = 1; k < sizeof last_loc / sizeof last_loc [ 0 ]; ++ k )
                        {
                            KTRACE ( TRACE_GEEK, "checking for offset in level %u", k );
#if DEBUG
                            ++ find_cost;
#endif
                            loc = last_loc [ k ] [ val ];
                            if ( loc < j )
                            {
                                KTRACE ( TRACE_GEEK, "nearest byte found at offset %zu - advancing alignment from table by %zu"
                                        , loc - 1
                                        , j - loc
                                    );
                                i += j - loc;
                                goto realign;
                            }
                        }

                        // overflowed - have little recourse
                        KTRACE ( TRACE_PRG, "highly repetitive value - table levels exhausted - advancing alignment by 1" );
                        i += 1;
                        goto realign;
                    }
                    
                    KTRACE ( TRACE_GEEK, "bytes match at offset %zu", loc - 1 );
                }

                // we arrived after looking at every byte in the needle, so
                // we've found its location.
                KTRACE ( TRACE_PRG, "regions align at ( 0, %zu )", i - ( nsize - 1 ) );
#if DEBUG
                KTRACE ( TRACE_QA, "cost of comparisons = %zu, total cost with init = %zu"
                        , find_cost - init_cost
                        , find_cost
                    );
#endif
                return i - ( nsize - 1 );
            }

            // no match anywhere - advance position
            KTRACE ( TRACE_GEEK, "impossible value at - advancing alignment from table by %zu"
                    , nsize
                );
            i += nsize;
        }

        // not found
        KTRACE ( TRACE_PRG, "regions do not align" );
#if DEBUG
        KTRACE ( TRACE_QA, "total cost of comparisons = %zu", find_cost );
#endif
        return hsize;
    }

    template < class T > static
    size_t rodarmer_rev_find ( const byte_t * haystack, size_t hsize,
                               const byte_t * needle, size_t nsize )
    {
        // should have been checked below
        assert ( nsize != 0 );
        assert ( hsize > nsize );

#if DEBUG
        size_t find_cost = nsize;
#endif

        // prepare a table

#if DEBUG
        const T max_loc = ~ ( T ) 0;
        assert ( ( size_t ) max_loc >= nsize );
#endif
        
        T next_loc [ 16 ] [ 256 ];

        // only 1 level has been initialized
        // it tells how deep the table has to be in order to handle repeated values
        // if there are so many repeats that the table gets overflowed, it becomes useless
        N32 k, next_loc_levels = 0;

        // fill out table with value locations from needle
        size_t i, j;
        for ( j = 0; j < nsize; ++ j )
        {
            // the value
            byte_t val = needle [ j ];
            for ( k = 0; k < sizeof next_loc / sizeof next_loc [ 0 ]; ++ k )
            {
                // if the table level has not been initialized,
                // zero it out now
                if ( k == next_loc_levels )
                {
                    KTRACE ( TRACE_PRG, "initializing next_loc [ %u ] with cost of %u", k, sizeof next_loc [ k ] );
                    :: memset ( & next_loc [ k ], 0, sizeof next_loc [ k ] );
                    ++ next_loc_levels;
#if DEBUG
                    find_cost += sizeof next_loc [ 0 ];
#endif
                }

                // if there is no entry for this value
                KTRACE ( TRACE_PRG, "checking next_loc [ %u ] [ %u ]", k, val );
                if ( next_loc [ k ] [ val ] == 0 )
                {
                    // record it as a 1-based offset
                    KTRACE ( TRACE_PRG, "setting next_loc [ %u ] [ %u ] to %zu", k, val, j + 1 );
                    next_loc [ k ] [ val ] = ( T ) j + 1;
                    break;
                }
            }
        }
        
#if DEBUG
        size_t init_cost = find_cost;        
        KTRACE ( TRACE_QA, "assumed initialization overhead = %zu", find_cost );
#endif

        // move toward left across haystack
        size_t dsize = hsize - nsize;
        for ( i = 0; i <= dsize; )
        {
            // see if this matches ANYWHERE within the needle
            KTRACE ( TRACE_PRG, "testing alignment ( 0, %zu ) offset 0: 0x%02x = 0x%02x ( '%c' = '%c' )"
                    , dsize - i
                    , needle [ 0 ]
                    , haystack [ dsize - i ]
                    , ( :: isprint ( needle [ 0 ] ) ? ( char ) needle [ 0 ] : '?' )
                    , ( :: isprint ( haystack [ dsize - i ] ) ? ( char ) haystack [ dsize - i ] : '?' )
                );
#if DEBUG
            ++ find_cost;
#endif
            size_t loc = next_loc [ 0 ] [ haystack [ dsize - i ] ];
            if ( loc != 0 )
            {
                // if the match is at some other position, realign
                if ( loc != 1 )
                {
                    KTRACE ( TRACE_GEEK, "nearest byte found at offset %zu - advancing alignment from table by %zu"
                            , loc - 1
                            , loc - 1
                        );
                    i += loc - 1;
                realign:
                    continue;
                }

                KTRACE ( TRACE_GEEK, "bytes match at offset %zu", loc - 1 );

                // we have a match of the first byte in the needle
                for ( j = 1; j < nsize; ++ j )
                {
                    KTRACE ( TRACE_PRG, "testing alignment ( 0, %zu ) offset %zu: 0x%02x = 0x%02x ( '%c' = '%c' )"
                            , dsize - i
                            , j
                            , needle [ j ]
                            , haystack [ dsize - i + j ]
                            , ( :: isprint ( needle [ j ] ) ? ( char ) needle [ j ] : '?' )
                            , ( :: isprint ( haystack [ dsize - i + j ] ) ? ( char ) haystack [ dsize - i + j ] : '?' )
                        );
#if DEBUG
                    ++ find_cost;
#endif
                    byte_t val = haystack [ dsize - i + j ];
                    loc = next_loc [ 0 ] [ val ];

                    // if there was no match here, need to look at how to realign
                    if ( loc == 0 )
                    {
                        KTRACE ( TRACE_GEEK, "impossible value at - advancing alignment from table by %zu"
                                , j
                            );
                        i += j;
                        goto realign;
                    }

                    // the location at current table level may be to right
                    // quicker check for mismatch is to compare values
                    if ( needle [ j ] != val )
                    {
                        // create 1-based "j"
                        ++ j;
                        
                        // we're interested in finding a loc > current position
                        if ( loc > j )
                        {
                            KTRACE ( TRACE_GEEK, "nearest byte found at offset %zu - advancing alignment from table by %zu"
                                    , loc - 1
                                    , loc - j
                                );
                            i += loc - j;
                            goto realign;
                        }

                        // search table depth
                        for ( k = 1; k < sizeof next_loc / sizeof next_loc [ 0 ]; ++ k )
                        {
                            KTRACE ( TRACE_GEEK, "checking for offset in level %u", k );
#if DEBUG
                            ++ find_cost;
#endif
                            loc = next_loc [ k ] [ val ];
                            if ( loc > j )
                            {
                                KTRACE ( TRACE_GEEK, "nearest byte found at offset %zu - advancing alignment from table by %zu"
                                        , loc - 1
                                        , loc - j
                                    );
                                i += loc - j;
                                goto realign;
                            }
                        }

                        // overflowed - have little recourse
                        KTRACE ( TRACE_PRG, "highly repetitive value - table levels exhausted - advancing alignment by 1" );
                        i += 1;
                        goto realign;
                    }
                    
                    KTRACE ( TRACE_GEEK, "bytes match at offset %zu", loc - 1 );
                }

                // we arrived after looking at every byte in the needle, so
                // we've found its location.
                KTRACE ( TRACE_PRG, "regions align at ( 0, %zu )", dsize - i );
#if DEBUG
                KTRACE ( TRACE_QA, "cost of comparisons = %zu, total cost with init = %zu"
                        , find_cost - init_cost
                        , find_cost
                    );
#endif
                return dsize - i;
            }

            // no match anywhere - advance position
            KTRACE ( TRACE_GEEK, "impossible value at - advancing alignment from table by %zu"
                    , nsize
                );

            i += nsize;
        }

        // not found
        KTRACE ( TRACE_PRG, "regions do not align" );
#if DEBUG
        KTRACE ( TRACE_QA, "total cost of comparisons = %zu", find_cost );
#endif
        return hsize;
    }
    

    /*=====================================================*
     *                         Rgn                         *
     *=====================================================*/

    /**
     * getByte
     *  @param idx quantity in 0..size()-1
     *  @return indexed byte
     */
    byte_t Rgn :: getByte ( size_t idx ) const
    {
        if ( idx >= sz )
        {
            throw BoundsException (
                XP ( XLOC )
                << "byte index ( "
                << idx
                << " ) is at or beyond end of region ( "
                << sz
                << " )"
                );
        }

        return getAddr () [ idx ];
    }

    /**
     * equal
     *  @brief test two regions for content equality
     *  @param r a region to compare against self
     *  @return true iff two regions have equal content
     */
    bool Rgn :: equal ( const Rgn & r ) const noexcept
    {
        // TBD - we may need to utilize constant time versions...
        return ( sz == r . sz &&
                 :: memcmp ( self . getAddr (), r . getAddr (), sz ) == 0
            );
    }

    /**
     * compare
     *  @brief compare self against another region
     *  @r region to compare against self
     *  @return "self" - "r" => { < Z-, 0, Z+ }
     */
    int Rgn :: compare ( const Rgn & r ) const noexcept
    {
        // TBD - we may need to utilize constant time versions...
        size_t min_sz = r . sz;
        if ( min_sz > sz )
             min_sz = sz;

        int diff = :: memcmp ( self . getAddr (), r . getAddr (), min_sz );
        if ( diff != 0 )
            return ( diff < 0 ) ? -1 : 1;

        if ( sz < r . sz )
            return -1;
        return sz > r . sz;
    }

    /**
     * fwdFind
     * @overload forward search to find a sub-region
     * @param sub the sub-region being sought
     * @return the location of start of sub-region if found, otherwise end of region
     */
    size_t Rgn :: fwdFind ( const Rgn & sub ) const noexcept
    {
        // perform early pruning
        if ( self . size () < sub . size () )
            return end ();
        if ( sub . size () == 0 )
            return 0;
        
        // there are a number of different algorithms
        // while the brute force way is generally considered bad,
        // it will make sense for small regions where the overhead
        // for preprocessing makes the efficient algorithms overkill.
        if ( sub . size () < brute_force_size_limit )
            return brute_force_fwd_find ( self . getAddr (), self . size (), sub . getAddr (), sub . size () );

        // run the version based upon length of sub-region
        if ( sub . size () < 0xFFU )
            return rodarmer_fwd_find < N8 > ( self . getAddr (), self . size (), sub . getAddr (), sub . size () );
        if ( sub . size () < 0xFFFFU )
            return rodarmer_fwd_find < N16 > ( self . getAddr (), self . size (), sub . getAddr (), sub . size () );
        if ( sub . size () <= 0xFFFFFFFFU )
            return rodarmer_fwd_find < N32 > ( self . getAddr (), self . size (), sub . getAddr (), sub . size () );
        return rodarmer_fwd_find < N64 > ( self . getAddr (), self . size (), sub . getAddr (), sub . size () );
    }

    /**
     * fwdFind
     * @overload forward search to find a single character
     * @param b a byte_t
     * @return the location of start of sub-region if found, otherwise end of region
     */
    size_t Rgn :: fwdFind ( byte_t b ) const noexcept
    {
        // kinda weird, but it's what you have
        if ( self . size () == 0 )
            return end ();

        // there aren't too many ways around this one, other than
        // using SSE registers and the like
        for ( size_t i = 0; i < sz; ++ i )
        {
            if ( a [ i ] == b )
                return i;
        }

        // not found
        return end ();
    }

    /**
     * revFind
     * @overload reverse search to find a sub-region
     * @param sub the sub-region being sought
     * @return the location of start of sub-region if found, otherwise end of region
     */
    size_t Rgn :: revFind ( const Rgn & sub ) const noexcept
    {
        // perform early pruning
        if ( self . size () < sub . size () )
            return end ();
        if ( sub . size () == 0 )
            return 0;
        
        // there are a number of different algorithms
        // while the brute force way is generally considered bad,
        // it will make sense for small regions where the overhead
        // for preprocessing makes the efficient algorithms overkill.
        if ( sub . size () < brute_force_size_limit )
            return brute_force_rev_find ( self . getAddr (), self . size (), sub . getAddr (), sub . size () );

        // run the version based upon length of sub-region
        if ( sub . size () < 0xFFU )
            return rodarmer_rev_find < N8 > ( self . getAddr (), self . size (), sub . getAddr (), sub . size () );
        if ( sub . size () < 0xFFFFU )
            return rodarmer_rev_find < N16 > ( self . getAddr (), self . size (), sub . getAddr (), sub . size () );
        if ( sub . size () <= 0xFFFFFFFFU )
            return rodarmer_rev_find < N32 > ( self . getAddr (), self . size (), sub . getAddr (), sub . size () );
        return rodarmer_rev_find < N64 > ( self . getAddr (), self . size (), sub . getAddr (), sub . size () );
    }

    /**
     * revFind
     * @overload reverse search to find a single character
     * @param b a byte_t
     * @return the location of start of sub-region if found, otherwise end of region
     */
    size_t Rgn :: revFind ( byte_t b ) const noexcept
    {
        // kinda weird, but it's what you have
        if ( self . size () == 0 )
            return end ();

        // there aren't too many ways around this one, other than
        // using SSE registers and the like
        for ( size_t i = sz; i != 0; )
        {
            -- i;
            if ( a [ i ] == b )
                return i;
        }

        // not found
        return end ();
    }
    
    /**
     * findFirstOf
     * @overload forward search to find a character of a set
     * @param cset the set of characters being sought
     * @return the location of character if found, otherwise end of region
     */
    size_t Rgn :: findFirstOf ( const Rgn & cset ) const noexcept
    {
        // kinda weird, but it's what you have
        if ( self . size () == 0 )
            return end ();
        if ( cset . size () == 0 )
            return 0;

        // TBD - there are a number of different algorithms
        // while the brute force way is generally considered bad,
        // it will make sense for small regions where the overhead
        // for preprocessing makes the efficient algorithms overkill.
        return brute_force_first_of ( self . getAddr (), self . size (), cset . getAddr (), cset . size () );
    }
    
    /**
     * findLastOf
     * @overload reverse search to find a character of a set
     * @param cset the set of characters being sought
     * @return the location of character if found, otherwise end of region
     */
    size_t Rgn :: findLastOf ( const Rgn & cset ) const noexcept
    {
        // kinda weird, but it's what you have
        if ( self . size () == 0 )
            return end ();
        if ( cset . size () == 0 )
            return 0;

        // TBD - there are a number of different algorithms
        // while the brute force way is generally considered bad,
        // it will make sense for small regions where the overhead
        // for preprocessing makes the efficient algorithms overkill.
        return brute_force_last_of ( self . getAddr (), self . size (), cset . getAddr (), cset . size () );
    }

    /**
     * beginsWith
     * @overload test whether self region begins with sub region
     * @param sub region with subsequence in question
     * @return Boolean true if self begins with "sub"
     */
    bool Rgn :: beginsWith ( const Rgn & sub ) const noexcept
    {
        if ( self . size () < sub . size () )
            return false;
        if ( sub . size () == 0 )
            return true;
        return brute_force_fwd_find
            ( self . getAddr (), sub . size (), sub . getAddr (), sub . size () ) == 0;
    }

    /**
     * beginsWith
     * @overload test whether self region begins with character
     * @param b byte_t with value in question
     * @return Boolean true if self begins with "ch"
     */
    bool Rgn :: beginsWith ( byte_t b ) const noexcept
    {
        if ( self . size () == 0 )
            return false;
        return a [ 0 ] == b;
    }

    /**
     * endsWith
     * @overload test whether self region ends with sub region
     * @param sub region with subsequence in question
     * @return Boolean true if self ends with "sub"
     */
    bool Rgn :: endsWith ( const Rgn & sub ) const noexcept
    {
        if ( self . size () < sub . size () )
            return false;
        if ( sub . size () == 0 )
            return true;
        size_t offset = self . size () - sub . size ();
        return brute_force_fwd_find
            ( self . getAddr () + offset, sub . size (), sub . getAddr (), sub . size () ) == 0;
    }

    /**
     * endsWith
     * @overload test whether self region ends with character
     * @param b byte_t with value in question
     * @return Boolean true if self ends with "ch"
     */
    bool Rgn :: endsWith ( byte_t b ) const noexcept
    {
        if ( self . size () == 0 )
            return false;
        return a [ sz - 1 ] == b;
    }
    
    /*=====================================================*
     *                         MRgn                        *
     *=====================================================*/

    /**
     * copy
     *  @brief copy bytes from a source rgn
     *  @return subset of dest region that was updated
     */
    MRgn MRgn :: copy ( const CRgn & src ) const noexcept
    {
        if ( self . size () > src . size () )
        {
            :: memmove ( a, src . addr (), src . size () );
            return MRgn ( a, src . size () );
        }
        
        :: memmove ( a, src . addr (), sz );
        return MRgn ( self );
    }

    /**
     * fill
     *  @brief set all bytes of region to single value
     */
    void MRgn :: fill ( byte_t val ) const noexcept
    {
        :: memset ( a, val, sz );
    }

    /**
     * subRgn
     *  @brief create a new mutable region that is a subset of original
     *  @param offset the number of bytes from origin to start of sub-region
     *  @return subset of original region - empty if no intersection
     */
    MRgn MRgn :: subRgn ( size_t offset ) const noexcept
    {
        // whole thing
        if ( offset == 0 )
            return MRgn ( self );
        
        // offset off the end
        if ( offset >= sz )
            return MRgn ( a + sz, 0 );

        // legitimate offset
        return MRgn ( a + offset, sz - offset );
    }

    /**
     * subRgn
     *  @brief create a new mutable region that is a subset of original
     *  @param offset the number of bytes from origin to start of sub-region
     *  @param bytes the size in bytes of the sub-region intersected with original
     *  @return subset of original region - empty if no intersection
     */
    MRgn MRgn :: subRgn ( size_t offset, size_t bytes ) const noexcept
    {
        MRgn sub = self . subRgn ( offset );
        if ( sub . sz > bytes )
            sub . sz = bytes;
        return sub;
    }

    /**
     * intersect
     *  @return intersection between regions
     */
    MRgn MRgn :: intersect ( const MRgn & r ) const noexcept
    {
        byte_t * endp = self . a + self . sz;
        byte_t * r_endp = r . a + r . sz;

        // detect overlap where r >= self
        if ( self . a <= r . a && r . a <= endp )
        {
            // find nearest end
            if ( endp > r_endp )
                endp = r_endp;

            // return intersection
            return MRgn ( r . a, endp - r . a );
        }

        // detect overlap where r < self
        if ( r . a <= self . a && self . a <= r_endp )
        {
            // find nearest end
            if ( endp > r_endp )
                endp = r_endp;

            // return intersection
            return MRgn ( self . a, endp - self . a );
        }

        // no overlap
        return MRgn ();
    }

    /**
     * join
     *  @return union of regions
     *
     *  this is a little fuzzy...
     *  in terms of memory allocations, we would be prohibited
     *  from joining two adjascent but non-overlapping regions.
     *  and yet, having two valid regions that are contiguous,
     *  there should be no harm in joining them, except that
     *  this could produce Frankenstein regions that cross
     *  allocation boundaries and become invalid at later times...
     */
    MRgn MRgn :: join ( const MRgn & r ) const
    {
        byte_t * endp = self . a + self . sz;
        byte_t * r_endp = r . a + r . sz;

        // detect overlap where r >= self
        if ( self . a <= r . a && r . a <= endp )
        {
            // find farthest end
            if ( endp < r_endp )
                endp = r_endp;

            // return union
            return MRgn ( self . a, endp - self . a );
        }

        // detect overlap where r < self
        if ( r . a <= self . a && self . a <= r_endp )
        {
            // find farthest end
            if ( endp < r_endp )
                endp = r_endp;

            // return union
            return MRgn ( r . a, endp - r . a );
        }

        // no overlap
        throw NoCommonRgnException (
            XP ( XLOC )
            << xprob
            << "cannot join regions"
            << xcause
            << "regions are disjoint"
            );
    }

    
    /*=====================================================*
     *                         CRgn                        *
     *=====================================================*/

    /**
     * subRgn
     *  @brief create a new mutable region that is a subset of original
     *  @param offset the number of bytes from origin to start of sub-region
     *  @return subset of original region - empty if no intersection
     */
    CRgn CRgn :: subRgn ( size_t offset ) const noexcept
    {
        // whole thing
        if ( offset == 0 )
            return CRgn ( self );
        
        // offset off the end
        if ( offset >= sz )
            return CRgn ( a + sz, 0 );
        
        // legitimate offset
        return CRgn ( a + offset, sz - offset );
    }

    /**
     * subRgn
     *  @brief create a new mutable region that is a subset of original
     *  @param offset the number of bytes from origin to start of sub-region
     *  @param bytes the size in bytes of the sub-region intersected with original
     *  @return subset of original region - empty if no intersection
     */
    CRgn CRgn :: subRgn ( size_t offset, size_t bytes ) const noexcept
    {
        CRgn sub = self . subRgn ( offset );
        if ( sub . sz > bytes )
            sub . sz = bytes;
        return sub;
    }

    /**
     * intersect
     *  @return intersection between regions
     */
    CRgn CRgn :: intersect ( const CRgn & r ) const noexcept
    {
        const byte_t * endp = self . a + self . sz;
        const byte_t * r_endp = r . a + r . sz;

        // detect overlap where r >= self
        if ( self . a <= r . a && r . a <= endp )
        {
            // find nearest end
            if ( endp > r_endp )
                endp = r_endp;

            // return intersection
            return CRgn ( r . a, endp - r . a );
        }

        // detect overlap where r < self
        if ( r . a <= self . a && self . a <= r_endp )
        {
            // find nearest end
            if ( endp > r_endp )
                endp = r_endp;

            // return intersection
            return CRgn ( self . a, endp - self . a );
        }

        // no overlap
        return CRgn ();
    }

    /**
     * join
     *  @return union of regions
     */
    CRgn CRgn :: join ( const CRgn & r ) const
    {
        const byte_t * endp = self . a + self . sz;
        const byte_t * r_endp = r . a + r . sz;

        // detect overlap where r >= self
        if ( self . a <= r . a && r . a <= endp )
        {
            // find farthest end
            if ( endp < r_endp )
                endp = r_endp;

            // return union
            return CRgn ( self . a, endp - self . a );
        }

        // detect overlap where r < self
        if ( r . a <= self . a && self . a <= r_endp )
        {
            // find farthest end
            if ( endp < r_endp )
                endp = r_endp;

            // return union
            return CRgn ( r . a, endp - r . a );
        }

        // no overlap
        throw NoCommonRgnException (
            XP ( XLOC )
            << xprob
            << "cannot join regions"
            << xcause
            << "regions are disjoint"
            );
    }
}
