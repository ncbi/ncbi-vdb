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

#include <ktst/unit_test.hpp> // TEST_SUITE

#include <kapp/args.h> // KAppVersion

#include <klib/debug.h>

#include <vdb/vdb-priv.h> // VDBManagerMakeReadWithVFSManager
#include <vdb/manager.h> // VDBManager
#include <vdb/database.h> /* VDatabase */
#include <vdb/dependencies.h> /* VDBDependencies */
#include <vdb/schema.h> /* VSchema */

#include <kfg/kfg-priv.h> /* KConfigMakeLocal */

#include <vfs/manager.h> // VFSManager
#include <vfs/manager-priv.h> // VFSManagerGetConfig
#include <vfs/resolver.h> // VResolverRelease
#include <vfs/path.h> // VPathMake

#include <kfs/file.h> // KFileRelease

#include <sra/sraschema.h>

#include <sysalloc.h>

#include <stdlib.h> // free

#include <stdio.h> // puts

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

class DepFixture {
    rc_t StringRelease(String *self) {
        free(self);
        return 0;
    }
protected:
    DepFixture(const char *path)
        : mgr(NULL)
        , vmgr(NULL)
        , resolver(NULL)
        , siteless(false)
    {
        rc_t rc = 0;

        KDirectory *wd = NULL;
        if (KDirectoryNativeDir(&wd)) {
            FAIL("failed to KDirectoryNativeDir");
        }

        const KDirectory *dir = NULL;
        KConfig *cfg = NULL;

        if (KDirectoryOpenDirRead(wd, &dir, false, path)) {
            FAIL("failed to KDirectoryOpenDirRead()");
        }
        if (KConfigMakeLocal(&cfg, dir)) {
            FAIL("failed to KConfigMake()");
        }
        RELEASE(KDirectory, dir);
        if (VFSManagerMakeFromKfg(&vmgr, cfg)) {
            FAIL("failed to VFSManagerMakeFromKfg()");
        }
        if (VFSManagerGetResolver(vmgr, &resolver)) {
            FAIL("failed to VFSManagerGetResolver");
        }

        String *result = NULL;
        rc = KConfigReadString(cfg, "repository/site/main/tracearc/root",
            &result);
        if (rc != 0) {
            if (rc == SILENT_RC(rcKFG, rcNode, rcOpening, rcPath, rcNotFound)) {
                rc = 0;
                siteless = true;
            }
            else {
                FAIL(
              "failed to KConfigReadString(repository/site/main/tracearc/root)");
            }
        }
        else {
            assert(result);
            KPathType t = KDirectoryPathType(wd, result->addr);
            if (t != kptDir) {
                siteless = true;
            }
        }
        RELEASE(String, result);

        RELEASE(KConfig, cfg);
        if (VDBManagerMakeReadWithVFSManager(&mgr, NULL, vmgr)) {
            FAIL("failed to VDBManagerMakeReadWithVFSManager()");
        }

        RELEASE(KDirectory, wd);
    }

    ~DepFixture(void) {
        rc_t rc = 0;
        RELEASE(VResolver, resolver);
        RELEASE(VDBManager, mgr);
        RELEASE(VFSManager, vmgr);
    }
public:
    const VDBManager *mgr;
    VFSManager *vmgr;
    VResolver *resolver;
    bool siteless;
};
class EmptyFixture : public DepFixture {
public:
    EmptyFixture() : DepFixture("kfg/empty") {}
};

class RefseqFixture : public DepFixture {

  #if LINUX
    #define PATH "kfg/linux"
  #elif MAC
    #define PATH "kfg/mac"
  #elif WINDOWS
    #define PATH "kfg/win"
  #endif

public:
    RefseqFixture() : DepFixture(PATH) {}
};

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(TestDependenciesSuite, argsHandler);

FIXTURE_TEST_CASE(TestNoDeps, RefseqFixture) {
    if (siteless) {
        TEST_MESSAGE("Test skipped because site repository does not exist");
        return;
    }

    rc_t rc = 0;

    const VDatabase *db = NULL;
    const char SRR600096[] = "SRR600096";
    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, SRR600096));
//  RELEASE(VDatabase, db);

    VPath* acc = NULL;
    REQUIRE_RC(VFSManagerMakePath(vmgr, &acc, SRR600096));
    const VPath *local = NULL;
/*  REQUIRE_RC(VResolverLocal(resolver, acc, &local));
    RELEASE(VPath, acc); noved to cldn */
    const String *s = NULL;
