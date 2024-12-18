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
* Test of using telemetry bits in UserAgent HTTP header
*/

#include <kfg/kfg-priv.h> /* KConfigMakeEmpty */
#include <kns/manager.h> /* KNSManagerGetUserAgent */
#include <ktst/unit_test.hpp> // TEST_SUITE

using std::string;

bool ends_with(string const &fullString, string const &ending) {
    if (fullString.length() >= ending.length())
        return (0 == fullString.compare(
            fullString.length() - ending.length(),
            ending.length(), ending));
    else
        return false;
}

TEST_SUITE(KNS_opt_bitmapTestSuite)

TEST_CASE(TestTelemetry) {
    KConfig * kfg(nullptr);
    REQUIRE_RC(KConfigMakeEmpty(&kfg));

    REQUIRE_RC_FAIL(KNSManagerGetUserAgent(nullptr));
    const char * ua(nullptr);

    // SENDING TELEMETRY IS ALWAYS ENABLED

    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
    REQUIRE(ends_with(ua, ",bmap=nob)"));

    putenv(const_cast<char*>("VDB_OPT_BITMAP="));
    ua = nullptr;
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
#ifndef WINDOWS
    REQUIRE(ends_with(ua, ",bmap=)"));
#else
    REQUIRE(ends_with(ua, ",libc=,bmap=nob)"));
#endif

    putenv(const_cast<char*>("VDB_OPT_BITMAP=0"));
    ua = nullptr;
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
    REQUIRE(ends_with(ua, ",bmap=0)"));

    putenv(const_cast<char*>("VDB_OPT_BITMAP=1"));
    ua = nullptr;
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
    REQUIRE(ends_with(ua, ",bmap=1)"));

    // ENABLE SENDING TELEMETRY IN CONFIGURATION: IT IS IGNORED

    REQUIRE_RC(KConfigWriteString(kfg, "libs/kns/send-telemetry", "true"));
    
#ifndef WINDOWS
    unsetenv("VDB_OPT_BITMAP");
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
    REQUIRE(ends_with(ua, ",bmap=nob)"));
#endif

    putenv(const_cast<char*>("VDB_OPT_BITMAP="));
    ua = nullptr;
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
 // std::cerr << "UA='" << ua << "'\n";
#ifndef WINDOWS
    REQUIRE(ends_with(ua, ",bmap=)"));
#else
    REQUIRE(ends_with(ua, ",libc=,bmap=nob)"));
#endif

    putenv(const_cast<char*>("VDB_OPT_BITMAP=0"));
    ua = nullptr;
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
    REQUIRE(ends_with(ua, ",bmap=0)"));

    putenv(const_cast<char*>("VDB_OPT_BITMAP=101"));
    ua = nullptr;
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
    REQUIRE(ends_with(ua, ",bmap=101)"));

    // DISABLE SENDING TELEMETRY IN CONFIGURATION: IT IS IGNORED

    REQUIRE_RC(KConfigWriteString(kfg, "libs/kns/send-telemetry", "false"));

#ifndef WINDOWS
    unsetenv("VDB_OPT_BITMAP");
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
    REQUIRE(ends_with(ua, ",bmap=nob)"));
#endif

    putenv(const_cast<char*>("VDB_OPT_BITMAP="));
    ua = nullptr;
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
#ifndef WINDOWS
    REQUIRE(ends_with(ua, ",bmap=)"));
#else
    REQUIRE(ends_with(ua, ",libc=,bmap=nob)"));
#endif

    putenv(const_cast<char*>("VDB_OPT_BITMAP=0"));
    ua = nullptr;
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
    REQUIRE(ends_with(ua, ",bmap=0)"));

    putenv(const_cast<char*>("VDB_OPT_BITMAP=1"));
    ua = nullptr;
    REQUIRE_RC(KNSManagerGetUserAgent(&ua));
    REQUIRE(ends_with(ua, ",bmap=1)"));

    REQUIRE_RC(KConfigRelease(kfg));
}

extern "C" {
    ver_t CC KAppVersion(void) { return 0; }

    rc_t CC KMain(int argc, char *argv[]) {
        KConfigDisableUserSettings();
        return KNS_opt_bitmapTestSuite(argc, argv);
    }
}
