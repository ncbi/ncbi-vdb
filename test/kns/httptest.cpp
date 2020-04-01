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
* Unit tests for HTTP interfaces
*/

#include "HttpFixture.hpp"

#include <kapp/args.h> // Args

#include <ktst/unit_test.hpp>

#include <klib/rc.h>
#include <kfg/config.h>
#include <kfg/kfg-priv.h>

#include <kns/adapt.h> /* KStreamFromKFilePair */
#include <kns/manager.h>
#include <kns/kns-mgr-priv.h>
#include <kns/http.h>
#include <kns/stream.h>
#include <kns/http-priv.h>

#include <../libs/kns/mgr-priv.h>
#include <../libs/kns/http-priv.h>

#include <kfs/directory.h>
#include <kfs/file.h>

#include <vfs/manager.h>
#include <vfs/manager-priv.h>

#include <kproc/thread.h>

#include <sstream>

static rc_t argsHandler ( int argc, char * argv [] );
TEST_SUITE_WITH_ARGS_HANDLER ( HttpTestSuite, argsHandler );

using namespace std;
using namespace ncbi::NK;

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

#define ALL

//////////////////////////
// Regular HTTP
#ifdef ALL
FIXTURE_TEST_CASE(Http_Make, HttpFixture)
{
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}

/*FIXME: 100 used to be retried regardless of whether URL is reliable, now it is not, so the test fails */
FIXTURE_TEST_CASE(Http_Make_Continue_100_Retry, HttpFixture)
{
    TestStream::AddResponse("HTTP/1.1 100 continue\r\n");
    TestStream::AddResponse("HTTP/1.1 100 continue\r\n");
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}

FIXTURE_TEST_CASE(Http_Make_500_Fail, HttpFixture)
{   // a regular Http client does not retry
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    REQUIRE_RC_FAIL ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );
}

FIXTURE_TEST_CASE(Http_Read, HttpFixture)
{
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n"); // response to HEAD
    REQUIRE_RC ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );
    char buf[1024];
    size_t num_read;
    TestStream::AddResponse(    // response to GET
        "HTTP/1.1 206 Partial Content\r\n"
        "Accept-Ranges: bytes\r\n"
        "Transfer-Encoding: chunked\r\n"
        /*"Content-Length: 7\r\n" */ /* bug fix in KClientHttpResultHandleContentRange: used to break if Content-Length was not there */
        "Content-Range: bytes 0-6/7\r\n"
        "\r\n"
        "7\r\n"
        "content",
        true
    );
    REQUIRE_RC( KFileTimedRead ( m_file, 0, buf, sizeof buf, &num_read, NULL ) );
    REQUIRE_EQ( string ( "content" ), string ( buf, num_read ) );
}

#ifndef WINDOWS
//TODO: figure out certificate validation on Windows
FIXTURE_TEST_CASE(VDB_3661, HttpFixture)
{
    KEndPoint ep;
#define PUBLIC_DOCS "public_docs.crg.es"
    String dns;
    CONST_STRING(&dns, PUBLIC_DOCS);
    rc_t rc = KNSManagerInitDNSEndpoint(m_mgr, &ep, &dns, 80);
    if (rc ==
        SILENT_RC(rcNS, rcNoTarg, rcValidating, rcConnection, rcNotFound))
    {
        cerr << "unable to resolve host address " << PUBLIC_DOCS <<
            ". Skipping VDB_3661 test..." << endl;
        return;
    }
    REQUIRE_RC(rc);

    const KFile * file = NULL;
    REQUIRE_RC ( KNSManagerMakeHttpFile ( m_mgr, & file, NULL, 0x01010000,
       "http://" PUBLIC_DOCS "/rguigo/Papers/2017_lagarde-uszczynska_CLS"
        "/data/trackHub//dataFiles/hsAll_Cap1_Brain_hiSeq.bam"
      ) );

    uint64_t size = 0;
    REQUIRE_RC ( KFileSize ( file, & size ) );
    size = min(size, decltype(size)(4 * 1024 * 1024));

    void * buffer = malloc ( size );
    REQUIRE_NOT_NULL ( buffer );

    size_t num_read = 0;

    // read small chunk; should succeed
    size_t bsize = size;
    REQUIRE_RC ( KFileRead ( file, 0, buffer, bsize, & num_read ) );
    REQUIRE_EQ ( num_read, bsize );

    free ( buffer );

    REQUIRE_RC ( KFileRelease ( file ) );
}
#endif
#endif

