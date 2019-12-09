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
 * ===========================================================================
 *
 */

#include <klib/container.h> /* BSTree */
#include <klib/log.h> /* PLOGERR */
#include <klib/rc.h> /* RC */
#include <klib/vector.h> /* Vector */

#include <vfs/services.h> /* KSrvResponse */

#include "json-response.h" /* struct Response4 */
#include "path-priv.h" /* VPathGetScheme_t */
#include "resolver-priv.h" /* DEFAULT_PROTOCOLS */
#include "services-priv.h" /* KSrvResponseGetMapping */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)


struct VPathSet {
    atomic32_t refcount;

    char * reqId;
    char * respId;

    const VPath * fasp;
    const VPath * file;
    const VPath * http;
    const VPath * https;
    const VPath * s3;
    uint64_t osize; /*size of VPath object */

    /* vdbcache */
    const VPath * cacheFasp;
    const VPath * cacheFile;
    const VPath * cacheHttp;
    const VPath * cacheHttps;
    const VPath * cacheS3;

    const struct KSrvError * error;

    const VPath * mapping;
    const VPath * cacheMapping; /* vdbcache */

    const VPath * local;
    const VPath * cache;
    /* rc code after call to VResolverQuery(&local,&cache) */
    rc_t          localRc;
    rc_t          cacheRc;
};

struct KSrvResponse {
    atomic32_t refcount;

    Vector list;

    Response4 * r4;

    BSTree locations;
};


/* VPathSet */
rc_t VPathSetAddRef ( const VPathSet * self ) {
    if ( self != NULL )
        atomic32_inc ( & ( ( VPathSet * ) self ) -> refcount );

    return 0;
}

rc_t VPathSetWhack ( VPathSet * self ) {
    rc_t rc = 0;

    if ( self != NULL ) {
        RELEASE ( VPath, self -> fasp );
        RELEASE ( VPath, self -> file );
        RELEASE ( VPath, self -> http );
        RELEASE ( VPath, self -> https );
        RELEASE ( VPath, self -> s3 );
        RELEASE ( VPath, self -> cacheFasp );
        RELEASE ( VPath, self -> cacheFile );
        RELEASE ( VPath, self -> cacheHttp );
        RELEASE ( VPath, self -> cacheHttps );
        RELEASE ( VPath, self -> cacheS3 );

        RELEASE ( VPath, self -> mapping );
        RELEASE ( VPath, self -> cacheMapping );

        RELEASE ( VPath, self -> local );
        RELEASE ( VPath, self -> cache );

        RELEASE ( KSrvError, self -> error );

        free ( self -> reqId  );    self -> reqId  = NULL;
        free ( self -> respId );    self -> respId = NULL;

        free ( self );
    }

    return rc;
}

static void whackVPathSet  ( void * self, void * ignore ) {
    VPathSetWhack ( ( VPathSet * ) self);
}

rc_t VPathSetRelease ( const VPathSet * cself ) {
    VPathSet * self = ( VPathSet * ) cself;

    if ( self != NULL && atomic32_dec_and_test ( & self -> refcount ) )
        return VPathSetWhack ( self );

    return 0;
}

rc_t VPathSetGet ( const VPathSet * self, VRemoteProtocols protocols,
    const VPath ** path, const VPath ** vdbcache )
{
    rc_t rc = 0;
    VRemoteProtocols protocol = protocols;
    const VPath * p = NULL;
    const VPath * c = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );
    if ( protocols == eProtocolDefault )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcInvalid );
    if ( self -> error != NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcError, rcExists );

    for ( ; protocol != 0; protocol >>= 3 ) {
        switch ( protocol & eProtocolMask ) {
            case eProtocolFasp:
                p = self -> fasp;
                c = self -> cacheFasp;
                break;
            case eProtocolFile:
                p = self -> file;
                c = self -> cacheFile;
                break;
            case eProtocolGS:
                p = c = NULL; /* not implemented y */
                break;
            case eProtocolHttp:
                p = self -> http;
                c = self -> cacheHttp;
                break;
            case eProtocolHttps:
                p = self -> https;
                c = self -> cacheHttps;
                break;
            case eProtocolS3:
                p = self -> s3;
                c = self -> cacheS3;
                break;
            default:
                return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcInvalid );
        }

        if ( p != NULL || c != NULL ) {
            if ( path != NULL ) {
                rc = VPathAddRef ( p );
                if ( rc == 0 )
                    * path = p;
            }

            if ( vdbcache != NULL ) {
                rc_t r2 = VPathAddRef ( c );
                if ( r2 == 0 )
                    * vdbcache = c;
                else if ( rc == 0)
                    rc = r2;
            }

            return rc;
        }
    }

    return 0;
}

