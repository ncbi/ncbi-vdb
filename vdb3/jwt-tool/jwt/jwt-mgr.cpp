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
 *  Author: Kurt Rodarmer
 *
 * ===========================================================================
 *
 */

#include <ncbi/jwt.hpp>
#include <ncbi/jws.hpp>
#include <ncbi/jwe.hpp>

#include <ncbi/secure/base64.hpp>

#include <climits>
#include <cassert>

#include <sys/time.h>
#include <string.h>

namespace ncbi
{

    const unsigned int max_jose_recursion_depth = 20;
    const unsigned int max_claims_recursion_depth = 50;

    // hard-coded policy defaults
    const bool dflt_iss_required = false;
    const bool dflt_sub_required = false;
    const bool dflt_aud_required = false;
    const bool dflt_exp_required = true;
    const bool dflt_nbf_required = false;
    const bool dflt_iat_required = false;
    const bool dflt_jti_required = false;

    const bool dflt_exp_gen = dflt_exp_required | true;
    const bool dflt_nbf_gen = dflt_nbf_required | false;
    const bool dflt_iat_gen = dflt_iat_required | false;
    const bool dflt_jti_gen = dflt_jti_required | true;

#if JWT_TESTING
    // allow raw inspection
    const bool dflt_sig_val_required = false;
    const bool dflt_nested_sig_val_required = false;
    // always check that what we generate will pass our own validation
    const bool dflt_pre_serial_verify = true;
    // allow testing code to create expired tickets
    const bool dflt_zero_dur_allowed = true;
#else
    // disallow raw inspection
    const bool dflt_sig_val_required = true;
    const bool dflt_nested_sig_val_required = false;
    // this default is safe and can be turned off
    const bool dflt_pre_serial_verify = true;
    // don't allow production code to create expired tickets
    const bool dflt_zero_dur_allowed = false;
#endif

    // this may be a VERY long time...
    const long long int skew_max = 2 * 60; // 2 minutes

    JWTMgr :: Policy JWTMgr :: dflt;

    /*------------------------------------------
     * makeClaimSetBuilder
     *  create an empty object
     */
    JWTClaimSetBuilderRef JWTMgr :: makeClaimSetBuilder ()
    {
        // the claims are not supposed to be related to the header
        // but there are edge cases to be dealt with...
        JSONObjectRef jose = JSON :: makeObject ();
        JSONObjectRef claims = JSON :: makeObject ();

        // pair the two together
        return JWTClaimSetBuilderRef ( new JWTClaimSetBuilder ( jose, claims ) );
    }

    /*------------------------------------------
     * sign
     *  sign a claim set
     */
    JWT JWTMgr :: sign ( const JWK & key, const JWTClaimSet & cs )
    {
        // 1. STAT attempt to sign JWT claims
        // TBD

        // 2. clone the JOSE header JSON object and claims JSON object
        // since "cs" is immutable, this is thread-safe
        JSONObjectRef jose = cs . ucs . jose -> cloneObject ();
        JSONObjectRef claims = cs . ucs . claims -> cloneObject ();

        // 3. finalize claims
        // 3a. execute private JWT Finalize Claims
        long long int iat = now ();
        finalizeClaims ( * claims, cs . duration, iat );

        // 3b. auto-generate id if 'jti' absent but required
        if ( dflt . jti_gen && ! cs . hasID () )
        {
            String jti = makeID ();
            claims -> addValue ( "jti", JSON :: makeString ( jti ) );
        }

        // 4. REQUIRE that the claims pass validation if policy specifies it
        if ( dflt . pre_serial_verify )
            verifyClaims ( * claims, iat, 1 );

        // 5. serialize the claims into JSON text
        String payload = claims -> toJSON ();

        // 6. the JSON text will be the binary payload
        // RFC7519 section 7.1 step 2.

        // 7. execute private JWS Compact Signing Process 2
        JWT jwt = JWSMgr :: signCompact ( key, * jose,
            payload . data (), payload . size () );

        // 8. if exists 'jti' claim
        if ( claims -> exists ( "jti" ) )
        {
            // 8a. REQUIRE 'jti' to be unique
            // 8b. LOG JWT into database
            // TBD
        }

        // 9. ACCEPT
        return jwt;
    }

#if ALLOW_NESTED_JWT
    /*------------------------------------------
     * sign
     *  sign a nested JWT
     */
    JWT JWTMgr :: sign ( const JWK & key, const JWT & jwt )
    {
        // 1. STAT attempt to sign nested JWT
        // TBD

        // 2. create a new JOSE header JSON object
        JSONObjectRef jose = JSON :: makeObject ();

        // 3. add 'cty' header as 'JWT'
        jose -> addValue ( "cty", JSON :: makeString ( "JWT" ) );

        // 4. the jwt text will be the binary payload
        // RFC7519 section 7.1 step 5.

        // 5. execute private JWS Compact Signing Process 2
        JWT nested_jwt = JWSMgr :: signCompact ( key, * jose,
            jwt . data (), jwt . size () );

        // 6. NOTE - this presents obscure logging/audit issues
        // this is because there is no 'kid' for nested token

        // 7. ACCEPT
        return nested_jwt;
    }
#endif

