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
#include <cloud/aws.h>

#include <kapp/args.h> /* ArgsMakeAndHandle */
#include <kfg/kfg-priv.h> /* KConfigMakeEmpty */
#include <kfg/properties.h> /* KConfig_Set_Report_Cloud_Instance_Identity */

#include <klib/data-buffer.h> /* KDataBufferWhack */
#include <klib/debug.h> /* KDbgSetString */
#include <klib/printf.h> /* string_printf */
#include <klib/text.h> /* String */
#include <klib/rc.h> /* SILENT_RC */

#include <kns/http.h> /* KNSManagerMakeRequest */
#include <kns/manager.h> /* KNSManagerMake */

#include <ktst/unit_test.hpp>

#include "../../libs/cloud/aws-priv.h" /* TestBase64IIdentityDocument */
#include "../../libs/cloud/cloud-priv.h" /* AWS */
#include "../../libs/cloud/cloud-cmn.h" /* KNSManager_Read */

#include "mbedtls/md.h" /* mbedtls_md_free */

using std::string;
using namespace::std;

#define TO_SHOW_RESULTS 0
#define ALL

TEST_SUITE(AwsTestSuite)

static KConfig* KFG = nullptr;

//
// Unit tests for functions in cloud/aws-auth.c
//

#ifdef ALL
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

//
// Unit tests for functions in cloud/aws.c
//

TEST_CASE(Get_IMDS_version)
{
    CloudMgr * mgr = nullptr;
    REQUIRE_RC(CloudMgrMake(&mgr, KFG, nullptr));
    CloudProviderId cloud_provider = cloud_provider_none;
    REQUIRE_RC(CloudMgrCurrentProvider(mgr, &cloud_provider));
    if (cloud_provider == cloud_provider_aws )
    {
        Cloud * cloud = nullptr;
        REQUIRE_RC( CloudMgrGetCurrentCloud ( mgr, & cloud ) );
        AWS * aws = nullptr;
        REQUIRE_RC( CloudToAWS ( cloud, & aws ) );
#if TO_SHOW_RESULTS
        std::cout << "***IMDSv=" << (unsigned int)aws -> IMDS_version << endl;
#endif
        REQUIRE( aws -> IMDS_version == 1 || aws -> IMDS_version == 2);
    }
    REQUIRE_RC(CloudMgrRelease(mgr));
}

TEST_CASE(Get_Pkcs7)
{
    CloudMgr * mgr = nullptr;
    REQUIRE_RC(CloudMgrMake(&mgr, KFG, nullptr));
    CloudProviderId cloud_provider = cloud_provider_none;
    REQUIRE_RC(CloudMgrCurrentProvider(mgr, &cloud_provider));
    if (cloud_provider == cloud_provider_aws )
    {
        Cloud * cloud = nullptr;
        REQUIRE_RC( CloudMgrGetCurrentCloud ( mgr, & cloud ) );
        AWS * aws = nullptr;
        REQUIRE_RC( CloudToAWS ( cloud, & aws ) );

        char pkcs7[4096];
        REQUIRE_RC( GetPkcs7( aws, pkcs7, sizeof( pkcs7 ) ) );

        uint32_t len = string_measure(pkcs7, nullptr);
        REQUIRE_LT( 1000u, len);
        REQUIRE_GT( 3000u, len);
#if TO_SHOW_RESULTS
        std::cout << "***Pkcs7=" << string( pkcs7, len ) << endl;
#endif

        REQUIRE_RC( AWSRelease( aws ) );
    }

    REQUIRE_RC(CloudMgrRelease(mgr));
}

TEST_CASE(PrintInstance_NotAllowed) {
    REQUIRE_RC(KConfig_Set_Report_Cloud_Instance_Identity(KFG, false));

    CloudMgr * mgr = nullptr;
    REQUIRE_RC(CloudMgrMake(&mgr, KFG, nullptr));
    CloudProviderId cloud_provider = cloud_provider_none;
    REQUIRE_RC(CloudMgrCurrentProvider(mgr, &cloud_provider));
    if (cloud_provider == cloud_provider_aws )
    {
        Cloud * cloud = nullptr;
        REQUIRE_RC( CloudMgrGetCurrentCloud ( mgr, & cloud ) );
        AWS * aws = nullptr;
        REQUIRE_RC( CloudToAWS ( cloud, & aws ) );

        const String * ce_token = nullptr;
        REQUIRE_RC_FAIL(CloudMakeComputeEnvironmentToken(cloud, &ce_token));

        REQUIRE_RC(CloudRelease(cloud));
    }

    REQUIRE_RC(CloudMgrRelease(mgr));
}

