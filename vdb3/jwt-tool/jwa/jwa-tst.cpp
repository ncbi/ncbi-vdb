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

#include <ncbi/test/gtest.hpp>

#undef JWA_TESTING
#define JWA_TESTING 1

#include "jwa-mgr.cpp"
#include "jwa-registry.cpp"
#include "jwa-none.cpp"
#include "jwa-hmac.cpp"
#include "jwa-rsa.cpp"
#include "jwa-ecdsa.cpp"

#include <string>
#include <cstring>

namespace ncbi
{

    /*=====================================================*
     *                       JWAMgr                        *
     *=====================================================*/

    TEST ( JWAMgrTest, accept_jwk_defined_oct_algorithms )
    {
        bool val;
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "none" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "HS256" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "HS384" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "HS512" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "RS256" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "RS384" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "RS512" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "ES256" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "ES384" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "ES512" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "PS256" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "PS384" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "oct", "PS512" ) );
        EXPECT_EQ ( val, false );
    }

    TEST ( JWAMgrTest, accept_jwk_defined_RSA_algorithms )
    {
        bool val;
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "none" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "HS256" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "HS384" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "HS512" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "RS256" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "RS384" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "RS512" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "ES256" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "ES384" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "ES512" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "PS256" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "PS384" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "RSA", "PS512" ) );
        EXPECT_EQ ( val, false );
    }

    TEST ( JWAMgrTest, accept_jwk_defined_EC_algorithms )
    {
        bool val;
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "none" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "HS256" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "HS384" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "HS512" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "RS256" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "RS384" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "RS512" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "ES256" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "ES384" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "ES512" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "PS256" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "PS384" ) );
        EXPECT_EQ ( val, false );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWKAlgorithm ( "EC", "PS512" ) );
        EXPECT_EQ ( val, false );
    }

    TEST ( JWAMgrTest, accept_defined_jws_algorithms )
    {
        bool val;
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "none" ) );
#if JWA_TESTING
        EXPECT_EQ ( val, true );
#else
        EXPECT_EQ ( val, false );
#endif
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "HS256" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "HS384" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "HS512" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "RS256" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "RS384" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "RS512" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "ES256" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "ES384" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "ES512" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "PS256" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "PS384" ) );
        EXPECT_EQ ( val, true );
        EXPECT_NO_THROW ( val = JWAMgr :: acceptJWSAlgorithm ( "PS512" ) );
        EXPECT_EQ ( val, true );
    }

    TEST ( JWAMgrTest, get_defined_signer )
    {
        JWASignerRef signer;
#if JWA_TESTING
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "none" ) );
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "HS256" ) );
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "HS384" ) );
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "HS512" ) );
#else
        EXPECT_ANY_THROW ( signer = JWAMgr :: getSigner ( "none" ) );
        EXPECT_ANY_THROW ( signer = JWAMgr :: getSigner ( "HS256" ) );
        EXPECT_ANY_THROW ( signer = JWAMgr :: getSigner ( "HS384" ) );
        EXPECT_ANY_THROW ( signer = JWAMgr :: getSigner ( "HS512" ) );
#endif
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "RS256" ) );
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "RS384" ) );
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "RS512" ) );
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "ES256" ) );
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "ES384" ) );
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "ES512" ) );
        EXPECT_ANY_THROW ( signer = JWAMgr :: getSigner ( "PS256" ) );
        EXPECT_ANY_THROW ( signer = JWAMgr :: getSigner ( "PS384" ) );
        EXPECT_ANY_THROW ( signer = JWAMgr :: getSigner ( "PS512" ) );
    }

    TEST ( JWAMgrTest, get_defined_verifier )
    {
        JWAVerifierRef verifier;
#if JWA_TESTING
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "none" ) );
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "HS256" ) );
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "HS384" ) );
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "HS512" ) );
#else
        EXPECT_ANY_THROW ( verifier = JWAMgr :: getVerifier ( "none" ) );
        EXPECT_ANY_THROW ( verifier = JWAMgr :: getVerifier ( "HS256" ) );
        EXPECT_ANY_THROW ( verifier = JWAMgr :: getVerifier ( "HS384" ) );
        EXPECT_ANY_THROW ( verifier = JWAMgr :: getVerifier ( "HS512" ) );
#endif
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "RS256" ) );
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "RS384" ) );
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "RS512" ) );
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "ES256" ) );
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "ES384" ) );
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "ES512" ) );
        EXPECT_ANY_THROW ( verifier = JWAMgr :: getVerifier ( "PS256" ) );
        EXPECT_ANY_THROW ( verifier = JWAMgr :: getVerifier ( "PS384" ) );
        EXPECT_ANY_THROW ( verifier = JWAMgr :: getVerifier ( "PS512" ) );
    }

    TEST ( JWAMgrTest, get_defined_signer_idempotent )
    {
        JWASignerRef signer1, signer2;
        EXPECT_NO_THROW ( signer1 = JWAMgr :: getSigner ( "RS256" ) );
        EXPECT_NO_THROW ( signer2 = JWAMgr :: getSigner ( "RS256" ) );
        EXPECT_EQ ( signer1, signer2 );
    }

    TEST ( JWAMgrTest, get_defined_verifier_idempotent )
    {
        JWAVerifierRef verifier1, verifier2;
        EXPECT_NO_THROW ( verifier1 = JWAMgr :: getVerifier ( "RS256" ) );
        EXPECT_NO_THROW ( verifier2 = JWAMgr :: getVerifier ( "RS256" ) );
        EXPECT_EQ ( verifier1, verifier2 );
    }
}

extern "C"
{
    int main ( int argc, const char * argv [], const char * envp []  )
    {
        testing :: InitGoogleTest ( & argc, ( char ** ) argv );
        return RUN_ALL_TESTS ();
    }
}
