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

#include <ncbi/secure/string.hpp>
#include <ncbi/jwk.hpp>

#include <set>
#include <vector>

/**
 * @file ncbi/jws.hpp
 * @brief JSON Web Signature - RFC 7515
 */

namespace ncbi
{

    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    class Payload;


    /*=====================================================*
     *                      TYPEDEFS                       *
     *=====================================================*/

    /**
     * @typedef JWS
     * @brief JSON Web Signature - RFC 7515, section 2
     *
     * "A data structure representing a digitally signed
     *  or MACed message."
     */
    typedef String JWS;


    /*=====================================================*
     *                       JWSMgr                        *
     *=====================================================*/
    
    /**
     * @class JWSMgr
     * @brief JWS Management
     *
     * Globally accessible factory functions for signing
     * and extracting ( verifying ) payloads.
     */
    class JWSMgr
    {
    public:


        /*=================================================*
         *            ENCAPSULATION ( SIGNING )            *
         *=================================================*/

        /**
         * signCompact
         * @overload sign using compact serialization - RFC 7515 section 7.1
         * @param key C++ reference to the JWK signing key
         * @param payload a pointer to bytes to be signed
         * @param bytes the size in bytes of payload
         * @return JWS encoded string
         *
         * Sign using compact serialization method.
         * Generates a JOSE header to describe signature,
         * including algorithm and key-id.
         */
        static JWS signCompact ( const JWK & key, const void * payload, size_t bytes );


        /*=================================================*
         *            EXTRACTION ( VERIFYING )             *
         *=================================================*/

        /**
         * extract
         * @overload verify a JWS and extract its payload
         * @param jose a return parameter for JOSE header
         * @param payload a return parameter for JWS payload
         * @param val_keys a set of signature validation keys
         * @param jws the JWS to be verified
         * @return a JWKRef with the key that verified signature
         *
         * A JWS may have been serialized using compact serialization
         * ( RFC 7515 section 7.1 ) or "JSON" serialization
         * ( RFC 7515 section 7.2 ). The "jws" input in this case
         * is examined to determine which method was used.
         *
         * JSON serialization is generally not supported because it
         * is extremely unsafe and violates all notions of secure
         * digital signatures.
         */
        static JWKRef extract ( OUT JSONObjectRef & jose, OUT Payload & payload,
            const JWKSet & val_keys, const JWS & jws );


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

            // JOSE validation policy
            bool kid_required;            //!< "kid" header is required if true

            // JOSE generation policy
            bool kid_gen;                 //!< generate "kid" if true

            // verification policy
            bool require_simple_hdr;      //!< the JOSE must be trivially simple
            bool require_prestored_kid;   //!< require matching key in keyset if "kid" is given
            bool require_prestored_key;   //!< require key in keyset
        };

        /**
         * getPolicy
         * @brief read the current default values
         */
        static const Policy & getPolicy () noexcept;

        /**
         * setPolicy
         * @brief set default values
         * @param dflts bundle of Policy
         *
         * This method will OVERWRITE existing defaults.
         *
         * To update select values, read the Policy first and update the
         * desired values before setting them again.
         */
        static void setPolicy ( const Policy & dflts );

        /**
         * hdrReserved
         * @param name header name being tested
         * @return true if "name" is a reserved JOSE header name in JWS
         */
        static bool hdrReserved ( const String & name ) noexcept;

    private:

        /**
         * signCompact
         * @overload sign with ext. header using compact serialization - RFC 7515: Line 376
         * @param key C++ reference to the JWK signing key
         * @param jose a JSONObject reference representing user JOSE header values
         * @param payload a pointer to bytes to be signed
         * @param bytes the size in bytes of payload
         * @return JWS encoded string
         *
         * Makes use of an externally provided JOSE header object.
         * The JOSE header SHOULD be generated by JWS, but is sometimes abused
         * to include headers from JWT or ...
         *
         * "jose" is intended to be a temporary structure and is modified
         * in place by this method.
         */
        static JWS signCompact ( const JWK & key, INOUT JSONObject & jose,
            const void * payload, size_t bytes );

        /**
         * locateDelimiters
         * @brief scan the String for dot ( '.' ) delimiters
         * @return 0..5 where 5 means 5 or more
         */
        static count_t locateDelimiters ( OUT count_t dots [ 5 ], const String & encoded );

