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

#include <kfg/properties.h> // KConfig_Set_Report_Cloud_Instance_Identity

#include <klib/data-buffer.h>
#include <klib/printf.h> // string_printf
#include <klib/vdb_release_version.h> // VDB_RELEASE_VERSION

#include <kns/http.h>
#include <kns/http-priv.h>
#include <kns/manager.h>
#include <kns/kns-mgr-priv.h>

#include "../libs/klib/base64-priv.h" // BASE64_PAD_ENCODING
#include "../libs/kns/http-priv.h"
#include "../libs/vfs/resolver-cgi.h" /* SDL_CGI */

#include <kapp/args.h> // Args

#include <ktst/unit_test.hpp>

#include <algorithm>

#define ALL

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
        KDataBuffer buffer;
        THROW_ON_RC( KDataBufferMake( & buffer, 8, 0 ) );
        if ( KClientHttpRequestFormatPostMsg(m_req, & buffer) != 0 )
        {
            throw logic_error( "HttpRequestFixture::FormatRequest(): KClientHttpRequestFormatPostMsg() failed" );
        }
        string ret = string ( (char*)buffer.base, buffer.elem_count );
        THROW_ON_RC( KDataBufferWhack( &buffer ) );
        return ret;
    }

    string m_url;
};

FIXTURE_TEST_CASE(HttpRequest_POST_Failure, HttpRequestFixture) {
    // Bug: KClientHttpRequestPOST crashed if KStreamRead returned rc
    MakeRequest(GetName());

    KClientHttpResult *rslt = (KClientHttpResult*)1;
    TestStream::ForceFailure();
    REQUIRE_RC_FAIL(KClientHttpRequestPOST(m_req, &rslt));
    REQUIRE_NULL(rslt);
    TestStream::ForceFailure(false);
}

#ifdef ALL
FIXTURE_TEST_CASE(HttpRequest_POST_NoParams, HttpRequestFixture)
{   // Bug: KClientHttpRequestPOST crashed if request had no parameters
    MakeRequest( GetName() );

    KClientHttpResult *rslt;
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n");
    REQUIRE_RC ( KClientHttpRequestPOST ( m_req, & rslt ) );
    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}
#endif

FIXTURE_TEST_CASE(HttpRequest_PUT_sra, HttpRequestFixture)
{
    MakeRequest( GetName() );

    KClientHttpResult *rslt;
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n");
    REQUIRE_RC ( KClientHttpRequestPUT ( m_req, & rslt, true ) ); // format for SRA
    REQUIRE_EQ( size_t(1), TestStream::m_requests.size() );

    char version[16] = "";
    REQUIRE_RC(string_printf(
        version, sizeof version, NULL, "%V", VDB_RELEASE_VERSION));
    assert(m_req && m_req->http);
    string expected("PUT ");
    if (m_req->http->uf == eUFAbsolute)
        expected += "http://HttpRequest_PUT_sra.com";
    expected +=
        "/blah HTTP/1.1\r\n"
        "Host: HttpRequest_PUT_sra.com\r\n"
        "Accept: */*\r\n"
        "X-SRA-Release: " + string(version) + "\r\n"
        "X-VDB-Release: " + string(version) + "\r\n"
        "User-Agent: ";
    assert(!expected.empty());

    // match expected against the start of the actual (stop before OS)
    auto m = mismatch(expected.begin(), expected.end(), TestStream::m_requests.front().begin() );
    if ( m.first != expected.end() )
    {
        cout << (int)*m.first << " != " << (int)*m.second << ", position=" << ( m.first - expected.begin() ) << endl;
        REQUIRE_EQ( expected, TestStream::m_requests.front() );
    }

    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}

FIXTURE_TEST_CASE(HttpRequest_PUT_non_sra, HttpRequestFixture)
{
    MakeRequest( GetName() );

    KClientHttpResult *rslt;
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n");
    REQUIRE_RC ( KClientHttpRequestPUT ( m_req, & rslt, false ) );  // formas as non-SRA
    REQUIRE_EQ( size_t(1), TestStream::m_requests.size() );

    assert(m_req && m_req->http);
    string expected("PUT ");
    if (m_req->http->uf == eUFAbsolute)
        expected += "http://HttpRequest_PUT_non_sra.com";
    expected +=
        "/blah HTTP/1.1\r\n"
        "Host: HttpRequest_PUT_non_sra.com\r\n"
        "\r\n";
    assert(!expected.empty());

    auto m = mismatch(expected.begin(), expected.end(), TestStream::m_requests.front().begin() );
    if ( m.first != expected.end() )
    {
        cout << (int)*m.first << " != " << (int)*m.second << ", position=" << ( m.first - expected.begin() ) << endl;
        REQUIRE_EQ( expected, TestStream::m_requests.front() );
    }

    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}

