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
#define DATAFILESIZE ( ( 1024 * 1024 ) + 300 )
#define BLOCKSIZE ( 1024 * 16 )

TEST_SUITE( CacheTeeTests );

static uint32_t rand_32( uint32_t min, uint32_t max )
{
       double scaled = ( ( double )rand() / RAND_MAX );
       return ( ( max - min + 1 ) * scaled ) + min;
}

static rc_t fill_file_with_random_data( KFile * file, size_t file_size )
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

static rc_t create_random_file( const char * filename, uint64_t file_size )
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


static rc_t remove_file( const char * filename )
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


static void report_diff( uint8_t * b1, uint8_t * b2, size_t n, uint32_t max_diffs )
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

static rc_t compare_file_content( const KFile * file1, const KFile * file2, uint64_t pos, size_t bytes )
{
    rc_t rc = 0;
    uint8_t * buffer1 = ( uint8_t * )malloc( bytes );
    if ( buffer1 == NULL )
        rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
    else
    {
        uint8_t * buffer2 = ( uint8_t * )malloc( bytes );
        if ( buffer2 == NULL )
            rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
        else
        {
            size_t num_read1;
            rc = KFileReadAll ( file1, pos, buffer1, bytes, &num_read1 );
            if ( rc == 0 )
            {
                size_t num_read2;
                rc = KFileReadAll ( file2, pos, buffer2, bytes, &num_read2 );
                if ( rc == 0 )
                {
                    if ( num_read1 != num_read2 )
                        rc = RC ( rcRuntime, rcBuffer, rcReading, rcMemory, rcInvalid );
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


// we create a data-file that we want to wrap ....
static rc_t prepare_cachetee_tests( void )
{
    rc_t rc = create_random_file( DATAFILE, DATAFILESIZE );
    remove_file( CACHEFILE );
    remove_file( CACHEFILE1 );
    return rc;
}

// we remove everything that was created ...
static void finish_cachetee_tests( void )
{
    remove_file( DATAFILE );
    remove_file( CACHEFILE );
    remove_file( CACHEFILE1 );
}

//////////////////////////////////////////// Test-cases
TEST_CASE( CacheTee3_Basic )
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
    
#if 0    
    bool is_complete;
    REQUIRE_RC( CacheTee3FileIsComplete( cache, &is_complete ) );    
    REQUIRE( !is_complete );
    
    float percent;
    uint64_t bytes_in_cache;
    REQUIRE_RC( CacheTee3FileGetCompleteness( cache, &percent, &bytes_in_cache ) );
    REQUIRE( ( percent == 0.0 ) );
    REQUIRE( ( bytes_in_cache == 0 ) );
#endif

    REQUIRE_RC( KFileRelease( cache ) );
    REQUIRE_RC( KDirectoryRelease( dir ) );
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

    rc_t rc = ArgsMakeAndHandle( &args, argc, argv,
            1, TestOptions, sizeof TestOptions / sizeof TestOptions [ 0 ] );
    if ( rc == 0 )
    {
        srand( time( NULL ) );
        KConfigDisableUserSettings();
        rc = prepare_cachetee_tests();
        if ( rc == 0 )
        {
            rc = CacheTeeTests( argc, argv );
            finish_cachetee_tests();
        }
        KOutMsg( "and the result is: %R\n", rc );
    }
    return rc;
}

}