    /*------------------------------------------
     * inspect
     *  unpack but do not verify claims
     */
    UnverifiedJWTClaimsRef JWTMgr :: inspect ( const JWKSet & keys, const JWT & outer_jwt )
    {
        JSON :: Limits lim;
        JWT jwt ( outer_jwt );
        std :: vector < JWKRef > vkeys;

        // capture policy around signature verification
        bool verify_signature = ! keys . isEmpty ();
        if ( ! verify_signature && dflt . sig_val_required )
        {
            throw InternalPolicyViolation (
                XP ( XLOC )
                << "unverified inspection is prohibited by policy"
                );
        }

        while ( 1 )
        {
            // 1. STAT attempt to examine JWT

            // 2. REJECT empty JWT
            if ( jwt . isEmpty () )
            {
                // 2a. upstream, this exception should be associated with source
                throw MalformedJWT ( XP ( XLOC ) << "empty JWT" );
            }

            // 3. detect class of first character:
            UTF32 ch = jwt [ 0 ];

            // 3a. non-ASCII
            if ( ch >= 128 )
                throw MalformedJWT ( XP ( XLOC ) << "badly formed JWT" );

            // 3b. base64url => required for and only legal in compact

            // 4. REJECT if first character is not base64url
            if ( ! isalnum ( ch ) && ch != '-' && ch != '_' )
            {
                // 4a. upstream, this exception should be associated with source
                throw MalformedJWT ( XP ( XLOC ) << "corrupt base64 encoding" );
            }

            // 5. REQUIRE that the putative JWS string be 100% ASCII
            if ( ! jwt . isAscii () )
            {
                throw MalformedJWT (
                    XP ( XLOC )
                    << "JWT contains non-ASCII characters"
                    );
            }

            // 6. count and map the location of the dots in the string
            count_t dots [ 5 ];
            count_t num_dots = JWSMgr :: locateDelimiters ( dots, jwt );

            // 7. REJECT any count of dots not in { 2, 4 }
            if ( num_dots != 2 && num_dots != 4 )
            {
                throw MalformedJWT (
                    XP ( XLOC )
                    << "JWT contains "
                    << ( num_dots + 1 )
                    << " '.' delimited sections - expected 3 or 5"
                    );
            }

            // 8. create output parameters for JOSE and payload
            JSONObjectRef jose;
            Payload payload;

            // 9. extract JOSE header and payload
            JWKRef key;
            if ( num_dots == 4 )
            {
                // 9a. using private JWE Compact Decoding Process if dots = 4
                key = JWEMgr :: decryptCompact ( jose, payload, keys, jwt, dots );
            }
            else
            {
                assert ( num_dots == 2 );

                // 9b. using private JWS Compact Decoding Process 2 if dots = 2 and keys
                if ( verify_signature || ! keys . isEmpty () )
                {
                    try
                    {
                        // at least TRY to verify signature
                        // in order to extract a key
                        key = JWSMgr :: extractCompact ( jose, payload, keys, jwt, dots );
                    }
                    catch ( JWSNoKeyAvailable & x )
                    {
                        // only an error if signature verification was required
                        if ( verify_signature )
                            throw;
                    }
                }

                if ( key == nullptr )
                {
                    // 9c. manually to inflate JOSE and extract payload if dots = 2 and ! keys
                    String hdr_base64 = jwt . subString ( 0, dots [ 0 ] );
                    String hdr_json = Base64 :: urlDecodeText ( hdr_base64, Base64 :: strict_charset );
                    lim . recursion_depth = max_jose_recursion_depth;
                    jose = JSON :: parseObject ( lim, hdr_json );

                    String pay_base64 = jwt . subString
                        ( dots [ 0 ] + 1, dots [ 1 ] - dots [ 0 ] - 1 );
                    if ( ! pay_base64 . isEmpty () )
                    {
                        UTF32 ch = pay_base64 [ 0 ];
                        if ( ch < 128 && ( isalnum ( ch ) || ch == '-' || ch == '_' ) )
                            payload = Base64 :: urlDecode ( pay_base64, Base64 :: strict_charset );
                        else
                        {
                            payload . increaseCapacity ( pay_base64 . size () );
                            memmove ( payload . data (), pay_base64 . data (), pay_base64 . size () );
                            payload . setSize ( pay_base64 . size () );
                        }
                    }

                    // there is no key
                    key = JWKMgr :: getInvalidKey ();
                }
            }

            // we assume the payload is text
            String pay_text ( ( const UTF8 * ) payload . data (), payload . size () );

            // 10. goto #1 if header 'cty' = 'JWT' using payload as jwt
            if ( jose -> exists ( "cty" ) && jose -> getValue ( "cty" ) . toString () == "JWT" )
            {
                // take the nested JWT as the one we need to work on
                jwt = pay_text;

                // record the nested keys in order
                vkeys . push_back ( key );

                // influence the signature verification behavior for inner tokens
                verify_signature = dflt . nested_sig_val_required;
                continue;
            }

            // 11. parse the JSON payload text into a JSON Object
            lim . recursion_depth = max_claims_recursion_depth;
            JSONObjectRef claims = JSON :: parseObject ( lim, pay_text );

            // 12. create an unverified claims object with keys, JOSE header, and claims
            UnverifiedJWTClaimsRef ucs = new UnverifiedJWTClaims ( vkeys, jose, claims );

            // 13. ACCEPT
            return ucs;
        }
    }

