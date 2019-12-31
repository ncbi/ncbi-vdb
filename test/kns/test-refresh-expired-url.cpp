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
#include "../../libs/kns/mgr-priv.h"

#include <cassert>
#include <sstream>

using namespace std;
using namespace ncbi::NK;

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

KConfig * kfg = NULL;

static rc_t argsHandler ( int argc, char * argv [] );
TEST_SUITE_WITH_ARGS_HANDLER ( HttpRefreshTestSuite, argsHandler );

class CloudFixture : public HttpFixture
{
public:
    // fake responses.
    // Call these methods in the exact order oif intended responses since they do not check the requests
    void RespondWithRedirect( const string & url, KTime_t expTime )
    {
        char expirationStr[100];
        KTimeIso8601 ( expTime, expirationStr, sizeof expirationStr );
        TestStream::AddResponse( string ( "HTTP/1.1 307 Temporary Redirect\r\n" ) +
                            "Location: " + url + "\r\n" +
                            "Expires: " + expirationStr + "\r\n" );
    }
    void RespondToHEAD()
    {
        ostringstream ostr;
        ostr << "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: " << sizeof m_buf << "\r\n";
        TestStream::AddResponse( ostr.str() );
    }
    void RespondToHEAD( const string & data )
    {   /* in cases when HEAD is converted into a POST or a GET, we actually read up to 256 bytes from the beginning */
        ostringstream ostr;
        ostr << "HTTP/1.1 206 Partial Content\r\n" <<
                "Content-Range: bytes 0-" << ( data.size() - 1 ) << "/" << data.size() << "\r\n" <<
                "Content-Length: " << data.size()  << "\r\n" <<
                "Accept-Ranges: bytes" << "\r\n" <<
                "\r\n" <<
                data <<
                "\r\n";
        TestStream::AddResponse( ostr.str() );
    }

    void RespondToGET()
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
    void RespondToGET_Full( const string & data )
    {
        ostringstream ostr;
        ostr << "HTTP/1.1 200 \r\n" <<
                "Content-Length: " << data.size()  << "\r\n" <<
                "\r\n" <<
                data <<
                "\r\n";
        TestStream::AddResponse( ostr.str() );
    }

    bool EnvironmentTokenPresent( const string & url )
    {
        return url.find("&ident=") != string::npos;
    }
    bool StringPresent( const string & url, const string & header )
    {
        return url.find(header) != string::npos;
    }

    const bool EnvTokenRequired = true;
    const bool PayRequired = true;
    const struct KHttpFile& MakeHttpFile ( const string & url, bool ce_required, bool payer_required )
    {
        THROW_ON_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, true, ce_required, payer_required, url . c_str () ) );
        THROW_ON_FALSE ( m_file != NULL ) ;
        return * reinterpret_cast < const struct KHttpFile* > ( m_file );
    }

    const struct KHttpFile& SetUpForExpiration( const string & url, bool ce_required, bool payer_required )
    {
        KTime_t expTime = KTimeStamp () + 65;
        RespondWithRedirect ( AwsUrl, expTime );
        RespondToHEAD();

        const struct KHttpFile& httpFile = MakeHttpFile( url, ce_required, payer_required );

        // read a portion of the file
        RespondToGET();
        THROW_ON_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );

        // wait 6s to cross the refresh threshold, read again, see the URL refreshed and the expiration updated
        cout << "Sleep 6 sec" << endl;
        KSleep(6);
        return httpFile;
    }

    void VerifyRequest ( const string & method, const string & url, bool tokenPresent, bool autorizationPresent, bool payerPresent )
    {
        // make sure there is no environment token added to the original URL
        THROW_ON_FALSE ( ! EnvironmentTokenPresent ( TestStream::m_requests.front() ) );
        // make sure there are no cloud-related headers added to the redirect URL
        string redirReq = TestStream::m_requests.back();
        THROW_ON_FALSE ( ! StringPresent ( redirReq, "Authorization" ) );
        THROW_ON_FALSE ( ! StringPresent ( redirReq, "Date" ) );
        THROW_ON_FALSE ( ! StringPresent ( redirReq, "x-amz-request-payer" ) );
    }

    char m_buf[1024];
    size_t num_read;
    static constexpr const char * AwsUrl = "https://amazonaws.com/accession";
    static constexpr const char * NonCloudUrl = "https://ncbi.nlm.nih.gov/accession";
};

