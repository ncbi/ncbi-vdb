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

#undef JWS_TESTING
#define JWS_TESTING 1

#include "jws-mgr.cpp"
#include "uri-mgr.cpp"

#include <string>
#include <cstring>

namespace ncbi
{

    class JWSMgrTest : public :: testing :: Test
    {
    public:

        void makeRSAKey1 ()
        {
            // the RSA key in JSON text
            String key_json (
                "{"
                "\"alg\":\"RS256\","
                "\"kty\":\"RSA\","
                "\"kid\":\"rsa-key-1\","
                "\"use\":\"sig\","
                "\"n\":\"ofgWCuLjybRlzo0tZWJjNiuSfb4p4fAkd_wWJcyQoTbji9k0l8W26mPddx"
                "HmfHQp-Vaw-4qPCJrcS2mJPMEzP1Pt0Bm4d4QlL-yRT-SFd2lZS-pCgNMs"
                "D1W_YpRPEwOWvG6b32690r2jZ47soMZo9wGzjb_7OMg0LOL-bSf63kpaSH"
                "SXndS5z5rexMdbBYUsLA9e-KXBdQOS-UTo7WTBEMa2R2CapHg665xsmtdV"
                "MTBQY4uDZlxvb3qCo5ZwKh9kG4LT6_I5IhlJH7aGhyxXFvUK-DWNmoudF8"
                "NAco9_h9iaGNj8q2ethFkMLs91kzk2PAcDTW9gb54h4FRWyuXpoQ\","
                "\"e\":\"AQAB\","
                "\"d\":\"Eq5xpGnNCivDflJsRQBXHx1hdR1k6Ulwe2JZD50LpXyWPEAeP88vLNO97I"
                "jlA7_GQ5sLKMgvfTeXZx9SE-7YwVol2NXOoAJe46sui395IW_GO-pWJ1O0"
                "BkTGoVEn2bKVRUCgu-GjBVaYLU6f3l9kJfFNS3E0QbVdxzubSu3Mkqzjkn"
                "439X0M_V51gfpRLI9JYanrC4D4qAdGcopV_0ZHHzQlBjudU2QvXt4ehNYT"
                "CBr6XCLQUShb1juUO1ZdiYoFaFQT5Tw8bGUl_x_jTj3ccPDVZFD9pIuhLh"
                "BOneufuBiB4cS98l2SR_RQyGWSeWjnczT0QU91p1DhOVRuOopznQ\","
                "\"p\":\"4BzEEOtIpmVdVEZNCqS7baC4crd0pqnRH_5IB3jw3bcxGn6QLvnEtfdUdi"
                "YrqBdss1l58BQ3KhooKeQTa9AB0Hw_Py5PJdTJNPY8cQn7ouZ2KKDcmnPG"
                "BY5t7yLc1QlQ5xHdwW1VhvKn-nXqhJTBgIPgtldC-KDV5z-y2XDwGUc\","
                "\"q\":\"uQPEfgmVtjL0Uyyx88GZFF1fOunH3-7cepKmtH4pxhtCoHqpWmT8YAmZxa"
                "ewHgHAjLYsp1ZSe7zFYHj7C6ul7TjeLQeZD_YwD66t62wDmpe_HlB-TnBA"
                "-njbglfIsRLtXlnDzQkv5dTltRJ11BKBBypeeF6689rjcJIDEz9RWdc\","
                "\"dp\":\"BwKfV3Akq5_MFZDFZCnW-wzl-CCo83WoZvnLQwCTeDv8uzluRSnm71I3Q"
                "CLdhrqE2e9YkxvuxdBfpT_PI7Yz-FOKnu1R6HsJeDCjn12Sk3vmAktV2zb"
                "34MCdy7cpdTh_YVr7tss2u6vneTwrA86rZtu5Mbr1C1XsmvkxHQAdYo0\","
                "\"dq\":\"h_96-mK1R_7glhsum81dZxjTnYynPbZpHziZjeeHcXYsXaaMwkOlODsWa"
                "7I9xXDoRwbKgB719rrmI2oKr6N3Do9U0ajaHF-NKJnwgjMd2w9cjz3_-ky"
                "NlxAr2v4IKhGNpmM5iIgOS1VZnOZ68m6_pbLBSp3nssTdlqvd0tIiTHU\","
                "\"qi\":\"IYd7DHOhrWvxkwPQsRM2tOgrjbcrfvtQJipd-DlcxyVuuM9sQLdgjVk2o"
                "y26F0EmpScGLq2MowX7fhd_QJQ3ydy5cY7YIBi87w93IKLEdfnbJtoOPLU"
                "W0ITrJReOgo1cq9SbsxYawBgfp_gh6A5603k2-ZQwVK0JKSHuLFkuQ3U\""
                "}"
                );
            
            rsa_key1 = JWKMgr :: parseJWK ( key_json );
        }

