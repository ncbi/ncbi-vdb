/*

  vdb3.cmn.str-buffer

 */

#pragma once

#include <vdb3/cmn/string.hpp>
#include <vdb3/cmn/buffer.hpp>

#include <string>
#include <ostream>

/**
 * @file cmn/str-buffer.hpp
 * @brief UTF-8 string buffer class
 *
 * These classes may be implemented in terms of plain old STL string, that
 * has the benefit of a lot of testing. But the STL interfaces has some
 * properties that can lead to unintentional and potentially exploitable errors
 * that we can compensate for with a more careful interface.
 */

namespace vdb3
{

    /*=====================================================*
     *                    StringBuffer                     *
     *=====================================================*/
    
    /**
     * @class StringBuffer
     * @brief a mutable string class for performing text manipulation
     */
    class StringBuffer : TextFactory
    {
    public:


        /*=================================================*
         *                   PREDICATES                    *
         *=================================================*/

        /**
         * isEmpty
         *  @brief predicate for detecting an empty string
         *  @return Boolean true if empty, false if not empty
         */
        inline bool isEmpty () const noexcept
        { return b . isEmpty (); }

        /**
         * isAscii
         *  @return Boolean true if string is 100% ASCII subset of UTF8
         */
        inline bool isAscii () const noexcept
        { return b . isAscii (); }
        
        /**
         * contains
         *  @brief test a CharLoc for inclusion in String
         *  @return Boolean true if location is valid for region
         */
        inline bool contains ( const CharLoc & loc ) const noexcept
        { return b . contains ( loc ); }


        /*=================================================*
         *                   PROPERTIES                    *
         *=================================================*/

        /**
         * capacity
         * @return the number of BYTES in the UTF-8 string buffer
         */
        inline size_t capacity () const noexcept
        { return buff . capacity (); }

        /**
         * size
         * @return the number of BYTES in the UTF-8 string data
         *
         * NB: since the data contained are potentially multi-byte characters,
         * the size in bytes should never be mistakenly used for the count
         * of characters. For the latter, use the count() method.
         */
        inline size_t size () const noexcept
        { return b . size (); }

        /**
         * count
         * @return the number of CHARACTERS in the container
         *
         * In the common case where the character count is identical to the
         * byte count, we can know that all characters are single byte, which
         * in UTF-8 encoding means they are all ASCII with 7 significant bits.
         */
        inline count_t count () const noexcept
        { return b . count (); }

        /**
         * length
         * @return the sequence length measured in CHARACTERS
         *
         * The string length refers to the same quantity as the character count.
         */
        inline count_t length () const noexcept
        { return b . count (); }

        /**
         * end
         *  @return a location of the exclusive end of the String
         */
        inline CharLoc end () const noexcept
        { return b . end (); }

        /**
         * wipeBeforeDelete
         *  @return Boolean true if the storage object will be wiped before delete
         */
        inline bool wipeBeforeDelete () const noexcept
        { return buff . wipeBeforeDelete (); }


        /*=================================================*
         *                    ACCESSORS                    *
         *=================================================*/

        /**
         * data
         * @return a constant MText region reference
         */
        inline const MText & data () const noexcept
        { return b; }

        /**
         * getChar
         * @param idx an ordinal character index into the string
         * @exception IndexOutOfBounds
         * @return a UTF32 character at the given index
         */
        inline UTF32 getChar ( index_t idx ) const
        { return b . getChar ( idx ); }

        /**
         * getChar
         *  @return indexed UNICODE character
         */
        inline UTF32 getChar ( const CharLoc & loc ) const
        { return b . getChar ( loc ); }


        /*=================================================*
         *                   COMPARISON                    *
         *=================================================*/

        /**
         * equal
         * @param sb the string being compared
         * @return boolean if the values are identical
         */
        inline bool equal ( const StringBuffer & sb ) const noexcept
        { return b . equal ( sb . b ); }

        /**
         * equal
         * @param s the string being compared
         * @return boolean if the values are identical
         */
        inline bool equal ( const String & s ) const noexcept
        { return b . equal ( s . data () ); }

        /**
         * equal
         * @param s the string being compared
         * @return boolean if the values are identical
         */
        inline bool equal ( const CText & t ) const noexcept
        { return b . equal ( t ); }

        /**
         * compare
         *  @param sb the string being compared
         *  @return "self" - "r" => { < Z-, 0, Z+ }
         */
        inline int compare ( const StringBuffer & sb ) const noexcept
        { return b . compare ( sb . b ); }

        /**
         * compare
         *  @brief compare self against another region
         *  @param s a String to compare against self
         *  @return "self" - "r" => { < Z-, 0, Z+ }
         */
        inline int compare ( const String & s ) const noexcept
        { return b . compare ( s . data () ); }

        /**
         * compare
         *  @brief compare self against another region
         *  @param t a text region to compare against self
         *  @return "self" - "r" => { < Z-, 0, Z+ }
         */
        inline int compare ( const CText & t ) const noexcept
        { return b . compare ( t ); }