FIXTURE_TEST_CASE( HttpRefreshTestSuite_RedirectSignedURL_NotCloud, CloudFixture )
{   //TODO: make sure not in a cloud
    //TestEnv::verbosity = LogLevel::e_message;
    string url = MakeURL(GetName());

    // simulates a 2-stage (redirect, real) response to a HEAD request from the "signer" service.
    // Pretend this is the object we need expiring 65 seconds in the future; the refresh timer will be set to 60 seconds before that
    KTime_t expTime = KTimeStamp () + 65;
    RespondWithRedirect ( NonCloudUrl, expTime );
    RespondToHEAD ();

    const struct KHttpFile& httpFile = MakeHttpFile( url, ! EnvTokenRequired, ! PayRequired );

    // make sure both original and redirection URLs and the expiration time are reflected on the HttpFile object
    REQUIRE_EQ ( url, string ( (const char*) httpFile . orig_url_buffer . base ) );
    REQUIRE_EQ ( string ( NonCloudUrl ), string ( (const char*) httpFile . url_buffer . base ) );
    REQUIRE ( httpFile . url_is_temporary );
    REQUIRE_EQ ( expTime, KTimeMakeTime ( & httpFile . url_expiration ) );

    // make sure there is no environment token added to the original URL
    REQUIRE ( ! EnvironmentTokenPresent ( TestStream::m_requests.front() ) );
    // make sure there are no cloud-related headers added to the redirect URL
    string redirReq = TestStream::m_requests.back();
    REQUIRE ( ! StringPresent ( redirReq, "Authorization" ) );
    REQUIRE ( ! StringPresent ( redirReq, "Date" ) );
    REQUIRE ( ! StringPresent ( redirReq, "x-amz-request-payer" ) );
}

// for AWS, autorization is only added with the payer info
FIXTURE_TEST_CASE( HttpRefreshTestSuite_RedirectSignedURL_AWS_NoToken_NoPayer, CloudFixture )
{
    putenv ( "AWS_ACCESS_KEY_ID=access_key_id" );
    putenv ( "AWS_SECRET_ACCESS_KEY=secret_access_key" );

    RespondWithRedirect ( AwsUrl, KTimeStamp () + 65 );
    RespondToHEAD ( string( 256, 'q' ) );

    MakeHttpFile( MakeURL(GetName()), EnvTokenRequired, ! PayRequired );

    // make sure there is no environment token added to the original URL
    REQUIRE ( ! EnvironmentTokenPresent ( TestStream::m_requests.front() ) );
    // make sure AWS autorization headers but no payer info header are added to the redirect URL
    string redirReq = TestStream::m_requests.back();
    REQUIRE ( ! StringPresent ( redirReq, "Authorization: AWS access_key_id:" ) );
    REQUIRE ( ! StringPresent ( redirReq, "x-amz-request-payer: requester" ) );
}

