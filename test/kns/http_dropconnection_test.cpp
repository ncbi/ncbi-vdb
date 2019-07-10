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

#include <kapp/args.h> /* Args */

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

static rc_t argsHandler(int argc, char* argv[]) {
    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

TEST_SUITE_WITH_ARGS_HANDLER(HttpTestSuite, argsHandler)

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

static const string Response_HEAD_OK = "HTTP/1.1 200 OK\nAccept-Ranges: bytes\nContent-Length: 7\n";
static const string Response_GET_Content = "HTTP/1.1 206 Partial Content\n"
                                            "Accept-Ranges: bytes\n"
                                            "Transfer-Encoding: chunked\n"
                                            "Content-Range: bytes 0-6/7\n"
                                            "\n"
                                            "7\n"
                                            "content\n";

static rc_t RC_TransferIncomplete = SILENT_RC ( rcNS, rcFile, rcWriting, rcTransfer, rcIncomplete );
static rc_t RC_Timeout = SILENT_RC ( rcNS, rcStream, rcReading, rcTimeout, rcExhausted );

//
// these implementations of TestStream/HttpFixture look very different from the ones in HttpFixture.hpp/cpp, so
// leaving them here as they are for now. In the future, consider merging.
//
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
        throw logic_error ( "TestStream::<non-timed>Read called" );
    }
    static rc_t CC Write ( KSTREAM_IMPL *self, const void *buffer, size_t size, size_t *num_writ )
    {
        throw logic_error ( "TestStream::<non-timed>Write called" );
    }
    static rc_t CC TimedRead ( const KSTREAM_IMPL *self, void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
    {
        if ( TestEnv::verbosity == LogLevel::e_message )
            cout << "TestStream::TimedRead() called" << endl;

        string response;
        if ( m_readResponses.size()> 0)
        {
            response = m_readResponses.front();
            m_readResponses.pop_front();
        }
        else
        {
            throw logic_error ( "TestStream::TimedRead: out of responses" );
        }

        if ( response == "DROP" )
        {
            num_read = 0;
            return 0;
        }
        if ( response == "TIMEOUT" )
        {
            return RC_Timeout;
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

        rc_t response;
        if ( m_writeResponses.size()> 0)
        {
            response = m_writeResponses.front();
            m_writeResponses.pop_front();
        }
        else
        {
            throw logic_error ( "TestStream::TimedWrite: out of responses" );
        }

        if ( response == 0 )
        {
            * num_writ = size;
            return 0;
        }
        else
        {
            * num_writ = 0;
            return response;
        }
    }

    static void AddReadResponse ( const string& p_str )
    {
        m_readResponses.push_back(p_str);
    }

    static void AddWriteRC ( rc_t p_rc )
    {
        m_writeResponses.push_back(p_rc);
    }

    static list<string> m_readResponses;
    static list<rc_t> m_writeResponses;
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

list<string> TestStream::m_readResponses;
list<rc_t> TestStream::m_writeResponses;

class HttpFixture
{
public:
    HttpFixture()
    : m_mgr(0), m_file(0), m_numRead(0)
    {
        if ( KNSManagerMake ( & m_mgr ) != 0 )
            throw logic_error ( "HttpFixture: KNSManagerMake failed" );

        if ( KStreamInit ( & m_stream, ( const KStream_vt* ) & TestStream::vt, "TestStream", "", true, true ) != 0 )
            throw logic_error ( "HttpFixture: KStreamInit failed" );

        TestStream::m_readResponses.clear();

        m_reconnected = false;

#if _DEBUGGING
    SetClientHttpReopenCallback( Reconnect ); // NB. this hook is only available in DEBUG mode
#endif
    }

    ~HttpFixture()
    {
        if ( m_file && KFileRelease ( m_file ) != 0 )
        {
            cout << "HttpFixture::~HttpFixture KFileRelease failed" << endl;
        }

        //if ( ! TestStream::m_readResponses.empty() )
        //    throw logic_error ( "HttpFixture::~HttpFixture not all TestStream::m_readResponses have been consumed" );

        DBG_KNS_OFF();
        KNSManagerSetVerbose ( m_mgr, false );
        if ( m_mgr && KNSManagerRelease ( m_mgr ) != 0 )
        {
            cout << "HttpFixture::~HttpFixture KNSManagerRelease failed" << endl;
        }
    }

    static string MakeURL(const char* base)
    {
        return string("http://") + base + ".com/";
    }
    static struct KStream * Reconnect ()
    {
        m_reconnected = true;
        return & m_stream;
    }

    void TraceOn()
    {
        KNSManagerSetVerbose ( m_mgr, true );
        DBG_KNS_ON();
    }

    void SendReceiveHEAD(const char* p_url)
    {
        TestStream::AddWriteRC(0); // send HEAD succeeds
        TestStream::AddReadResponse ( Response_HEAD_OK );
        if ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL ( p_url ) . c_str () ) != 0 )
            throw logic_error ( "HttpFixture::SendReceiveHEAD KNSManagerMakeHttpFile failed" );
    }


    static KStream m_stream;
    static bool m_reconnected;

    KNSManager* m_mgr;
    KFile* m_file;
    char m_buf[1024];
    size_t m_numRead;
};

