/*

  vdb3.cmn.string

 */

#pragma once

#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/rgn-lock.hpp>
#include <vdb3/kfc/except.hpp>
#include <vdb3/kfc/mmgr.hpp>
#include <vdb3/kfc/rsrc-mem.hpp>
#include <vdb3/cmn/text.hpp>
#include <vdb3/cmn/serial.hpp>
#include <vdb3/cmn/fmt.hpp>

#include <string>
#include <ostream>

/**
 * @file cmn/string.hpp
 * @brief UTF-8 string classes
 */

namespace vdb3
{

    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    class StringIterator;

    
    /*=====================================================*
     *                        String                       *
     *=====================================================*/
    
    /**
     * @class String
     * @brief an immutable UTF-8 formatted UNICODE string
     */
    class String : protected TextFactory
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
        { return data () . isEmpty (); }

        /**
         * isAscii
         *  @return Boolean true if string is 100% ASCII subset of UTF8
         */
        inline bool isAscii () const noexcept
        { return data () . isAscii (); }

        /**
         * contains
         *  @brief test a CharLoc for inclusion in String
         *  @return Boolean true if location is valid for region
         */
        inline bool contains ( const CharLoc & loc ) const noexcept
        { return data () . contains ( loc ); }


        /*=================================================*
         *                   PROPERTIES                    *
         *=================================================*/

        /**
         * size
         * @return the number of BYTES in the UTF-8 string data
         *
         * NB: since the data contained are potentially multi-byte characters,
         * the size in bytes should NEVER be mistakenly used for the count
         * of characters. For the latter, use the count() method.
         */
        inline size_t size () const noexcept
        { return data () . size (); }

        /**
         * count
         * @return the number of CHARACTERS in the String container
         *
         * In the common case where the character count is identical to the
         * byte count, we can know that all characters are single byte, which
         * in UTF-8 encoding means they are all ASCII with 7 significant bits.
         */
        inline count_t count () const noexcept
        { return data () . count (); }

        /**
         * length
         * @return the number of CHARACTERS in the String container
         *
         * The string length refers to the same quantity as count().
         */
        inline count_t length () const noexcept
        { return count (); }

        /**
         * end
         *  @return a location of the exclusive end of the String
         */
        inline CharLoc end () const noexcept
        { return data () . end (); }

        /**
         * wipeBeforeDelete
         *  @return Boolean true if the storage object will be wiped before delete
         */
        inline bool wipeBeforeDelete () const
        { return wipe; }


        /*=================================================*
         *                    ACCESSORS                    *
         *=================================================*/
        
        /**
         * data
         * @return a constant CText region reference
         */
        inline const CText & data () const noexcept
        { return rgn; }

        /**
         * getChar
         *  @return indexed UNICODE character
         */
        inline UTF32 getChar ( index_t idx ) const
        { return data () . getChar ( idx ); }

        /**
         * getChar
         *  @return indexed UNICODE character
         */
        inline UTF32 getChar ( const CharLoc & loc ) const
        { return data () . getChar ( loc ); }

        /**
         * firstChar
         *  @return UNICODE character at index 0
         */
        inline UTF32 firstChar () const
        { return data () . firstChar (); }

        /**
         * lastChar
         *  @return UNICODE last character
         */
        inline UTF32 lastChar () const
        { return data () . lastChar (); }

        /**
         * getLoc
         *  @return CharLoc of given index
         */
        inline CharLoc getLoc ( index_t idx ) const
        { return data () . getLoc ( idx ); }

        /**
         * nextLoc
         *  @return location advanced by 1 character
         */
        inline CharLoc nextLoc ( const CharLoc & pos ) const
        { return data () . nextLoc ( pos ); }

        /**
         * prevLoc
         *  @return location rewound by 1 character
         */
        inline CharLoc prevLoc ( const CharLoc & pos ) const
        { return data () . prevLoc ( pos ); }


        /*=================================================*
         *                   COMPARISON                    *
         *=================================================*/

        /**
         * equal
         *  @brief test two text regions for equality
         *  @param s a String to compare against self
         *  @return true iff the two regions have equal content
         */
        inline bool equal ( const String & s ) const noexcept
        { return data () . equal ( s . data () ); }

