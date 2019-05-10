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
* Unit tests for cacheteefile3
*/

#include <cstring>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#include <ktst/unit_test.hpp>

#include <klib/out.h>
#include <klib/rc.h>
#include <klib/time.h>

#include <kproc/thread.h>

#include <kfs/defs.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/cachetee3file.h>
#include <kfs/recorder.h>

using namespace std;

#define DATAFILE "org.dat"
#define CACHEFILE "cache.dat"
#define CACHEFILE1 "cache.dat.cache"
#define DATAFILESIZE ( ( 1024 * 1024 * 32 ) + 300 )
#define BLOCKSIZE ( 1024 * 16 )

TEST_SUITE( CacheTeeTests );

class CT3Fixture
{
    public:
        CT3Fixture()
        {
            create_random_file( DATAFILE, DATAFILESIZE );
            remove_file( CACHEFILE );
            remove_file( CACHEFILE1 );
        }
        
        ~CT3Fixture()
        {
            remove_file( DATAFILE );
            remove_file( CACHEFILE );
            remove_file( CACHEFILE1 );
        }
        
        uint32_t rand_32( uint32_t min, uint32_t max )
        {
           double scaled = ( ( double )rand() / RAND_MAX );
           return ( ( max - min + 1 ) * scaled ) + min;
        }
        
        rc_t fill_file_with_random_data( KFile * file, size_t file_size )
        {
            rc_t rc = KFileSetSize( file, file_size );
            if ( rc == 0 )
            {
                uint64_t pos = 0;
                size_t total = 0;
                while ( rc == 0 && total < file_size )
                {
                    uint32_t data[ 512 ];
                    uint32_t i;
                    size_t to_write, num_writ;
                    
                    for ( i = 0; i < 512; ++i ) data[ i ] = rand_32( 0, 0xFFFFFFFF - 1 );
                    to_write = ( file_size - total );
                    if ( to_write > sizeof data ) to_write = sizeof data;
                    rc = KFileWriteAll ( file, pos, data, to_write, &num_writ );
                    if ( rc == 0 )
                    {
                        pos += num_writ;
                        total += num_writ;
                    }
                }
            }
            return rc;
        }

        rc_t create_random_file( const char * filename, uint64_t file_size )
        {
            KDirectory * dir;
            rc_t rc = KDirectoryNativeDir( &dir );
            if ( rc == 0 )
            {
                KFile * file;
                rc = KDirectoryCreateFile ( dir, &file, false, 0664, kcmInit, filename );
                if ( rc == 0 )
                {
                    if ( rc == 0 )
                        rc = fill_file_with_random_data( file, file_size );
                    KFileRelease( file );
                }
                KDirectoryRelease( dir );
            }
            return rc;
        }

        rc_t remove_file( const char * filename )
        {
            KDirectory * dir;
            rc_t rc = KDirectoryNativeDir( &dir );
            if ( rc == 0 )
            {
                rc = KDirectoryRemove ( dir, true, "%s", filename );
                KDirectoryRelease( dir );
            }
            return rc;
        }

        void report_diff( uint8_t * b1, uint8_t * b2, size_t n, uint32_t max_diffs )
        {
            size_t i, d;
            for ( i = 0, d = 0; i < n && d < max_diffs; ++i )
            {
                if ( b1[ i ] != b2[ i ] )
                {
                    KOutMsg( "[ %.08d ] %.02X %.02X\n", i, b1[ i ], b2[ i ] );
                    d++;
                }
            }
        }

