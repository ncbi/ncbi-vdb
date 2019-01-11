/*===========================================================================
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

/**
* Unit tests for JWT interface
*/

#include <ktst/unit_test.hpp>

#include <iostream>

using namespace std;

#include <jwt/jws.hpp>
#include <jwt/jwk.hpp>
#include <jwt/jwt.hpp>

using namespace ncbi;

TEST_SUITE(JwtTestSuite);

// call this function with a reasonable but fixed value
// before generating any JWT so that the result is predictable
//
// setting the value to 0 will revert to a real timestamp
void jwt_setStaticCurrentTime ( long long cur_time );

/* JWT
    *
    **********************************************************************************/
class JWTFixture_BasicConstruction
{
public:
    JWTFixture_BasicConstruction ()
    :   jws_fact ( nullptr ),
        jwt_fact ( nullptr )
    {
        // fix the current time to a known value
        jwt_setStaticCurrentTime ( 1540664164 );

        // make a symmetric key
        const HMAC_JWKey * key = HMAC_JWKey :: make ( 384, "sig", "HS284", "wonder-key-id" );
        try
        {
            jws_fact = new JWSFactory ( "ncbi", "HS384", key );
            jwt_fact = new JWTFactory ( * jws_fact );
            jwt_fact -> setIssuer ( "ncbi" );
            jwt_fact -> setDuration ( 15 );
        }
        catch ( ... )
        {
            key -> release ();
            throw;
        }
        key -> release ();
    }

    ~JWTFixture_BasicConstruction ()
    {
        delete jwt_fact;
        delete jws_fact;

        // restore timestamp behavior
        jwt_setStaticCurrentTime ( 0 );
    }

public:
    void printClaims ( JWTClaims claims, bool recovered = false )
    {
        if ( ! recovered )
        {
            std :: cout
            << "---- JSON Claims ----\n"
            << "    "
            << claims . toJSON () . data ()
            << "\n---- JSON Claims ----\n"
            << std :: endl
            ;
        }
        else
        {
            std :: cout
            << "---- Decoded Claims ----\n"
            << "    "
            << claims . toJSON () . data ()
            << "\n---- Decoded Claims ----"
            << std :: endl
            ;
        }
    }

    void printJWT ( JWT jwt )
    {
        std :: cout
        << "---- JWT ----\n"
        << "    "
        << jwt . data ()
        << "\n---- JWT ----\n"
        << std :: endl
        ;
    }

    void printJWTTransitionStack ( JWTClaims json, JWT jwt, JWTClaims decoded )
    {
        std :: cout << "{" << std :: endl;
        printClaims ( json );
        printJWT ( jwt );
        printClaims ( decoded, true );
        std :: cout << "}\n" << std :: endl;
    }

protected:

    JWSFactory * jws_fact;
    JWTFactory * jwt_fact;
};

FIXTURE_TEST_CASE ( JWT_Example, JWTFixture_BasicConstruction )
{
    JWTClaims claims = jwt_fact -> make ();
    claims . addClaimOrDeleteValue ( "example", JSONValue :: makeString ( "hello there" ) );
    CHECK_EQUAL( string ( "{\"example\":\"hello there\",\"iss\":\"ncbi\"}" ), string ( claims . toJSON () . data () ) );

    JWT jwt = jwt_fact -> sign ( claims );
    CHECK_EQUAL( string ( "eyJhbGciOiJIUzM4NCIsImtpZCI6IndvbmRlci1rZXktaWQiLCJ0eXAiOiJKV1QifQ.eyJleGFtcGxlIjoiaGVsbG8gdGhlcmUiLCJleHAiOjE1NDA2NjQxNzksImlhdCI6MTU0MDY2NDE2NCwiaXNzIjoibmNiaSJ9.UWIn88KYxos1aiSKxqBdsak9VUMy0t9kylcwB1yEqHKb6cXHlwAoRA4NcDcGzf4N"), string ( jwt . data () ) );

    JWTClaims decoded = jwt_fact -> decode ( jwt );
    CHECK_EQUAL( string ( "{\"example\":\"hello there\",\"exp\":1540664179,\"iat\":1540664164,\"iss\":\"ncbi\"}" ), string ( decoded . toJSON () . data () ) );

    printJWTTransitionStack ( claims, jwt, decoded );

}

//////////////////////////////////////////// Main

extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void )
{
return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
return 0;
}

rc_t CC Usage ( const Args * args )
{
return 0;
}

const char UsageDefaultName[] = "test-jwt";

rc_t CC KMain ( int argc, char *argv [] )
{
KConfigDisableUserSettings();
rc_t rc=JwtTestSuite(argc, argv);
return rc;
}

}
