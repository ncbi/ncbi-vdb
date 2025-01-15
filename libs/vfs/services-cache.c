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
* ============================================================================*/

#include "services-cache.h" /* ServicesCache */

#include <kdb/manager.h> /* kptTable */

#include <kfg/config.h> /* KConfigRelease */

#include <kfs/directory.h> /* KDirectoryRelease */

#include <klib/container.h> /* BSTree */
#include <klib/debug.h> /* DBGMSG */
#include <klib/printf.h> /* string_printf */
#include <klib/rc.h> /* RC */
#include <klib/status.h> /* STSMSG */
#include <klib/strings.h> /* ENV_MAGIC_LOCAL */
#include <klib/text.h> /* StringWhack */

#include <kns/kns-mgr-priv.h> /* KNSManagerGetResolveToCache */
#include <kns/manager.h> /* KNSManagerRelease */

#include <vdb/database.h> /* VDatabaseRelease */
#include <vdb/manager.h> /* VDBManagerRelease */
#include <vdb/table.h> /* VTableRelease */
#include <vdb/vdb-priv.h> /* VDBManagerMakeWithVFSManager */

#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/path.h> /* VPathRelease */
#include <vfs/services.h> /* KSrvRunQuery */

#include <stdint.h> /* uint32_t */

#include "path-priv.h" /* VPathAttachVdbcache */
#include "resolver-priv.h" /* VResolverLocalForCache */
#include "services-priv.h" /* KSrvRunIteratorGetResponse */

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

/* #if WINDOWS
#define SLASH "\\"
#else
#endif */
#define SLASH "/"

struct Response4;

typedef enum {
    eIdxNo,
    eIdxYes,
    eIdxDbl,
    eIdxAsk,
    eIdxRunDir,
    eIdxMx
} EQualIdx;

typedef enum {
    eUnknown,
    eFalse,
    eTrue
} ETrinary;

typedef struct {
    VPath ** path;
    size_t allocated;
    uint32_t cnt;
    int32_t localIdx;
} Remote;

typedef struct {
    const VPath * path; /* don't release */

    VPath * magic;
    VPath * ad;
    VPath * repo;
    VPath * out;
    const VPath * resolved;

    int32_t remoteIdx;
    ETrinary obsolete;
    bool unusable;
} Local;

typedef struct {
    VPath * path;
} Cache;

/* TODO: .cache file */
typedef struct {
    VPath * path;
} CacheFile;

/* result for KSrvRunQuery() */
typedef struct {
    EQualIdx localIdx;
    EQualIdx remoteIdx;

    bool vdbcache;

    /* don't release: */
    const VPath * local;
    const VPath * remote;
    const VPath * cache;
} Result;

typedef struct KSrvRun {
    ServicesCache * dad;

    const String * acc;

    Remote remote[eIdxMx];
    Remote remoteVc[eIdxMx];

    Local local[eIdxMx];
    Local localVc[eIdxMx];

    Cache cache[eIdxMx];
    Cache cacheVc[eIdxMx];

    /* TODO: .cache files */
    CacheFile cacheFile[eIdxMx];
    CacheFile cacheVcFile[eIdxMx];

    Result result;
    KSrvRunIterator * it;
} KSrvRun;
typedef KSrvRun KRun;

/* KRun node */
typedef struct {
    BSTNode n;
    const String * acc;
    KRun * run;
} BSTItem;

/* to be used when processing KRun */
typedef struct {
    rc_t rc;
    int32_t idx;
    int64_t projectId;
    const char * outDir;
    const char * outFile;
    bool vdbcache;
} BSTData;

struct ServicesCache {
    int64_t projectId;
    const char * quality;

    KDirectory * dir;
    KConfig * kfg;
    const VFSManager * vfs;
    const KNSManager * kns;
    const VDBManager * vdb;
    VResolver * resolver;

    KRun * run;
    BSTree runs;
    KRun * disabledRun;

    BSTree responses; /* TODO */
};

/******************************************************************************/

#ifdef HAS_SERVICE_CACHE
static rc_t VPath_DetectQuality(VPath * self, ServicesCache * sc) {
    /* Try to load sra description file... */
    rc_t rc = VPathLoadQuality(self);
    if (rc == 0 && self->quality == eQualLast) {
        /* Sra description file was not found.
           Try to open run to detect quality it supports... */
        bool fullQualt = false, synthQualt = false;
        String path;
        int type = kptNotFound;
        assert(sc);
        rc = VPathGetPath(self, &path);
        if (rc == 0 && sc->vdb == NULL)
            rc = VDBManagerMakeWithVFSManager(&sc->vdb,
                sc->dir, (VFSManager*)sc->vfs);
        if (rc == 0)
         // type = VDBManagerPathType(sc->vdb, "%.*s", path.size, path.addr);
        if (rc == 0) switch (type) {
        case kptDatabase: {
            const VDatabase * db = NULL;
            rc = VDBManagerOpenDBReadVPathLight(sc->vdb, &db, NULL, self);
            if (rc == 0)
                rc = VDatabaseGetQualityCapability(db, &fullQualt, &synthQualt);
            RELEASE(VDatabase, db);
            break;
        }
        case kptTable: {
            const VTable * tbl = NULL;
            rc = VDBManagerOpenTableReadVPath(sc->vdb, &tbl, NULL, self);
            if (rc == 0)
                rc = VTableGetQualityCapability(tbl, &fullQualt, &synthQualt);
            RELEASE(VTable, tbl);
            break;
        }
        default: return 0;
        }
        if (rc == 0) {
            VQuality q = eQualLast;
            if (fullQualt && synthQualt)
                q = eQualDefault;
            else if (synthQualt)
                q = eQualNo;
            else if (fullQualt)
                q = eQualFull;
            else
                assert(0);
            rc = VPathSetQuality(self, q);
        }
    }
    return rc;
}

static rc_t VPath_SetQuality(const VPath * cself, VQuality q,
    ServicesCache * sc)
{
    VPath * self = (VPath*)cself;
    if (self == NULL)
        return 0;
    switch (q) {
    case eQualNo:
    case eQualFull:
    case eQualDefault: return VPathSetQuality(self, q);
    case eQualLast: return VPath_DetectQuality(self, sc);
    default: assert(0); return 1;
    }
}
#endif

/******************************************************************************/

/* Release Remote content */
static rc_t RemoteFini(Remote * self) {
    rc_t rc = 0;

    uint32_t i = 0;

    assert(self);

    for (i = 0; i < self->allocated; ++i)
        RELEASE(VPath, self->path[i]);

    free(self->path);

    memset(self, 0, sizeof *self);

    return rc;
}

/* initialize */
static void RemoteInit(Remote * self) {
    assert(self);

    memset(self, 0, sizeof *self);

    self->localIdx = -1;
}

static rc_t RemoteRealloc(Remote * self, bool first) {
    assert(self);

    if (self->allocated == 0) {
        uint32_t nmemb = 1;
        self->path = calloc(1, sizeof *self->path);
        if (self->path == NULL)
            return RC(rcVFS, rcStorage, rcAllocating, rcMemory, rcExhausted);
        self->allocated = nmemb;
    }

    if (first)
        return 0;

    assert(self->cnt <= self->allocated);

    if (self->cnt == self->allocated) {
        size_t nmemb = self->allocated + 1;
        void * tmp = realloc(self->path, nmemb * sizeof * self->path);
        if (tmp == NULL)
            return RC(rcVFS, rcStorage, rcAllocating, rcMemory, rcExhausted);

        self->path = tmp;
        self->allocated = nmemb;
        self->path[self->cnt] = NULL;
    }

    return 0;
}

/* Add a VPath to Remote */
static rc_t RemoteAddVPath(Remote * self, const VPath * aPath) {
    rc_t rc = 0;

    VPath * path = (VPath*)aPath;

    assert(self);

    rc = RemoteRealloc(self, false);

    if (rc == 0)
        rc = VPathAddRef(path);

    if (rc == 0)
        self->path[self->cnt++] = path;

    return rc;
}

/* Attach vdbcache-s to Remote */
static rc_t RemoteAttachVdbcache(Remote * self, const Remote * vc) {
    rc_t rc = 0;

    assert(self && self->cnt < 2 && vc && vc->cnt < 2);

    if (self->cnt == 1) {
        const VPath * vcp = NULL;
        if (vc->path != NULL)
            vcp = vc->path[0];
        rc = VPathAttachVdbcache(self->path[0], vcp);
    }

    return rc;
}

/* Set magic Path in Remote */
static rc_t RemoteSetMagicPath(Remote * self, const char * path) {
    rc_t rc = 0;
    assert(self);

    rc = RemoteRealloc(self, true);

    RELEASE(VPath, self->path[0]);

    rc = VPathMakeFmt(&self->path[0], path);
    if (rc == 0 && self->cnt == 0)
        ++self->cnt;

    return rc;
}

/* TODO */
static rc_t RemoteSameSize(
    const Remote * self, const VPath * l, bool * same)
{
    return 0;
}

/* TODO */
static rc_t RemoteHasVdbcache(const Remote * self,
    bool * has, bool * vdbcacheChecked)
{
    return 0;
}

/* TODO */
static rc_t RemoteGetVdbcache(const Remote * self,
    const VPath ** vdbcache, bool * vdbcacheChecked)
{
    return 0;
}

/******************************************************************************/

/* Release Local content */
static rc_t LocalFini(Local * self) {
    rc_t rc = 0;

    assert(self);

    RELEASE(VPath, self->ad);
    RELEASE(VPath, self->magic);
    RELEASE(VPath, self->out);
    RELEASE(VPath, self->repo);
    RELEASE(VPath, self->resolved);

    memset(self, 0, sizeof *self);

    return rc;
}

/* Initialize */
static void LocalInit(Local * self) {
    assert(self);

    memset(self, 0, sizeof *self);

    self->remoteIdx = -1;
}

/* Set magic Path in Local */
static rc_t LocalSetMagicPath(Local * self, const char * path) {
    rc_t rc = 0;
    assert(self);

    RELEASE(VPath, self->magic);
    rc = VPathMakeFmt(&self->magic, path);

    return rc;
}

/* Set resolved Path in Local */
static rc_t LocalSetResolverPath(Local * self, const VPath * path) {
    rc_t rc = 0;

    assert(self);

    RELEASE(VPath, self->resolved);

    if (rc == 0)
        self->resolved = path;

    return rc;
}

/* Set ad Path in Local */
static rc_t LocalSetAdPath(Local * self, const char * path) {
    rc_t rc = 0;

    assert(self);

    rc = VPathRelease(self->ad);

    if (rc == 0)
        rc = VPathMakeFmt(&self->ad, path);

    return rc;
}

/* Set repo Path in Local */
static rc_t LocalSetRepoPath(Local * self, const char * path) {
    rc_t rc = 0;

    assert(self);

    rc = VPathRelease(self->repo);

    if (rc == 0)
        rc = VPathMakeFmt(&self->repo, path);

    return rc;
}

/* Set out Path in Local */
static rc_t LocalSetOutPath(Local * self, const char * path) {
    rc_t rc = 0;

    assert(self);

    rc = VPathRelease(self->out);

    if (rc == 0)
        rc = VPathMakeFmt(&self->out, path);

    return rc;
}

/* Attach vdbcache-s to Local */
static rc_t LocalAttachVdbcache(Local * self, const Local * vc) {
    rc_t rc = 0;

    assert(self && vc);

    if (rc == 0)
        rc = VPathAttachVdbcache(self->magic, vc->magic);

    if (rc == 0)
        rc = VPathAttachVdbcache(self->ad, vc->ad);

    if (rc == 0)
        rc = VPathAttachVdbcache(self->repo, vc->repo);

    return rc;
}

/* Does Local have vdbcache ? */
static rc_t LocalHasVdbcache(
    const Local * self, bool * has, bool * vdbcacheChecked)
{
    rc_t rc = 0;

    const VPath * vdbcache = NULL;

    assert(self && has && vdbcacheChecked);
    *has = *vdbcacheChecked = false;

    if (self->resolved != NULL)
        rc = VPathGetVdbcache(self->resolved, &vdbcache, vdbcacheChecked);
    else if (self->ad != NULL)
        rc = VPathGetVdbcache(self->ad, &vdbcache, vdbcacheChecked);
    else
        rc = VPathGetVdbcache(self->repo, &vdbcache, vdbcacheChecked);

    if (rc == 0 && vdbcache != NULL) {
        rc = VPathRelease(vdbcache);
        *has = true;
    }

    return rc;
}