        /**
         * equal
         *  @brief test two text regions for equality
         *  @param t a text region to compare against self
         *  @return true iff the two regions have equal content
         */
        inline bool equal ( const Text & t ) const noexcept
        { return data () . equal ( t ); }

        /**
         * compare
         *  @brief compare self against another region
         *  @param s a String to compare against self
         *  @return "self" - "r" => { < Z-, 0, Z+ }
         */
        inline int compare ( const String & s ) const noexcept
        { return data () . compare ( s . data () ); }

        /**
         * compare
         *  @brief compare self against another region
         *  @param t a text region to compare against self
         *  @return "self" - "r" => { < Z-, 0, Z+ }
         */
        inline int compare ( const Text & t ) const noexcept
        { return data () . compare ( t ); }

        /**
         * caseInsensitiveCompare
         *  @brief compare self against another region disregarding case
         *  @param s a String to compare against self
         *  @return tolower ( "self" ) - tolower ( "r" ) => { < Z-, 0, Z+ }
         */
        inline int caseInsensitiveCompare ( const String & s ) const noexcept
        { return data () . caseInsensitiveCompare ( s . data () ); }

        /**
         * caseInsensitiveCompare
         *  @brief compare self against another region disregarding case
         *  @param t a text region to compare against self
         *  @return tolower ( "self" ) - tolower ( "r" ) => { < Z-, 0, Z+ }
         */
        inline int caseInsensitiveCompare ( const Text & t ) const noexcept
        { return data () . caseInsensitiveCompare ( t ); }


        /*=================================================*
         *                     SEARCH                      *
         *=================================================*/

        /**
         * fwdFind
         * @overload forward search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc fwdFind ( const String & sub ) const noexcept
        { return data () . fwdFind ( sub . data () ); }

        /**
         * fwdFind
         * @overload forward search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc fwdFind ( const Text & sub ) const noexcept
        { return data () . fwdFind ( sub ); }

        /**
         * fwdFind
         * @overload forward search to find a single character
         * @param ch a UNICODE character
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc fwdFind ( UTF32 ch ) const noexcept
        { return data () . fwdFind ( ch ); }

        /**
         * revFind
         * @overload reverse search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc revFind ( const String & sub ) const noexcept
        { return data () . revFind ( sub . data () ); }

        /**
         * revFind
         * @overload reverse search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc revFind ( const Text & sub ) const noexcept
        { return data () . revFind ( sub ); }

        /**
         * revFind
         * @overload reverse search to find a single character
         * @param ch a UNICODE character
         * @return the location of start of sub-region if found, otherwise end()
         */
        inline CharLoc revFind ( UTF32 ch ) const noexcept
        { return data () . revFind ( ch ); }
    
        /**
         * findFirstOf
         * @overload forward search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end()
         */
        inline CharLoc findFirstOf ( const String & cset ) const noexcept
        { return data () . findFirstOf ( cset . data () ); }
    
        /**
         * findFirstOf
         * @overload forward search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end()
         */
        inline CharLoc findFirstOf ( const Text & cset ) const noexcept
        { return data () . findFirstOf ( cset ); }
    
        /**
         * findLastOf
         * @overload reverse search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end()
         */
        inline CharLoc findLastOf ( const String & cset ) const noexcept
        { return data () . findLastOf ( cset . data () ); }
    
        /**
         * findLastOf
         * @overload reverse search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end()
         */
        inline CharLoc findLastOf ( const Text & cset ) const noexcept
        { return data () . findLastOf ( cset ); }

        /**
         * beginsWith
         * @overload test whether self region begins with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self begins with "sub"
         */
        inline bool beginsWith ( const String & sub ) const noexcept
        { return data () . beginsWith ( sub . data () ); }

        /**
         * beginsWith
         * @overload test whether self region begins with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self begins with "sub"
         */
        inline bool beginsWith ( const Text & sub ) const noexcept
        { return data () . beginsWith ( sub ); }

