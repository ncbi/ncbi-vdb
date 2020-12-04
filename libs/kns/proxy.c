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


#include <kfg/config.h> /* KConfigOpenNodeRead */

#include <klib/container.h> /* BSTree */
#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h> /* PLOGERR */
#include <klib/printf.h> /* string_vprintf */
#include <klib/rc.h>
#include <klib/text.h> /* String */

#include <kns/http.h> /* KNSManagerSetHTTPProxyPath */

#include "mgr-priv.h" /* KNSProxiesVSetHTTPProxyPath */

#include <sysalloc.h>

#include <assert.h>
#include <time.h> /* time */


#define RELEASE( type, obj ) do { rc_t rc2 = type##Release ( obj ); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while ( false )


/* String *********************************************************************/

static bool StringCmp ( const String * self, const char * val ) {
    String v;
    StringInitCString ( & v, val );

    return StringEqual ( self, & v );
}

static rc_t StringRelease ( String * self ) {
    StringWhack ( self );

    return 0;
}

/****************************************************************** String */

/* HttpProxy ******************************************************************/

typedef struct HttpProxy HttpProxy;

struct HttpProxy {
    const struct String * proxy_host;
    uint16_t proxy_port;

    /* points to a next HttpProxy in array for debugging only */
    struct HttpProxy * next;
};

static void HttpProxyClear ( HttpProxy * self ) {
    assert ( self );

    StringWhack ( self -> proxy_host );
}

static rc_t HttpProxyWhack ( HttpProxy * self ) {
    HttpProxyClear ( self );
    memset ( self, 0, sizeof *self );
    free ( self );
    return 0;
}

static rc_t HttpProxyGetPath ( const HttpProxy * self,
                               const String ** proxy )
{
    rc_t rc = 0;

    if ( proxy == NULL )
        rc = RC ( rcNS, rcMgr, rcAccessing, rcParam, rcNull );
    else {
        if ( self == NULL )
            rc = RC ( rcNS, rcMgr, rcAccessing, rcSelf, rcNull );
        else if ( self -> proxy_host != NULL ) {
            return StringCopy ( proxy, self -> proxy_host );
        }

        * proxy = NULL;
    }

    return rc;
}

/****************************************************************** HttpProxy */

typedef struct KNSProxies {
    bool http_proxy_enabled;
    bool http_proxy_only; /* don't try direct connection - proxy only */
    BSTree proxie_tree;

    HttpProxy ** http_proxies;
    size_t http_proxies_idx;
    size_t http_proxies_cnt;

    int rand;
    int tmpI;
    size_t tmpS;

    BSTNode * tmpB;
} KNSProxies;

/* BSTItem ******************************************************************/

typedef struct {
    BSTNode n;
    HttpProxy * proxy;
} BSTItem;

static void BSTItemWhack ( BSTNode * n, void * ignore ) {
    free ( n );
}

static int64_t CC BSTItemCmp ( const void * item, const BSTNode * n ) {
    int64_t res = 0;

    const HttpProxy * s = item;
    const BSTItem * i = ( BSTItem * ) n;

    assert ( s && i );

    res = StringCompare ( i -> proxy -> proxy_host,  s -> proxy_host );
    if ( res != 0 )
        return res;
    else if ( i -> proxy -> proxy_port == s -> proxy_port )
        return 0;
    else if ( i -> proxy -> proxy_port < s -> proxy_port )
        return -1;
    else
        return 1;
}

static
int64_t CC BSTreeSort ( const BSTNode * item, const BSTNode * n )
{
    const BSTItem * i = ( BSTItem * ) item;
    assert ( i );
    return BSTItemCmp ( i -> proxy, n );
}

static void CC KNSProxiesBSTreeCount ( BSTNode * n, void * data ) {
    size_t * http_proxies_cnt = data;

    assert ( http_proxies_cnt );

    ++ * http_proxies_cnt;
}

