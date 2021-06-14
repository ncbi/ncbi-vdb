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

#pragma once

#include <ncbi/secure/ref.hpp>
#include <ncbi/secure/string.hpp>
#include <ncbi/uri.hpp>
#include <ncbi/jwk.hpp>

#include <set>
#include <atomic>

/**
 * @file ncbi/jwt.hpp
 * @brief JSON Web Tokens - RFC 7519
 */

#define ALLOW_NESTED_JWT JWT_TESTING

namespace ncbi
{

    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    class JWTClaimSet;
    class JWTClaimSetBuilder;
    class UnverifiedJWTClaims;


    /*=====================================================*
     *                      TYPEDEFS                       *
     *=====================================================*/

    /**
     * @typedef JWT
     * @brief a JSON Web Token - RFC 7519 Section 1.
     *
     * A string representing a set of claims as a JSON object,
     * encapsulated and encoded as a compact JWS or compact JWE.
     *
     * See <ncbi/jws.hpp> for definition of JWS, and
     * <ncbi/jwe.hpp> for definition of JWE.
     */
    typedef String JWT;

    /**
     * @typedef JWTClaimSetRef
     * @brief a reference to a JWTClaimSet
     *
     * This is a shared reference since the JWTClaimSet is immutable.
     */
    typedef SRef < JWTClaimSet > JWTClaimSetRef;

    /**
     * @typedef JWTClaimSetBuilderRef
     * @brief a reference to a JWTClaimSetBuilder
     *
     * This is an unique reference since the JWTClaimSetBuilder
     * is by nature mutable.
     */
    typedef XRef < JWTClaimSetBuilder > JWTClaimSetBuilderRef;

    /**
     * @typedef UnverifiedJWTClaimsRef
     * @brief a reference to an UnverifiedJWTClaims
     *
     * This is a shared reference since the UnverifiedJWTClaims
     * is immutable.
     */
    typedef XRef < UnverifiedJWTClaims > UnverifiedJWTClaimsRef;


    /*=====================================================*
     *                       JWTMgr                        *
     *=====================================================*/
    
    /**
     * @class JWTMgr
     * @brief JWT Management
     *
     * Globally accessible functions for creating and processing
     * claims.
     */
    class JWTMgr
    {
    public:

        /*=================================================*
         *              BUILDING A CLAIM SET               *
         *=================================================*/

        /**
         * @fn makeClaimSetBuilder
         * @brief create an empty JWTClaimSetBuilder object
         * @return JWTClaimSetRef object reference
         *
         * This is a factory function for creating a set of
         * claims that can be converted to a JWT.
         *
         * The set is initially empty and is populated by using
         * the API of the JWTClaimSetBuilder.
         */
        static JWTClaimSetBuilderRef makeClaimSetBuilder ();


        /*=================================================*
         *                     SIGNING                     *
         *=================================================*/

        /**
         * @fn sign
         * @overload sign a JWTClaimSet object
         * @param key C++ reference to the JWK signing key
         * @param claims C++ reference to a JWTClaimSet object
         * @return JWS encapsulated JWT string
         *
         * This method will create the missing reserved claims
         * according to the JWT specification RFC7519. It may also
         * create certain optional claims according to policy.
         * @see finalizeClaims() for details.
         *
         * @see <ncbi/jwk.hpp> for definition of JWK.
         */
        static JWT sign ( const JWK & key, const JWTClaimSet & claims );

#if ALLOW_NESTED_JWT
        /**
         * @fn sign
         * @overload sign a JWT string
         * @param key C++ reference to the JWK signing key
         * @param jwt an existing JWT
         * @return JWS encapsulated nested-JWT string
         *
         * Adds a layer of JWS signature. This may be used to apply
         * standard signing in addition to encryption, but has other
         * potential uses. Anexample of where this might be desired is
         * to combine two separate but equal authorities, where the
         * second adds an additional signature to the first. Another
         * example might be to endorse a less-well-known authority,
         * e.g. the original signer, with the signature of a well-known
         * authority.
         *
         * @see <ncbi/jwk.hpp> for definition of JWK.
         */
        static JWT sign ( const JWK & key, const JWT & jwt );
#endif

        /*=================================================*
         *                   INSPECTION                    *
         *=================================================*/

        /**
         * @fn inspect
         * @brief decode but do not validate a JWT
         * @param keys set of keys to validate signature and potentially decrypt
         * @param jwt the JWT to decode
         * @exception JWSInvalidSignature
         * @return UnverifiedJWTClaims reference
         *
         * A JWT is defined as a multi-part string that has been encoded
         * with base64url, after being encapsulated by JWS or JWE which apply
         * a signature or encryption, respectively. At the time a JWT is to be
         * inspected or decoded, it is unknown how the JWT was generated.
         *
         * This method requires that the JWT pass signature verification, but
         * does not examine the claims for validity, resulting in an object that
         * can be inspected, e.g. for auditing purposes, but cannot used in APIs
         * that assume a validated JWTClaimSet.
         */
        static UnverifiedJWTClaimsRef inspect ( const JWKSet & keys, const JWT & jwt );


        /*=================================================*
         *                    DECODING                     *
         *=================================================*/

        /**
         * @struct DecodeParams
         * @brief fine tune the behavior of decode()
         *
         * All values are set to defaults by constructor and may be
         * modified by the caller before use.
         */
        struct DecodeParams
        {
            /**
             * @fn DecodeParams
             * @overload sets all values to internal defaults
             */
            DecodeParams ();

            /**
             * @fn DecodeParam
             * @overload takes cur_time from param but sets remaining values to defaults
             * @param cur_time a timestamp of seconds since epoch
             */
            DecodeParams ( long long int cur_time );

            long long int cur_time;                     //!< default -1 - may be ignored
            long long int skew_secs;                    //!< default dflt_skew
        };