    /*------------------------------------------
     * DecodeParams
     *  package together caller parameters
     */
    JWTMgr :: DecodeParams :: DecodeParams ()
        : cur_time ( -1 )
        , skew_secs ( dflt . skew_seconds )
    {
    }

    JWTMgr :: DecodeParams :: DecodeParams ( long long int _cur_time )
        : cur_time ( _cur_time )
        , skew_secs ( dflt . skew_seconds )
    {
    }

    /*------------------------------------------
     * decode
     *  extract and verify claims using default params
     */
    JWTClaimSetRef JWTMgr :: decode ( const JWKSet & keys, const JWT & jwt )
    {
        // create default params
        DecodeParams params;

        // run normal decode
        return decode ( keys, jwt, params );
    }

    /*------------------------------------------
     * decode
     *  extract and verify claims using custom params
     */
    JWTClaimSetRef JWTMgr :: decode ( const JWKSet & keys,
        const JWT & jwt, const DecodeParams & params )
    {
        // 1. STAT attempt to decode JWT
        // TBD

        // 2. examine JWT with keys
        UnverifiedJWTClaimsRef ucs = inspect ( keys, jwt );

        // 3. continue with JWT Validate Claims
        long long int cur_time = params . cur_time;
        long long int skew_secs = params . skew_secs;

#if JWT_TESTING
        // detect an unspecified or otherwise wildly illegal time
        if ( cur_time < 0 )
            cur_time = now ();
#else
        // allowing the time to be set from outside
        // violates validation. Not exclusively, i.e.
        // there are other ways to violate it, but
        // they are outside of the enforcement of this API
        cur_time = now ();
#endif
        // skew must also be reasonable
        if ( skew_secs < 0 || skew_secs > skew_max )
        {
            throw JWTInvalidSkewAjust (
                XP ( XLOC )
                << "skew adjustment out of range: "
                << skew_secs
                );
        }

        // validate the claims
        verifyClaims ( * ucs -> claims, cur_time, skew_secs );

        // it is done.
        return JWTClaimSetRef ( new JWTClaimSet ( * ucs ) );
    }


#if ALLOW_NESTED_JWT
    /*------------------------------------------
     * unwrap
     *  attempt to remove a layer of nesting
     */
    JWKRef JWTMgr :: unwrap ( const JWKSet & keys, JWT & jwt )
    {
        // 1. STAT attempt to unwrap JWT

        // 2. REJECT empty JWT
        if ( jwt . isEmpty () )
        {
            // 2a. upstream, this exception should be associated with source
            throw MalformedJWT ( XP ( XLOC ) << "empty JWT" );
        }

        // 3. detect class of first character:
        UTF32 ch = jwt [ 0 ];

        // 3a. non-ASCII
        if ( ch >= 128 )
            throw MalformedJWT ( XP ( XLOC ) << "badly formed JWT" );

        // 3b. base64url => required for and only legal in compact

        // 4. REJECT if first character is not base64url
        if ( ! isalnum ( ch ) && ch != '-' && ch != '_' )
        {
            // 4a. upstream, this exception should be associated with source
            throw MalformedJWT ( XP ( XLOC ) << "corrupt base64 encoding" );
        }

        // 5. REQUIRE that the putative JWS string be 100% ASCII
        if ( ! jwt . isAscii () )
        {
            throw MalformedJWT (
                XP ( XLOC )
                << "JWT contains non-ASCII characters"
                );
        }

        // 6. count and map the location of the dots in the string
        count_t dots [ 5 ];
        count_t num_dots = locateDelimiters ( dots, jwt );

        // 7. REJECT any count of dots not in { 2, 4 }
        if ( num_dots != 2 && num_dots != 4 )
        {
            throw MalformedJWT (
                XP ( XLOC )
                << "JWT contains "
                << ( num_dots + 1 )
                << " '.' delimited sections - expected 3 or 5"
                );
        }

        // 8. create a JSON Object to act as JOSE header and a payload
        JSONObjectRef jose;
        Payload payload;

        // 9. extract JOSE header and payload
        JWKRef key;
        if ( num_dots == 4 )
        {
            // 9a. using private JWE Compact Decoding Process if dots = 4
            key = JWEMgr :: decryptCompact ( jose, payload, keys, jwt, dots );
        }
        else
        {
            // 9b. using private JWS Compact Decoding Process 2 if dots = 2
            assert ( num_dots == 2 );
            key = JWSMgr :: extractCompact ( jose, payload, keys, jwt, dots );
        }

        // we assume the payload is text
        String pay_text ( ( const UTF8 * ) payload . data (), payload . size () );

        // 10. goto #1 if header 'cty' = 'JWT' using payload as jwt
        if ( jose -> exists ( "cty" ) && jose -> getValue ( "cty" ) . toString () == "JWT" )
        {
            jwt = pay_text;
            return key;
        }

        return JWKMgr :: getInvalidKey ();
    }
#endif