static void CC KNSProxiesBSTreeInit ( BSTNode * n, void * data ) {
    const BSTItem * node = ( BSTItem * ) n;
    KNSProxies * self = data;

    assert ( node && self && self -> http_proxies );

    if ( self -> tmpS < self -> http_proxies_cnt )
        self -> http_proxies [ self -> tmpS ++ ] = node -> proxy;
}

static bool CC KNSProxiesBSTreeSetRand ( BSTNode * n, void * data ) {
    KNSProxies * self = data;

    assert ( self );

    if ( self -> tmpI ++ == self -> rand ) {
        self -> tmpB = n;
        return true;
    }
    else
        return false;
}

/****************************************************************** BSTItem */

/* KNSProxies *****************************************************************/

/* N.B.: DO NOT WHACK THE RETURNED proxy_host String !!! */
bool KNSProxiesGet ( KNSProxies * self, const String ** proxy_host,
                     uint16_t * proxy_port, size_t * cnt, bool * last )
{
    bool dummy;
    if ( last == NULL )
        last = & dummy;

    assert ( proxy_host && proxy_port && cnt );

    if ( self != NULL && self -> http_proxies != NULL ) {
        if ( ( * cnt ) ++ < self -> http_proxies_cnt ) {
            const HttpProxy * proxy = NULL;
            if ( self -> http_proxies_idx >= self ->http_proxies_cnt )
                self -> http_proxies_idx = 0;
            proxy = self -> http_proxies [ self -> http_proxies_idx ++ ];
            * last = self -> http_proxies_idx == self -> http_proxies_cnt;
            * proxy_host = proxy -> proxy_host;
            * proxy_port = proxy -> proxy_port;
            return true;
        }
    }

    * proxy_host = NULL;
    * proxy_port = 0;
    return false;
}

KNSProxies * KNSProxiesGetHttpProxy ( KNSProxies * self,
                                      size_t * cnt )
{
    assert ( self && cnt );

    if ( self -> http_proxies == NULL )
        return NULL;
    else {
        if ( self -> http_proxies_cnt == 0 )
            return NULL;
        else {
            * cnt = self -> http_proxies_cnt;
            return self;
        }
    }
}

/* DEPRECATED */
rc_t KNSProxiesGetHttpProxyPath ( const KNSProxies* self,
                                  const String ** proxy )
{
    const HttpProxy * p = NULL;

    assert ( self );

    if ( self -> http_proxies != NULL )
        p = * self -> http_proxies;

    return HttpProxyGetPath ( p, proxy );
}

static rc_t KNSProxiesHttpProxyClear ( KNSProxies * self ) {
    size_t i = 0;

    assert ( self );

    for ( i = 0; i < self -> http_proxies_cnt; ++ i )
        HttpProxyClear ( self -> http_proxies [ i ] );

    return 0;
}

rc_t KNSProxiesWhack ( KNSProxies * self ) {
    size_t i = 0;

    assert ( self );

    for ( i = 0; i < self -> http_proxies_cnt; ++ i ) {
        HttpProxyWhack ( self -> http_proxies [ i ] );
        self -> http_proxies [ i ] = NULL;
    }

    BSTreeWhack ( & self -> proxie_tree, BSTItemWhack, NULL );

    free ( self -> http_proxies );
    self -> http_proxies = NULL;

    free ( self );

    return 0;
}

