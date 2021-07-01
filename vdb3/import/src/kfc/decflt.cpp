/*

  vdb3.kfc.decflt

 */

#include <vdb3/kfc/decflt.hpp>
#include <vdb3/kfc/except.hpp>
#include <vdb3/kfc/ascii.hpp>

#include <math.h>

namespace vdb3
{
    //  Z64_MAX =                           9,223,372,036,854,775,807 : 18 digits
    //  N64_MAX =                          18,446,744,073,709,551,615 : 19 digits
    // Z128_MAX = 170,141,183,460,469,231,731,687,303,715,884,105,727 : 38 digits
    // N128_MAX = 340,282,366,920,938,463,463,374,607,431,768,211,455 : 38 digits

    const Z32 Z64_MAX_EXP   =  18;
    const Z32 Z64_MIN_EXP   = -18;
    const Z32 Z128_MAX_EXP  =  38;
    const Z32 Z128_MIN_EXP  = -38;
#if TESTING
    const Z32 N64_MAX_EXP   =  19;
    const Z32 N128_MAX_EXP  =  38;
#endif

    const Z64 Z64_MAX_MANT  =  999999999999999999LL;
    const Z64 Z64_MIN_MANT  = -999999999999999999LL;
#if HAVE_Z128
    const Z128 Z128_MAX_MANT =
        ( ( Z128 ) 5421010862427522170LL << 64 ) | 687399551400673279ULL;
    const Z128 Z128_MIN_MANT =
        ( ( Z128 ) -5421010862427522171LL << 64 ) | 17759344522308878337ULL;
#else
    const Z128 Z128_MAX_MANT ( 5421010862427522170LL, 687399551400673279ULL );
    const Z128 Z128_MIN_MANT ( -5421010862427522171LL, 17759344522308878337ULL );
#endif

    const Z32 NORM_EXP      = - 6;
    const Z64 Z64_NORM_FACT = 1000000;
    const R64 R64_NORM_FACT = Z64_NORM_FACT;
    
    /*

      DECIMAL FLOATING-POINT REPRESENTATION

      The mantissa or "significand" is a 2's complement binary integer.
      The exponent is also a 2's complement binary integer.
      This is basically the tuple ( mant, exp ).

      The mantissa is normalized at -6, such that all values
      that can be represented with 6 decimal places will have
      an exponent of -6. This is chosen somewhat arbitrarily.

      The stored exponent could be normalized to a value of
      zero and adjusted in code, but if in the future this normalization
      value were to be parameterized, keeping the true exponent
      would keep everything compatible.

      Assuming the normalized exponent to be -6:
        the value 1 is represented as ( 1000000, -6 ),
        the value 0 is represented as ( 0, -6 ), and
        the value -1.25 is represented as ( -1250000, -6 ).

      IEEE-754 FLOATING-POINT REPRESENTATIONS

      An R64 has a 53-bit significand and an 11 bit exponent.
      An R128 has a 113-bit significand and a 15 bit exponent.

      There are dangers in conversion that result from loss of
      significance.

     */
    
        // functions for throwing exceptions                                                                                                                                                     
    [[ noreturn ]] static
    void divide_by_zero ( const char * func, unsigned int lineno )
    {
        throw ArithmeticException (
            XP ( __FILE__, func, lineno )
            << "divide by zero"
            );
    }

#define DIV0()                                  \
    divide_by_zero ( __func__, __LINE__ )


    [[ noreturn ]] static
    void overflow ( const char * func, unsigned int lineno, const char * op )
    {
        throw OverflowException (
            XP ( __FILE__, func, lineno )
            << "operator "
            << ASCIIText ( op )
            << " overflow"
            );
    }

#define OVER( op )                              \
    overflow ( __func__, __LINE__, op )

#if 0
    [[ noreturn ]] static
	void underflow ( const char * func, unsigned int lineno, const char * op )
	{
        throw UnderflowException (
	        XP ( __FILE__, func, lineno )
	        << "operator "
            << ASCIIText ( op )
	        << " underflow"
            );
    }
    
#define UNDER( op )                             \
    underflow ( __func__, __LINE__, op )
#endif
    
    static
    N32 count_significant_bits ( Z64 val )
    {
        if ( val == 0 )
            return 0;
        
        return 64 - __builtin_clzll ( val < 0 ? -val : val );
    }

#if HAVE_Z128
    static
    N32 count_significant_bits ( Z128 val )
    {
        if ( val < 0 )
            val = - val;

        Z64 hi = ( Z64 ) ( val >> 64 );
        if ( hi == 0 )
            return 64 - __builtin_clzll ( ( N64 ) val );
        
        return 128 - __builtin_clzll ( hi );
    }
#else
    static
    N32 count_significant_bits ( const Z128 & val )
    {
        Z64 hi = val . high64 ();
        N64 low = val . low64 ();
        
        if ( hi < 0 )
        {
            Z128 abs = - val;
            hi = abs . high64 ();
            low = abs . low64 ();
        }

        // everything fits in 64 bits anyway
        if ( hi == 0 )
            return 64 - __builtin_clzll ( low );
        
        return 128 - __builtin_clzll ( hi );
    }
#endif

    /*=====================================================*
     *                     DecConst<T>                     *
     *=====================================================*/

    template < class T >
    struct DecConst
    {
        static inline T minVal () noexcept;
        static inline T maxVal () noexcept;
        static inline Z32 minExp () noexcept;
        static inline Z32 maxExp () noexcept;
        static T exp10 ( Z32 exp ) noexcept;
    };

    template <>
    inline Z64 DecConst < Z64 > :: minVal () noexcept
    {
        return Z64_MIN_MANT;
    }

    template <>
    inline Z128 DecConst < Z128 > :: minVal () noexcept
    {
        return Z128_MIN_MANT;
    }

    template <>
    inline Z64 DecConst < Z64 > :: maxVal () noexcept
    {
        return Z64_MAX_MANT;
    }

    template <>
    inline Z128 DecConst < Z128 > :: maxVal () noexcept
    {
        return Z128_MAX_MANT;
    }

    template <>
    inline Z32 DecConst < Z64 > :: minExp () noexcept
    {
        return Z64_MIN_EXP;
    }

    template <>
    inline Z32 DecConst < Z128 > :: minExp () noexcept
    {
        return Z128_MIN_EXP;
    }

    template <>
    inline Z32 DecConst < Z64 > :: maxExp () noexcept
    {
        return Z64_MAX_EXP;
    }

    template <>
    inline Z32 DecConst < Z128 > :: maxExp () noexcept
    {
        return Z128_MAX_EXP;
    }

