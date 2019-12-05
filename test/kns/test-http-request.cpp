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

#include <../libs/kns/http-priv.h>
#include <../libs/vfs/resolver-cgi.h> /* SDL_CGI */

#include <kapp/args.h> // Args

#include <ktst/unit_test.hpp>

static rc_t argsHandler ( int argc, char * argv [] );
TEST_SUITE_WITH_ARGS_HANDLER ( HttpRequestVerifyURLSuite, argsHandler );

using namespace std;
using namespace ncbi::NK;

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

class HttpRequestFixture : public HttpFixture
{
public:
    void MakeRequest(const char * p_urlBase)
    {
        m_url = MakeURL( p_urlBase );
        if ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, m_url.c_str() ) != 0 )
        {
            throw logic_error( "HttpRequestFixture::MakeRequest(): KNSManagerMakeClientRequest() failed" );
        }
    }

    string FormatRequest()
    {
        char buffer[4096] = "";
        size_t len;
        if ( KClientHttpRequestFormatPostMsg(m_req, buffer, sizeof buffer, & len) != 0 )
        {
            throw logic_error( "HttpRequestFixture::FormatRequest(): KClientHttpRequestFormatPostMsg() failed" );
        }
        return string ( buffer, len );
    }

    string m_url;
};

FIXTURE_TEST_CASE(HttpRequest_POST_NoParams, HttpRequestFixture)
{   // Bug: KClientHttpRequestPOST crashed if request had no parameters
    MakeRequest( GetName() );

    KClientHttpResult *rslt;
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n");
    REQUIRE_RC ( KClientHttpRequestPOST ( m_req, & rslt ) );
    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}

// KClientHttpRequestAddQueryParam

class HttpRequestVerifyURL : public SharedTest
{
public:
    HttpRequestVerifyURL( TestCase * dad, KClientHttpRequest * req, string expectedUrl )
    : SharedTest ( dad, "" )
    {
        KDataBuffer rslt;
        REQUIRE_RC ( KDataBufferMakeBytes( & rslt, 0 ) );
        REQUIRE_RC ( KClientHttpRequestURL ( req, & rslt ) );
        string s2 = string ( (const char*)rslt.base, (size_t)rslt.elem_count - 1); // 0 terminator is included in elem_count
        REQUIRE_EQ ( expectedUrl, s2 );
        KDataBufferWhack( & rslt );
    }
};

FIXTURE_TEST_CASE(HttpRequestAddQueryParam_SelfNull, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddQueryParam ( NULL, "name", "fmt" ) );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_FmtNull, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddQueryParam ( m_req, "name", NULL ) );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_FmtEmpty, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddQueryParam ( m_req, "name", "" ) );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_First, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "name", "value" ) );

    HttpRequestVerifyURL ( this, m_req, m_url + "?name=value" );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_Second, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "name1", "value1" ) );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "name2", "value2" ) );

    HttpRequestVerifyURL ( this, m_req, m_url + "?name1=value1&name2=value2" );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_NameNull, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, NULL, "value" ) );

    HttpRequestVerifyURL ( this, m_req, m_url + "?value" );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_NameEmpty, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "", "value" ) );

    HttpRequestVerifyURL ( this, m_req, m_url + "?value" );
}
FIXTURE_TEST_CASE(HttpRequestAddQueryParam_URL_encoding, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "", "value & \x1f" "a" "\x7f space \x81" ) );

    HttpRequestVerifyURL ( this, m_req, m_url + "?value%20%26%20%1fa%7f%20space%20%81" );
}

FIXTURE_TEST_CASE(HttpRequestAddHeader, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC( KClientHttpRequestAddHeader(m_req, "Accept", "text/html") );
    char buffer[4096] = "";
    REQUIRE_RC( KClientHttpRequestFormatMsg(m_req, buffer, sizeof buffer, "HEAD", NULL) );
    REQUIRE( strstr(buffer, "Accept: */*") == NULL) ;
}

// KClientHttpRequestAddPostParam
FIXTURE_TEST_CASE(RequestAddPostParam, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC ( KClientHttpRequestAddPostParam ( m_req, "acc=%s", "SRR2043623" ) );
    REQUIRE_EQ ( string ("acc=SRR2043623"),  string ( KClientHttpRequestGetBody( m_req ) ) );
}

// KClientHttpRequestAddPostFileParam

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_SelfNull, HttpRequestFixture)
{
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( nullptr, "name", "data/fileToPost", false ) );
}
FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_NameParamNull, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( m_req, nullptr, "data/fileToPost", false ) );
}
FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_PathParamNull, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( m_req, "name", nullptr, false ) );
}
FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_PathParamEmpty, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( m_req, "name", "", false ) );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_FileMissing, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( m_req, "name", "not-there.txt", false ) );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_EmptyFile, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( m_req, "name", "data/empty-file-to-post.txt", false ) );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_NonEmptyFile_Encoded, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "name", "data/file-to-post.txt", true ) );
    HttpRequestVerifyURL ( this, m_req, m_url + "?name=Y29udGVudHMgb2YgdGhlIGZpbGUKCg%3d%3d" );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_NonEmptyFile_NotEncoded, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "name", "data/file-to-post.txt", false ) );
    HttpRequestVerifyURL ( this, m_req, m_url + "?name=contents%20of%20the%20file%0a%0a" );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_SendReceiveEncoded, HttpRequestFixture)
{
    const char * Server = SDL_CGI;

    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, NULL, Server ) );

    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "acc", "SRR2043623" ) );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "filetype", "run" ) );
    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "ngc", "data/prj_phs710EA_test.ngc", true ) );

