/*

  vdb3.cmn.fmt


 */

#pragma once

#include <vdb3/cmn/defs.hpp>
#include <vdb3/kfc/fmt-base.hpp>
#include <vdb3/kfc/ascii.hpp>
#include <vdb3/kfc/decflt.hpp>
#include <vdb3/kfc/time.hpp>
#include <vdb3/cmn/text.hpp>
#include <vdb3/cmn/txt-writer.hpp>

namespace vdb3
{

    /*=====================================================*
     *                         Fmt                         *
     *=====================================================*/

    /**
     * Fmt
     *  @brief a general formatting class
     *   that handles numeric and symbolic data
     *   including Boolean and UNICODE
     */
    class Fmt : public FmtBase, RgnFactory, TextFactory
    {
    public:
        // Boolean
        inline Fmt & operator << ( bool val )
        { putBool ( val ); return self; }

        // Integer
        inline Fmt & operator << ( signed short int val )
        { putSSInt ( val ); return self; }
        inline Fmt & operator << ( signed int val )
        { putSInt ( val ); return self; }
        inline Fmt & operator << ( signed long int val )
        { putSLInt ( val ); return self; }
        inline Fmt & operator << ( signed long long int val )
        { putSLLInt ( val ); return self; }
#if HAVE_Z128
        inline Fmt & operator << ( signed __int128 val )
        { putSInt128 ( val ); return self; }
#else
        inline Fmt & operator << ( const Z128 & val )
        { putSInt128 ( val ); return self; }
#endif
        // Natural
        inline Fmt & operator << ( unsigned short int val )
        { putUSInt ( val ); return self; }
        inline Fmt & operator << ( unsigned int val )
        { putUInt ( val ); return self; }
        inline Fmt & operator << ( unsigned long int val )
        { putULInt ( val ); return self; }
        inline Fmt & operator << ( unsigned long long int val )
        { putULLInt ( val ); return self; }
#if HAVE_Z128
        inline Fmt & operator << ( unsigned __int128 val )
        { putUInt128 ( val ); return self; }
#else
        inline Fmt & operator << ( const N128 & val )
        { putUInt128 ( val ); return self; }
#endif
        // Real
        inline Fmt & operator << ( float val )
        { putF ( val ); return self; }
        inline Fmt & operator << ( double val )
        { putD ( val ); return self; }
        inline Fmt & operator << ( long double val )
        { putLD ( val ); return self; }
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128 && HAVE_QUADMATH
        inline Fmt & operator << ( __float128 val )
        { putF128 ( val ); return self; }
#endif
        template < class T >
        Fmt & operator << ( const DecFltPoint < T > & val );

        // virtual address
        inline Fmt & operator << ( const Ptr & val )
        { putPtr ( val ); return self; }

        // function pointer
        inline Fmt & operator << ( Fmt & ( * f ) ( Fmt & fmt ) )
        { return f ( self ); }

        // the only naked pointer-type allowed
        // is for manifest text constants
        Fmt & operator << ( const UTF8 * val );

        // explicit text
        inline Fmt & operator << ( const ASCIIText & val )
        { putTxtRgn ( val ); return self; }
        inline Fmt & operator << ( const ASCIIChar & val )
        { putTxtRgn ( val ); return self; }
        inline Fmt & operator << ( const CText & val )
        { putTxtRgn ( val ); return self; }
        inline Fmt & operator << ( const Char & val )
        { putTxtRgn ( val ); return self; }

