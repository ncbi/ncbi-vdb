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
* ==============================================================================
*
*/

#include <kapp/args.h> /* ArgsMakeAndHandle */
#include <kfg/config.h> /* KConfigDisableUserSettings */
#include <klib/debug.h> /* KDbgSetString */
#include <kns/kns-mgr-priv.h> /* KNSManagerMakeLocal */
#include <kns/manager.h> /* KNSManagerRelease */
#include <ktst/unit_test.hpp> /* KMain */
#include <vfs/path.h> /* VPathRelease */
#include <vfs/services-priv.h> /* KServiceMakeWithMgr */

#define ALL

static rc_t argsHandler(int argc, char* argv[]) {
    return ArgsMakeAndHandle(NULL, argc, argv, 0, NULL, 0);
}

TEST_SUITE_WITH_ARGS_HANDLER(TestVdbcache, argsHandler)

class TestHelper : protected ncbi::NK::TestCase {
    TestCase * dad;

public:
    KSrvRespObjIterator * it;
    KSrvRespFile * file;
    KSrvRespFileIterator * fi = NULL;

    TestHelper(TestCase * aDad, const char * acc, const char * format = NULL,
        const char * ngc = NULL)
        : TestCase(aDad->GetName()), dad(aDad), it(0), file(0), fi(0)
    {
        KNSManager * mgr = NULL;
        REQUIRE_RC(KNSManagerMakeLocal(&mgr, NULL));
        REQUIRE_RC(KNSManagerSetAdCaching(mgr, true));

        KService * service = NULL;
        REQUIRE_RC(KServiceMakeWithMgr(&service, NULL, mgr, NULL));
        REQUIRE_RC(KServiceResolve(service, false, true));
        REQUIRE_RC(KServiceAddId(service, acc));
        if (format != NULL)
            REQUIRE_RC(KServiceSetFormat(service, format));
        if (ngc != NULL)
            REQUIRE_RC(KServiceSetNgcFile(service, ngc));

        const KSrvResponse * response = NULL;
        REQUIRE_RC(KServiceNamesQuery(service, 0, &response));

        const KSrvRespObj * obj = NULL;
        REQUIRE_RC(KSrvResponseGetObjByIdx(response, 0, &obj));

        REQUIRE_RC(KSrvRespObjMakeIterator(obj, &it));
        REQUIRE_RC(KSrvRespObjIteratorNextFile(it, &file));
        REQUIRE_RC(KSrvRespFileMakeIterator(file, &fi));

        REQUIRE_RC(KSrvRespObjRelease(obj));
        REQUIRE_RC(KSrvResponseRelease(response));
        REQUIRE_RC(KServiceRelease(service));
        REQUIRE_RC(KNSManagerRelease(mgr));
    }

    void Release() {
        REQUIRE_RC(KSrvRespFileIteratorRelease(fi)); fi = NULL;
        REQUIRE_RC(KSrvRespFileRelease(file)); file = NULL;
        REQUIRE_RC(KSrvRespObjIteratorRelease(it)); it = NULL;
    }

    ~TestHelper() {
        Release();
        assert(dad);
        dad->ErrorCounterAdd(GetErrorCounter());
    }

    void NextPath(const String * aPath = 0, const String * aVC = 0,
        bool last = false)
    {
        const String * s;
        const VPath * vdbcache = NULL;
        bool vdbcacheChecked = false;
        const VPath * path = NULL;
        REQUIRE_RC(KSrvRespFileIteratorNextPath(fi, &path));

        if (aPath) {
            REQUIRE_RC(VPathMakeString(path, &s));
            REQUIRE(StringEqual(aPath, s));
            StringWhack(s);

            REQUIRE_RC(VPathGetVdbcache(path, &vdbcache, &vdbcacheChecked));
            REQUIRE(vdbcacheChecked);
            if (aVC) {
                REQUIRE_RC(VPathMakeString(vdbcache, &s));
                REQUIRE(StringEqual(aVC, s));
                StringWhack(s);
                REQUIRE_RC(VPathRelease(vdbcache));
            }
            else
                REQUIRE_NULL(vdbcache);
            REQUIRE_RC(VPathRelease(path));
        }
        else {
            REQUIRE_NULL(path);
            REQUIRE_RC(KSrvRespFileIteratorRelease(fi)); fi = 0;
            REQUIRE_RC(KSrvRespFileRelease(file));
            REQUIRE_RC(KSrvRespObjIteratorNextFile(it, &file));
            if (last)
                REQUIRE_NULL(file);
            else
                REQUIRE_RC(KSrvRespFileMakeIterator(file, &fi));
        }
    }

    void LastPath() { NextPath(0, 0, true); }
};

