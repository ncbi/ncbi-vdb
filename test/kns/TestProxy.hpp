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

#include <kfg/config.h> /* KConfigRelease */
#include <kns/kns-mgr-priv.h> /* KNSManagerMakeConfig */
#include <kns/manager.h> /* KNSManagerRelease */
#include <ktst/unit_test_suite.hpp> // TestCase

#include "test-proxy.h" // KNSManagerMakeKEndPointArgsIterator

#define RELEASE( type, obj ) do { rc_t rc2 = type##Release ( obj ); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while ( false )

struct C { // Configuration Helper
    C ( const std::string & aPath, const std::string & aValue )
        : path ( aPath )
        , value ( aValue )
        , next ( NULL)
    {}

    ~ C ( void )
    {
        delete next;
        next = NULL;
    }

    void add ( const std::string & path, const std::string & value )
    {
        C * p = this;
        while ( p -> next != NULL) {
            p = p -> next;
        }

        p -> next = new C ( path, value );
    }

    bool contains ( const std::string & name, const std::string & aValue )
        const
    {
        const C * c = this;
        while ( c ) {
            if ( name == c -> path && aValue == c -> value ) {
                return true;
            }
            c = c -> next;
        }

        return false;
    }

    const std::string path;
    const std::string value;

    C * next;
};

struct E { // Expected Proxy Values
    E ( const std::string & aPath, uint16_t aPort )
        : path ( aPath )
        , port ( aPort )
        , next ( NULL )
    {}

    ~ E ( void )
    {
        delete next;
        next = NULL;
    }

    E * clone ( void ) const {
        E * r = NULL;

        const E * p = this;

        do {
            if ( r == NULL )
                r = new E ( p -> path, p -> port );
            else
                r -> add ( p -> path, p -> port );
            p = p -> next;
        } while ( p != NULL);

        return r;
    }

    void add ( const std::string & path, uint16_t port )
    {
        E * p = this;
        while ( p -> next != NULL) {
            p = p -> next;
        }

        p -> next = new E ( path, port );
    }

    const std::string path;
    const uint16_t port;

    E * next;
};

class CKConfig {
    KConfig * _self;

public:
    CKConfig ( const C * c = NULL ) : _self (NULL) {
        rc_t rc = KConfigMake ( & _self, NULL );
        if ( rc != 0 ) {
            throw rc;
        }

        while ( c ) {
            rc_t rc = KConfigWriteString
                ( _self, c -> path . c_str (), c -> value . c_str () );
            if ( rc != 0 ) {
                throw rc;
            }

            c = c -> next;
        }
    }

    ~CKConfig ( void ) {
        KConfigRelease ( _self );
        _self = NULL;
    }

    KConfig * get ( void ) const {
        return _self;
    }
};

class TestProxy : private ncbi::NK::TestCase {
    static bool _StringEqual ( const String * a, const String * b ) {
        assert ( a && b );
        return ( ( a ) -> len == ( b ) -> len &&
            memcmp ( ( a ) -> addr, ( b ) -> addr, ( a ) -> len ) == 0 );
    }

    TestCase * _dad;
    CKConfig _kfg;

    void testProxies ( const KNSManager * mgr, E * exp ) {
        size_t crnt = 0;
        E * e = exp;
        KNSProxies * p = KNSManagerGetProxies ( mgr, NULL );
        if ( p != NULL && e == NULL)
            REQUIRE ( ! p );
        E * first = e;
        while ( p ) {
            E * prev = first;
            const String * http_proxy = NULL;
            uint16_t http_proxy_port = 0;
            bool last = false;
            if ( ! KNSProxiesGet
                ( p, & http_proxy, & http_proxy_port, & crnt, & last ) )
            {
                REQUIRE ( ! e );
                break;
            }
            std::string proxy ( http_proxy -> addr, http_proxy -> len );
            while ( e -> path != proxy
                 || e -> port != http_proxy_port )
            {
                prev = e;
                e = e -> next;
                if ( e == NULL ) {
                    REQUIRE_EQ ( proxy,
                                 std::string ( "Cannot find in expected" ) );
                    break;
                }
            }
            if ( e == NULL )
                REQUIRE_EQ ( proxy, std::string ( "Cannot find in expected" ) );
            else if ( e -> path == proxy
                   && e -> port == http_proxy_port )
            {
                if ( prev != e ) {
                    prev -> next = e -> next;
                    e -> next = NULL;
                    delete e;
                }
                else {
                    assert ( e == first );
                    first = first -> next;
                    e -> next = NULL;
                    if ( e != exp )
                        delete e;
                }
            }
            e = first;
        }
    }

