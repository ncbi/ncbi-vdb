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

#include <kns/manager.h>
#include <kns/kns-mgr-priv.h>

#include <kfs/file.h>

#include <sysalloc.h>
#include <stdexcept>
#include <cstring>

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
        if ( m_response.size() > bsize )
        {
            memcpy(buffer, m_response.c_str(), bsize);
            * num_read = bsize; 
            m_response = m_response.substr(bsize);
        }
        else
        {
            memcpy(buffer, m_response.c_str(), m_response.size());
            * num_read = m_response.size(); 
            m_response.clear();
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

    static void SetResponse ( const string& p_str )
    {
        m_response = p_str;
    }
    
    static string m_response;
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

string TestStream::m_response;

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
    }
    
    ~HttpFixture()
    {
        if ( m_mgr && KNSManagerRelease ( m_mgr ) != 0 )
            throw logic_error ( "HttpFixture: KNSManagerRelease failed" );
        if ( m_file && KFileRelease ( m_file ) != 0 )
            throw logic_error ( "HttpFixture: KFileRelease failed" );
    }
    
    KNSManager* m_mgr;
    KStream m_stream;
    KFile* m_file;
};

FIXTURE_TEST_CASE(HttpReliable_Make, HttpFixture)
{
    TestStream::SetResponse("HTTP/1.1 200 OK\nContent-Length: 7\n");
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, "http://abc.go.com/") ); 
    REQUIRE_NOT_NULL ( m_file ) ;
}

FIXTURE_TEST_CASE(HttpReliable_Read, HttpFixture)
{
    TestStream::SetResponse("HTTP/1.1 200 OK\nContent-Length: 7\n"); // response to HEAD
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, "http://abc.go.com/") ); 
    char buf[1024];
    size_t num_read;
    TestStream::SetResponse(    // response to GET
        "HTTP/1.1 206 Partial Content\n"
        "Transfer-Encoding: chunked\n"
        /*"Content-Length: 7\n" */ /* bug fix in KClientHttpResultHandleContentRange: used to break if Content-Length was not there */
        "Content-Range: bytes 0-6/7\n" 
        "\n"
        "7\n"
        "content\n"
    ); 
    REQUIRE_RC( KFileTimedRead ( m_file, 0, buf, sizeof buf, &num_read, NULL ) );
    REQUIRE_EQ( string ( "content" ), string ( buf, num_read ) );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

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

	// uncomment to see messages from KNS
    //KLogLevelSet(klogInfo);
	
	// this makes messages from the test code appear
	// (same as running the executable with "-l=message")
	// TestEnv::verbosity = LogLevel::e_message;
	
    rc_t rc=HttpTestSuite(argc, argv);
    return rc;
}

}
