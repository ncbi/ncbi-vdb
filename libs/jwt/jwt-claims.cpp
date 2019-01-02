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

#include <jwt/jwt.hpp>
#include <jwt/jws.hpp>
#include "base64-priv.hpp"

#include <iostream>
#include <climits>
#include <cassert>

namespace ncbi
{

    /* JWTClaims
     *
     **********************************************************************************/
    void JWTClaims :: setIssuer ( const StringOrURI & iss )
    {
        validateStringOrURI ( iss );

        JWTLocker locker ( obj_lock );

        setFinalValueOrDelete( "iss", JSONValue :: makeString ( iss ) );
    }

    void JWTClaims :: setSubject ( const StringOrURI & sub )
    {
        validateStringOrURI ( sub );

        JWTLocker locker ( obj_lock );

        setFinalValueOrDelete( "sub", JSONValue :: makeString ( sub ) );
    }

    void JWTClaims :: addAudience ( const StringOrURI & aud )
    {
        validateStringOrURI ( aud );

        JWTLocker locker ( obj_lock );

        // store as an array
        if ( ! claims -> exists ( "aud" ) )
            setFinalValueOrDelete ( "aud", JSONArray :: make () );

        JSONArray & array = claims -> getValue ( "aud " ) . toArray ();
        JSONValue * value = JSONValue :: makeString ( aud );
        try
        {
            array . appendValue ( value );
        }
        catch ( ... )
        {
            delete value;
            throw;
        }
    }

    void JWTClaims :: setDuration ( long long int dur_seconds )
    {
        JWTLocker locker ( obj_lock );
        if ( dur_seconds >= 0 )
        {
            if ( have_duration )
                throw JWTException ( __func__, __LINE__, "Cannot overwrite final member" );

            duration = dur_seconds;
            have_duration = true;
        }
    }

    void JWTClaims :: setNotBefore ( long long int nbf_seconds )
    {
        JWTLocker locker ( obj_lock );
        if ( nbf_seconds >= 0 )
        {
            if ( have_not_before )
                throw JWTException ( __func__, __LINE__, "Cannot overwrite final member" );

            not_before = nbf_seconds;
            have_not_before = true;
        }
    }

    void JWTClaims :: addClaim ( const std :: string & name, JSONValue * value, bool isFinal )
    {
        JWTLocker locker ( obj_lock );

        if ( name . size () == 3 )
        {
            switch ( name [ 0 ] )
            {
            case 'a': // aud
                if ( name . compare ( "aud" ) == 0 )
                {
                    validateStringOrURI ( value );
                    if ( ! claims -> exists ( "aud" ) )
                        setFinalValueOrDelete ( "aud", JSONArray :: make () );
                    JSONArray & array = claims -> getValue ( "aud " ) . toArray ();
                    array . appendValue ( value );
                    return;
                }
                break;
            case 'e': // exp
                if ( name . compare ( "exp" ) == 0 )
                    throw JWTException ( __func__, __LINE__, "Cannot write protected member" );
                break;
            case 'i': // iss, iat
                if ( name . compare ( "iat" ) == 0 )
                    throw JWTException ( __func__, __LINE__, "Cannot write protected member" );
                else if ( name . compare ( "iss" ) == 0 )
                    validateStringOrURI ( value );
                break;
            case 'j': // jti
                if ( name . compare ( "jti" ) == 0 )
                {
                    // TBD - determine how to process this
                }
                break;
            case 'n': // nbf
                if ( name . compare ( "nbf" ) == 0 )
                    throw JWTException ( __func__, __LINE__, "Cannot write protected member" );
                break;
            case 's': // sub
                if ( name . compare ( "sub" ) == 0 )
                    validateStringOrURI ( value );
                break;
            default:
                break;
            }
        }

        if ( isFinal )
            claims -> setFinalValue ( name, value );
        else
            claims -> setValue ( name, value );
    }

    void JWTClaims :: addClaimOrDeleteValue ( const std :: string & name, JSONValue * value, bool isFinal )
    {
        try
        {
            addClaim ( name, value, isFinal );
        }
        catch ( ... )
        {
            delete value;
            throw;
        }
    }

    JSONValue & JWTClaims :: getClaim ( const std :: string & name ) const
    {
        return claims -> getValue ( name );
    }

