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

#include <kfg/kfg-priv.h> /* KConfigMakeEmpty */
#include <klib/debug.h> /* KDbgSetString */
#include <klib/rc.h> /* SILENT_RC */
#include <ktst/unit_test.hpp> /* TEST_SUITE */
#include <vfs/manager.h> // VFSManagerRelease
#include <vfs/manager-priv.h> // VFSManagerMakeLocal
#include <vfs/path.h> // VPathRelease
#include <vfs/resolver.h> // VResolverRelease
#include <vfs/services-priv.h> /* KServiceMakeWithMgr */

#define ALL
const bool PRINT_KFG = 0;
const bool PRINT_SDL = 0;

extern "C" {rc_t LegacyVPathMake(VPath** new_path, const char* posix_path); }

TEST_SUITE(Resolver3TestSuite)

class Fixture {
    KConfig* k; VFSManager* m;

    rc_t Begin(const char* path, const char* value) {
        rc_t rc(0), r2(0);

        if (value)
            rc = KConfigWriteString(k, path, value);

        if (PRINT_KFG) {
            r2 = KConfigPrint(k, 0);
            if (rc == 0 && r2 != 0)
                rc = r2;
        }

        r2 = VFSManagerMakeLocal(&m, k);
        if (rc == 0 && r2 != 0)
            rc = r2;

        return rc;
    }

    rc_t RunResolver(bool succeed,
        const char* path, const char* value)
    {
        rc_t rc(Begin(path, value));

        VResolver* vr(NULL);
        rc_t r2(VFSManagerGetResolver(m, &vr));
        if (rc == 0 && r2 != 0)
            rc = r2;

        VPath* q(NULL);
        r2 = VFSManagerMakePath(m, &q, "SRR619505");
        if (rc == 0 && r2 != 0)
            rc = r2;

        const VPath* r(NULL);
        rc_t re = VResolverQuery(vr, 0, q, NULL, &r, NULL);

        r2 = VPathRelease(r);
        if (rc == 0 && r2 != 0)
            rc = r2;

        r2 = VPathRelease(q);
        if (rc == 0 && r2 != 0)
            rc = r2;

        r2 = VResolverRelease(vr);
        if (rc == 0 && r2 != 0)
            rc = r2;

        if (rc == 0) {
            if (succeed) {
                if (re != 0)
                    rc = re;
                else if (r == NULL)
                    rc = SILENT_RC(rcVFS, rcQuery, rcResolving, rcPath, rcNull);
            }
            else {
                if (re == 0 || r)
                    rc = SILENT_RC(
                        rcVFS, rcQuery, rcResolving, rcError, rcNotFound);
            }
        }

        return rc;
    }

protected:
    Fixture() :k(0), m(0) {
        rc_t rc(KConfigMakeEmpty(&k));
        if (rc)
            throw rc;
    }

    ~Fixture() { End(); }

    rc_t End() {
        rc_t rc(VFSManagerRelease(m)); m = NULL;

        rc_t r2(KConfigRelease(k)); k = NULL;
        if (rc == 0 && r2 != 0)
            rc = r2;

        return rc;
    }

    rc_t RunService(const char* path = 0, const char* value = 0) {
        rc_t rc(Begin(path, value));

        KService* s(0);
        rc_t r2(KServiceMakeWithMgr(&s, NULL, NULL, k));
        if (rc == 0 && r2 != 0)
            rc = r2;

        r2 = KServiceAddId(s, "SRR053325");
        if (rc == 0 && r2 != 0)
            rc = r2;

        const KSrvResponse* r(0);
        r2 = KServiceNamesQuery(s, 0, &r);
        if (rc == 0 && r2 != 0)
            rc = r2;

        r2 = KSrvResponseRelease(r);
        if (rc == 0 && r2 != 0)
            rc = r2;

        r2 = KServiceRelease(s); s = NULL;
        if (rc == 0 && r2 != 0)

            rc = r2;
        return rc;
    }

    rc_t ResolverFails(const char* path = 0, const char* value = 0) {
        return RunResolver(false, path, value);
    }
    rc_t ResolverSucceeds(const char* path, const char* value) {
        return RunResolver(true, path, value);
    }
};

FIXTURE_TEST_CASE(ServiceWorksWithoutUrlInConfig, Fixture) {
    REQUIRE_RC(RunService());
    REQUIRE_RC(End());
}
FIXTURE_TEST_CASE(ServiceCallsSdlWithCgiInConfig, Fixture) {
    REQUIRE_RC(RunService("/repository/remote/main/CGI/resolver-cgi",
        "https://trace.ncbi.nlm.nih.gov/Traces/names/names.fcgi"));
    REQUIRE_RC(End());
}
FIXTURE_TEST_CASE(ServiceCallsSdlWithSglInConfig, Fixture) {
    REQUIRE_RC(RunService("/repository/remote/main/SDL.2/resolver-cgi",
        "https://locate.ncbi.nlm.nih.gov/sdl/2/retrieve"));
    REQUIRE_RC(End());
}
FIXTURE_TEST_CASE(ServiceCallsCustomSdl, Fixture) {
    REQUIRE_RC(RunService("/repository/remote/main/SDL.2/resolver-cgi",
        "https://locate.be-md.ncbi.nlm.nih.gov/sdl/2/retrieve"));
    REQUIRE_RC(End());
}

FIXTURE_TEST_CASE(ResolverFailsWithoutUrlInConfig, Fixture) {
    REQUIRE_RC(ResolverFails());
    REQUIRE_RC(End());
}
FIXTURE_TEST_CASE(ResolverFailsWithCgiInConfig, Fixture) {
    REQUIRE_RC(ResolverFails("/repository/remote/main/CGI/resolver-cgi",
        "https://trace.ncbi.nlm.nih.gov/Traces/names/names.fcgi"));
    REQUIRE_RC(End());
}

#ifdef ALL
FIXTURE_TEST_CASE(ResolverSucceedsWithSdlInConfig, Fixture) {
    REQUIRE_RC(ResolverSucceeds("/repository/remote/main/SDL.2/resolver-cgi",
        "https://locate.ncbi.nlm.nih.gov/sdl/2/retrieve"));
    REQUIRE_RC(End());
}
FIXTURE_TEST_CASE(ResolverSucceedsWithCustomSdl, Fixture) {
    REQUIRE_RC(ResolverSucceeds("/repository/remote/main/SDL.2/resolver-cgi",
        "https://locate.be-md.ncbi.nlm.nih.gov/sdl/2/retrieve"));
    REQUIRE_RC(End());
}
#endif

extern "C" {
    ver_t CC KAppVersion(void) { return 0; }
    rc_t CC KMain(int argc, char* argv[]) {
        if (PRINT_SDL)
            KDbgSetString("VFS");
        return Resolver3TestSuite(argc, argv);
    }
}
