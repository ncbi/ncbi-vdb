/*

  vdb3.cmn.nconv

 */

#include <vdb3/cmn/nconv.hpp>
#include <vdb3/cmn/auto-rgn.hpp>

#include <stdlib.h>
#include <ctype.h>

#if HAVE_QUADMATH
#include <quadmath.h>
#endif

namespace vdb3
{
    
    /*=====================================================*
     *                     realToText                      *
     *=====================================================*/

    static
    MText trimTrailingZeros ( MText & sub )
    {
        // look for a dot
        CharLoc dot = sub . fwdFind ( '.' );
        if ( sub . contains ( dot ) )
        {
            // don't bother if it went into scientific notation
            CharLoc eloc = sub . findLastOf ( CTextLiteral ( "Ee" ) );
            if ( ! sub . contains ( eloc ) )
            {
                // trim off trailing zeros
                // leave at least 1 fractional digit
                    
                // set stop at digit following dot
                CharLoc stop = sub . nextLoc ( dot );

                // walk backward
                CharLoc it = sub . end ();
                for ( it = sub . prevLoc ( it ); it != stop; it = sub . prevLoc ( it ) )
                {
                    if ( sub . getChar ( it ) != '0' )
                        break;
                    sub . removeChar ( it );
                }

                // ??? - should I remove the ".0" for general purpose?
                // I think this is the meaning of general purpose, which
                // is to act as an integer OR floating point, but here
                // we're trying to keep them more specific.
            }
        }

        return sub;
    }
    
    MText NConv :: R64ToText ( MText & mrgn, R64 val, Z32 prec ) const
    {
        if ( prec < 0 )
        {
            // let libc do its thing
            MText sub = mrgn . format ( "%g", val );

            // fix it up
            return trimTrailingZeros ( sub );
        }

        return mrgn . format ( "%.*f", ( N32 ) prec, val );
    }

#if HAVE_R128
    MText NConv :: R128ToText ( MText & mrgn, R128 val, Z32 prec ) const
    {
#if LONG_DOUBLE_IS_NOT_R128 && HAVE_QUADMATH

        // pull out the buffer
        char * buf = mrgn . addr ();
        size_t cap = mrgn . capacity ();

        // this would be reasonable to place inside MText, except that
        // the quadmath people don't have a va_list version... incredible!
        // so we have to do it inline
        int status;
        if ( prec < 0 )
            status = :: quadmath_snprintf ( buf, cap, "%Qg", val );
        else
            status = :: quadmath_snprintf ( buf, cap, ".*%Qf", ( N32 ) prec, val );

        // respond to errors
        if ( status < 0 )
        {
            status = errno;
            
            mrgn . fill ( 0 );

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
            mrgn . fill ( 0 );

            throw LogicException (
                XP ( XLOC )
                << xprob
                << "failed to print into MText region"
                << xcause
                << "insufficient buffer"
                );
        }

        // restore the region as it might have been done within MText
        mrgn = makeMText ( buf, cap, ( size_t ) status, ( count_t ) status );

        // now, continue
        if ( prec < 0 )
            return trimTrailingZeros ( mrgn );

        return mrgn;
        
#else // LONG_DOUBLE_IS_NOT_R128
        if ( prec < 0 )
        {
            // let libc do its thing
            MText sub = mrgn . format ( "%Lg", ( long double ) val );

            /*

              BY THE WAY...

              I've about had it with this whole "short, long, long long, ..."
              garbage introduced so many years ago and propagated to the present.

              While "double" is standardized as 64-bit, long double is either
              synonymous with double (great, but in keeping with these stupid
              conventions) or it might mean one of these Intel bastard children
              with 96 bits (or the older Motorola with 80 bits). On ARM, long
              double means what you might expect - 128 bits.

              Within libc, because of all of the Eunichs stuff above, "%f" means
              "double" ( not float ), and "%ld" sort of means that too. The Intel
              world has their "%Qf" for 128-bit, and on ARM they couldn't use
              "%lf" to mean long double. So instead, they use "%Lf". What a
              mess...

             */

            // fix it up
            return trimTrailingZeros ( sub );
        }

        return mrgn . format ( "%.*Lf", ( N32 ) prec, ( long double ) val );
#endif
    }
#endif

