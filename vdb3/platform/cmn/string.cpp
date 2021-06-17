/*

  vdb3.cmn.string

 */

#include <vdb3/cmn/string.hpp>
#include <vdb3/kfc/ktrace.hpp>
#include <vdb3/kfc/mem.hpp>
#include <vdb3/cmn/dserial.hpp>

#include <utf8proc/utf8proc.h>

namespace vdb3
{
    
    /*=====================================================*
     *                       UTILITY                       *
     *=====================================================*/

    // defined in text-utf8.cpp
    size_t utf32_to_utf8_strict ( UTF8 * buff, size_t bsize, UTF32 ch );
    
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


    /* string_elems
     *  measure a NUL-terminated string in elements
     */
    template < class T >
    static inline count_t string_elems ( const T * zstr )
    {
        size_t i;
        for ( i = 0; zstr [ i ] != 0; ++ i )
            ( void ) 0;
        return i;
    }
    
    static
    count_t string_length ( const UTF16 * data, count_t elems )
    {
        count_t cnt = 0;

        if ( elems != 0 )
        {
            for ( count_t i = 0; i < elems; ++ cnt, ++ i )
            {
                UTF16 ch = data [ i ];
                if ( ch >= 0xD800 && ch <= 0xDFFF )
                {
                    if ( ( ch & 0xDC00 ) != 0xD800 || i + 1 == elems )
                        throw InvalidUTF16String ( XP ( XLOC ) << "badly formed UTF-16 surrogate pair" );
                    ch = data [ i + 1 ];
                    if ( ( ch & 0xDC00 ) != 0xDC00 )
                        throw InvalidUTF16String ( XP ( XLOC ) << "badly formed UTF-16 surrogate pair" );
                    ++ i;
                }
            }
        }

        return cnt;
    }

    static
    count_t string_length_bswap ( const UTF16 * data, count_t elems )
    {
        count_t cnt = 0;

        if ( elems != 0 )
        {
            for ( count_t i = 0; i < elems; ++ cnt, ++ i )
            {
                UTF16 ch = __bswap_16 ( data [ i ] );
                if ( ch >= 0xD800 && ch <= 0xDFFF )
                {
                    if ( ( ch & 0xDC00 ) != 0xD800 || i + 1 == elems )
                        throw InvalidUTF16String ( XP ( XLOC ) << "badly formed UTF-16 surrogate pair" );
                    ch = __bswap_16 ( data [ i + 1 ] );
                    if ( ( ch & 0xDC00 ) != 0xDC00 )
                        throw InvalidUTF16String ( XP ( XLOC ) << "badly formed UTF-16 surrogate pair" );
                    ++ i;
                }
            }
        }

        return cnt;
    }

    /* utf8_to_utf32_strict
     *  gather 1..4 bytes of UTF-8 into a single UTF-32 character, and
     *  reject any input that does not pass validation.
     */
    static
    UniChar utf8_to_utf32_strict ( const signed char * data, size_t bytes, size_t offset )
    {
        // UTF-8 characters are formed with 1..4 bytes
        // the format supports up to 6 bytes

        // TBD - there are faster ways of doing this

        if ( offset >= bytes )
            throw BoundsException ( XP ( XLOC ) << "offset leaves no data" );

        // bytes guaranteed > 0
        bytes -= offset;

        // ease of typecast
        const unsigned char * p = reinterpret_cast < const unsigned char * > ( data + offset );

        // get the leading byte
        UTF32 ch = p [ 0 ];

        // assume size of 1
        size_t len = 1;

        // detect multi-byte
        if ( ch >= 0x80 )
        {
            // auxiliary bytes
            unsigned int b2, b3, b4;

            // detect 2 byte
            if ( ch < 0xE0 )
            {
                len = 2;
                if ( bytes < 2 )
                    throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );

                b2 = p [ 1 ];

                // leading byte must be in 0xC2..0xDF
                if ( ch < 0xC2 || ( b2 & 0xC0 ) != 0x80 )
                    throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );

                // compose character
                // valid UTF-16 or UTF-32
                ch = ( ( ch & 0x1F ) << 6 ) | ( b2 & 0x3F );
            }

