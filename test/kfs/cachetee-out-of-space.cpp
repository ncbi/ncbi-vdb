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

#include <cstring>
#include <cstdlib>

#include <ktst/unit_test.hpp>

#include <klib/out.h>
#include <klib/rc.h>

#include <kfs/defs.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/cacheteefile.h>
#include <kfs/cachetee2file.h>
#include <kfs/cachetee3file.h>

using namespace std;

#define DATAFILE "org.dat"
#define DATAFILESIZE ( 1024 * 12 )
#define CACHEFILE "mount_point/cache.dat"
#define BLOCKSIZE 128

TEST_SUITE( CacheTeeOutOfSpaceSuite );

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

static rc_t create_random_file( KDirectory * dir, const char * filename, uint64_t file_size )
{
    KFile * file;
    rc_t rc = KDirectoryCreateFile ( dir, &file, false, 0664, kcmInit, filename );
    if ( rc == 0 )
    {
        if ( rc == 0 )
            rc = fill_file_with_random_data( file, file_size );
        KFileRelease( file );
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
			if ( rc == 0 )
                pos += num_read;
            else
            {
                KOutMsg( "KFileReadAll( pos: %ld, block_size: %ld ) -> num_read: %ld, %R\n",
                         pos, block_size, num_read, rc );
            }
		}
        free( buffer );
        rc = 0;
    }
    return rc;
}


static rc_t remove_file( KDirectory * dir, const char * filename )
{
    return KDirectoryRemove ( dir, true, "%s", filename );
}


//////////////////////////////////////////// Test-cases

TEST_CASE( CacheTee_out_of_space )
{
	KOutMsg( "Test: CacheTee_out_of_space\n" );
	
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    // we are creating a file filled with random data, to stand in as the remote file
    // which has to be cached...
    REQUIRE_RC( create_random_file( dir, DATAFILE, DATAFILESIZE ) );

    // we open this file with random-data, created above
	const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
	
    // we wrap this data-file into a cache-file
	const KFile * tee;
	REQUIRE_RC( KDirectoryMakeCacheTee ( dir, &tee, org, BLOCKSIZE, "%s", CACHEFILE ) );

    // now we can release the original data-file, the tee-file holds a reference to it...
	REQUIRE_RC( KFileRelease( org ) );
	
    // this is the part that should not fail: we are reading the whole content of the tee-file.
    // because we have created the tee-file in a directory that has a quota...
    // the tee-file cannot grow to the necessary size, it should internally switch into
    // a passtrough-mode and just read the org-file instead of trying to cache...
    REQUIRE_RC( read_all( tee, ( 1024 * 7 ) ) );
    
    // we clean up, by releasing the tee-file, and removing all the temp. files we created
	REQUIRE_RC( KFileRelease( tee ) );
    REQUIRE_RC( remove_file( dir, CACHEFILE ) );
    REQUIRE_RC( remove_file( dir, DATAFILE ) );
	REQUIRE_RC( KDirectoryRelease( dir ) );
}                                 

TEST_CASE( CacheTee_v2_out_of_space )
{
	KOutMsg( "Test: CacheTee_v2_out_of_space\n" );
	
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    // we are creating a file filled with random data, to stand in as the remote file
    // which has to be cached...
    REQUIRE_RC( create_random_file( dir, DATAFILE, DATAFILESIZE ) );

    // we open this file with random-data, created above
	const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
	
    // we wrap this data-file into a cache-file
	const KFile * tee;
    REQUIRE_RC( KDirectoryMakeCacheTee2 ( dir, &tee, org, BLOCKSIZE, "%s", CACHEFILE ) );
    
    // now we can release the original data-file, the tee-file holds a reference to it...
	REQUIRE_RC( KFileRelease( org ) );
	
    // this is the part that should not fail: we are reading the whole content of the tee-file.
    // because we have created the tee-file in a directory that has a quota...
    // the tee-file cannot grow to the necessary size, it should internally switch into
    // a passtrough-mode and just read the org-file instead of trying to cache...
    REQUIRE_RC( read_all( tee, ( 1024 * 7 ) ) );
    
    // we clean up, by releasing the tee-file, and removing all the temp. files we created
	REQUIRE_RC( KFileRelease( tee ) );
    REQUIRE_RC( remove_file( dir, CACHEFILE ) );
    REQUIRE_RC( remove_file( dir, DATAFILE ) );
	REQUIRE_RC( KDirectoryRelease( dir ) );
}                                 

TEST_CASE( CacheTee_v3_out_of_space )
{
	KOutMsg( "Test: CacheTee_v3_out_of_space\n" );
	
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    // we are creating a file filled with random data, to stand in as the remote file
    // which has to be cached...
    REQUIRE_RC( create_random_file( dir, DATAFILE, DATAFILESIZE ) );

    // we open this file with random-data, created above
	const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
	
    // we wrap this data-file into a cache-file
	const KFile * tee;
    REQUIRE_RC( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE, 2, 0, false, false, "%s", CACHEFILE ) );
    
    // now we can release the original data-file, the tee-file holds a reference to it...
	REQUIRE_RC( KFileRelease( org ) );
	
    // this is the part that should not fail: we are reading the whole content of the tee-file.
    // because we have created the tee-file in a directory that has a quota...
    // the tee-file cannot grow to the necessary size, it should internally switch into
    // a passtrough-mode and just read the org-file instead of trying to cache...
    REQUIRE_RC( read_all( tee, ( 1024 * 7 ) ) );
    
    // we clean up, by releasing the tee-file, and removing all the temp. files we created
	REQUIRE_RC( KFileRelease( tee ) );
    REQUIRE_RC( remove_file( dir, CACHEFILE ) );
    REQUIRE_RC( remove_file( dir, DATAFILE ) );
	REQUIRE_RC( KDirectoryRelease( dir ) );
}                                 

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

    ver_t CC KAppVersion ( void ) { return 0x1000000; }
    rc_t CC UsageSummary ( const char * progname ) { return 0; }
    rc_t CC Usage ( const Args * args ) { return 0; }
    const char UsageDefaultName[] = "test-cachetee-out-of-space";

    rc_t CC KMain ( int argc, char *argv [] )
    {
        KConfigDisableUserSettings();
        return CacheTeeOutOfSpaceSuite( argc, argv );
    }

}

