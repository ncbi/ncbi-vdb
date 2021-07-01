/*

  vdb3.kfc.xp


 */

#pragma once

#include <vdb3/kfc/defs.hpp>
#include <vdb3/kfc/fmt-base.hpp>
#include <vdb3/kfc/ascii.hpp>

#include <string>
#include <ostream>

/**
 * @file kfc/xp.hpp
 * @brief Exception parameter classes
 */

namespace vdb3
{

    /*=====================================================*
     *                     ReturnCodes                     *
     *=====================================================*/

    enum ReturnCodes
    {
        rc_okay,
        rc_param_err,
        rc_init_err,
        rc_logic_err,
        rc_runtime_err,
        rc_input_err,
        rc_protocol_err,
        rc_test_err
    };
    
    /*=====================================================*
     *                        XMsg                         *
     *=====================================================*/
    
    /**
     * @struct XMsg
     * @brief an object to hold a well-defined block of memory for msg responses
     *
     * Used as a return value to avoid pointers
     */
    struct XMsg
    {
        size_t msg_size;            //!< size of "zmsg" excluding NUL termination
        UTF8 zmsg [ 512 ];          //!< NUL-terminated string of UTF-8 characters
    };

    //!< support for XMsg and std::ostream
    inline std :: ostream & operator << ( std :: ostream & o, const XMsg & m )
    { return o << m . zmsg; }


    /*=====================================================*
     *                        XBuf                         *
     *=====================================================*/
    
    /**
     * @struct XBuf
     * @brief an object to buffer text within an Exception
     */
    struct XBuf
    {
        bool isEmpty () const noexcept;
        XBuf & append ( ASCII ch ) noexcept;
        XBuf & append ( const UTF8 * str ) noexcept;
        XBuf & append ( const UTF8 * str, size_t bytes ) noexcept;
        void operator = ( XBuf && b ) noexcept;
        void operator = ( const XBuf & b ) = delete;
        XBuf ( XBuf && b ) noexcept;
        XBuf ( const XBuf & b ) = delete;
        XBuf () noexcept;
        ~ XBuf () noexcept;
        
        size_t marker;              //!< number of valid bytes in "buffer"
        size_t bsize;               //!< size of "buffer"
        UTF8 * buffer;              //!< buffer of UTF-8 characters
    };


    /*=====================================================*
     *                         XP                          *
     *=====================================================*/
    
    /**
     * @class XP
     * @brief a parameter block for constructing an Exception
     *
     * The class name is purposely kept short for convenience
     * of implementation notation.
     */
    class XP : public FmtBase
    {
    public:

        // Boolean
        inline XP & operator << ( bool val )
        { putBool ( val ); return self; }

        // integer numbers
        inline XP & operator << ( signed short int val )
        { putSSInt ( val ); return self; }
        inline XP & operator << ( signed int val )
        { putSInt ( val ); return self; }
        inline XP & operator << ( signed long int val )
        { putSLInt ( val ); return self; }
        inline XP & operator << ( signed long long int val )
        { putSLLInt ( val ); return self; }
#if HAVE_Z128
        inline XP & operator << ( signed __int128 val )
        { putSInt128 ( val ); return self; }
#else
        inline XP & operator << ( const Z128 & val )
        { putSInt128 ( val ); return self; }
#endif
        // natural numbers
        inline XP & operator << ( unsigned short int val )
        { putUSInt ( val ); return self; }
        inline XP & operator << ( unsigned int val )
        { putUInt ( val ); return self; }
        inline XP & operator << ( unsigned long int val )
        { putULInt ( val ); return self; }
        inline XP & operator << ( unsigned long long int val )
        { putULLInt ( val ); return self; }
#if HAVE_Z128
        inline XP & operator << ( unsigned __int128 val )
        { putUInt128 ( val ); return self; }
#else
        inline XP & operator << ( const N128 & val )
        { putUInt128 ( val ); return self; }
#endif
        // real numbers
        inline XP & operator << ( float val )
        { putF ( val ); return self; }
        inline XP & operator << ( double val )
        { putD ( val ); return self; }
        inline XP & operator << ( long double val )
        { putLD ( val ); return self; }
#if HAVE_R128 && LONG_DOUBLE_IS_NOT_R128 && HAVE_QUADMATH
        inline XP & operator << ( __float128 val )
        { putF128 ( val ); return self; }
#endif
        // virtual address
        inline XP & operator << ( const Ptr & val )
        { putPtr ( val ); return self; }
        // function pointer
        inline XP & operator << ( XP & ( * f ) ( XP & fmt ) )
        { return f ( self ); }

