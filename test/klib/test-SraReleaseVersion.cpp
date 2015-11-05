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
* Unit tests for SraReleaseVersion
*/

#include <ktst/unit_test.hpp>
#include <ktst/unit_test_suite.hpp> // REQUIRE_RC
#include <kapp/args.h>
#include <klib/sra-release-version.h> /* SraReleaseVersion */
#include <klib/text.h> /* string_size */
#include <string.h> /* memset */
using std::string;
TEST_SUITE(SraReleaseVersionTestSuite);

class Test : private ncbi::NK::TestCase {
public:
    Test(TestCase &dad, const string &s, const char *expected = NULL)
        : TestCase(s)
    {
        if (expected == NULL) {
            expected = s.c_str();
        }
        SraReleaseVersion v;
        REQUIRE_RC(SraReleaseVersionInit(&v, s.c_str(), s.size()));
        char buffer[256] = "";
        size_t num_writ = 0;
        REQUIRE_RC(
            SraReleaseVersionPrint(&v, buffer, sizeof buffer, &num_writ));
        CHECK_EQ(num_writ, string_size(expected));
        CHECK_EQ(memcmp(expected, buffer, num_writ), 0);

        SraReleaseVersion v2;
        REQUIRE_RC(SraReleaseVersionInit(&v2, buffer, string_size(buffer)));

        int32_t result = 0;
        REQUIRE_RC(SraReleaseVersionCmp(&v, &v2, &result));
        CHECK_EQ(result, 0);

        dad.ErrorCounterAdd(GetErrorCounter());
    }
};

