/*

  vdb3.cmn.text-utf8

 */

#include <vdb3/cmn/text.hpp>

#include <ctype.h>
#include <wctype.h>

// which is faster - the "if" or always rewriting and blowing cache?
#define TOLOWER_USE_IF 1

namespace vdb3
{
    
    /*=====================================================*
     *                       UTILITY                       *
     *=====================================================*/

    /* is_ascii
     *  easier to check with constant code than using lookup table
     */
    static inline
    bool is_ascii ( UTF32 ch )
    {
        return ch < 128;
    }

    /* scan_fwd
     *  used mainly by iterators to step across a single character
     *  returns the number of bytes advanced
     */
    static
    size_t scan_fwd ( const signed char * data, size_t bytes, size_t offset )
    {
        if ( offset >= bytes )
            return 0;

        if ( data [ offset ] >= 0 )
            return 1;

        N32 ch = data [ offset ];
        return __builtin_clz ( ~ ( ch << 24 ) );
    }

    static inline
    size_t scan_fwd ( const UTF8 * data, size_t bytes, size_t offset )
    {
        return scan_fwd ( ( const signed char * ) data, bytes, offset );
    }

    /* scan_rev
     *  used by iterators to back up by a single character
     *  returns the number of bytes backed over
     */
    static
    size_t scan_rev ( const signed char * data, size_t bytes, size_t offset )
    {
        if ( offset == 0 || offset > bytes )
            return 0;
    
        if ( data [ offset - 1 ] >= 0 )
            return 1;

        ssize_t clen = 1;
        for ( ; offset > 0; ++ clen )
        {
            if ( ( data [ -- offset ] & 0xC0 ) != 0x80 )
                break;
        }

        N32 ch = data [ offset ];
        if ( clen != __builtin_clz ( ~ ( ch << 24 ) ) )
            throw InvalidUTF8 ( XP ( XLOC ) << "badly formed UTF-8 character" );

        return clen;
    }

    static inline
    size_t scan_rev ( const UTF8 * data, size_t bytes, size_t offset )
    {
        return scan_rev ( ( const signed char * ) data, bytes, offset );
    }

    /* region_length_verify
     *  measure the text region in characters and verify integrity
     */
    static
    count_t region_length_verify ( const signed char * data, size_t bytes, size_t * stop_on_bad = nullptr )
    {
        // allow empty string
        count_t cnt = 0;

        // walk across all bytes
        for ( size_t i = 0; i < bytes; ++ cnt )
        {
            // skip over ASCII; drop into UTF-8
            if ( data [ i ] < 0 )
            {
                // create word from leading byte
                unsigned int ch = data [ i ];

                // remember starting position
                size_t start = i;

                // by inverting leading byte and counting leading zeros,
                // we should find exactly the number of bytes in char
                size_t clen = __builtin_clz ( ~ ( ch << 24 ) );
                if ( clen < 2 || clen > 4 )
                {
                    if ( stop_on_bad != nullptr )
                    {
                        * stop_on_bad = start;
                        return cnt;
                    }
                    
                    throw InvalidUTF8 ( XP ( XLOC ) << "badly formed UTF-8 character" );
                }

                // walk across non-leading bytes
                for ( ++ i; i < bytes; ++ i )
                {
                    // any that is not 0x80 | 6-bits ends char
                    if ( ( data [ i ] & 0xC0 ) != 0x80 )
                        break;
                }

                // the total length of multi-byte char should match
                if ( i - start != clen )
                {
                    if ( stop_on_bad != nullptr )
                    {
                        * stop_on_bad = start;
                        return cnt;
                    }
                    
                    throw InvalidUTF8 ( XP ( XLOC ) << "badly formed UTF-8 character" );
                }

                continue;
            }

            // advance over ASCII character
            ++ i;
        }

        return cnt;
    }

    static inline
    count_t region_length_verify ( const UTF8 * data, size_t bytes, size_t * stop_on_bad = nullptr )
    {
        return region_length_verify ( ( const signed char * ) data, bytes, stop_on_bad );
    }

    /* region_length
     *  measure the length in characters of an already verified region
     */
    static
    count_t region_length ( const signed char * data, size_t bytes )
    {
        // allow empty string
        count_t cnt = 0;

        // walk across all bytes
        for ( size_t i = 0; i < bytes; ++ cnt )
        {
            // skip over ASCII; drop into UTF-8
            if ( data [ i ] < 0 )
            {
                // create word from leading byte
                unsigned int ch = data [ i ];

                // by inverting leading byte and counting leading zeros,
                // we should find exactly the number of bytes in char
                size_t clen = __builtin_clz ( ~ ( ch << 24 ) );
                if ( clen < 2 || clen > 4 )
                    throw InvalidUTF8 ( XP ( XLOC ) << "badly formed UTF-8 character" );

                // advance over UNICODE character
                i += clen;
                continue;
            }

            // advance over ASCII character
            ++ i;
        }

        return cnt;
    }

    static inline
    count_t region_length ( const UTF8 * data, size_t bytes )
    {
        return region_length ( ( const signed char * ) data, bytes );
    }

    /* char_idx_to_byte_off
     *  linearly scan a UTF-8 text region to find a character
     *  return the byte offset from the beginning of data
     */
    static
    size_t char_idx_to_byte_off ( const signed char * data, size_t bytes, count_t idx )
    {
        // minimally ASCII
        assert ( idx <= ( count_t ) bytes );

        count_t cnt = 0;
        for ( size_t i = 0; i < bytes; ++ cnt )
        {
            // see if this is the spot
            if ( cnt == idx )
                return i;

            // detect non-ASCII character
            // will have bit 7 set
            if ( data [ i ] < 0 )
            {
                unsigned int ch = data [ i ];
                unsigned int clen = __builtin_clz ( ~ ( ch << 24 ) );

                // this may not be necessary for pre-validated strings
                // without it, however, there is no guarantee of advancement
                // across the string or exit from the loop.
                if ( clen < 2 || clen > 4 )
                {
                    throw InvalidUTF8 (
                        XP ( XLOC )
                        << xprob
                        << "badly formed UTF-8 character"
                        << xcause
                        << "expected 2..4 bytes but found "
                        << clen
                        );
                }

                i += clen;
                continue;
            }

            // advance over ASCII character
            ++ i;
        }

        throw InvalidUTF8 ( XP ( XLOC ) << "character count mismatch" );
    }
    
    static inline
    size_t char_idx_to_byte_off ( const UTF8 * data, size_t bytes, count_t idx )
    {
        return char_idx_to_byte_off ( ( const signed char * ) data, bytes, idx );
    }

    /* utf8_to_utf32
     *  gather 1..4 bytes of UTF-8 into a single UTF-32 character
     *  input data are trusted to have been pre-validated.
     *
     *  at present, there are guards in there to assert invariants,
     *  but they may be conditionally relaxed for performance if deemed
     *  a significant improvement.
     */
#if ! UniChar_DEFINED
#define  UniChar_DEFINED 1
    struct UniChar
    {
        UniChar ( UTF32 _ch, N32 sz )
            : ch ( _ch )
            , size ( sz )
        {
        }

        UTF32 ch;
        N32 size;
    };
#endif
    
