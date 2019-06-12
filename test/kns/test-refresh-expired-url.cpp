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
* =========================================================================== */

#include "HttpFixture.hpp"

#include <ktst/unit_test.hpp>

#include <kfg/kfg-priv.h>
#include <kns/kns-mgr-priv.h>
#include <kfs/file.h>
#include <klib/time.h>
#include <kproc/timeout.h>

#include "../../libs/kns/http-file-priv.h"

#include <cassert>
#include <sstream>

using namespace std;
using namespace ncbi::NK;

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

static rc_t argsHandler ( int argc, char * argv [] );
TEST_SUITE_WITH_ARGS_HANDLER ( HttpRefreshTestSuite, argsHandler );

class CloudFixture : public HttpFixture
{
public:
    void AddRedirect( const string & url, KTime_t expTime )
    {
        char expirationStr[100];
        KTimeIso8601 ( expTime, expirationStr, sizeof expirationStr );
        TestStream::AddResponse( string ( "HTTP/1.1 307 Temporary Redirect\r\n" ) +
                            "Location: " + url + "\r\n" +
                            "Expires: " + expirationStr + "\r\n" );
    }
    void AddHead()
    {
        ostringstream ostr;
        ostr << "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: " << sizeof m_buf << "\r\n";
        TestStream::AddResponse( ostr.str() );
    }
    void AddGet()
    {
        ostringstream ostr;
        ostr << "HTTP/1.1 206 Partial Content\r\n" <<
                "Content-Range: bytes 0-" << ( sizeof m_buf - 1 ) << "/" << sizeof m_buf << "\r\n" <<
                "Content-Length: " << sizeof m_buf  << "\r\n" <<
                "\r\n" <<
                string(sizeof m_buf, 'z') <<
                "\r\n";
        TestStream::AddResponse( ostr.str() );
    }

    char m_buf[1024];
    size_t num_read;
};

FIXTURE_TEST_CASE( HttpRefreshTestSuite_UntimedRead, CloudFixture )
{
    //TestEnv::verbosity = LogLevel::e_message;
    string url = MakeURL(GetName()).c_str();

    // simulates a 2-stage (redirect, real) response to HEAD from "signer" service. Pretend this is the object we need
    // expiring 65 seconds in the future; the refresh timer will be set to 60 seconds before that
    KTime_t expTime = KTimeStamp () + 65;
    AddRedirect ( "https://somewhere.in.the.cloud/accession", expTime );
    AddHead();

    // needs an environment token, no requester-pays
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, true, false, url . c_str () ) );
    REQUIRE_NOT_NULL ( m_file ) ;

    // make sure expiration time is reflected on the HttpFile object
    REQUIRE_EQ ( expTime, KTimeMakeTime ( & ( (const struct KHttpFile*)m_file ) -> url_expiration ) );

    // use GET to read a portion of the file
    AddGet();
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );

    // wait 6s to cross the refresh threshold, read again, see the URL refreshed and the expiration updated
    cout << "Sleeping" << endl;
    KSleep(6);

    // another simulated response from the "signer" service. The URL is now different, with new expiration
    KTime_t newExpTime = KTimeStamp () + 65;
    AddRedirect ( "https://ELSEWHERE.in.the.cloud/accession", newExpTime );
    AddHead();

    // KFileTimedRead without a timeout
    // this Read will notice that the expiration time is nigh, and call the "signer" for a new temporary URL
    AddGet();
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );
    REQUIRE_EQ ( newExpTime, KTimeMakeTime ( & ( (const struct KHttpFile*)m_file ) -> url_expiration ) );

    // the next Read (right away) will not refresh the URL
    AddGet();
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );
    REQUIRE_EQ ( newExpTime, KTimeMakeTime ( & ( (const struct KHttpFile*)m_file ) -> url_expiration ) ); // expiration did not change
}
#if 0
FIXTURE_TEST_CASE( HttpRefreshTestSuite_TimedRead, CloudFixture )
{
    //TestEnv::verbosity = LogLevel::e_message;
    string url = MakeURL(GetName()).c_str();

    // simulates a 2-stage (redirect, real) response to HEAD from "signer" service. Pretend this is the object we need
    // expiring 65 seconds in the future; the refresh timer will be set to 60 seconds before that
    KTime_t expTime = KTimeStamp () + 65;
    AddRedirect ( "https://somewhere.in.the.cloud/accession", expTime );
    AddHead();

    // needs an environment token, no requester-pays
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, true, false, url . c_str () ) );
    REQUIRE_NOT_NULL ( m_file ) ;

    // make sure expiration time is reflected on the HttpFile object
    REQUIRE_EQ ( expTime, KTimeMakeTime ( & ( (const struct KHttpFile*)m_file ) -> url_expiration ) );

    // use GET to read a portion of the file
    AddGet();
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );

    // wait 6s to cross the refresh threshold, read again, see the URL refreshed and the expiration updated
    cout << "Sleeping" << endl;
    KSleep(6);

    // another simulated response from the "signer" service. The URL is now different, with new expiration
    KTime_t newExpTime = KTimeStamp () + 65;
    AddRedirect ( "https://ELSEWHERE.in.the.cloud/accession", newExpTime );
    AddHead();

    // KFileTimedRead with a timeout
    AddGet();
    // this Read will notice that the timeout may happen after the URL expiration time - 1 min, and call the "signer" for a new temporary URL
    timeout_t tm;
    TimeoutInit ( & tm, 6 * 1000 /*6 seconds in milliseconds*/ );
    REQUIRE_RC ( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, & tm ) );
    REQUIRE_EQ ( newExpTime, KTimeMakeTime ( & ( (const struct KHttpFile*)m_file ) -> url_expiration ) );

    // the next Read (right away) will not refresh the URL
    AddGet();
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );
    REQUIRE_EQ ( newExpTime, KTimeMakeTime ( & ( (const struct KHttpFile*)m_file ) -> url_expiration ) ); // expiration did not change
}
#endif
//////////////////////////////////////////// Main

#include <kapp/args.h> // Args
#include <klib/debug.h>
#include <kfg/config.h>

static rc_t argsHandler ( int argc, char * argv [] ) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0, NULL, 0 );
    ArgsWhack ( args );
    return rc;
}

extern "C" {
    const char UsageDefaultName[] = "test-refresh-expired";
    rc_t CC UsageSummary ( const char     * progname) { return 0; }
    rc_t CC Usage        ( const struct Args * args ) { return 0; }
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t CC KMain ( int argc, char * argv [] )
    {
        if ( 1 ) assert ( ! KDbgSetString ( "KNS-HTTP" ) );
        KConfigDisableUserSettings ();

        KConfig * kfg = NULL;
        rc_t rc = KConfigMakeEmpty ( & kfg );
     // turn off certificate validation to download from storage.googleapis.com
        if ( rc == 0 )
            rc = KConfigWriteString ( kfg, "/tls/allow-all-certs", "true" );

        if ( rc == 0 )
            rc = HttpRefreshTestSuite ( argc, argv );

        RELEASE ( KConfig, kfg );

        return rc;
    }
}