    template <>
    Z64 DecConst < Z64 > :: exp10 ( Z32 exp ) noexcept
    {
        switch ( exp < 0 ? - exp : exp )
        {
        case 0:
            return 1;
        case 1:
            return 10;
        case 2:
            return 100;
        case 3:
            return 1000;
        case 4:
            return 10000;
        case 5:
            return 100000;
        case 6:
            return 1000000;
        case 7:
            return 10000000;
        case 8:
            return 100000000;
        case 9:
            return 1000000000;
        case 10:
            return 10000000000LL;
        case 11:
            return 100000000000LL;
        case 12:
            return 1000000000000LL;
        case 13:
            return 10000000000000LL;
        case 14:
            return 100000000000000LL;
        case 15:
            return 1000000000000000LL;
        case 16:
            return 10000000000000000LL;
        case 17:
            return 100000000000000000LL;
        case 18:
            return 1000000000000000000LL;
        default:
            break;
        }
        
        return Z64_MAX;
    }

    template <>
    Z128 DecConst < Z128 > :: exp10 ( Z32 exp ) noexcept
    {
        Z32 abs = exp < 0 ? - exp : exp;
        
        // detect small-enough exponents
        if ( abs <= Z64_MAX_EXP )
            return Z128 ( DecConst < Z64 > :: exp10 ( abs ) );

        // detect a representable exponent
        if ( abs <= Z128_MAX_EXP )
        {
            // start with a factor
            assert ( Z128_MAX_EXP > Z64_MAX_EXP );
            Z128 fact ( DecConst < Z64 > :: exp10 ( Z64_MAX_EXP ) );

            // build up remainder
            for ( abs -= Z64_MAX_EXP; abs > Z64_MAX_EXP; abs -= Z64_MAX_EXP )
                fact *= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );

            fact *= DecConst < Z64 > :: exp10 ( abs );

            return fact;
        }

        // too large
        return Z128_MAX;
    }
    

    /*=====================================================*
     *                    DecFltBase<T>                    *
     *=====================================================*/

    /*

      IN THE WONDERFUL WORLD OF TEMPLATES,
      there are additional and odd rules about
      template instantiation.

      The normal rules about declaration and forward
      reference do not apply, since specific decisions are
      taken about a method upon reference even though no
      definition has been seen.

      So the ordering here is a little bit "all over the place"
      so that we get specializations out of the way as soon as
      possible.

     */

    // get the destructor out of the way
    template < class T >
    DecFltBase < T > :: ~ DecFltBase () noexcept
    {
        mant = 0;
        exp = 0;
    }

    // native constructors
    template < class T >
    DecFltBase < T > :: DecFltBase () noexcept
        : mant ( 0 )
        , exp ( NORM_EXP )
    {
    }

    template <>
    DecFltBase < Z64 > :: DecFltBase ( const DecFltBase < Z64 > & val )
        : mant ( val . mant )
        , exp ( val . exp )
    {
    }
    
    template <>
    DecFltBase < Z128 > :: DecFltBase ( const DecFltBase < Z128 > & val )
        : mant ( val . mant )
        , exp ( val . exp )
    {
    }

    // native cpy
    template <>
    void DecFltBase < Z64 > :: cpy ( const DecFltBase < Z64 > & val )
    {
        mant = val . mant;
        exp = val . exp;
    }
    
    template <>
    void DecFltBase < Z128 > :: cpy ( const DecFltBase < Z128 > & val )
    {
        mant = val . mant;
        exp = val . exp;
    }

    // cross-type accessors
    template <>
    Z64 DecFltBase < Z64 > :: getMantissa64 () const
    {
        return mant;
    }
    
    template <>
    Z64 DecFltBase < Z128 > :: getMantissa64 () const
    {
        if ( mant < Z64_MIN || mant > Z64_MAX )
            OVER ( "( Z64 )" );

        return ( Z64 ) mant;
    }
    
    template < class T >
    Z128 DecFltBase < T > :: getMantissa128 () const noexcept
    {
        return Z128 ( mant );
    }
    
    template < class T >
    Z32 DecFltBase < T > :: getExponent () const noexcept
    {
        return exp;
    }

    // cross-type cpy
    template <>
    void DecFltBase < Z64 > :: cpy ( const DecFltBase < Z128 > & val )
    {
        Z128 mant_b = val . getMantissa128 ();
        Z32 scale = val . getExponent ();

        Z128 abs_mant_b = mant_b < 0 ? - mant_b : mant_b;
        while ( abs_mant_b > Z64_MAX )
        {
            abs_mant_b /= 10;
            ++ scale;
        }

        mant = ( Z64 ) abs_mant_b;
        exp = scale;

        if ( mant_b < 0 )
            mant = - mant;
    }
    
    template <>
    void DecFltBase < Z128 > :: cpy ( const DecFltBase < Z64 > & val )
    {
        mant = val . getMantissa128 ();
        exp = val . getExponent ();
    }

    // cross-type constructors that rely upon "cpy()" for work
    template <>
    DecFltBase < Z64 > :: DecFltBase ( const DecFltBase < Z128 > & val )
    {
        cpy ( val );
    }
    
    template <>
    DecFltBase < Z128 > :: DecFltBase ( const DecFltBase < Z64 > & val )
    {
        cpy ( val );
    }
    
    template < class T >
    void DecFltBase < T > :: normalize ()
    {
        if ( mant == 0 )
            exp = NORM_EXP;
        else if ( exp > NORM_EXP )
        {
            Z32 scale = exp;
            T abs_mant = mant < 0 ? - mant : mant;
            const T mul_lim = DecConst < T > :: maxVal () / 10;

            for ( ; scale > NORM_EXP; -- scale )
            {
                if ( abs_mant > mul_lim )
                    break;
                abs_mant *= 10;
            }

            mant = mant < 0 ? - abs_mant : abs_mant;
            exp = scale;
        }
        else if ( exp < NORM_EXP )
        {
            Z32 scale = exp;
            T abs_mant = mant < 0 ? - mant : mant;

            for ( ; scale < NORM_EXP; ++ scale )
            {
                if ( abs_mant % 10 != 0 )
                    break;
                abs_mant /= 10;
            }

            mant = mant < 0 ? - abs_mant : abs_mant;
            exp = scale;
        }
    }
    
    template < class T >
    void DecFltBase < T > :: denormalize ()
    {
        if ( mant == 0 )
            exp = 0;
        else
        {
            Z32 scale = exp;
            T abs_mant = mant < 0 ? - mant : mant;
            
            for ( ; ; ++ scale )
            {
                if ( abs_mant % 10 != 0 )
                    break;
                abs_mant /= 10;
            }
            
            mant = mant < 0 ? - abs_mant : abs_mant;
            exp = scale;
        }
    }

    // construction from R64
    template < class T >
    void DecFltBase < T > :: cpy ( R64 val )
    {
        if ( val == 0 )
        {
            mant = 0;
            exp = NORM_EXP;
            return;
        }

        // work in absolute value
        R64 abs = val < 0 ? - val : val;
        
        // counter for scaling input
        Z32 diff, scale = 0;

        // if the integer portion of value won't fit in mantissa
        Z32 zdig = ( Z32 ) :: ceil ( :: log10 ( abs ) );
        if ( zdig > DecConst < T > :: maxExp () )
        {
            // this is a close but inexact distance
            diff = zdig - DecConst < T > :: maxExp () + 1;

            // rapid convergence
            for ( ; diff > Z64_MAX_EXP; diff -= Z64_MAX_EXP )
            {
                scale += Z64_MAX_EXP;
                abs /= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            }
            scale += diff;
            abs /= DecConst < Z64 > :: exp10 ( diff );

            // now measure again more carefully
            zdig = ( Z32 ) :: ceil ( :: log10 ( abs ) );
            while ( zdig > DecConst < T > :: maxExp () )
            {
                scale += 1;
                abs /= 10;
                zdig = ( Z32 ) :: ceil ( :: log10 ( abs ) );
            }
        }

        // scale up to fill available precision
        diff = DecConst < T > :: maxExp () - zdig;
        if ( diff > 0 )
        {
            for ( ; diff > Z64_MAX_EXP; diff -= Z64_MAX_EXP )
            {
                scale -= Z64_MAX_EXP;
                abs *= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            }

            scale -= diff;
            abs *= DecConst < Z64 > :: exp10 ( diff );
        }

        // it should now be within range
        assert ( ( R64 ) DecConst < T > :: maxVal () >= abs );
        assert ( ( R64 ) DecConst < T > :: maxVal () < ( abs * 10 ) );

        // go fixed point
        mant = ( T ) ( val < 0 ? - abs : abs );
        exp = scale;

        // final normalization
        normalize ();
    }

