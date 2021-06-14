/*

  vdb3.cmn.obj

 */

#pragma once

#include <vdb3/kfc/except.hpp>
#include <vdb3/kfc/rgn-lock.hpp>
#include <vdb3/cmn/reader.hpp>
#include <vdb3/cmn/writer.hpp>

/**
 * @file cmn/obj.hpp
 * @brief an interface to an area of addressable storage having a defined size.
 */

namespace vdb3
{
    
    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    class MObj;
    
    /*=====================================================*
     *                        ObjItf                       *
     *=====================================================*/

    /**
     * ObjItf
     *  @brief interface to an implementation of a static data resource
     */
    interface ObjItf
    {
        /**
         * size
         *  @return the object's size in bytes
         */
        MSG ( bytes_t size () const );

        /**
         * isReadable
         *  @return bool true if getReader() and mapCRgn() can succeed
         */
        MSG ( bool isReadable () const noexcept );
        
        /**
         * getReader
         *  @return a positioned reader interface allowing random access
         */
        MSG ( PosReaderRef getReader () );

        /**
         * mapCRgn
         *  @overload ensure object @ offset is mapped into vaddr space
         *  @param offset a starting offset into object of mapped region
         *  @return a constant mapped region locked into VAddrSpace
         */
        MSG ( CRgnLockRef mapCRgn ( bytes_t offset ) );

        /**
         * mapCRgn
         *  @overload ensure a region of object @ offset is mapped into vaddr space
         *  @param offset a starting offset into object of mapped region
         *  @param bytes the number of constant bytes within the mapped region
         *  @return a constant mapped region locked into VAddrSpace
         */
        MSG ( CRgnLockRef mapCRgn ( bytes_t offset, size_t bytes ) );

        /**
         * ~ObjItf
         *  @brief releases and destroys object
         */
        virtual ~ ObjItf () noexcept {}
    };

    /**
     * @typedef ObjRef
     * @brief a shared reference to an ObjItf
     */
    typedef IRef < ObjItf > ObjRef;

    /**
     * MObjItf
     *  @brief interface to a mutable implementation of a static data resource
     */
    interface MObjItf
    {

        /**
         * toObj
         *  @return ObjRef
         */
        MSG ( ObjRef toObj () const noexcept );

        /**
         * isResizable
         *  @return true if the object supports the "resize" message
         */
        MSG ( bool isResizable () const noexcept );
        
        /**
         * resize
         *  @param new_size is the desired object size upon success
         */
        MSG ( void resize ( bytes_t new_size ) );

        /**
         * isWritable
         *  @return bool true if getWriter() and mapMRgn() can succeed
         */
        MSG ( bool isWritable () const noexcept );

        /**
         * getWriter
         *  @return a positioned writer interface allowing random access
         */
        MSG ( PosWriterRef getWriter () );
        
        /**
         * mapMRgn
         *  @overload ensure object @ offset is mapped into vaddr space
         *  @param offset a starting offset into object of mapped region
         *  @return a mutable mapped region locked into VAddrSpace
         */
        MSG ( MRgnLockRef mapMRgn ( bytes_t offset ) );

        /**
         * mapMRgn
         *  @overload ensure a region of object @ offset is mapped into vaddr space
         *  @param offset a starting offset into object of mapped region
         *  @param bytes the number of mutable bytes within the mapped region
         *  @return a mutable mapped region locked into VAddrSpace
         */
        MSG ( MRgnLockRef mapMRgn ( bytes_t offset, size_t bytes ) );

        /**
         * ~MObjItf
         *  @brief releases and destroys object
         */
        virtual ~ MObjItf () noexcept {}
    };

    /**
     * @typedef MObjRef
     * @brief a shared reference to an MObjItf
     */
    typedef IRef < MObjItf > MObjRef;

    
    /*=====================================================*
     *                         Obj                         *
     *=====================================================*/

    class Obj
    {
    public:
        
        /**
         * size
         *  @return the object's size in bytes
         */
        inline bytes_t size () const
        { return obj -> size (); }

        /**
         * isReadable
         *  @return bool true if getReader() and mapCRgn() can succeed
         */
        inline bool isReadable () const noexcept
        { return obj -> isReadable (); }

        /**
         * getReader
         *  @return a positioned reader interface allowing random access
         */
        inline PosReader getReader () noexcept
        { return obj -> getReader (); }

