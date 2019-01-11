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

#include <jwt/jwa.hpp>
#include <jwt/jwt.hpp>

#include <iostream>
#include <cstring>
#include <cassert>

namespace ncbi
{
    struct NONE_Signer : JWASigner
    {
        virtual JwtString sign ( const void * data, size_t bytes ) const
        {
            return "";
        }

        virtual JWASigner * clone () const
        {
            return new NONE_Signer ( alg, nam, key );
        }

        NONE_Signer ( const JwtString & name,
                const JwtString & alg, const JWK * key )
            : JWASigner ( name, alg, key )
        {
        }
    };

    struct NONE_Verifier : JWAVerifier
    {
        virtual bool verify ( const void * data, size_t bytes, const JwtString & signature ) const
        {
            return true;
        }

        virtual JWAVerifier * clone () const
        {
            return new NONE_Verifier ( nam, alg, key );
        }

        NONE_Verifier ( const JwtString & name,
                const JwtString & alg, const JWK * key )
            : JWAVerifier ( name, alg, key )
        {
        }
    };

    struct NONE_SignerFact : JWASignerFact
    {
        virtual JWASigner * make ( const JwtString & name,
            const JwtString & alg, const JWK * key ) const
        {
            return new NONE_Signer ( name, alg, key );
        }

        NONE_SignerFact ( const JwtString & alg )
        {
            gJWAFactory . registerSignerFact ( alg, this );
        }
    };

    struct NONE_VerifierFact : JWAVerifierFact
    {
        virtual JWAVerifier * make ( const JwtString & name,
            const JwtString & alg, const JWK * key ) const
        {
            return new NONE_Verifier ( name, alg, key );
        }

        NONE_VerifierFact ( const JwtString & alg )
        {
            gJWAFactory . registerVerifierFact ( alg, this );
        }
    };


    static struct NONE_Registry
    {
        NONE_Registry ( const JwtString & alg )
            : signer_fact ( alg )
            , verifier_fact ( alg )
        {
        }

        ~ NONE_Registry ()
        {
        }

        NONE_SignerFact signer_fact;
        NONE_VerifierFact verifier_fact;

    } none_registry ( "none" );

    void includeJWA_none ( bool always_false )
    {
        if ( always_false )
        {
            JwtString empty;
            none_registry . signer_fact . make ( empty, empty, nullptr );
        }
    }
}
