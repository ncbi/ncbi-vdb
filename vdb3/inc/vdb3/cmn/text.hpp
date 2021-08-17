/*

  vdb3.cmn.text

 */

#pragma once

#include <vdb3/cmn/defs.hpp>
#include <vdb3/kfc/rgn.hpp>
#include <vdb3/kfc/txt-rgn.hpp>
#include <vdb3/kfc/xp.hpp>
#include <vdb3/kfc/except.hpp>

#include <ostream>

#include <cstdarg>

/**
 * @file cmn/text.hpp
 * @brief accessor to a region of addressable memory with text
 */

namespace vdb3
{

    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    class CText;
    

    /*=====================================================*
     *                       CharLoc                       *
     *=====================================================*/

    /**
     * @class CharLoc
     * @brief describes a location within a Text
     */
    class CharLoc
    {
    public:

        inline size_t offset () const noexcept
        { return off; }
        inline index_t index () const noexcept
        { return idx; }

        inline bool operator < ( const CharLoc & loc ) const noexcept
        { return idx < loc . idx; }
        inline bool operator <= ( const CharLoc & loc ) const noexcept
        { return idx <= loc . idx; }
        inline bool operator == ( const CharLoc & loc ) const noexcept
        { return idx == loc . idx; }
        inline bool operator != ( const CharLoc & loc ) const noexcept
        { return idx != loc . idx; }
        inline bool operator >= ( const CharLoc & loc ) const noexcept
        { return idx >= loc . idx; }
        inline bool operator > ( const CharLoc & loc ) const noexcept
        { return idx > loc . idx; }

        inline bool operator < ( index_t loc ) const noexcept
        { return idx < loc; }
        inline bool operator <= ( index_t loc ) const noexcept
        { return idx <= loc; }
        inline bool operator == ( index_t loc ) const noexcept
        { return idx == loc; }
        inline bool operator != ( index_t loc ) const noexcept
        { return idx != loc; }
        inline bool operator >= ( index_t loc ) const noexcept
        { return idx >= loc; }
        inline bool operator > ( index_t loc ) const noexcept
        { return idx > loc; }

        inline CharLoc () noexcept
            : off ( 0 ), idx ( -1 ) {}

    private:

        inline void operator += ( const CharLoc & loc )
        {
            off += loc . off;
            idx += loc . idx;
        }
        
        inline CharLoc ( size_t sz, index_t _idx ) noexcept
            : off ( sz ), idx ( _idx ) {}
        inline CharLoc ( size_t sz, count_t cnt ) noexcept
            : off ( sz ), idx ( ( index_t ) cnt ) {}

        size_t off;
        index_t idx;

        friend class Text;
        friend class StringIterator;
        friend class TextStreamWriter;
    };

    inline std :: ostream & operator << ( std :: ostream & o, const CharLoc & loc )
    {
        return o << "{ " << loc . offset () << ", " << loc . index () << " }";
    }

    inline XP & operator << ( XP & xp, const CharLoc & loc )
    {
        return xp << "{ " << loc . offset () << ", " << loc . index () << " }";
    }


    /*=====================================================*
     *                         Text                        *
     *=====================================================*/
    
    /**
     * @class Text
     * @brief describes a region of addressable virtual memory
     */
    class Text : public TextRgn, public RgnFactory
    {
    public:

        /*=================================================*
         *                   PREDICATES                    *
         *=================================================*/

        /**
         * isEmpty
         *  @brief predicate for detecting an empty region
         *  @return Boolean true if empty, false if not empty
         */
        inline bool isEmpty () const noexcept
        { return sz == 0; }

        /**
         * isAscii
         *  @return Boolean true if region is 100% ASCII subset of UTF8
         */
        inline bool isAscii () const noexcept
        { return cnt == sz; }

        /**
         * contains
         *  @brief test a CharLoc for inclusion in text region
         *  @return Boolean true if location is valid for region
         */
        inline bool contains ( const CharLoc & loc ) const noexcept
        { return loc . index () >= 0 && ( count_t ) loc . index () < cnt; }


        /*=================================================*
         *                   PROPERTIES                    *
         *=================================================*/

#if 0 // these are inherited from TextRgn
        
        /**
         * size
         *  @return number of bytes in the region
         */
        inline size_t size () const noexcept
        { return sz; }

