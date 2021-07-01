/*

  vdb3.kfc.ascii

 */

#pragma once

#include <vdb3/kfc/txt-rgn.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                      ASCIIText                      *
     *=====================================================*/
    
    /**
     * @class ASCIIText
     * @brief describes a region of addressable virtual memory
     */
    class ASCIIText : public TextRgn
    {
    public:

        /**
         * ASCIIText
         *  @brief constructor for ASCII string with known size
         *  @param str is a pointer to assumed ASCII-7 text
         *  @param sz is the number of bytes in the ASCII text
         */
        ASCIIText ( const ASCII7 * str, size_t sz ) noexcept;

        /**
         * operator=
         *  @brief assignment from NUL-terminated ASCII string
         *  @param zstr is a pointer to assumed, NUL-terminated ASCII-7
         */
        ASCIIText & operator = ( const ASCII7 * zstr ) noexcept;

        /**
         * ASCIIText
         *  @brief constructor for NUL-terminated ASCII string
         *  @param zstr is a pointer to assumed, NUL-terminated ASCII-7
         */
        ASCIIText ( const ASCII7 * zstr ) noexcept;

        inline ASCIIText () noexcept {}
        inline ~ ASCIIText () noexcept {}
    };

    
    /*=====================================================*
     *                      ASCIIChar                      *
     *=====================================================*/
    
    /**
     * @class ASCIIChar
     * @brief a simple ASCII-7 character
     */
    class ASCIIChar : public TextRgn
    {
    public:

        /**
         * operator=
         *  @brief assignment from single ASCII character
         *  @param ch is a byte: accepted if ASCII and substituted by '?' otherwise
         */
        ASCIIChar & operator = ( ASCII7 ch ) noexcept;

        /**
         * ASCIIChar
         *  @brief constructor for single ASCII character
         *  @param ch is a byte: accepted if ASCII and substituted by '?' otherwise
         */
        ASCIIChar ( ASCII7 ch ) noexcept;

        ASCIIChar () noexcept : ASCIIChar ( 0 ) {}
        inline ~ ASCIIChar () noexcept {}

    private:

        ASCII ch [ 1 ];
    };
}
