/*

  vdb3.kfc.decflt

 */

#pragma once

#include <vdb3/kfc/defs.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                     DecFltBase                      *
     *=====================================================*/

    template < class T >
    class DecFltBase
    {
    public:

        void normalize ();
        void denormalize ();

        Z64 getMantissa64 () const;
        Z128 getMantissa128 () const noexcept;
        Z32 getExponent () const noexcept;

        R64 toR64 () const;
#if HAVE_R128
        R128 toR128 () const;
#endif
        Z64 toZ64 () const;
        Z128 toZ128 () const;

        void negate ();

        void trunc () noexcept;
        void round () noexcept;
        void ceil () noexcept;
        void frac () noexcept;

        // real numbers
        void add ( R64 val );
        void sub ( R64 val );
        void mul ( R64 val );
        void div ( R64 val );
        void cpy ( R64 val );
        int  cmp ( R64 val ) const;
        DecFltBase ( R64 val );
#if HAVE_R128
        void add ( R128 val );
        void sub ( R128 val );
        void mul ( R128 val );
        void div ( R128 val );
        void cpy ( R128 val );
        int  cmp ( R128 val ) const;
        DecFltBase ( R128 val );
#endif
        // integers
        void add ( Z64 val );
        void sub ( Z64 val );
        void mul ( Z64 val );
        void div ( Z64 val );
        void cpy ( Z64 val );
        int  cmp ( Z64 val ) const;
        DecFltBase ( Z64 val );
#if HAVE_Z128
        void add ( Z128 val );
        void sub ( Z128 val );
        void mul ( Z128 val );
        void div ( Z128 val );
        void cpy ( Z128 val );
        int  cmp ( Z128 val ) const;
        DecFltBase ( Z128 val );
#else
        void add ( const Z128 & val );
        void sub ( const Z128 & val );
        void mul ( const Z128 & val );
        void div ( const Z128 & val );
        void cpy ( const Z128 & val );
        int  cmp ( const Z128 & val ) const;
        DecFltBase ( const Z128 & val );
#endif
        // decimal floating point
        void add ( const DecFltBase < Z64 > & val );
        void sub ( const DecFltBase < Z64 > & val );
        void mul ( const DecFltBase < Z64 > & val );
        void div ( const DecFltBase < Z64 > & val );
        void cpy ( const DecFltBase < Z64 > & val );
        int  cmp ( const DecFltBase < Z64 > & val ) const;
        DecFltBase ( const DecFltBase < Z64 > & val );

        void add ( const DecFltBase < Z128 > & val );
        void sub ( const DecFltBase < Z128 > & val );
        void mul ( const DecFltBase < Z128 > & val );
        void div ( const DecFltBase < Z128 > & val );
        void cpy ( const DecFltBase < Z128 > & val );
        int  cmp ( const DecFltBase < Z128 > & val ) const;
        DecFltBase ( const DecFltBase < Z128 > & val );

        // reconstitute from serialization
        DecFltBase ( Z64 mant, Z32 exp );
#if HAVE_Z128
        DecFltBase ( Z128 mant, Z32 exp );
#else
        DecFltBase ( const Z128 & mant, Z32 exp );
#endif

        DecFltBase () noexcept;
        ~ DecFltBase () noexcept;

    private:

        void div ( DecFltBase < T > & num, const DecFltBase < T > & den );

        T mant;         // mantissa or "significand"
        Z32 exp;        // decimal exponent
    };

    /*=====================================================*
     *                     DecFltPoint                     *
     *=====================================================*/

    /**
     * DecFltPoint
     * @brief a base-10 floating-point alternative to base-2 floating-point
     *
     * rather than using binary floating point, which often cannot
     * represent zero, and acquires noise over time, use a
     * decimal floating point representation.
     *
     * all arithmetic operations detect over- and under-flow
     * and throw exceptions when discovered.
     */
    template < class T = Z64 >
    class DecFltPoint : public DecFltBase < T >
    {
    public:

        typedef DecFltBase < T > DFB;
        typedef DecFltPoint < T > DFP;
        
        inline void normalize ()
        { DFB :: normalize (); }
        inline void denormalize ()
        { DFB :: denormalize (); }
        
        inline Z64 getMantissa64 () const
        { return DFB :: getMantissa64 (); }
        inline Z128 getMantissa128 () const noexcept
        { return DFB :: getMantissa128 (); }
        inline Z32 getExponent () const noexcept
        { return DFB :: getExponent (); }

        // cast operators
        inline operator R64 () const
        { return DFB :: toR64(); }
#if HAVE_R128
        inline operator R128 () const
        { return DFB :: toR128(); }
#endif
        inline operator Z64 () const
        { return DFB :: toZ64(); }
        inline operator Z128 () const
        { return DFB :: toZ128(); }

        // C++ is so incredibly useless that
        // even given the explicit casts above,
        // and explicit constructors, a statement like
        //
        //        Z128 z = ( Z128 ) r;
        //
        // the right-hand expression MUST evaluate to
        // the cast operator above, leaving zero
        // ambiguity as to how to construct "z" on the
        // left-hand side. instead, the cast is being
        // selected based upon the LH side, which can
        // supposedly be constructed from either Z64
        // OR Z128. but that's not true, either: the
        // construction from Z64 is itself EXPLICIT.
        inline Z64 toZ64 () const
        { return DFB :: toZ64 (); }
        inline Z128 toZ128 () const
        { return DFB :: toZ128 (); }

        // negation
        inline DFP operator - () const
        { return DFP ( self ) . negate (); }
        inline DFP & negate ()
        { DFB :: negate (); return self; }

        // trunc, round, ceil and frac
        inline DFP & trunc ()
        { DFB :: trunc (); return self; }
        inline DFP & round ()
        { DFB :: round (); return self; }
        inline DFP & ceil ()
        { DFB :: ceil (); return self; }
        inline DFP & frac ()
        { DFB :: frac (); return self; }

        // addition
        inline DFP operator + ( signed short int val ) const
        { return DFP ( self ) += ( Z64 ) val; }
        inline DFP operator + ( signed int val ) const
        { return DFP ( self ) += ( Z64 ) val; }
        inline DFP operator + ( signed long int val ) const
        { return DFP ( self ) += ( Z64 ) val; }
        inline DFP operator + ( signed long long int val ) const
        { return DFP ( self ) += ( Z64 ) val; }
        inline DFP operator + ( const Z128 & val ) const
        { return DFP ( self ) += val; }
        inline DFP operator + ( float val ) const
        { return DFP ( self ) += ( R64 ) val; }
        inline DFP operator + ( double val ) const
        { return DFP ( self ) += ( R64 ) val; }
#if ! HAVE_R128
        inline DFP operator + ( long double val ) const
        { return DFP ( self ) += ( R64 ) val; }
#else
        inline DFP operator + ( long double val ) const
        { return DFP ( self ) += ( R128 ) val; }
#if LONG_DOUBLE_IS_NOT_R128
        inline DFP operator + ( R128 val ) const
        { return DFP ( self ) += ( R128 ) val; }
#endif
#endif
        template < class T2 >
        inline DFP operator + ( const DecFltPoint < T2 > & val ) const
        { return DFP ( self ) += val; }

        // subtraction
        inline DFP operator - ( signed short int val ) const
        { return DFP ( self ) -= ( Z64 ) val; }
        inline DFP operator - ( signed int val ) const
        { return DFP ( self ) -= ( Z64 ) val; }
        inline DFP operator - ( signed long int val ) const
        { return DFP ( self ) -= ( Z64 ) val; }
        inline DFP operator - ( signed long long int val ) const
        { return DFP ( self ) -= ( Z64 ) val; }
        inline DFP operator - ( const Z128 & val ) const
        { return DFP ( self ) -= val; }
        inline DFP operator - ( float val ) const
        { return DFP ( self ) -= ( R64 ) val; }
        inline DFP operator - ( double val ) const
        { return DFP ( self ) -= ( R64 ) val; }
#if ! HAVE_R128
        inline DFP operator - ( long double val ) const
        { return DFP ( self ) -= ( R64 ) val; }
#else
        inline DFP operator - ( long double val ) const
        { return DFP ( self ) -= ( R128 ) val; }
#if LONG_DOUBLE_IS_NOT_R128
        inline DFP operator - ( R128 val ) const
        { return DFP ( self ) -= ( R128 ) val; }
#endif
#endif
        template < class T2 >
        inline DFP operator - ( const DecFltPoint < T2 > & val ) const
        { return DFP ( self ) -= val; }

        // multiplication
        inline DFP operator * ( signed short int val ) const
        { return DFP ( self ) *= ( Z64 ) val; }
        inline DFP operator * ( signed int val ) const
        { return DFP ( self ) *= ( Z64 ) val; }
        inline DFP operator * ( signed long int val ) const
        { return DFP ( self ) *= ( Z64 ) val; }
        inline DFP operator * ( signed long long int val ) const
        { return DFP ( self ) *= ( Z64 ) val; }
        inline DFP operator * ( const Z128 & val ) const
        { return DFP ( self ) *= val; }
        inline DFP operator * ( float val ) const
        { return DFP ( self ) *= ( R64 ) val; }
        inline DFP operator * ( double val ) const
        { return DFP ( self ) *= ( R64 ) val; }
#if ! HAVE_R128
        inline DFP operator * ( long double val ) const
        { return DFP ( self ) *= ( R64 ) val; }
#else
        inline DFP operator * ( long double val ) const
        { return DFP ( self ) *= ( R128 ) val; }
#if LONG_DOUBLE_IS_NOT_R128
        inline DFP operator * ( R128 val ) const
        { return DFP ( self ) *= ( R128 ) val; }
#endif
#endif
        template < class T2 >
        inline DFP operator * ( const DecFltPoint < T2 > & val ) const
        { return DFP ( self ) *= val; }

        // division
        inline DFP operator / ( signed short int val ) const
        { return DFP ( self ) /= ( Z64 ) val; }
        inline DFP operator / ( signed int val ) const
        { return DFP ( self ) /= ( Z64 ) val; }
        inline DFP operator / ( signed long int val ) const
        { return DFP ( self ) /= ( Z64 ) val; }
        inline DFP operator / ( signed long long int val ) const
        { return DFP ( self ) /= ( Z64 ) val; }
        inline DFP operator / ( const Z128 & val ) const
        { return DFP ( self ) /= val; }
        inline DFP operator / ( float val ) const
        { return DFP ( self ) /= ( R64 ) val; }
        inline DFP operator / ( double val ) const
        { return DFP ( self ) /= ( R64 ) val; }
#if ! HAVE_R128
        inline DFP operator / ( long double val ) const
        { return DFP ( self ) /= ( R64 ) val; }
#else
        inline DFP operator / ( long double val ) const
        { return DFP ( self ) /= ( R128 ) val; }
#if LONG_DOUBLE_IS_NOT_R128
        inline DFP operator / ( R128 val ) const
        { return DFP ( self ) /= ( R128 ) val; }
#endif
#endif
        template < class T2 >
        inline DFP operator / ( const DecFltPoint < T2 > & val ) const
        { return DFP ( self ) /= val; }

        // comparison - lt
        inline bool operator < ( signed short int val ) const
        { return self < ( Z64 ) val; }
        inline bool operator < ( signed int val ) const
        { return self < ( Z64 ) val; }
        inline bool operator < ( signed long int val ) const
        { return self < ( Z64 ) val; }
        inline bool operator < ( signed long long int val ) const
        { return DFB :: cmp ( ( Z64 ) val ) < 0; }
        inline bool operator < ( const Z128 & val ) const
        { return DFB :: cmp ( val ) < 0; }
        inline bool operator < ( float val ) const
        { return self < ( R64 ) val; }
        inline bool operator < ( double val ) const
        { return DFB :: cmp ( ( R64 ) val ) < 0; }
#if ! HAVE_R128
        inline bool operator < ( long double val ) const
        { return self < ( R64 ) val; }
#else
        inline bool operator < ( long double val ) const
        { return DFB :: cmp ( ( R128 ) val ) < 0; }
#if LONG_DOUBLE_IS_NOT_R128
        inline bool operator < ( R128 val ) const
        { return DFB :: cmp ( ( R128 ) val ) < 0; }
#endif
#endif
        template < class T2 >
        inline bool operator < ( const DecFltPoint < T2 > & val ) const
        { return DFB :: cmp ( val ) < 0; }

        // comparison - le
        inline bool operator <= ( signed short int val ) const
        { return self <= ( Z64 ) val; }
        inline bool operator <= ( signed int val ) const
        { return self <= ( Z64 ) val; }
        inline bool operator <= ( signed long int val ) const
        { return self <= ( Z64 ) val; }
        inline bool operator <= ( signed long long int val ) const
        { return DFB :: cmp ( ( Z64 ) val ) <= 0; }
        inline bool operator <= ( const Z128 & val ) const
        { return DFB :: cmp ( val ) <= 0; }
        inline bool operator <= ( float val ) const
        { return self <= ( R64 ) val; }
        inline bool operator <= ( double val ) const
        { return DFB :: cmp ( ( R64 ) val ) <= 0; }
#if ! HAVE_R128
        inline bool operator <= ( long double val ) const
        { return self <= ( R64 ) val; }
#else
        inline bool operator <= ( long double val ) const
        { return DFB :: cmp ( ( R128 ) val ) <= 0; }
#if LONG_DOUBLE_IS_NOT_R128
        inline bool operator <= ( R128 val ) const
        { return DFB :: cmp ( ( R128 ) val ) <= 0; }
#endif
#endif
        template < class T2 >
        inline bool operator <= ( const DecFltPoint < T2 > & val ) const
        { return DFB :: cmp ( val ) <= 0; }

        // comparison - eq
        inline bool operator == ( signed short int val ) const
        { return self == ( Z64 ) val; }
        inline bool operator == ( signed int val ) const
        { return self == ( Z64 ) val; }
        inline bool operator == ( signed long int val ) const
        { return self == ( Z64 ) val; }
        inline bool operator == ( signed long long int val ) const
        { return DFB :: cmp ( ( Z64 ) val ) == 0; }
        inline bool operator == ( const Z128 & val ) const
        { return DFB :: cmp ( val ) == 0; }
        inline bool operator == ( float val ) const
        { return self == ( R64 ) val; }
        inline bool operator == ( double val ) const
        { return DFB :: cmp ( ( R64 ) val ) == 0; }
#if ! HAVE_R128
        inline bool operator == ( long double val ) const
        { return self == ( R64 ) val; }
#else
        inline bool operator == ( long double val ) const
        { return DFB :: cmp ( ( R128 ) val ) == 0; }
#if LONG_DOUBLE_IS_NOT_R128
        inline bool operator == ( R128 val ) const
        { return DFB :: cmp ( ( R128 ) val ) == 0; }
#endif
#endif
        template < class T2 >
        inline bool operator == ( const DecFltPoint < T2 > & val ) const
        { return DFB :: cmp ( val ) == 0; }

        // comparison - ne
        inline bool operator != ( signed short int val ) const
        { return self != ( Z64 ) val; }
        inline bool operator != ( signed int val ) const
        { return self != ( Z64 ) val; }
        inline bool operator != ( signed long int val ) const
        { return self != ( Z64 ) val; }
        inline bool operator != ( signed long long int val ) const
        { return DFB :: cmp ( ( Z64 ) val ) != 0; }
        inline bool operator != ( const Z128 & val ) const
        { return DFB :: cmp ( val ) != 0; }
        inline bool operator != ( float val ) const
        { return self != ( R64 ) val; }
        inline bool operator != ( double val ) const
        { return DFB :: cmp ( ( R64 ) val ) != 0; }
#if ! HAVE_R128
        inline bool operator != ( long double val ) const
        { return self != ( R64 ) val; }
#else
        inline bool operator != ( long double val ) const
        { return DFB :: cmp ( ( R128 ) val ) != 0; }
#if LONG_DOUBLE_IS_NOT_R128
        inline bool operator != ( R128 val ) const
        { return DFB :: cmp ( ( R128 ) val ) != 0; }
#endif
#endif
        template < class T2 >
        inline bool operator != ( const DecFltPoint < T2 > & val ) const
        { return DFB :: cmp ( val ) != 0; }

        // comparison - ge
        inline bool operator >= ( signed short int val ) const
        { return self >= ( Z64 ) val; }
        inline bool operator >= ( signed int val ) const
        { return self >= ( Z64 ) val; }
        inline bool operator >= ( signed long int val ) const
        { return self >= ( Z64 ) val; }
        inline bool operator >= ( signed long long int val ) const
        { return DFB :: cmp ( ( Z64 ) val ) >= 0; }
        inline bool operator >= ( const Z128 & val ) const
        { return DFB :: cmp ( val ) >= 0; }
        inline bool operator >= ( float val ) const
        { return self >= ( R64 ) val; }
        inline bool operator >= ( double val ) const
        { return DFB :: cmp ( ( R64 ) val ) >= 0; }
#if ! HAVE_R128
        inline bool operator >= ( long double val ) const
        { return self >= ( R64 ) val; }
#else
        inline bool operator >= ( long double val ) const
        { return DFB :: cmp ( ( R128 ) val ) >= 0; }
#if LONG_DOUBLE_IS_NOT_R128
        inline bool operator >= ( R128 val ) const
        { return DFB :: cmp ( ( R128 ) val ) >= 0; }
#endif
#endif
        template < class T2 >
        inline bool operator >= ( const DecFltPoint < T2 > & val ) const
        { return DFB :: cmp ( val ) >= 0; }

        // comparison - gt
        inline bool operator > ( signed short int val ) const
        { return self > ( Z64 ) val; }
        inline bool operator > ( signed int val ) const
        { return self > ( Z64 ) val; }
        inline bool operator > ( signed long int val ) const
        { return self > ( Z64 ) val; }
        inline bool operator > ( signed long long int val ) const
        { return DFB :: cmp ( ( Z64 ) val ) > 0; }
        inline bool operator > ( const Z128 & val ) const
        { return DFB :: cmp ( val ) > 0; }
        inline bool operator > ( float val ) const
        { return self > ( R64 ) val; }
        inline bool operator > ( double val ) const
        { return DFB :: cmp ( ( R64 ) val ) > 0; }
#if ! HAVE_R128
        inline bool operator > ( long double val ) const
        { return self > ( R64 ) val; }
#else
        inline bool operator > ( long double val ) const
        { return DFB :: cmp ( ( R128 ) val ) > 0; }
#if LONG_DOUBLE_IS_NOT_R128
        inline bool operator > ( R128 val ) const
        { return DFB :: cmp ( ( R128 ) val ) > 0; }
#endif
#endif
        template < class T2 >
        inline bool operator > ( const DecFltPoint < T2 > & val ) const
        { return DFB :: cmp ( val ) > 0; }

        // add-assign
        inline DFP & operator += ( signed short int val )
        { return self += ( Z64 ) val; }
        inline DFP & operator += ( signed int val )
        { return self += ( Z64 ) val; }
        inline DFP & operator += ( signed long int val )
        { return self += ( Z64 ) val; }
        inline DFP & operator += ( signed long long int val )
        { DFB :: add ( ( Z64 ) val ); return self; }
        inline DFP & operator += ( const Z128 & val )
        { DFB :: add ( val ); return self; }
        inline DFP & operator += ( float val )
        { return self += ( R64 ) val; }
        inline DFP & operator += ( double val )
        { DFB :: add ( ( R64 ) val ); return self; }
#if ! HAVE_R128
        inline DFP & operator += ( long double val )
        { DFB :: add ( ( R64 ) val ); return self; }
#else
        inline DFP & operator += ( long double val )
        { DFB :: add ( ( R128 ) val ); return self; }
#if LONG_DOUBLE_IS_NOT_R128
        inline DFP & operator += ( R128 val )
        { DFB :: add ( val ); return self; }
#endif
#endif
        template < class T2 >
        inline DFP & operator += ( const DecFltPoint < T2 > & val )
        { DFB :: add ( val ); return self; }

        // subtract-assign
        inline DFP & operator -= ( signed short int val )
        { return self -= ( Z64 ) val; }
        inline DFP & operator -= ( signed int val )
        { return self -= ( Z64 ) val; }
        inline DFP & operator -= ( signed long int val )
        { return self -= ( Z64 ) val; }
        inline DFP & operator -= ( signed long long int val )
        { DFB :: sub ( ( Z64 ) val ); return self; }
        inline DFP & operator -= ( const Z128 & val )
        { DFB :: sub ( val ); return self; }
        inline DFP & operator -= ( float val )
        { return self -= ( R64 ) val; }
        inline DFP & operator -= ( double val )
        { DFB :: sub ( ( R64 ) val ); return self; }
#if ! HAVE_R128
        inline DFP & operator -= ( long double val )
        { DFB :: sub ( ( R64 ) val ); return self; }
#else
        inline DFP & operator -= ( long double val )
        { DFB :: sub ( ( R128 ) val ); return self; }
#if LONG_DOUBLE_IS_NOT_R128
        inline DFP & operator -= ( R128 val )
        { DFB :: sub ( val ); return self; }
#endif
#endif
        template < class T2 >
        inline DFP & operator -= ( const DecFltPoint < T2 > & val )
        { DFB :: sub ( val ); return self; }

        // multiply-assign
        inline DFP & operator *= ( signed short int val )
        { return self *= ( Z64 ) val; }
        inline DFP & operator *= ( signed int val )
        { return self *= ( Z64 ) val; }
        inline DFP & operator *= ( signed long int val )
        { return self *= ( Z64 ) val; }
        inline DFP & operator *= ( signed long long int val )
        { DFB :: mul ( ( Z64 ) val ); return self; }
        inline DFP & operator *= ( const Z128 & val )
        { DFB :: mul ( val ); return self; }
        inline DFP & operator *= ( float val )
        { return self *= ( R64 ) val; }
        inline DFP & operator *= ( double val )
        { DFB :: mul ( ( R64 ) val ); return self; }
#if ! HAVE_R128
        inline DFP & operator *= ( long double val )
        { DFB :: mul ( ( R64 ) val ); return self; }
#else
        inline DFP & operator *= ( long double val )
        { DFB :: mul ( ( R128 ) val ); return self; }
#if LONG_DOUBLE_IS_NOT_R128
        inline DFP & operator *= ( R128 val )
        { DFB :: mul ( val ); return self; }
#endif
#endif
        template < class T2 >
        inline DFP & operator *= ( const DecFltPoint < T2 > & val )
        { DFB :: mul ( val ); return self; }

        // divide-assign
        inline DFP & operator /= ( signed short int val )
        { return self /= ( Z64 ) val; }
        inline DFP & operator /= ( signed int val )
        { return self /= ( Z64 ) val; }
        inline DFP & operator /= ( signed long int val )
        { return self /= ( Z64 ) val; }
        inline DFP & operator /= ( signed long long int val )
        { DFB :: div ( ( Z64 ) val ); return self; }
        inline DFP & operator /= ( const Z128 & val )
        { DFB :: div ( val ); return self; }
        inline DFP & operator /= ( float val )
        { return self /= ( R64 ) val; }
        inline DFP & operator /= ( double val )
        { DFB :: div ( ( R64 ) val ); return self; }
#if ! HAVE_R128
        inline DFP & operator /= ( long double val )
        { DFB :: div ( ( R64 ) val ); return self; }
#else
        inline DFP & operator /= ( long double val )
        { DFB :: div ( ( R128 ) val ); return self; }
#if LONG_DOUBLE_IS_NOT_R128
        inline DFP & operator /= ( R128 val )
        { DFB :: div ( val ); return self; }
#endif
#endif
        template < class T2 >
        inline DFP & operator /= ( const DecFltPoint < T2 > & val )
        { DFB :: div ( val ); return self; }

        // copy operator
        inline DFP & operator = ( signed short int val )
        { return self = ( Z64 ) val; }
        inline DFP & operator = ( signed int val )
        { return self = ( Z64 ) val; }
        inline DFP & operator = ( signed long int val )
        { return self = ( Z64 ) val; }
        inline DFP & operator = ( signed long long int val )
        { DFB :: cpy ( ( Z64 ) val ); return self; }
        inline DFP & operator = ( const Z128 & val )
        { DFB :: cpy ( val ); return self; }
        inline DFP & operator = ( float val )
        { return self = ( R64 ) val; }
        inline DFP & operator = ( double val )
        { DFB :: cpy ( ( R64 ) val ); return self; }
#if ! HAVE_R128
        inline DFP & operator = ( long double val )
        { DFB :: cpy ( ( R64 ) val ); return self; }
#else
        inline DFP & operator = ( long double val )
        { DFB :: cpy ( ( R128 ) val ); return self; }
#if LONG_DOUBLE_IS_NOT_R128
        inline DFP & operator = ( R128 val )
        { DFB :: cpy ( val ); return self; }
#endif
#endif
        template < class T2 >
        inline DFP & operator = ( const DecFltPoint < T2 > & val )
        { DFB :: cpy ( val ); return self; }

        // constructors
        DecFltPoint ( signed short int val )
            : DecFltPoint ( ( Z64 ) val ) {}
        DecFltPoint ( signed int val )
            : DecFltPoint ( ( Z64 ) val ) {}
        DecFltPoint ( signed long int val )
            : DecFltPoint ( ( Z64 ) val ) {}
        DecFltPoint ( signed long long int val )
            : DFB ( val ) {}
        DecFltPoint ( const Z128 & val )
            : DFB ( val ) {}
        DecFltPoint ( float val )
            : DecFltPoint ( ( R64 ) val ) {}
        DecFltPoint ( double val )
            : DFB ( ( R64 ) val ) {}
#if ! HAVE_R128
        DecFltPoint ( long double val )
            : DFB ( ( R64 ) val ) {}
#else
        DecFltPoint ( long double val )
            : DFB ( ( R128 ) val ) {}
#if LONG_DOUBLE_IS_NOT_R128
        DecFltPoint ( R128 val )
            : DFB ( ( R128 ) val ) {}
#endif
#endif
        template < class T2 >
        DecFltPoint ( const DecFltPoint < T2 > & val )
            : DFB ( val ) {}

        // reconstitute from serialization
        template < class T2 >
        DecFltPoint ( const T2 & mant, Z32 exp )
            : DFB ( mant, exp ) {}

        DecFltPoint () noexcept {}
    };
}
