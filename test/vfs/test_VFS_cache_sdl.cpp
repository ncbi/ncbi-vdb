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

#include <kapp/args.h> /* ArgsMakeAndHandle */

#include <kfg/config.h> /* KConfigDisableUserSettings */

#include <klib/debug.h> /* KDbgSetString */
#include <klib/time.h> /* KSleep */

#include <kproc/thread.h> // KThread

#include <ktst/unit_test.hpp> /* KMain */

#include <vfs/path.h> /* VFSMaVPathReleasenager */
#include <vfs/manager.h> /* VFSManager */
#include <vfs/resolver.h> /* VResolverRelease */

#include <os-native.h> // setenv

#include "../../libs/vdb/dbmgr-priv.h" // VDBManagerWhackStatic
#include "../../libs/vfs/manager-priv.h" // VFSManagerSdlCacheEmpty

#define ALL

static rc_t argsHandler(int argc, char * argv[]) {
    return ArgsMakeAndHandle(NULL, argc, argv, 0, NULL, 0);
}
TEST_SUITE_WITH_ARGS_HANDLER(Test_VFS_cache_sdlSuite, argsHandler)

using std::string;

class CachingFixture {
protected:
    const VPath * remote;

    CachingFixture(bool caching = true)
        : remote(0)
        , mgr(0)
        , resolver(0)
        , query(0)
    {
        if (caching)
            unsetenv("NCBI_VDB_NO_CACHE_SDL_RESPONSE");
        else
            setenv("NCBI_VDB_NO_CACHE_SDL_RESPONSE", "", 1);

        rc_t rc(VFSManagerMake(&mgr));
        if (rc != 0)
            throw rc;

        rc = VFSManagerGetResolver(mgr, &resolver);
        if (rc != 0)
            throw rc;
    }

    ~CachingFixture() {
        VPathRelease(remote);
        VPathRelease(query);
        VResolverRelease(resolver);
        VFSManagerRelease(mgr);

        VDBManagerWhackStatic();
    }

    rc_t ResetQuery(const char * path_str) {
        rc_t rc(VPathRelease(query));
        query = NULL;

        if (rc == 0)
            rc = VFSManagerMakePath(mgr, &query, path_str);

        return rc;
    }

    rc_t QueryRemote(const char * path_str) {
        rc_t rc(ResetQuery(path_str));

        if (rc == 0)
            rc = VPathRelease(remote);
        remote = NULL;

        if (rc == 0)
            rc = VResolverRemote(resolver, 0, query, &remote);

        return rc;
    }

    rc_t RemoteEquals(const char * path_str) const {
        char path[4096]("");
        rc_t rc(VPathReadUri(remote, path, sizeof path, NULL));
        if (rc == 0)
            if (strcmp(path, path_str) != 0)
                rc = 115;

        return rc;
    }

    static string MkSdlJson(const string & url, int sec = -1) {
        time_t now(0);
        time(&now);
        now += sec;
        struct tm * ptr(gmtime(&now));
        char timeString[99]("");
        strftime(timeString, sizeof timeString, "%Y-%m-%dT%H:%M:%SZ", ptr);

        string json(
            "{ \"result\": [ { \"files\": [ { \"locations\": [ {\n");
        if (sec >= 0) {
            json += "              \"expirationDate\": \"";
            json += timeString;
            json += "\",\n";
        }
        json += "              \"link\": \"" + url + "\" } ] } ] } ] }\n";

        return json;
    }

public:
    VFSManager * mgr;
    VResolver * resolver;
    VPath * query;
};

class NotCachingFixture : protected CachingFixture {
protected:
    NotCachingFixture() : CachingFixture(false) {}
};

#ifdef ALL
// Caching by default
FIXTURE_TEST_CASE(CountCaching, CachingFixture) {
    const char acc[] = "SRR000001";
    string json(MkSdlJson("http://a1/"));
    setenv(acc, (char*)json.c_str(), 1);

    const char acc2[] = "SRR000002";
    json = MkSdlJson("http://a2/");
    setenv(acc, (char*)json.c_str(), 1);

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_NOT_NULL(remote);
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);

    REQUIRE_RC(QueryRemote(acc2));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 2);
}
#endif