    static
    UniChar utf8_to_utf32 ( const signed char * data, size_t bytes, size_t offset )
    {
        // should have been checked by caller
        if ( offset >= bytes )
            throw BoundsException ( XP ( XLOC ) << "offset leaves no data" );

        // detect ASCII
        if ( data [ offset ] >= 0 )
            return UniChar ( data [ offset ], 1 );

        // ease of typecast
        const unsigned char * p = reinterpret_cast < const unsigned char * > ( data + offset );

        // determine UTF-8 character length
        UTF32 ch = p [ 0 ];
        unsigned int b2, b3, b4;
        unsigned int clen = __builtin_clz ( ~ ( ch << 24 ) );

        // should have already been pre-validated
        // this test could be removed if deemed significant
        // but then, there are much faster algorithms if performance is at issue
        if ( offset + clen > bytes )
            throw InvalidUTF8 ( XP ( XLOC ) << "badly formed UTF-8 character" );

        // all characters are at least 2 bytes
        b2 = p [ 1 ];

        // mechanically assemble the code
        // assuming that correctness has already been verified
        switch ( clen )
        {
        case 2:
            ch = ( ( ch & 0x1F ) << 6 ) | ( b2 & 0x3F );
            break;
        case 3:
            b3 = p [ 2 ];
            ch = ( ( ch & 0x0F ) << 12 ) | ( ( b2 & 0x3F ) << 6 ) | ( b3 & 0x3F );
            break;
        case 4:
            b3 = p [ 2 ];
            b4 = p [ 3 ];
            ch = ( ( ch & 0x07 ) << 18 ) | ( ( b2 & 0x3F ) << 12 ) | ( ( b3 & 0x3F ) << 6 ) | ( b4 & 0x3F );
            break;
        default:
            // should never happen
            throw InvalidUTF8 ( XP ( XLOC ) << "badly formed UTF-8 character" );
        }

        return UniChar ( ch, clen );
    }

    static inline
    UniChar utf8_to_utf32 ( const UTF8 * data, size_t bytes, size_t offset )
    {
        return utf8_to_utf32 ( ( const signed char * ) data, bytes, offset );
    }

    static
    size_t utf32_to_utf8 ( UTF8 * buff, size_t bsize, UTF32 ch )
    {
        // detect ASCII
        if ( is_ascii ( ch ) )
        {
            if ( bsize == 0 )
            {
                throw InsufficientBuffer (
                    XP ( XLOC )
                    << "buffer size ( "
                    << bsize
                    << " bytes ) insufficient ( "
                    << 1
                    << " needed ) when converting UTF-32 to UTF-8"
                    );
            }

            buff [ 0 ] = static_cast < UTF8 > ( ch );
            return 1;
        }

        // given that this is assumed to be a well-formed UTF-32 character,
        // we should observe the following pattern of leading zeros:
        //
        //   25..32 => single byte ( ASCII )
        //   21..24 => two byte
        //   16..20 => three byte
        //   11..15 => four byte

        unsigned int zeros = __builtin_clz ( ch );
        if ( zeros > 20 )
        {
            // if ch >= 128, there can only be up to 24 zero bits
            // this asserts that our reliance on __builtin_clz() is
            // correct and that it's being used properly.
            assert ( zeros < 25U );

            if ( bsize < 2 )
            {
                throw InsufficientBuffer (
                    XP ( XLOC )
                    << "buffer size ( "
                    << bsize
                    << " bytes ) insufficient ( "
                    << 2
                    << " needed ) when converting UTF-32 to UTF-8"
                    );
            }

            buff [ 0 ] = static_cast < UTF8 > ( ( ch >> 6 ) | 0xC0 );
            buff [ 1 ] = static_cast < UTF8 > ( ( ch & 0x3F ) | 0x80 );
            return 2;
        }
        if ( zeros > 15 )
        {
            if ( bsize < 3 )
            {
                throw InsufficientBuffer (
                    XP ( XLOC )
                    << "buffer size ( "
                    << bsize
                    << " bytes ) insufficient ( "
                    << 3
                    << " needed ) when converting UTF-32 to UTF-8"
                    );
            }

            buff [ 0 ] = static_cast < UTF8 > ( ( ch >> 12 ) | 0xE0 );
            buff [ 1 ] = static_cast < UTF8 > ( ( ( ch >> 6 ) & 0x3F ) | 0x80 );
            buff [ 2 ] = static_cast < UTF8 > ( ( ch & 0x3F ) | 0x80 );
            return 3;
        }
        if ( zeros > 10 )
        {
            if ( bsize < 4 )
            {
                throw InsufficientBuffer (
                    XP ( XLOC )
                    << "buffer size ( "
                    << bsize
                    << " bytes ) insufficient ( "
                    << 4
                    << " needed ) when converting UTF-32 to UTF-8"
                    );
            }

            buff [ 0 ] = static_cast < UTF8 > ( ( ch >> 18 ) | 0xF0 );
            buff [ 1 ] = static_cast < UTF8 > ( ( ( ch >> 12 ) & 0x3F ) | 0x80 );
            buff [ 2 ] = static_cast < UTF8 > ( ( ( ch >> 6 ) & 0x3F ) | 0x80 );
            buff [ 3 ] = static_cast < UTF8 > ( ( ch & 0x3F ) | 0x80 );
            return 4;
        }

        throw InvalidUTF32Character ( XP ( XLOC ) << "badly formed UTF-32 character" );
    }

    size_t utf32_to_utf8_strict ( UTF8 * buff, size_t bsize, UTF32 ch )
    {
        if ( ch >= 0xD800 && ch <= 0xDFFF )
            throw InvalidUTF32Character ( XP ( XLOC ) << "surrogate code points are not included in the set of Unicode scalar values" );
        if ( ch > 0x10FFFF )
            throw InvalidUTF32Character ( XP ( XLOC ) << "badly formed UTF-32 character" );

        // TBD - check for other oddities

        return utf32_to_utf8 ( buff, bsize, ch );
    }

#if 0
    static
    void test_for_split_UTF8 ( const UTF8 * data, size_t bytes, size_t offset )
    {
        UTF8 ch;
        assert ( offset < bytes );
        ch = data [ offset ];
        if ( ( ch & 0xC0 ) == 0x80 )
        {
            // this is because in order to avoid splitting,
            // a remainder would have to be left in each
            throw SplitCharacterException (
                XP ( XLOC )
                << "copy would create an invalid UTF-8 character"
                );
        }
    }
#endif


    /*=====================================================*
     *                       Text                       *
     *=====================================================*/

    UTF32 Text :: getChar ( index_t idx ) const
    {
        if ( idx < 0 || ( count_t ) idx >= cnt )
        {
            throw BoundsException (
                XP ( XLOC )
                << "character index ( "
                << idx
                << " ) is at or beyond end of text region ( "
                << cnt
                << " )"
                );
        }

        // rather than accessing the member variable directly,
        // which is stored as a non-const pointer,
        // use the accessor that will return a const pointer
        const UTF8 * data = getAddr ();

        // if the region is 100% ASCII, just retrieve it directly
        if ( self . isAscii () )
            return data [ idx ];

        // TBD - introduce a reverse scan from end in the case
        // that "idx" is closer to the end of the region

        // scan the UTF-8 to find where the character should start
        size_t offset = char_idx_to_byte_off ( data, sz, idx );

        // no exception, so the offset should be good
        assert ( offset < sz );
        return utf8_to_utf32 ( data, sz, offset ) . ch;
    }

#if TESTING
    static
    void sanity_check ( const CharLoc & loc, size_t sz, count_t cnt, const UTF8 * a )
    {
        // a location decremented into negative range
        if ( loc . index () <= 0 )
        {
            // must pin offset at 0
            if ( loc . offset () != 0 )
            {
                throw InvalidStateException (
                    XP ( XLOC )
                    << "CharLoc ( "
                    << loc . offset ()
                    << ", "
                    << loc . index ()
                    << " ) has character index <= 0 and non-zero offset"
                    );
            }
        }
        // a location at or beyond right edge, exclusive
        else if ( ( count_t ) loc . index () >= cnt )
        {
            // must pin offset at region size, exclusive
            if ( loc . offset () != sz )
            {
                throw InvalidStateException (
                    XP ( XLOC )
                    << "CharLoc ( "
                    << loc . offset ()
                    << ", "
                    << loc . index ()
                    << " ) has out-of-bounds character index and offset != "
                    << sz
                    );
            }
        }
        // an ASCII region
        else if ( sz == cnt )
        {
            // must have offset match character index
            if ( loc . offset () != ( size_t ) loc . index () )
            {
                throw InvalidStateException (
                    XP ( XLOC )
                    << "CharLoc ( "
                    << loc . offset ()
                    << ", "
                    << loc . index ()
                    << " ) expects offset and index to be identical"
                    );
            }
        }
        // a UTF-8 region
        else
        {
            // must have offset at calculated place
            size_t offset = char_idx_to_byte_off ( a, sz, loc . index () );
            if ( loc . offset () != offset )
            {
                throw InvalidStateException (
                    XP ( XLOC )
                    << "CharLoc ( "
                    << loc . offset ()
                    << ", "
                    << loc . index ()
                    << " ) expects offset at "
                    << offset
                    );
            }
        }
    }
#endif

