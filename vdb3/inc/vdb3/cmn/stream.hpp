/*

  vdb3.cmn.stream

 */

#pragma once

#include <vdb3/cmn/reader.hpp>
#include <vdb3/cmn/writer.hpp>

/**
 * @file cmn/stream.hpp
 * @brief an interface to unbounded data sources and sinks
 *  unbounded in the sense that the boundaries are not defined
 *  at this level.
 */

namespace vdb3
{

    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    class MStream;
    
    /*=====================================================*
     *                      StreamItf                      *
     *=====================================================*/

    interface StreamItf
    {

        /**
         * isReadable
         *  @return bool true if getReader() can succeed
         */
        MSG ( bool isReadable () const noexcept );
        
        /**
         * getReader
         *  @return a reader interface allowing serial access
         */
        MSG ( StreamReaderRef getReader () );

        /**
         * ~StreamItf
         *  @brief releases and destroys object
         */
        virtual ~ StreamItf () noexcept {}
    };

    /**
     * @typedef StreamRef
     * @brief a shared reference to a StreamItf
     */
    typedef IRef < StreamItf > StreamRef;

    interface MStreamItf
    {
        
        /**
         * toStream
         *  @return a const StreamRef
         */
        MSG ( StreamRef toStream () const noexcept );

        /**
         * isWritable
         *  @return bool true if getWriter() can succeed
         */
        MSG ( bool isWritable () const noexcept );
        
        /**
         * getWriter
         *  @return a writer interface allowing serial access
         */
        MSG ( StreamWriterRef getWriter () );

        /**
         * ~MStreamItf
         *  @brief releases and destroys object
         */
        virtual ~ MStreamItf () noexcept {}
    };

    /**
     * @typedef MStreamRef
     * @brief a shared reference to a MStreamItf
     */
    typedef IRef < MStreamItf > MStreamRef;

    
    /*=====================================================*
     *                        Stream                       *
     *=====================================================*/

    class Stream
    {
    public:

        /**
         * isReadable
         *  @return bool true if getReader() can succeed
         */
        inline bool isReadable () const noexcept
        { return strm -> isReadable (); }
        
        /**
         * getReader
         *  @return a reader interface allowing serial access
         */
        inline StreamReader getReader () const
        { return strm -> getReader (); }

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * cast-assign operator
         */
        inline Stream & operator = ( const MStream & mstrm );

        /**
         * cast-assign constructor
         */
        inline Stream ( const MStream & mstrm );

        CXX_RULE_OF_EIGHT_NE ( Stream );

    private:
        
        StreamRef strm;
    };

    class MStream
    {
    public:

        /**
         * toStream
         *  @return a const StreamRef
         */
        inline Stream toStream () const
        { return strm -> toStream (); }

        /**
         * isReadable
         *  @return bool true if toStream() can succeed
         */
        inline bool isReadable () const noexcept
        { return toStream () . isReadable (); }
        
        /**
         * getReader
         *  @return a reader interface allowing serial access
         */
        inline StreamReader getReader () const
        { return toStream () . getReader (); }

        /**
         * isWritable
         *  @return bool true if getWriter() can succeed
         */
        inline bool isWritable () const noexcept
        { return strm -> isWritable (); }

        /**
         * getWriter
         *  @return a writer interface allowing serial access
         */
        inline StreamWriter getWriter ()
        { return strm -> getWriter (); }

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_EIGHT_NE ( MStream );

    private:
        
        MStreamRef strm;
    };

    /*=====================================================*
     *                       INLINES                       *
     *=====================================================*/

    /**
     * cast-assign operator
     */
    inline Stream & Stream :: operator = ( const MStream & mstrm )
    {
        return operator = ( mstrm . toStream () );
    }

    /**
     * cast-assign constructor
     */
    inline Stream :: Stream ( const MStream & mstrm )
        : Stream ( mstrm . toStream () )
    {
    }

    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}

