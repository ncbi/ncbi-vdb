/*==============================================================================
*
*                            Public Domain Notice
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
* ===========================================================================
*
*/

#include <vfs/extern.h>

#include <kfg/config.h> /* KConfigRelease */

#include <kfs/directory.h> /* KDirectoryNativeDir */
#include <kfs/file.h> /* KFileRelease */
#include <kfg/kfg-priv.h> /* KConfigMakeEmpty */
#include <kfg/ngc.h> /* KNgcObjGetProjectId */
#include <kfg/repository.h> /* KRepositoryMgrGetProtectedRepository */

#include <klib/container.h> /* BSTree */
#include <klib/debug.h> /* DBGMSG */
#include <klib/printf.h> /* string_printf */
#include <klib/rc.h> /* RC */
#include <klib/strings.h> /* ENV_MAGIC_LOCAL */

#include <kns/manager.h> /* KNSManagerRelease */

#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path.h> /* VFSManagerMakePath */
#include <vfs/resolver-priv.h> /* VResolverQueryWithDir */
#include <vfs/services-priv.h> /* KServiceNamesExecuteExt */

#include "manager-priv.h" /* VFSManager */
#include "path-priv.h" /* EVPathInitError */
#include "resolver-priv.h" /* VResolverResolveName */
#include "services-cache.h" /* ServicesCacheWhack */
#include "services-priv.h" /* KServiceGetResolver */

#include <ctype.h> /* isdigit */

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)


typedef struct {
    BSTNode n;
    const String * ticket;
    VResolver * resolver;
} BSTItem;

static void BSTItemWhack ( BSTNode * n, void * ignore ) {
    BSTItem * i = ( BSTItem * ) n;

    assert ( i );

    free ( ( void * ) i -> ticket );
    VResolverRelease ( i -> resolver );

    memset ( i, 0, sizeof * i );

    free ( i );
}

static int64_t CC BSTItemCmp ( const void * item, const BSTNode * n ) {
    const String * s = item;
    const BSTItem * i = ( BSTItem * ) n;
 
    assert ( s && i );
 
    return string_cmp ( s -> addr, s -> size,
        i -> ticket -> addr, i -> ticket -> size, s -> len );
}

static
int64_t CC BSTreeSort ( const BSTNode * item, const BSTNode * n )
{
    const BSTItem * i = ( BSTItem * ) item;

    assert ( i );

    return BSTItemCmp ( i -> ticket, n );
}


typedef struct {
    KService * service; /* DO NOT RELEASE */
    VFSManager * mgr;
    ServicesCache * cache;
    const KConfig * kfg;
    VResolver * resolver;
    BSTree ticketsToResolvers;
} H;

static rc_t HInit ( H * self, KService * s, ServicesCache * cache ) {
    rc_t rc = 0;

    assert ( self && s );

    memset ( self, 0, sizeof * self );

    self -> cache = cache;

    self -> service = s;

    if ( rc == 0 )
        rc = KServiceGetVFSManager ( s, & self -> mgr );

    if ( rc == 0 )
        rc = KServiceGetConfig ( s, & self -> kfg );

    return rc;
}

static rc_t HFini ( H * self ) {
    rc_t rc = 0;

    assert ( self );

    RELEASE ( VResolver, self -> resolver );
    RELEASE ( KConfig, self -> kfg );
    RELEASE ( VFSManager, self -> mgr );

    BSTreeWhack ( & self -> ticketsToResolvers, BSTItemWhack, NULL );

    return rc;
}

static rc_t HResolver(H * self, const KService * service,
    const String * aTicket, VResolver ** resolver, const VPath * path)
{
    rc_t rc = 0;
    bool found = false;
    const String * ticket = aTicket;
    const KNgcObj * ngc = NULL;
    uint32_t projectId = 0;
    bool isProtected = VPathGetProjectId(path, &projectId);

    assert(resolver && self && self->service);

    *resolver = NULL;

    if (isProtected) {
        bool isProtected = false;
        ngc = KServiceGetNgcFile(service, &isProtected);
        if (isProtected) {
            if (ticket == NULL || ticket->addr == NULL || ticket->size == 0) {
                char tic[256] = "";
                rc = KNgcObjGetTicket(ngc, tic, sizeof tic, NULL);
                if (rc == 0) {
                    String s;
                    StringInitCString(&s, tic);
                    rc = StringCopy(&ticket, &s);
                }
            }
        }
    }

    if (ticket && ticket->addr != NULL && ticket->size > 0) {
        BSTItem * i = (BSTItem *)BSTreeFind(
            &self->ticketsToResolvers, ticket, BSTItemCmp);

        if (i != NULL) {
            found = true;
            *resolver = i->resolver;
        }
        else if (!isProtected) {
            /* here find resolver in configuration by ticket 
               ( when ngc file was not provided) */
            rc = KServiceGetResolver(self->service, ticket, resolver);
            if (rc == 0 && *resolver != NULL) {
                found = true;
                i = calloc(1, sizeof * i);
                if (i == NULL)
                    return RC(rcVFS, rcStorage, rcAllocating,
                        rcMemory, rcExhausted);

                rc = StringCopy(&i->ticket, ticket);
                if (rc != 0)
                    return rc;

                i->resolver = *resolver;
                rc = BSTreeInsert(&self->ticketsToResolvers,
                    (BSTNode *)i, BSTreeSort);
            }
        }
    }

    if (isProtected && !found) {
        char buffer[256] = "";
        String s;
        rc = string_printf(buffer, sizeof buffer, NULL, "dbGaP-%d", projectId);
        if (rc != 0)
            return rc;
        StringInitCString(&s, buffer);
        BSTItem * i = (BSTItem *)BSTreeFind(
            &self->ticketsToResolvers, &s, BSTItemCmp);

        if (i != NULL)
            * resolver = i->resolver;
        else {
            rc = KServiceGetResolverForProject(
                self->service, projectId, resolver);
            if (rc != 0)
                /* cannot find resolver in configuration by projectId:
                   ngc file was provided but not imported:
                   use non-protected resolver */
                rc = 0;
            else if (*resolver != NULL) {
                found = true;
                i = calloc(1, sizeof * i);
                if (i == NULL)
                    return RC(rcVFS, rcStorage, rcAllocating,
                        rcMemory, rcExhausted);
                rc = StringCopy(&i->ticket, &s);
                if (rc != 0)
                    return rc;
                i->resolver = *resolver;
                rc = BSTreeInsert(&self->ticketsToResolvers,
                    (BSTNode *)i, BSTreeSort);
            }
        }
    }

    if (rc == 0 && *resolver == NULL) {
        if (self->resolver == NULL)
            rc = VFSManagerMakeResolver(self->mgr, &self->resolver,
                self->kfg);
        *resolver = self->resolver;
    }

    if (rc == 0) { /* use original manager in case it's not a singleton */
        KNSManager * kns = NULL;
        rc = VFSManagerGetKNSMgr(self->mgr, &kns);
        if (rc == 0)
            rc = VResolverResetKNSManager(*resolver, kns);
        RELEASE(KNSManager, kns);
    }

/*  if (rc == 0 && *resolver != NULL)
        VResolverCacheEnable(*resolver, KServiceGetCacheEnable(service)); */

    if (rc == 0) {
        const char * quality = NULL;
        rc = KServiceGetQuality(service, &quality);
        if (rc == 0)
            rc = VResolverSetQuality(*resolver, quality);
    }

    if (ticket != aTicket)
        StringWhack(ticket);

    RELEASE(KNgcObj, ngc);

    return rc;
}

