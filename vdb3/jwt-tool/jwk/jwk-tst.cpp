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

#undef JWK_TESTING
#define JWK_TESTING 1

#include "jwk-mgr.cpp"
#include "jwk-key.cpp"
#include "jwk-set.cpp"

#include <string>
#include <cstring>

namespace ncbi
{
    class DummyJWASigner : public JWASigner
    {
    public:

        virtual Payload sign ( const JWK & key,
            const void * data, size_t bytes ) const override
        {
            return Payload ();
        }

        static String _getKeyProp ( const JWK & key, const String & name )
        {
            return JWASigner :: getKeyProp ( key, name );
        }
    };

    /*=====================================================*
     *                       JWKMgr                        *
     *=====================================================*/

    TEST ( JWKMgrTest, get_invalid_key )
    {
        JWKRef key;
        EXPECT_NO_THROW ( key = JWKMgr :: getInvalidKey () );
        EXPECT_NE ( key, nullptr );
    }

    TEST ( JWKMgrTest, make_jwk_set )
    {
        JWKSetRef keyset;
        EXPECT_NO_THROW ( keyset = JWKMgr :: makeJWKSet () );
    }

    // only usable if HMAC signing is allowed
#if JWA_TESTING
    TEST ( JWASigner, rfc7515_example_A_1 )
    {
        // concatenation of header and payload, base64url-encoded
        String contents (
            "eyJ0eXAiOiJKV1QiLA0KICJhbGciOiJIUzI1NiJ9"
            "."
            "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt"
            "cGxlLmNvbS9pc19yb290Ijp0cnVlfQ"
            );

        unsigned char expected_cont_bin [] =
        {
            101, 121, 74, 48, 101, 88, 65, 105, 79, 105, 74, 75, 86, 49, 81,
            105, 76, 65, 48, 75, 73, 67, 74, 104, 98, 71, 99, 105, 79, 105, 74,
            73, 85, 122, 73, 49, 78, 105, 74, 57, 46, 101, 121, 74, 112, 99, 51,
            77, 105, 79, 105, 74, 113, 98, 50, 85, 105, 76, 65, 48, 75, 73, 67,
            74, 108, 101, 72, 65, 105, 79, 106, 69, 122, 77, 68, 65, 52, 77, 84,
            107, 122, 79, 68, 65, 115, 68, 81, 111, 103, 73, 109, 104, 48, 100,
            72, 65, 54, 76, 121, 57, 108, 101, 71, 70, 116, 99, 71, 120, 108, 76,
            109, 78, 118, 98, 83, 57, 112, 99, 49, 57, 121, 98, 50, 57, 48, 73,
            106, 112, 48, 99, 110, 86, 108, 102, 81
        };
        EXPECT_EQ ( contents . size (), sizeof expected_cont_bin );
        EXPECT_EQ ( memcmp ( contents . data (), expected_cont_bin, sizeof expected_cont_bin ), 0 );

        // the symmetric key in JSON text
        String key_json (
            "{"
                "\"kty\":\"oct\","
                "\"kid\":\"123\","
                "\"k\":\"AyM1SysPpbyDfgZld3umj1qzKObwVMkoqQ-EstJQLr_T-1qS0gZH75aKtMN3Yj0iPS4hcgUuTwjAzZr1Z9CAow\""
            "}"
            );

        JWKRef key;
        EXPECT_NO_THROW ( key = JWKMgr :: parseJWK ( key_json ) );

        // get signer
        JWASignerRef signer;
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "HS256" ) );

        // sign
        Payload signature;
        EXPECT_NO_THROW ( signature = signer -> sign ( * key,
            contents . data (), contents . size () ) );

        // expected binary signature value
        unsigned char expected_sig_bin [] =
        {
            116, 24, 223, 180, 151, 153, 224, 37, 79, 250, 96, 125, 216, 173,
            187, 186, 22, 212, 37, 77, 105, 214, 191, 240, 91, 88, 5, 88, 83,
            132, 141, 121
        };
        EXPECT_EQ ( signature . size (), sizeof expected_sig_bin );
        EXPECT_EQ ( memcmp ( signature . data (), expected_sig_bin, sizeof expected_sig_bin ), 0 );

        // verify
        JWAVerifierRef verifier;
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "HS256" ) );

        bool match;
        EXPECT_NO_THROW ( match = verifier -> verify ( * key,
            contents . data (), contents . size (), signature ) );
        EXPECT_EQ ( match, true );
    }