    MText NConv :: DecFlt64ToText ( MText & mrgn,
        const DecFltPoint < Z64 > & v, Z32 prec ) const
    {
        // going to mangle the quantity
        DecFltPoint < Z64 > val ( v );

        // take absolute value
        bool negative = false;
        if ( val < 0 )
        {
            negative = true;
            val . negate ();
        }

        // use the exponent
        const Z32 exp = val . getExponent ();

        // get the integer portion
        Z64 ival = exp >= 0 ? val . getMantissa64 () : val . toZ64 ();

        // get at the buffer
        ASCII * const buffer = mrgn . addr ();
        const size_t sizeof_buffer = mrgn . capacity ();

        // generate the integer
        index_t i;
        for ( i = sizeof_buffer - 1; i >= 0; -- i )
        {
            buffer [ i ] = ( ASCII ) ( ival % 10 + '0' );
            ival /= 10;
            if ( ival == 0 )
                break;
        }

        // prepend sign
        assert ( i > 0 );
        if ( negative )
            buffer [ -- i ] = '-';

        // size of integer portion
        const size_t isize = sizeof_buffer - ( size_t ) i;

        // normalize this portion
        mrgn . copy ( makeCText ( & buffer [ i ], isize, ( count_t ) isize ) );

        // extend with zeros if mantissa too small for integer
        if ( exp > 0 )
        {
            MText ext = mrgn . subRgn ( mrgn . count () );
            ext . fill ( '0', exp );
            mrgn = mrgn . join ( ext );
        }

        // get precision
        N32 precision = prec < 0 ? 18 : prec;

        // generate fraction
        if ( precision != 0 )
        {
            // separator
            mrgn . appendChar ( '.' );

            // detect no fraction
            if ( exp >= 0 )
            {
                MText ext = mrgn . subRgn ( mrgn . count () );
                ext . fill ( '0', precision );
                mrgn = mrgn . join ( ext );
            }
            else
            {
                // drop integer portion
                val . frac ();

                // limit precision to 18 decimal places
                N32 over_precision = 0;
                if ( precision > 18 )
                {
                    over_precision = precision - 18;
                    precision = 18;
                }

                // generate a multiplication factor
                Z64 factor;
                if ( precision == 6 )
                    factor = 1000000;
                else
                {
                    factor = 1;
                    
                    N32 j;
                    for ( j = 6; j <= precision; j += 6 )
                        factor *= 1000000;
                    for ( j -= 6; j < precision; ++ j )
                        factor *= 10;
                }

                // scale by factor
                val *= factor;

                // generate a stop index
                index_t stop = sizeof_buffer - precision;
                if ( ( size_t ) stop < mrgn . size () )
                    stop = ( index_t ) mrgn . size ();

                // convert to ASCII
                Z64 fval = val . toZ64 ();
                for ( i = sizeof_buffer - 1; i >= stop; -- i )
                {
                    buffer [ i ] = ( ASCII ) ( fval % 10 + '0' );
                    fval /= 10;
                }
                
                // size of fraction portion
                const size_t fsize = sizeof_buffer - ( size_t ) stop;

                // copy into place
                mrgn . append ( makeCText ( & buffer [ stop ], fsize, ( count_t ) fsize ) );

                // now finally, extend if there is more precision needed
                if ( over_precision != 0 )
                {
                    MText ext = mrgn . subRgn ( mrgn . count () );
                    ext . fill ( '0', over_precision );
                    mrgn = mrgn . join ( ext );
                }
            }

            if ( prec < 0 )
                return trimTrailingZeros ( mrgn );
        }

        return mrgn;
    }
    
