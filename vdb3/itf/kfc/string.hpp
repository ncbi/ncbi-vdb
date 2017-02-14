/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#ifndef _hpp_vdb3_kfc_string_
#define _hpp_vdb3_kfc_string_

namespace vdb3 { class String; } // forwards for includes

#include <kfc/memory.hpp>
#include <kfc/ptr.hpp>

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards
     */
    class StreamBuffer;
    class StringPair;
    class StringBuffer;


    /*------------------------------------------------------------------
     * String
     *  an immutable string object
     */
    class String
    {
    public:

        // length in characters
        count_t length () const
        { return len; }

        // size in bytes
        bytes_t size () const
        { return mem . size (); }

        // access underlying mem
        const Mem & to_mem () const
        { return mem; }

        // search for a character
        index_t find_first ( UTF32 ch ) const;
        index_t find_first ( UTF32 ch, index_t offset ) const;
        index_t find_first ( UTF32 ch, index_t offset, count_t len ) const;

        // create substring
        String substr ( index_t offset ) const;
        String substr ( index_t offset, count_t length ) const;

        // split string at separator point
        // removes separator if width is given
        StringPair split ( index_t offset ) const;
        StringPair split ( index_t offset, count_t sep_width ) const;

        // convert to upper-case
        String toupper () const;

        // trim white-space at both ends
        String trim () const;

        // comparison
        bool operator == ( const String & s ) const;
        bool operator != ( const String & s ) const;

        // C++
        String ();
        String ( const String & s );
        void operator = ( const String & s );
        String ( const String & s, caps_t reduce );
        ~ String ();

    protected:

        // create from ConstString or StringBuffer
        String ( const Mem & m, const bytes_t & ascii_size, count_t len );

    private:

        // the string memory is stored here
        Mem mem;

        // the number of bytes containing ascii-only characters
        // when ascii_size == size (), string is 100% ascii
        bytes_t ascii_size;

        // the number of characters in string
        // when ascii_size == len, string is 100% ascii
        count_t len;

        friend class StringBuffer;
        friend class NULTermString;
    };


    /*------------------------------------------------------------------
     * StringPair
     */
    struct StringPair
    {
        String left, right;

        StringPair () {}
        StringPair ( const String & l, const String r )
            : left ( l ), right ( r ) {}
    };


    /*------------------------------------------------------------------
     * ConstString
     *  create a String from constant data
     */
    class ConstString : public String
    {
    public:

        ConstString ( const char * text, size_t bytes );
    };

#define CONST_STRING( str )                      \
    vdb3 :: ConstString ( str, sizeof str - 1 )


    /*------------------------------------------------------------------
     * NULTermString
     *  create a string that is NUL-terminated,
     *  and sports a cast to const char *
     *  for use with native OS
     */
    class NULTermString : public String
    {
    public:

        // supports nasty cast to NUL-terminated string
        operator char const * () const;

        NULTermString ( const String & str );
        void operator = ( const String & str );
    private:
        const Ptr < char > rgn;
    };


    /*------------------------------------------------------------------
     * StringBuffer
     *  an editible string object
     */
    class StringBuffer
    {
    public:

        count_t length () const
        { return len; }

        bytes_t size () const
        { return bytes; }

        bytes_t capacity () const
        { return buffer . size (); }

        String to_str () const;

        // forward search
        // returns < 0 if not found
        // returns 0..length()-1 if found
        index_t find_first ( UTF32 ch ) const;
        index_t find_first ( UTF32 ch, index_t starting_pos ) const;

        // replace existing contents with new value
        StringBuffer & assign ( const String & s );
        StringBuffer & assign ( const StringBuffer & s );
        StringBuffer & assign ( const char * fmt, ... );
        StringBuffer & vassign ( const char * fmt, va_list args );

        // append text to buffer
        StringBuffer & append ( const String & s );
        StringBuffer & append ( const StringBuffer & s );
        StringBuffer & append ( const char * fmt, ... );
        StringBuffer & vappend ( const char * fmt, va_list args );

        // append repeated character
        StringBuffer & append ( UTF32 ch );
        StringBuffer & append ( UTF32 ch, count_t repeat );

        // contents from a buffer
        bool assign_until ( const bytes_t & limit, UTF32 stop_ch, StreamBuffer & src );
        bool append_until ( const bytes_t & limit, UTF32 stop_ch, StreamBuffer & src );

        // truncate to empty content
        void reset ();

        // trim white-space at both ends
        void trim ();

        // trim end-of-line, if any
        // returns true if found ( and therefore trimmed )
        bool trim_eoln ();

        // convert to upper case
        void toupper ();

        // operators
        StringBuffer & operator  = ( const String & s )        { return assign ( s ); }
        StringBuffer & operator  = ( const StringBuffer & s ) { return assign ( s ); }
        StringBuffer & operator += ( const String & s )        { return append ( s ); }
        StringBuffer & operator += ( const StringBuffer & s ) { return append ( s ); }

        StringBuffer ();
        ~ StringBuffer ();

        StringBuffer ( const Mem & buffer );

        StringBuffer ( const char * fmt, ... );
        StringBuffer ( const char * fmt, va_list args );

    private:

        size_t fappend_simple ( const char * fmt, va_list args, size_t idx );
        size_t fappend_long ( const char * fmt, va_list args, size_t idx );
        void resize ( const bytes_t & size );

        StringBuffer ( const StringBuffer & s );

        Mem buffer;
        bytes_t bytes;
        bytes_t ascii_size;
        count_t len;
    };
}

#endif // _hpp_vdb3_kfc_str_