#if NOT_IMPLEMENTED
FIXTURE_TEST_CASE( HttpRefreshTestSuite_RedirectSignedURL_AWS_Token_NoPayer, CloudFixture )
{   //TODO: have to be on AWS
    TestEnv::verbosity = LogLevel::e_message;
    setenv ( "AWS_ACCESS_KEY_ID", "access_key_id", 1 );
    setenv ( "AWS_SECRET_ACCESS_KEY", "secret_access_key", 1 );
    m_mgr -> accept_aws_charges = false;

    RespondWithRedirect ( AwsUrl, KTimeStamp () + 65 );
    RespondToHEAD ( string( 256, 'q' ) );

    MakeHttpFile( MakeURL(GetName()), EnvTokenRequired, ! PayRequired );

    // make sure there is an environment token added to the original URL
    string origReq = TestStream::m_requests.front();
    REQUIRE ( EnvironmentTokenPresent ( origReq ) );
    // make sure HEAD was converted into POST 0..255, with User-agent header appended "-head" to, for analytics purposes
    REQUIRE ( ! StringPresent ( origReq, "HEAD" ) );
    REQUIRE ( StringPresent ( origReq, "POST" ) );
    REQUIRE ( StringPresent ( origReq, "0-255" ) );
    REQUIRE ( StringPresent ( origReq, "-head" ) );

    // make sure there is no payer info added to the redirect URL, User-agent header restored
    string lastReq = TestStream::m_requests.back();
    REQUIRE ( ! StringPresent ( lastReq, "x-amz-request-payer" ) );
    REQUIRE ( ! StringPresent ( lastReq, "-head" ) );
}
#endif

#if NOT_IMPLEMENTED
FIXTURE_TEST_CASE( HttpRefreshTestSuite_RedirectSignedURL_AWS_NoToken_Payer, CloudFixture )
{
    //TestEnv::verbosity = LogLevel::e_message;
    string url = MakeURL(GetName());

    setenv ( "AWS_ACCESS_KEY_ID", "access_key_id", 1 );
    setenv ( "AWS_SECRET_ACCESS_KEY", "secret_access_key", 1 );

    KTime_t expTime = KTimeStamp () + 65;
    RespondWithRedirect ( AwsUrl, expTime );
    // HEAD will be converted to GET and return the initial portion of the target file
    RespondToHEAD(string(2048, 'a'));

    MakeHttpFile( url, ! EnvTokenRequired, PayRequired );

    // make sure there is no environment token added to the original URL
    string origReq = TestStream::m_requests.front();
    REQUIRE ( ! EnvironmentTokenPresent ( origReq ) );
    // payment info is required and no token present, HEAD is converted into GET 0..255,
    // User-Agent appended -head to
    REQUIRE ( ! StringPresent ( origReq, "HEAD" ) );
    REQUIRE ( StringPresent ( origReq, "GET" ) );
    REQUIRE ( StringPresent ( origReq, "-head" ) );

    // make sure there are authorization and payer info added to the redirect URL
    // User-Agent restored
    string redirReq = TestStream::m_requests.back();
    REQUIRE ( StringPresent ( redirReq, "Authorization: AWS access_key_id:" ) );
    REQUIRE ( StringPresent ( redirReq, "Date: " ) );
    REQUIRE ( StringPresent ( redirReq, "x-amz-request-payer" ) );
    REQUIRE ( ! StringPresent ( redirReq, "-head" ) );
}
#endif

#if NOT_IMPLEMENTED

FIXTURE_TEST_CASE( HttpRefreshTestSuite_RedirectSignedURL_AWS_Token_Payer, CloudFixture )
{
    //TestEnv::verbosity = LogLevel::e_message;
    string url = MakeURL(GetName());

    setenv ( "AWS_ACCESS_KEY_ID", "access_key_id", 1 );
    setenv ( "AWS_SECRET_ACCESS_KEY", "secret_access_key", 1 );
    m_mgr -> accept_aws_charges = true;

    KTime_t expTime = KTimeStamp () + 65;
    RespondWithRedirect ( AwsUrl, expTime );
    // HEAD will be converted to POST and return the initial portion of the target file
    RespondToHEAD(string(2048, 'a'));

    MakeHttpFile( url, EnvTokenRequired, PayRequired );

    // make sure there is an environment token added to the original URL
    string origReq = TestStream::m_requests.front();
    REQUIRE ( EnvironmentTokenPresent ( origReq ) );
    // when token is required, HEAD is converted to POST
    REQUIRE ( ! StringPresent ( origReq, "HEAD" ) );
    REQUIRE ( StringPresent ( origReq, "POST" ) );

    // make sure there is authorization and payer info added to the redirect URL
    string redirReq = TestStream::m_requests.back();
    REQUIRE ( StringPresent ( redirReq, "Authorization: AWS access_key_id:" ) );
    REQUIRE ( StringPresent ( redirReq, "Date: " ) );
    REQUIRE ( StringPresent ( redirReq, "x-amz-request-payer" ) );
}
#endif

