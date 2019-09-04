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
* Unit tests for HttpRequest
*/

#include "HttpFixture.hpp"

#include <klib/data-buffer.h>

#include <kns/http.h>
#include <kns/http-priv.h>
#include <kns/manager.h>
#include <kns/kns-mgr-priv.h>

#include <kapp/args.h> // Args

#include <ktst/unit_test.hpp>

static rc_t argsHandler ( int argc, char * argv [] );
TEST_SUITE_WITH_ARGS_HANDLER ( HttpRequestTestSuite, argsHandler );

using namespace std;
using namespace ncbi::NK;

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

FIXTURE_TEST_CASE(HttpRequest_POST_NoParams, HttpFixture)
{   // Bug: KClientHttpRequestPOST crashed if request had no parameters
    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, MakeURL(GetName()).c_str() ) );

    KClientHttpResult *rslt;
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n");
    REQUIRE_RC ( KClientHttpRequestPOST ( m_req, & rslt ) );
    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}

// KClientHttpRequestAddQueryParam

class HttpRequestTest : public SharedTest
{
public:
    HttpRequestTest( TestCase * dad, KClientHttpRequest * req, string expectedUrl )
    : SharedTest ( dad, "" )
    {
        string url = GetName();
        KDataBuffer rslt;
        REQUIRE_RC ( KDataBufferMakeBytes( & rslt, 0 ) );
        REQUIRE_RC ( KClientHttpRequestURL ( req, & rslt ) );
        string s2 = string ( (const char*)rslt.base, (size_t)rslt.elem_count - 1); // 0 terminator is included in elem_count
        REQUIRE_EQ ( expectedUrl, s2 );
        KDataBufferWhack( & rslt );
    }
};

FIXTURE_TEST_CASE(HttpRequestAddQueryParam_SelfNull, HttpFixture)
{
    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, MakeURL(GetName()).c_str() ) );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddQueryParam ( NULL, "name", "fmt" ) );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_FmtNull, HttpFixture)
{
    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, MakeURL(GetName()).c_str() ) );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddQueryParam ( m_req, "name", NULL ) );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_FmtEmpty, HttpFixture)
{
    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, MakeURL(GetName()).c_str() ) );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddQueryParam ( m_req, "name", "" ) );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_First, HttpFixture)
{
    string url = MakeURL( GetName() );
    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, url.c_str() ) );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "name", "value" ) );

    HttpRequestTest ( this, m_req, url + "?name=value" );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_Second, HttpFixture)
{
    string url = MakeURL( GetName() );
    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, url.c_str() ) );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "name1", "value1" ) );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "name2", "value2" ) );

    HttpRequestTest ( this, m_req, url + "?name1=value1&name2=value2" );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_NameNull, HttpFixture)
{
    string url = MakeURL( GetName() );
    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, url.c_str() ) );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, NULL, "value" ) );

    HttpRequestTest ( this, m_req, url + "?value" );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_NameEmpty, HttpFixture)
{
    string url = MakeURL( GetName() );
    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, url.c_str() ) );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "", "value" ) );

    HttpRequestTest ( this, m_req, url + "?value" );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_URL_encoding, HttpFixture)
{
    string url = MakeURL( GetName() );
    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, url.c_str() ) );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "",
        "value & \x1f" "a" "\x7f space \x81" ) );

    HttpRequestTest ( this, m_req, url + "?value%20%26%20%1fa%7f%20space%20%81" );
}

FIXTURE_TEST_CASE(KClientHttpRequestAddPostFileParam_SelfNull, HttpFixture)
{
//    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( nullptr, "data/fileToPost" ) );
}


//////////////////////////
// Reliable HTTP request
FIXTURE_TEST_CASE(HttpReliableRequest_Make, HttpFixture)
{
    KNSManagerMakeReliableClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, MakeURL(GetName()).c_str()  );
    REQUIRE_NOT_NULL ( m_req ) ;
}

FIXTURE_TEST_CASE(HttpReliableRequest_POST_5xx_retry, HttpFixture)
{   // use default configuration for 5xx to be retried
    KNSManagerMakeReliableClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, MakeURL(GetName()).c_str()  );

    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n"); // response to GET
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n");

    KClientHttpResult *rslt;
    REQUIRE_RC ( KClientHttpRequestPOST ( m_req, & rslt ) );

    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}

/* VDB-3059: KHttpRequestPOST generates incorrect Content-Length after retry :
 it makes web server to return 400 Bad Request */
