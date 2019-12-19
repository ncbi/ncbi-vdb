/* =============================================================================
 * Copy this file to create an new unit test application
 * to add debug arguments to to command line use: "-app_args='-+VFS -s'"
 * ========================================================================== */

#include <kapp/args.h> /* Args */
#include <kfg/kfg-priv.h> /* KConfigMakeEmpty */
#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */
#include <ktst/unit_test.hpp> /* TEST_SUITE_WITH_ARGS_HANDLER */
#include <vfs/services.h> /* KServiceRelease */
#include "../../libs/vfs/jwt.h" /* JwtKartValidate */

static rc_t argsHandler(int argc, char* argv[]) {
    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

TEST_SUITE_WITH_ARGS_HANDLER(TestPermSuite, argsHandler)

TEST_CASE(TestService) {
    /* service is NULL */
    REQUIRE_RC_FAIL(JwtKartValidateFile(NULL, NULL));
    REQUIRE_RC_FAIL(KServiceSetNgcFile(NULL, NULL));
    REQUIRE_RC_FAIL(KServiceSetJwtKartFile(NULL, NULL));

    KService * service = NULL;
    REQUIRE_RC(KServiceMake(&service));

    /* path is NULL */
    REQUIRE_RC_FAIL(KServiceSetNgcFile(service, NULL));
    REQUIRE_RC_FAIL(KServiceSetJwtKartFile(service, NULL));

    const char notExist[] = "/this/file/does/not/exist";
    REQUIRE_RC_FAIL(KServiceSetNgcFile(service, notExist));
    REQUIRE_RC_FAIL(KServiceSetJwtKartFile(service, notExist));

    KDirectory * dir = NULL;
    REQUIRE_RC(KDirectoryNativeDir(&dir));

    const char path[] = "tmp.perm-file.tmp";
    REQUIRE_RC(KDirectoryRemove(dir, false, path));

    KFile * f = NULL;
    REQUIRE_RC(KDirectoryCreateFile(dir, &f, false,
        0664, kcmOpen | kcmInit | kcmCreate, path));

    /* empty cart cannot be valid */
    REQUIRE_RC_FAIL(KServiceSetJwtKartFile(service, path));

    REQUIRE_RC_FAIL(KServiceSetNgcFile(service, path));

    REQUIRE_RC(KFileWriteExactly(f, 0, "1", 1));
    REQUIRE_RC_FAIL(KServiceSetNgcFile(service, path));

    REQUIRE_RC(KFileWriteExactly(f, 0, "1", 1));
    /* too small cart cannot be valid */
    REQUIRE_RC_FAIL(KServiceSetJwtKartFile(service, path));

    REQUIRE_RC_FAIL(KServiceSetNgcFile(service, path));

    REQUIRE_RC(KFileWriteExactly(f, 1, ".2.3", 4));

    /* minimum size (now 5) of valid cart file has to be defined */
    REQUIRE_RC(KServiceSetJwtKartFile(service, path));

    REQUIRE_RC_FAIL(KServiceSetNgcFile(service, path));

    REQUIRE_RC(KDirectoryRemove(dir, false, path));
    REQUIRE_RC(KFileRelease(f));
    REQUIRE_RC(KDirectoryRelease(dir));

    /* a valid ngc file */
    REQUIRE_RC(KServiceSetNgcFile(service, "prj_phs710EA_test.ngc"));

    REQUIRE_RC(KServiceRelease(service));
}

TEST_CASE(TestJwtKartValidateString) {
    String s;
    memset(&s, 0, sizeof s);
    REQUIRE_RC_FAIL(JwtKartValidateString(&s, NULL));

    size_t os = ~0;

    /* too big cart is invalid */
    size_t size = 20 * 1024 * 1024;
    char * buffer = static_cast<char*>(malloc(size));
    REQUIRE_NOT_NULL(buffer);
    REQUIRE_NOT_NULL(memset(buffer, 'a', size));
    buffer[5] = buffer[9] = '.';
    StringInit(&s, buffer, size, size);
    REQUIRE_RC_FAIL(JwtKartValidateString(&s, &os));
    REQUIRE_EQ(os, (size_t)0);
    free(buffer);

    /* good enough */
    size = 2 * 1024 * 1024;
    buffer = static_cast<char*>(malloc(size));
    REQUIRE_NOT_NULL(buffer);
    REQUIRE_NOT_NULL(memset(buffer, 'A', size));
    buffer[5] = buffer[9] = '.';
    StringInit(&s, buffer, size, size);
    REQUIRE_RC(JwtKartValidateString(&s, NULL));
    free(buffer);

    /* a real jwt cart */
    const char * b = "eyJhbGciOiJSUzI1NiIsImtpZCI6ImtleS1pZC0xMjM0IiwidHlwIjoiS"
        "ldUIn0.eyJleHAiOjE1Njk5NjAzODIsImlhdCI6MTU2OTM1NTU4MiwiaXNzIjoiZGJnYXBfcnVuX3N"
        "lbGVjdG9yIiwibGlzdCI6W3siY2F0YWxvZyI6IlNSQSIsImlkcyI6WyJTUlIwMDAwMDIiXX1dLCJ2Z"
        "XJzIjoxfQ.G7dfags0L0P89mVFg2WTlHyxvv9C4F1wLc6tiyzCzkSLwjIoijXqnqFhUBX0SnKCYyk-"
        "kKSrkJSs8u0aquxn8N8W8eYJ8vZ0Q8qeNSuQjfOPII6qV3re2dDdkjecgWvyulERN8uRMUiwUYipvN"
        "HxoFKQLzYhJajGt6p9rf8lNOPo1_AAuyLQeqCoEUY8bRTDnlVu90lU965eg2QZcvduQrI49p2aZIsC"
        "SV5e3TJj7SmXMlvNOJMlWD76rYoitWU82uA_S6mXXkTTxNd2KfmeEsj7g1djc1M7yUkQRET46eIp1q"
        "bk6gOAiiF5AL8eC8-SOLGrxqmdF8AU52_L31_pwA";
    size = strlen(b);
    StringInit(&s, b, size, size);
    REQUIRE_RC(JwtKartValidateString(&s, NULL));
}

TEST_CASE(TestJwtKartValidateStringWhite) {
    char b1[] = "abcdefghijklmnopqrstuvwxyz."
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ."
        "0123456789-_\n"
        "\r\n"
        "\n\r"
        "\r";
    size_t size = sizeof b1 - 1;
    String s;
    StringInit(&s, b1, size, size);
    REQUIRE_RC(JwtKartValidateString(&s, NULL));

    /* check valid characters */
    for (int i = 0; i < 256; ++i) {
        b1[9] = i;
        if ((i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z') ||
            (i >= '0' && i <= '9') || i == '-' || i == '_')
        {
            REQUIRE_RC(JwtKartValidateString(&s, NULL));
        }
        else
            REQUIRE_RC_FAIL(JwtKartValidateString(&s, NULL));
    }

    /* check second section */
    const char * b = "abcdefghijklmnopqrstuvwxyz."
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ.0123456789-_\n\r\n\n\r\r";
    StringInit(&s, b, size, size);
    REQUIRE_RC(JwtKartValidateString(&s, NULL));

    b = "abcdefghijklmnopqrstuvwxyz."
        "ABCDEFGHIJKL NOPQRSTUVWXYZ.0123456789-_\n\r\n\n\r\r";
    StringInit(&s, b, size, size);
    REQUIRE_RC_FAIL(JwtKartValidateString(&s, NULL));

    /* check third section */
    const char p[] = "abcdefghijklmnopqrstuvwxyz."
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ.0123456789-_";
    size_t es = sizeof p - 1;

    b = "abcdefghijklmnopqrstuvwxyz."
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ.0123456789-_\n\r\n\n\r\r";
    StringInit(&s, b, size, size);
    size_t os = ~0;
    REQUIRE_RC(JwtKartValidateString(&s, &os));
    REQUIRE_EQ(os, es);

    b = "abcdefghijklmnopqrstuvwxyz."
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ.01234 6789-_\n\r\n\n\r\r";
    StringInit(&s, b, size, size);
    REQUIRE_RC_FAIL(JwtKartValidateString(&s, NULL));
}

TEST_CASE(TestJwtKartValidateStringWhiteSections) {
    size_t os = 0;

    const char * b = "1234567810123.567820123456.8301234567840";
    size_t size = strlen(b);
    String s;
    StringInit(&s, b, size, size);
    REQUIRE_RC(JwtKartValidateString(&s, &os));
    REQUIRE_EQ(os, size);

    /* 1 dot */
    b = "12345678101234567820123456.8301234567840";
    StringInit(&s, b, size, size);
    REQUIRE_RC_FAIL(JwtKartValidateString(&s, NULL));

    /* 3 dots */
    b = "1234567810123.567820123456.830123.567840";
    StringInit(&s, b, size, size);
    REQUIRE_RC_FAIL(JwtKartValidateString(&s, NULL));

    /* first section empty */
    b = ".2345678101234567820123456.8301234567840";
    StringInit(&s, b, size, size);
    REQUIRE_RC_FAIL(JwtKartValidateString(&s, NULL));

    /* first section size is min */
    b = "1.345678101234567820123456.8301234567840";
    StringInit(&s, b, size, size);
    REQUIRE_RC(JwtKartValidateString(&s, NULL));

    /* second section empty */
    b = "1234567810123456782012345..8301234567840";
    StringInit(&s, b, size, size);
    REQUIRE_RC_FAIL(JwtKartValidateString(&s, NULL));

    /* second section size is min */
    b = "123456781012345678201234.6.8301234567840";
    StringInit(&s, b, size, size);
    REQUIRE_RC(JwtKartValidateString(&s, NULL));

    /* third section empty */
    b = "1234567810123.5678201234567830123456784.";
    StringInit(&s, b, size, size);
    REQUIRE_RC_FAIL(JwtKartValidateString(&s, NULL));

    /* third section size is min */
    b = "1234567810123.567820123456783012345678.0";
    StringInit(&s, b, size, size);
    REQUIRE_RC(JwtKartValidateString(&s, NULL));
}

const char UsageDefaultName[] = "test";
rc_t CC UsageSummary(const char * progname) { return 0; }
rc_t CC Usage(const Args * args) { return 0; }

extern "C" {

    ver_t CC KAppVersion(void) { return 0; }

    int KMain(int argc, char *argv[]) {
        KConfigDisableUserSettings(); // ignore ~/.ncbi/user-settings.mkfg

        rc_t rc = TestPermSuite(argc, argv);
        return rc;
    }

}

/******************************************************************************/