        rc_t compare_file_content_1( const KFile * file1, const KFile * file2,
                                     uint64_t pos, size_t num_bytes, const char * msg )
        {
            rc_t rc = 0;
            uint8_t * buffer1 = ( uint8_t * )malloc( num_bytes );
            if ( msg != NULL )
                KOutMsg( "Test: KFileReadAll ( pos %lu, %u bytes, '%s' )\n", pos, num_bytes, msg );
            if ( buffer1 == NULL )
            {
                rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
                KOutMsg( "Test: cannot make buffer1 of size %u\n", num_bytes );
            }
            else
            {
                uint8_t * buffer2 = ( uint8_t * )malloc( num_bytes );
                if ( buffer2 == NULL )
                {
                    rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
                    KOutMsg( "Test: cannot make buffer2 of size %u\n", num_bytes );
                }
                else
                {
                    size_t num_read1;
                    rc = KFileReadAll ( file1, pos, buffer1, num_bytes, &num_read1 );
                    if ( rc != 0 )
                        KOutMsg( "Test: KFileReadAll( 1 ) -> %R\n", rc );
                    else
                    {
                        size_t num_read2;
                        rc = KFileReadAll ( file2, pos, buffer2, num_bytes, &num_read2 );
                        if ( rc != 0 )
                            KOutMsg( "Test: KFileReadAll( 2 ) -> %R\n", rc );
                        else
                        {
                            if ( num_read1 != num_read2 )
                            {
                                rc = RC ( rcRuntime, rcBuffer, rcReading, rcMemory, rcInvalid );
                                KOutMsg( "Test %d vs %d\n", num_read1, num_read2 );
                            }
                            else
                            {
                                int diff = memcmp( buffer1, buffer2, num_read1 );
                                if ( diff != 0 )
                                {
                                    report_diff( buffer1, buffer2, num_read1, 20 );
                                    rc = RC ( rcRuntime, rcBuffer, rcReading, rcMemory, rcCorrupt );
                                }
                            }
                        }
                    }
                    free( buffer2 );
                }
                free( buffer1 );
            }
            return rc;
        }

        rc_t compare_file_content_2( const KFile * file1, const KFile * file2,
                                     uint64_t pos, size_t num_bytes, const char * msg )
        {
            rc_t rc = 0;
            uint8_t * buffer1 = ( uint8_t * )malloc( num_bytes );
            if ( msg != NULL )
                KOutMsg( "Test: KFileReadExactly ( pos %lu, %u bytes, '%s' )\n", pos, num_bytes, msg );
            if ( buffer1 == NULL )
            {
                rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
                KOutMsg( "Test: cannot make buffer1 of size %u\n", num_bytes );
            }
            else
            {
                uint8_t * buffer2 = ( uint8_t * )malloc( num_bytes );
                if ( buffer2 == NULL )
                {
                    rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
                    KOutMsg( "Test: cannot make buffer2 of size %u\n", num_bytes );
                }
                else
                {
                    rc = KFileReadExactly ( file1, pos, buffer1, num_bytes );
                    if ( rc != 0 )
                        KOutMsg( "Test: KFileReadExactly( 1 ) -> %R\n", rc );
                    else
                    {
                        rc = KFileReadExactly ( file2, pos, buffer2, num_bytes );
                        if ( rc != 0 )
                            KOutMsg( "Test: KFileReadExactly( 2 ) -> %R\n", rc );
                        else
                        {
                            int diff = memcmp( buffer1, buffer2, num_bytes );
                            if ( diff != 0 )
                            {
                                report_diff( buffer1, buffer2, num_bytes, 20 );
                                rc = RC ( rcRuntime, rcBuffer, rcReading, rcMemory, rcCorrupt );
                            }
                        }
                    }
                    free( buffer2 );
                }
                free( buffer1 );
            }
            return rc;
        }

        rc_t read_all_loop( const KFile * f, uint64_t pos, uint8_t * buffer, size_t to_read )
        {
            rc_t rc = 0;
            size_t num_read_total = 0;
            /* uint32_t loop = 1; */
            uint8_t * dst = buffer;
            /* KOutMsg( "read_all_loop( at %lu, %u bytes )\n", pos, to_read ); */
            while( rc == 0 && num_read_total < to_read )
            {
                size_t num_read, n = to_read - num_read_total;
                rc = KFileRead ( f, pos, dst, n, &num_read );
                if ( rc != 0 )
                {
                    /* KOutMsg( "Test: KFileRead( at %lu, %u bytes ) -> %R\n", pos, n, rc ); */
                    if ( rcExhausted == GetRCState( rc ) && 
                         ( enum RCObject ) rcTimeout == GetRCObject( rc ) )
                    {
                        KSleepMs( 50 );
                        rc = 0;
                    }
                }
                else
                {
                    /* KOutMsg( "#%d read_all_loop( at %lu, %u bytes ) -> %u bytes\n", loop++, pos, n, num_read ); */
                    num_read_total += num_read;
                    pos += num_read;
                    dst += num_read;
                }
            }
            return rc;
        }