struct ReadThreadData
{
    int tid;
    int num_threads;
    int num_requests;
    int content_length;
    const KFile * kHttpFile;
    const char ** contents;
};

#ifdef ALL
static rc_t CC read_thread_func( const KThread *self, void *data )
{
    rc_t rc;
    ReadThreadData * td = ( ReadThreadData * ) data;
    char buf[1024];
    size_t num_read;
    assert ( td->content_length <= (int)sizeof buf );

    for ( int i = 0; i < td->num_requests; ++i )
    {
        rc = KFileTimedRead ( td->kHttpFile, 0, buf, td->content_length, &num_read, NULL );
        if ( rc != 0 || num_read == 0 )
        {
            LOG(LogLevel::e_fatal_error, "read_thread_func: KFileTimedRead failed on kHttpFile\n");
            if ( rc == 0 )
                rc = RC( rcNS, rcFile, rcValidating, rcData, rcEmpty );
            return rc;
        }

        std::string read_content = std::string ( buf, num_read );
        int content_type = read_content[0] - '0';
        if (content_type < 0 || content_type >= td->num_threads || td->contents[content_type] != read_content)
        {
            LOG(LogLevel::e_fatal_error, "read_thread_func: received data is corrupt and/or does not match test data\n");
            rc = RC( rcNS, rcFile, rcValidating, rcData, rcCorrupt );
            return rc;
        }
    }

    return rc;
}

FIXTURE_TEST_CASE(Http_Read_Multi_User, HttpFixture)
{
    const int num_threads = 8;
    const int content_length = 256; // has to be at least 256 b, otherwise httpclient will use internal buffer and load 256 b or the whole file from http server instead of partial content
    const int content_length_total = content_length * num_threads;

    const int num_requests = 1024;
    const int num_responses_total = num_threads * num_requests;

    const char * contents[] = {
        "0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
        "1bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
        "2ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
        "3ddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd",
        "4eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee",
        "5fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
        "6ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg",
        "7hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh"
    };

    assert( sizeof contents / sizeof *contents == num_threads );

    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: " << (content_length_total) << "\r\n";
    TestStream::AddResponse(ss.str()); // response to HEAD
    REQUIRE_RC ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );

    for ( int i = 0; i < num_responses_total; ++i )
    {
        int content_type = i % num_threads;
        int range_start = 0;
        int range_end = content_length - 1;
        ss.str(std::string());
        ss << "HTTP/1.1 206 Partial Content\r\n"
                "Accept-Ranges: bytes\r\n"
                "Transfer-Encoding: chunked\r\n"
                "Content-Range: bytes " << range_start << "-" << range_end << "/" << content_length_total << "\r\n"
                "\r\n"
                "" << content_length << "\r\n"
                "" << contents[content_type];

        TestStream::AddResponse(ss.str(), true);    // response to GET
    }

    KThread *t [ num_threads ];
    ReadThreadData td [ num_threads ];
    rc_t thread_rcs [ num_threads ];
    rc_t rc = 0;
    for ( int i = 0; i < num_threads; ++i )
    {
        td[ i ].tid = i + 1;
        td[ i ].num_threads = num_threads;
        td[ i ].num_requests = num_requests;
        td[ i ].content_length = content_length;
        td[ i ].kHttpFile = m_file;
        td[ i ].contents = (const char**)contents;
        rc = KThreadMake ( &( t[ i ] ), read_thread_func, &( td[ i ] ) );
        REQUIRE_RC( rc );
    }

    for ( int i = 0; i < num_threads; ++i )
    {
        rc_t rc2 = KThreadWait ( t[ i ], &thread_rcs[i] );
        if (rc2 != 0)
        {
            LOG(LogLevel::e_fatal_error, "KThreadWait failed\n");
            if (rc == 0)
                rc = rc2;
        }
        rc2 = KThreadRelease ( t[ i ] );
        if (rc2 != 0)
        {
            LOG(LogLevel::e_fatal_error, "KThreadRelease failed\n");
            if (rc == 0)
                rc = rc2;
        }
    }
    REQUIRE_RC( rc );
    for (int i = 0; i < num_threads; ++i)
    {
        REQUIRE_RC( thread_rcs[i] );
    }


}

