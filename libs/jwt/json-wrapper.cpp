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

#include <assert.h>

namespace ncbi
{
    // JSONWrapper - an implementation of JSONValue

    bool JSONWrapper :: isNull () const
    {
        return type == jvt_null;
    }

    bool JSONWrapper :: isBool () const
    {
        return type == jvt_bool;
    }

    bool JSONWrapper :: isInteger () const
    {
        return type == jvt_int;
    }

    bool JSONWrapper :: isNumber () const
    {
        return type == jvt_num;
    }

    bool JSONWrapper :: isString () const
    {
        return type == jvt_str;
    }

    JSONValue & JSONWrapper :: setNull ()
    {
        delete value;
        value = nullptr;
        type = jvt_null;

        return * this;
    }

    JSONValue & JSONWrapper :: setBool ( bool val )
    {
        delete value;
        value = new JSONBoolean ( val );
        type = jvt_bool;

        return * this;
    }

    JSONValue & JSONWrapper :: setInteger ( long long int val )
    {
        delete value;
        value = new JSONInteger ( val );
        type = jvt_int;

        return * this;
    }

    JSONValue & JSONWrapper :: setDouble ( long double val, unsigned int precision )
    {
        return setNumber ( double_to_string ( val, precision ) );
    }

    JSONValue & JSONWrapper :: setNumber ( const JwtString & val )
    {
        delete value;
        value = new JSONNumber ( val );
        type = jvt_num;

        return * this;
    }

    JSONValue & JSONWrapper :: setString ( const JwtString & val )
    {
        delete value;
        value = new JSONString ( val );
        type = jvt_str;

        return * this;
    }

    bool JSONWrapper :: toBool () const
    {
        if ( type == jvt_bool )
        {
            assert ( value != nullptr );
            return ( ( const JSONBoolean * ) value ) -> toBool ();
        }

        return JSONValue :: toBool ();
    }

    long long JSONWrapper :: toInteger () const
    {
        if ( type == jvt_int )
        {
            assert ( value != nullptr );
            return ( ( const JSONInteger * ) value ) -> toInteger ();
        }

        return JSONValue :: toInteger ();
    }

    JwtString JSONWrapper :: toNumber () const
    {
        switch ( type )
        {
        case jvt_int:
        case jvt_num:
            assert ( value != nullptr );
            return value -> toString ();
        default:
            break;
        }

        return JSONValue :: toNumber ();
    }

    JwtString JSONWrapper :: toString () const
    {
        if ( value == nullptr )
            return "null";

        return value -> toString ();
    }

    JwtString JSONWrapper :: toJSON () const
    {
        if ( value == nullptr )
            return "null";

        return value -> toJSON ();
    }

    JSONValue * JSONWrapper :: clone () const
    {
        if ( value == nullptr )
            return new JSONWrapper ( jvt_null );

        return new JSONWrapper ( type, value -> clone () );
    }

    void JSONWrapper :: invalidate ()
    {
        if ( value != nullptr )
            value -> invalidate ();
    }

    JSONWrapper & JSONWrapper :: operator = ( const JSONWrapper & val )
    {
        JSONPrimitive * copy = val . value ? val . value -> clone () : nullptr;
        delete value;
        value = copy;
        type = val . type;

        return * this;
    }

    JSONWrapper :: JSONWrapper ( const JSONWrapper & val )
        : value ( val . value ? val . value -> clone () : nullptr )
        , type ( val . type )
    {
    }

    JSONWrapper :: JSONWrapper ( JSONValType _type )
        : value ( nullptr )
        , type ( _type )
    {
        assert ( type == jvt_null );
    }

    JSONWrapper :: JSONWrapper ( JSONValType _type, JSONPrimitive * val )
        : value ( val )
        , type ( _type )
    {
    }

    JSONWrapper :: ~ JSONWrapper ()
    {
        delete value;
        value = nullptr;
        type = jvt_null;
    }
}
