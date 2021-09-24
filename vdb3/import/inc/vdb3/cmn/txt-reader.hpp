/*

  vdb3.cmn.txt-reader

 */

#pragma once

#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/time.hpp>
#include <vdb3/cmn/text.hpp>
#include <vdb3/cmn/reader.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                 TextStreamReaderItf                 *
     *=====================================================*/

    /**
     * TextStreamReaderItf
     *  @brief a streaming text source
     */
    interface TextStreamReaderItf
    {

        /*=================================================*
         *                     MESSAGES                    *
         *=================================================*/

        /**
         * read
         *  @brief reads as much text as possible from Stream
         *  @param rgn an MText to be overwritten
         *  @return MText reference to "rgn" with result of read
         *
         *  The amount of bytes read from Stream is limited by
         *  the region's capacity. The bytes read are potentially
         *  transformed to UTF-8, then measured to count characters
         *  where they are recorded in "rgn."
         *
         *  After the read has completed, the "rgn" is returned
         *  with a count in characters and bytes of the valid
         *  portion of the region.
         */
        MSG ( MText & read ( MText & rgn ) );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * ~TextStreamReaderItf
         *  @brief releases and destroys reader
         */
        virtual ~ TextStreamReaderItf () noexcept {}
    };

    /**
     * @typedef TextStreamReaderRef
     * @brief a shared reference to a TextStreamReaderItf
     */
    typedef IRef < TextStreamReaderItf > TextStreamReaderRef;

    
    /*=====================================================*
     *                   TextStreamReader                  *
     *=====================================================*/

    /**
     * TextStreamReader
     *  @brief a reference to a TextStreamReaderItf
     */
    class TextStreamReader
    {
    public:
        
        /**
         * read
         *  @brief reads as much text as possible from Stream
         *  @param rgn an MText to be overwritten
         *  @return MText reference to "rgn" with result of read
         *
         *  The amount of bytes read from Stream is limited by
         *  the region's capacity. The bytes read are potentially
         *  transformed to UTF-8, then measured to count characters
         *  where they are recorded in "rgn."
         *
         *  After the read has completed, the "rgn" is returned
         *  with a count in characters and bytes of the valid
         *  portion of the region.
         */
        inline MText & read ( MText & rgn )
        { return r -> read ( rgn ); }


        /*=================================================*
         *                    ADAPTERS                     *
         *=================================================*/

        explicit TextStreamReader ( const StreamReader & rdr );
        TextStreamReader ( const StreamReaderRef & rdr );
        explicit TextStreamReader ( const PosReader & rdr );
        TextStreamReader ( const PosReaderRef & rdr );
        TextStreamReader ( const PosReader & rdr, bytes_t marker );
        TextStreamReader ( const PosReaderRef & rdr, bytes_t marker );

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_EIGHT_NE ( TextStreamReader );

    private:

        TextStreamReaderRef r;
    };
}