    void JWTClaims :: validate ( long long cur_time, long long skew )
    {
        JWTLocker locker ( obj_lock );

        // TBD - cur_time must be rational

        // TBD - skew must be rational

        try
        {
            // timestamps
            long long iat, nbf, exp;

            // initialize timestamps to limits
            exp = LLONG_MAX;
            iat = nbf = 0;

            // validate expiration against "cur_time"
            if ( claims -> exists ( "exp" ) )
            {
                exp = claims -> getValue ( "exp" ) . toInteger ();
                if ( ( cur_time - skew ) >= exp )
                    throw JWTException ( __func__, __LINE__, "claims have expired" );
            }
            else if ( require_exp_on_validate )
            {
                throw JWTException ( __func__, __LINE__, "policy mandates existence of 'exp' claim" );
            }

            // validate issue time against "cur_time"
            if ( claims -> exists ( "iat" ) )
            {
                iat = claims -> getValue ( "iat" ) . toInteger ();

                // validate issue time against expiration
                if ( iat > exp )
                    throw JWTException ( __func__, __LINE__, "claims expired before being created" );

                // validate issue time against "cur_time"
                if ( ( cur_time + skew ) < iat )
                    throw JWTException ( __func__, __LINE__, "claims created in the future" );
            }
            else if ( require_iat_on_validate )
            {
                throw JWTException ( __func__, __LINE__, "policy mandates existence of 'exp' claim" );
            }

            // validate first valid time against "cur_time"
            if ( claims -> exists ( "nbf" ) )
            {
                nbf = claims -> getValue ( "nbf" ) . toInteger ();

                // validate valid time against expiration
                if ( nbf > exp )
                    throw JWTException ( __func__, __LINE__, "claims not valid until after expiration" );

                // validate valid time against issue time
                if ( iat > nbf )
                    throw JWTException ( __func__, __LINE__, "claims validity time is before creation" );

                if ( ( cur_time + skew ) < nbf )
                    throw JWTException ( __func__, __LINE__, "claims accessed before valid" );
            }
        }
        catch ( JSONException & x )
        {
            std :: string what ( "Invalid JWTClaims - " );
            what += x . what ();
            throw JWTException ( __func__, __LINE__, what . c_str () );
        }
    }

    std :: string JWTClaims :: toJSON () const
    {
        JWTLocker locker ( obj_lock );

        assert ( claims != nullptr );
        return claims -> toJSON ();
    }

    std :: string JWTClaims :: readableJSON ( unsigned int indent ) const
    {
        JWTLocker locker ( obj_lock );

        assert ( claims != nullptr );
        return claims -> readableJSON ( indent );
    }

    JWTClaims & JWTClaims :: operator = ( const JWTClaims & jwt )
    {
        JWTLocker locker ( obj_lock );

        assert ( jwt . claims != nullptr );
        JSONObject * old_claims = claims;
        try
        {
            claims = static_cast < JSONObject * > ( jwt . claims -> clone () );
        }
        catch ( ... )
        {
            claims = old_claims;
            throw;
        }

        delete old_claims;

        duration = jwt . duration;
        not_before = jwt . not_before;
        have_duration = jwt . have_duration;
        have_not_before = jwt . have_not_before;
        require_iat_on_validate = jwt . require_iat_on_validate;
        require_exp_on_validate = jwt . require_exp_on_validate;

        return *this;
    }

    JWTClaims :: JWTClaims ( const JWTClaims & jwt )
        : claims ( nullptr )
        , duration ( jwt . duration )
        , not_before ( jwt . not_before )
        , have_duration ( jwt . have_duration )
        , have_not_before ( jwt . have_not_before )
        , require_iat_on_validate ( jwt . require_iat_on_validate )
        , require_exp_on_validate ( jwt . require_exp_on_validate )
    {
        assert ( jwt . claims != nullptr );
        claims = static_cast < JSONObject * > ( jwt . claims -> clone () );
    }

    void JWTClaims :: validateStringOrURI ( const std::string &str )
    {
        // TBD
    }

    void JWTClaims :: validateStringOrURI ( JSONValue * value )
    {
        // TBD
    }

    void JWTClaims :: setValueOrDelete ( const std :: string & name, JSONValue * value ) const
    {
        try
        {
            claims -> setValue( name, value );
        }
        catch ( ... )
        {
            delete value;
            throw;
        }
    }

    void JWTClaims :: setFinalValueOrDelete ( const std :: string & name, JSONValue * value ) const
    {
        try
        {
            claims -> setFinalValue( name, value );
        }
        catch ( ... )
        {
            delete value;
            throw;
        }
    }

    JWTClaims :: JWTClaims ( JSONObject * _claims,
            bool _require_iat_on_validate, bool _require_exp_on_validate )
        : claims ( _claims )
        , duration ( -1 )
        , not_before ( -1 )
        , have_duration ( false )
        , have_not_before ( false )
        , require_iat_on_validate ( _require_iat_on_validate )
        , require_exp_on_validate ( _require_exp_on_validate )
    {
        if ( claims == nullptr )
            throw JWTException ( __func__, __LINE__, "INTERNAL ERROR: NULL claims reference" );
    }

    JWTClaims :: ~ JWTClaims ()
    {
        delete claims;
        claims = nullptr;
    }
}