rc_t VPathSetGetLocal ( const VPathSet * self, const VPath ** path )
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );
    if ( self -> error != NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcError, rcExists );
    if ( path == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * path = NULL;

    if ( self -> localRc != 0 )
        return self -> localRc;

    rc = VPathAddRef ( self -> local );
    if ( rc == 0 )
        * path = self -> local;

    return rc;
}

rc_t VPathSetGetCache ( const VPathSet * self, const VPath ** path )
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( self -> error != NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcError, rcExists );

    if ( path == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );

    * path = NULL;

    if ( self -> cacheRc != 0 )
        return self -> cacheRc;

    rc = VPathAddRef ( self -> cache );

    if ( rc == 0 )
        * path = self -> cache;

    return rc;
}

rc_t VPathSetMake ( VPathSet ** self, const EVPath * src,
                    bool singleUrl )
{
    VPathSet * p = NULL;
    rc_t rc = 0;
    rc_t r2 = 0;

    assert ( self && src );

    p = ( VPathSet * ) calloc ( 1, sizeof * p );
    if ( p == NULL )
        return RC ( rcVFS, rcPath, rcAllocating, rcMemory, rcExhausted );

    if ( src -> error != NULL ) {
        rc = KSrvErrorAddRef ( src -> error );
        if ( rc == 0 )
            p -> error = src -> error;
    }

    else if ( singleUrl ) {
        VPUri_t uri_type = vpuri_invalid;
        rc = VPathGetScheme_t ( src -> http, & uri_type );
        if ( rc == 0 ) {
            const VPath ** d = NULL;
            switch ( uri_type ) {
                case vpuri_fasp:
                    d = & p -> fasp;
                    break;
                case vpuri_file:
                    d = & p -> file;
                    break;
                case vpuri_http:
                    d = & p -> http;
                    break;
                case vpuri_https:
                    d = & p -> https;
                    break;
                default:
                    assert ( 0 );
                    return RC (
                        rcVFS, rcPath,  rcConstructing, rcParam, rcIncorrect );
            }

            r2 = VPathAddRef ( src -> http );
            if ( r2 == 0 )
                * d = src -> http;
            else if ( rc == 0 )
                rc = r2;
        }
    }
    else {
        r2 = VPathAddRef ( src -> fasp );
        if ( r2 == 0 )
            p -> fasp = src -> fasp;
        else if ( rc == 0 )
            rc = r2;
        r2 = VPathAddRef ( src -> vcFasp );
        if ( r2 == 0 )
            p -> cacheFasp = src -> vcFasp;
        else if ( rc == 0 )
            rc = r2;

        r2 = VPathAddRef ( src -> file );
        if ( r2 == 0 )
            p -> file = src -> file;
        else if ( rc == 0 )
            rc = r2;
        r2 = VPathAddRef ( src -> vcFile );
        if ( r2 == 0 )
            p -> cacheFile = src -> vcFile;
        else if ( rc == 0 )
            rc = r2;

        r2 = VPathAddRef ( src -> http );
        if ( r2 == 0 )
            p -> http = src -> http;
        else if ( rc == 0 )
            rc = r2;
        r2 = VPathAddRef ( src -> vcHttp );
        if ( r2 == 0 )
            p -> cacheHttp = src -> vcHttp;
        else if ( rc == 0 )
            rc = r2;

        r2 = VPathAddRef ( src -> https );
        if ( r2 == 0 )
            p -> https = src -> https;
        else if ( rc == 0 )
            rc = r2;
        r2 = VPathAddRef ( src -> vcHttps );
        if ( r2 == 0 )
            p -> cacheHttps = src -> vcHttps;
        else if ( rc == 0 )
            rc = r2;

        r2 = VPathAddRef ( src -> s3 );
        if ( r2 == 0 )
            p -> s3 = src -> s3;
        else if ( rc == 0 )
            rc = r2;
        r2 = VPathAddRef ( src -> vcS3 );
        if ( r2 == 0 )
            p -> cacheS3 = src -> vcS3;
        else if ( rc == 0 )
            rc = r2;

        r2 = VPathAddRef(src->mapping);
        if (r2 == 0)
            p->mapping = src->mapping;
        else if (rc == 0)
            rc = r2;

        r2 = VPathAddRef(src->vcMapping);
        if (r2 == 0)
            p->cacheMapping = src->vcMapping;
        else if (rc == 0)
            rc = r2;
    }

    if ( rc == 0 ) {
        if ( src -> reqId != NULL ) {
            p -> reqId = string_dup_measure ( src -> reqId, NULL );
            if ( p -> reqId == NULL )
                rc = RC ( rcVFS, rcPath, rcAllocating, rcMemory, rcExhausted );
        }
        else if ( p -> error != NULL ) {
            String message;
            rc = KSrvErrorMessage ( p -> error, & message );
            if ( rc == 0 ) {
                p -> reqId = string_dup ( message. addr, message. size );
                if ( p -> reqId == NULL )
                    rc = RC ( rcVFS, rcPath, rcAllocating,
                                     rcMemory, rcExhausted );
            }
        }

        if ( src -> respId != NULL ) {
            p -> respId = string_dup_measure ( src -> respId, NULL );
            if ( p -> respId == NULL )
                rc = RC ( rcVFS, rcPath, rcAllocating, rcMemory, rcExhausted );
        }
        else if ( p -> error != NULL ) {
            String message;
            rc = KSrvErrorMessage ( p -> error, & message );
            if ( rc == 0 ) {
                p -> respId = string_dup ( message. addr, message. size );
                if ( p -> respId == NULL )
                    rc = RC ( rcVFS, rcPath, rcAllocating,
                                     rcMemory, rcExhausted );
            }
        }

        p->osize = src->osize;
    }

    if ( rc == 0 ) {
        atomic32_set ( & p -> refcount, 1 );

        * self = p;
    }
    else
        VPathSetWhack ( p );

    return rc;
}

