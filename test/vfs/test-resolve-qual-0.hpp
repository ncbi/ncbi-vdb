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
* =========================================================================== */

FIXTURE_TEST_CASE(Setup, TRQFixture) {
    TRQHelper f(GetName());
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "/tmp/vdb"));
    f.CreateFile();
    f.Release();
}

// default quality / remote:not found
#ifdef ALL
/*         v requested quality - 0:dflt 1:no 2:full
            v remote quality   - 0:none 1:no 2:has 3:no&has 4:disabled
             v has vdbcache    - 0:none 1:no 2:has 3:no&has
               v has cache     - 0:no 1:AD 2:user-repo
                v has local    - 0:no 1:AD 2:user-repo 3:wrong-quality
                                                       4:no-local-vc
                 _v local is 0:.sra 1:.noqual.sra 2:both */
FIXTURE_TEST_CASE(_000_00, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"status\": 404"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING, eQualDefault, FAIL_DATA);
    REQUIRE_NULL(f.file);
    REQUIRE_NULL(f.run);
    f.Release();
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(_000_00_env, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"status\": 404"
        "  }"
        " ]"
        "}");
    putenv((char*)ENV_MAGIC_REMOTE "=h");
    putenv((char*)ENV_MAGIC_REMOTE_VDBCACHE "=hv");
    putenv((char*)ENV_MAGIC_LOCAL "=/tmp/vdb");
    putenv((char*)ENV_MAGIC_LOCAL_VDBCACHE "=/tmp");
    putenv((char*)ENV_MAGIC_CACHE "=/t");
    putenv((char*)ENV_MAGIC_CACHE_VDBCACHE "=/tv");

    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING, eQualDefault, FAIL_ENV);
    REQUIRE_NULL(f.file);

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qLocal, "/tmp/vdb");
    f.VdbcacheEquals("/tmp", f.qLocal);
    f.PathEquals(f.qCache, "/t");
    f.VdbcacheEquals("/tv", f.qCache);
    f.PathEquals(f.qRemote, "h");
    f.VdbcacheEquals("hv", f.qRemote);

    f.NextRun();
    f.Release();
}
#endif

// default quality / remote:no-qual&no vdbcache / no-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_010_00, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    REQUIRE_NULL(f.qLocal);
    REQUIRE_NULL(f.qCache);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();
    f.VdbcacheEquals("", f.qRemote);

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    f.NextRun();
    f.NextPath();
    f.NextFile();

    f.Release();
}
#endif

// default quality / remote:no-qual&no vdbcache / no-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_010_00_noqual, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    REQUIRE_NULL(f.qLocal);
    REQUIRE_NULL(f.qCache);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();
    f.VdbcacheEquals("", f.qRemote);

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    f.NextRun();
    f.NextPath();
    f.NextFile();

    f.Release();
}
#endif

// default quality / remote:no-qual&no vdbcache / cache-in-AD / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_010_10, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
if (servicesCacheDisabled) return;
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
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

// default quality / remote:no-qual&no vdbcache / cache-in-AD / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_010_10_noqual, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
if (servicesCacheDisabled) return;
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
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

