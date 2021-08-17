/*

  vdb3.cmn.nconv

 */

#pragma once

#include <vdb3/cmn/string.hpp>
#include <vdb3/kfc/decflt.hpp>

namespace vdb3
{
    /*=====================================================*
     *                        NConv                        *
     *=====================================================*/

    /**
     * NConv
     * @brief a collection of number <=> text conversion functions
     */
    class NConv : protected TextFactory
    {
    public:
    
        /*=================================================*
         *                   realToText                    *
         *=================================================*/
        
        // convert reals to text
        inline MText R32ToText ( MText & mrgn, R32 val, Z32 prec = -1 ) const
        { return R64ToText ( mrgn, val, prec ); }
        MText R64ToText ( MText & mrgn, R64 val, Z32 prec = -1 ) const;
#if HAVE_R128
        MText R128ToText ( MText & mrgn, R128 val, Z32 prec = -1 ) const;
#endif
        MText DecFlt64ToText ( MText & mrgn,
            const DecFltPoint < Z64 > & val, Z32 prec = -1 ) const;
        MText DecFlt128ToText ( MText & mrgn,
            const DecFltPoint < Z128 > & val, Z32 prec = -1 ) const;
        template < class T > inline
        MText realToText ( MText & mrgn, const T & val, Z32 prec = -1 ) const;
        
        /*=================================================*
         *                    intToText                    *
         *=================================================*/

        // convert integers to text
        MText Z16ToText ( MText & mrgn, Z16 val, N32 base = 10 ) const;
        MText Z32ToText ( MText & mrgn, Z32 val, N32 base = 10 ) const;
        MText Z64ToText ( MText & mrgn, Z64 val, N32 base = 10 ) const;
        MText N16ToText ( MText & mrgn, N16 val, N32 base = 10 ) const;
        MText N32ToText ( MText & mrgn, N32 val, N32 base = 10 ) const;
        MText N64ToText ( MText & mrgn, N64 val, N32 base = 10 ) const;
#if HAVE_Z128
        MText Z128ToText ( MText & mrgn, Z128 val, N32 base = 10 ) const;
        MText N128ToText ( MText & mrgn, N128 val, N32 base = 10 ) const;
#else
        MText Z128ToText ( MText & mrgn, const Z128 & val, N32 base = 10 ) const;
        MText N128ToText ( MText & mrgn, const N128 & val, N32 base = 10 ) const;
#endif
        template < class T > inline
        MText intToText ( MText & mrgn, const T & val, N32 base = 10 ) const;

        /*=================================================*
         *                   boolToText                    *
         *=================================================*/

        // convert Boolean to text
        CText boolToText ( bool val ) const;
        MText boolToText ( MText & mrgn, bool val ) const;

        
        /*=================================================*
         *                   textToReal                    *
         *=================================================*/
        
        // convert text to reals
        R32 textToR32 ( const CText & str ) const;
        R64 textToR64 ( const CText & str ) const;
#if HAVE_R128
        R128 textToR128 ( const CText & str ) const;
#endif
        DecFltPoint < Z64 > textToDecFlt64 ( const CText & str ) const;
        DecFltPoint < Z128 > textToDecFlt128 ( const CText & str ) const;
        template < class T > inline
        T textToReal ( T & val, const CText & str ) const;
        
        /*=================================================*
         *                    textToInt                    *
         *=================================================*/

        // convert text to integers
        Z16 textToZ16 ( const CText & str, N32 base = 0 ) const;
        Z32 textToZ32 ( const CText & str, N32 base = 0 ) const;
        Z64 textToZ64 ( const CText & str, N32 base = 0 ) const;
        Z128 textToZ128 ( const CText & str, N32 base = 0 ) const;
        N16 textToN16 ( const CText & str, N32 base = 0 ) const;
        N32 textToN32 ( const CText & str, N32 base = 0 ) const;
        N64 textToN64 ( const CText & str, N32 base = 0 ) const;
        N128 textToN128 ( const CText & str, N32 base = 0 ) const;
        
        template < class T > inline
        T textToInt ( T & val, const CText & str, N32 base = 0 ) const;

        /*=================================================*
         *                   textToBool                    *
         *=================================================*/

        // convert text to Boolean
        bool textToBool ( const CText & str ) const;

        /*=================================================*
         *                     String                      *
         *=================================================*/

        // real to String
        template < class T > inline
        String realToString ( const RsrcKfc & rsrc, const T & val, Z32 prec = -1 ) const;

        // integer to String
        template < class T > inline
        String intToString ( const RsrcKfc & rsrc, const T & val, N32 base = 10 ) const;

        // Boolean to String
        String boolToString ( bool val ) const;
        String boolToString ( const RsrcKfc & rsrc, bool val ) const;