rc_t VPathSetMakeQuery ( VPathSet ** self, const VPath * local, rc_t localRc,
                         const VPath * cache, rc_t cacheRc )
{
    rc_t rc = 0;

    VPathSet * p = NULL;

    assert ( self );

    p = ( VPathSet * ) calloc ( 1, sizeof * p );
    if ( p == NULL )
        return RC ( rcVFS, rcPath, rcAllocating, rcMemory, rcExhausted );

    if ( localRc == 0 ) {
        rc = VPathAddRef ( local );
        if ( rc == 0 )
            p -> local = local;
    }
    else
        p -> localRc = localRc;

    if ( cacheRc == 0 ) {
        rc = VPathAddRef ( cache );
        if ( rc == 0 )
            p -> cache = cache;
    }
    else
        p -> cacheRc = cacheRc;

    if ( rc == 0 ) {
        atomic32_set ( & p -> refcount, 1 );

        * self = p;
    }
    else
        VPathSetWhack ( p );

    return rc;
}

typedef struct {
    const String * acc;
    const String * name;

    const KSrvRespFile * file;
} LocalAndCache;

static int LocalAndCacheCmp(const LocalAndCache * lhs,
    const LocalAndCache * rhs)
{
    int c = 0;

    assert(lhs && rhs);

    c = StringCompare(lhs->acc, rhs->acc);

    if (c == 0)
        c = StringCompare(lhs->name, rhs->name);

    return c;
}