// default quality / remote:no-qual&no vdbcache / cache-in-AD / local-in-AD
#ifdef ALL
FIXTURE_TEST_CASE(_010_11, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
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

    f.Start(AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("", f.qRemote);

    const VPath * path = NULL;
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
if (servicesCacheDisabled) return;
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.PathEquals(f.qLocal, f.spath);
    f.VdbcacheEquals("", f.qLocal);

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.PathEquals(f.qCache, f.spath);
    f.VdbcacheEquals("", f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:no-qual&no vdbcache / cache-in-AD / local-in-AD
#ifdef ALL
FIXTURE_TEST_CASE(_010_11_noqual, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
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

    f.Start(AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("", f.qRemote);

    const VPath * path = NULL;
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
if (servicesCacheDisabled) return;
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.PathEquals(f.qLocal, f.spath);
    f.VdbcacheEquals("", f.qLocal);

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.PathEquals(f.qCache, f.spath);
    f.VdbcacheEquals("", f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:no-qual&no vdbcache / user-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_010_20, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.SetUserRepo();

    f.Start(AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.noqual.sra", ACC));
if (servicesCacheDisabled) return;
    f.PathEquals(path, f.spath);
    f.VdbcacheEquals("", path);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
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

/* default quality / remote:no-qual&no vdbcache / user-cache / no-local
   name with .noqual */
#ifdef ALL
FIXTURE_TEST_CASE(_010_20_noqual, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.SetUserRepo();

    f.Start(AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.noqual.sra", ACC));
if (servicesCacheDisabled) return;
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
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

// default quality / remote:no-qual&has vdbcache / no-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_011_00, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING);
    f.PathEquals(f.path, "http://h/ns");
if (servicesCacheDisabled) return;
    f.VdbcacheEquals("http://h/nv");

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("http://h/nv", f.qRemote);
    REQUIRE_NULL(f.qCache);
    REQUIRE_NULL(f.qLocal);

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

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

// default quality / remote:no-qual&has vdbcache / no-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_011_00_noqual, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.noqual.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING);
    f.PathEquals(f.path, "http://h/ns");
if (servicesCacheDisabled) return;
    f.VdbcacheEquals("http://h/nv");

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("http://h/nv", f.qRemote);
    REQUIRE_NULL(f.qCache);
    REQUIRE_NULL(f.qLocal);

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

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

// default quality / remote:no-qual&has vdbcache / cache-in-AD / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_011_10, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/ns");
if (servicesCacheDisabled) return;
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
    f.PathEquals(f.qCache, f.spath);
    REQUIRE_NULL(f.qLocal);

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

// default quality / remote:no-qual&has vdbcache / cache-in-AD / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_011_10_noqual, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.noqual.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/ns");
if (servicesCacheDisabled) return;
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
    f.PathEquals(f.qCache, f.spath);
    REQUIRE_NULL(f.qLocal);

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

// default quality / remote:no-qual&has vdbcache / cache-in-AD / local-in-AD
#ifdef ALL
FIXTURE_TEST_CASE(_011_11, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
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

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/ns");
if (servicesCacheDisabled) return;
    f.VdbcacheEquals("http://h/nv");

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("http://h/nv", f.qRemote);

    const VPath * path = NULL;
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.PathEquals(f.qLocal, f.spath);

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
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

// default quality / remote:no-qual&has vdbcache / cache-in-AD / local-in-AD
#ifdef ALL
FIXTURE_TEST_CASE(_011_11_env, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    putenv((char*)ENV_MAGIC_REMOTE "=h");
    putenv((char*)ENV_MAGIC_REMOTE_VDBCACHE "=hv");
    putenv((char*)ENV_MAGIC_LOCAL "=/tmp/vdb");
    putenv((char*)ENV_MAGIC_LOCAL_VDBCACHE "=/tmp");
    putenv((char*)ENV_MAGIC_CACHE "=/t");
    putenv((char*)ENV_MAGIC_CACHE_VDBCACHE "=/tv");
    TRQHelper f(GetName());
    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.CreateFile();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra.vdbcache", ACC, ACC));
    f.CreateFile();

    f.Start(AD_CACHING, eQualDefault, FAIL_ENV);
    REQUIRE_NULL(f.path);
    if (servicesCacheDisabled) return;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qLocal, "/tmp/vdb");
    f.VdbcacheEquals("/tmp", f.qLocal);
    f.PathEquals(f.qCache, "/t");
    f.VdbcacheEquals("/tv", f.qCache);
    f.PathEquals(f.qRemote, "h");
    f.VdbcacheEquals("hv", f.qRemote);

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    f.NextRun();
    f.Release();
}
#endif

// default quality / remote:no-qual&has vdbcache / cache-in-AD / local-in-AD
#ifdef ALL
FIXTURE_TEST_CASE(_011_11_noqual, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.noqual.sra.vdbcache\","
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

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/ns");
if (servicesCacheDisabled) return;
    f.VdbcacheEquals("http://h/nv");

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("http://h/nv", f.qRemote);

    const VPath * path = NULL;
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.PathEquals(f.qLocal, f.spath);

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
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
    f.VdbcacheEquals(f.spath, f.qLocal);

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:no-qual&has vdbcache / user-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_011_22, TRQFixture) {
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
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());
    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775,
        kcmOpen | kcmInit | kcmCreate | kcmParents, "tmp/sra"));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.noqual.sra", ACC));
    f.CreateFile();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.noqual.sra.vdbcache", ACC));
    f.CreateFile();

    f.SetUserRepo();

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/ns");
if (servicesCacheDisabled) return;
    f.VdbcacheEquals("http://h/nv");

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("http://h/nv", f.qRemote);

    const VPath * path = NULL;
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.noqual.sra", ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.PathEquals(f.qLocal, f.spath);

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.PathEquals(f.qCache, f.spath);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/nv");
    f.VdbcacheNotChecked();
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.noqual.sra.vdbcache", ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, f.qLocal);

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:no-qual&has vdbcache / user-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_011_22_noqual, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.noqual.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());
    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775,
        kcmOpen | kcmInit | kcmCreate | kcmParents, "tmp/sra"));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.noqual.sra", ACC));
    f.CreateFile();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.noqual.sra.vdbcache", ACC));
    f.CreateFile();

    f.SetUserRepo();

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/ns");
if (servicesCacheDisabled) return;
    f.VdbcacheEquals("http://h/nv");

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("http://h/nv", f.qRemote);

    const VPath * path = NULL;
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.noqual.sra", ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.PathEquals(f.qLocal, f.spath);

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.PathEquals(f.qCache, f.spath);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/nv");
    f.VdbcacheNotChecked();
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.noqual.sra.vdbcache", ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, f.qLocal);

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:has-qual / no-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_020_00, TRQFixture) {
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
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/s");
    f.VdbcacheEquals("", f.qRemote);
    REQUIRE_NULL(f.qCache);
    REQUIRE_NULL(f.qLocal);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:has-qual / cache-in-AD / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_020_10, TRQFixture) {
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
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
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

// default quality / remote:has-qual / user-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_020_20, TRQFixture) {
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
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.SetUserRepo();

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.sra", ACC));
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

// default quality / remote:has-qual / cache-in-AD / local-in-AD
#ifdef ALL
FIXTURE_TEST_CASE(_020_11, TRQFixture) {
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
        "%s/%s.sra", ACC, ACC));
    f.CreateFile();

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/s");
    f.VdbcacheEquals("", f.qRemote);
    f.PathEquals(f.qCache, f.spath);
    f.VdbcacheEquals("", f.qCache);
    f.PathEquals(f.qLocal, f.spath);
    f.VdbcacheEquals("", f.qLocal);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:has-qual&vdbcache / no-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_022_00, TRQFixture) {
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
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/v\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING);
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals("http://h/v");

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/s");
    REQUIRE_NULL(f.qCache);
    REQUIRE_NULL(f.qLocal);

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/v");
    f.VdbcacheNotChecked();
    f.VdbcacheEquals("http://h/v", f.qRemote);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:has-qual&vdbcache / cache-in-AD / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_022_10, TRQFixture) {
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
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/v\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals("http://h/v");

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/s");
    f.PathEquals(f.qCache, f.spath);
    REQUIRE_NULL(f.qLocal);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/v");
    f.VdbcacheNotChecked();
    f.VdbcacheEquals("http://h/v", f.qRemote);
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra.vdbcache", ACC, ACC));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, f.qCache);

    f.NextPath();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:has-qual&vdbcache / cache-in-AD /local-in-AD
#ifdef ALL
FIXTURE_TEST_CASE(_022_11, TRQFixture) {
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
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/v\" } ]"
        "    }"        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    f.CreateFile();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra.vdbcache", ACC, ACC));
    f.CreateFile();

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals("http://h/v");

    const VPath * path = NULL;
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/s");
    f.VdbcacheEquals("http://h/v", f.qRemote);
    f.PathEquals(f.qCache, f.spath);
    f.PathEquals(f.qLocal, f.spath);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/v");
    f.VdbcacheNotChecked();
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra.vdbcache", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, f.qLocal);

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;
    f.VdbcacheEquals(f.spath, f.qCache);

    f.NextRun();
    f.NextPath();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:no-qual+has-qual&no vdbcache / no-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_030_00, TRQFixture) {
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
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("", f.qRemote);
    REQUIRE_NULL(f.qCache);
    REQUIRE_NULL(f.qLocal);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// default quality / remote:no-qual+has-qual&no vdbcache / no-cache / no-local
#ifdef ALL
FIXTURE_TEST_CASE(_030_00_noqual, TRQFixture) {
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
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    f.Start(NO_AD_CACHING);
    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals();

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("", f.qRemote);
    REQUIRE_NULL(f.qCache);
    REQUIRE_NULL(f.qLocal);

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// dfl quality / remote:no-qual+has-qual&vdbcache / no-cache
// local-with-full-qual-in-AD, no-vc
#ifdef ALL
FIXTURE_TEST_CASE(_033_04, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.noqual.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/v\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"sra\","
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
        "%s/%s.sra", ACC, ACC));
    f.CreateFile();

    f.Start(NO_AD_CACHING);
    f.PathEquals(f.path, "http://h/nv");
    f.VdbcacheNotChecked();

    const VPath * path = NULL;
    if (servicesCacheDisabled) return;
    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));

    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(f.qVc);
    f.PathEquals(f.qRemote, "http://h/s");
    f.VdbcacheEquals("http://h/v", f.qRemote);
    REQUIRE_NULL(f.qCache);
    f.VdbcacheEquals("", f.qLocal);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/v");
    f.VdbcacheNotChecked();

    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals("http://h/nv");

    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    REQUIRE_NULL(f.qCache);
    f.PathEquals(f.qLocal, f.spath);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals("http://h/v");

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// dfl qlt / remote:no-qual+has-qual&no vdbcache / cache-in-AD / local-with-qual
#ifdef ALL
FIXTURE_TEST_CASE(_030_13, TRQFixture) {
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
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    f.CreateFile();

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals();

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("", f.qRemote);

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    const VPath * path = NULL;
    f.PathEquals(f.qLocal, f.spath);
    f.VdbcacheEquals("", f.qLocal);
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
if (servicesCacheDisabled) return;
    f.PathEquals(f.qCache, f.spath);
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// dfl qlt / remote:no-qual+has-qual&no vdbcache / cache-in-AD / local-with-qual
#ifdef ALL
FIXTURE_TEST_CASE(_030_13_noqual, TRQFixture) {
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
        "     \"locations\":[ { \"link\":\"http://h/s\" } ]"
        "    },"
        "    {"
        "     \"object\":\"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    }"
        "   ]"
        "  }"
        " ]"
        "}");
    TRQHelper f(GetName());

    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    f.CreateFile();

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals();

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    f.PathEquals(f.qRemote, "http://h/ns");
    f.VdbcacheEquals("", f.qRemote);

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    const VPath * path = NULL;
    f.PathEquals(f.qLocal, f.spath);
    f.VdbcacheEquals("", f.qLocal);
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
if (servicesCacheDisabled) return;
    f.PathEquals(f.qCache, f.spath);
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC_FAIL(KSrvRespFileGetLocal(f.file, &path));

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// dfault quality / remote:no-qual+has-qual&vdbcache / cache-in-AD / local-in-AD
#ifdef ALL
FIXTURE_TEST_CASE(_033_11, TRQFixture) {
    putenv((char*)ACC "="
        "{"
        " \"result\":["
        "  {"
        "   \"bundle\": \"SRR053325\","
        "   \"files\":["
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_vdbcache\","
        "     \"name\":\"SRR053325.noqual.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/nv\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"vdbcache\","
        "     \"name\":\"SRR053325.vdbcache\","
        "     \"locations\":[ { \"link\":\"http://h/v\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"noqual_sra\","
        "     \"name\":\"SRR053325.noqual.sra\","
        "     \"locations\":[ { \"link\":\"http://h/ns\" } ]"
        "    },"
        "    {"
        "     \"object\": \"srapub|SRR053325\","
        "     \"type\":\"sra\","
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
        "%s/%s.noqual.sra", ACC, ACC));
    f.CreateFile();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra.vdbcache", ACC, ACC));
    f.CreateFile();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    f.CreateFile();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra.vdbcache", ACC, ACC));
    f.CreateFile();

    f.Start(AD_CACHING);
    f.PathEquals(f.path, "http://h/nv");
    f.VdbcacheNotChecked();

    const VPath * path = NULL;
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra.vdbcache", ACC, ACC));
if (servicesCacheDisabled) return;
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
    f.VdbcacheEquals(f.spath, f.qCache);
    f.VdbcacheEquals(f.spath, f.qLocal);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/v");
    f.VdbcacheNotChecked();

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra.vdbcache", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/ns");
    f.VdbcacheEquals("http://h/nv");

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    f.PathEquals(f.qCache, f.spath);
    f.PathEquals(f.qLocal, f.spath);

    f.NextPath();
    f.NextFile(true);

    f.PathEquals(f.path, "http://h/s");
    f.VdbcacheEquals("http://h/v");

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    f.NextRun();
    f.NextPath();
    f.NextFile();
    f.Release();
}
#endif

