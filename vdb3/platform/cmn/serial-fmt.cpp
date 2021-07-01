/*

  vdb3.cmn.dserial-fmt

 */

#include <vdb3/cmn/serial.hpp>
#include <vdb3/cmn/dserial.hpp>
#include <vdb3/cmn/serial-ids.hpp>
#include <vdb3/kfc/xp.hpp>
#include <vdb3/kfc/except.hpp>
#include <vdb3/cmn/nconv.hpp>

namespace vdb3
{
    XP & operator << ( XP & xp, const Serial & s )
    {
        Deserial d ( s . content () );
        return xp << d;
    }
    
    Fmt & operator << ( Fmt & fmt, const Serial & s )
    {
        Deserial d ( s . content () );
        return fmt << d;
    }
    
    std :: ostream & operator << ( std :: ostream & o, const Serial & s )
    {
        Deserial d ( s . content () );
        return o << d;
    }

    struct MultiFmt
    {
        MSG ( void putBool ( bool val ) const );
        MSG ( void putN32 ( N32 val ) const );
        MSG ( void putN64 ( N64 val ) const );
        MSG ( void putN128 ( const N128 & val ) const );
        MSG ( void putZ32 ( Z32 val ) const );
        MSG ( void putZ64 ( Z64 val ) const );
        MSG ( void putZ128 ( const Z128 & val ) const );
        MSG ( void putR32 ( R32 val ) const );
        MSG ( void putR64 ( R64 val ) const );
        MSG ( void putBlob ( const CRgn & val ) const );
        MSG ( void putText ( const CRgn & val ) const );
        MSG ( void putCString ( const char * val ) const );
        void putParenCString ( const char * val ) const
        {
            putCString ( "<" );
            putCString ( val );
            putCString ( ">" );
        }
    };

    std :: ostream & operator << ( std :: ostream & o, const N128 & val )
    {
        N128 v ( val );
        
        size_t i;
        char buffer [ 64 ];
        buffer [ sizeof buffer - 1 ] = 0;
        for ( i = sizeof buffer - 1; i != 0; )
        {
            buffer [ -- i ] = '0' + ( ASCII ) ( N64 ) ( v % 10 );
            v /= 10;
            if ( v == 0U )
                break;
        }

        return o << & buffer [ i ];
    }

    std :: ostream & operator << ( std :: ostream & o, const Z128 & val )
    {
        if ( val < 0 )
            o << '-';
        return operator << ( o, N128 ( val ) );
    }

    template < class T >
    struct TMultiFmt : MultiFmt
    {
        METH ( void putBool ( bool val ) const ) { fmt << val; }
        METH ( void putN32 ( N32 val ) const ) { fmt << val; }
        METH ( void putN64 ( N64 val ) const ) { fmt << val; };
        METH ( void putN128 ( const N128 & val ) const ) { fmt << val; }
        METH ( void putZ32 ( Z32 val ) const ) { fmt << val; }
        METH ( void putZ64 ( Z64 val ) const ) { fmt << val; }
        METH ( void putZ128 ( const Z128 & val ) const ) { fmt << val; }
        METH ( void putR32 ( R32 val ) const ) { fmt << val; }
        METH ( void putR64 ( R64 val ) const ) { fmt << val; }
        METH ( void putBlob ( const CRgn & val ) const )
        { fmt << "octet[" << val . size () << ']'; }
        METH ( void putText ( const CRgn & val ) const )
        {
            putBlob ( val );
            fmt << "('" << CTextLiteral ( ( const UTF8 * ) val . addr (), val . size () ) << "')";
        }
        METH ( void putCString ( const char * val ) const ) { fmt << val; }

        TMultiFmt ( T & f ) : fmt ( f ) {}

        T & fmt;
    };