    MText NConv :: DecFlt128ToText ( MText & mrgn,
        const DecFltPoint < Z128 > & v, Z32 prec ) const
    {
        // going to mangle the quantity
        DecFltPoint < Z128 > val ( v );

        // take absolute value
        bool negative = false;
        if ( val < 0 )
        {
            negative = true;
            val . negate ();
        }

        // use the exponent
        const Z32 exp = val . getExponent ();

        // get the integer portion
        Z128 ival;
        ival = exp >= 0 ? val . getMantissa128 () : val . toZ128 ();

        // get at the buffer
        ASCII * const buffer = mrgn . addr ();
        const size_t sizeof_buffer = mrgn . capacity ();

        // generate the integer
        index_t i;
        for ( i = sizeof_buffer - 1; i >= 0; -- i )
        {
            buffer [ i ] = ( ASCII ) ( ( Z64 ) ( ival % 10 ) + '0' );
            ival /= 10;
            if ( ival == 0 )
                break;
        }

        // prepend sign
        assert ( i > 0 );
        if ( negative )
            buffer [ -- i ] = '-';

        // size of integer portion
        const size_t isize = sizeof_buffer - ( size_t ) i;

        // normalize this portion
        mrgn . copy ( makeCText ( & buffer [ i ], isize, ( count_t ) isize ) );

        // extend with zeros if mantissa too small for integer
        if ( exp > 0 )
        {
            MText ext = mrgn . subRgn ( mrgn . count () );
            ext . fill ( '0', exp );
            mrgn = mrgn . join ( ext );
        }

        // get precision
        N32 precision = prec < 0 ? 38 : prec;

        // generate fraction
        if ( precision != 0 )
        {
            // separator
            mrgn . appendChar ( '.' );

            // detect no fraction
            if ( exp >= 0 )
            {
                MText ext = mrgn . subRgn ( mrgn . count () );
                ext . fill ( '0', precision );
                mrgn = mrgn . join ( ext );
            }
            else
            {
                // drop integer portion
                val . frac ();

                // limit precision to 38 decimal places
                N32 over_precision = 0;
                if ( precision > 38 )
                {
                    over_precision = precision - 38;
                    precision = 38;
                }

                // scale by precision
                if ( precision == 6 )
                    val *= 1000000;
                else
                {
                    Z64 fact64 = 1;
                    
                    N32 j;
                    for ( j = 6; j <= precision && j <= 18; j += 6 )
                        fact64 *= 1000000;
                    for ( j -= 6; j < precision && j < 18; ++ j )
                        fact64 *= 10;

                    Z128 fact128 ( fact64 );
                    for ( ; j < precision; ++ j )
                        fact128 *= 10;

                    val *= fact128;
                }

                // generate a stop index
                index_t stop = sizeof_buffer - precision;
                if ( ( size_t ) stop < mrgn . size () )
                    stop = ( index_t ) mrgn . size ();

                // convert to ASCII
                Z128 fval;
                fval = val . toZ128 ();
                for ( i = sizeof_buffer - 1; i >= stop; -- i )
                {
                    buffer [ i ] = ( ASCII ) ( ( Z64 ) ( fval % 10 ) + '0' );
                    fval /= 10;
                }
                
                // size of fraction portion
                size_t fsize = sizeof_buffer - ( size_t ) stop;

                // copy into place
                mrgn . append ( makeCText ( & buffer [ stop ], fsize, ( count_t ) fsize ) );

                // now finally, extend if there is more precision needed
                if ( over_precision != 0 )
                {
                    MText ext = mrgn . subRgn ( mrgn . count () );
                    ext . fill ( '0', over_precision );
                    mrgn = mrgn . join ( ext );
                }
            }

            if ( prec < 0 )
                return trimTrailingZeros ( mrgn );
        }
        
        return mrgn;
    }
    

    /*=====================================================*
     *                      intToText                      *
     *=====================================================*/

    const ASCII lower_numerals [] =
        "0123456789abcdefghijklmnopqrstuvwxyz";