        /**
         * @fn decode
         * @overload decode a JWT into a JWTClaimSet object with default params
         * @param keys set of keys to validate signatures and potentially decrypt
         * @param jwt the JWT to decode
         * @exception JWSInvalidSignature
         * @exception JWTExpired
         * @return JWTClaimSet object
         *
         * Verifies signatures and decodes JWT string,
         * processes all nested layers toward innermost JWT,
         * checks timestamp for validity,
         * returns claims in a JWTClaimSet object.
         *
         * @see <ncbi/jwk.hpp> for definition of JWKSet.
         */
        static JWTClaimSetRef decode ( const JWKSet & keys, const JWT & jwt );

        /**
         * @fn decode
         * @overload decode a JWT into a JWTClaimSet object with custom params
         * @param keys set of keys to validate signatures and potentially decrypt
         * @param jwt the JWT to decode
         * @param params a DecodeParams block for fine-tuning behavior
         * @exception JWSInvalidSignature
         * @exception JWTExpired
         * @return JWTClaimSet object
         *
         * Verifies signatures and decodes JWT string,
         * checks timestamp for validity.
         */
        static JWTClaimSetRef decode ( const JWKSet & keys,
            const JWT & jwt, const DecodeParams & params );


        /*=================================================*
         *                   UNWRAPPING                    *
         *=================================================*/

#if ALLOW_NESTED_JWT
        /**
         * @fn unwrap
         * @brief attempt to remove the outermost layer of nesting
         * @param keys set of keys to validate wrapping signatures
         * @param jwt the JWT to unwrap on input and the inner JWT on output
         * @return JWKRef to signature verification key
         *
         * This method verifies the signature of "jwt" and determines
         * if it is nested on input. If so, it will update the value
         * of "jwt" on output and return the JWK that verified the signature.
         *
         * If "jwk" is determined to not be nested, it ("jwt") will
         * be left unchanged on output, and the return value will
         * be the value of JWKMgr::getInvalidKey(). Programs should
         * check the validity of the returned key by comparing it
         * with JWKMgr::getInvalidKey().
         */
        JWKRef unwrap ( const JWKSet & keys, INOUT JWT & jwt );
#endif

        /*=================================================*
         *                     POLICY                      *
         *=================================================*/

        /**
         * @struct Policy
         * @brief holds a number of policy properties
         */
        struct Policy
        {
            Policy ();

            Policy & operator = ( const Policy & p );
            Policy & operator = ( Policy && p ) = delete;
            Policy ( const Policy & p );
            Policy ( Policy && p ) = delete;
            ~ Policy ();

            // clock skew
            long long int skew_seconds;   //!< tolerance to clock mismatch ( skew )

            // JWT inspection policy
            bool sig_val_required;        //!< inspect still requires valid signature
            bool nested_sig_val_required; //!< nested signatures must also pass

            // JWT validation policy
            bool iss_required;            //!< "iss" claim is required if true
            bool sub_required;            //!< "sub" claim is required if true
            bool aud_required;            //!< "aud" claim is required if true
            bool exp_required;            //!< "exp" claim is required if true
            bool nbf_required;            //!< "nbf" claim is required if true
            bool iat_required;            //!< "iat" claim is required if true
            bool jti_required;            //!< "jti" claim is required if true

            // JWT generation policy
            bool exp_gen;                 //!< "exp" claim is generated if true
            bool nbf_gen;                 //!< "nbf" claim is generated if true
            bool iat_gen;                 //!< "iat" claim is generated if true
            bool jti_gen;                 //!< "jti" claim is auto-generated if true

            bool pre_serial_verify;       //!< validate claims prior to serialization
            bool zero_dur_allowed;        //!< true if zero is a valid duration

        };

        /**
         * @fn getPolicy
         * @brief read the current policy values
         */
        static const Policy & getPolicy ();

        /**
         * @fn setPolicy
         * @brief set policy values
         * @param p bundle of policies
         *
         * This method will OVERWRITE existing policies.
         *
         * To update select values, read the Policy first and
         * update the desired values before setting.
         */
        static void setPolicy ( const Policy & p );


        /*=================================================*
         *                     UTILITY                     *
         *=================================================*/

        /**
         * @fn now
         * @return long long int timestamp in seconds since epoch
         */
        static long long int now ();

        /**
         * @fn makeID
         * @return String with new JWT id
         */
        static String makeID ();


    private:

        //!< calculate and set reserved claims
        static void finalizeClaims ( INOUT JSONObject & claims,
            long long int duration, long long int iat );

        //!< verify raw claims
        static void verifyClaims ( const JSONObject & claims,
            long long int cur_time, long long int skew_secs );

        JWTMgr ();

        //!< all of the default policies
        static Policy dflt;

        friend class JWTClaimSet;
        friend class JWTClaimSetBuilder;
        friend class UnverifiedJWTClaims;
    };


    /*=====================================================*
     *                 UnverifiedJWTClaims                 *
     *=====================================================*/

    /**
     * @class UnverifiedJWTClaims
     * @brief an object with unverified claims
     *
     * This class represents the contents of a decoded JWT
     * but does not imply that the claims are verified, valid,
     * or even well-formed.
     */
    class UnverifiedJWTClaims
    {
    public:

        /*=================================================*
         *       REGISTERED CLAIM ACCESS PREDICATES        *
         *=================================================*/

        /**
         * @fn hasIssuer
         * @brief test for existence of "iss" claim (section 4.1.1)
         * @return Boolean true if present
         */
        bool hasIssuer () const noexcept;

        /**
         * @fn hasSubject
         * @brief test for existence of "sub" claim (section 4.1.2)
         * @return Boolean true if present
         */
        bool hasSubject () const noexcept;

        /**
         * @fn hasAudience
         * @brief test for existence of "aud" claim (section 4.1.3)
         * @return Boolean true if present
         */
        bool hasAudience () const noexcept;

        /**
         * @fn hasExpiration
         * @brief test for existence of "exp" claim (section 4.1.4)
         * @return Boolean true if present
         */
        bool hasExpiration () const noexcept;

