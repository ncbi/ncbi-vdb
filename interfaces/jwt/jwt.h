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

#include <klib/rc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct String;
typedef struct String String;
typedef String StringOrURI;

struct JSONValue;
typedef struct JSONValue JSONValue;

    // class JSONValue
    // {
    // public:

    //     struct Limits
    //     {
    //         Limits ();                        // set default limits

    //         unsigned int json_string_size;    // total size of JSON string
    //         unsigned int recursion_depth;     // parser stack depth
    //         unsigned int numeral_length;      // maximum number of characters in number
    //         unsigned int string_size;         // maximum number of bytes in string
    //         unsigned int array_elem_count;    // maximum number of elements in array
    //         unsigned int object_mbr_count;    // maximum number of members in object
    //     };

    //     // make various value types
    //     static JSONValue * makeNull ();
    //     static JSONValue * makeBool ( bool val );
    //     static JSONValue * makeInteger ( long long int val );
    //     static JSONValue * makeDouble ( long double val, unsigned int precision );
    //     static JSONValue * makeNumber ( const JwtString & val );
    //     static JSONValue * makeString ( const JwtString & val );
JWT_EXTERN JSONValue * CC JSONValue_makeString ( const String * val );

    //     // query value type
    //     virtual bool isNull () const;
    //     virtual bool isBool () const;
    //     virtual bool isInteger () const;        // a number that is an integer
    //     virtual bool isNumber () const;         // is any type of number
    //     virtual bool isString () const;         // is specifically a string
    //     virtual bool isArray () const;
    //     virtual bool isObject () const;

    //     // set value - can change value type
    //     virtual JSONValue & setNull ();
    //     virtual JSONValue & setBool ( bool val );
    //     virtual JSONValue & setInteger ( long long int val );
    //     virtual JSONValue & setDouble ( long double val, unsigned int precision );
    //     virtual JSONValue & setNumber ( const JwtString & val );
    //     virtual JSONValue & setString ( const JwtString & val );

    //     // retrieve a value - will attempt to convert if possible
    //     // throws an exception if conversion is not supported
    //     virtual bool toBool () const;
    //     virtual long long toInteger () const;
    //     virtual JwtString toNumber () const;
    //     virtual JwtString toString () const;
JWT_EXTERN rc_t CC JSONValue_toString ( const JSONValue * self, const String ** str );

    //     virtual JwtString toJSON () const = 0;

    //     // retrieve as structured value - will not convert
    //     // throws an exception if not of the correct container type
    //     virtual JSONArray & toArray ();
    //     virtual const JSONArray & toArray () const;
    //     virtual JSONObject & toObject ();
    //     virtual const JSONObject & toObject () const;

    //     // create a copy
    //     virtual JSONValue * clone () const;

    //     // invalidate and overwrite contents
    //     virtual void invalidate () = 0;

    //     virtual ~JSONValue ();

    // protected:

    //     static JSONValue * makeParsedNumber ( const JwtString & val );
    //     static JSONValue * makeParsedString ( const JwtString & val );

    //     static JSONValue * parse ( const Limits & lim, const JwtString & json, size_t & pos, unsigned int depth );
    //     static Limits default_limits;

    //     JSONValue ();

    //     friend class JSON;
    //     friend struct JSONNumber;
    //     friend struct JSONString;
    // };


struct HMAC_JWKey;
typedef struct HMAC_JWKey HMAC_JWKey;

// make a new symmetric HMAC shared-secret key with randomly generated value
//  "key_bits" size of the key in bits - must be multiple of 8
//  "use" - "sig" for signing, "enc" for encryption
//  "alg" - algorithm id in { "HS256", "HS384", "HS512" }
JWT_EXTERN const HMAC_JWKey * CC HMAC_JWKey_make ( unsigned int key_bits,
    const String * use, const String * alg, const String * kid );
// JWK overrides
// virtual bool isSymmetric () const;
// virtual const HMAC_JWKey * toHMAC () const;
// // get symmetric key "k"
// JwtString getValue () const;
JWT_EXTERN void CC HMAC_JWKey_dtor ( const HMAC_JWKey * );