static rc_t KNSProxiesAddHttpProxyPath ( KNSProxies * self,
    const char * proxy, size_t proxy_size,
    uint16_t proxy_port )
{
    const String * proxy_host = NULL;

    rc_t rc = 0;

    HttpProxy * new_proxy = NULL;
    BSTItem * node = NULL;

    HttpProxy add = { proxy_host, proxy_port, 0 };

    assert ( self );

    if ( proxy == NULL )
        return 0;

    if ( rc == 0 ) {
        String tmp;
        StringInit ( & tmp, proxy, proxy_size,
                     string_len ( proxy, proxy_size ) );
        rc = StringCopy ( & proxy_host, & tmp );
        if ( rc == 0 )
            add . proxy_host = proxy_host;
        else
            return rc;
    }

    if ( BSTreeFind ( & self -> proxie_tree, & add, BSTItemCmp )
         != NULL )
    {
        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_PROXY ),
            ( "Ignored duplicate proxy '%S:%d'\n", proxy_host, proxy_port ) );
        free ( ( void * ) proxy_host );
        return 0;
    }

    new_proxy = calloc ( 1, sizeof * new_proxy );
    if ( new_proxy == NULL )
        return RC ( rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted );
    new_proxy -> proxy_host = proxy_host;
    new_proxy -> proxy_port = proxy_port;
    node = calloc ( 1, sizeof * node );
    if ( node == NULL ) {
        free ( new_proxy );
        return RC ( rcNS, rcMgr, rcAllocating, rcMemory, rcExhausted );
    }
    node -> proxy = new_proxy;

    rc = BSTreeInsert ( & self -> proxie_tree, ( BSTNode * ) node, BSTreeSort );

    DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_PROXY ),
        ( "Added proxy '%S:%d'\n", proxy_host, proxy_port ) );

    if ( ! self -> http_proxy_enabled )
        self -> http_proxy_enabled = ( proxy_host != NULL );

    return rc;
}

static rc_t CC KNSProxiesAddHTTPProxyPath ( KNSProxies * self,
                                            const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    assert ( self );

    rc = KNSProxiesVSetHTTPProxyPath ( self, fmt, args, false );

    va_end ( args );

    return rc;
}

static bool KNSProxiesHttpProxyInitFromEnvVar ( KNSProxies * self,
                                                const char * name )
{
    const char * path = getenv ( name );

    if ( path != NULL ) {
        assert ( self );

        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_PROXY ),
            ( "Loading proxy env.var. %s='%s'\n", name, path ) );

        if ( KNSProxiesAddHTTPProxyPath ( self, path ) != 0 )
            return false;
        assert ( self -> http_proxy_enabled );

        return true;
    }

    return false;
}

static bool KNSProxiesHttpProxyInitFromEnv ( KNSProxies * self ) {
    bool loaded = false;

    const char * env_list [] = {
        "https_proxy",
        "HTTPS_PROXY",
        "all_proxy",
        "ALL_PROXY",
        "http_proxy",
        "HTTP_PROXY",
    };

    int i = 0;
    for ( i = 0; i < sizeof env_list / sizeof env_list [ 0 ]; ++ i )
        loaded |= KNSProxiesHttpProxyInitFromEnvVar ( self,
                                                      env_list [i] );

    return loaded;
}

static bool KNSProxiesHttpProxyInitFromKfg ( KNSProxies * self,
                                             const KConfig * kfg )
{
    bool fromKfg = false;

    const KConfigNode * proxy;
    rc_t rc = KConfigOpenNodeRead ( kfg, & proxy, "/http/proxy" );
    if ( rc == 0 ) {
        const KConfigNode * proxy_path;
        rc = KConfigNodeOpenNodeRead ( proxy, & proxy_path, "path" );
        if ( rc == 0 ) {
            String * path;
            rc = KConfigNodeReadString ( proxy_path, & path );
            if ( rc == 0 ) {
                DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_PROXY ),
                    ( "Loading proxy '%S' from configuration\n", path ) );
                rc = KNSProxiesAddHTTPProxyPath ( self, "%S", path );
                if ( rc == 0 )
                    fromKfg = true;

                StringWhack ( path );
            }

            KConfigNodeRelease ( proxy_path );
        }

        KConfigNodeRelease ( proxy );
    }

    return fromKfg;
}

bool KNSProxiesHttpProxyOnly ( const KNSProxies * self ) {
    assert ( self );

    return self -> http_proxy_only;
}

