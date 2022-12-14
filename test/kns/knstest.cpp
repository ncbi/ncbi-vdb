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

#include <kapp/args.h> /* ArgsMakeAndHandle */

#include <kfg/config.h>

#include <klib/base64.h>
#include <klib/debug.h>
#include <klib/text.h>
#include <klib/strings.h>

#include <thread>

#include "KNSManagerFixture.hpp"

#include "../../libs/kns/kns_manager-singleton.h" // KNSManagerUseSingleton

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(KnsTestSuite, argsHandler);

using namespace std;
using namespace ncbi::NK;

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Session Id variables

class SessionIdFixture : public KNSManagerFixture
{
public:
    SessionIdFixture()
    {
        const char * ua = nullptr;
        KNSManagerGetUserAgent(&ua);
        original_ua = ua;
    }
    ~SessionIdFixture()
    {
        // restore the original; only use the initial portion
        size_t len = original_ua.find( " (phid=" );
        KNSManagerSetUserAgent(nullptr, "%s", original_ua.substr(0, len).c_str() );
        KNSManagerSetUserAgentSuffix("");
        KNSManagerSetClientIP(m_mgr, "");
        KNSManagerSetSessionID(m_mgr, "");
    }

    bool UserAgent_Contains( const string & str ) const
    {
        const char * ua = nullptr;
        KNSManagerGetUserAgent( & ua );
        return string::npos != string( ua ) . find( str );
    }

    string enc64( const string & str ) const
    {
        const String * s64;
        encodeBase64 ( &s64, str.c_str(), str.size() );
        string ret ( s64->addr, s64->size );
        StringWhack ( s64 );
        return ret;
    }

    string original_ua;
};

// KNSManagerGetUserAgent

FIXTURE_TEST_CASE(KNSManagerGetUserAgent_Null, SessionIdFixture)
{
   REQUIRE_RC_FAIL(KNSManagerGetUserAgent(nullptr));
}

FIXTURE_TEST_CASE(KNSManagerGetUserAgent_Default, SessionIdFixture)
{
    const char * ua = nullptr;
    KNSManagerGetUserAgent(&ua);
    const string ua_contains = "sra-toolkit test-kns.1 (phid=noc";
    //fprintf(stderr,"Got: '%s', expected '%s'\n", ua, ua_contains.data());
    REQUIRE_NE( string::npos, string(ua).find(ua_contains) );
    // VDB-4896: no double quotes inside UA
    REQUIRE_EQ( string::npos, string(ua).find("\"") );
}

FIXTURE_TEST_CASE(KNSManagerGetUserAgentEnv, SessionIdFixture)
{
    const char* ua = nullptr;
    setenv(ENV_MAGIC_OPT_BITMAP, "bmaphere", 1);
    KNSManagerGetUserAgent(&ua);
    const string ua_contains = "bmap=bmaphere";
    //fprintf(stderr,"Got: '%s', expected '%s'\n", ua, ua_contains.data());
    REQUIRE_NE(string::npos, string(ua).find(ua_contains));
    unsetenv(ENV_MAGIC_OPT_BITMAP);
}

FIXTURE_TEST_CASE(KNSManagerGetUserAgentPlatform, SessionIdFixture)
{
    const char* ua = nullptr;
    setenv(ENV_MAGIC_PLATFORM_NAME, "software", 1);
    setenv(ENV_MAGIC_PLATFORM_VERSION, "softwhat", 1);
    KNSManagerGetUserAgent(&ua);
    const string ua_contains = " via software softwhat";
    //fprintf(stderr,"Got: '%s', expected '%s'\n", ua, ua_contains.data());
    REQUIRE_NE(string::npos, string(ua).find(ua_contains));
    unsetenv(ENV_MAGIC_PLATFORM_NAME);
    unsetenv(ENV_MAGIC_PLATFORM_VERSION);
}

// KNSManagerSetUserAgent

