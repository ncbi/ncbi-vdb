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
* Unit tests for Properties API
*/

#include <ktst/unit_test.hpp>

#include "kfg-fixture.hpp"

#include <kfg/properties.h>

using namespace std;
using namespace ncbi::NK;

TEST_SUITE(PropertiesTestSuite);

// KConfig_Get_Http_Proxy_Path, KConfig_Get_Http_Proxy_Path

FIXTURE_TEST_CASE( GetHttpProxyPath_NotFound, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    buf[0] = '1';
    REQUIRE_RC_FAIL ( KConfig_Get_Http_Proxy_Path( kfg, buf, BufSize, & num_writ ) );
    // in case of failure, clears the buffer
    REQUIRE_EQ ( string(), string ( buf, num_writ ) );
}
FIXTURE_TEST_CASE( GetHttpProxyPath, KfgFixture )
{
    CreateAndLoad(GetName(), "/http/proxy/path='value'");
    REQUIRE_RC ( KConfig_Get_Http_Proxy_Path( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "value"), string ( buf, num_writ ) );
}

FIXTURE_TEST_CASE( SetHttpProxyPath, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC ( KConfig_Set_Http_Proxy_Path( kfg, "path" ) );
    REQUIRE_RC ( KConfig_Get_Http_Proxy_Path( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "path"), string ( buf, num_writ ) );
}

// KConfig_Get_Prefetch_Download_To_Cache, KConfig_Set_Prefetch_Download_To_Cache

FIXTURE_TEST_CASE( Get_Prefetch_Download_To_Cache_Default, KfgFixture )
{   // default = true
    CreateAndLoad(GetName(), "#");
    bool b = false;
    REQUIRE_RC ( KConfig_Get_Prefetch_Download_To_Cache( kfg, & b ) );
    REQUIRE ( b );
}
FIXTURE_TEST_CASE( Get_Prefetch_Download_To_Cache, KfgFixture )
{
    CreateAndLoad(GetName(), "/tools/prefetch/download_to_cache='false'");
    bool b = true;
    REQUIRE_RC ( KConfig_Get_Prefetch_Download_To_Cache( kfg, & b ) );
    REQUIRE ( ! b );
}
FIXTURE_TEST_CASE( Set_Prefetch_Download_To_Cache, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC ( KConfig_Set_Prefetch_Download_To_Cache( kfg, false ) );
    bool b = true;
    REQUIRE_RC ( KConfig_Get_Prefetch_Download_To_Cache( kfg, & b ) );
    REQUIRE ( ! b );
}

// KConfig_Get_User_Accept_Aws_Charges, KConfig_Set_User_Accept_Aws_Charges

FIXTURE_TEST_CASE( Get_User_Accept_Aws_Charges_Default, KfgFixture )
{   // default = false
    CreateAndLoad(GetName(), "#");
    bool b = true;
    REQUIRE_RC ( KConfig_Get_User_Accept_Aws_Charges( kfg, &b ) );
    REQUIRE ( ! b );
}
FIXTURE_TEST_CASE( Get_User_Accept_Aws_Charges, KfgFixture )
{
    CreateAndLoad(GetName(), "/libs/cloud/accept_aws_charges='true'");
    bool b = false;
    REQUIRE_RC ( KConfig_Get_User_Accept_Aws_Charges( kfg, & b ) );
    REQUIRE ( b );
}
FIXTURE_TEST_CASE( Set_User_Accept_Aws_Charges, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC ( KConfig_Set_User_Accept_Aws_Charges( kfg, true ) );
    bool b = false;
    REQUIRE_RC ( KConfig_Get_User_Accept_Aws_Charges( kfg, & b ) );
    REQUIRE ( b );
}

// KConfig_Get_User_Accept_Gcp_Charges, KConfig_Set_User_Accept_Gcp_Charges

FIXTURE_TEST_CASE( Get_User_Accept_Gcp_Charges_Default, KfgFixture )
{   // default = false
    CreateAndLoad(GetName(), "#");
    bool b = true;
    REQUIRE_RC ( KConfig_Get_User_Accept_Gcp_Charges( kfg, &b ) );
    REQUIRE ( ! b );
}
FIXTURE_TEST_CASE( Get_User_Accept_Gcp_Charges, KfgFixture )
{
    CreateAndLoad(GetName(), "/libs/cloud/accept_gcp_charges='true'");
    bool b = false;
    REQUIRE_RC ( KConfig_Get_User_Accept_Gcp_Charges( kfg, & b ) );
    REQUIRE ( b );
}
FIXTURE_TEST_CASE( Set_User_Accept_Gcp_Charges, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC ( KConfig_Set_User_Accept_Gcp_Charges( kfg, true ) );
    bool b = false;
    REQUIRE_RC ( KConfig_Get_User_Accept_Gcp_Charges( kfg, & b ) );
    REQUIRE ( b );
}

