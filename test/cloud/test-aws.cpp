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
* Tests for libs/cloud/aws-auth.c
*/

#include <kapp/args.h> /* ArgsMakeAndHandle */
#include <kfg/config.h> /* KConfigDisableUserSettings */
#include <ktst/unit_test.hpp>

#include "../../libs/cloud/aws-priv.h" /* TestBase64IIdentityDocument */

using std::string;

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(AwsTestSuite, argsHandler)

TEST_CASE(TestRealBase64InIdentityDocument) {
    const char src[] =
"{\n"
"  \"accountId\" : \"250813660784\",\n"
"  \"availabilityZone\" : \"us-east-1a\",\n"
"  \"ramdiskId\" : null,\n"
"  \"kernelId\" : null,\n"
"  \"pendingTime\" : \"2019-06-25T16:52:26Z\",\n"
"  \"architecture\" : \"x86_64\",\n"
"  \"privateIp\" : \"172.16.142.228\",\n"
"  \"version\" : \"2017-09-30\",\n"
"  \"devpayProductCodes\" : null,\n"
"  \"marketplaceProductCodes\" : null,\n"
"  \"imageId\" : \"ami-0565af6e282977273\",\n"
"  \"billingProducts\" : null,\n"
"  \"instanceId\" : \"i-06a91ac746b572ef4\",\n"
"  \"instanceType\" : \"t2.medium\",\n"
"  \"region\" : \"us-east-1\"\n"
"}";
    char dst[999] = "";
    REQUIRE_RC(Base64InIdentityDocument(src, dst, sizeof dst));
//  std::cout << dst << "\n";
    REQUIRE_EQ(string(dst),
        string("ewogICJhY2NvdW50SWQiIDogIjI1MDgxMzY2MDc4NCIsCiAgImF2YWlsYWJpbGl0eVpvbmUiIDogInVzLWVhc3QtMWEiLAogICJyYW1kaXNrSWQiIDogbnVsbCwKICAia2VybmVsSWQiIDogbnVsbCwKICAicGVuZGluZ1RpbWUiIDogIjIwMTktMDYtMjVUMTY6NTI6MjZaIiwKICAiYXJjaGl0ZWN0dXJlIiA6ICJ4ODZfNjQiLAogICJwcml2YXRlSXAiIDogIjE3Mi4xNi4xNDIuMjI4IiwKICAidmVyc2lvbiIgOiAiMjAxNy0wOS0zMCIsCiAgImRldnBheVByb2R1Y3RDb2RlcyIgOiBudWxsLAogICJtYXJrZXRwbGFjZVByb2R1Y3RDb2RlcyIgOiBudWxsLAogICJpbWFnZUlkIiA6ICJhbWktMDU2NWFmNmUyODI5NzcyNzMiLAogICJiaWxsaW5nUHJvZHVjdHMiIDogbnVsbCwKICAiaW5zdGFuY2VJZCIgOiAiaS0wNmE5MWFjNzQ2YjU3MmVmNCIsCiAgImluc3RhbmNlVHlwZSIgOiAidDIubWVkaXVtIiwKICAicmVnaW9uIiA6ICJ1cy1lYXN0LTEiCn0=")
    );
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
const char UsageDefaultName[] = "test-aws";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();

    // this makes messages from the test code appear
    // (same as running the executable with "-l=message")
    //TestEnv::verbosity = LogLevel::e_message;

    return AwsTestSuite(argc, argv);
}

}