#if ! HAVE_Z128
    template <>
    void DecFltBase < Z128 > :: cpy ( R64 val )
    {
        if ( val == 0 )
        {
            mant = 0;
            exp = NORM_EXP;
            return;
        }

        // work in absolute value
        R64 abs = val < 0 ? - val : val;
        
        // counter for scaling input
        Z32 diff, scale = 0;

        // if the integer portion of value won't fit in mantissa
        Z32 zdig = ( Z32 ) :: ceil ( :: log10 ( abs ) );
        if ( zdig > DecConst < Z64 > :: maxExp () )
        {
            // this is the distance
            diff = zdig - DecConst < Z64 > :: maxExp () + 1;

            // rapid convergence
            for ( ; diff > Z64_MAX_EXP; diff -= Z64_MAX_EXP )
            {
                scale += Z64_MAX_EXP;
                abs /= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            }

            // penunltimate convergence
            scale += diff;
            abs /= DecConst < Z64 > :: exp10 ( diff );

            // now measure again more carefully
            zdig = ( Z32 ) :: ceil ( :: log10 ( abs ) );
            while ( zdig > DecConst < Z64 > :: maxExp () )
            {
                scale += 1;
                abs /= 10;
                zdig = ( Z32 ) :: ceil ( :: log10 ( abs ) );
            }
        }

        // scale up to fill available precision
        diff = DecConst < Z64 > :: maxExp () - zdig;
        if ( diff > 0 )
        {
            for ( ; diff > Z64_MAX_EXP; diff -= Z64_MAX_EXP )
            {
                scale -= Z64_MAX_EXP;
                abs *= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            }

            scale -= diff;
            abs *= DecConst < Z64 > :: exp10 ( diff );
        }

        // it should now be within range
        assert ( ( R64 ) Z64_MAX >= abs );
        assert ( ( R64 ) Z64_MAX < ( abs * 10 ) );
        
        // go fixed point
        mant = ( Z64 ) ( val < 0 ? - abs : abs );
        exp = scale;

        normalize ();
    }
#endif
    
    template < class T >
    DecFltBase < T > :: DecFltBase ( R64 val )
    {
        cpy ( val );
    }
    
#if HAVE_R128
    // construction from R128
    template < class T >
    void DecFltBase < T > :: cpy ( R128 val )
    {
        if ( val == 0 )
        {
            mant = 0;
            exp = NORM_EXP;
            return;
        }

        // work in absolute value
        R128 abs = val < 0 ? - val : val;
        
        // counter for scaling input
        Z32 diff, scale = 0;

        // if the integer portion of value won't fit in mantissa
        Z32 zdig = ( Z32 ) :: ceill ( :: log10l ( ( long double ) abs ) );
        if ( zdig > DecConst < T > :: maxExp () )
        {
            // this is the distance
            diff = zdig - DecConst < T > :: maxExp () + 1;

            // rapid convergence
            for ( ; diff > Z64_MAX_EXP; diff -= Z64_MAX_EXP )
            {
                scale += Z64_MAX_EXP;
                abs /= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            }

            // penunltimate convergence
            scale += diff;
            abs /= DecConst < Z64 > :: exp10 ( diff );

            // now measure again more carefully
            zdig = ( Z32 ) :: ceill ( :: log10l ( ( long double ) abs ) );
            while ( zdig > DecConst < T > :: maxExp () )
            {
                scale += 1;
                abs /= 10;
                zdig = ( Z32 ) :: ceill ( :: log10l ( ( long double ) abs ) );
            }
        }

        // scale up to fill available precision
        diff = DecConst < T > :: maxExp () - zdig;
        if ( diff > 0 )
        {
            for ( ; diff > Z64_MAX_EXP; diff -= Z64_MAX_EXP )
            {
                scale -= Z64_MAX_EXP;
                abs *= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            }

            scale -= diff;
            abs *= DecConst < Z64 > :: exp10 ( diff );
        }

        // it should now be within range
        assert ( ( R128 ) DecConst < T > :: maxVal () >= abs );
        assert ( ( R128 ) DecConst < T > :: maxVal () < ( abs * 10 ) );

        // go fixed point
        mant = ( T ) ( val < 0 ? - abs : abs );
        exp = scale;

        normalize ();
    }
    
    template < class T >
    DecFltBase < T > :: DecFltBase ( R128 val )
    {
        cpy ( val );
    }
