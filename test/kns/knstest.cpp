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
* Unit tests for KNS interfaces
*/

#include <ktst/unit_test.hpp>

#include <klib/rc.h>
#include <klib/log.h>

#include <klib/printf.h>

#include <kproc/timeout.h>

#include <kns/manager.h>
#include <kns/endpoint.h>
#include <kns/stream.h>
#include <kns/adapt.h>
#include <kns/socket.h>

#include <kfs/directory.h>
#include <kfs/file.h>

#include <kapp/args.h> /* ArgsMakeAndHandle */

#include <kproc/thread.h>
#include <kproc/timeout.h>

#include <os-native.h>

#include <sysalloc.h>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <sstream>

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(KnsTestSuite, argsHandler);

using namespace std;
using namespace ncbi::NK;

//////////////////////////////////////////// KStream
class KnsStreamFixture
{
public:
    KnsStreamFixture ()
        : dir ( 0 ), strm ( 0 ), file ( 0 ), name ("./adapter-test.txt")
        {
            write_buffer = "something nice";

            THROW_ON_RC (  KDirectoryNativeDir ( &dir ) );
            KDirectoryRemove ( dir, true, name );
            THROW_ON_RC ( KDirectoryCreateFile ( dir, &file, false, 0664, kcmCreate, name ) );
        }
    ~KnsStreamFixture ()
        {
            if ( strm )
                KStreamRelease ( strm );
            if ( file )
                KFileRelease ( file );
            if ( dir )
            {
                KDirectoryRemove ( dir, true, name );
                KDirectoryRelease ( dir );
            }
        }

    KDirectory *dir;
    KStream *strm;
    KFile *file;
    size_t numRead, numWrit;
    const char *write_buffer;
    char read_buffer [ 256 ];
    const char *name;
};

FIXTURE_TEST_CASE ( KStreamAdaptersFileWriteOnly, KnsStreamFixture )
{

    REQUIRE_RC ( KStreamFromKFilePair ( & strm, NULL, file ) );
    REQUIRE_RC ( KStreamWrite ( strm, write_buffer, strlen ( write_buffer ), & numWrit ) );

    KFileRelease ( file );

    REQUIRE_RC ( KDirectoryOpenFileRead ( dir, ( const KFile ** ) & file, name ) );
    REQUIRE_RC ( KFileRead ( file, 0, read_buffer, sizeof read_buffer, & numRead ) );

    REQUIRE_EQ ( numRead, numWrit );
    REQUIRE_EQ ( strncmp ( write_buffer, read_buffer, numRead ), 0 );

}

FIXTURE_TEST_CASE ( KStreamAdaptersFileReadOnly, KnsStreamFixture )
{

    REQUIRE_RC ( KFileWrite ( file, 0,  write_buffer, strlen ( write_buffer ), &numWrit ) );

    KFileRelease ( file );

    REQUIRE_RC ( KDirectoryOpenFileRead ( dir, ( const KFile ** ) & file, name ) );
    REQUIRE_RC ( KStreamFromKFilePair ( & strm, file, NULL ) );
    REQUIRE_RC ( KStreamRead ( strm, read_buffer, sizeof read_buffer, & numRead ) );

    REQUIRE_EQ ( numRead, numWrit );
    REQUIRE_EQ ( strncmp ( write_buffer, read_buffer, numRead ), 0 );
}

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// IPC connections

TEST_CASE(KnsManagerMakeRelease)
{
    KNSManager* mgr;
    REQUIRE_RC(KNSManagerMake(&mgr));
    REQUIRE_NOT_NULL(mgr);
    REQUIRE_RC(KNSManagerRelease(mgr));
}

TEST_CASE(KnsManagerSets)
{
    KNSManager* mgr;
    REQUIRE_RC(KNSManagerMake(&mgr));
    REQUIRE_NOT_NULL(mgr);

    const char * ua=NULL;
    KNSManagerGetUserAgent(&ua);
    fprintf ( stderr, "UA #1 is '%s'\n", ua);

    REQUIRE_RC(KNSManagerSetClientIP(mgr,"1.2.3.4"));
    REQUIRE_RC(KNSManagerSetSessionID(mgr,"0x123"));
    REQUIRE_RC(KNSManagerSetPageHitID(mgr,"abcd"));
    KNSManagerGetUserAgent(&ua);
    fprintf ( stderr, "UA #2 is '%s'\n", ua);

    REQUIRE_RC_FAIL(KNSManagerSetClientIP(NULL, NULL));
    REQUIRE_RC_FAIL(KNSManagerSetClientIP(mgr, NULL));
    REQUIRE_RC(KNSManagerSetClientIP(mgr, "5.67.89.12"));
    KNSManagerGetUserAgent(&ua);
    fprintf ( stderr, "UA #3 is '%s'\n", ua);

    char toobig[8192];
    memset(toobig,'X',sizeof toobig);
    toobig[sizeof toobig-1]='\0';
    REQUIRE_RC(KNSManagerSetClientIP(mgr, toobig));
    REQUIRE_RC(KNSManagerSetSessionID(mgr, toobig));
    REQUIRE_RC(KNSManagerSetPageHitID(mgr, toobig));

    REQUIRE_RC(KNSManagerRelease(mgr));
}

class KnsManagerFixture
{
public:
    KnsManagerFixture()
    : mgr(0)
    {
        THROW_ON_RC ( KNSManagerMake(&mgr) );
    }
    ~KnsManagerFixture()
    {
        if (mgr)
            KNSManagerRelease(mgr);
    }

    KNSManager* mgr;