        /**
         * caseInsensitiveCompare
         *  @brief similar to compare() but ignores case for alphabetic characters
         *  @param sb the string being compared
         *  @return tolower ( "self" ) - tolower ( "r" ) => { < Z-, 0, Z+ }
         */
        inline int caseInsensitiveCompare ( const StringBuffer & sb ) const noexcept
        { return b . caseInsensitiveCompare ( sb . b ); }

        /**
         * caseInsensitiveCompare
         *  @brief compare self against another region disregarding case
         *  @param s a String to compare against self
         *  @return tolower ( "self" ) - tolower ( "r" ) => { < Z-, 0, Z+ }
         */
        inline int caseInsensitiveCompare ( const String & s ) const noexcept
        { return b . caseInsensitiveCompare ( s . data () ); }

        /**
         * caseInsensitiveCompare
         *  @brief compare self against another region disregarding case
         *  @param t a text region to compare against self
         *  @return tolower ( "self" ) - tolower ( "r" ) => { < Z-, 0, Z+ }
         */
        inline int caseInsensitiveCompare ( const CText & t ) const noexcept
        { return b . caseInsensitiveCompare ( t ); }


        /*=================================================*
         *                     SEARCH                      *
         *=================================================*/

        /**
         * fwdFind
         * @overload forward search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc fwdFind ( const StringBuffer & sub ) const noexcept
        { return b . fwdFind ( sub . b ); }

        /**
         * fwdFind
         * @overload forward search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc fwdFind ( const String & sub ) const noexcept
        { return b . fwdFind ( sub . data () ); }

        /**
         * fwdFind
         * @overload forward search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc fwdFind ( const CText & sub ) const noexcept
        { return b . fwdFind ( sub ); }

        /**
         * fwdFind
         * @overload forward search to find a single character
         * @param ch a UNICODE character
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc fwdFind ( UTF32 ch ) const noexcept
        { return b . fwdFind ( ch ); }

        /**
         * revFind
         * @overload reverse search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc revFind ( const StringBuffer & sub ) const noexcept
        { return b . revFind ( sub . b ); }

        /**
         * revFind
         * @overload reverse search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc revFind ( const String & sub ) const noexcept
        { return b . revFind ( sub . data () ); }

        /**
         * revFind
         * @overload reverse search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc revFind ( const CText & sub ) const noexcept
        { return b . revFind ( sub ); }

        /**
         * revFind
         * @overload reverse search to find a single character
         * @param ch a UNICODE character
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc revFind ( UTF32 ch ) const noexcept
        { return b . revFind ( ch ); }
    
        /**
         * findFirstOf
         * @overload forward search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end()
         */
        inline CharLoc findFirstOf ( const StringBuffer & cset ) const noexcept
        { return b . findFirstOf ( cset . b ); }
    
        /**
         * findFirstOf
         * @overload forward search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end()
         */
        inline CharLoc findFirstOf ( const String & cset ) const noexcept
        { return b . findFirstOf ( cset . data () ); }
    
        /**
         * findFirstOf
         * @overload forward search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end()
         */
        inline CharLoc findFirstOf ( const CText & cset ) const noexcept
        { return b . findFirstOf ( cset ); }
    
        /**
         * findLastOf
         * @overload reverse search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end()
         */
        inline CharLoc findLastOf ( const StringBuffer & cset ) const noexcept
        { return b . findLastOf ( cset . b ); }
    
        /**
         * findLastOf
         * @overload reverse search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end()
         */
        inline CharLoc findLastOf ( const String & cset ) const noexcept
        { return b . findLastOf ( cset . data () ); }
    
        /**
         * findLastOf
         * @overload reverse search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end()
         */
        inline CharLoc findLastOf ( const CText & cset ) const noexcept
        { return b . findLastOf ( cset ); }

        /**
         * beginsWith
         * @overload test whether self region begins with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self begins with "sub"
         */
        inline bool beginsWith ( const StringBuffer & sub ) const noexcept
        { return b . beginsWith ( sub . b ); }

        /**
         * beginsWith
         * @overload test whether self region begins with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self begins with "sub"
         */
        inline bool beginsWith ( const String & sub ) const noexcept
        { return b . beginsWith ( sub . data () ); }

        /**
         * beginsWith
         * @overload test whether self region begins with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self begins with "sub"
         */
        inline bool beginsWith ( const CText & sub ) const noexcept
        { return b . beginsWith ( sub ); }

        /**
         * beginsWith
         * @overload test whether self region begins with character
         * @param ch UTF32 with character in question
         * @return Boolean true if self begins with "ch"
         */
        inline bool beginsWith ( UTF32 ch ) const noexcept
        { return b . beginsWith ( ch ); }

        /**
         * endsWith
         * @overload test whether self region ends with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self ends with "sub"
         */
        inline bool endsWith ( const StringBuffer & sub ) const noexcept
        { return b . endsWith ( sub . b ); }

        /**
         * endsWith
         * @overload test whether self region ends with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self ends with "sub"
         */
        inline bool endsWith ( const String & sub ) const noexcept
        { return b . endsWith ( sub . data () ); }

