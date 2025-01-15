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
#include <ktst/unit_test.hpp> /* KMain */

#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path.h> /* VPathRelease */

#include "../../libs/vfs/path-priv.h" // VPathEqual

extern "C" { rc_t LegacyVPathMake(VPath ** new_path, const char * posix_path); }

static rc_t argsHandler(int argc, char* argv[]) {
    return ArgsMakeAndHandle(NULL, argc, argv, 0, NULL, 0);
}

TEST_SUITE_WITH_ARGS_HANDLER(TestResolveSuite, argsHandler);

TEST_CASE(VPathTest) {
    const String * ps(NULL);
    String s;
    VPath * out(NULL);

    REQUIRE_RC_FAIL(VPathMakeString(out, &ps));
    REQUIRE_NULL(ps);
    REQUIRE_RC_FAIL(VPathGetPath(out, &s));

    REQUIRE_RC(LegacyVPathMake(&out, "http://h"));
    REQUIRE(VPathFromUri(out));
    REQUIRE(!VPathIsFSCompatible(out));
    REQUIRE(VPathIsRemote(out));
    REQUIRE_RC(VPathMakeString(out, &ps));
    REQUIRE_RC(VPathGetPath(out, &s));
    REQUIRE(!StringEqual(&s, ps));
    StringWhack(ps);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "http://h/"));
    REQUIRE(VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "http://h/f"));
    REQUIRE(VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "http://h/d/"));
    REQUIRE(VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "http://h/d/f"));
    REQUIRE(VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "/"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "h"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "/h"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathMakeString(out, &ps));
    REQUIRE_RC(VPathGetPath(out, &s));
    REQUIRE(StringEqual(&s, ps));
    StringWhack(ps);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "./h"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "SRR000001"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathMakeString(out, &ps));
    REQUIRE_RC(VPathGetPath(out, &s));
    REQUIRE(StringEqual(&s, ps));
    StringWhack(ps);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "/SRR000001"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "./SRR000001"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "/d/"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "./d/"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "/d/f"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(LegacyVPathMake(&out, "./d/f"));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));
}

class Fixture {
protected:
    Fixture() {
        rc_t rc(VFSManagerMake(&mgr));
        if (rc)
            throw rc;
    }
    ~Fixture() { VFSManagerRelease(mgr); }
    VFSManager * mgr;
};

