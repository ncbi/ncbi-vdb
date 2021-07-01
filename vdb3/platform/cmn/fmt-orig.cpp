/*

  vdb3.cmn.fmt


 */

#include <vdb3/cmn/fmt.hpp>
#include <vdb3/cmn/string.hpp>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#if HAVE_R128
#include <quadmath.h>
#endif

namespace cmn
{

    static
    bool vprint_use_crlf ( const char * fmt )
    {
        size_t size = strlen ( fmt );
        return ( size > 1 && fmt [ size - 2 ] == '\r' && fmt [ size - 1 ] == '\n' );
    }

    static
    void vprint ( FmtWriter & writer, const char * fmt, va_list args )
    {
        size_t bytes;
        char buffer [ 4096 ];
        const char * msg = buffer;

        int status = vsnprintf ( buffer, sizeof buffer, fmt, args );
        bytes = status;

        if ( status < 0 )
        {
            bool use_crlf = vprint_use_crlf ( fmt );
            if ( use_crlf )
            {
                const char too_long [] = "message too long...\r\n";
                msg = too_long;
                bytes = sizeof too_long - 1;
            }
            else
            {
                const char too_long [] = "message too long...\n";
                msg = too_long;
                bytes = sizeof too_long - 1;
            }
        }
        else if ( bytes > sizeof buffer )
        {
            bool use_crlf = vprint_use_crlf ( fmt );
            bytes = sizeof buffer;
            if ( use_crlf )
                memcpy ( & buffer [ sizeof buffer - 5 ], "...\r\n", 5 );
            else
                memcpy ( & buffer [ sizeof buffer - 4 ], "...\n", 4 );
        }

        writer . write ( msg, bytes );
    }

    void FmtWriter :: print ( const char * fmt, ... )
    {
        va_list args;
        va_start ( args, fmt );

        vprint ( self, fmt, args );

        va_end ( args );
    }

    Fmt & Fmt :: operator << ( bool val )
    {
        if ( val )
            write ( "true", 4 );
        else
            write ( "false", 5 );

        return self;
    }

    Fmt & Fmt :: operator << ( long long sval )
    {
        // since we print directly into the buffer,
        // perform our own indentation
        if ( at_line_start && indentation > 0 )
            indent ();

        unsigned long long val = sval;

        bool neg = false;
        if ( sval < 0 )
        {
            neg = true;
            val = - sval;
        }

        char digits [ sizeof val * 8 + 3 ];

        unsigned int i = sizeof digits;
        do
        {
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ val % radix ];
            val /= radix;
        }
        while ( val != 0 );

        // the number of actual digits
        unsigned int cur_width = sizeof digits - i;

        // figure in sign and prefix if not base 10
        unsigned int total_width = cur_width + neg + 2 * ( radix != 10 );

        // calculate the amount of padding
        unsigned int padding = 0;
        if ( total_width < ( unsigned int ) width )
            padding = ( unsigned int ) width - total_width;

        // the total field width
        total_width += padding;

        // test against what is available
        size_t marker = b . size ();
        checkSize ( marker, total_width );
        char * buffer = ( char * ) b . data ();