// Refresh temporary URL on a read within 1 min of expiration

FIXTURE_TEST_CASE( HttpRefreshTestSuite_ReadCloseToExpiration_AWS_NoToken_NoPayer, CloudFixture )
{
    string url = MakeURL(GetName());

    const struct KHttpFile& httpFile = SetUpForExpiration ( url, ! EnvTokenRequired, ! PayRequired );

    // another simulated response from the "signer" service. The URL is now different, with new expiration
    KTime_t newExpTime = KTimeStamp () + 65;
    string newAwsHost = "ELSEWHERE.in.the.cloud";
    RespondWithRedirect ( MakeURL( newAwsHost ), newExpTime );
    RespondToGET();

    // this Read will notice that the expiration time is nigh, re-issue GET with the original URL
    // and get redirected by the "signer" to a new temporary URL
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );
    REQUIRE_EQ ( newExpTime, KTimeMakeTime ( & httpFile . url_expiration ) );

    {   // verify that the second to last request (refresh the temporary URL)
        // was done on the original URL with a GET
        string req = * ( ++ TestStream::m_requests . rbegin() );
        REQUIRE ( StringPresent ( req, "GET" ) );
        REQUIRE ( StringPresent ( req, GetName() ) );
        REQUIRE ( ! EnvironmentTokenPresent ( req ) );
    }
    {   // verify that the last request (read the data) was done on the new redirected URL
        // with a GET, no token
        string req = TestStream::m_requests . back();
        REQUIRE ( StringPresent ( req, "GET" ) );
        REQUIRE ( StringPresent ( req, newAwsHost ) );
        REQUIRE ( ! EnvironmentTokenPresent ( req ) );
    }

    // the next Read (right away) will not refresh the URL or expiration
    RespondToGET();
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );
    REQUIRE_EQ ( newExpTime, KTimeMakeTime ( & httpFile . url_expiration ) ); // expiration did not change

    {   // verify that the last request was done on the same redirected URL
        string req = TestStream::m_requests . back();
        REQUIRE ( StringPresent ( req, newAwsHost ) );
        REQUIRE ( ! EnvironmentTokenPresent ( req ) );
        REQUIRE ( ! StringPresent ( req, "x-amz-request-payer: requester" ) );
    }
}

#if NOT_IMPLEMENTED
FIXTURE_TEST_CASE( HttpRefreshTestSuite_ReadCloseToExpiration_AWS_Token_NoPayer, CloudFixture )
{
    //TestEnv::verbosity = LogLevel::e_message;
    string url = MakeURL(GetName());

    const struct KHttpFile& httpFile = SetUpForExpiration ( url, EnvTokenRequired, ! PayRequired );

    // another simulated response from the "signer" service. The URL is now different, with new expiration
    KTime_t newExpTime = KTimeStamp () + 65;
    string newAwsHost = "ELSEWHERE.in.the.cloud";
    RespondWithRedirect ( MakeURL( newAwsHost ), newExpTime );
    RespondToGET(); // will be converted to POST

    // this Read will notice that the expiration time is nigh, use the original URL and get redirected to the "signer" for a new temporary URL
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );
    REQUIRE_EQ ( newExpTime, KTimeMakeTime ( & httpFile . url_expiration ) );

    {   // verify that the second to last request was done on the original URL,
        // as a POST with a CE token
        string req = * ( ++ TestStream::m_requests . rbegin() );
        REQUIRE ( StringPresent ( req, GetName() ) );
        REQUIRE ( EnvironmentTokenPresent ( req ) );
        REQUIRE ( StringPresent ( req, "POST" ) );
    }

    {   // verify that the last request was done on the new redirected URL with a GET
        // without the token
        string req = TestStream::m_requests . back();
        REQUIRE ( StringPresent ( req, newAwsHost ) );
        REQUIRE ( ! EnvironmentTokenPresent ( req ) );
        REQUIRE ( StringPresent ( req, "GET" ) );
    }

    // the next Read (right away) will not refresh the URL or expiration
    RespondToGET();
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );
    REQUIRE_EQ ( newExpTime, KTimeMakeTime ( & httpFile . url_expiration ) ); // expiration did not change
    // verify that the last request was done on the same redirected URL
    REQUIRE ( StringPresent ( TestStream::m_requests . back(), newAwsHost ) );
}
#endif