        /**
         * count
         *  @return number of characters in the region
         */
        inline count_t count () const noexcept
        { return cnt; }
#endif
        /**
         * end
         *  @return a location of the exclusive end of the region
         */
        inline CharLoc end () const noexcept
        { return CharLoc ( ( ssize_t ) sz, ( index_t ) cnt ); }


        /*=================================================*
         *                    ACCESSORS                    *
         *=================================================*/

        /**
         * getChar
         *  @return indexed UNICODE character
         */
        UTF32 getChar ( index_t idx ) const;

        /**
         * getChar
         *  @return fully-indexed UNICODE character
         */
        UTF32 getChar ( const CharLoc & pos ) const;

        /**
         * firstChar
         *  @return UNICODE character at index 0
         */
        inline UTF32 firstChar () const
        { return getChar ( 0 ); }

        /**
         * lastChar
         *  @return UNICODE last character
         */
        inline UTF32 lastChar () const
        { return getChar ( ( index_t ) cnt - 1 ); }


        /*=================================================*
         *                    ITERATION                    *
         *=================================================*/

        /**
         * getLoc
         *  @return CharLoc of given index
         */
        CharLoc getLoc ( index_t idx ) const;

        /**
         * nextLoc
         *  @return location advanced by 1 character
         */
        CharLoc nextLoc ( const CharLoc & pos ) const;

        /**
         * prevLoc
         *  @return location rewound by 1 character
         */
        CharLoc prevLoc ( const CharLoc & pos ) const;
        

        /*=================================================*
         *                   COMPARISON                    *
         *=================================================*/

        /**
         * equal
         *  @brief test two text regions for equality
         *  @param r a text region to compare against self
         *  @return true iff the two regions have equal content
         */
        bool equal ( const Text & r ) const noexcept;

        /**
         * compare
         *  @brief compare self against another region
         *  @r text region to compare against self
         *  @return "self" - "r" => { < Z-, 0, Z+ }
         */
        int compare ( const Text & r ) const noexcept;

        /**
         * caseInsensitiveCompare
         *  @brief compare self against another region disregarding case
         *  @r text region to compare against self
         *  @return tolower ( "self" ) - tolower ( "r" ) => { < Z-, 0, Z+ }
         */
        int caseInsensitiveCompare ( const Text & r ) const noexcept;


        /*=================================================*
         *                     SEARCH                      *
         *=================================================*/

        /**
         * fwdFind
         * @overload forward search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found,
         *  a negative location otherwise
         */
        CharLoc fwdFind ( const Text & sub ) const noexcept;

        /**
         * fwdFind
         * @overload forward search to find a single character
         * @param ch a UNICODE character
         * @return the location of start of sub-region if found,
         *  a negative location otherwise
         */
        CharLoc fwdFind ( UTF32 ch ) const noexcept;

        /**
         * revFind
         * @overload reverse search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found,
         *  a negative location otherwise
         */
        CharLoc revFind ( const Text & sub ) const noexcept;

        /**
         * revFind
         * @overload reverse search to find a single character
         * @param ch a UNICODE character
         * @return the location of start of sub-region if found,
         *  a negative location otherwise
         */
        CharLoc revFind ( UTF32 ch ) const noexcept;
    
        /**
         * findFirstOf
         * @overload forward search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found,
         *  a negative location otherwise
         */
        CharLoc findFirstOf ( const Text & cset ) const noexcept;
    
        /**
         * findLastOf
         * @overload reverse search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found,
         *  a negative location otherwise
         */
        CharLoc findLastOf ( const Text & cset ) const noexcept;

        /**
         * beginsWith
         * @overload test whether self region begins with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self begins with "sub"
         */
        bool beginsWith ( const Text & sub ) const noexcept;

        /**
         * beginsWith
         * @overload test whether self region begins with character
         * @param ch UTF32 with character in question
         * @return Boolean true if self begins with "ch"
         */
        bool beginsWith ( UTF32 ch ) const noexcept;

        /**
         * endsWith
         * @overload test whether self region ends with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self ends with "sub"
         */
        bool endsWith ( const Text & sub ) const noexcept;

        /**
         * endsWith
         * @overload test whether self region ends with character
         * @param ch UTF32 with character in question
         * @return Boolean true if self ends with "ch"
         */
        bool endsWith ( UTF32 ch ) const noexcept;


        /*=================================================*
         *           C++ OPERATOR OVERLOADS                *
         *=================================================*/