        /**
         * beginsWith
         * @overload test whether self region begins with character
         * @param ch UTF32 with character in question
         * @return Boolean true if self begins with "ch"
         */
        inline bool beginsWith ( UTF32 ch ) const noexcept
        { return data () . beginsWith ( ch ); }

        /**
         * endsWith
         * @overload test whether self region ends with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self ends with "sub"
         */
        inline bool endsWith ( const String & sub ) const noexcept
        { return data () . endsWith ( sub . data () ); }

        /**
         * endsWith
         * @overload test whether self region ends with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self ends with "sub"
         */
        inline bool endsWith ( const Text & sub ) const noexcept
        { return data () . endsWith ( sub ); }

        /**
         * endsWith
         * @overload test whether self region ends with character
         * @param ch UTF32 with character in question
         * @return Boolean true if self ends with "ch"
         */
        inline bool endsWith ( UTF32 ch ) const noexcept
        { return data () . endsWith ( ch ); }


        /*=================================================*
         *               STRING GENERATION                 *
         *=================================================*/

        /**
         * subString
         *  @overload create a subset string that refers to original
         *  @param start the starting character of substring
         *  @return String from start to end
         */
        inline String subString ( index_t start ) const
        { return String ( rgn_lock, rgn . subRgn ( start ), wipe ); }

        /**
         * subString
         *  @overload create a subset string that refers to original
         *  @param start the starting character of substring
         *  @param len the maximum character length of substring
         *  @return String from start to min ( start + len, end )
         */
        inline String subString ( index_t start, count_t len ) const
        { return String ( rgn_lock, rgn . subRgn ( start, len ), wipe ); }

        /**
         * subString
         *  @overload create a subset string that refers to original
         *  @param start the starting character of substring
         *  @param xend exclusive ending location
         *  @return String from start to min ( xend, end )
         */
        inline String subString ( index_t start, const CharLoc & xend ) const
        { return String ( rgn_lock, rgn . subRgn ( start, xend ), wipe ); }

        /**
         * subString
         *  @overload create a subset string that refers to original
         *  @param start the starting character of substring
         *  @return String from start to end
         */
        inline String subString ( const CharLoc & start ) const
        { return String ( rgn_lock, rgn . subRgn ( start ), wipe ); }

        /**
         * subString
         *  @overload create a subset string that refers to original
         *  @param start the starting character of substring
         *  @param len the maximum character length of substring
         *  @return String from start to min ( start + len, end )
         */
        inline String subString ( const CharLoc & start, count_t len ) const
        { return String ( rgn_lock, rgn . subRgn ( start, len ), wipe ); }

        /**
         * subString
         *  @overload create a subset string that refers to original
         *  @param start the starting character of substring
         *  @param xend exclusive ending location
         *  @return String from start to min ( xend, end )
         */
        inline String subString ( const CharLoc & start, const CharLoc & xend ) const
        { return String ( rgn_lock, rgn . subRgn ( start, xend ), wipe ); }

        /**
         * concat
         * @overload concatenate self and provided String into a new String
         * @param s String to be concatenated to self
         * @return a new String
         */
        String concat ( const RsrcKfc & rsrc, const String & s ) const;

        /**
         * toupper
         * @return upper-cased copy of String
         */
        String toupper ( const RsrcKfc & rsrc ) const;

        /**
         * tolower
         * @return lower-cased copy of String
         */
        String tolower ( const RsrcKfc & rsrc ) const;

        /**
         * toSTLString
         * @return an STL version of craziness
         */
        inline std :: string toSTLString () const
        { return std :: string ( data () . addr (), data () . size () ); }

        /**
         * makeIterator
         * @return an Iterator to an initial point within String
         */
        StringIterator makeIterator ( index_t origin = 0 ) const noexcept;

        /**
         * makeIterator
         * @return an Iterator to an initial point within String
         */
        StringIterator makeIterator ( const CharLoc & loc ) const noexcept;


        /*=================================================*
         *                  SERIALIZATION                  *
         *=================================================*/

