/*

  vdb3.kfc.ptr

 */

#pragma once

#include <vdb3/kfc/defs.hpp>

namespace vdb3
{

    /*=====================================================*
     *                         Ptr                         *
     *=====================================================*/
    
    /**
     * @class Ptr
     * @brief a virtual memory address
     */
    class Ptr
    {
    public:

        /*=================================================*
         *                   PROPERTIES                    *
         *=================================================*/

        /**
         * addr
         *  @return the pointer address
         */
        inline void * addr () noexcept
        { return a; }
        
        /**
         * addr
         *  @return the pointer address
         */
        inline const void * addr () const noexcept
        { return a; }
        
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        inline Ptr () noexcept : a ( nullptr ) {}
        inline ~ Ptr () noexcept { a = nullptr; }

        template < class T > inline
        Ptr ( const T * _a ) noexcept
            : a ( ( void * ) const_cast < T * > ( _a ) )
        {
        }
        
    protected:

        void * a;
    };
}
