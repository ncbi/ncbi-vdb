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
* ==============================================================================
* Tests of cache of names resolve results / SDL responses.
*/

#include "CachingFixture.hpp" // CachingFixture
#include <ktst/unit_test.hpp> // TEST_SUITE

using std::string;

TEST_SUITE(Test_VFS_cache_sdlSuite)

// caching by default
FIXTURE_TEST_CASE(Default, CachingFixture) {
    const char acc[]("SRR000001");
    string json(MkSdlJson(acc,
        "https://sra-pub-run-odp.s3.amazonaws.com/sra/SRR053325/SRR053325"));
    putenv((char*)json.c_str());

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    bool disabled(true);
    int32_t limit(-1);
    REQUIRE_RC(VFSManagerGetSdlCacheState(mgr, &disabled, &limit));
    REQUIRE(!disabled);
    REQUIRE_EQ(limit, 0);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_NOT_NULL(remote);
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);
}

// caching disabled by environment
FIXTURE_TEST_CASE(NotCachingByEnv, NotCachingFixture) {
    const char acc[]("SRR000001");
    string json(MkSdlJson(acc,
        "https://sra-pub-run-odp.s3.amazonaws.com/sra/SRR053325/SRR053325"));
    putenv((char*)json.c_str());

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    bool disabled(false);
    int32_t limit(-1);
    REQUIRE_RC(VFSManagerGetSdlCacheState(mgr, &disabled, &limit));
    REQUIRE_EQ(limit, 0);
    REQUIRE(disabled);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_NOT_NULL(remote);
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
}

// caching disabled by config
FIXTURE_TEST_CASE(NotCachingByKfg, NotCachingByKfgFixture) {
    const char acc[]("SRR000001");
    string json(MkSdlJson(acc,
        "https://sra-pub-run-odp.s3.amazonaws.com/sra/SRR053325/SRR053325"));
    putenv((char*)json.c_str());

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    bool disabled(false);
    int32_t limit(-1);
    REQUIRE_RC(VFSManagerGetSdlCacheState(mgr, &disabled, &limit));
    REQUIRE_EQ(limit, 0);
    REQUIRE(disabled);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_NOT_NULL(remote);
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
}

// manage caching by API
FIXTURE_TEST_CASE(CachingByApi, CachingFixture) {
    bool disabled(false);
    int32_t limit(-1);
    REQUIRE_RC(VFSManagerGetSdlCacheState(mgr, &disabled, &limit));
    REQUIRE_EQ(limit, 0);
    REQUIRE(!disabled);

    REQUIRE_RC(VFSManagerDisableSdlCaching(mgr, true));

    REQUIRE_RC(VFSManagerGetSdlCacheState(mgr, &disabled, &limit));
    REQUIRE(disabled);

    const char acc[]("SRR000001");
    string json(MkSdlJson(acc,
        "https://sra-pub-run-odp.s3.amazonaws.com/sra/SRR053325/SRR053325"));
    putenv((char*)json.c_str());

    REQUIRE_NULL(remote);
    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_NOT_NULL(remote);
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);

    REQUIRE_RC(VFSManagerDisableSdlCaching(mgr, false));

    REQUIRE_RC(VFSManagerGetSdlCacheState(mgr, &disabled, &limit));
    REQUIRE(!disabled);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_NOT_NULL(remote);
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);
}

// limit cache size by API
FIXTURE_TEST_CASE(LimitByApi, CachingFixture) {
    REQUIRE_RC(VFSManagerSetSdlCacheLimit(mgr, 1));

    const char acc[]("SRR000001");
    string json(MkSdlJson(acc,
        "https://sra-pub-run-odp.s3.amazonaws.com/sra/SRR053325/SRR053325"));
    putenv((char*)json.c_str());

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    bool disabled(true);
    int32_t limit(-1);
    REQUIRE_RC(VFSManagerGetSdlCacheState(mgr, &disabled, &limit));
    REQUIRE_EQ(limit, 1);
    REQUIRE(!disabled);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_NOT_NULL(remote);
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);

    const char acc2[] = "SRR000002";
    json = MkSdlJson(acc2, "http://a2/");
    putenv((char*)json.c_str());

    REQUIRE_RC(QueryRemote(acc2));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);
}

// limit cache size by environment
FIXTURE_TEST_CASE(LimitByEnv, LimitCachingByEnvFixture) {
    const char acc[]("SRR000001");
    string json(MkSdlJson(acc,
        "https://sra-pub-run-odp.s3.amazonaws.com/sra/SRR053325/SRR053325"));
    putenv((char*)json.c_str());

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    bool disabled(true);
    int32_t limit(-1);
    REQUIRE_RC(VFSManagerGetSdlCacheState(mgr, &disabled, &limit));
    REQUIRE_EQ(limit, 1);
    REQUIRE(!disabled);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_NOT_NULL(remote);
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);

    const char acc2[] = "SRR000002";
    json = MkSdlJson(acc2, "http://a2/");
    putenv((char*)json.c_str());

    REQUIRE_RC(QueryRemote(acc2));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);
}

// limit cache size by configuration
FIXTURE_TEST_CASE(LimitByKfg, LimitCachingByKfgFixture) {
    const char acc[]("SRR000001");
    string json(MkSdlJson(acc,
        "https://sra-pub-run-odp.s3.amazonaws.com/sra/SRR053325/SRR053325"));
    putenv((char*)json.c_str());

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    bool disabled(true);
    int32_t limit(-1);
    REQUIRE_RC(VFSManagerGetSdlCacheState(mgr, &disabled, &limit));
    REQUIRE_EQ(limit, 1);
    REQUIRE(!disabled);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_NOT_NULL(remote);
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);

    const char acc2[] = "SRR000002";
    json = MkSdlJson(acc2, "http://a2/");
    putenv((char*)json.c_str());

    REQUIRE_RC(QueryRemote(acc2));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);
}

int main(int argc, char* argv[]) { return Test_VFS_cache_sdlSuite(argc, argv); }