    [[ noreturn ]] static
    void badBase ( const char * func, unsigned int lineno, N32 base )
    {
        throw ConstraintViolation (
            XP ( __FILE__, func, lineno, rc_logic_err )
            << xprob
            << "failed to convert string to integer"
            << xcause
            << "base spec ( "
            << base
            << " ) is out of range 2..36"
            );
    }
#define CHECK_BASE( base )                        \
    do {                                          \
        if ( base < 2 || base > 36 )              \
            badBase ( __func__, __LINE__, base ); \
    } while ( 0 )

    [[ noreturn ]] static
    void outOfRgnSpace ( const char * func, unsigned int lineno )
    {
        throw InsufficientBuffer (
            XP ( __FILE__, func, lineno )
            << xprob
            << "failed to format integer"
            << xcause
            << "output region insufficient"
            );
    }

#define CHECK_RGN_SPACE( i, negative )            \
    do {                                          \
        if ( i < 0 || ( i == 0 && negative ) )    \
            outOfRgnSpace ( __func__, __LINE__ ); \
    } while ( 0 )

    template < class Z, class N, class C >
    MText NConv :: ZtoText ( MText & mrgn, const Z & sval, N32 base ) const
    {
        CHECK_BASE ( base );

        // take care of sign
        N val ( sval );
        bool negative = false;
        if ( sval < 0 && base == 10 )
        {
            val = - sval;
            negative = true;;
        }

        // perform manipulation directly in memory
        // because we fill from right to left
        MRgn dst = mrgn . toMRgn ();
        assert ( dst . size () == mrgn . capacity () );
        
        index_t i = ( index_t ) dst . size ();
        ASCII * buffer = ( ASCII * ) dst . addr ();
        for ( -- i; i >= 0; -- i )
        {
            buffer [ i ] = lower_numerals [ ( C ) ( val % base ) ];
            val /= base;
            if ( val == 0U )
                break;
        }

        // detect running out of space
        CHECK_RGN_SPACE ( i, negative );
        
        if ( negative )
            buffer [ -- i ] = '-';

        // the actual bytes ( ASCII characters )
        size_t sz = dst . size () - ( size_t ) i;

        // if we can get away with returning as-is, do
        if ( ! normalize_mrgn || i == 0 )
            return makeMText ( & buffer [ i ], sz, sz, ( count_t ) sz );

        // under normal circumstances, want to left align
        mrgn . copy ( makeCText ( & buffer [ i ], sz, ( count_t ) sz ) );
        dst . subRgn ( sz ) . fill ( 0 );

        return mrgn;
    }
    
    MText NConv :: Z16ToText ( MText & mrgn, Z16 val, N32 base ) const
    {
        return ZtoText < Z16, N16, N32 > ( mrgn, val, base );
    }

    MText NConv :: Z32ToText ( MText & mrgn, Z32 val, N32 base ) const
    {
        return ZtoText < Z32, N32, N32 > ( mrgn, val, base );
    }

    MText NConv :: Z64ToText ( MText & mrgn, Z64 val, N32 base ) const
    {
        return ZtoText < Z64, N64, N32 > ( mrgn, val, base );
    }

#if HAVE_Z128
    MText NConv :: Z128ToText ( MText & mrgn, Z128 val, N32 base ) const
    {
        return ZtoText < Z128, N128, N32 > ( mrgn, val, base );
    }
#else
    MText NConv :: Z128ToText ( MText & mrgn, const Z128 & val, N32 base ) const
    {
        return ZtoText < Z128, N128, N64 > ( mrgn, val, base );
    }
#endif