TEST_CASE(PrintInstance_Allowed) {
    REQUIRE_RC(KConfig_Set_Report_Cloud_Instance_Identity(KFG, false));

    CloudMgr * mgr = nullptr;
    REQUIRE_RC(CloudMgrMake(&mgr, KFG, nullptr));
    CloudProviderId cloud_provider = cloud_provider_none;
    REQUIRE_RC(CloudMgrCurrentProvider(mgr, &cloud_provider));
    if (cloud_provider == cloud_provider_aws )
    {
        Cloud * cloud = nullptr;
        REQUIRE_RC( CloudMgrGetCurrentCloud ( mgr, & cloud ) );
        AWS * aws = nullptr;
        REQUIRE_RC( CloudToAWS ( cloud, & aws ) );

        CloudSetUserAgreesToRevealInstanceIdentity(cloud, true);

        const String * ce_token = nullptr;
        REQUIRE_RC( CloudMakeComputeEnvironmentToken(cloud, &ce_token) );
        REQUIRE_NOT_NULL(ce_token);
    #if TO_SHOW_RESULTS
            std::cout << "***ce_token=" << ce_token->addr << endl;
    #endif
        StringWhack(ce_token);

        REQUIRE_RC(CloudRelease(cloud));
    }

    REQUIRE_RC(CloudMgrRelease(mgr));
}

TEST_CASE(CallCloudAddComputeEnvironmentTokenForSigner) {
    CloudMgr * mgr = nullptr;
    REQUIRE_RC(CloudMgrMake(&mgr, KFG, nullptr));

    Cloud * cloud = nullptr;
    rc_t rc = CloudMgrGetCurrentCloud(mgr, &cloud);
    if (rc != 0) {
        if (rc !=
            SILENT_RC(rcCloud, rcMgr, rcAccessing, rcCloudProvider, rcNotFound))
        {
            REQUIRE_RC(rc);
        }
    }
    else {
        KNSManager * kns = nullptr;
        REQUIRE_RC(KNSManagerMake(&kns));
        KClientHttpRequest *req = nullptr;
        REQUIRE_RC(KNSManagerMakeRequest(kns, &req, 0x01010000, nullptr,
            "https://www.nih.gov"));
        REQUIRE_RC(KHttpRequestAddPostParam(req, "foo=bar"));
        CloudSetUserAgreesToRevealInstanceIdentity(cloud, true);
        REQUIRE_RC(CloudAddComputeEnvironmentTokenForSigner(cloud, req));
        KClientHttpResult * rslt = nullptr;
        REQUIRE_RC(KClientHttpRequestPOST(req, &rslt));
        REQUIRE_RC(KClientHttpResultRelease(rslt));
        REQUIRE_RC(KClientHttpRequestRelease(req));
        REQUIRE_RC(KNSManagerRelease(kns));
    }
    REQUIRE_RC(CloudRelease(cloud));

    REQUIRE_RC(CloudMgrRelease(mgr));
}

TEST_CASE(GetLocation) {
    CloudMgr * mgr = nullptr;
    REQUIRE_RC( CloudMgrMake(&mgr, KFG, nullptr) );

    Cloud * cloud = nullptr;
    if ( CloudMgrMakeCloud(mgr, &cloud, cloud_provider_aws) == 0 )
    {
        String const * location = nullptr;
        rc_t rc = CloudGetLocation ( cloud, & location );
        if ( rc != 0 )
        {
            if ( rc
                != SILENT_RC(rcNS, rcFile, rcCreating, rcTimeout, rcExhausted) )
            {
                REQUIRE_RC( rc );
            }
        }
        else
        {
            REQUIRE_NOT_NULL( location );
#if TO_SHOW_RESULTS
            cout << "***location=" << string( location -> addr, location -> size ) << endl;
#endif
            StringWhack( location );
        }
        REQUIRE_RC( CloudRelease(cloud) );
    }

    REQUIRE_RC( CloudMgrRelease(mgr) );
}
#endif