#define ACC "SRR1219879"
#ifdef ALL
/* Single vdbcache is attached to all sra-s */
TEST_CASE(SingleVdbcache) {
    putenv((char*)ACC "="
"{"
"    \"result\": ["
"        {"
"            \"bundle\": \"SRR1219879\","
"            \"files\": ["
"                {"
"                    \"type\": \"vdbcache\","
"                    \"locations\": ["
"                     {"
"                      \"service\": \"sra-ncbi\","
"                      \"link\": \"https://g/sragap/5FC0C409-4.vdbcache\""
"                     }"
"                    ]"
"                },"
"                {"
"                    \"type\": \"sra\","
"                    \"locations\": ["
"                     {"
"                      \"link\": \"fasp://d@g:data/sracloud/sragap/5FC0C409-4\""
"                     },"
"                     {"
"                      \"service\": \"sra-ncbi\","
"                      \"link\": \"https://g/sragap/5FC0C409-4\""
"                     },"
"                     {"
"                      \"service\": \"s3\","
"                      \"link\": \"https://sra-pu.s3.amazonaws.com/SRR1219879\""
"                     },"
"                     {"
"                      \"service\": \"gs\","
"                      \"link\": \"https://storage.googleapis.com/SRR1219879\""
"                     }"
"                    ]"
"                }"
"            ]"
"        }"
"    ]"
"}");
    TestHelper x(this, ACC, NULL, "prj_phs710EA_test.ngc");

    String e;
    String ev;
    CONST_STRING(&ev, "https://g/sragap/5FC0C409-4.vdbcache");

    x.NextPath(&ev);
    x.NextPath();

    CONST_STRING(&e, "fasp://d@g:data/sracloud/sragap/5FC0C409-4");
    x.NextPath(&e, &ev);

    CONST_STRING(&e, "https://g/sragap/5FC0C409-4");
    x.NextPath(&e, &ev);

    CONST_STRING(&e, "https://sra-pu.s3.amazonaws.com/SRR1219879");
    x.NextPath(&e, &ev);

    CONST_STRING(&e, "https://storage.googleapis.com/SRR1219879");
    x.NextPath(&e, &ev);

    x.LastPath();
}
#endif

#ifdef ALL
/* > 1 vdbcache-s are attached corresponding to service */
TEST_CASE(MultipleVdbcache) {
    putenv((char*)ACC "="
"{"
"    \"result\": ["
"        {"
"            \"bundle\": \"SRR1219879\","
"            \"files\": ["
"                {"
"                    \"type\": \"vdbcache\","
"                    \"locations\": ["
"                     {"
"                      \"link\": \"fasp://d@g:data/sraclo/5FC0C409-4.vdbcache\""
"                     },"
"                     {"
"                      \"service\": \"sra-ncbi\","
"                      \"link\": \"https://g/sragap/5FC0C409-4.vdbcache\""
"                     },"
"                     {"
"                      \"service\": \"s3\","
"                      \"link\": \"https://amazonaws.com/SRR1219879.vdbcache\""
"                     }"
"                    ]"
"                },"
"                {"
"                    \"type\": \"sra\","
"                    \"locations\": ["
"                     {"
"                      \"link\": \"fasp://d@g:data/sracloud/sragap/5FC0C409-4\""
"                     },"
"                     {"
"                      \"service\": \"sra-ncbi\","
"                      \"link\": \"https://g/sragap/5FC0C409-4\""
"                     },"
"                     {"
"                      \"service\": \"gs\","
"                      \"link\": \"https://storage.googleapis.com/SRR1219879\""
"                     },"
"                     {"
"                      \"service\": \"s3\","
"                      \"link\": \"https://sra-pu.s3.amazonaws.com/SRR1219879\""
"                     }"
"                    ]"
"                }"
"            ]"
"        }"
"    ]"
"}");
    TestHelper x(this, ACC, NULL, "prj_phs710EA_test.ngc");

    String e;
    String ev;

    CONST_STRING(&ev, "fasp://d@g:data/sraclo/5FC0C409-4.vdbcache");
    x.NextPath(&ev);

    CONST_STRING(&ev, "https://g/sragap/5FC0C409-4.vdbcache");
    x.NextPath(&ev);

    CONST_STRING(&ev, "https://amazonaws.com/SRR1219879.vdbcache");
    x.NextPath(&ev);

    x.NextPath();

    CONST_STRING(&e, "fasp://d@g:data/sracloud/sragap/5FC0C409-4");
    CONST_STRING(&ev, "fasp://d@g:data/sraclo/5FC0C409-4.vdbcache");
    x.NextPath(&e, &ev);

    CONST_STRING(&e, "https://g/sragap/5FC0C409-4");
    CONST_STRING(&ev, "https://g/sragap/5FC0C409-4.vdbcache");
    x.NextPath(&e, &ev);

    CONST_STRING(&e, "https://storage.googleapis.com/SRR1219879");
    CONST_STRING(&ev, "https://g/sragap/5FC0C409-4.vdbcache");
    x.NextPath(&e, &ev);

    CONST_STRING(&e, "https://sra-pu.s3.amazonaws.com/SRR1219879");
    CONST_STRING(&ev, "https://amazonaws.com/SRR1219879.vdbcache");
    x.NextPath(&e, &ev);

    x.LastPath();
}
#endif

extern "C" {
    const char UsageDefaultName[] = "test-resolve";
    rc_t CC UsageSummary(const char * progname) { return 0; }
    rc_t CC Usage(const struct Args * args) { return 0; }
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char * argv [] ) {
#if 0
        KDbgSetString ( "VFS" );
#endif
        KConfigDisableUserSettings ();
        return TestVdbcache(argc, argv);
    }
}