// remote repo is disabled; local not found
#ifdef ALL
FIXTURE_TEST_CASE(_040_00, TRQFixture) {
    TRQHelper f(GetName());
    f.DisableRemoteRepo();
    f.Start(NO_AD_CACHING, eQualDefault, FAIL_QUERY);
    f.Release();
}
#endif

// remote repo is disabled; local .sra in AD
#ifdef ALL
FIXTURE_TEST_CASE(_040_01_0, TRQFixture) {
    TRQHelper f(GetName());
    f.DisableRemoteRepo();
    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    f.CreateFile();
    f.Start(NO_AD_CACHING, eQualDefault, FAIL_REMOTE);
    REQUIRE_NULL(f.path);

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    f.VdbcacheEquals("", path);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    REQUIRE_NULL(f.qRemote);
    REQUIRE_NULL(f.qCache);
    f.PathEquals(f.qLocal, f.spath);
    f.VdbcacheEquals("", f.qLocal);

    f.NextRun();
    f.NextFile();
    f.Release();
}
#endif

// remote repo is disabled; local .sra in AD
#ifdef ALL
FIXTURE_TEST_CASE(_040_01_1, TRQFixture) {
    TRQHelper f(GetName());
    f.DisableRemoteRepo();
    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.CreateFile();
    f.Start(NO_AD_CACHING, eQualDefault, FAIL_REMOTE);
    REQUIRE_NULL(f.path);

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    // NOT_TODO: doesn't work for noqual runs. Use KSrvRunQuery instead
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path)); 
    f.PathNOT_Equals(path, f.spath);
    f.VdbcacheNotChecked(path);

    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE(!f.qVc);
    REQUIRE_NULL(f.qRemote);
    REQUIRE_NULL(f.qCache);