static rc_t _VFSManagerVPathMakeAndTest ( const VFSManager * self,
    const char * outFile, const VPath ** aPath, bool * exists )
{
    rc_t rc = 0;

    VPath * path = NULL;

    KDirectory * dir = NULL;

    assert ( aPath && exists );
    * exists = false;

    rc = KDirectoryNativeDir ( & dir );
    if ( rc != 0 )
        return rc;

    * exists
        = ( KDirectoryPathType ( dir, outFile ) & ~ kptAlias ) != kptNotFound;

    rc = VFSManagerMakePath ( self, & path, outFile );
    if ( rc == 0 )
        * aPath = path;

    RELEASE ( KDirectory, dir );

    return rc;
}

static rc_t VPathCacheLocationForSource(const VPath * self,
    const KDirectory * dir, VPath ** path, rc_t *rcOut)
{
    rc_t rc = 0;

    String acc;
    String name;

    char sPath[PATH_MAX] = "";

    /* Getting cache location for source files.
       When acc & name (come from SDL) are unknown:
        it should be cwd/acc/filename.
        Using a hack: filename is the last part of path;
        acc is the last directory name
       Otherwise cache location is acc/name */

    size_t size = 0;
    const char * s = NULL;

    assert(self && rcOut);

    size = self->path.size;
    s = string_rchr(self->path.addr, size, '/');
    /* the part after the latest slash is the filename */

    rc = VPathGetId(self, &acc);
    if (rc == 0)
        rc = VPathGetName(self, &name);

    if (rc == 0 && acc.size > 0 && name.size > 0) {
        /* acc & name (come from SDL) are known */
        const char * ext = "";
        if (VPathGetProjectId(self, NULL))
            ext = ".ncbi_enc";
        rc = KDirectoryResolvePath(dir, true, sPath, sizeof sPath,
            "%.*s/%.*s%s", acc.size, acc.addr, name.size, name.addr, ext);
    }
    else {
        /* acc & name (come from SDL) are unknown */
        if (s != NULL && s > self->path.addr) {
            /* size of path without filename */
            size = s - self->path.addr;
            /* rfind the second slash for accesion directory */
            s = string_rchr(self->path.addr, size, '/');
        }
        if (s != NULL)
            ++s;
        if (s == NULL)
            s = self->path.addr + self->path.size;
        size = self->path.size - (s - self->path.addr);
        rc = KDirectoryResolvePath(dir, true, sPath, sizeof sPath,
            "%.*s", (int)size, s);
    }

    if (rc == 0)
        *rcOut = VPathMakeFmt(path, "%s", sPath);

    return rc;
}

