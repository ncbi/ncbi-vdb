/*

  vdb3.cmn.reader

 */

#pragma once

#include <vdb3/cmn/defs.hpp>
#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/rgn.hpp>
#include <vdb3/kfc/time.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                     PosReaderItf                    *
     *=====================================================*/

    /**
     * PosReaderItf
     *  @brief an arbitrarily addressable data sink
     */
    interface PosReaderItf
    {

        /*=================================================*
         *                     MESSAGES                    *
         *=================================================*/

        /**
         * size
         *  @return the object's size in bytes
         */
        MSG ( bytes_t size () const );

        /**
         * read
         *  @brief reads as much as possible from Object into a region
         *  @param rgn has the mapped address space to be filled
         *  @param pos is starting offset into Object for reading
         *  @return CRgn with valid subregion of "rgn" - possibly empty
         */
        MSG ( CRgn read ( const MRgn & rgn, bytes_t pos ) );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * ~PosReaderItf
         *  @brief releases and destroys reader
         */
        virtual ~ PosReaderItf () noexcept {}
    };

    /**
     * @typedef PosReaderRef
     * @brief a shared reference to a PosReaderItf
     */
    typedef IRef < PosReaderItf > PosReaderRef;

    
    /*=====================================================*
     *                       PosReader                     *
     *=====================================================*/

    /**
     * PosReader
     *  @brief a reference to a PosReaderItf
     */
    class PosReader
    {
    public:
        
        /**
         * size
         *  @return the object's size in bytes
         */
        inline bytes_t size () const
        { return r -> size (); }
        
        /**
         * read
         *  @brief reads as much as possible from Object into a region
         *  @param rgn has the mapped address space to be filled
         *  @param pos is starting offset into Object for reading
         *  @return CRgn with valid subregion of "rgn" - possibly empty
         */
        inline CRgn read ( const MRgn & rgn, bytes_t pos ) const
        { return r -> read ( rgn, pos ); }


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        void release () noexcept
        { r . release (); }

        CXX_RULE_OF_EIGHT_NE ( PosReader );

    private:

        PosReaderRef r;
    };

    
    /*=====================================================*
     *                   StreamReaderItf                   *
     *=====================================================*/

    /**
     * StreamReaderItf
     *  @brief a streaming data source
     */
    interface StreamReaderItf
    {

        /*=================================================*
         *                     MESSAGES                    *
         *=================================================*/

        /**
         * read
         *  @brief reads as much as possible from Stream into a region
         *  @param rgn has the mapped address space to be filled
         *  @return CRgn with valid subregion of "rgn" - possibly empty
         */
        MSG ( CRgn read ( const MRgn & rgn ) );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * ~StreamReaderItf
         *  @brief releases and destroys reader
         */
        virtual ~ StreamReaderItf () noexcept {}
    };

    /**
     * @typedef StreamReaderRef
     * @brief a shared reference to a StreamReaderItf
     */
    typedef IRef < StreamReaderItf > StreamReaderRef;

    
    /*=====================================================*
     *                     StreamReader                    *
     *=====================================================*/

    /**
     * StreamReader
     *  @brief a reference to a StreamReaderItf
     */
    class StreamReader
    {
    public:
        
        /**
         * read
         *  @brief reads as much as possible from Stream into a region
         *  @param rgn has the mapped address space to be filled
         *  @return CRgn with valid subregion of "rgn" - possibly empty
         */
        inline CRgn read ( const MRgn & rgn ) const
        { return r -> read ( rgn ); }


        /*=================================================*
         *                    ADAPTERS                     *
         *=================================================*/

        explicit StreamReader ( const PosReader & rdr );
        StreamReader ( const PosReaderRef & rdr );
        StreamReader ( const PosReader & rdr, bytes_t marker );
        StreamReader ( const PosReaderRef & rdr, bytes_t marker );

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        void release () noexcept
        { r . release (); }

        CXX_RULE_OF_EIGHT_NE ( StreamReader );

    private:

        StreamReaderRef r;
    };
}