//cout << "req=\"" << FormatRequest() << "\"" << endl;

    KClientHttpResult *rslt;
    REQUIRE_RC ( KClientHttpRequestPOST ( m_req, & rslt ) );
    uint32_t code;
    char buf[1024];
    size_t msg_size;
    REQUIRE_RC ( KClientHttpResultStatus ( rslt, & code, buf, sizeof buf, & msg_size ) );
    REQUIRE_EQ ( 200u, code );
    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}

// FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_SendReceiveNotEncoded, HttpRequestFixture)
// {    // this test resides in the private repo since it uses a real JWT cart file
//     const char * Server = SDL_CGI;

//     REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, NULL, Server ) );

//     REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "filetype", "run" ) );
//     REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "ngc", "data/jwt-cart", false ) );

// //cout << "req=\"" << FormatRequest() << "\"" << endl;

//     KClientHttpResult *rslt;
//     REQUIRE_RC ( KClientHttpRequestPOST ( m_req, & rslt ) );
//     uint32_t code;
//     char buf[1024];
//     size_t msg_size;
//     REQUIRE_RC ( KClientHttpResultStatus ( rslt, & code, buf, sizeof buf, & msg_size ) );
//     REQUIRE_EQ ( 200u, code );
//     REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
// }

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_MixedPOSTparams, HttpRequestFixture)
{
    MakeRequest( GetName() );

    REQUIRE_RC ( KClientHttpRequestAddPostParam ( m_req, "acc=%s", "SRR2043623" ) );
    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "name", "data/file-to-post.txt", true ) );
    HttpRequestVerifyURL ( this, m_req, m_url + "?name=Y29udGVudHMgb2YgdGhlIGZpbGUKCg%3d%3d" );
    // "acc=SRR2043623" goes into the body
    REQUIRE_EQ ( string ("acc=SRR2043623"),  string ( KClientHttpRequestGetBody( m_req ) ) );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_POSTmultipleFiles, HttpRequestFixture)
{
    MakeRequest( GetName() );

    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "name1", "data/file-to-post.txt", false ) );
    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "name2", "data/prj_phs710EA_test.ngc", true ) );
    //TODO: verify URL
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
FIXTURE_TEST_CASE(HttpReliableRequest_ContentLength, HttpFixture)
{
    /* calling good cgi returns 200 and resolved path */
    REQUIRE_RC ( KNSManagerMakeReliableClientRequest ( m_mgr, &m_req, 0x01010000, NULL,
        "https://trace.ncbi.nlm.nih.gov/Traces/names/names.fcgi" ) );
    REQUIRE_RC ( KHttpRequestAddPostParam ( m_req, "acc=AAAB01" ) );
    REQUIRE_RC ( KHttpRequestAddPostParam ( m_req, "accept-proto=https" ) );
    REQUIRE_RC ( KHttpRequestAddPostParam ( m_req, "version=1.2" ) );

    KHttpResult * rslt;
    REQUIRE_RC ( KHttpRequestPOST ( m_req, & rslt ) );
    uint32_t code = 0;
    REQUIRE_RC ( KClientHttpResultStatus ( rslt, & code, NULL, 0, NULL ) );
    REQUIRE_EQ ( code, 200u );

    KStream * response;
    REQUIRE_RC ( KHttpResultGetInputStream ( rslt, & response ) );
    char buffer [ 512 ] = "";
    size_t num_read = 0;
    REQUIRE_RC (KStreamRead( response, buffer, sizeof buffer - 1,  &num_read ));
    REQUIRE_LT ( num_read, sizeof buffer );
    buffer [ num_read ] = '\0';
    REQUIRE_EQ ( string ( buffer + num_read - 7 ), string ( "200|ok\n" ) );
    REQUIRE_RC ( KStreamRelease ( response ) );

    REQUIRE_RC ( KHttpResultRelease( rslt ) );
}

FIXTURE_TEST_CASE(HttpReliableRequest_BadCgi, HttpFixture)
{
    /* calling non-existing cgi returns 404 */
    REQUIRE_RC ( KNSManagerMakeReliableClientRequest ( m_mgr, & m_req, 0x01000000,
        NULL, "https://trace.ncbi.nlm.nih.gov/Traces/names/bad.cgi" ) );
    REQUIRE_RC ( KHttpRequestAddPostParam ( m_req, "acc=AAAB01" ) );

    KHttpResult * rslt;
    REQUIRE_RC ( KHttpRequestPOST ( m_req, & rslt ) );
    uint32_t code = 0;
    REQUIRE_RC ( KClientHttpResultStatus ( rslt, & code, NULL, 0, NULL ) );
    REQUIRE_EQ ( code, 404u );
    REQUIRE_RC ( KHttpResultRelease( rslt ) );
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

    rc_t rc=HttpRequestVerifyURLSuite(argc, argv);
    return rc;
}

}