#ifdef ALL
// Caching can be disabled
FIXTURE_TEST_CASE(CountNotCaching, NotCachingFixture) {
    const char acc[] = "SRR000001";
    string json(MkSdlJson("http://a1/"));
    setenv(acc, (char*)json.c_str(), 1);

    const char acc2[]("SRR000002");
    json = MkSdlJson("http://a2/");
    setenv(acc2, (char*)json.c_str(), 1);

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_NOT_NULL(remote);
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);

    REQUIRE_RC(QueryRemote(acc2));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
}

// Not caching
FIXTURE_TEST_CASE(NotCaching, NotCachingFixture) {
    const char acc[]("SRR000001");

    string json(MkSdlJson("http://a1/"));
    setenv(acc, (char*)json.c_str(), 1);
    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_RC(RemoteEquals("http://a1/"));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);

    json = MkSdlJson("http://a2/");
    setenv(acc, (char*)json.c_str(), 1);
    REQUIRE_RC(QueryRemote(acc));
    // not caching
    REQUIRE_RC(RemoteEquals("http://a2/"));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
}

// Caching
FIXTURE_TEST_CASE(Caching, CachingFixture) {
    const char acc[] = "SRR000001";

    string json(MkSdlJson("http://a1/"));
    setenv(acc, (char*)json.c_str(), 1);

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_RC(RemoteEquals("http://a1/"));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);

    json = MkSdlJson("http://a2/");
    setenv(acc, (char*)json.c_str(), 1);
    REQUIRE_RC(QueryRemote(acc));
    // use cached response
    REQUIRE_RC(RemoteEquals("http://a1/"));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);
}

// test of expiration; not caching
FIXTURE_TEST_CASE(ExpirationNotCaching, NotCachingFixture) {
    const char acc[] = "SRR000001";

    string json(MkSdlJson("http://a1/", 99)); // will expire in 99 seconds
    setenv(acc, (char*)json.c_str(), 1);
    
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);
    
    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_RC(RemoteEquals("http://a1/"));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);

    json = MkSdlJson("http://a2/", 1); // will expire in 1 second
    setenv(acc, (char*)json.c_str(), 1);
    REQUIRE_RC(QueryRemote(acc));
    // no caching; expiration is ignored
    REQUIRE_RC(RemoteEquals("http://a2/"));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
}
#endif

// test of expiration; caching
FIXTURE_TEST_CASE(ExpirationCaching, CachingFixture) {
    const char acc[] = "SRR000001";

    // will expire in 61 seconds;
    // removed from cache in 60 seconds before expiration
    string json(MkSdlJson("http://a1/", 61));
    setenv(acc, (char*)json.c_str(), 1);

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);
    
    REQUIRE_RC(QueryRemote(acc));
    // not expired
    REQUIRE_RC(RemoteEquals("http://a1/"));
    
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);
    json = MkSdlJson("http://a2/");
    setenv(acc, (char*)json.c_str(), 1);
    REQUIRE_RC(QueryRemote(acc));
    // not expired; still use old result
    REQUIRE_RC(RemoteEquals("http://a1/"));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);

    KSleep(2);
    REQUIRE_RC(QueryRemote(acc));
    // expired; use new result
    REQUIRE_RC(RemoteEquals("http://a2/"));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);
}

#ifdef ALL
// cannot reuse WGS file when not caching
FIXTURE_TEST_CASE(WgsNotCaching, NotCachingFixture) {
    const char acc[] = "AAAB01000001";
    string json(MkSdlJson("http://a1/"));
    setenv(acc, (char*)json.c_str(), 1);

    const char acc2[] = "AAAB01000002";
    string json2(MkSdlJson("http://a2/"));
    setenv(acc2, (char*)json2.c_str(), 1);

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);
    
    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_RC(RemoteEquals("http://a1/"));
    
    // use 2 SDL calls for the same WGS file when not caching
    REQUIRE_RC(QueryRemote(acc2));
    REQUIRE_RC(RemoteEquals("http://a2/"));
    
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
}