static rc_t VResolversQuery ( const VResolver * self,
    const VFSManager * mgr, ServicesCache * servicesCache,
    VRemoteProtocols protocols, const VPath * path, const String * acc,
    uint64_t id, VPathSet ** result, ESrvFileFormat ff, const char * outDir,
    const char * outFile, const VPath * mapping, const char * origAcc,
    bool checkCache, bool checkLocal )
{
    rc_t rc = 0;

    VPath * query = NULL;

    uint32_t oid = 0;
    uint32_t i = 0;

    assert ( self && result && acc );

    if ( id == 0 )
        for ( i = 0; i < acc -> size; ++i ) {
            char c = acc -> addr [ i ];
            if ( c < '0' || c > '9' ) {
                oid = 0;
                break;
            }
            oid = oid * 10 + c - '0';
        }
    else
        oid = id;

    if ( oid == 0 ) {
        bool vdbcache = false;
        if (ff == eSFFVdbcache && servicesCache == NULL) {
            String s;
            CONST_STRING(&s, ".vdbcache");
            if (acc->len <= s.len 
                || strstr(acc->addr, s.addr) == NULL)
            {
                vdbcache = true;
            }
        }
        if (vdbcache)
            rc = VFSManagerMakePath ( mgr, & query, "%S.vdbcache", acc );
        else
            rc = VFSManagerMakePath ( mgr, & query, "%S", acc );
    }
    else
        rc = VFSManagerMakeOidPath ( mgr, & query, oid );
    if (rc == 0 && path != NULL) {
        if (path->projectId >= 0) {
            assert(query);
            query->projectId = path->projectId;
        }
        query->quality = path->quality;
    }

    if ( rc == 0 ) {
        VResolverAppID app = appUnknown;

        bool isSource = false;

        const VPath * local = NULL;
        const VPath * cache = NULL;

        rc_t localRc = 0;
        rc_t cacheRc = 0;

        String sragap_files;
        String srapub_files;
        String remote;

        CONST_STRING(&sragap_files, "sragap_files");
        CONST_STRING(&srapub_files, "srapub_files");
        CONST_STRING(&remote      , "remote"      );

        if (path != NULL) {
            isSource = StringEqual(&path->objectType, &srapub_files);
            if (!isSource)
                isSource = StringEqual(&path->objectType, &sragap_files);
            if (!isSource)
                isSource = StringEqual(&path->objectType, &remote);
        }

        app = get_accession_app(acc, false, NULL, NULL,
            false, NULL, NULL, NULL, -1, false);

        if ( outFile != NULL ) {
            bool exists = false;
            cacheRc = _VFSManagerVPathMakeAndTest
                ( mgr, outFile, & cache, & exists );
            if ( cacheRc == 0 && exists ) {
                localRc = VPathAddRef ( cache );
                if ( localRc == 0 )
                    local = cache;
            }
            else if ( cacheRc != 0 )
                localRc = cacheRc;
            else
                localRc = RC ( rcVFS, rcResolver, rcResolving,
                                      rcName, rcNotFound );
        }

        else if (!VResolverIsProtected(self)
            && app == appSRA && origAcc != NULL && origAcc[0] == 'S'
            && servicesCache != NULL)
        {
            rc = ServicesCacheResolve(servicesCache, path, &local, &cache);
            if (rc == 0) {
                if (local == NULL)
                    localRc = RC(rcVFS, rcResolver, rcResolving,
                        rcName, rcNotFound);
                if (cache == NULL)
                    cacheRc = RC(rcVFS, rcResolver, rcResolving,
                        rcName, rcNotFound);
            }
        }

        else if ( VPathFromUri ( path ) ) {
            if (isSource) {
                KDirectory * dir = NULL;
                VPath * tmp = NULL;
                rc_t rcOut = 0;
                if (rc == 0)
                    rc = KDirectoryNativeDir(&dir);
                if (rc == 0)
                    rc = VPathCacheLocationForSource(path, dir, &tmp, &rcOut);
                if (rc == 0) {
                    char sPath[PATH_MAX] = "";
                    rc = VPathReadPath(tmp, sPath, sizeof sPath, NULL);
                    if (rc == 0) {
                        if ((KDirectoryPathType(dir, sPath) & ~kptAlias)
                            != kptNotFound)
                        {
                            rc = VPathAddRef(tmp);
                            if (rc == 0) {
                                local = tmp;
                                localRc = 0;
                            }
                            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
                                ("VResolversQuery: local location "
                                    "of '%S' resolved to '%S' with %R\n",
                                    acc, &(tmp->path), rc));
                        }
                        else {
                            localRc = RC(rcVFS, rcResolver, rcResolving,
                                rcName, rcNotFound);
                            DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
                                ("VResolversQuery: "
                                    "local location of '%S' not found\n", acc));
                        }
                    }
                    if (VResolverResolveToAd(self)) {
                        rc = VPathAddRef(tmp);
                        if (rc == 0) {
                            cache = tmp;
                            cacheRc = 0;
                        }
                        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS), ("VResolversQuery: "
                            "cache location of '%S' resolved to '%S' with %R\n",
                            acc, &(tmp->path), rc));

                    }
                    else {
                        cacheRc = RC(rcVFS, rcResolver, rcResolving,
                            rcName, rcNotFound);
                        DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS),
                            ("VResolversQuery: "
                                "cache location of '%S' not found\n", acc));
                    }
                }
                RELEASE(VPath, tmp);
                RELEASE(KDirectory, dir);
            }
            else {
                if (checkCache)
                    cacheRc = VResolverQueryWithDir(self, protocols, query,
                        NULL, NULL, &cache, false, outDir, NULL, true, path,
                        mapping);
                if (checkLocal)
                    localRc = VResolverQueryWithDir(self, protocols, query,
                        &local, NULL, NULL, false, outDir, NULL, true, path,
                        mapping);
            }
        }

        else {
            if (checkCache)
                cacheRc = VResolverQueryWithDir(self, protocols, query,
                    NULL, NULL, &cache, false, outDir, NULL, true, path,
                    mapping);
            if (checkLocal)
                localRc = VResolverQueryWithDir(self, protocols, query,
                    &local, NULL, NULL, false, outDir, NULL, true, path,
                    mapping);
/*          if (checkCache)
                cacheRc = VResolverQueryForCache ( self, protocols, query,
                                    NULL, NULL, & cache );
            if (checkLocal)
                localRc = VResolverQueryForCache ( self, protocols, query,
                                    & local, NULL, NULL ); */
        }

        VPathSetMakeQuery ( result, local, localRc, cache, cacheRc );

        RELEASE ( VPath, local );
        RELEASE ( VPath, cache );
    }

    RELEASE ( VPath, query );

    return rc;
}

static void _StringFixSrrWithVersion(String * self) {
    size_t dot = 0;
    size_t i = 0;

    const char * p = NULL;

    assert(self);

    p = self->addr;

    if (self->size < 4)
        return;

    if (self->size != self->len)
        return;

    if (p[0] != 'S' || p[1] != 'R' || p[2] != 'R')
        return;

    for (i = 3; i < self->size; ++i)
        if (p[i] == '.') {
            dot = i;
            break;
        }
    if (dot == 0)
        return;

    for (++i; i < self->size; ++i)
        if (!isdigit(p[i]))
            return;

    self->size = self->len = dot;
}

static rc_t _VPathGetId ( const VPath * self, const String ** newId,
                          String * oldId, const VFSManager * mgr ) 
{
    rc_t rc = 0;

    VPath * acc_or_oid = NULL;

    String id;

    String fasp;
    String http;
    String https;

    bool replace = true;

    assert ( newId && oldId );

    * newId = NULL;

    if ( oldId -> addr == NULL )
        return 0;

    CONST_STRING ( & fasp , "fasp://"  );
    CONST_STRING ( & http , "http://"  );
    CONST_STRING ( & https, "https://" );

    if ( oldId -> size <= https . size )
        return 0;

    /* what is being attempted with this code?
    if ( string_cmp ( oldId -> addr, oldId -> size, https . addr, https. size, https. size ) != 0 &&
         string_cmp ( oldId -> addr, oldId -> size, fasp  . addr, fasp . size, fasp . size ) != 0 &&
         string_cmp ( oldId -> addr, oldId -> size, http  . addr, fasp . size, fasp . size ) != 0 )
    {   return 0; } */

    rc = VPathGetId ( self, & id );
    if ( rc == 0 && id . size == 0 ) {
        rc = VFSManagerExtractAccessionOrOID ( mgr, & acc_or_oid, self );
        if ( rc != 0 ) {
            const String * str = NULL;
            rc_t r = VPathMakeString ( self, & str );
            if ( r == 0 ) assert ( str );

            if ( r == 0 && str != NULL && str -> size > 0 &&
                            str -> addr [ str -> size - 1 ] == '/' )
            {
                rc = VFSManagerMakePath ( mgr, & acc_or_oid,
                                          "ncbi-file:index.html" );
            }
            else {
                const char * start = str -> addr;
                size_t size = str -> size;
                const char * end = start + size;
                const char * slash = string_rchr ( start, size, '/' );
                const char * scol = NULL;

                String scheme;

                String fasp;
                CONST_STRING ( & fasp, "fasp" );

                rc = VPathGetScheme ( self, & scheme );
                if ( rc == 0 ) {
                    if ( StringEqual ( & scheme, & fasp ) )
                        scol = string_rchr ( start, size, ':' );
                    if ( slash != NULL )
                        start = slash + 1;
                    if ( scol != NULL && scol > start )
                        start = scol + 1;

                    rc = VFSManagerMakePath ( mgr, & acc_or_oid,
                        "%.*s", ( uint32_t ) ( end - start ), start );
                }
            }

            free ( ( void * ) str );
        }

        if ( rc == 0 )
            rc = VPathGetPath ( acc_or_oid, & id );
    }

    if ( rc == 0 ) {
        /* return ID just when it's different from oldId
        (replace file name by numeric [dbGaP] id) */
        if (oldId != NULL &&
            oldId->addr != NULL && oldId->size > 0 &&
            id.addr != NULL && id.size > 0 &&
            oldId->addr[0] == id.addr[0])
        {
            replace = false;
        }
        else {
            _StringFixSrrWithVersion(&id);
            rc = StringCopy(newId, &id);
        }
    }

    RELEASE ( VPath, acc_or_oid );

    if ( rc == 0 && replace )
        * oldId = * * newId;

    return rc;
}