static rc_t LocalAndCacheFini(LocalAndCache * self) {
    rc_t rc = 0;

    assert(self);

    StringWhack(self->acc);
    StringWhack(self->name);

    rc = KSrvRespFileRelease(self->file);

    memset(self, 0, sizeof *self);

    return rc;
}

static rc_t LocalAndCacheRelease(LocalAndCache * self) {
    if (self != NULL) {
        LocalAndCacheFini(self);
        free(self);
    }

    return 0;
}

typedef struct {
    BSTNode n;
    LocalAndCache * lnc;
} BSTItem;

static int64_t CC BSTItemCmp(const void * item, const BSTNode * n) {
    const LocalAndCache * lnc = item;

    const BSTItem * i = (BSTItem *)n;

    assert(i);

    return LocalAndCacheCmp(lnc, i->lnc);
}

static void BSTItemWhack(BSTNode * n, void * ignore) {
    BSTItem * i = (BSTItem *)n;

    assert(i);

    LocalAndCacheRelease(i->lnc);

    memset(i, 0, sizeof * i);

    free(i);
}

static int64_t CC BSTreeSort(const BSTNode * item, const BSTNode * n) {
    const BSTItem * i = (BSTItem *)item;

    assert(i);

    return BSTItemCmp(i->lnc, n);
}

static rc_t LocalAndCacheInit(LocalAndCache * self,
    const char * acc, const char * name)
{
    rc_t rc = 0;

    String tmp;

    assert(self);

    memset(self, 0, sizeof(*self));

    if (acc != NULL) {
        StringInitCString(&tmp, acc);
        rc = StringCopy(&self->acc, &tmp);
    }

    if (rc == 0 && name != NULL) {
        StringInitCString(&tmp, name);
        rc = StringCopy(&self->name, &tmp);
    }

    if (rc != 0)
        LocalAndCacheFini(self);

    return rc;
}

rc_t KSrvResponseAddLocalAndCacheToTree(
    KSrvResponse * self, const KSrvRespFile * file)
{
    rc_t rc = 0;

    const char * acc = NULL;
    const char * name = NULL;
    LocalAndCache * lnc = NULL;
    String tmp;

    assert(self);

    lnc = calloc(1, sizeof * lnc);
    if (lnc == NULL)
        rc = RC(rcVFS, rcStorage, rcAllocating, rcMemory, rcExhausted);

    if (rc == 0)
        rc = KSrvRespFileGetAccOrId(file, &acc, NULL);
    if (rc == 0)
        KSrvRespFileGetAccOrName(file, &name, NULL);

    if (rc == 0 && acc != NULL) {
        StringInitCString(&tmp, acc);
        rc = StringCopy(&lnc->acc, &tmp);
    }
    if (rc == 0 && name != NULL) {
        StringInitCString(&tmp, name);
        rc = StringCopy(&lnc->name, &tmp);
    }

    if (rc == 0) {
        rc = KSrvRespFileAddRef(file);
        if (rc == 0)
            lnc->file = file;
    }

    if (rc == 0) {
        BSTItem * i = (BSTItem *)BSTreeFind(&self->locations, lnc, BSTItemCmp);
        if (i != NULL) {
            BSTreeWhack(&self->locations, BSTItemWhack, NULL);
            PLOGERR(klogFatal, (klogFatal,
                RC(rcVFS, rcQuery, rcExecuting, rcString, rcUnexpected),
                "duplicate names in the same bundle: "
                "'$acc'/'$(name)'", "acc=%c,name=%c", acc, name));
            RELEASE(LocalAndCache, lnc);
        }
        else {
            i = calloc(1, sizeof * i);
            if (i == NULL)
                rc = RC(rcVFS, rcStorage, rcAllocating, rcMemory, rcExhausted);
            else {
                i->lnc = lnc;
                rc = BSTreeInsert(&self->locations, (BSTNode *)i, BSTreeSort);
            }
        }
    }

    if (rc != 0)
        LocalAndCacheRelease(lnc);

    return rc;
}

