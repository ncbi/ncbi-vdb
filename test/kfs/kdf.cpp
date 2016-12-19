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
* Tests for KDirectoryGetDiskFreeSpace.
*/


#include <kapp/args.h> /* ArgsMakeAndHandle */

#include <kfs/directory.h> /* KDirectoryGetDiskFreeSpace */

#include <klib/out.h> /* OUTMSG */
#include <klib/printf.h> /* string_printf */

#include <strtol.h> /* strtou64 */

#include <ktst/unit_test.hpp> // TEST_SUITE

#include <cmath> // ceil
#include <cstdio> // popen


static rc_t argsHandler ( int argc, char * argv [] );
TEST_SUITE_WITH_ARGS_HANDLER ( DuSuite, argsHandler )
static Args * args = NULL;
static const char * s_path = NULL;


#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)


struct FIXTURE {
    const char * _path;
    const KDirectory * _dir;

    FIXTURE ( const char * aPath = NULL )
        : _path ( aPath ? aPath : s_path )
        , _dir ( NULL )
    {
        if ( _path == NULL )
            return;

        if ( _path [ 0 ] == '~' && _path [ 1 ] == '\0' )
            _path = getenv ( "HOME" );

        if ( _path == NULL )
            return;

        KDirectory * native = NULL;
        rc_t rc = KDirectoryNativeDir ( & native );
        if ( rc != 0 )
            throw rc;

        rc = KDirectoryOpenDirRead ( native, & _dir, false, _path );
        if ( rc != 0 )
            throw rc;

        rc = KDirectoryRelease ( native );
        if ( rc != 0 )
            throw rc;
    }

    ~FIXTURE ( void ) {
        rc_t rc = 0;
        RELEASE ( KDirectory, _dir );
    }
};


/* The following 2 test cases ( Mac and Linux )
 *  generate output somewhat similar to df <dir>
 * To generate the output run "kdf -app_args=<dir>"
 *  and visually compare it with the output of "df".
 *
 * The next case runs "df ~", parses its output
 *  and compare it with KDirectoryGetDiskFreeSpace results.
 */

#if MAC

FIXTURE_TEST_CASE ( Mac, FIXTURE ) {
    uint64_t free_bytes_available = 0;
    uint64_t total_number_of_bytes = 0;

    REQUIRE_RC_FAIL ( KDirectoryGetDiskFreeSpace ( NULL,
                        & free_bytes_available, & total_number_of_bytes ) );

    if ( _path != NULL ) {
        REQUIRE_RC ( KDirectoryGetDiskFreeSpace ( _dir,
                        & free_bytes_available, & total_number_of_bytes ) );

        OUTMSG ( ( "Filesystem   512-blocks       Used Available Capacity   "
                   "iused     ifree %%iused  Mounted on" ) );

        uint64_t used = total_number_of_bytes - free_bytes_available;
        double percent = 100. * used / total_number_of_bytes ;
        OUTMSG ( ( "\n                        %d %d    %d%%",
                   used / 512, free_bytes_available / 512,
                   static_cast < int > ( ceil ( percent ) ) ) ) ;
    }
}

#else

FIXTURE_TEST_CASE ( Linux, FIXTURE ) {
    uint64_t free_bytes_available = 0;
    uint64_t total_number_of_bytes = 0;

    REQUIRE_RC_FAIL ( KDirectoryGetDiskFreeSpace ( NULL,
                        & free_bytes_available, & total_number_of_bytes ) );

    if ( _path != NULL ) {
        REQUIRE_RC ( KDirectoryGetDiskFreeSpace ( _dir,
                        & free_bytes_available, & total_number_of_bytes ) );

        OUTMSG ( (
            "Filesystem           1K-blocks     Used Available Use%% Mounted on"
        ) );

        uint64_t used = total_number_of_bytes - free_bytes_available;
        double percent = 100. * used / total_number_of_bytes ;
        OUTMSG ( ( "\n                               %d   %d  %d%% %s",
                   used / 1024, free_bytes_available / 1024,
                   static_cast < int > ( ceil ( percent ) ), _path ) ) ;
    }
}

#endif

TEST_CASE ( testKDirectoryGetDiskFreeSpace ) {
    FIXTURE fixture ( "~" );
    uint64_t free_bytes_available = 0;
    uint64_t total_number_of_bytes = 0;
    REQUIRE_RC ( KDirectoryGetDiskFreeSpace ( fixture . _dir,
                    & free_bytes_available, & total_number_of_bytes ) );

    char command [ 256 ] = "";
    REQUIRE_RC ( string_printf
        ( command, sizeof command, NULL, "df -k %s", fixture . _path ) );

    FILE * fp = popen ( command, "r" );
    REQUIRE ( fp );

    char line [ 2 ] [ 1035 ];
    char * crnt = line [ 0 ];
    char * prev = line [ 1 ];
    while ( fgets ( crnt, sizeof line [ 0 ] - 1, fp ) != NULL ) {
        char * tmp = crnt;
        crnt = prev;
        prev = tmp;
    }

    REQUIRE_EQ ( pclose ( fp ), 0 );

    REQUIRE ( prev );

    crnt = prev;
    for ( int i = 0, in = true; * crnt != '\0'; ++ crnt ) {
        if ( * crnt == ' ' ) {
            if ( in ) {
                in = false;
                ++ i;
            }
        }
        else
            if ( ! in ) {
                in = true;
                if ( i == 1 || i == 3 ) {
                    uint64_t r = strtou64 ( crnt, & crnt, 10 );
                    switch ( i ) {
                        case 1:
                            CHECK_EQ ( r, total_number_of_bytes / 1024 );
                            break;
                        case 3:
                            CHECK_EQ ( r, free_bytes_available / 1024 );
                    }
                    -- crnt;
                }
            }
    }
}

static rc_t argsHandler ( int argc, char * argv [] ) {
    uint32_t params = 0;

    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0, NULL, 0 );

    if ( rc == 0 )
        rc = ArgsParamCount ( args, & params );

    if ( rc == 0 && params > 0 )
        rc = ArgsParamValue ( args, 0,
                              reinterpret_cast < const void ** > ( & s_path ) );

    return rc;
}

rc_t CC UsageSummary (const char * progname) { return 0; }
rc_t CC Usage ( const Args * args ) { return 0; }
const char UsageDefaultName [] = "kdf";

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t KMain ( int argc, char * argv [] ) {
        rc_t rc = DuSuite ( argc, argv );

        s_path = NULL;

        ArgsWhack ( args );
        args = NULL;

        return rc;
    }
}