    template < class N, class C >
    MText NConv :: NtoText ( MText & mrgn, const N & v, N32 base ) const
    {
        CHECK_BASE ( base );

        N val ( v );

        // perform manipulation directly in memory
        // because we fill from right to left
        MRgn dst = mrgn . toMRgn ();
        
        index_t i = ( index_t ) dst . size ();
        ASCII * buffer = ( ASCII * ) dst . addr ();
        for ( -- i; i >= 0; -- i )
        {
            buffer [ i ] = lower_numerals [ ( C ) ( val % base ) ];
            val /= base;
            if ( val == 0U )
                break;
        }

        // detect running out of space
        CHECK_RGN_SPACE ( i, false );

        size_t sz = dst . size () - ( size_t ) i;

        // if we can get away with returning as-is, do
        if ( ! normalize_mrgn || i == 0 )
            return makeMText ( & buffer [ i ], sz, sz, ( count_t ) sz );

        // under normal circumstances, want to left align
        mrgn . copy ( makeCText ( & buffer [ i ], sz, ( count_t ) sz ) );
        dst . subRgn ( sz ) . fill ( 0 );

        return mrgn;
    }

#undef CHECK_RGN_SPACE
#undef CHECK_BASE

    MText NConv :: N16ToText ( MText & mrgn, N16 val, N32 base ) const
    {
        return NtoText < N16, N32 > ( mrgn, val, base );
    }
    
    MText NConv :: N32ToText ( MText & mrgn, N32 val, N32 base ) const
    {
        return NtoText < N32, N32 > ( mrgn, val, base );
    }
    
    MText NConv :: N64ToText ( MText & mrgn, N64 val, N32 base ) const
    {
        return NtoText < N64, N32 > ( mrgn, val, base );
    }
    
#if HAVE_Z128
    MText NConv :: N128ToText ( MText & mrgn, N128 val, N32 base ) const
    {
        return NtoText < N128, N32 > ( mrgn, val, base );
    }
#else
    MText NConv :: N128ToText ( MText & mrgn, const N128 & val, N32 base ) const
    {
        return NtoText < N128, N64 > ( mrgn, val, base );
    }
#endif

    /*=====================================================*
     *                     boolToText                      *
     *=====================================================*/

    CText NConv :: boolToText ( bool val ) const
    {
        return CTextLiteral ( val ? "true" : "false" );
    }

    MText NConv :: boolToText ( MText & mrgn, bool val ) const
    {
        return mrgn . copy ( CTextLiteral ( val ? "true" : "false" ) );
    }

    String NConv :: boolToString ( bool val ) const
    {
        return String ( val ? "true" : "false" );
    }

    /*=====================================================*
     *                     textToReal                      *
     *=====================================================*/

    template < class R, class F > static
    R textToR ( const CText & str, F ( * f ) ( const char *, char ** ) )
    {
        // absurdly large buffer
        char buffer [ 4 * 1024 ], * end;
        if ( str . size () >= sizeof buffer )
        {
            throw LengthViolation (
                XP ( XLOC )
                << xprob
                << "failed to convert floating point numeral"
                << xcause
                << "length ( "
                << str . size ()
                << " ) exceeds length constraint ( "
                << sizeof buffer
                << " )"
                );
        }
        
        // copy text
        :: memcpy ( buffer, str . addr (), str . size () );
        
        // all to get a $%^& terminating NUL byte on there
        buffer [ str . size () ] = 0;
        
        R val = f ( buffer, & end );
        if ( end == buffer || * end != 0 )
        {
            throw InvalidArgument (
                XP ( XLOC )
                << xprob
                << "failed to convert floating point numeral"
                << xcause
                << "malformed floating point"
                );
        }

        return val;
    }
    
    R32 NConv :: textToR32 ( const CText & str ) const
    {
#if NCONV_SEL_TEST
        return 1;
#else
        return textToR < R32, float > ( str, strtof );
#endif
    }

    R64 NConv :: textToR64 ( const CText & str ) const
    {
#if NCONV_SEL_TEST
        return 2;
#else
        return textToR < R64, double > ( str, strtod );
#endif
    }

#if HAVE_R128
    R128 NConv :: textToR128 ( const CText & str ) const
    {
#if NCONV_SEL_TEST
        return 3;
#else
        // this is almost certainly wrong
        // need another function for doing 128-bit...
        return textToR < R128, long double > ( str, strtold );
#endif
    }
#endif

