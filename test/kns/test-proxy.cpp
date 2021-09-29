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

#include <ktst/unit_test.hpp> // TEST_SUITE

#include "TestProxy.hpp" // TestProxy

#include <fstream> // ifstream
#include <sstream> // ostringstream

using std :: ostringstream;
using std :: string;

class TestRunner : private ncbi :: NK :: TestCase {
    TestCase * _dad;

public:
    TestRunner ( TestCase * dad, const string & testcase )
        : TestCase ( dad -> GetName () )
        , _dad ( dad )
    {
        ostringstream cmd;
        cmd << "unset http_proxy ;";

        ostringstream s;
        s << "data/test-proxy/" << testcase << "/environment";
        std :: ifstream myfile ( s . str () . c_str () );
        string line;
        while ( getline ( myfile, line) ) {
            cmd << "export " << line << "; ";
        }

        ncbi :: NK :: TestRunner * t ( ncbi :: NK :: GetTestSuite () );
        assert ( t );
        assert ( t -> argv );
        assert ( t -> argv [ 0 ] );
        const char * testBin ( t -> argv [ 0 ] );
        const char * file = strrchr ( testBin, '/' );
        cmd << string ( testBin, file - testBin )
            << "/../bin/test-proxy-with-env data/test-proxy/" << testcase;

//std::cerr<<cmd . str ()<<"\n";

        REQUIRE_EQ ( system ( cmd . str (). c_str () ), 0 );
    }

    ~ TestRunner ( void )
    {
        assert ( _dad );
        _dad -> ErrorCounterAdd ( GetErrorCounter () );
    }
};

TEST_SUITE ( ProxyTestSuite );

TEST_CASE ( TEST_NOPROXY ) { // no proxy
    TestProxy ( this );
}

TEST_CASE ( TEST_PROXY_1PATH ) { // 1 proxy with port
    C c ( "/http/proxy/path", "proxy.gov:7678");
    E e ( "proxy.gov", 7678 );
    TestProxy ( this, & c, & e );
}

TEST_CASE ( TEST_PROXY_1PATH_NoPort ) { // 1 proxy without port
    C c ( "/http/proxy/path", "proxy.gov");
    E e ( "proxy.gov", 0 );
    TestProxy ( this, & c, & e );
}

#ifdef MULTIPLE_PROXIES

TEST_CASE ( TEST_PROXY_2PATH ) { // 2 proxies with port
    C c ( "/http/proxy/path", "proxy.gov:7678,proxy2.org:768");
    E e ( "proxy.gov", 7678 );
    e . add ( "proxy2.org", 768 );
    TestProxy ( this, & c, & e );
}

TEST_CASE ( TEST_PROXY_2PATH_NoPort ) { // 2 proxies with/without port
    C c ( "/http/proxy/path", "proxy.gov:7678,proxy2.org");
    E e ( "proxy.gov", 7678 );
    e . add ( "proxy2.org", 0 );
    TestProxy ( this, & c, & e );
}

// 2 proxies from config: proxy only, no direct access
TEST_CASE ( TEST_PROXY_onlyWithoutEnv_withComma ) {
    C c (
     "/http/proxy/path", "port.config.proxy.gov:678,no-port.config.proxy2.org");
    c . add ( "/http/proxy/only", "true" );
    E e ( "port.config.proxy.gov", 678 );
    e . add ( "no-port.config.proxy2.org", 0 );
    TestProxy ( this, & c, & e );
}

#endif

// 1 proxy from config: proxy only, no direct access
TEST_CASE ( TEST_PROXY_onlyWithoutEnv ) {
    C c ( "/http/proxy/path", "port.config.proxy.gov:678" );
    c . add ( "/http/proxy/only", "true" );
    E e ( "port.config.proxy.gov", 678 );
    TestProxy ( this, & c, & e );
}

// error in proxy configuration
TEST_CASE ( TEST_PROXY_kfgErr ) {
    C c ( "/http/proxy/path", "port.config.proxy.go:v678" );
    TestProxy ( this, & c );
}

#ifndef MULTIPLE_PROXIES
// comma in proxy configuration
TEST_CASE ( TEST_PROXY_commaErr ) {
    C c ( "/http/proxy/path",
        "port.config.proxy.gov:678,port.config.proxy.gov:67" );
    TestProxy ( this, & c );
}
#endif

TEST_CASE ( TEST_PROXY_NOPROXY_NoENV ) { // no proxy from kfg, no environment
    TestRunner ( this, "none" );
}

TEST_CASE ( TEST_PROXY_1PATH_NoENV ) { // 1 proxy from kfg, no environment
    TestRunner ( this, "no-env" );
}

// 1 proxy without port, no environment
TEST_CASE ( TEST_PROXY_1PATH_NoPort_NoENV ) {
    TestRunner ( this, "no-port-no-env" );
}