#endif

    TEST ( JWASigner, rfc7515_example_A_2 )
    {
        // concatenation of header and payload, base64url-encoded
        String contents (
            "eyJhbGciOiJSUzI1NiJ9"
            "."
            "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt"
            "cGxlLmNvbS9pc19yb290Ijp0cnVlfQ"
            );

        unsigned char expected_cont_bin [] =
        {
            101, 121, 74, 104, 98, 71, 99, 105, 79, 105, 74, 83, 85, 122, 73,
            49, 78, 105, 74, 57, 46, 101, 121, 74, 112, 99, 51, 77, 105, 79, 105,
            74, 113, 98, 50, 85, 105, 76, 65, 48, 75, 73, 67, 74, 108, 101, 72,
            65, 105, 79, 106, 69, 122, 77, 68, 65, 52, 77, 84, 107, 122, 79, 68,
            65, 115, 68, 81, 111, 103, 73, 109, 104, 48, 100, 72, 65, 54, 76,
            121, 57, 108, 101, 71, 70, 116, 99, 71, 120, 108, 76, 109, 78, 118,
            98, 83, 57, 112, 99, 49, 57, 121, 98, 50, 57, 48, 73, 106, 112, 48,
            99, 110, 86, 108, 102, 81
        };
        EXPECT_EQ ( contents . size (), sizeof expected_cont_bin );
        EXPECT_EQ ( memcmp ( contents . data (), expected_cont_bin, sizeof expected_cont_bin ), 0 );

        // the RSA key in JSON text
        String key_json (
            "{"
            "\"kty\":\"RSA\","
            "\"kid\":\"key123\","
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

        JWKRef key;
        EXPECT_NO_THROW ( key = JWKMgr :: parseJWK ( key_json ) );

        // get signer
        JWASignerRef signer;
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "RS256" ) );

        // sign
        Payload signature;
        EXPECT_NO_THROW ( signature = signer -> sign ( * key,
            contents . data (), contents . size () ) );

        // expected binary signature value
        unsigned char expected_sig_bin [] =
        {
            112, 46, 33, 137, 67, 232, 143, 209, 30, 181, 216, 45, 191, 120, 69,
            243, 65, 6, 174, 27, 129, 255, 247, 115, 17, 22, 173, 209, 113, 125,
            131, 101, 109, 66, 10, 253, 60, 150, 238, 221, 115, 162, 102, 62, 81,
            102, 104, 123, 0, 11, 135, 34, 110, 1, 135, 237, 16, 115, 249, 69,
            229, 130, 173, 252, 239, 22, 216, 90, 121, 142, 232, 198, 109, 219,
            61, 184, 151, 91, 23, 208, 148, 2, 190, 237, 213, 217, 217, 112, 7,
            16, 141, 178, 129, 96, 213, 248, 4, 12, 167, 68, 87, 98, 184, 31,
            190, 127, 249, 217, 46, 10, 231, 111, 36, 242, 91, 51, 187, 230, 244,
            74, 230, 30, 177, 4, 10, 203, 32, 4, 77, 62, 249, 18, 142, 212, 1,
            48, 121, 91, 212, 189, 59, 65, 238, 202, 208, 102, 171, 101, 25, 129,
            253, 228, 141, 247, 127, 55, 45, 195, 139, 159, 175, 221, 59, 239,
            177, 139, 93, 163, 204, 60, 46, 176, 47, 158, 58, 65, 214, 18, 202,
            173, 21, 145, 18, 115, 160, 95, 35, 185, 232, 56, 250, 175, 132, 157,
            105, 132, 41, 239, 90, 30, 136, 121, 130, 54, 195, 212, 14, 96, 69,
            34, 165, 68, 200, 242, 122, 122, 45, 184, 6, 99, 209, 108, 247, 202,
            234, 86, 222, 64, 92, 178, 33, 90, 69, 178, 194, 85, 102, 181, 90,
            193, 167, 72, 160, 112, 223, 200, 163, 42, 70, 149, 67, 208, 25, 238,
            251, 71
        };
        EXPECT_EQ ( signature . size (), sizeof expected_sig_bin );
        EXPECT_EQ ( memcmp ( signature . data (), expected_sig_bin, sizeof expected_sig_bin ), 0 );

        // verify
        JWAVerifierRef verifier;
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "RS256" ) );

        bool match;
        EXPECT_NO_THROW ( match = verifier -> verify ( * key,
            contents . data (), contents . size (), signature ) );
        EXPECT_EQ ( match, true );
    }

    TEST ( JWASigner, rfc7515_example_A_3 )
    {
        // concatenation of header and payload, base64url-encoded
        String contents (
            "eyJhbGciOiJFUzI1NiJ9"
            "."
            "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt"
            "cGxlLmNvbS9pc19yb290Ijp0cnVlfQ"
            );

        unsigned char expected_cont_bin [] =
        {
            101, 121, 74, 104, 98, 71, 99, 105, 79, 105, 74, 70, 85, 122, 73,
            49, 78, 105, 74, 57, 46, 101, 121, 74, 112, 99, 51, 77, 105, 79, 105,
            74, 113, 98, 50, 85, 105, 76, 65, 48, 75, 73, 67, 74, 108, 101, 72,
            65, 105, 79, 106, 69, 122, 77, 68, 65, 52, 77, 84, 107, 122, 79, 68,
            65, 115, 68, 81, 111, 103, 73, 109, 104, 48, 100, 72, 65, 54, 76,
            121, 57, 108, 101, 71, 70, 116, 99, 71, 120, 108, 76, 109, 78, 118,
            98, 83, 57, 112, 99, 49, 57, 121, 98, 50, 57, 48, 73, 106, 112, 48,
            99, 110, 86, 108, 102, 81
        };
        EXPECT_EQ ( contents . size (), sizeof expected_cont_bin );
        EXPECT_EQ ( memcmp ( contents . data (), expected_cont_bin, sizeof expected_cont_bin ), 0 );

        // the elliptic key in JSON text
        String key_json (
            "{"
            "\"kty\":\"EC\","
            "\"kid\":\"key123\","
            "\"use\":\"sig\","

            "\"crv\":\"P-256\","
            "\"x\":\"f83OJ3D2xF1Bg8vub9tLe1gHMzV76e8Tus9uPHvRVEU\","
            "\"y\":\"x_FEzRu9m36HLN_tue659LNpXW6pCyStikYjKIWI5a0\","
            "\"d\":\"jpsQnnGQmL-YBIffH1136cspYG6-0iY7X1fCE9-E9LI\""

            "}"
            );

        JWKRef key;
        EXPECT_NO_THROW ( key = JWKMgr :: parseJWK ( key_json ) );

        // example binary signature value
        // from RFC-7520:
        //   "Note that ECDSA uses random data to generate the signature;
        //    it might not be possible to exactly replicate the results
        //    in this section."
        unsigned char example_sig_bin [] =
        {
            // "R"
            14, 209, 33, 83, 121, 99, 108, 72, 60, 47, 127, 21, 88,
            7, 212, 2, 163, 178, 40, 3, 58, 249, 124, 126, 23, 129,
            154, 195, 22, 158, 166, 101,
            // "S"
            197, 10, 7, 211, 140, 60, 112, 229, 216, 241, 45, 175,  
            8, 74, 84, 128, 166, 101, 144, 197, 242, 147, 80, 154,   
            143, 63, 127, 138, 131, 163, 84, 213
        };

        Payload signature ( sizeof example_sig_bin );
        memmove ( signature . data (), example_sig_bin, sizeof example_sig_bin );
        signature . setSize ( sizeof example_sig_bin );

        // get verifier
        JWAVerifierRef verifier;
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "ES256" ) );

        // check that our verifier will accept the signature given in example
        // of RFC7515 A.3
        bool match;
        EXPECT_NO_THROW ( match = verifier -> verify ( * key,
            contents . data (), contents . size (), signature ) );
        EXPECT_EQ ( match, true );

        // get signer
        JWASignerRef signer;
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "ES256" ) );

        // sign
        EXPECT_NO_THROW ( signature = signer -> sign ( * key,
            contents . data (), contents . size () ) );
        EXPECT_EQ ( signature . size (), sizeof example_sig_bin );

        // verify that our signature also verifies
        EXPECT_NO_THROW ( match = verifier -> verify ( * key,
            contents . data (), contents . size (), signature ) );
        EXPECT_EQ ( match, true );
    }

    TEST ( JWASigner, rfc7515_example_A_4 )
    {
        // concatenation of header and payload, base64url-encoded
        String contents (
            "eyJhbGciOiJFUzUxMiJ9"
            "."
            "UGF5bG9hZA"
            );

        unsigned char expected_cont_bin [] =
        {
            101, 121, 74, 104, 98, 71, 99, 105, 79, 105, 74, 70, 85, 122, 85,
            120, 77, 105, 74, 57, 46, 85, 71, 70, 53, 98, 71, 57, 104, 90, 65
        };
        EXPECT_EQ ( contents . size (), sizeof expected_cont_bin );
        EXPECT_EQ ( memcmp ( contents . data (), expected_cont_bin, sizeof expected_cont_bin ), 0 );

        // the elliptic key in JSON text
        String key_json (
            "{"
            "\"kty\":\"EC\","
            "\"kid\":\"key123\","
            "\"use\":\"sig\","

            "\"crv\":\"P-521\","
            "\"x\":\"AekpBQ8ST8a8VcfVOTNl353vSrDCLLJXmPk06wTjxrrjcBpXp5EOnYG_"
                    "NjFZ6OvLFV1jSfS9tsz4qUxcWceqwQGk\","
            "\"y\":\"ADSmRA43Z1DSNx_RvcLI87cdL07l6jQyyBXMoxVg_l2Th-x3S1WDhjDl"
                    "y79ajL4Kkd0AZMaZmh9ubmf63e3kyMj2\","
            "\"d\":\"AY5pb7A0UFiB3RELSD64fTLOSV_jazdF7fLYyuTw8lOfRhWg6Y6rUrPA"
                     "xerEzgdRhajnu0ferB0d53vM9mE15j2C\""

            "}"
            );

        JWKRef key;
        EXPECT_NO_THROW ( key = JWKMgr :: parseJWK ( key_json ) );

        // example binary signature value
        // from RFC-7520:
        //   "Note that ECDSA uses random data to generate the signature;
        //    it might not be possible to exactly replicate the results
        //    in this section."
        unsigned char example_sig_bin [] =
        {
            // "R"
            1, 220, 12, 129, 231, 171, 194, 209, 232, 135, 233,     
            117, 247, 105, 122, 210, 26, 125, 192, 1, 217, 21, 82,   
            91, 45, 240, 255, 83, 19, 34, 239, 71, 48, 157, 147,     
            152, 105, 18, 53, 108, 163, 214, 68, 231, 62, 153, 150,  
            106, 194, 164, 246, 72, 143, 138, 24, 50, 129, 223, 133, 
            206, 209, 172, 63, 237, 119, 109,
            // "S"
            0, 111, 6, 105, 44, 5, 41, 208, 128, 61, 152, 40, 92,   
            61, 152, 4, 150, 66, 60, 69, 247, 196, 170, 81, 193,     
            199, 78, 59, 194, 169, 16, 124, 9, 143, 42, 142, 131,    
            48, 206, 238, 34, 175, 83, 203, 220, 159, 3, 107, 155,   
            22, 27, 73, 111, 68, 68, 21, 238, 144, 229, 232, 148,    
            188, 222, 59, 242, 103
        };

        Payload signature ( sizeof example_sig_bin );
        memmove ( signature . data (), example_sig_bin, sizeof example_sig_bin );
        signature . setSize ( sizeof example_sig_bin );

        // get verifier
        JWAVerifierRef verifier;
        EXPECT_NO_THROW ( verifier = JWAMgr :: getVerifier ( "ES512" ) );

        // check that our verifier will accept the signature given in example
        // of RFC7515 A.3
        bool match;
        EXPECT_NO_THROW ( match = verifier -> verify ( * key,
            contents . data (), contents . size (), signature ) );
        EXPECT_EQ ( match, true );

        // get signer
        JWASignerRef signer;
        EXPECT_NO_THROW ( signer = JWAMgr :: getSigner ( "ES512" ) );

        // sign
        EXPECT_NO_THROW ( signature = signer -> sign ( * key,
            contents . data (), contents . size () ) );
        EXPECT_EQ ( signature . size (), sizeof example_sig_bin );

        // verify that our signature also verifies
        EXPECT_NO_THROW ( match = verifier -> verify ( * key,
            contents . data (), contents . size (), signature ) );
        EXPECT_EQ ( match, true );
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