    /*=====================================================*
     *                      textToInt                      *
     *=====================================================*/

#if ! NCONV_SEL_TEST
    static
    CText fixBase ( const char * func, unsigned int lineno,
        const CText & cstr, N32 & base )
    {
        // mutable copy
        CText str = cstr;
        
        // if base is magical value of 0, we need to guess
        if ( base == 0 )
        {
            base = 10;

            try
            {
                // will blow an exception if empty
                if ( str . firstChar () == '0' )
                {
                    // don't trigger exceptions after this
                    if ( str . count () == 1 )
                        return str;

                    bool has_prefix = true;
                    switch ( str . getChar ( 1 ) )
                    {
                    case 'b':
                        base = 2;
                        break;
                    case 'o':
                        base = 8;
                        break;
                    case 'd':
                        base = 10;
                        break;
                    case 'x':
                    case 'X':
                        base = 16;
                        break;
                    default:
                        has_prefix = false;
                    }

                    // strip off prefix
                    if ( has_prefix )
                        str = str . subRgn ( 2 );
                }
            }
            catch ( BoundsException & x )
            {
                throw InvalidNumeral (
                    XP ( __FILE__, func, lineno )
                    << "empty numeric string"
                    );
            }
        }

        return str;
    }

#define FIX_BASE( str, base ) \
    fixBase ( __func__, __LINE__, str, base )

    static
    CText fixBaseAndSign ( const char * func, unsigned int lineno,
        const CText & cstr, N32 & base, bool & negative )
    {
        // mutable copy
        CText str = cstr;

        // has to have at least one character
        // look for a sign
        bool has_sign = false;
        try
        {
            UTF32 ch = str . firstChar ();
            if ( ch == '-' || ch == '+' )
            {
                has_sign = true;
                negative = ( ch == '-' );
                str = str . subRgn ( 1 );
            }
        }
        catch ( BoundsException & x )
        {
            throw InvalidNumeral (
                XP ( __FILE__, func, lineno )
                << "empty numeric string"
                );
        }
        
        str = fixBase ( func, lineno, str, base );
        
        if ( has_sign && base != 10 )
        {
            throw InvalidNumeral (
                XP ( __FILE__, func, lineno )
                << xprob
                << "failed to parse numeric string at index 0"
                << xcause
                << "sign applied to non-decimal numeral"
                );
        }

        return str;
    }

#define FIX_BASE_AND_SIGN( str, base, negative ) \
    fixBaseAndSign ( __func__, __LINE__, str, base, negative )

    static
    int getDigit ( const char * func, unsigned int lineno,
        const CText & str, const CharLoc & pos, N32 base )
    {
        UTF32 ch = str . getChar ( pos );
        
        // anything that is not alphanumeric is garbage
        if ( ch >= 128 || ! :: isalnum ( ( int ) ch ) )
        {
            throw InvalidNumeral (
                XP ( __FILE__, func, lineno )
                << xprob
                << "failed to parse numeric string"
                << xcause
                << "expected alphanumeric digit"
                );
        }
        
        int bin = ( ch <= '9' ) ? ch - '0' : :: tolower ( ( int ) ch ) - 'a' + 10;
        assert ( bin >= 0 );
        if ( ( N32 ) bin >= base )
        {
            throw InvalidNumeral (
                XP ( __FILE__, func, lineno )
                << xprob
                << "failed to parse numeric string"
                << xcause
                << "expected alphanumeric digit"
                );
        }

        return bin;
    }

#define GETDIGIT( str, pos, base ) \
    getDigit ( __func__, __LINE__, str, pos, base )

    [[ noreturn ]] static
    void numericOverflow ( const char * func, unsigned int lineno )
    {
        throw OverflowException (
            XP ( __FILE__, func, lineno )
            << xprob
            << "failed to parse numeric string"
            << xcause
            << "overflow of output format"
            );
    }

#define TEST_MULTIPLY_LIMIT( val, multiply_limit )  \
    do {                                            \
        if ( val > multiply_limit )                 \
            numericOverflow ( __func__, __LINE__ ); \
    } while ( 0 )

#define TEST_ADD_LIMIT( val, digit, add_max )       \
    do {                                            \
        if ( val > ( add_max - digit ) )            \
            numericOverflow ( __func__, __LINE__ ); \
    } while ( 0 )

