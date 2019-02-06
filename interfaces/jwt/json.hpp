/*==============================================================================
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

#ifndef _hpp_ncbi_oauth_json_
#define _hpp_ncbi_oauth_json_

#include "jwt-pair.hpp"
#include "jwt-string.hpp"
#include "jwt-vector.hpp"
#include "jwt-map.hpp"

namespace ncbi
{
    class JSON;
    class JSONArray;
    class JSONObject;
    class JSONValue;
    struct JSONString;
    struct JSONNumber;

    /* JSONException
     **********************************************************************************/
    class JSONException //: public std :: exception
    {
    public:

        virtual const char * what () const
            throw ();

        explicit JSONException ( const char * function, unsigned int line, const char * message );
        virtual ~JSONException ()
            throw ();

    private:

        JwtString msg;
        const char * fl_msg;
    };

    /* JSONValue interface
     **********************************************************************************/
    class JSONValue
    {
    public:

        struct Limits
        {
            Limits ();                        // set default limits

            unsigned int json_string_size;    // total size of JSON string
            unsigned int recursion_depth;     // parser stack depth
            unsigned int numeral_length;      // maximum number of characters in number
            unsigned int string_size;         // maximum number of bytes in string
            unsigned int array_elem_count;    // maximum number of elements in array
            unsigned int object_mbr_count;    // maximum number of members in object
        };

        // make various value types
        static JSONValue * makeNull ();
        static JSONValue * makeBool ( bool val );
        static JSONValue * makeInteger ( long long int val );
        static JSONValue * makeDouble ( long double val, unsigned int precision );
        static JSONValue * makeNumber ( const JwtString & val );
        static JSONValue * makeString ( const JwtString & val );

        // query value type
        virtual bool isNull () const;
        virtual bool isBool () const;
        virtual bool isInteger () const;        // a number that is an integer
        virtual bool isNumber () const;         // is any type of number
        virtual bool isString () const;         // is specifically a string
        virtual bool isArray () const;
        virtual bool isObject () const;

        // set value - can change value type
        virtual JSONValue & setNull ();
        virtual JSONValue & setBool ( bool val );
        virtual JSONValue & setInteger ( long long int val );
        virtual JSONValue & setDouble ( long double val, unsigned int precision );
        virtual JSONValue & setNumber ( const JwtString & val );
        virtual JSONValue & setString ( const JwtString & val );

        // retrieve a value - will attempt to convert if possible
        // throws an exception if conversion is not supported
        virtual bool toBool () const;
        virtual long long toInteger () const;
        virtual JwtString toNumber () const;
        virtual JwtString toString () const;
        virtual JwtString toJSON () const = 0;

        // retrieve as structured value - will not convert
        // throws an exception if not of the correct container type
        virtual JSONArray & toArray ();
        virtual const JSONArray & toArray () const;
        virtual JSONObject & toObject ();
        virtual const JSONObject & toObject () const;

        // create a copy
        virtual JSONValue * clone () const;

        // invalidate and overwrite contents
        virtual void invalidate () = 0;

        virtual ~JSONValue ();

    protected:

        static JSONValue * makeParsedNumber ( const JwtString & val );
        static JSONValue * makeParsedString ( const JwtString & val );

        static JSONValue * parse ( const Limits & lim, const JwtString & json, size_t & pos, unsigned int depth );
        static Limits default_limits;

        JSONValue ();

        friend class JSON;
        friend struct JSONNumber;
        friend struct JSONString;
    };

    /* JSONArray
     * array of JSONValues
     **********************************************************************************/
    class JSONArray : public JSONValue
    {
    public:

        // make an empty array
        static JSONArray * make ();

        // JSONValue interface implementations
        virtual bool isArray () const
        { return true; }
        virtual JwtString toString () const;
        virtual JwtString toJSON () const;
        virtual JSONArray & toArray ()
        { return * this; }
        virtual const JSONArray & toArray () const
        { return * this; }
        virtual JSONValue * clone () const;

        // for creating a "readable" JSON text
        JwtString readableJSON ( unsigned int indent = 0 ) const;

        // asks whether array is empty
        bool isEmpty () const;

        // return the number of elements
        unsigned long int count () const;

        // does an element exist
        bool exists ( long int idx ) const;

        // add a new element to end of array
        void appendValue ( JSONValue * elem );

        // set entry to a new value
        // will fill any undefined intermediate elements with null values
        // throws exception on negative index
        void setValue ( long int idx, JSONValue * elem );

        // get value at index
        // throws exception on negative index or when element is undefined
        JSONValue & getValue ( long int idx );
        const JSONValue & getValue ( long int idx ) const;

        // remove and return an entry if valid
        // returns nullptr if index was negative or element undefined
        // replaces valid internal entries with null element
        // deletes trailing null elements making them undefined
        JSONValue * removeValue ( long int idx );

        // lock the array against change
        void lock ();

        // invalidate and overwrite contents
        void invalidate ();

        // C++ assignment
        JSONArray & operator = ( const JSONArray & array );
        JSONArray ( const JSONArray & a );

        virtual ~JSONArray ();

    private:

        static JSONArray * parse ( const Limits & lim, const JwtString & json, size_t & pos, unsigned int depth );

        // used to empty out the array before copy
        void clear ();

        JSONArray ();

        JwtVector < JSONValue * > array;
        bool locked;

        friend class JSON;
        friend class JSONValue;

    };

    /* JSONObject
     * map of key <string> / value <JSONValue*> pairs
     **********************************************************************************/
    class JSONObject : public JSONValue
    {
    public:

        // make an empty object
        static JSONObject * make ();

        // make an object from source
        static JSONObject * parse ( const JwtString & json );
        static JSONObject * parse ( const Limits & lim, const JwtString & json );

        // JSONValue interface implementations
        virtual bool isObject () const
        { return true; }
        virtual JwtString toString () const;
        virtual JwtString toJSON () const;
        virtual JSONObject & toObject ()
        { return * this; }
        virtual const JSONObject & toObject () const
        { return * this; }
        virtual JSONValue * clone () const;

        // for creating a "readable" JSON text
        JwtString readableJSON ( unsigned int indent = 0 ) const;

        // asks whether object is empty
        bool isEmpty () const;

        // does a member exist
        bool exists ( const JwtString & name ) const;

        // return the number of members
        unsigned long int count () const;

        // return names/keys
        JwtVector < JwtString > getNames () const;

        // add a new ( name, value ) pair
        // "name" must be unique or an exception will be thrown
        void addNameValuePair ( const JwtString & name, JSONValue * val );
        void addFinalNameValuePair ( const JwtString & name, JSONValue * val );

        // set entry to a new value
        // throws exception if entry exists and is final
        void setValue ( const JwtString & name, JSONValue * val );
        void setValueOrDelete ( const JwtString & name, JSONValue * val );

        // set entry to a final value
        // throws exception if entry exists and is final
        void setFinalValue ( const JwtString & name, JSONValue * val );
        void setFinalValueOrDelete ( const JwtString & name, JSONValue * val );

        // get named value
        JSONValue & getValue ( const JwtString & name );
        const JSONValue & getValue ( const JwtString & name ) const;

        // remove and delete named value
        void removeValue ( const JwtString & name );

        // lock the object against change
        void lock ();

        // invalidate and overwrite contents
        void invalidate ();

        // C++ assignment
        JSONObject & operator = ( const JSONObject & obj );
        JSONObject ( const JSONObject & obj );

        virtual ~JSONObject ();


    private:

        static JSONObject * parse ( const Limits & lim, const JwtString & json, size_t & pos, unsigned int depth );

        void clear ();

        JSONObject ();

        JwtMap < JwtString, JwtPair<bool, JSONValue *> > members;
        bool locked;

        friend class JSON;
        friend class JSONValue;
    };

    class JSON
    {
    public:

        // make an object from JSON source
        static JSONValue * parse ( const JwtString & json );
        static JSONValue * parse ( const JSONValue :: Limits & lim, const JwtString & json );
    };

}
#endif /* _hpp_ncbi_oauth_json_ */
