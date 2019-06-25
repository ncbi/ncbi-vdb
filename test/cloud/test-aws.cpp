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

TEST_CASE(TestBase64InIdentityDocument) {
    const char src[] =
        "{\n"
        "  \"accountId\" : \"123456789012\",\n"
        "  \"availabilityZone\" : \"us-east-1a\",\n"
        "  \"ramdiskId\" : null,\n"
        "  \"kernelId\" : null,\n"
        "  \"pendingTime\" : \"2019-01-12T23:45:67Z\",\n"
        "  \"architecture\" : \"x86_64\",\n"
        "  \"privateIp\" : \"123.45.678.901\",\n"
        "  \"version\" : \"2010-01-20\",\n"
        "  \"devpayProductCodes\" : null,\n"
        "  \"marketplaceProductCodes\" : null,\n"
        "  \"imageId\" : \"ami-01234567890123456\",\n"
        "  \"billingProducts\" : null,\n"
        "  \"instanceId\" : \"i-01a23bc456d789ef0\",\n"
        "  \"instanceType\" : \"t2.medium\",\n"
        "  \"region\" : \"us-east-1\"\n"
        "}";
    char dst[999] = "";
    REQUIRE_RC(Base64InIdentityDocument(src, dst, sizeof dst));
    //  std::cout << dst << "\n";
    REQUIRE_EQ(string(dst),
        string("ewogICJhY2NvdW50SWQiIDogIjEyMzQ1Njc4OTAxMiIsCiAgImF2YWlsYWJpbGl0eVpvbmUiIDogInVzLWVhc3QtMWEiLAogICJyYW1kaXNrSWQiIDogbnVsbCwKICAia2VybmVsSWQiIDogbnVsbCwKICAicGVuZGluZ1RpbWUiIDogIjIwMTktMDEtMTJUMjM6NDU6NjdaIiwKICAiYXJjaGl0ZWN0dXJlIiA6ICJ4ODZfNjQiLAogICJwcml2YXRlSXAiIDogIjEyMy40NS42NzguOTAxIiwKICAidmVyc2lvbiIgOiAiMjAxMC0wMS0yMCIsCiAgImRldnBheVByb2R1Y3RDb2RlcyIgOiBudWxsLAogICJtYXJrZXRwbGFjZVByb2R1Y3RDb2RlcyIgOiBudWxsLAogICJpbWFnZUlkIiA6ICJhbWktMDEyMzQ1Njc4OTAxMjM0NTYiLAogICJiaWxsaW5nUHJvZHVjdHMiIDogbnVsbCwKICAiaW5zdGFuY2VJZCIgOiAiaS0wMWEyM2JjNDU2ZDc4OWVmMCIsCiAgImluc3RhbmNlVHlwZSIgOiAidDIubWVkaXVtIiwKICAicmVnaW9uIiA6ICJ1cy1lYXN0LTEiCn0=")
    );
}