TEST_CASE(SraReleaseVersionTest) {
    int32_t result = 0;
    SraReleaseVersion v;
    SraReleaseVersion etalon;
    memset(&etalon, 0, sizeof etalon);
    etalon.type = SraReleaseVersion::eSraReleaseVersionTypeFinal;

    REQUIRE_RC(SraReleaseVersionInit(&v, NULL, 0));
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    REQUIRE_RC(SraReleaseVersionInit(&v, "", 0));
    Test      (*this,                    "", "0.0.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    REQUIRE_RC(SraReleaseVersionInit(&v, "0", 1));
    Test      (*this,                    "0", "0.0.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x1000000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "1", 1));
    Test      (*this,                    "1", "1.0.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0xC000000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "12", 2));
    Test      (*this,                    "12", "12.0.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x1000000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "1.", 2));
    Test      (*this,                    "1.", "1.0.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0;
    REQUIRE_RC(SraReleaseVersionInit(&v, "0.", 2));
    Test      (*this,                    "0.", "0.0.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0xD000000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "13.", 3));
    Test      (*this,                    "13", "13.0.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x2010000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "2.1", 3));
    Test      (*this,                    "2.1", "2.1.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x2000000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "2.0", 3));
    Test      (*this,                    "2.0", "2.0.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0xE010000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "14.1", 4));
    Test      (*this,                    "14.1", "14.1.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0xE000000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "14.0", 4));
    Test      (*this,                    "14.0", "14.0.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0xF0A0000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "15.10", 5));
    Test      (*this,                    "15.10", "15.10.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x2000000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "2.0.", 4));
    Test      (*this,                    "2.0", "2.0.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x2010000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "2.1.", 4));
    Test      (*this,                    "2.1.", "2.1.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0xE010000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "14.1.", 5));
    Test      (*this,                    "14.1.", "14.1.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0xF0A0000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "15.10.", 6));
    Test      (*this,                    "15.10", "15.10.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x3020001;
    REQUIRE_RC(SraReleaseVersionInit(&v, "3.2.1", 5));
    Test      (*this,                    "3.2.1", "3.2.1");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x3020000;
    REQUIRE_RC(SraReleaseVersionInit(&v, "3.2.0", 5));
    Test      (*this,                    "3.2.0", "3.2.0");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0xD030002;
    REQUIRE_RC(SraReleaseVersionInit(&v, "13.3.2", 6));
    Test      (*this,                    "13.3.2", "13.3.2");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0xC100001;
    REQUIRE_RC(SraReleaseVersionInit(&v, "12.16.1", 7));
    Test      (*this,                    "12.16.1", "12.16.1");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x10110012;
    REQUIRE_RC(SraReleaseVersionInit(&v, "16.17.18", 8));
    Test      (*this,                    "16.17.18", "16.17.18");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x3020001;
    REQUIRE_RC(SraReleaseVersionInit(&v, "3.2.1-", 6));
    Test      (*this,                    "3.2.1-", "3.2.1");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x10110012;
    REQUIRE_RC(SraReleaseVersionInit(&v, "16.17.18-", 9));
    Test      (*this,                    "16.17.18-", "16.17.18");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x4030002;
    etalon.revision = 5;
    REQUIRE_RC(SraReleaseVersionInit(&v, "4.3.2-5", 7));
    Test      (*this,                    "4.3.2-5");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x4030002;
    etalon.revision = 0;
    REQUIRE_RC(SraReleaseVersionInit(&v, "4.3.2-0", 7));
    Test      (*this,                    "4.3.2-0", "4.3.2");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x13110012;
    etalon.revision = 20;
    REQUIRE_RC(SraReleaseVersionInit(&v, "19.17.18-20", 11));
    Test      (*this,                    "19.17.18-20");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x4030002;
    etalon.revision = 0;
    etalon.type = SraReleaseVersion::eSraReleaseVersionTypeAlpha;
    REQUIRE_RC(SraReleaseVersionInit(&v, "4.3.2-a", 7));
    Test      (*this,                    "4.3.2-a");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x13110012;
    REQUIRE_RC(SraReleaseVersionInit(&v, "19.17.18-a", 10));
    Test      (*this,                    "19.17.18-a");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x4030002;
    REQUIRE_RC(SraReleaseVersionInit(&v, "4.3.2-a0", 8));
    Test      (*this,                    "4.3.2-a0", "4.3.2-a");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x13110012;
    etalon.revision = 21;
    REQUIRE_RC(SraReleaseVersionInit(&v, "19.17.18-a21", 12));
    Test      (*this,                    "19.17.18-a21");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x4030002;
    etalon.revision = 1;
    etalon.type = SraReleaseVersion::eSraReleaseVersionTypeBeta;
    REQUIRE_RC(SraReleaseVersionInit(&v, "4.3.2-b1", 8));
    Test      (*this,                    "4.3.2-b1");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x4030002;
    etalon.revision = 0;
    etalon.type = SraReleaseVersion::eSraReleaseVersionTypeRC;
    REQUIRE_RC(SraReleaseVersionInit(&v, "4.3.2-rc", 8));
    Test      (*this,                    "4.3.2-rc");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x13110012;
    REQUIRE_RC(SraReleaseVersionInit(&v, "19.17.18-rc", 11));
    Test      (*this,                    "19.17.18-rc");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x4030002;
    REQUIRE_RC(SraReleaseVersionInit(&v, "4.3.2-rc0", 9));
    Test      (*this,                    "4.3.2-rc0", "4.3.2-rc");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x4030002;
    etalon.revision = 1;
    REQUIRE_RC(SraReleaseVersionInit(&v, "4.3.2-rc1", 9));
    Test      (*this,                    "4.3.2-rc1");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);

    etalon.version = 0x13110012;
    etalon.revision = 21;
    REQUIRE_RC(SraReleaseVersionInit(&v, "19.17.18-rc21", 13));
    Test      (*this,                    "19.17.18-rc21");
    REQUIRE_RC(SraReleaseVersionCmp(&v, &etalon, &result));
    CHECK_EQ(result, 0);
}

//////////////////////////////////////////////////// Main
extern "C" {
    ver_t CC KAppVersion(void) { return 0; }
    rc_t CC Usage(const Args *args) { return 0; }
    const char UsageDefaultName[] = "test-SraReleaseVersion";
    rc_t CC UsageSummary(const char *progname) { return 0; }

    rc_t CC KMain(int argc, char *argv[]) {
        return SraReleaseVersionTestSuite(argc, argv);
    }
}
