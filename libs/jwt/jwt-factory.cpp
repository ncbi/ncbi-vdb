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
#include <jwt/jwk.hpp>
#include "base64-priv.hpp"
#include "jwt-vector-impl.hpp"

#include <iostream>
#include <climits>

    static long long jwt_static_cur_time = 0;

    void jwt_setStaticCurrentTime ( long long cur_time )
    {
        jwt_static_cur_time = cur_time;
    }

namespace ncbi
{
    JWTClaims JWTFactory :: make () const
    {
        // make empty container for claims
        JSONObject *obj = JSONObject :: make ();

        // construct the claims object
        // while retaining access to object
        JWTClaims claims ( obj, require_iat_on_validate, require_exp_on_validate );

        // issuer may have been set on factory
        // install into claims if so
        if ( ! iss . empty() )
            claims . setIssuer ( iss );

        // subject may have been set on factory
        // install into claims if so
        if ( ! sub . empty() )
            claims . setSubject ( sub );

        // test for any audience members
        if ( ! aud . empty() )
        {
            // add each audience member
            size_t count = aud . size ();
            for ( size_t i = 0; i < count; ++ i )
                claims . addAudience ( aud [ i ] );

            // lock the array against further modification
            obj -> getValue ( "aud" ) . toArray () . lock ();
        }

        // a positive duration is transferred to claims
        if ( duration > 0 )
            claims . setDuration ( duration );

        // a non-negative "nbf" is transferred
        if ( not_before >= 0 )
            claims . setNotBefore ( not_before );

        // the claims object has been constructed
        return claims;
    }

    JWT JWTFactory :: sign ( const JWTClaims & claims ) const
    {
        JwtString jwt;

        JwtString payload = claimsToPayload ( claims );

        // this would be a case when we encode with "none" algorithm
        if ( jws_fact == nullptr )
            throw JWTException ( __func__, __LINE__, "no JWS factory specified and 'none' algorithm is unsupported" );

        // create a JWS-compatible JOSE header
        JSONObject * hdr = JSONObject :: make ();
        try
        {
            // the main thing we can say about this is that it is a JWT
            JSONValue * typ = JSONValue :: makeString ( "JWT" );
            try
            {
                hdr -> setValue ( "typ", typ );
            }
            catch ( ... )
            {
                delete typ;
                throw;
            }

            // let the JWS factory fill out the remainder of the header
            jwt = jws_fact -> signCompact ( * hdr, payload . data (), payload . size () );
        }
        catch ( ... )
        {
            delete hdr;
            throw;
        }

        delete hdr;
        return jwt;
    }

    JwtString JWTFactory :: claimsToPayload ( const JWTClaims & claims ) const
    {
        JwtString payload;

        JWTLocker claims_lock ( claims . obj_lock );

        // test for duration
        if ( require_exp_on_generate && claims . duration <= 0 )
            throw JWTException ( __func__, __LINE__, "claims must have a specific non-zero duration" );

        JSONObject * json = claims . claims;

        // capture time, which is "iat"
        long long iat = now ();

        // generate nbf
        long long nbf = iat;
        if ( claims . not_before > 0 )
            nbf += claims . not_before;

        // generate exp
        long long exp = LLONG_MAX;
        if ( claims . duration > 0 )
            exp = nbf + claims . duration;

        // set "iat" if required or if either of the other timestamps are there
        if ( require_iat_on_generate || claims . duration > 0 || claims . not_before > 0 )
            claims . setValueOrDelete ( "iat", JSONValue :: makeInteger ( iat ) );
        try
        {
            // set "exp"
            if ( claims . duration > 0 )
                claims . setValueOrDelete ( "exp", JSONValue :: makeInteger ( exp ) );
            try
            {
                // potentially set "nbf"
                if ( claims . not_before >= 0 )
                    claims . setValueOrDelete ( "nbf", JSONValue :: makeInteger ( nbf ) );
                try
                {
                    // at this point, we can convert the claims object to a string
                    payload = json -> toJSON ();
                }
                catch ( ... )
                {
                    json -> removeValue ( "nbf" );
                    throw;
                }

                json -> removeValue ( "nbf" );
            }
            catch ( ... )
            {
                json -> removeValue ( "exp" );
                throw;
            }

            json -> removeValue ( "exp" );
        }
        catch ( ... )
        {
            json -> removeValue ( "iat" );
            throw;
        }

        json -> removeValue ( "iat" );

        return payload;
    }