    KEndPoint ep;
    String name;
};

FIXTURE_TEST_CASE(IPCEndpoint_Create, KnsManagerFixture)
{
    StringInitCString(&name, GetName());
    REQUIRE_RC(KNSManagerInitIPCEndpoint(mgr, &ep, &name));
    REQUIRE_EQ(ep.type, (KEndPointType)epIPC);
    REQUIRE_EQ(string(ep.u.ipc_name), string(GetName()));
}

FIXTURE_TEST_CASE(MakeListener, KnsManagerFixture)
{
    CONST_STRING(&name, "socket");
    REQUIRE_RC(KNSManagerInitIPCEndpoint(mgr, &ep, &name));
    KListener* listener;
    REQUIRE_RC(KNSManagerMakeListener( mgr, &listener, &ep ));
    REQUIRE_NOT_NULL(listener);
    KListenerRelease(listener);
}

FIXTURE_TEST_CASE(MakeIPCConnection_NoListener, KnsManagerFixture)
{
    CONST_STRING(&name, "socket");
    REQUIRE_RC(KNSManagerInitIPCEndpoint(mgr, &ep, &name));
    timeout_t tm;
    TimeoutInit ( & tm, 0 );
    KSocket* socket;
    REQUIRE_RC_FAIL(KNSManagerMakeRetryConnection(mgr, &socket, &tm, NULL, &ep)); /* no server; no retries */
    REQUIRE_NULL(socket);
}

//////////////////////////////////////////// IPC, non-timed reads and writes

const string SocketName = string("knstest") + TestEnv::GetPidString();

class SocketFixture : public KnsManagerFixture
{
// Sets up a server thread. The server thread will:
// - wait for an incoming message,
// - upon receiving the message, convert it to upper case
// - send the converted message back
// - wait for an incoming message "done"
// - shut down the IPC connection

// Test cases' bodies represent client logic
// The fixture on the client will send "done" from the destructor and close its IPC connection

public:
    typedef rc_t (*WorkerThreadFn) ( const KThread *self, void *data );

	static const size_t MaxMessageSize = 256; // for this suite, we will agree not to send messages longer that this

public:
    SocketFixture()
    : server(0), listener(0), threadWorker(0)
    {
        StringInitCString(&name, SocketName.c_str());
        THROW_ON_RC (KNSManagerInitIPCEndpoint(mgr, &ep, &name) );

        if ( ! TestEnv::in_child_process)
        {   // start a server thread
            LOG(LogLevel::e_message, "starting a server" << endl);

            if (KThreadMake ( &server, ServerThreadFn, this ) != 0 || server == 0)
                throw logic_error ( "SocketFixture: KThreadMake failed" );
        }
        else
            throw logic_error ( "SocketFixture() called from child" );
    }
    ~SocketFixture()
    {
        if ( ! TestEnv::in_child_process)
        {
            if (server)
            {
                LOG(LogLevel::e_message, "server stopping" << endl);

                KThreadCancel(server);
                KThreadWait(server, NULL);
                KThreadRelease(server);
                KThreadRelease(server);/* for some reason KThread is initialized with refcout = 2 */

                if (listener)
                {   /* shutdown the (possibly blocked) listener */
                    LOG(LogLevel::e_message, "server releasing the listener" << endl);
                    KListenerRelease(listener);
                }
            }
        }
        else
            cout << "~SocketFixture() called from child" << endl;
    }
    static rc_t ServerThreadFn ( const KThread *self, void *data )
    {
        try
        {
            SocketFixture* me = (SocketFixture*)data;
            return me->Listen();
        }
        catch (const exception& ex)
        {
            cout << "SocketFixture server thread threw " << ex.what() << endl;
            throw;
        }
    }

    static string ToUpper(const string& str)
    {
        string ret(str);
        transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
        return ret;
    }

    rc_t Listen()
    {
        if (KNSManagerMakeListener ( mgr, &listener, &ep ) == 0)
        {
            rc_t rc = 0;
            while (rc == 0)
            {
                LOG(LogLevel::e_message, "server listening" << endl);
                KSocket* socket;
                rc = KListenerAccept ( listener, &socket ); // may not return from here if no more incoming connections for this test case
                if (rc == 0)
                {
                    KStream* stream;
                    THROW_ON_RC ( KSocketGetStream ( socket, & stream ) );
                    THROW_ON_RC ( KSocketRelease ( socket ) );

                    LOG(LogLevel::e_message, "server detected connection, starting worker" << endl);
                    KThread* worker;
                    if (KThreadMake ( &worker, threadWorker == 0 ? DefaultWorkerThreadFn : threadWorker, stream) != 0 || worker == 0)
                        throw logic_error ( "SocketFixture: KThreadMake failed" );
                }
            }
            LOG(LogLevel::e_message, "server  exiting" << endl);
            return rc;
        }
        else
            throw logic_error ( "SocketFixture: KNSMakeListener failed" );
    }

