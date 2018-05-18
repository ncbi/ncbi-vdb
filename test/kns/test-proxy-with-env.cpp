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

#include <kfs/directory.h> /* KDirectoryRelease */
#include <ktst/unit_test.hpp> // TEST_SUITE

#include "TestProxy.hpp" // TestProxy

#include <fstream> // ifstream
#include <sstream> // ostringstream

using std :: ifstream;
using std :: istringstream;
using std :: ostringstream;
using std :: string;

TEST_SUITE ( ProxyWithEnvTestSuite );

TEST_CASE ( TEST_PROXY_FROM_ENV ) {
    ncbi :: NK :: TestRunner * t ( ncbi :: NK :: GetTestSuite () );
    assert ( t );
    REQUIRE_GT ( t -> argc, 1 );
    assert ( t -> argv );
    assert ( t -> argv [ 1 ] );
    const char * testcase ( t -> argv [ 1 ] );
    REQUIRE ( testcase );
    C * c = NULL;
    {
        ostringstream s;
        s << testcase << "/config";
        ifstream myfile ( s . str () . c_str () );
        string line;
        while ( getline ( myfile, line) ) {
            istringstream s ( line );
            string path, value;
            s >> path >> value;
            if ( c == NULL ) {
                c = new C ( path, value );
            } else {
                c -> add ( path, value );
            }
        }
    }
    E * e = NULL;
    {
        ostringstream s;
        s << testcase << "/expected";
        ifstream myfile ( s . str () . c_str () );
        string line;
        while ( getline ( myfile, line) ) {
            istringstream s ( line );
            string host;
            int port;
            s >> host >> port;
            if ( e == NULL ) {
                e = new E ( host, port );
            } else {
                e -> add ( host, port );
            }
        }
    }
    TestProxy ( this, c, e );
    delete e;
    e = NULL;
    delete c;
    c = NULL;
}

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }

#define TODO -1

    rc_t CC KMain ( int argc, char * argv [] )  {

#if 0
const char name[] = "http_proxy"; char *e= getenv(name); ostringstream s;
s << "getenv("<<name<<")="<<(e==NULL?"NULL":e)<<"\n";std::cerr <<s.str();
#endif

        if ( argc == 0 ) {
            return TODO;
        }

        KConfigDisableUserSettings ();

        return ProxyWithEnvTestSuite ( argc, argv );
    }
}