    JWTClaims JWTFactory :: decode ( const JWT & jwt ) const
    {
        return this -> decode ( jwt, now (), dflt_skew );
    }

    // Decoding follows RFC 7519: section 7.2
    JWTClaims JWTFactory :: decode ( const JWT & jwt, long long cur_time, long long skew ) const
    {
        // RFC 7519: section 1
        // JWTs are always represented using the JWS Compact Serialization or the JWE Compact Serialization.

        /*
          The RFC specifies an unsafe means of decoding the JWT
          predicated on a design flaw that gave rise to the JOSE header.
          The flaw requires reliance upon content of the header to
          validate the JWT itself, including the header. This still
          could have been done in a way that would be relatively safe
          to implement, but instead the JOSE header is represented as...
          ...JSON, meaning that 99.9% of implementations will send the
          header to their off-the-shelf JSON parser as the first step,
          before any validation of any sort, and most JSON parsers are
          not secure enough to be exposed to this.

          The present implementation follows the RFC, and will thus
          permit multiple signing algorithms. A hardened version would
          have a single allowed algorithm and verify the signature BEFORE
          passing any text to a JSON parser.
         */

        // 1. verify that the JWT contains at least one '.'
        size_t pos = 0;
        size_t p = jwt . find_first_of ( '.' );
        if ( p == JwtString :: npos )
            throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: 3 or 5 sections" );

        // 2. split off the JOSE header from the start of "jwt" to the period.
        // this must be a base64url-encoded string representing a JSONObject
        JwtString header = jwt . substr ( pos, p - pos );
        pos = ++ p;

        // 3. run decodeBase64URL() on the JOSE string
        // this should produce raw JSON text, although we won't know until later
        {
            // declare base64 payload outside of try block ( thanks so much, C++! )
            // so that we can properly interpret any exceptions caught by decoding
            JwtString payload;
            try
            {
                // decode the header text
                payload = decodeBase64URLString ( header );
            }
            catch ( JWTException & x )
            {
                // any error in base64URL encoding is an invalid JWT/JOSE object
                JwtString what ( "Invalid JWT - " );
                what += x . what ();
                throw JWTException ( __func__, __LINE__, what . c_str () );
            }
            catch ( ... )
            {
                // any error in base64URL encoding is an invalid JWT/JOSE object
                throw JWTException ( __func__, __LINE__, "Invalid JWT - illegal JOSE base64URL encoding" );
            }

            // replace the header
            header = payload;
        }

        // 4. trust JSON parser enough to parse the raw JSON text of the JOSE header
        // THIS IS EXACTLY THE PART THAT IS DANGEROUS
        // use restricted limits
        JSONValue :: Limits lim;
        JSONObject *jose = nullptr;
        try
        {
            // JOSE doesn't have any recursive structure
            lim . recursion_depth = 1;

            // TBD - set other limits

            // parse header
            jose = JSONObject :: parse ( lim, header );
        }
        catch ( JSONException & x )
        {
            JwtString what ( "Invalid JWT - " );
            what += x . what ();
            throw JWTException ( __func__, __LINE__, what . c_str () );
        }
        catch ( ... )
        {
            // any error in JSON format is an invalid JWT/JOSE object
            throw JWTException ( __func__, __LINE__, "Invalid JWT - malformed JOSE JSON" );
        }

        try
        {
            // 5. count the number of compact segments
            unsigned int period_count = 1; // already have the header

            // "p" points to just after the first period
            p = jwt . find_first_of ( '.', p );

            // retain the end of payload for JWS
            size_t payload_pos = p;
            size_t last_pos = p;
            while ( p != JwtString :: npos )
            {
                // found another period
                if ( ++ period_count > 4 )
                    throw JWTException ( __func__, __LINE__, "Invalid JWT - excessive number of sections." );

                // remember where the last seen period was
                last_pos = p;

                // look for another period
                p = jwt . find_first_of ( '.', p + 1 );
            }

            // 6. detect JWS or JWE
            // RFC 7516 section 9
            // "The JOSE Header for a JWS can also be distinguished from the JOSE
            // Header for a JWE by determining whether an "enc" (encryption
            // algorithm) member exists.  If the "enc" member exists, it is a
            // JWE; otherwise, it is a JWS."
            if ( jose -> exists ( "enc" ) )
            {
                // compact JWE has 5 sections
                if ( period_count != 4 )
                    throw JWTException ( __func__, __LINE__, "Invalid JWT - malformed JWE. Expected 5 sections." );

                // TBD
                throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED - JWE is not supported at this time." );
            }

            // compact JWS has 3 sections
            if ( period_count != 2 )
                throw JWTException ( __func__, __LINE__, "Invalid JWT - malformed JWS. Expected 3 sections." );

            // 7. let the JWSFactory validate the JOSE header and signature - RFC 7515: Section 5.2
            // "Verify that the resulting JOSE Header includes only parameters and values whose syntax and
            // semantics are both understood and supported or that are specified as being ignored when not understood."
            if ( jws_fact == nullptr )
                throw JWTException ( __func__, __LINE__, "no JWS factory specified and 'none' algorithm is unsupported" );

            jws_fact -> validate ( * jose, jwt, last_pos );

            // 8. check for header member "cty"
            // if exists, the payload is a nested JWT and need to repeat the previous steps
            if ( jose -> exists( "cty" ) )
            {
                delete jose;
                throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED - nested JWTs are not supported at this time." );
            }

            // done with "jose"
            delete jose;
            jose = nullptr;

            // 9. decode payload
            {
                JwtString pay;
                try
                {
                    pay = decodeBase64URLString ( jwt . substr ( pos, payload_pos - pos ) );
                }
                catch ( JWTException & x )
                {
                    // any error in base64URL encoding is an invalid JWT object
                    JwtString what ( "Invalid JWT - " );
                    what += x . what ();
                    throw JWTException ( __func__, __LINE__, what . c_str () );
                }
                catch ( ... )
                {
                    // any error in base64URL encoding is an invalid JWT object
                    throw JWTException ( __func__, __LINE__, "Invalid JWT - illegal base64URL encoding" );
                }

                // 10. trust JSON parser enough to parse the raw JSON text of the payload
                // we should have already validated the sender
                lim . recursion_depth = 100; // TBD - determine valid limit
                JSONObject *payload = JSONObject :: parse ( lim, pay );
                try
                {

                    // create claims from JSON payload
                    JWTClaims claims ( payload, require_iat_on_validate, require_exp_on_validate );
                    payload = nullptr;

                    // claim set is already built, but not validated
                    // TBD - validate claims, mark protected claims as final
                    claims . validate ( cur_time, skew );

                    return claims;
                }
                catch ( ... )
                {
                    delete payload;
                    throw;
                }
            }
        }
        catch ( ... )
        {
            delete jose;
            throw;
        }
    }