        void makeECDSAKey1 ()
        {
            // the EC key in JSON text
            String key_json (
                "{"
                "\"alg\":\"ES256\","
                "\"kty\":\"EC\","
                "\"kid\":\"ecdsa-key-1\","
                "\"use\":\"sig\","
                "\"crv\":\"P-256\","
                "\"x\":\"f83OJ3D2xF1Bg8vub9tLe1gHMzV76e8Tus9uPHvRVEU\","
                "\"y\":\"x_FEzRu9m36HLN_tue659LNpXW6pCyStikYjKIWI5a0\","
                "\"d\":\"jpsQnnGQmL-YBIffH1136cspYG6-0iY7X1fCE9-E9LI\""
                "}"
                );
            
            ecdsa_key1 = JWKMgr :: parseJWK ( key_json );
        }

        void makeKeyset1 ()
        {
            keyset1 = JWKMgr :: makeJWKSet ();
            keyset1 -> addKey ( rsa_key1 );
            keyset1 -> addKey ( ecdsa_key1 );
        }

        void clearPolicy ()
        {
            JWSMgr :: Policy p = JWSMgr :: getPolicy ();
            p . kid_required = false;
            p . kid_gen = false;
            p . require_simple_hdr = false;
            p . require_prestored_kid = false;
            p . require_prestored_key = false;
            JWSMgr :: setPolicy ( p );
        }

        void strictPolicy ()
        {
            JWSMgr :: Policy p = JWSMgr :: getPolicy ();
            p . kid_required = true;
            p . kid_gen = true;
            p . require_simple_hdr = true;
            p . require_prestored_kid = true;
            p . require_prestored_key = true;
            JWSMgr :: setPolicy ( p );
        }

        virtual void SetUp () override
        {
            makeRSAKey1 ();
            makeECDSAKey1 ();
            makeKeyset1 ();
        }

        virtual void TearDown () override
        {
        }

        virtual ~ JWSMgrTest () override
        {
        }

    protected:

        JWKRef rsa_key1;
        JWKRef ecdsa_key1;

        JWKSetRef keyset1;
    };