    /*------------------------------------------
     * getPolicy
     *  return current process-global policy
     */
    const JWTMgr :: Policy & JWTMgr :: getPolicy ()
    {
        return dflt;
    }

    /*------------------------------------------
     * setPolicy
     *  update current process-global policy
     */
    void JWTMgr :: setPolicy ( const Policy & p )
    {
        // catch this error here rather than in the assignment operator
        if ( p . skew_seconds < 0 || p . skew_seconds > skew_max )
        {
            throw JWTInvalidSkewAjust (
                XP ( XLOC )
                << "skew adjustment out of range: "
                << p . skew_seconds
                << " ( limits 0.."
                << skew_max
                << " )"
                );
        }

        dflt = p;
    }

#if JWT_TESTING
    /*------------------------------------------
     * jwt_setStaticCurrentTime
     *  allow testing code to set static clock
     */
    static long long jwt_static_cur_time;
    void jwt_setStaticCurrentTime ( long long cur_time )
    {
        jwt_static_cur_time = cur_time;
    }
#endif
    
    /*------------------------------------------
     * now
     *  return clock time
     */
    long long int JWTMgr :: now ()
    {
#if JWT_TESTING
        if ( jwt_static_cur_time > 0 )
            return jwt_static_cur_time;
#endif
        return ( long long int ) time ( nullptr );
    }

