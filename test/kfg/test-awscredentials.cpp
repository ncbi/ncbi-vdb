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
 * Unit tests for AWS Credentials
 */

#include <ktst/unit_test.hpp>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/lockfile.h>

#include <kfg/config.h>
#include <kfg/kfg-priv.h>

#include <klib/text.h>

#include <kproc/thread.h>

#include <cstdlib>
#include <fstream>
#include <stdexcept>

using namespace std;
using namespace ncbi::NK;

TEST_SUITE ( awsCredentialsTestSuite );

TEST_CASE ( awsCredentialsEnv )
{
    // rc_t rc=0;

    KDirectory *native = NULL;
    REQUIRE_RC ( KDirectoryNativeDir ( &native ) );

    const KDirectory *dirc = NULL;
    KConfig *cfg = NULL;
    // VFSManager * mgr = NULL;

    char aws_access_key_id[512] = "";
    char aws_secret_access_key[512] = "";

    const char *test_key = "ABCDEFH";
    const char *test_secret = "SECRETSECRET";

    setenv ( "AWS_ACCESS_KEY_ID", test_key, 1 );
    setenv ( "AWS_SECRET_ACCESS_KEY", test_secret, 1 );

    REQUIRE_RC ( KDirectoryOpenDirRead ( native, &dirc, false, "cloud-kfg" ) );
    REQUIRE_RC ( KConfigMake ( &cfg, dirc ) );

    size_t buf_sz;

    REQUIRE_RC ( KConfigRead ( cfg, "/aws/aws_access_key_id", 0,
        aws_access_key_id, sizeof aws_access_key_id, &buf_sz, NULL ) );
    fprintf ( stderr, "returned %s\n", aws_access_key_id );
    REQUIRE_EQ ( strcmp ( aws_access_key_id, test_key ), 0 );
    REQUIRE_RC ( KConfigRead ( cfg, "/aws/aws_secret_access_key", 0,
        aws_secret_access_key, sizeof aws_secret_access_key, &buf_sz, NULL ) );
    REQUIRE_EQ ( strcmp ( aws_secret_access_key, test_secret ), 0 );

    unsetenv ( "AWS_ACCESS_KEY_ID" );
    unsetenv ( "AWS_SECRET_ACCESS_KEY" );

    REQUIRE_RC ( KConfigMake ( &cfg, dirc ) );
    REQUIRE_RC ( KConfigRead ( cfg, "/aws/aws_access_key_id", 0,
        aws_access_key_id, sizeof aws_access_key_id, &buf_sz, NULL ) );
    REQUIRE_EQ ( strcmp ( aws_access_key_id, "test_from_file" ), 0 );
    REQUIRE_RC ( KConfigRead ( cfg, "/aws/aws_secret_access_key", 0,
        aws_secret_access_key, sizeof aws_secret_access_key, &buf_sz, NULL ) );
    REQUIRE_EQ ( strcmp ( aws_secret_access_key, "secret_from_file" ), 0 );
}


//////////////////////////////////////////// Main

extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void ) { return 0x1000000; }
rc_t CC UsageSummary ( const char *progname ) { return 0; }

rc_t CC Usage ( const Args *args ) { return 0; }

const char UsageDefaultName[] = "test-awscredentials";

rc_t CC KMain ( int argc, char *argv[] )
{
    KConfigDisableUserSettings ();
    rc_t rc = awsCredentialsTestSuite ( argc, argv );
    return rc;
}
}
