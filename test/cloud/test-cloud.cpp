/*==============================================================================
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
* Unit tests for the Cloud interface
*/

#include <ktst/unit_test.hpp>

#include <cloud/aws.h>
#include <cloud/gcp.h>

#include <../../libs/cloud/cloud-priv.h>

#include <kfg/kfg-priv.h>
#include <kfs/directory.h> /* KDirectoryRelease */
#include <klib/rc.h>

#include <os-native.h>

#include <fstream>

#define PATH_MAX 4096

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(CloudTestSuite, argsHandler);

using namespace std;
using namespace ncbi::NK;

#define ALL

//////////////////////////////////////////// CloudMgr

#ifdef ALL
TEST_CASE(MgrMake_NullParam)
{
    REQUIRE_RC_FAIL ( CloudMgrMake ( NULL, NULL, NULL ) );
}

TEST_CASE(MgrRelease_NullParam)
{
    REQUIRE_RC ( CloudMgrRelease ( NULL ) );
}

TEST_CASE(MgrMake_Release)
{
    CloudMgr * mgr;
    REQUIRE_RC ( CloudMgrMake ( & mgr, NULL, NULL ) );
    REQUIRE_NOT_NULL ( mgr );
    REQUIRE_RC ( CloudMgrRelease ( mgr ) );
}
//TODO: provide KFG, KNS to CloudMgrMake()

TEST_CASE(MgrAddRef_NullParam)
{
    REQUIRE_RC ( CloudMgrAddRef ( NULL ) );
}

TEST_CASE(MgrCurrentProvider_NullSelf)
{
    CloudProviderId id;
    REQUIRE_RC_FAIL ( CloudMgrCurrentProvider ( NULL, & id ) );
}
#endif

#define CONFIG "./cloud-kfg/tmp.kfg"

class CloudMgrFixture
{
    string m_created[4];
public:
    CloudMgrFixture ()
    :   m_mgr ( nullptr ),
        m_id ( cloud_num_providers ),
        m_cloud ( nullptr ),
        m_dir ( nullptr )
    {
        THROW_ON_RC ( CloudMgrMake ( & m_mgr, NULL, NULL ) );
        THROW_ON_RC ( KDirectoryNativeDir ( & m_dir ) );
    }
    ~CloudMgrFixture ()
    {
        for (unsigned i = 0; i < sizeof m_created / sizeof m_created[0]; ++i)
            if (!m_created[i].empty()) {
                if (KDirectoryRemove(m_dir, false, m_created[i].c_str()) != 0)
                  cout << "CloudMgrFixture::~CloudMgrFixture: KDirectoryRemove("
                    << m_created[i] << ") failed" << endl;
            }
            else break;

        if ( CloudMgrRelease ( m_mgr ) != 0 )
        {
            cout << "CloudMgrFixture::~CloudMgrFixture: CloudMgrRelease() failed" << endl;
        }
        if ( CloudRelease ( m_cloud ) != 0 )
        {
            cout << "CloudMgrFixture::~CloudMgrFixture: CloudRelease() failed" << endl;
        }
        if (KDirectoryRelease(m_dir) != 0)
        {
            cout <<
                "CloudMgrFixture::~CloudMgrFixture: KDirectoryRelease() failed"
                << endl;
        }
    }

    void CreateFile(const string& p_name, const string& p_content,
        const string& p_content2 = "", const string& p_content3 = "")
    {
        ofstream out(p_name.c_str());
        out << p_content << p_content2 << p_content3;
        for (unsigned i = 0; i < sizeof m_created / sizeof m_created[0]; ++i) {
            if (m_created[i].empty()) {
                m_created[i] = p_name;
                return;
            }
        }
        throw "cannot save file name";
    }

    CloudMgr * m_mgr;
    CloudProviderId m_id;
    Cloud * m_cloud;
    KDirectory * m_dir;
};

#ifdef ALL
FIXTURE_TEST_CASE(MgrCurrentProvider_NullParam, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrCurrentProvider ( m_mgr, NULL ) );
}

FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCloud_NullSelf, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrMakeCloud ( NULL, & m_cloud, cloud_provider_aws ) );
    REQUIRE_NULL ( m_cloud );
}
FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCloud_NullParam, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrMakeCloud ( m_mgr, NULL, cloud_provider_aws ) );
}
FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCloud_BadParam_NoCloud, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_none ) );
    REQUIRE_NULL ( m_cloud );
}
FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCloud_BadParam, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_num_providers ) );
    REQUIRE_NULL ( m_cloud );
}

FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCurrentCloud_NullSelf, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrGetCurrentCloud ( NULL, & m_cloud ) );
    REQUIRE_NULL ( m_cloud );
}
FIXTURE_TEST_CASE(MgrCurrentProvider_MakeCurrentCloud_NullParam, CloudMgrFixture)
{
    REQUIRE_RC_FAIL ( CloudMgrGetCurrentCloud ( m_mgr, NULL ) );
}
#endif

//////////////////////////////////////////// AWS

class AwsFixture : public CloudMgrFixture
{
public:
    AwsFixture()
    : m_aws ( nullptr )
    {
        putenv ( (char*)"AWS_ACCESS_KEY_ID=" );
        putenv ( (char*)"AWS_CONFIG_FILE=" );
        putenv ( (char*)"AWS_PROFILE=" );
        putenv ( (char*)"AWS_SECRET_ACCESS_KEY=" );
        putenv ( (char*)"AWS_SHARED_CREDENTIAL_FILE=" );
        putenv ( (char*)"VDB_CONFIG=" );
    }
    ~AwsFixture()
    {
        if ( AWSRelease ( m_aws ) != 0 )
        {
            cout << "AwsFixture::~AwsFixture: AWSRelease() failed" << endl;
        }
    }

    void CheckKeys(
        const char * key = nullptr, const char * secret_key = nullptr,
        const char * region = nullptr, const char * output = nullptr,
        bool accept_charges = true)
    {
        KConfig * kfg = NULL;
        THROW_ON_RC(KConfigMakeLocal(&kfg, NULL));
        if (accept_charges) {
            THROW_ON_RC(KConfigWriteBool(
                kfg, "/libs/cloud/accept_aws_charges", true));
            THROW_ON_RC(KConfigWriteBool(
                kfg, "/libs/cloud/accept_gcp_charges", true));
        }

        MakeAWS(kfg);

        if (key == nullptr && secret_key == nullptr) {
            THROW_ON_FALSE(m_aws->access_key_id == NULL);
            THROW_ON_FALSE(m_aws->secret_access_key == NULL);
        }
        else {
            THROW_ON_FALSE(m_aws->access_key_id != NULL);
            THROW_ON_FALSE(m_aws->secret_access_key != NULL);

            THROW_ON_FALSE(string(key) == string(m_aws->access_key_id));
            THROW_ON_FALSE
            (string(secret_key) == string(m_aws->secret_access_key));

            if (region != nullptr)
            {
                THROW_ON_FALSE(m_aws->region != NULL);
                THROW_ON_FALSE(string(region) == string(m_aws->region));
            }
            if (output != nullptr)
            {
                THROW_ON_FALSE(m_aws->output != NULL);
                THROW_ON_FALSE(string(output) == string(m_aws->output));
            }
        }

        THROW_ON_RC(KConfigRelease(kfg));
    }

    void CheckKeysNoPay(void) { CheckKeys(0, 0, 0, 0, false); }

    void MakeAWS(const KConfig * kfg = nullptr)
    {
        CloudMgrRelease(m_mgr);
        m_mgr = nullptr;
        THROW_ON_RC ( CloudMgrMakeWithProvider( & m_mgr, cloud_provider_aws,
            kfg) );
        THROW_ON_FALSE ( nullptr != m_mgr );
        THROW_ON_RC ( CloudMgrGetCurrentCloud ( m_mgr, & m_cloud ) );
        THROW_ON_FALSE ( nullptr != m_cloud );
        THROW_ON_RC ( CloudToAWS ( m_cloud, & m_aws ) );
        THROW_ON_FALSE ( nullptr != m_aws );
    }

    AWS * m_aws;
};

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Make, CloudMgrFixture)
{
    REQUIRE_RC ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_aws ) );
    if (m_cloud == NULL) {
        CloudProviderId cloud_provider = cloud_provider_none;
        REQUIRE_RC(CloudMgrCurrentProvider(m_mgr, &cloud_provider));
        REQUIRE_EQ(cloud_provider, static_cast<uint32_t>(cloud_provider_gcp));
    }
}