    /*------------------------------------------
     * makeID
     *  make a token id ( 'jti' )
     */
    String JWTMgr :: makeID ()
    {
        // just borrow JWK ids for now
        return JWKMgr :: makeID ();
    }

    /*------------------------------------------
     * finalizeClaims
     *  add in needed time-related claims
     */
    void JWTMgr :: finalizeClaims ( INOUT JSONObject & claims,
        long long int duration, long long int iat )
    {
        // we allow the user to have set 'nbf' and 'exp'
        // although the most common case is for them to
        // set "duration" rather than the 'exp' claim.
        // ( the offsets by 1 allow for safe arithmetic )
        bool nbf_exists = claims . exists ( "nbf" );
        bool exp_exists = claims . exists ( "exp" );
        long long int nbf = nbf_exists ?
            claims . getValue ( "nbf" ) . toInteger () : 0 + 1;
        long long int exp = exp_exists ?
            claims . getValue ( "exp" ) . toInteger () : LLONG_MAX - 1;

        // 'iat' is supplied by trusted caller
        assert ( iat >  1551632500LL ); // March 3, 2019
        assert ( iat < 17319632500LL ); // 500 years from now

        // a non-existing or expired 'nbf' is converted to 'iat'
        bool reset_nbf = false;
        if ( nbf < iat )
        {
            // bring up to current time
            nbf = iat;

            // the decision to set or reset entry
            // is based upon whether 'nbf' is required
            // or whether it was already there but bad
            reset_nbf = nbf_exists | dflt . nbf_gen;
        }

        // ENFORCE POLICY
        // with regard to generation

        // detect missing mandatory 'exp'
        // this makes it impossible to GENERATE a token
        if ( dflt . exp_gen && ! exp_exists && duration < 0 )
        {
            throw JWTMissingTokenDuration (
                XP ( XLOC )
                << "JWT expiration is mandatory and duration has not been set"
                );
        }

        // calculate 'exp' if not already given
        if ( ! exp_exists )
            exp = nbf + duration;

        // ENFORCE POLICY
        // with regard to validation

        // catch error conditions if NOT testing for them later anyway
        if ( ! dflt . pre_serial_verify )
        {
            // detect a token that has already expired
            // meaning 'exp' is in the past or it is
            // expiring right now and that is not allowed
            assert ( exp <= LLONG_MAX - 1 );
            if ( nbf >= exp + ( long long int ) dflt . zero_dur_allowed )
            {
                throw JWTExpired (
                    XP ( XLOC )
                    << "JWT "
                    << ( ( exp < nbf ) ? "expired even before" : "expires immediately upon" )
                    << " being created"
                    );
            }
        }

        // now finish the claims

        // set 'iat' always by policy, or if it was set by user somehow
        // ( although this shouldn't be possible - just defensive )
        if ( dflt . iat_gen || claims . exists ( "iat" ) )
            claims . setValue ( "iat", JSON :: makeInteger ( iat ) );

        // set 'nbf' if it was absent but required by policy, or
        // if it was present but needed update ( see above )
        if ( reset_nbf )
            claims . setValue ( "nbf", JSON :: makeInteger ( nbf ) );
        if ( dflt . exp_gen && ! exp_exists )
            claims . setValue ( "exp", JSON :: makeInteger ( exp ) );

        // we DON'T set 'jti' here because this function is used
        // during arbitrary JSON serialization of the claims, in
        // addition to creation of a JWT. Only generate/burn a ticket
        // id during JWT creation.
    }

