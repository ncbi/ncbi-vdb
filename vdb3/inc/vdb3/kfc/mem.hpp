/*

  vdb3.kfc.mem

 */

#pragma once

#include <vdb3/kfc/defs.hpp>
#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/rgn.hpp>
#include <vdb3/kfc/rgn-lock.hpp>

/**
 * @file kfc/mem.hpp
 * @brief 
 */

namespace vdb3
{

    
    /*=====================================================*
     *                        MemItf                       *
     *=====================================================*/

    /**
     * MemItf
     *  @brief a region of allocated memory mapped into VAddrSpace
     */
    interface MemItf
    {
        /**
         * size
         *  @return the region's size in bytes
         */
        MSG ( size_t size () const );

        /**
         * isWritable
         *  @return true if mapMRgn is supported
         */
        MSG ( bool isWritable () const noexcept );

        /**
         * isResizable
         *  @return true if the region can be resized
         */
        MSG ( bool isResizable () const noexcept );

        /**
         * wipeBeforeDelete
         *  @return true if the region will be wiped before delete
         */
        MSG ( bool wipeBeforeDelete () const noexcept );
        
        /**
         * resize
         *  @param new_size is the desired object size upon success
         */
        MSG ( void resize ( size_t new_size ) );
        
        /**
         * mapMRgn
         *  @return an exclusive mutable mapped region locked into VAddrSpace
         */
        MSG ( MRgnLockRef mapMRgn () );

        /**
         * mapCRgn
         *  @return a shared constant mapped region locked into VAddrSpace
         */
        MSG ( CRgnLockRef mapCRgn () const );


        /**
         * ~MemItf
         *  @brief releases and destroys object
         */
        virtual ~ MemItf () noexcept {}
    };

    /**
     * @typedef MemRef
     * @brief a shared reference to an MemItf
     */
    typedef IRef < MemItf > MemRef;

    
    /*=====================================================*
     *                         Mem                         *
     *=====================================================*/

    class Mem
    {
    public:
        
        /**
         * size
         *  @return the region's size in bytes
         */
        inline size_t size () const
        { return mem -> size (); }

        /**
         * isWritable
         *  @return true if mapMRgn is supported
         */
        inline bool isWritable () const noexcept
        { return mem -> isWritable (); }

        /**
         * isResizable
         *  @return true if the region can be resized
         */
        inline bool isResizable () const noexcept
        { return mem -> isResizable (); }

        /**
         * wipeBeforeDelete
         *  @return true if the region will be wiped before delete
         */
        inline bool wipeBeforeDelete () const noexcept
        { return mem -> wipeBeforeDelete (); }
        
        /**
         * resize
         *  @param new_size is the desired object size upon success
         */
        inline void resize ( size_t new_size )
        { mem -> resize ( new_size ); }

        /**
         * mapMRgn
         *  @return an exclusive mutable mapped region locked into VAddrSpace
         */
        inline MRgnLockRef mapMRgn () const
        { return mem -> mapMRgn (); }

        /**
         * mapCRgn
         *  @return a shared constant mapped region locked into VAddrSpace
         */
        inline CRgnLockRef mapCRgn () const
        { return mem -> mapCRgn (); }

        
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_EIGHT_NE ( Mem );

        // leaves self in an invalid state
        inline void release () { mem . release (); }

    private:
        
        MemRef mem;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