    static rc_t DefaultWorkerThreadFn ( const KThread *self, void *data )
    {
        try
        {   // this server worker converts the incoming message to all uppercase and sends it back
            LOG(LogLevel::e_message, "worker "  << (void*)self << " starting" << endl);

            KStream* stream = (KStream*)data;
            char localBuf[MaxMessageSize];
            size_t num;
            THROW_ON_RC (KStreamTimedRead(stream, localBuf, sizeof(localBuf), &num, NULL) ); // wait forever
            LOG(LogLevel::e_message, "worker "  << (void*)self << " after KStreamRead(" << string(localBuf, num) << ")" << endl);

            for (size_t i = 0 ; i < num; ++i)
                localBuf[i] = toupper(localBuf[i]);

            THROW_ON_RC ( KStreamWrite(stream, localBuf, num, &num) );
            LOG(LogLevel::e_message, "worker "  << (void*)self << " after KStreamWrite" << endl);

            // wait until the reader says "done"
            THROW_ON_RC ( KStreamTimedRead(stream, localBuf, sizeof(localBuf), &num, NULL) ); // wait forever

            string doneMsg(localBuf, num);
            LOG(LogLevel::e_message, "worker "  << (void*)self << " after KStreamRead = '" << doneMsg << "'" << endl);
            if (doneMsg != "done")
                throw logic_error ( "SocketFixture worker: out of sequence message received: '" + doneMsg + "'" );

            LOG(LogLevel::e_message, "worker "  << (void*)self << " after KStreamRelease" << endl);

            THROW_ON_RC ( KStreamRelease(stream) );
            LOG(LogLevel::e_message, "worker "  << (void*)self << " after KStreamRelease" << endl);
        }
        catch (const exception& ex)
        {
            cout << "SocketFixture worker thread threw " << ex.what() << endl;
            throw;
        }
        LOG(LogLevel::e_message, "worker "  << (void*)self << " exiting" << endl);
        return KThreadRelease(self);
    }

    void CloseClientStream(KStream* p_stream)
    {
        if (p_stream != 0)
        {
            // signal to server to shut down the connection
            string done("done");
            size_t num;
            THROW_ON_RC ( KStreamTimedWrite(p_stream, done.c_str(), done.length(), &num, NULL) );
            THROW_ON_RC ( KStreamRelease(p_stream) );
        }
    }

    KStream* MakeStream( int32_t p_retryTimeout )
    {
        timeout_t tm;
        TimeoutInit ( & tm, p_retryTimeout );

        KSocket* socket;
        THROW_ON_RC ( KNSManagerMakeRetryConnection(mgr, &socket, &tm, NULL, &ep) );
        if (socket == 0)
           throw logic_error ( "MakeStream: KStreamRelease failed" );

        KStream* stream;
        THROW_ON_RC ( KSocketGetStream ( socket, & stream ) );
        if (stream == 0)
           throw logic_error ( "MakeStream: KStreamRelease failed" );
        KSocketRelease ( socket );

        return stream;
    }

    KThread* server;

    // created by the listener thread
    KListener* listener;

    // may be set by subclasses
    WorkerThreadFn threadWorker;

    // for use in test cases
    size_t num;
    char buf[MaxMessageSize];
};

PROCESS_FIXTURE_TEST_CASE(IPCEndpoint_Basic, SocketFixture, 0, 5)
{   // client runs in a child process
    string content = GetName();

    KStream* stream = MakeStream ( 50 * 1000 ); /* this might make some retries while the server is setting up */
    LOG(LogLevel::e_message, "client '" << GetName() << "' after KNSMakeConnection" << endl);

    REQUIRE_RC(KStreamWrite(stream, content.c_str(), content.length(), &num));
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    REQUIRE_RC(KStreamTimedRead(stream, buf, sizeof(buf), &num, NULL));
    LOG(LogLevel::e_message, "client after KStreamRead" << endl);
    REQUIRE_EQ(string(buf, num), ToUpper(content));

    CloseClientStream(stream);
}

PROCESS_FIXTURE_TEST_CASE(IPCEndpoint_MultipleListeners, SocketFixture, 0, 100)
{   // client runs in a child process

    KStream* stream = MakeStream ( 50 * 1000 ); /* this might make some retries while the server is setting up */
    LOG(LogLevel::e_message, "client '" << GetName() << "' after KNSMakeConnection1" << endl);

    TestEnv::Sleep(1); // on Windows 32, when the two calls to KNSManagerMakeConnection follow too closely, sometimes things get messed up

    KStream* stream2 = MakeStream ( 5 * 1000 ); /* should work from the first try now*/
    LOG(LogLevel::e_message, "client '" << GetName() << "' after KNSMakeConnection2" << endl);

    string content = string(GetName())+"_1";
    REQUIRE_RC(KStreamWrite(stream, content.c_str(), content.length(), &num));
    LOG(LogLevel::e_message, "client after KStreamWrite1" << endl);
    REQUIRE_EQ(content.length(), num);

    string content2(string(GetName())+"_2");
    REQUIRE_RC(KStreamWrite(stream2, content2.c_str(), content2.length(), &num));
    LOG(LogLevel::e_message, "client after KStreamWrite2" << endl);
    REQUIRE_EQ(content2.length(), num);

    REQUIRE_RC(KStreamTimedRead(stream2, buf, sizeof(buf), &num, NULL));
    LOG(LogLevel::e_message, "client after KStreamRead2" << endl);
    REQUIRE_EQ(string(buf, num), ToUpper(content2));

    REQUIRE_RC(KStreamTimedRead(stream, buf, sizeof(buf), &num, NULL));
    LOG(LogLevel::e_message, "client after KStreamRead1" << endl);
    REQUIRE_EQ(string(buf, num), ToUpper(content));

    CloseClientStream(stream);
    CloseClientStream(stream2);
}

PROCESS_FIXTURE_TEST_CASE(IPCEndpoint_ReadAll, SocketFixture, 0, 5)
{   // call ReadAll requesting more bytes than available, see it return only what is available
    string content = GetName();

    KStream* stream = MakeStream ( 5 * 1000 );
    LOG(LogLevel::e_message, "client '" << GetName() << "' after KNSMakeConnection" << endl);

    REQUIRE_RC(KStreamWrite(stream, content.c_str(), content.length(), &num));
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    REQUIRE_RC(KStreamReadAll(stream, buf, content.length()*2, &num));
    REQUIRE_EQ(content.length(), num);

    CloseClientStream(stream);
}