if (servicesCacheDisabled) return;
    f.PathEquals(f.qLocal, f.spath);
    f.VdbcacheEquals("", f.qLocal);

    f.NextRun();
    f.NextFile();
    f.Release();
}
#endif

// remote repo is disabled; vdbcache; local .sra in user repo
#ifdef ALL
FIXTURE_TEST_CASE(_043_02_0, TRQFixture) {
    TRQHelper f(GetName());
    f.DisableRemoteRepo();
    f.SetUserRepo();
    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775,
        kcmOpen | kcmInit | kcmCreate | kcmParents, "tmp/sra"));

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.sra.vdbcache", ACC));
    f.CreateFile();
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.sra", ACC));
    f.CreateFile();
    f.Start(NO_AD_CACHING, eQualDefault, FAIL_REMOTE);
    REQUIRE_NULL(f.path);

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE_NULL(f.qRemote);
if (servicesCacheDisabled) return;
    REQUIRE_NULL(f.qCache);
    f.PathEquals(f.qLocal, f.spath);
    REQUIRE(f.qVc);

    const VPath * path = NULL;
    REQUIRE_RC(KSrvRespFileGetCache(f.file, &path));
    f.PathEquals(path, f.spath);
    f.VdbcacheNotChecked(path);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "tmp/sra/%s.sra.vdbcache", ACC));
    f.VdbcacheEquals(f.spath, path);
    f.VdbcacheEquals(f.spath, f.qLocal);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    f.NextRun();
    f.NextFile();
    f.Release();
}
#endif