#endif

    // construction from Z64
    template < class T >
    void DecFltBase < T > :: cpy ( Z64 val )
    {
        mant = val;
        exp = 0;
        normalize ();
    }
    
    template < class T >
    DecFltBase < T > :: DecFltBase ( Z64 val )
    {
        cpy ( val );
    }

    // construction from Z128
#if HAVE_Z128
    template < class T >
    void DecFltBase < T > :: cpy ( Z128 val )
    {
        if ( val == 0 )
        {
            mant = 0;
            exp = NORM_EXP;
            return;
        }

        Z32 scale = 0;
        Z128 abs = val < 0 ? - val : val;
        const Z128 max_val = DecConst < T > :: maxVal ();

        for ( ; abs > max_val; ++ scale )
            abs /= 10;

        mant = ( T ) ( val < 0 ? - abs : abs );
        exp = scale;

        normalize ();
    }
    
    template < class T >
    DecFltBase < T > :: DecFltBase ( Z128 val )
    {
        cpy ( val );
    }
#else
    template < class T >
    void DecFltBase < T > :: cpy ( const Z128 & val )
    {
        if ( val == 0 )
        {
            mant = 0;
            exp = NORM_EXP;
            return;
        }

        Z32 scale = 0;
        Z128 abs = val < 0 ? - val : val;
        const Z128 max_val ( DecConst < T > :: maxVal () );

        for ( ; abs > max_val; ++ scale )
            abs /= 10;

        mant = ( T ) abs;
        if ( val < 0 )
            mant = - mant;
        
        exp = scale;

        normalize ();
    }
    
    template < class T >
    DecFltBase < T > :: DecFltBase ( const Z128 & val )
    {
        cpy ( val );
    }
#endif

    // native operations
    template < class T >
    void DecFltBase < T > :: trunc () noexcept
    {
        // nothing to truncate unless exponent is negative
        if ( exp < 0 )
        {
            // if it's really negative, just drop everything
            if ( exp < DecConst < T > :: minExp () )
            {
                mant = 0;
                exp = NORM_EXP;
            }
            else
            {
                // first, truncate by dividing out fractional part
                T tmant;
                if ( mant < 0 )
                    tmant = - ( - mant / DecConst < T > :: exp10 ( exp ) );
                else
                    tmant = mant / DecConst < T > :: exp10 ( exp );

                // re-normalize but limit the fractional place-holders
                if ( exp < NORM_EXP )
                    mant = tmant * DecConst < Z64 > :: exp10 ( exp = NORM_EXP );
                else
                    mant = tmant * DecConst < Z64 > :: exp10 ( exp );
            }
        }
    }

    template < class T >
    void DecFltBase < T > :: negate ()
    {
        mant = - mant;
    }
    
#if ! HAVE_Z128
    template <>
    void DecFltBase < Z128 > :: negate ()
    {
        mant . negate ();
    }
