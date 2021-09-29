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

#include <ncbi/secure/busy.hpp>
#include <ncbi/secure/string.hpp>
#include <ncbi/json.hpp>

#include <atomic>
#include <vector>
#include <map>

/**
 * @file ncbi/jwk.hpp
 * @brief JSON Web Key Management - RFC 7517
 *
 * API to the JWK component, exposing only elements needed
 * by a client application.
 */

namespace ncbi
{

    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    class JWK;
    class JWKSet;

    /*=====================================================*
     *                      TYPEDEFS                       *
     *=====================================================*/

    /**
     * @typedef JWKRef
     * @brief shared reference to a JWK
     */
    typedef SRef < const JWK > JWKRef;

    /**
     * @typedef JWKSetRef
     * @brief unique reference to a JWKSet
     */
    typedef XRef < JWKSet > JWKSetRef;


    /*=====================================================*
     *                       JWKMgr                        *
     *=====================================================*/
    
    /**
     * @class JWKMgr
     * @brief JSON Web Key Manager
     *
     * Globally accessible factory functions for managing
     * JWKs and JWKSets.
     */
    class JWKMgr
    {
    public:

        /**
         * getInvalidKey
         * @brief access singleton invalid key object
         * @return JWKRef to invalid key
         *
         * A very few interfaces have the need to use or return
         * an invalid JWK. Rather than using a JWKRef to nullptr
         * which is certainly very invalid and can be detected,
         * this key can be used instead and it will not segfault.
         */
        static JWKRef getInvalidKey () noexcept;

        /**
         * makeJWKSet
         * @brief create an empty JWKSet
         * @return JWKSetRef
         */
        static JWKSetRef makeJWKSet ();

        /**
         * parseJWK
         * @brief inflate a JWK from JSON text
         * @param json_text a serialized version of a JWK
         * @return JWKRef
         */
        static JWKRef parseJWK ( const String & json_text );

        /**
         * parseJWKSet
         * @brief inflate a JWK from JSON text
         * @param json_text a serialized version of a JWKSet
         * @return JWKSetRef
         */
        static JWKSetRef parseJWKSet ( const String & json_text );

        /**
         * parsePEM
         * @overload inflate public JWK from PEM text format
         * @param pem_text an encrypted PEM object
         * @param use the intended key usage "sig" or "enc"
         * @param alg the algorithm to apply with key, e.g. "RS256"
         * @param kid a globally unique identifier for this key
         * @return JWKRef
         */
        static JWKRef parsePEM ( const String & pem_text,
            const String & use, const String & alg, const String & kid );

        /**
         * parsePEM
         * @overload inflate private JWK from PEM text format
         * @param pem_text an encrypted PEM object
         * @param pwd password for decrypting PEM
         * @param use the intended key usage "sig" or "enc"
         * @param alg the algorithm to apply with key, e.g. "RS256"
         * @param kid a globally unique identifier for this key
         * @return JWKRef
         */
        static JWKRef parsePEM ( const String & pem_text, const String & pwd,
            const String & use, const String & alg, const String & kid );

        /**
         * makeID
         * @return String with new JWK id
         */
        static String makeID ();

        /**
         * validateJWK
         * @brief examine a key for well-formedness
         * @param key a JSONObject with key contents
         */
        static void validateJWK ( const JSONObject & key );

        /**
         * validateJWKSet
         * @brief examine a key for well-formedness
         * @param key_set a JSONObject with key-set contents
         */
        static void validateJWKSet ( const JSONObject & key_set );

    private:

        JWKMgr ();

        struct InvalKey
        {
            void make ();
            InvalKey ();
            ~ InvalKey ();

            JWKRef key;
        };

        static InvalKey inval;
    };


    /*=====================================================*
     *                         JWK                         *
     *=====================================================*/
    
    /**
     * @class JWK
     * @brief a partially-opaque (translucent?) representation of a key
     */
    class JWK
    {
    public:

        /*=================================================*
         *                TYPE PREDICATES                  *
         *=================================================*/

        /**
         * forSigning
         * @return Boolean true if this key is intended for signing
         *
         * Detects private or symmetrical signing keys.
         */
        bool forSigning () const noexcept;

        /**
         * forVerifying
         * @return Boolean true if this key is intended for signature verification
         *
         * Detects public or symmetrical signature verification keys.
         */
        bool forVerifying () const noexcept;

        /**
         * forEncryption
         * @return Boolean true if this key is intended for encrypting
         *
         * Detects public or symmetrical encryption keys.
         */
        bool forEncryption () const noexcept;

        /**
         * forDecryption
         * @return Boolean true if this key is intended for decryption
         *
         * Detects private or symmetrical encryption keys.
         */
        bool forDecryption () const noexcept;

        /**
         * isPrivate
         * @return Boolean true if the key contains private material
         *
         * A private key is required for signing or decryption.
         */
        bool isPrivate () const noexcept;

        /**
         * isSymmetric
         * @return Boolean true if the key is symmetric
         */
        bool isSymmetric () const noexcept;

        /**
         * isRSA
         * @return Boolean true if the key is for use in RSA algorithms
         */
        bool isRSA () const noexcept;

        /**
         * isEllipticCurve
         * @return Boolean true if the key is for use in elliptical curve algorithms
         */
        bool isEllipticCurve () const noexcept;


        /*=================================================*
         *          REGISTERED PROPERTY GETTERS            *
         *=================================================*/

        /**
         * getType
         * @return std::string with value of "kty" property
         *
         * This property is MANDATORY in a JWK (section 4.1).
         * Legal values are { "oct", "RSA", "EC" }
         */
        String getType () const;

        /**
         * getUse
         * @return std:: string with value of "use" property

         * This property is needed for public keys (section 4.2).
         * Legal values are "sig" (signature) and "enc" (encryption)
         */
        String getUse () const;

