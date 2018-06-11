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

#include "resolver-priv.h" /* DEFAULT_PROTOCOLS */
#include "path-priv.h" /* VPathGetScheme_t */
#include <vfs/services.h> /* KSrvResponse */
#include <klib/rc.h> /* RC */
#include <klib/vector.h> /* Vector */


#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

struct VPathSet {
    atomic32_t refcount;

    const VPath * fasp;
    const VPath * file;
    const VPath * http;
    const VPath * https;
    const VPath * s3;
    const VPath * cacheFasp;
    const VPath * cacheFile;
    const VPath * cacheHttp;
    const VPath * cacheHttps;
    const VPath * cacheS3;

    const struct KSrvError * error;

    const VPath * local;
    const VPath * cache;
    /* rc code after call to VResolverQuery(&local,&cache) */
    rc_t          localRc;
    rc_t          cacheRc;
};

struct KSrvResponse {
    atomic32_t refcount;

    Vector list;
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

        RELEASE ( VPath, self -> local );
        RELEASE ( VPath, self -> cache );

        RELEASE ( KSrvError, self -> error );

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

static
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

static
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

/* KSrvResponse */
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
    KSrvResponse * self = ( KSrvResponse * ) cself;

    if ( self != NULL && atomic32_dec_and_test ( & self -> refcount ) ) {
        VectorWhack ( & self -> list, whackVPathSet, NULL );
        memset ( self, 0, sizeof * self );
        free ( self );
    }

    return 0;
}

rc_t KSrvResponseAppend ( KSrvResponse * self, const VPathSet * set ) {
    rc_t rc = 0;

    assert ( self );

    rc = VPathSetAddRef ( set );

    if ( rc == 0 )
        rc = VectorAppend ( & self -> list, NULL, set );

    return rc;
}

rc_t KSrvResponseAddLocalAndCache ( KSrvResponse * self, uint32_t idx,
                                    const VPathSet * localAndCache )
{
    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    if ( localAndCache == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    else {
        VPathSet * s = ( VPathSet * ) VectorGet ( & self -> list, idx );
        if ( s == NULL )
            return RC ( rcVFS, rcPath, rcAccessing, rcItem, rcNotFound );
        else {
            rc_t rc = 0;
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
    }
}

uint32_t KSrvResponseLength ( const KSrvResponse * self ) {
    if ( self == NULL )
         return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    return VectorLength ( & self -> list );
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

rc_t KSrvResponseGetPath ( const KSrvResponse * self, uint32_t idx,
    VRemoteProtocols p, const VPath ** path, const VPath ** vdbcache,
    const KSrvError ** error )
{
    const VPathSet * s = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

    s = ( VPathSet * ) VectorGet ( & self -> list, idx );

    if ( p == eProtocolDefault )
        p = DEFAULT_PROTOCOLS;

    if ( s == NULL )
        return RC ( rcVFS, rcPath, rcAccessing, rcItem, rcNotFound );
    else {
        if ( path != NULL )
            * path = NULL;
        if ( vdbcache != NULL )
            * vdbcache = NULL;
        if ( error != NULL )
            * error = NULL;
        if ( s -> error == NULL )
            return VPathSetGet ( s, p, path, vdbcache );
        else {
            if ( error != NULL ) {
                rc_t rc = KSrvErrorAddRef ( s -> error );
                if ( rc == 0 )
                    * error = s -> error;
                return rc;
            }
            return RC ( rcVFS, rcQuery, rcExecuting, rcError, rcExists );
        }
    }
}

rc_t KSrvResponseGetLocal ( const KSrvResponse * self, uint32_t idx,
                            const VPath ** path )
{
    const VPathSet * s = NULL;

    if ( self == NULL )
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );

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