FIXTURE_TEST_CASE(KNSManagerSetUserAgent_Null, SessionIdFixture)
{
    REQUIRE_RC(KNSManagerSetUserAgent(nullptr, "%s", "KNS Manager is a singleton so it is not checked"));
}

FIXTURE_TEST_CASE(KNSManagerSetUserAgent_, SessionIdFixture)
{
    const string new_ua = "new UserAgent";
    REQUIRE_RC(KNSManagerSetUserAgent(nullptr, "%s", new_ua.c_str()));

    REQUIRE( UserAgent_Contains( new_ua + " (phid=noc" ) );
}

// KNSManagerSetUserAgentSuffix, KNSManagerGetUserAgentSuffix

FIXTURE_TEST_CASE(KNSManagerSetUserAgentSuffix_NULL, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetUserAgentSuffix(nullptr));
}
FIXTURE_TEST_CASE(KNSManagerSetUserAgentSuffix_TooLong, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetUserAgentSuffix(string(KNSMANAGER_STRING_MAX, 'X').c_str()));
}

FIXTURE_TEST_CASE(KNSManagerGetUserAgentSuffix_NULL, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerGetUserAgentSuffix(nullptr));
}
FIXTURE_TEST_CASE(KNSManagerSetUserAgentSuffix_Get, SessionIdFixture)
{
    const string suffix = "suffix";
    REQUIRE_RC(KNSManagerSetUserAgentSuffix(suffix.c_str()));
    const char * s;
    REQUIRE_RC(KNSManagerGetUserAgentSuffix( & s ));
    REQUIRE_EQ( suffix, string( s ) );
    REQUIRE( UserAgent_Contains( "sra-toolkit test-kns.1suffix (phid=noc" ) );
}

FIXTURE_TEST_CASE(KNSManagerSetUserAgentSuffix_Restore, SessionIdFixture)
{
    REQUIRE_RC(KNSManagerSetUserAgentSuffix("suffix1"));
    REQUIRE( UserAgent_Contains( "sra-toolkit test-kns.1suffix1 (phid=noc" ) );

    REQUIRE_RC(KNSManagerSetUserAgentSuffix(""));
    const char * ua = nullptr;
    KNSManagerGetUserAgent(&ua);
    REQUIRE_EQ( original_ua, string(ua) );
}

// KNSManagerSetClientIP

FIXTURE_TEST_CASE(KNSManagerSetClientIP_SelfNull, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetClientIP(nullptr,"1.2.3.4"));
}
FIXTURE_TEST_CASE(KNSManagerSetClientIP_ParamNull, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetClientIP(m_mgr, nullptr));
}
FIXTURE_TEST_CASE(KNSManagerSetClientIP_TooLong, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetClientIP(m_mgr, string(KNSMANAGER_STRING_MAX, 'X').c_str()));
}
FIXTURE_TEST_CASE(KNSManagerSetClientIP_, SessionIdFixture)
{
    REQUIRE_RC(KNSManagerSetClientIP(m_mgr, "1.2.3.4"));
    REQUIRE( UserAgent_Contains( string(",") + enc64("cip=1.2.3.4,sid=,pagehit=") ) );
}

// KNSManagerSetSessionID

FIXTURE_TEST_CASE(KNSManagerSetSessionID_SelfNull, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetSessionID(nullptr,"sessionid"));
}
FIXTURE_TEST_CASE(KNSManagerSetSessionID_ParamNull, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetSessionID(m_mgr, nullptr));
}
FIXTURE_TEST_CASE(KNSManagerSetSessionID_TooLong, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetSessionID(m_mgr, string(KNSMANAGER_STRING_MAX, 'X').c_str()));
}
FIXTURE_TEST_CASE(KNSManagerSetSessionID_, SessionIdFixture)
{
    REQUIRE_RC(KNSManagerSetSessionID(m_mgr, "1.2.3.4"));
    REQUIRE( UserAgent_Contains( string(",") + enc64("cip=,sid=1.2.3.4,pagehit=") ) );
}

// KNSManagerSetPageHitID