    /**
     * getChar
     *  @return fully-indexed UNICODE character
     */
    UTF32 Text :: getChar ( const CharLoc & pos ) const
    {
#if TESTING
        sanity_check ( pos, sz, cnt, getAddr () );
#endif
        if ( pos . index () < 0 || ( count_t ) pos . index () >= cnt )
        {
            throw BoundsException (
                XP ( XLOC )
                << "character index ( "
                << pos . index ()
                << " ) is out of bounds of text region [0, "
                << cnt
                << ")"
                );
        }

        // rather than accessing the member variable directly,
        // which is stored as a non-const pointer,
        // use the accessor that will return a const pointer
        const UTF8 * data = getAddr ();

        // if the region is 100% ASCII, just retrieve it directly
        if ( self . isAscii () )
        {
            assert ( pos . offset () == ( size_t ) pos . index () );
            return data [ pos . index () ];
        }

        // no exception, so the offset should be good
        assert ( pos . offset () < sz );
        return utf8_to_utf32 ( data, sz, pos . offset () ) . ch;
    }
    

    /**
     * getLoc
     *  @return CharLoc of given index
     */
    CharLoc Text :: getLoc ( index_t idx ) const
    {
        if ( idx <= 0 )
            return CharLoc ( 0, idx );

        if ( ( count_t ) idx >= cnt )
            return CharLoc ( sz, idx );

        if ( idx == 0 || self . isAscii () )
            return CharLoc ( ( size_t ) idx, idx );
        
        size_t offset = char_idx_to_byte_off ( getAddr (), sz, idx );
        return CharLoc ( offset, idx );
    }

    /**
     * nextLoc
     *  @return location advanced by 1 character
     */
    CharLoc Text :: nextLoc ( const CharLoc & pos ) const
    {
#if TESTING
        sanity_check ( pos, sz, cnt, getAddr () );
#endif
        // incrementing an out of bounds guy leaves offset intact
        if ( pos . index () < 0 || ( count_t ) pos . index () >= self . cnt )
            return CharLoc ( pos . offset (), pos . index () + 1 );

        // incrementing an ASCII location is relatively simple
        if ( isAscii () )
            return CharLoc ( pos . offset () + 1, pos . index () + 1 );

        // scan over UTF-8 character at pos
        size_t delta = scan_fwd ( getAddr (), sz, pos . offset () );

        // the next character will be at this position
        CharLoc next ( pos . offset () + delta, pos . index () + 1 );
#if TESTING
        sanity_check ( next, sz, cnt, getAddr () );
#endif
        return next;
    }

    /**
     * prevLoc
     *  @return location rewound by 1 character
     */
    CharLoc Text :: prevLoc ( const CharLoc & pos ) const
    {
#if TESTING
        sanity_check ( pos, sz, cnt, getAddr () );
#endif
        // decrementing an out of bounds guy leaves offset intact
        if ( pos . index () <= 0 || ( count_t ) pos . index () > self . cnt )
            return CharLoc ( pos . offset (), pos . index () - 1 );

        // decrementing an ASCII location is simple
        if ( isAscii () )
            return CharLoc ( pos . offset () - 1, pos . index () - 1 );

        // scan in reverse to find start of prior character
        size_t delta = scan_rev ( getAddr (), sz, pos . offset () );

        // the previous character will be at this position
        assert ( delta <= pos . offset () );
        CharLoc next ( pos . offset () - delta, pos . index () - 1 );
#if TESTING
        sanity_check ( next, sz, cnt, getAddr () );
#endif
        return next;
    }

    /**
     * equal
     *  @brief test two text regions for equality
     *  @param r a text region to compare against self
     *  @return true iff the two regions have equal content
     */
    bool Text :: equal ( const Text & r ) const noexcept
    {
        return self . count () == r . count () &&
            self . toCRgn () . equal ( r . toCRgn () );
    }

    /**
     * compare
     *  @brief compare self against another region
     *  @r text region to compare against self
     *  @return "self" - "r" => { < Z-, 0, Z+ }
     */
    int Text :: compare ( const Text & r ) const noexcept
    {
        count_t min_len = r . cnt;
        if ( cnt < min_len )
            min_len = cnt;
        
        if ( isAscii () && r . isAscii () )
            return self . toCRgn () . compare ( r . toCRgn () );

        const UTF8 * p1 = getAddr ();
        const UTF8 * p2 = r . getAddr ();

        size_t off1 = 0;
        size_t off2 = 0;

        for ( count_t i = 0; i < min_len; ++ i )
        {
            UniChar ch1 = utf8_to_utf32 ( p1, sz, off1 );
            UniChar ch2 = utf8_to_utf32 ( p2, r . sz, off2 );
            if ( ch1 . ch != ch2 . ch )
                return ( ch1 . ch < ch2 . ch ) ? -1 : 1;
            off1 += ch1 . size;
            off2 += ch2 . size;
        }
        
        if ( cnt < r . cnt )
            return -1;
        return cnt > r . cnt;
    }

    /**
     * caseInsensitiveCompare
     *  @brief compare self against another region disregarding case
     *  @r text region to compare against self
     *  @return tolower ( "self" ) - tolower ( "r" ) => { < Z-, 0, Z+ }
     */
    int Text :: caseInsensitiveCompare ( const Text & r ) const noexcept
    {
        count_t min_len = r . cnt;
        if ( cnt < min_len )
            min_len = cnt;
        
        if ( isAscii () && r . isAscii () )
        {
            const ASCII * p1 = getAddr ();
            const ASCII * p2 = r . getAddr ();

            for ( count_t i = 0; i < min_len; ++ i )
            {
                int diff = p1 [ i ] - p2 [ i ];
                if ( diff != 0 )
                {
                    diff = :: tolower ( p1 [ i ] ) - :: tolower ( p2 [ i ] );
                    if ( diff != 0 )
                        return ( diff < 0 ) ? -1 : 1;
                }
            }
        }
        else
        {
            const UTF8 * p1 = getAddr ();
            const UTF8 * p2 = r . getAddr ();

            size_t off1 = 0;
            size_t off2 = 0;

            for ( count_t i = 0; i < min_len; ++ i )
            {
                UniChar ch1 = utf8_to_utf32 ( p1, sz, off1 );
                UniChar ch2 = utf8_to_utf32 ( p2, r . sz, off2 );
                if ( ch1 . ch != ch2 . ch )
                {
                    ch1 . ch = :: towlower ( ch1 . ch );
                    ch2 . ch = :: towlower ( ch2 . ch );
                    if ( ch1 . ch != ch2 . ch )
                        return ( ch1 . ch < ch2 . ch ) ? -1 : 1;
                }
                off1 += ch1 . size;
                off2 += ch2 . size;
            }
        }
        
        if ( cnt < r . cnt )
            return -1;
        return cnt > r . cnt;
    }

