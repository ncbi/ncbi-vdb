/*

  vdb3.cmn.str-buffer

 */

#include <vdb3/cmn/str-buffer.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                       UTILITY                       *
     *=====================================================*/

    // defined in text-utf8.cpp
    size_t utf32_to_utf8_strict ( UTF8 * buff, size_t bsize, UTF32 ch );


    /*=====================================================*
     *                    StringBuffer                     *
     *=====================================================*/

    /**
     * append
     * @overload append provided String onto self
     * @param sb StringBuffer to be appended to self
     * @return C++ reference to self
     */
    StringBuffer & StringBuffer :: append ( const StringBuffer & sb )
    {
        return append ( sb . b );
    }

    /**
     * append
     * @overload append provided String onto self
     * @param s String to be appended to self
     * @return C++ reference to self
     */
    StringBuffer & StringBuffer :: append ( const String & s )
    {
        return append ( s . data () );
    }

    /**
     * append
     * @overload append provided region onto self
     * @param t a text region
     * @return C++ reference to self
     */
    StringBuffer & StringBuffer :: append ( const CText & t )
    {
        buff . ensureAdditionalCapacity ( t . size () );

        // get the sub-region to copy
        MRgn append_rgn = buff . buffer () . subRgn ( b . size () );

        // copy in new data
        append_rgn . copy ( t . toCRgn () );

        // set new buffer size
        buff . setSize ( self . size () + t . size () );

        // update our MText
        b = makeMText ( buff . content (), b . count () + t . count () );

        return self;
    }

    /**
     * append
     * @overload append provided character onto self
     * @param ch a UTF-32 UNICODE character
     * @return C++ reference to self
     */
    StringBuffer & StringBuffer :: append ( UTF32 ch )
    {
        UTF8 ch_str [ 8 ];
        size_t ch_sz = utf32_to_utf8_strict ( ch_str, sizeof ch_str, ch );
        CText t = makeCText ( ch_str, ch_sz, 1 );
        return append ( t );
    }

    /**
     * toupper
     * @brief upper-case contents
     * @return C++ reference to self
     */
    StringBuffer & StringBuffer :: toupper ()
    {
        b . toupper ();
        return self;
    }

    /**
     * tolower
     * @brief upper-case contents
     * @return C++ reference to self
     */
    StringBuffer & StringBuffer :: tolower ()
    {
        b . tolower ();
        return self;
    }

    /**
     * clear
     * @brief reset internal state to empty
     * @param wipe overwrite existing contents if true
     */
    void StringBuffer :: clear ()
    {
        buff . reinitialize ( 0 );
        b = makeMText ( buff . content (), 0 );
    }

    /**
     * toString
     * @brief create an immutable String from internal data
     * @return a new String
     */
    String StringBuffer :: toString ( const RsrcMem & rsrc ) const
    {
        return String ( rsrc, b, buff . wipeBeforeDelete () );
    }

    /**
     * stealString
     * @brief create an immutable String from internal data and leave contents useless
     * @return a new String
     */
    String StringBuffer :: stealString ()
    {
        // pass this along
        bool wipe = buff . wipeBeforeDelete ();

        // grab the memory
        Mem mem = buff . releaseMem ();

        // AT THIS POINT, BUFF IS PRETTY MUCH USELESS.

        // lock memory region for read-only
        CRgnLockRef rlock = mem . mapCRgn ();

        // make the string
        String s ( rlock, makeCText ( rlock -> getCRgn (), b . count () ), wipe );

        // whack our MText region
        b . MText :: ~ MText ();

        return s;
    }


    StringBuffer & StringBuffer :: operator = ( StringBuffer && s )
    {
        buff = std :: move ( s . buff );
        b = std :: move ( s . b );
        return self;
    }

    StringBuffer & StringBuffer :: operator = ( const StringBuffer & s )
    {
        buff . setCapacity ( s . buff . capacity () );
        buff . setSize ( s . buff . size () );
        buff . buffer () . copy ( s . buff . content () );
        b = makeMText ( buff . content (), s . b . count () );
        return self;
    }

    StringBuffer :: StringBuffer ( StringBuffer && s )
        : buff ( std :: move ( s . buff ) )
        , b ( std :: move ( s . b ) )
    {
    }

    //!< reclaim string data
    StringBuffer :: ~ StringBuffer () noexcept
    {
    }

    //!< create an empty buffer
    StringBuffer :: StringBuffer ( Buffer & _buff )
        : buff ( std :: move ( _buff ) )
    {
        buff . setSize ( 0 );
        b = makeMText ( buff . content (), 0 );
    }

    StringBuffer :: StringBuffer ( const RsrcMem & rsrc )
        : buff ( rsrc )
    {
        b = makeMText ( buff . content (), 0 );
    }
}