//////////////////////////////////////////// IPC, timed reads
class TimedReadSocketFixture : public SocketFixture
{
// Sets up a server thread. The server thread will:
// - wait for an incoming message,
// - upon receiving the message, convert it to upper case
// - (this is different from SocketFixture) sleep for SERVER_WRITE_DELAY_MS (the client can time out or wait, depending on the test case)
// - send the converted message back
// - wait for an incoming message "done"
// - shut down the IPC connection

// Test cases' bodies represent client logic
// (this is different from SocketFixture) Call SetupClient() to initialize timeout value
// The fixture on the client will send "done" from the destructor and close its IPC connection
public:
    static const uint32_t SERVER_WRITE_DELAY_MS = 2000;
public:
    TimedReadSocketFixture()
    : m_stream(0)
    {
        threadWorker = StutteringWorkerThreadFn;
    }
    ~TimedReadSocketFixture()
    {
    }

	void SetupClient(const string& p_content)
	{
        LOG(LogLevel::e_message, "TimedReadSocketFixture::SetupClient(" + p_content + ")" << endl);
        StringInitCString(&name, SocketName.c_str());
        if (KNSManagerInitIPCEndpoint(mgr, &ep, &name) != 0)
			throw logic_error ( string("TimedReadSocketFixture: SetupClient(") + p_content + "), KNSManagerInitIPCEndpoint failed" );

        m_stream = MakeStream ( 5 * 1000 );
        LOG(LogLevel::e_message, "client '" << p_content << "' after KNSMakeConnection" << endl);
	}
	void SetupClient(const string& p_content, uint32_t p_timeoutMs)
	{
		TimeoutInit(&tm, p_timeoutMs);
		SetupClient(p_content);
	}
	void TeardownClient()
	{
        CloseClientStream(m_stream);
    }

    static rc_t StutteringWorkerThreadFn ( const KThread *self, void *data )
    {
        try
        {   // this server worker converts the incoming message to all uppercase, pauses for SERVER_WRITE_DELAY_MS, and sends it back
            KStream* stream = (KStream*)data;

            char localBuf[MaxMessageSize];
            size_t localNumRead;
            THROW_ON_RC ( KStreamTimedRead(stream, localBuf, sizeof(localBuf), &localNumRead, NULL) ); // wait forever
            if (localNumRead == 0)
                throw logic_error ( "TimedReadSocketFixture worker: 0 bytes read" );
            LOG(LogLevel::e_message, "worker "  << (void*)self << " after KStreamRead(" << string(localBuf, localNumRead) << ")" << endl);

            for (size_t i = 0 ; i < localNumRead; ++i)
                localBuf[i] = toupper(localBuf[i]);

            // send outgoing message after a pause for SERVER_WRITE_DELAY_MS
            LOG(LogLevel::e_message, "worker "  << (void*)self << " sleeping for " << SERVER_WRITE_DELAY_MS << " ms" << endl);
            TestEnv::SleepMs(SERVER_WRITE_DELAY_MS);

            LOG(LogLevel::e_message, "worker "  << (void*)self << " writing " << localNumRead << " bytes" << endl);
            size_t localNumWrit;
            THROW_ON_RC ( KStreamWrite(stream, localBuf, localNumRead, &localNumWrit) ); // localNumWrit may be 0 if the client is not reading, as in timeout cases
            LOG(LogLevel::e_message, "worker "  << (void*)self << " after KStreamWrite" << endl);

            // wait until the reader says "done"
            LOG(LogLevel::e_message, "worker "  << (void*)self << " waiting for 'done'" << endl);
            THROW_ON_RC ( KStreamTimedRead(stream, localBuf, sizeof(localBuf), &localNumRead, NULL) ); // wait forever

            string doneMsg(localBuf, localNumRead);
            LOG(LogLevel::e_message, "worker "  << (void*)self << " after KStreamRead = '" << doneMsg << "'" << endl);
            if (doneMsg != "done")
                throw logic_error ( "SocketFixture worker: out of sequence message received: '" + doneMsg + "'" );

            LOG(LogLevel::e_message, "worker "  << (void*)self << " closing stream" << endl);
            THROW_ON_RC ( KStreamRelease(stream) );
            LOG(LogLevel::e_message, "worker "  << (void*)self << " after KStreamRelease" << endl);
        }
        catch (const exception& ex)
        {
            cout << "TimedReadSocketFixture worker thread threw " << ex.what() << endl;
            throw;
        }
        LOG(LogLevel::e_message, "worker "  << (void*)self << " exiting" << endl);
        return KThreadRelease(self);
    }

    // for use in test cases
    KStream* m_stream;
    timeout_t tm;
};

////////////////////// 1. KNSManagerMakeConnection (no time-out specified), then use KStreamTimedRead/Write
PROCESS_FIXTURE_TEST_CASE(TimedRead_NULL_Timeout, TimedReadSocketFixture, 0, 20)
{   // 1.1. wait indefinitely until the server responds
    string content = GetName();
	SetupClient(content);

    REQUIRE_RC(KStreamTimedWrite(m_stream, content.c_str(), content.length(), &num, NULL)); // waits indefinitely
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    REQUIRE_RC(KStreamTimedRead(m_stream, buf, sizeof(buf), &num, NULL)); // waits indefinitely
    LOG(LogLevel::e_message, "client after KStreamRead" << endl);
    REQUIRE_EQ(string(buf, num), ToUpper(content));

	TeardownClient();
}