// reuse WGS file when caching
FIXTURE_TEST_CASE(WgsCaching, CachingFixture) {
    const char acc[] = "AAAB01000001";
    string json(MkSdlJson("http://a1/"));
    setenv(acc, (char*)json.c_str(), 1);

    const char acc2[] = "AAAB01000002";
    string json2(MkSdlJson("http://a2/"));
    setenv(acc2, (char*)json2.c_str(), 1);

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    REQUIRE_RC(QueryRemote(acc));
    REQUIRE_RC(RemoteEquals("http://a1/"));

    // reuse cached result for a different accession from the same WGS file
    REQUIRE_RC(QueryRemote(acc2));
    REQUIRE_RC(RemoteEquals("http://a1/"));

    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 1);
}

static rc_t DefaultWorkerThreadFn(const KThread * self, void * data) {
    CachingFixture * f((CachingFixture*)data);
    assert(f);

    rc_t rc(0);

    for (int i = 0; i < 50 && rc == 0; ++i) {
        const VPath * remote(NULL);
        rc = VResolverRemote(f->resolver, 0, f->query, &remote);
//      if (rc != 0) int i = 0;

        char path[4096] = "";
        if (rc == 0)
            rc = VPathReadUri(remote, path, sizeof path, NULL);

        rc_t r2(VPathRelease(remote));
        remote = NULL;
        if (r2 != 0 && rc == 0)
            rc = r2;

        ESdlCacheState state(eSCSEmpty);
        VFSManagerSdlCacheCount(f->mgr, &state);
        if (state > eSCSFound)
            VFSManagerSdlCacheClear(f->mgr);
    }

    return rc;
}

// stress test; calling from multiple threads; caching
FIXTURE_TEST_CASE(ThreadsCaching, CachingFixture) {
    const char acc[] = "SRR000001";
    string json(MkSdlJson("http://a1/"));
    setenv(acc, (char*)json.c_str(), 1);

    REQUIRE_RC(ResetQuery(acc));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    CachingFixture * cf(dynamic_cast<CachingFixture*>(this));
    KThread * t[5];
    for (uint i = 0; i < sizeof t / sizeof t[0]; ++i)
        REQUIRE_RC(KThreadMake(&t[i], DefaultWorkerThreadFn, cf));

    rc_t rc(DefaultWorkerThreadFn(0, cf));
    REQUIRE_RC(rc);

    for (uint i = 0; i < sizeof t / sizeof t[0] && rc == 0; ++i)
        REQUIRE_RC(KThreadWait(t[i], &rc));

    for (uint i = 0; i < sizeof t / sizeof t[0] && rc == 0; ++i)
        REQUIRE_RC(KThreadRelease(t[i]));

    REQUIRE_RC(rc);
}

// stress test; calling from multiple threads; not caching
FIXTURE_TEST_CASE(ThreadsNotCaching, NotCachingFixture) {
    const char acc[] = "SRR000001";
    string json(MkSdlJson("http://a1/"));
    setenv(acc, (char*)json.c_str(), 1);

    REQUIRE_RC(ResetQuery(acc));
    REQUIRE(VFSManagerSdlCacheCount(mgr, NULL) == 0);
    REQUIRE_NULL(remote);

    CachingFixture * cf(dynamic_cast<CachingFixture*>(this));
    KThread * t[5];
    for (uint i = 0; i < sizeof t / sizeof t[0]; ++i)
        REQUIRE_RC(KThreadMake(&t[i], DefaultWorkerThreadFn, cf));
    rc_t rc(DefaultWorkerThreadFn(0, cf));
    REQUIRE_RC(rc);

    for (uint i = 0; i < sizeof t / sizeof t[0] && rc == 0; ++i)
        REQUIRE_RC(KThreadWait(t[i], &rc));

    for (uint i = 0; i < sizeof t / sizeof t[0] && rc == 0; ++i)
        REQUIRE_RC(KThreadRelease(t[i]));

    REQUIRE_RC(rc);
}
#endif

extern "C" {
    const char UsageDefaultName[] = "Test_VFS_cache_sdl";
    rc_t CC UsageSummary(const char * progname) { return 0; }
    rc_t CC Usage(const struct Args * args) { return 0; }
    ver_t CC KAppVersion(void) { return 0; }
 
    rc_t CC KMain(int argc, char * argv[]) {
if(0) { KDbgSetString("VFS"); }
        KConfigDisableUserSettings();
        return Test_VFS_cache_sdlSuite(argc, argv);
    }
}
