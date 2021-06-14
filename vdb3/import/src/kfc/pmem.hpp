/*

  vdb3.kfc.pmem

 */

#pragma once

#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/rgn.hpp>
#include <vdb3/kfc/mem.hpp>
#include <vdb3/kfc/busy.hpp>

/**
 * @file kfc/pmem.hpp
 * @brief primordial memory object
 */

namespace vdb3
{

    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    struct PMemMgrImpl;

    
    /*=====================================================*
     *                       PMemImpl                      *
     *=====================================================*/

    /**
     * PMemImpl
     *  @brief implements the primordial MemItf
     */
    struct PMemImpl
        : Refcount
        , MemItf
        , RgnFactory
    {
        METH ( size_t size () const );
        METH ( bool isWritable () const noexcept )
        { return writable; }
        METH ( bool isResizable () const noexcept )
        { return resizeable; }
        METH ( bool wipeBeforeDelete () const noexcept )
        { return wipe; }
        METH ( void resize ( size_t new_size ) );
        METH ( MRgnLockRef mapMRgn () );
        METH ( CRgnLockRef mapCRgn () const );

        PMemImpl ( const IRef < PMemMgrImpl > & mmgr, const MRgn & rgn, bool clear, bool wipe ) noexcept;
        virtual ~ PMemImpl () noexcept;

        IRef < PMemMgrImpl > mmgr;
        MRgn rgn;
        BusyLock busy;
        bool writable;
        bool resizeable;
        bool clear;
        bool wipe;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