        /**
         * getOperations
         * @return std::vector<std::string> with value of "key_ops" property
         *
         * This property is an alternate for "use" (section 4.3).
         * Registered values are:
         *   "sign", "verify", "encrypt", "decrypt", "wrapKey",
         *   "unwrapKey", "deriveKey", "deriveBits"
         */
        std :: vector < String > getOperations () const;

        /**
         * getAlg
         * @return std:: string with value of "alg" property
         *
         * This property is OPTIONAL (section 4.4).
         */
        String getAlg () const;

        /**
         * getId
         * @return std::string with value of "kid" property
         * @exception PropertyNotFound if the property is not present
         *
         * This property is considered OPTIONAL under RFC (section 4.5)
         * but has been configured to be MANDATORY in this implementation.
         */
        String getID () const;


        /*=================================================*
         *                  SERIALIZATION                  *
         *=================================================*/

        /**
         * toJSON
         * @return C++ std::string with JSON representation of properties
         *
         * NB - this representation may not be universally compatible
         * with other systems. In particular, sensitive information
         * may be encrypted.
         */
        String toJSON () const;

        /**
         * readableJSON
         * @return C++ std::string with human-formatted JSON representation of properties..
         *
         * Differs from toJSON() in that spacing, indentation and line endings are inserted.
         */
        String readableJSON ( unsigned int indent = 0 ) const;


        /**
         * ~JWK
         * @brief deletes any contents and destroys internal structures
         */        
        ~ JWK () noexcept;


    private:

        JWK ( const JSONObjectRef & props );

        void operator = ( const JWK & jwk ) = delete;
        void operator = ( JWK && jwk ) = delete;
        JWK ( const JWK & jwk ) = delete;
        JWK ( JWK && jwk ) = delete;
        JWK () = delete;

        JSONObjectRef props;

        friend class JWKMgr;
        friend class JWKSet;
        friend class JWASigner;
        friend class JWAVerifier;
    };


    /*=====================================================*
     *                        JWKSet                       *
     *=====================================================*/
    
    /**
     * @class JWKSet
     * @brief a set of JWK objects, indexed by kid
     */
    class JWKSet
    {
    public:

        /**
         * isEmpty
         * @return Boolean true if set has no keys
         */
        bool isEmpty () const;

        /**
         * count
         * @return Natural number with the set cardinality
         */
        unsigned long int count () const;

        /**
         * hasVerificationKeys
         * @return true if 1 or more contained keys are for verification
         */
        bool hasVerificationKeys () const;

        /**
         * contains
         * @brief answers whether the indicated entry exists
         * @param kid std::string with the key identifier
         * @return Boolean true if entry exists
         */
        bool contains ( const String & kid ) const;

        /**
         * getKeyIDs
         * @return std::vector<std::string> of key identifiers
         */
        std :: vector < String > getKeyIDs () const;

        /**
         * addKey
         * @brief add a new JWK
         * @param key const JWKRef
         */
        void addKey ( const JWKRef & key );

        /**
         * getKey
         * @brief return JWK for key identifier
         * @param kid std::string with key id
         * @return JWKRef
         */
        JWKRef getKey ( const String & kid ) const;

        /**
         * removeKey
         * @brief remove and release JWK found with kid
         * @param kid std::string with key id
         *
         * Ignored if entry is not found.
         */
        void removeKey ( const String & kid );

        /**
         * clone
         * @return creates a deep copy of set
         */
        JWKSetRef clone () const;

        /**
         * invalidate
         * @brief overwrite potentially sensitive contents in memory
         */
        void invalidate ();

        /**
         * operator =
         * @overload copy assignment operator
         * @param ks source of contents to clone
         * @return C++ self-reference for use in idiomatic C++ expressions
         *
         * Will delete any current contents.
         * Clones contents of source set.
         */
        JWKSet & operator = ( const JWKSet & ks );

        /**
         * operator =
         * @overload move assignment operator
         * @param ks source of contents to move
         * @return C++ self-reference for use in idiomatic C++ expressions
         *
         * Will delete any current contents.
         * Moves contents of source set.
         */
        JWKSet & operator = ( JWKSet && ks );

        /**
         * JWKSet
         * @overload copy constructor
         * @param ks source of contents to clone
         *
         * Clones contents of source set.
         */
        JWKSet ( const JWKSet & ks );

        /**
         * JWKSet
         * @overload move constructor
         * @param ks source of contents to move
         *
         * Moves contents of source set.
         */
        JWKSet ( JWKSet && ks );

        /**
         * ~JWKSet
         * @brief deletes any contents and destroys internal structures
         */        
        ~ JWKSet () noexcept;

    private:

        //!< index JSON by 'kid' of contained keys
        void extractKeys ();

        JWKSet ( const JSONObjectRef & kset );
        JWKSet () = delete;

        JSONObjectRef kset;
        std :: map < String, std :: pair < unsigned long int, JWKRef > > map;
        count_t num_verification_keys;
        BusyLock busy;

        friend class JWKMgr;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

    DECLARE_SEC_MSG_EXCEPTION ( MalformedJWK, InvalidArgument );
    DECLARE_SEC_MSG_EXCEPTION ( MalformedJWKSet, InvalidArgument );
    DECLARE_SEC_MSG_EXCEPTION ( MalformedPEM, InvalidArgument );
    DECLARE_SEC_MSG_EXCEPTION ( JWKUniqueConstraintViolation, UniqueConstraintViolation );
    DECLARE_SEC_MSG_EXCEPTION ( JWKKeyNotFound, NotFoundException );
    DECLARE_SEC_MSG_EXCEPTION ( UnsupportedCurve, UnsupportedException );


}