    void JWTFactory :: setIssuer ( const StringOrURI & iss )
    {
        JWTClaims :: validateStringOrURI ( iss );
        JWTLocker locker ( obj_lock );
        this -> iss = iss;
    }

    void JWTFactory :: setSubject ( const StringOrURI & sub )
    {
        JWTClaims :: validateStringOrURI ( sub );
        JWTLocker locker ( obj_lock );
        this -> sub = sub;
    }

    void JWTFactory :: addAudience ( const StringOrURI & aud )
    {
        JWTClaims :: validateStringOrURI ( aud );
        JWTLocker locker ( obj_lock );
        this -> aud . push_back ( aud );
    }

    void JWTFactory :: setDuration ( long long int dur_seconds )
    {
        JWTLocker locker ( obj_lock );
        if ( dur_seconds >= 0 )
            duration = dur_seconds;
    }

    void JWTFactory :: setNotBefore ( long long int nbf_seconds )
    {
        JWTLocker locker ( obj_lock );
        if ( nbf_seconds >= 0 )
            duration = nbf_seconds;
    }

    void JWTFactory :: setDefaultSkew ( long long dflt )
    {
        JWTLocker locker ( obj_lock );
        if ( dflt >= 0 && dflt < 10 * 60 )
            dflt_skew = dflt;
    }

