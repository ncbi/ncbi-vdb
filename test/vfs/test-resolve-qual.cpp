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

#include <kfg/kfg-priv.h> /* KConfigMakeLocal */

#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */

#include <klib/debug.h> /* KDbgSetString */
#include <klib/strings.h> /* ENV_MAGIC_REMOTE */

#include <kns/kns-mgr-priv.h> /* KNSManagerSetAdCaching */
#include <kns/manager.h> /* KNSManagerMake */

#include <ktst/unit_test.hpp> /* KMain */

#include <vdb/vdb-priv.h> /* VDBManagerGetQuality */

#include <vfs/manager.h> /* VFSManagerLogNamesServiceErrors */
#include <vfs/path.h> /* VPathRelease */
#include <vfs/services-priv.h> /* KServiceMakeWithMgr */

#include "../libs/vfs/services-priv.h" /* KServiceSetQuality */

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

TEST_SUITE(TestResolveQualSuite)

#define ALL
//#ifndef

#define ACC "SRR053325"

#define AD_CACHING true
#define NO_AD_CACHING false

#define FAIL_DATA 1
#define FAIL_QUERY 2
#define FAIL_REMOTE 3
#define FAIL_ENV 4

static bool servicesCacheDisabled = VDBManagerGetQuality(NULL) >= eQualLast;

struct TRQFixture { TRQFixture() {
    unsetenv((char*)ACC);
    unsetenv(ENV_MAGIC_REMOTE);
    unsetenv(ENV_MAGIC_REMOTE_VDBCACHE);
    unsetenv(ENV_MAGIC_LOCAL);
    unsetenv(ENV_MAGIC_LOCAL_VDBCACHE);
    unsetenv(ENV_MAGIC_CACHE);
    unsetenv(ENV_MAGIC_CACHE_VDBCACHE);
} };

class TRQHelper : protected ncbi::NK::TestCase {
public:
    KDirectory * dir;
    KConfig * kfg;
    KFile * f;
    KNSManager * kns;
    KService * service;
    const KSrvResponse * response;
    const KSrvRespObj * obj;
    KSrvRespObjIterator * it;
    KSrvRespFile * file;
    KSrvRespFileIterator * fi;
    KSrvRunIterator * ri;
    const KSrvRun * run;
    const VPath * path;
    const VPath * qLocal;
    const VPath * qRemote;
    const VPath * qCache;
    bool qVc;
    char root[PATH_MAX];
    char spath[PATH_MAX];

    TRQHelper(const std::string & name) : TestCase(name)
        , dir(0), kfg(0), f(0), kns(0), service(0), response(0)
        , obj(0), it(0), file(0), fi(0), ri(0), run(0)
        , path(0), qLocal(0), qRemote(0), qCache(0)
    {
        REQUIRE_RC(KDirectoryNativeDir(&dir));
        const char tmp[] = "tmp";
        REQUIRE_RC(KDirectoryResolvePath(dir, true, root, sizeof root, tmp));
        REQUIRE_RC(KDirectoryRemove(dir, true, tmp));
        REQUIRE_RC(KDirectoryRemove(dir, true, ACC));
    }

    void CreateFile() {
        REQUIRE_RC(KDirectoryCreateFile(dir, &f, false,
            0664, kcmOpen | kcmInit | kcmCreate, spath));
        REQUIRE_RC(KFileRelease(f)); f = NULL;
    }

    void SetUserRepo() {
        if (kfg == NULL)
            REQUIRE_RC(KConfigMakeLocal(&kfg, dir));
        REQUIRE_RC(KConfigWriteString(kfg,
            "/repository/user/main/public/root", root));
        REQUIRE_RC(KConfigWriteString(kfg,
            "/repository/user/main/public/apps/sra/volumes/sraFlat", "sra"));
        REQUIRE_RC(KConfigWriteString(kfg,
            "/repository/user/main/public/apps/sraNoqual/volumes/noqualSraFlat",
            "sra"));
    }