/* KSrvResponse */

rc_t KSrvResponseGetLocation(const KSrvResponse * self,
    const char * acc, const char * name,
    const struct VPath ** local, rc_t * localRc,
    const struct VPath ** cache, rc_t * cacheRc)
{
    rc_t rc = 0;

    LocalAndCache lnc;

    if (local == NULL && localRc != NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcParam, rcNull);
    if (cache == NULL && cacheRc != NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcParam, rcNull);

    if (local != NULL)
        * local = NULL;
    if (localRc != NULL)
        * localRc = 0;
    if (cache != NULL)
        * cache = NULL;
    if (cacheRc != NULL)
        * cacheRc = 0;

    if (self == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcSelf, rcNull);

    rc = LocalAndCacheInit(&lnc, acc, name);
    if (rc == 0) {
        BSTItem * i = (BSTItem *)BSTreeFind(&self->locations, &lnc, BSTItemCmp);
        if (i == NULL)
            rc = RC(rcVFS, rcQuery, rcResolving, rcName, rcNotFound);
        else {
            if (local != NULL) {
                const KSrvRespFile * f = i->lnc->file;
                rc_t rc = KSrvRespFileGetLocal(f, local);
                if (localRc != NULL)
                    *localRc = rc;
            }

            if (cache != NULL) {
                const KSrvRespFile * f = i->lnc->file;
                rc_t rc = KSrvRespFileGetCache(f, cache);
                if (cacheRc != NULL)
                    *cacheRc = rc;
            }
        }
        LocalAndCacheFini(&lnc);
    }

    return rc;
}

rc_t KSrvResponseGetR4 ( const KSrvResponse * self, Response4 ** r ) {

    rc_t rc = 0;

    assert ( self && r );

    rc = Response4AddRef ( self -> r4 );
    if ( rc == 0 )
        * r = self -> r4;

    return rc;
}

rc_t KSrvResponseSetR4 ( KSrvResponse * self, Response4 * r )
{
    rc_t rc = 0;

    assert ( self );

    if (self->r4 == r)
        return 0;

    rc = Response4Release ( self -> r4 );

    rc = Response4AddRef ( r );
    if ( rc == 0 )
        self -> r4 = r;

    return rc;
}

rc_t KSrvResponseMake ( KSrvResponse ** self ) {
    KSrvResponse * p = ( KSrvResponse * ) calloc ( 1, sizeof * p );
    if ( p == NULL )
        return RC ( rcVFS, rcPath, rcAllocating, rcMemory, rcExhausted );

    atomic32_set ( & p -> refcount, 1 );

    assert ( self );

    * self = p;

    return 0;
}

rc_t KSrvResponseAddRef ( const KSrvResponse * self ) {
    if ( self != NULL )
        atomic32_inc ( & ( ( KSrvResponse * ) self ) -> refcount );

    return 0;
}

rc_t KSrvResponseRelease ( const KSrvResponse * cself ) {
    rc_t rc = 0;

    KSrvResponse * self = ( KSrvResponse * ) cself;

    if ( self != NULL && atomic32_dec_and_test ( & self -> refcount ) ) {
        VectorWhack ( & self -> list, whackVPathSet, NULL );

        RELEASE ( Response4, self -> r4 );

        BSTreeWhack(&self->locations, BSTItemWhack, NULL);

        memset ( self, 0, sizeof * self );
        free ( self );
    }

    return rc;
}

rc_t KSrvResponseAppend ( KSrvResponse * self, const VPathSet * set ) {
    rc_t rc = 0;

    assert ( self );

    rc = VPathSetAddRef ( set );

    if ( rc == 0 )
        rc = VectorAppend ( & self -> list, NULL, set );

    return rc;
}

