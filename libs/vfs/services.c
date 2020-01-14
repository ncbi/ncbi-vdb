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

#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path.h> /* VFSManagerMakePath */
#include <vfs/resolver-priv.h> /* VResolverQueryWithDir */
#include <vfs/services-priv.h> /* KServiceNamesExecuteExt */

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include "path-priv.h" /* EVPathInitError */
#include "resolver-priv.h" /* VResolverResolveName */
#include "services-priv.h" /* KServiceGetResolver */

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
        i -> ticket -> addr, i -> ticket -> size, s -> size );
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
    const VFSManager * mgr;
    const KConfig * kfg;
    VResolver * resolver;
    BSTree ticketsToResolvers;
} H;

static rc_t HInit ( H * self, KService * s ) {
    rc_t rc = 0;

    assert ( self && s );

    memset ( self, 0, sizeof * self );

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

/*  if (rc == 0 && *resolver != NULL)
        VResolverCacheEnable(*resolver, KServiceGetCacheEnable(service)); */

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

static rc_t VPathCacheLocationForSource(
    const VPath * self, const KDirectory * dir, VPath ** path, rc_t *rcOut)
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

    if (rc == 0 && acc.size > 0 && name.size > 0)
        /* acc & name (come from SDL) are known */
        rc = KDirectoryResolvePath(dir, true, sPath, sizeof sPath,
            "%.*s/%.*s", acc.size, acc.addr, name.size, name.addr);

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

static rc_t VResolversQuery ( const VResolver * self, const VFSManager * mgr,
    VRemoteProtocols protocols, const VPath * path, const String * acc,
    uint64_t id, VPathSet ** result, ESrvFileFormat ff,
    const char * outDir, const char * outFile, const VPath * mapping )
{
    rc_t rc = 0;

    VPath * query = NULL;

    uint32_t oid = 0;
    uint32_t i = 0;

    assert ( self && result );

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
        rc = VFSManagerMakePath ( mgr, & query, "%S", acc );
        if (rc == 0 && path != NULL && path->projectId >= 0) {
            assert(query);
            query->projectId = path->projectId;
        }
    }
    else
        rc = VFSManagerMakeOidPath ( mgr, & query, oid );

    if ( rc == 0 ) {
        bool isSource = false;

        const VPath * local = NULL;
        const VPath * cache = NULL;

        rc_t localRc = 0;
        rc_t cacheRc = 0;

        String srapub_files;
        String remote;

        CONST_STRING(&srapub_files, "srapub_files");
        CONST_STRING(&remote      , "remote"      );

        isSource = StringEqual(&path->objectType, &srapub_files);
        if ( ! isSource )
            isSource = StringEqual(&path->objectType, &remote);

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
                cacheRc = VResolverQueryWithDir(self, protocols, query, NULL,
                    NULL, &cache, false, outDir, NULL, true, path, mapping);
                localRc = VResolverQueryWithDir(self, protocols, query, &local,
                    NULL, NULL, false, outDir, NULL, true, path, mapping);
            }
        }

        else {
            cacheRc = VResolverQuery ( self, protocols, query,
                                    NULL, NULL, & cache );
            localRc = VResolverQuery ( self, protocols, query,
                                    & local, NULL, NULL );
        }

        VPathSetMakeQuery ( result, local, localRc, cache, cacheRc );

        RELEASE ( VPath, local );
        RELEASE ( VPath, cache );
    }

    RELEASE ( VPath, query );

    return rc;
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

    assert ( newId && oldId );

    * newId = NULL;

    if ( oldId -> addr == NULL )
        return 0;

    CONST_STRING ( & fasp , "fasp://"  );
    CONST_STRING ( & http , "http://"  );
    CONST_STRING ( & https, "https://" );

    if ( oldId -> size <= https . size )
        return 0;

    /* what is being attempted with this code? */
    if ( string_cmp ( oldId -> addr, oldId -> size, https . addr, https. size, https. size ) != 0 &&
         string_cmp ( oldId -> addr, oldId -> size, fasp  . addr, fasp . size, fasp . size ) != 0 &&
         string_cmp ( oldId -> addr, oldId -> size, http  . addr, fasp . size, fasp . size ) != 0 )
    {   return 0; }

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
    }

    if ( rc == 0 )
        rc = VPathGetPath ( acc_or_oid, & id );

    if ( rc == 0 )
        rc = StringCopy ( newId, & id );

    RELEASE ( VPath, acc_or_oid );

    if ( rc == 0 )
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