        /**
         * @fn hasNotBefore
         * @brief test for existence of "nbf" claim (section 4.1.5)
         * @return Boolean true if present
         */
        bool hasNotBefore () const noexcept;

        /**
         * @fn hasIssuedAt
         * @brief test for existence of "iat" claim (section 4.1.6)
         * @return Boolean true if present
         */
        bool hasIssuedAt () const noexcept;

        /**
         * @fn hasID
         * @brief test for existence of "jti" claim (section 4.1.7)
         * @return Boolean true if present
         */
        bool hasID () const noexcept;


        /*=================================================*
         *            REGISTERED CLAIM GETTERS             *
         *=================================================*/

        /**
         * @fn getIssuer
         * @brief get "iss" claim value (section 4.1.1)
         * @exception JSONNoSuchName if "iss" is not a member of claims set
         * @return StringOrURI representing the issuer
         *
         * @see <ncbi/uri.hpp> for definition of StringOrURI.
         */
        StringOrURI getIssuer () const;

        /**
         * @fn getSubject
         * @brief get "sub" claim value (section 4.1.2)
         * @exception JSONNoSuchName if "sub" is not a member of claims set
         * @return StringOrURI representing the subject
         *
         * @see <ncbi/uri.hpp> for definition of StringOrURI.
         */
        StringOrURI getSubject () const;

        /**
         * @fn getAudience
         * @brief get "aud" claim array (section 4.1.3)
         * @exception JSONNoSuchName if "aud" is not a member of claims set
         * @return std::vector<StringOrURI> representing all audiences
         *
         * According to RFC7519:
         *  "In the general case, the "aud" value is an array of case-sensitive
         *   strings, each containing a StringOrURI value.  In the special case
         *   when the JWT has one audience, the "aud" value MAY be a single
         *   case-sensitive string containing a StringOrURI value."
         *
         * Therefor, if "aud" is not present, an exception is thrown. But if
         * it is present, a vector will be returned containing 0 or more elements.
         */
        std :: vector < StringOrURI > getAudience () const;

        /**
         * @fn getExpiration
         * @brief get "exp" claim value if set (section 4.1.4)
         * @exception JSONNoSuchName if "exp" is not a member of claims set
         * @return long long int representing the absolute time of token expiration
         */
        long long int getExpiration () const;

        /**
         * @fn getNotBefore
         * @brief get "nbf" claim value if set (section 4.1.5)
         * @exception JSONNoSuchName if "nbf" is not a member of claims set
         * @return long long int representing the absolute time of token validity
         */
        long long int getNotBefore () const;

        /**
         * @fn getIssuedAt
         * @brief get "iat" claim value if set (section 4.1.6)
         * @exception JSONNoSuchName if "iat" is not a member of claims set
         * @return long long int representing the absolute time of token creation
         */
        long long int getIssuedAt () const;

        /**
         * @fn getID
         * @brief get "jti" claim value if set (section 4.1.7)
         * @exception JSONNoSuchName if "jti" is not a member of claims set
         * @return String with ticket id
         */
        String getID () const;


        /*=================================================*
         *                 CLAIMS BY NAME                  *
         *=================================================*/

        /**
         * @fn getNames
         * @return std::vector <String> of claim names
         *
         * Use this set ( unordered list ) of names to access
         * all contained claims via getClaim().
         */
        std :: vector < String > getNames () const;

        /**
         * @fn hasClaim
         * @brief test for existence of named claim
         * @param name String with the claim name
         * @return Boolean true if present
         */
        bool hasClaim ( const String & name ) const noexcept;

        /**
         * @fn getClaim
         * @brief attempts to find a claim by name and return its value
         * @param name String with the claim name
         * @exception JSONNoSuchName if name is not a member of claims set
         * @return const JSONValue reference to existing value
         *
         * The returned value can be any valid JSON type, i.e.
         * null, true, false, numeric or binary integer number,
         * string, array or object.
         *
         * @see <ncbi/json.hpp> JSONValue interface.
         */
        const JSONValue & getClaim ( const String & name ) const;


        /*=================================================*
         *              JOSE HEADER MEMBERS                *
         *=================================================*/

        /**
         * @fn getHdrNames
         * @return std::vector<String> of header names
         *
         * Use this set ( unordered list ) of names to access
         * all contained header members via getHeader().
         */
        std :: vector < String > getHdrNames () const;

        /**
         * @fn hasHeader
         * @brief test for existence of named header member
         * @param name String with the header name
         * @return Boolean true if present
         */
        bool hasHeader ( const String & name ) const noexcept;

        /**
         * @fn getHeader
         * @brief attempts to find a header by name and return its value
         * @param name String with the header name
         * @exception JSONNoSuchName if name is not a member of claims set
         * @return const JSONValue reference to existing value
         *
         * The returned value can be any valid JSON type, i.e.
         * null, true, false, numeric or binary integer number,
         * string, array or object.
         *
         * @see <ncbi/json.hpp> JSONValue interface.
         */
        const JSONValue & getHeader ( const String & name ) const;
 

        /*=================================================*
         *                VERIFICATION KEYS                *
         *=================================================*/

        /**
         * @fn getVerificationKeySeq
         * @brief retrieve all signing keys used to decode JWT
         * @return std::vector<JWKRef> to all signature verification keys
         *
         * The returned vector represents a sequence ( ordered list )
         * of keys resulting from potentially nested signature verification.
         * The order of keys is from outer to inner.
         *
         * The number of keys depends upon the nesting, where a typical
         * JWT is not nested at all and should contain a single key.
         *
         * The degenerate case of having zero keys in the vector is
         * normally prevented by policies prohibiting the "none" signature
         * algorithm. However, programs should still be prepared to discover
         * an empty vector for robustness and testing purposes.
         */
        const std :: vector < JWKRef > & getVerificationKeySeq () const noexcept;


        /*=================================================*
         *                  SERIALIZATION                  *
         *=================================================*/

        /**
         * @fn toJSON
         * @return String with JSON representation of claims set
         *
         * This is a generated string and may differ significantly from any
         * original in the case of a decoded JWT.
         */
        String toJSON () const;

