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
* Tests for libs/cloud/gcp.c
*/

#include <cloud/cloud-priv.h>
#include <cloud/manager.h>

#include <kns/manager.h>
#include <kns/http.h>
#include <kns/http-priv.h>

#include <klib/text.h>
#include <klib/data-buffer.h>

#include <kapp/args.h> /* ArgsMakeAndHandle */
#include <kfg/config.h> /* KConfigDisableUserSettings */

#include <ktst/unit_test.hpp>

#include <iostream>
#include <sstream>

#include <../libs/cloud/gcp-priv.h>
#include <../libs/cloud/cloud-priv.h>
#include <../libs/kns/http-priv.h>

#include "../kns/HttpFixture.hpp" //TODO: mobe TestStream to a better place

using namespace std;

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(GcpTestSuite, argsHandler)

TEST_CASE(GCP_AddUserPays_NoCredentials)
{
    CloudMgr * mgr;
    REQUIRE_RC ( CloudMgrMakeWithProvider ( & mgr, cloud_provider_gcp ) );

    // no user credentials
    char env[1024];
    strcpy(env, "GOOGLE_APPLICATION_CREDENTIALS=" );
    REQUIRE_EQ ( 0, putenv ( env ) );

    Cloud * cloud;
    REQUIRE_RC ( CloudMgrMakeCloud ( mgr, &cloud, cloud_provider_gcp ) );
    KClientHttp * client;
    {
        KNSManager * kns;
        REQUIRE_RC ( KNSManagerMake ( & kns ) );
        String host;
        CONST_STRING( &host, "www.googleapis.com" );
        REQUIRE_RC ( KNSManagerMakeClientHttps ( kns, &client, NULL, 0x01010000, & host, 443 ) );
        REQUIRE_RC ( KNSManagerRelease ( kns ) );
    }

    KClientHttpRequest * req;
    REQUIRE_RC ( KClientHttpMakeRequest ( client, & req, "https://www.googleapis.com/oauth2/v4/token" ) );

    REQUIRE_RC_FAIL ( CloudAddUserPaysCredentials ( cloud, req, "POST" ) );

    REQUIRE_RC ( KClientHttpRelease ( client ) );
    REQUIRE_RC ( KClientHttpRequestRelease ( req ) );
    REQUIRE_RC ( CloudRelease ( cloud ) );
    REQUIRE_RC ( CloudMgrRelease ( mgr ) );
}

TEST_CASE(GCP_AddUserPays_Credentials)
{
    char env[1024];
    strcpy(env, "GOOGLE_APPLICATION_CREDENTIALS=./cloud-kfg/gcp_service.json" );
    REQUIRE_EQ ( 0, putenv ( env ) );

    CloudMgr * mgr;
    REQUIRE_RC ( CloudMgrMakeWithProvider ( & mgr, cloud_provider_gcp ) );

    Cloud * cloud;
    REQUIRE_RC ( CloudMgrMakeCloud ( mgr, &cloud, cloud_provider_gcp ) );

    GCP * gcp;
    REQUIRE_RC ( CloudToGCP ( cloud, & gcp ) );
    REQUIRE_NOT_NULL ( gcp );
    string PK = "-----BEGIN PRIVATE KEY-----\nMIICdwIBADANBgkqhkiG9w0BAQEFA";
    REQUIRE_NOT_NULL ( gcp -> privateKey );
    REQUIRE_EQ ( PK, string( gcp -> privateKey ) . substr( 0, PK . size() ) );
    REQUIRE_NOT_NULL ( gcp -> client_email );
    REQUIRE_EQ ( string("ncbivdb-compute@developer.gserviceaccount.com"), string( gcp -> client_email ) );
    REQUIRE_NOT_NULL ( gcp -> project_id );
    REQUIRE_EQ ( string("test"), string( gcp -> project_id ) );

    REQUIRE_RC ( GCPRelease ( gcp ) );
    REQUIRE_RC ( CloudRelease ( cloud ) );
    REQUIRE_RC ( CloudMgrRelease ( mgr ) );
}