PROCESS_FIXTURE_TEST_CASE(TimedRead_0_Timeout, TimedReadSocketFixture, 0, 20)
{   // 1.2. time out immediately when the server has not yet responded
    string content = GetName();
	SetupClient(content, 0); /* no wait */

    REQUIRE_RC(KStreamTimedWrite(m_stream, content.c_str(), content.length(), &num, &tm)); // returns immediately if socket is not writeable
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    rc_t rc = (KStreamTimedRead(m_stream, buf, sizeof(buf), &num, &tm)); // returns immediately if no data

    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(rc, RC(rcNS,rcFile,rcReading,rcTimeout,rcExhausted));
    LOG(LogLevel::e_message, "client timed out on read, as expected" << endl);

    TestEnv::SleepMs(SERVER_WRITE_DELAY_MS * 2); // let the server wake up to handle the 'done' message
	TeardownClient();
}

PROCESS_FIXTURE_TEST_CASE(TimedRead_Short_Timeout, TimedReadSocketFixture, 0, 20)
{   // 1.3. time out when the server has not responded quickly enough
    string content = GetName();
	SetupClient(content, SERVER_WRITE_DELAY_MS / 2);

    REQUIRE_RC(KStreamTimedWrite(m_stream, content.c_str(), content.length(), &num, &tm)); // times out if socket is not writeable
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    rc_t rc = (KStreamTimedRead(m_stream, buf, sizeof(buf), &num, &tm)); // returns after timing out
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(rc, RC(rcNS,rcFile,rcReading,rcTimeout,rcExhausted));
    LOG(LogLevel::e_message, "client timed out on read, as expected" << endl);

    TestEnv::SleepMs(SERVER_WRITE_DELAY_MS * 2); // let the server wake up to handle the 'done' message
	TeardownClient();
}

PROCESS_FIXTURE_TEST_CASE(TimedRead_Long_Timeout, TimedReadSocketFixture, 0, 20)
{   // 1.4. wait enough time time for the server to respond
    string content = GetName();
	SetupClient(content, SERVER_WRITE_DELAY_MS * 2);

    REQUIRE_RC(KStreamTimedWrite(m_stream, content.c_str(), content.length(), &num, &tm)); // times out if socket is not writeable
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    REQUIRE_RC(KStreamTimedRead(m_stream, buf, sizeof(buf), &num, &tm)); // should not time out
    LOG(LogLevel::e_message, "client after KStreamRead" << endl);
    REQUIRE_EQ(string(buf, num), ToUpper(content));

	TeardownClient();
}

////////////////////// 2. KNSManagerMakeTimedConnection, then use KStreamRead/Write, or override using TimedRead/Write,
//////////////////////      or override using KNSManagerSetConnectionTimeouts
class TimedConnection_ReadSocketFixture : public TimedReadSocketFixture
{   // same as TimedReadSocketFixture but creates times connections as opposed to
    // issuing timed reads/writes
public:
	void SetupClient(const string& p_content, int32_t p_readMillis, int32_t p_writeMillis)
	{   // same as TimedReadSocketFixture::SetupClient but calls KNSManagerMakeTimedConnection instead of KNSManagerMakeConnection
        StringInitCString(&name, SocketName.c_str());
        if (KNSManagerInitIPCEndpoint(mgr, &ep, &name) != 0)
			throw logic_error ( string("TimedConnection_ReadSocketFixture: SetupClient(") + p_content + "), KNSManagerInitIPCEndpoint failed" );

        m_stream = MakeStreamTimed( 5 * 1000, p_readMillis, p_writeMillis );
        LOG(LogLevel::e_message, "client '" << p_content << "' after KNSMakeConnection" << endl);
	}

    KStream* MakeStreamTimed( int32_t p_retryTimeout, int32_t p_readMillis, int32_t p_writeMillis  )
    {
        timeout_t tm;
        TimeoutInit ( & tm, p_retryTimeout );

        KSocket* socket;
        THROW_ON_RC ( KNSManagerMakeRetryTimedConnection(mgr, &socket, &tm, p_readMillis, p_writeMillis, NULL, &ep) );
        if (socket == 0)
           throw logic_error ( "MakeStreamTimed: KStreamRelease failed" );

        KStream* stream;
        THROW_ON_RC ( KSocketGetStream ( socket, & stream ) );
        if (stream == 0)
           throw logic_error ( "MakeStreamTimed: KStreamRelease failed" );
        KSocketRelease ( socket );

        return stream;
    }
};

PROCESS_FIXTURE_TEST_CASE(TimedConnection_Read_NULL_Timeout, TimedConnection_ReadSocketFixture, 0, 20)
{   // 2.1. wait indefinitely until the server responds
    string content = GetName();
	SetupClient(content, -1, -1); // wait indefinitely

    REQUIRE_RC(KStreamWrite(m_stream, content.c_str(), content.length(), &num));
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl); // waits indefinitely
    REQUIRE_EQ(content.length(), num);

    REQUIRE_RC(KStreamRead(m_stream, buf, sizeof(buf), &num)); // waits indefinitely
    LOG(LogLevel::e_message, "client after KStreamRead" << endl);
    REQUIRE_EQ(string(buf, num), ToUpper(content));

	TeardownClient();
}
PROCESS_FIXTURE_TEST_CASE(TimedConnection_TimedReadOverride_NULL_Timeout, TimedConnection_ReadSocketFixture, 0, 20)
{   // 2.1.1 wait indefinitely until the server responds
    string content = GetName();
	SetupClient(content, 0, 0); // the connection is created as no-wait
                            // but the reads/writes override that with "wait indefinitely"

    REQUIRE_RC(KStreamTimedWrite(m_stream, content.c_str(), content.length(), &num, NULL)); // waits indefinitely
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    REQUIRE_RC(KStreamTimedRead(m_stream, buf, sizeof(buf), &num, NULL)); // waits indefinitely
    LOG(LogLevel::e_message, "client after KStreamRead" << endl);
    REQUIRE_EQ(string(buf, num), ToUpper(content));

	TeardownClient();
}