//////////////////////////
// HttpRetrySpecs

FIXTURE_TEST_CASE(HttpRetrySpecs_Construct_Defaults, HttpFixture)
{
    HttpRetrySpecs rs;
    KConfig* kfg = MakeConfig( GetName(), "a=\"b\"" );
    REQUIRE_RC(HttpRetrySpecsInit(&rs, kfg));

    uint8_t  max_retries;
    const uint16_t *sleep_before_retry;
    bool open_ended;

    REQUIRE ( HttpGetRetryCodes ( & rs, 502, & max_retries, & sleep_before_retry, & open_ended ) );

    REQUIRE ( ! open_ended );
    REQUIRE_EQ ( 6, (int)max_retries );
    REQUIRE_NOT_NULL(sleep_before_retry);
    REQUIRE_EQ (  0, (int)sleep_before_retry[0] );
    REQUIRE_EQ (  5, (int)sleep_before_retry[1] );
    REQUIRE_EQ ( 10, (int)sleep_before_retry[2] );
    REQUIRE_EQ ( 15, (int)sleep_before_retry[3] );
    REQUIRE_EQ ( 30, (int)sleep_before_retry[4] );
    REQUIRE_EQ ( 60, (int)sleep_before_retry[5] );

    REQUIRE_RC(KConfigRelease(kfg));
    REQUIRE_RC(HttpRetrySpecsDestroy(&rs));
}

FIXTURE_TEST_CASE(HttpRetrySpecs_Construct_Error, HttpFixture)
{
    HttpRetrySpecs rs;
    REQUIRE_RC_FAIL(HttpRetrySpecsInit(&rs, NULL));
}

FIXTURE_TEST_CASE(HttpRetrySpecs_Construct, HttpFixture)
{
    HttpRetrySpecs rs;
    KConfig* kfg = MakeConfig( GetName(), "http/reliable/5xx=\"0,1\"\n" "http/reliable/501=\"2,3,4\"\n");
    REQUIRE_RC(HttpRetrySpecsInit(&rs, kfg));

    uint8_t  max_retries;
    const uint16_t *sleep_before_retry;
    bool open_ended;

    REQUIRE ( HttpGetRetryCodes ( & rs, 502, & max_retries, & sleep_before_retry, & open_ended ) );
    REQUIRE ( ! open_ended );
    REQUIRE_EQ ( 2, (int)max_retries );
    REQUIRE_NOT_NULL ( sleep_before_retry );
    REQUIRE_EQ ( 0, (int)sleep_before_retry[0] );
    REQUIRE_EQ ( 1, (int)sleep_before_retry[1] );

    REQUIRE ( HttpGetRetryCodes(&rs, 501, &max_retries, & sleep_before_retry, & open_ended) );
    REQUIRE ( ! open_ended );
    REQUIRE_EQ ( 3, (int)max_retries );
    REQUIRE_NOT_NULL ( sleep_before_retry );
    REQUIRE_EQ ( 2, (int)sleep_before_retry[0] );
    REQUIRE_EQ ( 3, (int)sleep_before_retry[1] );
    REQUIRE_EQ ( 4, (int)sleep_before_retry[2] );

    REQUIRE_RC ( KConfigRelease(kfg) );
    REQUIRE_RC ( HttpRetrySpecsDestroy ( & rs ) );
}