FIXTURE_TEST_CASE(AWS_CloudToAws_NullSelf, AwsFixture)
{
    REQUIRE_RC ( CloudToAWS ( NULL, & m_aws ) );
    REQUIRE_NULL ( m_aws );
}
FIXTURE_TEST_CASE(AWS_CloudToAws_NullParam, AwsFixture)
{
    REQUIRE_RC_FAIL ( CloudToAWS ( m_cloud, NULL ) );
}
FIXTURE_TEST_CASE(AWS_CloudToAws, AwsFixture)
{
    REQUIRE_RC ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_aws ) );
    REQUIRE_NOT_NULL(m_cloud);
    REQUIRE_RC(CloudToAWS(m_cloud, &m_aws));
    REQUIRE_NOT_NULL(m_aws);
}

FIXTURE_TEST_CASE(AWS_CloudToAws_Fail, AwsFixture)
{
    rc_t rc = CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_gcp );
    if ( rc == 0 )
    {
        REQUIRE_NOT_NULL ( m_cloud );
        REQUIRE_RC_FAIL ( CloudToAWS ( m_cloud, & m_aws ) );
    }
    else
    {
        REQUIRE_NULL ( m_cloud );
        REQUIRE_EQ ( ( uint32_t ) GetRCState ( rc ), ( uint32_t ) rcUnsupported );
    }
}

FIXTURE_TEST_CASE(AWS_ToCloud_NullSelf, AwsFixture)
{
    Cloud * cloud;
    REQUIRE_RC ( AWSToCloud ( NULL, & cloud ) );
    REQUIRE_NULL(cloud);
}
FIXTURE_TEST_CASE(AWS_ToCloud_NullParam, AwsFixture)
{
    MakeAWS();
    REQUIRE_RC_FAIL ( AWSToCloud ( m_aws, NULL ) );
}
FIXTURE_TEST_CASE(AWS_ToCloud, AwsFixture)
{
    MakeAWS();
    REQUIRE_NOT_NULL ( m_aws );

    Cloud * cloud;
    REQUIRE_RC ( AWSToCloud ( m_aws, & cloud ) );
    REQUIRE_NOT_NULL ( cloud );
    REQUIRE_RC ( CloudRelease ( cloud ) );
}

FIXTURE_TEST_CASE(AWS_Credentials_Blank, AwsFixture)
{
    // block cloud discovery and home directory lookup
    // to make sure the credentials are left blank
    setenv("HOME",".", 1);

    MakeAWS();

    REQUIRE_NULL ( m_aws -> access_key_id );
    REQUIRE_NULL ( m_aws -> secret_access_key );
    REQUIRE_NULL ( m_aws -> region );
    REQUIRE_NULL ( m_aws -> output );
}

FIXTURE_TEST_CASE(AWS_Credentials_Env, AwsFixture)
{
#define TEST_KEY "ABCDEFH"
#define TEST_SECRET "SECRETSECRET"

    setenv ( "AWS_ACCESS_KEY_ID", TEST_KEY, 1);
    setenv ( "AWS_SECRET_ACCESS_KEY", TEST_SECRET, 1);

    CheckKeys(TEST_KEY, TEST_SECRET);
#undef TEST_KEY
#undef TEST_SECRET
}

FIXTURE_TEST_CASE(AWS_Credentials_AwsConfigFile_DefaultProfile, AwsFixture)
{
    setenv ( "AWS_CONFIG_FILE", "cloud-kfg/aws_other_config", 1 );

    CheckKeys( "ABC123", "SECRET", "reg", "out" );
}
FIXTURE_TEST_CASE
(AWS_Credentials_AwsConfigFile_DefaultProfileNoPay, AwsFixture)
{
    setenv("AWS_CONFIG_FILE", "cloud-kfg/aws_other_config", 1);

    CheckKeysNoPay();
}
FIXTURE_TEST_CASE(AWS_Credentials_AwsConfigFile_ProfileFromEnv, AwsFixture)
{
    setenv ( "AWS_PROFILE", "another_profile", 1 );
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
#endif

#define CREDENTIALS_KFG "cloud-kfg/credentialsKfg"

#ifdef ALL
/* AWS_SHARED_CREDENTIAL_FILE overrides configuration and .aws/c[or]*
   AWS_PROFILE overrides profile from configuration */
FIXTURE_TEST_CASE(AWS_Credentials_AwsSharedCredentialFile, AwsFixture)
{
    // 1) env AWS_SHARED_CREDENTIAL_FILE
    putenv((char*)"AWS_SHARED_CREDENTIAL_FILE=cloud-kfg/aws_other_config");
    putenv((char*)"AWS_PROFILE=another_profile");

    // 2) create ~/.aws/config
    CreateFile("./cloud-kfg/.aws/config",
        "[other_profile]\r\n"
        "aws_access_key_id = ABC123_CFG\r\n"
        "aws_secret_access_key = SECRET_CFG\r\n");

    // 3) create ~/.aws/credentials
#if WINDOWS
    putenv("USERPROFILE=./cloud-kfg");
#else
    putenv((char*)"HOME=./cloud-kfg");
#endif

    CreateFile("./cloud-kfg/.aws/credentials",
        "[other_profile]\r\n"
        "aws_access_key_id = ABC123\r\n"
        "aws_secret_access_key = SECRET\r\n");

    // 4) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    CreateFile(CREDENTIALS_KFG,
        "[other_profile]\r\n"
        "aws_access_key_id = ABC123_KFG\r\n"
        "aws_secret_access_key = SECRET_KFG\r\n");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    CheckKeys( "ABC123_OTHER", "SECRET_OTHER" );
}