        // String to real
        template < class T > inline
        T stringToReal ( T & val, const String & str ) const;

        // String to integer
        template < class T > inline
        T stringToInt ( T & val, const String & str, N32 base = 0 ) const;

        // String to Boolean
        inline bool stringToBool ( const String & str ) const
        { return textToBool ( str . data () ); }

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        inline NConv ( bool do_normalize = true ) noexcept
            : normalize_mrgn ( do_normalize )
        {
        }

    private:

        template < class Z, class N, class C >
        MText ZtoText ( MText & mrgn, const Z & val, N32 base ) const;

        template < class N, class C >
        MText NtoText ( MText & mrgn, const N & val, N32 base ) const;

        bool normalize_mrgn;
        
    };

    // streaming operators that are possible now
    template < class T >
    std :: ostream & operator << ( std :: ostream & o, const DecFltPoint < T > & val );
    template < class T >
    XP & operator << ( XP & xp, const DecFltPoint < T > & val );

    
    /*=====================================================*
     *                       INLINES                       *
     *=====================================================*/

    /*

      Given that the C++ compiler will be generating and matching
      the type "T" for the templates, express them in native
      C/C++ types.

     */

    // REALS
    
    // float, double, long double, R128, DecFltPoint<Z64>, DecFltPoint<Z128>
    template <> inline
    MText NConv :: realToText < float > ( MText & mrgn, const float & val, Z32 prec ) const
    { return R64ToText ( mrgn, val, prec ); }

    template <> inline
    MText NConv :: realToText < double > ( MText & mrgn, const double & val, Z32 prec ) const
    { return R64ToText ( mrgn, val, prec ); }

    template <> inline
    MText NConv :: realToText < long double > ( MText & mrgn, const long double & val, Z32 prec ) const
    {
#if HAVE_R128
        return R128ToText ( mrgn, val, prec );
#else
        return R64ToText ( mrgn, ( double ) val, prec );
#endif
    }

#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
    template <> inline
    MText NConv :: realToText < R128 > ( MText & mrgn, const R128 & val, Z32 prec ) const
    { return R128ToText ( mrgn, val, prec ); }
#endif
    template <> inline
    MText NConv :: realToText < DecFltPoint < Z64 > > ( MText & mrgn,
        const DecFltPoint < Z64 > & val, Z32 prec ) const
    { return DecFlt64ToText ( mrgn, val, prec ); }
    template <> inline
    MText NConv :: realToText < DecFltPoint < Z128 > > ( MText & mrgn,
        const DecFltPoint < Z128 > & val, Z32 prec ) const
    { return DecFlt128ToText ( mrgn, val, prec ); }

    // INTEGERS and NATURALS

    // short
    template <> inline
    MText NConv :: intToText < signed short int >
        ( MText & mrgn, const signed short int & val, N32 base ) const
    { return Z16ToText ( mrgn, val, base ); }

    template <> inline
    MText NConv :: intToText < unsigned short int >
        ( MText & mrgn, const unsigned short int & val, N32 base ) const
    { return N16ToText ( mrgn, val, base ); }

    // int
    template <> inline
    MText NConv :: intToText < signed int >
        ( MText & mrgn, const signed int & val, N32 base ) const
    { return Z32ToText ( mrgn, val, base ); }

    template <> inline
    MText NConv :: intToText < unsigned int >
        ( MText & mrgn, const unsigned int & val, N32 base ) const
    { return N32ToText ( mrgn, val, base ); }

    // long
    template <> inline
    MText NConv :: intToText < signed long int >
        ( MText & mrgn, const signed long int & val, N32 base ) const
    { return Z64ToText ( mrgn, val, base ); }

    template <> inline
    MText NConv :: intToText < unsigned long int >
        ( MText & mrgn, const unsigned long int & val, N32 base ) const
    { return N64ToText ( mrgn, val, base ); }

    // long long
    template <> inline
    MText NConv :: intToText < signed long long int >
        ( MText & mrgn, const signed long long int & val, N32 base ) const
    { return Z64ToText ( mrgn, val, base ); }

    template <> inline
    MText NConv :: intToText < unsigned long long int >
        ( MText & mrgn, const unsigned long long int & val, N32 base ) const
    { return N64ToText ( mrgn, val, base ); }

    // Z128, N128
    template <> inline
    MText NConv :: intToText < Z128 > ( MText & mrgn, const Z128 & val, N32 base ) const
    { return Z128ToText ( mrgn, val, base ); }

    template <> inline
    MText NConv :: intToText < N128 > ( MText & mrgn, const N128 & val, N32 base ) const
    { return N128ToText ( mrgn, val, base ); }


    // REAL
    