    void DisableRemoteRepo() {
        if (kfg == NULL)
            REQUIRE_RC(KConfigMakeLocal(&kfg, dir));
        REQUIRE_RC(KConfigWriteString(kfg,
            "/repository/remote/disabled", "true"));
    }

    void Start(bool setAdCaching,
        VQuality quality = eQualDefault, int fail = 0)
    {
        REQUIRE_RC(KNSManagerMakeLocal(&kns, kfg));
        if (setAdCaching)
            REQUIRE_RC(KNSManagerSetAdCaching(kns, true));
        else
            REQUIRE_RC(KNSManagerSetAdCaching(kns, false));

        REQUIRE_RC(KServiceMakeWithMgr(&service, NULL, kns, kfg));
        if (quality > 0)
            KServiceSetQuality(service, quality);
        REQUIRE_RC(KServiceAddId(service, ACC));

        if (fail == FAIL_QUERY)
            REQUIRE_RC_FAIL(KServiceNamesQuery(service, 0, &response));
        else {
            REQUIRE_RC(KServiceNamesQuery(service, 0, &response));

            REQUIRE_RC(KSrvResponseMakeRunIterator(response, &ri));

            REQUIRE_RC(KSrvRunIteratorNextRun(ri, &run));

            if (fail == FAIL_ENV)
                REQUIRE_RC_FAIL(KSrvResponseGetObjByIdx(response, 0, &obj));
            else {
                REQUIRE_RC(KSrvResponseGetObjByIdx(response, 0, &obj));
                REQUIRE_RC(KSrvRespObjMakeIterator(obj, &it));

                REQUIRE_RC(KSrvRespObjIteratorNextFile(it, &file));
            }
            if (fail == FAIL_DATA) {
                REQUIRE_NULL(file);
                REQUIRE_NULL(run);
            }
            else if (fail != FAIL_REMOTE) {
                REQUIRE_NOT_NULL(run);
                if (fail != FAIL_ENV) {
                    REQUIRE_RC(KSrvRespFileMakeIterator(file, &fi));
                    REQUIRE_RC(KSrvRespFileIteratorNextPath(fi, &path));
                }
            }
        }
    }

    void NextPath(bool notNull = false) {
        REQUIRE_RC(VPathRelease(path)); path = 0;
        REQUIRE_RC(KSrvRespFileIteratorNextPath(fi, &path));

        if (notNull)
            REQUIRE_NOT_NULL(path);
        else
            REQUIRE_NULL(path);
    }

    void NextFile(bool notNull = false) {
        REQUIRE_RC(KSrvRespFileIteratorRelease(fi)); fi = 0;
        REQUIRE_RC(KSrvRespFileRelease(file)); file = 0;
        REQUIRE_RC(KSrvRespObjIteratorNextFile(it, &file));

        if (notNull) {
            REQUIRE_NOT_NULL(file);
            REQUIRE_RC(KSrvRespFileMakeIterator(file, &fi));
            REQUIRE_RC(VPathRelease(path)); path = 0;
            REQUIRE_RC(KSrvRespFileIteratorNextPath(fi, &path));
        }
        else
            REQUIRE_NULL(file);
    }

    void NextRun(bool notNull = false) {
        REQUIRE_RC(VPathRelease(qRemote)); qRemote = 0;
        REQUIRE_RC(VPathRelease(qLocal)); qLocal = 0;
        REQUIRE_RC(VPathRelease(qCache)); qCache = 0;
        REQUIRE_RC(KSrvRunRelease(run)); run = 0;

        REQUIRE_RC(KSrvRunIteratorNextRun(ri, &run));

        if (notNull)
            REQUIRE_NOT_NULL(run);
        else
            REQUIRE_NULL(run);
    }

    void PathEquals(const VPath * path, const std::string & e) {
        String expPath;
        StringInitCString(&expPath, e.c_str());

        const String * str = NULL;
        REQUIRE_RC(VPathMakeString(path, &str));

        REQUIRE(StringEqual(str, &expPath));

        free(const_cast <String *> (str));
    }