struct JWK;
typedef struct JWK JWK;

struct JWSFactory;
typedef struct JWSFactory JWSFactory;

// RFC 7515: Line 376
// sign using compact serialization
// signing input is:
//   ASCII(BASE64URL(UTF8(JWS Protected Header)) || '.' || BASE64URL(JWS Payload))
// returned string is BAE64URL encoded
//JWS signCompact ( JSONObject & hdr, const void * payload, size_t bytes ) const;

// check that the JOSE header is completely understood
// validates signature
// or throw exception
// returns authority-name of matching validator
//const JwtString & validate ( const JSONObject & hdr, const JWS & jws, size_t last_period ) const;

// additional verifiers
// duplicates reference when successful
// void addVerifier ( const JwtString & authority_name,
//     const JwtString & alg, const JWK * key );

// copy construction
// JWSFactory & operator = ( const JWSFactory & fact );
// JWSFactory ( const JWSFactory & fact );

// create a standard factory
// duplicates key reference when successful
// JWSFactory ( const JwtString & authority_name, const JwtString & alg, const JWK * key );
JWT_EXTERN JWSFactory * CC JWSFactory_ctor ( const String * authority_name, const String * alg, const JWK * key );

JWT_EXTERN void CC JWSFactory_dtor ( JWSFactory * self );


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
JWT_EXTERN rc_t CC JWTClaims_addClaimOrDeleteValue ( JWTClaims * self, const String * name, JSONValue * value, bool isFinal );

//     // validate claims read from JWT payload
//     // mark protected claims as final
//     // test validity based on time +/- skew amount
//     void validate ( long long cur_time, long long skew = 0 );

//     // serialization
//     JwtString toJSON () const;
JWT_EXTERN rc_t CC JWTClaims_toJSON ( const JWTClaims * self, const String ** json );

//     JwtString readableJSON ( unsigned int indent = 0 ) const;

//     // C++ assignment
//     JWTClaims & operator = ( const JWTClaims & jwt );
//     JWTClaims ( const JWTClaims & jwt );

JWT_EXTERN void CC JWTClaims_dtor ( JWTClaims * self );


struct JWTFactory;
typedef struct JWTFactory JWTFactory;


// class JWTFactory
// {

//     // copy construction
//     JWTFactory & operator = ( const JWTFactory & jwt_fact );
//     JWTFactory ( const JWTFactory & jwt_fact );

//     // create a factory without signing or encrypting capability
JWT_EXTERN JWTFactory * CC JWTFactory_ctor_default ();

//     // create a standard factory with signing capability
JWT_EXTERN JWTFactory * CC JWTFactory_ctor ( const JWSFactory * jws_fact );

/* make a new, more or less empty JWT object */
JWT_EXTERN JWTClaims * CC JWTFactory_make ( JWTFactory * );

//     // create a signed JWT as a compact JWS from the claims set
//     JWT JWTFactory_sign ( const JWTClaims & claims ) const;

//     // decode a JWT against current time with default skew
//     JWTClaims decode ( const JWT & jwt ) const;
//     // decode a JWT against provided time with optional explicit skew
//     JWTClaims decode ( const JWT & jwt, long long cur_time, long long skew = 0 ) const;

//     // registered claim factory parameters
//     void setIssuer ( const StringOrURI & iss );
JWT_EXTERN rc_t CC JWTFactory_setIssuer ( JWTFactory * self, const StringOrURI * iss );

//     void setSubject ( const StringOrURI & sub );
//     void addAudience ( const StringOrURI & aud );
//     void setDuration ( long long int dur_seconds );
JWT_EXTERN rc_t CC JWTFactory_setDuration ( JWTFactory * self, long long int dur_seconds );

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

JWT_EXTERN void CC JWTFactory_dtor ( JWTFactory * self );

#ifdef __cplusplus
}
#endif

#endif /* _hpp_ncbi_oauth_jwt_ */