        /**
         * @fn readableJSON
         * @return String with human-formatted JSON representation of claims set.
         *
         * Differs from toJSON() in that spacing, indentation and line endings are inserted.
         */
        String readableJSON ( unsigned int indent = 0 ) const;


        /*=================================================*
         *                   C++ SUPPORT                   *
         *=================================================*/

        /**
         * @fn operator =
         * @overload copy assignment operator
         * @param claims source of contents to clone
         * @return C++ self-reference for use in idiomatic C++ expressions
         *
         * Will delete/overwrite any current contents.
         * Clones the contents of source claim set.
         */
        UnverifiedJWTClaims & operator = ( const UnverifiedJWTClaims & claims );

        /**
         * @fn operator =
         * @overload move assignment operator
         * @param claims source of contents to move
         * @return C++ self-reference for use in idiomatic C++ expressions
         *
         * Will delete/overwrite any current contents.
         * moves the contents of source claim set.
         */
        UnverifiedJWTClaims & operator = ( UnverifiedJWTClaims && claims );

        /**
         * @fn UnverifiedJWTClaims
         * @overload copy constructor
         * @param claims source of contents to clone
         *
         * Clones contents of source claim set.
         */
        UnverifiedJWTClaims ( const UnverifiedJWTClaims & claims );

        /**
         * @fn UnverifiedJWTClaims
         * @overload move constructor
         * @param claims source of contents to move
         *
         * Moves contents of source claim set.
         */
        UnverifiedJWTClaims ( UnverifiedJWTClaims && claims );

        /**
         * @fn ~UnverifiedJWTClaims
         * @brief deletes any contents and destroys internal structures
         */        
        ~ UnverifiedJWTClaims ();

    private:

        //!< constructor called from builder
        UnverifiedJWTClaims ( const JSONObjectRef & jose, const JSONObjectRef & claims );

        //!< constructor called after inspecting a JWT
        UnverifiedJWTClaims ( const std :: vector < JWKRef > & keys,
            const JSONObjectRef & jose, const JSONObjectRef & claims );

        UnverifiedJWTClaims () = delete;

        JSONObjectRef jose;               //!< headers are stored in a JSON object
        JSONObjectRef claims;             //!< claims are stored in a JSON object
        std :: vector < JWKRef > vkeys;   //!< ordered signature validation keys

        friend class JWTMgr;
        friend class JWTClaimSet;
        friend class JWTClaimSetBuilder;
    };


    /*=====================================================*
     *                     JWTClaimSet                     *
     *=====================================================*/

    /**
     * @class JWTClaimSet
     * @brief an object for holding a set of valid/verified claims
     *
     * Claims may be registered or application specific.
     */
    class JWTClaimSet
    {
    public:

        /*=================================================*
         *       REGISTERED CLAIM ACCESS PREDICATES        *
         *=================================================*/

        /**
         * @fn hasIssuer
         * @brief test for existence of "iss" claim (section 4.1.1)
         * @return Boolean true if present
         */
        bool hasIssuer () const noexcept
        { return ucs . hasIssuer (); }

        /**
         * @fn hasSubject
         * @brief test for existence of "sub" claim (section 4.1.2)
         * @return Boolean true if present
         */
        bool hasSubject () const noexcept
        { return ucs . hasSubject (); }

        /**
         * @fn hasAudience
         * @brief test for existence of "aud" claim (section 4.1.3)
         * @return Boolean true if present
         */
        bool hasAudience () const noexcept
        { return ucs . hasAudience (); }

        /**
         * @fn hasExpiration
         * @brief test for existence of "exp" claim (section 4.1.4)
         * @return Boolean true if present
         */
        bool hasExpiration () const noexcept
        { return ucs . hasExpiration (); }

        /**
         * @fn hasNotBefore
         * @brief test for existence of "nbf" claim (section 4.1.5)
         * @return Boolean true if present
         */
        bool hasNotBefore () const noexcept
        { return ucs . hasNotBefore (); }

        /**
         * @fn hasIssuedAt
         * @brief test for existence of "iat" claim (section 4.1.6)
         * @return Boolean true if present
         */
        bool hasIssuedAt () const noexcept
        { return ucs . hasIssuedAt (); }

        /**
         * @fn hasID
         * @brief test for existence of "jti" claim (section 4.1.7)
         * @return Boolean true if present
         */
        bool hasID () const noexcept
        { return ucs . hasID (); }


        /*=================================================*
         *            REGISTERED CLAIM GETTERS             *
         *=================================================*/

        /**
         * @fn getIssuer
         * @brief get "iss" claim value (section 4.1.1)
         * @exception JSONNoSuchName if "iss" is not a member of claims set
         * @return StringOrURI representing the issuer
         *
         * @see <ncbi/uri.hpp> for definition of StringOrURI.
         */
        StringOrURI getIssuer () const
        { return ucs . getIssuer (); }

        /**
         * @fn getSubject
         * @brief get "sub" claim value (section 4.1.2)
         * @exception JSONNoSuchName if "sub" is not a member of claims set
         * @return StringOrURI representing the subject
         *
         * @see <ncbi/uri.hpp> for definition of StringOrURI.
         */
        StringOrURI getSubject () const
        { return ucs . getSubject (); }

        /**
         * @fn getAudience
         * @brief get "aud" claim array (section 4.1.3)
         * @exception JSONNoSuchName if "aud" is not a member of claims set
         * @return std::vector<StringOrURI> representing all audiences
         *
         * According to RFC7519:
         *  "In the general case, the "aud" value is an array of case-sensitive
         *   strings, each containing a StringOrURI value.  In the special case
         *   when the JWT has one audience, the "aud" value MAY be a single
         *   case-sensitive string containing a StringOrURI value."
         *
         * Therefor, if "aud" is not present, an exception is thrown. But if
         * it is present, a vector will be returned containing 0 or more elements.
         */
        std :: vector < StringOrURI > getAudience () const
        { return ucs . getAudience (); }

