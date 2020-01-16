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

#include <klib/debug.h> /* KDbgSetString */
#include <ktst/unit_test.hpp> // TEST_SUITE
#include <vfs/path.h> /* VPathRelease */
#include <vfs/services-priv.h> /* KServiceTestNamesExecuteExt */

#include "../../libs/vfs/json-response.h" /* Response4MakeSdl */

using std::string;

TEST_SUITE(TestSdlResolver)

/*static rc_t validate ( const char * input ) {
    KJsonValue * root = NULL;
    rc_t rc = KJsonValueMake ( & root, input, NULL, 0 );
    KJsonValueWhack ( root );
    return rc;
}*/

#define ALL

#ifdef ALL
TEST_CASE(testArrayExample) {
    Response4 * response = NULL;

    REQUIRE_RC      ( Response4MakeSdl ( & response,
"{"
    "\"version\": \"2\","
    "\"result\": ["
        "{"
            "\"bundle\": \"SRR850901\","
            "\"status\": 200,"
            "\"msg\": \"ok\","
            "\"files\": ["
                "{"
                    "\"object\": \"srapub|SRR850901\","
                    "\"type\": \"sra\","
                    "\"name\": \"SRR850901\","
                    "\"size\": 323741972,"
                    "\"md5\": \"5e213b2319bd1af17c47120ee8b16dbc\","
                    "\"modificationDate\": \"2016-11-19T07:35:20Z\","
                    "\"locations\": ["
                        "{"
                            "\"link\": \"https://sra-download.ncbi.nlm.nih.gov/traces/sra3/SRR/000830/SRR850901\","
                            "\"service\": \"sra-ncbi\","
                            "\"region\": \"public\""
                        "}"
                    "]"
                "}"
            "]"
        "}"
    "]"
"}"
) );
    REQUIRE_RC      ( Response4Release ( response ) );
}
#endif

#ifdef ALL
TEST_CASE(testExample) {
    Response4 * response = NULL;

    REQUIRE_RC      ( Response4MakeSdl ( & response,
"{"
    "\"version\": \"2\","
    "\"result\": ["
        "{"
            "\"bundle\": \"SRR850901\","
            "\"status\": 200,"
            "\"msg\": \"ok\","
            "\"files\": ["
                "{"
                    "\"object\": \"srapub|SRR850901\","
                    "\"type\": \"sra\","
                    "\"name\": \"SRR850901\","
                    "\"size\": 323741972,"
                    "\"md5\": \"5e213b2319bd1af17c47120ee8b16dbc\","
                    "\"modificationDate\": \"2016-11-19T07:35:20Z\","
                    "\"link\": \"https://sra-download.ncbi.nlm.nih.gov/traces/sra3/SRR/000830/SRR850901\","
                    "\"service\": \"sra-ncbi\","
                    "\"region\": \"public\""
                "},"
                "{"
                    "\"object\": \"srapub|SRR850901\","
                    "\"type\": \"vdbcache\","
                    "\"name\": \"SRR850901.vdbcache\","
                    "\"size\": 17615526,"
                    "\"md5\": \"2eb204cedf5eefe45819c228817ee466\","
                    "\"modificationDate\": \"2016-02-08T16:31:35Z\","
                    "\"link\": \"https://sra-download.ncbi.nlm.nih.gov/traces/sra11/SRR/000830/SRR850901.vdbcache\","
                    "\"service\": \"sra-ncbi\","
                    "\"region\": \"public\""
                "}"
            "]"
        "}"
    "]"
"}"
) );
    REQUIRE_RC      ( Response4Release ( response ) );
}
#endif

#ifdef ALL
TEST_CASE(doubleWhack) {
    Response4 * response = NULL;
    REQUIRE_RC_FAIL(Response4MakeSdl(&response, "{}"));
    REQUIRE_RC(Response4Release(response));
}
#endif

