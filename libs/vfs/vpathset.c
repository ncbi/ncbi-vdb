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

rc_t VPathSetMake
    ( VPathSet ** self, const EVPath * src, bool singleUrl )
{
    VPathSet * p = NULL;
    rc_t rc = 0;
    rc_t r2 = 0;

    assert ( self && src );

    p = ( VPathSet * ) calloc ( 1, sizeof * p );
    if ( p == NULL )
        return RC ( rcVFS, rcPath, rcAllocating, rcMemory, rcExhausted );

    if ( singleUrl ) {
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
    assert ( self );

    return VectorAppend ( & self -> list, NULL, set );
}

uint32_t KSrvResponseLength ( const KSrvResponse * self ) {
    assert ( self );

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
    VRemoteProtocols p, const VPath ** path, const VPath ** vdbcache )
{
    const VPathSet * s = NULL;

    if ( self == NULL ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );
    }

    s = ( VPathSet * ) VectorGet ( & self -> list, idx );

    if ( p == eProtocolDefault )
        p = DEFAULT_PROTOCOLS;

    if ( s == NULL )
        return RC ( rcVFS, rcPath, rcAccessing, rcItem, rcNotFound );
    else
        return VPathSetGet ( s, p, path, vdbcache );
}

////////////////////////////////////////////////////////////////////////////////