// ~/.aws/config overrides ~/.aws/credentials and configuration
FIXTURE_TEST_CASE(AWS_Credentials_AwsUserHomeCredentials_ConfigOverrides, AwsFixture)
{
    // 2) create ~/.aws/config
    // ~/.aws/config overrides if present
    CreateFile("./cloud-kfg/.aws/config",
        "[another_profile]\r\n"
        "aws_access_key_id = ABC123_CFG\r\n"
        "aws_secret_access_key = SECRET_CFG\r\n");

    // 3) create ~/.aws/credentials
#if WINDOWS
    putenv("USERPROFILE=./cloud-kfg");
#else
    putenv((char*)"HOME=./cloud-kfg");
#endif

    CreateFile("./cloud-kfg/.aws/credentials",
        "[another_profile]\r\n"
        "aws_access_key_id = ABC123\r\n"
        "aws_secret_access_key = SECRET\r\n");

    // 4) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    CreateFile(CREDENTIALS_KFG,
        "[another_profile]\r\naws_access_key_id = ABC123_KFG\r\n"
        "aws_secret_access_key = SECRET_KFG\r\n");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    CheckKeys("ABC123_CFG", "SECRET_CFG");
}

// ~/.aws/credentials overrides configuration
FIXTURE_TEST_CASE(AWS_Credentials_AwsUserHomeCredentials, AwsFixture)
{
    // 3) create ~/.aws/credentials
#if WINDOWS
    putenv("USERPROFILE=./cloud-kfg");
#else
    putenv((char*)"HOME=./cloud-kfg");
#endif

    CreateFile("./cloud-kfg/.aws/credentials",
        "[another_profile]\r\n"
        "aws_access_key_id = ABC123\r\n"
        "aws_secret_access_key = SECRET\r\n");

    // 4) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    CreateFile(CREDENTIALS_KFG,
        "[other_profile]\r\n"
        "aws_access_key_id = ABC123_KFG\r\n"
        "aws_secret_access_key = SECRET_KFG\r\n");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    CheckKeys( "ABC123", "SECRET" );
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_KfgCredentialsProfileNotComplete, AwsFixture)
{
    // 4) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    CreateFile(CREDENTIALS_KFG,
        "[another_profile]");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    cerr << "Expect error message: profile is not complete in credentials...\n";
    CheckKeys();
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_KfgCredentials, AwsFixture)
{
    // 4) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    CreateFile(CREDENTIALS_KFG,
        "[another_profile]\r\n"
        "aws_access_key_id = ABC123_KFG\r\n"
        "aws_secret_access_key = SECRET_KFG\r\n");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    CheckKeys("ABC123_KFG", "SECRET_KFG");
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_KfgCredentialsNoProfile, AwsFixture) {
   // 4) create configuration
   char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    CreateFile(CREDENTIALS_KFG,
        "[profile]\r\n"
        "aws_access_key_id = ABC123_KFG\r\n"
        "aws_secret_access_key = SECRET_KFG\r\n");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    cerr << "Expect error message: profile is not found in credentials file \n";
    CheckKeys();
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_KfgCredentialsEmpty, AwsFixture) {
    // 4) create configuration
        char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    CreateFile(CREDENTIALS_KFG,
        "[another_profile]\r\n"
        "aws_access_key_id = \r\n"
        "aws_secret_access_key = \r\n");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    CheckKeys("", "");
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_KfgCvsCredentials, AwsFixture) {
    // 4.1) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    // CSV credentials file
    CreateFile(CREDENTIALS_KFG,
        "One,Secret access key,Two,Access key ID,three\r\n"
        "111,SECRET_ACCESS_KEY,222,ACCESS_KEY_ID,33333\r\n");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    CheckKeys("ACCESS_KEY_ID", "SECRET_ACCESS_KEY");
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_KfgCvsCredentialsEmpty, AwsFixture) {
    // 4.3) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    // CSV credentials file
    CreateFile(CREDENTIALS_KFG,
        "Secret access key,Access key ID\r\n"
        ",\r\n");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    CheckKeys("", "");
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_NotFound, AwsFixture) {
    // 4.3) create configuration
    char credentialsKfg[PATH_MAX]("");
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    // no credentials file
    CreateFile(CONFIG, "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    CheckKeys();
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_ErrorEmpty, AwsFixture) {
    // 4.3) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
                     sizeof credentialsKfg, CREDENTIALS_KFG));
    // empty credentials file
    CreateFile(CREDENTIALS_KFG, "");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    cerr << "========= Expect error message: credentials file 'XXX' is empty\n";
    CheckKeys();
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_ErrorUnrecognized, AwsFixture) {
    // 4.3) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
                                  sizeof credentialsKfg, CREDENTIALS_KFG));
    // unrecognized credentials file
    CreateFile(CREDENTIALS_KFG, " ");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    cerr << "= Expect error message: unrecognized format of credentials file\n";
    CheckKeys();
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_ErrorNoUnrecognized, AwsFixture) {
    // 4.3) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    // unrecognized credentials file is ignored when user don't accest charges
    CreateFile(CREDENTIALS_KFG, " ");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    cerr << "== No error message: unrecognized format of credentials file ==\n";
    CheckKeysNoPay();
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_ErrorIncompleteIni, AwsFixture) {
    // 4.3) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
                                  sizeof credentialsKfg, CREDENTIALS_KFG));
    // incomplete credentials ini file
    CreateFile(CREDENTIALS_KFG, "[");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    cerr << "Expect error message: profile is not found in credentials file\n";
    CheckKeys();
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_ErrorIncompleteCsvHeader, AwsFixture) {
    // 4.3) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
                                  sizeof credentialsKfg, CREDENTIALS_KFG));
    // incomplete header in credentials cvs file
    CreateFile(CREDENTIALS_KFG,
        "One,Secret access key,Two,three\r\n"
        "111,SECRET_ACCESS_KEY,222,33333");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    cerr << "== Expect error message: credentials file has incomplete header\n";

    CheckKeys();
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_Credentials_ErrorIncompleteCsvData, AwsFixture) {
    // 4.3) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
                                  sizeof credentialsKfg, CREDENTIALS_KFG));
    // incomplete data in credentials cvs file
    CreateFile(CREDENTIALS_KFG,
        "One,Secret access key,Two,Access key ID,three\r\n"
        "111,SECRET_ACCESS_KEY,222");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    cerr << "==== Expect error message: credentials file 'XXX' is incomplete\n";
    CheckKeys();
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_CredentialsCsv_NoWarnExtraData, AwsFixture) {
    // 4.3) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
                                  sizeof credentialsKfg, CREDENTIALS_KFG));
    // no comma in extra line is ignored
    CreateFile(CREDENTIALS_KFG, "Access key ID,Secret access key\r\n"
                                "1,2\r\n"
                                "3");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    CheckKeys("1", "2");
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(AWS_CredentialsCsv_WarnExtraData, AwsFixture) {
    // 4.3) create configuration
        char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
                                  sizeof credentialsKfg, CREDENTIALS_KFG));
    // extra data in credentials cvs file
    CreateFile(CREDENTIALS_KFG, "Access key ID,Secret access key\r\n"
                                "1,2\r\n"
                                ",");
    CreateFile(CONFIG,
        "/aws/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    cerr << "Expect warning message: credentials file 'XXX' has extra lines\n";
    CheckKeys("1", "2");
}
#endif

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
        putenv((char*)"GOOGLE_APPLICATION_CREDENTIALS=");
        putenv((char*)"NCBI_SETTINGS=/");
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
        KConfig * kfg = NULL;
        THROW_ON_RC(KConfigMakeLocal(&kfg, NULL));

        CloudMgrRelease(m_mgr);
        m_mgr = nullptr;
        THROW_ON_RC ( CloudMgrMakeWithProvider( & m_mgr, cloud_provider_gcp,
            kfg ) );
        THROW_ON_RC ( CloudMgrGetCurrentCloud ( m_mgr, & m_cloud ) );
        THROW_ON_FALSE ( nullptr != m_cloud );
        THROW_ON_RC ( CloudToGCP ( m_cloud, & m_gcp ) );
        THROW_ON_FALSE ( nullptr != m_gcp );

        THROW_ON_RC(KConfigRelease(kfg));
    }

    GCP * m_gcp;
};