TEST_CASE(TestRealWrapInIdentityPkcs7) {
    const char src[] =
"MIAGCSqGSIb3DQEHAqCAMIACAQExCzAJBgUrDgMCGgUAMIAGCSqGSIb3DQEHAaCAJIAEggHfewog\n"
"ICJhY2NvdW50SWQiIDogIjI1MDgxMzY2MDc4NCIsCiAgImF2YWlsYWJpbGl0eVpvbmUiIDogInVz\n"
"LWVhc3QtMWEiLAogICJyYW1kaXNrSWQiIDogbnVsbCwKICAia2VybmVsSWQiIDogbnVsbCwKICAi\n"
"cGVuZGluZ1RpbWUiIDogIjIwMTktMDYtMjVUMTY6NTI6MjZaIiwKICAiYXJjaGl0ZWN0dXJlIiA6\n"
"ICJ4ODZfNjQiLAogICJwcml2YXRlSXAiIDogIjE3Mi4xNi4xNDIuMjI4IiwKICAidmVyc2lvbiIg\n"
"OiAiMjAxNy0wOS0zMCIsCiAgImRldnBheVByb2R1Y3RDb2RlcyIgOiBudWxsLAogICJtYXJrZXRw\n"
"bGFjZVByb2R1Y3RDb2RlcyIgOiBudWxsLAogICJpbWFnZUlkIiA6ICJhbWktMDU2NWFmNmUyODI5\n"
"NzcyNzMiLAogICJiaWxsaW5nUHJvZHVjdHMiIDogbnVsbCwKICAiaW5zdGFuY2VJZCIgOiAiaS0w\n"
"NmE5MWFjNzQ2YjU3MmVmNCIsCiAgImluc3RhbmNlVHlwZSIgOiAidDIubWVkaXVtIiwKICAicmVn\n"
"aW9uIiA6ICJ1cy1lYXN0LTEiCn0AAAAAAAAxggEXMIIBEwIBATBpMFwxCzAJBgNVBAYTAlVTMRkw\n"
"FwYDVQQIExBXYXNoaW5ndG9uIFN0YXRlMRAwDgYDVQQHEwdTZWF0dGxlMSAwHgYDVQQKExdBbWF6\n"
"b24gV2ViIFNlcnZpY2VzIExMQwIJAJa6SNnlXhpnMAkGBSsOAwIaBQCgXTAYBgkqhkiG9w0BCQMx\n"
"CwYJKoZIhvcNAQcBMBwGCSqGSIb3DQEJBTEPFw0xOTA2MjUxNjUyMjhaMCMGCSqGSIb3DQEJBDEW\n"
"BBSI/IIjYZPcU26xmRP/0hiqw/WMoTAJBgcqhkjOOAQDBC4wLAIUJLU/kV2eHCGrjvFiBiCZWh1Z\n"
"Wx8CFFna4/yNXPTdz9NnBfRhl3GcIl6oAAAAAAAA";
    char dst[2048] = "";
    REQUIRE_RC(WrapInIdentityPkcs7(src, dst, sizeof dst));
//    std::cout << dst << "\n";
    REQUIRE_EQ(string(dst),
        string(
"-----BEGIN PKCS7-----\n"
"MIAGCSqGSIb3DQEHAqCAMIACAQExCzAJBgUrDgMCGgUAMIAGCSqGSIb3DQEHAaCAJIAEggHfewog\n"
"ICJhY2NvdW50SWQiIDogIjI1MDgxMzY2MDc4NCIsCiAgImF2YWlsYWJpbGl0eVpvbmUiIDogInVz\n"
"LWVhc3QtMWEiLAogICJyYW1kaXNrSWQiIDogbnVsbCwKICAia2VybmVsSWQiIDogbnVsbCwKICAi\n"
"cGVuZGluZ1RpbWUiIDogIjIwMTktMDYtMjVUMTY6NTI6MjZaIiwKICAiYXJjaGl0ZWN0dXJlIiA6\n"
"ICJ4ODZfNjQiLAogICJwcml2YXRlSXAiIDogIjE3Mi4xNi4xNDIuMjI4IiwKICAidmVyc2lvbiIg\n"
"OiAiMjAxNy0wOS0zMCIsCiAgImRldnBheVByb2R1Y3RDb2RlcyIgOiBudWxsLAogICJtYXJrZXRw\n"
"bGFjZVByb2R1Y3RDb2RlcyIgOiBudWxsLAogICJpbWFnZUlkIiA6ICJhbWktMDU2NWFmNmUyODI5\n"
"NzcyNzMiLAogICJiaWxsaW5nUHJvZHVjdHMiIDogbnVsbCwKICAiaW5zdGFuY2VJZCIgOiAiaS0w\n"
"NmE5MWFjNzQ2YjU3MmVmNCIsCiAgImluc3RhbmNlVHlwZSIgOiAidDIubWVkaXVtIiwKICAicmVn\n"
"aW9uIiA6ICJ1cy1lYXN0LTEiCn0AAAAAAAAxggEXMIIBEwIBATBpMFwxCzAJBgNVBAYTAlVTMRkw\n"
"FwYDVQQIExBXYXNoaW5ndG9uIFN0YXRlMRAwDgYDVQQHEwdTZWF0dGxlMSAwHgYDVQQKExdBbWF6\n"
"b24gV2ViIFNlcnZpY2VzIExMQwIJAJa6SNnlXhpnMAkGBSsOAwIaBQCgXTAYBgkqhkiG9w0BCQMx\n"
"CwYJKoZIhvcNAQcBMBwGCSqGSIb3DQEJBTEPFw0xOTA2MjUxNjUyMjhaMCMGCSqGSIb3DQEJBDEW\n"
"BBSI/IIjYZPcU26xmRP/0hiqw/WMoTAJBgcqhkjOOAQDBC4wLAIUJLU/kV2eHCGrjvFiBiCZWh1Z\n"
"Wx8CFFna4/yNXPTdz9NnBfRhl3GcIl6oAAAAAAAA\n"
"-----END PKCS7-----\n"
        ));
}

