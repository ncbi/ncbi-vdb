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
* Unit tests for the Cloud interface
*/

#include <ktst/unit_test.hpp>

#include <cloud/aws.h>
#include <cloud/gcp.h>

#include <../../libs/cloud/cloud-priv.h>

#include <kfg/config.h>

#include <fstream>

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(CloudTestSuite, argsHandler);

using namespace std;
using namespace ncbi::NK;

//////////////////////////////////////////// CloudMgr

TEST_CASE(MgrMake_NullParam)
{
    REQUIRE_RC_FAIL ( CloudMgrMake ( NULL ) );
}

TEST_CASE(MgrRelease_NullParam)
{
    REQUIRE_RC ( CloudMgrRelease ( NULL ) );
}

TEST_CASE(MgrMake_Release)
{
    CloudMgr * mgr;
    REQUIRE_RC ( CloudMgrMake ( & mgr ) );
    REQUIRE_NOT_NULL ( mgr );
    REQUIRE_RC ( CloudMgrRelease ( mgr ) );
}

TEST_CASE(MgrAddRef_NullParam)
{
    REQUIRE_RC ( CloudMgrAddRef ( NULL ) );
}

TEST_CASE(MgrCurrentProvider_NullSelf)
{
    CloudProviderId id;
    REQUIRE_RC_FAIL ( CloudMgrCurrentProvider ( NULL, & id ) );
}

class CloudMgrFixture
{
public:
    CloudMgrFixture ()
    :   m_mgr ( nullptr ),
        m_id ( cloud_num_providers ),
        m_cloud ( nullptr )
    {
        THROW_ON_RC ( CloudMgrMake ( & m_mgr ) );
    }
    ~CloudMgrFixture ()
    {
        if ( CloudMgrRelease ( m_mgr ) != 0 )
        {
            cout << "CloudMgrFixture::~CloudMgrFixture: CloudMgrRelease() failed" << endl;
        }
        if ( CloudRelease ( m_cloud ) != 0 )
        {
            cout << "CloudMgrFixture::~CloudMgrFixture: CloudRelease() failed" << endl;
        }
    }

    CloudMgr * m_mgr;
    CloudProviderId m_id;
    Cloud * m_cloud;
};

FIXTURE_TEST_CASE(MgrCurrentProvider_NullParam, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrCurrentProvider ( m_mgr, NULL ) );
}
FIXTURE_TEST_CASE(MgrCurrentProvider_Aws, CloudMgrFixture)
{
    // need to have a predictable providerId in testing, so force it to be AWS here
    CloudMgrSetProvider( m_mgr, cloud_provider_aws );
    REQUIRE_RC ( CloudMgrCurrentProvider ( m_mgr, & m_id ) );
    REQUIRE_EQ ( (int)cloud_provider_aws, (int)m_id );
}

FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCloud_NullSelf, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrMakeCloud ( NULL, & m_cloud, cloud_provider_aws ) );
}
FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCloud_NullParam, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrMakeCloud ( m_mgr, NULL, cloud_provider_aws ) );
}
FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCloud_BadParam_NoCloud, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_none ) );
}
FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCloud_BadParam, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_num_providers ) );
}

FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCurrentCloud_NullSelf, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrMakeCurrentCloud ( NULL, & m_cloud ) );
}
FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCurrentCloud_NullParam, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrMakeCurrentCloud ( m_mgr, NULL ) );
}
FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCurrentCloud, CloudMgrFixture)
{
    // need to have a predictable providerId in testing, so force it to be AWS here
    CloudMgrSetProvider( m_mgr, cloud_provider_aws );
    REQUIRE_RC ( CloudMgrMakeCurrentCloud ( m_mgr, & m_cloud ) );
    // to verify, try to cast to AWS
    AWS * aws;
    REQUIRE_RC ( CloudToAWS ( m_cloud, & aws ) );
    REQUIRE_NOT_NULL ( aws );
    REQUIRE_RC ( AWSRelease ( aws ) );
}

//////////////////////////////////////////// AWS

class AwsFixture : public CloudMgrFixture
{
public:
    AwsFixture()
    : m_aws ( nullptr )
    {
        unsetenv ( "AWS_ACCESS_KEY_ID" );
        unsetenv ( "AWS_SECRET_ACCESS_KEY" );
        unsetenv ( "AWS_PROFILE" );
        unsetenv ( "AWS_CONFIG_FILE" );
        unsetenv ( "AWS_SHARED_CREDENTIAL_FILE" );
        unsetenv ( "VDB_CONFIG" );
    }
    ~AwsFixture()
    {
        if ( AWSRelease ( m_aws ) != 0 )
        {
            cout << "AwsFixture::~AwsFixture: AWSRelease() failed" << endl;
        }
    }

