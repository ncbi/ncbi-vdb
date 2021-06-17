/*

  vdb3.kfc.xp


 */

#include <vdb3/kfc/xp.hpp>
#include <vdb3/kfc/ktrace.hpp>
#include <vdb3/kfc/ascii.hpp>

#include <cstring>
#include <execinfo.h>

#include "strerror_r.h"

namespace vdb3
{
    
    /*=====================================================*
     *                       UTILITY                       *
     *=====================================================*/

    XMsg copyXMsg ( const XBuf & b )
    {
        XMsg x;

        size_t bytes = b . marker;
        if ( bytes >= sizeof x . zmsg )
            bytes = sizeof x . zmsg - 1;

        :: memmove ( x . zmsg, b . buffer, bytes );

        x . msg_size = bytes;
        x . zmsg [ bytes ] = 0;

        return x;
    }

    XMsg copyXMsg ( const char * s )
    {
        XMsg x;

        size_t i;
        for ( i = 0; s [ i ] != 0 && i < sizeof x . zmsg - 1; ++ i )
            x . zmsg [ i ] = s [ i ];

        x . msg_size = i;
        x . zmsg [ i ] = 0;

        return x;
    }

    XMsg & concatXMsg ( XMsg & x, const XBuf & b )
    {
        assert ( x . msg_size < sizeof x . zmsg );
        size_t bytes = b . marker;

        if ( x . msg_size + bytes >= sizeof x . zmsg )
            bytes = sizeof x . zmsg - x . msg_size - 1;

        :: memmove ( & x . zmsg [ x . msg_size ], b . buffer, bytes );

        x . msg_size += bytes;
        x . zmsg [ x . msg_size ] = 0;

        return x;
    }

    XMsg & concatXMsg ( XMsg & x, const char * s )
    {
        assert ( x . msg_size < sizeof x . zmsg );
        size_t i, remain = sizeof x . zmsg - x . msg_size - 1;

        for ( i = 0; s [ i ] != 0 && i < remain; ++ i )
            x . zmsg [ x . msg_size + i ] = s [ i ];

        x . msg_size += i;
        x . zmsg [ x . msg_size ] = 0;

        return x;
    }

    static
    UTF8 * reallocate ( UTF8 * old_buffer, size_t & bytes, size_t additional )
    {
        KTRACE ( TRACE_GEEK, "reallocating %zu bytes @ %p with %zu additional bytes"
                , bytes
                , old_buffer
                , additional
            );

        size_t new_size = ( bytes + additional + 4095 ) & ~ ( size_t ) 4095;
        UTF8 * new_buffer = ( UTF8 * ) :: realloc ( old_buffer, new_size );
        if ( new_buffer == nullptr )
            :: exit ( rc_runtime_err );

        bytes = new_size;

        KTRACE ( TRACE_GEEK, "new buffer size is %zu bytes @ %p", bytes, new_buffer );
        return new_buffer;
    }

    static
    const UTF8 * string_rchr ( const UTF8 * str, int ch, const UTF8 * from )
    {
        for ( size_t sz = from - str; sz > 0; )
        {
            if ( str [ -- sz ] == ch )
                return & str [ sz ];
        }

        return nullptr;
    }


    /*=====================================================*
     *                        XBuf                         *
     *=====================================================*/

    bool XBuf :: isEmpty () const noexcept
    {
        return marker == 0;
    }
    
    XBuf & XBuf :: append ( ASCII ch ) noexcept
    {
        KTRACE ( TRACE_GEEK, "appending single character '%c'", ch );
        if ( marker + 2 > bsize )
            buffer = reallocate ( buffer, bsize, 2 );
        buffer [ marker ] = ch;
        buffer [ ++ marker ] = 0;
        KTRACE ( TRACE_GEEK, "new buffer value @ $p is '%s'", buffer, buffer );
        return self;
    }
    
    XBuf & XBuf :: append ( const UTF8 * zstr ) noexcept
    {
        return append ( zstr, :: strlen ( zstr ) );
    }
    
    XBuf & XBuf :: append ( const UTF8 * str, size_t bytes ) noexcept
    {
        KTRACE ( TRACE_GEEK, "appending string '%.*s'", ( int ) bytes, str );
        if ( marker + bytes + 1 > bsize )
            buffer = reallocate ( buffer, bsize, bytes + 1 );
        :: memmove ( & buffer [ marker ], str, bytes );
        buffer [ marker += bytes ] = 0;
        KTRACE ( TRACE_GEEK, "new buffer value @ %p is '%s'", buffer, buffer );
        return self;
    }