        // default time values
        inline Fmt & operator << ( const Ticks & tm )
        { return self << tm . toR64 () << " ticks"; }
        inline Fmt & operator << ( const NanoSeconds & tm )
        { return self << tm . toR64 () << "nS"; }
        inline Fmt & operator << ( const CSharpTicks & tm )
        { return self << tm . toR64 () << " c#ticks"; }
        inline Fmt & operator << ( const MicroSeconds & tm )
        { return self << tm . toR64 () << "uS"; }
        inline Fmt & operator << ( const MilliSeconds & tm )
        { return self << tm . toR64 () << "mS"; }
        inline Fmt & operator << ( const Seconds & tm )
        { return self << tm . toR64 () << " sec"; }
        inline Fmt & operator << ( const Minutes & tm )
        { return self << tm . toR64 () << " min"; }
        inline Fmt & operator << ( const Hours & tm )
        { return self << tm . toR64 () << " hrs"; }
        inline Fmt & operator << ( const Days & tm )
        { return self << tm . toR64 () << " days"; }

        // default formatting of DayTime
        Fmt & operator << ( const DayTime & dt );

        // default formatting of CalTime
        Fmt & operator << ( const CalTime & ct );

        // sometimes useful
        Fmt & operator << ( const std :: string & val );
        Fmt & operator << ( const XMsg & val );

        // handle a UNICODE text region
        void putTxtRgn ( const TextRgn & t );

        void indent ( Z32 num_tabs );
        inline bool atLineStart () const
        { return at_line_start; }

        explicit Fmt ( const TextStreamWriter & writer, bool add_eoln = false, bool use_crlf = false ) noexcept;
        virtual ~ Fmt () noexcept;

        Fmt () = delete;
        Fmt ( Fmt && ) noexcept;
        Fmt ( const Fmt & ) = delete;
        Fmt & operator = ( Fmt && ) noexcept;
        Fmt & operator = ( const Fmt & ) = delete;

    protected:
        
        virtual void write ( const TextRgn & val ) override;
        virtual void fill ( ASCII ch, count_t repeat ) override;

        virtual CText writeAll ( const CText & rgn );
        virtual void writeExactly ( const CText & rgn );
        virtual void flush ();

    private:

        void indent ();

        TextStreamWriter writer;
        Z16 indentation;
        N8 tabwidth;
        bool at_line_start;
        bool last_char_is_nl;
        bool add_eoln;
        bool use_crlf;

        friend Fmt & endm ( Fmt & f );
    };

    // control indentation
    inline Fmt & ind ( Fmt & f ) { f . indent ( 1 ); return f; }
    inline Fmt & outd ( Fmt & f ) { f . indent ( -1 ); return f; }

    // include a newline
    Fmt & eoln ( Fmt & f );

    // this is how to indicate end of format message
    // will automatically terminate with eoln if "add_eoln" is true
    Fmt & endm ( Fmt & f );
    
    // somehow, placing these as global operators helps
    // with C++ ostream, so I won't argue
    inline Fmt & operator << ( Fmt & f, char c )
    { return f << ASCIIChar ( c ); }

    // radix
    inline Fmt & binary ( Fmt & f ) { f . setRadix ( 2 ); return f; }
    inline Fmt & octal ( Fmt & f ) { f . setRadix ( 8 ); return f; }
    inline Fmt & decimal ( Fmt & f ) { f . setRadix ( 10 ); return f; }
    inline Fmt & hex ( Fmt & f ) { f . setRadix ( 16 ); return f; }
    inline Fmt & operator << ( Fmt & f, const FmtRadix & r )
    { f . setRadix ( r . radix ); return f; }

    // field width
    inline Fmt & operator << ( Fmt & f, const FmtFieldWidth & r )
    { f . setFieldWidth ( r . width ); return f; }

    // integer width
    inline Fmt & operator << ( Fmt & f, const FmtIntWidth & r )
    { f . setMinIntegerDigits ( r . width, r . fill ); return f; }

    // fraction width
    inline Fmt & operator << ( Fmt & f, const FmtFracWidth & r )
    { f . setMinFractionDigits ( r . prec ); return f; }

    struct FmtIndent { Z32 amt; };
    inline FmtIndent indent ( Z32 amt ) { FmtIndent i; i . amt = amt; return i; }
    inline Fmt & operator << ( Fmt & f, const FmtIndent & i )
    { f . indent ( i . amt ); return f; }
}
