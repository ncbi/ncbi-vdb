/*

  vdb3.cmn.writer

 */

#pragma once

#include <vdb3/cmn/defs.hpp>
#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/rgn.hpp>
#include <vdb3/kfc/time.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                     PosWriterItf                    *
     *=====================================================*/

    /**
     * PosWriterItf
     *  @brief an arbitrarily addressable data sink
     */
    interface PosWriterItf
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
         * write
         *  @brief writes as much as possible from a region
         *  @param rgn has the mapped data to be written
         *  @param pos is starting offset into Object for writing
         *  @return sub-region containing all bytes that were written
         */
        MSG ( CRgn write ( const CRgn & rgn, bytes_t pos ) );

        /**
         * flush
         *  @brief flushes out any buffers in chain
         */
        MSG ( void flush () );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * ~PosWriterItf
         *  @brief releases and destroys writer
         */
        virtual ~ PosWriterItf () noexcept {}
    };

    /**
     * @typedef PosWriterRef
     * @brief a shared reference to a PosWriterItf
     */
    typedef IRef < PosWriterItf > PosWriterRef;

    
    /*=====================================================*
     *                       PosWriter                     *
     *=====================================================*/

    /**
     * PosWriter
     *  @brief a reference to a PosWriterItf
     */
    class PosWriter
    {
    public:
        
        /**
         * size
         *  @return the object's size in bytes
         */
        inline bytes_t size () const
        { return w -> size (); }
        
        /**
         * write
         *  @brief writes as much as possible from a region
         *  @param rgn has the mapped data to be written
         *  @param pos is starting offset into Object for writing
         *  @return sub-region containing all bytes that were written
         */
        inline CRgn write ( const CRgn & rgn, bytes_t pos )
        { return w -> write ( rgn, pos ); }

        /**
         * writeAll
         *  @brief writes as many times as possible until all bytes are written
         *  @param rgn has the mapped data to be written
         *  @param pos is starting offset into Object for writing
         *  @return sub-region containing all bytes that were written
         */
        CRgn writeAll ( const CRgn & rgn, bytes_t pos );

        /**
         * writeExactly
         *  @brief does not return until all bytes are written or an exception
         *  @param rgn has the mapped data to be written
         *  @param pos is starting offset into Object for writing
         */
        void writeExactly ( const CRgn & rgn, bytes_t pos );

        /**
         * flush
         *  @brief flushed out any buffers in chain
         */
        inline void flush ()
        { w -> flush (); }


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        void release () noexcept
        { w . release (); }

        CXX_RULE_OF_EIGHT_NE ( PosWriter );

    private:

        PosWriterRef w;
    };

    
    /*=====================================================*
     *                   StreamWriterItf                   *
     *=====================================================*/

    /**
     * StreamWriterItf
     *  @brief a streaming data sink
     */
    interface StreamWriterItf
    {

        /*=================================================*
         *                     MESSAGES                    *
         *=================================================*/

        /**
         * write
         *  @brief writes as much as possible from a region
         *  @param rgn has the mapped data to be written
         *  @return sub-region containing all bytes that were written
         */
        MSG ( CRgn write ( const CRgn & rgn ) );

        /**
         * flush
         *  @brief flushed out any buffers in chain
         */
        MSG ( void flush () );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * ~StreamWriterItf
         *  @brief releases and destroys writer
         */
        virtual ~ StreamWriterItf () noexcept {}
    };

    /**
     * @typedef StreamWriterRef
     * @brief a shared reference to a StreamWriterItf
     */
    typedef IRef < StreamWriterItf > StreamWriterRef;

    
    /*=====================================================*
     *                     StreamWriter                    *
     *=====================================================*/

    /**
     * StreamWriter
     *  @brief a reference to a StreamWriterItf
     */
    class StreamWriter
    {
    public:
        
        /**
         * write
         *  @brief writes as much as possible from a region
         *  @param rgn has the mapped data to be written
         *  @return sub-region containing all bytes that were written
         */
        inline CRgn write ( const CRgn & rgn )
        { return w -> write ( rgn ); }

        /**
         * writeAll
         *  @brief writes as many times as possible until all bytes are written
         *  @param rgn has the mapped data to be written
         *  @return sub-region containing all bytes that were written
         */
        CRgn writeAll ( const CRgn & rgn );

        /**
         * writeExactly
         *  @brief does not return until all bytes are written or an exception
         *  @param rgn has the mapped data to be written
         */
        void writeExactly ( const CRgn & rgn );

        /**
         * flush
         *  @brief flushed out any buffers in chain
         */
        inline void flush ()
        { w -> flush (); }


        /*=================================================*
         *                    ADAPTERS                     *
         *=================================================*/

        explicit StreamWriter ( const PosWriter & pw );
        StreamWriter ( const PosWriterRef & pw );
        StreamWriter ( const PosWriter & pw, bytes_t marker );
        StreamWriter ( const PosWriterRef & pw, bytes_t marker );

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        void release () noexcept
        { w . release (); }

        CXX_RULE_OF_EIGHT_EQOPS_NE ( StreamWriter, w );

    private:

        StreamWriterRef w;
    };
}