        /**
         * @fn getExpiration
         * @brief get "exp" claim value if set (section 4.1.4)
         * @exception JSONNoSuchName if "exp" is not a member of claims set
         * @return long long int representing the absolute time of token expiration
         */
        long long int getExpiration () const
        { return ucs . getExpiration (); }

        /**
         * @fn getNotBefore
         * @brief get "nbf" claim value if set (section 4.1.5)
         * @exception JSONNoSuchName if "nbf" is not a member of claims set
         * @return long long int representing the absolute time of token validity
         */
        long long int getNotBefore () const
        { return ucs . getNotBefore (); }

        /**
         * @fn getIssuedAt
         * @brief get "iat" claim value if set (section 4.1.6)
         * @exception JSONNoSuchName if "iat" is not a member of claims set
         * @return long long int representing the absolute time of token creation
         */
        long long int getIssuedAt () const
        { return ucs . getIssuedAt (); }

        /**
         * @fn getID
         * @brief get "jti" claim value if set (section 4.1.7)
         * @exception JSONNoSuchName if "jti" is not a member of claims set
         * @return String with ticket id
         */
        String getID () const
        { return ucs . getID (); }

        /**
         * @fn getValidAsOf
         * @brief return absolute time at which token becomes/became valid
         * @return long long int representing the absolute time of token validity
         *
         * Here "token validity" means either "nbf" claim or "iat" or epoch.
         * If neither "nbf" nor "iat" are present among the claims, the returned
         * value will be zero ( 0 ), meaning the claims have always been valid.
         *
         * The return value is guaranteed in code to be non-negative. The API
         * utilizes a signed integer here for compatibility with other languages
         * that lack support for unsigned integers, and to overcome the fact that
         * C-based languages violate number theory by making Integer a subset of
         * Natural numbers (even though this can be explained away by noting that
         * C types represent finite subsets of each and so the subset relationship
         * is not implied [this is contradicted by the "type promotion" rules,
         * however, that allow conversion of a negative Integer to a Natural]).
         */
        long long int getValidAsOf () const noexcept;

        /**
         * @fn getDuration
         * @brief get token duration affecting "exp" claim value
         * @return long long int representing seconds between "exp" and token validity
         *
         * Here "token validity" means the value returned by getValidAsOf().
         * If the "exp" claim is not present or could not be generated, the return
         * value will be the maximum positive integer representable by a "long long int",
         * known symbolically as LLONG_MAX.
         *
         * @see <climits> for LLONG_MAX.
         */
        long long int getDuration () const noexcept;


        /*=================================================*
         *                 CLAIMS BY NAME                  *
         *=================================================*/

        /**
         * @fn getNames
         * @return std::vector < String > of claim names
         *
         * Use this set ( unordered list ) of names to access
         * all contained claims via getClaim().
         */
        std :: vector < String > getNames () const
        { return ucs . getNames (); }

        /**
         * @fn hasClaim
         * @brief test for existence of named claim
         * @param name String with the claim name
         * @return Boolean true if present
         */
        bool hasClaim ( const String & name ) const noexcept
        { return ucs . hasClaim ( name ); }

        /**
         * @fn getClaim
         * @brief attempts to find a claim by name and return its value
         * @param name String with the claim name
         * @exception JSONNoSuchName if name is not a member of claims set
         * @return const JSONValue reference to existing value
         *
         * The returned value can be any valid JSON type, i.e.
         * null, true, false, numeric or binary integer number,
         * string, array or object.
         *
         * @see <ncbi/json.hpp> JSONValue interface.
         */
        const JSONValue & getClaim ( const String & name ) const
        { return ucs . getClaim ( name ); }


        /*=================================================*
         *              JOSE HEADER MEMBERS                *
         *=================================================*/

        /**
         * @fn getHdrNames
         * @return std::vector < String > of header names
         *
         * Use this set ( unordered list ) of names to access
         * all contained header members via getHeader().
         */
        std :: vector < String > getHdrNames () const
        { return ucs . getHdrNames (); }

        /**
         * @fn hasHeader
         * @brief test for existence of named header member
         * @param name String with the header name
         * @return Boolean true if present
         */
        bool hasHeader ( const String & name ) const noexcept
        { return ucs . hasHeader ( name ); }

        /**
         * @fn getHeader
         * @brief attempts to find a header by name and return its value
         * @param name String with the header name
         * @exception JSONNoSuchName if name is not a member of claims set
         * @return const JSONValue reference to existing value
         *
         * The returned value can be any valid JSON type, i.e.
         * null, true, false, numeric or binary integer number,
         * string, array or object.
         *
         * @see <ncbi/json.hpp> JSONValue interface.
         */
        const JSONValue & getHeader ( const String & name ) const
        { return ucs . getHeader ( name ); }
 

        /*=================================================*
         *                VERIFICATION KEYS                *
         *=================================================*/

        /**
         * @fn getVerificationKeySeq
         * @brief retrieve all signing keys used to decode JWT
         * @return std::vector<JWKRef> to all signature verification keys
         *
         * The returned vector represents a sequence ( ordered list )
         * of keys resulting from potentially nested signature verification.
         * The order of keys is from outer to inner.
         *
         * The number of keys depends upon the nesting, where a typical
         * JWT is not nested at all and should contain a single key.
         *
         * The degenerate case of having zero keys in the vector is
         * normally prevented by policies prohibiting the "none" signature
         * algorithm. However, programs should still be prepared to discover
         * an empty vector for robustness and testing purposes.
         */
        const std :: vector < JWKRef > & getVerificationKeySeq () const
        { return ucs . getVerificationKeySeq (); }


        /*=================================================*
         *                  SERIALIZATION                  *
         *=================================================*/

        /**
         * @fn toJSON
         * @return String with JSON representation of claims set
         *
         * This is a generated string and may differ significantly from any
         * original in the case of a decoded JWT.
         */
        String toJSON () const;

