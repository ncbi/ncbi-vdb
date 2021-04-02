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
 * Unit tests for timeout conditions in cacheteefile3
 */

#include <ktst/unit_test.hpp>
#include <klib/out.h>
#include <klib/rc.h>
#include <klib/status.h>
#include <kfs/file-impl.h>
#include <kfs/directory.h>
#include <kfs/cachetee3file.h>

#include <os-native.h>

#include <thread>

using namespace std;

TEST_SUITE ( CacheTee3TimeoutTests );

rc_t destroy( KFILE_IMPL *self ) { return 0; }
struct KSysFile_v1* get_sysfile ( const KFILE_IMPL *self, uint64_t *offset ) { return nullptr; }
rc_t random_access ( const KFILE_IMPL *self ) { return 0; }
rc_t get_size ( const KFILE_IMPL *self, uint64_t *size )
{
    *size = 1024*1024;
    return 0;
}
rc_t set_size ( KFILE_IMPL *self, uint64_t size ) { return 0; }
rc_t read ( const KFILE_IMPL *self, uint64_t pos,void *buffer, size_t bsize, size_t *num_read ) { return 0; }
rc_t write ( KFILE_IMPL *self, uint64_t pos,    const void *buffer, size_t size, size_t *num_writ ) { return 0; }
uint32_t get_type ( const KFILE_IMPL * self ) { return 0; }

rc_t timed_read ( const KFILE_IMPL *self, uint64_t pos, void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
    return 0;
}

rc_t timed_write ( KFILE_IMPL *self, uint64_t pos,    const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm ) { return 0; }
rc_t read_chunked ( const KFILE_IMPL *self, uint64_t pos,    struct KChunkReader * chunks, size_t bsize, size_t * num_read ) { return 0; }

uint32_t sleep_for_ms = 0;

rc_t timed_read_chunked ( const KFILE_IMPL *self, uint64_t pos,    struct KChunkReader * chunks, size_t bsize, size_t * num_read, struct timeout_t * tm )
{
    this_thread::sleep_for (chrono::milliseconds( sleep_for_ms ));
    cout << "Hallo Chunked Herr Raetz" << endl;
    *num_read = 0;
    return 0;
}

KFile_vt_v1 test_vt =
{
    1, 3,
    destroy,
    get_sysfile,
    random_access,
    get_size ,
    set_size,
    read,
    write,
    get_type,
    timed_read,
    timed_write,
    read_chunked,
    timed_read_chunked
};

struct TestKFile
{
    KFile_v1 dad;

    TestKFile()
    {
        THROW_ON_RC( KFileInit( & dad, (const KFile_vt*) &test_vt, "TestKFile", "dummy", true, false ) );
    }
};

TEST_CASE( TestKFile_Timeout)
{
    TestKFile f;
    struct timeout_t tm;
    tm . mS = 50;
    tm . prepared = false;
    char buf[1024];
    size_t num_read;
    sleep_for_ms = 0;
    REQUIRE_RC( KFileTimedRead( & f.dad, 0, buf, sizeof buf, &num_read, &tm ) );
}

TEST_CASE( CacheTee3_Timeout )
{
KStsLibHandlerSetStdErr();

    struct KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    struct KFile const * tee;
    TestKFile f;
    sleep_for_ms = 15000;

    REQUIRE_RC( KDirectoryMakeKCacheTeeFile_v3 (
        dir,
        & tee,
        & f.dad,
        1024 * 16,
        0,
        0,
        false,
        false,
        "%s", GetName() ) );

    struct timeout_t tm = { { 0, 0 }, 5000, false };
    char buf[1024];
    size_t num_read;

KStsLevelSet (5);
    REQUIRE_RC_FAIL( KFileTimedRead( tee, 0, buf, sizeof buf, &num_read, &tm ) );
    tm.prepared = false;
    REQUIRE_RC( KFileTimedRead( tee, 0, buf, sizeof buf, &num_read, &tm ) );
KStsLevelSet (0);
    REQUIRE_EQ( 0, (int)num_read );

    // remove ./cache
    remove( ( string( GetName() ) + ".cache" ) . c_str() );

}

//////////////////////////////////////////// Main
extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void ) { return 0x1000000; }

rc_t CC UsageSummary ( const char *progname ) { return 0; }

rc_t CC Usage ( const Args *args ) { return 0; }

const char UsageDefaultName[] = "cachetee3-test";

#define OPTION_DUMMY "dummy"
#define ALIAS_DUMMY "d"

static const char *dummy_usage[] = {"dummy argument", NULL};

OptDef TestOptions[]
    = {{OPTION_DUMMY, ALIAS_DUMMY, NULL, dummy_usage, 1, false, false}};

rc_t CC KMain ( int argc, char *argv[] )
{
    Args *args;

    /* we are adding this dummy argument to enable commandline parsing for the
     * verbose flag(s) -vvvvvvvv */
    rc_t rc = ArgsMakeAndHandle ( &args, argc, argv, 1, TestOptions,
                                  sizeof TestOptions / sizeof TestOptions[0] );
    if ( rc == 0 )
    {
        srand ( time ( NULL ) );
        KConfigDisableUserSettings ();

        rc = CacheTee3TimeoutTests ( argc, argv );

        if ( (rc_t)-1 != rc )
            KOutMsg ( "and the result is: %R\n", rc );
        else
            KOutMsg ( "and the result is: %d\n", rc );
        ArgsWhack ( args );
    }
    return rc;
}

} /* extern "C" */