FIXTURE_TEST_CASE(HttpRetrySpecs_Construct_OpenEnded, HttpFixture)
{
    HttpRetrySpecs rs;
    KConfig* kfg = MakeConfig( GetName(), "http/reliable/5xx=\"0,1+\"\n");
    REQUIRE_RC(HttpRetrySpecsInit(&rs, kfg));

    uint8_t  max_retries;
    const uint16_t *sleep_before_retry;
    bool open_ended;

    REQUIRE ( HttpGetRetryCodes ( & rs, 502, & max_retries, & sleep_before_retry, & open_ended ) );
    REQUIRE ( open_ended );
    REQUIRE_EQ ( 2, (int)max_retries );
    REQUIRE_NOT_NULL ( sleep_before_retry );
    REQUIRE_EQ ( 0, (int)sleep_before_retry[0] );
    REQUIRE_EQ ( 1, (int)sleep_before_retry[1] );

    REQUIRE_RC ( KConfigRelease(kfg) );
    REQUIRE_RC ( HttpRetrySpecsDestroy ( & rs ) );
}

FIXTURE_TEST_CASE(HttpRetrySpecs_Construct_SleepsDoNotDecrease, HttpFixture)
{
    HttpRetrySpecs rs;
    KConfig* kfg = MakeConfig( GetName(), "http/reliable/5xx=\"0,1,10,5,7\"\n");
    REQUIRE_RC(HttpRetrySpecsInit(&rs, kfg));

    uint8_t  max_retries;
    const uint16_t *sleep_before_retry;
    bool open_ended;

    REQUIRE ( HttpGetRetryCodes ( & rs, 502, & max_retries, & sleep_before_retry, & open_ended ) );
    REQUIRE ( ! open_ended );
    REQUIRE_EQ ( 5, (int)max_retries );
    REQUIRE_NOT_NULL ( sleep_before_retry );
    REQUIRE_EQ (  0, (int)sleep_before_retry[0] );
    REQUIRE_EQ (  1, (int)sleep_before_retry[1] );
    REQUIRE_EQ ( 10, (int)sleep_before_retry[2] ); // set to equal the prior sleep time
    REQUIRE_EQ ( 10, (int)sleep_before_retry[3] );
    REQUIRE_EQ ( 10, (int)sleep_before_retry[4] );

    REQUIRE_RC ( KConfigRelease(kfg) );
    REQUIRE_RC ( HttpRetrySpecsDestroy ( & rs ) );
}
#endif

//////////////////////////
// HttpRetrier

class RetrierFixture : public HttpFixture
{
public:
    static const uint8_t MaxRetries = 10;
    static const uint32_t MaxTotalWait= 600000;

    ~RetrierFixture()
    {
        if ( KHttpRetrierDestroy ( & m_retrier ) != 0 )
            cerr << "RetrierFixture::~RetrierFixture KHttpRetrierDestroy failed" << endl;
    }

    void Configure ( const char* kfg_name, const char* kfg_content, uint8_t max_retries = MaxRetries, uint32_t max_total_wait = MaxTotalWait )
    {
        KConfig* kfg = MakeConfig( kfg_name, kfg_content );

        if ( m_mgr && KNSManagerRelease ( m_mgr ) != 0 )
            throw logic_error ( "RetrierFixture::Configure KNSManagerRelease failed" );

        if ( KNSManagerMakeConfig ( & m_mgr, kfg ) != 0 )
            throw logic_error ( "RetrierFixture::Configure KNSManagerMakeConfig failed" );

        m_mgr -> maxNumberOfRetriesOnFailureForReliableURLs = max_retries;
        m_mgr -> maxTotalWaitForReliableURLs_ms = max_total_wait;

        if ( KHttpRetrierInit ( & m_retrier, kfg_name, m_mgr ) != 0 )
            throw logic_error ( "RetrierFixture::Configure KHttpRetrierInit failed" );

        KConfigRelease ( kfg );
    }

    KHttpRetrier m_retrier;
};

#ifdef ALL
FIXTURE_TEST_CASE(HttpRetrier_Construct, RetrierFixture)
{
    REQUIRE_RC ( KHttpRetrierInit ( & m_retrier, GetName(), m_mgr ) );
}

FIXTURE_TEST_CASE(HttpRetrier_NoRetry, RetrierFixture)
{
    Configure( GetName(), "http/reliable/501=\"\"\n" );

    REQUIRE ( ! KHttpRetrierWait ( & m_retrier, 501 ) );  // configured to never retry
    REQUIRE_EQ ( 0u, m_retrier . last_sleep );

    REQUIRE ( ! KHttpRetrierWait ( & m_retrier, 300 ) );  // only 4xx and 5xx retry
    REQUIRE_EQ ( 0u, m_retrier . last_sleep );
}