        /**
         * @fn readableJSON
         * @return String with human-formatted JSON representation of claims set.
         *
         * Differs from toJSON() in that spacing, indentation and line endings are inserted.
         */
        String readableJSON ( unsigned int indent = 0 ) const;


        /*=================================================*
         *                   C++ SUPPORT                   *
         *=================================================*/

        /**
         * @fn operator =
         * @overload copy assignment operator
         * @param claims source of contents to clone
         * @return C++ self-reference for use in idiomatic C++ expressions
         *
         * Will delete any current contents.
         * Clones the contents of source claim set.
         */
        JWTClaimSet & operator = ( const JWTClaimSet & claims );

        /**
         * @fn operator =
         * @overload move assignment operator
         * @param claims source of contents to move
         * @return C++ self-reference for use in idiomatic C++ expressions
         *
         * Will delete any current contents.
         * Moves the contents of source claim set.
         */
        JWTClaimSet & operator = ( JWTClaimSet && claims );

        /**
         * @fn JWTClaimSet
         * @overload copy constructor
         * @param claims source of contents to clone
         *
         * Clones contents of source claim set.
         */
        JWTClaimSet ( const JWTClaimSet & claims );

        /**
         * @fn JWTClaimSet
         * @overload move constructor
         * @param claims source of contents to move
         *
         * Moves contents of source claim set.
         */
        JWTClaimSet ( JWTClaimSet && claims );

        /**
         * @fn ~JWTClaimSet
         * @brief deletes any contents and destroys internal structures
         */        
        ~ JWTClaimSet ();

    private:

        //!< constructor called from builder
        JWTClaimSet ( const JSONObjectRef & jose, const JSONObjectRef & claims );

        //!< constructor called after decoding and verifying a JWT
        JWTClaimSet ( const UnverifiedJWTClaims & ucs );

        JWTClaimSet () = delete;

        UnverifiedJWTClaims ucs;          //!< claims we assert as verified
        long long int duration;           //!< offset to produce "exp" from start

        friend class JWTMgr;
        friend class JWTClaimSetBuilder;
     };


    /*=====================================================*
     *                 JWTClaimSetBuilder                  *
     *=====================================================*/

    /**
     * @class JWTClaimSetBuilder
     * @brief an object for building a set of valid claims
     *
     * Claims may be registered or application specific.
     */
    class JWTClaimSetBuilder
    {
    public:

        /*=================================================*
         *       REGISTERED CLAIM ACCESS PREDICATES        *
         *=================================================*/

        /**
         * @fn hasIssuer
         * @brief test for existence of "iss" claim (section 4.1.1)
         * @exception JWTBuilderBusy
         * @return Boolean true if present
         */
        bool hasIssuer () const;

        /**
         * @fn hasSubject
         * @brief test for existence of "sub" claim (section 4.1.2)
         * @exception JWTBuilderBusy
         * @return Boolean true if present
         */
        bool hasSubject () const;

        /**
         * @fn hasAudience
         * @brief test for existence of "aud" claim (section 4.1.3)
         * @exception JWTBuilderBusy
         * @return Boolean true if present
         */
        bool hasAudience () const;

        /**
         * @fn hasExpiration
         * @brief test for existence of "exp" claim (section 4.1.4)
         * @exception JWTBuilderBusy
         * @return Boolean true if present
         */
        bool hasExpiration () const;

        /**
         * @fn hasNotBefore
         * @brief test for existence of "nbf" claim (section 4.1.5)
         * @exception JWTBuilderBusy
         * @return Boolean true if present
         */
        bool hasNotBefore () const;

        /**
         * @fn hasID
         * @brief test for existence of "jti" claim (section 4.1.7)
         * @exception JWTBuilderBusy
         * @return Boolean true if present
         */
        bool hasID () const;


        /*=================================================*
         *            REGISTERED CLAIM SETTERS             *
         *=================================================*/

        /**
         * @fn setIssuer
         * @brief set "iss" claim value (section 4.1.1)
         * @param iss a StringOrURI representing the issuer
         * @exception URIInvalid
         * @exception URIInvalidString
         * @exception JWTBuilderBusy
         *
         * @see <ncbi/uri.hpp> for definition of StringOrURI
         * and URI exceptions.
         */
        void setIssuer ( const StringOrURI & iss );

        /**
         * @fn setSubject
         * @brief set "sub" claim value (section 4.1.2)
         * @param sub a StringOrURI representing the subject
         * @exception URIInvalid
         * @exception URIInvalidString
         * @exception JWTBuilderBusy
         *
         * @see <ncbi/uri.hpp> for definition of StringOrURI
         * and URI exceptions.
         */
        void setSubject ( const StringOrURI & sub );

        /**
         * @fn addAudience
         * @brief set initial or add to "aud" claim array (section 4.1.3)
         * @param aud a StringOrURI representing an audience
         * @exception URIInvalid
         * @exception URIInvalidString
         * @exception JWTBuilderBusy
         *
         * According to RFC7519:
         *  "In the general case, the "aud" value is an array of case-sensitive
         *   strings, each containing a StringOrURI value.  In the special case
         *   when the JWT has one audience, the "aud" value MAY be a single
         *   case-sensitive string containing a StringOrURI value."
         *
         * The first audience StringOrURI that is added will create
         * claims with a single text value. The second audience StringOrURI
         * that is added will cause the "aud" value to be converted to an array
         * containing both the first and second StringOrURI entries. Any subsequent
         * additions will be appended in order to the array.
         *
         * @see <ncbi/uri.hpp> for definition of StringOrURI
         * and URI exceptions.
         */
        void addAudience ( const StringOrURI & aud );