            // detect 3 byte
            else if ( ch < 0xF0 )
            {
                len = 3;
                if ( bytes < 3 )
                    throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );

                b2 = p [ 1 ];
                b3 = p [ 2 ];

                // case for leading byte of exactly 0xE0
                if ( ch == 0xE0 )
                {
                    if ( b2 < 0xA0 || b2 > 0xBF || ( b3 & 0xC0 ) != 0x80 )
                        throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );
                }

                // case for leading byte in 0xE1..0xEC
                else if ( ch < 0xED )
                {
                    if ( ( b2 & 0xC0 ) != 0x80 || ( b3 & 0xC0 ) != 0x80 )
                        throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );
                }

                // case for leading byte of exactly 0xED
                else if ( ch == 0xED )
                {
                    if ( b2 < 0x80 || b2 > 0x9F || ( b3 & 0xC0 ) != 0x80 )
                        throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );
                }

                // case for leading byte in 0xEE..0xEF
                else
                {
                    if ( ( b2 & 0xC0 ) != 0x80 || ( b3 & 0xC0 ) != 0x80 )
                        throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );
                }

                // compose character
                // valid UTF-16 or UTF-32
                ch = ( ( ch & 0x0F ) << 12 ) | ( ( b2 & 0x3F ) << 6 ) | ( b3 & 0x3F );
            }

            // detect 4 byte
            else if ( ch <= 0xF4 )
            {
                len = 4;
                if ( bytes < 4 )
                    throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );

                b2 = p [ 1 ];
                b3 = p [ 2 ];
                b4 = p [ 3 ];

                // case for leading byte of exactly 0xF0
                if ( ch == 0xF0 )
                {
                    if ( b2 < 0x90 || b2 > 0xBF || ( b3 & 0xC0 ) != 0x80 || ( b4 & 0xC0 ) != 0x80 )
                        throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );
                }

                // case for leading byte in 0xE1..0xEC
                else if ( ch < 0xF4 )
                {
                    if ( ( b2 & 0xC0 ) != 0x80 || ( b3 & 0xC0 ) != 0x80 || ( b4 & 0xC0 ) != 0x80 )
                        throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );
                }

                // case for leading byte of exactly 0xF4
                else
                {
                    if ( b2 < 0x80 || b2 > 0x8F || ( b3 & 0xC0 ) != 0x80 || ( b4 & 0xC0 ) != 0x80 )
                        throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );
                }

                // compose character
                // valid UTF-32 only
                ch = ( ( ch & 0x07 ) << 18 ) | ( ( b2 & 0x3F ) << 12 ) | ( ( b3 & 0x3F ) << 6 ) | ( b4 & 0x3F );
            }

            // error
            else
            {
                throw InvalidUTF8String ( XP ( XLOC ) << "badly formed UTF-8 character" );
            }
        }

        return UniChar ( ch, len );
    }

    static inline
    UniChar utf8_to_utf32_strict ( const UTF8 * data, size_t bytes, size_t offset )
    {
        return utf8_to_utf32_strict ( ( const signed char * ) data, bytes, offset );
    }

    static
    UniChar utf16_to_utf32 ( const UTF16 * data, count_t elems, count_t offset )
    {
        if ( offset >= elems )
            throw BoundsException ( XP ( XLOC ) << "offset leaves no data" );

        UTF16 sur1 = data [ offset ];
        if ( sur1 < 0xD800 || sur1 > 0xDFFF )
            return UniChar ( sur1, 1 );

        if ( ( sur1 & 0xDC00 ) != 0xD800 || offset + 1 == elems )
            throw InvalidUTF16String ( XP ( XLOC ) << "badly formed UTF-16 surrogate pair" );

        UTF16 sur2 = data [ offset + 1 ];
        if ( ( sur2 & 0xDC00 ) != 0xDC00 )
            throw InvalidUTF16String ( XP ( XLOC ) << "badly formed UTF-16 surrogate pair" );

        UTF32 ch = ( ( ( UTF32 ) sur1 - 0xD800 ) << 10 ) + ( sur2 - 0xDC00 ) + 0x10000;
        return UniChar ( ch, 2 );
    }

    static
    UniChar utf16_to_utf32_bswap ( const UTF16 * data, count_t elems, count_t offset )
    {
        if ( offset >= elems )
            throw BoundsException ( XP ( XLOC ) << "offset leaves no data" );

        UTF16 sur1 = __bswap_16 ( data [ offset ] );
        if ( sur1 < 0xD800 || sur1 > 0xDFFF )
            return UniChar ( sur1, 1 );

        if ( ( sur1 & 0xDC00 ) != 0xD800 || offset + 1 == elems )
            throw InvalidUTF16String ( XP ( XLOC ) << "badly formed UTF-16 surrogate pair" );

        UTF16 sur2 = __bswap_16 ( data [ offset + 1 ] );
        if ( ( sur2 & 0xDC00 ) != 0xDC00 )
            throw InvalidUTF16String ( XP ( XLOC ) << "badly formed UTF-16 surrogate pair" );

        UTF32 ch = ( ( ( UTF32 ) sur1 - 0xD800 ) << 10 ) + ( sur2 - 0xDC00 ) + 0x10000;
        return UniChar ( ch, 2 );
    }
    
    /*=====================================================*
     *                        String                       *
     *=====================================================*/

    /**
     * concat
     * @overload concatenate self and provided String into a new String
     * @param s String to be concatenated to self
     * @return a new String
     */
    String String :: concat ( const RsrcMem & rsrc, const String & s ) const
    {
        // allocate a new block of memory
        // does not need clearing on alloc, but may need wiping on delete
        bool need_wipe = self . wipeBeforeDelete () | s . wipeBeforeDelete ();
        Mem mem = rsrc . mmgr . alloc ( self . size () + s . size (), false, need_wipe );

        // get its writable region
        {
            MRgnLocker mlock ( mem . mapMRgn () );

            // copy self data
            mlock . rgn () . copy ( self . data () . toCRgn () );

            // concat additional data
            mlock . rgn () . subRgn ( self . size () ) . copy ( s . data () . toCRgn () );
        }

        // now lock it as an immutable region
        CRgnLockRef clock ( mem . mapCRgn () );

        // create a text region
        CText ctext = makeCText ( clock -> getCRgn (), self . count () + s . count () );

        // return the new String
        return String ( clock, ctext, need_wipe );
    }

    /**
     * toupper
     * @return upper-cased copy of String
     */
    String String :: toupper ( const RsrcMem & rsrc ) const
    {
        // allocate a new block of memory
        // does not need clearing on alloc, but may need wiping on delete
        Mem mem = rsrc . mmgr . alloc ( self . size (), false, wipeBeforeDelete () );

        // get its writable region
        {
            MRgnLocker mlock ( mem . mapMRgn () );

            // copy self data
            mlock . rgn () . copy ( self . data () . toCRgn () );

            // convert to mutable text region
            MText mtext = makeMText ( mlock . rgn (), self . count () );

            // upper-case the region
            mtext . toupper ();
        }

        // now lock it as an immutable region
        CRgnLockRef clock ( mem . mapCRgn () );

        // create a text region
        CText ctext = makeCText ( clock -> getCRgn (), self . count () );

        // return the new String
        return String ( clock, ctext, wipeBeforeDelete () );
    }

    /**
     * tolower
     * @return lower-cased copy of String
     */
    String String :: tolower ( const RsrcMem & rsrc ) const
    {
        // allocate a new block of memory
        // does not need clearing on alloc, but may need wiping on delete
        Mem mem = rsrc . mmgr . alloc ( self . size (), false, wipeBeforeDelete () );

        // get its writable region
        {
            MRgnLocker mlock ( mem . mapMRgn () );

            // copy self data
            mlock . rgn () . copy ( self . data () . toCRgn () );

            // convert to mutable text region
            MText mtext = makeMText ( mlock . rgn (), self . count () );

            // lower-case the region
            mtext . tolower ();
        }

        // now lock it as an immutable region
        CRgnLockRef clock ( mem . mapCRgn () );

        // create a text region
        CText ctext = makeCText ( clock -> getCRgn (), self . count () );

        // return the new String
        return String ( clock, ctext, wipeBeforeDelete () );
    }

    /**
     * makeIterator
     * @return an Iterator to an initial point within String
     */
    StringIterator String :: makeIterator ( index_t origin ) const noexcept
    {
        return StringIterator ( rgn_lock, data (), data () . getLoc ( origin ) );
    }

    /**
     * makeIterator
     * @return an Iterator to an initial point within String
     */
    StringIterator String :: makeIterator ( const CharLoc & loc ) const noexcept
    {
        return StringIterator ( rgn_lock, data (), loc );
    }

    String & String :: operator = ( String && s ) noexcept
    {
        rgn_lock = std :: move ( s . rgn_lock );
        rgn = std :: move ( s . rgn );
        wipe = s . wipe;
        return self;
    }
    
    String & String :: operator = ( const String & s ) noexcept
    {
        rgn_lock = s . rgn_lock;
        rgn = s . rgn;
        wipe = s . wipe;
        return self;
    }
    
    String :: String ( String && s ) noexcept
        : rgn_lock ( std :: move ( s . rgn_lock ) )
        , rgn ( std :: move ( s . rgn ) )
        , wipe ( s . wipe )
    {
    }
    
    String :: String ( const String & s ) noexcept
        : rgn_lock ( s . rgn_lock )
        , rgn ( s . rgn )
        , wipe ( s . wipe )
    {
    }
    
    String :: String () noexcept
        : wipe ( false )
    {
    }
    
    String :: ~ String () noexcept
    {
    }
    
    String & String :: operator = ( const UTF8 * zstr )
    {
        return operator = ( CTextLiteral ( zstr ) );
    }
    
    String :: String ( const UTF8 * zstr )
        : rgn ( CTextLiteral ( zstr ) )
        , wipe ( false )
    {
    }
    
    String & String :: operator = ( const CTextLiteral & t ) noexcept
    {
        rgn_lock . release ();
        rgn = t;
        wipe = false;
        return self;
    }
    
    String :: String ( const CTextLiteral & t ) noexcept
        : rgn ( t )
        , wipe ( false )
    {
    }

    String :: String ( const RsrcMem & rsrc, const CText & t, bool _wipe )
        : wipe ( _wipe )
    {
        Mem mem = rsrc . mmgr . alloc ( t . size (), false, wipe );
        {
            MRgnLocker mlock ( mem . mapMRgn () );
            mlock . rgn () . copy ( t . toCRgn () );
        }

        rgn_lock = mem . mapCRgn ();
        rgn = makeCText ( rgn_lock -> getCRgn (), t . count () );
    }

    String :: String ( const RsrcMem & rsrc, const std :: string & str, bool _wipe )
        : wipe ( _wipe )
    {
        KTRACE ( TRACE_PRG, "copy from std::string = '%s'\n", str . c_str () );
        CTextLiteral t ( str . data (), str . size () );
        KTRACE ( TRACE_GEEK, "CTextLiteral t = '%.*s', %llu characters\n"
                , ( int ) t . size (), t . addr ()
                , t . count () );
        KTRACE ( TRACE_PRG, "allocating %zu bytes of memory\n", str . size () );
        Mem mem = rsrc . mmgr . alloc ( str . size (), false, wipe );
        {
            KTRACE ( TRACE_PRG, "locking memory allocation for update\n" );
            MRgnLocker mlock ( mem . mapMRgn () );
            KTRACE ( TRACE_PRG, "copying from CTextLiteral region\n" );
            mlock . rgn () . copy ( t . toCRgn () );
            KTRACE ( TRACE_GEEK, "copied mem = '%.*s'\n"
                    , ( int ) mlock . rgn () . size (), ( const char * ) mlock . rgn () . addr ()
                );
            KTRACE ( TRACE_PRG, "unlocking memory allocation for update\n" );
        }

        KTRACE ( TRACE_PRG, "locking memory allocation for read\n" );
        rgn_lock = mem . mapCRgn ();
        KTRACE ( TRACE_PRG, "assigning CText region from memory CRgn\n" );
        rgn = makeCText ( rgn_lock -> getCRgn (), t . count () );
        KTRACE ( TRACE_GEEK, "rgn = '%.*s', %llu characters\n"
                , ( int ) rgn . size (), rgn . addr ()
                , rgn . count () );
    }

    String :: String ( const RsrcMem & rsrc, const UTF8 * zstr, bool _wipe )
        : wipe ( _wipe )
    {
        if ( zstr == nullptr )
        {
            throw NullArgumentException (
                XP ( XLOC )
                << xprob
                << "failed to construct String from UTF-8"
                << xcause
                << "null source pointer"
                );
        }

        CTextLiteral t ( zstr );
        if ( t . isAscii () )
        {
            Mem mem = rsrc . mmgr . alloc ( t . size (), false, wipe );
            {
                MRgnLocker mlock ( mem . mapMRgn () );
                mlock . rgn () . copy ( t . toCRgn () );
            }

            rgn_lock = mem . mapCRgn ();
            rgn = makeCText ( rgn_lock -> getCRgn (), t . count () );
        }
        else
        {
            // detect BOM - useless, but legal.
            const UTF8 * s = zstr;
            count_t elems = t . size ();
            if ( elems >= 3 )
            {
                const unsigned char * us = ( const unsigned char * ) s;
                if ( us [ 0 ] == 0xEF && us [ 1 ] == 0xBB && us [ 2 ] == 0xBF )
                {
                    s += 3;
                    elems -= 3;
                }
            }

            // count the number of UTF-8 characters
            count_t len = t . count ();

            // create a UTF-32 buffer
            std :: unique_ptr < UTF32 > utf32_sp ( new UTF32 [ len + 1 ] );
            UTF32 * utf32 = utf32_sp . get ();

            // convert UTF-8 to UTF-32 using strict checking
            count_t i, offset = 0;
            for ( i = 0; i < len; ++ i )
            {
                UniChar uch = utf8_to_utf32_strict ( s, elems, offset );
                utf32 [ i ] = uch . ch;
                offset += uch . size;
            }
            
            // NUL-terminate UTF-32 string
            utf32 [ i ] = 0;

            // use buffer memory to normalize UTF-32 string
            // and assign it into the UTF-8 data member
            normalize ( rsrc, utf32, i );
        }
    }

    String :: String ( const RsrcMem & rsrc, const UTF16 * zstr, bool _wipe )
        : wipe ( _wipe )
    {
        if ( zstr == nullptr )
        {
            throw NullArgumentException (
                XP ( XLOC )
                << xprob
                << "failed to construct String from UTF-16"
                << xcause
                << "null source pointer"
                );
        }

        const UTF16 * s = zstr;
        count_t elems = string_elems ( zstr );
        
        // BOM - not expected, but must be handled
        bool bswap = false;
        if ( elems >= 1 )
        {
            if ( s [ 0 ] == 0xFEFF )
            {
                ++ s;
                -- elems;
            }
            else if ( s [ 0 ] == 0xFFFE )
            {
                bswap = true;
                ++ s;
                -- elems;
            }
        }

        // count the number of UTF-16 characters
        count_t len = bswap ?
            string_length_bswap ( s, elems ):
            string_length ( s, elems );

        // create a UTF-32 buffer
        std :: unique_ptr < UTF32 > utf32_sp ( new UTF32 [ len + 1 ] );
        UTF32 * utf32 = utf32_sp . get ();

        // convert UTF-16 to UTF-32 using strict checking
        count_t i, offset = 0;
        if ( bswap )
        {
            for ( i = 0; i < len; ++ i )
            {
                UniChar uch = utf16_to_utf32_bswap ( s, elems, offset );
                utf32 [ i ] = uch . ch;
                offset += uch . size;
            }
        }
        else
        {
            for ( i = 0; i < len; ++ i )
            {
                UniChar uch = utf16_to_utf32 ( s, elems, offset );
                utf32 [ i ] = uch . ch;
                offset += uch . size;
            }
        }

        // NUL-terminate UTF-32 string
        utf32 [ i ] = 0;

        // use buffer memory to normalize UTF-32 string
        // and assign it into the UTF-8 data member
        normalize ( rsrc, utf32, i );
    }

    String :: String ( const RsrcMem & rsrc, const UTF32 * zstr, bool _wipe )
        : wipe ( _wipe )
    {
        if ( zstr == nullptr )
        {
            throw NullArgumentException (
                XP ( XLOC )
                << xprob
                << "failed to construct String from UTF-32"
                << xcause
                << "null source pointer"
                );
        }

        // convert UTF32 to UTF8
        const UTF32 * s = zstr;
        count_t elems = string_elems ( zstr );
        
        // BOM - not expected, but must be handled
        assert ( s != nullptr );
        bool bswap = false;
        if ( elems >= 1 )
        {
            if ( s [ 0 ] == 0xFEFF )
            {
                ++ s;
                -- elems;
            }
            else if ( s [ 0 ] == 0xFFFE )
            {
                bswap = true;
                ++ s;
                -- elems;
            }
        }

        // count the number of UTF-32 characters
        count_t len = elems;

        // create a UTF-32 buffer
        std :: unique_ptr < UTF32 > utf32_sp ( new UTF32 [ len + 1 ] );
        UTF32 * utf32 = utf32_sp . get ();

        // copy UTF-32 characters to buffer
        count_t i;
        if ( bswap )
        {
            for ( i = 0; i < len; ++ i )
                utf32 [ i ] = __bswap_32 ( s [ i ] );
        }
        else
        {
            for ( i = 0; i < len; ++ i )
                utf32 [ i ] = s [ i ];
        }

        // NUL-terminate UTF-32 string
        utf32 [ i ] = 0;

        // use buffer memory to normalize UTF-32 string
        // and assign it into the UTF-8 data member
        normalize ( rsrc, utf32, i );
    }

    // recreate from serialization
    String :: String ( const RsrcMem & rsrc, Deserial & d, bool _wipe )
        : wipe ( _wipe )
    {
        CText t;
        d . get ( t );
        
        Mem mem = rsrc . mmgr . alloc ( t . size (), false, wipe );
        {
            MRgnLocker mlock ( mem . mapMRgn () );
            mlock . rgn () . copy ( t . toCRgn () );
        }

        rgn_lock = mem . mapCRgn ();
        rgn = makeCText ( rgn_lock -> getCRgn (), t . count () );
    }

    void String :: normalize ( const RsrcMem & rsrc, UTF32 * buffer, count_t ccnt )
    {
        // use external library for the time being to normalize in place
        utf8proc_int32_t * b = reinterpret_cast < utf8proc_int32_t * > ( buffer );
        utf8proc_ssize_t ncnt = utf8proc_normalize_utf32 ( b, ccnt, UTF8PROC_COMPOSE );
        if ( ncnt < 0 )
        {
            switch ( ncnt )
            {
            case UTF8PROC_ERROR_NOMEM:
                throw MemoryExhausted (
                    XP ( XLOC )
                    << "utf8proc_normalize_utf32 - "
                    << utf8proc_errmsg ( ncnt )
                    );
            case UTF8PROC_ERROR_OVERFLOW:
                throw BoundsException (
                    XP ( XLOC )
                    << "utf8proc_normalize_utf32 - "
                    << utf8proc_errmsg ( ncnt )
                    );
            case UTF8PROC_ERROR_INVALIDUTF8:
            case UTF8PROC_ERROR_NOTASSIGNED:
            case UTF8PROC_ERROR_INVALIDOPTS:
                throw InvalidUTF32Character (
                    XP ( XLOC )
                    << "utf8proc_normalize_utf32 - "
                    << utf8proc_errmsg ( ncnt )
                    );
            default:
                throw InvalidArgument (
                    XP ( XLOC )
                    << "utf8proc_normalize_utf32 - "
                    << utf8proc_errmsg ( ncnt )
                    );
            }
        }

        // we can now transform to UTF-8 in place, given the restriction that
        // UTF-8 uses 1..4 bytes per character while UTF-32 uses 4 bytes, so
        // it will work in order.
        const UTF32 * utf32 = buffer;
        UTF8 * utf8 = reinterpret_cast < UTF8 * > ( buffer );

        count_t i, offset;
        for ( i = 0, ccnt = ncnt, offset = 0; i < ccnt; ++ i )
        {
            // if our code disagrees with libutf8proc, we could see exceptions
            size_t sz = utf32_to_utf8_strict ( & utf8 [ offset ], sizeof ( UTF32 ), utf32 [ i ] );
            offset += sz;
        }

        CText t = makeCText ( utf8, offset, i );
        Mem mem = rsrc . mmgr . alloc ( t . size (), false, wipe );
        {
            MRgnLocker mlock ( mem . mapMRgn () );
            mlock . rgn () . copy ( t . toCRgn () );
        }

        rgn_lock = mem . mapCRgn ();
        rgn = makeCText ( rgn_lock -> getCRgn (), t . count () );
    }

    String :: String ( const CRgnLockRef & lock, const CText & t, bool _wipe )
        : rgn_lock ( lock )
        , rgn ( t )
        , wipe ( _wipe )
    {
    }


    /*=====================================================*
     *                    NULTermString                    *
     *=====================================================*/

    /**
     * c_str
     * @return a raw pointer to a NUL-terminated C++ string
     */
    const UTF8 * NULTermString :: c_str () const noexcept
    {
        assert ( rgn . addr () [ rgn . size () ] == 0 );
        return rgn . addr ();
    }

    /**
     * NULTermString
     * @overload constructor from a String
     *
     * guarantees NUL termination
     */
    NULTermString :: NULTermString ( const RsrcMem & rsrc, const String & s )
    {
        // allocate a block of memory 1 byte larger than source
        Mem mem = rsrc . mmgr . alloc ( s . size () + 1, false, s . wipeBeforeDelete () );
        {
            // lock region for update
            MRgnLocker mlock ( mem . mapMRgn () );

            // copy in string data - leaving 1 byte uninitialized
            mlock . rgn () . copy ( s . data () . toCRgn () );

            // fill in the uninitialized region with NUL
            mlock . rgn () . subRgn ( s . size () ) . fill ( 0 );
        }

        // acquire lock on memory
        rgn_lock = mem . mapCRgn ();

        // make a CText region from sub-region of memory CRgn and count from String
        rgn = makeCText ( rgn_lock -> getCRgn () . subRgn ( 0, s . size () ), s . count () );

        // remember to wipe
        wipe = s . wipeBeforeDelete ();

    }

    NULTermString & NULTermString :: operator = ( NULTermString && zs )
    {
        String :: operator = ( std :: move ( zs ) );
        return self;
    }
    
    NULTermString & NULTermString :: operator = ( const NULTermString & zs )
    {
        String :: operator = ( zs );
        return self;
    }
    
    NULTermString :: NULTermString ( NULTermString && zs )
        : String ( std :: move ( zs ) )
    {
    }
    
    NULTermString :: NULTermString ( const NULTermString & zs )
        : String ( zs )
    {
    }
    
}