typedef struct {
    char * acc;
    char * name;

    const KSrvRespFile * file; /* don't release */
} LocalAndCache;

static rc_t LocalAndCacheRelease(LocalAndCache * self) {
    if (self != NULL) {
        free(self->acc);
        free(self->name);

        memset(self, 0, sizeof *self);

        free(self);
    }

    return 0;
}

static rc_t KSrvResponseRegisterLocalAndCache(KSrvResponse * self,
    rc_t rc, KSrvRespFile * file, const VPathSet * localAndCache)
{
    LocalAndCache * lnc = NULL;

    if (rc != 0)
        return rc;

    if (localAndCache != NULL)
        rc = KSrvRespFileAddLocalAndCache(file, localAndCache);

    if (rc == 0)
        rc = KSrvResponseAddLocalAndCacheToTree(self, file);

    RELEASE(LocalAndCache, lnc);

    return rc;
}

static rc_t KServiceResolvers(const KService * self, VRemoteProtocols protocols,
    const char * outDir, const char * outFile,
    H * h, VPathSet ** vps, const VPath * path,
    ESrvFileFormat ff, const char * tic, uint64_t iid, const VPath * mapping,
    String * id, const char * origAcc, bool checkCache, bool checkLocal)
{
    rc_t rc = 0;
    VResolver * resolver = NULL;
    const String * pId = NULL;
    String acc;
    String ticket;
    memset(&acc, 0, sizeof acc);
    memset(&ticket, 0, sizeof ticket);
    assert(h);
    if (rc == 0 && tic != NULL)
        StringInitCString(&ticket, tic);
    if (rc == 0)
        rc = HResolver(h, self, &ticket, &resolver, path);
    if (rc == 0 && path != NULL) {
        rc = VPathGetAccession(path, &acc);
        if (rc == 0 && acc.size > 0) {
            if (!(id->size > 0 && id->addr != NULL && id->addr[0] == 'S' &&
                acc.size > 0 && acc.addr != NULL && isdigit(acc.addr[0])))
                /* Don't replace id with acc
                   when id starts with S but acc is number.
                   E.g., it happens when id is SRRnnn.pileup */
            {
                id = &acc;
            }
        }
        else
            rc = _VPathGetId(path, &pId, id, h->mgr);
    }
    if (rc == 0) {
        assert(resolver);
        VResolverResolveName(resolver, KServiceGetResolveName(self));
        rc = VResolversQuery(resolver, h->mgr, h->cache, protocols, path,
            id, iid, vps, ff, outDir, outFile, mapping, origAcc,
            checkCache, checkLocal);
    }
    free((void *)pId);
    return rc;
}

#ifdef DBGNG
#define STS_FIN  3
#endif

static
rc_t KServiceNamesQueryExtImpl ( KService * self, VRemoteProtocols protocols, 
    const char * cgi, const char * version, const KSrvResponse ** aResponse,
    const char * outDir, const char * outFile, const char * expected )
{
    rc_t RC_NOT_FND
        = SILENT_RC(rcVFS, rcQuery, rcResolving, rcName, rcNotFound);
    rc_t rc = 0;
    ServicesCache * cache = NULL;
    uint32_t i = 0;
    const KSrvRespObj * obj = NULL;
    const VPath * path = NULL;
    KSrvResponse * response = NULL;
    rc_t rx = 0;
    KSrvRespObjIterator * it = NULL;
    KSrvRespFile * file = NULL;
    KSrvRespFileIterator * fi = NULL;
    bool skipLocal = false;

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: entered", __func__));
#endif
    if ( aResponse == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    * aResponse = NULL;

    if (self == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcSelf, rcNull);

/* THIS FUNCTION WAS REOGRANIZED TO RESOLVE LOCALLY FIRST, THEN REMOTELY
   TO INTEGRATE RUNS HAVING DIFFERENT QUALITY TYPES. */

    {   /* call External Services */
        const KSrvResponse * r = NULL;

#ifdef DBGNG
        STSMSG(STS_FIN, ("%s: entering KServiceNamesExecuteExtImpl...",
            __func__));
#endif
        rc = KServiceHasQuery(self);
        if (rc != 0) /* request is empty */
            return rc;
        else {
            uint32_t i = 0;
            H h;

            KServiceInitQuality(self);

/*************** WE ALWAYS RESOLVE LOCALLY FIRST, THEN REMOTELY. **************/
            bool isProtected = false;
            const KNgcObj * o = KServiceGetNgcFile(self, &isProtected); 
            RELEASE(KNgcObj, o);
/******** Except when we accessing protected data (they never have 0-quality) */
            skipLocal = isProtected;
/******** or when prefetch is run with --type all
          (we expect multiple files in response) ******************************/
            if (!skipLocal)
                skipLocal = KServiceAnyFormatRequested(self);
            if (rc == 0)
                rc = HInit(&h, self, cache);

#ifdef DBGNG
            STSMSG(STS_FIN, ("%s: iterating  KServiceGetId...", __func__));
#endif

/********* Iterate every ID in request.
           (Really, all sra-tools always call this function with a single ID) */
            for (i = 0; ; ++i) {
                rc_t lRc = 0, rRc = 0;
                VPath * path = NULL;
                VPathSet * vps = NULL;
                ESrvFileFormat ff = eSFFInvalid;
                uint64_t iid = 0;
                String id;

                const char * acc = KServiceGetId(self, i);
                if (acc == NULL)
                    break;

                DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_SERVICE), (
                    "VVVVVVVVVVVVVVVVVVVVVVVVVV KServiceNamesQueryExtImpl:\n"));

                StringInitCString(&id, acc);
                if (rc == 0)
                    rc = VPathMake(&path, acc);
                if (rc == 0 && !VPathFromUri(path))
                    RELEASE(VPath, path);

/********* RESOLVE LOCALLY *********/
                if (rc == 0 && !skipLocal)
                    rc = KServiceResolvers(self, protocols, outDir, outFile,
                        &h, &vps, path, ff, NULL, iid, NULL, &id, NULL,
                        false, true);

                if (rc == 0) {
/********* ADD LOCAL RESULTS TO RESPONSE *********/
                    if (!skipLocal)
                        lRc = KServiceAddLocalAndCacheToResponse(
                            self, acc, vps);

/********* RESOLVE REMOTELY *********/
                    if (lRc != 0 || skipLocal || !KServiceSkipRemote(self)) {
                        rc_t r2 = 0;
                        if (version == NULL) version = "130";
                        r2 = KServiceInitNamesRequestWithVersion(self,
                            protocols, cgi, version,
                            false, expected == NULL, i);
                        if (r2 == 0) {
                            rRc = KServiceNamesExecuteExtImpl(self,
                                protocols, cgi, version, &r, expected,
                                i);
                            if (rRc != 0 && rRc != RC_NOT_FND)
                                r2 = rRc;
                        }
                        if (rc == 0 && r2 != 0)
                            rc = r2;
                    } else {
                        if (rc == 0)
                            rc = KServiceGetResponse(self, &r);
                    }
                }
                else
                    rc = 0;

                RELEASE(VPathSet, vps);
                RELEASE(VPath, path);
                if (rc == 0 && lRc != 0 && rRc != 0)
                    rc = rRc;
            }
            {
                rc_t r2 = HFini(&h);
                if (r2 != 0 && rc == 0)
                    rc = r2;
            }
        }

        if ( rc == 0 )
            response = ( KSrvResponse* ) r;
    }

