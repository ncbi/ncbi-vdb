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

#include <kapp/args.h> // Args

#include <ktst/unit_test.hpp>

#include <klib/debug.h> /* KDbgSetString */
#include <klib/log.h>
#include <klib/rc.h>
#include <kfg/config.h>

#include <kns/adapt.h> /* KStreamFromKFilePair */
#include <kns/manager.h>
#include <kns/kns-mgr-priv.h>
#include <kns/http.h>

#include <../libs/kns/mgr-priv.h>
#include <../libs/kns/http-priv.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/defs.h>

#include <kproc/thread.h>

#include <sysalloc.h>
#include <stdexcept>
#include <cstring>
#include <list>
#include <sstream>

#define ALL

static rc_t argsHandler ( int argc, char * argv [] );
TEST_SUITE_WITH_ARGS_HANDLER ( HttpTestSuite, argsHandler );

using namespace std;
using namespace ncbi::NK;

class TestStream;
#define KSTREAM_IMPL TestStream
#include <kns/impl.h>

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)


class TestStream
{
public:
    static KStream_vt_v1 vt;

    static rc_t CC Whack ( KSTREAM_IMPL *self ) 
    { 
        if ( TestEnv::verbosity == LogLevel::e_message )
            cout << "TestStream::Whack() called" << endl;
        return 0; 
    }
    static rc_t CC Read ( const KSTREAM_IMPL *self, void *buffer, size_t bsize, size_t *num_read )
    { 
        if ( TestEnv::verbosity == LogLevel::e_message )
            cout << "TestStream::Read() called" << endl;
        * num_read = 0; 
        return 0; 
    }
    static rc_t CC Write ( KSTREAM_IMPL *self, const void *buffer, size_t size, size_t *num_writ )
    { 
        if ( TestEnv::verbosity == LogLevel::e_message )
            cout << "TestStream::Write() called" << endl;
        * num_writ = size; 
        return 0; 
    }
    static rc_t CC TimedRead ( const KSTREAM_IMPL *self, void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
    { 
        if ( TestEnv::verbosity == LogLevel::e_message )
            cout << "TestStream::TimedRead() called" << endl;
            
        string response;
        if ( m_responses.size()> 0)
        {
            response = m_responses.front();
            m_responses.pop_front();
        }
        
        if ( response.size() >= bsize )
        {
            memmove(buffer, response.c_str(), bsize);
            * num_read = bsize; 
            response = response.substr(bsize);
        }
        else
        {
            memmove(buffer, response.c_str(), response.size());
            * num_read = response.size();
            response.clear();
        }
        if ( TestEnv::verbosity == LogLevel::e_message )
            cout << "TestStream::TimedRead returned \"" << string((const char*)buffer, * num_read) << "\"" << endl;
        
        return 0; 
    }
    static rc_t CC TimedWrite ( KSTREAM_IMPL *self, const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm )
    { 
        if ( TestEnv::verbosity == LogLevel::e_message )
            cout << "TestStream::TimedWrite(\"" << string((const char*)buffer, size) << "\") called" << endl;
        * num_writ = size; 
        return 0; 
    }

    static void AddResponse ( const string& p_str, bool end_binary = false )
    {
        if (end_binary)
            m_responses.push_back(p_str);
        else
            m_responses.push_back(std::string(p_str.c_str(), p_str.size() + 1));
    }
    
    static list<string> m_responses;
};

KStream_vt_v1 TestStream::vt =
{
    1, 1,
    TestStream::Whack,
    TestStream::Read,
    TestStream::Write,
    TestStream::TimedRead,
    TestStream::TimedWrite
};

list<string> TestStream::m_responses;

class HttpFixture
{
public:
    HttpFixture()
    : m_mgr(0), m_file(0)
    {
        if ( KNSManagerMake ( & m_mgr ) != 0 )
            throw logic_error ( "HttpFixture: KNSManagerMake failed" );
                
        if ( KStreamInit ( & m_stream, ( const KStream_vt* ) & TestStream::vt, "TestStream", "", true, true ) != 0 )
            throw logic_error ( "HttpFixture: KStreamInit failed" );
            
        TestStream::m_responses.clear();
    }
    
    ~HttpFixture()
    {
        if ( m_mgr && KNSManagerRelease ( m_mgr ) != 0 )
            throw logic_error ( "HttpFixture::~HttpFixture KNSManagerRelease failed" );
            
        if ( m_file && KFileRelease ( m_file ) != 0 )
            throw logic_error ( "HttpFixture::~HttpFixture KFileRelease failed" );
            
        if ( ! TestStream::m_responses.empty() )
            throw logic_error ( "HttpFixture::~HttpFixture not all TestStream::m_responses have been consumed" );
    }
    