KStream HttpFixture::m_stream;
bool HttpFixture::m_reconnected = false;

//////////////////////////

// the tests involving reconnection require Reconnect hook, thus cannot be run in Release build
#define CAN_USE_RECONNECT_HOOK _DEBUGGING

FIXTURE_TEST_CASE(Http_Normal, HttpFixture)
{
    TestStream::AddWriteRC ( 0 ); // send HEAD succeeds
    TestStream::AddReadResponse ( Response_HEAD_OK );
    REQUIRE_RC ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );

    TestStream::AddWriteRC ( 0 ); // send GET succeeds
    TestStream::AddReadResponse( Response_GET_Content );

    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, &m_numRead, NULL ) );
    REQUIRE_EQ( string("content"), string(m_buf, m_numRead) );
}

// problems with HEAD

FIXTURE_TEST_CASE(HEAD_BadResponse, HttpFixture)
{
    TestStream::AddWriteRC(0); // send HEAD succeeds
    TestStream::AddReadResponse("garbage"); // bad response to HEAD
    // KNS tries to connect twice
    TestStream::AddWriteRC(0); // send HEAD succeeds
    TestStream::AddReadResponse("garbage"); // bad response to HEAD

    REQUIRE_RC_FAIL ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );
}

FIXTURE_TEST_CASE(HEAD_BrokenConnection, HttpFixture)
{
    TestStream::AddWriteRC(0); // send HEAD succeeds
    TestStream::AddReadResponse(""); // simulates reaction to POLLHUP/POLLRDHUP
    // KNS tries to connect twice
    TestStream::AddWriteRC(0); // send HEAD succeeds
    TestStream::AddReadResponse(""); // simulates reaction to POLLHUP/POLLRDHUP

    REQUIRE_RC_FAIL ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );
}

#if CAN_USE_RECONNECT_HOOK
FIXTURE_TEST_CASE(HEAD_Invalid_Reconnect_Succeed, HttpFixture)
{
    TestStream::AddWriteRC ( RC_TransferIncomplete ); // first send HEAD fails
    TestStream::AddWriteRC ( 0 ); // retry send HEAD succeeds
    TestStream::AddReadResponse ( Response_HEAD_OK );
    REQUIRE_RC ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );
    REQUIRE ( m_reconnected );

    // make sure GET works after reconnection
    TestStream::AddWriteRC ( 0 ); // send GET succeeds
    TestStream::AddReadResponse( Response_GET_Content );

    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, &m_numRead, NULL ) );
    REQUIRE_EQ( string("content"), string(m_buf, m_numRead) );
}
#endif

