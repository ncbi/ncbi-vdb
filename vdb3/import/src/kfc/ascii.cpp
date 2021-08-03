/*

  vdb3.kfc.ascii

 */

#include <vdb3/kfc/ascii.hpp>
#include <vdb3/kfc/ktrace.hpp>

namespace vdb3
{

    /*=====================================================*
     *                      ASCIIText                      *
     *=====================================================*/
    
    ASCIIText :: ASCIIText ( const ASCII7 * str, size_t _sz ) noexcept
        : TextRgn ( str, _sz, ( count_t ) _sz )
    {
        if ( str == nullptr )
        {
            const ASCII7 null_string [] = "<null-string>";
            a = const_cast < UTF8 * > ( null_string );
            sz = sizeof null_string - 1;
            cnt = ( count_t ) ( sizeof null_string - 1 );
        }
        else
        {
            const signed char * p = reinterpret_cast < const signed char * > ( str );
            for ( size_t i = 0; i < _sz; ++ i )
            {
                if ( p [ i ] < 0 )
                {
                    const ASCII7 bad_string [] = "<non-ASCII-string>";
                    a = const_cast < UTF8 * > ( bad_string );
                    sz = sizeof bad_string - 1;
                    cnt = ( count_t ) ( sizeof bad_string - 1 );
                    break;
                }
            }
        }
        assert ( ( count_t ) sz == cnt );
    }

    ASCIIText & ASCIIText :: operator = ( const ASCII7 * zstr ) noexcept
    {
        ASCIIText txt ( zstr );
        TextRgn :: operator = ( txt );
        assert ( ( count_t ) sz == cnt );
        return self;
    }

    ASCIIText :: ASCIIText ( const ASCII7 * zstr ) noexcept
        : TextRgn ( zstr, 0, 0 )
    {
        if ( zstr == nullptr )
        {
            const ASCII7 null_string [] = "<null-string>";
            a = const_cast < UTF8 * > ( null_string );
            sz = sizeof null_string - 1;
            cnt = ( count_t ) ( sizeof null_string - 1 );
        }
        else
        {
            size_t i;
            const signed char * p = reinterpret_cast < const signed char * > ( zstr );
            for ( i = 0; p [ i ] != 0; ++ i )
            {
                if ( p [ i ] < 0 )
                {
                    const ASCII7 bad_string [] = "<non-ASCII-string>";
                    a = const_cast < UTF8 * > ( bad_string );
                    sz = sizeof bad_string - 1;
                    cnt = ( count_t ) ( sizeof bad_string - 1 );
                    assert ( ( count_t ) sz == cnt );
                    return;
                }
            }
            
            sz = i;
            cnt = ( count_t ) i;
        }
        assert ( ( count_t ) sz == cnt );
    }
    
    /*=====================================================*
     *                      ASCIIChar                      *
     *=====================================================*/
    
    ASCIIChar & ASCIIChar :: operator = ( ASCII7 _ch ) noexcept
    {
        ch [ 0 ] = ( ( signed char ) _ch >= 0 ) ? _ch : '?';
        return self;
    }

    ASCIIChar :: ASCIIChar ( ASCII7 _ch ) noexcept
        : TextRgn ( ch, 1, 1 )
    {
        ch [ 0 ] = ( ( signed char ) _ch >= 0 ) ? _ch : '?';
    }
}