        /**
         * @fn setDuration
         * @brief set token duration affecting "exp" claim value (section 4.1.4)
         * @param dur_seconds the token duration in seconds
         * @exception JWTBuilderBusy
         *
         * The "exp" claim is not normally set directly. Instead, the caller
         * should provide a duration, in whole seconds, from the time of first
         * validity (by default, the time of encoding).
         *
         * The rationale is that since the claims have not yet been encoded into
         * a token, the time of first validity has not yet been determined, which
         * in the majority of cases means that the absolute expiration time cannot
         * be determined. It is true that the JWT spec should have anticipated this
         * and provided a "dur" claim when expiration is relative.
         *
         * If the application requires an absolute expiration time, this behavior
         * can be overridden by use of "setAbsExpiration()".
         */
        void setDuration ( long long int dur_seconds );

        /**
         * @fn setAbsExpiration
         * @brief set token "exp" claim value (section 4.1.4)
         * @param exp_seconds the absolute time at which token expires
         * @exception JWTBuilderBusy
         *
         * The "exp" claim is not normally set directly. Instead, the caller
         * should provide a duration via "setDuration()".
         *
         * The use case for "setAbsExpiration()" is when a token must expire
         * at a pre-determined time.
         */
        void setAbsExpiration ( long long int exp_seconds );

        /**
         * @fn setNotBefore
         * @brief set "nbf" claim value (section 4.1.5)
         * @param nbf_seconds the absolute time at which token becomes valid
         * @exception JWTBuilderBusy
         *
         * If this value is not set, the token will become valid at the
         * time of issue.
         */
        void setNotBefore ( long long int nbf_seconds );

        /*
          The "iat" claim (section 4.1.6) cannot be set under program control.
         */

        /**
         * @fn setID
         * @brief set "jti" claim value (section 4.1.7)
         * @param unique_jti a string that can be used to track individual JWTs
         * @exception JWTBuilderBusy
         *
         * The identifier provided is not evaluated by the library for
         * uniqueness and the claim is not used internally by the library.
         * Enforcement of Uniqueness is the responsibility of the program.
         */
        void setID ( const String & unique_jti );


        /*=================================================*
         *            REGISTERED CLAIM GETTERS             *
         *=================================================*/

        /**
         * @fn getIssuer
         * @brief get "iss" claim value (section 4.1.1)
         * @exception JWTBuilderBusy
         * @return StringOrURI representing the issuer
         *
         * @see <ncbi/uri.hpp> for definition of StringOrURI.
         */
        StringOrURI getIssuer () const;

        /**
         * @fn getSubject
         * @brief get "sub" claim value (section 4.1.2)
         * @return StringOrURI representing the subject
         *
         * @see <ncbi/uri.hpp> for definition of StringOrURI.
         */
        StringOrURI getSubject () const;

        /**
         * @fn getAudience
         * @brief get "aud" claim array (section 4.1.3)
         * @return std :: vector < StringOrURI > representing all audiences
         *
         * If "aud" is present, a vector is always returned regardless of
         * whether the claim value is an array or a string.
         */
        std :: vector < StringOrURI > getAudience () const;

        /**
         * @fn getDuration
         * @brief get token duration affecting "exp" claim value (section 4.1.4)
         * @return long long int representing seconds between "exp" and token validity
         *
         * A negative return value indicates that the duration has not been set
         * and could generate no "exp" claim. A value of 0 is valid but useless
         * except for testing purposes since it will generate a JWT that is immediately
         * expired at the moment it becomes valid.
         */
        long long int getDuration () const;

        /**
         * @fn getAbsExpiration
         * @brief get "exp" claim value if set (section 4.1.4)
         * @exception JSONNoSuchName if "exp" is not a member of claims set
         * @return long long int representing the absolute time of token expiration
         */
        long long int getAbsExpiration () const;

        /**
         * @fn getNotBefore
         * @brief get "nbf" claim value if set (section 4.1.5)
         * @return long long int representing the absolute time of token validity
         */
        long long int getNotBefore () const;

        /**
         * @fn getID
         * @brief get "jti" claim value if set (section 4.1.7)
         * @return String with ticket id
         */
        String getID () const;


        /*=================================================*
         *                 CLAIMS BY NAME                  *
         *=================================================*/

        /**
         * @fn setClaim
         * @brief adds or updates an unregistered claim
         * @param name String with claim name
         * @param value a JSONValueRef
         * @exception JSONNullValue if ! val == true
         * @exception JWTBuilderBusy
         *
         * The provided value can be any valid JSON type, i.e.
         * null, true, false, numeric or binary integer number,
         * string, array or object.
         *
         * @see <ncbi/json.hpp> JSONValue interface.
         */
        void setClaim ( const String & name, const JSONValueRef & value );

        /**
         * @fn deleteClaim
         * @brief deletes claim if found
         * @param name String with claim name
         * @exception JWTBuilderBusy
         *
         * Will delete the claim, whether registered or unregistered,
         * if found.
         */
        void deleteClaim ( const String & name );

        /**
         * @fn getNames
         * @return std::vector < String > of claim names
         *
         * Use this set ( unordered list ) of names to access
         * all contained claims via getClaim().
         */
        std :: vector < String > getNames () const;

        /**
         * @fn hasClaim
         * @brief test for existence of named claim
         * @param name String with the claim name
         * @return Boolean true if present
         */
        bool hasClaim ( const String & name ) const;

        /**
         * @fn getClaim
         * @brief attempts to find a claim by name and return its value
         * @param name String with the claim name
         * @exception JSONNoSuchName if name is not a member of claims set
         * @return const JSONValue reference to existing value
         *
         * The returned value can be any valid JSON type, i.e.
         * null, true, false, numeric or binary integer number,
         * string, array or object.
         *
         * @see <ncbi/json.hpp> JSONValue interface.
         */
        const JSONValue & getClaim ( const String & name ) const;


        /*=================================================*
         *              JOSE HEADER MEMBERS                *
         *=================================================*/