    void CheckKeys ( const char * key, const char * secret_key, const char * region = nullptr, const char * output = nullptr )
    {
        MakeAWS();

        THROW_ON_FALSE ( m_aws -> access_key_id != NULL );
        THROW_ON_FALSE ( m_aws -> secret_access_key != NULL );

        THROW_ON_FALSE ( string ( key ) == string ( m_aws -> access_key_id ) );
        THROW_ON_FALSE ( string ( secret_key ) == string ( m_aws -> secret_access_key ) );

        if ( region != nullptr )
        {
            THROW_ON_FALSE ( m_aws -> region != NULL );
            THROW_ON_FALSE ( string ( region ) == string ( m_aws -> region ) );
        }
        if ( output != nullptr )
        {
            THROW_ON_FALSE ( m_aws -> output != NULL );
            THROW_ON_FALSE ( string ( output ) == string ( m_aws -> output ) );
        }
    }

    void MakeAWS()
    {
        CloudMgrSetProvider( m_mgr, cloud_provider_aws );
        THROW_ON_RC ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_aws ) );
        THROW_ON_FALSE ( nullptr != m_cloud );
        THROW_ON_RC ( CloudToAWS ( m_cloud, & m_aws ) );
        THROW_ON_FALSE ( nullptr != m_aws );
    }

    void CreateFile ( const string& p_name, const string& p_content )
    {
        ofstream out( p_name . c_str() );
        out << p_content;
    }    

    AWS * m_aws;
};

FIXTURE_TEST_CASE(AWS_Make, CloudMgrFixture)
{
    CloudMgrSetProvider( m_mgr, cloud_provider_aws );
    REQUIRE_RC ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_aws ) );
    REQUIRE_NOT_NULL ( m_cloud );
}

FIXTURE_TEST_CASE(AWS_CloudToAws_NullSelf, AwsFixture)
{
    REQUIRE_RC_FAIL ( CloudToAWS ( NULL, & m_aws ) );
}
FIXTURE_TEST_CASE(AWS_CloudToAws_NullParam, AwsFixture)
{
    REQUIRE_RC_FAIL ( CloudToAWS ( m_cloud, NULL ) );
}
FIXTURE_TEST_CASE(AWS_CloudToAws, AwsFixture)
{
    CloudMgrSetProvider( m_mgr, cloud_provider_aws );
    REQUIRE_RC ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_aws ) );
    REQUIRE_NOT_NULL ( m_cloud );
    REQUIRE_RC ( CloudToAWS ( m_cloud, & m_aws ) );
    REQUIRE_NOT_NULL ( m_aws );
}
FIXTURE_TEST_CASE(AWS_CloudToAws_Fail, AwsFixture)
{
    CloudMgrSetProvider( m_mgr, cloud_provider_gcp );
    REQUIRE_RC ( CloudRelease ( m_cloud ) );
    REQUIRE_RC ( CloudMgrMakeCurrentCloud ( m_mgr, & m_cloud ) );

    REQUIRE_RC_FAIL ( CloudToAWS ( m_cloud, & m_aws ) );
}

FIXTURE_TEST_CASE(AWS_ToCloud_NullSelf, AwsFixture)
{
    Cloud * cloud;
    REQUIRE_RC_FAIL ( AWSToCloud ( NULL, & cloud ) );
}
FIXTURE_TEST_CASE(AWS_ToCloud_NullParam, AwsFixture)
{
    MakeAWS();
    REQUIRE_RC_FAIL ( AWSToCloud ( m_aws, NULL ) );
}
FIXTURE_TEST_CASE(AWS_ToCloud, AwsFixture)
{
    MakeAWS();

    Cloud * cloud;
    REQUIRE_RC ( AWSToCloud ( m_aws, & cloud ) );
    REQUIRE_NOT_NULL ( m_aws );
    REQUIRE_RC ( CloudRelease ( cloud ) );
}

FIXTURE_TEST_CASE(AWS_Credentials_Blank, AwsFixture)
{
    // block cloud discovery to make sure the credentials are left blank
    MakeAWS();

    REQUIRE_NULL ( m_aws -> access_key_id );
    REQUIRE_NULL ( m_aws -> secret_access_key );
    REQUIRE_NULL ( m_aws -> region );
    REQUIRE_NULL ( m_aws -> output );
}

FIXTURE_TEST_CASE(AWS_Credentials_Env, AwsFixture)
{
    const char *test_key = "ABCDEFH";
    const char *test_secret = "SECRETSECRET";

    setenv ( "AWS_ACCESS_KEY_ID", test_key, 1 );
    setenv ( "AWS_SECRET_ACCESS_KEY", test_secret, 1 );

    CheckKeys( test_key, test_secret );
}

