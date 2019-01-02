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

#ifndef _hpp_ncbi_oauth_jws_
#define _hpp_ncbi_oauth_jws_

#ifndef _hpp_ncbi_oauth_json_
#include <jwt/json.hpp>
#endif

#ifndef _hpp_ncbi_oauth_jwa_
#include <jwt/jwa.hpp>
#endif

#include <vector>
#include <string>

namespace ncbi
{
    // forwards
    class HMAC_JWKey;
    class RSAPublic_JWKey;
    class RSAPrivate_JWKey;
    class EllipticCurvePublic_JWKey;
    class EllipticCurvePrivate_JWKey;

    // JSON Web Signature - RFC 7515: Line 350
    // A data structure representing a digitally signed or MACed message
    typedef std :: string JWS;

    class JWSFactory
    {
    public:

        // RFC 7515: Line 376
        // sign using compact serialization
        // signing input is:
        //   ASCII(BASE64URL(UTF8(JWS Protected Header)) || '.' || BASE64URL(JWS Payload))
        // returned string is BAE64URL encoded
        JWS signCompact ( JSONObject & hdr, const void * payload, size_t bytes ) const;

        // check that the JOSE header is completely understood
        // validates signature
        // or throw exception
        // returns authority-name of matching validator
        const std :: string & validate ( const JSONObject & hdr, const JWS & jws, size_t last_period ) const;

        // additional verifiers
        // duplicates reference when successful
        void addVerifier ( const std :: string & authority_name,
            const std :: string & alg, const JWK * key );

        // copy construction
        JWSFactory & operator = ( const JWSFactory & fact );
        JWSFactory ( const JWSFactory & fact );

        // create a standard factory
        // duplicates key reference when successful
        JWSFactory ( const std :: string & authority_name,
            const std :: string & alg, const JWK * key );
        ~ JWSFactory ();

    private:

        const JWASigner * signer;
        const JWAVerifier * verifier;
        std :: vector < const JWAVerifier * > addl_verifiers;
    };
}

#endif /* _hpp_ncbi_oauth_jws_ */