#ifdef ALL
TEST_CASE(testNoNextToken) {
    const char resl[] =
        "{"
        " \"result\" : ["
        "  {"
        "   \"files\" : ["
        "    {"
        "     \"locations\" : ["
        "      {"
        "       \"link\": \"https://sdownload.ncbi.nlm.nih.gov/sra/SRR045450\""
        "      }"
        "     ]"
        "    }"
        "   ]"
        "  },"
        "  {"
        "   \"files\" : ["
        "    {"
        "     \"locations\" : ["
        "      {"
        "       \"link\": \"https://sdownload.ncbi.nlm.nih.gov/sra/SRR053325\""
        "      }"
        "     ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}"; // nextToken is not set
    KService * service = NULL;
    REQUIRE_RC(KServiceMake(&service));
    REQUIRE_RC_FAIL(KServiceTestNamesExecuteExt(service, 0, 0, 0, NULL, resl));
    const KSrvResponse * rspn = NULL;
    REQUIRE_RC_FAIL(KServiceTestNamesExecuteExt(service, 0, 0, 0, &rspn, resl));
    REQUIRE_RC(KServiceAddId(service, "SRR045450"));
    REQUIRE_RC(KServiceAddId(service, "SRR053325"));
    REQUIRE_RC(KServiceTestNamesExecuteExt(service, 0, 0, 0, &rspn, resl));
    REQUIRE_EQ(KSrvResponseLength(rspn), (uint32_t)2);
    for (uint32_t i = 0; i < KSrvResponseLength(rspn); ++i) {
        const KSrvRespObj * obj = NULL;
        REQUIRE_RC(KSrvResponseGetObjByIdx(rspn, 0, &obj));
        KSrvRespObjIterator * it = NULL;
        REQUIRE_RC(KSrvRespObjMakeIterator(obj, &it));
        int files = 0;
        while (true) {
            KSrvRespFile * file = NULL;
            REQUIRE_RC(KSrvRespObjIteratorNextFile(it, &file));
            if (file == NULL)
                break;
            ++files;
            KSrvRespFileIterator * fi = NULL;
            REQUIRE_RC(KSrvRespFileMakeIterator(file, &fi));
            int paths = 0;
            while (true) {
                const VPath * path = NULL;
                REQUIRE_RC(KSrvRespFileIteratorNextPath(fi, &path));
                if (path == NULL)
                    break;
                REQUIRE_RC(VPathRelease(path));
                ++paths;
            }
            REQUIRE_EQ(paths, 1);
            REQUIRE_RC(KSrvRespFileIteratorRelease(fi));
            REQUIRE_RC(KSrvRespFileRelease(file));
        }
        REQUIRE_EQ(files, 1);
        REQUIRE_RC(KSrvRespObjIteratorRelease(it));
        REQUIRE_RC(KSrvRespObjRelease(obj));
    }

    // nextToken is not set
    const char * nextToken = NULL;
    REQUIRE_RC(KSrvResponseGetNextToken(rspn, &nextToken));
    REQUIRE_NULL(nextToken);

    REQUIRE_RC(KSrvResponseRelease(rspn));

    const char * json = KServiceGetResponseCStr(service);
    REQUIRE_NOT_NULL(json);
    REQUIRE_EQ(string(resl), string(json));

    REQUIRE_RC(KServiceRelease(service));
}
#endif

#ifdef ALL
TEST_CASE(testNextToken) {
    const char resl[] =
        "{"
        " \"result\" : ["
        "  {"
        "   \"files\" : ["
        "    {"
        "     \"locations\" : ["
        "      {"
        "       \"link\": \"https://sdownload.ncbi.nlm.nih.gov/sra/SRR045450\""
        "      }"
        "     ]"
        "    }"
        "   ]"
        "  },"
        "  {"
        "   \"files\" : ["
        "    {"
        "     \"locations\" : ["
        "      {"
        "       \"link\": \"https://sdownload.ncbi.nlm.nih.gov/sra/SRR053325\""
        "      }"
        "     ]"
        "    }"
        "   ]"
        "  }"
        " ], "
        " \"nextToken\": \"blah\""
        "}"; // nextToken is set
    KService * service = NULL;
    REQUIRE_RC(KServiceMake(&service));
    REQUIRE_RC_FAIL(KServiceTestNamesExecuteExt(service, 0, 0, 0, NULL, resl));
    const KSrvResponse * rspn = NULL;
    REQUIRE_RC_FAIL(KServiceTestNamesExecuteExt(service, 0, 0, 0, &rspn, resl));
    REQUIRE_RC(KServiceAddId(service, "SRR045450"));
    REQUIRE_RC(KServiceAddId(service, "SRR053325"));
    REQUIRE_RC(KServiceAddId(service, "SRR056386"));
    REQUIRE_RC(KServiceTestNamesExecuteExt(service, 0, 0, 0, &rspn, resl));
    REQUIRE_EQ(KSrvResponseLength(rspn), (uint32_t)2);
    for (uint32_t i = 0; i < KSrvResponseLength(rspn); ++i) {
        const KSrvRespObj * obj = NULL;
        REQUIRE_RC(KSrvResponseGetObjByIdx(rspn, 0, &obj));
        KSrvRespObjIterator * it = NULL;
        REQUIRE_RC(KSrvRespObjMakeIterator(obj, &it));
        int files = 0;
        while (true) {
            KSrvRespFile * file = NULL;
            REQUIRE_RC(KSrvRespObjIteratorNextFile(it, &file));
            if (file == NULL)
                break;
            ++files;
            KSrvRespFileIterator * fi = NULL;
            REQUIRE_RC(KSrvRespFileMakeIterator(file, &fi));
            int paths = 0;
            while (true) {
                const VPath * path = NULL;
                REQUIRE_RC(KSrvRespFileIteratorNextPath(fi, &path));
                if (path == NULL)
                    break;
                REQUIRE_RC(VPathRelease(path));
                ++paths;
            }
            REQUIRE_EQ(paths, 1);
            REQUIRE_RC(KSrvRespFileIteratorRelease(fi));
            REQUIRE_RC(KSrvRespFileRelease(file));
        }
        REQUIRE_EQ(files, 1);
        REQUIRE_RC(KSrvRespObjIteratorRelease(it));
        REQUIRE_RC(KSrvRespObjRelease(obj));
    }

    // nextToken is set but currently cannot be processed
    const char * nextToken = NULL;
    REQUIRE_RC_FAIL(KSrvResponseGetNextToken(rspn, &nextToken));
    REQUIRE_NOT_NULL(nextToken);

    REQUIRE_RC(KSrvResponseRelease(rspn));

    const char * json = KServiceGetResponseCStr(service);
    REQUIRE_NOT_NULL(json);
    REQUIRE_EQ(string(resl), string(json));

    REQUIRE_RC(KServiceRelease(service));
}
#endif