        rc_t compare_file_content_3( const KFile * file1, const KFile * file2,
                                     uint64_t pos, size_t num_bytes, const char * msg )
        {
            rc_t rc = 0;
            uint8_t * buffer1 = ( uint8_t * )malloc( num_bytes );
            if ( msg != NULL )
                KOutMsg( "Test: read_all_loop( pos %lu, %u bytes, '%s' )\n", pos, num_bytes, msg );
            if ( buffer1 == NULL )
            {
                rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
                KOutMsg( "Test: cannot make buffer1 of size %u\n", num_bytes );
            }
            else
            {
                uint8_t * buffer2 = ( uint8_t * )malloc( num_bytes );
                if ( buffer2 == NULL )
                {
                    rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
                    KOutMsg( "Test: cannot make buffer2 of size %u\n", num_bytes );
                }
                else
                {
                    rc = read_all_loop ( file1, pos, buffer1, num_bytes );
                    if ( rc == 0 )
                    {
                        rc = read_all_loop ( file2, pos, buffer2, num_bytes );
                        if ( rc == 0 )
                        {
                            int diff = memcmp( buffer1, buffer2, num_bytes );
                            if ( diff != 0 )
                            {
                                report_diff( buffer1, buffer2, num_bytes, 20 );
                                rc = RC ( rcExe, rcBuffer, rcReading, rcMemory, rcCorrupt );
                            }
                        }
                    }
                    free( buffer2 );
                }
                free( buffer1 );
            }
            return rc;
        }

}; // CT3Fixture

/*
static rc_t read_partial( const KFile * src, size_t block_size, uint64_t to_read )
{
    rc_t rc = 0;
    uint8_t * buffer = ( uint8_t * )malloc( block_size );
    if ( buffer == NULL )
        rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
    else
    {
        uint64_t pos = 0;
        uint64_t total_read = 0;
        while ( rc == 0 && total_read < to_read )
        {
            size_t num_read;
            if ( block_size > ( to_read - total_read ) )
                rc = KFileReadAll ( src, pos, buffer, to_read - total_read, &num_read );
            else
                rc = KFileReadAll ( src, pos, buffer, block_size, &num_read );
            if ( rc == 0 )
            {
                pos += num_read;
                total_read += num_read;
            }
        }
        free( buffer );
    }
    return rc;
}

static rc_t read_all( const KFile * src, size_t block_size )
{
    rc_t rc = 0;
    uint8_t * buffer = ( uint8_t * )malloc( block_size );
    if ( buffer == NULL )
        rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
    else
    {
        uint64_t pos = 0;
        size_t num_read = 1;
        while ( rc == 0 && num_read > 0 )
        {
            rc = KFileReadAll ( src, pos, buffer, block_size, &num_read );
            if ( rc == 0 )    pos += num_read;
        }
        free( buffer );
    }
    return rc;
}
*/

//////////////////////////////////////////// Test-cases

#if 0
FIXTURE_TEST_CASE( CacheTee3_Basic, CT3Fixture )
{
    KOutMsg( "Test: CacheTee3_Basic\n" );
    
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
    
    const KFile * tee;
    uint32_t cluster_factor = 0;
    uint32_t ram_pages = 0;
    REQUIRE_RC( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                                          cluster_factor, ram_pages,
                                          "%s", CACHEFILE ) );

    REQUIRE_RC( KFileRelease( tee ) );
    REQUIRE_RC( KFileRelease( org ) );
    
    const KFile * cache;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &cache, "%s.cache", CACHEFILE ) );
    
    KOutMsg( "Test: CacheTee3 file opened\n" );

    /*
    bool is_complete;
    REQUIRE_RC( CacheTee3FileIsComplete( cache, &is_complete ) );    
    REQUIRE( !is_complete );
    
    float percent;
    uint64_t bytes_in_cache;
    REQUIRE_RC( CacheTee3FileGetCompleteness( cache, &percent, &bytes_in_cache ) );
    REQUIRE( ( percent == 0.0 ) );
    REQUIRE( ( bytes_in_cache == 0 ) );
    */
    
    REQUIRE_RC( KFileRelease( cache ) );
    KOutMsg( "Test: CacheTee3 file closed\n" );
    
    REQUIRE_RC( KDirectoryRelease( dir ) );
}                                 