    template < class Z, class N > static
    Z textToZ ( const CText & cstr, N32 base, const N nmax )
    {
        // mutable copy
        N32 base_ref = base;
        bool negative = false;
        CText str = FIX_BASE_AND_SIGN ( cstr, base_ref, negative );

        // move back to register variable
        base = base_ref;
        
        N val = 0;

        const N multiply_limit = nmax / base;
        const N add_max = nmax + negative;

        // "pos" is initialized to -1 by default
        CharLoc pos;

        // walk from 0..N-1, testing for inclusion in string
        for ( pos = str . nextLoc ( pos ); str . contains ( pos ); pos = str . nextLoc ( pos ) )
        {
            // read UNICODE character at position
            // convert it to binary in current base
            int digit = GETDIGIT ( str, pos, base );

            // test "val" for overflow on multiplication
            TEST_MULTIPLY_LIMIT ( val, multiply_limit );
            val *= base;

            // test "val" for overflow on addition
            TEST_ADD_LIMIT ( val, digit, add_max );
            val += digit;
        }

        return negative ? - ( Z ) val : ( Z ) val;

    }

    template < class N > static
    N textToN ( const CText & cstr, N32 base, const N nmax )
    {
        // mutable copy
        N32 base_ref = base;
        CText str = FIX_BASE ( cstr, base_ref );
        base = base_ref;
        
        N val = 0;

        const N multiply_limit = nmax / base;
        const N add_max = nmax;

        // "pos" is initialized to -1 by default
        CharLoc pos;

        // walk from 0..N-1, testing for inclusion in string
        for ( pos = str . nextLoc ( pos ); str . contains ( pos ); pos = str . nextLoc ( pos ) )
        {
            // read UNICODE character at position
            // convert it to binary in current base
            int digit = GETDIGIT ( str, pos, base );

            // test "val" for overflow on multiplication
            TEST_MULTIPLY_LIMIT ( val, multiply_limit );
            val *= base;

            // test "val" for overflow on addition
            TEST_ADD_LIMIT ( val, digit, add_max );
            val += digit;
        }

        return val;
    }

#endif // ! NCONV_SEL_TEST
    
    Z16 NConv :: textToZ16 ( const CText & str, N32 base ) const
    {
#if NCONV_SEL_TEST
        return -1;
#else
        return textToZ < Z16, N16 > ( str, base, Z16_MAX );
#endif
    }

    Z32 NConv :: textToZ32 ( const CText & str, N32 base ) const
    {
#if NCONV_SEL_TEST
        return -2;
#else
        return textToZ < Z32, N32 > ( str, base, Z32_MAX );
#endif
    }

    Z64 NConv :: textToZ64 ( const CText & str, N32 base ) const
    {
#if NCONV_SEL_TEST
        return -3;
#else
        return textToZ < Z64, N64 > ( str, base, Z64_MAX );
#endif
    }

    Z128 NConv :: textToZ128 ( const CText & cstr, N32 base ) const
    {
#if NCONV_SEL_TEST
        return Z128 ( -4 );
#elif HAVE_Z128
        return textToZ < Z128, N128 > ( cstr, base, Z128_MAX );
#else
        // mutable copy
        N32 base_ref = base;
        bool negative = false;
        CText str = FIX_BASE_AND_SIGN ( cstr, base_ref, negative );

        // move back to register variable
        base = base_ref;
        
        N128 val ( 0 );

        const N128 multiply_limit ( Z128_MAX / base );
        const N128 add_max ( ( N128 ) Z128_MAX + negative );

        // "pos" is initialized to -1 by default
        CharLoc pos;

        // walk from 0..N-1, testing for inclusion in string
        for ( pos = str . nextLoc ( pos ); str . contains ( pos ); pos = str . nextLoc ( pos ) )
        {
            // read UNICODE character at position
            // convert it to binary in current base
            int digit = GETDIGIT ( str, pos, base );

            // test "val" for overflow on multiplication
            TEST_MULTIPLY_LIMIT ( val, multiply_limit );
            val *= base;

            // test "val" for overflow on addition
            TEST_ADD_LIMIT ( val, digit, add_max );
            val += digit;
        }

        // in the case that "val" is already negative,
        // which can happen if the incoming numeral is decimal Z128_MIN,
        // TEST_ADD_LIMIT will allow "val" to go to N128
        // Z128_MAX + 1, which will look negative already.
        Z128 rtn ( val );

        // when this is negated, it should still allow for reaching Z128_MIN
        // which is wonderfully 1 beyond Z128_MAX in magnitude
        if ( negative )
            rtn . negate ();
        
        return rtn;
#endif
    }