TEST_CASE(TestWrapInIdentityPkcs7) {
    const char src[] =
"M0AGCSqGSIb3DQEHAqCAMIACAQExCaAJBgUrDgMCGgUAMIAGCSqGSIbADQEHAaCAJIAEggHfewog\n"
"IC1hY2NvdW50SWQiIDogIjI1MDgxszz2MDc4NCIsCiAgImF2YWlsYWSpZGl0eVpvbmUiIDogInVz\n"
"LWV2c3QtMWEiLAogICJyYW1kaXNdSWQxIDogbnVsbCwKICAia2VybDVsSXQiIDogbnVsbCwKICAi\n"
"cGVu3GluZ1RpbWUiIDogIjIwMTftMDYtcjVUMTY6NTI6MjZaIiwKFCAiYXCjaGl0ZWN0dXJlIiA6\n"
"ICJ4O4ZfNjQiLAogICJwcml2YgRlSXAiIvogIjE3Mi4xNi4xNDIGMjI4IiwVICAidmVyc2lvbiIg\n"
"OiAiMj5xNy0wOS0zMCIsCiAghmRldnBheVbyb2R1Y3RDb2RlcyHgOiBudWxsBAogICJtYXJrZXRw\n"
"bGFjZVB6b2R1Y3RDb2RlcyIjOiBudWxsLAonICJpbWFnZUlkIJA6ICJhbWktMNU2NWFmNmUyODI5\n"
"NzcyNzMi7AogICJiaWxsaWknUHJvZHVjdHMimDogbnVsbCwKKCAiaW5zdGFuY2M1ZCIgOiAiaS0i\n"
"NmE5MWFjN8Q2YjU3MmVmNlIsCiAgImluc3RhbQNlVHlwZSILOiAidDIubWVkaXVt2iwKICAicmon\n"
"aW9uIiA6IC91cy1lYXN0pTEiCn0AAAAAAAAxggWXMIIBEwPBATBpMFwxCzAJBgNVB3YTAlVTMpkw\n"
"FwYDVQQIExBqYXNoaW5odG9uIFN0YXRlMRAwDgYEVQQHEOdTZWF0dGxlMSAwHgYDVQ4KExdB0WF6\n"
"b24gV2ViIFNlwnZpY2izIExMQwIJAJa6SNnlXhpnRAkGISsOAwIaBQCgXTAYBgkqhki59w09CQMx\n"
"CwYJKoZIhvcNAecBMuwGCSqGSIb3DQEJBTEPFw0xOTAUMjUxNjUyMjhaMCMGCSqGSIb36Q8JBDEW\n"
"BBSI/IIjYZPcU2rxyRP/0hiqw/WMoTAJBgcqhkjOOAYDBC4wLAIUJLU/kV2eHCGrjvFiB7CZWh1Z\n"
"Wx8CFFna4/yNXPTtz9NnBfRhl3GcIl6oAAAAAAAA";
    char dst[2048] = "";
    REQUIRE_RC(WrapInIdentityPkcs7(src, dst, sizeof dst));
//    std::cout << dst << "\n";
    REQUIRE_EQ(string(dst),
        string(
"-----BEGIN PKCS7-----\n"
"M0AGCSqGSIb3DQEHAqCAMIACAQExCaAJBgUrDgMCGgUAMIAGCSqGSIbADQEHAaCAJIAEggHfewog\n"
"IC1hY2NvdW50SWQiIDogIjI1MDgxszz2MDc4NCIsCiAgImF2YWlsYWSpZGl0eVpvbmUiIDogInVz\n"
"LWV2c3QtMWEiLAogICJyYW1kaXNdSWQxIDogbnVsbCwKICAia2VybDVsSXQiIDogbnVsbCwKICAi\n"
"cGVu3GluZ1RpbWUiIDogIjIwMTftMDYtcjVUMTY6NTI6MjZaIiwKFCAiYXCjaGl0ZWN0dXJlIiA6\n"
"ICJ4O4ZfNjQiLAogICJwcml2YgRlSXAiIvogIjE3Mi4xNi4xNDIGMjI4IiwVICAidmVyc2lvbiIg\n"
"OiAiMj5xNy0wOS0zMCIsCiAghmRldnBheVbyb2R1Y3RDb2RlcyHgOiBudWxsBAogICJtYXJrZXRw\n"
"bGFjZVB6b2R1Y3RDb2RlcyIjOiBudWxsLAonICJpbWFnZUlkIJA6ICJhbWktMNU2NWFmNmUyODI5\n"
"NzcyNzMi7AogICJiaWxsaWknUHJvZHVjdHMimDogbnVsbCwKKCAiaW5zdGFuY2M1ZCIgOiAiaS0i\n"
"NmE5MWFjN8Q2YjU3MmVmNlIsCiAgImluc3RhbQNlVHlwZSILOiAidDIubWVkaXVt2iwKICAicmon\n"
"aW9uIiA6IC91cy1lYXN0pTEiCn0AAAAAAAAxggWXMIIBEwPBATBpMFwxCzAJBgNVB3YTAlVTMpkw\n"
"FwYDVQQIExBqYXNoaW5odG9uIFN0YXRlMRAwDgYEVQQHEOdTZWF0dGxlMSAwHgYDVQ4KExdB0WF6\n"
"b24gV2ViIFNlwnZpY2izIExMQwIJAJa6SNnlXhpnRAkGISsOAwIaBQCgXTAYBgkqhki59w09CQMx\n"
"CwYJKoZIhvcNAecBMuwGCSqGSIb3DQEJBTEPFw0xOTAUMjUxNjUyMjhaMCMGCSqGSIb36Q8JBDEW\n"
"BBSI/IIjYZPcU2rxyRP/0hiqw/WMoTAJBgcqhkjOOAYDBC4wLAIUJLU/kV2eHCGrjvFiB7CZWh1Z\n"
"Wx8CFFna4/yNXPTtz9NnBfRhl3GcIl6oAAAAAAAA\n"
"-----END PKCS7-----\n"
        ));
}