        Serial & put ( const RsrcKfc & rsrc, Serial & s ) const
        { return s . put ( rsrc, data () ); }
        Serial & putTyped ( const RsrcKfc & rsrc, Serial & s, SerialType type ) const
        { return s . putTyped ( rsrc, type, data () ); }
        Serial & putMbr ( const RsrcKfc & rsrc, Serial & s, N32 mbr ) const
        { return s . putMbr ( rsrc, mbr, data () ); }


        /*=================================================*
         *           C++ OPERATOR OVERLOADS                *
         *=================================================*/

        /**
         * operator[]
         * @param idx an ordinal character index into the string
         * @exception IndexOutOfBounds
         * @return a UTF32 character at the given index
         */
        inline UTF32 operator [] ( index_t idx ) const
        { return getChar ( idx ); }


        inline bool operator < ( const String & s ) const noexcept
        { return compare ( s ) < 0; }
        inline bool operator <= ( const String & s ) const noexcept
        { return compare ( s ) <= 0; }
        inline bool operator == ( const String & s ) const noexcept
        { return equal ( s ); }
        inline bool operator != ( const String & s ) const noexcept
        { return ! equal ( s ); }
        inline bool operator >= ( const String & s ) const noexcept
        { return compare ( s ) >= 0; }
        inline bool operator > ( const String & s ) const noexcept
        { return compare ( s ) > 0; }

        inline bool operator < ( const Text & t ) const noexcept
        { return compare ( t ) < 0; }
        inline bool operator <= ( const Text & t ) const noexcept
        { return compare ( t ) <= 0; }
        inline bool operator == ( const Text & t ) const noexcept
        { return equal ( t ); }
        inline bool operator != ( const Text & t ) const noexcept
        { return ! equal ( t ); }
        inline bool operator >= ( const Text & t ) const noexcept
        { return compare ( t ) >= 0; }
        inline bool operator > ( const Text & t ) const noexcept
        { return compare ( t ) > 0; }

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_SIX_NE ( String );

        // allow construction and assignment from manifest constants
        String & operator = ( const UTF8 * zs );
        String ( const UTF8 *zs );

        // manifest constants should be made explicit here
        String & operator = ( const CTextLiteral & t ) noexcept;
        String ( const CTextLiteral & t ) noexcept;

        // non-constants must be copied into a new allocation
        String ( const RsrcKfc & rsrc, const CText & t, bool wipe = true );
        String ( const RsrcKfc & rsrc, const std :: string & str, bool wipe = true );
        String ( const RsrcKfc & rsrc, const UTF8 * zstr, bool wipe = true );
        String ( const RsrcKfc & rsrc, const UTF16 * zstr, bool wipe = true );
        String ( const RsrcKfc & rsrc, const UTF32 * zstr, bool wipe = true );

        // recreate from serialization
        String ( const RsrcKfc & rsrc, Deserial & d, bool wipe = true );

    protected:

        void normalize ( const RsrcKfc & rsrc, UTF32 * buffer, count_t ccnt );
        
        String ( const CRgnLockRef & lock, const CText & t, bool wipe );
        
        CRgnLockRef rgn_lock;
        CText rgn;
        bool wipe;

