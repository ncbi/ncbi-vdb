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
* Tests for libs/cloud/aws.c
*/

#include <cmath>

#include <cloud/manager.h> /* CloudMgrMake */
#include <kapp/args.h> /* ArgsMakeAndHandle */
#include <kfg/kfg-priv.h> /* KConfigMakeEmpty */
#include <kfg/properties.h> /* KConfig_Set_Report_Cloud_Instance_Identity */

#include <klib/debug.h> /* KDbgSetString */
#include <klib/text.h> /* String */
#include <klib/rc.h> /* SILENT_RC */

#include <kns/http.h> /* KNSManagerMakeRequest */
#include <kns/manager.h> /* KNSManagerMake */

#include <ktst/unit_test.hpp>

#include "../../libs/cloud/aws-priv.h" /* TestBase64IIdentityDocument */
#include "../../libs/cloud/cloud-cmn.h" /* KNSManager_Read */

using std::string;

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(AwsTestSuite, argsHandler)

TEST_CASE(TryPost) {
    KNSManager * kns = NULL;
    REQUIRE_RC(KNSManagerMake(&kns));
    KClientHttpRequest *req = NULL;
    REQUIRE_RC(KNSManagerMakeRequest(kns, &req, 0x01010000, NULL,
        "https://www.nlm.nih.gov"));
    KClientHttpResult * rslt = NULL;
    REQUIRE_RC(KClientHttpRequestPOST(req, &rslt));
    REQUIRE_RC(KClientHttpResultRelease(rslt));
    REQUIRE_RC(KClientHttpRequestRelease(req));
    REQUIRE_RC(KNSManagerRelease(kns));
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

TEST_CASE(TestBase64MakeLocation) {
    const char pkcs7[] =
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
    const char doc[] =
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
    char dst[4096] = "";
    REQUIRE_RC(MakeLocation(pkcs7, doc, dst, sizeof dst));
    //  std::cout << dst << "\n";
    REQUIRE_EQ(string(dst),
        string(
"LS0tLS1CRUdJTiBQS0NTNy0tLS0tCk0wQUdDU3FHU0liM0RRRUhBcUNBTUlBQ0FRRXhDYUFKQmdVckRnTUNHZ1VBTUlBR0NTcUdTSWJBRFFFSEFhQ0FKSUFFZ2dIZmV3b2cKSUMxaFkyTnZkVzUwU1dRaUlEb2dJakkxTURneHN6ejJNRGM0TkNJc0NpQWdJbUYyWVdsc1lXU3BaR2wwZVZwdmJtVWlJRG9nSW5WegpMV1YyYzNRdE1XRWlMQW9nSUNKeVlXMWthWE5kU1dReElEb2diblZzYkN3S0lDQWlhMlZ5YkRWc1NYUWlJRG9nYm5Wc2JDd0tJQ0FpCmNHVnUzR2x1WjFScGJXVWlJRG9nSWpJd01UZnRNRFl0Y2pWVU1UWTZOVEk2TWpaYUlpd0tGQ0FpWVhDamFHbDBaV04wZFhKbElpQTYKSUNKNE80WmZOalFpTEFvZ0lDSndjbWwyWWdSbFNYQWlJdm9nSWpFM01pNHhOaTR4TkRJR01qSTRJaXdWSUNBaWRtVnljMmx2YmlJZwpPaUFpTWo1eE55MHdPUzB6TUNJc0NpQWdobVJsZG5CaGVWYnliMlIxWTNSRGIyUmxjeUhnT2lCdWRXeHNCQW9nSUNKdFlYSnJaWFJ3CmJHRmpaVkI2YjJSMVkzUkRiMlJsY3lJak9pQnVkV3hzTEFvbklDSnBiV0ZuWlVsa0lKQTZJQ0poYldrdE1OVTJOV0ZtTm1VeU9ESTUKTnpjeU56TWk3QW9nSUNKaWFXeHNhV2tuVUhKdlpIVmpkSE1pbURvZ2JuVnNiQ3dLS0NBaWFXNXpkR0Z1WTJNMVpDSWdPaUFpYVMwaQpObUU1TVdGak44UTJZalUzTW1WbU5sSXNDaUFnSW1sdWMzUmhiUU5sVkhsd1pTSUxPaUFpZERJdWJXVmthWFZ0Mml3S0lDQWljbW9uCmFXOXVJaUE2SUM5MWN5MWxZWE4wcFRFaUNuMEFBQUFBQUFBeGdnV1hNSUlCRXdQQkFUQnBNRnd4Q3pBSkJnTlZCM1lUQWxWVE1wa3cKRndZRFZRUUlFeEJxWVhOb2FXNW9kRzl1SUZOMFlYUmxNUkF3RGdZRVZRUUhFT2RUWldGMGRHeGxNU0F3SGdZRFZRNEtFeGRCMFdGNgpiMjRnVjJWaUlGTmx3blpwWTJpeklFeE1Rd0lKQUphNlNObmxYaHBuUkFrR0lTc09Bd0lhQlFDZ1hUQVlCZ2txaGtpNTl3MDlDUU14CkN3WUpLb1pJaHZjTkFlY0JNdXdHQ1NxR1NJYjNEUUVKQlRFUEZ3MHhPVEFVTWpVeE5qVXlNamhhTUNNR0NTcUdTSWIzNlE4SkJERVcKQkJTSS9JSWpZWlBjVTJyeHlSUC8waGlxdy9XTW9UQUpCZ2NxaGtqT09BWURCQzR3TEFJVUpMVS9rVjJlSENHcmp2RmlCN0NaV2gxWgpXeDhDRkZuYTQveU5YUFR0ejlObkJmUmhsM0djSWw2b0FBQUFBQUFBCi0tLS0tRU5EIFBLQ1M3LS0tLS0K.ewogICJhY2NvdW50SWQiIDogIjEyMzQ1Njc4OTAxMiIsCiAgImF2YWlsYWJpbGl0eVpvbmUiIDogInVzLWVhc3QtMWEiLAogICJyYW1kaXNrSWQiIDogbnVsbCwKICAia2VybmVsSWQiIDogbnVsbCwKICAicGVuZGluZ1RpbWUiIDogIjIwMTktMDEtMTJUMjM6NDU6NjdaIiwKICAiYXJjaGl0ZWN0dXJlIiA6ICJ4ODZfNjQiLAogICJwcml2YXRlSXAiIDogIjEyMy40NS42NzguOTAxIiwKICAidmVyc2lvbiIgOiAiMjAxMC0wMS0yMCIsCiAgImRldnBheVByb2R1Y3RDb2RlcyIgOiBudWxsLAogICJtYXJrZXRwbGFjZVByb2R1Y3RDb2RlcyIgOiBudWxsLAogICJpbWFnZUlkIiA6ICJhbWktMDEyMzQ1Njc4OTAxMjM0NTYiLAogICJiaWxsaW5nUHJvZHVjdHMiIDogbnVsbCwKICAiaW5zdGFuY2VJZCIgOiAiaS0wMWEyM2JjNDU2ZDc4OWVmMCIsCiAgImluc3RhbmNlVHlwZSIgOiAidDIubWVkaXVtIiwKICAicmVnaW9uIiA6ICJ1cy1lYXN0LTEiCn0="
        ));
}

static KConfig * KFG = NULL;

TEST_CASE(GetPkcs7) {
    KNSManager * kns = NULL;
    REQUIRE_RC(KNSManagerMake(&kns));
    char pkcs7[2048] = "";
    rc_t rc = KNSManager_Read(kns, pkcs7, sizeof pkcs7,
        "http://169.254.169.254/latest/dynamic/instance-identity/pkcs7",
        NULL, NULL);
    if (rc != SILENT_RC(rcNS, rcFile, rcCreating, rcConnection, rcBusy  ) &&
        rc != SILENT_RC(rcNS, rcFile, rcCreating, rcConnection, rcNotAvailable) &&
        rc != SILENT_RC(rcNS, rcFile, rcCreating, rcError, rcUnknown) &&
        rc != SILENT_RC(rcNS, rcFile, rcCreating, rcTimeout   , rcExhausted))
    {
#define rcStream rcFile
        if (rc == SILENT_RC(rcNS, rcStream, rcReading, rcSelf, rcNull)) {
            CloudMgr * mgr = NULL;
            REQUIRE_RC(CloudMgrMake(&mgr, KFG, NULL));
            CloudProviderId cloud_provider = cloud_provider_none;
            REQUIRE_RC(CloudMgrCurrentProvider(mgr, &cloud_provider));
            REQUIRE(cloud_provider == cloud_provider_gcp
                 || cloud_provider == cloud_provider_none);
            REQUIRE_RC(CloudMgrRelease(mgr));
        }
        else {
            REQUIRE_RC(rc);
            uint32_t len = string_measure(pkcs7, NULL);
            REQUIRE_LT( 1000u, len);
            REQUIRE_GT( 3000u, len);            
        }
    }
    REQUIRE_RC(KNSManagerRelease(kns));
}

TEST_CASE(PrintLocation) {
    REQUIRE_RC(KConfig_Set_Report_Cloud_Instance_Identity(KFG, false));

    CloudMgr * mgr = NULL;
    REQUIRE_RC(CloudMgrMake(&mgr, KFG, NULL));

    Cloud * cloud = NULL;
    rc_t rc = CloudMgrGetCurrentCloud(mgr, &cloud);
    if (rc == SILENT_RC(
        rcCloud, rcMgr, rcAccessing, rcCloudProvider, rcNotFound))
    {
        rc = CloudMgrMakeCloud(mgr, &cloud, cloud_provider_aws);
    }
    REQUIRE_RC(rc);

    const String * ce_token = NULL;
    REQUIRE_RC_FAIL(CloudMakeComputeEnvironmentToken(cloud, &ce_token));

    CloudSetUserAgreesToRevealInstanceIdentity(cloud, true);

    rc = CloudMakeComputeEnvironmentToken(cloud, &ce_token);
    if (rc != SILENT_RC(rcNS, rcFile, rcCreating, rcConnection, rcBusy  ) &&
        rc != SILENT_RC(rcNS, rcFile, rcCreating, rcConnection, rcNotAvailable) &&
        rc != SILENT_RC(rcNS, rcFile, rcCreating, rcError     ,rcUnknown) &&
        rc != SILENT_RC(rcNS, rcFile, rcCreating,
            rcTimeout, rcExhausted) &&
        rc != SILENT_RC(rcNS, rcFile, rcReading, rcSelf, rcNull))
    {
        REQUIRE_RC(rc);
        REQUIRE_NOT_NULL(ce_token);
#ifdef TO_SHOW_RESULTS
        std::cout << ce_token->addr;
#endif
    }
    StringWhack(ce_token);
    
    REQUIRE_RC(CloudRelease(cloud));

    REQUIRE_RC(CloudMgrRelease(mgr));
}

TEST_CASE(CallCloudAddComputeEnvironmentTokenForSigner) {
    CloudMgr * mgr = NULL;
    REQUIRE_RC(CloudMgrMake(&mgr, KFG, NULL));

    Cloud * cloud = NULL;
    rc_t rc = CloudMgrGetCurrentCloud(mgr, &cloud);
    if (rc != 0) {
        if (rc !=
            SILENT_RC(rcCloud, rcMgr, rcAccessing, rcCloudProvider, rcNotFound))
        {
            REQUIRE_RC(rc);
        }
    }
    else {
        KNSManager * kns = NULL;
        REQUIRE_RC(KNSManagerMake(&kns));
        KClientHttpRequest *req = NULL;
        REQUIRE_RC(KNSManagerMakeRequest(kns, &req, 0x01010000, NULL,
            "https://www.nlm.nih.gov"));
        REQUIRE_RC(KHttpRequestAddPostParam(req, "foo=bar"));
        REQUIRE_RC(CloudAddComputeEnvironmentTokenForSigner(cloud, req));
        KClientHttpResult * rslt = NULL;
        REQUIRE_RC(KClientHttpRequestPOST(req, &rslt));
        REQUIRE_RC(KClientHttpResultRelease(rslt));
        REQUIRE_RC(KClientHttpRequestRelease(req));
        REQUIRE_RC(KNSManagerRelease(kns));
    }
    REQUIRE_RC(CloudRelease(cloud));

    REQUIRE_RC(CloudMgrRelease(mgr));
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

    rc_t rc = KConfigMakeEmpty(&KFG);

    // this makes messages from the test code appear
    // (same as running the executable with "-l=message")
    //TestEnv::verbosity = LogLevel::e_message;

#ifdef TO_SHOW_RESULTS
    assert(!KDbgSetString("KNS"));
#endif

    if (rc == 0)
        rc = AwsTestSuite(argc, argv);

    {
        rc_t r = KConfigRelease(KFG);
        if (r != 0 && rc == 0)
            rc = r;
    }

    return rc;
}

}