    TEST_F ( JWSMgrTest, hdr_reserved )
    {
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "alg" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "kid" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "typ" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "cty" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "crit" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "jku" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "jwk" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "x5u" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "x5c" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "x5t" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "x5t#256" ) );
        EXPECT_TRUE ( JWSMgr :: hdrReserved ( "b64" ) );

        EXPECT_FALSE ( JWSMgr :: hdrReserved ( "Alg" ) );
        EXPECT_FALSE ( JWSMgr :: hdrReserved ( "ALG" ) );
        EXPECT_FALSE ( JWSMgr :: hdrReserved ( "b62" ) );
        EXPECT_FALSE ( JWSMgr :: hdrReserved ( "B64" ) );
    }

    TEST_F ( JWSMgrTest, sign_compact_RFC7515_A_2 )
    {
        clearPolicy ();

        // a problem with these tests is that the RFC was written using CR-LF
        // and particular white-space, which must be duplicated exactly in order
        // to get the exact same JWS...
        String payload (
            "{\"iss\":\"joe\",\r\n"
            " \"exp\":1300819380,\r\n"
            " \"http://example.com/is_root\":true}"
            );
        JWS jws;
        EXPECT_NO_THROW ( jws = JWSMgr :: signCompact ( * rsa_key1, payload . data (), payload . size () ) );

        JWS expected (
            "eyJhbGciOiJSUzI1NiJ9"
            "."
            "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt"
            "cGxlLmNvbS9pc19yb290Ijp0cnVlfQ"
            "."
            "cC4hiUPoj9Eetdgtv3hF80EGrhuB__dzERat0XF9g2VtQgr9PJbu3XOiZj5RZmh7"
            "AAuHIm4Bh-0Qc_lF5YKt_O8W2Fp5jujGbds9uJdbF9CUAr7t1dnZcAcQjbKBYNX4"
            "BAynRFdiuB--f_nZLgrnbyTyWzO75vRK5h6xBArLIARNPvkSjtQBMHlb1L07Qe7K"
            "0GarZRmB_eSN9383LcOLn6_dO--xi12jzDwusC-eOkHWEsqtFZESc6BfI7noOPqv"
            "hJ1phCnvWh6IeYI2w9QOYEUipUTI8np6LbgGY9Fs98rqVt5AXLIhWkWywlVmtVrB"
            "p0igcN_IoypGlUPQGe77Rw"
            );
        EXPECT_EQ ( jws, expected );
    }

    TEST_F ( JWSMgrTest, extract_RFC7515_A_2 )
    {
        clearPolicy ();

        JWS jws (
            "eyJhbGciOiJSUzI1NiJ9"
            "."
            "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt"
            "cGxlLmNvbS9pc19yb290Ijp0cnVlfQ"
            "."
            "cC4hiUPoj9Eetdgtv3hF80EGrhuB__dzERat0XF9g2VtQgr9PJbu3XOiZj5RZmh7"
            "AAuHIm4Bh-0Qc_lF5YKt_O8W2Fp5jujGbds9uJdbF9CUAr7t1dnZcAcQjbKBYNX4"
            "BAynRFdiuB--f_nZLgrnbyTyWzO75vRK5h6xBArLIARNPvkSjtQBMHlb1L07Qe7K"
            "0GarZRmB_eSN9383LcOLn6_dO--xi12jzDwusC-eOkHWEsqtFZESc6BfI7noOPqv"
            "hJ1phCnvWh6IeYI2w9QOYEUipUTI8np6LbgGY9Fs98rqVt5AXLIhWkWywlVmtVrB"
            "p0igcN_IoypGlUPQGe77Rw"
            );
        JWKRef key;
        Payload payload;
        JSONObjectRef jose;
        EXPECT_NO_THROW ( key = JWSMgr :: extract ( jose, payload, * keyset1, jws ) );

        String expected (
            "{\"iss\":\"joe\",\r\n"
            " \"exp\":1300819380,\r\n"
            " \"http://example.com/is_root\":true}"
            );
        EXPECT_EQ ( payload . size (), expected . size () );
        EXPECT_EQ ( memcmp ( payload . data (), expected . data (), payload . size () ), 0 );

        EXPECT_FALSE ( ! key );
        EXPECT_EQ ( & * key, & * rsa_key1 );
    }

    TEST_F ( JWSMgrTest, sign_compact_RFC7515_A_3 )
    {
        clearPolicy ();

        // a problem with these tests is that the RFC was written using CR-LF
        // and particular white-space, which must be duplicated exactly in order
        // to get the exact same JWS...
        String payload (
            "{\"iss\":\"joe\",\r\n"
            " \"exp\":1300819380,\r\n"
            " \"http://example.com/is_root\":true}"
            );
        JWS jws;
        EXPECT_NO_THROW ( jws = JWSMgr :: signCompact ( * ecdsa_key1, payload . data (), payload . size () ) );

        JWS expected (
            "eyJhbGciOiJFUzI1NiJ9"
            "."
            "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt"
            "cGxlLmNvbS9pc19yb290Ijp0cnVlfQ"
            "."
            "DtEhU3ljbEg8L38VWAfUAqOyKAM6-Xx-F4GawxaepmXFCgfTjDxw5djxLa8ISlSA"
            "pmWQxfKTUJqPP3-Kg6NU1Q"
            );
        EXPECT_EQ ( jws . size (), expected . size () );
    }

    TEST_F ( JWSMgrTest, extract_RFC7515_A_3 )
    {
        clearPolicy ();

        JWS jws (
            "eyJhbGciOiJFUzI1NiJ9"
            "."
            "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt"
            "cGxlLmNvbS9pc19yb290Ijp0cnVlfQ"
            "."
            "DtEhU3ljbEg8L38VWAfUAqOyKAM6-Xx-F4GawxaepmXFCgfTjDxw5djxLa8ISlSA"
            "pmWQxfKTUJqPP3-Kg6NU1Q"
            );
        JWKRef key;
        Payload payload;
        JSONObjectRef jose;
        EXPECT_NO_THROW ( key = JWSMgr :: extract ( jose, payload, * keyset1, jws ) );

        String expected (
            "{\"iss\":\"joe\",\r\n"
            " \"exp\":1300819380,\r\n"
            " \"http://example.com/is_root\":true}"
            );
        EXPECT_EQ ( payload . size (), expected . size () );
        EXPECT_EQ ( memcmp ( payload . data (), expected . data (), payload . size () ), 0 );

        EXPECT_FALSE ( ! key );
        EXPECT_EQ ( & * key, & * ecdsa_key1 );
    }

    TEST_F ( JWSMgrTest, extract_dflt_policy_RFC7515_A_3 )
    {
        JWS jws (
            "eyJhbGciOiJFUzI1NiJ9"
            "."
            "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt"
            "cGxlLmNvbS9pc19yb290Ijp0cnVlfQ"
            "."
            "DtEhU3ljbEg8L38VWAfUAqOyKAM6-Xx-F4GawxaepmXFCgfTjDxw5djxLa8ISlSA"
            "pmWQxfKTUJqPP3-Kg6NU1Q"
            );
        JWKRef key;
        Payload payload;
        JSONObjectRef jose;
        EXPECT_NO_THROW ( key = JWSMgr :: extract ( jose, payload, * keyset1, jws ) );
    }

    TEST_F ( JWSMgrTest, extract_strict_policy_RFC7515_A_3 )
    {
        strictPolicy ();

        JWS jws (
            "eyJhbGciOiJFUzI1NiJ9"
            "."
            "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt"
            "cGxlLmNvbS9pc19yb290Ijp0cnVlfQ"
            "."
            "DtEhU3ljbEg8L38VWAfUAqOyKAM6-Xx-F4GawxaepmXFCgfTjDxw5djxLa8ISlSA"
            "pmWQxfKTUJqPP3-Kg6NU1Q"
            );
        JWKRef key;
        Payload payload;
        JSONObjectRef jose;
        EXPECT_THROW ( key = JWSMgr :: extract ( jose, payload, * keyset1, jws ), JWSMissingKeyIdInHeader );
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