#if CAN_USE_RECONNECT_HOOK
FIXTURE_TEST_CASE(HEAD_Invalid_Reconnect_Fail, HttpFixture)
{
    // KNS tries each HEAD twice, each attempt tries to write to the stream write
    TestStream::AddWriteRC( RC_TransferIncomplete ); // first send HEAD fails once
    TestStream::AddWriteRC( RC_TransferIncomplete ); // first send HEAD fails twice
    TestStream::AddWriteRC( RC_TransferIncomplete ); // retry send HEAD fails once
    TestStream::AddWriteRC( RC_TransferIncomplete ); // retry send HEAD fails twice
    REQUIRE_RC_FAIL ( KNSManagerMakeHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, MakeURL(GetName()).c_str() ) );
    REQUIRE ( m_reconnected );
}
#endif

// problems with GET

#if CAN_USE_RECONNECT_HOOK
FIXTURE_TEST_CASE(GET_Invalid_Reconnect_Succeed, HttpFixture)
{
    SendReceiveHEAD(GetName());

    TestStream::AddWriteRC(0); // send GET succeeds
    TestStream::AddReadResponse( // broken response to GET
        "HTTP/1.1 206 Partial Content\n"
        "Transfer-Encoding: chunked\n"
        "Content-Range: bytes 0-6/7\n"
        "\n");
    TestStream::AddWriteRC(0); // retry GET succeeds
    TestStream::AddReadResponse( Response_GET_Content );

    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, &m_numRead, NULL ) );
    REQUIRE_EQ( string("content"), string(m_buf, m_numRead) );

    REQUIRE ( m_reconnected );
}
#endif

#if CAN_USE_RECONNECT_HOOK
FIXTURE_TEST_CASE(GET_Failed_Reconnect_Succeed, HttpFixture)
{
    SendReceiveHEAD(GetName());

    TestStream::AddWriteRC( RC_TransferIncomplete ); // send GET fails
    TestStream::AddWriteRC(0); // retry send GET succeeds
    TestStream::AddReadResponse( Response_GET_Content );

    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, &m_numRead, NULL ) );
    REQUIRE_EQ( string("content"), string(m_buf, m_numRead) );

    REQUIRE ( m_reconnected );
}
#endif

#if CAN_USE_RECONNECT_HOOK
FIXTURE_TEST_CASE(GET_Timedout_Reconnect_Succeed, HttpFixture)
{
    SendReceiveHEAD(GetName());

    TestStream::AddWriteRC(0); // send GET succeeds
    TestStream::AddReadResponse( "TIMEOUT" ); // response to GET times out, first attempt
    TestStream::AddWriteRC(0); // send GET succeeds
    TestStream::AddReadResponse( "TIMEOUT" ); // response to GET times out, second attempt
    TestStream::AddWriteRC(0); // retry GET succeeds
    TestStream::AddReadResponse( Response_GET_Content );
    REQUIRE_RC ( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, &m_numRead, NULL ) );
    REQUIRE_EQ( string("content"), string(m_buf, m_numRead) );

    REQUIRE ( m_reconnected );
}
#endif

#if CAN_USE_RECONNECT_HOOK
FIXTURE_TEST_CASE(GET_Read_Failed_Reconnect_Failed, HttpFixture)
{
    SendReceiveHEAD(GetName());

    // firsty try
    TestStream::AddWriteRC(0); // send GET succeeds
    TestStream::AddReadResponse( "TIMEOUT" ); // response to GET times out, first attempt
    TestStream::AddWriteRC(0); // send GET succeeds
    TestStream::AddReadResponse( "TIMEOUT" ); // response to GET times out, second attempt
    // atempt to reconnect
    TestStream::AddWriteRC(0); // send GET succeeds
    TestStream::AddReadResponse( "TIMEOUT" ); // response to GET times out, first attempt
    TestStream::AddWriteRC(0); // send GET succeeds
    TestStream::AddReadResponse( "TIMEOUT" ); // response to GET times out, second attempt
    REQUIRE_RC_FAIL ( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, &m_numRead, NULL ) );

    REQUIRE ( m_reconnected );
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