        /**
         * extractCompact
         * @overload verify a JWS and extracts its payload
         * @param jose a return parameter for JOSE header
         * @param payload a return parameter for JWS payload
         * @param val_keys a set of signature validation keys
         * @param jws the JWS to be verified
         * @return a JWKRef with the key that verified signature
         */
        static JWKRef extractCompact ( OUT JSONObjectRef & jose, OUT Payload & payload,
            const JWKSet & val_keys, const JWS & jws );

        /**
         * extractCompact
         * @overload verify a JWS and extracts its payload
         * @param jose a return parameter for JOSE header
         * @param payload a return parameter for JWS payload
         * @param val_keys a set of signature validation keys
         * @param jws the JWS to be verified
         * @param dots location of the dots within "jws"
         * @return a JWKRef with the key that verified signature
         */
        static JWKRef extractCompact ( OUT JSONObjectRef & jose, OUT Payload & payload,
            const JWKSet & val_keys, const JWS & jws, const count_t dots [ 5 ] );

        /**
         * scanProtectedHdr
         * @brief scan the TEXT of the JOSE protected header for 'alg' and 'kid'.
         * @param alg OUT parameter for algorithm string
         * @param kid OUT parameter for key-id string
         * @param jose IN parameter with JSON header text to scan
         * @return true if scan was successful
         *
         * The intention is to avoid throwing the text at a full blown parser,
         * because of the dangers of data-driven algorithms processing untrusted
         * data.
         *
         * The challenge here is to remain simple enough to be verifiably safe,
         * while being sophisticated enough to handle arbitrary JSON.
         */
        static bool scanProtectedHeader ( OUT String & alg, OUT String & kid, const String & jose );

        /**
         * verifyHeader
         * @brief supposed to understand 100% of the header
         * @param jose the header form of a JSON object
         * JWS is an encapsulation and unfortunately doesn't own the JOSE
         * header, making it difficult or impossible to require it to
         * understand all possible contents without violating modularity
         * of the JWT layer, for example.
         */
        static void verifyHeader ( const JSONObject & jose );

        static void verifyHeader2 ( const JSONObject & jose );

        JWSMgr ();

        //!< bundle of policies
        static Policy dflt;

        /**
         * @struct Reserved
         * @brief container of reserved JWS JOSE header names
         */
        struct Reserved
        {
            bool headerExists ( const String & name ) const noexcept;
            bool headerReserved ( const String & name ) const noexcept;
            bool headerUnderstood ( const String & name ) const noexcept;

            Reserved ();
            ~ Reserved ();

            std :: set < String > hdrs;
            std :: set < String > understood;
        };

        static Reserved reserved;

        friend class JWTMgr;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

    DECLARE_SEC_MSG_EXCEPTION ( MalformedJWS, InvalidArgument );
    DECLARE_SEC_MSG_EXCEPTION ( JWSUnsupported, UnsupportedException );
    DECLARE_SEC_MSG_EXCEPTION ( JWSNullPayload, NullArgumentException );
    DECLARE_SEC_MSG_EXCEPTION ( JWSEmptyPayload, InvalidArgument );
    DECLARE_SEC_MSG_EXCEPTION ( JWSIncompatibleKey, IncompatibleTypeException );
    DECLARE_SEC_MSG_EXCEPTION ( JWSKeyNotFound, NotFoundException );
    DECLARE_SEC_MSG_EXCEPTION ( JWSNoKeyAvailable, NotFoundException );
    DECLARE_SEC_MSG_EXCEPTION ( JWSSignatureTruncated, ConstraintViolation );
    DECLARE_SEC_MSG_EXCEPTION ( JWSSignatureMismatch, ConstraintViolation );
    DECLARE_SEC_MSG_EXCEPTION ( JWSInvalidHeader, ConstraintViolation );
    DECLARE_SEC_MSG_EXCEPTION ( JWSMissingKeyIdInHeader, JWSInvalidHeader );
    DECLARE_SEC_MSG_EXCEPTION ( JWSBadKeySet, InvalidArgument );
    DECLARE_SEC_MSG_EXCEPTION ( JWSAlgUnsupported, JWSUnsupported );
}