        // explicit ASCII-7 text
        inline XP & operator << ( const ASCIIText & val ) noexcept
        { putTxtRgn ( val ); return self; }
        inline XP & operator << ( const ASCIIChar & val ) noexcept
        { putTxtRgn ( val ); return self; }

        // NUL-terminated UTF-8 text, typically manifest ASCII constant
        XP & operator << ( const UTF8 * s ) noexcept;

        // text output of an Exception
        XP & operator << ( const XMsg & s ) noexcept;

        // explicit support for STL strings
        XP & operator << ( const std :: string & s ) noexcept;

        // handle an arbitrary text region
        void putTxtRgn ( const TextRgn & t ) noexcept;

        // reset the exit status to pass on to Exception
        void setStatus ( ReturnCodes status ) noexcept;

        // set the target text buffer
        void useProblem () noexcept;
        void useContext () noexcept;
        void useCause () noexcept;
        void useSuggestion () noexcept;

        // standard construction
        XP ( const UTF8 * file_name,
             const ASCII * func_name,
             unsigned int lineno,
             ReturnCodes status = rc_runtime_err ) noexcept;

        // these guys are not to be used
        XP & operator = ( XP && xp ) = delete;
        XP & operator = ( const XP & xp ) = delete;
        XP ( XP && xp ) = delete;
        XP ( const XP & xp ) = delete;
        XP () = delete;

        // cleanup
        ~ XP () noexcept;

    protected:

        // write formatted text to target text buffer
        virtual void write ( const TextRgn & val ) override;

        // ignored
        virtual void fill ( ASCII ch, count_t repeat ) override;

    private:

        // preserved function pointers up the call stack
        void * callstack [ 128 ];

        // text buffers
        mutable XBuf problem;
        mutable XBuf context;
        mutable XBuf cause;
        mutable XBuf suggestion;

        // text buffer selection
        XBuf * which;

        // constants around point of origin
        const UTF8 * file_name;
        const ASCII * func_name;
        unsigned int lineno;
        
        int stack_frames;
        ReturnCodes rc;

        friend class Exception;
    };


    /*=====================================================*
     *                  C++ INLINES, ETC.                  *
     *=====================================================*/

    //!< select which message is being populated
    inline XP & xprob ( XP & xp ) { xp . useProblem (); return xp; }
    inline XP & xctx ( XP & xp ) { xp . useContext (); return xp; }
    inline XP & xcause ( XP & xp ) { xp . useCause (); return xp; }
    inline XP & xsuggest ( XP & xp ) { xp . useSuggestion (); return xp; }

    //!< handle an ASCII character
    inline XP & operator << ( XP & xp, char c )
    { xp << ASCIIChar ( c ); return xp; }

    //!< modify numeric radix
    inline XP & binary ( XP & xp ) { xp . setRadix ( 2 ); return xp; }
    inline XP & octal ( XP & xp ) { xp . setRadix ( 8 ); return xp; }
    inline XP & decimal ( XP & xp ) { xp . setRadix ( 10 ); return xp; }
    inline XP & hex ( XP & xp ) { xp . setRadix ( 16 ); return xp; }
    inline XP & operator << ( XP & xp, const FmtRadix & r )
    { xp . setRadix ( r . radix ); return xp; }

    //!< set exit status
    struct XPStatus { ReturnCodes rc; };
    inline XPStatus xstatus ( ReturnCodes rc ) { XPStatus s; s . rc = rc; return s; }
    inline XP & operator << ( XP & xp, const XPStatus & s )
    { xp . setStatus ( s . rc ); return xp; }

    /**
     * @def XLOC
     * @brief list of macro args that supply current __FILE__, __func__, __LINE__
     */
#define XLOC __FILE__, __func__, __LINE__
}