TEST_CASE(GCP_Sign_RSA_SHA256)
{
    // same (throwaway) private key as in cloud-kfg/gcp-service.json
    const char * key =
"-----BEGIN PRIVATE KEY-----\n"
"MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBANoWq8DqARNncY/f\n"
"PFcRKhZ4MueoAPwsfFsrDaifNSW0wfemzAz/N8kkHqyqX2ZdGOaszKI3qBqZ2qxJ\n"
"d7FEDDbOoc8ezFvN5if3gla9vap8rc1Rjb6x31HSbWdPDvnZg+QTI2ASwk45354w\n"
"ocs6BNmSwXn1pZr/OvAuLv3WIXwjAgMBAAECgYA1jY2dcJjVB/jF6H5rruZT4C43\n"
"3nRneBENXhQbjQTC/pEG3CmNI3qyZLE3mxqUC1ZbBqG1T89ywMcGuX+vwtLwiisd\n"
"mF50WSS47neaWtxPcpemk3c/fX2ezrdQ2MafhwJQ4266inxOi+ZRpySiixVCf0Zi\n"
"9kbV4u8D93g0HNDFuQJBAPrFGTphfrCsDkj8SwvN244tzVyPS3aqNrXUVk6soXkq\n"
"Pc+NE+4wrmXfs7HSgSRLMTSKOrS97X5lEPGiLJicfIUCQQDeoxTOCOTJuiehL0HK\n"
"CsAz6z7qzM2rdxXrpgGV7Gbj0ATi0+A9FbRAB6d4NBm262AbhHFN0zvlMplAZaC4\n"
"1iqHAkEAh4+tB6ZUumUIg+P/Ha53FfEkpvi/rrJtSPY0getIjxuKtnxpXcXrQR8h\n"
"IOvO7DPJscFX4wUZMc6ozjcBLK7LkQJAElrJjU7oZcUmHUDDIMAQJnefgUYPMrKn\n"
"qPzPpqeNt/xfWr/y/bY7XQgEg4FwGUeAbeRWXv8qMfQg9FEslfB6IwJBAIi+wddr\n"
"T0xa+PdDKciepM0k1zyno5Pn5ltW58ncbzdwdCtanB+xoKPzOtZFcHRdL99HDXpk\n"
"vvleyN6iF4oqO3M=\n"
"-----END PRIVATE KEY-----\n";
    const char * input = "TBD";
    const String * output;
    REQUIRE_RC ( Sign_RSA_SHA256( key, input, & output) );
    REQUIRE_NOT_NULL ( output );

    // verify output
    unsigned char expected[128] = {
        55, 81, 6, 159, 56, 117, 47, 203,
        134, 12, 141, 35, 129, 71, 28, 25,
        156, 54, 124, 188, 4, 160, 157, 122,
        158, 25, 70, 41, 36, 46, 206, 172,
        170, 184, 74, 21, 166, 255, 100, 68,
        118, 171, 6, 154, 50, 6, 35, 135,
        74, 247, 87, 197, 175, 52, 144, 60,
        51, 176, 195, 26, 255, 198, 14, 247,
        187, 172, 38, 162, 108, 112, 145, 237,
        99, 177, 171, 44, 33, 213, 130, 18,
        57, 42, 229, 64, 164, 129, 130, 189,
        205, 204, 1, 245, 89, 113, 50, 2,
        246, 164, 53, 155, 69, 202, 201, 117,
        204, 147, 57, 128, 136, 233, 121, 17,
        80, 220, 239, 243, 236, 156, 59, 64,
        3, 222, 238, 31, 146, 4, 43, 71
    };
    REQUIRE_EQ ( sizeof ( expected ), output -> size );
    REQUIRE_EQ ( 0, memcmp(expected, output->addr, output->size) );
    StringWhack ( output );
}

TEST_CASE(GCP_AddUserPays)
{
    // prepare user credentials
    char env[1024];
    strcpy(env, "GOOGLE_APPLICATION_CREDENTIALS=./cloud-kfg/gcp_service.json" );
    REQUIRE_EQ ( 0, putenv ( env ) );

    CloudMgr * mgr;
    REQUIRE_RC ( CloudMgrMakeWithProvider ( & mgr, cloud_provider_gcp ) );

    Cloud * cloud;
    REQUIRE_RC ( CloudMgrMakeCloud ( mgr, &cloud, cloud_provider_gcp ) );

    KClientHttp * client;
    {
        KNSManager * kns;
        REQUIRE_RC ( KNSManagerMake ( & kns ) );
        String host;
        CONST_STRING( &host, "storage.googleapis.com" );
        REQUIRE_RC ( KNSManagerMakeClientHttps ( kns, &client, NULL, 0x01010000, & host, 443 ) );
        REQUIRE_RC ( KNSManagerRelease ( kns ) );
    }

    KClientHttpRequest * req;
    REQUIRE_RC ( KClientHttpMakeRequest ( client, & req, "https://storage.googleapis.com/sra-pub-run-1/DRR000711/DRR000711.1" ) );

    static KStream m_stream;
    REQUIRE_RC ( KStreamInit ( & m_stream, ( const KStream_vt* ) & TestStream::vt, "TestStream", "", true, true ) );
    string json =
        "{\"access_token\" : \"bogustokenmadefortesting\","
        "  \"token_type\" : \"Bearer\","
        "   \"expires_in\" : 3600"
        "}";
    ostringstream ostr;
    ostr << "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " << json.size() << "\r\n"
        "\r\n" << json << "\r\n";
    TestStream::m_responses.push_back(ostr.str());
    // to have GCP contact Google authorization server for real, comment out this line:
    // and copy a user credentials file to ./cloud-kfg/gcp_service.json (do not check in!)
    CloudSetHttpConnection( cloud, & m_stream );

    REQUIRE_RC ( CloudAddUserPaysCredentials ( cloud, req, "POST" ) );
    // adds header:
    // Authorization: Bearer <access_token>
    char msg[4096];
    size_t len;
    REQUIRE_RC ( KClientHttpRequestFormatPostMsg( req, msg, sizeof ( msg ), & len ) );
// this will show the access token, either bogusfortesting or the real one
cout << msg << endl;
// if the token is real, the following check will fail. please do not "fix".
    REQUIRE_NE ( string::npos, string( msg ).find( "Authorization: Bearer bogustokenmadefortesting" ) );
    // URL needs contain "alt=media&userProject=<project_id>" as parameters"
    const char  * body = KClientHttpRequestGetBody ( req );
    REQUIRE_NE ( string::npos, string( body ).find( "alt=media" ) );
    REQUIRE_NE ( string::npos, string( body ).find( "userProject=test" ) );

    REQUIRE_RC ( KClientHttpRelease ( client ) );
    REQUIRE_RC ( KClientHttpRequestRelease ( req ) );
    REQUIRE_RC ( CloudRelease ( cloud ) );
    REQUIRE_RC ( CloudMgrRelease ( mgr ) );
}

//////////////////////////////////////////// Main

static rc_t argsHandler(int argc, char * argv[]) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
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
const char UsageDefaultName[] = "test-gcp";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();

    // this makes messages from the test code appear
    // (same as running the executable with "-l=message")
    //TestEnv::verbosity = LogLevel::e_message;

#ifdef TO_SHOW_RESULTS
    assert(!KDbgSetString("KNS"));
#endif

    return GcpTestSuite(argc, argv);
}

}
