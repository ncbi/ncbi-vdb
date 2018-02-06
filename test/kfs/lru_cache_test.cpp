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
* Unit tests for lru_cache in libs/kfs/poolpages.c
*/

#include <ktst/unit_test.hpp>

#include <klib/out.h>
#include <klib/rc.h>
#include <klib/time.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/rrcachedfile.h>

static size_t rand_32( size_t min, size_t max )
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
                    {
                        rc = RC ( rcRuntime, rcBuffer, rcReading, rcMemory, rcInvalid );
                        KOutMsg( "compare_file_content() requested:%lu, read 1:%lu, 2:%lu", bytes, num_read1, num_read2 );
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

void seed_random_number_generator( void )
{
    KTime_t t = KTimeStamp ();  /* klib/time.h */
    srand( t );
}

TEST_SUITE( LRU_Cache_Test );

#define PAGE_SIZE 128 * 1024
#define PAGE_COUNT 1024 

TEST_CASE( LRU_Cache_Test_Basic )
{
    KOutMsg( "Test: LRU-Cache-Test-Basic\n" );
    
    REQUIRE_RC_FAIL( MakeRRCached ( NULL, NULL, 0, 0 ) );
    REQUIRE_RC_FAIL( MakeRRCached ( NULL, NULL, PAGE_SIZE, PAGE_COUNT ) );
    REQUIRE_RC_FAIL( MakeRRCached ( NULL, NULL, 0, PAGE_COUNT ) );
    REQUIRE_RC_FAIL( MakeRRCached ( NULL, NULL, PAGE_SIZE, 0 ) );
    
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const char * filename = "./LRU_Cache_Test_Basic.txt";
    KFile * file;

    REQUIRE_RC( KDirectoryCreateFile ( dir, &file, false, 0664, kcmInit, filename ) );
    REQUIRE_RC( fill_file_with_random_data( file, PAGE_SIZE * 2 ) );
    KFileRelease( file );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", filename ) );
    
    REQUIRE_RC_FAIL( MakeRRCached ( NULL, org, 0, 0L ) );
    REQUIRE_RC_FAIL( MakeRRCached ( NULL, org, PAGE_SIZE, PAGE_COUNT ) );
    REQUIRE_RC_FAIL( MakeRRCached ( NULL, org, 0, PAGE_COUNT ) );
    REQUIRE_RC_FAIL( MakeRRCached ( NULL, org, PAGE_SIZE, 0 ) );
    
    const KFile * cache;
    REQUIRE_RC_FAIL( MakeRRCached ( &cache, org, 0, 0 ) );
    REQUIRE_RC_FAIL( MakeRRCached ( &cache, org, 0, PAGE_COUNT ) );
    REQUIRE_RC_FAIL( MakeRRCached ( &cache, org, PAGE_SIZE, 0 ) );

    REQUIRE_RC( MakeRRCached ( &cache, org, PAGE_SIZE, PAGE_COUNT ) );
    
    KFileRelease( org );
    KFileRelease( cache );
    KDirectoryRemove ( dir, true, "%s", filename );
    
    REQUIRE_RC( KDirectoryRelease( dir ) );
}

typedef struct events
{
    uint32_t requests, found, enter, discard, failed;
} events;

void on_event( void * data, enum cache_event event, uint64_t pos, size_t len, uint32_t block_nr )
{
    events * ev = ( events * )data;
    if ( ev != NULL )
    {
        switch ( event )
        {
            case CE_REQUEST : ev -> requests++;
                              //KOutMsg( "R %lx.%lx (%u)\n", pos, len, block_nr );
                              break;

            case CE_FOUND   : ev -> found++;
                              //KOutMsg( "F %lx.%lx (%u)\n", pos, len, block_nr );
                              break;

            case CE_ENTER   : ev -> enter++;
                              //KOutMsg( "E %lx.%lx (%u)\n", pos, len, block_nr );
                              break;
                              
            case CE_DISCARD : ev -> discard++;
                              //KOutMsg( "D %lx.%lx (%u)\n", pos, len, block_nr );
                              break;

            case CE_FAILED  : ev -> failed++; break;
        }
    }
}

void print_events( events * events )
{
    KOutMsg( "req     = %u\n", events -> requests );
    KOutMsg( "found   = %u\n", events -> found );
    KOutMsg( "enter   = %u\n", events -> enter );
    KOutMsg( "discard = %u\n", events -> discard );
    KOutMsg( "failed  = %u\n", events -> failed );
}

TEST_CASE( LRU_Cache_Test_Linear_Reading )
{
    KOutMsg( "Test: LRU-Cache-Test-Linear-Reading\n" );
    
	KDirectory * dir;
	REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const char * filename = "./LRU_Cache_Test_Linear_Reading.txt";
    KFile * file;

    REQUIRE_RC( KDirectoryCreateFile ( dir, &file, false, 0664, kcmInit, filename ) );
    REQUIRE_RC( fill_file_with_random_data( file, PAGE_SIZE * 10 ) );
    REQUIRE_RC( KFileRelease( file ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", filename ) );

    const KFile * cache;
    REQUIRE_RC( MakeRRCached ( &cache, org, 64 * 1024, 22 ) );
    
    uint64_t file_size;
    REQUIRE_RC( KFileSize ( cache, &file_size ) );
    REQUIRE( ( file_size == ( PAGE_SIZE * 10 ) ) );

    events events;
    memset( &events, 0, sizeof events );
    REQUIRE_RC( SetRRCachedEventHandler( cache, &events, on_event ) );
   
    REQUIRE_RC( compare_file_content( org, cache, 0, file_size ) );
    REQUIRE_RC( compare_file_content( org, cache, 0, file_size ) );

    REQUIRE_RC( KFileRelease( cache ) );
    REQUIRE_RC( KFileRelease( org ) );

#if _DEBUGGING
    // print_events( &events );
    REQUIRE( events . requests == 21 );
    REQUIRE( events . found == 20 );
    REQUIRE( events . enter == 20 );    
    REQUIRE( events . discard == 0 );
    REQUIRE( events . failed == 0 );
#endif

    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", filename ) );    
    REQUIRE_RC( MakeRRCached ( &cache, org, 64 * 1024, 20 ) );
    memset( &events, 0, sizeof events );
    REQUIRE_RC( SetRRCachedEventHandler( cache, &events, on_event ) );

    REQUIRE_RC( compare_file_content( org, cache, 0, file_size ) );
    REQUIRE_RC( compare_file_content( org, cache, 0, file_size ) );

#if _DEBUGGING
    //print_events( &events );
    REQUIRE( events . requests == 21 );
    REQUIRE( events . found == 20 );
    REQUIRE( events . enter == 20 );    
    REQUIRE( events . discard == 0 );
    REQUIRE( events . failed == 0 );
#endif

    REQUIRE_RC( KFileRelease( cache ) );
    REQUIRE_RC( KFileRelease( org ) );
    REQUIRE_RC( KDirectoryRemove ( dir, true, "%s", filename ) );
	REQUIRE_RC( KDirectoryRelease( dir ) );
}

TEST_CASE( LRU_Cache_Test_Random_Reading )
{
    KOutMsg( "Test: LRU-Cache-Test-Random-Reading\n" );

	KDirectory * dir;
	REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const char * filename = "./LRU_Cache_Test_Random_Reading.txt";    
    KFile * file;

    size_t file_size = PAGE_SIZE * 10;
    REQUIRE_RC( KDirectoryCreateFile ( dir, &file, false, 0664, kcmInit, filename ) );
    REQUIRE_RC( fill_file_with_random_data( file, file_size ) );
    REQUIRE_RC( KFileRelease( file ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", filename ) );

    uint32_t page_count = 20;
    const KFile * cache;
    REQUIRE_RC( MakeRRCached ( &cache, org, 64 * 1024, page_count ) );

    events events;
    memset( &events, 0, sizeof events );
    REQUIRE_RC( SetRRCachedEventHandler( cache, &events, on_event ) );

    uint32_t loops = 100000;
    KOutMsg( "---testing %u loops\n", loops );
    for ( uint32_t i = 0; i < loops; ++i )
    {
        size_t bsize = rand_32( 10, 50000 );
        uint64_t pos = rand_32( 0, file_size - ( bsize + 1 ) );
        rc_t rc = compare_file_content( org, cache, pos, bsize );
        if ( rc != 0 )
            KOutMsg( "Test: LRU-Cache-Test-Random-Reading in loop #%u : %lu.%lu = %R\n", i, pos, bsize, rc );
        REQUIRE_RC( rc );
    }

#if _DEBUGGING
    //print_events( &events );
    REQUIRE( events . requests >= loops );
    REQUIRE( events . found >= loops );
    REQUIRE( events . enter = page_count );    
    REQUIRE( events . failed == 0 );
#endif

    REQUIRE_RC( KFileRelease( cache ) );
    REQUIRE_RC( KFileRelease( org ) );
    REQUIRE_RC( KDirectoryRemove ( dir, true, "%s", filename ) );
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

const char UsageDefaultName[] = "lru-cache-test";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc;
    seed_random_number_generator();
    rc = LRU_Cache_Test( argc, argv );
    KOutMsg( "lru-cache-test : %R\n", rc );
    return rc;
}

}