/*  REQUIRE_RC(VPathMakeString(local, &s));
    REQUIRE(s && s->addr);

    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, s->addr)); */

    const VDBDependencies *dep = NULL;

    REQUIRE_RC(VDatabaseListDependencies(db, &dep, true));
    uint32_t count = 1;
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)0);
    RELEASE(VDBDependencies, dep);

    REQUIRE_RC(VDatabaseListDependencies(db, &dep, false));
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)0);
    RELEASE(VDBDependencies, dep);

    RELEASE(VDatabase, db);
    free(const_cast<String*>(s));
    RELEASE(VPath, local);
}

FIXTURE_TEST_CASE(Test1NoDep, EmptyFixture) {
    rc_t rc = 0;

    const char SRR619505[] = "SRR619505";

    VPath *acc = NULL;
    REQUIRE_RC(VFSManagerMakePath(vmgr, &acc, SRR619505));
    const VPath *local = NULL;
    REQUIRE_RC_FAIL(VResolverLocal(resolver, acc, &local));
    RELEASE(VPath, acc);

    const VDatabase *db = NULL;
    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, SRR619505));

    const VDBDependencies *dep = NULL;

    //                                             missing
    REQUIRE_RC(VDatabaseListDependencies(db, &dep, true));
    uint32_t count = 1;
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)1);
    RELEASE(VDBDependencies, dep);

    REQUIRE_RC(VDatabaseListDependencies(db, &dep, false));
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)1);
    RELEASE(VDBDependencies, dep);

    RELEASE(VDatabase, db);
    RELEASE(VPath, local);
}

FIXTURE_TEST_CASE(TestManyNoDep, EmptyFixture) {
    rc_t rc = 0;

    const char SRR543323[] = "SRR543323";

    VPath* acc = NULL;
    REQUIRE_RC(VFSManagerMakePath(vmgr, &acc, SRR543323));
    const VPath *local = NULL;
    REQUIRE_RC_FAIL(VResolverLocal(resolver, acc, &local));
    RELEASE(VPath, acc);

	struct VSchema *schema;
	REQUIRE_RC(VDBManagerMakeSRASchema(mgr, &schema));

    const VDatabase *db = NULL;
    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, schema, SRR543323));
    REQUIRE_RC(VSchemaRelease(schema));

    const VDBDependencies *dep = NULL;

    //                                             missing
    REQUIRE_RC(VDatabaseListDependencies(db, &dep, true));
    uint32_t count = 1;
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)21);
    RELEASE(VDBDependencies, dep);

    REQUIRE_RC(VDatabaseListDependencies(db, &dep, false));
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)21);
    RELEASE(VDBDependencies, dep);

    RELEASE(VDatabase, db);
    RELEASE(VPath, local);
}

FIXTURE_TEST_CASE(TestDoubleNoDep, EmptyFixture) {
    rc_t rc = 0;

    const char SRR353827[] = "SRR353827";

    VPath* acc = NULL;
    REQUIRE_RC(VFSManagerMakePath(vmgr, &acc, SRR353827));
    const VPath *local = NULL;
    REQUIRE_RC_FAIL(VResolverLocal(resolver, acc, &local));
    RELEASE(VPath, acc);

    const VDatabase *db = NULL;
    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, SRR353827));

    const VDBDependencies *dep = NULL;

    //                                             missing
    REQUIRE_RC(VDatabaseListDependencies(db, &dep, true));
    uint32_t count = 1;
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)1);
    RELEASE(VDBDependencies, dep);

    REQUIRE_RC(VDatabaseListDependencies(db, &dep, false));
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)2);
    RELEASE(VDBDependencies, dep);

    RELEASE(VDatabase, db);
    RELEASE(VPath, local);
}

FIXTURE_TEST_CASE(TestDoubleYesDep, RefseqFixture) {
    if (siteless) {
        TEST_MESSAGE("Test skipped because site repository does not exist");
        return;
    }

    rc_t rc = 0;

    const VDatabase *db = NULL;
    const char ACC[] = "SRR618409";
    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, ACC));
    RELEASE(VDatabase, db);

    VPath* acc = NULL;
    REQUIRE_RC(VFSManagerMakePath(vmgr, &acc, ACC));
    const VPath *local = NULL;
    REQUIRE_RC(VResolverRemote(resolver, eProtocolHttps, acc, &local));
    RELEASE(VPath, acc);
    const String *s = NULL;
    REQUIRE_RC(VPathMakeString(local, &s));
    REQUIRE(s && s->addr);

    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, s->addr));

    const VDBDependencies *dep = NULL;

    //                                             missing
    REQUIRE_RC(VDatabaseListDependencies(db, &dep, true));
    uint32_t count = 1;
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)0);
    RELEASE(VDBDependencies, dep);

    REQUIRE_RC(VDatabaseListDependencies(db, &dep, false));
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)1);
    RELEASE(VDBDependencies, dep);

    RELEASE(VDatabase, db);
    free(const_cast<String*>(s));
    RELEASE(VPath, local);
}