/******* HERE [LOCAL] [AND] [REMOTE] RESOLVING WAS DONE,
         NOW WE RESOLVE CACHE LOCATION AND LOCAL - IF IT WAS NOT DONE BEFORE. */
    if ( rc == 0 ) {
        H h;
        rc = HInit ( & h, self, cache );

#ifdef DBGNG
        STSMSG(STS_FIN, ("%s: iterating  KSrvResponse...", __func__));
#endif

        if ( protocols == eProtocolDefault )
             protocols = DEFAULT_PROTOCOLS;
        
        {
            uint32_t n = KSrvResponseLength  ( response );
            for ( i = 0; rc == 0 && i < n; ++ i ) {
                VPathSet * vps = NULL;
                const KSrvError * error = NULL;
                rc = KSrvResponseGetObjByIdx ( response, i, & obj );
                if ( rc == 0 ) {
                    rc = KSrvRespObjGetError(obj, &rx, NULL, NULL);
                    if (rc == 0 && rx == 0) {
                        bool noRecheckLocal = true;
/******* WE RESOLVE LOCALLY HERE IF IT WAS NOT DONE BEFORE. *******/
                        if (skipLocal)
                            noRecheckLocal = false;
/******* IF REMOTE RESPONSE HAS MULTIPLE FILES - WE RESOLVE LOCALLY AGAIN. ****/
                        else
                            rc = KSrvRespObjIsSimple(obj, &noRecheckLocal);

                        if (rc == 0)
                            rc = KSrvRespObjMakeIterator(obj, &it);
                        while (rc == 0) {
                            rc = KSrvRespObjIteratorNextFile(it, &file);
                            if (rc != 0 || file == NULL)
                                break;
                            else {
                                ESrvFileFormat ff = eSFFInvalid;
                                const char * acc = NULL;
                                const char * tic = NULL;
                                uint64_t iid = 0;
                                const VPath * mapping = NULL;
                                String id;
                                const char * origAcc = NULL;
                                memset(&id, 0, sizeof id);
                                rc = KSrvRespFileGetAccOrId(file,
                                    &origAcc, NULL);
                                if (rc == 0)
                                    rc = KSrvRespFileGetAccOrName(file,
                                        &acc, &tic);
                                if (rc == 0) {
                                    if (acc != NULL) {
                                        if (origAcc != NULL &&
                                            isdigit(origAcc[0]))
                                        {
                                            StringInitCString(&id, origAcc);
                                        }
                                        else
                                            StringInitCString(&id, acc);
                                        rc = KSrvRespFileGetFormat(file,
                                            &ff);
                                    }
                                    else {
                                        rc = KSrvRespFileGetId(file, &iid,
                                            &tic);
                                    }
                                }
                                if (rc == 0)
                                    KSrvRespFileGetMapping(file, &mapping);
                                if (rc == 0)
                                    rc = KSrvRespFileMakeIterator(file,
                                        &fi);
                                if (rc == 0) {
                                    rc = KSrvRespFileIteratorNextPath(
                                        fi, &path);
                                    if (rc == 0) {
                                        if (error == NULL)
                                            rc = KServiceResolvers(self,
                                                protocols, outDir, outFile,
                                                &h, &vps, path, ff, tic, iid,
                                                mapping, &id, origAcc,
                                                true, !noRecheckLocal);
                                        else
                                            RELEASE(KSrvError, error);
                                    }
                                    rc = KSrvResponseRegisterLocalAndCache(
                                        response, rc, file, vps);
                                    RELEASE(VPathSet, vps);
                                    RELEASE(VPath, path);
                                }
                                if (rc == SILENT_RC(rcVFS,
                                    rcQuery, rcExecuting, rcItem, rcNotFound))
                                {
        /* returned by KSrvRespFileMakeIterator when no remote path exists */
                                    rc = 0;
                                }
                                RELEASE(KSrvRespFileIterator, fi);
                                RELEASE(VPath, mapping);
                            }
                            RELEASE(KSrvRespFile, file);
                        }
                        RELEASE(KSrvRespObjIterator, it);
                    }
                }
                else {
                    rc = KSrvResponseGetPath
                        ( response, i, protocols, & path, NULL, & error );
                    if ( rc == 0 ) {
                        if ( error == NULL ) {
                            VPath * acc_or_oid = NULL;
                            VResolver * resolver = NULL;
                            String id;
                            String ticket;
                            rc = VPathGetId ( path, & id );
                            if ( rc == 0 && id . size == 0) {
                                rc = VFSManagerExtractAccessionOrOID (h . mgr,
                                        & acc_or_oid, path );
                                if ( rc == 0 )
                                    rc = VPathGetPath ( acc_or_oid, & id );
                            }
                            if ( rc == 0 )
                                rc = VPathGetTicket ( path, & ticket );
                            if ( rc == 0 )
                                rc = HResolver ( &h, self, &ticket, &resolver,
                                    NULL );
                            if ( rc == 0 ) {
                                assert ( resolver );
                                VResolverResolveName ( resolver,
                                            KServiceGetResolveName ( self ) );
                                rc = VResolversQuery ( resolver, h . mgr,
                                    h . cache, protocols, path, & id, 0, & vps,
                                    eSFFInvalid, outDir, outFile, NULL, NULL,
                                    false, true );
                            }
                            RELEASE ( VPath, acc_or_oid );
                        }
                        else
                            RELEASE ( KSrvError, error );
                    }
                    if ( vps != NULL ) {
                        rc = KSrvResponseAddLocalAndCache ( response, i, vps );
                        RELEASE ( VPathSet, vps );
                    }
                    RELEASE ( VPath, path );
                }
                RELEASE ( KSrvRespObj, obj );
            }
            * aResponse = response;
        }
        {
            rc_t r2 = HFini ( & h );
            if ( rc == 0 )
                rc = r2;
        }
    }

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: exiting with %R", __func__, rc));
#endif
    return rc;
}