        /**
         * @fn setHeader
         * @brief adds or updates an unregistered JOSE header
         * @param name String with header member name
         * @param value a JSONValueRef
         * @exception JSONNullValue if ! val == true
         * @exception JWTBuilderBusy
         *
         * the JOSE header is SUPPOSED to be generated ENTIRELY
         * by the JWS or JWE component since its purpose is to
         * describe the treatment given during encapsulation and
         * encoding ( JSON Object >>> SIGNING and ENCRYPTION <<< ).
         * Yet, the RFC permits addition of any random and arbitrarily
         * complex information to a header. Makes a great place to
         * include Grandma's cooking recipes.
         *
         * The value provided can be any valid JSON type, i.e.
         * null, true, false, numeric or binary integer number,
         * string, array or object.
         *
         * @see <ncbi/json.hpp> JSONValue interface.
         */
        void setHeader ( const String & name, const JSONValueRef & value );

        /**
         * @fn getHdrNames
         * @return std::vector < String > of header names
         *
         * Use this set ( unordered list ) of names to access
         * all contained header members via getHeader().
         */
        std :: vector < String > getHdrNames () const;

        /**
         * @fn hasHeader
         * @brief test for existence of named header member
         * @param name String with the header name
         * @return Boolean true if present
         */
        bool hasHeader ( const String & name ) const;

        /**
         * @fn getHeader
         * @brief attempts to find a header by name and return its value
         * @param name String with the header name
         * @exception JSONNoSuchName if name is not a member of claims set
         * @return const JSONValue reference to existing value
         *
         * The returned value can be any valid JSON type, i.e.
         * null, true, false, numeric or binary integer number,
         * string, array or object.
         *
         * @see <ncbi/json.hpp> JSONValue interface.
         */
        const JSONValue & getHeader ( const String & name ) const;


        /*=================================================*
         *                   TYPE CASTS                    *
         *=================================================*/

        /**
         * @fn toClaimSet
         * @exception JWTBuilderBusy
         * @return a COPY of contents as an immutable JWTClaimSet
         *
         * Leaves builder object intact, copying claims and header
         * members into an immutable JWTClaimSet.
         */
        JWTClaimSetRef toClaimSet () const;

        /**
         * @fn stealClaimSet
         * @exception JWTBuilderBusy
         * @return the actual builder contents as an immutable JWTClaimSet
         *
         * Leaves builder object in an empty state.
         */
        JWTClaimSetRef stealClaimSet ();
 

        /*=================================================*
         *                  SERIALIZATION                  *
         *=================================================*/

        /**
         * @fn toJSON
         * @exception JWTBuilderBusy
         * @return String with JSON representation of claims set
         */
        String toJSON () const;

        /**
         * @fn readableJSON
         * @exception JWTBuilderBusy
         * @return String with human-formatted JSON representation of claims set.
         * differs from toJSON() in that spacing, indentation and line endings are inserted.
         */
        String readableJSON ( unsigned int indent = 0 ) const;


        /*=================================================*
         *                   C++ SUPPORT                   *
         *=================================================*/

        /**
         * @fn operator =
         * @overload copy assignment operator
         * @param claims source of contents to clone
         * @exception JWTBuilderBusy
         * @return C++ self-reference for use in idiomatic C++ expressions
         *
         * Will delete any current contents, failing if locked.
         * Clones contents of source claim set, failing if source is locked.
         */
        JWTClaimSetBuilder & operator = ( const JWTClaimSetBuilder & claims );

        /**
         * @fn operator =
         * @overload move assignment operator
         * @param claims source of contents to move
         * @exception JWTBuilderBusy
         * @return C++ self-reference for use in idiomatic C++ expressions
         *
         * Will delete any current contents, failing if locked.
         * Moves contents of source claim set, failing if source is locked.
         */
        JWTClaimSetBuilder & operator = ( JWTClaimSetBuilder && claims );

        /**
         * @fn JWTClaimSetBuilder
         * @overload copy constructor
         * @param claims source of contents to clone
         * @exception JWTBuilderBusy
         *
         * Clones contents of source object, failing if source is locked.
         */
        JWTClaimSetBuilder ( const JWTClaimSetBuilder & claims );

        /**
         * @fn JWTClaimSetBuilder
         * @overload move constructor
         * @param claims source of contents to move
         * @exception JWTBuilderBusy
         *
         * Moves contents of source object, failing if source is locked.
         */
        JWTClaimSetBuilder ( JWTClaimSetBuilder && claims );

        /**
         * @fn ~JWTClaimSetBuilder
         * @brief deletes any contents and destroys internal structures
         */
        ~ JWTClaimSetBuilder ();

    private:

        JWTClaimSetBuilder ( const JSONObjectRef & jose, const JSONObjectRef & claims );

        JWTClaimSetBuilder () = delete;

        /**
         * @struct Reserved
         * @brief container of reserved claim and header names
         */
        struct Reserved
        {
            bool claimExists ( const String & name ) const noexcept;
            bool headerExists ( const String & name ) const noexcept;

            Reserved ();
            ~ Reserved ();

            std :: set < String > claims;
        };

        static Reserved reserved;

        JWTClaimSet cs;
        BusyLock busy;

        friend class JWTMgr;
     };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

    DECLARE_SEC_MSG_EXCEPTION ( MalformedJWT, InvalidArgument );
    DECLARE_SEC_MSG_EXCEPTION ( JWTInvalidSkewAjust, BoundsException );
    DECLARE_SEC_MSG_EXCEPTION ( JWTInvalidClaims, InvalidArgument );
    DECLARE_SEC_MSG_EXCEPTION ( JWTMissingTokenDuration, JWTInvalidClaims );
    DECLARE_SEC_MSG_EXCEPTION ( JWTBadDurationValue, BoundsException );
    DECLARE_SEC_MSG_EXCEPTION ( JWTBadTimestamp, JWTInvalidClaims );
    DECLARE_SEC_MSG_EXCEPTION ( JWTInconsistentTimestamps, JWTInvalidClaims );
    DECLARE_SEC_MSG_EXCEPTION ( JWTTimeBoundsException, BoundsException );
    DECLARE_SEC_MSG_EXCEPTION ( JWTNotYetValid, JWTTimeBoundsException );
    DECLARE_SEC_MSG_EXCEPTION ( JWTExpired, JWTTimeBoundsException );

}