PROCESS_FIXTURE_TEST_CASE(TimedConnection_Read_0_Timeout, TimedConnection_ReadSocketFixture, 0, 20)
{   // 2.2. time out immediately when the server has not yet responded
    string content = GetName();
	SetupClient(content, 0, 0); /* no wait */

    REQUIRE_RC(KStreamWrite(m_stream, content.c_str(), content.length(), &num)); // returns immediately if socket is not writeable
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    rc_t rc = (KStreamRead(m_stream, buf, sizeof(buf), &num)); // returns immediately if no data

    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(rc, RC(rcNS,rcFile,rcReading,rcTimeout,rcExhausted));
    LOG(LogLevel::e_message, "client timed out on read, as expected" << endl);

    TestEnv::SleepMs(SERVER_WRITE_DELAY_MS * 2); // let the server wake up to handle the 'done' message
	TeardownClient();
}
PROCESS_FIXTURE_TEST_CASE(TimedConnection_ReadOverride_0_Timeout, TimedConnection_ReadSocketFixture, 0, 20)
{   // 2.2.1 time out immediately when the server has not yet responded
    string content = GetName();
	SetupClient(content, -1, -1);   // the connection is created as "wait indefinitely"
    TimeoutInit(&tm, 0);       // but the reads/writes override that with "no wait"

    REQUIRE_RC(KStreamTimedWrite(m_stream, content.c_str(), content.length(), &num, &tm)); // returns immediately if socket is not writeable
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    rc_t rc = (KStreamTimedRead(m_stream, buf, sizeof(buf), &num, &tm)); // returns immediately if no data

    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(rc, RC(rcNS,rcFile,rcReading,rcTimeout,rcExhausted));
    LOG(LogLevel::e_message, "client timed out on read, as expected" << endl);

    TestEnv::SleepMs(SERVER_WRITE_DELAY_MS * 2); // let the server wake up to handle the 'done' message
	TeardownClient();
}
PROCESS_FIXTURE_TEST_CASE(TimedConnection_SettingsOverride_0_Timeout, TimedConnection_ReadSocketFixture, 0, 20)
{   // 2.2.2 time out immediately when the server has not yet responded
    REQUIRE_RC(KNSManagerSetConnectionTimeouts(mgr, 5000, 0, 0)); // override default setting (long time-out) to "no wait"
    string content = GetName();
	TimedReadSocketFixture::SetupClient(content);

    REQUIRE_RC(KStreamWrite(m_stream, content.c_str(), content.length(), &num)); // returns immediately if socket is not writeable
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    rc_t rc = (KStreamRead(m_stream, buf, sizeof(buf), &num)); // returns immediately if no data

    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(rc, RC(rcNS,rcFile,rcReading,rcTimeout,rcExhausted));
    LOG(LogLevel::e_message, "client timed out on read, as expected" << endl);

    TestEnv::SleepMs(SERVER_WRITE_DELAY_MS * 2); // let the server wake up to handle the 'done' message
	TeardownClient();
}

PROCESS_FIXTURE_TEST_CASE(TimedConnection_Read_Short_Timeout, TimedConnection_ReadSocketFixture, 0, 20)
{   // 2.3. time out when the server has not responded quickly enough
    string content = GetName();
	SetupClient(content, SERVER_WRITE_DELAY_MS / 2, SERVER_WRITE_DELAY_MS / 2);

    REQUIRE_RC(KStreamWrite(m_stream, content.c_str(), content.length(), &num)); // times out if socket is not writeable
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    rc_t rc = (KStreamRead(m_stream, buf, sizeof(buf), &num)); // returns after timing out
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(rc, RC(rcNS,rcFile,rcReading,rcTimeout,rcExhausted));
    LOG(LogLevel::e_message, "client timed out on read, as expected" << endl);

    TestEnv::SleepMs(SERVER_WRITE_DELAY_MS * 2); // let the server wake up to handle the 'done' message
	TeardownClient();
}
PROCESS_FIXTURE_TEST_CASE(TimedConnection_ReadOverride_Short_Timeout, TimedConnection_ReadSocketFixture, 0, 20)
{   // 2.3.1. time out when the server has not responded quickly enough
    string content = GetName();
	SetupClient(content, -1, -1);                       // the connection is created as "wait indefinitely"
    TimeoutInit(&tm, SERVER_WRITE_DELAY_MS / 2);    // but the reads/writes override that with a short time-out

    REQUIRE_RC(KStreamTimedWrite(m_stream, content.c_str(), content.length(), &num, &tm)); // times out if socket is not writeable
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    rc_t rc = (KStreamTimedRead(m_stream, buf, sizeof(buf), &num, &tm)); // returns after timing out
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(rc, RC(rcNS,rcFile,rcReading,rcTimeout,rcExhausted));
    LOG(LogLevel::e_message, "client timed out on read, as expected" << endl);

    TestEnv::SleepMs(SERVER_WRITE_DELAY_MS * 2); // let the server wake up to handle the 'done' message
	TeardownClient();
}

