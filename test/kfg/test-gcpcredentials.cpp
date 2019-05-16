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
 * Unit tests for GCP Credentials
 */

#include <ktst/unit_test.hpp>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/lockfile.h>

#include <kfg/config.h>
#include <kfg/gcp-credentials.h>
#include <kfg/kfg-priv.h>

#include <klib/text.h>

#include <kproc/thread.h>

#include <fstream>
#include <stdexcept>

using namespace std;
using namespace ncbi::NK;

TEST_SUITE ( GCPCredentialsTestSuite );

TEST_CASE ( GCPCredentialsMake )
{
    KDirectory *wd = NULL;
    REQUIRE_RC ( KDirectoryNativeDir ( &wd ) );
    const char *fileName = "test-gcp.json";
    const KFile *file;
    KJsonValue *root;
    REQUIRE_RC ( KDirectoryOpenFileRead ( wd, &file, fileName ) );
    REQUIRE_RC ( LoadGcpCredentials ( file, &root ) );

    REQUIRE_NOT_NULL ( root );

    REQUIRE_RC ( KFileRelease ( file ) );

    KJsonValueWhack ( root );
}


//////////////////////////////////////////// Main

extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void ) { return 0x1000000; }
rc_t CC UsageSummary ( const char *progname ) { return 0; }

rc_t CC Usage ( const Args *args ) { return 0; }

const char UsageDefaultName[] = "test-gcpcredentials";

rc_t CC KMain ( int argc, char *argv[] )
{
    KConfigDisableUserSettings ();
    rc_t rc = GCPCredentialsTestSuite ( argc, argv );
    return rc;
}
}
