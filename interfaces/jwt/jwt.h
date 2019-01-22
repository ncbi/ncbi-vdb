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

#ifndef _h_jwt_
#define _h_jwt_

/*
* C facade for classes in jwt.hpp:
*   ncbi::JWTClaims,
*   ncbi::JWTFactory,
*   ncbi::HMAC_JWKey,
*   ncbi::JWSFactory
*/

#ifndef _h_jwt_extern_
#include <jwt/extern.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct String;
typedef struct String String;

struct HMAC_JWKey;
typedef struct HMAC_JWKey HMAC_JWKey;

// make a new symmetric HMAC shared-secret key with randomly generated value
//  "key_bits" size of the key in bits - must be multiple of 8
//  "use" - "sig" for signing, "enc" for encryption
//  "alg" - algorithm id in { "HS256", "HS384", "HS512" }
JWT_EXTERN const HMAC_JWKey * HMAC_JWKey_make ( unsigned int key_bits,
    const String * use, const String * alg, const String * kid );
// JWK overrides
// virtual bool isSymmetric () const;
// virtual const HMAC_JWKey * toHMAC () const;
// // get symmetric key "k"
// JwtString getValue () const;

struct JWTClaims;
typedef struct JWTClaims JWTClaims;

// // A JSON object that contains the claims conveyed by the JWT
// class JWTClaims
// {
//     void setIssuer ( const StringOrURI & iss );
//     void setSubject ( const StringOrURI & sub );
//     void addAudience ( const StringOrURI & aud );
//     void setDuration ( long long int dur_seconds );
//     void setNotBefore ( long long int nbf_seconds );

//     // claims can be any valid JSONValue
//     void addClaim ( const JwtString & name, JSONValue * value, bool isFinal = false );
//     void addClaimOrDeleteValue ( const JwtString & name, JSONValue * value, bool isFinal = false );
//     JSONValue & getClaim ( const JwtString & name ) const;

//     // validate claims read from JWT payload
//     // mark protected claims as final
//     // test validity based on time +/- skew amount
//     void validate ( long long cur_time, long long skew = 0 );

//     // serialization
//     JwtString toJSON () const;
//     JwtString readableJSON ( unsigned int indent = 0 ) const;

//     // C++ assignment
//     JWTClaims & operator = ( const JWTClaims & jwt );
//     JWTClaims ( const JWTClaims & jwt );
//     ~ JWTClaims ();
// }

// class JWTFactory
// {

//     // copy construction
//     JWTFactory & operator = ( const JWTFactory & jwt_fact );
//     JWTFactory ( const JWTFactory & jwt_fact );

//     // create a factory without signing or encrypting capability
//     JWTFactory ();

//     // create a standard factory with signing capability
//     JWTFactory ( const JWSFactory & jws_fact );


/* make a new, more or less empty JWT object */
JWT_EXTERN JWTClaims * CC JWTFactory_make ();

//     // create a signed JWT as a compact JWS from the claims set
//     JWT JWTFactory_sign ( const JWTClaims & claims ) const;

//     // decode a JWT against current time with default skew
//     JWTClaims decode ( const JWT & jwt ) const;
//     // decode a JWT against provided time with optional explicit skew
//     JWTClaims decode ( const JWT & jwt, long long cur_time, long long skew = 0 ) const;

//     // registered claim factory parameters
//     void setIssuer ( const StringOrURI & iss );
//     void setSubject ( const StringOrURI & sub );
//     void addAudience ( const StringOrURI & aud );
//     void setDuration ( long long int dur_seconds );
//     void setNotBefore ( long long int nbf_seconds );

//     // skew access
//     long long getDefaultSkew () const
//     { return dflt_skew; }
//     void setDefaultSkew ( long long dflt );

//     // behavior of "exp" claim
//     void requireGenerateExp ( bool required );
//     void requireValidateExp ( bool required );

//     // prevent further modifications
//     void lock ();

//     ~ JWTFactory ();
// }

#ifdef __cplusplus
}
#endif

#endif /* _hpp_ncbi_oauth_jwt_ */