PROCESS_FIXTURE_TEST_CASE(TimedConnection_Read_Long_Timeout, TimedConnection_ReadSocketFixture, 0, 20)
{   // 2.4. wait enough time for the server to respond
    string content = GetName();
	SetupClient(content, SERVER_WRITE_DELAY_MS * 2, SERVER_WRITE_DELAY_MS * 2);

    REQUIRE_RC(KStreamWrite(m_stream, content.c_str(), content.length(), &num)); // times out if socket is not writeable
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    REQUIRE_RC(KStreamRead(m_stream, buf, sizeof(buf), &num)); // should not time out
    LOG(LogLevel::e_message, "client after KStreamRead" << endl);
    REQUIRE_EQ(string(buf, num), ToUpper(content));

	TeardownClient();
}
PROCESS_FIXTURE_TEST_CASE(TimedConnection_ReadOverride_Long_Timeout, TimedConnection_ReadSocketFixture, 0, 20)
{   // 2.4.1. wait enough time for the server to respond
    string content = GetName();
	SetupClient(content, 0, 0);                         // the connection is created as "no wait"
    TimeoutInit(&tm, SERVER_WRITE_DELAY_MS * 2);    // but the reads/writes override that with a sufficient time-out

    REQUIRE_RC(KStreamTimedWrite(m_stream, content.c_str(), content.length(), &num, &tm)); // times out if socket is not writeable
    LOG(LogLevel::e_message, "client after KStreamWrite" << endl);
    REQUIRE_EQ(content.length(), num);

    REQUIRE_RC(KStreamTimedRead(m_stream, buf, sizeof(buf), &num, &tm)); // should not time out
    LOG(LogLevel::e_message, "client after KStreamRead" << endl);
    REQUIRE_EQ(string(buf, num), ToUpper(content));

	TeardownClient();
}

//////////////////////////////////////////// IPC, timed writes
class TimedWriteSocketFixture : public SocketFixture
{
// There will be 2 IPC connections between the server and the client, a data channel and a control channel
// The server will not read from the data channel until the client sends "go" through the control channel
// The clients will write to the data channel until it overflows (thus setting up subsequent timed writes to wait),
//  then send "go" as required by the test case's logic
public:
    static const size_t SERVER_WRITE_DELAY_MS = 2000;
public:
    TimedWriteSocketFixture()
    : m_data(0), m_control(0)
    {
		go = false;
        threadWorker = TimedWriteServerFn;
    }
    ~TimedWriteSocketFixture()
    {

    }

	void SetupClient(const string& p_name)
	{
        StringInitCString(&name, SocketName.c_str());
        if (KNSManagerInitIPCEndpoint(mgr, &ep, &name) != 0)
			throw logic_error ( string("TimedWriteSocketFixture: SetupClient(") + p_name + "), KNSManagerInitIPCEndpoint failed" );

        m_data = MakeStream ( 5 * 1000 );
        m_control = MakeStream ( 5 * 1000 );

		// identify data/control channels to the server
		WriteMessage(m_data, "data");
		WriteMessage(m_control, "ctrl");

        LOG(LogLevel::e_message, "client '" << p_name << "' waiting for server to send 'ready'" << endl);
        string message = ReadMessage(m_control, 5, -1);
        if (message != "ready")
			throw logic_error ( "TimedWriteSocketFixture: ReadMessage('ready') failed" );

        LOG(LogLevel::e_message, "client '" << p_name << "' after KNSMakeConnection" << endl);
	}

	void TeardownClient()
    {
        CloseClientStream(m_data);
        CloseClientStream(m_control);
    }

	static string ReadMessage(KStream* p_stream, size_t size = 0, int p_timeoutMs = -1)
	{
		char localBuf[MaxMessageSize];
		size_t num;
        timeout_t tm;
        tm.mS = p_timeoutMs;
		THROW_ON_RC ( KStreamTimedRead(p_stream, localBuf, size == 0 ? sizeof(localBuf) : size, &num, p_timeoutMs == -1 ? NULL : &tm) );

		return string(localBuf, num);
	}
	static bool TryReadMessage(KStream* p_stream)
	{
		char localBuf[MaxMessageSize];
		size_t num;
        timeout_t tm;
        tm.mS = 1000;
		return KStreamTimedRead(p_stream, localBuf, sizeof(localBuf), &num, &tm) == 0;
	}
	static rc_t WriteMessage(KStream* p_stream, const string& p_msg, int p_timeoutMs = -1)
	{
		size_t num;
        timeout_t tm;
        tm.mS = p_timeoutMs;
        LOG(LogLevel::e_message, "WriteMessage, timeout=" << p_timeoutMs << "ms" << endl);
		return KStreamTimedWrite(p_stream, p_msg.c_str(), p_msg.size(), &num, p_timeoutMs == -1 ? NULL : &tm);
	}