    void testEndPoints ( const KNSManager * mgr,
                         E * exp, const C * c )
    {
        E * e = exp;
        String aHost;
        CONST_STRING ( & aHost, "a.host.domain" );
        uint16_t aPort = 8976;
        size_t cnt = 0; /* number of expected proxies */
        struct KEndPointArgsIterator * i = KNSManagerMakeKEndPointArgsIterator
            ( mgr, & aHost, aPort, & cnt );
        REQUIRE ( i );
        const String * hostname = NULL;
        uint16_t port = ~ 0;
        bool proxy_default_port = true;
        bool proxy_ep = false;
        E * first = e;
        bool firstI = true;
        size_t crnt_proxy_idx = 0;
        bool last_proxy = true;
        while ( e ) {
            E * prev = first;
            String host;
            StringInit ( & host,
                e -> path. c_str (), e -> path. size (), e -> path. size () );
            REQUIRE ( KEndPointArgsIterator_Next ( i, & hostname, & port,
                & proxy_default_port, & proxy_ep,
                & crnt_proxy_idx, & last_proxy ) );
            REQUIRE ( proxy_ep );
            while ( ! StringEqual ( hostname, & host ) ) {
                prev = e;
                e = e -> next;
                if ( e == NULL ) {
                    REQUIRE_EQ ( std::string ( hostname -> addr ),
                                 std::string ( "Cannot find in expected" ) );
                    break;
                }
                StringInit ( & host, e -> path. c_str (),
                             e -> path. size (), e -> path. size () );
            }
            if ( e == NULL )
                REQUIRE_EQ ( std::string ( hostname -> addr ),
                             std::string ( "Cannot find in expected" ) );
            else if ( e -> port != 0 ) {
                REQUIRE_EQ ( port, e -> port );
                REQUIRE ( ! proxy_default_port );
            }
            else {
                REQUIRE_EQ ( static_cast < int> ( port ), 3128 );
                REQUIRE ( proxy_default_port );

                uint16_t port3 = ~ 0;
                REQUIRE ( KEndPointArgsIterator_Next ( i, & hostname, & port3,
                    & proxy_default_port, & proxy_ep,
                    & crnt_proxy_idx, & last_proxy ) );
                REQUIRE ( proxy_ep );
                REQUIRE ( StringEqual ( hostname, & host ) );
                REQUIRE_EQ ( static_cast < int> ( port3 ), 8080 );
                REQUIRE ( proxy_default_port );

                REQUIRE ( KEndPointArgsIterator_Next ( i, & hostname, & port3,
                    & proxy_default_port, & proxy_ep,
                    & crnt_proxy_idx, & last_proxy ) );
                REQUIRE ( proxy_ep );
                REQUIRE ( StringEqual ( hostname, & host ) );
                REQUIRE_EQ ( static_cast < int> ( port3 ), 80 );
                REQUIRE ( proxy_default_port );
            }
            if ( prev != e ) {
                prev -> next = e -> next;
                e -> next = NULL;
                delete e;
            }
            else {
                assert ( e == first );
                first = first -> next;
                e -> next = NULL;
                if ( e != exp )
                    delete e;
            }
            e = prev = first;
            if ( e )
                StringInit ( & host, e -> path. c_str (),
                    e -> path. size (), e -> path. size () );
            if ( firstI ) {
                size_t cnt2 = 0;
                struct KEndPointArgsIterator * i2 =
                    KNSManagerMakeKEndPointArgsIterator ( mgr,
                        & aHost, aPort, & cnt2 );
                REQUIRE_EQ ( cnt            , cnt2 );
                const String * hostname2 = NULL;
                uint16_t port2 = ~ 0;
                bool proxy_default_port2 = true;
                bool proxy_ep2 = false;
                REQUIRE ( KEndPointArgsIterator_Next ( i2, & hostname2, & port2,
                    & proxy_default_port2, & proxy_ep2,
                    & crnt_proxy_idx, NULL ) );
                if ( cnt == 1 ) {
                    REQUIRE ( StringEqual ( hostname, hostname2 ) );
                    REQUIRE_EQ            ( port    , port2 );
                    REQUIRE_EQ ( proxy_default_port , proxy_default_port2 );
                }
                else {
                    REQUIRE ( ! StringEqual ( hostname, hostname2 ) );
                    if ( proxy_default_port && proxy_default_port2 )
                        REQUIRE_EQ          ( port    , port2 );
                    else
                        REQUIRE_NE          ( port    , port2 );

                    while ( ! StringEqual ( hostname2, & host ) ) {
                        prev = e;
                        e = e -> next;
                        if ( e == NULL ) {
                            REQUIRE_EQ ( std::string ( hostname2 -> addr ),
                                    std::string ( "Cannot find in expected" ) );
                            break;
                        }
                        StringInit ( & host, e -> path. c_str (),
                                     e -> path. size (), e -> path. size () );
                    }
                    if ( e == NULL )
                        REQUIRE_EQ ( std::string ( hostname2 -> addr ),
                                    std::string ( "Cannot find in expected" ) );
                    else if ( e -> port != 0 ) {
                        REQUIRE_EQ ( port2, e -> port );
                        REQUIRE ( ! proxy_default_port2 );
                    }
                    else {
                        REQUIRE_EQ ( static_cast < int> ( port2 ), 3128 );
                        REQUIRE ( proxy_default_port2 );

                        REQUIRE ( KEndPointArgsIterator_Next ( i2, & hostname2,
                                & port2, & proxy_default_port2, & proxy_ep2,
                                & crnt_proxy_idx, NULL ) );
                        REQUIRE ( proxy_ep2 );
                        REQUIRE ( StringEqual ( hostname2, & host ) );
                        REQUIRE_EQ ( static_cast < int> ( port2 ), 8080 );
                        REQUIRE ( proxy_default_port2 );
                        REQUIRE ( proxy_ep2 );

                        REQUIRE ( KEndPointArgsIterator_Next ( i2, & hostname2,
                                & port2, & proxy_default_port2, & proxy_ep2,
                                & crnt_proxy_idx, NULL ) );
                        REQUIRE ( proxy_ep2 );
                        REQUIRE ( StringEqual ( hostname2, & host ) );
                        REQUIRE_EQ ( static_cast < int> ( port2 ), 80 );
                        REQUIRE ( proxy_default_port2 );
                    }
                    if ( prev != e ) {
                        prev -> next = e -> next;
                        e -> next = NULL;
                        delete e;
                    }
                    else {
                        assert ( e == first );
                        first = first -> next;
                        e -> next = NULL;
                        if ( e != exp )
                            delete e;
                    }
                    e = first;
                }
                REQUIRE_EQ ( proxy_ep           , proxy_ep2 );
                firstI = false;
                free ( i2 );
                i2 = NULL;
            }
        }

        bool checkLast
            = c == NULL || ! c -> contains ( "/http/proxy/only", "true" );
        bool checkPrev = false;
        if ( last_proxy ) {
            if ( crnt_proxy_idx < cnt && ! checkLast )
                checkPrev = true;
        }
        else
            checkPrev = true;
        if ( checkPrev ) {
            REQUIRE ( KEndPointArgsIterator_Next ( i, & hostname,
                    & port, & proxy_default_port, & proxy_ep, NULL, NULL ) );
            REQUIRE ( proxy_ep );
            if ( proxy_default_port ) {
                REQUIRE_EQ ( static_cast < int> ( port ), 3128 );

                REQUIRE ( KEndPointArgsIterator_Next ( i, & hostname,
                    & port, & proxy_default_port, & proxy_ep, NULL, NULL ) );
                REQUIRE_EQ ( static_cast < int> ( port ), 8080 );
                REQUIRE ( proxy_default_port );
                REQUIRE ( proxy_ep );

                REQUIRE ( KEndPointArgsIterator_Next ( i, & hostname,
                    & port, & proxy_default_port, & proxy_ep, NULL, NULL ) );
                REQUIRE_EQ ( static_cast < int> ( port ), 80 );
                REQUIRE ( proxy_default_port );
                REQUIRE ( proxy_ep );
            }
        }

        if ( checkLast ) {
            REQUIRE (     KEndPointArgsIterator_Next ( i, & hostname,
                    & port, & proxy_default_port, & proxy_ep, NULL, NULL ) );
            REQUIRE ( ! proxy_ep );
            REQUIRE ( _StringEqual ( hostname, & aHost ) );
            REQUIRE_EQ ( port, aPort );
            REQUIRE ( ! proxy_default_port );
        }
        REQUIRE ( !   KEndPointArgsIterator_Next ( i, & hostname, & port,
            & proxy_default_port, & proxy_ep, NULL, NULL ) );

        free ( i );
        i = NULL;
    }

public:
    TestProxy
            ( TestCase * dad, const C * c = NULL, E * e = NULL )
        : TestCase ( dad -> GetName () ), _dad ( dad ), _kfg ( c )
    {
        rc_t rc = 0;
        KNSManager * mgr = NULL;
        REQUIRE_RC ( KNSManagerMakeConfig ( & mgr, _kfg . get () ) );

        if ( e == NULL )
            return;

        E * ec = e -> clone ();
        testProxies   ( mgr, e );
        testEndPoints ( mgr, ec, c );
        delete ec;

        RELEASE ( KNSManager, mgr );
        REQUIRE_RC ( rc );
    }

    ~ TestProxy ( void )
    {   assert ( _dad ); _dad -> ErrorCounterAdd ( GetErrorCounter () ); }
};

////////////////////////////////////////////////////////////////////////////////