        /**
         * operator[]
         * @param idx an ordinal character index into the region
         * @exception IndexOutOfBounds
         * @return a UTF32 character at the given index
         */
        inline UTF32 operator [] ( index_t idx ) const
        { return getChar ( idx ); }


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

        
    protected:

        inline CRgn toCRgn () const noexcept
        {
            return makeCRgn ( getAddr (), size () );
        }

        /*=================================================*
         *                       C++                       *
         *=================================================*/

#if 0 // inherited from TextRgn
        inline Text () noexcept
            : a ( nullptr ), sz ( 0 ), cnt ( 0 ) {}
        
        inline ~ Text () noexcept { a = nullptr; sz = 0; cnt = 0; }
#else
        inline Text () noexcept {}
        inline ~ Text () noexcept {}
#endif
        
        /**
         * Text
         *  @brief controlled-access primary constructor
         *  @param _sz the size in bytes of the mapped region
         *  @param _cnt the number of elements in the mapped region
         */
        inline Text ( const UTF8 * _a, size_t _sz, count_t _cnt ) noexcept
            : TextRgn ( _a, _sz, _cnt ) {}
    };

    inline std :: ostream & operator << ( std :: ostream & o, const Text & t )
    {
        o . write ( t . constAddr (), t . size () );
        return o;
    }

    /*=====================================================*
     *                        MText                        *
     *=====================================================*/
    
    /**
     * @class MText
     * @brief describes a region of mutable addressable text
     *
     *  This object fits uncomfortably between two clean concepts:
     *  1. the Rgn describes a range of mapped virtual addresses,
     *     but does not say anything about data contents.
     *  2. the Text describes a sequence of valid UTF-8 characters
     *     that have separate byte and character counts.
     *
     *  When working with text, we need the concept of a
     *  buffer. By definition, such a buffer is uninitialized,
     *  and has zero valid characters, but offers free space.
     *  So there can be no description of character count
     *  within free space of a region. It would be sensible
     *  to draw free space from an MRgn, but the latter has
     *  no indication of potential content type, nor any way
     *  of representing partially valid content.
     *
     *  The MText clearly is a textual object, and it combines
     *  the idea of unformatted free space with valid text.
     */
    class MText : public Text
    {
    public:

        /*=================================================*
         *                    ACCESSORS                    *
         *=================================================*/

        /**
         * addr
         *  @brief access the starting virtual address of memory-mapped region
         *  @return pointer to leading element
         */
        inline UTF8 * addr () const noexcept
        { return a; }

        /**
         * capacity
         *  @return the number of bytes in mutable region
         */
        inline size_t capacity () const noexcept
        { return cap; }


        /*=================================================*
         *                   TYPE CASTS                    *
         *=================================================*/

        /**
         * toMRgn
         * @return MRgn onto entire capacity
         */
        inline MRgn toMRgn () const noexcept
        { return makeMRgn ( addr (), capacity () ); }

        /**
         * toValidMRgn
         * @return MRgn onto valid text
         */
        inline MRgn toValidMRgn () const noexcept
        { return makeMRgn ( addr (), size () ); }


        /*=================================================*
         *                     UPDATE                      *
         *=================================================*/

        /**
         * setChar
         * @overload attempt to set the value of a character
         * @param idx is the index of the character to set
         * @param ch is the new character value
         * @return self
         */
        MText & setChar ( index_t idx, UTF32 ch );

        /**
         * setChar
         * @overload attempt to set the value of a character
         * @param pos is the location of the character to set
         * @param ch is the new character value
         * @return self
         */
        MText & setChar ( const CharLoc & pos, UTF32 ch );

        /**
         * insertChar
         * @overload attempt to insert a character before indicated one
         * @param idx is the index of the existing character
         * @param ch is the new character value
         * @return self
         */
        MText & insertChar ( index_t idx, UTF32 ch );

        /**
         * insertChar
         * @overload attempt to insert a character before indicated one
         * @param pos is the location of the character to set
         * @param ch is the new character value
         * @return self
         */
        MText & insertChar ( const CharLoc & pos, UTF32 ch );

        /**
         * appendChar
         * @overload attempt to add a new character to end of valid region
         * @param ch is the new character value
         * @return self
         */
        MText & appendChar ( UTF32 ch );
        
        /**
         * removeChar
         * @overload attempt to remove an existing character
         * @param idx is the index of the character to remove
         * @return removed character
         */
        UTF32 removeChar ( index_t idx );

        /**
         * removeChar
         * @overload attempt to remove an existing character
         * @param pos is the location of the character to remove
         * @return removed character
         */
        UTF32 removeChar ( const CharLoc & pos );