FIXTURE_TEST_CASE( CacheTee3_Read, CT3Fixture )
{
    KOutMsg( "Test: CacheTee3_Read\n" );
    
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
    
    const KFile * tee;
    uint32_t cluster_factor = 0;
    uint32_t ram_pages = 0;
    REQUIRE_RC( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                                          cluster_factor, ram_pages,
                                          "%s", CACHEFILE ) );

    uint64_t at = 0;
    size_t len = 100;

    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read at pos zero, not yet chached" ) );
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read at pos zero, now cached" ) );
    
    at = 10;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read at pos 10, cached" ) );
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "again, cached" ) );

    at = 1024L * BLOCKSIZE;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read at block boundary, not yet cached" ) );
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "again, now cached" ) );

    at = BLOCKSIZE / 2; len = BLOCKSIZE;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "spans 2 blocks, not yet cached" ) );
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "again, now cached" ) );

    len = BLOCKSIZE * 5 + 100;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "spans 5 blocks, partly cached" ) );
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "again, now cached" ) );

    at = 100; len = 1024 * BLOCKSIZE + 500;
    REQUIRE_RC( compare_file_content_3( org, tee, at, len, "large read crossing block boundary, partly cached" ) );

    at = 200; len = 2048 * BLOCKSIZE;
    REQUIRE_RC( compare_file_content_3( org, tee, at, len, "very large read, partly cached" ) );

    at = 1024 * BLOCKSIZE * 2 + 10; len = 100;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read after block boundary" ) );
    at = 1024 * BLOCKSIZE * 2 - 10;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read crossing block boundary" ) );

    at = DATAFILESIZE - 100; len = 300;
    REQUIRE_RC( compare_file_content_1( org, tee, at, len, "small read crossing EOF" ) );
    
    len = BLOCKSIZE * 10;
    REQUIRE_RC( compare_file_content_1( org, tee, at, len, "large read crossing EOF" ) );

    at = DATAFILESIZE - 10000; len = 10000;
    REQUIRE_RC( compare_file_content_1( org, tee, at, len, "large read at EOF" ) );

    at = DATAFILESIZE + 100; len = 100;
    REQUIRE_RC( compare_file_content_1( org, tee, at, len, "beyond EOF" ) );

    REQUIRE_RC( KFileRelease( tee ) );    
    REQUIRE_RC( KFileRelease( org ) );
    REQUIRE_RC( KDirectoryRelease( dir ) );
}
#endif

/* ------------------- CacheTee2_Multiple_Users_Multiple_Inst -------------------------------- */

static rc_t cache_access( CT3Fixture *fixture,
                          int tid, int num_threads,
                          const KFile * origfile, const KFile * cacheteefile )
{
    rc_t rc = 0;
    const int num_chunks = 16;
    int chunk_pos[ num_chunks ];
    int chunk_len[ num_chunks ];
    
    for ( int i = 0; i < num_chunks; ++i )
    {
        chunk_pos[ i ] = fixture -> rand_32( 0, DATAFILESIZE );
        chunk_len[ i ] = fixture -> rand_32( 100, DATAFILESIZE / 128 );
    }
    
    for ( int i = 0; i < num_chunks; ++i )
    {
        KOutMsg( "THREAD #%d / CHUNK #%d (%d.%d)\n", tid, i, chunk_pos[ i ], chunk_len[ i ] );
        rc = fixture -> compare_file_content_3( origfile, cacheteefile, chunk_pos[ i ], chunk_len[ i ], NULL );
        if ( rc != 0 )
            break;
        KOutMsg( "THREAD #%d / CHUNK #%d done\n", tid, i );
    }
    return rc;
}

