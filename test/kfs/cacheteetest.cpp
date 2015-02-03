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
* Unit tests for cacheteefile
*/

#include <cstring>

#include <ktst/unit_test.hpp>

#include <klib/out.h>
#include <klib/rc.h>

#include <kproc/thread.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/cacheteefile.h>

using namespace std;

#define DATAFILE "org.dat"
#define CACHEFILE "cache.dat"
#define CACHEFILE1 "cache.dat.cache"
#define DATAFILESIZE ( ( 1024 * 1024 ) + 300 )

TEST_SUITE( CacheTeeTests );

// this is not what we are testing...

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
			if ( rc == 0 )	pos += num_read;
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

TEST_CASE( CacheTee_Basic )
{
	KOutMsg( "Test: CacheTee_Basic\n" );
	
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

	const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
	
	const KFile * tee;
	REQUIRE_RC( KDirectoryMakeCacheTee ( dir, &tee, org, NULL, 0, 0, false, "%s", CACHEFILE ) );

	REQUIRE_RC( KFileRelease( tee ) );
	REQUIRE_RC( KFileRelease( org ) );
	
	const KFile * cache;
	REQUIRE_RC( KDirectoryOpenFileRead( dir, &cache, "%s.cache", CACHEFILE ) );
	
	bool is_complete;
	REQUIRE_RC( IsCacheFileComplete( cache, &is_complete, true ) );	
	REQUIRE( !is_complete );
	
	float percent;
	uint64_t bytes_in_cache;
	REQUIRE_RC( GetCacheCompleteness( cache, &percent, &bytes_in_cache ) );
	REQUIRE( ( percent == 0.0 ) );
	REQUIRE( ( bytes_in_cache == 0 ) );
	
	REQUIRE_RC( KFileRelease( cache ) );
	REQUIRE_RC( KDirectoryRelease( dir ) );
}                                 


TEST_CASE( CacheTee_Read )
{
	KOutMsg( "Test: CacheTee_Read\n" );
	
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

	const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
	
	const KFile * tee;
	REQUIRE_RC( KDirectoryMakeCacheTee ( dir, &tee, org, NULL, 0, 0, false, "%s", CACHEFILE ) );

	REQUIRE_RC( compare_file_content( org, tee, 0, 100 ) );						// small read at pos zero
	REQUIRE_RC( compare_file_content( org, tee, 10, 100 ) );						// small read at pos 10
	REQUIRE_RC( compare_file_content( org, tee, 1024 * 128, 100 ) );				// small read at block boundary
	REQUIRE_RC( compare_file_content( org, tee, 100, 1024 * 128 + 500 ) );			// large read crossing block boundary
	REQUIRE_RC( compare_file_content( org, tee, 200, 1024 * 128 * 3 ) );			// very large read at pos 200
	REQUIRE_RC( compare_file_content( org, tee, 1024 * 128 * 2 + 10, 100 ) );		// small read after block boundary
	REQUIRE_RC( compare_file_content( org, tee, 1024 * 128 * 2 - 10, 100 ) );		// small read crossing block boundary	
	REQUIRE_RC( compare_file_content( org, tee, DATAFILESIZE - 100, 300 ) );		// small read crossing EOF
	
	REQUIRE_RC( KFileRelease( tee ) );	
	REQUIRE_RC( KFileRelease( org ) );
	REQUIRE_RC( KDirectoryRelease( dir ) );
}


TEST_CASE( CacheTee_Promoting )
{
	KOutMsg( "Test: CacheTee_Promoting\n" );
	
	remove_file( CACHEFILE );	// to start with a clean slate on caching...
	remove_file( CACHEFILE1 );

    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

	const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
	
	const KFile * tee;
	REQUIRE_RC( KDirectoryMakeCacheTee ( dir, &tee, org, NULL, 0, 0, false, "%s", CACHEFILE ) );

	REQUIRE_RC( read_partial( tee, 1024 * 32, DATAFILESIZE / 2 ) );
	REQUIRE_RC( KFileRelease( tee ) );
	
	const KFile * cache;
	REQUIRE_RC( KDirectoryOpenFileRead( dir, &cache, "%s.cache", CACHEFILE ) );

	bool is_complete;
	REQUIRE_RC( IsCacheFileComplete( cache, &is_complete, true ) );	
	REQUIRE( !is_complete );

	float percent;
	REQUIRE_RC( GetCacheCompleteness( cache, &percent, NULL ) );
	REQUIRE( ( percent > 50.0 ) );

	remove_file( CACHEFILE1 );

	REQUIRE_RC( KDirectoryMakeCacheTee ( dir, &tee, org, NULL, 0, 0, false, "%s", CACHEFILE ) );
	REQUIRE_RC( read_all( tee, 1024 * 32 ) );	//	this should trigger the promotion of the cache file from cache.dat.cache to cache.dat
	REQUIRE_RC( KFileRelease( tee ) );	
	
	REQUIRE_RC( KDirectoryOpenFileRead( dir, &cache, "%s", CACHEFILE ) ); // if we can open this file then, it was promoted...
	REQUIRE_RC( KFileRelease( cache ) );
	
	REQUIRE_RC( KFileRelease( org ) );	
	REQUIRE_RC( KDirectoryRelease( dir ) );
}


static rc_t cache_access( int id )
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
			rc = KDirectoryMakeCacheTee ( dir, &tee, org, NULL, 0, 0, false, "%s", CACHEFILE );
			if ( rc == 0 )
			{
				KOutMsg( "Test: Thread #%d\n", id );
				rc = compare_file_content( org, tee, 0, 100 );
				KFileRelease( tee );
			}
			KFileRelease( org );
		}
		KDirectoryRelease( dir );
	}
	return rc;
}


static rc_t CC thread_func( const KThread *self, void *data )
{
	int * id = ( int * ) data;
	return cache_access( * id );
}


TEST_CASE( CacheTee_Multiple_Users )
{
	KOutMsg( "Test: CacheTee_Multiple_Users\n" );
	remove_file( CACHEFILE );	// to start with a clean slate on caching...
	remove_file( CACHEFILE1 );

	REQUIRE_RC( cache_access( 0 ) ); // if this line is commented out: at least one of the threads will fail!
	
	int n = 8;
	KThread *t [ n ];
	int id[ n ];
	rc_t rc = 0;
	for ( int i = 0; i < n && rc == 0; ++i )
	{
		id[ i ] = i + 1;
		rc = KThreadMake ( &( t[ i ] ), thread_func, &( id[ i ] ) );
		REQUIRE_RC( rc );
	}
	
	for ( int i = 0; i < n && rc == 0; ++i )
	{
		rc_t rc_thread;
		rc = KThreadWait ( t[ i ], &rc_thread );
		REQUIRE_RC( rc );
		REQUIRE_RC( rc_thread );
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

const char UsageDefaultName[] = "cachetee-test";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
	rc_t rc = prepare_cachetee_tests();
	if ( rc == 0 )
	{
		rc = CacheTeeTests( argc, argv );
		finish_cachetee_tests();
	}
    return rc;
}

}