FIXTURE_TEST_CASE(HttpRetrier_RetryDefault4xx, RetrierFixture)
{
    REQUIRE_RC ( KHttpRetrierInit ( & m_retrier, GetName(), m_mgr ) );

    REQUIRE ( KHttpRetrierWait ( & m_retrier, 400 ) );  // default for 4xx retries
    REQUIRE_EQ ( 0u, m_retrier . last_sleep );
}

FIXTURE_TEST_CASE(HttpRetrier_RetryDefault5xx, RetrierFixture)
{
    REQUIRE_RC ( KHttpRetrierInit ( & m_retrier, GetName(), m_mgr ) );

    REQUIRE ( KHttpRetrierWait ( & m_retrier, 500 ) );  // default for 5xx retries
    REQUIRE_EQ ( 0u, m_retrier . last_sleep );
}

FIXTURE_TEST_CASE(HttpRetrier_Retry, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/501=\"1,2,3\"\n");

    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 1u, m_retrier . last_sleep );
    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 2u, m_retrier . last_sleep );
    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 3u, m_retrier . last_sleep );
    REQUIRE ( ! KHttpRetrierWait ( & m_retrier, 501 ) );  // out of retries
}

FIXTURE_TEST_CASE(HttpRetrier_Retry_ExceedMaxRetries, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/501=\"1,2,3\"\n", 2 ); // set global max retries to 2

    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 1u, m_retrier . last_sleep );
    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 2u, m_retrier . last_sleep );
    REQUIRE ( ! KHttpRetrierWait ( & m_retrier, 501 ) );  // globally set max retries overrides config
}
FIXTURE_TEST_CASE(HttpRetrier_Retry_DiffStatus, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/501=\"1,2,3\"\n" "http/reliable/502=\"4,5,6\"\n", 2 );

    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 1u, m_retrier . last_sleep );
    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 2u, m_retrier . last_sleep );
    REQUIRE ( KHttpRetrierWait ( & m_retrier, 502 ) );  // with a different status, retry count is reset
    REQUIRE_EQ ( 4u, m_retrier . last_sleep );
}

FIXTURE_TEST_CASE(HttpRetrier_Retry_ExceedMaxTime, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/501=\"200,300,400,400\"\n", MaxRetries, 600 ); // set max total to 600ms

    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 200u, m_retrier . last_sleep );
    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 300u, m_retrier . last_sleep );
    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 100u, m_retrier . last_sleep ); // cut short to the total of 600
    REQUIRE ( ! KHttpRetrierWait ( & m_retrier, 501 ) );  // no more retries
}

FIXTURE_TEST_CASE(HttpRetrier_Retry_OpenEnded_MaxRetries, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/501=\"200,300+\"\n", 4 ); // 300ms until out of retries

    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 200u, m_retrier . last_sleep );
    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 300u, m_retrier . last_sleep ); // 300 ms from now on
    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 300u, m_retrier . last_sleep );
    REQUIRE ( KHttpRetrierWait ( & m_retrier, 501 ) );
    REQUIRE_EQ ( 300u, m_retrier . last_sleep );
    REQUIRE ( ! KHttpRetrierWait ( & m_retrier, 501 ) );  // out of retries
}

//////////////////////////
// Reliable HTTP file

FIXTURE_TEST_CASE(HttpReliable_Make, HttpFixture)
{
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, true, false, false, MakeURL(GetName()).c_str() ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}

#if 0
/* 100 used to be retried regardless, now it is not, so the test fails */
FIXTURE_TEST_CASE(HttpReliable_Make_Continue_100_Retry, HttpFixture)
{
    TestStream::AddResponse("HTTP/1.1 100 continue\r\n");
    TestStream::AddResponse("HTTP/1.1 100 continue\r\n");
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, true, false, false, MakeURL(GetName()).c_str() ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}
#endif