rc_t KNSProxiesVSetHTTPProxyPath ( KNSProxies * self,
    const char * fmt, va_list args, bool clear )
{
    rc_t rc = 0;

    if ( clear )
        rc = KNSProxiesHttpProxyClear ( self );

    if ( rc == 0 && fmt != NULL && fmt [ 0 ] != 0 ) {
        size_t psize;
        char path [ 4096 * 2 ];
        const char * p = path;
        rc = string_vprintf ( path, sizeof path, & psize, fmt, args );
        if ( rc == 0 ) {
            while ( psize != 0 ) {
                size_t s = psize;
                uint16_t proxy_port = 0;
                const char * colon = NULL;
                const char * comma = string_chr ( p, psize, ',' );
                if ( comma != NULL ) {
#ifdef MULTIPLE_PROXIES
                    s = comma - p;
#else
                    rc = RC ( rcNS, rcMgr, rcUpdating, rcPath, rcInvalid );
                    break;
#endif
                }

                colon = string_chr ( p, s, ':' );
                if ( colon != NULL ) {
                    char * end = NULL;
                    const char * port_spec = NULL;
                    long port_num = 0;

                    int have = colon - p;
                    int remains = s - have;
                    if ( remains > 2 ) {
                        assert ( colon [ 0 ] == ':' );
                        if ( colon [ 1 ] == '/' && colon [ 2 ] == '/' ) {
          /* strip off the scheme from proxy specification: it is ignored now */
                            psize -= have + 3;
                            p = colon + 3;
                            if ( psize == 0 )
                                return RC ( rcNS, rcMgr, rcUpdating,
                                            rcPath, rcInvalid );
                            continue;
                        }
                    }

                    port_spec = colon + 1;
             /* it is true that some day we might read symbolic port names... */
                    port_num = strtol ( port_spec, & end, 10 );
                    if ( port_num <= 0 || port_num >= 0x10000) {
                        PLOGERR ( klogErr, ( klogErr, rc,
                            "Proxy '$(proxy)' was ignored",
                            "proxy=%.*s", ( int ) s, p ) );
                        rc = RC ( rcNS, rcMgr, rcUpdating, rcPath, rcInvalid );
                    }
                    else if ( end [ 0 ] != 0 && comma == NULL ) {
                        if ( * end != '/' && * end != '?' ) {
                            /* skip everyting after '/' or '?' */
                            PLOGERR ( klogErr, ( klogErr, rc,
                                "Proxy '$(proxy)' was ignored",
                                "proxy=%.*s", ( int ) s, p ) );
                                rc = RC ( rcNS, rcMgr, rcUpdating,
                                          rcPath, rcInvalid );
                        }
                    }
                    if ( rc == 0 ) {
                        proxy_port = ( uint16_t ) port_num;
                        s = colon - p;
                    }
                }

                if ( rc == 0 )
                    rc = KNSProxiesAddHttpProxyPath ( self, p, s, proxy_port );

                if ( comma == NULL)
                    psize = 0;
                else {
                    s = comma - p + 1;
                    if ( s > psize )
                        psize = 0;
                    else {
                        psize -= s;
                        p += s;
                    }
                }
            }
        }
    }

    return rc;
}

bool KNSProxiesGetHTTPProxyEnabled ( const KNSProxies * self ) {
    assert ( self );

    return self -> http_proxy_enabled;
}

bool KNSProxiesSetHTTPProxyEnabled ( KNSProxies * self, bool enabled )
{
    bool prior = false;

    assert ( self );

    prior = self -> http_proxy_enabled;
    self -> http_proxy_enabled = enabled;

    return prior;
}