#ifdef ALL
TEST_CASE(nullMsg) {
    Response4 * response = NULL;

    REQUIRE_RC_FAIL(Response4MakeSdl(&response,
        "{"
        "    \"status\": 200"
        "}"));

    REQUIRE_RC(Response4Release(response));
}
#endif

#ifdef ALL
TEST_CASE(noMsg) {
    Response4 * response = NULL;

    std::cerr << "vvvvvvvvvvvvvv expect to see error message vvvvvvvvvvvvvvv\n";
    REQUIRE_RC(Response4MakeSdl(&response,
        "{"
        "    \"status\": 200,"
        "    \"message\" : \"\""
        "}"));

    rc_t rc = 0;
    REQUIRE_RC(Response4GetRc(response, &rc));
    REQUIRE_RC_FAIL(rc);

    REQUIRE_RC(Response4Release(response));
}
#endif

#ifdef ALL
TEST_CASE(noMsgWithRes) {
    Response4 * response = NULL;

    REQUIRE_RC(Response4MakeSdl(&response,
        "{"
        "  \"status\": 200,"
        "  \"result\": ["
        "    {"
        "    }"
        "  ]"
        "}"));

    REQUIRE_RC(Response4Release(response));
}
#endif

#ifdef ALL
TEST_CASE(invalidExpiration) {
    Response4 * response = NULL;

    std::cerr << "vvvvvvvvvvvvvv expect to see error message vvvvvvvvvvvvvvv\n";
    REQUIRE_RC(Response4MakeSdl(&response,
        "{"
        "    \"status\": 200,"
        "    \"message\" : \"Claims have expired\""
        "}"));

    rc_t rc = 0;
    REQUIRE_RC(Response4GetRc(response, &rc));
    REQUIRE_RC_FAIL(rc);

    REQUIRE_RC(Response4Release(response));
}
#endif

#ifdef ALL
TEST_CASE(invalidMsg) {
    Response4 * response = NULL;

    std::cerr << "vvvvvvvvvvvvvv expect to see error message vvvvvvvvvvvvvvv\n";
    REQUIRE_RC(Response4MakeSdl(&response,
        "{"
        "    \"status\": 200,"
        "    \"msg\" : \"Signature not recognized\""
        "}"));

    rc_t rc = 0;
    REQUIRE_RC(Response4GetRc(response, &rc));
    REQUIRE_RC_FAIL(rc);

    REQUIRE_RC(Response4Release(response));
}
#endif

#ifdef ALL
TEST_CASE(expired) {
    Response4 * response = NULL;
    
    std::cerr << "vvvvvvvvvvvvvv expect to see error message vvvvvvvvvvvvvvv\n";
    REQUIRE_RC(Response4MakeSdl(&response,
        "{"
        "    \"status\": 440,"
        "    \"message\" : \"Claims have expired\""
        "}"));
    
    rc_t rc = 0;
    REQUIRE_RC(Response4GetRc(response, &rc));
    REQUIRE_RC_FAIL(rc);

    REQUIRE_RC(Response4Release(response));
}
#endif

#ifdef ALL
TEST_CASE(expiredNsg) {
    Response4 * response = NULL;

    std::cerr << "vvvvvvvvvvvvvv expect to see error message vvvvvvvvvvvvvvv\n";
    REQUIRE_RC(Response4MakeSdl(&response,
        "{"
        "    \"status\": 440,"
        "    \"msg\" : \"Claims have expired\""
        "}"));

    rc_t rc = 0;
    REQUIRE_RC(Response4GetRc(response, &rc));
    REQUIRE_RC_FAIL(rc);

    REQUIRE_RC(Response4Release(response));
}
#endif

TEST_SUITE ( TestResolverSdl )

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char * argv [] ) {
if (
0 ) assert ( ! KDbgSetString ( "VFS" ) );
if (
0 ) assert ( ! KDbgSetString ( "VFS-JSON" ) );
    return TestSdlResolver( argc, argv );
    }
}
