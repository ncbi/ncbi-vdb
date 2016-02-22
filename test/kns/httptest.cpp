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

/**
* Unit tests for HTTP interfaces
*/

#include <ktst/unit_test.hpp>

#include <klib/log.h>

#include <kfg/config.h>

#include <kns/manager.h>
#include <kns/kns-mgr-priv.h>
#include <kns/http.h>

#include <../libs/kns/mgr-priv.h>
#include <../libs/kns/http-priv.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/defs.h>

#include <sysalloc.h>
#include <stdexcept>
#include <cstring>
#include <list>

TEST_SUITE(HttpTestSuite);

using namespace std;
using namespace ncbi::NK;

class TestStream;
#define KSTREAM_IMPL TestStream
#include <kns/impl.h>

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
            memcpy(buffer, response.c_str(), bsize);
            * num_read = bsize; 
            response = response.substr(bsize);
        }
        else
        {
            memcpy(buffer, response.c_str(), response.size());
            * num_read = response.size() + 1;  // include 0-terminator
            ( ( char * ) buffer ) [ * num_read - 1 ] = 0;
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

    static void AddResponse ( const string& p_str )
    {
        m_responses.push_back(p_str);
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
        "content\r\n"
    ); 
    REQUIRE_RC( KFileTimedRead ( m_file, 0, buf, sizeof buf, &num_read, NULL ) );
    REQUIRE_EQ( string ( "content" ), string ( buf, num_read ) );
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

//////////////////////////////////////////// Main
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
    KConfigDisableUserSettings();

	// this makes messages from the test code appear
	// (same as running the executable with "-l=message")
	// TestEnv::verbosity = LogLevel::e_message;
	
    rc_t rc=HttpTestSuite(argc, argv);
    return rc;
}

}