    /*------------------------------------------
     * verifyClaims
     *  enforce existence of mandatory claims and
     *  test the claims that exist for consistency
     */
    void JWTMgr :: verifyClaims ( const JSONObject & payload,
        long long int cur_time, long long int skew_secs )
    {
        // march through the registered claims
        // rely upon JSON to throw an exception if required claim not present

        // RFC7519 Section 4.1.1 - "iss" OPTIONAL StringOrURI
        if ( dflt . iss_required || payload . exists ( "iss" ) )
            URIMgr :: validateStringOrURI ( payload . getValue ( "iss" ) . toString () );

        // RFC7519 Section 4.1.2 - "sub" OPTIONAL StringOrURI
        if ( dflt . sub_required || payload . exists ( "sub" ) )
            URIMgr :: validateStringOrURI ( payload . getValue ( "sub" ) . toString () );

        // RFC7519 Section 4.1.3 - "aud" OPTIONAL StringOrURI
        if ( dflt . aud_required || payload . exists ( "aud" ) )
        {
            // the "aud" claim can be a string or an array
            const JSONValue & val = payload . getValue ( "aud" );

            // if it's a string, check that it is a StringOrURI
            if ( ! val . isArray () )
                URIMgr :: validateStringOrURI ( val . toString () );
            else
            {
                // if it's an array, check each element for StringOrURI
                const JSONArray & a = val . toArray ();
                unsigned long i, count = a . count ();
                for ( i = 0; i < count; ++ i )
                    URIMgr :: validateStringOrURI ( a [ i ] . toString () );
            }
        }

        // all tokens, by default, never expire
        long long int exp = LLONG_MAX;

        // RFC7519 Section 4.1.4 - "exp" OPTIONAL Integer
        if ( dflt . exp_required || payload . exists ( "exp" ) )
            exp = payload . getValue ( "exp" ) . toInteger ();

        // all tokens have, by default, been valid since the beginning of time
        long long int nbf = 0;

        // RFC7519 Section 4.1.5 - "nbf" OPTIONAL Integer
        if ( dflt . nbf_required || payload . exists ( "nbf" ) )
            nbf = payload . getValue ( "nbf" ) . toInteger ();

        // the time issued is optional and can substitute for 'nbf'
        long long int iat = 0;

        // RFC7519 Section 4.1.6 - "iat" OPTIONAL Integer
        if ( dflt . iat_required || payload . exists ( "iat" ) )
            iat = payload . getValue ( "iat" ) . toInteger ();

        // ensure that the basic relationships between timestamps hold
        if ( iat > nbf || nbf >= exp )
        {
            throw JWTInconsistentTimestamps (
                XP ( XLOC )
                << "token has self-inconsistent timestamps: "
                << "the relationship "
                << iat
                << " <= "
                << nbf
                << " < "
                << exp
                << " does not hold."
                );
        }

        // RFC7519 Section 4.1.7 - "jti" OPTIONAL string
        if ( dflt . jti_required || payload . exists ( "jti" ) )
        {
            String jti = payload . getValue ( "jti" ) . toString ();
            if ( jti . isEmpty () )
            {
                throw JWTInvalidClaims (
                    XP ( XLOC )
                    << "token has an empty 'jti' claim"
                    );
            }
        }

        // now test that the times make sense against "cur_time"
        // if compensating for clock-skew, factor that into tests

        // token CANNOT have been issued in the future
        if ( cur_time + skew_secs < iat )
        {
            throw JWTBadTimestamp (
                XP ( XLOC )
                << "token claims to have been created at "
                << iat
                << " ( "
                << ( iat - cur_time )
                << " seconds in the future )"
                );
        }

        // token MUST NOT have expired
        if ( cur_time - skew_secs >= exp )
        {
            throw JWTExpired (
                XP ( XLOC )
                << "token expired at "
                << exp
                << " ( "
                << ( cur_time - exp )
                << " seconds ago )"
                );
        }

        // token MUST be ready to use
        if ( cur_time + skew_secs < nbf )
        {
            throw JWTNotYetValid (
                XP ( XLOC )
                << "token will not be valid until "
                << nbf
                << " ( "
                << ( nbf - cur_time )
                << " seconds from now )"
                );
        }

    }