	static volatile bool go;
    static rc_t TimedWriteServerFn ( const KThread *self, void *data )
    {
        // this function does not always exit, so using STL string in this function leads to occasional leaks.
        // for this reason, C strings are used for local variables (STL temporaries seems to be ok)
        char prefix[1024];
        string_printf ( prefix, sizeof prefix, NULL, "TimedWriteSocketFixture worker %p: ", (void*)self );

        try
        {
            KStream* stream = (KStream*)data;

            // local STL string here leaks somehow; use C string for now
			// string message = ReadMessage(stream, 4, 1000);
            char message[MaxMessageSize];
            size_t num;
            timeout_t tm;
            tm.mS = 1000;
            THROW_ON_RC ( KStreamTimedRead(stream, message, 4, &num, &tm) );

            LOG(LogLevel::e_message, (string(prefix) + " after KStreamRead(" + message + ")\n"));
			if (string(message, num) == "data")
			{	// from now on, wait until control thread allows us to read
                LOG(LogLevel::e_message, "data thread waiting for 'go'\n");
				while (!go)
				{
                    LOG(LogLevel::e_message, "data thread waiting for 'go'\n");
					TestEnv::SleepMs(1);
				}
                LOG(LogLevel::e_message, "data thread received 'go'\n");
                // consume the input messages and go away
                LOG(LogLevel::e_message, "un-flooding data channel\n");
                size_t every=0;
				while (TryReadMessage(stream))
                {
                    if (every == 0)
                    {
                        LOG(LogLevel::e_message, "still un-flooding data channel...\n");
                        every=5000;
                    }
                    else
                        --every;
				}
                LOG(LogLevel::e_message, "data thread complete\n");
            }
			else if (string(message, num) == "ctrl")
			{	// when received "go", allow the data thread to read
                WriteMessage(stream, "ready", -1);

				while (true)
				{
					string ctrlMessage = ReadMessage(stream, 4, 10000);
					if (ctrlMessage == "gogo")
                    {
                        LOG(LogLevel::e_message, "control thread received 'gogo'\n");
						go = true;
                    }
					else if (ctrlMessage == "done")
					{
                        LOG(LogLevel::e_message, "control thread received 'done'\n");
						break;
					}
				}
                LOG(LogLevel::e_message, "control thread complete\n");
			}
			else
                throw logic_error ( string(prefix) + "unexpected message\n" );

			THROW_ON_RC ( KStreamRelease(stream) );
		}
        catch (const exception& ex)
        {
            cout << (string(prefix) + "threw " + ex.what() +"\n");
            throw;
        }
        catch (...)
        {
            cout << (string(prefix) + "threw something\n");
            throw;
        }
        LOG(LogLevel::e_message, (string(prefix) + " exiting\n"));
        return KThreadRelease(self);
    }

    // for use in test cases (=client code)
	void FloodDataChannel()
	{
        LOG(LogLevel::e_message, "flooding" << endl);
		char localBuf[MaxMessageSize];
        memset(localBuf, 0xab, sizeof(localBuf)); // to keep valgrind happy
		size_t num;
        struct timeout_t tm;
        tm.mS = 0; /* do not wait */
		while (true)
		{
			LOG(LogLevel::e_message, "writing " << MaxMessageSize << " bytes\n");
			rc_t rc = KStreamTimedWrite(m_data, localBuf, sizeof(localBuf), &num, &tm);
			if (rc != 0)
			{
				LOG(LogLevel::e_message, "KStreamWrite failed - flooding complete\n");
				break;
			}
			if (num != sizeof(localBuf))
			{
				LOG(LogLevel::e_message, "written " << num << " bytes, expected " << sizeof(localBuf) << endl);
				break;
			}
		}
	}

    KStream* m_data;
    KStream* m_control;
};

volatile bool TimedWriteSocketFixture::go = false;

//  1. flood the socket, see KStreamTimedWrite time out
PROCESS_FIXTURE_TEST_CASE(TimedWrite_Short_Timeout, TimedWriteSocketFixture, 0, 20)
{
	SetupClient(GetName());
	FloodDataChannel(); // the last WriteMessage(data) failed since nobody is reading from the server side

    /* important: attempt to write at least as many bytes as a block used by FloodDataChannel , otherwise this write has a chance to succeed */
	rc_t rc = WriteMessage(m_data, string(MaxMessageSize, 'z'), 0);  /* do not wait */
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(rc, RC(rcNS,rcFile,rcWriting,rcTimeout,rcExhausted));

	REQUIRE_RC(WriteMessage(m_control, "gogo", 0)); // signal the server to start reading
    // the data channel is no longer flooded; give the server time to empty the pipe and finish
    TestEnv::SleepMs(100);

    TeardownClient();
}

//  2. flood the socket, see KStreamTimedWrite wait indefinitely
PROCESS_FIXTURE_TEST_CASE(TimedWrite_NULL_Timeout, TimedWriteSocketFixture, 0, 20)
{
	SetupClient(GetName());
	FloodDataChannel(); // the last WriteMessage(data) failed since nobody is reading from the server side

	REQUIRE_RC(WriteMessage(m_control, "gogo", 0)); // signal the server to start reading

	REQUIRE_RC(WriteMessage(m_data, "something", -1)); // this should wait for the server to un-flood the data channel

    // the data channel is no longer flooded; give the server time to empty the pipe and finish
    TestEnv::SleepMs(100);

    TeardownClient();
}

//  TODO: KStreamReadAll, KStreamTimedReadAll,
//	TODO: KStreamWriteAll, KStreamTimedWriteAll,

//  TODO: KStreamReadExactly, KStreamTimedReadExactly
//  TODO: KStreamWriteExactly, KStreamTimedWriteExactly

//////////////////////////////////////////// Main

static rc_t argsHandler(int argc, char * argv[]) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
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
const char UsageDefaultName[] = "test-kns";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();

	// uncomment to see messages from socket code
    // KDbgSetModConds ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), DBG_FLAG ( DBG_KNS_SOCKET ) );

	// this makes messages from the test code appear
	// (same as running the executable with "-l=message")
	//TestEnv::verbosity = LogLevel::e_message;

    rc_t rc=KnsTestSuite(argc, argv);
    return rc;
}

}
