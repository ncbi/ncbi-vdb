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

#ifndef _hpp_ncbi_oauth_jwk_
#define _hpp_ncbi_oauth_jwk_

#include <atomic>
#include <string>
#include <vector>
#include <map>

namespace ncbi
{
    // forwards
    class JWKSet;
    class JSONObject;
    class HMAC_JWKey;
    class RSAPublic_JWKey;
    class RSAPrivate_JWKey;
    class EllipticCurvePublic_JWKey;
    class EllipticCurvePrivate_JWKey;

    // JSON Web Key - RFC 7517

    class JWK
    {
    public:

        // inflate a JWK from JSON text
        static const JWK * parse ( const std :: string & json_text );

        // inflate from PEM text format
        static const JWK * parsePEM ( const std :: string & pem_text,
            const std :: string & use, const std :: string & alg, const std :: string & kid );
        static const JWK * parsePEM ( const std :: string & pem_text, const std :: string & pwd,
            const std :: string & use, const std :: string & alg, const std :: string & kid );

        // inflate from DER format
        static const JWK * parseDER ( const void * key, size_t key_size,
            const std :: string & use, const std :: string & alg, const std :: string & kid );
        static const JWK * parseDER ( const void * key, size_t key_size, const std :: string & pwd,
            const std :: string & use, const std :: string & alg, const std :: string & kid );

        // inflate from PEM or DER format
        static const JWK * parsePEMorDER ( const void * key, size_t key_size,
            const std :: string & use, const std :: string & alg, const std :: string & kid );
        static const JWK * parsePEMorDER ( const void * key, size_t key_size, const std :: string & pwd,
            const std :: string & use, const std :: string & alg, const std :: string & kid );

        // questions about key that don't involve looking at values
        virtual bool forSigning () const;
        virtual bool forEncryption () const;
        virtual bool isSymmetric () const;
        virtual bool isPrivate () const;
        virtual bool isRSA () const;
        virtual bool isEllipticCurve () const;

        // safe casts
        virtual const HMAC_JWKey * toHMAC () const;
        virtual const RSAPrivate_JWKey * toRSAPrivate () const;
        virtual const RSAPublic_JWKey * toRSAPublic () const;
        virtual const EllipticCurvePrivate_JWKey * toEllipticCurvePrivate () const;
        virtual const EllipticCurvePublic_JWKey * toEllipticCurvePublic () const;

        // "kty"
        //  MANDATORY in a JWK (section 4.1)
        //  legal values "oct", "RSA", "EC"
        std :: string getType () const;

        // "kid"
        //  optional (section 4.5), but
        //  our library currently makes it MANDATORY
        std :: string getID () const;

        // "alg"
        //  identifies the algorithm (section 4.4)
        std :: string getAlg () const;

        // "use"
        //  needed for public keys (section 4.2)
        //  legal values are "sig" (signature) and "enc" (encryption)
        std :: string getUse () const;

        // serialize as JSON text
        std :: string toJSON () const;

        // for creating a "readable" JSON text
        std :: string readableJSON ( unsigned int indent = 0 ) const;

        // primitive memory management
        const JWK * duplicate () const;
        void release () const;

    protected:

        // check common JWK properties before making object
        static void checkProperties ( const JSONObject * props );

        virtual ~ JWK ();

        JWK ( JSONObject * props );

        JSONObject * props;
        mutable std :: atomic < unsigned int > refcount;

        friend class JWKSet;
    };

    class HMAC_JWKey : public JWK
    {
    public:

        // make a new symmetric HMAC shared-secret key with randomly generated value
        //  "key_bits" size of the key in bits - must be multiple of 8
        //  "use" - "sig" for signing, "enc" for encryption
        //  "alg" - algorithm id in { "HS256", "HS384", "HS512" }
        static const HMAC_JWKey * make ( unsigned int key_bits,
            const std :: string & use, const std :: string & alg, const std :: string & kid );

        // JWK overrides
        virtual bool isSymmetric () const;
        virtual const HMAC_JWKey * toHMAC () const;

        // get symmetric key "k"
        std :: string getValue () const;

    private:

        static HMAC_JWKey * make ( JSONObject * props );

        // "kty" = "oct"
        HMAC_JWKey ( JSONObject * props );

        friend class JWK;
    };

    class RSAPrivate_JWKey : public JWK
    {
    public:

        // make a new asymmetric RSA private key with randomly generated value
        //  "key_bits" size of the key in bits - must be multiple of 8
        //  "use" - "sig" for signing, "enc" for encryption
        //  "alg" - algorithm id in { "RS256", "RS384", "RS512" }
        static const RSAPrivate_JWKey * make ( unsigned int key_bits,
            const std :: string & use, const std :: string & alg, const std :: string & kid );

