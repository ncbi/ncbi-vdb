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

#include <ncbi/jwa.hpp>

#include <set>
#include <map>

namespace ncbi
{

    /**
     * @class JWARegistry
     * @brief a global algorithm registry
     */
    class JWARegistry
    {
    public:

        bool acceptJWKAlgorithm ( const String & kty, const String & alg ) const noexcept;
        bool acceptJWSAlgorithm ( const String & alg ) const noexcept;

        JWASignerRef getSigner ( const String & alg ) const;
        JWAVerifierRef getVerifier ( const String & alg ) const;

        void registerSigner ( const String & alg, const JWASignerRef & signer ) noexcept;
        void registerVerifier ( const String & alg, const JWAVerifierRef & verifier ) noexcept;

        void doNothing () noexcept;

        JWARegistry () noexcept;
        ~ JWARegistry () noexcept;

    private:

        void makeMaps () const noexcept;

        struct Maps
        {
            Maps ();
            ~ Maps ();

            std :: set < String > sign_accept;
            std :: set < String > verify_accept;
            std :: map < String, JWASignerRef > signers;
            std :: map < String, JWAVerifierRef > verifiers;
            std :: map < String, std :: set < String > > key_accept;

        };

        mutable Maps * maps;

    };

    extern JWARegistry gJWARegistry;

    void includeJWA_none ( bool always_false );
    void includeJWA_hmac ( bool always_false );
    void includeJWA_rsa ( bool always_false );
    void includeJWA_ecdsa ( bool always_false );

}
