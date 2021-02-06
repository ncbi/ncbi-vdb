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
#include <kfs/directory.h> /* KDirectoryRelease */
#include <klib/debug.h> /* KDbgSetString */
#include <kns/kns-mgr-priv.h> /* KNSManagerMakeLocal */
#include <kns/manager.h> /* KNSManagerRelease */
#include <ktst/unit_test.hpp> /* KMain */
#include <vfs/path.h> /* VPathRelease */
#include <vfs/services-priv.h> /* KServiceMakeWithMgr */

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
    #define PATH_MAX 4096
#endif

static rc_t argsHandler(int argc, char* argv[]) {
    return ArgsMakeAndHandle(NULL, argc, argv, 0, NULL, 0);
}

TEST_SUITE_WITH_ARGS_HANDLER(TestResolveSuite, argsHandler)

class TestHelper : protected ncbi::NK::TestCase {
    TestCase * dad;
    const String * cache;

public:
    KDirectory * dir;
    char resolved[PATH_MAX];

    TestHelper(TestCase * aDad, const char * acc, const char * format,
        const char * ngc = NULL)
        : TestCase(aDad->GetName()), dad(aDad), cache(0), dir(0), resolved("")
    {
        REQUIRE_RC(KDirectoryNativeDir(&dir));
        KNSManager * mgr = NULL;
        REQUIRE_RC(KNSManagerMakeLocal(&mgr, NULL));
        REQUIRE_RC(KNSManagerSetAdCaching(mgr, true));
        KService * service = NULL;
        REQUIRE_RC(KServiceMakeWithMgr(&service, NULL, mgr, NULL));
        REQUIRE_RC(KServiceAddId(service, acc));
        REQUIRE_RC(KServiceSetFormat(service, format));
        if (ngc != NULL)
            REQUIRE_RC(KServiceSetNgcFile(service, ngc));
        const KSrvResponse * response = NULL;
        REQUIRE_RC(KServiceNamesQuery(service, 0, &response));
        const KSrvRespObj * obj = NULL;
        REQUIRE_RC(KSrvResponseGetObjByIdx(response, 0, &obj));
        KSrvRespObjIterator * it = NULL;
        REQUIRE_RC(KSrvRespObjMakeIterator(obj, &it));
        KSrvRespFile * file = NULL;
        REQUIRE_RC(KSrvRespObjIteratorNextFile(it, &file));
        const VPath * path = NULL;
        REQUIRE_RC(KSrvRespFileGetCache(file, &path));
        REQUIRE_RC(VPathMakeString(path, &cache));
        REQUIRE_RC(VPathRelease(path));
        REQUIRE_RC(KSrvRespFileRelease(file));
        REQUIRE_RC(KSrvRespObjIteratorRelease(it));
        REQUIRE_RC(KSrvRespObjRelease(obj));
        REQUIRE_RC(KSrvResponseRelease(response));
        REQUIRE_RC(KServiceRelease(service));
        REQUIRE_RC(KNSManagerRelease(mgr));
    }

    bool Compare() const {
        String s;
        StringInitCString(&s, resolved);
        return StringEqual(cache, &s);
    }

    void Release() {
        REQUIRE_RC(KDirectoryRelease(dir)); dir = NULL;
    }

    ~TestHelper() {
        Release();
        free(const_cast <String *>(cache)); cache = NULL;
        assert(dad);
        dad->ErrorCounterAdd(GetErrorCounter());
    }
};

TEST_CASE(Public) {

#undef ACC
#define ACC "ERR011396"

    putenv((char*)ACC "="
"{"
"    \"result\": ["
"        {"
"            \"bundle\": \"ERR011396\","
"            \"files\": ["
"                {"
"                    \"object\": \"srapub_files|10623261\","
"                    \"name\": \"amoAC14_seq.fastq\","
"                    \"locations\": ["
"                     {"
"                      \"link\": \"http://ftp.sra.ebi.ac.uk/vol1/run/ERR011/E\""
"                     }"
"                    ]"
"                }"
"            ]"
"        }"
"    ]"
"}");

    TestHelper x(this, ACC, "fastq");

    REQUIRE_RC(KDirectoryResolvePath(x.dir, true, x.resolved, sizeof x.resolved,
        "%s/amoAC14_seq.fastq", ACC));

    REQUIRE(x.Compare());

    x.Release();
}

TEST_CASE(Protected) {

#undef ACC
#define ACC "SRR10587095"

    putenv((char*)ACC "="
        "{"
        "    \"result\": ["
        "        {"
        "            \"bundle\": \"SRR10587095\","
        "            \"files\": ["
        "                {"
        "                    \"object\": \"sragap_files|24893105\","
        "                    \"name\": \"L005474_scRNA_464_cr2.bam\","
        "                    \"locations\": ["
        "                     {"
        "                      \"encryptedForProjectId\": 25285,"
        "                      \"link\": \"https://gap-download.be-md.ncbi.n/\""
        "                     }"
        "                    ]"
        "                }"
        "            ]"
        "        }"
        "    ]"
        "}");

    TestHelper x(this, ACC, "tenx", "data/fake-prj_25285.ngc");

    REQUIRE_RC(KDirectoryResolvePath(x.dir, true, x.resolved, sizeof x.resolved,
        "%s/L005474_scRNA_464_cr2.bam.ncbi_enc", ACC));

    REQUIRE(x.Compare());

    x.Release();
}

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
        return TestResolveSuite(argc, argv);
    }
}