    /*------------------------------------------
     * Policy
     *  create an object with default policies
     */
    JWTMgr :: Policy :: Policy ()
        : skew_seconds ( 0 )
        , sig_val_required ( dflt_sig_val_required )
        , nested_sig_val_required ( dflt_nested_sig_val_required )
        , iss_required ( dflt_iss_required )
        , sub_required ( dflt_sub_required )
        , aud_required ( dflt_aud_required )
        , exp_required ( dflt_exp_required )
        , nbf_required ( dflt_nbf_required )
        , iat_required ( dflt_iat_required )
        , jti_required ( dflt_jti_required )
        , exp_gen ( dflt_exp_gen )
        , nbf_gen ( dflt_nbf_gen )
        , iat_gen ( dflt_iat_gen )
        , jti_gen ( dflt_jti_gen )
        , pre_serial_verify ( dflt_pre_serial_verify )
        , zero_dur_allowed ( dflt_zero_dur_allowed )
    {
    }

    /*------------------------------------------
     * copy assignment
     *  copy other policies, potentially correct
     */
    JWTMgr :: Policy & JWTMgr :: Policy :: operator = ( const Policy & p )
    {
        skew_seconds = p . skew_seconds;

        sig_val_required = p. sig_val_required;
        nested_sig_val_required = p . nested_sig_val_required;

        iss_required = p . iss_required;
        sub_required = p . sub_required;
        aud_required = p . aud_required;
        exp_required = p . exp_required;
        nbf_required = p . nbf_required;
        iat_required = p . iat_required;
        jti_required = p . jti_required;

        exp_gen = p . exp_required | p . exp_gen;
        nbf_gen = p . nbf_required | p . nbf_gen;
        iat_gen = p . iat_required | p . iat_gen;
        jti_gen = p . jti_required | p . jti_gen;

        pre_serial_verify = p . pre_serial_verify;
        zero_dur_allowed = p . zero_dur_allowed;

        return * this;
    }

    /*------------------------------------------
     * copy construction
     *  copy other policies, potentially correct
     */
    JWTMgr :: Policy :: Policy ( const Policy & p )
        : skew_seconds ( p . skew_seconds )
        , sig_val_required ( p. sig_val_required )
        , nested_sig_val_required ( p . nested_sig_val_required )
        , iss_required ( p . iss_required )
        , sub_required ( p . sub_required )
        , aud_required ( p . aud_required )
        , exp_required ( p . exp_required )
        , nbf_required ( p . nbf_required )
        , iat_required ( p . iat_required )
        , jti_required ( p . jti_required )
        , exp_gen ( p . exp_required )
        , nbf_gen ( p . nbf_required )
        , iat_gen ( p . iat_required )
        , jti_gen ( p . jti_required )
        , pre_serial_verify ( p . pre_serial_verify )
        , zero_dur_allowed ( p . zero_dur_allowed )
    {
        exp_gen |= p . exp_gen;
        nbf_gen |= p . nbf_gen;
        iat_gen |= p . iat_gen;
        jti_gen |= p . jti_gen;
    }

    JWTMgr :: Policy :: ~ Policy ()
    {
    }

}