        // JWK overrides
        virtual bool isRSA () const;
        virtual bool isPrivate () const;
        virtual const RSAPrivate_JWKey * toRSAPrivate () const;
        virtual const RSAPublic_JWKey * toRSAPublic () const;

        // modulus "n"
        std :: string getModulus () const;

        // exponent "e"
        std :: string getExponent () const;

        // private exponent "d"
        std :: string getPrivateExponent () const;

        // first prime factor "p"
        std :: string getFirstPrimeFactor () const;

        // second prime factor "q"
        std :: string getSecondPrimeFactor () const;

        // first factor CRT exponent "dp"
        std :: string getFirstFactorCRTExponent () const;

        // second factor CRT exponent "dq"
        std :: string getSecondFactorCRTExponent () const;

        // first CRT coefficient "qi"
        std :: string getFirstCRTCoefficient () const;

        // other primes "oth"
        //  prime factor "r"
        //  factor CRT exponent "d"
        //  factor CRT coefficient "t"
        size_t numOtherPrimes () const;
        const JSONObject & getOtherPrime ( unsigned int idx ) const;

    private:

        static RSAPrivate_JWKey * make ( JSONObject * props );

        // "kty" = "RSA"
        RSAPrivate_JWKey ( JSONObject * props );

        friend class JWK;
    };

    class RSAPublic_JWKey : public JWK
    {
    public:

        // derive a public key from private
        static const RSAPublic_JWKey * derive ( const RSAPrivate_JWKey * priv );

        // JWK overrides
        virtual bool isRSA () const;
        virtual const RSAPublic_JWKey * toRSAPublic () const;

        // modulus "n"
        std :: string getModulus () const;

        // exponent "e"
        std :: string getExponent () const;

    private:

        static RSAPublic_JWKey * make ( JSONObject * props );

        // "kty" = "RSA"
        RSAPublic_JWKey ( JSONObject * props );

        friend class JWK;
    };


    class EllipticCurvePrivate_JWKey : public JWK
    {
    public:

        // make a new asymmetric EC private key with randomly generated value
        //  "key_bits" size of the key in bits - must be multiple of 8
        //  "use" - "sig" for signing, "enc" for encryption
        //  "alg" - algorithm id in { "ES256", "ES384", "ES512" }
        static const EllipticCurvePrivate_JWKey * make ( const std :: string & curve,
            const std :: string & use, const std :: string & alg, const std :: string & kid );

        // JWK overrides
        virtual bool isEllipticCurve () const;
        virtual bool isPrivate () const;
        virtual const EllipticCurvePrivate_JWKey * toEllipticCurvePrivate () const;
        virtual const EllipticCurvePublic_JWKey * toEllipticCurvePublic () const;

        // curve "crv"
        std :: string getCurve () const;

        // X coordinate "x"
        std :: string getXCoordinate () const;

        // Y coordinate "y"
        std :: string getYCoordinate () const;

        // ECC private key "d"
        std :: string getECCPrivateKey () const;

    private:

        static EllipticCurvePrivate_JWKey * make ( JSONObject * props );

        // "kty" = "EC"
        EllipticCurvePrivate_JWKey ( JSONObject * props );

        friend class JWK;
    };

    class EllipticCurvePublic_JWKey : public JWK
    {
    public:

        // derive a public key from private
        static const EllipticCurvePublic_JWKey * derive ( const EllipticCurvePrivate_JWKey * priv );

        // JWK overrides
        virtual bool isEllipticCurve () const;
        virtual const EllipticCurvePublic_JWKey * toEllipticCurvePublic () const;

        // curve "crv"
        std :: string getCurve () const;

        // X coordinate "x"
        std :: string getXCoordinate () const;

        // Y coordinate "y"
        std :: string getYCoordinate () const;

    private:

        static EllipticCurvePublic_JWKey * make ( JSONObject * props );

        // "kty" = "EC"
        EllipticCurvePublic_JWKey ( JSONObject * props );

        friend class JWK;
    };


    class JWKSet
    {
    public:

        bool isEmpty () const;
        unsigned long int count () const;

        bool contains ( const std :: string & kid ) const;

        std :: vector < std :: string > getKeyIDs () const;

        void addKey ( const JWK * jwk );
        const JWK * getKey ( const std :: string & kid ) const;
        void removeKey ( const std :: string & kid );

        JWKSet ( const JWKSet & ks );
        JWKSet & operator = ( const JWKSet & ks );

        JWKSet ();
        ~ JWKSet ();

    private:

        JSONObject * kset;
        std :: map < std :: string, const JWK * > map;
    };
}

#endif /* _hpp_ncbi_oauth_jwk_ */