#if NOT_IMPLEMENTED
FIXTURE_TEST_CASE( HttpRefreshTestSuite_ReadCloseToExpiration_AWS_NoToken_Payer, CloudFixture )
{
    //TestEnv::verbosity = LogLevel::e_message;
    string url = MakeURL(GetName());

    const struct KHttpFile& httpFile = SetUpForExpiration ( url, ! EnvTokenRequired, PayRequired );

    // another simulated response from the "signer" service. The URL is now different, with new expiration
    KTime_t newExpTime = KTimeStamp () + 65;
    string newAwsHost = "ELSEWHERE.in.the.cloud";
    RespondWithRedirect ( MakeURL( newAwsHost ), newExpTime );
    RespondToGET(); // will be converted to POST

    // this Read will notice that the expiration time is nigh, use the original URL and get redirected to the "signer" for a new temporary URL
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );
    REQUIRE_EQ ( newExpTime, KTimeMakeTime ( & httpFile . url_expiration ) );

    {   // verify that the second to last request was done on the original URL,
        // as a POST with a CE token
        string req = * ( ++ TestStream::m_requests . rbegin() );
        REQUIRE ( StringPresent ( req, GetName() ) );
        REQUIRE ( ! EnvironmentTokenPresent ( req ) );
        REQUIRE ( StringPresent ( req, "POST" ) );
    }

    {   // verify that the last request was done on the new redirected URL with a GET
        // without the token
        string req = TestStream::m_requests . back();
        REQUIRE ( StringPresent ( req, newAwsHost ) );
        REQUIRE ( ! EnvironmentTokenPresent ( req ) );
        REQUIRE ( StringPresent ( req, "GET" ) );
    }

    // the next Read (right away) will not refresh the URL or expiration
    RespondToGET();
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );
    REQUIRE_EQ ( newExpTime, KTimeMakeTime ( & httpFile . url_expiration ) ); // expiration did not change
    // verify that the last request was done on the same redirected URL
    REQUIRE ( StringPresent ( TestStream::m_requests . back(), newAwsHost ) );
}
//TODO
//FIXTURE_TEST_CASE( HttpRefreshTestSuite_ReadCloseToExpiration_AWS_NoToken_NoPayer, CloudFixture )
//FIXTURE_TEST_CASE( HttpRefreshTestSuite_ReadCloseToExpiration_GCP_xxToken_xxPayer, CloudFixture )

#endif

FIXTURE_TEST_CASE( HttpRefreshTestSuite_HeadAsPost_ShortFile, CloudFixture )
{
    string url = MakeURL(GetName());

    // HEAD will be converted to POST and return the initial portion of the target file
    string data (10, 'a');// this is shorter than POST will request (256)
    RespondToHEAD(data);

    MakeHttpFile( url, EnvTokenRequired, ! PayRequired );

    RespondToGET_Full(data); // return the complete file
    REQUIRE_RC( KFileTimedRead ( m_file, 0, m_buf, sizeof m_buf, & num_read, NULL ) );
}

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
        //if ( 1 ) assert ( ! KDbgSetString ( "KNS-HTTP" ) );
        KConfigDisableUserSettings ();

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