#ifdef ALL
FIXTURE_TEST_CASE(GCP_Make, GcpFixture)
{
    rc_t rc = CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_gcp );
    if ( rc == 0 )
        REQUIRE_NOT_NULL ( m_cloud );

    else
    {
        REQUIRE_NULL ( m_cloud );
        REQUIRE_EQ ( ( uint32_t ) GetRCState ( rc ), ( uint32_t ) rcUnsupported );
    }
}

FIXTURE_TEST_CASE(GCP_CloudToGcp_NullSelf, GcpFixture)
{
    REQUIRE_RC ( CloudToGCP ( NULL, & m_gcp ) );
    REQUIRE_NULL ( m_gcp );
}
FIXTURE_TEST_CASE(GCP_CloudToGcp_NullParam, GcpFixture)
{
    REQUIRE_RC_FAIL ( CloudToGCP ( m_cloud, NULL ) );
}
FIXTURE_TEST_CASE(GCP_CloudToGcp, GcpFixture)
{
    rc_t rc = CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_gcp );
    if ( rc == 0 )
    {
        REQUIRE_NOT_NULL ( m_cloud );
        REQUIRE_RC ( CloudToGCP ( m_cloud, & m_gcp ) );
        REQUIRE_NOT_NULL ( m_gcp );
    }

    else
    {
        REQUIRE_NULL ( m_cloud );
        REQUIRE_EQ ( ( uint32_t ) GetRCState ( rc ), ( uint32_t ) rcUnsupported );
    }
}
FIXTURE_TEST_CASE(GCP_CloudToGcp_Fail, GcpFixture)
{
    REQUIRE_RC ( CloudMgrMakeCloud ( m_mgr, & m_cloud, cloud_provider_aws ) ); // wrong cloud
    REQUIRE_RC_FAIL ( CloudToGCP ( m_cloud, & m_gcp ) );
}

