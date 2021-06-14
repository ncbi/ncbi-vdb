/*

  vdb3.cmn.buffer

 */

#pragma once

#include <vdb3/cmn/defs.hpp>
#include <vdb3/kfc/except.hpp>
#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/rgn.hpp>
#include <vdb3/kfc/mem.hpp>
#include <vdb3/kfc/busy.hpp>


/**
 * @file cmn/buffer.hpp
 * @brief a wrapper on a Mem supporting concepts
 *  of general capacity region with valid content region
 */

namespace vdb3
{

    /*=====================================================*
     *                      BufferItf                      *
     *=====================================================*/

    interface BufferItf
    {

        /**
         * getBuffer
         *  @return a mutable mapped region over entire buffer
         */
        MSG ( const MRgn & getBuffer () const noexcept );

        /**
         * getSize
         *  @return the size of valid data within buffer
         */
        MSG ( size_t getSize () const noexcept );

        /**
         * setCapacity
         * @brief set buffer size minimally to amount specified
         * @param bytes amount of capacity in bytes
         *
         *  NB: this should require resource managers
         */
        MSG ( void setCapacity ( size_t bytes ) );

        /**
         * setSize
         * @brief record actual size of valid content
         * @param bytes content size in bytes
         */
        MSG ( void setSize ( size_t bytes ) );

        /**
         * wipeBeforeDelete
         *  @return true if the region will be wiped before delete
         */
        MSG ( bool wipeBeforeDelete () const noexcept );

        /**
         * wipe
         * @brief overwrite buffer with zeros
         */
        MSG ( void wipe () );

        MSG ( Mem releaseMem () );


        /**
         * ~BufferItf
         *  @brief releases and destroys object
         */
        virtual ~ BufferItf () noexcept {}
    };

    /**
     * @typedef BufferRef
     * @brief a shared reference to a BufferItf
     */
    typedef IRef < BufferItf > BufferRef;
    

    /*=====================================================*
     *                        Buffer                       *
     *=====================================================*/
    
    /**
     * @class Buffer
     * @brief contains binary data in a managed package
     */
    class Buffer
    {
    public:

        /**
         * buffer - DEPRECATED
         *  @return a mutable mapped region over entire buffer
         */
        inline const MRgn & buffer () const noexcept
        { return r -> getBuffer (); }

        /**
         * getBuffer
         *  @return a mutable mapped region over entire buffer
         */
        inline const MRgn & getBuffer () const noexcept
        { return r -> getBuffer (); }

        /**
         * capacity - DEPRECATED
         *  @return the capacity of the buffer
         */
        inline size_t capacity () const noexcept
        { return getBuffer () . size (); }

        /**
         * getCapacity
         *  @return the capacity of the buffer
         */
        inline size_t getCapacity () const noexcept
        { return getBuffer () . size (); }

        /**
         * size - DEPRECATED
         *  @return the size of valid data within buffer
         */
        inline size_t size () const noexcept
        { return r -> getSize (); }

        /**
         * getSize
         *  @return the size of valid data within buffer
         */
        inline size_t getSize () const noexcept
        { return r -> getSize (); }

        /**
         * content - DEPRECATED
         *  @return a constant mapped region over valid data
         */
        inline MRgn content () const noexcept
        { return getBuffer () . subRgn ( 0, getSize () ); }

        /**
         * getContent
         *  @return a constant mapped region over valid data
         */
        inline MRgn getContent () const noexcept
        { return getBuffer () . subRgn ( 0, getSize () ); }

        /**
         * setCapacity
         * @brief set buffer size minimally to amount specified
         * @param bytes amount of capacity in bytes
         */
        inline Buffer & setCapacity ( size_t bytes )
        { r -> setCapacity ( bytes ); return self; }
        
        /**
         * increaseCapacity
         * @brief increase buffer size minimally by amount specified
         * @param amt amount of size increment
         */
        Buffer & increaseCapacity ( size_t amt = 4096U );
        
        /**
         * ensureAdditionalCapacity
         * @brief ensure that there is capacity for additional amount
         * @param amt amount of size increment
         */
        Buffer & ensureAdditionalCapacity ( size_t amt );

        /**
         * setSize
         * @brief record actual size of valid content
         * @param bytes content size in bytes
         */
        inline Buffer & setSize ( size_t bytes )
        { r -> setSize ( bytes ); return self; }

        /**
         * wipeBeforeDelete
         *  @return true if the region will be wiped before delete
         */
        inline bool wipeBeforeDelete () const noexcept
        { return r -> wipeBeforeDelete (); }

        /**
         * wipe
         * @brief overwrite buffer with zeros
         */
        inline Buffer & wipe ()
        { r -> wipe (); return self; }

        /**
         * reinitialize
         * @brief resets capacity to indicated amount and size to 0
         */
        Buffer & reinitialize ( size_t amt = 4096U );

        
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_EIGHT_EQOPS_NE ( Buffer, r );

        Buffer ( const RsrcKfc & rsrc, size_t initial_capacity = 4096 );

    private:

        Mem releaseMem ()
        { return r -> releaseMem (); }

        BufferRef r;

        friend class StringBuffer;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
