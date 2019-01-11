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

namespace ncbi
{
    JwtString double_to_string ( long double val, unsigned int precision )
    {
        // TBD - come up with a more precise cutoff
        if ( precision > 40 )
            precision = 40;

        char buffer [ 1024 ];
        int len = std :: snprintf ( buffer, sizeof buffer, "%.*Lg", precision, val );
        if ( len < 0 || ( size_t ) len >= sizeof buffer )
            throw JSONException ( __func__, __LINE__, "failed to convert long double to string" );

        return JwtString ( buffer, len );
    }

    // factory functions
    // they make private implementations
    JSONValue * JSONValue :: makeNull ()
    {
        return new JSONWrapper ( jvt_null );
    }

    JSONValue * JSONValue :: makeBool ( bool val )
    {
        return new JSONWrapper ( jvt_bool, new JSONBoolean ( val ) );
    }

    JSONValue * JSONValue :: makeInteger ( long long int val )
    {
        return new JSONWrapper ( jvt_int, new JSONInteger ( val ) );
    }

    JSONValue * JSONValue :: makeDouble ( long double val, unsigned int precision )
    {
        return makeParsedNumber ( double_to_string ( val, precision ) );
    }

    JSONValue * JSONValue :: makeParsedNumber ( const JwtString & val )
    {
        return new JSONWrapper ( jvt_num, new JSONNumber ( val ) );
    }

    JSONValue * JSONValue :: makeParsedString ( const JwtString & val )
    {
        return new JSONWrapper ( jvt_str, new JSONString ( val ) );
    }

    // These are all false because it's a base class and can't know what it is
    bool JSONValue :: isNull () const
    {
        return false;
    }

    bool JSONValue :: isBool () const
    {
        return false;
    }

    bool JSONValue :: isInteger () const
    {
        return false;
    }

    bool JSONValue :: isNumber () const
    {
        return false;
    }

    bool JSONValue :: isString () const
    {
        return false;
    }

    bool JSONValue :: isArray () const
    {
        return false;
    }

    bool JSONValue :: isObject () const
    {
        return false;
    }

    // the sets all throw exceptions
    JSONValue & JSONValue :: setNull ()
    {
        // this is an internal error
        // same for other setters
        throw JSONException ( __func__, __LINE__, "INTERNAL ERROR" ); // test hit
    }

    JSONValue & JSONValue :: setBool ( bool val )
    {
        throw JSONException ( __func__, __LINE__, "INTERNAL ERROR" ); // test hit
    }

    JSONValue & JSONValue :: setInteger ( long long int val )
    {
        throw JSONException ( __func__, __LINE__, "INTERNAL ERROR" ); // test hit
    }

    JSONValue & JSONValue :: setDouble ( long double val, unsigned int precision )
    {
        throw JSONException ( __func__, __LINE__, "INTERNAL ERROR" ); // test hit
    }

    JSONValue & JSONValue :: setNumber ( const JwtString & val )
    {
        throw JSONException ( __func__, __LINE__, "INTERNAL ERROR" ); // test hit
    }

    JSONValue & JSONValue :: setString ( const JwtString & val )
    {
        throw JSONException ( __func__, __LINE__, "INTERNAL ERROR" ); // test hit
    }

    // these are accessors
    bool JSONValue :: toBool () const
    {
        // this is a logic-error
        // same for other casts
        throw JSONException ( __func__, __LINE__, "this value cannot be converted to bool" ); //test hit
    }

    long long JSONValue :: toInteger () const
    {
        throw JSONException ( __func__, __LINE__, "this value cannot be converted to an integer" ); // test hit
    }

    JwtString JSONValue :: toNumber () const
    {
        throw JSONException ( __func__, __LINE__, "this value cannot be converted to a number" ); // test hit
    }

    JwtString JSONValue :: toString () const
    {
       // default behavior is to return JSON text
        return toJSON ();
    }

    JSONArray & JSONValue :: toArray ()
    {
        throw JSONException ( __func__, __LINE__, "this value cannot be converted to an array" ); // test hit
    }

    const JSONArray & JSONValue :: toArray () const
    {
        throw JSONException ( __func__, __LINE__, "this value cannot be converted to an array" );
    }

    JSONObject & JSONValue :: toObject ()
    {
        throw JSONException ( __func__, __LINE__, "this value cannot be converted to an object" ); // test hit
    }

    const JSONObject & JSONValue :: toObject () const
    {
        throw JSONException ( __func__, __LINE__, "this value cannot be converted to an object" );
    }

    JSONValue * JSONValue :: clone () const
    {
        // this is an internal
        throw JSONException ( __func__, __LINE__, "this value cannot be cloned" );
    }

    JSONValue :: JSONValue ()
    {
        // nothing to do
    }

    JSONValue :: ~JSONValue ()
    {
        // nothing to do
    }

}
