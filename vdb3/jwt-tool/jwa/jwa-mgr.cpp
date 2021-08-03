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
#include <ncbi/jwk.hpp>
#include "jwa-registry.hpp"

#include <cassert>

namespace ncbi
{

    bool JWAMgr :: acceptJWKAlgorithm ( const String & kty, const String & alg )
    {
        return gJWARegistry . acceptJWKAlgorithm ( kty, alg );
    }

    bool JWAMgr :: acceptJWSAlgorithm ( const String & alg )
    {
        return gJWARegistry . acceptJWSAlgorithm ( alg );
    }

    JWASignerRef JWAMgr :: getSigner ( const String & alg )
    {
        return gJWARegistry . getSigner ( alg );
    }

    JWAVerifierRef JWAMgr :: getVerifier ( const String & alg )
    {
        return gJWARegistry . getVerifier ( alg );
    }

    String JWASigner :: getKeyProp ( const JWK & key, const String & name )
    {
        return key . props -> getValue ( name ) . toString ();
    }

    JWASigner :: JWASigner ()
    {
    }

    JWASigner :: ~ JWASigner () noexcept
    {
    }

    String JWAVerifier :: getKeyProp ( const JWK & key, const String & name )
    {
        return key . props -> getValue ( name ) . toString ();
    }

    JWAVerifier :: JWAVerifier ()
    {
    }

    JWAVerifier :: ~ JWAVerifier () noexcept
    {
    }
}