FIXTURE_TEST_CASE(AWS_Credentials_AwsConfigFile_DefaultProfile, AwsFixture)
{
    setenv ( "AWS_CONFIG_FILE", "cloud-kfg/aws_other_config", 1 );

    CheckKeys( "ABC123", "SECRET", "reg", "out" );
}

FIXTURE_TEST_CASE(AWS_Credentials_AwsConfigFile_ProfileFromEnv, AwsFixture)
{
    setenv ( "AWS_PROFILE", "other_profile", 1 );
    setenv ( "AWS_CONFIG_FILE", "cloud-kfg/aws_other_config", 1 );

    CheckKeys( "ABC123_OTHER", "SECRET_OTHER", "reg_OTHER", "out_OTHER" );
}

FIXTURE_TEST_CASE(AWS_Credentials_AwsConfigFile_ProfileFromKfg, AwsFixture)
{
    setenv ( "VDB_CONFIG", "cloud-kfg/aws.kfg", 1 );
    setenv ( "AWS_CONFIG_FILE", "cloud-kfg/aws_other_config", 1 );

    CheckKeys( "ABC123_OTHER", "SECRET_OTHER" );
}

FIXTURE_TEST_CASE(AWS_Credentials_AwsConfigFile_ProfileFromKfg_IsEmpty, AwsFixture)
{
    setenv ( "VDB_CONFIG", "cloud-kfg/empty", 1 );
    setenv ( "AWS_CONFIG_FILE", "cloud-kfg/aws_other_config", 1 );

    CheckKeys( "ABC123", "SECRET" );
}

FIXTURE_TEST_CASE(AWS_Credentials_AwsSharedCredentialFile, AwsFixture)
{
    setenv ( "AWS_SHARED_CREDENTIAL_FILE", "cloud-kfg/aws_other_config", 1 );
    setenv ( "AWS_PROFILE", "other_profile", 1 );

    CheckKeys( "ABC123_OTHER", "SECRET_OTHER" );
}

FIXTURE_TEST_CASE(AWS_Credentials_AwsUserHomeCredentials, AwsFixture)
{
    setenv ( "HOME", "./cloud-kfg", 1 );

    KConfig * kfg;
    REQUIRE_RC ( KConfigMake( & kfg, NULL ) );    
    const KConfigNode * home_node;
    REQUIRE_RC ( KConfigOpenNodeRead ( kfg, &home_node, "HOME" ) );
    char path[4096];
    size_t num_read;
    REQUIRE_RC ( KConfigNodeRead ( home_node, 0, path, sizeof path, &num_read, NULL ) );

    string home ( path, num_read );
    CreateFile ( home + "/.aws/credentials", "[default]\naws_access_key_id = ABC123\naws_secret_access_key = SECRET\n" );
    remove( (home + "/.aws/config") . c_str() ); // otherwise it may override

    CheckKeys( "ABC123", "SECRET" );
}

FIXTURE_TEST_CASE(AWS_Credentials_AwsUserHomeCredentials_ConfigOverrides, AwsFixture)
{
    setenv ( "HOME", "./cloud-kfg", 1 );

    KConfig * kfg;
    REQUIRE_RC ( KConfigMake( & kfg, NULL ) );    
    const KConfigNode * home_node;
    REQUIRE_RC ( KConfigOpenNodeRead ( kfg, &home_node, "HOME" ) );
    char path[4096];
    size_t num_read;
    REQUIRE_RC ( KConfigNodeRead ( home_node, 0, path, sizeof path, &num_read, NULL ) );

    string home ( path, num_read );
    CreateFile ( home + "/.aws/credentials", "[default]\naws_access_key_id = ABC123\naws_secret_access_key = SECRET\n" );
    // ~/.aws/config overrides if present
    CreateFile ( home + "/.aws/config", "[default]\naws_access_key_id = ABC123_CFG\naws_secret_access_key = SECRET_CFG\n" );
    CheckKeys( "ABC123_CFG", "SECRET_CFG" );
}

//TODO: add region, output

// CLOUD_EXTERN rc_t CC CloudMakeComputeEnvironmentToken ( const Cloud * self, struct String const ** ce_token );
// CLOUD_EXTERN rc_t CC CloudAddComputeEnvironmentTokenForSigner ( const Cloud * self, struct KClientHttpRequest * req );
// CLOUD_EXTERN rc_t CC CloudAddUserPaysCredentials ( const Cloud * self, struct KClientHttpRequest * req );

//////////////////////////////////////////// GCP

class GcpFixture : public CloudMgrFixture
{
public:
    GcpFixture()
    : m_gcp ( nullptr )
    {
    }
    ~GcpFixture()
    {
        if ( GCPRelease ( m_gcp ) != 0 )
        {
            cout << "GcpFixture::~GcpFixture: GCPRelease() failed" << endl;
        }
    }