    /**
     * fwdFind
     * @overload forward search to find a sub-region
     * @param sub the sub-region being sought
     * @return the location of start of sub-region if found, otherwise end of region
     */
    CharLoc Text :: fwdFind ( const Text & sub ) const noexcept
    {
        if ( self . count () < sub . count () )
            return CharLoc ();
        size_t off = self . toCRgn () . fwdFind ( sub . toCRgn () );
        if ( off == self . size () )
            return CharLoc ();
        if ( off == 0 || self . isAscii () )
            return CharLoc ( off, ( index_t ) off );
        return CharLoc ( off, region_length ( getAddr (), off ) );
    }

    /**
     * fwdFind
     * @overload forward search to find a single character
     * @param ch a UNICODE character
     * @return the location of start of sub-region if found, otherwise end of region
     */
    CharLoc Text :: fwdFind ( UTF32 ch ) const noexcept
    {
        if ( ! is_ascii ( ch ) )
        {
            UTF8 chstr [ 4 ];
            size_t chsz = utf32_to_utf8_strict ( chstr, sizeof chstr, ch );
            return fwdFind ( Text ( chstr, chsz, 1 ) );
        }
        
        if ( self . count () == 0 )
            return CharLoc ();
        size_t off = self . toCRgn () . fwdFind ( ( byte_t ) ch );
        if ( off == self . size () )
            return CharLoc ();
        if ( off == 0 || self . isAscii () )
            return CharLoc ( off, ( count_t ) off );
        return CharLoc ( off, region_length ( getAddr (), off ) );
    }

    /**
     * revFind
     * @overload reverse search to find a sub-region
     * @param sub the sub-region being sought
     * @return the location of start of sub-region if found, otherwise end of region
     */
    CharLoc Text :: revFind ( const Text & sub ) const noexcept
    {
        if ( self . count () < sub . count () )
            return CharLoc ();
        size_t off = self . toCRgn () . revFind ( sub . toCRgn () );
        if ( off == self . size () )
            return CharLoc ();
        if ( off == 0 || self . isAscii () )
            return CharLoc ( off, ( count_t ) off );
        return CharLoc ( off, region_length ( getAddr (), off ) );
    }

    /**
     * revFind
     * @overload reverse search to find a single character
     * @param ch a UNICODE character
     * @return the location of start of sub-region if found, otherwise end of region
     */
    CharLoc Text :: revFind ( UTF32 ch ) const noexcept
    {
        if ( ! is_ascii ( ch ) )
        {
            UTF8 chstr [ 4 ];
            size_t chsz = utf32_to_utf8_strict ( chstr, sizeof chstr, ch );
            return revFind ( Text ( chstr, chsz, 1 ) );
        }
        
        if ( self . count () == 0 )
            return CharLoc ();
        size_t off = self . toCRgn () . revFind ( ( byte_t ) ch );
        if ( off == self . size () )
            return CharLoc ();
        if ( off == 0 || self . isAscii () )
            return CharLoc ( off, ( count_t ) off );
        return CharLoc ( off, region_length ( getAddr (), off ) );
    }
    
    /**
     * findFirstOf
     * @overload forward search to find a character of a set
     * @param cset the set of characters being sought
     * @return the location of character if found, otherwise end of region
     */
    CharLoc Text :: findFirstOf ( const Text & cset ) const noexcept
    {
        if ( self . count () == 0 )
            return CharLoc ();

        if ( cset . isAscii () )
        {
            size_t off = self . toCRgn () . findFirstOf ( cset . toCRgn () );
            if ( off == self . size () )
                return CharLoc ();
            if ( off == 0 || self . isAscii () )
                return CharLoc ( off, ( count_t ) off );
            return CharLoc ( off, region_length ( getAddr (), off ) );
        }
            
        const UTF8 * p1 = self . getAddr ();
        const UTF8 * p2 = cset . getAddr ();

        size_t off1 = 0;
        for ( count_t i = 0; i < cnt; ++ i )
        {
            UniChar ch1 = utf8_to_utf32 ( p1, sz, off1 );
            
            size_t off2 = 0;
            for ( count_t j = 0; j < cset . cnt; ++ j )
            {
                UniChar ch2 = utf8_to_utf32 ( p2, cset . sz, off2 );
                if ( ch1 . ch == ch2 . ch )
                    return CharLoc ( off1, i );
                off2 += ch2 . size;
            }
            
            off1 += ch1 . size;
        }

        return CharLoc ();
    }
    
    /**
     * findLastOf
     * @overload reverse search to find a character of a set
     * @param cset the set of characters being sought
     * @return the location of character if found, otherwise end of region
     */
    CharLoc Text :: findLastOf ( const Text & cset ) const noexcept
    {
        if ( self . count () == 0 )
            return CharLoc ();

        if ( cset . isAscii () )
        {
            size_t off = self . toCRgn () . findLastOf ( cset . toCRgn () );
            if ( off == self . size () )
                return CharLoc ();
            if ( off == 0 || self . isAscii () )
                return CharLoc ( off, ( count_t ) off );
            return CharLoc ( off, region_length ( getAddr (), off ) );
        }

        // roll out the code that steps across UTF8
        const UTF8 * p1 = self . getAddr ();
        const UTF8 * p2 = cset . getAddr ();

        size_t off1 = sz;
        for ( count_t i = cnt; i != 0; )
        {
            -- i;
            off1 -= scan_rev ( p1, sz, off1 );
            UniChar ch1 = utf8_to_utf32 ( p1, sz, off1 );
            
            size_t off2 = 0;
            for ( count_t j = 0; j < cset . cnt; ++ j )
            {
                UniChar ch2 = utf8_to_utf32 ( p2, cset . sz, off2 );
                if ( ch1 . ch == ch2 . ch )
                    return CharLoc ( off1, i );
                off2 += ch2 . size;
            }
        }

        return CharLoc ();
    }

    /**
     * beginsWith
     * @overload test whether self region begins with sub region
     * @param sub region with subsequence in question
     * @return Boolean true if self begins with "sub"
     */
    bool Text :: beginsWith ( const Text & sub ) const noexcept
    {
        return self . count () >= sub . count () &&
            self . toCRgn () . beginsWith ( sub . toCRgn () );
    }

    /**
     * beginsWith
     * @overload test whether self region begins with character
     * @param ch UTF32 with character in question
     * @return Boolean true if self begins with "ch"
     */
    bool Text :: beginsWith ( UTF32 ch ) const noexcept
    {
        if ( ! is_ascii ( ch ) )
        {
            UTF8 chstr [ 4 ];
            size_t chsz = utf32_to_utf8_strict ( chstr, sizeof chstr, ch );
            return beginsWith ( Text ( chstr, chsz, 1 ) );
        }
        
        return self . count () >= 1 && self . toCRgn () . beginsWith ( ( byte_t ) ch );
    }

    /**
     * endsWith
     * @overload test whether self region ends with sub region
     * @param sub region with subsequence in question
     * @return Boolean true if self ends with "sub"
     */
    bool Text :: endsWith ( const Text & sub ) const noexcept
    {
        return self . count () >= sub . count () &&
            self . toCRgn () . endsWith ( sub . toCRgn () );
    }

    /**
     * endsWith
     * @overload test whether self region ends with character
     * @param ch UTF32 with character in question
     * @return Boolean true if self ends with "ch"
     */
    bool Text :: endsWith ( UTF32 ch ) const noexcept
    {
        if ( ! is_ascii ( ch ) )
        {
            UTF8 chstr [ 4 ];
            size_t chsz = utf32_to_utf8_strict ( chstr, sizeof chstr, ch );
            return endsWith ( Text ( chstr, chsz, 1 ) );
        }
        
        return self . count () >= 1 && self . toCRgn () . endsWith ( ( byte_t ) ch );
    }
    

    /*=====================================================*
     *                        MText                        *
     *=====================================================*/