    void XBuf :: operator = ( XBuf && b ) noexcept
    {
        KTRACE ( TRACE_GEEK, "move assigning XBuf @ %p", b . buffer );
        buffer = b . buffer;
        bsize = b . bsize;
        marker = b . marker;
        
        b . buffer = nullptr;
        b . bsize = 0;
        b . marker = 0;
    }
    
    XBuf :: XBuf ( XBuf && b ) noexcept
        : marker ( b . marker )
        , bsize ( b . bsize )
        , buffer ( b . buffer )
    {
        KTRACE ( TRACE_GEEK, "move constructing XBuf @ %p", buffer );
        b . buffer = nullptr;
        b . bsize = 0;
        b . marker = 0;
    }
    
    XBuf :: XBuf () noexcept
        : marker ( 0 )
        , bsize ( 0 )
        , buffer ( nullptr )
    {
    }
    
    XBuf :: ~ XBuf () noexcept
    {
        KTRACE ( TRACE_GEEK, "destroying XBuf" );
        free ( ( void * ) buffer );
        buffer = nullptr;
        bsize = 0;
        marker = 0;
    }


    /*=====================================================*
     *                         XP                          *
     *=====================================================*/

    XP & XP :: operator << ( const UTF8 * utf8 ) noexcept
    {
        if ( utf8 == nullptr )
            utf8 = "<NULL PTR>";
        which -> append ( utf8 );
        return self;
    }

    XP & XP :: operator << ( const XMsg & val ) noexcept
    {
        size_t bytes = val . msg_size;
        if ( bytes < sizeof val . zmsg )
        {
            for ( size_t i = 0; i < bytes; ++ i )
            {
                if ( val . zmsg [ i ] == 0 )
                {
                    bytes = i;
                    break;
                }
            }
            which -> append ( val . zmsg, bytes );
        }
        else
        {
            which -> append ( "<BAD MESSAGE BLOCK>" );
        }

        return self;
    }

    XP & XP :: operator << ( const std :: string & val ) noexcept
    {
        which -> append ( val . data (), val . size () );
        return self;
    }

    void XP :: putTxtRgn ( const TextRgn & val ) noexcept
    {
        which -> append ( val . constAddr (), val . size () );
    }

    void XP :: setStatus ( ReturnCodes status ) noexcept
    {
        rc = status;
    }

    void XP :: useProblem () noexcept
    {
        which = & problem;
        radix = 10;
    }

    void XP :: useContext () noexcept
    {
        which = & context;
        radix = 10;
    }

    void XP :: useCause () noexcept
    {
        which = & cause;
        radix = 10;
    }

    void XP :: useSuggestion () noexcept
    {
        which = & suggestion;
        radix = 10;
    }

    XP :: XP ( const UTF8 * file, const ASCII * func, unsigned int line, ReturnCodes status ) noexcept
        : which ( & problem )
        , file_name ( file )
        , func_name ( func )
        , lineno ( line )
        , stack_frames ( -1 )
        , rc ( status )
    {
        // capture stack
        stack_frames = backtrace ( callstack, sizeof callstack / sizeof callstack [ 0 ] );

        // eliminate random path prefix
        const UTF8 * cmn = __FILE__;
        const UTF8 * end = :: strrchr ( cmn, '/' );
        if ( end != nullptr )
        {
            assert ( :: strcmp ( end + 1, "xp.cpp" ) == 0 );
            end = string_rchr ( cmn, '/', end );
            if ( end != nullptr )
            {
                assert ( :: strcmp ( end + 1, "kfc/xp.cpp" ) == 0 );
                size_t cmn_sz = end - cmn + 1;
                if ( :: memcmp ( file_name, cmn, cmn_sz ) == 0 )
                    file_name += cmn_sz;
            }
        }
    }

    XP :: ~ XP () noexcept
    {
        KTRACE ( TRACE_GEEK, "destroying XP" );
    }

    void XP :: write ( const TextRgn & val )
    {
        which -> append ( val . constAddr (), val . size () );
    }
    
    void XP :: fill ( ASCII ch, count_t repeat )
    {
    }

    XMsg syserr ( int err ) noexcept
    {
        // get an exception message buffer and set to empty C-string
        XMsg xm;
        xm . zmsg [ 0 ] = 0;

        // pull out the error code
        // use external function compiled in C
        // because g++ appears to destroy this in some cases
        :: string_error_r ( err, xm . zmsg, sizeof xm . zmsg );

        // ENSURE that there is a NUL byte to terminate buffer
        xm . zmsg [ sizeof xm . zmsg - 1 ] = 0;

        // measure the size of the returned string
        xm . msg_size = :: strlen ( xm . zmsg );
        
        return xm;
    }
}