// remote disabled; vdbcache; local .noqual and .sra in AD: .noqual used
#ifdef ALL
FIXTURE_TEST_CASE(_043_01_2, TRQFixture) {
    TRQHelper f(GetName());
    f.DisableRemoteRepo();
    REQUIRE_RC(KDirectoryCreateDir(f.dir, 0775, kcmOpen | kcmInit | kcmCreate,
        ACC));
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra.vdbcache", ACC, ACC));
    f.CreateFile();
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    f.CreateFile();
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra.vdbcache", ACC, ACC));
    f.CreateFile();
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra", ACC, ACC));
    f.CreateFile();
    f.Start(NO_AD_CACHING, eQualDefault, FAIL_REMOTE);
    REQUIRE_NULL(f.path);

    const VPath * path = NULL;
    REQUIRE_RC_FAIL(KSrvRespFileGetCache(f.file, &path));

    REQUIRE_RC(KSrvRunQuery(f.run, &f.qLocal, &f.qRemote, &f.qCache, &f.qVc));
    REQUIRE_NULL(f.qRemote);
if (servicesCacheDisabled) return;
    REQUIRE(f.qVc);
    REQUIRE_NULL(f.qCache);
    f.PathEquals(f.qLocal, f.spath);

    // NOT_TODO: doesn't work for noqual runs. Use KSrvRunQuery instead
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra", ACC, ACC));
    REQUIRE_RC(KSrvRespFileGetLocal(f.file, &path));
    f.PathEquals(path, f.spath);
    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.sra.vdbcache", ACC, ACC));
    f.VdbcacheEquals(f.spath, path);
    REQUIRE_RC(VPathRelease(path)); path = NULL;

    REQUIRE_RC(KDirectoryResolvePath(f.dir, true, f.spath, sizeof f.spath,
        "%s/%s.noqual.sra.vdbcache", ACC, ACC));
    f.VdbcacheEquals(f.spath, f.qLocal);

    f.NextRun();
    f.NextFile();
    f.Release();
}
#endif
