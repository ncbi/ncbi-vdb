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

#include <ncbi/jwa.hpp>
#include "jwa-registry.hpp"

namespace ncbi
{

    JWARegistry gJWARegistry;

    bool JWARegistry :: acceptJWKAlgorithm ( const String & kty, const String & alg ) const noexcept
    {
        makeMaps ();

        try
        {
            const Maps * cmaps = maps;
            auto it1 = cmaps -> key_accept . find ( kty );
            if ( it1 != cmaps -> key_accept . cend () )
            {
                const std :: set < String > & accept = it1 -> second;
                auto it2 = accept . find ( alg );
                if ( it2 != accept . cend () )
                    return true;
            }
        }
        catch ( ... )
        {
        }

        return false;
    }

    bool JWARegistry :: acceptJWSAlgorithm ( const String & alg ) const noexcept
    {
        makeMaps ();

        try
        {
            const Maps * cmaps = maps;
            auto it1 = cmaps -> verify_accept . find ( alg );
            return it1 != cmaps -> verify_accept . cend ();
        }
        catch ( ... )
        {
        }

        return false;
    }

    JWASignerRef JWARegistry :: getSigner ( const String & alg ) const
    {
        makeMaps ();

        const Maps * cmaps = maps;
        auto it = cmaps -> signers . find ( alg );
        if ( it == cmaps -> signers . cend () )
        {
            throw JWAUnsupported (
                XP ( XLOC )
                << "signer"
                << " for algorithm '"
                << alg
                << "' is not supported"
                );
        }

        return it -> second;
    }

    JWAVerifierRef JWARegistry :: getVerifier ( const String & alg ) const
    {
        makeMaps ();

        const Maps * cmaps = maps;
        auto it = cmaps -> verifiers . find ( alg );
        if ( it == cmaps -> verifiers . cend () )
        {
            throw JWAUnsupported (
                XP ( XLOC )
                << "verifier"
                << " for algorithm '"
                << alg
                << "' is not supported"
                );
        }

        return it -> second;
    }

    void JWARegistry :: registerSigner ( const String & alg, const JWASignerRef & signer ) noexcept
    {
        makeMaps ();

        const Maps * cmaps = maps;
        auto it = cmaps -> sign_accept . find ( alg );
        if ( it != cmaps -> sign_accept . cend () )
            maps -> signers . emplace ( alg, signer );
    }

    void JWARegistry :: registerVerifier ( const String & alg, const JWAVerifierRef & verifier ) noexcept
    {
        makeMaps ();

        const Maps * cmaps = maps;
        auto it = cmaps -> verify_accept . find ( alg );
        if ( it != cmaps -> verify_accept . cend () )
            maps -> verifiers . emplace ( alg, verifier );
    }

    void JWARegistry :: doNothing () noexcept
    {
        includeJWA_none ( false );
        includeJWA_hmac ( false );
        includeJWA_rsa ( false );
        includeJWA_ecdsa ( false );
    }

    void JWARegistry :: makeMaps () const noexcept
    {
        if ( maps == nullptr )
        {
            try
            {
                Maps * tmp = new Maps ();
                maps = tmp;
            }
            catch ( ... )
            {
            }
        }
    }

    JWARegistry :: JWARegistry () noexcept
    {
        makeMaps ();
    }

    JWARegistry :: ~ JWARegistry () noexcept
    {
        try
        {
            delete maps;
            maps = nullptr;
        }
        catch ( ... )
        {
        }
    }

    JWARegistry :: Maps :: Maps ()
    {
        size_t i;

        const char * sign_accept_algs [] =
        {
#if JWA_TESTING
            "none",
            "HS256", "HS384", "HS512",
#endif
            "RS256", "RS384", "RS512",
            "ES256", "ES384", "ES512",
            "PS256", "PS384", "PS512"
        };

        const char * verify_accept_algs [] =
        {
#if JWA_TESTING
            "none",
#endif
            "HS256", "HS384", "HS512",
            "RS256", "RS384", "RS512",
            "ES256", "ES384", "ES512",
            "PS256", "PS384", "PS512"
        };

        const char * oct_key_accept_algs [] =
        {
            "HS256", "HS384", "HS512"
        };

        const char * RSA_key_accept_algs [] =
        {
            "RS256", "RS384", "RS512"
        };

        const char * EC_key_accept_algs [] =
        {
            "ES256", "ES384", "ES512"
        };

        for ( i = 0; i < sizeof sign_accept_algs / sizeof sign_accept_algs [ 0 ]; ++ i )
            sign_accept . emplace ( String ( sign_accept_algs [ i ] ) );

        for ( i = 0; i < sizeof verify_accept_algs / sizeof verify_accept_algs [ 0 ]; ++ i )
            verify_accept . emplace ( String ( verify_accept_algs [ i ] ) );

        std :: set < String > oct_set;
        for ( i = 0; i < sizeof oct_key_accept_algs / sizeof oct_key_accept_algs [ 0 ]; ++ i )
            oct_set . emplace ( String ( oct_key_accept_algs [ i ] ) );

        std :: set < String > RSA_set;
        for ( i = 0; i < sizeof RSA_key_accept_algs / sizeof RSA_key_accept_algs [ 0 ]; ++ i )
            RSA_set . emplace ( String ( RSA_key_accept_algs [ i ] ) );

        std :: set < String > EC_set;
        for ( i = 0; i < sizeof EC_key_accept_algs / sizeof EC_key_accept_algs [ 0 ]; ++ i )
            EC_set . emplace ( String ( EC_key_accept_algs [ i ] ) );

        key_accept . emplace ( "oct", oct_set );
        key_accept . emplace ( "RSA", RSA_set );
        key_accept . emplace ( "EC",  EC_set  );

    }

    JWARegistry :: Maps :: ~ Maps ()
    {
    }
}