FIXTURE_TEST_CASE(HttpReliable_Make_5xx_retry, HttpFixture)
{   // use default configuration for 5xx to be retried
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, true, false, false, MakeURL(GetName()).c_str() ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}

FIXTURE_TEST_CASE(HttpReliable_Make_500_Fail, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/500=\"\"\n"); // do not retry 500
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    REQUIRE_RC_FAIL ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, true, false, false, MakeURL(GetName()).c_str() ) );
}

FIXTURE_TEST_CASE(HttpReliable_Make_500_Retry, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/500=\"0+\"\n" "http/reliable/5xx=\"\"\n"); // do not retry 5xx, retry 500
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, true, false, false, MakeURL(GetName()).c_str() ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}

FIXTURE_TEST_CASE(HttpReliable_Make_500_TooManyRetries, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/500=\"0\"\n" "http/reliable/5xx=\"\"\n"); // do not retry 5xx, retry 500 once
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    REQUIRE_RC_FAIL ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, true, false, false, MakeURL(GetName()).c_str() ) );
}

#ifdef _DEBUGGING
FIXTURE_TEST_CASE(HttpReliable_Read_Retry, HttpFixture)
{
    SetClientHttpReopenCallback ( Reconnect ); // this hook is only available in DEBUG mode

    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n"); // response to HEAD
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, true, false, false, MakeURL(GetName()).c_str() ) );
    char buf[1024];
    size_t num_read;
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\n"); // response to GET
    TestStream::AddResponse(
        "HTTP/1.1 206 Partial Content\r\n"
        "Accept-Ranges: bytes\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Length: 7\r\n"
        "Content-Range: bytes 0-6/7\r\n"
        "\r\n"
        "7\r\n"
        "content\r\n"
    );
    REQUIRE_RC( KFileTimedRead ( m_file, 0, buf, sizeof buf, &num_read, NULL ) );
    REQUIRE_EQ( string ( "content" ), string ( buf, num_read ) );
}
#endif
#endif

struct NV {
    String AcceptRanges;
    const string bytes;

    String host;

    NV ( void ) : bytes ( "bytes"  ) {
        CONST_STRING ( & AcceptRanges, "Accept-Ranges" );

#define HOST "www.ncbi.nlm.nih.gov"
        CONST_STRING ( & host, HOST );
    }
};
static const NV s_v;

#ifdef ALL
TEST_CASE ( RepeatedHeader ) {
    rc_t rc = 0;
    KDirectory * dir = NULL;
    REQUIRE_RC ( KDirectoryNativeDir ( & dir ) );

    const KFile * f = NULL;
    REQUIRE_RC ( KDirectoryOpenFileRead ( dir, & f, "data/double_header.txt" ) );

    KNSManager * mgr = NULL;
    REQUIRE_RC ( KNSManagerMake ( & mgr ) );
    KStream * sock = NULL;
    REQUIRE_RC ( KStreamFromKFilePair ( & sock, f, NULL ) );

    KClientHttp * http = NULL;
    REQUIRE_RC ( KNSManagerMakeHttp
                 ( mgr, & http, sock, 0x01010000, & s_v . host, 80 ) );

    String msg;
    uint32_t status = 0;
    ver_t version = 0;;
    REQUIRE_RC ( KClientHttpGetStatusLine
                 ( http, NULL, & msg, & status, & version ) );

    BSTree hdrs;
    BSTreeInit ( & hdrs );
    for ( bool blank = false, close_connection = false, len_zero = false;
         ! blank; )
    {
        REQUIRE_RC ( KClientHttpGetHeaderLine
            ( http, NULL, & hdrs, & blank, & len_zero, & close_connection ) );
    }

    bool doubleChecked   = false;
    bool repeatedChecked = false;
    bool singleChecked   = false;

    String Server;
    CONST_STRING ( & Server, "Server" );
    string Apache ( "Apache" );

    String Via;
    CONST_STRING ( & Via, "Via" );
    string via ( "1.0 fred,1.1 example.com (Apache/1.1)" );

    for ( const KHttpHeader * hdr = reinterpret_cast
                < const KHttpHeader * > ( BSTreeFirst ( & hdrs ) );
          hdr != NULL;
          hdr = reinterpret_cast
                < const KHttpHeader * > ( BSTNodeNext ( & hdr -> dad ) )
        )
    {
        if ( StringEqual ( & hdr -> name, & s_v . AcceptRanges ) )  {
            REQUIRE_EQ ( string ( hdr -> value . addr, hdr -> value . size ),
                         s_v . bytes );
            repeatedChecked = true;
        }
        else if ( StringEqual ( & hdr -> name, & Server ) )  {
            REQUIRE_EQ ( string ( hdr -> value . addr, hdr -> value . size ),
                         Apache );
            singleChecked = true;
        }
        else if ( StringEqual ( & hdr -> name, & Via ) ) {
            REQUIRE_EQ ( string ( hdr -> value . addr, hdr -> value . size ),
                         via );
            doubleChecked = true;
        }
    }

    REQUIRE ( doubleChecked && repeatedChecked && singleChecked );

    BSTreeWhack ( & hdrs, KHttpHeaderWhack, NULL );

    RELEASE ( KClientHttp, http );
    RELEASE ( KStream, sock );
    RELEASE ( KNSManager, mgr );

    RELEASE ( KFile, f );

    RELEASE ( KDirectory, dir );
    REQUIRE_RC ( rc );
}