        /**
         * mapCRgn
         *  @overload ensure object @ offset is mapped into vaddr space
         *  @param offset a starting offset into object of mapped region
         *  @return a constant mapped region locked into VAddrSpace
         */
        inline CRgnLockRef mapCRgn ( bytes_t offset = 0 ) const
        { return obj -> mapCRgn ( offset ); }

        /**
         * mapCRgn
         *  @overload ensure a region of object @ offset is mapped into vaddr space
         *  @param offset a starting offset into object of mapped region
         *  @param bytes the number of constant bytes within the mapped region
         *  @return a constant mapped region locked into VAddrSpace
         */
        inline CRgnLockRef mapCRgn ( bytes_t offset, size_t bytes ) const
        { return obj -> mapCRgn ( offset, bytes ); }

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * cast-assign operator
         */
        inline Obj & operator = ( const MObj & mobj );

        /**
         * cast-assign constructor
         */
        inline Obj ( const MObj & mobj );

        CXX_RULE_OF_EIGHT_NE ( Obj );

    private:
        
        ObjRef obj;
    };

    class MObj
    {
    public:

        /**
         * toObj
         * @return Obj
         */
        inline Obj toObj () const noexcept
        { return obj -> toObj (); }
        
        /**
         * size
         *  @return the object's size in bytes
         */
        inline bytes_t size () const
        { return toObj () . size (); }

        /**
         * isResizable
         *  @return true if the object supports the "resize" message
         */
        inline bool isResizable () const noexcept
        { return obj -> isResizable (); }
        
        /**
         * resize
         *  @param new_size is the desired object size upon success
         */
        inline void resize ( bytes_t new_size )
        { obj -> resize ( new_size ); }

        /**
         * isReadable
         *  @return bool true if getReader() and mapCRgn() can succeed
         */
        inline bool isReadable () const noexcept
        { return toObj () . isReadable (); }

        /**
         * getReader
         *  @return a positioned reader interface allowing random access
         */
        inline PosReader getReader () noexcept
        { return toObj () . getReader (); }

        /**
         * isWritable
         *  @return bool true if getWriter() and mapMRgn() can succeed
         */
        inline bool isWritable () const noexcept
        { return obj -> isWritable (); }

        /**
         * getWriter
         *  @return a positioned writer interface allowing random access
         */
        inline PosWriter getWriter ()
        { return obj -> getWriter (); }

        /**
         * mapMRgn
         *  @overload ensure object @ offset is mapped into vaddr space
         *  @param offset a starting offset into object of mapped region
         *  @return a mutable mapped region locked into VAddrSpace
         */
        inline MRgnLockRef mapMRgn ( bytes_t offset = 0 )const 
        { return obj -> mapMRgn ( offset ); }

        /**
         * mapMRgn
         *  @overload ensure a region of object @ offset is mapped into vaddr space
         *  @param offset a starting offset into object of mapped region
         *  @param bytes the number of mutable bytes within the mapped region
         *  @return a mutable mapped region locked into VAddrSpace
         */
        inline MRgnLockRef mapMRgn ( bytes_t offset, size_t bytes ) const
        { return obj -> mapMRgn ( offset, bytes ); }

        /**
         * mapCRgn
         *  @overload ensure object @ offset is mapped into vaddr space
         *  @param offset a starting offset into object of mapped region
         *  @return a constant mapped region locked into VAddrSpace
         */
        inline CRgnLockRef mapCRgn ( bytes_t offset = 0 ) const
        { return toObj () . mapCRgn ( offset ); }

        /**
         * mapCRgn
         *  @overload ensure a region of object @ offset is mapped into vaddr space
         *  @param offset a starting offset into object of mapped region
         *  @param bytes the number of constant bytes within the mapped region
         *  @return a constant mapped region locked into VAddrSpace
         */
        inline CRgnLockRef mapCRgn ( bytes_t offset, size_t bytes ) const
        { return toObj () . mapCRgn ( offset, bytes ); }

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_EIGHT_NE ( MObj );

    private:
        
        MObjRef obj;
    };

    /*=====================================================*
     *                       INLINES                       *
     *=====================================================*/

    /**
     * cast-assign operator
     */
    inline Obj & Obj :: operator = ( const MObj & mobj )
    {
        return operator = ( mobj . toObj () );
    }

    /**
     * cast-assign constructor
     */
    inline Obj :: Obj ( const MObj & mobj )
        : Obj ( mobj . toObj () )
    {
    }

    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
