/*

  vdb3.cmn.atoi

 */

#include <vdb3/cmn/string.hpp>

namespace vdb3
{

    Z64 decToZ64 ( const String & s )
    {
        // walk across the string
        auto it = s . makeIterator ();

        // must be non-empty
        if ( ! it . isValid () )
            throw InvalidNumeral ( XP ( XLOC ) << "empty numeric string" );

        // detect negation
        bool negate = false;
        if ( * it == '-' )
        {
            negate = true;
            if ( ! ( ++ it ) . isValid () )
                throw InvalidNumeral ( XP ( XLOC ) << "expected digits in numeric string" );
        }
        else if ( * it == '+' )
        {
            if ( ! ( ++ it ) . isValid () )
                throw InvalidNumeral ( XP ( XLOC ) << "expected digits in numeric string" );
        }

        // must have at least 1 digit
        if ( ! iswdigit ( * it ) )
        {
            throw InvalidNumeral (
                XP ( XLOC )
                << "non-numeric string - expected digit but found '"
                << Char ( * it )
                << "' at position "
                << it . pos () . index ()
                );
        }

        // initial value
        long long int val = * it - '0';

        // loop over remaining string
        while ( ( ++ it ) . isValid () )
        {
            // non-digits are fatal
            if ( ! iswdigit ( * it ) )
            {
                throw InvalidNumeral (
                    XP ( XLOC )
                    << "non-numeric string - expected digit but found '"
                    << Char ( * it )
                    << "' at position "
                    << it . pos () . index ()
                    );
            }

            // detect overflow
            if ( val > ( Z64_MAX / 10 ) )
            {
                throw OverflowException (
                    XP ( XLOC )
                    << "numeric string of length "
                    << s . length ()
                    << " overflows long long int"
                    << " on mult: val => " << val
                    );
            }

            // shift previous value to next decimal place
            val *= 10;

            // get the value of ones place
            int d = * it - '0';

            // detect overflow
            if ( val > ( Z64_MAX - d ) )
            {
                // ah, the asymmetry of negative space
                if ( negate )
                {
                    // perform the test again
                    if ( - val >= ( Z64_MIN + d ) )
                    {
                        // make sure we're at end of string
                        if ( ! ( ++ it ) . isValid () )
                        {
                            // do it now
                            return - val - d;
                        }
                    }
                }

                throw OverflowException (
                    XP ( XLOC )
                    << "numeric string of length "
                    << s . length ()
                    << " overflows long long int"
                    << " on add: val => " << val
                    );
            }

            // complete number
            val += d;
        }

        return negate ? - val : val;
    }

}