    /**
     * setChar
     * @overload attempt to set the value of a character
     * @param idx is the index of the character to set
     * @param ch is the new character value
     * @return self
     */
    MText & MText :: setChar ( index_t idx, UTF32 ch )
    {
        // transform index into CharLoc
        return setChar ( self . getLoc ( idx ), ch );
    }

    /**
     * setChar
     * @overload attempt to set the value of a character
     * @param pos is the location of the character to set
     * @param ch is the new character value
     * @return self
     */
    MText & MText :: setChar ( const CharLoc & pos, UTF32 ch )
    {
        // first question - is "pos" valid?
        if ( ! self . contains ( pos ) )
        {
            throw BoundsException (
                XP ( XLOC )
                << "character index ( "
                << pos . index ()
                << " ) is out of bounds of text region [0, "
                << cnt
                << ")"
                );
        }

        // handle all ASCII case
        if ( self . isAscii () && is_ascii ( ch ) )
        {
            self . a [ pos . index () ] = ( ASCII ) ch;
            return self;
        }

        // get full MRgn from pos
        MRgn mrgn = self . toMRgn () . subRgn ( pos . offset () );

        // need to know the size of existing character
        size_t existing_size = scan_fwd ( self . a, self . sz, pos . offset () );

        // and the size of its replacement
        Char replacement ( ch );

        // in the highly likely case that the sizes will match,
        // the replacement is simple as it was above
        if ( existing_size == replacement . size () )
        {
            // copy UTF-8 bytes 1-1
            mrgn . copy ( replacement . toCRgn () );
        }
        else
        {

            // get the valid MRgn from pos
            MRgn vrgn = self . toValidMRgn () . subRgn ( pos . offset () );

            // in the event that the new character is smaller
            if ( existing_size > replacement . size () )
            {
                // copy UTF-8 bytes, leaving a hole with garbage
                mrgn . copy ( replacement . toCRgn () );

                // close the hole, leaving trailing garbage
                mrgn . subRgn ( pos . offset () + replacement . size () )
                    . copy ( vrgn . subRgn ( pos . offset () + existing_size ) );

                // update size - count has not changed
                self . sz -= existing_size - replacement . size ();

                // clear the tail
                assert ( self . sz >= pos . offset () );
                vrgn . subRgn ( self . sz - pos . offset () ) . fill ( 0 );
            }
            else if ( self . sz + replacement . size () - existing_size > self . cap )
            {
                throw InsufficientBuffer (
                    XP ( XLOC )
                    << xprob
                    << "failed to set character value"
                    << xcause
                    << "capacity of mutable region exhausted"
                    );
            }
            else
            {
                // update size - count has not changed
                self . sz += replacement . size () - existing_size;

                // open the hole
                mrgn . subRgn ( replacement . size () )
                    . copy ( vrgn . subRgn ( existing_size ) );
                
                // copy UTF-8 bytes, filling hole
                mrgn . copy ( replacement . toCRgn () );
            }
        }

        return self;
    }

    /**
     * insertChar
     * @overload attempt to insert a character before indicated one
     * @param idx is the index of the existing character
     * @param ch is the new character value
     * @return self
     */
    MText & MText :: insertChar ( index_t idx, UTF32 ch )
    {
        // transform index into CharLoc
        return insertChar ( self . getLoc ( idx ), ch );
    }

    /**
     * insertChar
     * @overload attempt to insert a character before indicated one
     * @param pos is the location of the character to set
     * @param ch is the new character value
     */
    MText & MText :: insertChar ( const CharLoc & pos, UTF32 ch )
    {
        // first question - is "pos" valid?
        if ( ! self . contains ( pos ) )
        {
            throw BoundsException (
                XP ( XLOC )
                << "character index ( "
                << pos . index ()
                << " ) is out of bounds of text region [0, "
                << cnt
                << ")"
                );
        }

        // get full MRgn from pos
        MRgn mrgn = self . toMRgn () . subRgn ( pos . offset () );

        // the character to insert
        Char insertion ( ch );

        // test for capacity
        if ( self . sz + insertion . size () > self . cap )
        {
            throw InsufficientBuffer (
                XP ( XLOC )
                << xprob
                << "failed to insert character value"
                << xcause
                << "capacity of mutable region exhausted"
                );
        }
        
        // get the valid MRgn from pos
        MRgn vrgn = self . toValidMRgn () . subRgn ( pos . offset () );

        // update size
        self . sz += insertion . size ();

        // open the hole
        mrgn . subRgn ( insertion . size () ) . copy ( vrgn );
                
        // copy UTF-8 bytes, filling hole
        mrgn . copy ( insertion . toCRgn () );

        // update the character count
        self . cnt += 1;

        return self;
    }

    /**
     * appendChar
     * @overload attempt to add a new character to end of valid region
     * @param ch is the new character value
     * @return self
     */
    MText & MText :: appendChar ( UTF32 ch )
    {
        // get full MRgn from valid end - the free space
        MRgn mrgn = self . toMRgn () . subRgn ( self . sz );

        // the character to append
        Char appendage ( ch );

        // copy UTF-8 bytes, potentially extending valid region
        MRgn cpy = mrgn . copy ( appendage . toCRgn () );

        // fail if < all bytes were copied
        if ( cpy . size () != appendage . size () )
        {
            throw InsufficientBuffer (
                XP ( XLOC )
                << xprob
                << "failed to append character value"
                << xcause
                << "capacity of mutable region exhausted"
                );
        }

        // update
        self . sz += appendage . size ();
        self . cnt += 1;

        return self;
    }

    /**
     * removeChar
     * @overload attempt to remove an existing character
     * @param idx is the index of the character to remove
     */
    UTF32 MText :: removeChar ( index_t idx )
    {
        // transform index into CharLoc
        return removeChar ( self . getLoc ( idx ) );
    }

    /**
     * removeChar
     * @overload attempt to remove an existing character
     * @param pos is the location of the character to remove
     */
    UTF32 MText :: removeChar ( const CharLoc & pos )
    {
        // first question - is "pos" valid?
        if ( ! self . contains ( pos ) )
        {
            throw BoundsException (
                XP ( XLOC )
                << "character index ( "
                << pos . index ()
                << " ) is out of bounds of text region [0, "
                << cnt
                << ")"
                );
        }

        // get full MRgn from pos
        MRgn mrgn = self . toMRgn () . subRgn ( pos . offset () );

        // retrieve existing character
        UniChar uch = utf8_to_utf32 ( self . a, self . sz, pos . offset () );

        // close hole
        mrgn . copy ( mrgn . subRgn ( uch . size ) );

        // update counts
        self . sz -= uch . size;
        self . cnt -= 1;

        // clear garbage
        mrgn . subRgn ( self . sz, uch . size ) . fill ( 0 );

        // return the UTF32 character
        return uch . ch;
    }

    /**
     * copy
     *  @brief copy bytes from a source rgn
     *  @return self
     */
    MText & MText :: copy ( const CText & src )
    {
        // get full MRgn
        MRgn mrgn = self . toMRgn ();

        // copy bytes
        MRgn cpy = mrgn . copy ( src . toCRgn () );

        // full copy
        if ( cpy . size () == src . size () )
        {
            // detect garbage
            if ( self . sz > src . size () )
                self . toValidMRgn () . subRgn ( src . size () ) . fill ( 0 );

            // update counts
            self . sz = src . size ();
            self . cnt = src . count ();
            
            return self;
        }
        
        // update size
        self . sz = cpy . size ();

        // partial copy, but all ASCII
        if ( src . isAscii () )
        {
            // update count
            self . cnt = ( count_t ) self . sz;
            
            return self;
        }

        // detect a split UTF-8 character
        assert ( self . sz < src . size () );
        if ( ( src . addr () [ self. sz ] & 0xC0 ) == 0x80 )
        {
            // back over split character
            while ( self . sz != 0 )
            {
                // break when "sz" finds a leading byte
                if ( ( self . a [ -- self . sz ] & 0xC0 ) != 0x80 )
                    break;

                // forget the byte
                self . a [ self . sz ] = 0;
            }
        }

        // discount the portion we're going to replace
        // use the smallest region to measure
        if ( self . sz > src . size () / 2 )
        {
            // measure the part of src that didn't get copied
            // remove it from its overall count
            self . cnt = src . count () -
                region_length ( src . addr () + self . sz, src . size () - self . sz );
        }
        else
        {
            // measure the part that was copied
            self . cnt -= region_length ( self . a, self . sz );
        }

        return self;
    }