#ifdef MULTIPLE_PROXIES
TEST_CASE ( TEST_PROXY_2PATH_NoENV ) { // 2 proxies with port, no environment
    TestRunner ( this, "2-path-no-env" );
}
// 2 proxies with/without port, no environment
TEST_CASE ( TEST_PROXY_2PATH_NoPort_NoENV ) {
    TestRunner ( this, "2-path-no-port-no-env" );
}
#endif

TEST_CASE ( TEST_PROXY_http_proxy ) { // http_proxy from environment
    TestRunner ( this, "http_proxy" );
}

#ifdef MULTIPLE_PROXIES // 2 proxies from config, 3 environment variables
#endif
TEST_CASE ( TEST_PROXY_all ) { // 1 proxy from config, 3 environment variables
    TestRunner ( this, "all" );
}

#ifdef MULTIPLE_PROXIES
// 2 proxies from config, 3 environment variables : ignore config
#endif
// 1 proxy from config, 3 environment variables : ignore config
TEST_CASE ( TEST_PROXY_env_only ) {
    TestRunner ( this, "env-only" );
}

// 1 proxy from config, 3 environment variables : ignore environment
TEST_CASE ( TEST_PROXY_kfg_only ) {
    TestRunner ( this, "kfg-only" );
}

#ifdef MULTIPLE_PROXIES
// 2 proxies from config, 3 environment variables : config, then environment
#endif
// 1 proxy from config, 3 environment variables : config, then environment
TEST_CASE ( TEST_PROXY_kfg_env ) {
    TestRunner ( this, "kfg-env" );
}

#ifdef MULTIPLE_PROXIES
// 2 proxies from config, 3 environment variables : environment, then config
#endif
// 1 proxy from config, 3 environment variables : environment, then config
TEST_CASE ( TEST_PROXY_env_kfg ) {
    TestRunner ( this, "env-kfg" );
}

#ifdef MULTIPLE_PROXIES
// 2 proxies from config, 3 environment variables : ignore all
#endif
// 1 proxy from config, 3 environment variables : ignore all
TEST_CASE ( TEST_PROXY_ignore ) {
    TestRunner ( this, "ignore" );
}

#ifdef MULTIPLE_PROXIES
// 2 proxies from config, 3 environment variables : proxy/enabled = true
#endif
// 1 proxy from config, 3 environment variables : proxy/enabled = true
TEST_CASE ( TEST_PROXY_enabled ) {
    TestRunner ( this, "enabled" );
}

#ifdef MULTIPLE_PROXIES
// 2 proxies from config, 3 environment variables : proxy/enabled = false
#endif
// 1 proxy from config, 3 environment variables : proxy/enabled = false
TEST_CASE ( TEST_PROXY_disabled ) {
    TestRunner ( this, "disabled" );
}

#ifdef MULTIPLE_PROXIES
// 2 proxies from config, 3 environment variables : proxy only, no direct access
#endif
// 1 proxy from config, 3 environment variables : proxy only, no direct access
TEST_CASE ( TEST_PROXY_onlyWithEnv ) {
    TestRunner ( this, "proxy-only" );
}

#ifdef MULTIPLE_PROXIES
// 2 proxies from config, 3 environment variables : proxy-only=false
#endif
// 1 proxy from config, 3 environment variables : proxy-only=false
TEST_CASE ( TEST_PROXY_notOnlyWithEnv ) {
    TestRunner ( this, "proxy-not-only" );
}

// error in congiguration and environment
TEST_CASE ( TEST_PROXY_error ) {
    TestRunner ( this, "error" );
}

#ifndef MULTIPLE_PROXIES
// comma in proxy configuration
TEST_CASE ( TEST_PROXY_ENV_commaErr ) {
    TestRunner ( this, "comma-error" );
}
#endif

TEST_CASE ( TEST_PROXY_ENV_envWithSchema ) {
    TestRunner ( this, "env-with-schema" );
}

TEST_CASE ( TEST_PROXY_ENV_envWithSchemaAndPort ) {
    TestRunner ( this, "env-with-schema-and-port" );
}

TEST_CASE ( TEST_PROXY_ENV_kfgWithSchema ) {
    TestRunner ( this, "kfg-with-schema" );
}

TEST_CASE ( TEST_PROXY_ENV_kfgWithSchemaAndPort ) {
    TestRunner ( this, "kfg-with-schema-and-port" );
}

TEST_CASE ( TEST_PROXY_ENV_withTrailingSlash ) { // VDB-3329
    TestRunner ( this, "env-with-trailing-slash" );
}

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char * argv [] )  {
        KConfigDisableUserSettings ();
        return ProxyTestSuite ( argc, argv );
    }
}

////////////////////////////////////////////////////////////////////////////////
