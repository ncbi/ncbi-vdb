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

#ifndef _hpp_ncbi_oauth_jwt_
#define _hpp_ncbi_oauth_jwt_

#ifndef _hpp_ncbi_oauth_json_
#include <jwt/json.hpp>
#endif

#include <atomic>

#include "jwt-string.hpp"

namespace ncbi
{
    class JWSFactory;
    class JWTFactory;
//    class JWTFixture_BasicConstruction;

    // a JSON Web Token - RFC 7519: Line 233
    // A string representing a set of claims as a JSON object
    typedef JwtString JWT;

    // RFC 7519: Line 273
    // if the string contains a ':', then it MUST be a URI [RFC3986]
    typedef JwtString StringOrURI;

    /* JWTException
     **********************************************************************************/
    class JWTException : public std :: logic_error
    {
    public:

        virtual const char * what () const
        throw ();

        explicit JWTException ( const char * function, unsigned int line, const char * message );
        virtual ~JWTException ()
        throw ();

    private:

        JwtString msg;
        const char * fl_msg;
    };

    // JWTLock
    struct JWTLock
    {

        JWTLock ();
        ~ JWTLock ();

        mutable std :: atomic_flag flag;
    };

    class JWTLocker
    {
    public:

        JWTLocker ( const JWTLock & lock );
        ~ JWTLocker ();

    private:

        const JWTLock & lock;
    };

    // A JSON object that contains the claims conveyed by the JWT
    class JWTClaims
    {
    public:

        // registered claims
        void setIssuer ( const StringOrURI & iss );
        void setSubject ( const StringOrURI & sub );
        void addAudience ( const StringOrURI & aud );
        void setDuration ( long long int dur_seconds );
        void setNotBefore ( long long int nbf_seconds );

        // claims can be any valid JSONValue
        void addClaim ( const JwtString & name, JSONValue * value, bool isFinal = false );
        void addClaimOrDeleteValue ( const JwtString & name, JSONValue * value, bool isFinal = false );
        JSONValue & getClaim ( const JwtString & name ) const;

        // validate claims read from JWT payload
        // mark protected claims as final
        // test validity based on time +/- skew amount
        void validate ( long long cur_time, long long skew = 0 );

        // serialization
        JwtString toJSON () const;
        JwtString readableJSON ( unsigned int indent = 0 ) const;

        // C++ assignment
        JWTClaims & operator = ( const JWTClaims & jwt );
        JWTClaims ( const JWTClaims & jwt );
        ~ JWTClaims ();

    private:

        // any JwtString parameter typed as StringOrURI MUST be validated
        // throws an exception for an invalid string
        static void validateStringOrURI ( const JwtString & str );
        static void validateStringOrURI ( JSONValue * value );

        // store a newly allocated value under claim name
        // delete the value if there are any problems
        void setValueOrDelete ( const JwtString & name, JSONValue * value ) const;
        void setFinalValueOrDelete ( const JwtString & name, JSONValue * value ) const;

        JWTClaims ();
        JWTClaims ( JSONObject * claims, bool require_iat_on_validate, bool require_exp_on_validate );

        JSONObject * claims;
        long long duration;
        long long not_before;
        JWTLock obj_lock;
        bool have_duration;
        bool have_not_before;
        bool require_iat_on_validate;
        bool require_exp_on_validate;

        friend class JWTFactory;
        friend class JWTClaimsLock;
//        friend class JWTFixture_BasicConstruction;
    };

    class JWTFactory
    {
    public:

        // make a new, more or less empty JWT object
        JWTClaims make () const;

        // create a signed JWT as a compact JWS from the claims set
        JWT sign ( const JWTClaims & claims ) const;

        // decode a JWT against current time with default skew
        JWTClaims decode ( const JWT & jwt ) const;
        // decode a JWT against provided time with optional explicit skew
        JWTClaims decode ( const JWT & jwt, long long cur_time, long long skew = 0 ) const;

        // registered claim factory parameters
        void setIssuer ( const StringOrURI & iss );
        void setSubject ( const StringOrURI & sub );
        void addAudience ( const StringOrURI & aud );
        void setDuration ( long long int dur_seconds );
        void setNotBefore ( long long int nbf_seconds );

        // skew access
        long long getDefaultSkew () const
        { return dflt_skew; }
        void setDefaultSkew ( long long dflt );

        // behavior of "exp" claim
        void requireGenerateExp ( bool required );
        void requireValidateExp ( bool required );

        // prevent further modifications
        void lock ();

        // copy construction
        JWTFactory & operator = ( const JWTFactory & jwt_fact );
        JWTFactory ( const JWTFactory & jwt_fact );

        // create a factory without signing or encrypting capability
        JWTFactory ();

        // create a standard factory with signing capability
        JWTFactory ( const JWSFactory & jws_fact );

        ~ JWTFactory ();

    private:

        // make a new identifier
        JwtString newJTI () const;

        // convert claims to JSON text
        JwtString claimsToPayload ( const JWTClaims & claims ) const;

        // return timestamp in seconds since epoch
        static long long int now ();

        // we obtain this as a reference
        // but hold it as a pointer to allow for
        // copies and NULL
        const JWSFactory * jws_fact;

        // factory claims
        JwtString iss;
        JwtString sub;
        std :: vector < JwtString > aud;
        long long duration;
        long long not_before;

        // properties for validation
        long long dflt_skew;
        JWTLock obj_lock;
        bool require_iat_on_generate;
        bool require_exp_on_generate;
        bool require_iat_on_validate;
        bool require_exp_on_validate;

        static std :: atomic < unsigned long long > id_seq;
    };
}

#endif /* _hpp_ncbi_oauth_jwt_ */