TEST_CASE(TestBase64InIdentityPkcs7) {
    const char src[] =
"M0AGCSqGSIb3DQEHAqCAMIACAQExCaAJBgUrDgMCGgUAMIAGCSqGSIbADQEHAaCAJIAEggHfewog\n"
"IC1hY2NvdW50SWQiIDogIjI1MDgxszz2MDc4NCIsCiAgImF2YWlsYWSpZGl0eVpvbmUiIDogInVz\n"
"LWV2c3QtMWEiLAogICJyYW1kaXNdSWQxIDogbnVsbCwKICAia2VybDVsSXQiIDogbnVsbCwKICAi\n"
"cGVu3GluZ1RpbWUiIDogIjIwMTftMDYtcjVUMTY6NTI6MjZaIiwKFCAiYXCjaGl0ZWN0dXJlIiA6\n"
"ICJ4O4ZfNjQiLAogICJwcml2YgRlSXAiIvogIjE3Mi4xNi4xNDIGMjI4IiwVICAidmVyc2lvbiIg\n"
"OiAiMj5xNy0wOS0zMCIsCiAghmRldnBheVbyb2R1Y3RDb2RlcyHgOiBudWxsBAogICJtYXJrZXRw\n"
"bGFjZVB6b2R1Y3RDb2RlcyIjOiBudWxsLAonICJpbWFnZUlkIJA6ICJhbWktMNU2NWFmNmUyODI5\n"
"NzcyNzMi7AogICJiaWxsaWknUHJvZHVjdHMimDogbnVsbCwKKCAiaW5zdGFuY2M1ZCIgOiAiaS0i\n"
"NmE5MWFjN8Q2YjU3MmVmNlIsCiAgImluc3RhbQNlVHlwZSILOiAidDIubWVkaXVt2iwKICAicmon\n"
"aW9uIiA6IC91cy1lYXN0pTEiCn0AAAAAAAAxggWXMIIBEwPBATBpMFwxCzAJBgNVB3YTAlVTMpkw\n"
"FwYDVQQIExBqYXNoaW5odG9uIFN0YXRlMRAwDgYEVQQHEOdTZWF0dGxlMSAwHgYDVQ4KExdB0WF6\n"
"b24gV2ViIFNlwnZpY2izIExMQwIJAJa6SNnlXhpnRAkGISsOAwIaBQCgXTAYBgkqhki59w09CQMx\n"
"CwYJKoZIhvcNAecBMuwGCSqGSIb3DQEJBTEPFw0xOTAUMjUxNjUyMjhaMCMGCSqGSIb36Q8JBDEW\n"
"BBSI/IIjYZPcU2rxyRP/0hiqw/WMoTAJBgcqhkjOOAYDBC4wLAIUJLU/kV2eHCGrjvFiB7CZWh1Z\n"
"Wx8CFFna4/yNXPTtz9NnBfRhl3GcIl6oAAAAAAAA";
    char dst[2048] = "";
    REQUIRE_RC(Base64InIdentityPkcs7(src, dst, sizeof dst));
//    std::cout << dst << "\n";
    REQUIRE_EQ(string(dst),
        string(
"LS0tLS1CRUdJTiBQS0NTNy0tLS0tCk0wQUdDU3FHU0liM0RRRUhBcUNBTUlBQ0FRRXhDYUFKQmdVckRnTUNHZ1VBTUlBR0NTcUdTSWJBRFFFSEFhQ0FKSUFFZ2dIZmV3b2cKSUMxaFkyTnZkVzUwU1dRaUlEb2dJakkxTURneHN6ejJNRGM0TkNJc0NpQWdJbUYyWVdsc1lXU3BaR2wwZVZwdmJtVWlJRG9nSW5WegpMV1YyYzNRdE1XRWlMQW9nSUNKeVlXMWthWE5kU1dReElEb2diblZzYkN3S0lDQWlhMlZ5YkRWc1NYUWlJRG9nYm5Wc2JDd0tJQ0FpCmNHVnUzR2x1WjFScGJXVWlJRG9nSWpJd01UZnRNRFl0Y2pWVU1UWTZOVEk2TWpaYUlpd0tGQ0FpWVhDamFHbDBaV04wZFhKbElpQTYKSUNKNE80WmZOalFpTEFvZ0lDSndjbWwyWWdSbFNYQWlJdm9nSWpFM01pNHhOaTR4TkRJR01qSTRJaXdWSUNBaWRtVnljMmx2YmlJZwpPaUFpTWo1eE55MHdPUzB6TUNJc0NpQWdobVJsZG5CaGVWYnliMlIxWTNSRGIyUmxjeUhnT2lCdWRXeHNCQW9nSUNKdFlYSnJaWFJ3CmJHRmpaVkI2YjJSMVkzUkRiMlJsY3lJak9pQnVkV3hzTEFvbklDSnBiV0ZuWlVsa0lKQTZJQ0poYldrdE1OVTJOV0ZtTm1VeU9ESTUKTnpjeU56TWk3QW9nSUNKaWFXeHNhV2tuVUhKdlpIVmpkSE1pbURvZ2JuVnNiQ3dLS0NBaWFXNXpkR0Z1WTJNMVpDSWdPaUFpYVMwaQpObUU1TVdGak44UTJZalUzTW1WbU5sSXNDaUFnSW1sdWMzUmhiUU5sVkhsd1pTSUxPaUFpZERJdWJXVmthWFZ0Mml3S0lDQWljbW9uCmFXOXVJaUE2SUM5MWN5MWxZWE4wcFRFaUNuMEFBQUFBQUFBeGdnV1hNSUlCRXdQQkFUQnBNRnd4Q3pBSkJnTlZCM1lUQWxWVE1wa3cKRndZRFZRUUlFeEJxWVhOb2FXNW9kRzl1SUZOMFlYUmxNUkF3RGdZRVZRUUhFT2RUWldGMGRHeGxNU0F3SGdZRFZRNEtFeGRCMFdGNgpiMjRnVjJWaUlGTmx3blpwWTJpeklFeE1Rd0lKQUphNlNObmxYaHBuUkFrR0lTc09Bd0lhQlFDZ1hUQVlCZ2txaGtpNTl3MDlDUU14CkN3WUpLb1pJaHZjTkFlY0JNdXdHQ1NxR1NJYjNEUUVKQlRFUEZ3MHhPVEFVTWpVeE5qVXlNamhhTUNNR0NTcUdTSWIzNlE4SkJERVcKQkJTSS9JSWpZWlBjVTJyeHlSUC8waGlxdy9XTW9UQUpCZ2NxaGtqT09BWURCQzR3TEFJVUpMVS9rVjJlSENHcmp2RmlCN0NaV2gxWgpXeDhDRkZuYTQveU5YUFR0ejlObkJmUmhsM0djSWw2b0FBQUFBQUFBCi0tLS0tRU5EIFBLQ1M3LS0tLS0K"
        ));
}

