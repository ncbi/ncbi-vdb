/*

  vdb3.cmn.txt-writer

 */

#pragma once

#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/time.hpp>
#include <vdb3/cmn/text.hpp>
#include <vdb3/cmn/writer.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                 TextStreamWriterItf                 *
     *=====================================================*/

    /**
     * TextStreamWriterItf
     *  @brief a streaming data sink
     */
    interface TextStreamWriterItf
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
        MSG ( CText write ( const CText & rgn ) );

        /**
         * flush
         *  @brief flushed out any buffers in chain
         */
        MSG ( void flush () );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * ~TextStreamWriterItf
         *  @brief releases and destroys writer
         */
        virtual ~ TextStreamWriterItf () noexcept {}
    };

    /**
     * @typedef TextStreamWriterRef
     * @brief a shared reference to a TextStreamWriterItf
     */
    typedef IRef < TextStreamWriterItf > TextStreamWriterRef;

    
    /*=====================================================*
     *                   TextStreamWriter                  *
     *=====================================================*/

    /**
     * TextStreamWriter
     *  @brief a reference to a TextStreamWriterItf
     */
    class TextStreamWriter
    {
    public:
        
        /**
         * write
         *  @brief writes as much as possible from a region
         *  @param rgn has the mapped data to be written
         *  @return sub-region containing all characters that were written
         */
        inline CText write ( const CText & rgn )
        { return r -> write ( rgn ); }

        /**
         * writeAll
         *  @brief writes as many times as possible until all characters are written
         *  @param rgn has the mapped data to be written
         *  @return sub-region containing all characters that were written
         */
        CText writeAll ( const CText & rgn );

        /**
         * writeExactly
         *  @brief does not return until all characters are written or an exception
         *  @param rgn has the mapped data to be written
         */
        void writeExactly ( const CText & rgn );

        /**
         * flush
         *  @brief flushed out any buffers in chain
         */
        inline void flush ()
        { r -> flush (); }


        /*=================================================*
         *                    ADAPTERS                     *
         *=================================================*/

        explicit TextStreamWriter ( const StreamWriter & w );
        TextStreamWriter ( const StreamWriterRef & w );
        explicit TextStreamWriter ( const PosWriter & w );
        TextStreamWriter ( const PosWriterRef & w );
        TextStreamWriter ( const PosWriter & w, bytes_t marker );
        TextStreamWriter ( const PosWriterRef & w, bytes_t marker );

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_EIGHT_EQOPS_NE ( TextStreamWriter, r );

    private:

        TextStreamWriterRef r;
    };
}