    // float, double, long double, R128, DecFltPoint<Z64>, DecFltPoint<Z128>
    template <> inline
    float NConv :: textToReal < float > ( float & val, const CText & str ) const
    { return val = textToR32 ( str ); }

    template <> inline
    double NConv :: textToReal < double > ( double & val, const CText & str ) const
    { return val = textToR64 ( str ); }

    template <> inline
    long double NConv :: textToReal < long double >
        ( long double & val, const CText & str ) const
    {
#if ! HAVE_R128
        val = textToR64 ( str );
#else
        val = sizeof ( long double ) == sizeof ( R128 ) ?
            ( long double ) textToR128 ( str ):
            ( long double ) textToR64 ( str );
#endif
        return val;
    }

#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128
    template <> inline
    R128 NConv :: textToReal < R128 > ( R128 & val, const CText & str ) const
    { return val = textToR128 ( str ); }
#endif
    template <> inline
    DecFltPoint < Z64 >  NConv :: textToReal < DecFltPoint < Z64 > >
        ( DecFltPoint < Z64 > & val, const CText & str ) const
    { return val = textToDecFlt64 ( str ); }

    template <> inline
    DecFltPoint < Z128 >  NConv :: textToReal < DecFltPoint < Z128 > >
        ( DecFltPoint < Z128 > & val, const CText & str ) const
    { return val = textToDecFlt128 ( str ); }

    // INTEGERS and NATURALS
    
    // short
    template <> inline
    signed short int NConv :: textToInt < signed short int >
        ( signed short int & val, const CText & str, N32 base ) const
    { return val = textToZ16 ( str, base ); }

    template <> inline
    unsigned short int NConv :: textToInt < unsigned short int >
        ( unsigned short int & val, const CText & str, N32 base ) const
    { return val = textToN16 ( str, base ); }

    // int
    template <> inline
    signed int NConv :: textToInt < signed int >
        ( signed int & val, const CText & str, N32 base ) const
    { return val = textToZ32 ( str, base ); }

    template <> inline
    unsigned int NConv :: textToInt < unsigned int >
        ( unsigned int & val, const CText & str, N32 base ) const
    { return val = textToN32 ( str, base ); }

    // long
    template <> inline
    signed long int NConv :: textToInt < signed long int >
        ( signed long int & val, const CText & str, N32 base ) const
    {
        val = ( sizeof ( signed long int ) == 4 ) ?
            ( signed long int ) textToZ32 ( str, base ):
            ( signed long int ) textToZ64 (  str, base );
        return val;
    }

    template <> inline
    unsigned long int NConv :: textToInt < unsigned long int >
        ( unsigned long int & val, const CText & str, N32 base ) const
    {
        val = ( sizeof ( unsigned long int ) == 4 ) ?
            ( unsigned long int ) textToN32 ( str, base ):
            ( unsigned long int ) textToN64 ( str, base );
        return val;
    }

    // long long
    template <> inline
    signed long long int NConv :: textToInt < signed long long int >
        ( signed long long int & val, const CText & str, N32 base ) const
    { return val = textToZ64 ( str, base ); }

    template <> inline
    unsigned long long int NConv :: textToInt < unsigned long long int >
        ( unsigned long long int & val, const CText & str, N32 base ) const
    { return val = textToN64 ( str, base ); }

    // Z128, N128
    template <> inline
    Z128 NConv :: textToInt < Z128 > ( Z128 & val, const CText & str, N32 base ) const
    { return val = textToZ128 ( str, base ); }

    template <> inline
    N128 NConv :: textToInt < N128 > ( N128 & val, const CText & str, N32 base ) const
    { return val = textToN128 ( str, base ); }


    // STRINGS

    template < class T > inline
    String NConv :: realToString ( const RsrcKfc & rsrc, const T & val, Z32 prec ) const
    {
        UTF8 buffer [ 1024 ];
        MText mrgn = makeMText ( buffer, sizeof buffer, 0, 0 );

        // don't normalize MText
        NConv nc ( false );
        
        CText t = nc . realToText < T > ( mrgn, val, prec );
        return String ( rsrc, t );
    }

    template < class T > inline
    String NConv :: intToString ( const RsrcKfc & rsrc, const T & val, N32 base ) const
    {
        UTF8 buffer [ 256 ];
        MText mrgn = makeMText ( buffer, sizeof buffer, 0, 0 );

        // don't normalize MText
        NConv nc ( false );
        
        CText t = nc . intToText < T > ( mrgn, val, base );
        return String ( rsrc, t );
    }

    template < class T > inline
    T NConv :: stringToReal ( T & val, const String & str ) const
    {
        return textToReal < T > ( val, str . data () );
    }

    template < class T > inline
    T NConv :: stringToInt ( T & val, const String & str, N32 base ) const
    {
        return textToInt < T > ( val, str . data (), base );
    }
}