KNSProxies * KNSManagerKNSProxiesMake ( struct KNSManager * mgr,
                                        const KConfig * kfg )
{
    rc_t rc = 0;

    int i = 0;
    int n = 2;

    typedef enum {
        eEnv,
        eKfg,
    } EType;

    EType type [ 2 ] = { eKfg, eEnv };

    KNSProxies * self = calloc ( 1, sizeof * self ); 
    if ( self == NULL )
        return NULL;

    assert ( self );

    BSTreeInit ( & self -> proxie_tree );

    rc = KConfigReadBool
        ( kfg, "/http/proxy/enabled", & self -> http_proxy_enabled );
    if ( rc != 0 ) {
        if ( GetRCState ( rc ) == rcNotFound )
            rc = 0;
        else {
            KNSManagerSetHTTPProxyPath ( mgr, NULL );
            assert ( self -> http_proxy_enabled == false );
        }
    }
    else if ( ! self -> http_proxy_enabled )
        return self;

    {
        bool proxy_only = false;
        rc_t rc = KConfigReadBool ( kfg, "/http/proxy/only",  & proxy_only );
        if ( rc == 0 && proxy_only )
            self-> http_proxy_only = true;
    }

    {
        String * result = NULL;
        rc = KConfigReadString ( kfg, "/http/proxy/use", & result );
        if ( rc == 0 ) {
            if ( StringCmp ( result, "env") ) {
                n = 1;
                type [ 0 ] = eEnv;
            } else if ( StringCmp ( result, "kfg") ) {
                n = 1;
                type [ 0 ] = eKfg;
            } else if ( StringCmp ( result, "none") ) {
                n = 0;
            } else if ( StringCmp ( result, "env,kfg") ) {
                n = 2;
                type [ 0 ] = eEnv;
                type [ 1 ] = eKfg;
            } else if ( StringCmp ( result, "kfg,env") ) {
                n = 2;
                type [ 0 ] = eKfg;
                type [ 1 ] = eEnv;
            }
        }
        RELEASE ( String, result );
    }

    for ( i = 0; i < n; ++ i ) {
        switch ( type [ i ] ) {
            case eEnv:
                KNSProxiesHttpProxyInitFromEnv ( self );
                break;
            case eKfg:
                KNSProxiesHttpProxyInitFromKfg ( self, kfg );
                break;
            default:
                assert ( 0 );
                break;
        }
    }

    BSTreeForEach ( & self -> proxie_tree, false, KNSProxiesBSTreeCount,
                    & self -> http_proxies_cnt );

    if ( self -> http_proxies_cnt > 0 ) {
        self -> http_proxies = calloc ( self -> http_proxies_cnt,
                                        sizeof * self -> http_proxies );
        if ( self -> http_proxies == NULL )
            return NULL;
        DBGMSG ( DBG_KNS, DBG_FLAG ( DBG_KNS_PROXY ),
            ( "Will use %zu proxy spec%s%s\n", self -> http_proxies_cnt,
              self -> http_proxies_cnt > 1 ? "s" : "",
              self -> http_proxy_only ? "" : " and direct connection") );
    }

    self -> tmpS = 0;
    n = self -> http_proxies_cnt;
    srand ( time ( NULL ) );
    while ( n > 0 ) {
        self -> rand = rand () % n;
        self -> tmpI = 0;
        if ( ! BSTreeDoUntil ( & self -> proxie_tree, false,
                               KNSProxiesBSTreeSetRand, self ) )
        {
            BSTreeForEach ( & self -> proxie_tree, false,
                               KNSProxiesBSTreeInit, self ) ;
            n = 0;
        }
        else {
            const BSTItem * item = ( BSTItem * ) self -> tmpB;
            self -> http_proxies [ self -> tmpS ++ ] = item -> proxy;
            BSTreeUnlink ( & self -> proxie_tree, self -> tmpB );
            BSTItemWhack ( self -> tmpB, NULL );
            self -> tmpB = NULL;
            -- n;
        }
    }

/* BSTreeForEach ( & self -> proxie_tree, false, KNSProxiesBSTreeInit, self );*/

    for ( self -> tmpS = 1; self -> tmpS < self ->http_proxies_cnt;
       ++ self -> tmpS )
    {
        self -> http_proxies [ self -> tmpS - 1 ] -> next
            = self -> http_proxies [ self -> tmpS ];
    }

    return self;
}

/***************************************************************** KNSProxies */
