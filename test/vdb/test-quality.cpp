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
* =========================================================================== */

#include <kfg/kfg-priv.h> /* KConfigDisableUserSettings */
#include <ktst/unit_test.hpp> // TEST_SUITE
#include <vdb/vdb-priv.h> /* VDBManagerGetQualityString */
#include "../libs/vdb/dbmgr-priv.h" /* VDBManagerQualityReset */

TEST_SUITE(TestQualSuite)

#define ALL

using std::string;

TEST_CASE(Default) {
    KConfig* k(nullptr);
    REQUIRE_RC(KConfigMake(&k, nullptr));

    String* q(nullptr);
    REQUIRE_RC(KConfigReadString(k, "OS", &q));
    free(q); q = nullptr;

    REQUIRE_RC_FAIL(KConfigReadString(k, "/libs/vdb/quality", &q));
    REQUIRE_RC_FAIL(VDBManagerGetQualityString(nullptr, nullptr));

    const char* quality(nullptr);
    REQUIRE_RC(VDBManagerGetQualityString(nullptr, &quality));
    REQUIRE_EQ(string(quality), string("RZ"));

    REQUIRE_RC(VDBManagerPreferFullQuality(nullptr));
    REQUIRE_RC(VDBManagerGetQualityString(nullptr, &quality));
    REQUIRE_EQ(string(quality), string("RZ"));

    REQUIRE_RC(VDBManagerPreferZeroQuality(nullptr));
    REQUIRE_RC(VDBManagerGetQualityString(nullptr, &quality));
    REQUIRE_EQ(string(quality), string("ZR"));

    REQUIRE_RC(KConfigRelease(k));
}

TEST_CASE(Config) {
    VDBManagerQualityReset();

    KConfig* k(nullptr);
    REQUIRE_RC(KConfigMake(&k, nullptr));

    String* q(nullptr);
    REQUIRE_RC(KConfigReadString(k, "OS", &q));
    free(q); q = nullptr;

    REQUIRE_RC_FAIL(KConfigReadString(k, "/libs/vdb/quality", &q));
    REQUIRE_RC_FAIL(VDBManagerGetQualityString(nullptr, nullptr));

    REQUIRE_RC(KConfigWriteString(k, "/libs/vdb/quality", "012345678ZR9_AaBb"));

    const char* quality(nullptr);
    REQUIRE_RC(VDBManagerGetQualityString(nullptr, &quality));
    REQUIRE_EQ(string(quality), string("012345678ZR9_AaBb"));

    REQUIRE_RC(VDBManagerPreferFullQuality(nullptr));
    REQUIRE_RC(VDBManagerGetQualityString(nullptr, &quality));
    REQUIRE_EQ(string(quality), string("RZ"));

    REQUIRE_RC(VDBManagerPreferZeroQuality(nullptr));
    REQUIRE_RC(VDBManagerGetQualityString(nullptr, &quality));
    REQUIRE_EQ(string(quality), string("ZR"));

    REQUIRE_RC(KConfigRelease(k));
}

TEST_CASE(Env) {
    VDBManagerQualityReset();

    KConfig* k(nullptr);
    REQUIRE_RC(KConfigMake(&k, nullptr));

    String* q(nullptr);
    REQUIRE_RC(KConfigReadString(k, "OS", &q));
    free(q); q = nullptr;

    REQUIRE_RC_FAIL(KConfigReadString(k, "/libs/vdb/quality", &q));
    REQUIRE_RC_FAIL(VDBManagerGetQualityString(nullptr, nullptr));

    REQUIRE_EQ(putenv((char*)"NCBI_VDB_QUALITY=bBaA_9RZ976543210"), 0);
    REQUIRE_RC(KConfigWriteString(k, "/libs/vdb/quality", "012345678ZR9_AaBb"));

    const char* quality(nullptr);
    REQUIRE_RC(VDBManagerGetQualityString(nullptr, &quality));
    REQUIRE_EQ(string(quality), string("bBaA_9RZ976543210"));

    REQUIRE_RC(VDBManagerPreferFullQuality(nullptr));
    REQUIRE_RC(VDBManagerGetQualityString(nullptr, &quality));
    REQUIRE_EQ(string(quality), string("RZ"));

    REQUIRE_RC(VDBManagerPreferZeroQuality(nullptr));
    REQUIRE_RC(VDBManagerGetQualityString(nullptr, &quality));
    REQUIRE_EQ(string(quality), string("ZR"));

    REQUIRE_RC(KConfigRelease(k));
}

int main( int argc, char * argv [] ) {
    KConfigDisableUserSettings();
    return TestQualSuite(argc, argv);
}
