/*

  vdb3.kfc.rgn-lock

 */

#pragma once

#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/rgn.hpp>

/**
 * @file kfc/rgn-lock.hpp
 * @brief a mechanism for locking a region into mapped VAddrSpace
 */

namespace vdb3
{

    /*=====================================================*
     *                     MRgnLockItf                     *
     *=====================================================*/

    /**
     * MRgnLockItf
     *  @brief an interface to a lock on a mutable mapped region
     */
    interface MRgnLockItf
    {

        /*=================================================*
         *                     MESSAGES                    *
         *=================================================*/

        /**
         * getMRgn
         *  @return mapped region of VAddrSpace
         */
        MSG ( MRgn getMRgn () noexcept );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * ~MRgnLockItf
         *  @brief releases and destroys mapping-lock on region
         */
        virtual ~ MRgnLockItf () noexcept {}
    };

    /**
     * @typedef MRgnLockRef
     * @brief a unique reference to an MRgnLockItf
     */
    typedef IRef < MRgnLockItf > MRgnLockRef;


    /*=====================================================*
     *                     CRgnLockItf                     *
     *=====================================================*/

    /**
     * CRgnLockItf
     *  @brief an interface to a lock on a constant mapped region
     */
    interface CRgnLockItf
    {

        /*=================================================*
         *                     MESSAGES                    *
         *=================================================*/

        /**
         * rgn
         *  @return mapped region of VAddrSpace
         */
        MSG ( CRgn getCRgn () const noexcept );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * ~CRgnLockItf
         *  @brief releases and destroys mapping-lock on region
         */
        virtual ~ CRgnLockItf () noexcept {}
    };

    /**
     * @typedef CRgnLockRef
     * @brief a unique reference to a CRgnLockItf
     */
    typedef IRef < CRgnLockItf > CRgnLockRef;

    
    /*=====================================================*
     *                      MRgnLocker                     *
     *=====================================================*/

    /**
     * MRgnLocker
     *  @brief a reference to an MRgnLockItf
     *   holds a mutable region in VAddrSpace 
     */
    class MRgnLocker
    {
    public:

        /**
         * rgn
         *  @return mapped region of VAddrSpace
         */
        inline const MRgn & rgn () noexcept
        { return mrgn; }


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        MRgnLocker & operator = ( MRgnLocker && ) = delete;
        MRgnLocker & operator = ( const MRgnLocker & ) = delete;
        MRgnLocker ( MRgnLocker && ) = delete;
        MRgnLocker ( const MRgnLocker & ) = delete;
        MRgnLocker () = delete;

        /**
         * MRgnLocker
         *  @brief constructor from lock
         *  @param lock an MRgnLockItf from mapping source
         */
        explicit MRgnLocker ( const MRgnLockRef & lock ) noexcept;

        /**
         * ~MRgnLocker
         *  @brief destructor that releases lock on region
         */
        ~ MRgnLocker () noexcept;

    private:

        MRgnLockRef lock;
        MRgn mrgn;
    };

    /*=====================================================*
     *                      CRgnLocker                     *
     *=====================================================*/

    /**
     * CRgnLocker
     *  @brief a reference to an CRgnLockItf
     *   holds a constant region in VAddrSpace
     */
    class CRgnLocker
    {
    public:

        /**
         * rgn
         *  @return mapped region of VAddrSpace
         */
        inline const CRgn & rgn () const noexcept
        { return crgn; }


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CRgnLocker & operator = ( CRgnLocker && ) = delete;
        CRgnLocker & operator = ( const CRgnLocker & ) = delete;
        CRgnLocker ( CRgnLocker && ) = delete;
        CRgnLocker ( const CRgnLocker & ) = delete;
        CRgnLocker () = delete;

        /**
         * CRgnLocker
         *  @brief constructor from lock
         *  @param lock an CRgnLockItf from mapping source
         */
        explicit CRgnLocker ( const CRgnLockRef & lock ) noexcept;

        /**
         * ~CRgnLocker
         *  @brief destructor that releases lock on region
         */
        ~ CRgnLocker () noexcept;

    private:

        CRgnLockRef lock;
        CRgn crgn;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