    N16 NConv :: textToN16 ( const CText & str, N32 base ) const
    {
#if NCONV_SEL_TEST
        return 1U;
#else
        return textToN < N16 > ( str, base, N16_MAX );
#endif
    }

    N32 NConv :: textToN32 ( const CText & str, N32 base ) const
    {
#if NCONV_SEL_TEST
        return 2U;
#else
        return textToN < N32 > ( str, base, N32_MAX );
#endif
    }

    N64 NConv :: textToN64 ( const CText & str, N32 base ) const
    {
#if NCONV_SEL_TEST
        return 3U;
#else
        return textToN < N64 > ( str, base, N64_MAX );
#endif
    }

    N128 NConv :: textToN128 ( const CText & str, N32 base ) const
    {
#if NCONV_SEL_TEST
        return N128 ( 4U );
#else
        return N128 ( 0 );
#endif
    }

    /*=====================================================*
     *                     textToBool                      *
     *=====================================================*/

    bool NConv :: textToBool ( const CText & str ) const
    {
        if ( str . caseInsensitiveCompare ( CTextLiteral ( "true" ) ) == 0 )
            return true;
        if ( str . caseInsensitiveCompare ( CTextLiteral ( "false" ) ) == 0 )
            return false;

        if ( str . caseInsensitiveCompare ( CTextLiteral ( "yes" ) ) == 0 )
            return true;
        if ( str . caseInsensitiveCompare ( CTextLiteral ( "no" ) ) == 0 )
            return false;

        if ( str . caseInsensitiveCompare ( CTextLiteral ( "T" ) ) == 0 )
            return true;
        if ( str . caseInsensitiveCompare ( CTextLiteral ( "F" ) ) == 0 )
            return false;

        throw InvalidInputException (
            XP ( XLOC )
            << xprob
            << "could not convert string '"
            << str
            << "' to Boolean value"
            << xcause
            << "not member of controlled vocabulary"
            );
    }
    
#undef TEST_ADD_LIMIT
#undef TEST_MULTIPLY_LIMIT
#undef GETDIGIT
#undef FIX_BASE_AND_SIGN
#undef FIX_BASE
    

    // streaming operators that are possible now
    template < class T >
    std :: ostream & operator << ( std :: ostream & o, const DecFltPoint < T > & val )
    {
        AutoRgn < 1024 > argn;
        MText mrgn = argn . getMText ();
        CText txt = NConv () . realToText < DecFltPoint < T > > ( mrgn, val );
        return o << txt;
    }

    template std :: ostream & operator << < Z64 >
        ( std :: ostream & o, const DecFltPoint < Z64 > & val );
    template std :: ostream & operator << < Z128 >
        ( std :: ostream & o, const DecFltPoint < Z128 > & val );
    
    template < class T >
    XP & operator << ( XP & xp, const DecFltPoint < T > & val )
    {
        AutoRgn < 1024 > argn;
        MText mrgn = argn . getMText ();
        CText txt = NConv () . realToText < DecFltPoint < T > > ( mrgn, val, 2 );
        return xp << txt;
    }

    template XP & operator << < Z64 > ( XP & xp, const DecFltPoint < Z64 > & val );
    template XP & operator << < Z128 > ( XP & xp, const DecFltPoint < Z128 > & val );
}