    /**
     * append
     *  @brief append text from a source rgn
     *  @return self
     */
    MText & MText :: append ( const CText & src )
    {
        MText cpy = self . subRgn ( self . cnt ) . copy ( src );
        self . sz += cpy . size ();
        self . cnt += cpy . count ();

        return self;
    }
        
    /**
     * fill
     *  @brief set all elements of region to single value
     *  @param val is fill value
     */
    MText & MText :: fill ( ASCII val, count_t repeat )
    {
        if ( ! is_ascii ( val ) || ( val != 0 && ! :: isprint ( ( int ) val ) ) )
        {
            throw InvalidASCII (
                XP ( XLOC )
                << xprob
                << "failed to fill text region"
                << xcause
                << "illegal fill character"
                );
        }
        
        MRgn mrgn = toMRgn ();
        if ( repeat != 0 )
            mrgn = mrgn . subRgn ( 0, ( size_t ) repeat );

        mrgn . fill ( ( byte_t ) val );

        if ( val == 0 )
        {
            sz = 0;
            cnt = 0;
        }
        else
        {
            sz = mrgn . size ();
            cnt = ( count_t ) mrgn . size ();;
        }

        return self;
    }

    /**
     * clear
     *  @brief forget all contents
     *  @param zero if true, fill valid content with zero first
     *  @return self
     */
    MText & MText :: clear ( bool zero ) noexcept
    {
        if ( zero )
            self . toValidMRgn () . fill ( 0 );

        sz = 0;
        cnt = 0;

        return self;
    }


    /**
     * format
     * @brief prints into text region using vsnprintf
     */
    MText & MText :: format ( const UTF8 * fmt, ... )
    {
        va_list args;
        va_start ( args, fmt );
        
        vformat ( fmt, args );
        
        va_end ( args );
        
        return self;
    }

    /**
     * vformat
     * @brief prints into text region using vsnprintf
     */
    MText & MText :: vformat ( const UTF8 * fmt, va_list args )
    {
        if ( fmt == nullptr )
        {
            throw NullArgumentException (
                XP ( XLOC )
                << xprob
                << "failed to print into text region"
                << xcause
                << "printf format string is null"
                );
        }

        int status = :: vsnprintf ( self . a, self . cap, fmt, args );
        if ( status < 0 )
        {
            status = errno;
            
            fill ( 0 );

            throw RuntimeException (
                XP ( XLOC )
                << xprob
                << "failed to print into MText region"
                << xcause
                << syserr ( status )
                );
        }

        if ( ( size_t ) status >= cap )
        {
            fill ( 0 );

            throw LogicException (
                XP ( XLOC )
                << xprob
                << "failed to print into MText region"
                << xcause
                << "insufficient buffer"
                );
        }

        // zero fill anything there before that wasn't overwritten
        if ( self . sz > ( size_t ) status )
            self . toValidMRgn () . subRgn ( status ) . fill ( 0 );

        // update size and character count
        self . sz = status;
        self . cnt = region_length ( self . a, self . sz );

        return self;
    }
        
    /**
     * tolower
     *  @brief lower-case all characters
     *  @return self
     */
    MText & MText :: tolower ()
    {
        if ( self . isAscii () )
        {
            for ( index_t i = 0; ( count_t ) i < cnt; ++ i )
            {
                // read character
                int ch = a [ i ];

                // generate lower-case
                int lch = :: tolower ( ch );
#if TOLOWER_USE_IF
                // only modify if needed
                if ( ch != lch )
#endif
                    // update
                    a [ i ] = lch;
            }
        }
        else
        {
            for ( size_t i = 0; i < sz; )
            {
                // read character
                UniChar ch = utf8_to_utf32 ( a, sz, i );

                // generate lower-case
                UTF32 lch = :: towlower ( ch . ch );

                // only modify if needed
                if ( lch != ch . ch )
                {
                    try
                    {
                        // just try overwriting into existing space
                        size_t lsize = utf32_to_utf8 ( a + i, ch . size, lch );
                        if ( lsize != ch . size )
                        {
                            // didn't blow an exception but sizes don't match
                            // lower-case is smaller than upper!
                            utf32_to_utf8 ( a + i, ch . size, ch . ch );

                            // throw exception
                            throw UTF8ConversionError (
                                XP ( XLOC, rc_logic_err )
                                << "size of lower-case '"
                                << Char ( lch )
                                << "' ( "
                                << lsize
                                << " ) differs from upper case '"
                                << Char ( ch . ch )
                                << "' ( "
                                << ch . size
                                << " )"
                                );
                        }
                    }

                    // lower-case is larger than upper!
                    catch ( InsufficientBuffer & x )
                    {
                        // measure size of lower
                        char ignore [ 8 ];
                        size_t lsize = utf32_to_utf8 ( ignore, sizeof ignore, lch );

                        // throw exception
                        throw UTF8ConversionError (
                            XP ( XLOC, rc_logic_err )
                            << "size of lower-case '"
                            << Char ( lch )
                            << "' ( "
                            << lsize
                            << " ) differs from upper case '"
                            << Char ( ch . ch )
                            << "' ( "
                            << ch . size
                            << " )"
                            );
                    }
                }

                // advance to next character
                i += ch . size;
            }
        }

        return self;
    }
        
    /**
     * toupper
     *  @brief upper-case all characters
     */
    MText & MText :: toupper ()
    {
        if ( self . isAscii () )
        {
            // which is faster - the if or always rewriting and blowing cache?
            for ( index_t i = 0; ( count_t ) i < cnt; ++ i )
            {
                int ch = a [ i ];
                int uch = :: toupper ( ch );
#if TOLOWER_USE_IF
                if ( ch != uch )
#endif
                    a [ i ] = uch;
            }
        }
        else
        {
            for ( size_t i = 0; i < sz; )
            {
                UniChar ch = utf8_to_utf32 ( a, sz, i );
                UTF32 uch = :: towupper ( ch . ch );
                if ( uch != ch . ch )
                {
                    try
                    {
                        size_t usize = utf32_to_utf8 ( a + i, ch . size, uch );
                        if ( usize != ch . size )
                        {
                            utf32_to_utf8 ( a + i, ch . size, ch . ch );
                            throw UTF8ConversionError (
                                XP ( XLOC, rc_logic_err )
                                << "size of upper-case '"
                                << Char ( uch )
                                << "' ( "
                                << usize
                                << " ) differs from lower case '"
                                << Char ( ch . ch )
                                << "' ( "
                                << ch . size
                                << " )"
                                );
                        }
                    }
                    catch ( InsufficientBuffer & x )
                    {
                        char ignore [ 8 ];
                        size_t usize = utf32_to_utf8 ( ignore, sizeof ignore, uch );
                        throw UTF8ConversionError (
                            XP ( XLOC, rc_logic_err )
                            << "size of upper-case '"
                            << Char ( uch )
                            << "' ( "
                            << usize
                            << " ) differs from lower case '"
                            << Char ( ch . ch )
                            << "' ( "
                            << ch . size
                            << " )"
                            );
                    }
                }
                
                i += ch . size;
            }
        }

        return self;
    }