#endif

    template < class T >
    void DecFltBase < T > :: add ( const DecFltBase < Z64 > & val )
    {
        T mant_a ( mant );
        T mant_b ( val . getMantissa64 () );
        Z32 exp_a = exp;
        Z32 exp_b = val . getExponent ();

        // establish "a" as >= "b" by exponent
        if ( exp_a < exp_b )
        {
            T mant_tmp ( mant_a );
            Z32 exp_tmp = exp_a;
            mant_a = mant_b;
            exp_a = exp_b;
            mant_b = mant_tmp;
            exp_b = exp_tmp;
        }

        // try to denormalize larger to the extent possible
        if ( exp_a > exp_b )
        {
            bool needs_negate = false;
            if ( mant_a < 0 )
            {
                needs_negate = true;
                mant_a = - mant_a;
            }

            const T mul_lim = DecConst < T > :: maxVal () / 10;
            while ( mant_a <= mul_lim && exp_a > exp_b )
            {
                mant_a *= 10;
                -- exp_a;
            }

            if ( needs_negate )
                mant_a = - mant_a;
        }

        // if there's still a disparity, truncate digits
        if ( exp_a > exp_b )
        {
            T factor ( DecConst < T > :: exp10 ( exp_a - exp_b ) );
            if ( mant_b < 0 )
                mant_b = - ( - mant_b / factor );
            else
                mant_b /= factor;
        }

        // perform overflow-detecting operation
        // when signs differ - no overflow is possible
        mant = mant_a + mant_b;
        if ( ( mant_a ^ mant_b ) >= 0 )
        {
            // when signs are same,
            // result must maintain same sign
            if ( ( mant ^ mant_a ) < 0 )
            {
                // perhaps not necessary, but
                // going to abandon some precision
                // so add the lowest digits together first
                Z32 carry;
                if ( mant_a < 0 )
                {
                    // work with absolute value
                    carry = ( Z32 )
                        ( ( Z64 ) ( ( - mant_a ) % 10 ) + ( Z64 ) ( ( - mant_b )  % 10 ) );

                    // scale the mantissa
                    mant_a = - ( - mant_a / 10 );
                    mant_b = - ( - mant_b / 10 );

                    // convert the carry to a 0 or -1
                    carry = carry >= 5 ? -1 : 0;
                }
                else
                {
                    carry = ( Z32 ) ( ( Z64 ) ( mant_a % 10 ) + ( Z64 ) (  mant_b % 10 ) );
                    mant_a /= 10;
                    mant_b /= 10;
                    carry = carry >= 5 ? 1 : 0;
                }

                // account for downscale
                ++ exp_a;

                // rewrite mantissa with carry
                mant = mant_a + mant_b + carry;
            }
        }

        // record scale
        exp = exp_a;

        // always attempt to normalize
        normalize ();
    }
    
    template < class T >
    void DecFltBase < T > :: mul ( const DecFltBase < Z64 > & val )
    {
        T mant_a ( mant );
        T mant_b ( val . getMantissa64 () );

        if ( mant_a == 0 || mant_b == 0 )
        {
            mant = 0;
            exp = NORM_EXP;
            return;
        }
        
        Z32 exp_a = exp;
        Z32 exp_b = val . getExponent ();

        N32 bits_a = count_significant_bits ( mant_a );
        N32 bits_b = count_significant_bits ( mant_b );
        if ( bits_a + bits_b >= sizeof ( T ) * 8 )
        {
            // denormalize each as much as possible
            while ( ( Z64 ) ( mant_a % 10 ) == 0 )
            {
                mant_a /= 10;
                ++ exp_a;
            }
            
            bits_a = count_significant_bits ( mant_a );
            if ( bits_a + bits_b >= sizeof ( T ) * 8 )
            {
                while ( ( Z64 ) ( mant_b % 10 ) == 0 )
                {
                    mant_b /= 10;
                    ++ exp_b;
                }
                bits_b = count_significant_bits ( mant_b );
                if ( bits_a + bits_b >= sizeof ( T ) * 8 )
                {
                    // well, this guy is stubborn
                    // we'll have to loose some bits
                    //
                    // TBD - which bits are best to loose:
                    //   do we drop from largest because
                    //   the difference is 10 x smallest?
                    
                    OVER ( "*" );
                }
            }
        }

        mant = mant_a * mant_b;
        exp = exp_a + exp_b;

        normalize ();
    }
    
    template < class T >
    void DecFltBase < T > :: div ( const DecFltBase < Z64 > & val )
    {
        // extract 64-bit version
        T mant_b ( val . getMantissa64 () );
        Z32 exp_b = val . getExponent ();

        // early detection of divide by zero
        if ( mant_b == 0 )
            DIV0 ();

        // homogenize
        T mant_a = mant;
        Z32 exp_a = exp;

        // absolute value now - we know the sign for restoration
        T abs_mant_a = mant_a < 0 ? - mant_a : mant_a;
        T abs_mant_b = mant_b < 0 ? - mant_b : mant_b;

        // want ourselves to become as large as possible
        const T mul_lim = DecConst < T > :: maxVal () / 10;
        while ( abs_mant_a <= mul_lim )
        {
            abs_mant_a *= 10;
            -- exp_a;
        }

        // we want the integer divisor to be as small as possible
        while ( ( abs_mant_b % 10 ) == 0 )
        {
            abs_mant_b /= 10;
            ++ exp_b;
        }

        // this becomes the denormalized denominator
        DecFltBase < T > den;
        den . mant = abs_mant_b;
        den . exp = exp_b;

        // now, we can only make forward progress if the
        // numerator ( ourselves ) >= denominator, so we
        // discard to the right as required
        while ( abs_mant_a < abs_mant_b )
        {
            abs_mant_b /= 10;
            ++ exp_b;
        }

        // I don't believe this is possible
        // because abs_mant_a will always be >= 10
        // and an originally non-zero denominator
        // will therefore always be < numerator
        // with denominator between 1 and 9
        if ( abs_mant_b == 0 )
            DIV0 ();

        // the integer portion is here
        T abs_mant_c = abs_mant_a / abs_mant_b;

        // the remainder is here
        T abs_rem_c = abs_mant_a % abs_mant_b;

        // update with integer division
        mant = abs_mant_c;
        exp = exp_a - exp_b;
        normalize ();

        // if there's a remainder...
        if ( abs_rem_c != 0 )
        {
            // create a remainder object
            DecFltBase < T > num;
            num . mant = abs_rem_c;
            num . exp = exp_a;

            // continue the division
            div ( num, den );
        }

        // repair the sign
        if ( ( mant_a ^ mant_b ) < 0 )
            negate ();
    }
    
    template < class T >
    void DecFltBase < T > :: div ( DecFltBase < T > & num, const DecFltBase < T > & den )
    {
        T abs_mant_a = num . mant;
        T abs_mant_b = den . mant;
        Z32 exp_a = num . exp;
        Z32 exp_b = den . exp;

        assert ( abs_mant_b != 0 );

        // want ourselves to become as large as possible
        const T mul_lim = DecConst < T > :: maxVal () / 10;
        while ( abs_mant_a <= mul_lim )
        {
            abs_mant_a *= 10;
            -- exp_a;
        }

        // now, we can only make forward progress if the
        // numerator ( ourselves ) >= denominator, so we
        // discard to the right as required
        while ( abs_mant_a < abs_mant_b )
        {
            abs_mant_b /= 10;
            ++ exp_b;
        }

        assert ( abs_mant_b != 0 );

        // the remainder is here
        T abs_rem_c = abs_mant_a % abs_mant_b;

        // update with integer division
        num . mant = abs_mant_a / abs_mant_b;
        num . exp = exp_a - exp_b;
        num . normalize ();

        // incorporate this into our sum
        add ( num );

        // if there's a remainder...
        // and if it's not too tiny...
        if ( abs_rem_c != 0 )
        {
            // create a remainder object
            num . mant = abs_rem_c;
            num . exp = exp_a;

            // recur
            div ( num, den );
        }

    }
    
    template <>
    int DecFltBase < Z64 > :: cmp ( const DecFltBase < Z64 > & val ) const
    {
        if ( exp == val . exp )
        {
            if ( mant < val . mant )
                return -1;
            return mant > val . mant;
        }

        DecFltBase < Z64 > copy ( val );
        copy . negate ();
        copy . add ( self );

        if ( copy . mant < 0 )
            return -1;
        return copy . mant > 0;
    }
    
    template <>
    void DecFltBase < Z64 > :: add ( const DecFltBase < Z128 > & val )
    {
        Z128 mant_a ( mant );
        Z128 mant_b = val . getMantissa128 ();
        Z32 exp_a = exp;
        Z32 exp_b = val . getExponent ();

        // establish "a" as >= "b" by exponent
        if ( exp_a < exp_b )
        {
            Z128 mant_tmp ( mant_a );
            Z32 exp_tmp = exp_a;
            mant_a = mant_b;
            exp_a = exp_b;
            mant_b = mant_tmp;
            exp_b = exp_tmp;
        }

        // try to denormalize larger to the extent possible
        if ( exp_a > exp_b )
        {
            bool needs_negate = false;
            if ( mant_a < 0 )
            {
                needs_negate = true;
                mant_a = - mant_a;
            }

            const Z128 mul_lim ( DecConst < Z128 > :: maxVal () / 10 );
            while ( mant_a <= mul_lim && exp_a > exp_b )
            {
                mant_a *= 10;
                -- exp_a;
            }

            if ( needs_negate )
                mant_a = - mant_a;
        }

        // if there's still a disparity, truncate digits
        if ( exp_a > exp_b )
        {
            Z128 factor ( DecConst < Z128 > :: exp10 ( exp_a - exp_b ) );
            if ( mant_b < 0 )
                mant_b = - ( - mant_b / factor );
            else
                mant_b /= factor;
        }

        // perform overflow-detecting operation
        // when signs differ - no overflow is possible
        Z128 mant_c = mant_a + mant_b;
        if ( ( mant_a ^ mant_b ) >= 0 )
        {
            // when signs are same,
            // result must maintain same sign
            if ( ( mant_c ^ mant_a ) < 0 )
            {
                // perhaps not necessary, but
                // going to abandon some precision
                // so add the lowest digits together first
                Z32 carry;
                if ( mant_a < 0 )
                {
                    // work with absolute value
                    carry = ( Z32 )
                        ( ( Z64 ) ( ( - mant_a ) % 10 ) + ( Z64 ) ( ( - mant_b ) % 10 ) );

                    // scale the mantissa
                    mant_a = - ( - mant_a / 10 );
                    mant_b = - ( - mant_b / 10 );

                    // convert the carry to a 0 or -1
                    carry = carry >= 5 ? -1 : 0;
                }
                else
                {
                    carry = ( Z32 ) ( ( Z64 ) ( mant_a % 10 ) + ( Z64 ) ( mant_b % 10 ) );
                    mant_a /= 10;
                    mant_b /= 10;
                    carry = carry >= 5 ? 1 : 0;
                }

                // account for downscale
                ++ exp_a;

                // rewrite mantissa with carry
                mant_c = mant_a + mant_b + carry;
            }
        }

        // downscale
        Z128 mant_lim ( Z64_MAX );
        if ( mant_c < 0 )
        {
            mant_c = - mant_c;
            while ( mant_c > mant_lim )
            {
                mant_c /= 10;
                ++ exp_a;
            }
            mant_c = - mant_c;
        }
        else
        {
            while ( mant_c > mant_lim )
            {
                mant_c /= 10;
                ++ exp_a;
            }
        }

        // record scale
        mant = ( Z64 ) mant_c;
        exp = exp_a;

        // always attempt to normalize
        normalize ();
    }
    
    template <>
    void DecFltBase < Z128 > :: add ( const DecFltBase < Z128 > & val )
    {
        Z128 mant_a = mant;
        Z128 mant_b = val . mant;
        Z32 exp_a = exp;
        Z32 exp_b = val . exp;

        // establish "a" as >= "b" by exponent
        if ( exp_a < exp_b )
        {
            Z128 mant_tmp ( mant_a );
            Z32 exp_tmp = exp_a;
            mant_a = mant_b;
            exp_a = exp_b;
            mant_b = mant_tmp;
            exp_b = exp_tmp;
        }

        // try to denormalize larger to the extent possible
        if ( exp_a > exp_b )
        {
            bool needs_negate = false;
            if ( mant_a < 0 )
            {
                needs_negate = true;
                mant_a = - mant_a;
            }

            const Z128 mul_lim ( DecConst < Z128 > :: maxVal () / 10 );
            while ( mant_a <= mul_lim && exp_a > exp_b )
            {
                mant_a *= 10;
                -- exp_a;
            }

            if ( needs_negate )
                mant_a = - mant_a;
        }

        // if there's still a disparity, truncate digits
        if ( exp_a > exp_b )
        {
            Z128 factor ( DecConst < Z128 > :: exp10 ( exp_a - exp_b ) );
            if ( mant_b < 0 )
                mant_b = - ( - mant_b / factor );
            else
                mant_b /= factor;
        }
        
        // perform overflow-detecting operation
        // when signs differ - no overflow is possible
        mant = mant_a + mant_b;
        if ( ( mant_a ^ mant_b ) >= 0 )
        {
            // when signs are same,
            // result must maintain same sign
            if ( ( mant ^ mant_a ) < 0 )
            {
                // perhaps not necessary, but
                // going to abandon some precision
                // so add the lowest digits together first
                Z32 carry;
                if ( mant_a < 0 )
                {
                    // work with absolute value
                    carry = ( Z32 )
                        ( ( Z64 ) ( ( - mant_a ) % 10 ) + ( Z64 ) ( ( - mant_b ) % 10 ) );

                    // scale the mantissa
                    mant_a = - ( - mant_a / 10 );
                    mant_b = - ( - mant_b / 10 );

                    // convert the carry to a 0 or -1
                    carry = carry >= 5 ? -1 : 0;
                }
                else
                {
                    carry = ( Z32 ) ( ( Z64 ) ( mant_a % 10 ) + ( Z64 ) ( mant_b % 10 ) );
                    mant_a /= 10;
                    mant_b /= 10;
                    carry = carry >= 5 ? 1 : 0;
                }

                // account for downscale
                ++ exp_a;

                // rewrite mantissa with carry
                mant = mant_a + mant_b + carry;
            }
        }

        // record scale
        exp = exp_a;

        // always attempt to normalize
        normalize ();
    }
    
    template < class T >
    void DecFltBase < T > :: mul ( const DecFltBase < Z128 > & val )
    {
        Z128 mant_a ( mant );
        Z128 mant_b ( val . getMantissa128 () );
        Z32 exp_a = exp;
        Z32 exp_b = val . getExponent ();

        N32 bits_a = count_significant_bits ( mant_a );
        N32 bits_b = count_significant_bits ( mant_b );
        if ( bits_a + bits_b >= sizeof ( T ) * 8 )
        {
            // denormalize one as much as possible
            while ( ( Z64 ) ( mant_a % 10 ) == 0 )
            {
                mant_a /= 10;
                ++ exp_a;
            }
            
            bits_a = count_significant_bits ( mant_a );
            if ( bits_a + bits_b >= sizeof ( T ) * 8 )
            {
                while ( ( Z64 ) ( mant_b % 10 ) == 0 )
                {
                    mant_b /= 10;
                    ++ exp_b;
                }
                bits_b = count_significant_bits ( mant_b );
                if ( bits_a + bits_b >= sizeof ( T ) * 8 )
                {
                    // well, this guy is stubborn
                    // we'll have to loose some bits
                    OVER ( "*" );
                }
            }
        }

        mant = ( T ) ( mant_a * mant_b );
        exp = exp_a + exp_b;

        normalize ();
    }
    
    template <>
    void DecFltBase < Z128 > :: div ( const DecFltBase < Z128 > & val )
    {
        // extract 128-bit version
        Z128 mant_b ( val . getMantissa128 () );
        Z32 exp_b = val . getExponent ();

        // early detection of divide by zero
        if ( mant_b == 0 )
            DIV0 ();

        // homogenize
        Z128 mant_a ( mant );
        Z32 exp_a = exp;

        // absolute value now - we know the sign for restoration
        Z128 abs_mant_a = mant_a < 0 ? - mant_a : mant_a;
        Z128 abs_mant_b = mant_b < 0 ? - mant_b : mant_b;

        // want ourselves to become as large as possible
        const Z128 mul_lim ( DecConst < Z128 > :: maxVal () / 10 );
        while ( abs_mant_a <= mul_lim )
        {
            abs_mant_a *= 10;
            -- exp_a;
        }

        // we want the integer divisor to be as small as possible
        while ( ( abs_mant_b % 10 ) == 0 )
        {
            abs_mant_b /= 10;
            ++ exp_b;
        }

        // this becomes the denormalized denominator
        DecFltBase < Z128 > den;
        den . mant = abs_mant_b;
        den . exp = exp_b;

        // now, we can only make forward progress if the
        // numerator ( ourselves ) >= denominator, so we
        // discard to the right as required
        while ( abs_mant_a < abs_mant_b )
        {
            abs_mant_b /= 10;
            ++ exp_b;
        }

        // I don't believe this is possible
        // because abs_mant_a will always be >= 10
        // and an originally non-zero denominator
        // will therefore always be < numerator
        // with denominator between 1 and 9
        if ( abs_mant_b == 0 )
            DIV0 ();

        // the integer portion is here
        Z128 abs_mant_c ( abs_mant_a / abs_mant_b );

        // the remainder is here
        Z128 abs_rem_c ( abs_mant_a % abs_mant_b );

        // update with integer division
        mant = abs_mant_c;
        exp = exp_a - exp_b;
        normalize ();

        // if there's a remainder...
        if ( abs_rem_c != 0 )
        {
            // create a remainder object
            DecFltBase < Z128 > num;
            num . mant = abs_rem_c;
            num . exp = exp_a;

            // continue the division
            div ( num, den );
        }

        // repair the sign
        if ( ( mant_a ^ mant_b ) < 0 )
            negate ();
    }
    
    template <>
    void DecFltBase < Z64 > :: div ( const DecFltBase < Z128 > & val )
    {
        DecFltBase < Z128 > copy ( self );
        copy . div ( val );
        cpy ( copy );
    }
    
    template <>
    int DecFltBase < Z128 > :: cmp ( const DecFltBase < Z128 > & val ) const
    {
        if ( exp == val . exp )
        {
            if ( mant < val . mant )
                return -1;
            return mant > val . mant;
        }

        DecFltBase < Z128 > copy ( val );
        copy . negate ();
        copy . add ( self );

        if ( copy . mant < 0 )
            return -1;
        return copy . mant > 0;
    }

    // cross-type operations
    template <>
    int DecFltBase < Z128 > :: cmp ( const DecFltBase < Z64 > & val ) const
    {
        DecFltBase < Z128 > copy ( val );
        return cmp ( copy );
    }
    
    template <>
    int DecFltBase < Z64 > :: cmp ( const DecFltBase < Z128 > & val ) const
    {
        int diff = val . cmp ( self );
        if ( diff > 0 )
            return -1;
        return diff < 0;
    }

    // dependent
    template < class T >
    void DecFltBase < T > :: round () noexcept
    {
        if ( exp < 0 )
        {
            // get the 1.0 scaling factor
            T factor ( DecConst < T > :: exp10 ( exp ) );

            // divide it by 2 gives a value of 0.5
            factor >>= 1;

            // add it away from 0
            if ( mant < 0 )
                mant -= factor;
            else
                mant += factor;

            // truncate
            trunc ();
        }
    }
    
    template < class T >
    void DecFltBase < T > :: ceil () noexcept
    {
        if ( exp < 0 )
        {
            // get the 1.0 scaling factor
            T factor ( DecConst < T > :: exp10 ( exp ) );

            // subtract 1 will bump any fraction up to next int
            factor -= 1;

            // add it away from 0
            if ( mant < 0 )
                mant -= factor;
            else
                mant += factor;

            // truncate
            trunc ();
        }
    }
    
    template < class T >
    void DecFltBase < T > :: frac () noexcept
    {
        DecFltBase < T > copy ( self );
        copy . trunc ();
        sub ( copy );
    }
    
    template < class T >
    void DecFltBase < T > :: sub ( const DecFltBase < Z64 > & val )
    {
        // recast in terms of addition
        DecFltBase < T > copy ( val );
        copy . negate ();
        add ( copy );
    }
    
    template < class T >
    void DecFltBase < T > :: sub ( const DecFltBase < Z128 > & val )
    {
        DecFltBase < T > copy ( val );
        copy . negate ();
        add ( copy );
    }

    // R64 operations
    template < class T >
    void DecFltBase < T > :: add ( R64 val )
    {
        add ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: sub ( R64 val )
    {
        sub ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: mul ( R64 val )
    {
        mul ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: div ( R64 val )
    {
        div ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    int DecFltBase < T > :: cmp ( R64 val ) const
    {
        return cmp ( DecFltBase < T > ( val ) );
    }
    
#if HAVE_R128
    // R128 operations
    template < class T >
    void DecFltBase < T > :: add ( R128 val )
    {
        add ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: sub ( R128 val )
    {
        sub ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: mul ( R128 val )
    {
        mul ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: div ( R128 val )
    {
        div ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    int DecFltBase < T > :: cmp ( R128 val ) const
    {
        return cmp ( DecFltBase < T > ( val ) );
    }
#endif

    // Z64 operations
    template < class T >
    void DecFltBase < T > :: add ( Z64 val )
    {
        add ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: sub ( Z64 val )
    {
        sub ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: mul ( Z64 val )
    {
        mul ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: div ( Z64 val )
    {
        div ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    int DecFltBase < T > :: cmp ( Z64 val ) const
    {
        return cmp ( DecFltBase < T > ( val ) );
    }

    // Z128 operations
#if HAVE_Z128
    template < class T >
    void DecFltBase < T > :: add ( Z128 val )
    {
        add ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: sub ( Z128 val )
    {
        sub ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: mul ( Z128 val )
    {
        mul ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: div ( Z128 val )
    {
        div ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    int DecFltBase < T > :: cmp ( Z128 val ) const
    {
        return cmp ( DecFltBase < T > ( val ) );
    }
#else
    template < class T >
    void DecFltBase < T > :: add ( const Z128 & val )
    {
        add ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: sub ( const Z128 & val )
    { 
        sub ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: mul ( const Z128 & val )
    {
        mul ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    void DecFltBase < T > :: div ( const Z128 & val )
    {
        div ( DecFltBase < T > ( val ) );
    }
    
    template < class T >
    int DecFltBase < T > :: cmp ( const Z128 & val ) const
    {
        return cmp ( DecFltBase < T > ( val ) );
    }
#endif

    // constant stuff
    template <>
    R64 DecFltBase < Z64 > :: toR64 () const
    {
        // IEEE-754 will truncate down to 53 bit mantissa if needed
        // this will create an integer-only representation
        R64 rslt = mant;

        // optimize for normalized exponent
        if ( exp == NORM_EXP )
        {
            assert ( NORM_EXP < 0 );
            rslt /= R64_NORM_FACT;
        }
        else if ( exp < 0 )
        {
            Z32 e = - exp;
            for ( ; e > Z64_MAX_EXP; e -= Z64_MAX_EXP )
                rslt /= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            rslt /= DecConst < Z64 > :: exp10 ( e );
        }
        else if ( exp > 0 )
        {
            Z32 e = exp;
            for ( ; e > Z64_MAX_EXP; e -= Z64_MAX_EXP )
                rslt *= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            rslt *= DecConst < Z64 > :: exp10 ( e );
        }

        return rslt;
    }
    
    template <>
    R64 DecFltBase < Z128 > :: toR64 () const
    {
        // copy mantissa - we might have to modify it
        Z128 tmant ( mant );
        if ( tmant < 0 )
            tmant = - tmant;
        
        // detect size in bits
        N32 mbits = count_significant_bits ( tmant );

        // scale down mantissa by truncation
        R64 factor = 1.0;
        if ( mbits > 63 )
        {
            tmant >>= mbits - 63;
            factor = ( 1 << ( mbits - 63 ) );
        }

        // assign the significant bits
        R64 rslt = ( Z64 ) tmant;

        // scale by adjustment factor
        rslt *= factor;

        // restore sign
        if ( mant < 0 )
            rslt = - rslt;

        // optimize for normalized exponent
        if ( exp == NORM_EXP )
        {
            assert ( NORM_EXP < 0 );
            rslt /= R64_NORM_FACT;
        }
        else if ( exp < 0 )
        {
            Z32 e = - exp;
            for ( ; e > Z64_MAX_EXP; e -= Z64_MAX_EXP )
                rslt /= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            rslt /= DecConst < Z64 > :: exp10 ( e );
        }
        else if ( exp > 0 )
        {
            Z32 e = exp;
            for ( ; e > Z64_MAX_EXP; e -= Z64_MAX_EXP )
                rslt *= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            rslt *= DecConst < Z64 > :: exp10 ( e );
        }

        return rslt;
    }
    
#if HAVE_R128
    template < class T >
    R128 DecFltBase < T > :: toR128 () const
    {
        // floating point library will truncate mantissa down to 113 bits
        R128 rslt = mant;

        // optimize for normalized exponent
        if ( exp == NORM_EXP )
        {
            assert ( NORM_EXP < 0 );
            rslt /= R64_NORM_FACT;
        }
        else if ( exp < 0 )
        {
            Z32 e = - exp;
            for ( ; e > Z64_MAX_EXP; e -= Z64_MAX_EXP )
                rslt /= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            rslt /= DecConst < Z64 > :: exp10 ( e );
        }
        else if ( exp > 0 )
        {
            Z32 e = exp;
            for ( ; e > Z64_MAX_EXP; e -= Z64_MAX_EXP )
                rslt *= DecConst < Z64 > :: exp10 ( Z64_MAX_EXP );
            rslt *= DecConst < Z64 > :: exp10 ( e );
        }

        return rslt;
    }
#endif
    
    template < class T >
    Z64 DecFltBase < T > :: toZ64 () const
    {
        // if the exponent is too negative, we're all fraction
        if ( mant == 0 || exp < DecConst < T > :: minExp () )
            return 0;

        // already scaled as integer
        if ( exp == 0 )
        {
            if ( mant >= Z64_MIN && mant <= Z64_MAX )
                return ( Z64 ) mant;
        }

        // copy the mantissa
        T tmant ( mant );
        if ( mant < 0 )
            tmant = - tmant;

        // if the mantissa is negative, truncate
        if ( exp < 0 )
        {
            tmant /= DecConst < T > :: exp10 ( exp );
            if ( tmant <= Z64_MAX )
            {
                // perform sign correction
                if ( mant <= 0 )
                    return - ( Z64 ) tmant;

                // integer fits into 64 bits
                return ( Z64 ) tmant;
            }
        }
#if DEBUG
        else
        {
            // in the case of a positive exponent,
            // check for headroom, which we can't have
            // if the number is properly normalized
            T factor ( DecConst < T > :: exp10 ( exp ) );
            N32 mbits = count_significant_bits ( mant );
            N32 fbits = count_significant_bits ( factor );
            if ( mbits + fbits < sizeof ( T ) * 8 )
            {
                throw InternalError (
                    XP ( XLOC )
                    << "non-normalized decimal float"
                    );
            }
        }
#endif
        throw OverflowException (
            XP ( XLOC )
            << "numeric overflow converting decimal float to Z64"
            );
    }
    
    template < class T >
    Z128 DecFltBase < T > :: toZ128 () const
    {
        // if the exponent is zero
        if ( exp == 0 )
            return Z128 ( mant );
        
        // if the exponent is too negative, we're all fraction
        if ( mant == 0 || exp < DecConst < T > :: minExp () )
            return Z128 ( 0 );

        // copy the mantissa
        T tmant ( mant );
        if ( mant < 0 )
            tmant = - tmant;

        // if the exponent is negative
        if ( exp < 0 )
        {
            tmant /= DecConst < T > :: exp10 ( exp );

            // perform sign correction
            if ( mant <= 0 )
                tmant = - tmant;

            return Z128 ( tmant );
        }

        // in the case of a positive exponent,
        // check for sufficient headroom
        Z128 factor ( DecConst < Z128 > :: exp10 ( exp ) );
        N32 mbits = count_significant_bits ( mant );
        N32 fbits = count_significant_bits ( factor );
        if ( mbits + fbits < 128 )
            return factor * mant;
        
        throw OverflowException (
            XP ( XLOC )
            << "numeric overflow converting decimal float to Z128"
            );
    }

    // reconstitute from serialization
    template < class T >
    DecFltBase < T > :: DecFltBase ( Z64 _mant, Z32 _exp )
        : mant ( _mant )
        , exp ( _exp )
    {
        normalize ();
    }
    
#if HAVE_Z128
    template <>
    DecFltBase < Z128 > :: DecFltBase ( Z128 _mant, Z32 _exp )
        : mant ( _mant )
        , exp ( _exp )
    {
        normalize ();
    }
    
    template <>
    DecFltBase < Z64 > :: DecFltBase ( Z128 _mant, Z32 _exp )
    {
        DecFltBase < Z128 > copy ( _mant, _exp );
        cpy ( copy );
    }
#else    
    template <>
    DecFltBase < Z128 > :: DecFltBase ( const Z128 & _mant, Z32 _exp )
        : mant ( _mant )
        , exp ( _exp )
    {
        normalize ();
    }
    
    template <>
    DecFltBase < Z64 > :: DecFltBase ( const Z128 & _mant, Z32 _exp )
    {
        DecFltBase < Z128 > copy ( _mant, _exp );
        cpy ( copy );
    }
#endif
    
    template class DecFltBase < Z64 >;
    template class DecFltBase < Z128 >;
    
}