FIXTURE_TEST_CASE(GCP_ToCloud_NullSelf, GcpFixture)
{
    Cloud * cloud;
    REQUIRE_RC ( GCPToCloud ( NULL, & cloud ) );
    REQUIRE_NULL ( cloud );
}
FIXTURE_TEST_CASE(GCP_ToCloud_NullParam, GcpFixture)
{
    MakeGCP();
    REQUIRE_RC_FAIL ( GCPToCloud ( m_gcp, NULL ) );
}
FIXTURE_TEST_CASE(GCP_ToCloud, GcpFixture)
{
    MakeGCP();
    REQUIRE_NOT_NULL ( m_gcp );

    Cloud * cloud;
    REQUIRE_RC ( GCPToCloud ( m_gcp, & cloud ) );
    REQUIRE_NOT_NULL ( cloud );
    REQUIRE_RC ( CloudRelease ( cloud ) );
}

FIXTURE_TEST_CASE(GCP_Credentials_Blank, GcpFixture)
{
    // block cloud discovery to make sure the credentials are left blank
    putenv ( (char*)"GOOGLE_APPLICATION_CREDENTIALS" );
    MakeGCP();

    REQUIRE_NULL ( m_gcp -> privateKey );
    REQUIRE_NULL ( m_gcp -> client_email );
}