FIXTURE_TEST_CASE(TestManyYesDep, RefseqFixture) {
    if (siteless) {
        TEST_MESSAGE("Test skipped because site repository does not exist");
        return;
    }

    rc_t rc = 0;

    const VDatabase *db = NULL;
    const char SRR543323[] = "SRR543323";
    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, SRR543323));
//  RELEASE(VDatabase, db);

    VPath* acc = NULL;
    REQUIRE_RC(VFSManagerMakePath(vmgr, &acc, SRR543323));
    const VPath *local = NULL;
//  REQUIRE_RC(VResolverLocal(resolver, acc, &local)); noved to cldn
    RELEASE(VPath, acc);
    const String *s = NULL;
/*  REQUIRE_RC(VPathMakeString(local, &s));
    REQUIRE(s && s->addr);

    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, s->addr)); */

    const VDBDependencies *dep = NULL;

    //                                             missing
    REQUIRE_RC(VDatabaseListDependencies(db, &dep, true));
    uint32_t count = 1;
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)0);
    RELEASE(VDBDependencies, dep);

    REQUIRE_RC(VDatabaseListDependencies(db, &dep, false));
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)21);
    RELEASE(VDBDependencies, dep);

    RELEASE(VDatabase, db);
    free(const_cast<String*>(s));
    RELEASE(VPath, local);
}

FIXTURE_TEST_CASE(Test1YesDep, RefseqFixture) {
    if (siteless) {
        TEST_MESSAGE("Test skipped because site repository does not exist");
        return;
    }

    rc_t rc = 0;

    const VDatabase *db = NULL;
    const char SRR619505[] = "SRR619505";
    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, SRR619505));
    RELEASE(VDatabase, db);

    VPath* acc = NULL;
    REQUIRE_RC(VFSManagerMakePath(vmgr, &acc, SRR619505));
    const VPath *local = NULL;

//  REQUIRE_RC(VResolverLocal(resolver, acc, &local));
    REQUIRE_RC(VResolverRemote(resolver, eProtocolHttps, acc, &local));


    RELEASE(VPath, acc);
    const String *s = NULL;
    REQUIRE_RC(VPathMakeString(local, &s));
    REQUIRE(s && s->addr);

    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, s->addr));

    const VDBDependencies *dep = NULL;

    //                                             missing
    REQUIRE_RC(VDatabaseListDependencies(db, &dep, true));
    uint32_t count = 1;
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)0);
    RELEASE(VDBDependencies, dep);

    REQUIRE_RC(VDatabaseListDependencies(db, &dep, false));
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)1);
    RELEASE(VDBDependencies, dep);

    RELEASE(VDatabase, db);
    free(const_cast<String*>(s));
    RELEASE(VPath, local);
}

FIXTURE_TEST_CASE(Test1LocalDep, EmptyFixture) {
    rc_t rc = 0;

    const char SRR413283[] = "SRR413283";

    VPath* acc = NULL;
    REQUIRE_RC(VFSManagerMakePath(vmgr, &acc, SRR413283));
    const VPath *local = NULL;
    REQUIRE_RC_FAIL(VResolverLocal(resolver, acc, &local));
    RELEASE(VPath, acc);

    const VDatabase *db = NULL;
    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, SRR413283));

    const VDBDependencies *dep = NULL;

    //                                             missing
    REQUIRE_RC(VDatabaseListDependencies(db, &dep, true));
    uint32_t count = 1;
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)0);
    RELEASE(VDBDependencies, dep);

    REQUIRE_RC(VDatabaseListDependencies(db, &dep, false));
    REQUIRE_RC(VDBDependenciesCount(dep, &count));
    CHECK_EQUAL(count, (uint32_t)1);
    RELEASE(VDBDependencies, dep);

    RELEASE(VDatabase, db);
    RELEASE(VPath, local);
}

static rc_t argsHandler(int argc, char* argv[]) {
    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

extern "C" {
    ver_t CC KAppVersion(void) { return 0x1000000; }
    const char UsageDefaultName[] = "test-dependencies";
    rc_t CC UsageSummary(const char *progname) { return 0; }
    rc_t CC Usage(const Args *args) { return 0; }
    rc_t CC KMain(int argc, char *argv[]) {
        KConfigDisableUserSettings();
if(
0)      assert(!KDbgSetString("VFS"));
        return TestDependenciesSuite(argc, argv);
    }
}
