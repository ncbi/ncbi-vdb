/*

  vdb3.kfc.txt-rgn

 */

#pragma once

#include <vdb3/kfc/defs.hpp>

namespace vdb3
{

    /*=====================================================*
     *                         Text                        *
     *=====================================================*/
    
    /**
     * @class TextRgn
     * @brief describes a region of addressable virtual memory
     */
    class TextRgn
    {
    public:

        /*=================================================*
         *                   PROPERTIES                    *
         *=================================================*/

        /**
         * constAddr
         *  @return the text region address as const pointer
         */
        inline const UTF8 * constAddr () const noexcept
        { return a; }

        /**
         * size
         *  @return number of bytes in the region
         */
        inline size_t size () const noexcept
        { return sz; }

        /**
         * count
         *  @return number of characters in the region
         */
        inline count_t count () const noexcept
        { return cnt; }
        
    protected:

        inline UTF8 * getAddr () noexcept { return a; }
        inline const UTF8 * getAddr () const noexcept { return a; }
        
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        inline TextRgn () noexcept
            : a ( nullptr ), sz ( 0 ), cnt ( 0 ) {}

        inline ~ TextRgn () noexcept { a = nullptr; sz = 0; cnt = 0; }

        inline TextRgn ( const UTF8 * _a, size_t _sz, count_t _cnt ) noexcept
            : a ( const_cast < UTF8 * > ( _a ) )
            , sz ( _sz )
            , cnt ( _cnt )
        {
        }

        UTF8 * a;
        size_t sz;
        count_t cnt;
    };
    
}