// GOOGLE_APPLICATION_CREDENTIALS overrides configuration
FIXTURE_TEST_CASE(GCP_Credentials, GcpFixture)
{
    // A) env GOOGLE_APPLICATION_CREDENTIALS
    putenv ( (char*)"GOOGLE_APPLICATION_CREDENTIALS=cloud-kfg/gcp_service.json" );

    // B) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    CreateFile(CREDENTIALS_KFG,
        "{\n"
        "  \"type\": \"service_account\",\n"
        "  \"project_id\": \"prid\",\n"
        "  \"private_key_id\": \"234567\",\n"
        "  \"private_key\":\n"
        "\"-----BEGIN PRIVATE KEY-----\n"
        "01234567820123456783012345678401234567850123456786012345678701234567\n"
        "012345678A0123456783B123456784C12345678D012345678E012345678F01234567\n"
        "-----END PRIVATE KEY-----\",\n"
        "  \"client_email\": \"ncbi@gserviceaccount.com\",\n"
        "  \"client_id\": \"2345678901\",\n"
        "  \"auth_uri\": \"https://accounts.google.com/o/oauth2/auth2\",\n"
        "  \"token_uri\": \"https://accounts.google.com/o/oauth2/token2\",\n"
        "  \"auth_provider_x509_cert_url\": \"https://google.com/auth/cert\",\n"
        "  \"client_x509_cert_url\": \"https://google.com/dev.servicaccount\"\n"
        "}\n");
    CreateFile(CONFIG,
        "/gcp/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    MakeGCP();

    REQUIRE_NOT_NULL ( m_gcp -> privateKey );
    REQUIRE_NOT_NULL ( m_gcp -> client_email );

    const string private_key_1 = "-----BEGIN PRIVATE KEY-----"; // skip EOL which can be \n or \r\n depending on Git settings
    const string private_key_2 = "MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBANoWq8DqARNncY/f";
    REQUIRE_EQ ( private_key_1, string ( m_gcp -> privateKey ) . substr ( 0, private_key_1 . size () ) );
    REQUIRE_NE ( string::npos, string ( m_gcp->privateKey ) . find( private_key_2 ) );

    const string client_email = "ncbivdb-compute@developer.gserviceaccount.com";
    REQUIRE_EQ ( client_email, string ( m_gcp -> client_email ) );
}

FIXTURE_TEST_CASE(GCP_Credentials_KfgCredentials, GcpFixture) {
    // B) create configuration
    char credentialsKfg[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(m_dir, true, credentialsKfg,
        sizeof credentialsKfg, CREDENTIALS_KFG));
    CreateFile(CREDENTIALS_KFG,
        "{\n"
        "  \"type\": \"service_account\",\n"
        "  \"project_id\": \"prid\",\n"
        "  \"private_key_id\": \"234567\",\n"
        "  \"private_key\":\n"
        "\"-----BEGIN PRIVATE KEY-----\n"
        "01234567820123456783012345678401234567850123456786012345678701234567\n"
        "012345678A0123456783B123456784C12345678D012345678E012345678F01234567\n"
        "-----END PRIVATE KEY-----\",\n"
        "  \"client_email\": \"ncbi@gserviceaccount.com\",\n"
        "  \"client_id\": \"2345678901\",\n"
        "  \"auth_uri\": \"https://accounts.google.com/o/oauth2/auth2\",\n"
        "  \"token_uri\": \"https://accounts.google.com/o/oauth2/token2\",\n"
        "  \"auth_provider_x509_cert_url\": \"https://google.com/auth/cert\",\n"
        "  \"client_x509_cert_url\": \"https://google.com/dev.servicaccount\"\n"
        "}\n");
    CreateFile(CONFIG,
        "/gcp/credential_file = \"", credentialsKfg, "\"\n");
    putenv((char*)"VDB_CONFIG=cloud-kfg");

    MakeGCP();

    REQUIRE_NOT_NULL(m_gcp->privateKey);
    REQUIRE_NOT_NULL(m_gcp->client_email);

    const string private_key_1("-----BEGIN PRIVATE KEY-----");
    REQUIRE_EQ(private_key_1,
        string(m_gcp->privateKey).substr(0, private_key_1.size()));

    const string private_key_2("A0123456783B123456784C12345678D012345678E0123");
    REQUIRE_NE(string::npos, string(m_gcp->privateKey).find(private_key_2));

    const string client_email("ncbi@gserviceaccount.com");
    REQUIRE_EQ(client_email, string(m_gcp->client_email));
}
#endif

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
#include <klib/log.h> /* KLogLevelSet */

extern "C" {
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
        setenv("HOME", ".", 1);

        KConfigDisableUserSettings();

     // assert(!KDbgSetString("CLOUD"));
     // KLogLevelSet( klogInfo );

     // this makes messages from the test code appear
     // (same as running the executable with "-l=message")
     // TestEnv::verbosity = LogLevel::e_message;

        rc_t rc=CloudTestSuite(argc, argv);
        return rc;
    }
}
