/*===========================================================================
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

#include <klib/container.h> /* BSTree */
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
    VFSManager * mgr;
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
        rc = VFSManagerMake ( & self -> mgr );

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

static rc_t HResolver ( H * self, const String * ticket,
                        VResolver ** resolver )
{
    rc_t rc = 0;

    assert ( self && resolver );

    if ( ticket && ticket -> addr && ticket -> size ) {
        BSTItem * i = ( BSTItem * ) BSTreeFind
            ( & self -> ticketsToResolvers, ticket, BSTItemCmp );

        if ( i != NULL )
            * resolver = i -> resolver;
        else {
            VResolver * resolver = NULL;
            rc = KServiceGetResolver ( self -> service, ticket, & resolver );
            if ( rc != 0 )
                return rc;
            else if ( resolver != NULL ) {
                i = calloc ( 1, sizeof * i );
                if ( i == NULL )
                    return RC (
                        rcVFS, rcStorage, rcAllocating, rcMemory, rcExhausted );

                rc = StringCopy ( & i -> ticket, ticket );
                if ( rc != 0 )
                    return rc;

                i -> resolver = resolver;
                rc = BSTreeInsert ( & self -> ticketsToResolvers,
                    ( BSTNode * ) i, BSTreeSort );
            }
        }

        assert ( i );
        
        if ( i -> resolver != NULL )
            * resolver = i -> resolver;

        return rc;
    }

    if ( self -> resolver == NULL )
        rc = VFSManagerMakeResolver ( self -> mgr, & self -> resolver,
                                        self -> kfg );

    * resolver = self -> resolver;

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

static rc_t VResolversQuery ( const VResolver * self, const VFSManager * mgr,
    VRemoteProtocols protocols, const VPath * path, const String * acc,
    uint64_t id, VPathSet ** result, ESrvFileFormat ff,
    const char * outDir, const char * outFile, const VPath * mapping )
{
    rc_t rc = 0;

    VPath * query = NULL;

    uint32_t oid = 0;
    uint32_t i = 0;

    assert ( result );

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

    if ( oid == 0 )
        rc = VFSManagerMakePath ( mgr, & query, "%S", acc );
    else
        rc = VFSManagerMakeOidPath ( mgr, & query, oid );

    if ( rc == 0 ) {
        const VPath * local = NULL;
        const VPath * cache = NULL;

        rc_t localRc = 0;
        rc_t cacheRc = 0;

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
            cacheRc = VResolverQueryWithDir ( self, protocols, query,
                NULL, NULL, & cache, false, outDir, NULL, true, path, mapping);
            localRc = VResolverQueryWithDir ( self, protocols, query,
                & local, NULL, NULL, false, outDir, NULL, true, path, mapping);
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

    if ( ! string_cmp ( oldId -> addr, oldId -> size, https . addr, https. size,
                      https. size ) == 0  &&
         ! string_cmp ( oldId -> addr, oldId -> size, fasp  . addr, fasp . size,
                        fasp . size ) == 0 &&
         ! string_cmp ( oldId -> addr, oldId -> size, http  . addr, fasp . size,
                        fasp . size ) == 0 )
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
        VFSManager * mgr = NULL;
        rc = HInit ( & h, self );

        if ( rc == 0 ) {
            rc = VFSManagerMake ( & mgr );
            if ( rc != 0 )
                return rc;
        }

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
                                                rc = HResolver(&h,
                                                    &ticket, &resolver);
                                            if (rc == 0)
                                                rc = _VPathGetId(path, &pId,
                                                    &id, mgr);
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
                                    if (vps != NULL) {
                                        rc = KSrvRespFileAddLocalAndCache
                                        (file, vps);
                                        RELEASE(VPathSet, vps);
                                    }
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
                                rc = VFSManagerExtractAccessionOrOID ( mgr,
                                        & acc_or_oid, path );
                                if ( rc == 0 )
                                    rc = VPathGetPath ( acc_or_oid, & id );
                            }
                            if ( rc == 0 )
                                rc = VPathGetTicket ( path, & ticket );
                            if ( rc == 0 )
                                rc = HResolver ( & h, & ticket, & resolver );
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

        RELEASE ( VFSManager, mgr );
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
    const char * version, const KSrvResponse ** response,
    const char * dir, const char * file, const char * expected )
{
    return KServiceNamesQueryExtImpl
        ( self, protocols, NULL, version, response, dir, file, expected );
}
