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

#include <errno.h>

#include <klib/rc.h>
#include <klib/text.h>
#include <kproc/timeout.h>
#include <kns/manager.h>
#include <kns/endpoint.h>
#include <kns/socket.h>

#include <../libs/kns/stream-priv.h>

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(KnsTestSuite, argsHandler);

using namespace std;
using namespace ncbi::NK;

//////////////////////////////////////////// connecting to Internet sockets

class ConnectFixture
{
public:
    ConnectFixture()
    :   mgr ( nullptr ),
        socket ( nullptr )
    {
        THROW_ON_RC ( KNSManagerMake ( & mgr ) );
        String url;
        CONST_STRING( &url, "www.google.com" );
        THROW_ON_RC ( KNSManagerInitDNSEndpoint ( mgr, & ep, &url, 80 ) );

        TimeoutInit ( & tm, 1 );

    }

    ~ConnectFixture()
    {
        KSocketRelease( socket );
        KNSManagerRelease( mgr );
    }

    KNSManager* mgr;
    KEndPoint ep;
    timeout_t tm;
    KSocket* socket;
};

// mock system call
int return_val = 0;
int set_errno = 0;
uint32_t tries = 0;

#ifdef MAC
    extern "C"
    int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents,
         const struct timespec *timeout)
#else
    extern "C"
    int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
#endif
{   // fake an error
    errno = set_errno;
    ++tries;
    return return_val;
}

FIXTURE_TEST_CASE(Connect_OK, ConnectFixture)
{   //VDB-3754: asynch connnection, success
    return_val = 1; /* epoll_wait: success */
    rc_t rc = KNSManagerMakeRetryTimedConnection( mgr, & socket, & tm, 0, 0, NULL, & ep );
    REQUIRE_RC ( rc );
}

#ifndef WINDOWS
// VDB-4012:no async connection on Windows

FIXTURE_TEST_CASE(Connect_Timeout, ConnectFixture)
{   //VDB-3754: asynch connnection, test timeout, no retries
#ifdef DEBUG
    cerr << "vvv expect to see 'connect_wait() timed out'" << endl;
#endif
    return_val = 0; /* epoll_wait: timeout */
    tries = 0;
    rc_t rc = KNSManagerMakeRetryTimedConnection( mgr, & socket, & tm, 0, 0, NULL, & ep);
    REQUIRE_RC_FAIL ( rc );
    REQUIRE_EQ ( ( int ) rcTimeout, ( int ) GetRCObject ( rc ) );
    REQUIRE_EQ ( ( int ) rcExhausted, ( int ) GetRCState ( rc ) );
    REQUIRE_EQ ( 1u, tries );
#ifdef DEBUG
    cerr << "^^^ expect to see 'connect_wait() timed out'" << endl;
#endif
}

FIXTURE_TEST_CASE(Connect_CtrlC, ConnectFixture)
{   //VDB-3754: asynch connnection, test interruption by CtrlC, no retries
#ifdef DEBUG
    cerr << "vvv expect to see 'connect_wait() interrupted'" << endl;
#endif
    return_val = -1; /* epoll_wait: error */
    set_errno = EINTR;
    tries = 0;
    rc_t rc = KNSManagerMakeRetryTimedConnection( mgr, & socket, & tm, 0, 0, NULL, & ep);
    REQUIRE_RC_FAIL ( rc );
    REQUIRE_EQ ( ( int ) rcConnection, ( int ) GetRCObject ( rc ) );
    REQUIRE_EQ ( ( int ) rcInterrupted, ( int ) GetRCState ( rc ) );
    REQUIRE_EQ ( 1u, tries );
#ifdef DEBUG
    cerr << "^^^ expect to see 'connect_wait() interrupted'" << endl;
#endif
}

#endif

//////////////////////////////////////////// Main

#include <kapp/args.h> /* ArgsMakeAndHandle */

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

const char UsageDefaultName[] = "test-connect";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();

#ifdef DEBUG
    // to see messages from socket code
    KDbgSetModConds ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), DBG_FLAG ( DBG_KNS_SOCKET ) );
#endif

    rc_t rc=KnsTestSuite(argc, argv);
    return rc;
}

}