FIXTURE_TEST_CASE(HttpRequest_head_as_get, HttpRequestFixture)
{
    MakeRequest( GetName() );

#define NAME "NCBI_VDB_GET_AS_HEAD"
    putenv(const_cast<char*>(NAME "=1")); // triggers GET for HEAD

    TestStream::AddResponse(
        "HTTP/1.1 206 Partial Content\r\n"
        "Content-Range: bytes 0-6/7\r\n"
        "Content-Length: 7\r\n"
        "\r\n"
        "1234567"
        "\r\n");
    KClientHttpResult *rslt;
    REQUIRE_RC ( KClientHttpRequestHEAD ( m_req, & rslt ) );
    putenv(const_cast<char*>(NAME "="));
    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );

    string req = TestStream::m_requests.front();
    // the request is a GET
    REQUIRE_NE( string::npos, req.find("GET ") );
    // -head is temporarily appended to the (thread-local) UserAgent string
    REQUIRE_NE( string::npos, req.find("-head") );
    // and then removed
    const char * agent;
    REQUIRE_RC( KNSManagerGetUserAgent( & agent ) );
    REQUIRE_EQ( string::npos, string(agent).find("-head") );
}

FIXTURE_TEST_CASE(HttpRequest_head_as_post, HttpRequestFixture)
{
    MakeRequest( GetName() );
    m_req->ceRequired = true; // triggers POST for HEAD

    TestStream::AddResponse(
        "HTTP/1.1 206 Partial Content\r\n"
        "Content-Range: bytes 0-6/7\r\n"
        "Content-Length: 7\r\n"
        "\r\n"
        "1234567"
        "\r\n");
    KClientHttpResult *rslt;
    REQUIRE_RC ( KClientHttpRequestHEAD ( m_req, & rslt ) );
    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );

    string req = TestStream::m_requests.front();
    // the request is a POST
    REQUIRE_NE( string::npos, req.find("POST ") );
    // -head is temporarily appended to the (thread-local) UserAgent string
    REQUIRE_NE( string::npos, req.find("-head") );
    // and then removed
    const char * agent;
    REQUIRE_RC( KNSManagerGetUserAgent( & agent ) );
    REQUIRE_EQ( string::npos, string(agent).find("-head") );
}

FIXTURE_TEST_CASE(HttpRequest_HEAD_as_POST_preserveUAsuffix, HttpRequestFixture)
{
    KNSManagerSetUserAgentSuffix("suffix"); // has to survive KClientHttpRequestHEAD

    MakeRequest( GetName() );
    m_req->ceRequired = true; // triggers POST for HEAD

    TestStream::AddResponse(
        "HTTP/1.1 206 Partial Content\r\n"
        "Content-Range: bytes 0-6/7\r\n"
        "Content-Length: 7\r\n"
        "\r\n"
        "1234567"
        "\r\n");
    KClientHttpResult *rslt;
    REQUIRE_RC ( KClientHttpRequestHEAD ( m_req, & rslt ) );
    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );

    const char * agent;
    REQUIRE_RC( KNSManagerGetUserAgent( & agent ) );
    // the original suffix is still there
    REQUIRE_NE( string::npos, string(agent).find("suffix") );

    string req = TestStream::m_requests.front();
    // the request is a POST
    REQUIRE_NE(string::npos, req.find("POST "));
    // -head is appended to the UserAgent string with original suffix
    REQUIRE_NE(string::npos, req.find("suffix-head"));
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

#ifdef ALL
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
    KDataBuffer buffer;
    THROW_ON_RC( KDataBufferMake( & buffer, 8, 0 ) );
    REQUIRE_RC( KClientHttpRequestFormatMsg(m_req, & buffer, "HEAD") );
    REQUIRE( strstr((char*)buffer.base, "Accept: */*") == NULL) ;
    REQUIRE_RC ( KDataBufferWhack( &buffer ) );
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
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( nullptr, "name", "data/fileToPost" ) );
}
FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_NameParamNull, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( m_req, nullptr, "data/fileToPost" ) );
}
FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_PathParamNull, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( m_req, "name", nullptr ) );
}
FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_PathParamEmpty, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( m_req, "name", "" ) );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_FileMissing, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( m_req, "name", "not-there.txt" ) );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_EmptyFile, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC_FAIL ( KClientHttpRequestAddPostFileParam ( m_req, "name", "data/empty-file-to-post.txt" ) );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_NonEmptyFile, HttpRequestFixture)
{
    MakeRequest( GetName() );
    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "name", "data/file-to-post.txt" ) );

#if BASE64_PAD_ENCODING
	string expected ("name=Y29udGVudHMgb2YgdGhlIGZpbGUKCg==");
#else
	string expected ("name=Y29udGVudHMgb2YgdGhlIGZpbGUKCg");