    /**
     * subRgn
     *  @overload create a new mutable region that is a subset of original
     *  @param start the index of the starting character
     *  @return MText from start to end
     */
    MText MText :: subRgn ( index_t start ) const
    {
        // detect whole thing
        if ( start == 0 )
            return MText ( self );

        // detect out of bounds
        if ( start < 0 )
            return MText ( a, 0, 0 );
        if ( ( count_t ) start >= cnt )
        {
            // special case of being exactly at end
            // preserve the capacity
            if ( ( count_t ) start == cnt )
                return MText ( a + sz, cap - sz, 0, 0 );

            // return an empty region with zero capacity
            return MText ( a + sz, 0, 0 );
        }

        // detect ASCII optimization
        if ( self . isAscii () )
            return MText ( a + start, cap - start, sz - start, cnt - start );

        // locate the byte offset to the start element
        size_t off = char_idx_to_byte_off ( a, sz, start );
        assert ( 0 < off && off < sz );

        // create a remainder region
        return MText ( a + off, cap - off, sz - off, cnt - start );
    }

    /**
     * subRgn
     *  @overload create a new mutable region that is a subset of original
     *  @param start the index of the starting character
     *  @param len the maximum number of characters to include
     *  @return MText from start to min ( start + len, end )
     */
    MText MText :: subRgn ( index_t start, count_t len ) const
    {
        // perform initial work on left edge
        MText sub = self . subRgn ( start );

        // if "len" is small enough to affect right edge
        if ( sub . cnt > len )
        {
            // special case zero length
            if ( len == 0 )
                sub . sz = 0;
            
            // special case ASCII
            else if ( sub . isAscii () )
                sub . sz = ( size_t ) len;
            
            // handle UTF-8
            else
                sub . sz = char_idx_to_byte_off ( sub . a, sub . sz, len );

            // set length
            sub . cnt = len;

            // capacity is limited to size
            sub . cap = sub . sz;
        }

        return sub;
    }

    /**
     * subRgn
     *  @overload create a new mutable region that is a subset of original
     *  @param start the index of the starting character
     *  @param xend exclusive ending location
     *  @return MText from start to min ( xend, end )
     */
    MText MText :: subRgn ( index_t start, const CharLoc & xend ) const
    {
        // convert start to CharLoc
        return self . subRgn ( getLoc ( start ), xend );
    }

    /**
     * subRgn
     *  @overload create a new mutable region that is a subset of original
     *  @param start the location of the starting character
     *  @return MText from start to end
     */
    MText MText :: subRgn ( const CharLoc & start ) const
    {
#if TESTING
        sanity_check ( start, sz, cnt, a );
#endif
        // detect out of bounds
        if ( start < 0 )
            return MText ( a, 0, 0 );
        if ( start >= ( index_t ) cnt )
        {
            if ( start == ( index_t ) cnt )
                return MText ( a + sz, cap - sz, 0, 0 );
            
            return MText ( a + sz, 0, 0 );
        }

        // create region
        size_t off = start . offset ();
        return MText ( a + off, cap - off, sz - off, cnt - start . index () );
    }
    
    /**
     * subRgn
     *  @overload create a new mutable region that is a subset of original
     *  @param start the location of the starting character
     *  @param len the maximum number of characters to include
     *  @return MText from start to min ( start + len, end )
     */
    MText MText :: subRgn ( const CharLoc & start, count_t len ) const
    {
        // perform initial work on left edge
        MText sub = self . subRgn ( start );

        // if "len" is small enough to affect right edge
        if ( sub . cnt > len )
        {
            // special case zero length
            if ( len == 0 )
                sub . sz = 0;
            
            // special case ASCII
            else if ( sub . isAscii () )
                sub . sz = ( size_t ) len;
            
            // handle UTF-8
            else
                sub . sz = char_idx_to_byte_off ( sub . a, sub . sz, len );

            // set length
            sub . cnt = len;

            // capacity is limited to size
            sub . cap = sub . sz;
        }

        return sub;
    }

    /**
     * subRgn
     *  @overload create a new mutable region that is a subset of original
     *  @param start the location of the starting character
     *  @param xend exclusive ending location
     *  @return MText from start to min ( xend, end )
     */
    MText MText :: subRgn ( const CharLoc & start, const CharLoc & xend ) const
    {
#if TESTING
        sanity_check ( start, sz, cnt, a );
        sanity_check ( xend, sz, cnt, a );
#endif
        // detect out of bounds
        if ( start < 0 )
            return MText ( a, 0, 0 );
        if ( start >= ( index_t ) cnt )
        {
            // preserve capacity IFF xend is at end
            if ( start == ( index_t ) cnt && start == xend )
                return MText ( a + sz, cap - sz, 0, 0 );
            
            return MText ( a + sz, 0, 0 );
        }
        
        size_t off = start . offset ();

        // an empty region
        if ( start >= xend )
            return MText ( a + off, 0, 0 );

        // xend beyond end of region
        if ( xend >= end () )
            return MText ( a + off, cap - off, sz - off, cnt - start . index () );

        // legitimate range
        return MText ( a + off, xend . offset () - off, xend . index () - start . index () );
    }

    
    /**
     * intersect
     *  @return intersection between regions
     */
    MText MText :: intersect ( const MText & r ) const noexcept
    {
        // intersect the regions in memory
        MRgn isect = self . toValidMRgn () . intersect ( r . toValidMRgn () );

        // measure in characters
        UTF8 * start = ( UTF8 * ) isect . addr ();
        UTF8 * xend = start + isect . size ();
        count_t len = region_length ( start, isect . size () );

        // intersect regions of free space
        MRgn lfRgn = self . toMRgn () . subRgn ( self . sz );
        MRgn rfRgn = r . toMRgn () . subRgn ( r . sz );
        MRgn fisect = lfRgn . intersect (  rfRgn );

        // if the valid intersection connects to free space
        if ( xend == ( UTF8 * ) fisect . addr () )
            return MText ( start, isect . size () + fisect . size (), isect . size (), len );

        // mutable region but with no free space
        return MText ( start, isect . size (), len );
    }

    /**
     * join
     *  @return union of regions
     */
    MText MText :: join ( const MText & r ) const
    {
        // join the regions in memory
        // cannot join two disjoint regions, may blow...
        MRgn lRgn = self . toMRgn ();
        MRgn rRgn = r . toMRgn ();
        MRgn total = lRgn . join ( rRgn );

        // now join the valid portions
        MRgn lvRgn = self . toValidMRgn ();
        MRgn rvRgn = r . toValidMRgn ();
        MRgn u = lvRgn . join ( rvRgn );

        assert ( total . addr () == u . addr () );
        assert ( total . size () >= u . size () );

        // measure in characters and return union
        UTF8 * start = ( UTF8 * ) u . addr ();
        count_t len = region_length ( start, u . size () );

        // return region with free space
        return MText ( start, total . size (), u . size (), len );
    }

    
    /*=====================================================*
     *                        CText                        *
     *=====================================================*/

    /**
     * subRgn
     *  @overload create a new region that is a subset of original
     *  @param start the index of the starting character
     *  @return CText from start to end
     */
    CText CText :: subRgn ( index_t start ) const
    {
        // detect whole thing
        if ( start == 0 )
            return CText ( self );
        
        // detect out of bounds
        if ( start < 0 )
            return CText ( a, 0, 0 );
        if ( ( count_t ) start >= cnt )
            return CText ( a + sz, 0, 0 );

        // detect ASCII optimization
        if ( self . isAscii () )
            return CText ( a + start, sz - start, cnt - start );

        // locate the byte offset to the start element
        size_t off = char_idx_to_byte_off ( a, sz, start );
        assert ( 0 < off && off < sz );

        // create a remainder region
        return CText ( a + off, sz - off, cnt - start );
    }