rc_t KSrvResponseGetIds ( const KSrvResponse * self, uint32_t idx,
                          const char ** reqId, const char ** respId )
{
    VPathSet * s = NULL;

    assert ( self && reqId && respId );

    s = ( VPathSet * ) VectorGet ( & self -> list, idx );
    if ( s == NULL )
        return RC ( rcVFS, rcPath, rcAccessing, rcItem, rcNotFound );

    * reqId  = s -> reqId ;
    * respId = s -> respId;

    return 0;
}

rc_t KSrvResponseAddLocalAndCache ( KSrvResponse * self, uint32_t idx,
                                    const VPathSet * localAndCache )
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( localAndCache == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    else {
        VPathSet * s = ( VPathSet * ) VectorGet ( & self -> list, idx );
        if ( s == NULL ) {
            if ( self -> r4 == NULL )
                return RC ( rcVFS, rcPath, rcAccessing, rcItem, rcNotFound );
            else {
                s = calloc ( 1, sizeof * s );
                if ( s == NULL )
                    return RC
                        ( rcVFS, rcPath, rcAllocating, rcMemory,  rcExhausted );
                rc = VectorSet ( & self -> list, idx, s );
                if ( rc != 0 )
                    return rc;
            }
        }
        if ( s != NULL ) {
            RELEASE ( VPath, s -> local );
            if ( rc == 0 ) {
                if ( localAndCache -> localRc == 0 ) {
                    rc = VPathAddRef ( localAndCache -> local );
                    if ( rc == 0 )
                        s -> local = localAndCache -> local;
                }
                else
                    s -> localRc = localAndCache -> localRc;
            }
            RELEASE ( VPath, s -> cache );
            if ( rc == 0 ) {
                if ( localAndCache -> cacheRc == 0 ) {
                    rc = VPathAddRef ( localAndCache -> cache );
                    if ( rc == 0 )
                        s -> cache = localAndCache -> cache;
                }
                else
                    s -> cacheRc = localAndCache -> cacheRc;
            }
            return rc;
        }
        assert ( 0 );
        return 1;
    }
}

uint32_t KSrvResponseLength ( const KSrvResponse * self ) {
    if ( self == NULL )
         return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( self -> r4 != NULL ) {
        uint32_t l = 0;
        return Response4GetKSrvRespObjCount ( self -> r4, & l ) == 0 ? l : 0;
    }

    return VectorLength ( & self -> list );
}

rc_t KSrvResponseGetNextToken(const KSrvResponse * self,
    const char ** nextToken)
{
    if (self == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcSelf, rcNull);
    else
        return Response4GetNextToken(self->r4, nextToken);
}

rc_t KSrvResponseGetObjByIdx ( const KSrvResponse * self, uint32_t idx,
                               const KSrvRespObj ** box )
{
    if ( self == NULL )
         return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( self -> r4 == NULL )
         return RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcNotFound );

    return Response4GetKSrvRespObjByIdx ( self -> r4, idx, box );
}

rc_t KSrvResponseGetObjByAcc ( const KSrvResponse * self, const char * acc,
                               const KSrvRespObj ** box )
{
    if ( self == NULL )
         return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( self -> r4 == NULL )
         return RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcNotFound );

    return Response4GetKSrvRespObjByAcc ( self -> r4, acc, box );
}