        // padd with spaces
        if ( padding != 0 && fill == ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // set sign
        if ( neg )
            buffer [ marker ++ ] = '-';

        // handle prefix
        switch ( radix )
        {
        case 10:
            break;
        case 2:
            memcpy ( & buffer [ marker ], "0b", 2 );
            marker += 2;
            break;
        case 8:
            memcpy ( & buffer [ marker ], "0o", 2 );
            marker += 2;
            break;
        case 16:
            memcpy ( & buffer [ marker ], "0x", 2 );
            marker += 2;
            break;
        }

        // padd with zeros
        if ( padding != 0 && fill != ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // now finish with digits
        memcpy ( & buffer [ marker ], & digits [ i ], cur_width );
        marker += cur_width;

        // in wonderful C++ fashion, reset everything
        radix = 10;
        width = 0;
        precision = 0;
        fill = ' ';
        have_precision = false;
        at_line_start = false;

        b . setSize ( marker );

        return self;
    }

    Fmt & Fmt :: operator << ( unsigned long long val )
    {
        // since we print directly into the buffer,
        // perform our own indentation
        if ( at_line_start && indentation > 0 )
            indent ();

        char digits [ sizeof val * 8 + 2 ];

        unsigned int i = sizeof digits;
        do
        {
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ val % radix ];
            val /= radix;
        }
        while ( val != 0 );

        // the number of actual digits
        unsigned int cur_width = sizeof digits - i;

        // figure in prefix if not base 10
        unsigned int total_width = cur_width + 2 * ( radix != 10 );

        // calculate the amount of padding
        unsigned int padding = 0;
        if ( total_width < ( unsigned int ) width )
            padding = ( unsigned int ) width - total_width;

        // the total field width
        total_width += padding;

        // test against what is available
        size_t marker = b . size ();
        checkSize ( marker, total_width );
        char * buffer = ( char * ) b . data ();

        // padd with spaces
        if ( padding != 0 && fill == ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // handle prefix
        switch ( radix )
        {
        case 10:
            break;
        case 2:
            memcpy ( & buffer [ marker ], "0b", 2 );
            marker += 2;
            break;
        case 8:
            memcpy ( & buffer [ marker ], "0o", 2 );
            marker += 2;
            break;
        case 16:
            memcpy ( & buffer [ marker ], "0x", 2 );
            marker += 2;
            break;
        }

        // padd with zeros
        if ( padding != 0 && fill != ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // now finish with digits
        memcpy ( & buffer [ marker ], & digits [ i ], cur_width );
        marker += cur_width;

        // in wonderful C++ fashion, reset everything
        radix = 10;
        width = 0;
        precision = 0;
        fill = ' ';
        have_precision = false;
        at_line_start = false;

        b . setSize ( marker );

        return self;
    }

#if HAVE_Z128
    Fmt & Fmt :: operator << ( __int128 sval )
    {
        unsigned __int128 val = sval;

        bool neg = false;
        if ( sval < 0 )
        {
            neg = true;
            val = - sval;
        }

#if 0
        // test for being within 64-bit range
        if ( ( val >> 64 ) == 0 )
            return operator << ( long ) sval;
#endif
        // since we print directly into the buffer,
        // perform our own indentation
        if ( at_line_start && indentation > 0 )
            indent ();

        char digits [ sizeof val * 8 + 3 ];

        unsigned int i = sizeof digits;
        do
        {
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ val % radix ];
            val /= radix;
        }
        while ( val != 0 );

        // the number of actual digits
        unsigned int cur_width = sizeof digits - i;

        // figure in sign and prefix if not base 10
        unsigned int total_width = cur_width + neg + 2 * ( radix != 10 );

        // calculate the amount of padding
        unsigned int padding = 0;
        if ( total_width < ( unsigned int ) width )
            padding = ( unsigned int ) width - total_width;

        // the total field width
        total_width += padding;

        // test against what is available
        size_t marker = b . size ();
        checkSize ( marker, total_width );
        char * buffer = ( char * ) b . data ();

        // padd with spaces
        if ( padding != 0 && fill == ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // set sign
        if ( neg )
            buffer [ marker ++ ] = '-';

        // handle prefix
        switch ( radix )
        {
        case 10:
            break;
        case 2:
            memcpy ( & buffer [ marker ], "0b", 2 );
            marker += 2;
            break;
        case 8:
            memcpy ( & buffer [ marker ], "0o", 2 );
            marker += 2;
            break;
        case 16:
            memcpy ( & buffer [ marker ], "0x", 2 );
            marker += 2;
            break;
        }

        // padd with zeros
        if ( padding != 0 && fill != ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // now finish with digits
        memcpy ( & buffer [ marker ], & digits [ i ], cur_width );
        marker += cur_width;

        // in wonderful C++ fashion, reset everything
        radix = 10;
        width = 0;
        precision = 0;
        fill = ' ';
        have_precision = false;
        at_line_start = false;

        b . setSize ( marker );

        return self;
    }
#endif // HAVE_Z128

#if HAVE_Z128
    Fmt & Fmt :: operator << ( unsigned __int128 val )
    {
#if 0
        // test for being within 64-bit range
        if ( ( val >> 64 ) == 0 )
            return operator << ( unsigned long ) val;
#endif
        // since we print directly into the buffer,
        // perform our own indentation
        if ( at_line_start && indentation > 0 )
            indent ();

        char digits [ sizeof val * 8 + 2 ];

        unsigned int i = sizeof digits;
        do
        {
            digits [ -- i ] = "0123456789abcdefhijklmnopqrstuvwxyz" [ val % radix ];
            val /= radix;
        }
        while ( val != 0 );

        // the number of actual digits
        unsigned int cur_width = sizeof digits - i;

        // figure in prefix if not base 10
        unsigned int total_width = cur_width + 2 * ( radix != 10 );

        // calculate the amount of padding
        unsigned int padding = 0;
        if ( total_width < ( unsigned int ) width )
            padding = ( unsigned int ) width - total_width;

        // the total field width
        total_width += padding;

        // test against what is available
        size_t marker = b . size ();
        checkSize ( marker, total_width );
        char * buffer = ( char * ) b . data ();

        // padd with spaces
        if ( padding != 0 && fill == ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // handle prefix
        switch ( radix )
        {
        case 10:
            break;
        case 2:
            memcpy ( & buffer [ marker ], "0b", 2 );
            marker += 2;
            break;
        case 8:
            memcpy ( & buffer [ marker ], "0o", 2 );
            marker += 2;
            break;
        case 16:
            memcpy ( & buffer [ marker ], "0x", 2 );
            marker += 2;
            break;
        }

        // padd with zeros
        if ( padding != 0 && fill != ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // now finish with digits
        memcpy ( & buffer [ marker ], & digits [ i ], cur_width );
        marker += cur_width;

        // in wonderful C++ fashion, reset everything
        radix = 10;
        width = 0;
        precision = 0;
        fill = ' ';
        have_precision = false;
        at_line_start = false;

        b . setSize ( marker );

        return self;
    }
#endif // HAVE_Z128

    Fmt & Fmt :: operator << ( double val )
    {
        // rely upon stdclib printf engine for the time being
        int cur_width = 0;
        char digits [ 512 ];

        if ( have_precision )
            cur_width = snprintf ( digits, sizeof digits, "%.*lf", precision, val );
        else
            cur_width = snprintf ( digits, sizeof digits, "%lf", val );

        if ( cur_width < 0 || ( size_t ) cur_width >= sizeof digits )
        {
            write ( "<bad-value>", sizeof "<bad-value>" - 1 );
            return self;
        }

        // since we print directly into the buffer,
        // perform our own indentation
        if ( at_line_start && indentation > 0 )
            indent ();

        // recover sign
        bool neg = digits [ 0 ] == '-';
        size_t i = neg;
        cur_width -= neg;

        // figure in sign and prefix if not base 10
        unsigned int total_width = cur_width + neg;

        // calculate the amount of padding
        unsigned int padding = 0;
        if ( total_width < ( unsigned int ) width )
            padding = ( unsigned int ) width - total_width;

        // the total field width
        total_width += padding;

        // test against what is available
        size_t marker = b . size ();
        checkSize ( marker, total_width );
        char * buffer = ( char * ) b . data ();

        // padd with spaces
        if ( padding != 0 && fill == ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // set sign
        if ( neg )
            buffer [ marker ++ ] = '-';

        // padd with zeros
        if ( padding != 0 && fill != ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // now finish with digits
        memcpy ( & buffer [ marker ], & digits [ i ], cur_width );
        marker += cur_width;

        // in wonderful C++ fashion, reset everything
        radix = 10;
        width = 0;
        precision = 0;
        fill = ' ';
        have_precision = false;
        at_line_start = false;

        b . setSize ( marker );

        return self;
    }

#if HAVE_R128
    Fmt & Fmt :: operator << ( __float128 val )
#else
    Fmt & Fmt :: operator << ( long double val )
#endif
    {
        // rely upon stdclib printf engine for the time being
        int cur_width = 0;
        char digits [ 4096 ];

#if HAVE_R128
        if ( have_precision )
            cur_width = quadmath_snprintf ( digits, sizeof digits, "%.*f", precision, val );
        else
            cur_width = quadmath_snprintf ( digits, sizeof digits, "%f", val );
#else
        if ( have_precision )
            cur_width = snprintf ( digits, sizeof digits, "%.*Lf", precision, val );
        else
            cur_width = snprintf ( digits, sizeof digits, "%Lf", val );
#endif

        if ( cur_width < 0 || ( size_t ) cur_width >= sizeof digits )
        {
            write ( "<bad-value>", sizeof "<bad-value>" - 1 );
            return self;
        }

        // since we print directly into the buffer,
        // perform our own indentation
        if ( at_line_start && indentation > 0 )
            indent ();

        // recover sign
        bool neg = digits [ 0 ] == '-';
        size_t i = neg;
        cur_width -= neg;

        // figure in sign and prefix if not base 10
        unsigned int total_width = cur_width + neg;

        // calculate the amount of padding
        unsigned int padding = 0;
        if ( total_width < ( unsigned int ) width )
            padding = ( unsigned int ) width - total_width;

        // the total field width
        total_width += padding;

        // test against what is available
        size_t marker = b . size ();
        checkSize ( marker, total_width );
        char * buffer = ( char * ) b . data ();

        // padd with spaces
        if ( padding != 0 && fill == ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // set sign
        if ( neg )
            buffer [ marker ++ ] = '-';

        // padd with zeros
        if ( padding != 0 && fill != ' ' )
        {
            memset ( & buffer [ marker ], fill, padding );
            marker += padding;
        }

        // now finish with digits
        memcpy ( & buffer [ marker ], & digits [ i ], cur_width );
        marker += cur_width;

        // in wonderful C++ fashion, reset everything
        radix = 10;
        width = 0;
        precision = 0;
        fill = ' ';
        have_precision = false;
        at_line_start = false;

        b . setSize ( marker );

        return self;
    }

    Fmt & Fmt :: operator << ( const void * val )
    {
        return self
            << hex ()
            << ( unsigned long ) val
            ;
    }

    Fmt & Fmt :: operator << ( Fmt & ( * f ) ( Fmt & f ) )
    {
        return ( * f ) ( self );
    }

    void Fmt :: put ( UTF32 c )
    {
        char * buffer;
        size_t marker;
        
        // look for need to use cr-lf
        if ( c == '\n' )
        {
            if ( use_crlf )
            {
                marker = b . size ();
                checkSize ( marker, 2 );
                buffer = ( char * ) b . data ();

                // add CR
                buffer [ marker ++ ] = '\r';

                b . setSize ( marker );
            }

            at_line_start = true;
        }

        // since we print directly into the buffer,
        // perform our own indentation, but
        // only if not printing a newline already
        else if ( at_line_start && indentation > 0 )
            indent ();

        if ( c < 128 )
        {
            // resize buffer to accept single-byte character
            marker = b . size ();
            checkSize ( marker, 1 );
            buffer = ( char * ) b . data ();

            // add character
            buffer [ marker ++ ] = c;

            b . setSize ( marker );
        }
        else
        {
            String s ( c );
            write ( s . data (), s . size () );
        }
    }

    void Fmt :: zstr ( const char * str )
    {
        if ( str == nullptr )
            str = "<null>";

        write ( str, strlen ( str ) );
    }

    void Fmt :: write ( const void * data, size_t bytes )
    {
        if ( data != 0 && bytes != 0 )
        {
            bool still_at_line_start = false;

            // since we print directly into the buffer,
            // perform our own indentation, but
            // only if not printing a newline already
            if ( at_line_start && indentation > 0 )
            {
                const char * text = ( const char * ) data;
                switch ( bytes )
                {
                case 1:
                    if ( text [ 0 ] == '\n' )
                        break;
                    else
                    {
                case 2:
                    if ( text [ 0 ] == '\r' && text [ 1 ] == '\n' )
                        break;
                    }
                    // no break;
                default:
                    indent ();
                }

                still_at_line_start = at_line_start;
            }

            size_t marker = b . size ();
            checkSize ( marker, bytes );
            char * buffer = ( char * ) b . data ();

            memcpy ( & buffer [ marker ], data, bytes );
            marker += bytes;
            at_line_start = still_at_line_start;

            b . setSize ( marker );
        }
    }

    void Fmt :: setRadix ( unsigned int _radix )
    {
        assert ( _radix > 1 && _radix <= 36 );
        radix = ( unsigned short ) _radix;
    }

    void Fmt :: setWidth ( unsigned int _width )
    {
        assert ( _width < 0x10000 );
        width = ( unsigned short ) _width;
    }

    void Fmt :: setPrecision ( unsigned int _precision )
    {
        assert ( _precision < 0x10000 );
        precision = ( unsigned short ) _precision;
        have_precision = true;
    }

    void Fmt :: setFill ( char _fill )
    {
        assert ( _fill == ' ' || _fill == '0' );
        fill = _fill;
    }

    void Fmt :: indent ( int num_tabs )
    {
        indentation += num_tabs;
        if ( indentation < 0 )
            indentation = 0;
    }

    void Fmt :: sysError ( int status )
    {
        String msg ( :: strerror ( status ) );
        write ( msg . data (), msg . size () );
        const char ERRNO [] = " ( errno = ";
        write ( ERRNO, sizeof ERRNO - 1 );
        self << ( long ) status;
        write ( " )", 2 );
    }

    Fmt :: Fmt ( FmtWriter & _writer, bool _add_eoln, bool _use_crlf )
        : writer ( _writer )
        , fill ( ' ' )
        , radix ( 10 )
        , width ( 0 )
        , precision ( 0 )
        , indentation ( 0 )
        , tabwidth ( 4 )
        , have_precision ( false )
        , at_line_start ( true )
        , add_eoln ( _add_eoln )
        , use_crlf ( _use_crlf )
        , wipe_on_flush ( true )
    {
    }

    Fmt :: ~ Fmt ()
    {
        size_t marker = b . size ();
        if ( marker != 0 )
        {
            writer . write ( ( const char * ) b . data (), marker );
            b . wipe ();
        }
        writer . flush ( true );
    }

    void Fmt :: resize ( size_t marker, size_t increase )
    {
        assert ( marker >= b . size () );
        size_t new_size = ( marker + increase + 4095 ) & ~ ( size_t ) 4095;
        if ( new_size > b . capacity () )
            b . increaseCapacity ( new_size - b . capacity () );
    }

    void Fmt :: indent ()
    {
        size_t bytes = ( unsigned int ) indentation * tabwidth;

        size_t marker = b . size ();
        checkSize ( marker, bytes );
        char * buffer = ( char * ) b . data ();

        memset ( & buffer [ marker ], ' ', bytes );
        marker += bytes;

        at_line_start = false;

        b . setSize ( marker );
    }

    Fmt & ind ( Fmt & f )
    {
        f . indent ( 1 );
        return f;
    }

    Fmt & outd ( Fmt & f )
    {
        f . indent ( -1 );
        return f;
    }

    Fmt & eoln ( Fmt & f )
    {
        f . put ( '\n' );
        assert ( f . at_line_start == true );
        return f;
    }

    Fmt & endm ( Fmt & f )
    {
        const char * data = ( char * ) f . b . data ();
        size_t bytes = f . b . size ();

        f . indentation = 0;

        if ( f . add_eoln )
        {
            if ( bytes != 0 && data [ bytes - 1 ] != '\n' )
            {
                f . put ( '\n' );

                data = ( char * ) f . b . data ();
                bytes = f . b . size ();
            }
        }

        f . radix = 10;
        f . width = 0;
        f . precision = 0;
        f . tabwidth = 4;
        f . fill = ' ';
        f . have_precision = false;
        f . at_line_start = true;

        f . writer . write ( data, bytes );

        f . b . reinitialize ( f . wipe_on_flush );

        // TBD - if a special inline function is used to record
        // the desire to wipe on flush, then this can be reset
        // to a default

        return f;
    }

    Fmt & flushm ( Fmt & f )
    {
        size_t marker = f . b . size ();
        if ( marker != 0 )
        {
            f . writer . write ( ( const char * ) f . b . data (), marker );
            f . b . reinitialize ( f . wipe_on_flush );
        }

        f . writer . flush ( false );
        return f;
    }

#if TESTING
    struct BufferWriter : FmtWriter
    {
        virtual void write ( const char * data, size_t bytes )
        {
            size_t cursz = b . size ();
            size_t needed = cursz + bytes;
            if ( needed > b . capacity () )
                b . increaseCapacity ( ( needed + 4095 ) & ~ ( size_t ) 4095 );

            :: memcpy ( ( char * ) b . data () + cursz, data, bytes );
            b . setSize ( cursz + bytes );
        }

        BufferWriter ()
            : b ( 4096 )
        {
        }

        ~ BufferWriter ()
        {
            b . wipe ();
        }

        Buffer b;
    };
#endif
}