    KConfig* MakeConfig( const char* name, const char* contents )
    {
        KDirectory* wd;
        if ( KDirectoryNativeDir ( & wd ) != 0 )
            throw logic_error("KfgFixture: KDirectoryNativeDir failed");
            
        {
            KFile* file;    
            if (KDirectoryCreateFile(wd, &file, true, 0664, kcmInit, name) != 0)
                throw logic_error("MakeConfig: KDirectoryCreateFile failed");

            size_t num_writ=0;
            if (KFileWrite(file, 0, contents, strlen(contents), &num_writ) != 0)
                throw logic_error("MakeConfig: KFileWrite failed");

            if (KFileRelease(file) != 0)
                throw logic_error("MakeConfig: KFileRelease failed");    
        }
        
        KConfig* ret;
        {
            if (KConfigMake ( & ret, wd ) != 0) 
                throw logic_error("MakeConfig: KConfigMake failed");        
                
            KFile* file;    
            if (KDirectoryOpenFileRead(wd, (const KFile**)&file, name) != 0)
                throw logic_error("MakeConfig: KDirectoryOpenFileRead failed");
            
            if (KConfigLoadFile ( ret, name, file) != 0)
                throw logic_error("MakeConfig: KConfigLoadFile failed");

            if (KFileRelease(file) != 0)
                throw logic_error("MakeConfig: KFileRelease failed");
        }
        
        if (KDirectoryRemove(wd, true, name) != 0)
            throw logic_error("MakeConfig: KDirectoryRemove failed");
        if (KDirectoryRelease(wd) != 0)
            throw logic_error("MakeConfig: KDirectoryRelease failed");
        
        return ret;
    }
    
    static struct KStream * Reconnect ()
    {   
        return & m_stream; 
    }
    
    static string MakeURL(const char* base)
    {
        return string("http://") + base + ".com/";
    }    
    
    KNSManager* m_mgr;
    static KStream m_stream;
    KFile* m_file;
};

KStream HttpFixture::m_stream;

#ifdef ALL
//////////////////////////
// Regular HTTP 
FIXTURE_TEST_CASE(Http_Make, HttpFixture)
{
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}

#if 1
/*FIXME: 100 used to be retried regardless of whether URL is reliable, now it is not, so the test fails */
FIXTURE_TEST_CASE(Http_Make_Continue_100_Retry, HttpFixture)
{
    TestStream::AddResponse("HTTP/1.1 100 continue\r\n");
    TestStream::AddResponse("HTTP/1.1 100 continue\r\n");
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}
#endif

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

struct ReadThreadData
{
    int tid;
    int num_threads;
    int num_requests;
    int content_length;
    const KFile * kHttpFile;
    const char ** contents;
};

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

FIXTURE_TEST_CASE(HttpRequest_POST_NoParams, HttpFixture)
{   // Bug: KClientHttpRequestPOST crashed if request had no parameters
    KClientHttpRequest *req;
    KNSManagerMakeClientRequest ( m_mgr, &req, 0x01010000, & m_stream, MakeURL(GetName()).c_str()  );

    KClientHttpResult *rslt;
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n");
    REQUIRE_RC ( KClientHttpRequestPOST ( req, & rslt ) );
    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );

    REQUIRE_RC ( KClientHttpRequestRelease ( req ) );
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
            throw logic_error ( "RetrierFixture::~RetrierFixture KHttpRetrierDestroy failed" );
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
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str()  ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}
#if 0
/* 100 used to be retried regardless, now it is not, so the test fails */
FIXTURE_TEST_CASE(HttpReliable_Make_Continue_100_Retry, HttpFixture)
{
    TestStream::AddResponse("HTTP/1.1 100 continue\r\n");
    TestStream::AddResponse("HTTP/1.1 100 continue\r\n");
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str()  ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}
#endif

FIXTURE_TEST_CASE(HttpReliable_Make_5xx_retry, HttpFixture)
{   // use default configuration for 5xx to be retried
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str()  ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}

FIXTURE_TEST_CASE(HttpReliable_Make_500_Fail, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/500=\"\"\n"); // do not retry 500
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    REQUIRE_RC_FAIL ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str()  ) );
}

FIXTURE_TEST_CASE(HttpReliable_Make_500_Retry, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/500=\"0+\"\n" "http/reliable/5xx=\"\"\n"); // do not retry 5xx, retry 500
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n");
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str()  ) );
    REQUIRE_NOT_NULL ( m_file ) ;
}

