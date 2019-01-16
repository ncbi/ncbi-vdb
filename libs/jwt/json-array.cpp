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

#include "json-priv.hpp"
#include "jwt-vector-impl.hpp"

namespace ncbi
{
    // make an empty array
    JSONArray * JSONArray :: make ()
    {
        return new JSONArray ();
    }

    JwtString JSONArray :: toString () const
    {
        throw JSONException ( __func__, __LINE__, "this value cannot be converted to a string" );
    }

    JwtString JSONArray :: toJSON () const
    {
        JwtString to_string = "[";

        const char* sep = "";
        auto size = array . size ();

        for ( size_t i = 0; i < size; ++ i )
        {
            const JSONValue* value = array [ i ];

            to_string += sep;
            to_string += value -> toJSON();

            sep = ",";
        }

        to_string += "]";

        return to_string;
    }

    JwtString JSONArray :: readableJSON ( unsigned int indent ) const
    {
        JwtString margin;
        for ( unsigned int i = 0; i < indent; ++ i )
            margin += "    ";

        JwtString to_string = margin + '[';
        margin += "    ";

        const char* sep = "\n";

        auto size = array . size ();

        for ( size_t i = 0; i < size; ++ i )
        {
            const JSONValue* value = array [ i ];

            to_string += sep;
            if ( value -> isArray () )
            {
                to_string += value -> toArray () . readableJSON ( indent + 1 );
            }
            else if ( value -> isObject () )
            {
                to_string += value -> toObject () . readableJSON ( indent + 1 );
            }
            else
            {
                to_string += margin + value -> toJSON ();
            }

            sep = ",\n";
        }

        to_string += "\n";
        to_string += margin . substr ( 4 );
        to_string += "]";

        return to_string;
    }

    JSONValue * JSONArray :: clone () const
    {
        JSONArray * copy = new JSONArray ();

        *copy = *this;

        return copy;
    }

    void JSONArray :: invalidate ()
    {
        size_t i, count = array . size ();
        for ( i = 0; i < count; ++ i )
        {
            array [ i ] -> invalidate ();
        }
    }

    // asks whether array is empty
    bool JSONArray :: isEmpty () const
    {
        return array . empty ();
    }

    // return the number of elements
    unsigned long int JSONArray :: count () const
    {
        return ( unsigned long int ) array . size ();
    }

    // does an element exist
    bool JSONArray :: exists ( long int idx ) const
    {
        if ( idx < 0 || ( size_t ) idx >= array . size () )
            return false;

        // TBD - determine whether null objects are considered to exist...
        return array [ idx ] -> isNull ();
    }

    // add a new element to end of array
    void JSONArray :: appendValue ( JSONValue * elem )
    {
        if ( locked )
            throw JSONException ( __func__, __LINE__, "array object cannot be modified" );
        if ( elem == nullptr )
            elem = JSONValue :: makeNull ();

        array . push_back ( elem );
    }

    // set entry to a new value
    // will fill any undefined intermediate elements with null values
    // throws exception on negative index
    void JSONArray :: setValue ( long int idx, JSONValue * elem )
    {
        if ( locked )
            throw JSONException ( __func__, __LINE__, "array object cannot be modified" );
        if ( idx < 0 )
            throw JSONException ( __func__, __LINE__, "illegal index value" );

        if ( elem == nullptr )
        {
            // basically trying to remove what's there
            removeValue ( idx );
        }
        else
        {
            // fill whatever is in-between
            while ( ( size_t ) idx > array . size () )
                appendValue ( JSONValue :: makeNull () );

            // append...
            if ( ( size_t ) idx == array . size () )
                array . push_back ( elem );

            // or replace
            else
            {
                delete array [ idx ];
                array [ idx ] = elem;
            }
        }

    }

    // get value at index
    // throws exception on negative index or when element is undefined
    JSONValue & JSONArray :: getValue ( long int idx )
    {
        if ( idx < 0 || ( size_t ) idx >= array . size () )
            throw JSONException ( __func__, __LINE__, "illegal index value" );

        return * array [ idx ];
    }

    const JSONValue & JSONArray :: getValue ( long int idx ) const
    {
        if ( idx < 0 || ( size_t ) idx >= array . size () )
            throw JSONException ( __func__, __LINE__, "illegal index value" );

        return * array [ idx ];
    }

    // remove and return an entry if valid
    // returns nullptr if index was negative or element undefined
    // replaces valid internal entries with null element
    // deletes trailing null elements making them undefined
    JSONValue * JSONArray :: removeValue ( long int idx )
    {
        if ( locked )
            throw JSONException ( __func__, __LINE__, "array object cannot be modified" );
       // test for illegal index
        if ( idx < 0 || ( size_t ) idx >= array . size () )
            return nullptr;

        // if the element is already null
        if ( array [ idx ] -> isNull () )
            return nullptr;

        // grab existing element to return
        JSONValue * prior = array [ idx ];

        // if it was not the last element in the array
        // just replace it with a null value
        if ( ( size_t ) idx + 1 < array . size () )
            array [ idx ] = JSONValue :: makeNull ();
        else
        {
            // otherwise, forget the last element in the array
            array . pop_back ();

            // and test from the back toward front for null elements
            // up until the first non-null (resize the array)
            while ( ! array . empty () )
            {
                // any non-null element breaks loop
                if ( ! array . back () -> isNull () )
                    break;

                // delete the null
                //delete array . back ();
                array . pop_back ();
            }
        }

        return prior;
    }

    // C++ assignment
    JSONArray & JSONArray :: operator = ( const JSONArray & a )
    {
        // forget everything we have right now
        clear ();

        // run through all of the new ones
        size_t i, count = a . array . size ();
        for ( i = 0; i < count; ++ i )
        {
            // clone them
            JSONValue * elem = a . array [ i ] -> clone ();
            // append them
            appendValue ( elem );
        }

        locked = a . locked;

        return * this;
    }

    JSONArray :: JSONArray ( const JSONArray & a )
    : locked ( false )
    {
        size_t i, count = a . array . size ();
        for ( i = 0; i < count; ++ i )
        {
            JSONValue * elem = a . array [ i ] -> clone ();
            appendValue ( elem );
        }
        locked = a . locked;
    }

    JSONArray :: ~ JSONArray ()
    {
        locked = false;
        try
        {
            clear ();
        }
        catch ( ... )
        {
        }
    }

    // used to empty out the array before copy
    void JSONArray :: clear ()
    {
        if ( locked )
            throw JSONException ( __func__, __LINE__, "array object cannot be modified" );
        while ( ! array . empty () )
        {
            delete array . back ();
            array . pop_back ();
        }
    }

    void JSONArray :: lock ()
    {
        locked = true;
    }

    JSONArray :: JSONArray ()
    : locked ( false )
    {
    }

}
