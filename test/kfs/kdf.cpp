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


#include <kapp/args.h> /* ArgsMakeAndHandle */

#include <kfs/directory.h> /* KDirectoryGetDiskFreeSpace */

#include <klib/out.h> /* OUTMSG */

#include <ktst/unit_test.hpp> // TEST_SUITE

#include <cmath> // ceil


static rc_t argsHandler ( int argc, char * argv [] );
TEST_SUITE_WITH_ARGS_HANDLER ( DuSuite, argsHandler )
static Args * args = NULL;
static const char * path = NULL;


#if MAC

TEST_CASE ( Mac ) {
    uint64_t free_bytes_available = 0;
    uint64_t total_number_of_bytes = 0;

    REQUIRE_RC_FAIL ( KDirectoryGetDiskFreeSpace ( NULL,
                        & free_bytes_available, & total_number_of_bytes ) );

    KDirectory * native = NULL;
    REQUIRE_RC ( KDirectoryNativeDir ( & native ) );

    if ( path != NULL ) {
        const KDirectory * dir = NULL;
        REQUIRE_RC ( KDirectoryOpenDirRead ( native, & dir, false, path ) );

        REQUIRE_RC ( KDirectoryGetDiskFreeSpace ( dir,
                        & free_bytes_available, & total_number_of_bytes ) );

        OUTMSG ( ( "Filesystem   512-blocks       Used Available Capacity   "
                   "iused     ifree %%iused  Mounted on" ) );

        uint64_t used = total_number_of_bytes - free_bytes_available;
        double percent = 100. * used / total_number_of_bytes ;
        OUTMSG ( ( "\n                        %d %d    %d%%",
                   used / 512, free_bytes_available / 512,
                   static_cast < int > ( ceil ( percent ) ) ) ) ;

        REQUIRE_RC ( KDirectoryRelease ( dir ) );
    }

    REQUIRE_RC ( KDirectoryRelease ( native ) );
}

#else

TEST_CASE ( Linux ) {
    uint64_t free_bytes_available = 0;
    uint64_t total_number_of_bytes = 0;

    REQUIRE_RC_FAIL ( KDirectoryGetDiskFreeSpace ( NULL,
                        & free_bytes_available, & total_number_of_bytes ) );

    KDirectory * native = NULL;
    REQUIRE_RC ( KDirectoryNativeDir ( & native ) );

    if ( path != NULL ) {
        const KDirectory * dir = NULL;
        REQUIRE_RC ( KDirectoryOpenDirRead ( native, & dir, false, path ) );

        REQUIRE_RC ( KDirectoryGetDiskFreeSpace ( dir,
                        & free_bytes_available, & total_number_of_bytes ) );

        OUTMSG ( (
            "Filesystem           1K-blocks     Used Available Use%% Mounted on"
        ) );

        uint64_t used = total_number_of_bytes - free_bytes_available;
        double percent = 100. * used / total_number_of_bytes ;
        OUTMSG ( ( "\n                               %d   %d  %d%% %s",
                   used / 1024, free_bytes_available / 1024,
                   static_cast < int > ( ceil ( percent ) ), path ) ) ;

        REQUIRE_RC ( KDirectoryRelease ( dir ) );
    }

    REQUIRE_RC ( KDirectoryRelease ( native ) );
}

#endif

static rc_t argsHandler ( int argc, char * argv [] ) {
    uint32_t params = 0;

    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0, NULL, 0 );

    if ( rc == 0 )
        rc = ArgsParamCount ( args, & params );

    if ( rc == 0 && params > 0 )
        rc = ArgsParamValue ( args, 0,
                              reinterpret_cast < const void ** > ( & path ) );

    return rc;
}

rc_t CC UsageSummary (const char * progname) { return 0; }
rc_t CC Usage ( const Args * args ) { return 0; }
const char UsageDefaultName [] = "kdf";

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t KMain ( int argc, char * argv [] ) {
        rc_t rc = DuSuite ( argc, argv );

        path = NULL;

        ArgsWhack ( args );
        args = NULL;

        return rc;
    }
}