        friend class StringBuffer;
    };

    inline XP & operator << ( XP & xp, const String & s )
    { return xp << s . data (); }

    inline Fmt & operator << ( Fmt & fmt, const String & s )
    { return fmt << s . data (); }

    inline std :: ostream & operator << ( std :: ostream & o, const String & s )
    { return o << s . data (); }
    

    /*=====================================================*
     *                    StringIterator                   *
     *=====================================================*/
    
    class StringIterator
    {
    public:
        
        /**
         * isValid
         * @return Boolean true if Iterator index is within bounds
         */
        inline bool isValid () const noexcept
        { return rgn . contains ( cur ); }

        /**
         * pos
         * @return current iterator position
         */
        inline const CharLoc & pos () const noexcept
        { return cur; }

        /**
         * getChar
         * @brief retrieve character at current position
         * @exception IndexOutOfBounds
         * @return UTF-32 UNICODE character
         */
        inline UTF32 getChar () const
        { return rgn . getChar ( cur ); }

        /**
         * fwdFind
         * @overload find and relocate to position of sub string
         * @param sub the substring to locate
         * @return Boolean true if found
         *
         * The idea is to find a substring and position the iterator
         * to point to its first character. If the substring is not found,
         * the iterator remains unchanged.
         */
        bool fwdFind ( const String & sub );

        /**
         * fwdFind
         * @overload forward search to find a sub-region
         * @param sub the sub-region being sought
         * @return Boolean true if found
         */
        bool fwdFind ( const Text & sub );

        /**
         * fwdFind
         * @overload forward search to find a single character
         * @param ch a UNICODE character
         * @return Boolean true if found
         */
        bool fwdFind ( UTF32 ch );

        /**
         * revFind
         * @overload reverse search to find a sub-region
         * @param sub the sub-region being sought
         * @return Boolean true if found
         */
        bool revFind ( const String & sub );

        /**
         * revFind
         * @overload reverse search to find a sub-region
         * @param sub the sub-region being sought
         * @return Boolean true if found
         */
        bool revFind ( const Text & sub );

        /**
         * revFind
         * @overload reverse search to find a single character
         * @param ch a UNICODE character
         * @return Boolean true if found
         */
        bool revFind ( UTF32 ch );
    
        /**
         * findFirstOf
         * @overload forward search to find a character of a set
         * @param cset the set of characters being sought
         * @return Boolean true if found
         */
        bool findFirstOf ( const String & cset );
    
        /**
         * findFirstOf
         * @overload forward search to find a character of a set
         * @param cset the set of characters being sought
         * @return Boolean true if found
         */
        bool findFirstOf ( const Text & cset );
    
        /**
         * findLastOf
         * @overload reverse search to find a character of a set
         * @param cset the set of characters being sought
         * @return Boolean true if found
         */
        bool findLastOf ( const String & cset );
    
        /**
         * findLastOf
         * @overload reverse search to find a character of a set
         * @param cset the set of characters being sought
         * @return Boolean true if found
         */
        bool findLastOf ( const Text & cset );


        /*=================================================*
         *           C++ OPERATOR OVERLOADS                *
         *=================================================*/

        inline StringIterator & operator ++ ()
        { cur = rgn . nextLoc ( cur ); return self; }
        inline StringIterator & operator -- ()
        { cur = rgn . prevLoc ( cur ); return self; }

        inline UTF32 operator * () const
        { return getChar (); }

        bool operator < ( const StringIterator & i ) const;
        bool operator <= ( const StringIterator & i ) const;
        bool operator == ( const StringIterator & i ) const;
        bool operator != ( const StringIterator & i ) const;
        bool operator >= ( const StringIterator & i ) const;
        bool operator > ( const StringIterator & i ) const;


        CXX_RULE_OF_SIX_NE ( StringIterator );

    private:

        StringIterator ( const CRgnLockRef & lock, const CText & rgn, const CharLoc & cur ) noexcept;
        

        CRgnLockRef rgn_lock;
        CText rgn;
        CharLoc cur;

        friend class String;
    };


    /*=====================================================*
     *                    NULTermString                    *
     *=====================================================*/
    
    /**
     * @class NULTermString
     * @brief an immutable string class that guarantees upon construction a NUL terminator
     */
    class NULTermString : public String
    {
    public:

        /*=================================================*
         *                    ACCESSORS                    *
         *=================================================*/

        /**
         * c_str
         * @return a raw pointer to a NUL-terminated C++ string
         */
        const UTF8 * c_str () const noexcept;

        /**
         * NULTermString
         * @overload constructor from a String
         *
         * guarantees NUL termination
         */
        NULTermString ( const RsrcKfc & rsrc, const String & s );

        CXX_RULE_OF_FOUR ( NULTermString );

        inline NULTermString () noexcept {}
        inline ~ NULTermString () noexcept {}
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

    DECLARE_EXCEPTION ( InvalidUTF8String, InvalidArgument );
    DECLARE_EXCEPTION ( InvalidUTF16String, InvalidArgument );
    DECLARE_EXCEPTION ( InvalidNumeral, InvalidArgument );
}
