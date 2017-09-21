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


#include "path-priv.h" /* EVPathInitError */
#include "services-priv.h" /* KServiceGetResolver */
#include <kfg/config.h> /* KConfigRelease */
#include <klib/container.h> /* BSTree */
#include <klib/rc.h> /* RC */
#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path.h> /* VFSManagerMakePath */
#include <vfs/services-priv.h> /* KServiceNamesExecuteExt */


#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)


typedef struct {
    BSTNode n;
    const String * ticket;
    const VResolver * resolver;
} BSTItem;

static void BSTItemWhack ( BSTNode * n, void * ignore ) {
    BSTItem * i = ( BSTItem * ) n;

    assert ( i );

    free ( ( void * ) i -> ticket );
    VResolverRelease ( i -> resolver );

    memset ( i, 0, sizeof * i );
}

static int64_t CC BSTItemCmp ( const void * item, const BSTNode * n ) {
    const String * s = item;
    const BSTItem * i = ( BSTItem * ) n;
 
    assert ( s && i );
 
    return string_cmp ( s -> addr, s -> size,
        i -> ticket -> addr, i -> ticket -> size, s -> size );
}

static int64_t CC BSTreeSort ( const BSTNode * item, const BSTNode * n ) {
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
                        const VResolver ** resolver )
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
            if ( rc == 0 ) {
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

        * resolver = i -> resolver;
    }
    else {
        if ( self -> resolver == NULL )
            rc = VFSManagerMakeResolver ( self -> mgr, & self -> resolver,
                                          self -> kfg );

        * resolver = self -> resolver;
    }

    return rc;
}


static rc_t VResolversQuery ( const VResolver * self, const VFSManager * mgr,
    VRemoteProtocols protocols, const String * acc, VPathSet ** result )
{
    rc_t rc = 0;

    VPath * query = NULL;

    uint32_t oid = 0;
    uint32_t i = 0;

    assert ( result );

    for ( i = 0; i < acc -> size; ++i ) {
        char c = acc -> addr [ i ];
        if ( c < '0' || c > '9' ) {
            oid = 0;
            break;
        }
        oid = oid * 10 + c - '0';
    }

    if ( oid == 0 )
        rc = VFSManagerMakePath ( mgr, & query, "%S", acc );
    else
        rc = VFSManagerMakeOidPath ( mgr, & query, oid );

    if ( rc == 0 ) {
        const VPath * local = NULL;
        const VPath * cache = NULL;

        rc_t localRc = 0;
        rc_t cacheRc = 0;

        localRc = VResolverQuery ( self, protocols, query,
                                   & local, NULL, NULL );
        cacheRc = VResolverQuery ( self, protocols, query,
                                   NULL, NULL, & cache );

        VPathSetMakeQuery ( result, local, localRc, cache, cacheRc );

        RELEASE ( VPath, local );
        RELEASE ( VPath, cache );
    }

    RELEASE ( VPath, query );

    return rc;
}

rc_t KServiceNamesQueryExt ( KService * self, VRemoteProtocols protocols, 
                             const char * cgi, const char * version,
                             const KSrvResponse ** aResponse )
{
    rc_t rc = 0;
    KSrvResponse * response = NULL;
    if ( aResponse == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    * aResponse = NULL;

    {
        const KSrvResponse * r = NULL;
        rc = KServiceNamesExecuteExt ( self, protocols, cgi,
                                       version, & r );
        if ( rc == 0 )
            response = ( KSrvResponse* ) r;
    }

    if ( rc == 0 ) {
        H h;
        rc = HInit ( & h, self );
        {
            uint32_t i = 0;
            uint32_t n = KSrvResponseLength  ( response );
            for ( i = 0; rc == 0 && i < n; ++ i ) {
                VPathSet * vps = NULL;
                const VPath * path = NULL;
                const KSrvError * error = NULL;
                rc = KSrvResponseGetPath
                    ( response, i, protocols, & path, NULL, & error );
                if ( error == NULL && rc == 0 ) {
                    const VResolver * resolver = NULL;
                    String id;
                    String ticket;
                    rc = VPathGetId ( path, & id );
                    if ( rc == 0 )
                        rc = VPathGetTicket ( path, & ticket );
                    if ( rc == 0 )
                        rc = HResolver ( & h, & ticket, & resolver );
                    if ( rc == 0 ) {
                        assert ( resolver );
                        rc = VResolversQuery ( resolver, h . mgr,
                                                protocols, & id, & vps );
                    }
                }
                if ( vps != NULL ) {
                    rc = KSrvResponseAddLocalAndCache ( response, i, vps );
                    RELEASE ( VPathSet, vps );
                }
                RELEASE ( VPath, path );
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

rc_t KServiceNamesQuery ( KService * self, VRemoteProtocols protocols,
                          const KSrvResponse ** aResponse )
{   return KServiceNamesQueryExt ( self, protocols, NULL, NULL, aResponse ); }
