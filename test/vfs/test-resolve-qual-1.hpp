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
* =========================================================================== */

// no quality / remote:no-qual / no-cache / no-local
#ifndef ALL
FIXTURE_TEST_CASE(_110_00, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING, eQualNo);
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("", f.qRemote);
    REQUIRE_NULL(f.qLocal);
    REQUIRE_NULL(f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// no quality / remote:no-qual&no vdbcache / cache-in-AD / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_110_10, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(AD_CACHING, eQualNo);
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("", f.qRemote);
    REQUIRE_NULL(f.qLocal);
    f.PathEquals(f.qCache, f.spath);
    f.VdbcacheEquals("", f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// no quality / remote:no-qual&no vdbcache / cache-in-AD / local-in-AD
#ifdef ALL
FIXTURE_TEST_CASE(_110_11, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());
    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.CreateFile();

    f.Start(AD_CACHING, eQualNo);
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("", f.qRemote);
    f.PathEquals(f.qLocal, f.spath);
    f.VdbcacheEquals("", f.qLocal);
    f.PathEquals(f.qCache, f.spath);
    f.VdbcacheEquals("", f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// no quality / remote:no-qual&no vdbcache / cache-in-AD / local-w-wrong-quality
#ifdef ALL
FIXTURE_TEST_CASE(_110_13, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());
    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.hasqual.sra", ACC, ACC));
    f.CreateFile();

    f.Start(AD_CACHING, eQualNo);
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/s");
    f.VdbcacheEquals("", f.qRemote);
    f.PathEquals(f.qCache, f.spath);
    f.VdbcacheEquals("", f.qCache);
    REQUIRE_NULL(f.qLocal);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// no quality / remote:no-qual&vdbcache / no-cache / no-local
#ifndef ALL
FIXTURE_TEST_CASE(_111_00, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING, eQualNo);
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals("http://h/nv");

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("http://h/nv", f.qRemote);
    REQUIRE_NULL(f.qLocal);
    REQUIRE_NULL(f.qCache);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/nv");
    f.VdbcacheNotChecked();

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// no quality / remote:no-qual&vdbcache / cache-in-AD / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_111_10, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(AD_CACHING, eQualNo);
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals("http://h/nv");

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("http://h/nv", f.qRemote);
    REQUIRE_NULL(f.qLocal);
    f.PathEquals(f.qCache, f.spath);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/nv");
    f.VdbcacheNotChecked();
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra.vdbcache", ACC, ACC));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// no quality / remote:no-qual&vdbcache / cache-in-AD / local-in-AD
#ifdef ALL
FIXTURE_TEST_CASE(_111_11, TRQFixture) {
    putenv((char*)ACC "_noqual="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());
    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.CreateFile();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra.vdbcache", ACC, ACC));
    f.CreateFile();

    f.Start(AD_CACHING, eQualNo);
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals("http://h/nv");

    const VPath * path = NULL;
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("http://h/nv", f.qRemote);
    f.PathEquals(f.qLocal, f.spath);
    f.PathEquals(f.qCache, f.spath);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/nv");
    f.VdbcacheNotChecked();
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra.vdbcache", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, f.qCache);
    f.VdbcacheEquals(f.spath, f.qLocal);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// no quality / remote:has-qual / no-remote / no-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_120_00_empty, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING, eQualNo, FAIL_DATA);
    REQUIRE_NULL(f.file);
    f.Release();
}
#endif

// no quality / remote:has-qual / no-remote / no-cache / no-local
#ifndef ALL
FIXTURE_TEST_CASE(_120_00, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING, eQualNo);
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("", f.qRemote);
    REQUIRE_NULL(f.qLocal);
    REQUIRE_NULL(f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// no quality / remote repo is disabled; local not found
#ifdef ALL
FIXTURE_TEST_CASE(_140_00, TRQFixture) {
    TRQHelper f(GetName());
    f.DisableRemoteRepo();
    f.Start(NO_AD_CACHING, eQualNo, FAIL_QUERY);
    f.Release();
}
#endif

// no quality / remote repo is disabled;
//  local: .sra in AD -> found because it can be double quality
#ifndef ALL
FIXTURE_TEST_CASE(_141_00_0, TRQFixture) {
    TRQHelper f(GetName());

    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra.vdbcache", ACC, ACC));
    f.CreateFile();
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    f.CreateFile();

    f.DisableRemoteRepo();
    f.Start(NO_AD_CACHING, eQualNo, FAIL_REMOTE);

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    REQUIRE_NULL(f.qRemote);
    REQUIRE_NULL(f.qCache);
    f.PathEquals(f.qLocal, f.spath);

    REQUIRE_NULL(f.path);
    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra.vdbcache", ACC, ACC));
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, path);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    f.VdbcacheEquals(f.spath, f.qLocal);

    f.NextRun();
    f.NextFile();
    f.Release();
}
#endif

// no quality / remote repo is disabled / local: noqual.sra in AD
#ifndef ALL //TODO
FIXTURE_TEST_CASE(_141_00_1, TRQFixture) {
    TRQHelper f(GetName());

    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra.vdbcache", ACC, ACC));
    f.CreateFile();
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.CreateFile();

    f.DisableRemoteRepo();
    f.Start(NO_AD_CACHING, eQualNo, FAIL_REMOTE);

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    REQUIRE_NULL(f.qRemote);
    REQUIRE_NULL(f.qCache);
    f.PathEquals(f.qLocal, f.spath);

    REQUIRE_NULL(f.path);
    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    // NOT_TODO: doesn't work for noqual runs. Use KSrvRunQuery instead
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathNOT_Equals(path, f.spath);
    f.VdbcacheNotChecked(path);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra.vdbcache", ACC, ACC));

    f.VdbcacheEquals(f.spath, f.qLocal);

    f.NextRun();
    f.NextFile();
    f.Release();
}
#endif