FIXTURE_TEST_CASE(KNSManagerSetPageHitID_SelfNull, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetPageHitID(nullptr,"pagehit"));
}
FIXTURE_TEST_CASE(KNSManagerSetPageHitID_ParamNull, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetPageHitID(m_mgr, nullptr));
}
FIXTURE_TEST_CASE(KNSManagerSetPageHitID_TooLong, SessionIdFixture)
{
    REQUIRE_RC_FAIL(KNSManagerSetPageHitID(m_mgr, string(KNSMANAGER_STRING_MAX, 'X').c_str()));
}
FIXTURE_TEST_CASE(KNSManagerSetPageHitID_, SessionIdFixture)
{
    REQUIRE_RC(KNSManagerSetPageHitID(m_mgr, "1.2.3.4"));
    REQUIRE( UserAgent_Contains( string(",") + enc64("cip=,sid=,pagehit=1.2.3.4") ) );
}

// all 3 KNSManagerSetXXX
FIXTURE_TEST_CASE(KNSManagerSet_SessionAll, SessionIdFixture)
{
    REQUIRE_RC(KNSManagerSetClientIP(m_mgr, "1.2.3.4"));
    REQUIRE_RC(KNSManagerSetSessionID(m_mgr, "sessId"));
    REQUIRE_RC(KNSManagerSetPageHitID(m_mgr, "pageHitId"));
    REQUIRE( UserAgent_Contains( string(",") + enc64("cip=1.2.3.4,sid=sessId,pagehit=pageHitId") ) );
}

// thread locality of session Ids
FIXTURE_TEST_CASE(KNSManagerSet_ThreadLocal, SessionIdFixture)
{
    string s1, s2;
    std::thread t1 ([&]
                    {
                        REQUIRE_RC(KNSManagerSetClientIP(m_mgr, "1.2.3.4"));
                        REQUIRE_RC(KNSManagerSetSessionID(m_mgr, "sessId1"));
                        REQUIRE_RC(KNSManagerSetPageHitID(m_mgr, "pageHitId2"));
                        REQUIRE_RC(KNSManagerSetUserAgentSuffix("suffix1"));
                        std::this_thread::sleep_for (std::chrono::milliseconds(100));

                        const char * ua = nullptr;
                        KNSManagerGetUserAgent(&ua);
                        s1 = ua;
                    });
    std::thread t2 ([&]
                    {
                        REQUIRE_RC(KNSManagerSetClientIP(m_mgr, "11.22.33.44"));
                        REQUIRE_RC(KNSManagerSetSessionID(m_mgr, "sessId2"));
                        REQUIRE_RC(KNSManagerSetPageHitID(m_mgr, "pageHitId2"));
                        REQUIRE_RC(KNSManagerSetUserAgentSuffix("suffix2"));
                        std::this_thread::sleep_for (std::chrono::milliseconds(100));

                        const char * ua = nullptr;
                        KNSManagerGetUserAgent(&ua);
                        s2 = ua;
                    });
    t1.join();
    t2.join();

    REQUIRE(string::npos != s1.find(string(",")
        + enc64("cip=1.2.3.4,sid=sessId1,pagehit=pageHitId2")));
    REQUIRE(string::npos != s1.find("suffix1"));

    REQUIRE(string::npos != s2.find(string(",")
        + enc64("cip=11.22.33.44,sid=sessId2,pagehit=pageHitId2")));
    REQUIRE(string::npos != s2.find("suffix2"));

    // the above threads did not change the main thread's session Ids
    const char * ua = nullptr;
    KNSManagerGetUserAgent(&ua);
    REQUIRE_EQ( original_ua, string(ua) );
}

//////////////////////////////////////////// Main

static rc_t argsHandler(int argc, char * argv[]) {
    Args * args = nullptr;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, nullptr, 0);
    ArgsWhack(args);
    return rc;
}

extern "C"
{

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
    // make sure to use singleton, otherwise some tests fail
    KNSManagerUseSingleton(true);

    KConfigDisableUserSettings();

    return KnsTestSuite(argc, argv);
}

}