struct ThreadData
{
    int tid;
    int num_threads;
    const KFile * origfile; // optional
    const KFile * cacheteefile; // optional
    CT3Fixture * fixture;
};

static rc_t CC thread_func( const KThread *self, void *data )
{
    ThreadData * td = ( ThreadData * ) data;
    if ( td -> cacheteefile == NULL || td -> origfile == NULL )
    {
        KDirectory * dir;
        rc_t rc = KDirectoryNativeDir( &dir );
        if ( rc == 0 )
        {
            const KFile * org;
            rc = KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE );
            if ( rc == 0 )
            {
                const KFile * tee;
                uint32_t cluster_factor = 0;
                uint32_t ram_pages = 0;
                rc = KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                                                     cluster_factor, ram_pages,
                                                     "%s", CACHEFILE );
                if ( rc == 0 )
                {
                    //KOutMsg( "Thread #%d\n", td -> tid );
                    rc = cache_access( td -> fixture, td -> tid, td -> num_threads, org, tee );
                    KOutMsg( "Thread #%d : %s\n", td -> tid, rc == 0 ? "OK" : "ERR" );
                    KFileRelease( tee );
                }
                KFileRelease( org );
            }
            KDirectoryRelease( dir );
        }
        return rc;
    }
    return 0;
}

const int num_treads = 2;

FIXTURE_TEST_CASE( CacheTee3_Multiple_Users_Multiple_Inst, CT3Fixture )
{
    KOutMsg( "Test: CacheTee3_Multiple_Users_Multiple_Inst\n" );
    remove_file( CACHEFILE );
    remove_file( CACHEFILE1 );

    KThread *t [ num_treads ];
    ThreadData td [ num_treads ];
    rc_t rc = 0;
    for ( int i = 0; i < num_treads && rc == 0; ++i )
    {
        //KOutMsg( "creating thread #%d\n", i );
        td[ i ].tid = i + 1;
        td[ i ].num_threads = num_treads;
        td[ i ].origfile = NULL;
        td[ i ].cacheteefile = NULL;
        td[ i ].fixture = this;        
        rc = KThreadMake ( &( t[ i ] ), thread_func, &( td[ i ] ) );
        REQUIRE_RC( rc );
    }
    
    for ( int i = 0; i < num_treads && rc == 0; ++i )
    {
        //KOutMsg( "waiting for thread #%d\n", i );        
        rc_t rc_thread;
        rc = KThreadWait ( t[ i ], &rc_thread );
        REQUIRE_RC( rc );
        REQUIRE_RC( rc_thread );
        REQUIRE_RC( KThreadRelease ( t[ i ] ) );
    }
}


//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

rc_t CC UsageSummary ( const char * progname )
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "cachetee3-test";

#define OPTION_DUMMY    "dummy"
#define ALIAS_DUMMY     "d"

static const char * dummy_usage[] = { "dummy argument", NULL };

OptDef TestOptions[] =
{
    { OPTION_DUMMY, ALIAS_DUMMY, NULL, dummy_usage, 1, false,  false }
};

rc_t CC KMain ( int argc, char *argv [] )
{
    Args * args;
    /* bool has_info = true; */
    /* we are adding this dummy argument to enable commandline parsing for the verbose flag(s) -vvvvvvvv */
    rc_t rc = ArgsMakeAndHandle( &args, argc, argv,
            1, TestOptions, sizeof TestOptions / sizeof TestOptions [ 0 ] );
    if ( rc == 0 )
    {
        srand( time( NULL ) );
        KConfigDisableUserSettings();
        rc = CacheTeeTests( argc, argv );
        KOutMsg( "and the result is: %R\n", rc );
    }

    /*
    while( has_info )
    {
        rc_t rc1;
        const char * filename;
        const char * funcname;
        uint32_t lineno;
        has_info = GetUnreadRCInfo ( &rc1, &filename, &funcname, &lineno );    
    }
    */
    return rc;
}

}
