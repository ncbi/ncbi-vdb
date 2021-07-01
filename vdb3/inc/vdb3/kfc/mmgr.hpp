/*

  vdb3.kfc.mmgr

 */

#pragma once

#include <vdb3/kfc/defs.hpp>
#include <vdb3/kfc/except.hpp>
#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/mem.hpp>

/**
 * @file kfc/mmgr.hpp
 * @brief memory manager interface
 */

namespace vdb3
{
    
    /*=====================================================*
     *                      MemMgrItf                      *
     *=====================================================*/

    /**
     * MemMgrItf
     *  @brief a source of memory
     */
    interface MemMgrItf
    {

        /**
         * alloc
         *  @brief allocates a new block of VAddrSpace
         *  @param bytes gives the size of the new block in bytes
         *  @param clear if true, the memory is pre-zeroed
         *  @param wipe if true, the memory is post-zeroed
         */
        MSG ( MemRef alloc ( size_t bytes, bool clear, bool wipe ) );

        /**
         * ~MemMgrItf
         *  @brief releases and destroys object
         */
        virtual ~ MemMgrItf () noexcept {}
        
    };

    /**
     * @typedef MemMgrRef
     * @brief a shared reference to an MemMgrItf
     */
    typedef IRef < MemMgrItf > MemMgrRef;

    
    /*=====================================================*
     *                        MemMgr                       *
     *=====================================================*/

    class MemMgr
    {
    public:

        /**
         * alloc
         *  @brief allocates a new block of VAddrSpace
         *  @param bytes gives the size of the new block in bytes
         *  @param clear if true, the memory is pre-zeroed
         *  @param wipe if true, the memory is post-zeroed
         */
        inline Mem alloc ( size_t bytes, bool clear = false, bool wipe = false ) const
        { return mgr -> alloc ( bytes, clear, wipe ); }


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_EIGHT_EQOPS_NE ( MemMgr, mgr );

    private:
        
        MemMgrRef mgr;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

    DECLARE_EXCEPTION ( MemoryExhausted, RuntimeException );
}