// this test relies on real server responses
TEST_CASE ( TestKClientHttpResultTestHeaderValue ) {
    rc_t rc = 0;

    KNSManager * mgr = NULL;
    REQUIRE_RC ( KNSManagerMake ( & mgr ) );

    KClientHttp * http = NULL;
    REQUIRE_RC ( KNSManagerMakeHttp
                 ( mgr, & http, NULL, 0x01010000, & s_v . host, 80 ) );

    string url ( "http://" HOST );
    KClientHttpRequest * req = NULL;
    REQUIRE_RC ( KClientHttpMakeRequest ( http, & req, url . c_str () ) );

    KClientHttpResult * rslt = NULL;
    REQUIRE_RC ( KClientHttpRequestHEAD ( req, & rslt ) );

    REQUIRE ( KClientHttpResultTestHeaderValue ( rslt,
                s_v . AcceptRanges . addr, s_v . bytes . c_str () ) );

    REQUIRE ( ! KClientHttpResultTestHeaderValue ( rslt, "foo", "bar" ) );

    RELEASE ( KClientHttpResult, rslt );

    RELEASE ( KClientHttpRequest, req );

    RELEASE ( KClientHttp, http );

    RELEASE ( KNSManager, mgr );

    REQUIRE_RC ( rc );
}

TEST_CASE ( AllowAllCertificates )
{   // as of
    rc_t rc = 0;

    KNSManager * mgr = NULL;
    REQUIRE_RC ( KNSManagerMake ( & mgr ) );

    // capture prior settings
    bool allow_all_certs = false;
    REQUIRE_RC ( KNSManagerGetAllowAllCerts ( mgr, & allow_all_certs ) );

    // ensure that manager is NOT allowing, i.e. default behavior
    // as of 7-01-2019, this function is a no-op, so the connection will succeed with
    // or without this call. NOTE: on WINDOWS, it still works as before!
    REQUIRE_RC ( KNSManagerSetAllowAllCerts ( mgr, false ) );

    // create a connection to something that normally fails; it should fail
    String host;
    CONST_STRING ( & host, "www.google.com" );

    KClientHttp * https = NULL;
    REQUIRE_RC ( KNSManagerMakeClientHttps( mgr, &https, NULL, 0x01010000, &host, 443 ) );
    RELEASE(KClientHttp, https);

    // tell manager to allow all certs
    REQUIRE_RC ( KNSManagerSetAllowAllCerts ( mgr, true ) );

    // repeat connection to something that used to fail; it should succeed
    REQUIRE_RC ( KNSManagerMakeClientHttps
              ( mgr, & https, NULL, 0x01010000, & host, 443 ) );
    RELEASE ( KClientHttp, https );

    // restore prior settings
    REQUIRE_RC ( KNSManagerSetAllowAllCerts ( mgr, allow_all_certs ) );
}

// KClientHttpResult