        /**
         * endsWith
         * @overload test whether self region ends with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self ends with "sub"
         */
        inline bool endsWith ( const CText & sub ) const noexcept
        { return b . endsWith ( sub ); }

        /**
         * endsWith
         * @overload test whether self region ends with character
         * @param ch UTF32 with character in question
         * @return Boolean true if self ends with "ch"
         */
        inline bool endsWith ( UTF32 ch ) const noexcept
        { return b . endsWith ( ch ); }


        /*=================================================*
         *                     UPDATE                      *
         *=================================================*/

        /**
         * append
         * @overload append provided String onto self
         * @param sb StringBuffer to be appended to self
         * @return C++ reference to self
         */
        StringBuffer & append ( const StringBuffer & sb );

        /**
         * append
         * @overload append provided String onto self
         * @param s String to be appended to self
         * @return C++ reference to self
         */
        StringBuffer & append ( const String & s );

        /**
         * append
         * @overload append provided region onto self
         * @param t a text region
         * @return C++ reference to self
         */
        StringBuffer & append ( const CText & t );

        /**
         * append
         * @overload append provided character onto self
         * @param ch a UTF-32 UNICODE character
         * @return C++ reference to self
         */
        StringBuffer & append ( UTF32 ch );

        /**
         * toupper
         * @brief upper-case contents
         * @return C++ reference to self
         */
        StringBuffer & toupper ();

        /**
         * tolower
         * @brief upper-case contents
         * @return C++ reference to self
         */
        StringBuffer & tolower ();

        /**
         * clear
         * @brief reset internal state to empty
         */
        void clear ();


        /*=================================================*
         *               STRING GENERATION                 *
         *=================================================*/

        /**
         * toString
         * @brief create an immutable String from internal data
         * @return a new String
         */
        String toString ( const RsrcKfc & rsrc ) const;

        /**
         * stealString
         * @brief create an immutable String from internal data and leave contents invalid
         * @return a new String
         */
        String stealString ();


        /*=================================================*
         *           C++ OPERATOR OVERLOADS                *
         *=================================================*/

        /**
         * operator[]
         * @param idx an ordinal character index into the string
         * @exception IndexOutOfBounds
         * @return a UTF32 character at the given index
         */
        inline UTF32 operator [] ( count_t idx ) const
        { return getChar ( idx ); }


        inline bool operator < ( const StringBuffer & s ) const
        { return compare ( s ) < 0; }
        inline bool operator <= ( const StringBuffer & s ) const
        { return compare ( s ) <= 0; }
        inline bool operator == ( const StringBuffer & s ) const
        { return equal ( s ); }
        inline bool operator != ( const StringBuffer & s ) const
        { return ! equal ( s ); }
        inline bool operator >= ( const StringBuffer & s ) const
        { return compare ( s ) >= 0; }
        inline bool operator > ( const StringBuffer & s ) const
        { return compare ( s ) > 0; }

        inline bool operator < ( const String & s ) const
        { return compare ( s ) < 0; }
        inline bool operator <= ( const String & s ) const
        { return compare ( s ) <= 0; }
        inline bool operator == ( const String & s ) const
        { return equal ( s ); }
        inline bool operator != ( const String & s ) const
        { return ! equal ( s ); }
        inline bool operator >= ( const String & s ) const
        { return compare ( s ) >= 0; }
        inline bool operator > ( const String & s ) const
        { return compare ( s ) > 0; }

        inline bool operator < ( const CText & t ) const
        { return compare ( t ) < 0; }
        inline bool operator <= ( const CText & t ) const
        { return compare ( t ) <= 0; }
        inline bool operator == ( const CText & t ) const
        { return equal ( t ); }
        inline bool operator != ( const CText & t ) const
        { return ! equal ( t ); }
        inline bool operator >= ( const CText & t ) const
        { return compare ( t ) >= 0; }
        inline bool operator > ( const CText & t ) const
        { return compare ( t ) > 0; }

        inline StringBuffer & operator += ( const StringBuffer & sb )
        { return append ( sb ); }
        inline StringBuffer & operator += ( const String & s )
        { return append ( s ); }
        inline StringBuffer & operator += ( const CText & t )
        { return append ( t ); }
        inline StringBuffer & operator += ( UTF32 ch )
        { return append ( ch ); }

    
        /*=================================================*
         *                       C++                       *
         *=================================================*/

        StringBuffer & operator = ( StringBuffer && s );
        StringBuffer & operator = ( const StringBuffer & s );
        StringBuffer ( StringBuffer && s );
        StringBuffer ( const StringBuffer & s ) = delete;

        StringBuffer () = delete;
        ~ StringBuffer () noexcept;

        StringBuffer ( Buffer & buff );
        StringBuffer ( const RsrcKfc & rsrc );

    private:

        Buffer buff;
        MText b;
    };

    inline XP & operator << ( XP & xp, const StringBuffer & sb )
    { return xp << sb . data (); }

    inline std :: ostream & operator << ( std :: ostream & o, const StringBuffer & sb )
    { return o << sb . data (); }
    
}