FIXTURE_TEST_CASE(ResolveTestFailures, Fixture) {
    const char in[]("SRR000001");
    VPath * p(NULL);
    REQUIRE_RC(LegacyVPathMake(&p, in));

    const VPath * out(NULL);
    const VPath * cache(NULL), * remote(NULL);

    REQUIRE_RC_FAIL(VFSManagerResolve(NULL, in, &out));
    REQUIRE_RC_FAIL(VFSManagerResolve(mgr, NULL, &out));
    REQUIRE_RC_FAIL(VFSManagerResolve(mgr, in, NULL));

    REQUIRE_RC_FAIL(VFSManagerResolveVPath(NULL, p, &out));
    REQUIRE_RC_FAIL(VFSManagerResolveVPath(mgr, NULL, &out));
    REQUIRE_RC_FAIL(VFSManagerResolveVPath(mgr, p, NULL));

    REQUIRE_RC_FAIL(VFSManagerResolveLocal(NULL, in, &out));
    REQUIRE_RC_FAIL(VFSManagerResolveLocal(mgr, NULL, &out));
    REQUIRE_RC_FAIL(VFSManagerResolveLocal(mgr, in, NULL));

    REQUIRE_RC_FAIL(VFSManagerResolveVPathLocal(NULL, p, &out));
    REQUIRE_RC_FAIL(VFSManagerResolveVPathLocal(mgr, NULL, &out));
    REQUIRE_RC_FAIL(VFSManagerResolveVPathLocal(mgr, p, NULL));

    REQUIRE_RC_FAIL(VFSManagerResolveRemote(NULL, in, &out, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC_FAIL(VFSManagerResolveRemote(mgr, NULL, &out, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC_FAIL(VFSManagerResolveRemote(mgr, in, NULL, &cache));
    REQUIRE_NULL(cache);

    REQUIRE_RC_FAIL(VFSManagerResolveVPathRemote(NULL, p, &out, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC_FAIL(VFSManagerResolveVPathRemote(mgr, NULL, &out, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC_FAIL(VFSManagerResolveVPathRemote(mgr, p, NULL, &cache));
    REQUIRE_NULL(cache);

    REQUIRE_RC_FAIL(VFSManagerResolveWithCache(NULL, in, &out, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC_FAIL(VFSManagerResolveWithCache(mgr, NULL, &out, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC_FAIL(VFSManagerResolveWithCache(mgr, in, NULL, &cache));
    REQUIRE_NULL(cache);

    REQUIRE_RC_FAIL(VFSManagerResolveVPathWithCache(NULL, p, &out, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC_FAIL(VFSManagerResolveVPathWithCache(mgr, NULL, &out, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC_FAIL(VFSManagerResolveVPathWithCache(mgr, p, NULL, &cache));
    REQUIRE_NULL(cache);

    REQUIRE_RC_FAIL(VFSManagerResolveAll(NULL, in, &out, &remote, &cache));
    REQUIRE_RC_FAIL(VFSManagerResolveAll(mgr, NULL, &out, &remote, &cache));
    REQUIRE_RC_FAIL(VFSManagerResolveAll(mgr, in, NULL, NULL, &cache));
    REQUIRE_NULL(cache);

    REQUIRE_RC_FAIL(VFSManagerResolveVPathAll(NULL, p, &out, &remote, &cache));
    REQUIRE_RC_FAIL(VFSManagerResolveVPathAll(mgr,
        NULL, &out, &remote, &cache));
    REQUIRE_RC_FAIL(VFSManagerResolveVPathAll(mgr, p, NULL, NULL, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_NULL(out);

    REQUIRE_RC(VPathRelease(p));
}

FIXTURE_TEST_CASE(ResolveTestRemote, Fixture) {
    const char in[]("SRR000001");
    VPath * p(NULL);
    REQUIRE_RC(LegacyVPathMake(&p, in));

    const VPath * out(NULL), * outP(NULL), *remote(NULL), *cache(NULL);

    REQUIRE_RC(VFSManagerResolve(mgr, in, &out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolve(mgr, in, &out));
    REQUIRE(VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(VPathIsRemote(out));

    REQUIRE_RC(VFSManagerResolveVPath(mgr, p, &outP));
    REQUIRE(VPathIsRemote(outP));
    int notequal(-1);
    REQUIRE_RC(VPathEqual(out, outP, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC_FAIL(VFSManagerResolveLocal(mgr, in, &out));
    REQUIRE_RC_FAIL(VFSManagerResolveVPathLocal(mgr, p, &out));

    REQUIRE_RC(VFSManagerResolveRemote(mgr, in, &remote, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC(VPathEqual(outP, remote, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(remote));

    REQUIRE_RC(VFSManagerResolveVPathRemote(mgr, p, &remote, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC(VPathEqual(outP, remote, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(remote));

    REQUIRE_RC(VFSManagerResolveWithCache(mgr, in, &out, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveVPathWithCache(mgr, p, &out, &cache));
    REQUIRE_NULL(cache);
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveAll(mgr, in, &out, &remote, &cache));
    REQUIRE_NULL(out);
    REQUIRE_NULL(cache);
    REQUIRE_RC(VPathEqual(outP, remote, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveVPathAll(mgr, p, &out, &remote, &cache));
    REQUIRE_NULL(out);
    REQUIRE_NULL(cache);
    REQUIRE_RC(VPathEqual(outP, remote, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VPathRelease(outP));
    REQUIRE_RC(VPathRelease(p));
}

FIXTURE_TEST_CASE(ResolveTestLocal, Fixture) {
    const char in[]("SRR045450");
    VPath * p(NULL);
    REQUIRE_RC(LegacyVPathMake(&p, in));

    const VPath * out(NULL), *outP(NULL), *remote(NULL), *cache(NULL);

    REQUIRE_RC(VFSManagerResolve(mgr, in, &out));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));

    REQUIRE_RC(VFSManagerResolveVPath(mgr, p, &outP));
    REQUIRE(!VPathIsRemote(outP));
    int notequal(-1);
    REQUIRE_RC(VPathEqual(out, outP, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveRemote(mgr, in, &out, &cache));
    REQUIRE_RC(VFSManagerResolveVPathRemote(mgr, p, &remote, &cache));
    REQUIRE_RC(VPathEqual(remote, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE(VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(VPathIsRemote(out));
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveLocal(mgr, in, &out));
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveVPathLocal(mgr, p, &out));
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveWithCache(mgr, in, &out, &cache));
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveVPathWithCache(mgr, p, &out, &cache));
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    const VPath * remoteP(NULL);
    REQUIRE_RC(VFSManagerResolveAll(mgr, in, &out, &remoteP, &cache));

    REQUIRE_RC(VPathEqual(remoteP, remote, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(remoteP));

    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveVPathAll(mgr, p, &out, &remoteP, &cache));

    REQUIRE_NULL(cache);
    REQUIRE_RC(VPathEqual(remoteP, remote, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(remote));
    REQUIRE_RC(VPathRelease(remoteP));

    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VPathRelease(outP));
    REQUIRE_RC(VPathRelease(p));
}

FIXTURE_TEST_CASE(ResolveTestLocalPath, Fixture) {
    const char in[]("./SRR045450");
    VPath * p(NULL);
    REQUIRE_RC(LegacyVPathMake(&p, in));

    const VPath * out(NULL), *outP(NULL), *remote(NULL), *cache(NULL);

    REQUIRE_RC(VFSManagerResolve(mgr, in, &out));
    REQUIRE(!VPathFromUri(out));
    REQUIRE(VPathIsFSCompatible(out));
    REQUIRE(!VPathIsRemote(out));

    REQUIRE_RC(VFSManagerResolveVPath(mgr, p, &outP));
    REQUIRE(!VPathIsRemote(outP));
    int notequal(-1);
    REQUIRE_RC(VPathEqual(out, outP, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC_FAIL(VFSManagerResolveRemote(mgr, in, &out, &cache));
    REQUIRE_NULL(out);
    REQUIRE_RC_FAIL(VFSManagerResolveVPathRemote(mgr, p, &remote, &cache));
    REQUIRE_NULL(remote);

    REQUIRE_RC(VFSManagerResolveLocal(mgr, in, &out));
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveVPathLocal(mgr, p, &out));
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveWithCache(mgr, in, &out, &cache));
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveVPathWithCache(mgr, p, &out, &cache));
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveAll(mgr, in, &out, &remote, &cache));
    REQUIRE_NULL(remote);
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VFSManagerResolveVPathAll(mgr, p, &out, &remote, &cache));
    REQUIRE_NULL(remote);
    REQUIRE_RC(VPathEqual(outP, out, &notequal));
    REQUIRE(notequal == 0);
    REQUIRE_RC(VPathRelease(out));

    REQUIRE_RC(VPathRelease(outP));
    REQUIRE_RC(VPathRelease(p));
}

extern "C" {
    const char UsageDefaultName[] = "vfs-test-resolve";
    rc_t CC UsageSummary(const char * progname) { return 0; }
    rc_t CC Usage(const struct Args * args) { return 0; }
    ver_t CC KAppVersion(void) { return 0; }
    rc_t CC KMain(int argc, char * argv[]) {
        putenv((char*)"NCBI_VDB_NO_CACHE_SDL_RESPONSE=1");
#if 0
        KDbgSetString("VFS");
#endif
        KConfigDisableUserSettings();
        return TestResolveSuite(argc, argv);
    }
}