#if 0
static
rc_t KServiceNamesQueryExtImpl ( KService * self, VRemoteProtocols protocols, 
    const char * cgi, const char * version, const KSrvResponse ** aResponse,
    const char * outDir, const char * outFile, const char * expected )
{
    rc_t RC_NOT_FND
        = SILENT_RC(rcVFS, rcQuery, rcResolving, rcName, rcNotFound);
    rc_t rc = 0;
    ServicesCache * cache = NULL;
    uint32_t i = 0;
    const KSrvRespObj * obj = NULL;
    const VPath * path = NULL;
    KSrvResponse * response = NULL;
    rc_t rx = 0;
    KSrvRespObjIterator * it = NULL;
    KSrvRespFile * file = NULL;
    KSrvRespFileIterator * fi = NULL;

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: entered", __func__));
#endif
    if ( aResponse == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    * aResponse = NULL;

    {   /* call External Services */
        const KSrvResponse * r = NULL;
#ifdef DBGNG
        STSMSG(STS_FIN, ("%s: entering KServiceNamesExecuteExtImpl...",
            __func__));
#endif
        if (version == NULL) version = "130";
        rc = KServiceInitNamesRequestWithVersion(self, protocols, cgi, version,
            false, expected == NULL, -1);
        if (rc == 0)
            rc = KServiceNamesExecuteExtImpl ( self, protocols, cgi,
                                       version, & r, expected, -1 );
#ifdef DBGNG
        STSMSG(STS_FIN, ("%s: ...KServiceNamesExecuteExtImpl done with %R",
            __func__, rc));
#endif
        if ( rc == 0 )
            response = ( KSrvResponse* ) r;
    }

    if ( rc == 0 ) {
        H h;
        rc = HInit ( & h, self, cache );

#ifdef DBGNG
        STSMSG(STS_FIN, ("%s: iterating  KSrvResponse...", __func__));
#endif

        if ( protocols == eProtocolDefault )
             protocols = DEFAULT_PROTOCOLS;
        
        {
            uint32_t n = KSrvResponseLength  ( response );
            for ( i = 0; rc == 0 && i < n; ++ i ) {
                VPathSet * vps = NULL;
                const KSrvError * error = NULL;
                rc = KSrvResponseGetObjByIdx ( response, i, & obj );
                if ( rc == 0 ) {
                    rc = KSrvRespObjGetError(obj, &rx, NULL, NULL);
                    if (rc == 0 && rx == 0) {
                        rc = KSrvRespObjMakeIterator(obj, &it);
                        while (rc == 0) {
                            rc = KSrvRespObjIteratorNextFile(it, &file);
                            if (rc != 0 || file == NULL)
                                break;
                            else {
                                ESrvFileFormat ff = eSFFInvalid;
                                const char * acc = NULL;
                                const char * tic = NULL;
                                uint64_t iid = 0;
                                const VPath * mapping = NULL;
                                String id;
                                const char * origAcc = NULL;
                                memset(&id, 0, sizeof id);
                                rc = KSrvRespFileGetAccOrId(file,
                                    &origAcc, NULL);
                                if (rc == 0)
                                    rc = KSrvRespFileGetAccOrName(file,
                                        &acc, &tic);
                                if (rc == 0) {
                                    if (acc != NULL) {
                                        if (origAcc != NULL &&
                                            isdigit(origAcc[0]))
                                        {
                                            StringInitCString(&id, origAcc);
                                        }
                                        else
                                            StringInitCString(&id, acc);
                                        rc = KSrvRespFileGetFormat(file,
                                            &ff);
                                    }
                                    else {
                                        rc = KSrvRespFileGetId(file, &iid,
                                            &tic);
                                    }
                                }
                                if (rc == 0)
                                    KSrvRespFileGetMapping(file, &mapping);
                                if (rc == 0)
                                    rc = KSrvRespFileMakeIterator(file,
                                        &fi);
                                if (rc == 0) {
                                    rc = KSrvRespFileIteratorNextPath(
                                        fi, &path);
                                    if (rc == 0) {
                                        if (error == NULL)
                                            rc = KServiceResolvers(self,
                                                protocols, outDir, outFile,
                                                &h, &vps, path, ff, tic, iid,
                                                mapping, &id, origAcc);
                                        else
                                            RELEASE(KSrvError, error);
                                    }
                                    rc = KSrvResponseRegisterLocalAndCache(
                                        response, rc, file, vps);
                                    RELEASE(VPathSet, vps);
                                    RELEASE(VPath, path);
                                }
                                RELEASE(KSrvRespFileIterator, fi);
                                RELEASE(VPath, mapping);
                            }
                            RELEASE(KSrvRespFile, file);
                        }
                        RELEASE(KSrvRespObjIterator, it);
                    }
                }
                else {
                    rc = KSrvResponseGetPath
                        ( response, i, protocols, & path, NULL, & error );
                    if ( rc == 0 ) {
                        if ( error == NULL ) {
                            VPath * acc_or_oid = NULL;
                            VResolver * resolver = NULL;
                            String id;
                            String ticket;
                            rc = VPathGetId ( path, & id );
                            if ( rc == 0 && id . size == 0) {
                                rc = VFSManagerExtractAccessionOrOID (h . mgr,
                                        & acc_or_oid, path );
                                if ( rc == 0 )
                                    rc = VPathGetPath ( acc_or_oid, & id );
                            }
                            if ( rc == 0 )
                                rc = VPathGetTicket ( path, & ticket );
                            if ( rc == 0 )
                                rc = HResolver ( &h, self, &ticket, &resolver,
                                    NULL );
                            if ( rc == 0 ) {
                                assert ( resolver );
                                VResolverResolveName ( resolver,
                                            KServiceGetResolveName ( self ) );
                                rc = VResolversQuery ( resolver, h . mgr,
                                    h . cache, protocols, path, & id, 0, & vps,
                                    eSFFInvalid, outDir, outFile, NULL, NULL );
                            }
                            RELEASE ( VPath, acc_or_oid );
                        }
                        else
                            RELEASE ( KSrvError, error );
                    }
                    if ( vps != NULL ) {
                        rc = KSrvResponseAddLocalAndCache ( response, i, vps );
                        RELEASE ( VPathSet, vps );
                    }
                    RELEASE ( VPath, path );
                }
                RELEASE ( KSrvRespObj, obj );
            }
            * aResponse = response;
        }
        {
            rc_t r2 = HFini ( & h );
            if ( rc == 0 )
                rc = r2;
        }
    }
    else if (rc == RC_NOT_FND) {
        if (KServiceGetId(self, 0) == NULL) /* request is empty */
            return rc;
        else {
            uint32_t i = 0;
            H h;
            rc = HInit(&h, self, cache);

#ifdef DBGNG
            STSMSG(STS_FIN, ("%s: iterating  KServiceGetId...", __func__));
#endif

            for (i = 0; ; ++i) {
                VPathSet * vps = NULL;
                ESrvFileFormat ff = eSFFInvalid;
                uint64_t iid = 0;
                String id;
                const char * acc = KServiceGetId(self, i);
                if (acc == NULL)
                    break;
                StringInitCString(&id, acc);
                if (rc == 0)
                    rc = KServiceResolvers(self, protocols, outDir, outFile,
                        &h, &vps, NULL, ff, NULL, iid, NULL, &id, NULL);
                if (rc == 0) {
                    if (rc == 0)
                        rc = KServiceAddLocalAndCacheToResponse(self, acc, vps);
                    else
                        rc = 0;
                    RELEASE(VPathSet, vps);
                    if (rc == 0)
                        rc = KServiceGetResponse(self, aResponse);
                }
            }
            {
                rc_t r2 = HFini(&h);
                if (r2 != 0 && rc == 0)
                    rc = r2;
            }
        }
    }

#ifdef DBGNG
    STSMSG(STS_FIN, ("%s: exiting with %R", __func__, rc));
#endif
    return rc;
}
#endif

rc_t KServiceNamesQueryExt ( KService * self, VRemoteProtocols protocols, 
    const char * cgi, const char * version, const char * outDir,
    const char * outFile, const KSrvResponse ** aResponse )
{
    return KServiceNamesQueryExtImpl ( self, protocols, cgi, version,
                                       aResponse, outDir, outFile, NULL );
}
                             
rc_t KServiceNamesQuery ( KService * self, VRemoteProtocols protocols,
                          const KSrvResponse ** aResponse )
{
    return KServiceNamesQueryExt ( self, protocols, NULL, NULL,
                                   NULL, NULL, aResponse );
}

rc_t KServiceNamesQueryTo ( KService * self, VRemoteProtocols protocols,
    const char * outDir, const char * outFile,
    const KSrvResponse ** response )
{
    return KServiceNamesQueryExt ( self, protocols, NULL, NULL,
                                   outDir, outFile, response );
}

rc_t KServiceTestNamesQueryExt ( KService * self, VRemoteProtocols protocols,
    const char * cgi, const char * version, const KSrvResponse ** response,
    const char * dir, const char * file, const char * expected )
{
    return KServiceNamesQueryExtImpl
        ( self, protocols, cgi, version, response, dir, file, expected );
}

/* accepts VFSManager* == NULL */
const String * VFSManagerExtSra(const struct VFSManager * self) {
    static String s;
    if (s.len == 0)
        CONST_STRING(&s, ".sra");

    return &s;
}
const String * VFSManagerExtNoqual(const struct VFSManager * self) {
    static String xNoqual;
    if (xNoqual.len == 0)
        CONST_STRING(&xNoqual, ".sralite");

    return &xNoqual;
}
const String * VFSManagerExtNoqualOld(const struct VFSManager * self) {
    static String xNoqual;
    if (xNoqual.len == 0)
        CONST_STRING(&xNoqual, ".noqual");

    return &xNoqual;
}

static rc_t KDirectoryLocalMagicResolve(const KDirectory * self,
    const String * accession, const VPath ** path,
    VResolverAppID app, const struct VResolverAccToken * tok,
    bool legacy_wgs_refseq, const char * dir)
{
    const VPath * magic = NULL;
    bool checkAd = false;
    rc_t rc = 0;

    assert(path && self);
    *path = NULL;

    rc = KDirectoryMagicResolve(self, &magic, accession, app,
        ENV_MAGIC_LOCAL,
        eCheckExistTrue, eCheckFilePathTrue, eCheckUrlFalse, &checkAd);
    if (rc != 0)
        return rc;

    if (!checkAd) {
        const VPath * vdbcache = NULL;
        rc = KDirectoryMagicResolve(self, &vdbcache, accession, app,
            "VDB_LOCAL_VDBCACHE",
            eCheckExistTrue, eCheckFilePathTrue, eCheckUrlFalse, &checkAd);
        if (rc == 0) {
            if (vdbcache == NULL && magic != NULL) {
                rc = VFSManagerMakePathWithExtension((VFSManager*)1,
                    (VPath**)&vdbcache, magic, ".vdbcache");
                if (rc == 0) {
                    assert(vdbcache);
                    if ((KDirectoryPathType(self, vdbcache->path.addr)
                        & ~kptAlias) != kptFile)
                    {
                        RELEASE(VPath, vdbcache);
                    }
                }
            }
            VPathAttachVdbcache((VPath*)magic, vdbcache);
            RELEASE(VPath, vdbcache);
        }
        *path = magic;
        return rc;
    }
    else {
        assert(0);
        /* When LOCAL magic env.var. is found and it refers to AD
           (the same as accession but it's a directory in cwd)
           - we still need to resolve it to real path.
        const VPath * local = NULL;
        rc = VResolverCheckAD(self, &local, app, tok, legacy_wgs_refseq, dir);
        if (rc == 0)
            *path = local;
        RELEASE(VPath, magic);

        return rc; */
    }
}

LIB_EXPORT rc_t CC VFSManagerResolve(const VFSManager * self,
    const char * in, const VPath ** out)
{
    rc_t rc = 0;
    KService * s = NULL;
    const KSrvResponse * r = NULL;
    const KSrvRespObj * obj = NULL;
    KSrvRespObjIterator * it = NULL;
    KSrvRespFile * file = NULL;
    KSrvRespFileIterator * fi = NULL;
    bool ok = false;
    KSrvRunIterator * ri = NULL;
    const KSrvRun * run = NULL;
    String vdbcache;

    if (out == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcParam, rcNull);
    if (self == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcSelf, rcNull);

    {
        String s;
        StringInitCString(&s, in);
        VResolverAppID app = get_accession_app(&s, false, NULL, NULL,
            false, NULL, NULL, NULL, -1, false);
        rc = KDirectoryLocalMagicResolve(
            self->cwd, &s, out, app, NULL, false, NULL);
        if (rc != 0 || *out != NULL)
            return rc;

        rc = KDirectoryMagicResolve(self->cwd, out, &s, app,
            ENV_MAGIC_REMOTE,
            eCheckExistFalse, eCheckFilePathFalse, eCheckUrlTrue, NULL);
        if (rc != 0 || *out != NULL)
            return rc;
    }

    CONST_STRING(&vdbcache, "vdbcache");
    rc = KServiceMakeWithMgr(&s, self, NULL, NULL);
    if (rc == 0)
        rc = KServiceAddId(s, in);
    if (rc == 0) {
        const char * ngc = KConfigGetNgcFile();
        if (ngc != NULL)
            rc = KServiceSetNgcFile(s, ngc);
    }
    if (rc == 0)
        rc = KServiceNamesQuery(s, eProtocolHttps, &r);
    if (rc == 0)
        rc = KSrvResponseMakeRunIterator(r, &ri);
    if (rc == 0)
        rc = KSrvRunIteratorNextRun(ri, &run);
    if (rc == 0 && run != NULL) {
        const VPath * local = NULL;
        const VPath * remote = NULL;
        rc = KSrvRunQuery(run, &local, &remote, NULL, NULL);
        if (rc == 0) {
            if (local != NULL) {
                *out = local;
                RELEASE(VPath, remote);
            }
            else if (remote != NULL) {
                *out = remote;
                RELEASE(VPath, local);
            }
        }
        ok = true;
    }
    if (rc == 0 && ! ok) {
        uint32_t n = KSrvResponseLength(r);
        if (n != 1)
            rc = RC(rcVFS, rcQuery, rcExecuting, rcRow, rcIncorrect);
    }
    if (rc == 0 && !ok)
        rc = KSrvResponseGetObjByIdx(r, 0, &obj);
    if (rc == 0 && !ok)
        rc = KSrvRespObjMakeIterator(obj, &it);
    while (rc == 0 && !ok) {
        RELEASE(KSrvRespFile, file);
        rc = KSrvRespObjIteratorNextFile(it, &file);
        if (rc == 0) {
            if (file != NULL)
                rc = KSrvRespFileMakeIterator(file, &fi);
            else {
                rc = RC(
                    rcVFS, rcQuery, rcResolving, rcName, rcNotFound);
                break;
            }
        }
        if (rc == 0) {
            const VPath * tmp = NULL;
            String type;
            rc = KSrvRespFileIteratorNextPath(fi, &tmp);
            if (rc == 0 && tmp != NULL) {
                rc = VPathGetType(tmp, &type);
                if (rc == 0)
                    if (!StringEqual(&type, &vdbcache))
                        ok = true;
            }
            if (ok)
                *out = tmp;
            else
                RELEASE(VPath, tmp);
        }
    }
    RELEASE(KSrvRunIterator, ri);
    RELEASE(KSrvRun, run);
    RELEASE(KSrvRespFileIterator, fi);
    RELEASE(KSrvRespFile, file);
    RELEASE(KSrvRespObjIterator, it);
    RELEASE(KSrvRespObj, obj);
    RELEASE(KService, s);
    RELEASE(KSrvResponse, r);
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerResolveVPathAll(const VFSManager * self,
    const VPath * in, const VPath ** local,
    const VPath ** remote, const VPath ** cache)
{
    VResolver * resolver = NULL;
    rc_t rc = VFSManagerGetResolver(self, &resolver);
    if (rc == 0)
        rc = VResolverQuery(resolver, 0, in, local, remote, cache);
    if (rc == 0 && *remote == NULL)
        // ignore rc
        VResolverQuery(resolver, 0, in, NULL, remote, cache);
    RELEASE(VResolver, resolver);
    return rc;
}

VFS_EXTERN rc_t CC VFSManagerResolveVPathRemote(const VFSManager * self,
    const struct VPath * in,
    const struct VPath ** remote, const struct VPath ** cache)
{
    return VFSManagerResolveVPathAll(self, in, NULL, remote, cache);
}

VFS_EXTERN rc_t CC VFSManagerResolveRemote(const VFSManager * self,
    const char * in,
    const struct VPath ** remote, const struct VPath ** cache)
{
    return VFSManagerResolveAll(self, in, NULL, remote, cache);
}

LIB_EXPORT rc_t CC VFSManagerResolveVPathLocal(const VFSManager * self,
    const VPath * in, const VPath ** out)
{
    VResolver * resolver = NULL;
    rc_t rc = VFSManagerGetResolver(self, &resolver);
    if (rc == 0)
        rc = VResolverQuery(resolver, 0, in, out, NULL, NULL);
    RELEASE(VResolver, resolver);
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerResolveLocal(const VFSManager * self,
    const char * in, const struct VPath ** out)
{
    VPath * path = NULL;
    rc_t rc = VFSManagerMakePath(self, &path, "%s", in);
    if (rc == 0)
        rc = VFSManagerResolveVPathLocal(self, path, out);
    RELEASE(VPath, path);
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerResolveAll(const VFSManager * self,
    const char * in, const VPath ** local,
    const VPath ** remote, const VPath ** cache)
{
    rc_t rc = 0;
    VPath * path = NULL;
    if (in == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcParam, rcNull);
    rc = VFSManagerMakePath(self, &path, "%s", in);
    if (rc == 0)
        rc = VFSManagerResolveVPathAll(self, path, local, remote, cache);
    RELEASE(VPath, path);
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerResolveVPathWithCache(const VFSManager * self,
    const VPath * in, const VPath ** out, const VPath ** cache)
{
    rc_t rc = 0;
    VResolver * resolver = NULL;
    const VPath * local = NULL;
    const VPath * remote = NULL;
    if (out == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcParam, rcNull);
    *out = NULL;
    rc = VFSManagerGetResolver(self, &resolver);
    if (rc == 0)
        rc = VResolverQuery(resolver, 0, in, &local, &remote, cache);
    if (rc == 0) {
        if (local != NULL) {
            *out = local;
            RELEASE(VPath, remote);
        }
        else if (remote != NULL) {
            *out = remote;
            RELEASE(VPath, local);
        }
    }
    RELEASE(VResolver, resolver);
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerResolveWithCache(const VFSManager * self,
    const char * in, const VPath ** out, const VPath ** cache)
{
    rc_t rc = 0;
    VPath * path = NULL;
    if (in == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcParam, rcNull);
    rc = VFSManagerMakePath(self, &path, "%s", in);
    if (rc == 0)
        rc = VFSManagerResolveVPathWithCache(self, path, out, cache);
    RELEASE(VPath, path);
    return rc;
}

LIB_EXPORT rc_t CC VFSManagerResolveVPath(const VFSManager * self,
    const struct VPath * in, const struct VPath ** out)
{
    return VFSManagerResolveVPathWithCache(self, in, out, NULL);
}