static
rc_t KServiceNamesQueryExtImpl ( KService * self, VRemoteProtocols protocols, 
    const char * cgi, const char * version, const KSrvResponse ** aResponse,
    const char * outDir, const char * outFile, const char * expected )
{
    rc_t rc = 0;
    KSrvResponse * response = NULL;
    if ( aResponse == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    * aResponse = NULL;

    {
        const KSrvResponse * r = NULL;
        rc = KServiceNamesExecuteExtImpl ( self, protocols, cgi,
                                       version, & r, expected );
        if ( rc == 0 )
            response = ( KSrvResponse* ) r;
    }
    
    if ( rc == 0 ) {
        H h;
        rc = HInit ( & h, self );

        if ( protocols == eProtocolDefault )
             protocols = DEFAULT_PROTOCOLS;
        
        {
            uint32_t i = 0;
            uint32_t n = KSrvResponseLength  ( response );
            for ( i = 0; rc == 0 && i < n; ++ i ) {
                const KSrvRespObj * obj = NULL;
                VPathSet * vps = NULL;
                const VPath * path = NULL;
                const KSrvError * error = NULL;
                rc = KSrvResponseGetObjByIdx ( response, i, & obj );
                if ( rc == 0 ) {
                    rc_t rx = 0;
                    rc = KSrvRespObjGetError(obj, &rx, NULL, NULL);
                    if (rc == 0 && rx == 0) {
                        KSrvRespObjIterator * it = NULL;
                        rc = KSrvRespObjMakeIterator(obj, &it);
                        while (rc == 0) {
                            KSrvRespFile * file = NULL;
                            rc = KSrvRespObjIteratorNextFile(it, &file);
                            if (rc != 0 || file == NULL)
                                break;
                            else {
                                ESrvFileFormat ff = eSFFInvalid;
                                KSrvRespFileIterator * fi = NULL;
                                const char * acc = NULL;
                                const char * tic = NULL;
                                uint64_t iid = 0;
                                const VPath * mapping = NULL;
                                String id;
                                memset(&id, 0, sizeof id);
                                rc = KSrvRespFileGetAccOrName(file, &acc, &tic);
                                if (rc == 0) {
                                    if (acc != NULL) {
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
                                        if (error == NULL) {
                                            VResolver * resolver = NULL;
                                            const String * pId = NULL;
                                            String ticket;
                                            memset(&ticket,
                                                0, sizeof ticket);
                                            if (rc == 0 && tic != NULL)
                                                StringInitCString(&ticket,
                                                    tic);
                                            if (rc == 0)
                                                rc = HResolver(&h, self,
                                                    &ticket, &resolver, path);
                                            if (rc == 0)
                                                rc = _VPathGetId(path, &pId,
                                                    &id, h.mgr);
                                            if (rc == 0) {
                                                assert(resolver);
                                                VResolverResolveName(resolver,
                                                    KServiceGetResolveName(
                                                        self));
                                                rc = VResolversQuery(resolver,
                                                    h.mgr, protocols, path,
                                                    &id, iid, &vps, ff,
                                                    outDir, outFile, mapping);
                                            }
                                            free((void *)pId);
                                        }
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
                                    protocols, path, & id, 0, & vps,
                                    eSFFInvalid, outDir, outFile, NULL );
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

    return rc;
}

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