TEST_CASE(ContentLength) {
    rc_t rc = 0;
    KNSManager * kns = NULL;
    REQUIRE_RC ( KNSManagerMake ( & kns ) );

    uint32_t code = 0;
    KHttpRequest * req = NULL;
    KHttpResult * rslt = NULL;
    KStream * response = NULL;

    /* calling good cgi returns 200 and resolved path */
    REQUIRE_RC ( KNSManagerMakeReliableClientRequest ( kns, & req, 0x01000000,
        NULL, "https://trace.ncbi.nlm.nih.gov/Traces/names/names.fcgi" ) );
    REQUIRE_RC ( KHttpRequestAddPostParam ( req, "acc=AAAB01" ) );
    REQUIRE_RC ( KHttpRequestAddPostParam ( req, "accept-proto=https" ) );
    REQUIRE_RC ( KHttpRequestAddPostParam ( req, "version=1.2" ) );
    REQUIRE_RC ( KHttpRequestPOST ( req, & rslt ) );
    REQUIRE_RC ( KClientHttpResultStatus ( rslt, & code, NULL, 0, NULL ) );
    REQUIRE_EQ ( code, 200u );
    REQUIRE_RC ( KHttpResultGetInputStream ( rslt, & response ) );
    char buffer [ 512 ] = "";
    size_t num_read = 0;
    REQUIRE_RC (KStreamRead( response, buffer, sizeof buffer - 1,  &num_read ));
    REQUIRE_LT ( num_read, sizeof buffer );
    buffer [ num_read ] = '\0';
    REQUIRE_EQ ( string ( buffer + num_read - 7 ), string ( "200|ok\n" ) );
    RELEASE ( KStream, response );
    RELEASE ( KHttpResult, rslt );
    RELEASE ( KHttpRequest, req );

    /* calling non-existing cgi returns 404 */
    REQUIRE_RC ( KNSManagerMakeReliableClientRequest ( kns, & req, 0x01000000,
        NULL, "https://trace.ncbi.nlm.nih.gov/Traces/names/bad.cgi" ) );
    REQUIRE_RC ( KHttpRequestAddPostParam ( req, "acc=AAAB01" ) );
    REQUIRE_RC ( KHttpRequestPOST ( req, & rslt ) );
    REQUIRE_RC ( KClientHttpResultStatus ( rslt, & code, NULL, 0, NULL ) );
    REQUIRE_EQ ( code, 404u );
    RELEASE ( KHttpResult, rslt );
    RELEASE ( KHttpRequest, req );

    RELEASE ( KNSManager, kns );
    REQUIRE_RC ( rc );
}

TEST_CASE(TestAcceptHeader)
{
#define HOST "www.ncbi.nlm.nih.gov"
    String host;
    CONST_STRING ( & host, HOST );

    rc_t rc = 0;
    KNSManager * mgr = NULL;
    REQUIRE_RC(KNSManagerMake(&mgr));
    KClientHttp * http = NULL;
    REQUIRE_RC(KNSManagerMakeHttp(mgr, &http, NULL, 0x01010000, &host, 80));
    string url("http://" HOST);
    KClientHttpRequest * req = NULL;
    REQUIRE_RC(KClientHttpMakeRequest(http, &req, url.c_str()));
    REQUIRE_RC(KClientHttpRequestAddHeader(req, "Accept", "text/html"));
    char buffer[4096] = "";
    REQUIRE_RC(KClientHttpRequestFormatMsg(req, buffer, sizeof buffer, "HEAD", NULL));
    REQUIRE(strstr(buffer, "Accept: */*") == NULL);
    RELEASE(KClientHttpRequest, req);
    RELEASE(KClientHttp, http);
    RELEASE(KNSManager, mgr);
    REQUIRE_RC(rc);
}

//////////////////////////////////////////// Main

static rc_t argsHandler ( int argc, char * argv [] ) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0, NULL, 0 );
    ArgsWhack ( args );
    return rc;
}

extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/debug.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}
const char UsageDefaultName[] = "test-http";

rc_t CC KMain ( int argc, char *argv [] )
{
    if ( 0 ) assert ( ! KDbgSetString ( "KNS" ) );
    if ( 0 ) assert ( ! KDbgSetString ( "VFS" ) );

    KConfigDisableUserSettings();

	// this makes messages from the test code appear
	// (same as running the executable with "-l=message")
	// TestEnv::verbosity = LogLevel::e_message;

    rc_t rc=HttpRequestTestSuite(argc, argv);
    return rc;
}

}