rc_t KSrvResponseGet
    ( const KSrvResponse * self, uint32_t idx, const VPathSet ** set )
{
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( set == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    else {
        const VPathSet * s = ( VPathSet * ) VectorGet ( & self -> list, idx );
        if ( s == NULL )
            return RC ( rcVFS, rcPath, rcAccessing, rcItem, rcNotFound );
        else {
            rc_t rc = VPathSetAddRef ( s );
            if ( rc == 0 )
                * set = s;
            return rc;
        }
    }
}

rc_t KSrvResponseGetMapping(const KSrvResponse * self, uint32_t idx,
    const VPath ** mapping, const VPath ** vdbcacheMapping)
{
    rc_t rc = 0;

    const VPathSet * s = NULL;

    if (mapping == NULL || vdbcacheMapping == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcParam, rcNull);

    *mapping = NULL;

    if (self == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcSelf, rcNull);

    s = (VPathSet *)VectorGet(&self->list, idx);

    if (s != NULL) {
        if (s->error != NULL)
            return 0;

        if (rc == 0) {
            rc = VPathAddRef(s->mapping);
            if (rc == 0)
                * mapping = s->mapping;
        }

        if (rc == 0) {
            rc = VPathAddRef(s->cacheMapping);
            if (rc == 0)
                * vdbcacheMapping = s->cacheMapping;
        }
    }

    return rc;
}

rc_t KSrvResponseGetOSize(const KSrvResponse * self, uint32_t idx,
    uint64_t * osize)
{
    rc_t rc = 0;
    const VPathSet * s = NULL;
    if (osize == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcParam, rcNull);
    *osize = 0;
    if (self == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcSelf, rcNull);
    s = (VPathSet *)VectorGet(&self->list, idx);
    if (s != NULL) {
        if (s->error != NULL)
            return 0;
        if (rc == 0)
            * osize = s->osize;
    }
    return rc;
}

rc_t KSrvResponseGetPath ( const KSrvResponse * self, uint32_t idx,
    VRemoteProtocols p, const VPath ** aPath, const VPath ** vdbcache,
    const KSrvError ** error )
{
    rc_t rc = 0;
    const VPathSet * s = NULL;
    const KSrvRespObj * obj = NULL;
    bool has_proto[eProtocolMask + 1];
    uint32_t i;
    String fasp;
    String http;
    String https;
    if (self == NULL)
        return RC(rcVFS, rcQuery, rcExecuting, rcSelf, rcNull);
    if (p == eProtocolDefault)
        p = DEFAULT_PROTOCOLS;
    if (aPath != NULL)
        * aPath = NULL;
    if (vdbcache != NULL)
        * vdbcache = NULL;
    if (error != NULL)
        * error = NULL;
    CONST_STRING(&fasp, "fasp");
    CONST_STRING(&http, "http");
    CONST_STRING(&https, "https");
    memset(has_proto, 0, sizeof has_proto);
    for (i = 0; i < eProtocolMaxPref; ++i)
        has_proto[(p >> (i * 3)) & eProtocolMask] = true;
    s = (VPathSet *)VectorGet(&self->list, idx);
    if ( s != NULL ) {
        if (s->error == NULL)
            return VPathSetGet(s, p, aPath, vdbcache);
        else {
            if (error != NULL) {
                rc_t rc = KSrvErrorAddRef(s->error);
                if (rc == 0)
                    * error = s->error;
                return rc;
            }
            return RC(rcVFS, rcQuery, rcExecuting, rcError, rcExists);
        }
    }
    else {
        rc_t rx = 0;
        int64_t code = 0;
        const char * msg = NULL;
        bool found = false;
        bool hasAny = false;
        rc = KSrvResponseGetObjByIdx(self, idx, &obj);
        if (rc != 0)
            return rc;
        rc = KSrvRespObjGetError(obj, & rx, & code, & msg);
        if (rx != 0) {
            if (error == NULL)
                return RC(rcVFS, rcQuery, rcExecuting, rcError, rcExists);
            else
                return KSrvErrorMake4(error, rx, code, msg);
        }
        else {
            KSrvRespObjIterator * it = NULL;
            rc = KSrvRespObjMakeIterator(obj, &it);
            while (rc == 0) {
                KSrvRespFile * file = NULL;
                KSrvRespFileIterator * fi = NULL;
                const VPath * path = NULL;
                rc = KSrvRespObjIteratorNextFile(it, &file);
                if (rc != 0 || file == NULL)
                    break;
                rc = KSrvRespFileMakeIterator(file, &fi);
                while (rc == 0) {
                    rc = KSrvRespFileIteratorNextPath(fi, &path);
                    if (rc == 0) {
                        if (path != NULL) {
                            String scheme;
                            rc = VPathGetScheme(path, &scheme);
                            if (rc == 0) {
                                hasAny = true;
                                if (StringEqual(&scheme, &https)) {
                                    if (has_proto[eProtocolHttps]) {
                                        *aPath = path;
                                        found = true;
                                    }
                                }
                                else if (StringEqual(&scheme, &fasp)) {
                                    if (has_proto[eProtocolFasp]) {
                                        *aPath = path;
                                        found = true;
                                    }
                                }
                                else if (StringEqual(&scheme, &http)) {
                                    if (has_proto[eProtocolHttp]) {
                                        *aPath = path;
                                        found = true;
                                    }
                                }
                                if (found)
                                    break;
                            }
                        }
                        else
                            break;
                    }
                }
                RELEASE(KSrvRespFileIterator, fi);
                RELEASE(KSrvRespFile, file);
                if (found)
                    break;
            }
            RELEASE(KSrvRespObjIterator, it);
        }
        RELEASE(KSrvRespObj, obj);
        if (!found && !hasAny)
            rc = RC(rcVFS, rcPath, rcAccessing, rcItem, rcNotFound);

        return rc;
    }
}

static rc_t KSrvResponseGetFile ( const KSrvResponse * self, uint32_t idx,
                                  const KSrvRespFile ** aFile )
{
    rc_t rc = 0;

    const KSrvRespObj * obj = NULL;

    KSrvRespObjIterator * it = NULL;

    KSrvRespFile * file = NULL;

    assert ( self && self -> r4 && aFile );
    * aFile = NULL;

    rc = KSrvResponseGetObjByIdx ( self, idx, & obj );
    if ( rc != 0 )
        return rc;

    rc = KSrvRespObjMakeIterator ( obj, & it );

    if ( rc == 0 )
        rc = KSrvRespObjIteratorNextFile ( it, & file );
    if ( rc == 0 )
        * aFile = file;

    RELEASE ( KSrvRespObjIterator, it );

    RELEASE ( KSrvRespObj, obj );

    return rc;
}

rc_t KSrvResponseGetLocal ( const KSrvResponse * self, uint32_t idx,
                            const VPath ** path )
{
    const VPathSet * s = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( self -> r4 != NULL ) {
        const KSrvRespFile * file = NULL;

        rc_t rc = KSrvResponseGetFile ( self, idx, & file );

        if ( rc == 0 )
            rc = KSrvRespFileGetLocal ( file, path );

        RELEASE ( KSrvRespFile, file );

        return rc;
    }

    s = ( VPathSet * ) VectorGet ( & self -> list, idx );

    if ( s == NULL )
        return RC ( rcVFS, rcPath, rcAccessing, rcItem, rcNotFound );
    else {
        if ( path != NULL )
            * path = NULL;

        if ( s -> error == NULL )
            return VPathSetGetLocal ( s, path );

        else {
            rc_t erc = 0;
            rc_t rc = KSrvErrorRc ( s -> error, & erc );
            return rc == 0 ? erc : rc;
        }
    }
}

rc_t KSrvResponseGetCache ( const KSrvResponse * self, uint32_t idx,
                            const VPath ** path )
{
    const VPathSet * s = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( self -> r4 != NULL ) {
        const KSrvRespFile * file = NULL;

        rc_t rc = KSrvResponseGetFile ( self, idx, & file );

        if ( rc == 0 )
            rc = KSrvRespFileGetCache ( file, path );

        RELEASE ( KSrvRespFile, file );

        return rc;
    }

    s = ( VPathSet * ) VectorGet ( & self -> list, idx );

    if ( s == NULL )
        return RC ( rcVFS, rcPath, rcAccessing, rcItem, rcNotFound );
    else {
        if ( path != NULL )
            * path = NULL;

        if ( s -> error == NULL )
            return VPathSetGetCache ( s, path );

        else {
            rc_t erc = 0;
            rc_t rc = KSrvErrorRc ( s -> error, & erc );
            return rc == 0 ? erc : rc;
        }
    }
}

/******************************************************************************/