    static
    void putTuple ( const MultiFmt & fmt, Deserial & d, N32 id )
    {
        fmt . putCString ( "(" );
        N32 f, mbr = d . getMbrId ( f );
        fmt . putN32 ( mbr );
        fmt . putCString ( ":" );
        switch ( f )
        {
        case sfVarSize:
        case sfFixed16:
        case sfFixed32:
        case sfFixed48:
        case sfFixed64:
        {
            bool b;
            R64 rb64;
            N128 n128;
            Z128 z128;
            Ticks ticks;
            switch ( id )
            {
            case SERIAL_NATURAL:
                d . getTyped ( f, n128 );
                fmt . putN128 ( n128 );
                break;
            case SERIAL_INTEGER:
                d . getTyped ( f, z128 );
                fmt . putZ128 ( z128 );
                break;
            case SERIAL_BINARY_REAL:
                d . getTyped ( f, rb64 );
                fmt . putR64 ( rb64 );
                break;
            case SERIAL_BOOLEAN:
                d . getTyped ( f, b );
                fmt . putBool ( b );
                break;
            case SERIAL_TIME:
                d . getTyped ( f, ticks );
                fmt . putZ64 ( ticks . toZ64 () );
                break;
            }
            break;
        }
        case sfBlob:
        {
            CRgn rgn;
            d . getTyped ( f, rgn );
            fmt . putBlob ( rgn );
            break;
        }
        case sfTuple:
        {
            Deserial d2;
            d . getTyped ( f, d2 );
            putTuple ( fmt, d2, id );
            break;
        }
        case sfSeq:
        {
            count_t i, count;
            d . getTypedSeqCount ( f, count );
            fmt . putCString ( "seq[" );
            fmt . putN64 ( ( N64 ) count );
            fmt . putCString ( "]={" );
            const char * sep = "";
            if ( id == SERIAL_UNTYPED )
            {
                for ( i = 0; i < count; ++ i )
                {
                    fmt . putCString ( sep );
                    id = getTypeId ( f );
                    putTyped ( fmt, d, id, f );
                    sep = ",";
                }
            }
            else
            {
                switch ( id )
                {
                case SERIAL_NATURAL:
                    for ( i = 0; i < count; ++ i )
                    {
                        N128 n128;
                        d . get ( n128 );
                        fmt . putCString ( sep );
                        fmt . putN128 ( n128 );
                        sep = ",";
                    }
                    break;
                case SERIAL_INTEGER:
                    for ( i = 0; i < count; ++ i )
                    {
                        Z128 z128;
                        d . get ( z128 );
                        fmt . putCString ( sep );
                        fmt . putZ128 ( z128 );
                        sep = ",";
                    }
                    break;
                case SERIAL_BINARY_REAL:
                    for ( i = 0; i < count; ++ i )
                    {
                        R64 r64;
                        d . get ( r64 );
                        fmt . putCString ( sep );
                        fmt . putR64 ( r64 );
                        sep = ",";
                    }
                    break;
                case SERIAL_BOOLEAN:
                    for ( i = 0; i < count; ++ i )
                    {
                        bool b;
                        d . get ( b );
                        fmt . putCString ( sep );
                        fmt . putBool ( b );
                        sep = ",";
                    }
                    break;
                case SERIAL_TIME:
                    for ( i = 0; i < count; ++ i )
                    {
                        Ticks ticks;
                        d . get ( ticks );
                        fmt . putCString ( sep );
                        fmt . putZ64 ( ticks . toZ64 () );
                        sep = ",";
                    }
                    break;
                case SERIAL_DECIMAL_REAL:
                case SERIAL_TEXT:
                case SERIAL_FSPATH:
                case SERIAL_IPADDR:
                case SERIAL_IPENDPOINT:
                    for ( i = 0; i < count; ++ i )
                    {
                        Deserial d2;
                        d . get ( d2 );
                        fmt . putCString ( sep );
                        putTuple ( fmt, d2, id );
                        sep = ",";
                    }
                    break;
                default:
                    fmt . putCString ( "..." );
            }
            fmt . putCString ( "}" );
            break;
        }}
        fmt . putCString ( ")" );
    }

    static
    void putTyped ( const MultiFmt & fmt, Deserial & d, N32 f, N32 id )
    {
        fmt . putCString ( "(" );
        fmt . putN32 ( id );
        switch ( id )
        {
#define CASE( x ) case x: fmt . putParenCString ( #x ); break
        CASE ( SERIAL_NULL );
        CASE ( SERIAL_UNTYPED );
        CASE ( SERIAL_NATURAL );
        CASE ( SERIAL_INTEGER );
        CASE ( SERIAL_BINARY_REAL );
        CASE ( SERIAL_DECIMAL_REAL );
        CASE ( SERIAL_BOOLEAN );
        CASE ( SERIAL_TEXT );
        CASE ( SERIAL_TIME );
        CASE ( SERIAL_FSPATH );
        CASE ( SERIAL_IPADDR );
        CASE ( SERIAL_IPENDPOINT );
#undef CASE
        }
        fmt . putCString ( ":" );
        switch ( f )
        {
        case sfVarSize:
        case sfFixed16:
        case sfFixed32:
        case sfFixed48:
        case sfFixed64:
        {
            bool b;
            R64 rb64;
            N128 n128;
            Z128 z128;
            Ticks ticks;
            switch ( id )
            {
            case SERIAL_NATURAL:
                d . getTyped ( f, n128 );
                fmt . putN128 ( n128 );
                break;
            case SERIAL_INTEGER:
                d . getTyped ( f, z128 );
                fmt . putZ128 ( z128 );
                break;
            case SERIAL_BINARY_REAL:
                d . getTyped ( f, rb64 );
                fmt . putR64 ( rb64 );
                break;
            case SERIAL_BOOLEAN:
                d . getTyped ( f, b );
                fmt . putBool ( b );
                break;
            case SERIAL_TIME:
                d . getTyped ( f, ticks );
                fmt . putZ64 ( ticks . toZ64 () );
                break;
            }
            break;
        }
        case sfBlob:
        {
            CRgn rgn;
            d . getTyped ( f, rgn );
            fmt . putBlob ( rgn );
            break;
        }
        case sfTuple:
        {
            Deserial d2;
            d . getTyped ( f, d2 );
            putTuple ( fmt, d2, id );
            break;
        }
        case sfSeq:
        {
            count_t i, count;
            d . getTypedSeqCount ( f, count );
            fmt . putCString ( "seq[" );
            fmt . putN64 ( ( N64 ) count );
            fmt . putCString ( "]={" );
            const char * sep = "";
            if ( id == SERIAL_UNTYPED )
            {
                for ( i = 0; i < count; ++ i )
                {
                    fmt . putCString ( sep );
                    id = getTypeId ( f );
                    putTyped ( fmt, d, id, f );
                    sep = ",";
                }
            }
            else
            {
                switch ( id )
                {
                case SERIAL_NATURAL:
                    for ( i = 0; i < count; ++ i )
                    {
                        N128 n128;
                        d . get ( n128 );
                        fmt . putCString ( sep );
                        fmt . putN128 ( n128 );
                        sep = ",";
                    }
                    break;
                case SERIAL_INTEGER:
                    for ( i = 0; i < count; ++ i )
                    {
                        Z128 z128;
                        d . get ( z128 );
                        fmt . putCString ( sep );
                        fmt . putZ128 ( z128 );
                        sep = ",";
                    }
                    break;
                case SERIAL_BINARY_REAL:
                    for ( i = 0; i < count; ++ i )
                    {
                        R64 r64;
                        d . get ( r64 );
                        fmt . putCString ( sep );
                        fmt . putR64 ( r64 );
                        sep = ",";
                    }
                    break;
                case SERIAL_BOOLEAN:
                    for ( i = 0; i < count; ++ i )
                    {
                        bool b;
                        d . get ( b );
                        fmt . putCString ( sep );
                        fmt . putBool ( b );
                        sep = ",";
                    }
                    break;
                case SERIAL_TIME:
                    for ( i = 0; i < count; ++ i )
                    {
                        Ticks ticks;
                        d . get ( ticks );
                        fmt . putCString ( sep );
                        fmt . putZ64 ( ticks . toZ64 () );
                        sep = ",";
                    }
                    break;
                case SERIAL_DECIMAL_REAL:
                case SERIAL_TEXT:
                case SERIAL_FSPATH:
                case SERIAL_IPADDR:
                case SERIAL_IPENDPOINT:
                    for ( i = 0; i < count; ++ i )
                    {
                        Deserial d2;
                        d . get ( d2 );
                        fmt . putCString ( sep );
                        putTuple ( fmt, d2, id );
                        sep = ",";
                    }
                    break;
                default:
                    fmt . putCString ( "..." );
            }
            fmt . putCString ( "}" );
            break;
        }}
        fmt . putCString ( ")" );
    }

    static
    void putDeserial ( const MultiFmt & fmt, const Deserial & _d )
    {
        Deserial d ( _d );
        N32 f, id = d . getTypeId ( f );
        putTyped ( fmt, d, f, id );
    }

    XP & operator << ( XP & xp, const Deserial & d )
    {
        TMultiFmt < XP > f ( xp );
        putDeserial ( f, d );
        return xp;
    }
    
    Fmt & operator << ( Fmt & fmt, const Deserial & d )
    {
        TMultiFmt < Fmt > f ( fmt );
        putDeserial ( f, d );
        return fmt;
    }
    
    std :: ostream & operator << ( std :: ostream & o, const Deserial & d )
    {
        TMultiFmt < std :: ostream > f ( o );
        putDeserial ( f, d );
        return o;
    }
    
}
