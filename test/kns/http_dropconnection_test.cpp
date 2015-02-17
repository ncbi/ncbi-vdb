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

#include <klib/debug.h>
#include <klib/rc.h>

#include <kns/manager.h>
#include <kns/http.h>

#include <../libs/kns/stream-priv.h>
#include <../libs/kns/http-priv.h>

#include <kfs/file.h>

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

#if _DEBUGGING
#   define DBG_KNS_ON()  \
        KDbgSetModConds ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ), DBG_FLAG ( DBG_KNS_HTTP ) );          
#   define DBG_KNS_OFF() \
        KDbgSetModConds ( DBG_KNS, DBG_FLAG ( DBG_KNS_HTTP ), ~ DBG_FLAG ( DBG_KNS_HTTP ) );          
#else
#   define DBG_KNS_ON()
#   define DBG_KNS_OFF() 
#endif

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
        
        if ( response == "DROP" )
        {
            num_read = 0;
            return 0;
        }
        if ( response == "TIMEOUT" )
        {
            return RC ( rcNS, rcStream, rcReading, rcTimeout, rcExhausted );
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
        if ( m_file && KFileRelease ( m_file ) != 0 )
            throw logic_error ( "HttpFixture::~HttpFixture KFileRelease failed" );
            
        if ( ! TestStream::m_responses.empty() )
            throw logic_error ( "HttpFixture::~HttpFixture not all TestStream::m_responses have been consumed" );

        DBG_KNS_OFF();
        KNSManagerSetVerbose ( m_mgr, false );
        if ( m_mgr && KNSManagerRelease ( m_mgr ) != 0 )
            throw logic_error ( "HttpFixture::~HttpFixture KNSManagerRelease failed" );
    }
    
    static string MakeURL(const char* base)
    {
        return string("http://") + base + ".com/";
    }    
    static struct KStream * Reconnect ()
    {   
        return & m_stream; 
    }
    
    void TraceOn()
    {
        KNSManagerSetVerbose ( m_mgr, true );
        DBG_KNS_ON();
    }
    
    KNSManager* m_mgr;
    static KStream m_stream;
    KFile* m_file;
};

KStream HttpFixture::m_stream;

//////////////////////////
#if _DEBUGGING
FIXTURE_TEST_CASE(Http_Read_Drop, HttpFixture)
{
    SetClientHttpReopenCallback( Reconnect ); // NB. this hook is only available in DEBUG mode

    //TraceOn();

    TestStream::AddResponse("HTTP/1.1 200 OK\nContent-Length: 7\n"); // response to HEAD
    REQUIRE_RC ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) ); 
    
    char buf[1024];
    size_t num_read;
    TestStream::AddResponse( // broken response to GET
        "HTTP/1.1 206 Partial Content\n"
        "Transfer-Encoding: chunked\n"
        "Content-Range: bytes 0-6/7\n" 
        "\n");
    TestStream::AddResponse( // correct response to retry of GET
        "HTTP/1.1 206 Partial Content\n"
        "Transfer-Encoding: chunked\n"
        "Content-Range: bytes 0-6/7\n" 
        "\n"
        "7\n"
        "content\n"
    ); 
    
    REQUIRE_RC( KFileTimedRead ( m_file, 0, buf, sizeof buf, &num_read, NULL ) );
    REQUIRE_EQ( (size_t)7, num_read );
}
#endif

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
const char UsageDefaultName[] = "test-http-dropconn";

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