    void PathNOT_Equals(const VPath * path, const std::string & e) {
        String expPath;
        StringInitCString(&expPath, e.c_str());

        const String * str = NULL;
        REQUIRE_RC(VPathMakeString(path, &str));

        REQUIRE(!StringEqual(str, &expPath));

        free(const_cast <String *> (str));
    }

    void VdbcacheNotChecked(const VPath * path = NULL) {
        if (path == NULL)
            path = this->path;

        const VPath * vdbcache = NULL;
        bool vdbcacheChecked = false;
        REQUIRE_RC(VPathGetVdbcache(path, &vdbcache, &vdbcacheChecked));

        REQUIRE(!vdbcacheChecked);
        REQUIRE_NULL(vdbcache);
    }

    void VdbcacheEquals(const std::string & e = "",
        const VPath * path = NULL)
    {
        if (path == NULL)
            path = this->path;

        if (path == NULL)
            return;

        const VPath * vdbcache = NULL;
        bool vdbcacheChecked = false;
        REQUIRE_RC(VPathGetVdbcache(path, &vdbcache, &vdbcacheChecked));

        REQUIRE(vdbcacheChecked);

        if (e.empty())
            REQUIRE_NULL(vdbcache);
        else {
            String expPath;
            StringInitCString(&expPath, e.c_str());

            const String * str = NULL;
            REQUIRE_RC(VPathMakeString(vdbcache, &str));

            REQUIRE(StringEqual(str, &expPath));

            free(const_cast <String *> (str));

            REQUIRE_RC(VPathRelease(vdbcache));
        }
    }

    void Release() {
        REQUIRE_RC(VPathRelease(path)); path = 0;
        REQUIRE_RC(VPathRelease(qLocal)); qLocal = 0;
        REQUIRE_RC(VPathRelease(qRemote)); qRemote = 0;
        REQUIRE_RC(VPathRelease(qCache)); qCache = 0;
        REQUIRE_RC(KSrvRunRelease(run)); run = 0;
        REQUIRE_RC(KSrvRunIteratorRelease(ri)); ri = 0;
        REQUIRE_RC(KSrvRespFileIteratorRelease(fi)); fi = 0;
        REQUIRE_RC(KSrvRespFileRelease(file)); file = 0;
        REQUIRE_RC(KSrvRespObjIteratorRelease(it)); it = 0;
        REQUIRE_RC(KSrvRespObjRelease(obj)); obj = 0;
        REQUIRE_RC(KSrvResponseRelease(response)); response = 0;
        REQUIRE_RC(KServiceRelease(service)); service = 0;
        REQUIRE_RC(KNSManagerRelease(kns)); kns = 0;
        REQUIRE_RC(KFileRelease(f)); f = 0;
        REQUIRE_RC(KConfigRelease(kfg)); kfg = 0;

        if (dir != NULL) { REQUIRE_RC(KDirectoryRemove(dir, true, ACC)); }

        REQUIRE_RC(KDirectoryRelease(dir)); dir = 0;
    }

    ~TRQHelper() {
        Release();

        assert(!dir && !kfg && !kns && !service && !response &&
            !obj && !it && !file && !fi && !path);
    }
};

#include "test-resolve-qual-0.hpp"
#include "test-resolve-qual-1.hpp"
#include "test-resolve-qual-2.hpp"

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char * argv [] ) {
        rc_t rc = 0;
        if (
1)          VFSManagerLogNamesServiceErrors(0, false);
        if (
0)
            rc = KDbgSetString("VFS");
        KConfigDisableUserSettings();
#ifndef USE_SERVICES_CACHE
        return rc;
#else
        if ( rc == 0 )
            rc = TestResolveQualSuite(argc, argv);
        return rc;
#endif
    }
}

/* TODO: check the case when run is found locally in several places;
    but vdbcache exists in the last checked place: this run+vdbcache should be
    selected.
   TODO: check the case when unpacked run directory is in cwd
    (with and without remote repo)
   TODO: make it work with container objects (SRP, SRS, etc.) */