#ifdef ALL
TEST_CASE(TestSHA256OfTheEmptyString) {
    const unsigned char input[]("");
    char hex[65]("");
    REQUIRE_RC(CalculateSHA256Hash(input, sizeof input - 1, hex));
    REQUIRE_EQ(string(hex), string(
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
}

TEST_CASE(TestSHA256) {
    const unsigned char input[](
        "GET\n"
        "/test.txt\n"
        "\n"
        "host:examplebucket.s3.amazonaws.com\n"
        "range:bytes=0-9\n"
        "x-amz-content-sha256:"
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n"
        "x-amz-date:20130524T000000Z\n"
        "\n"
        "host;range;x-amz-content-sha256;x-amz-date\n"
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    char hex[65]("");
    REQUIRE_RC(CalculateSHA256Hash(input, sizeof input - 1, hex));
    REQUIRE_EQ(string(hex), string(
        "7344ae5b7ee6c3e7e6b0fe0640412a37625d1fbfff95c48bbb2dc43964946972"));
}
#endif

#ifdef ALL
TEST_CASE(TestBuildStringToSign) {
    const char requestDateTime[]("20130524T000000Z");
    const char region[]("us-east-1");
    const char hashedCanonicalRequest[](
        "7344ae5b7ee6c3e7e6b0fe0640412a37625d1fbfff95c48bbb2dc43964946972");

    KDataBuffer stringToSign;
    REQUIRE_RC(KDataBufferMake(&stringToSign, 8, 0));

    REQUIRE_RC(BuildStringToSign(
        requestDateTime, region, sizeof region - 1, "s3", 2,
        hashedCanonicalRequest, &stringToSign));

    REQUIRE_EQ(string((char*)stringToSign.base, stringToSign.elem_count - 1),
        string(
            "AWS4-HMAC-SHA256\n"
            "20130524T000000Z\n"
            "20130524/us-east-1/s3/aws4_request\n"
            "7344ae5b7ee6c3e7e6b0fe0640412a37625d1fbfff95c48bbb2dc43964946972")
    );

    REQUIRE_RC(KDataBufferWhack(&stringToSign));
}
#endif

#ifdef ALL
TEST_CASE(TestHMAC_SHA256) {
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    const mbedtls_md_info_t* md_info
        = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    REQUIRE(md_info);

    const unsigned char key[]("secretKey");
    const char payload[]("Hello HMAC SHA 256!");
    unsigned char h_output[32](""); // SHA-256 outputs 32 bytes
    REQUIRE_RC(HMAC_SHA256(md_info,
        key, sizeof key - 1, payload, sizeof payload - 1, h_output));

    char hex[65]("");
    for (int i = 0; i < 32; ++i)
        REQUIRE_RC(string_printf(hex + i * 2, 3, nullptr, "%02x", h_output[i]));
    REQUIRE_EQ(string(hex), string(
        "40f08b93b298f788109624ad3505882e0467fab1b30a76993a8327097c4f4e45"));

    mbedtls_md_free(&ctx);
}
#endif

#ifdef ALL
TEST_CASE(TestDeriveSigningKey) {
    const char secretKey[]("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY");
    const char stringToSign[](
        "AWS4-HMAC-SHA256\n"
        "20130524T000000Z\n"
        "20130524/us-east-1/s3/aws4_request\n"
        "7344ae5b7ee6c3e7e6b0fe0640412a37625d1fbfff95c48bbb2dc43964946972");
    const char date[]("20130524");
    const char region[]("us-east-1");
    const char service[]("s3");
    char hex[65]("");
    REQUIRE_RC(CalculateSignature(secretKey, date,
        region, sizeof region - 1, service, sizeof service - 1,
        stringToSign, sizeof stringToSign - 1, hex));
    REQUIRE_EQ(string(hex), string(
        "f0e8bdb87c964420e857bd35b5d6ed310bd44f0170aba48dd91039c6036bdb41"));
}

#endif

#ifdef ALL
TEST_CASE(TestCreateAuthorizationHeader) {
    KDataBuffer header;
    REQUIRE_RC(KDataBufferMake(&header, 8, 0)); //Authorization:

    REQUIRE_RC(CreateAuthorizationHeader(
        "AKIAIOSFODNN7EXAMPLE", "20220830", "us-east-1", "host;x-amz-date",
        "f0e8bdb87c964420e857bd35b5d6ed310bd44f0170aba48dd91039c6036bdb41",
        &header));

    string a((char*)header.base, 0, header.elem_count - 1);
    string e("AWS4-HMAC-SHA256 "
        "Credential="
                    "AKIAIOSFODNN7EXAMPLE/20220830/us-east-1/s3/aws4_request, "
        "SignedHeaders=host;x-amz-date, "
        "Signature="
            "f0e8bdb87c964420e857bd35b5d6ed310bd44f0170aba48dd91039c6036bdb41");
    REQUIRE_EQ(a, e);

    REQUIRE_RC(KDataBufferWhack(&header));
}
#endif

//////////////////////////////////////////// Main
int main ( int argc, char *argv [] )
{
    rc_t rc = KConfigMakeEmpty(&KFG);

    if (rc == 0)
        rc = (rc_t)AwsTestSuite(argc, argv);

    {
        rc_t r = KConfigRelease(KFG);
        if (r != 0 && rc == 0)
            rc = r;
    }

    return (int)rc;
}
