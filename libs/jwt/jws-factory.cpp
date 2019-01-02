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

#include <jwt/jws.hpp>
#include <jwt/jwk.hpp>
#include <jwt/jwt.hpp>
#include "base64-priv.hpp"

#include <iostream>
#include <cassert>

namespace ncbi
{
#if JWT_TESTING
    static bool ignore_signature_mismatch;
    static std :: string unrecognized_string ( "unrecognized" );

    void setIgnoreSignatureMismatch ( bool ignore )
    {
        ignore_signature_mismatch = ignore;
    }
#endif

    JWS JWSFactory :: signCompact ( JSONObject & hdr, const void * payload, size_t bytes ) const
    {
        // test the payload
        if ( payload == nullptr || bytes == 0 )
            throw JWTException ( __func__, __LINE__, "invalid payload" );

        // must have a signature
        if ( signer == nullptr )
            throw JWTException ( __func__, __LINE__, "INTERNAL EXCEPTION: invalid signature" );

        // assume that the header has been filled out to the payload's content
        // set the "alg" property in header
        hdr . setValueOrDelete ( "alg", JSONValue :: makeString ( signer -> algorithm () ) );

        // set the "kid" property
        std :: string signing_kid = signer -> keyID ();
        if ( ! signing_kid . empty () )
            hdr . setValueOrDelete ( "kid", JSONValue :: makeString ( signing_kid ) );

        // scope trickery
        JWS jws;
        {
            {
                // convert the header to text
                std :: string hdr_json = hdr . toJSON ();

                // encode the header with base64url
                jws = encodeBase64URL ( hdr_json . data (), hdr_json . size () );
            }

            // encode the payload with base64url
            std :: string pay_base64 = encodeBase64URL ( payload, bytes );

            // concatenate the two strings
            jws += '.';
            jws += pay_base64;

            // now we should be able to generate a signature
            std :: string sig_base64 = signer -> sign ( jws . data (), jws . size () );

            // concatenate the third strings
            jws += '.';
            jws += sig_base64;
        }

        return jws;
    }

    const std :: string & JWSFactory :: validate ( const JSONObject & hdr, const JWS & jws, size_t last_period ) const
    {
        // the "last_period" tells us already where to split the jws
        assert ( last_period < jws . size () );

        std :: string content = jws . substr ( 0, last_period );
        std :: string signature = jws . substr ( last_period + 1 );

        // loop through verifiers
        const JWAVerifier * v = verifier;
        assert ( v != nullptr );
        if ( ! v -> verify ( content . data (), content . size (), signature ) )
        {
            v = nullptr;

            size_t i, count = addl_verifiers . size ();
            for ( i = 0; i < count; ++ i )
            {
                v = addl_verifiers [ i ];
                assert ( v != nullptr );
                if ( v -> verify ( content . data (), content . size (), signature ) )
                    break;
                v = nullptr;
            }

            if ( v == nullptr )
            {
#if JWT_TESTING
                if ( ignore_signature_mismatch )
                {
                    std :: cerr
                        << __func__
                        << ':'
                        << __LINE__
                        << " - signature not recognized"
                        << '\n'
                        ;
                    return unrecognized_string;
                }
#endif
                throw JWTException ( __func__, __LINE__, "signature not recognized" );
            }
        }

        if ( hdr . exists ( "alg" ) )
        {
            std :: string alg = hdr . getValue ( "alg" ) . toString ();
            if ( alg . compare ( v -> algorithm () ) != 0 )
            {
#if JWT_TESTING
                if ( ignore_signature_mismatch )
                {
                    std :: cerr
                        << __func__
                        << ':'
                        << __LINE__
                        << " - algorithm does not match: "
                        << alg
                        << " vs. "
                        << v -> algorithm ()
                        << '\n'
                        ;
                }
                else
#endif
                throw JWTException ( __func__, __LINE__, "algorithm does not match" );
            }
        }

        return v -> authority_name ();
    }

    void JWSFactory :: addVerifier ( const std :: string & name, const std :: string & alg, const JWK * key )
    {
        JWAVerifier * verifier = gJWAFactory . makeVerifier ( name, alg, key );
        addl_verifiers . push_back ( verifier );
    }

    JWSFactory & JWSFactory :: operator = ( const JWSFactory & fact )
    {
        if ( signer != fact . signer )
        {
            JWASigner * new_signer = fact . signer -> clone ();
            delete signer;
            signer = new_signer;
        }

        if ( verifier != fact . verifier )
        {
            const JWAVerifier * new_verifier = fact . verifier -> clone ();
            delete verifier;
            verifier = new_verifier;
        }

        addl_verifiers . clear ();
        size_t i, count = fact . addl_verifiers . size ();
        for ( i = 0; i < count; ++ i )
        {
            const JWAVerifier * addl_verifier = fact . addl_verifiers [ i ] -> clone ();
            addl_verifiers . push_back ( addl_verifier );
        }

        return * this;
    }

    JWSFactory :: JWSFactory ( const JWSFactory & fact )
        : signer ( nullptr )
        , verifier ( nullptr )
    {
        signer = fact . signer -> clone ();
        verifier = fact . verifier -> clone ();

        size_t i, count = fact . addl_verifiers . size ();
        for ( i = 0; i < count; ++ i )
        {
            const JWAVerifier * addl_verifier = fact . addl_verifiers [ i ] -> clone ();
            addl_verifiers . push_back ( addl_verifier );
        }
    }

    JWSFactory :: JWSFactory ( const std :: string & name,
             const std :: string & alg, const JWK * key )
        : signer ( nullptr )
        , verifier ( nullptr )
    {
        signer = gJWAFactory . makeSigner ( name, alg, key );
        verifier = gJWAFactory . makeVerifier ( name, alg, key );
    }

    JWSFactory :: ~ JWSFactory ()
    {
        delete signer;
        signer = nullptr;

        delete verifier;
        verifier = nullptr;

        while ( ! addl_verifiers . empty () )
        {
            const JWAVerifier * verify = addl_verifiers . back ();
            addl_verifiers . pop_back ();
            delete verify;
        }
    }
}