    void MakeGCP()
    {
        CloudMgrSetProvider( m_mgr, cloud_provider_gcp );
        THROW_ON_RC ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_gcp ) );
        THROW_ON_FALSE ( nullptr != m_cloud );
        THROW_ON_RC ( CloudToGCP ( m_cloud, & m_gcp ) );
        THROW_ON_FALSE ( nullptr != m_gcp );
    }

    GCP * m_gcp;
};

FIXTURE_TEST_CASE(GCP_Make, GcpFixture)
{
    CloudMgrSetProvider( m_mgr, cloud_provider_gcp );
    REQUIRE_RC ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_gcp ) );
    REQUIRE_NOT_NULL ( m_cloud );
}

FIXTURE_TEST_CASE(GCP_CloudToGcp_NullSelf, GcpFixture)
{
    REQUIRE_RC_FAIL ( CloudToGCP ( NULL, & m_gcp ) );
}
FIXTURE_TEST_CASE(GCP_CloudToGcp_NullParam, GcpFixture)
{
    REQUIRE_RC_FAIL ( CloudToGCP ( m_cloud, NULL ) );
}
FIXTURE_TEST_CASE(GCP_CloudToGcp, GcpFixture)
{
    CloudMgrSetProvider( m_mgr, cloud_provider_gcp );
    REQUIRE_RC ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_gcp ) );
    REQUIRE_NOT_NULL ( m_cloud );
    REQUIRE_RC ( CloudToGCP ( m_cloud, & m_gcp ) );
    REQUIRE_NOT_NULL ( m_gcp );
}
FIXTURE_TEST_CASE(GCP_CloudToGcp_Fail, GcpFixture)
{
    CloudMgrSetProvider( m_mgr, cloud_provider_aws ); // wrong cloud
    REQUIRE_RC ( CloudRelease ( m_cloud ) );
    REQUIRE_RC ( CloudMgrMakeCurrentCloud ( m_mgr, & m_cloud ) );

    REQUIRE_RC_FAIL ( CloudToGCP ( m_cloud, & m_gcp ) );
}

FIXTURE_TEST_CASE(GCP_ToCloud_NullSelf, GcpFixture)
{
    Cloud * cloud;
    REQUIRE_RC_FAIL ( GCPToCloud ( NULL, & cloud ) );
}
FIXTURE_TEST_CASE(GCP_ToCloud_NullParam, GcpFixture)
{
    MakeGCP();
    REQUIRE_RC_FAIL ( GCPToCloud ( m_gcp, NULL ) );
}
FIXTURE_TEST_CASE(GCP_ToCloud, GcpFixture)
{
    MakeGCP();

    Cloud * cloud;
    REQUIRE_RC ( GCPToCloud ( m_gcp, & cloud ) );
    REQUIRE_NOT_NULL ( m_gcp );
    REQUIRE_RC ( CloudRelease ( cloud ) );
}

FIXTURE_TEST_CASE(GCP_Credentials_Blank, GcpFixture)
{
    // block cloud discovery to make sure the credentials are left blank
    unsetenv ( "GOOGLE_APPLICATION_CREDENTIALS" );
    MakeGCP();

    REQUIRE_NULL ( m_gcp -> privateKey );
    REQUIRE_NULL ( m_gcp -> client_email );
}

FIXTURE_TEST_CASE(GCP_Credentials, GcpFixture)
{
    setenv ( "GOOGLE_APPLICATION_CREDENTIALS", "cloud-kfg/gcp_service.json", 1 );
    MakeGCP();

    REQUIRE_NOT_NULL ( m_gcp -> privateKey );
    REQUIRE_NOT_NULL ( m_gcp -> client_email );

    const string private_key = "-----BEGIN NOTPRIVATE KEY";
    const string client_email = "ncbivdb-compute@developer.gserviceaccount.com";
    REQUIRE_EQ ( private_key, string ( m_gcp -> privateKey ) . substr ( 0, private_key . size () ) );
    REQUIRE_EQ ( client_email, string ( m_gcp -> client_email ) );
}

// CLOUD_EXTERN rc_t CC CloudMakeComputeEnvironmentToken ( const Cloud * self, struct String const ** ce_token );
// CLOUD_EXTERN rc_t CC CloudAddComputeEnvironmentTokenForSigner ( const Cloud * self, struct KClientHttpRequest * req );
// CLOUD_EXTERN rc_t CC CloudAddUserPaysCredentials ( const Cloud * self, struct KClientHttpRequest * req );

//////////////////////////////////////////// Main

#include <kapp/args.h>

static rc_t argsHandler(int argc, char * argv[]) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

#include <kfg/config.h>
#include <klib/debug.h>

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
    KConfigDisableUserSettings();

	// this makes messages from the test code appear
	// (same as running the executable with "-l=message")
	//TestEnv::verbosity = LogLevel::e_message;

    rc_t rc=CloudTestSuite(argc, argv);
    return rc;
}

}