    void JWTFactory :: requireGenerateExp ( bool required )
    {
        JWTLocker locker ( obj_lock );
        require_exp_on_generate = required;
    }

    void JWTFactory :: requireValidateExp ( bool required )
    {
        JWTLocker locker ( obj_lock );
        require_exp_on_validate = required;
    }

    void JWTFactory :: lock ()
    {
        obj_lock . flag . test_and_set ();
    }

    JWTFactory & JWTFactory :: operator = ( const JWTFactory & jwt_fact )
    {
        JWTLocker locker ( obj_lock );

        jws_fact = jwt_fact . jws_fact;
        iss = jwt_fact . iss;
        sub = jwt_fact . sub;
        duration = jwt_fact . duration;
        not_before = jwt_fact . not_before;

        dflt_skew = jwt_fact . dflt_skew;
        require_iat_on_generate = jwt_fact . require_iat_on_generate;
        require_exp_on_generate = jwt_fact . require_exp_on_generate;
        require_iat_on_validate = jwt_fact . require_iat_on_validate;
        require_exp_on_validate = jwt_fact . require_exp_on_validate;

        aud . clear ();
        size_t i, count = jwt_fact . aud . size ();
        for ( i = 0; i < count; ++ i )
        {
            aud . push_back ( jwt_fact . aud [ i ] );
        }

        return *this;
    }

    JWTFactory :: JWTFactory ( const JWTFactory & jwt_fact )
        : jws_fact ( jwt_fact . jws_fact )
        , iss ( jwt_fact . iss )
        , sub ( jwt_fact . sub )
        , duration ( jwt_fact . duration )
        , not_before ( jwt_fact . not_before )
        , dflt_skew ( jwt_fact . dflt_skew )
        , require_iat_on_generate ( jwt_fact . require_iat_on_generate )
        , require_exp_on_generate ( jwt_fact . require_exp_on_generate )
        , require_iat_on_validate ( jwt_fact . require_iat_on_validate )
        , require_exp_on_validate ( jwt_fact . require_exp_on_validate )
    {
        size_t i, count = jwt_fact . aud . size ();
        for ( i = 0; i < count; ++ i )
        {
            aud . push_back ( jwt_fact . aud [ i ] );
        }
    }

    JWTFactory :: JWTFactory ( const JWSFactory & jws )
        : jws_fact ( & jws )
        , duration ( -1 )
        , not_before ( -1 )
        , dflt_skew ( 0 )
        , require_iat_on_generate ( true )
        , require_exp_on_generate ( true )
        , require_iat_on_validate ( false )
        , require_exp_on_validate ( false )
    {
    }

    JWTFactory :: JWTFactory ()
        : jws_fact ( nullptr )
        , duration ( -1 )
        , not_before ( -1 )
        , dflt_skew ( 0 )
        , require_iat_on_generate ( true )
        , require_exp_on_generate ( true )
        , require_iat_on_validate ( false )
        , require_exp_on_validate ( false )
    {
    }

    JWTFactory :: ~ JWTFactory ()
    {
        jws_fact = nullptr;
        iss . clear ();
        sub . clear ();
        aud . clear ();
        duration = -1;
        not_before = -1;
        dflt_skew = 0;
    }

    JwtString JWTFactory :: newJTI () const
    {
        // TBD - generate from something involving id_seq
        // or some other source of unique numbers
        return "";
    }

    long long int JWTFactory :: now ()
    {
        if ( jwt_static_cur_time > 0 )
            return jwt_static_cur_time;
        return ( long long int ) time ( nullptr );
    }

    std :: atomic < unsigned long long > JWTFactory :: id_seq;
}