FIXTURE_TEST_CASE(HttpReliable_Make_500_TooManyRetries, RetrierFixture)
{
    Configure ( GetName(), "http/reliable/500=\"0\"\n" "http/reliable/5xx=\"\"\n"); // do not retry 5xx, retry 500 once
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n");
    REQUIRE_RC_FAIL ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str()  ) );
}

#ifdef _DEBUGGING
FIXTURE_TEST_CASE(HttpReliable_Read_Retry, HttpFixture)
{
    SetClientHttpReopenCallback ( Reconnect ); // this hook is only available in DEBUG mode

    TestStream::AddResponse("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 7\r\n"); // response to HEAD
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str()  ) );
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

//////////////////////////
// Reliable HTTP request
FIXTURE_TEST_CASE(HttpReliableRequest_Make, HttpFixture)
{
    KClientHttpRequest *req;
    KNSManagerMakeReliableClientRequest ( m_mgr, &req, 0x01010000, & m_stream, MakeURL(GetName()).c_str()  );
    REQUIRE_NOT_NULL ( req ) ;
    REQUIRE_RC ( KClientHttpRequestRelease ( req ) );
}

FIXTURE_TEST_CASE(HttpReliableRequest_POST_5xx_retry, HttpFixture)
{   // use default configuration for 5xx to be retried
    KClientHttpRequest *req;
    KNSManagerMakeReliableClientRequest ( m_mgr, &req, 0x01010000, & m_stream, MakeURL(GetName()).c_str()  );

    TestStream::AddResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n"); // response to GET
    TestStream::AddResponse("HTTP/1.1 200 OK\r\n");

    KClientHttpResult *rslt;
    REQUIRE_RC ( KClientHttpRequestPOST ( req, & rslt ) );

    REQUIRE_RC ( KClientHttpResultRelease ( rslt ) );
    REQUIRE_RC ( KClientHttpRequestRelease ( req ) );
}

#endif

#ifdef ALL
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
        NULL, "https://www.ncbi.nlm.nih.gov/Traces/names/names.fcgi" ) ); 
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
        NULL, "https://www.ncbi.nlm.nih.gov/Traces/names/bad.cgi" ) ); 
    REQUIRE_RC ( KHttpRequestAddPostParam ( req, "acc=AAAB01" ) );
    REQUIRE_RC ( KHttpRequestPOST ( req, & rslt ) );
    REQUIRE_RC ( KClientHttpResultStatus ( rslt, & code, NULL, 0, NULL ) );
    REQUIRE_EQ ( code, 404u );
    RELEASE ( KHttpResult, rslt );
    RELEASE ( KHttpRequest, req );

    RELEASE ( KNSManager, kns );
    REQUIRE_RC ( rc );
}
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

TEST_CASE ( RepeatedHeader ) {
    rc_t rc = 0;
    KDirectory * dir = NULL;
    REQUIRE_RC ( KDirectoryNativeDir ( & dir ) );

    const KFile * f = NULL;
    REQUIRE_RC ( KDirectoryOpenFileRead ( dir, & f, "double_header.txt" ) );

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
{
    rc_t rc = 0;
    
    KNSManager * mgr = NULL;
    REQUIRE_RC ( KNSManagerMake ( & mgr ) );

    // capture prior settings
    bool allow_all_certs = false;
    REQUIRE_RC ( KNSManagerGetAllowAllCerts ( mgr, & allow_all_certs ) );

    // ensure that manager is NOT allowing, i.e. default behavior
    REQUIRE_RC ( KNSManagerSetAllowAllCerts ( mgr, false ) );

    // create a connection to something that normally fails; it should fail
    String host;
    CONST_STRING ( & host, "www.google.com" );

    // capture log level
    KLogLevel log_level = KLogLevelGet ();
    
    // set log level to practically silent
    KLogLevelSet ( 0 );
    
    KClientHttp * https = NULL;
    REQUIRE ( KNSManagerMakeClientHttps
              ( mgr, & https, NULL, 0x01010000, & host, 443 ) != 0 );
    RELEASE ( KClientHttp, https );

    // restore log level
    KLogLevelSet ( log_level );

    // tell manager to allow all certs
    REQUIRE_RC ( KNSManagerSetAllowAllCerts ( mgr, true ) );

    // repeat connection to something that normally fails; it should succeed
    REQUIRE_RC ( KNSManagerMakeClientHttps
              ( mgr, & https, NULL, 0x01010000, & host, 443 ) );
    RELEASE ( KClientHttp, https );

    // restore prior settings
    REQUIRE_RC ( KNSManagerSetAllowAllCerts ( mgr, allow_all_certs ) );
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
	
    rc_t rc=HttpTestSuite(argc, argv);
    return rc;
}

}