        /**
         * copy
         *  @brief copy text from a source rgn
         *  @return self
         */
        MText & copy ( const CText & src );

        /**
         * append
         *  @brief append text from a source rgn
         *  @return self
         */
        MText & append ( const CText & src );
        
        /**
         * fill
         *  @brief set all elements of region to single value
         *  @param val is fill value
         *  @param repeat default 0 means all
         *  @return self
         */
        MText & fill ( ASCII val, count_t repeat = 0 );

        /**
         * clear
         *  @brief forget all contents
         *  @param zero if true, fill valid content with zero first
         *  @return self
         */
        MText & clear ( bool zero = false ) noexcept;

        /**
         * format
         *  @brief prints into text region using vsnprintf
         *  @return self
         */
        MText & format ( const UTF8 * fmt, ... )
            __attribute__ ( ( format ( printf, 2, 3 ) ) );

        /**
         * vformat
         *  @brief prints into text region using vsnprintf
         *  @return self
         */
        MText & vformat ( const UTF8 * fmt, va_list args );
        
        /**
         * tolower
         *  @brief lower-case all characters
         *  @return self
         */
        MText & tolower ();
        
        /**
         * toupper
         *  @brief upper-case all characters
         *  @return self
         */
        MText & toupper ();


        /*=================================================*
         *               REGION GENERATION                 *
         *=================================================*/

        /**
         * subRgn
         *  @overload create a new mutable region that is a subset of original
         *  @param start the index of the starting character
         *  @return MText from start to end
         */
        MText subRgn ( index_t start ) const;

        /**
         * subRgn
         *  @overload create a new mutable region that is a subset of original
         *  @param start the index of the starting character
         *  @param len the maximum number of characters to include
         *  @return MText from start to min ( start + len, end )
         */
        MText subRgn ( index_t start, count_t len ) const;

        /**
         * subRgn
         *  @overload create a new mutable region that is a subset of original
         *  @param start the index of the starting character
         *  @param xend exclusive ending location
         *  @return MText from start to min ( xend, end )
         */
        MText subRgn ( index_t start, const CharLoc & xend ) const;

        /**
         * subRgn
         *  @overload create a new mutable region that is a subset of original
         *  @param start the location of the starting character
         *  @return MText from start to end
         */
        MText subRgn ( const CharLoc & start ) const;

        /**
         * subRgn
         *  @overload create a new mutable region that is a subset of original
         *  @param start the location of the starting character
         *  @param len the maximum number of characters to include
         *  @return MText from start to min ( start + len, end )
         */
        MText subRgn ( const CharLoc & start, count_t len ) const;

        /**
         * subRgn
         *  @overload create a new mutable region that is a subset of original
         *  @param start the location of the starting character
         *  @param xend exclusive ending location
         *  @return MText from start to min ( xend, end )
         */
        MText subRgn ( const CharLoc & start, const CharLoc & xend ) const;

        /**
         * intersect
         *  @return intersection between regions
         */
        MText intersect ( const MText & r ) const noexcept;

        /**
         * join
         *  @return union of regions
         */
        MText join ( const MText & r ) const;


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * MText
         *  @brief default constructor
         */
        inline MText () noexcept
            : cap ( 0 )
        {
        }
        
        /**
         * ~MText
         *  @brief trivial destructor
         */
        inline ~ MText () noexcept
        {
            cap = 0;
        }

    protected:

        /**
         * MText
         *  @brief controlled-access primary constructor
         *  @param _a the starting address to a mapped region
         *  @param _cap the size in bytes of the mapped region
         *  @param _sz the number of valid bytes of the mapped region
         *  @param _cnt the number of valid elements in the mapped region
         */
        inline MText ( UTF8 * _a, size_t _sz, count_t _cnt ) noexcept
            : Text ( _a, _sz, _cnt )
            , cap ( _sz )
        {
        }

        /**
         * MText
         *  @brief controlled-access primary constructor
         *  @param _a the starting address to a mapped region
         *  @param _cap the size in bytes of the mapped region
         *  @param _sz the number of valid bytes of the mapped region
         *  @param _cnt the number of valid elements in the mapped region
         */
        inline MText ( UTF8 * _a, size_t _cap, size_t _sz, count_t _cnt ) noexcept
            : Text ( _a, _sz, _cnt )
            , cap ( _cap )
        {
        }