    /**
     * subRgn
     *  @overload create a new region that is a subset of original
     *  @param start the index of the starting character
     *  @param len the maximum number of characters to include
     *  @return CText from start to min ( start + len, end )
     */
    CText CText :: subRgn ( index_t start, count_t len ) const
    {
        // perform initial work on left edge
        CText sub = self . subRgn ( start );

        // if "len" is small enough to affect right edge
        if ( sub . cnt > len )
        {
            // special case zero length
            if ( len == 0 )
                sub . sz = 0;
            
            // special case ASCII
            else if ( self . isAscii () )
                sub . sz = ( size_t ) len;
            
            // handle UTF-8
            else
                sub . sz = char_idx_to_byte_off ( sub . a, sub . sz, len );

            // set length
            sub . cnt = len;
        }

        return sub;
    }

    /**
     * subRgn
     *  @overload create a new region that is a subset of original
     *  @param start the index of the starting character
     *  @param xend exclusive ending location
     *  @return CText from start to min ( xend, end )
     */
    CText CText :: subRgn ( index_t start, const CharLoc & xend ) const
    {
        // convert start to CharLoc
        return self . subRgn ( getLoc ( start ), xend );
    }

    /**
     * subRgn
     *  @overload create a new region that is a subset of original
     *  @param start the location of the starting character
     *  @return CText from start to end
     */
    CText CText :: subRgn ( const CharLoc & start ) const
    {
#if TESTING
        sanity_check ( start, sz, cnt, a );
#endif
        // detect out of bounds
        if ( start < 0 )
            return CText ( a, 0, 0 );
        if ( start >= ( index_t ) cnt )
            return CText ( a + sz, 0, 0 );

        // create region
        size_t off = start . offset ();
        return CText ( a + off, sz - off, cnt - start . index () );
    }

    /**
     * subRgn
     *  @overload create a new region that is a subset of original
     *  @param start the location of the starting character
     *  @param len the maximum number of characters to include
     *  @return CText from start to min ( start + len, end )
     */
    CText CText :: subRgn ( const CharLoc & start, count_t len ) const
    {
        // perform initial work on left edge
        CText sub = self . subRgn ( start );

        // if "len" is small enough to affect right edge
        if ( sub . cnt > len )
        {
            // special case zero length
            if ( len == 0 )
                sub . sz = 0;
            
            // special case ASCII
            else if ( self . isAscii () )
                sub . sz = ( size_t ) len;
            
            // handle UTF-8
            else
                sub . sz = char_idx_to_byte_off ( sub . a, sub . sz, len );

            // set length
            sub . cnt = len;
        }

        return sub;
    }

    /**
     * subRgn
     *  @overload create a new region that is a subset of original
     *  @param start the location of the starting character
     *  @param xend exclusive ending location
     *  @return CText from start to min ( xend, end )
     */
    CText CText :: subRgn ( const CharLoc & start, const CharLoc & xend ) const
    {
#if TESTING
        sanity_check ( start, sz, cnt, a );
        sanity_check ( xend, sz, cnt, a );
#endif
        // detect out of bounds
        if ( start < 0 )
            return CText ( a, 0, 0 );
        if ( start >= ( index_t ) cnt )
            return CText ( a + sz, 0, 0 );

        size_t off = start . offset ();

        // an empty region
        if ( start >= xend )
            return CText ( a + off, 0, 0 );

        // xend beyond end of region
        if ( xend >= end () )
            return CText ( a + off, sz - off, cnt - start . index () );

        // legitimate range
        return CText ( a + off, xend . offset () - off, xend . index () - start . index () );
    }
    
    /**
     * intersect
     *  @return intersection between regions
     */
    CText CText :: intersect ( const CText & r ) const noexcept
    {
        // intersect the regions in memory
        CRgn isect = self . toCRgn () . intersect ( r . toCRgn () );

        // measure in characters and return intersection
        const UTF8 * start = ( const UTF8 * ) isect . addr ();
        count_t len = region_length ( start, isect . size () );
        return CText ( start, isect . size (), len );
    }

    /**
     * join
     *  @return union of regions
     */
    CText CText :: join ( const CText & r ) const
    {
        // join the regions in memory
        CRgn u = self . toCRgn () . join ( r . toCRgn () );

        // measure in characters and return union
        const UTF8 * start = ( const UTF8 * ) u . addr ();
        count_t len = region_length ( start, u . size () );
        return CText ( start, u . size (), len );
    }


    
    /*=====================================================*
     *                     CTextLiteral                    *
     *=====================================================*/

    /**
     * CTextLiteral
     *  @brief constructor for UTF-8 string with known size
     *  @param str is a pointer to unverified UTF-8 text
     *  @param sz is the number of bytes in the UTF-8 text
     */
    CTextLiteral :: CTextLiteral ( const UTF8 * str, size_t _sz, bool stop_on_bad )
        : CText ( "", 0, 0 )
    {
        if ( str != nullptr && _sz != 0 )
        {
            a = const_cast < UTF8 * > ( str );
            sz = _sz;
            cnt = region_length_verify ( str, _sz, stop_on_bad ? & sz : nullptr );
        }
    }

    /**
     * operator=
     *  @brief assignment from NUL-terminated UTF-8 string
     *  @param zstr is a pointer to unverified, NUL-terminated UTF-8
     */
    CTextLiteral & CTextLiteral :: operator = ( const UTF8 * zstr )
    {
        self = CTextLiteral ( zstr );
        return self;
    }

    /**
     * CTextLiteral
     *  @brief constructor for NUL-terminated UTF-8 string
     *  @param zstr is a pointer to unverified, NUL-terminated UTF-8
     */
    CTextLiteral :: CTextLiteral ( const UTF8 * zstr )
        : CText ( "", 0, 0 )
    {
        if ( zstr != nullptr && zstr [ 0 ] != 0 )
        {
            a = const_cast < UTF8 * > ( zstr );
            sz = :: strlen ( zstr );
            cnt = region_length_verify ( zstr, sz );
        }
    }

    
    /*=====================================================*
     *                         Char                        *
     *=====================================================*/

    /**
     * operator=
     *  @param ch a UTF-32 formatted UNICODE character
     */
    Char & Char :: operator = ( UTF32 ch )
    {
        Char c ( ch );
        return self . operator = ( c );
    }

    /**
     * Char
     *  @param ch a UTF-32 formatted UNICODE character
     */
    Char :: Char ( UTF32 ch )
        : CText ( mbch, 1, 1 )
    {
        mbch [ 0 ] = mbch [ 1 ] = mbch [ 2 ] = mbch [ 3 ] = 0;
        if ( is_ascii ( ch ) )
            mbch [ 0 ] = ( UTF8 ) ch;
        else
            sz = utf32_to_utf8_strict ( mbch, sizeof mbch, ch );
    }

    Char & Char :: operator = ( Char && ch )
    {
        sz = ch . size ();
        cnt = ch . count ();
        :: memmove ( mbch, ch . mbch, sizeof mbch );
        return self;
    }

    Char & Char :: operator = ( const Char & ch )
    {
        sz = ch . size ();
        cnt = ch . count ();
        :: memmove ( mbch, ch . mbch, sizeof mbch );
        return self;
    }

    Char :: Char ( Char && ch )
        : CText ( mbch, ch . size (), ch . count () )
    {
        :: memmove ( mbch, ch . mbch, sizeof mbch );
    }

    Char :: Char ( const Char & ch )
        : CText ( mbch, ch . size (), ch . count () )
    {
        :: memmove ( mbch, ch . mbch, sizeof mbch );
    }

    Char :: Char () noexcept
        : CText ( mbch, 0, 0 )
    {
        mbch [ 0 ] = mbch [ 1 ] = mbch [ 2 ] = mbch [ 3 ] = 0;
    }

    Char :: ~ Char () noexcept
    {
        mbch [ 0 ] = mbch [ 1 ] = mbch [ 2 ] = mbch [ 3 ] = 0;
    }
}
