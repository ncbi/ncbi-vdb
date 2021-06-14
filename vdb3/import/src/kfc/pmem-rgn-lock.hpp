/*

  vdb3.kfc.pmem-rgn-lock

 */

#pragma once

#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/busy.hpp>
#include <vdb3/kfc/rgn-lock.hpp>

/**
 * @file kfc/pmem-rgn-lock.hpp
 * @brief primordial memory object
 */

namespace vdb3
{

    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    struct PMemImpl;

    
    /*=====================================================*
     *                   PMemMRgnLockImpl                  *
     *=====================================================*/

    /**
     * PMemMRgnLockImpl
     */
    struct PMemMRgnLockImpl
        : Refcount
        , MRgnLockItf
    {
        METH ( MRgn getMRgn () noexcept );

        PMemMRgnLockImpl ( const IRef < PMemImpl > & mem );
        virtual ~ PMemMRgnLockImpl () noexcept;

        IRef < PMemImpl > mem;
        XLocker lock;
        MRgn rgn;
    };

    
    /*=====================================================*
     *                   PMemCRgnLockImpl                  *
     *=====================================================*/

    /**
     * PMemCRgnLockImpl
     */
    struct PMemCRgnLockImpl
        : Refcount
        , CRgnLockItf
    {
        METH ( CRgn getCRgn () const noexcept );

        PMemCRgnLockImpl ( const IRef < PMemImpl > & mem );
        virtual ~ PMemCRgnLockImpl () noexcept;

        IRef < PMemImpl > mem;
        SLocker lock;
        CRgn rgn;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