        //!< capacity
        size_t cap;
        
        friend class TextFactory;
    };

    // this helps C++ to complete the cast
    inline XP & operator << ( XP & xp, const MText & t )
    { xp . putTxtRgn ( t ); return xp; }

    
    /*=====================================================*
     *                        CText                        *
     *=====================================================*/
    
    /**
     * @class CText
     * @brief describes a region of constant addressable virtual memory
     */
    class CText : public Text
    {
    public:

        /*=================================================*
         *                    ACCESSORS                    *
         *=================================================*/

        /**
         * addr
         *  @brief access the starting virtual address of memory-mapped region
         *  @return pointer to leading element
         */
        inline const UTF8 * addr () const noexcept
        { return a; }


        /*=================================================*
         *                   TYPE CASTS                    *
         *=================================================*/

        /**
         * toCRgn
         */
        inline CRgn toCRgn () const noexcept
        { return makeCRgn ( addr (), size () ); }


        /*=================================================*
         *               REGION GENERATION                 *
         *=================================================*/

        /**
         * subRgn
         *  @overload create a new region that is a subset of original
         *  @param start the index of the starting character
         *  @return CText from start to end
         */
        CText subRgn ( index_t start ) const;

        /**
         * subRgn
         *  @overload create a new region that is a subset of original
         *  @param start the index of the starting character
         *  @param len the maximum number of characters to include
         *  @return CText from start to min ( start + len, end )
         */
        CText subRgn ( index_t start, count_t len ) const;

        /**
         * subRgn
         *  @overload create a new region that is a subset of original
         *  @param start the index of the starting character
         *  @param xend exclusive ending location
         *  @return CText from start to min ( xend, end )
         */
        CText subRgn ( index_t start, const CharLoc & xend ) const;

        /**
         * subRgn
         *  @overload create a new region that is a subset of original
         *  @param start the location of the starting character
         *  @return CText from start to end
         */
        CText subRgn ( const CharLoc & start ) const;

        /**
         * subRgn
         *  @overload create a new region that is a subset of original
         *  @param start the location of the starting character
         *  @param len the maximum number of characters to include
         *  @return CText from start to min ( start + len, end )
         */
        CText subRgn ( const CharLoc & start, count_t len ) const;

        /**
         * subRgn
         *  @overload create a new region that is a subset of original
         *  @param start the location of the starting character
         *  @param xend exclusive ending location
         *  @return CText from start to min ( xend, end )
         */
        CText subRgn ( const CharLoc & start, const CharLoc & xend ) const;

        /**
         * intersect
         *  @return intersection between regions
         */
        CText intersect ( const CText & r ) const noexcept;

        /**
         * join
         *  @return union of regions
         */
        CText join ( const CText & r ) const;


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * operator=
         *  @brief copy assignment operator
         *  @param r the source region
         *  @return C++ reference to updated CText
         */
        inline CText & operator = ( const MText & r ) noexcept
        {
            Text :: operator = ( r );
            return self;
        }

        /**
         * CText
         *  @brief const casting constructor
         *  @param r the source region
         */
        inline CText ( const MText & r ) noexcept
            : Text ( r ) {}

        /**
         * CText
         *  @brief gain more operations on text
         *  @param r the source region
         */
        inline explicit CText ( const TextRgn & r ) noexcept
            : Text ( r . constAddr (), r . size (), r . count () ) {}

        /**
         * CText
         *  @brief default constructor
         */
        inline CText () noexcept {}
        
        /**
         * ~CText
         *  @brief trivial destructor
         */
        inline ~ CText () noexcept {}

    protected:

        /**
         * CText
         *  @brief controlled-access primary constructor
         *  @param _a the starting address to a mapped region
         *  @param _sz the size in bytes of the mapped region
         *  @param _cnt the number of elements in the mapped region
         */
        inline CText ( const UTF8 * _a, size_t _sz, count_t _cnt ) noexcept
            : Text ( _a, _sz, _cnt )
        {
        }

        friend class TextFactory;
    };

    // this helps C++ to complete the cast
    inline XP & operator << ( XP & xp, const CText & t )
    { xp . putTxtRgn ( t ); return xp; }

    
    /*=====================================================*
     *                     CTextLiteral                    *
     *=====================================================*/

    class CTextLiteral : public CText
    {
    public:

        /**
         * CTextLiteral
         *  @brief constructor for UTF-8 string with known size
         *  @param str is a pointer to unverified UTF-8 text
         *  @param sz is the number of bytes in the UTF-8 text
         *  @param stop_on_bad if false throw exception on bad
         */
        CTextLiteral ( const UTF8 * str, size_t sz, bool stop_on_bad = false );

        /**
         * operator=
         *  @brief assignment from NUL-terminated UTF-8 string
         *  @param zstr is a pointer to unverified, NUL-terminated UTF-8
         */
        CTextLiteral & operator = ( const UTF8 * zstr );

        /**
         * CTextLiteral
         *  @brief constructor for NUL-terminated UTF-8 string
         *  @param zstr is a pointer to unverified, NUL-terminated UTF-8
         */
        CTextLiteral ( const UTF8 * zstr );

        /**
         * CTextLiteral
         *  @brief default constructor
         */
        inline CTextLiteral () noexcept {}
    };

    
    /*=====================================================*
     *                         Char                        *
     *=====================================================*/

    class Char : public CText
    {
    public:

        /**
         * operator=
         *  @param ch a UTF-32 formatted UNICODE character
         */
        Char & operator = ( UTF32 ch );

        /**
         * Char
         *  @param ch a UTF-32 formatted UNICODE character
         */
        Char ( UTF32 ch );

        CXX_RULE_OF_FOUR ( Char );

        Char () noexcept;
        ~ Char () noexcept;

    private:

        UTF8 mbch [ 4 ];
    };

    // this helps C++ to complete the cast
    inline XP & operator << ( XP & xp, const Char & ch )
    { xp . putTxtRgn ( ch ); return xp; }

   
    /*=====================================================*
     *                     TextFactory                     *
     *=====================================================*/

    /**
     * @class TextFactory
     * @brief a RH mixin class giving an implementation
     *  the ability to create an MText and/or a CText from
     *  various other combinations.
     *
     *  these operations are protected because a Text region
     *  is guaranteed to consist of canonical unicode in UTF-8
     *  with properly related size and length.
     */
    class TextFactory
    {
    protected:

        /**
         * makeMText
         * @overload creates an empty MText region of given capacity
         * @param rgn the mapped memory region whose size indicates capacity
         * @return MText with given capacity, but empty text
         */
        static inline MText makeMText ( const MRgn & rgn ) noexcept
        { return MText ( reinterpret_cast < UTF8 * > ( rgn . addr () ), rgn . size (), 0, 0 ); }

        /**
         * makeMText
         * @overload creates a full MText region of given capacity, size and count
         * @param rgn the mapped memory region whose size indicates capacity AND size
         * @param cnt the number of characters in region
         * @return MText with given capacity, and full of text
         */
        static inline MText makeMText ( const MRgn & rgn, count_t cnt ) noexcept
        {
            return MText ( reinterpret_cast < UTF8 * >
                ( rgn . addr () ), rgn . size (), cnt );
        }

        /**
         * makeMText
         * @overload creates an MText region with independent parameters
         * @param a the base address of the region
         * @param cap the size of the mapped region
         * @param sz the number of valid text bytes
         * @param cnt the number of valid text characters
         * @return MText
         */
        static inline MText makeMText ( UTF8 * a, size_t cap, size_t sz, count_t cnt ) noexcept
        { return MText ( a, cap, sz, cnt ); }

        /**
         * makeCText
         * @overload combines a mapped region with a character count
         * @param rgn the mapped region
         * @param cnt the number of characters in region
         * @return CText
         */
        static inline CText makeCText ( const CRgn & rgn, count_t cnt ) noexcept
        {
            return CText ( reinterpret_cast < const UTF8 * >
                ( rgn . addr () ), rgn . size (), cnt );
        }
        
        /**
         * makeCText
         * @overload creates a CText with independent parameters
         * @param a the base address of the region
         * @param sz the number of mapped text bytes
         * @param cnt the number of characters in region
         * @return CText
         */
        static inline CText makeCText ( const UTF8 * a, size_t sz, count_t cnt ) noexcept
        { return CText ( a, sz, cnt ); }
    };

    
    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

    DECLARE_EXCEPTION ( InvalidASCII, InvalidDataException );
    DECLARE_EXCEPTION ( InvalidUTF8, InvalidDataException );
    DECLARE_EXCEPTION ( SplitCharacterException, InvalidStateException );
    DECLARE_EXCEPTION ( InvalidUTF32Character, InvalidArgument );
    DECLARE_EXCEPTION ( UTF8ConversionError, LogicException );

}