/* TODO */
static rc_t LocalAttachRemoteVdbcache(
    Local * self, const VPath * path)
{
    return 0;
}

/* Find the best local path if there are multiple */
static rc_t LocalResolve(Local * self, Local * vc) {
    assert(self && !self->path && !vc->path);

    if (self->magic != NULL) {
        self->path = self->magic;
        vc->path = vc->magic;
    }

    else if (self->out != NULL && vc->out != NULL) {
        self->path = self->out;
        vc->path = vc->out;
    }

    else if (self->resolved != NULL && vc->resolved != NULL) {
        self->path = self->resolved;
        vc->path = vc->resolved;
    }

    else if (self->ad != NULL && vc->ad != NULL) {
        self->path = self->ad;
        vc->path = vc->ad;
    }

    else if (self->repo != NULL && vc->repo != NULL) {
        self->path = self->repo;
        vc->path = vc->repo;
    }

    else if (self->out != NULL)
        self->path = self->out;

    else if(self->resolved != NULL)
        self->path = self->resolved;

    else if (self->ad != NULL)
        self->path = self->ad;

    else if (self->repo != NULL)
        self->path = self->repo;

    return VPathAttachVdbcache((VPath*)self->path, vc->path);
}

#ifdef HAS_SERVICE_CACHE
static rc_t LocalSetQuality(Local * self, VQuality quality,
    ServicesCache * sc)
{
    rc_t rc = 0, r2 = 0;
    assert(self);
    r2 = VPath_SetQuality(self->path, quality, sc);
    if (r2 != 0 && rc == 0)
        rc = r2;
    r2 = VPath_SetQuality(self->magic, quality, sc);
    if (r2 != 0 && rc == 0)
        rc = r2;
    r2 = VPath_SetQuality(self->ad, quality, sc);
    if (r2 != 0 && rc == 0)
        rc = r2;
    r2 = VPath_SetQuality(self->repo, quality, sc);
    if (r2 != 0 && rc == 0)
        rc = r2;
    r2 = VPath_SetQuality(self->out, quality, sc);
    if (r2 != 0 && rc == 0)
        rc = r2;
    r2 = VPath_SetQuality(self->resolved, quality, sc);
    if (r2 != 0 && rc == 0)
        rc = r2;
    return rc;
}
#endif

/******************************************************************************/
static rc_t CacheFini(Cache * self) {
    rc_t rc = 0;
    assert(self);
    RELEASE(VPath, self->path);
    memset(self, 0, sizeof *self);
    return rc;
}
static void CacheInit(Cache * self, EQuality quality) {}
static rc_t CacheSet/*Custom*/(Cache * self, VPath * path) {
    rc_t rc = 0;
    assert(self);
    RELEASE(VPath, self->path);
    if (rc == 0)
        rc = VPathAddRef(path);
    if (rc == 0)
        self->path = path;
    return rc;
}
/******************************************************************************/
static rc_t CacheFileFini(CacheFile * self) { return 0; }
static void CacheFileInit(CacheFile * self) {}
static rc_t CacheFileSetPath(CacheFile * self, const char * path) {
    rc_t rc = 0;
    assert(self);
    rc = VPathRelease(self->path);
    if (rc == 0)
        rc = VPathMakeFmt(&self->path, path);
    return rc;
}
/******************************************************************************/

/* Whack */
static rc_t KRunWhack(KRun * self) {
    rc_t rc = 0;

    int i = 0;

    if (self == NULL)
        return 0;

    for (i = 0; i < eIdxMx; ++i) {
        rc_t r2 = RemoteFini(&self->remote[i]);
        if (r2 != 0 && rc == 0)
            rc = r2;
        r2 = RemoteFini(&self->remoteVc[i]);
        if (r2 != 0 && rc == 0)
            rc = r2; 
        r2 = LocalFini(&self->local[i]);
        if (r2 != 0 && rc == 0)
            rc = r2;
        r2 = LocalFini(&self->localVc[i]);
        if (r2 != 0 && rc == 0)
            rc = r2;
        r2 = CacheFini(&self->cache[i]);
        if (r2 != 0 && rc == 0)
            rc = r2;
        r2 = CacheFini(&self->cacheVc[i]);
        if (r2 != 0 && rc == 0)
            rc = r2;
        r2 = CacheFileFini(&self->cacheFile[i]);
        if (r2 != 0 && rc == 0)
            rc = r2;
        r2 = CacheFileFini(&self->cacheVcFile[i]);
        if (r2 != 0 && rc == 0)
            rc = r2;
    }

    StringWhack(self->acc);

    memset(self, 0, sizeof *self);

    free(self);

    return rc;
}

/* Make */
static rc_t KRunMake(KRun ** self, const String * acc,
    ServicesCache * sc)
{
    rc_t rc = 0;

    KRun * p = NULL;

    int i = 0;

    assert(self);

    *self = NULL;

    p = calloc(1, sizeof *p);
    if (p == NULL)
        return RC(rcVFS, rcStorage, rcAllocating, rcMemory, rcExhausted);

    p->dad = sc;

    rc = StringCopy(&p->acc, acc);
    if (rc != 0) {
        free(p);
        return rc;
    }

    for (i = 0; i < eIdxMx; ++i) {
        RemoteInit(&p->remote[i]);
        RemoteInit(&p->remoteVc[i]);
        LocalInit(&p->local[i]);
        LocalInit(&p->localVc[i]);
        CacheFileInit(&p->cacheFile[i]);
        CacheFileInit(&p->cacheVcFile[i]);
    }

    p->result.localIdx = p->result.remoteIdx = eIdxMx;

    *self = p;

    return 0;
}

static rc_t KRunMakeWhenDisabled(KRun ** self,
    const ServicesCache * sc, KSrvRunIterator * it)
{
    rc_t rc = 0;
    KRun * p = NULL;

    bool found = false;

    const KSrvResponse * response = NULL; 
    uint32_t i = 0;
    uint32_t l = 0;

    assert(self);

    *self = NULL;
    
    response = KSrvRunIteratorGetResponse(it);
    l = KSrvResponseLength(response);

    for (i = 0; i < l && rc == 0; ++i) {
        const KSrvRespObj * obj = NULL;
        KSrvRespObjIterator * it = NULL;
        rc = KSrvResponseGetObjByIdx(response, i, &obj);
        if (rc == 0)
            rc = KSrvRespObjMakeIterator(obj, &it);
        while (rc == 0) {
            KSrvRespFile * file = NULL;
            rc = KSrvRespObjIteratorNextFile(it, &file);
            if (rc != 0 || file == NULL)
                break;
            found = true;
            RELEASE(KSrvRespFile, file);
            break;
        }
        RELEASE(KSrvRespObjIterator, it);
        RELEASE(KSrvRespObj, obj);
    }

    if (found) {
        p = calloc(1, sizeof *p);
        if (p == NULL)
            return RC(rcVFS, rcStorage, rcAllocating, rcMemory, rcExhausted);

        p->dad = (ServicesCache*)sc;
        p->it = it;

        *self = p;
    }

    return rc;
}

/* find path index for remote path */
static rc_t VPath_IdxForRemote(const VPath * self,
    int * idx, bool * vc, bool * notFound)
{
    rc_t rc = 0;
    String str;

    String sra, vdbcache, nq_sra, nq_vdbcache,
        q_sra, q_vdbcache, db_sra, db_vdbcache;
    CONST_STRING(&sra, "sra");
    CONST_STRING(&vdbcache, "vdbcache");
    CONST_STRING(&nq_sra, "noqual_sra");
    CONST_STRING(&nq_vdbcache, "noqual_vdbcache");
    CONST_STRING(&db_sra, "dblqual_sra");
    CONST_STRING(&db_vdbcache, "dblqual_vdbcache");
    CONST_STRING(&q_sra, "hasqual_sra");
    CONST_STRING(&q_vdbcache, "hasqual_vdbcache");

    assert(idx && vc && notFound);
    *idx = -1;
    *vc = *notFound = false;

    rc = VPathGetAcc(self, &str);
    if (rc != 0)
        return rc;
    if (str.size < 3 || str.addr == NULL || str.addr[0] == '\0'
        || str.addr[1] != 'R' || str.addr[2] != 'R')
    {
        *notFound = true;
        return 0;
    }

    rc = VPathGetType(self, &str);
    if (rc == 0) {
        if (StringCompare(&str, &sra) == 0)
            *idx = eIdxAsk;
        else if (StringCompare(&str, &vdbcache) == 0) {
            *idx = eIdxAsk;
            *vc = true;
        }

        else if (StringCompare(&str, &nq_sra) == 0)
            *idx = eIdxNo;
        else if (StringCompare(&str, &nq_vdbcache) == 0) {
            *idx = eIdxNo;
            *vc = true;
        }

        else if (StringCompare(&str, &q_sra) == 0)
            *idx = eIdxYes;
        else if (StringCompare(&str, &q_vdbcache) == 0) {
            *idx = eIdxYes;
            *vc = true;
        }

        else if (StringCompare(&str, &db_sra) == 0)
            *idx = eIdxDbl;
        else if (StringCompare(&str, &db_vdbcache) == 0) {
            *idx = eIdxDbl;
            *vc = true;
        }

        else
            rc = RC(rcVFS, rcType, rcComparing, rcType, rcUnexpected);
    }

    return rc;
}

/* resolve magic env.vars. */
static rc_t KRunResolveMagic(KRun * self) {
    rc_t rc = 0;
    const char * magic = getenv(ENV_MAGIC_REMOTE);
    const char * magicVc = getenv(ENV_MAGIC_REMOTE_VDBCACHE);
    if (magic != NULL && magic[0] != '\0') {
        rc_t r2 = RemoteSetMagicPath(&self->remote[eIdxAsk], magic);
        if (r2 != 0 && rc == 0)
            rc = r2;
        if (magicVc != NULL && magicVc[0] != '\0') {
            rc_t r2 = RemoteSetMagicPath(&self->remoteVc[eIdxAsk], magicVc);
            if (r2 != 0 && rc == 0)
                rc = r2;
        }
    }
    return rc;
}

/* Add a remote location to KRun */
static rc_t KRunAddRemote(KRun * self, const VPath * path) {
    rc_t rc = 0;

    bool vc = false;
    bool notFound = false;

    int idx = -1;
    rc = VPath_IdxForRemote(path, &idx, &vc, &notFound);
    if (rc != 0)
        return rc;
    if (notFound)
        return 0;

    if (rc == 0) {
        assert(idx >= 0 && idx < eIdxMx);
        if (vc)
            rc = RemoteAddVPath(&self->remoteVc[idx], path);
        else
            rc = RemoteAddVPath(&self->remote[idx], path);
    }

    return rc;
}

#ifdef DBGNG
#define STS_FIN  3
#endif

