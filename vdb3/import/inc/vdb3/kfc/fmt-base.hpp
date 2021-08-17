/*

  vdb3.kfc.fmt-base


 */

#pragma once

#include <vdb3/kfc/defs.hpp>
#include <vdb3/kfc/ptr.hpp>

namespace vdb3
{
    class TextRgn;

    /*=====================================================*
     *                       FmtBase                       *
     *=====================================================*/
    
    /**
     * FmtBase
     *  @brief performs mainly numeric conversions
     */
    class FmtBase
    {
    public:

        /**
         * setRadix
         *  @param radix an integer in 2..36
         */
        void setRadix ( N32 radix ) noexcept;
        
        /**
         * setMinIntegerDigits
         *  @brief sets the minimum number of integer digits
         *  @param count an integer in 0..255
         *
         *  causes integers and naturals to be left-extended with fill
         *  character in order to reach minimum width count.
         *
         *  also affects the number of digits in the integer portion
         *  of a floating point number.
         */
        void setMinIntegerDigits ( N32 count, ASCII fill = ' ' ) noexcept;

        /**
         * setMinFractionDigits
         *  @brief sets minimum number of fractional digits
         *  @param precision an integer in 0..255
         *
         *  causes integers to be zero-extended to minimum
         *  establishes number of fractional digits for reals
         *  sets upper limit on string length
         */
        void setMinFractionDigits ( N32 precision ) noexcept;

        /**
         * setFieldWidth
         *  @brief sets the minimum field width
         *  @param width an integer in 0..255
         *
         *  field will be padded with spaces to left or right or both
         *  depending upon alignment to achieve minimum field width.
         *
         *  default value is 0.
         */
        void setFieldWidth ( N32 width ) noexcept;

        /**
         * setAlignment
         *  @brief establish field alignment
         *  @param align is a value in { left, center, right }
         *
         *  default values are:
         *   right for numbers
         *   left for everything else.
         */
        enum FieldAlign { left, center, right };
        void setAlignment ( FieldAlign align ) noexcept;

    protected:

        /**
         * putXX
         *  @brief a number of specifically named conversion functions
         *   that are awkwardly specifically named to allow the list to
         *   be extended without confusing the delicate sensitiblities of
         *   C++ compilers.
         *
         *   these functions perform conversion, left-extensions, truncation,
         *   and alignment within a field, resulting in one or more calls to
         *   a "write()" function.
         *
         *   the typing is according to the C++ compiler's view of the world,
         *   instead of the programmer, so "signed long long int" may or may
         *   not be identical to "signed long int" in terms of bits and the
         *   hardware, but considered distinct within C.
         */

        //!< put a Boolean
        void putBool ( bool val );

        //!< put integer numbers
        inline void putSSInt ( signed short int val )
        { putSLLInt ( ( signed long long int ) val ); }
        inline void putSInt ( signed int val )
        { putSLLInt ( ( signed long long int ) val ); }
        inline void putSLInt ( signed long int val )
        { putSLLInt ( ( signed long long int ) val ); }
        void putSLLInt ( signed long long int val );
#if HAVE_Z128
        void putSInt128 ( signed __int128 val );
#else
        void putSInt128 ( const Z128 & val );
#endif
        //!< put natural numbers
        inline void putUSInt ( unsigned short int val )
        { putULLInt ( ( unsigned long long int ) val ); }
        inline void putUInt ( unsigned int val )
        { putULLInt ( ( unsigned long long int ) val ); }
        inline void putULInt ( unsigned long int val )
        { putULLInt ( ( unsigned long long int ) val ); }
        void putULLInt ( unsigned long long int val );
#if HAVE_Z128
        void putUInt128 ( unsigned __int128 val );
#else
        void putUInt128 ( const N128 & val );
#endif
        //!< put real numbers
        inline void putF ( float val )
        { putD ( ( double ) val ); }
        void putD ( double val );
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128 && HAVE_QUADMATH
        inline void putLD ( long double val )
        { putF128 ( ( __float128 ) val ); }
        void putF128 ( __float128 val );
#else
        void putLD ( long double val );
#endif
        //!< put an arbitrary pointer
        void putPtr ( const Ptr & val );

        /**
         * write
         *  @brief the main means of getting data back out
         *  @param val is a TextRgn providing maximum information
         *   but without risk of incurring exceptions
         */
        virtual void write ( const TextRgn & val ) = 0;

        /**
         * fill
         *  @brief generate filler to output
         *  @param ch is the ASCII fill character
         *  @param repeat is the repeat count
         */
        virtual void fill ( ASCII ch, count_t repeat ) = 0;

        FmtBase () noexcept;
        virtual ~ FmtBase () noexcept;
        void reset () noexcept;

        N16 radix;
        N16 min_int_width;
        N16 min_frac_width;
        N16 min_field_width;
        ASCII int_left_pad;
        FieldAlign field_alignment;
        bool have_precision;
        bool have_alignment;

        void writeField ( const TextRgn & val, FieldAlign dflt );
        void writeInt ( const TextRgn & num, bool neg, bool pre = false );
    };

    
    /*=====================================================*
     *                     META FORMAT                     *
     *=====================================================*/
    
    // radix
    struct FmtRadix { N32 radix; };
    inline FmtRadix radix ( N32 val ) { FmtRadix r; r . radix = val; return r; }

    // field width
    struct FmtFieldWidth { N32 width; };
    inline FmtFieldWidth setw ( N32 w ) { FmtFieldWidth r; r . width = w; return r; } // deprecated
    inline FmtFieldWidth fieldw ( N32 w ) { FmtFieldWidth r; r . width = w; return r; }

    // integer width - for integer and floating point
    struct FmtIntWidth { N32 width; ASCII fill; };
    inline FmtIntWidth intw ( N32 w, ASCII f = '0' ) { FmtIntWidth r; r . width = w; r . fill = f; return r; }

    // fraction width - for floating point
    struct FmtFracWidth { N32 prec; };
    inline FmtFracWidth fracw ( N32 p ) { FmtFracWidth r; r . prec = p; return r; }

}