FIXTURE_TEST_CASE( KClientHttpResult_Size_HEAD, HttpFixture)
{
    KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, MakeURL(GetName()).c_str()  );

    TestStream::AddResponse(
        "HTTP/1.1 200 \r\n"
        "content-length: 2975717 \r\n"
        "\r\n");

    KClientHttpResult *rslt;
    REQUIRE_RC ( KClientHttpRequestHEAD ( m_req, & rslt ) );
    uint64_t size = 0;
    REQUIRE ( KClientHttpResultSize ( rslt, &size ) );
    REQUIRE_EQ ( (uint64_t)2975717, size );

    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}

FIXTURE_TEST_CASE( KClientHttpResult_Size_RangedGET, HttpFixture)
{
    KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, MakeURL(GetName()).c_str()  );
    REQUIRE_RC ( KClientHttpRequestByteRange ( m_req, 0, 2 ) );

    TestStream::AddResponse(
        "HTTP/1.1 206 \r\n"
        "content-length: 2\r\n"
        "content-range: bytes 0-1/2975717        \r\n"  // if content-range is present, retrieve total from it, not content-length
        "\r\n");

    KClientHttpResult *rslt;
    REQUIRE_RC ( KClientHttpRequestGET ( m_req, & rslt ) );
    uint64_t size = 0;
    REQUIRE ( KClientHttpResultSize ( rslt, &size ) );
    REQUIRE_EQ ( (uint64_t)2975717, size );

    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}

FIXTURE_TEST_CASE( KClientHttpResult_Size_RangedPOST, HttpFixture)
{
    KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, MakeURL(GetName()).c_str()  );
    REQUIRE_RC ( KClientHttpRequestByteRange ( m_req, 0, 2 ) );

    TestStream::AddResponse(
        "HTTP/1.1 206 \r\n"
        "content-length: 2\r\n"
        "content-range: bytes 0-1/2975717        \r\n"  // if content-range is present, retrieve total from it, not content-length
        "\r\n");

    KClientHttpResult *rslt;
    REQUIRE_RC ( KClientHttpRequestPOST ( m_req, & rslt ) );
    uint64_t size = 0;
    REQUIRE ( KClientHttpResultSize ( rslt, &size ) );
    REQUIRE_EQ ( (uint64_t)2975717, size );

    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}

FIXTURE_TEST_CASE( KClientHttpResult_FormatMsg, HttpFixture)
{
    KNSManagerMakeClientRequest ( m_mgr, &m_req, 0x01010000, & m_stream, MakeURL(GetName()).c_str()  );
    REQUIRE_RC ( KClientHttpRequestByteRange ( m_req, 0, 2 ) );

    TestStream::AddResponse(
        "HTTP/1.1 206 \r\n"
        "content-length: 2\r\n"
        "\r\n");
    string expected =
        "->HTTP/1.1 206 \n" // \r is gone
        "->content-length: 2\r\n";

    KClientHttpResult *rslt;
    REQUIRE_RC ( KClientHttpRequestPOST ( m_req, & rslt ) );
    KDataBuffer buffer;
    REQUIRE_RC ( KDataBufferMake( & buffer, 8, 0 ) );
    REQUIRE_RC ( KClientHttpResultFormatMsg ( rslt, & buffer, "->", "\n" ) );
    REQUIRE_EQ ( expected, string ((char*)buffer.base) );
    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(GET_WITHOUT_CONTENT_LENGHT, HttpFixture) {
    REQUIRE_RC(KNSManagerMakeHttpFile(m_mgr,
        (const KFile **)&m_file, NULL, 0x01010000,
        "http://ftp.ensembl.org/pub/data_files/homo_sapiens"
        "/GRCh38/rnaseq/GRCh38.illumina.brain.1.bam.bai"));

    uint64_t size;
    REQUIRE_RC(KFileSize(m_file, &size));

    char* buffer = new char[size];
    REQUIRE(buffer);

    size_t num_read = 0;
    REQUIRE_RC(KFileRead(m_file, 0, buffer, size, &num_read));
    REQUIRE_EQ(num_read, size);

    delete[](buffer);
}
#endif

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

    rc_t rc=HttpTestSuite(argc, argv);
    return rc;
}

}