/* find local[-s] */
static void KRunFindLocal(KRun * self,
    int64_t projectId, const char * outDir, const char * outFile)
{
    rc_t rc = 0;
    const KConfig * kfg = NULL;
    const KDirectory * dir = NULL;
    const ServicesCache * sc = NULL;
    char path[PATH_MAX] = "";
#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: entered", __func__));
#endif
    assert(self && self->dad);
    sc = self->dad;
    dir = sc->dir;
    kfg = sc->kfg;

    if (outFile != NULL) {
        rc_t r2 = 0;

#ifdef DBGNG
        STSMSG(STS_FIN, ("%s: outFile != NULL...", __func__));
#endif

        r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
            "%s", outFile);
        if (r2 == 0 &&
            (KDirectoryPathType(dir, path) & ~kptAlias) == kptFile)
        {
            r2 = LocalSetOutPath(&self->local[eIdxAsk], path);
            if (r2 != 0 && rc == 0)
                rc = r2;
        }
    }

    else if (outDir != NULL) {
        rc_t r2 = 0;

#ifdef DBGNG
        STSMSG(STS_FIN, ("%s: outDir != NULL...", __func__));
#endif

        r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
            "%s" SLASH "%.*s.noqual.sra", outDir, self->acc->size, self->acc->addr);
        if (r2 == 0 &&
            (KDirectoryPathType(dir, path) & ~kptAlias) == kptFile)
        {
            r2 = LocalSetOutPath(&self->local[eIdxNo], path);
            if (r2 != 0 && rc == 0)
                rc = r2;
        }
        r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
            "%s" SLASH "%.*s.hasqual.sra", outDir, self->acc->size, self->acc->addr);
        if (r2 == 0 &&
            (KDirectoryPathType(dir, path) & ~kptAlias) == kptFile)
        {
            r2 = LocalSetOutPath(&self->local[eIdxYes], path);
            if (r2 != 0 && rc == 0)
                rc = r2;
        }
        r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
            "%s" SLASH "%.*s.dblqual.sra", outDir, self->acc->size, self->acc->addr);
        if (r2 == 0 &&
            (KDirectoryPathType(dir, path) & ~kptAlias) == kptFile)
        {
            r2 = LocalSetOutPath(&self->local[eIdxDbl], path);
            if (r2 != 0 && rc == 0)
                rc = r2;
        }
        r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
            "%s" SLASH "%.*s.sra", outDir, self->acc->size, self->acc->addr);
        if (r2 == 0 &&
            (KDirectoryPathType(dir, path) & ~kptAlias) == kptFile)
        {
            r2 = LocalSetOutPath(&self->local[eIdxAsk], path);
            if (r2 != 0 && rc == 0)
                rc = r2;
        }

        r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
            "%s" SLASH "%.*s.noqual.sra.vdbcache",
            outDir, self->acc->size, self->acc->addr);
        if (r2 == 0 &&
            (KDirectoryPathType(dir, path) & ~kptAlias) == kptFile)
        {
            r2 = LocalSetOutPath(&self->localVc[eIdxNo], path);
            if (r2 != 0 && rc == 0)
                rc = r2;
        }
        r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
            "%s" SLASH "%.*s.hasqual.sra.vdbcache",
            outDir, self->acc->size, self->acc->addr);
        if (r2 == 0 &&
            (KDirectoryPathType(dir, path) & ~kptAlias) == kptFile)
        {
            r2 = LocalSetOutPath(&self->localVc[eIdxYes], path);
            if (r2 != 0 && rc == 0)
                rc = r2;
        }
        r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
            "%s" SLASH "%.*s.dblqual.sra.vdbcache",
            outDir, self->acc->size, self->acc->addr);
        if (r2 == 0 &&
            (KDirectoryPathType(dir, path) & ~kptAlias) == kptFile)
        {
            r2 = LocalSetOutPath(&self->localVc[eIdxDbl], path);
            if (r2 != 0 && rc == 0)
                rc = r2;
        }
        r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
            "%s" SLASH "%.*s.sra.vdbcache",
            outDir, self->acc->size, self->acc->addr);
        if (r2 == 0 &&
            (KDirectoryPathType(dir, path) & ~kptAlias) == kptFile)
        {
            r2 = LocalSetOutPath(&self->localVc[eIdxAsk], path);
            if (r2 != 0 && rc == 0)
                rc = r2;
        }
    }

    else {
        String * volume = NULL;
        String * root = NULL;

        const char * magic = getenv(ENV_MAGIC_LOCAL);
        const char * magicVc = getenv(ENV_MAGIC_LOCAL_VDBCACHE);

#ifdef DBGNG
        STSMSG(STS_FIN, ("%s: outFile & outDir = NULL...", __func__));
#endif
        if (magic != NULL && magic[0] != '\0') {
            rc_t r2 = LocalSetMagicPath(&self->local[eIdxAsk], magic);
            if (r2 != 0 && rc == 0)
                rc = r2;
            if (magicVc != NULL && magicVc[0] != '\0') {
                rc_t r2 = LocalSetMagicPath(&self->localVc[eIdxAsk], magicVc);
                if (r2 != 0 && rc == 0)
                    rc = r2;
            }
        }

        if (sc->resolver != NULL) {
            VPath * accession = NULL;
            const VPath * path = NULL;
            rc_t r2 = 0;
#ifdef DBGNG
            STSMSG(STS_FIN, ("%s: sc->resolver != NULL...", __func__));
#endif
            r2 = VPathMake(&accession, self->acc->addr);
            if (r2 == 0)
                r2 = VResolverLocalForCache(sc->resolver, accession, &path);
            if (r2 == 0) {
                r2 = LocalSetResolverPath(&self->local[eIdxAsk], path);
                if (r2 != 0) {
                    if (rc == 0)
                        rc = r2;
                }
                else {
                    bool vdbcacheChecked = false;
                    const VPath * vdbcache = NULL;
                    r2 = VPathGetVdbcache(path, &vdbcache, &vdbcacheChecked);
                    if (r2 == 0 && vdbcache != NULL) {
                        r2 = LocalSetResolverPath(&self->localVc[eIdxAsk],
                            vdbcache);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                }
            }
            RELEASE(VPath, accession);
        }

        KConfigReadString(kfg,
            "/repository/user/main/public/apps/sra/volumes/sraFlat", &volume);
        KConfigReadString(kfg, "/repository/user/main/public/root", &root);
        if ((KDirectoryPathType(dir, "%.*s", self->acc->size, self->acc->addr)
            & ~kptAlias) == kptDir)
        {
#ifdef DBGNG
            STSMSG(STS_FIN, ("%s: KDirectoryPathType == kptDir...",__func__));
#endif
            if (projectId < 0) {
                rc_t r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s.noqual.sra", self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                    == kptFile)
                {
                    r2 = LocalSetAdPath(&self->local[eIdxNo], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s.hasqual.sra", self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                    == kptFile)
                {
                    r2 = LocalSetAdPath(&self->local[eIdxYes], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s.dblqual.sra", self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                    == kptFile)
                {
                    r2 = LocalSetAdPath(&self->local[eIdxDbl], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s.sra", self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                    == kptFile)
                {
                    r2 = LocalSetAdPath(&self->local[eIdxAsk], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }

                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s.noqual.sra.vdbcache",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                    == kptFile)
                {
                    r2 = LocalSetAdPath(&self->localVc[eIdxNo], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s.hasqual.sra.vdbcache",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                    == kptFile)
                {
                    r2 = LocalSetAdPath(&self->localVc[eIdxYes], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s.dblqual.sra.vdbcache",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                    == kptFile)
                {
                    r2 = LocalSetAdPath(&self->localVc[eIdxDbl], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s.sra.vdbcache",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                    == kptFile)
                {
                    r2 = LocalSetAdPath(&self->localVc[eIdxAsk], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
            }

            else {
                rc_t r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s_dbGaP-%d.noqual.sra",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr, sc->projectId);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias) == kptFile)
                {
                    r2 = LocalSetAdPath(&self->local[eIdxNo], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s_dbGaP-%d.hasqual.sra",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr, sc->projectId);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                    == kptFile)
                {
                    r2 = LocalSetAdPath(&self->local[eIdxYes], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s_dbGaP-%d.dblqual.sra",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr, sc->projectId);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                    == kptFile)
                {
                    r2 = LocalSetAdPath(&self->local[eIdxDbl], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s_dbGaP-%d.sra",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr, sc->projectId);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                        == kptFile)
                {
                    r2 = LocalSetAdPath(&self->local[eIdxAsk], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }

                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s_dbGaP-%d.noqual.sra.vdbcache",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr, sc->projectId);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                        == kptFile)
                {
                    r2 = LocalSetAdPath(&self->local[eIdxNo], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s_dbGaP-%d.hasqual.sra.vdbcache",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr, sc->projectId);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                        == kptFile)
                {
                    r2 = LocalSetAdPath(&self->localVc[eIdxYes], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s_dbGaP-%d.dblqual.sra.vdbcache",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr, sc->projectId);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                        == kptFile)
                {
                    r2 = LocalSetAdPath(&self->localVc[eIdxDbl], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
                r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                    "%.*s" SLASH "%.*s_dbGaP-%d.sra.vdbcache",
                    self->acc->size, self->acc->addr,
                    self->acc->size, self->acc->addr, sc->projectId);
                if (r2 == 0 &&
                    (KDirectoryPathType(dir, path) & ~kptAlias)
                        == kptFile)
                {
                    r2 = LocalSetAdPath(&self->localVc[eIdxAsk], path);
                    if (r2 != 0 && rc == 0)
                        rc = r2;
                }
            }
        }

        if (root != NULL && volume != NULL) {
            if ((KDirectoryPathType(dir, "%.*s" SLASH "%.*s", root->size, root->addr,
                volume->size, volume->addr) & ~kptAlias) == kptDir)
            {
#ifdef DBGNG
                STSMSG(STS_FIN, ("%s: root != NULL && volume != NULL "
                    "&& KDirectoryPathType == kptDir...", __func__));
#endif
                if (projectId < 0) {
                    rc_t r2 = 0;

#ifdef DBGNG
                    STSMSG(STS_FIN, ("%s: projectId < 0...", __func__));
#endif

                    r2 = KDirectoryResolvePath(dir, true,
                        path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.noqual.sra%s", root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, "");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->local[eIdxNo], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.hasqual.sra%s", root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, "");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->local[eIdxYes], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.dblqual.sra%s", root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, "");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->local[eIdxDbl], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.sra%s", root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, "");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->local[eIdxAsk], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }

                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.noqual.sra.vdbcache%s",
                        root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, "");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->localVc[eIdxNo], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.hasqual.sra.vdbcache%s",
                        root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, "");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->localVc[eIdxYes], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.dblqual.sra.vdbcache%s",
                        root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, "");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->localVc[eIdxDbl], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.sra.vdbcache%s", root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, "");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->localVc[eIdxAsk], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }

                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.noqual.sra%s", root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, ".cache");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = CacheFileSetPath(&self->cacheFile[eIdxNo], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.hasqual.sra%s",
                        root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, ".cache");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = CacheFileSetPath(&self->cacheFile[eIdxYes], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.dblqual.sra%s",
                        root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, ".cache");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = CacheFileSetPath(&self->cacheFile[eIdxDbl], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.sra%s", root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, ".cache");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = CacheFileSetPath(&self->cacheFile[eIdxAsk], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }

                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.noqual.sra.vdbcache%s",
                        root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, ".cache");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = CacheFileSetPath(&self->cacheVcFile[eIdxNo], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.hasqual.sra.vdbcache%s",
                        root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, ".cache");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = CacheFileSetPath(&self->cacheVcFile[eIdxYes],
                            path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.dblqual.sra.vdbcache%s",
                        root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, ".cache");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = CacheFileSetPath(&self->cacheVcFile[eIdxDbl],
                            path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s.sra.vdbcache%s",
                        root->size, root->addr,
                        volume->size, volume->addr,
                        self->acc->size, self->acc->addr, ".cache");
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = CacheFileSetPath(&self->cacheVcFile[eIdxAsk],
                            path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                }

                else {
                rc_t r2 = 0;
                
#ifdef DBGNG
                STSMSG(STS_FIN, ("%s: projectId == %d...", __func__,
                    projectId));
#endif

                r2 = KDirectoryResolvePath(dir, true,
                        path, sizeof path, "%.*s" SLASH "%.*s" SLASH "%.*s_dbGaP-%d.noqual.sra",
                        root->size, root->addr, volume->size, volume->addr,
                        self->acc->size, self->acc->addr, sc->projectId);
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->local[eIdxNo], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s_dbGaP-%d.hasqual.sra",
                        root->size, root->addr, volume->size, volume->addr,
                        self->acc->size, self->acc->addr, sc->projectId);
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->local[eIdxYes], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s_dbGaP-%d.dblqual.sra",
                        root->size, root->addr, volume->size, volume->addr,
                        self->acc->size, self->acc->addr, sc->projectId);
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->local[eIdxDbl], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s_dbGaP-%d.sra",
                        root->size, root->addr, volume->size, volume->addr,
                        self->acc->size, self->acc->addr, sc->projectId);
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->local[eIdxAsk], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }

                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s_dbGaP-%d.noqual.sra.vdbcache",
                        root->size, root->addr, volume->size, volume->addr,
                        self->acc->size, self->acc->addr, sc->projectId);
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->local[eIdxNo], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s_dbGaP-%d.hasqual.sra.vdbcache",
                        root->size, root->addr, volume->size, volume->addr,
                        self->acc->size, self->acc->addr, sc->projectId);
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->localVc[eIdxYes], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s_dbGaP-%d.dblqual.sra.vdbcache",
                        root->size, root->addr, volume->size, volume->addr,
                        self->acc->size, self->acc->addr, sc->projectId);
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->localVc[eIdxDbl], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                    r2 = KDirectoryResolvePath(dir, true, path, sizeof path,
                        "%.*s" SLASH "%.*s" SLASH "%.*s_dbGaP-%d.sra.vdbcache",
                        root->size, root->addr, volume->size, volume->addr,
                        self->acc->size, self->acc->addr, sc->projectId);
                    if (r2 == 0 &&
                        (KDirectoryPathType(dir, path) & ~kptAlias)
                            == kptFile)
                    {
                        r2 = LocalSetRepoPath(&self->localVc[eIdxAsk], path);
                        if (r2 != 0 && rc == 0)
                            rc = r2;
                    }
                }
            }
        }

        StringWhack(volume);
        StringWhack(root);
    }

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: exiting with void", __func__));
#endif
}

#ifdef HAS_SERVICE_CACHE
/* set qualities to local[-s]  */
static rc_t KRunSetQualities(KRun * self) {
    static int transate[] = {
        eQualNo, /* eIdxNo */
        eQualFull, /* eIdxYes */
        eQualDefault, /* eIdxDbl */
        eQualLast, /* eIdxAsk */
        eQualLast, /* eIdxRunDir */
    };
    rc_t rc = 0;
    int i = 0;
    assert(self && sizeof transate / sizeof transate[0] == eIdxMx);
    for (i = 0; i < eIdxMx; ++i) {
        rc_t r2 = LocalSetQuality(&self->local[i], transate[i], self->dad);
        if (r2 != 0 && rc == 0)
            rc = r2;
    }
    return rc;
}
#endif

#ifdef HAS_SERVICE_CACHE
/* resolve local[-s] */
static rc_t KRunResolveLocals(KRun * self,
    int64_t projectId, const char * outDir, const char * outFile)
{
    KRunFindLocal(self, projectId, outDir, outFile);
    return KRunSetQualities(self);
}
#endif

/* attach vdbcaches to local[-s] and remote[-s] */
static rc_t KRunAttachVdbcaches(KRun * self) {
    rc_t rc = 0;

    int i = 0;

    assert(self);

    for (i = 0; i < eIdxMx; ++i) {
        rc_t r2 = RemoteAttachVdbcache(&self->remote[i], &self->remoteVc[i]);
        if (r2 != 0 && rc == 0)
            rc = r2;

        r2 = LocalAttachVdbcache(&self->local[i], &self->localVc[i]);
        if (r2 != 0 && rc == 0)
            rc = r2;
    }

    return rc;
}

/* find the best local if there are multiple */
static rc_t KRunLocalResolve(KRun * self) {
    rc_t rc = 0;

    int i = 0;

    assert(self);

    for (i = 0; i < eIdxMx && rc == 0; ++i)
        rc = LocalResolve(&self->local[i], &self->localVc[i]);

    return rc;
}

/* link remote and local location: to be used by file iterator */
static void KRunLinkRemoteToLocal(
    KRun * self, int32_t remoteIdx, int32_t localIdx)
{
    assert(self);

    if (self->local[localIdx].remoteIdx < 0)
        self->local[localIdx].remoteIdx = remoteIdx;

    if (self->remote[remoteIdx].localIdx < 0)
        self->remote[remoteIdx].localIdx = localIdx;
}

/* local is obsolete (remote was updated) */
static void KRunLinkObsoleteRemoteToLocal(
    KRun * self, int32_t remoteIdx, int32_t localIdx)
{
    KRunLinkRemoteToLocal(self, remoteIdx, localIdx);

    assert(self);
    self->local[localIdx].obsolete = eTrue;
}

/* find local for each remote */
static rc_t KRunLinkLocalsToRemotes(KRun * self) {
    rc_t rc = 0;
    int i = 0;
    bool same = false;
    bool has = false, vdbcacheChecked = false;
    assert(self);

    /***************************** RUNS ***************************************/
    /* remote[eIdxNo] <-> local[eIdxNo] */
    if (self->remote[eIdxNo].cnt > 0)
        if (self->local[eIdxNo].ad != NULL || self->local[eIdxNo].repo != NULL)
            KRunLinkRemoteToLocal(self, eIdxNo, eIdxNo);

    /* remote[eIdxYes] <-> */
    if (self->remote[eIdxYes].cnt > 0) {
        /* remote[eIdxYes] <-> local[eIdxYes] */
        if (self->local[eIdxYes].ad != NULL || self->local[eIdxYes].repo != 0)
            KRunLinkRemoteToLocal(self, eIdxYes, eIdxYes);
        /* remote[eIdxYes] <-> local[eIdxAsk] */
        same = false;
        if (self->local[eIdxAsk].ad != NULL) {
            rc_t r2 = RemoteSameSize(&self->remote[eIdxYes],
                self->local[eIdxAsk].ad, &same);
            if (r2 != 0) {
                if (rc == 0)
                    rc = r2;
            }
            else if (same) {
                RELEASE(VPath, self->local[eIdxAsk].repo);
                KRunLinkRemoteToLocal(self, eIdxYes, eIdxAsk);
            }
        }
        if (!same && self->local[eIdxAsk].repo != NULL) {
            rc_t r2 = RemoteSameSize(&self->remote[eIdxYes],
                self->local[eIdxAsk].repo, &same);
            if (r2 != 0) {
                if (rc == 0)
                    rc = r2;
            }
            else if (same) {
                RELEASE(VPath, self->local[eIdxAsk].ad);
                KRunLinkRemoteToLocal(self, eIdxYes, eIdxAsk);
            }
        }
        if (!same &&
            (self->local[eIdxYes].ad != NULL || self->local[eIdxYes].repo != 0)
            &&
            (self->local[eIdxAsk].ad != NULL || self->local[eIdxAsk].repo != 0)
            )
        {
            KRunLinkObsoleteRemoteToLocal(self, eIdxYes, eIdxAsk);
        }
    }

    /* remote[eIdxDbl] <-> */
    if (self->remote[eIdxDbl].cnt > 0) {
        /* remote[eIdxDbl] <-> local[eIdxDbl] */
        if (self->local[eIdxDbl].ad != NULL || self->local[eIdxDbl].repo != 0)
            KRunLinkRemoteToLocal(self, eIdxDbl, eIdxDbl);
        /* remote[eIdxDbl] <-> local[eIdxAsk] */
        same = false;
        if (self->local[eIdxAsk].ad != NULL) {
            rc_t r2 = RemoteSameSize(&self->remote[eIdxDbl],
                self->local[eIdxAsk].ad, &same);
            if (r2 != 0) {
                if (rc == 0)
                    rc = r2;
            }
            else if (same) {
                RELEASE(VPath, self->local[eIdxAsk].repo);
                KRunLinkRemoteToLocal(self, eIdxDbl, eIdxAsk);
            }
        }
        if (!same && self->local[eIdxAsk].repo != NULL) {
            rc_t r2 = RemoteSameSize(&self->remote[eIdxDbl],
                self->local[eIdxAsk].repo, &same);
            if (r2 != 0) {
                if (rc == 0)
                    rc = r2;
            }
            else if (same) {
                RELEASE(VPath, self->local[eIdxAsk].ad);
                KRunLinkRemoteToLocal(self, eIdxDbl, eIdxAsk);
            }
        }
        if (!same &&
            (self->local[eIdxDbl].ad != NULL || self->local[eIdxDbl].repo != 0)
            &&
            (self->local[eIdxAsk].ad != NULL || self->local[eIdxAsk].repo != 0)
            )
        {
            KRunLinkObsoleteRemoteToLocal(self, eIdxDbl, eIdxAsk);
        }
    }

    /* remote[eIdxAsk] <-> */
    if (self->remote[eIdxAsk].cnt > 0) {
        /* remote[eIdxAsk] <-> local[eIdxYes|eIdxDbl] */
        if (self->local[eIdxYes].remoteIdx >= 0)
            KRunLinkRemoteToLocal(self, eIdxAsk, eIdxYes);
        else if (self->local[eIdxDbl].remoteIdx >= 0)
            KRunLinkRemoteToLocal(self, eIdxAsk, eIdxDbl);
        /* remote[eIdxAsk] <-> local[eIdxAsk] */
        else if (self->local[eIdxAsk].resolved != 0 ||
            self->local[eIdxAsk].ad != 0 || self->local[eIdxAsk].repo != 0)
        {
            KRunLinkRemoteToLocal(self, eIdxAsk, eIdxAsk);
        }
    }

    for (i = 0; i < eIdxMx; ++i) {
        int32_t remoteIdx = self->local[i].remoteIdx;
        if (remoteIdx >= 0)
            if (self->local[i].obsolete == eUnknown) {
                bool same = false;
                if (self->local[i].ad != NULL) {
                    rc_t r2 = RemoteSameSize(&self->remote[remoteIdx],
                        self->local[i].ad, &same);
                    if (r2 != 0) {
                        if (rc == 0)
                            rc = r2;
                    }
                    else if (same) {
                        RELEASE(VPath, self->local[i].repo);
                        self->local[i].obsolete = eFalse;
                    }
                }
                if (!same && self->local[i].repo != NULL) {
                    rc_t r2 = RemoteSameSize(&self->remote[remoteIdx],
                        self->local[i].repo, &same);
                    if (r2 != 0) {
                        if (rc == 0)
                            rc = r2;
                    }
                    else if (same) {
                        RELEASE(VPath, self->local[i].ad);
                        self->local[i].obsolete = eFalse;
                    }
                }
                if (!same)
                    self->local[i].obsolete = eTrue;
            }
    }

    /***************************** VDBCACHES **********************************/
    /* remoteVc[eIdxNo] <-> localVc[eIdxNo] */
    if (self->remoteVc[eIdxNo].cnt > 0) {
        if (self->localVc[eIdxNo].ad != NULL || self->localVc[eIdxNo].repo != 0)
            KRunLinkRemoteToLocal(self, eIdxNo, eIdxNo);
    }

    /* remoteVc[eIdxYes] <-> */
    if (self->remoteVc[eIdxYes].cnt > 0) {
        /* remoteVc[eIdxYes] <-> localVc[eIdxYes] */
        if (self->localVc[eIdxYes].ad != 0 || self->localVc[eIdxYes].repo != 0)
            KRunLinkRemoteToLocal(self, eIdxYes, eIdxYes);
        /* remoteVc[eIdxYes] <-> localVc[eIdxAsk] */
        same = false;
        if (self->localVc[eIdxAsk].ad != NULL) {
            rc_t r2 = RemoteSameSize(&self->remoteVc[eIdxYes],
                self->localVc[eIdxAsk].ad, &same);
            if (r2 != 0) {
                if (rc == 0)
                    rc = r2;
            }
            else if (same) {
                RELEASE(VPath, self->localVc[eIdxAsk].repo);
                KRunLinkRemoteToLocal(self, eIdxYes, eIdxAsk);
            }
        }
        if (!same && self->localVc[eIdxAsk].repo != NULL) {
            rc_t r2 = RemoteSameSize(&self->remoteVc[eIdxYes],
                self->localVc[eIdxAsk].repo, &same);
            if (r2 != 0) {
                if (rc == 0)
                    rc = r2;
            }
            else if (same) {
                RELEASE(VPath, self->localVc[eIdxAsk].ad);
                KRunLinkRemoteToLocal(self, eIdxYes, eIdxAsk);
            }
        }
        if (!same &&
            (self->localVc[eIdxYes].ad != 0 || self->localVc[eIdxYes].repo != 0)
            &&
            (self->localVc[eIdxAsk].ad != 0 || self->localVc[eIdxAsk].repo != 0)
            )
        {
            KRunLinkObsoleteRemoteToLocal(self, eIdxYes, eIdxAsk);
        }
    }

    /* remoteVc[eIdxDbl] <-> */
    if (self->remoteVc[eIdxDbl].cnt > 0) {
        /* remoteVc[eIdxDbl] <-> localVc[eIdxDbl] */
        if (self->localVc[eIdxDbl].ad != 0 || self->localVc[eIdxDbl].repo != 0)
            KRunLinkRemoteToLocal(self, eIdxDbl, eIdxDbl);
        /* remoteVc[eIdxDbl] <-> localVc[eIdxAsk] */
        same = false;
        if (self->localVc[eIdxAsk].ad != NULL) {
            rc_t r2 = RemoteSameSize(&self->remoteVc[eIdxDbl],
                self->localVc[eIdxAsk].ad, &same);
            if (r2 != 0) {
                if (rc == 0)
                    rc = r2;
            }
            else if (same) {
                RELEASE(VPath, self->localVc[eIdxAsk].repo);
                KRunLinkRemoteToLocal(self, eIdxDbl, eIdxAsk);
            }
        }
        if (!same && self->localVc[eIdxAsk].repo != NULL) {
            rc_t r2 = RemoteSameSize(&self->remoteVc[eIdxDbl],
                self->localVc[eIdxAsk].repo, &same);
            if (r2 != 0) {
                if (rc == 0)
                    rc = r2;
            }
            else if (same) {
                RELEASE(VPath, self->localVc[eIdxAsk].ad);
                KRunLinkRemoteToLocal(self, eIdxDbl, eIdxAsk);
            }
        }
        if (!same &&
            (self->localVc[eIdxDbl].ad != 0 || self->localVc[eIdxDbl].repo != 0)
            &&
            (self->localVc[eIdxAsk].ad != 0 || self->localVc[eIdxAsk].repo != 0)
            )
        {
            KRunLinkObsoleteRemoteToLocal(self, eIdxDbl, eIdxAsk);
        }
    }

    /* remoteVc[eIdxAsk] <-> */
    if (self->remoteVc[eIdxAsk].cnt > 0) {
        /* remoteVc[eIdxAsk] <-> localVc[eIdxYes|eIdxDbl] */
        if (self->localVc[eIdxYes].remoteIdx >= 0)
            KRunLinkRemoteToLocal(self, eIdxAsk, eIdxYes);
        else if (self->localVc[eIdxDbl].remoteIdx >= 0)
            KRunLinkRemoteToLocal(self, eIdxAsk, eIdxDbl);
        /* remoteVc[eIdxAsk] <-> localVc[eIdxAsk] */
        else if (self->localVc[eIdxAsk].ad != NULL ||
            self->localVc[eIdxAsk].repo != NULL)
        {
            KRunLinkRemoteToLocal(self, eIdxAsk, eIdxAsk);
        }
    }

    for (i = 0; i < eIdxMx; ++i) {
        int32_t remoteIdx = self->localVc[i].remoteIdx;
        if (remoteIdx >= 0)
            if (self->localVc[i].obsolete == eUnknown) {
                bool same = false;
                if (self->localVc[i].ad != NULL) {
                    rc_t r2 = RemoteSameSize(&self->remoteVc[remoteIdx],
                        self->localVc[i].ad, &same);
                    if (r2 != 0) {
                        if (rc == 0)
                            rc = r2;
                    }
                    else if (same) {
                        RELEASE(VPath, self->localVc[i].repo);
                        self->localVc[i].obsolete = eFalse;
                    }
                }
                if (!same && self->localVc[i].repo != NULL) {
                    rc_t r2 = RemoteSameSize(&self->remoteVc[remoteIdx],
                        self->localVc[i].repo, &same);
                    if (r2 != 0) {
                        if (rc == 0)
                            rc = r2;
                    }
                    else if (same) {
                        RELEASE(VPath, self->localVc[i].ad);
                        self->localVc[i].obsolete = eFalse;
                    }
                }
                if (!same)
                    self->localVc[i].obsolete = eTrue;
            }
    }

    /***************************** DETECT UNUSABLE LOCALS *********************/
    for (i = 0; i < eIdxMx; ++i) {
        int32_t remoteIdx = self->local[i].remoteIdx;
        if (remoteIdx >= 0)
            if (self->local[i].obsolete == eTrue) {
                rc_t r2 = LocalHasVdbcache(
                    &self->local[i], &has, &vdbcacheChecked);
                if (r2 != 0) {
                    if (rc == 0)
                        rc = r2;
                }
                else {
                    assert(vdbcacheChecked);
                    if (!has) {
                        r2 = RemoteHasVdbcache(&self->remote[remoteIdx],
                            &has, &vdbcacheChecked);
                        if (r2 != 0) {
                            if (rc == 0)
                                rc = r2;
                        }
                        else {
                            assert(vdbcacheChecked);
                            if (has)
                                self->local[i].unusable = true;
                        }
                    }
                }
            }
    }

    /***************************** LINK REMOTE VDBCACHES **********************/
    for (i = 0; i < eIdxMx; ++i) {
        int32_t remoteIdx = self->local[i].remoteIdx;
        if (remoteIdx >= 0)
            if (!self->local[i].unusable) {
                rc_t r2 = LocalHasVdbcache(
                    &self->local[i], &has, &vdbcacheChecked);
                if (r2 != 0) {
                    if (rc == 0)
                        rc = r2;
                }
                else {
                    assert(vdbcacheChecked);
                    if (!has) {
                        const VPath * vdbcache = NULL;
                        r2 = RemoteGetVdbcache(&self->remote[remoteIdx],
                            &vdbcache, &vdbcacheChecked);
                        if (r2 != 0) {
                            if (rc == 0)
                                rc = r2;
                        }
                        else {
                            assert(vdbcacheChecked);
                            r2 = LocalAttachRemoteVdbcache(
                                &self->local[i], vdbcache);
                            if (r2 != 0 && rc == 0)
                                rc = r2;
                        }
                    }
                }
            }
    }

    return rc;
}

/* find cache for remote */
static rc_t KRunCacheForRemote(KRun * self, int32_t idx, bool vdbcache,
    const char * outDir, const char * outFile)
{
    rc_t rc = 0;
    bool resolveToCache = false;
    bool adCaching = false;
    String * volume = NULL;
    String * root = NULL;
    char path[PATH_MAX] = "";
    char rslvd[PATH_MAX] = "";
    ServicesCache * sc = NULL;
    const char * p = NULL;
    bool resolved = false;
    const VFSManager * m = (VFSManager*)1;

    assert(self);

    p = getenv(ENV_MAGIC_CACHE);
    if (p != NULL) {
        resolved = true;

        if (vdbcache) {
            const char * c = getenv(ENV_MAGIC_CACHE_VDBCACHE);
            if (c != NULL)
                rc = VFSManagerMakePath(m, &(self->cacheVc[idx].path), "%s", c);
            else
                rc = VFSManagerMakePath(m, &(self->cacheVc[idx].path),
                    "%s.vdbcache", p);
        }
        else
            rc = VFSManagerMakePath(m, &(self->cache[idx].path), "%s", p);
    }

    sc = self->dad;

    if (rc == 0 && sc->kns == NULL)
        rc = KNSManagerMake((KNSManager**)&sc->kns);
    if (rc == 0)
        rc = KNSManagerGetResolveToCache(sc->kns, &resolveToCache);
    if (rc == 0)
        rc = KNSManagerGetAdCaching(sc->kns, &adCaching);

    if (rc == 0) {
        if (!resolved && outFile != NULL) {
            resolved = true;
            rc = string_printf(path, sizeof path, NULL, "%s", outFile);
            if (rc == 0)
                rc = KDirectoryResolvePath(sc->dir, true, rslvd, sizeof rslvd,
                    path);
            if (rc == 0) {
                if (vdbcache)
                    rc = VPathMakeFmt(&(self->cacheVc[idx].path), rslvd);
                else
                    rc = VPathMakeFmt(&(self->cache[idx].path), rslvd);
            }
            else {
                if (vdbcache)
                    rc = VPathMakeFmt(&(self->cacheVc[idx].path), path);
                else
                    rc = VPathMakeFmt(&(self->cache[idx].path), path);
            }
        }

        if (!resolved && outDir != NULL) {
            resolved = true;
            rc = string_printf(path, sizeof path, NULL, "%s" SLASH "%S.%s%s",
                outDir, self->acc, idx == eIdxNo ? "noqual.sra" : "sra",
                vdbcache ? ".vdbcache" : "");
            if (rc == 0)
                rc = KDirectoryResolvePath(sc->dir, true, rslvd, sizeof rslvd,
                    path);
            if (rc == 0) {
                if (vdbcache)
                    rc = VPathMakeFmt(&(self->cacheVc[idx].path), rslvd);
                else
                    rc = VPathMakeFmt(&(self->cache[idx].path), rslvd);
            }
            else {
                if (vdbcache)
                    rc = VPathMakeFmt(&(self->cacheVc[idx].path), path);
                else
                    rc = VPathMakeFmt(&(self->cache[idx].path), path);
            }
        }

        if (!resolved && resolveToCache) {
            if (sc->kfg == NULL)
                rc = KConfigMake(&sc->kfg, sc->dir);
            if (rc == 0) {
                KConfigReadString(sc->kfg,
                    "/repository/user/main/public/apps/sra/volumes/sraFlat",
                    &volume);
                KConfigReadString(sc->kfg, "/repository/user/main/public/root",
                    &root);
            }
            if (rc == 0 && volume != NULL && volume->size != 0
                && root != NULL && root->size != 0)
            {
                if (sc->projectId >= 0)
                    rc = string_printf(path, sizeof path, NULL,
                        "%S" SLASH "%S" SLASH "%S_dbGaP-%d.%s%s",
                        root, volume, self->acc, sc->projectId,
                        idx == eIdxNo ? "noqual.sra" : "sra",
                        vdbcache ? ".vdbcache" : "");
                else
                    rc = string_printf(path, sizeof path, NULL, "%S" SLASH "%S" SLASH "%S.%s%s",
                        root, volume, self->acc,
                        idx == eIdxNo ? "noqual.sra" : "sra",
                        vdbcache ? ".vdbcache" : "");
                if (rc == 0) {
                    rc = KDirectoryResolvePath(sc->dir, true,
                        rslvd, sizeof rslvd, path);
                    if (rc == 0) {
                        if (vdbcache)
                            rc = VPathMakeFmt(&(self->cacheVc[idx].path), rslvd);
                        else
                            rc = VPathMakeFmt(&(self->cache[idx].path), rslvd);
                    }
                    else {
                        if (vdbcache)
                            rc = VPathMakeFmt(&(self->cacheVc[idx].path), path);
                        else
                            rc = VPathMakeFmt(&(self->cache[idx].path), path);
                    }
                    resolved = true;
                }
            }
        }

        if (rc == 0 && !resolved && adCaching) {
            if (sc->projectId >= 0)
                rc = string_printf(path, sizeof path, NULL,
                    "%S" SLASH "%S_dbGaP-%d.%s%s", self->acc, self->acc, sc->projectId,
                    idx == eIdxNo ? "noqual.sra" : "sra",
                    vdbcache ? ".vdbcache" : "");
            else
                rc = string_printf(path, sizeof path, NULL, "%S" SLASH "%S.%s%s",
                    self->acc, self->acc, idx == eIdxNo ? "noqual.sra" : "sra",
                    vdbcache ? ".vdbcache" : "");
            if (rc == 0)
                rc = KDirectoryResolvePath(sc->dir, true, rslvd, sizeof rslvd,
                    path);
            if (rc == 0) {
                if (vdbcache)
                    rc = VPathMakeFmt(&(self->cacheVc[idx].path), rslvd);
                else
                    rc = VPathMakeFmt(&(self->cache[idx].path), rslvd);
            }
            else {
                if (vdbcache)
                    rc = VPathMakeFmt(&(self->cacheVc[idx].path), path);
                else
                    rc = VPathMakeFmt(&(self->cache[idx].path), path);
            }
            resolved = true;
        }
    }

    if (resolved && !vdbcache) {
        VQuality q = eQualLast;
        assert(self->remote[idx].path);
        q = VPathGetQuality(*self->remote[idx].path);
        rc = VPathSetQuality(self->cache[idx].path, q);
    }

    StringWhack(root);
    StringWhack(volume);

    return rc;
}

/* return local and cache locations for remote (to be used by file iterator) */
static rc_t KRunResolve(const KRun * self, const VPath * remote,
    const VPath ** local, const VPath ** cache)
{
    rc_t rc = 0, r2 = 0;
    const VPath * path = NULL;
    bool vc = false;
    bool notFound = false;
    int idx = -1;
    assert(local && cache);

    rc = VPath_IdxForRemote(remote, &idx, &vc, &notFound);
    if (rc != 0)
        return rc;
    if (notFound)
        return 0;

    if (vc) {
        if (self->localVc[idx].resolved != NULL)
            path = self->localVc[idx].resolved;
        else if (self->localVc[idx].out != NULL)
            path = self->localVc[idx].out;
        else if (self->localVc[idx].ad != NULL)
            path = self->localVc[idx].ad;
        else
            path = self->localVc[idx].repo;

        rc = VPathAddRef(path);
        if (rc == 0)
            *local = path;
        r2 = VPathAddRef(self->cacheVc[idx].path);
        if (r2 == 0)
            *cache = self->cacheVc[idx].path;
        if (r2 != 0 && rc == 0)
            rc = r2;
    }
    else {
        if (self->local[idx].resolved != NULL)
            path = self->local[idx].resolved;
        else if (self->local[idx].out != NULL)
            path = self->local[idx].out;
        else if (self->local[idx].ad != NULL)
            path = self->local[idx].ad;
        else
            path = self->local[idx].repo;

        rc = VPathAddRef(path);
        if (rc == 0)
            *local = path;
        r2 = VPathAddRef(self->cache[idx].path);
        if (r2 == 0)
            *cache = self->cache[idx].path;
        if (r2 != 0 && rc == 0)
            rc = r2;
    }

    return rc;
}

/* find cache for remote */
static rc_t KRunsCacheForRemote(KRun * self,
    const char * outDir, const char * outFile)
{
    rc_t rc = 0;

    int32_t i = 0;

    assert(self && self->dad);

    if (self->dad->kfg != NULL) {
        rc_t rc = 0;
        VResolver * r = NULL;
        KConfig * k = self->dad->kfg;
        bool val = false;

        VResolverEnableState state = vrUseConfig;
        if (self->dad->resolver != NULL)
            r = self->dad->resolver;
        else {
            if (self->dad->vfs == NULL) {
                VFSManager * p = NULL;
                rc = VFSManagerMakeFromKns(&p, k, (KNSManager*)self->dad->kns);
                self->dad->vfs = p;
            }
            if (rc == 0)
                rc = VFSManagerMakeResolver(self->dad->vfs, &r, k);
        }
        if (r != NULL) {
            state = VResolverCacheEnable(r, vrUseConfig);
            VResolverCacheEnable(r, state);
            if (r != self->dad->resolver)
                VResolverRelease(r);
        }

        if (state == vrAlwaysDisable) /* resolver was set to always-disable */
            return 0;
        else if (state != vrAlwaysEnable) {
            rc = KConfigReadBool(k, "/repository/user/cache-disabled", &val);
            if (rc == 0 && val) /* disabled by configuration */
                return 0;

            rc = KConfigReadBool(k,
                "/repository/user/main/public/cache-enabled", &val);
            if (rc == 0 && !val) /* disabled by configuration */
                return 0;

            rc = KConfigReadBool(k,
                "/repository/user/main/public/disabled", &val);
            if (rc == 0 && val) /* disabled by configuration */
                return 0;

            rc = KConfigReadBool(k,
                "/repository/user/main/public/apps/sra/cache-enabled", &val);
            if (rc == 0 && !val) /* disabled by configuration */
                return 0;
        }

        rc = KConfigReadBool(k,
            "/repository/user/ad/public/apps/sra/disabled", &val);
        if (rc == 0 && val) /* disabled by configuration */
            return 0;
    }

    for (i = 0; i < eIdxMx && rc == 0; ++i) {
        if (self->remoteVc[i].path != NULL) {
            rc = KRunCacheForRemote(self, i, true, outDir, outFile);
            if (rc != 0)
                break;
        }

        if (self->remote[i].path != NULL) {
            rc = KRunCacheForRemote(self, i, false, outDir, outFile);
            if (rc != 0)
                break;
        }

        rc = VPathAttachVdbcache(self->cache[i].path, self->cacheVc[i].path);
    }

    return rc;
}

/* TODO */
rc_t KSrvRunRelease(const KRun * self) {
    return 0;
}

/* get remote location for a run
considering requested quality and found files */
static rc_t KRunRemote(KRun * self) {
    rc_t rc = 0;
    EQualIdx idx = eIdxMx;
    EQualIdx idxLocal = eIdxMx;
    const VPath * path = NULL;
    assert(self && self->dad);

    idxLocal = self->result.localIdx;
    self->result.remoteIdx = eIdxMx;

    if (self->dad->quality == eQualDefault) {
        if (self->remote[eIdxNo].cnt > 0)
            idx = eIdxNo;
        else if (self->remote[eIdxDbl].cnt > 0)
            idx = eIdxDbl;
        else if (self->remote[eIdxAsk].cnt > 0)
            idx = eIdxAsk;
        else if (self->remote[eIdxYes].cnt > 0)
            idx = eIdxYes;

        if (idx != eIdxMx) {
            if (idxLocal != eIdxMx     /* local exists */
                && idx != idxLocal     /* remote type is different from local */
                && self->remoteVc[idx].cnt > 0 /* remote has vdbcache */
                && self->localVc[idxLocal].path
                    == NULL)                   /* local doesn't have vdbcache */
            {   /* there's no remote of the same type as local */
                if (self->remote[idxLocal].cnt == 0 ||
                    self->remoteVc[idxLocal].cnt == 0)
                {   /* vdbcache exists, local does not have vdbcache and we */
                    idx = eIdxMx; /* cannot find matching vdbcache remotely */
                    rc =          /* => we cannot use this local location */
                        RC(rcVFS, rcQuery, rcExecuting, rcPath, rcNotAvailable);
                }
                else
                    idx = idxLocal;
            }

            if (rc == 0) {
                assert(self->remote[idx].cnt > 0);
                path = self->remote[idx].path[0];
            }
        }
    }
#ifdef WILL_PROCESS_QUALIY_HERE
    else if (self->dad->quality == eQualNo) {
        if (self->remote[eIdxNo].cnt > 0)
            idx = eIdxNo;
        else if (self->remote[eIdxDbl].cnt > 0)
            idx = eIdxDbl;
        else if (self->remote[eIdxAsk].cnt > 0)
            idx = eIdxAsk;

        if (idx != eIdxMx) {
            path = self->remote[idx].path[0];
            assert(path);
        }
    }

    else if (self->dad->quality == eQualFull) {
        if (self->remote[eIdxYes].cnt > 0)
            idx = eIdxYes;
        else if (self->remote[eIdxDbl].cnt > 0)
            idx = eIdxDbl;
        else if (self->remote[eIdxAsk].cnt > 0)
            idx = eIdxAsk;

        if (idx != eIdxMx) {
            path = self->remote[idx].path[0];
            assert(path);
        }
    }

    else if (self->dad->quality == eQualFullOnly) {
        if (self->remote[eIdxYes].cnt > 0)
            idx = eIdxYes;
        else if (self->remote[eIdxAsk].cnt > 0)
            idx = eIdxAsk;

        if (idx != eIdxMx) {
            path = self->remote[idx].path[0];
            assert(path);
        }
    }

    else if (self->dad->quality == eQualDblOnly) {
        if (self->remote[eIdxDbl].cnt > 0)
            idx = eIdxDbl;
        else if (self->remote[eIdxAsk].cnt > 0)
            idx = eIdxAsk;

        if (idx != eIdxMx) {
            path = self->remote[idx].path[0];
            assert(path);
        }
    }
#endif
    if (rc == 0) {
        self->result.remote = path;
        self->result.remoteIdx = idx;
    }
    else
        self->result.remoteIdx = eIdxMx;

    return rc;
}

/* get cache location for a run */
static rc_t KRunCache(KRun * self) {
    rc_t rc = 0;

    const VPath * path = NULL;

    assert(self);

    if (self->result.remoteIdx != eIdxMx)
        path = self->cache[self->result.remoteIdx].path;

    self->result.cache = path;

    return rc;
}

/* get local location for a run considering requested quality */
static rc_t KRunLocal(KRun * self) {
    rc_t rc = 0;
    EQualIdx idx = eIdxMx;
    const VPath * path = NULL;
    assert(self && self->dad);

    self->result.localIdx = eIdxMx;

    if (self->local[eIdxAsk].magic != NULL) {
        idx = eIdxAsk;
        path = self->local[eIdxAsk].magic;
    }

    else if (self->dad->quality == eQualDefault) {
        if (self->local[eIdxNo].path != NULL
            && self->localVc[eIdxNo].path != NULL)
        {
            idx = eIdxNo;
        }
        else if (self->local[eIdxDbl].path != NULL
            && self->localVc[eIdxDbl].path != NULL)
        {
            idx = eIdxDbl;
        }
        else if (self->local[eIdxAsk].path != NULL
            && self->localVc[eIdxAsk].path != NULL)
        {
            idx = eIdxAsk;
        }
        else if (self->local[eIdxYes].path != NULL
            && self->localVc[eIdxYes].path != NULL)
        {
            idx = eIdxYes;
        }

        else if (self->local[eIdxNo].path != NULL)
            idx = eIdxNo;
        else if (self->local[eIdxDbl].path != NULL)
            idx = eIdxDbl;
        else if (self->local[eIdxAsk].path != NULL)
            idx = eIdxAsk;
        else if (self->local[eIdxYes].path != NULL)
            idx = eIdxYes;

        if (idx != eIdxMx)
            path = self->local[idx].path;
    }
#ifdef WILL_PROCESS_QUALIY_HERE
    else if (self->dad->quality == eQualNo) {
        if (self->local[eIdxNo].path != NULL
            && self->localVc[eIdxNo].path != NULL)
        {
            idx = eIdxNo;
        }
        else if (self->local[eIdxDbl].path != NULL
            && self->localVc[eIdxDbl].path != NULL)
        {
            idx = eIdxDbl;
        }
        else if (self->local[eIdxAsk].path != NULL
            && self->localVc[eIdxAsk].path != NULL)
        {
            idx = eIdxAsk;
        }

        else if (self->local[eIdxNo].path != NULL)
            idx = eIdxNo;
        else if (self->local[eIdxDbl].path != NULL)
            idx = eIdxDbl;
        else if (self->local[eIdxAsk].path != NULL)
            idx = eIdxAsk;

        if (idx != eIdxMx)
            path = self->local[idx].path;
    }

    else if (self->dad->quality == eQualFull) {
        if (self->local[eIdxYes].path != NULL
            && self->localVc[eIdxYes].path != NULL)
        {
            idx = eIdxYes;
        }
        else if (self->local[eIdxDbl].path != NULL
            && self->localVc[eIdxDbl].path != NULL)
        {
            idx = eIdxDbl;
        }
        else if (self->local[eIdxAsk].path != NULL
            && self->localVc[eIdxAsk].path != NULL)
        {
            idx = eIdxAsk;
        }

        else if (self->local[eIdxYes].path != NULL)
            idx = eIdxYes;
        else if (self->local[eIdxDbl].path != NULL)
            idx = eIdxDbl;
        else if (self->local[eIdxAsk].path != NULL)
            idx = eIdxAsk;

        if (idx != eIdxMx)
            path = self->local[idx].path;
    }

    else if (self->dad->quality == eQualFullOnly) {
        if (self->local[eIdxYes].path != NULL
            && self->localVc[eIdxYes].path != NULL)
        {
            idx = eIdxYes;
        }
        else if (self->local[eIdxAsk].path != NULL
            && self->localVc[eIdxAsk].path != NULL)
        {
            idx = eIdxAsk;
        }

        else if (self->local[eIdxYes].path != NULL)
            idx = eIdxYes;
        else if (self->local[eIdxAsk].path != NULL)
            idx = eIdxAsk;

        if (idx != eIdxMx)
            path = self->local[idx].path;
    }

    else if (self->dad->quality == eQualDblOnly) {
        if (self->local[eIdxDbl].path != NULL
            && self->localVc[eIdxDbl].path != NULL)
        {
            idx = eIdxDbl;
        }
        else if (self->local[eIdxAsk].path != NULL
            && self->localVc[eIdxAsk].path != NULL)
        {
            idx = eIdxAsk;
        }

        else if (self->local[eIdxDbl].path != NULL)
            idx = eIdxDbl;
        else if (self->local[eIdxAsk].path != NULL) 
            idx = eIdxAsk;

        if (idx != eIdxMx)
            path = self->local[idx].path;
    }
#endif
    if (rc == 0) {
        self->result.localIdx = idx;
        self->result.local = path;
    }
    else
        self->result.localIdx = eIdxMx;

    return rc;
}

/* prepare results for KSrvRunQuery() */
static rc_t KSrvRunPrepareQuery(KRun * self)
{
    rc_t rc = 0;

    const VPath * path = NULL;

    if (rc == 0)
        rc = KRunLocal(self);

    if (rc == 0)
        rc = KRunRemote(self);

    if (rc == 0)
        rc = KRunCache(self);

    self->result.vdbcache = false;

    if (rc == 0) {
        if (self->result.remote != NULL) {
            if (self->result.remote->vdbcache != NULL)
                self->result.vdbcache = true;
        }

        if (self->result.local != NULL)
            if (self->result.local->vdbcache != NULL)
                self->result.vdbcache = true;
    }

    path = self->result.local;
    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
        ("KSrvRunQuery: local location of '%S' resolved to '%s'. "
            "It %s vdbcache\n",
            self->acc, path == NULL ? "<NULL>" : path->path.addr,
            self->result.vdbcache ? "has" : "doesn't have"));
    if (path != NULL && self->result.vdbcache)
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("KSrvRunQuery: "
            "local location of '%S.vdbcache' resolved to '%s'\n", self->acc,
            path->vdbcache == NULL ? "<NULL>" : path->vdbcache->path.addr));

    path = self->result.remote;
    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
        ("KSrvRunQuery: remote location of '%S' resolved to '%s'. "
            "It %s vdbcache\n",
            self->acc, path == NULL ? "<NULL>" : path->scheme.addr,
            self->result.vdbcache ? "has" : "doesn't have"));
    if (path != NULL) {
        if (self->result.vdbcache)
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("KSrvRunQuery: "
                "remote location of '%S.vdbcache' resolved to '%s'\n",
                self->acc,
                path->vdbcache == NULL
                ? "<NULL>" : path->vdbcache->scheme.addr));

        path = self->result.cache;
        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
            ("KSrvRunQuery: cache location of '%S' resolved to '%s'\n",
                self->acc, path == NULL ? "<NULL>" : path->path.addr));
        if (path != NULL && self->result.vdbcache)
            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("KSrvRunQuery: "
                "cache location of '%S.vdbcache' resolved to '%s'\n",
                self->acc,
                path->vdbcache == NULL
                    ? "<NULL>" : path->vdbcache->path.addr));
    }
    return rc;
}

/* get local/remote/cache location for a run considering requested quality */
rc_t KSrvRunQuery(const KRun * self, const VPath ** local,
    const VPath ** remote, const VPath ** cache, bool * vdbcache)
{
    rc_t rc = 0, r2 = 0;
    const VPath * path = NULL;

    if (self == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcSelf, rcNull);

    assert(self->dad);
    if (self->dad->quality == NULL && self->it != NULL) {
        KSrvRunIterator * ri = self->it;
        const KSrvResponse * response = KSrvRunIteratorGetResponse(ri);
        uint32_t i = 0;
        uint32_t l = KSrvResponseLength(response);
        for (i = 0; i < l && rc == 0; ++i) {
            const KSrvRespObj * obj = NULL;
            KSrvRespObjIterator * it = NULL;
            KSrvRespFile * vcFile = NULL;
            ESrvFileFormat type = eSFFInvalid;
            const VPath * path = NULL;
            rc = KSrvResponseGetObjByIdx(response, i, &obj);
            if (rc == 0)
                rc = KSrvRespObjMakeIterator(obj, &it);
            while (rc == 0) {
                KSrvRespFile * file = NULL;
                rc_t r2 = 0;
                rc = KSrvRespObjIteratorNextFile(it, &file);
                if (rc != 0 || file == NULL)
                    break;
                r2 = KSrvRespFileGetFormat(file, &type);
                if (r2 != 0 || type != eSFFVdbcache) {
                    if (local != NULL) {
                        *local = NULL;
                        rc_t rc = KSrvRespFileGetLocal(file, &path);
                        if (rc == 0)
                            *local = path;
                    }
                    if (remote != NULL) {
                        *remote = NULL;
                        KSrvRespFileIterator * fi = NULL;
                        rc = KSrvRespFileMakeIterator(file, &fi);
                        if (rc == 0)
                            rc = KSrvRespFileIteratorNextPath(fi, &path);
                        if (rc == 0)
                            *remote = path;
                        RELEASE(KSrvRespFileIterator, fi);
                    }
                    if (cache != NULL) {
                        *cache = NULL;
                        rc_t rc = KSrvRespFileGetCache(file, &path);
                        if (rc == 0)
                            *cache = path;
                    }
                }
                if (type == eSFFVdbcache)
                    vcFile = file;
                else
                    RELEASE(KSrvRespFile, file);
            }
            if (vcFile == NULL) {
                if (vdbcache != NULL)
                    *vdbcache = false;
                if (local != NULL && *local != NULL)
                    rc = VPathAttachVdbcache((VPath*)*local, NULL);
                if (remote != NULL && *remote != NULL)
                    rc = VPathAttachVdbcache((VPath*)*remote, NULL);
                if (cache != NULL && *cache != NULL)
                    rc = VPathAttachVdbcache((VPath*)*cache, NULL);
            }
            else {
                if (vdbcache != NULL)
                    *vdbcache = true;
                if (local != NULL && *local != NULL) {
                    rc_t rc = KSrvRespFileGetLocal(vcFile, &path);
                    if (rc == 0)
                        rc = VPathAttachVdbcache((VPath*)*local, path);
                }
                if (remote != NULL && *remote != NULL) {
                    KSrvRespFileIterator * fi = NULL;
                    rc = KSrvRespFileMakeIterator(vcFile, &fi);
                    if (rc == 0)
                        rc = KSrvRespFileIteratorNextPath(fi, &path);
                    if (rc == 0)
                        rc = VPathAttachVdbcache((VPath*)*remote, path);
                    RELEASE(VPath, path);
                    RELEASE(KSrvRespFileIterator, fi);
                }
                if (cache != NULL && *cache != NULL) {
                    rc_t rc = KSrvRespFileGetCache(vcFile, &path);
                    if (rc == 0)
                        rc = VPathAttachVdbcache((VPath*)*cache, path);
                }
                RELEASE(KSrvRespFile, vcFile);
            }
            RELEASE(KSrvRespObjIterator, it);
            RELEASE(KSrvRespObj, obj);
        }
        return rc;
    }

    if (local != NULL) {
        path = self->result.local;
        rc = VPathAddRef(path);
        if (rc == 0)
            *local = path;
    }

    if (remote != NULL) {
        path = self->result.remote;
        r2 = VPathAddRef(path);
        if (r2 == 0)
            *remote = path;
        else if (rc == 0)
            rc = r2;
    }

    if (cache != NULL) {
        path = self->result.cache;
        r2 = VPathAddRef(path);
        if (r2 == 0)
            *cache = path;
        else if (rc == 0)
            rc = r2;
    }

    if (vdbcache != NULL)
        *vdbcache = self->result.vdbcache;

    return rc;
}

/******************************************************************************/

static void BSTItemWhack(BSTNode * n, void * ignore) {
    BSTItem * i = (BSTItem *)n;
    assert(i);

    StringWhack(i->acc);
    KRunWhack(i->run);

    memset(i, 0, sizeof * i);

    free(i);
}

/* compare runs by acc */
static int64_t CC BSTItemCmp(const void * item, const BSTNode * n) {
    const String * s = item;
    const BSTItem * i = (BSTItem *)n;
    return StringCompare(s, i->acc);
}

/* sort runs by acc */
static int64_t CC BSTreeSort(const BSTNode * item, const BSTNode * n) {
    const BSTItem * i = (BSTItem *)item;
    assert(i);
    return BSTItemCmp(i->acc, n);
}

/* find cache for each remote */
static void CC BSTNodeCacheForRemote(BSTNode *n, void *data) {
    rc_t rc = 0;

    BSTData * p = data;
    const BSTItem *sn = (const BSTItem*)n;
    assert(sn && p);

    rc = KRunsCacheForRemote(sn->run, p->outDir, p->outFile);
    if (rc != 0 && p->rc == 0)
        p->rc = rc;
}

/* find the best local if there are multiple */
static void CC BSTNodeLocalResolve(BSTNode *n, void *data) {
    rc_t rc = 0;

    BSTData * p = data;
    const BSTItem *sn = (const BSTItem*)n;
    assert(sn && p);

    rc = KRunLocalResolve(sn->run);
    if (rc != 0 && p->rc == 0)
        p->rc = rc;
}

/* prepare results for KSrvRunQuery() */
static void CC BSTNodePrepareQuery(BSTNode *n, void *data) {
    rc_t rc = 0;

    BSTData * p = data;
    const BSTItem *sn = (const BSTItem*)n;
    assert(sn && p);

    rc = KSrvRunPrepareQuery(sn->run);
    if (rc != 0 && p->rc == 0)
        p->rc = rc;
}

#ifdef HAS_SERVICE_CACHE
/* resolve local[-s] */
static void CC BSTNodeFindLocal(BSTNode *n, void *data) {
    rc_t rc = 0;

    BSTData * p = data;
    const BSTItem *sn = (const BSTItem*)n;
    assert(sn && p);

    rc = KRunResolveLocals(sn->run, p->projectId, p->outDir, p->outFile);
    if (rc != 0 && p->rc == 0)
        p->rc = rc;
}
#endif

/* attach vdbcaches to local[-s] and remote[-s] */
static void CC BSTNodeAttachVdbcaches(BSTNode *n, void *data) {
    rc_t rc = 0;

    BSTData * p = data;
    const BSTItem *sn = (const BSTItem*)n;
    assert(sn && p);

    rc = KRunAttachVdbcaches(sn->run);
    if (rc != 0 && p->rc == 0)
        p->rc = rc;
}

/* find local for each remote */
static void CC BSTNodeLinkLocalsToRemotes(BSTNode *n, void *data) {
    rc_t rc = 0;

    BSTData * p = data;
    const BSTItem *sn = (const BSTItem*)n;
    assert(sn && p);

    rc = KRunLinkLocalsToRemotes(sn->run);
    if (rc != 0 && p->rc == 0)
        p->rc = rc;
}

/******************************************************************************/

/* Initialize */
static rc_t ServicesCacheInit(ServicesCache * self, const VFSManager * vfs,
    const KNSManager * kns, const KConfig * kfg,
    int64_t projectId, const char * quality)
{
    rc_t rc = 0;

#ifdef WILL_PROCESS_QUALIY_HERE
    assert(self && quality);

    self->projectId = projectId;
    self->quality = quality;

    if (rc == 0) {
        if (kfg != NULL)
            rc = KConfigAddRef(kfg);
/*KConfigPrint(kfg, 0);*/
        if (rc == 0)
            self->kfg = (KConfig*)kfg;
    }

    if (rc == 0) {
        if (kns != NULL)
            rc = KNSManagerAddRef(kns);
        if (rc == 0)
            self->kns = kns;
    }

    if (rc == 0) {
        if (vfs != NULL)
            rc = VFSManagerAddRef(vfs);
        if (rc == 0)
            self->vfs = vfs;
    }
#endif

    return rc;
}

/* Make */
rc_t ServicesCacheMake(ServicesCache ** self, const VFSManager * vfs,
    const KNSManager * kns, const KConfig * kfg,
    int64_t projectId, const char * quality)
{
    rc_t rc = 0;

    ServicesCache * p = calloc(1, sizeof *p);

    assert(self);
    *self = NULL;

    if (p == NULL)
        return RC(rcVFS, rcStorage, rcAllocating, rcMemory, rcExhausted);

    rc = ServicesCacheInit(p, vfs, kns, kfg, projectId, quality);

    if (rc == 0)
        *self = p;

    return rc;
}

/* to be continued... */
static ServicesCache sCache;
ServicesCache * getCache(int64_t projectId) {
    static ServicesCache * cache = NULL;
    if (cache == NULL) {
        if (ServicesCacheInit(&sCache, NULL, NULL, NULL, projectId, 0) == 0)
            cache = &sCache;
    }
    return cache;
}

/* Release ServicesCache content */
static rc_t ServicesCacheFini(ServicesCache * self) {
    rc_t rc = 0;

    assert(self);

    BSTreeWhack(&self->runs, BSTItemWhack, NULL);

    rc = KRunWhack(self->run);
    self->run = NULL;

    rc = KRunWhack(self->disabledRun);
    self->disabledRun = NULL;

    RELEASE(KDirectory, self->dir);
    RELEASE(KConfig, self->kfg);
    RELEASE(KNSManager, self->kns);
    RELEASE(VResolver, self->resolver);
    RELEASE(VFSManager, self->vfs);
    RELEASE(VDBManager, self->vdb);

    return rc;
}

/* Whack */
rc_t ServicesCacheWhack(ServicesCache * self) {
    if (self != NULL) {
        rc_t rc = ServicesCacheFini(self);
        memset(self, 0, sizeof * self);
        free(self);
        return rc;
    }

    return 0;
}

/* Find a KRun for SRR accession. Created it if necessary. */
static rc_t ServicesCacheAddRun(ServicesCache * self,
    const String * acc, KRun ** aRun, bool * notFound)
{
    rc_t rc = 0;

    bool dummy = false;
    KRun * rummy = NULL;
    KRun * run = NULL;
    BSTItem * i = NULL;

    if (aRun == NULL)
        aRun = &rummy;
    if (notFound == NULL)
        notFound = &dummy;
    *aRun = NULL;
    *notFound = true;

    if (self->run == NULL) {
        rc = KRunMake(&self->run, acc, self);
        run = self->run;
    }

    else if (StringEqual(acc, self->run->acc))
        run = self->run;

    else {
        i = (BSTItem *)BSTreeFind(&self->runs, acc, BSTItemCmp);
        if (i != NULL)
            run = i->run;

        else {
            i = calloc(1, sizeof * i);
            if (i == NULL)
                return RC(rcVFS, rcStorage, rcAllocating,
                    rcMemory, rcExhausted);

            rc = StringCopy(&i->acc, acc);
            if (rc != 0)
                return rc;

            rc = KRunMake(&i->run, acc, self);
            if (rc != 0)
                return rc;

            rc = BSTreeInsert(&self->runs, (BSTNode *)i, BSTreeSort);
            run = i->run;
        }
    }

    if (rc == 0) {
        *aRun = run;
        *notFound = false;
    }

    return rc;
}

/* Find a KRun for remote VPath. */
static rc_t ServicesCacheFindRun(ServicesCache * self,
    const VPath * path, KRun ** run, bool * notFound)
{
    rc_t rc = 0;

    String acc;

    assert(run && notFound);
    *run = NULL;
    *notFound = true;

    rc = VPathGetAccession(path, &acc);
    if (rc != 0|| acc.addr == NULL)
        return rc;

    return ServicesCacheAddRun(self, &acc, run, notFound);
}

#ifdef HAS_SERVICE_CACHE
/* Add a remote location to ServicesCache.
We'll resolve its cache/local locations in ServicesCacheComplete */
rc_t ServicesCacheAddRemote(ServicesCache * self, const VPath * path) {
    KRun * run = NULL;
    bool notFound = false;
    rc_t rc = 0;

    assert(self);
    if (self->quality == NULL)
        return 0;

    rc = ServicesCacheFindRun(self, path, &run, &notFound);
    if (rc == 0 && !notFound)
        rc = KRunAddRemote(run, path);

    return rc;
}
#endif

/* add an accession to find its local location */
rc_t ServicesCacheAddId(ServicesCache * self, const char * acc) {
    rc_t rc = 0;

    String s;
    String srr;

    assert(self);
    if (self->quality == NULL)
        return 0;

    CONST_STRING(&srr, "RR");
    StringInitCString(&s, acc);

    /* we are adding just *RR accessions */

    if (s.size < 3)
        return 0;

    assert(srr.size == 2);

    if (string_cmp(srr.addr, srr.size, s.addr + 1, srr.size, srr.len) != 0)
        return 0;

    if (rc == 0)
        rc = ServicesCacheAddRun(self, &s, NULL, NULL);

    return rc;
}

/* resolve magic env.vars. */
static rc_t ServicesCacheResolveMagic(ServicesCache * self) {
    assert(self);
    if (self->run == NULL)
        return 0;
    /* TODO multiple runs? error? */
    return KRunResolveMagic(self->run);
}

#ifdef HAS_SERVICE_CACHE
/* resolve local[-s] */
static rc_t ServicesCacheFindLocal(ServicesCache * self,
    const char * outDir, const char * outFile)
{
    String * site = NULL;
    rc_t rc = 0, r2 = 0;

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: entered", __func__));
#endif

    assert(self);

    if (self->quality == NULL)
        return 0;

    if (self->dir == NULL)
        rc = KDirectoryNativeDir(&self->dir);
    if (self->kfg == NULL)
        rc = KConfigMake(&self->kfg, self->dir);

    r2 = KConfigReadString(self->kfg, "/repository/site", &site);
    if (r2 == 0) {
        StringWhack(site);
        site = NULL;
        if (self->vfs == NULL) {
            VFSManager * p = NULL;
            rc = VFSManagerMakeFromKns(&p, self->kfg, (KNSManager*)self->kns);
            self->vfs = p;
        }
        if (rc == 0)
            rc = VFSManagerMakeResolver(self->vfs, &self->resolver, self->kfg);
    }

    if (rc == 0) {
        BSTData data;
        data.rc = 0;
        data.outDir = outDir;
        data.outFile = outFile;
        data.projectId = self->projectId;

        if (self->run != NULL) {
#ifdef DBGNG
            STSMSG(STS_FIN,
                ("%s: before calling KRunResolveLocals...", __func__));
#endif
            rc = KRunResolveLocals(self->run, self->projectId, outDir, outFile);
        }

#ifdef DBGNG
        STSMSG(STS_FIN, ("%s: before calling Each BSTNodeFindLocal...",
            __func__));
#endif
        BSTreeForEach(&self->runs, false, BSTNodeFindLocal, &data);
        if (data.rc != 0 && rc == 0)
            rc = data.rc;
    }

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: exiting with %R", __func__, rc));
#endif
    return rc;
}
#endif

/* attach vdbcaches to local[-s] and remote[-s] */
static rc_t ServicesCacheAttachVdbcaches(ServicesCache * self) {
    rc_t rc = 0;

    BSTData data;
    data.rc = 0;

    if (self->run != NULL)
        rc = KRunAttachVdbcaches(self->run);

    BSTreeForEach(&self->runs, false, BSTNodeAttachVdbcaches, &data);
    if (data.rc != 0 && rc == 0)
        rc = data.rc;

    return rc;
}

/* find local for each remote */
static rc_t ServicesCacheLinkLocalToRemote(ServicesCache * self) {
    rc_t rc = 0;

    BSTData data;
    data.rc = 0;

    if (self->run != NULL)
        rc = KRunLinkLocalsToRemotes(self->run);

    BSTreeForEach(&self->runs, false, BSTNodeLinkLocalsToRemotes, &data);
    if (data.rc != 0 && rc == 0)
        rc = data.rc;

    return rc;
}

#ifdef HAS_SERVICE_CACHE
/* The cache has all remote[-s]; now resolve all cache[-s] and local[-s]
   Prepare results for KSrvRunQuery() */
rc_t ServicesCacheComplete(ServicesCache * self,
    const char * outDir, const char * outFile, bool skipLocal)
{
    rc_t rc = 0;

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: entered", __func__));
#endif

    assert(self);

    if (self->quality == NULL)
        return 0;

    if (rc == 0) /* resolve magic env.vars. */
        rc = ServicesCacheResolveMagic(self);

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: before calling ServicesCacheFindLocal...", __func__));
#endif
    if (rc == 0 && !skipLocal) /* resolve local[-s] */
        rc = ServicesCacheFindLocal(self, outDir, outFile);

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: before calling ServicesCacheAttachVdbcaches...",
        __func__));
#endif
    if (rc == 0) /* attach vdbcaches to local[-s] and remote[-s] */
        rc = ServicesCacheAttachVdbcaches(self);

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: before calling ServicesCacheLinkLocalToRemote...",
        __func__));
#endif
    if (rc == 0) /* find local for each remote */
        rc = ServicesCacheLinkLocalToRemote(self);

    /* find cache for each remote */

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: before calling KRunsCacheForRemote...", __func__));
#endif
    if (rc == 0) {
        KRun * run = self->run;
        if (run == NULL)
            return 0;
        rc = KRunsCacheForRemote(run, outDir, outFile);
    }

    if (rc == 0) {
        BSTData data;
        data.rc = 0;
        data.outDir = outDir;
        data.outFile = outFile;
        BSTreeForEach(&self->runs, false, BSTNodeCacheForRemote, &data);
        if (data.rc != 0 && rc == 0)
            rc = data.rc;
    }

    /* find the best local if there are multiple */
#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: before calling KRunLocalResolve...", __func__));
#endif
    KRunLocalResolve(self->run);
    if (rc == 0) {
        BSTData data;
        data.rc = 0;
        BSTreeForEach(&self->runs, false, BSTNodeLocalResolve, &data);
        if (data.rc != 0 && rc == 0)
            rc = data.rc;
    }

#ifdef DBGNG
    /* prepare results for KSrvRunQuery() */
    STSMSG(STS_FIN, ("%s: before calling KSrvRunPrepareQuery...", __func__));
#endif
    if (rc == 0) {
        rc = KSrvRunPrepareQuery(self->run);
        if (rc == 0) {
            BSTData data;
            data.rc = 0;
            BSTreeForEach(&self->runs, false, BSTNodePrepareQuery, &data);
            if (data.rc != 0 && rc == 0)
                rc = data.rc;
        }
    }

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: exiting with %R", __func__, rc));
#endif
    return rc;
}
#endif

/* return local and cache locations for remote (to be used by file iterator) */
rc_t ServicesCacheResolve(ServicesCache * self, const VPath * remote,
    const VPath ** local, const VPath ** cache)
{
    KRun * run = NULL;
    bool notFound = false;

    rc_t rc = 0;
    
    assert(self);
    if (self->quality == NULL)
        return 0;

    rc = ServicesCacheFindRun(self, remote, &run, &notFound);
    if (rc == 0 && !notFound)
        rc = KRunResolve(run, remote, local, cache);

    return rc;
}

/* get the first run or the first run from tree from ServicesCache */
rc_t ServicesCacheGetRun(const ServicesCache * cself, bool tree,
    const struct KSrvRun ** run, KSrvRunIterator * it)
{
    ServicesCache * self = (ServicesCache*) cself;

    assert(self && run);

    *run = NULL;

    if (self->quality == NULL) {
        KSrvRun * r = NULL;
        rc_t rc = 0;
        if (self->disabledRun == NULL)
            KRunMakeWhenDisabled(&r, self, it);
        else {
            rc = KRunWhack(self->disabledRun);
            self->disabledRun = NULL;
        }
        if (rc == 0) {
            self->disabledRun = r;
            *run = r;
        }
        return rc;
    }

    *run = tree ? (KRun*)BSTreeFirst(&self->runs) : self->run;

    return 0;
}

/******************************************************************************/
#ifdef HAS_SERVICE_CACHE
rc_t ServicesCacheGetResponse(const ServicesCache * self,
    const char * acc, const struct KSrvResponse ** response)
{
    assert(self);
    if (self->quality == NULL)
        return 0;

    return 1;
}
static rc_t ServicesCacheAddResponse4(
    ServicesCache * self, const struct Response4 * r)
{
    return 0;
}
#endif
void f() { /* these functions might be unneeded */
    CacheInit(0, 0); CacheSet(0, 0);
#ifdef HAS_SERVICE_CACHE
    ServicesCacheAddResponse4(0, 0);
    ServicesCacheGetResponse(0, 0, 0);
    /* TODO: detect unpacked run dir in cwd
    ServicesCacheComplete */
#endif
}
