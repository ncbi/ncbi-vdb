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
 * ========================================================================== */

#include <kapp/args.h> /* Args */

#include <kfg/kfg-priv.h> /* KConfigMakeEmpty */

#include <klib/text.h> /* string_cmp */

#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
 
#include <ktst/unit_test.hpp> /* TEST_SUITE_WITH_ARGS_HANDLER */

static rc_t argsHandler(int argc, char * argv[]) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

TEST_SUITE_WITH_ARGS_HANDLER(TestHashInPwd, argsHandler)

static const char pwdKfg[] = "o#";

TEST_CASE(TestKey) {
    KConfig * kfg = NULL;
    REQUIRE_RC(KConfigMakeEmpty(&kfg));

    REQUIRE_RC(KConfigWriteString(kfg,
        "/repository/user/protected/dbGaP-1/encryption-key", pwdKfg));
    REQUIRE_RC(KConfigWriteString(kfg,
        "/repository/user/protected/dbGaP-1/root", "."));

    struct VFSManager * mgr = NULL;
    REQUIRE_RC(VFSManagerMakeFromKfg(&mgr, kfg));

    char pwdRead[9] = "";
    size_t size = 0;

    // VDB-4394: protected configuration is ignored
    REQUIRE_RC_FAIL(
        VFSManagerGetKryptoPassword(mgr, pwdRead, sizeof pwdRead, &size));
/*  REQUIRE_EQ(string_cmp(
        pwdKfg, sizeof pwdKfg, pwdRead, size + 1, sizeof pwdRead), 0); */

    REQUIRE_RC(VFSManagerRelease(mgr));

    REQUIRE_RC(KConfigRelease(kfg));
}

TEST_CASE(TestFile) {
    KConfig * kfg = NULL;
    REQUIRE_RC(KConfigMakeEmpty(&kfg));

    REQUIRE_RC(KConfigWriteString(kfg,
        "/repository/user/protected/dbGaP-1/encryption-key-path",
        "test-hash-in-pwd.enc_key"));
    REQUIRE_RC(KConfigWriteString(kfg,
        "/repository/user/protected/dbGaP-1/root", "."));

    struct VFSManager * mgr = NULL;
    REQUIRE_RC(VFSManagerMakeFromKfg(&mgr, kfg));

    char pwdRead[9] = "";
    size_t size = 0;

    /* VDB-4394: protected configuration is ignored.
    We might want to use ngc file to perform this test. */
    REQUIRE_RC_FAIL(
        VFSManagerGetKryptoPassword(mgr, pwdRead, sizeof pwdRead, &size));
/*  REQUIRE_EQ(string_cmp(
        pwdKfg, sizeof pwdKfg, pwdRead, size + 1, sizeof pwdRead), 0); */

    REQUIRE_RC(VFSManagerRelease(mgr));

    REQUIRE_RC(KConfigRelease(kfg));
}

const char UsageDefaultName[] = "test-hash-in-pwd";
rc_t CC UsageSummary(const char * progname) { return 0; }
rc_t CC Usage(const Args * args) { return 0; }

extern "C" {

    ver_t CC KAppVersion(void) { return 0; }

    int KMain(int argc, char *argv[]) {
        KConfigDisableUserSettings(); // ignore ~/.ncbi/user-settings.mkfg

        rc_t rc = TestHashInPwd(argc, argv);
        return rc;
    }

}

/******************************************************************************/
