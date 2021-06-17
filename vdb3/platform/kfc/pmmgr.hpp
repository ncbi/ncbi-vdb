/*

  vdb3.kfc.pmmgr

 */

#pragma once

#include <vdb3/kfc/mmgr.hpp>
#include <vdb3/kfc/rgn.hpp>

#include <atomic>

/**
 * @file kfc/pmmgr.hpp
 * @brief primordial memory manager
 */

namespace vdb3
{
    
    /*=====================================================*
     *                     PMemMgrImpl                     *
     *=====================================================*/

    /**
     * PMemMgrImpl
     *  @brief primordial interface to process memory allocator
     */
    struct PMemMgrImpl
        : Refcount
        , MemMgrItf
        , RgnFactory
    {

        METH ( MemRef alloc ( size_t bytes, bool clear, bool wipe ) );

        MRgn allocRgn ( size_t bytes, bool clear );
        void reallocRgn ( MRgn & rgn, size_t new_size, bool clear, bool wipe );
        void freeRgn ( MRgn & rgn, bool wipe ) noexcept;
        
        PMemMgrImpl ();
        METH ( ~ PMemMgrImpl () noexcept );
        

    private:
        
        static std :: atomic_flag latch;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