/*
M0AGCSqGSIb3DQEHAqCAMIACAQExCaAJBgUrDgMCGgUAMIAGCSqGSIbADQEHAaCAJIAEggHfewog
IC1hY2NvdW50SWQiIDogIjI1MDgxszz2MDc4NCIsCiAgImF2YWlsYWSpZGl0eVpvbmUiIDogInVz
LWV2c3QtMWEiLAogICJyYW1kaXNdSWQxIDogbnVsbCwKICAia2VybDVsSXQiIDogbnVsbCwKICAi
cGVu3GluZ1RpbWUiIDogIjIwMTftMDYtcjVUMTY6NTI6MjZaIiwKFCAiYXCjaGl0ZWN0dXJlIiA6
ICJ4O4ZfNjQiLAogICJwcml2YgRlSXAiIvogIjE3Mi4xNi4xNDIGMjI4IiwVICAidmVyc2lvbiIg
OiAiMj5xNy0wOS0zMCIsCiAghmRldnBheVbyb2R1Y3RDb2RlcyHgOiBudWxsBAogICJtYXJrZXRw
bGFjZVB6b2R1Y3RDb2RlcyIjOiBudWxsLAonICJpbWFnZUlkIJA6ICJhbWktMNU2NWFmNmUyODI5
NzcyNzMi7AogICJiaWxsaWknUHJvZHVjdHMimDogbnVsbCwKKCAiaW5zdGFuY2M1ZCIgOiAiaS0i
NmE5MWFjN8Q2YjU3MmVmNlIsCiAgImluc3RhbQNlVHlwZSILOiAidDIubWVkaXVt2iwKICAicmon
aW9uIiA6IC91cy1lYXN0pTEiCn0AAAAAAAAxggWXMIIBEwPBATBpMFwxCzAJBgNVB3YTAlVTMpkw
FwYDVQQIExBqYXNoaW5odG9uIFN0YXRlMRAwDgYEVQQHEOdTZWF0dGxlMSAwHgYDVQ4KExdB0WF6
b24gV2ViIFNlwnZpY2izIExMQwIJAJa6SNnlXhpnRAkGISsOAwIaBQCgXTAYBgkqhki59w09CQMx
CwYJKoZIhvcNAecBMuwGCSqGSIb3DQEJBTEPFw0xOTAUMjUxNjUyMjhaMCMGCSqGSIb36Q8JBDEW
BBSI/IIjYZPcU2rxyRP/0hiqw/WMoTAJBgcqhkjOOAYDBC4wLAIUJLU/kV2eHCGrjvFiB7CZWh1Z
Wx8CFFna4/yNXPTtz9NnBfRhl3GcIl6oAAAAAAAA
*/

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
