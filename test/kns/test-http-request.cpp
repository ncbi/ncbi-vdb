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
#include <klib/debug.h>

#include <kfg/config.h>

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

TEST_SUITE( HttpRequestVerifyURLSuite );

using namespace std;
using namespace ncbi::NK;

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
              if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

class HttpRequestFixture : public HttpFixture
{
public:
    void MakeRequest(const char * p_urlBase, const char * query = nullptr,
        const char * url = nullptr)
    {
        string q, u;
        if (query != nullptr)
            q = query;
        if (url != nullptr)
            u = url;
        m_url = MakeURL( p_urlBase, q, u );
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

#ifdef ALL
FIXTURE_TEST_CASE(HttpRequest_POST_Failure, HttpRequestFixture) {
    // Bug: KClientHttpRequestPOST crashed if KStreamRead returned rc
    MakeRequest(GetName());

    KClientHttpResult *rslt = (KClientHttpResult*)1;
    TestStream::ForceFailure();
    REQUIRE_RC_FAIL(KClientHttpRequestPOST(m_req, &rslt));
    REQUIRE_NULL(rslt);
    TestStream::ForceFailure(false);
}

FIXTURE_TEST_CASE(HttpRequest_POST_NoParams, HttpRequestFixture)
{   // Bug: KClientHttpRequestPOST crashed if request had no parameters
    MakeRequest( GetName() );

    KClientHttpResult *rslt;
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n");
    REQUIRE_RC ( KClientHttpRequestPOST ( m_req, & rslt ) );
    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(HttpRequest_PUT_sra, HttpRequestFixture)
{
    MakeRequest( GetName() );

    KClientHttpResult *rslt;
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n");
    REQUIRE_RC ( KClientHttpRequestPUT ( m_req, & rslt, true ) ); // format for SRA
    REQUIRE_EQ( size_t(1), TestStream::m_requests.size() );

    char version[16] = "";
    REQUIRE_RC(string_printf(
        version, sizeof version, NULL, "%.3V", VDB_RELEASE_VERSION));
    assert(m_req && m_req->http);
    string expected("PUT ");
    if (m_req->http->uf == eUFAbsolute)
        expected += "http://HttpRequest_PUT_sra.com";
    expected +=
        "/blah HTTP/1.1\r\n"
        "host: HttpRequest_PUT_sra.com\r\n"
        "accept: */*\r\n"
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
        "host: HttpRequest_PUT_non_sra.com\r\n"
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
#endif

#ifdef ALL
FIXTURE_TEST_CASE(HttpRequest_head_as_get, HttpRequestFixture)
{
    MakeRequest( GetName() );

#define NAME "NCBI_VDB_GET_AS_HEAD"
    putenv(const_cast<char*>(NAME "=1")); // triggers GET for HEAD

    TestStream::AddResponse(
        "HTTP/1.1 206 Partial Content\r\n"
        "content-range: bytes 0-6/7\r\n"
        "content-length: 7\r\n"
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
        "content-range: bytes 0-6/7\r\n"
        "content-length: 7\r\n"
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
        "content-range: bytes 0-6/7\r\n"
        "content-length: 7\r\n"
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
#endif

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
    REQUIRE_RC( KClientHttpRequestAddHeader(m_req, "accept", "text/html") );
    KDataBuffer buffer;
    THROW_ON_RC( KDataBufferMake( & buffer, 8, 0 ) );
    REQUIRE_RC( KClientHttpRequestFormatMsg(m_req, & buffer, "HEAD") );
    REQUIRE( strstr((char*)buffer.base, "accept: */*") == NULL) ;
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
#endif

#ifdef ALL
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

#ifdef ALL
TEST_CASE(Test_urlEncodePluses) {
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(NULL));

    const String * encoding = NULL;
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(&encoding));

    encoding = (String*) calloc(1, sizeof *encoding);
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(&encoding));
    free((void*)encoding);

    String s, d;

    CONST_STRING(&s, "");
    REQUIRE_RC(StringCopy(&encoding, &s));
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(&encoding));
    REQUIRE_EQ(StringCompare(encoding, &s), 0);
    StringWhack(encoding);

    CONST_STRING(&s, "a");
    REQUIRE_RC(StringCopy(&encoding, &s));
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(&encoding));
    REQUIRE_EQ(StringCompare(encoding, &s), 0);
    StringWhack(encoding);

    CONST_STRING(&s, "+/");
    REQUIRE_RC(StringCopy(&encoding, &s));
    REQUIRE_RC(KClientHttpRequestUrlEncodeBase64(&encoding));
    CONST_STRING(&d, "%2b%2f");
    REQUIRE_EQ(StringCompare(encoding, &d), 0);
    StringWhack(encoding);
}
#endif

#ifdef ALL
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
#endif

#ifdef ALL
TEST_CASE(TestUriEncodeForS3) {
    const String* encoding(nullptr);
    String s, d;
    CONST_STRING(&s, "/test+A/b!1#-$.&_'~(Z)z*0,9:a.txt");
    CONST_STRING(&d, "/test%2BA/b%211%23-%24.%26_%27~%28Z%29z%2A0%2C9%3Aa.txt");
    REQUIRE_RC(StringCopy(&encoding, &s));
    REQUIRE_RC(UriEncodeForAWS(&encoding));
    REQUIRE_EQ(StringCompare(encoding, &d), 0);
    StringWhack(encoding);

    CONST_STRING(&s, ";b=c?d@e[f]g H\"i%J\\k<L>m^N\0");
    CONST_STRING(&d, "%3Bb%3Dc%3Fd%40e%5Bf%5Dg%20H%22i%25J%5Ck%3CL%3Em%5EN%00");
    REQUIRE_RC(StringCopy(&encoding, &s));
    REQUIRE_RC(UriEncodeForAWS(&encoding));
    REQUIRE_EQ(StringCompare(encoding, &d), 0);
    StringWhack(encoding);
}
#endif

#ifdef ALL
TEST_CASE(TestAbsolutePathComponentOfTheURIExtraction) {
    String s, q;
    const String* uri = NULL;
    /* parse the URL */
    URLBlock block;
    memset(&block, 0, sizeof block);
    const char *url("https://examplebucket.s3.amazonaws.com/photos/photo1.jpg");
    CONST_STRING(&s, "/photos/photo1.jpg");
    REQUIRE_RC(ParseUrl(&block, url, string_measure(url, nullptr)));
    REQUIRE_EQ(StringCompare(&block.path, &s), 0);

    /* Encode URI for S3 */
    REQUIRE_RC(StringCopy(&uri, &block.path));
    REQUIRE_RC(UriEncodeForAWS(&uri));
    REQUIRE_EQ(StringCompare(uri, &s), 0);
    StringWhack(uri);

    /* do not normalize URI paths for requests to Amazon S3 */
    url = "http://s3.amazonaws.com/my-object//example//photo.user";
    CONST_STRING(&s, "/my-object//example//photo.user");
    REQUIRE_RC(ParseUrl(&block, url, string_measure(url, nullptr)));
    REQUIRE_EQ(StringCompare(&block.path, &s), 0);

    REQUIRE_RC(StringCopy(&uri, &block.path));
    REQUIRE_RC(UriEncodeForAWS(&uri));
    REQUIRE_EQ(StringCompare(uri, &s), 0);
    StringWhack(uri);

    url = "http://s3.amazonaws.com/examplebucket"
        "?prefix=somePrefix&marker=someMarker&max-keys=20";
    //    12345678101234567 12345678101234567 12345678101
    //    123456 1234567810 123456 1234567810 12345678 12
    CONST_STRING(&s, "/examplebucket");
    REQUIRE_RC(ParseUrl(&block, url, string_measure(url, nullptr)));
    REQUIRE_EQ(StringCompare(&block.path, &s), 0);
    CONST_STRING(&q, "prefix=somePrefix&marker=someMarker&max-keys=20");
    REQUIRE_EQ(StringCompare(&block.query, &q), 0);

    REQUIRE_RC(StringCopy(&uri, &block.path));
    REQUIRE_RC(UriEncodeForAWS(&uri));
    REQUIRE_EQ(StringCompare(uri, &s), 0);
    StringWhack(uri);

    KDataBuffer canonicalQueryString;

    REQUIRE_RC(KDataBufferMake(&canonicalQueryString, 8, 0));
    REQUIRE_RC(PrepareCanonicalQueryStringForAWSRequest(
        &block.query, &canonicalQueryString));
    string a((char*)canonicalQueryString.base, 0,
        canonicalQueryString.elem_count - 1);
    string e("marker=someMarker&max-keys=20&prefix=somePrefix");
    REQUIRE_EQ(a, e);
    REQUIRE_RC(KDataBufferWhack(&canonicalQueryString));

    // When a request targets a subresource,
    // the corresponding query parameter value will be an empty string ("")
    url = "http://s3.amazonaws.com/examplebucket?acl";
    CONST_STRING(&s, "/examplebucket");
    REQUIRE_RC(ParseUrl(&block, url, string_measure(url, nullptr)));
    REQUIRE_EQ(StringCompare(&block.path, &s), 0);
    CONST_STRING(&q, "acl");
    REQUIRE_EQ(StringCompare(&block.query, &q), 0);

    REQUIRE_RC(StringCopy(&uri, &block.path));
    REQUIRE_RC(UriEncodeForAWS(&uri));
    REQUIRE_EQ(StringCompare(uri, &s), 0);
    StringWhack(uri);

    REQUIRE_RC(KDataBufferMake(&canonicalQueryString, 8, 0));
    REQUIRE_RC(PrepareCanonicalQueryStringForAWSRequest(
        &block.query, &canonicalQueryString));
    a = string((char*)canonicalQueryString.base, 0,
        canonicalQueryString.elem_count - 1);
    e = string("acl=");
    REQUIRE_EQ(a, e);
    KDataBufferWhack(&canonicalQueryString);

    // mo query
    url = "http://examplebucket.s3.amazonaws.com/test.txt";
    CONST_STRING(&s, "/test.txt");
    REQUIRE_RC(ParseUrl(&block, url, string_measure(url, nullptr)));
    REQUIRE_EQ(StringCompare(&block.path, &s), 0);
    CONST_STRING(&q, "");
    REQUIRE_EQ(StringCompare(&block.query, &q), 0);

    REQUIRE_RC(StringCopy(&uri, &block.path));
    REQUIRE_RC(UriEncodeForAWS(&uri));
    REQUIRE_EQ(StringCompare(uri, &s), 0);
    StringWhack(uri);

    REQUIRE_RC(KDataBufferMake(&canonicalQueryString, 8, 0));
    REQUIRE_RC(PrepareCanonicalQueryStringForAWSRequest(
        &block.query, &canonicalQueryString));
    a = string((char*)canonicalQueryString.base, 0,
        canonicalQueryString.elem_count - 1);
    e = string("");
    REQUIRE_EQ(a, e);
    KDataBufferWhack(&canonicalQueryString);
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(test_ClientHttpResultHeaders, HttpFixture) {
    REQUIRE_RC(KNSManagerMakeClientRequest(m_mgr, &m_req, 0x01010000, &m_stream,
        MakeURL(GetName()).c_str()));
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n"
        "Duplicated-Header: Value2\r\n"
        "Unique-Header: ValueU\r\n"
        "duplicateD-headeR: Value1\r\n");

    KClientHttpResult* rslt(nullptr);
    REQUIRE_RC(KClientHttpRequestGET(m_req, &rslt));

    REQUIRE_NULL(KClientHttpResultGetHeaders(nullptr));

    const BSTree* hdrs(KClientHttpResultGetHeaders(rslt));
    SAddHeaderData d;
    REQUIRE_RC(SAddHeaderDataInit(&d, false));
    BSTreeForEach(hdrs, false, AddHeaderForAWSRequest, &d);

    REQUIRE_RC(KClientHttpResultRelease(rslt));

    const KDataBuffer* canoniclHdrs(&d.canonicalHeaders);
    assert(canoniclHdrs);
    REQUIRE_EQ(string((char*)canoniclHdrs->base, canoniclHdrs->elem_count - 1),
        string(
            "duplicated-header:Value2,Value1\n"
            "unique-header:ValueU\n"));

    REQUIRE_RC(SAddHeaderDataFini(&d));
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(TestHeadersCanonization, HttpRequestFixture) {
    MakeRequest(GetName());

    REQUIRE_RC_FAIL(KClientHttpRequestAddHeader(m_req, "", "no-name"));

    REQUIRE_RC(KClientHttpRequestAddHeader(m_req, "X-VDB-Release", "1.2.3"));
    REQUIRE_RC(KClientHttpRequestAddHeader(m_req,
        "X-SRA-Release", "45.678.90XY"));

    REQUIRE_RC(KClientHttpRequestAddHeader(m_req, "X-aMz-DaTe",
        //                                         1234567810
        " \t\v\n\r\f20130708T220855Z \t\v\n\r\f"));
    //              1234567810123456

    REQUIRE_RC(KClientHttpRequestAddHeader(m_req, "x-AmZ-cOnTeNt-ShA256",
        //                                         12345678101234567820
        " \t\v\n\r\fe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b9 \t\v\n\r\f"));
    //              123456781012345678201234567830123456784012345

    REQUIRE_RC(KClientHttpRequestAddHeader(m_req,
        "simple-header", "simple-value"));

    SAddHeaderData d;
    REQUIRE_RC(KClientHttpRequestPrepareCanonicalHeaders(m_req, &d));

    const KDataBuffer* canoniclHdrs(&d.canonicalHeaders);
    assert(canoniclHdrs);
    REQUIRE_EQ(string((char*)canoniclHdrs->base, canoniclHdrs->elem_count - 1),
        string(
            "simple-header:simple-value\n"
            "x-amz-content-sha256" ":"
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b9" "\n"
            "x-amz-date" ":" "20130708T220855Z" "\n"
            "x-sra-release" ":" "45.678.90XY" "\n"
            "x-vdb-release" ":" "1.2.3" "\n"
        ));

    const KDataBuffer* signedHdrs(&d.signedHeaders);
    assert(signedHdrs);
    REQUIRE_EQ(string((char*)signedHdrs->base, signedHdrs->elem_count - 1),
        string(
            "simple-header;"
            "x-amz-content-sha256;"
            "x-amz-date;"
            "x-sra-release;"
            "x-vdb-release;"
        ));

    REQUIRE_RC(SAddHeaderDataFini(&d));
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(TestCanonicalRequestCreation, HttpRequestFixture) {
    MakeRequest(GetName());
    KDataBuffer request;
    REQUIRE_RC(KDataBufferMake(&request, 8, 0));
    REQUIRE_RC(KClientHttpRequestCreateCanonicalRequestString(
        m_req, &request));
    REQUIRE_EQ(string((char*)request.base, request.elem_count - 1), string(
        "GET\n"
        "/blah\n"
        "\n"
        "\n"
        "\n"
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    REQUIRE_RC(KDataBufferWhack(&request));

}

FIXTURE_TEST_CASE(TestCanonicalRequestWithQueryCreation,
    HttpRequestFixture)
{
    MakeRequest(GetName(), "max-keys=2&prefix=J");
    KDataBuffer request;
    REQUIRE_RC(KDataBufferMake(&request, 8, 0));
    REQUIRE_RC(KClientHttpRequestCreateCanonicalRequestString(
        m_req, &request));
    REQUIRE_EQ(string((char*)request.base, request.elem_count - 1), string(
        "GET\n"
        "/blah\n"
        "max-keys=2&prefix=J\n"
        "\n"
        "\n"
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    REQUIRE_RC(KDataBufferWhack(&request));

}

FIXTURE_TEST_CASE(TestCanonicalRequestWithQueryNoValueCreation,
    HttpRequestFixture)
{
    MakeRequest(GetName(), "acl");
    KDataBuffer request;
    REQUIRE_RC(KDataBufferMake(&request, 8, 0));
    REQUIRE_RC(KClientHttpRequestCreateCanonicalRequestString(
        m_req, &request));
    REQUIRE_EQ(string((char*)request.base, request.elem_count - 1), string(
        "GET\n"
        "/blah\n"
        "acl=\n"
        "\n"
        "\n"
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    REQUIRE_RC(KDataBufferWhack(&request));
}

FIXTURE_TEST_CASE(TestCanonicalRequestAwsExampleCreation,
    HttpRequestFixture)
{
    MakeRequest("", "max-keys=2&prefix=J",
        "https://examplebucket.s3.amazonaws.com/");
    assert(m_req);
    REQUIRE_RC(KClientHttpRequestAddHeader(m_req, "x-amz-content-sha256",
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    REQUIRE_RC(KClientHttpRequestAddHeader(m_req, "x-amz-date",
        "20130524T000000Z"));
    m_req->testing = true; // allow setting Host header
    REQUIRE_RC(KClientHttpRequestAddHeader(m_req, "Host",
        "examplebucket.s3.amazonaws.com"));
    KDataBuffer request;
    REQUIRE_RC(KDataBufferMake(&request, 8, 0));
    REQUIRE_RC(KClientHttpRequestCreateCanonicalRequestString(
        m_req, &request));
    REQUIRE_EQ(string((char*)request.base, request.elem_count - 1), string(
        "GET\n"
        "/\n"
        "max-keys=2&prefix=J\n"
        "host:examplebucket.s3.amazonaws.com\n"
        "x-amz-content-sha256:"
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n"
        "x-amz-date:20130524T000000Z\n"
        "\n"
        "host;x-amz-content-sha256;x-amz-date\n"
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    REQUIRE_RC(KDataBufferWhack(&request));
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(TestPrepareCanonicalHeaders, HttpRequestFixture) {
    MakeRequest(GetName());

    REQUIRE_RC(KClientHttpRequestAddHeader(m_req, "X-aMz-DaTe",
        //                                         1234567810
        " \t\v\n\r\f20130708T220855Z \t\v\n\r\f"));
    //              1234567810123456

    REQUIRE_RC(KClientHttpRequestAddHeader(m_req, "x-AmZ-cOnTeNt-ShA256",
        //                                         12345678101234567820
        " \t\v\n\r\fe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b9 \t\v\n\r\f"));
    //              123456781012345678201234567830123456784012345

    REQUIRE_RC(KClientHttpRequestAddHeader(m_req,
        "simple-header", "simple-value"));

    SAddHeaderData d;
    REQUIRE_RC(KClientHttpRequestPrepareCanonicalHeaders(m_req, &d));

    const KDataBuffer* canoniclHdrs(&d.canonicalHeaders);
    assert(canoniclHdrs);
    REQUIRE_EQ(string((char*)canoniclHdrs->base, canoniclHdrs->elem_count - 1),
        string(
            "simple-header:simple-value\n"
            "x-amz-content-sha256" ":"
                            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b9" "\n"
            "x-amz-date" ":" "20130708T220855Z" "\n"));

    const KDataBuffer* signedHdrs(&d.signedHeaders);
    assert(signedHdrs);
    REQUIRE_EQ(string((char*)signedHdrs->base, signedHdrs->elem_count - 1),
        string(
            "simple-header;"
            "x-amz-content-sha256;"
            "x-amz-date;"
        ));

    REQUIRE_RC(SAddHeaderDataFini(&d));
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(TestKClientHttpRequestAddReplaceHeader,
    HttpRequestFixture)
{
    MakeRequest(GetName());

    REQUIRE_RC(KClientHttpRequestAddHeader(m_req,
        "simple-header", "simplE-valuE"));

    SAddHeaderData d;

    REQUIRE_RC(KClientHttpRequestPrepareCanonicalHeaders(m_req, &d));

    const KDataBuffer* canoniclHdrs(&d.canonicalHeaders);
    assert(canoniclHdrs);
    REQUIRE_EQ(string((char*)canoniclHdrs->base, canoniclHdrs->elem_count - 1),
        string("simple-header:simplE-valuE\n"));

    const KDataBuffer* signedHdrs(&d.signedHeaders);
    assert(signedHdrs);
    REQUIRE_EQ(string((char*)signedHdrs->base, signedHdrs->elem_count - 1),
        string("simple-header;"));

    REQUIRE_RC(SAddHeaderDataFini(&d));
    
    REQUIRE_RC(KClientHttpRequestAddHeader(m_req,
        "simple-header", "simple-value"));
    REQUIRE_RC(KClientHttpRequestPrepareCanonicalHeaders(m_req, &d));
    REQUIRE_EQ(string((char*)canoniclHdrs->base, canoniclHdrs->elem_count - 1),
        string("simple-header:simple-value\n"));
    REQUIRE_EQ(string((char*)signedHdrs->base, signedHdrs->elem_count - 1),
        string("simple-header;"));
    REQUIRE_RC(SAddHeaderDataFini(&d));

    REQUIRE_RC(KClientHttpRequestAddHeader(m_req,
        "Simple-Header", "Simple-Value"));
    REQUIRE_RC(KClientHttpRequestPrepareCanonicalHeaders(m_req, &d));
    REQUIRE_EQ(string((char*)canoniclHdrs->base, canoniclHdrs->elem_count - 1),
        string("simple-header:Simple-Value\n"));
    REQUIRE_EQ(string((char*)signedHdrs->base, signedHdrs->elem_count - 1),
        string("simple-header;"));
    REQUIRE_RC(SAddHeaderDataFini(&d));

    BSTree* hdrs(const_cast<BSTree*>(KClientHttpRequestGetHeaders(m_req)));
    REQUIRE_RC(KClientHttpReplaceHeader(hdrs,
        "SIMPLE-HEADER", "not-simple-value"));
    REQUIRE_RC(KClientHttpRequestPrepareCanonicalHeaders(m_req, &d));
    REQUIRE_EQ(string((char*)canoniclHdrs->base, canoniclHdrs->elem_count - 1),
        string("simple-header:not-simple-value\n"));
    REQUIRE_EQ(string((char*)signedHdrs->base, signedHdrs->elem_count - 1),
        string("simple-header;"));
    REQUIRE_RC(SAddHeaderDataFini(&d));
}
#endif

//////////////////////////////////////////// Main

static rc_t argsHandler ( int argc, char * argv [] ) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0, NULL, 0 );
    ArgsWhack ( args );
    return rc;
}

int main ( int argc, char *argv [] )
{
    KConfig * kfg = NULL;
    rc_t rc = KConfigMake(&kfg, NULL);

    if (rc == 0) // needed to use ceRequired on cloud
        rc = KConfig_Set_Report_Cloud_Instance_Identity(kfg, true);

    if (rc == 0)
        rc = (rc_t)HttpRequestVerifyURLSuite(argc, argv);

    rc_t r2 = KConfigRelease(kfg);
    if (rc == 0 && r2 != 0)
        rc = r2;

    return (int)rc;
}