// KConfig_Get_Temp_Cache , KConfig_Set_Temp_Cache

FIXTURE_TEST_CASE( Get_Temp_Cache_Default, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC_FAIL ( KConfig_Get_Temp_Cache ( kfg, buf, BufSize, & num_writ ) );
}
FIXTURE_TEST_CASE( Get_Temp_Cache, KfgFixture )
{
    CreateAndLoad(GetName(), "/libs/temp_cache='value'");
    REQUIRE_RC ( KConfig_Get_Temp_Cache ( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "value"), string ( buf, num_writ ) );
}

FIXTURE_TEST_CASE( Set_Temp_Cache, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC ( KConfig_Set_Temp_Cache( kfg, "path" ) );
    REQUIRE_RC ( KConfig_Get_Temp_Cache( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "path" ), string ( buf, num_writ ) );
}

// KConfig_Get_Gcp_Credential_File  , KConfig_Set_Gcp_Credential_File

FIXTURE_TEST_CASE( Get_Gcp_Credential_File_Default, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC_FAIL ( KConfig_Get_Gcp_Credential_File ( kfg, buf, BufSize, & num_writ ) );
}
FIXTURE_TEST_CASE( Get_Gcp_Credential_File, KfgFixture )
{
    CreateAndLoad(GetName(), "/gcp/credential_file='value'");
    REQUIRE_RC ( KConfig_Get_Gcp_Credential_File ( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "value"), string ( buf, num_writ ) );
}

FIXTURE_TEST_CASE( Set_Gcp_Credential_File, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC ( KConfig_Set_Gcp_Credential_File( kfg, "path" ) );
    REQUIRE_RC ( KConfig_Get_Gcp_Credential_File( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "path" ), string ( buf, num_writ ) );
}

// KConfig_Get_Aws_Credential_File  , KConfig_Set_Aws_Credential_File

FIXTURE_TEST_CASE( Get_Aws_Credential_File_Default, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC_FAIL ( KConfig_Get_Aws_Credential_File ( kfg, buf, BufSize, & num_writ ) );
}
FIXTURE_TEST_CASE( Get_Aws_Credential_File, KfgFixture )
{
    CreateAndLoad(GetName(), "/aws/credential_file='value'");
    REQUIRE_RC ( KConfig_Get_Aws_Credential_File ( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "value"), string ( buf, num_writ ) );
}

FIXTURE_TEST_CASE( Set_Aws_Credential_File, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC ( KConfig_Set_Aws_Credential_File( kfg, "path" ) );
    REQUIRE_RC ( KConfig_Get_Aws_Credential_File( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "path" ), string ( buf, num_writ ) );
}

// KConfig_Get_Aws_Profile, KConfig_Set_Aws_Profile

FIXTURE_TEST_CASE( Get_Aws_Profile_Default, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC ( KConfig_Get_Aws_Profile ( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "default"), string ( buf, num_writ ) );
}
FIXTURE_TEST_CASE( Get_Aws_Profile_Empty, KfgFixture )
{
    CreateAndLoad(GetName(), "/aws/profile=''");
    REQUIRE_RC ( KConfig_Get_Aws_Profile ( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "default"), string ( buf, num_writ ) );
}
FIXTURE_TEST_CASE( Get_Aws_Profile, KfgFixture )
{
    CreateAndLoad(GetName(), "/aws/profile='value'");
    REQUIRE_RC ( KConfig_Get_Aws_Profile ( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "value"), string ( buf, num_writ ) );
}

FIXTURE_TEST_CASE( Set_Aws_Profile, KfgFixture )
{
    CreateAndLoad(GetName(), "#");
    REQUIRE_RC ( KConfig_Set_Aws_Profile( kfg, "path" ) );
    REQUIRE_RC ( KConfig_Get_Aws_Profile( kfg, buf, BufSize, & num_writ ) );
    REQUIRE_EQ ( string ( "path" ), string ( buf, num_writ ) );
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

const char UsageDefaultName[] = "test-properties";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=PropertiesTestSuite(argc, argv);
    return rc;
}

}