#endif
	REQUIRE_EQ (
                 expected,
                 string ( KClientHttpRequestGetBody( m_req ) ) );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_SendReceive, HttpRequestFixture)
{
    const char * Server = SDL_CGI;

    REQUIRE_RC ( KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, NULL, Server ) );

    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "acc", "SRR2043623" ) );
    REQUIRE_RC ( KClientHttpRequestAddQueryParam ( m_req, "filetype", "run" ) );
    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "ngc", "data/prj_phs710EA_test.ngc" ) );

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

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_MixedPOSTparams, HttpRequestFixture)
{
    MakeRequest( GetName() );

    REQUIRE_RC ( KClientHttpRequestAddPostParam ( m_req, "acc=%s", "SRR2043623" ) );
    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "name", "data/file-to-post.txt" ) );
    // the file goes into the body

#if BASE64_PAD_ENCODING
	string expected ("acc=SRR2043623&name=Y29udGVudHMgb2YgdGhlIGZpbGUKCg==");
#else
	string expected ("acc=SRR2043623&name=Y29udGVudHMgb2YgdGhlIGZpbGUKCg");
#endif
	REQUIRE_EQ (
                 expected,
                 string ( KClientHttpRequestGetBody( m_req ) ) );
}

FIXTURE_TEST_CASE(HttpRequestAddPostFileParam_POSTmultipleFiles, HttpRequestFixture)
{
    MakeRequest( GetName() );

    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "name1", "data/file-to-post.txt" ) );
    REQUIRE_RC ( KClientHttpRequestAddPostFileParam ( m_req, "name2", "data/prj_phs710EA_test.ngc" ) );
    //TODO: verify body
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
#endif

TEST_CASE(Test_urlEncodePluses) {
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(NULL));

    const String * encoding = NULL;
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(&encoding));

    encoding = (String*) calloc(1, sizeof *encoding);
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(&encoding));
    free((void*)encoding);

    String s, d;

    CONST_STRING(&s, "");
    StringCopy(&encoding, &s);
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(&encoding));
    REQUIRE_EQ(StringCompare(encoding, &s), 0);
    StringWhack(encoding);

    CONST_STRING(&s, "a");
    StringCopy(&encoding, &s);
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(&encoding));
    REQUIRE_EQ(StringCompare(encoding, &s), 0);
    StringWhack(encoding);

    CONST_STRING(&s, "+/");
    StringCopy(&encoding, &s);
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(&encoding));
    CONST_STRING(&d, "%2b%2f");
    REQUIRE_EQ(StringCompare(encoding, &d), 0);
    StringWhack(encoding);
}

// Tests of Version Headers
TEST_CASE(TestVersionHeaders) {
    char b[99]("");

    ver_t v(0);
    REQUIRE_RC(VdbVersionPrint(v, b, sizeof b, "", ""));
    REQUIRE_EQ(string(b), string("0.0.0"));

    v = 0x01000000;
    REQUIRE_RC(VdbVersionPrint(v, b, sizeof b, "", ""));
    REQUIRE_EQ(string(b), string("1.0.0"));

    v = 0x01020000;
    REQUIRE_RC(VdbVersionPrint(v, b, sizeof b, "", ""));
    REQUIRE_EQ(string(b), string("1.2.0"));

    v = 0x01020003;
    REQUIRE_RC(VdbVersionPrint(v, b, sizeof b, "", ""));
    REQUIRE_EQ(string(b), string("1.2.3"));

    v = 0x0A0B000C;
    REQUIRE_RC(VdbVersionPrint(v, b, sizeof b, "", ""));
    REQUIRE_EQ(string(b), string("10.11.12"));

    v = 0x03010000;
    REQUIRE_RC(VdbVersionPrint(v, b, sizeof b, "X-SRA-Release: ", "\r\n"));
    REQUIRE_EQ(string(b), string("X-SRA-Release: 3.1.0\r\n"));

    v = 0x04000000;
    REQUIRE_RC(VdbVersionPrint(v, b, sizeof b, "X-VDB-Release: ", "\r\n"));
    REQUIRE_EQ(string(b), string("X-VDB-Release: 4.0.0\r\n"));
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
    KConfig * kfg = NULL;
    rc_t rc = KConfigMake(&kfg, NULL);

    if (rc == 0) // needed to use ceRequired on cloud
        rc = KConfig_Set_Report_Cloud_Instance_Identity(kfg, true);

    if ( 0 ) assert ( ! KDbgSetString ( "KNS" ) );
    if ( 0 ) assert ( ! KDbgSetString ( "VFS" ) );

    KConfigDisableUserSettings();

	// this makes messages from the test code appear
	// (same as running the executable with "-l=message")
	// TestEnv::verbosity = LogLevel::e_message;

    if (rc == 0)
        rc = HttpRequestVerifyURLSuite(argc, argv);

    rc_t r2 = KConfigRelease(kfg);
    if (rc == 0 && r2 != 0)
        rc = r2;

    return rc;
}

}
