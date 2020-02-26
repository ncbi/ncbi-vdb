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

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <locale>
#include <mutex>
#include <thread>
#include <chrono>

#include <ktst/unit_test.hpp>

#include <klib/out.h>
#include <klib/rc.h>
#include <klib/time.h>
#include <klib/printf.h>

#include <kproc/thread.h>

#include <kfs/cachetee3file.h>
#include <kfs/defs.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/recorder.h>

#include <vfs/manager.h>
#include <vfs/path.h>

using namespace std;

#define DATAFILE "org.dat"
#define CACHEFILE "cache.dat"
#define CACHEFILE1 "cache.dat.cache"
#define DATAFILESIZE ( ( 1024 * 1024 * 64 ) + 300 )
#define BLOCKSIZE ( 1024 * 16 )

TEST_SUITE ( CacheTee3Tests );

class CT3Fixture {
public:
    CT3Fixture ()
    {
        create_random_file ( DATAFILE, DATAFILESIZE );
        remove_file ( CACHEFILE );
        remove_file ( CACHEFILE1 );
    }

    ~CT3Fixture ()
    {
        remove_file ( DATAFILE );
        remove_file ( CACHEFILE );
        remove_file ( CACHEFILE1 );
    }

    uint32_t rand_32 ( uint32_t min, uint32_t max )
    {
        double scaled = ( (double)rand () / RAND_MAX );
        return ( ( max - min + 1 ) * scaled ) + min;
    }

    rc_t fill_file_with_random_data ( KFile *file, size_t file_size )
    {
        rc_t rc = KFileSetSize ( file, file_size );
        if ( rc == 0 )
        {
            uint64_t pos = 0;
            size_t total = 0;
            while ( rc == 0 && total < file_size )
            {
                uint32_t data[512];
                uint32_t i;
                size_t to_write, num_writ;

                for ( i = 0; i < 512; ++i )
                    data[i] = rand_32 ( 0, 0xFFFFFFFF - 1 );
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

    rc_t create_random_file ( const char *filename, uint64_t file_size )
    {
        KDirectory *dir;
        rc_t rc = KDirectoryNativeDir ( &dir );
        if ( rc == 0 )
        {
            KFile *file;
            rc = KDirectoryCreateFile (
                dir, &file, false, 0664, kcmInit, filename );
            if ( rc == 0 )
            {
                if ( rc == 0 )
                    rc = fill_file_with_random_data ( file, file_size );
                KFileRelease ( file );
            }
            KDirectoryRelease ( dir );
        }
        return rc;
    }

    rc_t remove_file ( const char *filename )
    {
        KDirectory *dir;
        rc_t rc = KDirectoryNativeDir ( &dir );
        if ( rc == 0 )
        {
            rc = KDirectoryRemove ( dir, true, "%s", filename );
            KDirectoryRelease ( dir );
        }
        return rc;
    }

    bool file_exists( const char * filename )
    {
        bool res = false;
        KDirectory *dir;
        rc_t rc = KDirectoryNativeDir ( &dir );
        if ( rc == 0 )
        {
            uint32_t pt = KDirectoryPathType ( dir, "%s", filename );
            res = ( pt == kptFile );
            KDirectoryRelease ( dir );
        }
        return res;
    }
    
    void report_diff ( uint8_t *b1, uint8_t *b2, size_t n, uint32_t max_diffs )
    {
        size_t i, d;
        for ( i = 0, d = 0; i < n && d < max_diffs; ++i )
        {
            if ( b1[i] != b2[i] )
            {
                KOutMsg ( "[ %.08d ] %.02X %.02X\n", i, b1[i], b2[i] );
                d++;
            }
        }
    }

    rc_t compare_file_content_1 ( const KFile *file1, const KFile *file2,
                                  uint64_t pos, size_t num_bytes, const char *msg )
    {
        rc_t rc = 0;
        uint8_t *buffer1 = (uint8_t *)malloc ( num_bytes );
        if ( msg != NULL )
            KOutMsg ( "Test: KFileReadAll ( pos %lu, %u bytes, '%s' )\n", pos, num_bytes, msg );
        if ( buffer1 == NULL )
        {
            rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
            KOutMsg ( "Test: cannot make buffer1 of size %u\n", num_bytes );
        }
        else
        {
            uint8_t *buffer2 = (uint8_t *)malloc ( num_bytes );
            if ( buffer2 == NULL )
            {
                rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
                KOutMsg ( "Test: cannot make buffer2 of size %u\n", num_bytes );
            }
            else
            {
                size_t num_read1;
                rc = KFileReadAll ( file1, pos, buffer1, num_bytes, &num_read1 );
                if ( rc != 0 )
                    KOutMsg ( "Test: KFileReadAll( 1 ) -> %R\n", rc );
                else
                {
                    size_t num_read2;
                    rc = KFileReadAll ( file2, pos, buffer2, num_bytes, &num_read2 );
                    if ( rc != 0 )
                        KOutMsg ( "Test: KFileReadAll( 2 ) -> %R\n", rc );
                    else
                    {
                        if ( num_read1 != num_read2 )
                        {
                            rc = RC ( rcRuntime, rcBuffer, rcReading, rcMemory, rcInvalid );
                            KOutMsg ( "Test %d vs %d\n", num_read1, num_read2 );
                        }
                        else
                        {
                            int diff = memcmp ( buffer1, buffer2, num_read1 );
                            if ( diff != 0 )
                            {
                                report_diff ( buffer1, buffer2, num_read1, 20 );
                                rc = RC ( rcRuntime, rcBuffer, rcReading, rcMemory, rcCorrupt );
                            }
                        }
                    }
                }
                free ( buffer2 );
            }
            free ( buffer1 );
        }
        return rc;
    }

    rc_t compare_file_content_2 ( const KFile *file1, const KFile *file2,
                                  uint64_t pos, size_t num_bytes, const char *msg )
    {
        rc_t rc = 0;
        uint8_t *buffer1 = (uint8_t *)malloc ( num_bytes );
        if ( msg != NULL )
            KOutMsg ( "Test: KFileReadExactly ( pos %lu, %u bytes, '%s' )\n", pos, num_bytes, msg );
        if ( buffer1 == NULL )
        {
            rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
            KOutMsg ( "Test: cannot make buffer1 of size %u\n", num_bytes );
        }
        else
        {
            uint8_t *buffer2 = (uint8_t *)malloc ( num_bytes );
            if ( buffer2 == NULL )
            {
                rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
                KOutMsg ( "Test: cannot make buffer2 of size %u\n", num_bytes );
            }
            else
            {
                rc = KFileReadExactly ( file1, pos, buffer1, num_bytes );
                if ( rc != 0 )
                    KOutMsg ( "Test: KFileReadExactly( 1 ) -> %R\n", rc );
                else
                {
                    rc = KFileReadExactly ( file2, pos, buffer2, num_bytes );
                    if ( rc != 0 )
                        KOutMsg ( "Test: KFileReadExactly( 2 ) -> %R\n", rc );
                    else
                    {
                        int diff = memcmp ( buffer1, buffer2, num_bytes );
                        if ( diff != 0 )
                        {
                            report_diff ( buffer1, buffer2, num_bytes, 20 );
                            rc = RC ( rcRuntime, rcBuffer, rcReading, rcMemory, rcCorrupt );
                        }
                    }
                }
                free ( buffer2 );
            }
            free ( buffer1 );
        }
        return rc;
    }

    rc_t read_all_loop ( const KFile *f, uint64_t pos, uint8_t *buffer,
                         size_t to_read, size_t *NumRead )
    {
        rc_t rc = 0;
        size_t num_read_total = 0;
        uint8_t *dst = buffer;
        while ( rc == 0 && num_read_total < to_read )
        {
            size_t num_read, n = to_read - num_read_total;
            rc = KFileRead ( f, pos, dst, n, &num_read );
            if ( rc != 0 )
            {
                KOutMsg ( "Test: KFileRead( at %lu, %u bytes ) -> %R\n", pos, n, rc );
                if ( rcExhausted == GetRCState ( rc ) &&
                     (enum RCObject)rcTimeout == GetRCObject ( rc ) )
                {
                    KSleepMs ( 50 );
                    rc = 0;
                }
            }
            else
            {
                num_read_total += num_read;
                pos += num_read;
                dst += num_read;

                *NumRead = num_read_total;
                if ( num_read == 0 ) { break; }
            }
        }
        return rc;
    }

    rc_t compare_file_content_3 ( const KFile *file1, const KFile *file2,
                                  uint64_t pos, size_t num_bytes, const char *msg )
    {
        rc_t rc = 0;
        uint8_t *buffer1 = (uint8_t *)malloc ( num_bytes );
        if ( msg != NULL )
            KOutMsg ( "Test: read_all_loop( pos %lu, %u bytes, '%s' )\n", pos, num_bytes, msg );
        if ( buffer1 == NULL )
        {
            rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
            KOutMsg ( "Test: cannot make buffer1 of size %u\n", num_bytes );
        }
        else
        {
            uint8_t *buffer2 = (uint8_t *)malloc ( num_bytes );
            if ( buffer2 == NULL )
            {
                rc = RC ( rcRuntime, rcBuffer, rcConstructing, rcMemory, rcExhausted );
                KOutMsg ( "Test: cannot make buffer2 of size %u\n", num_bytes );
            }
            else
            {
                size_t NumRead1;
                rc = read_all_loop ( file1, pos, buffer1, num_bytes, &NumRead1 );
                if ( rc == 0 )
                {
                    size_t NumRead2;
                    rc = read_all_loop ( file2, pos, buffer2, num_bytes, &NumRead2 );
                    if ( rc == 0 )
                    {
                        if ( NumRead1 != NumRead2 )
                        {
                            rc = RC ( rcRuntime, rcBuffer, rcRetrieving, rcItem, rcInvalid );
                        }
                        if ( rc == 0 )
                        {
                            if ( NumRead1 != 0 )
                            {
                                int diff = memcmp ( buffer1, buffer2, NumRead1 );
                                if ( diff != 0 )
                                {
                                    report_diff ( buffer1, buffer2, NumRead1, 20 );
                                    rc = RC ( rcExe, rcBuffer, rcReading, rcMemory, rcCorrupt );
                                }
                            }
                        }
                    }
                }
                free ( buffer2 );
            }
            free ( buffer1 );
        }
        return rc;
    }
    
    static rc_t read_whole_file( const KFile * src, size_t block_size )
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
            }
            free( buffer );
        }
        return rc;
    }
    

}; // CT3Fixture

//////////////////////////////////////////// Test-cases

FIXTURE_TEST_CASE( CacheTee3_Basic_1, CT3Fixture )
{
    KOutMsg( "Test: CacheTee3_Basic-1 (cache-file based)\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );
    
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
    
    const KFile * tee;
    uint32_t cluster_factor = 0;
    uint32_t ram_pages = 0;
    REQUIRE_RC( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                                          cluster_factor, ram_pages, false, false,
                                          "%s", CACHEFILE ) );
    REQUIRE_RC( KFileRelease( org ) );

    REQUIRE( KFileIsKCacheTeeFile_v3( tee ) );
    uint64_t org1;
    REQUIRE_RC( CacheTee3FileGetOriginalSize ( tee, &org1 ) );
    
    REQUIRE_RC( KFileRelease( tee ) );
    
    const KFile * cache;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &cache, "%s.cache", CACHEFILE ) );
    
    KOutMsg( "Test: CacheTee3 file opened\n" );

    bool is_valid;
    REQUIRE_RC( CacheTee3FileIsValid( cache, &is_valid ) );    
    REQUIRE( is_valid );

    bool is_complete;
    REQUIRE_RC( CacheTee3FileIsComplete( cache, &is_complete ) );
    REQUIRE( !is_complete );

    REQUIRE( !KFileIsKCacheTeeFile_v3( cache ) );

    uint64_t org2;
    REQUIRE_RC( CacheTee3FileGetOriginalSize ( cache, &org2 ) );
    REQUIRE( org1 == org2 );
    
    double percent;
    uint64_t bytes_in_cache;
    REQUIRE_RC( CacheTee3FileGetCompleteness( cache, &percent, &bytes_in_cache ) );
    REQUIRE( ( percent == 0.0 ) );
    REQUIRE( ( bytes_in_cache == 0 ) );

    REQUIRE_RC( KFileRelease( cache ) );
    KOutMsg( "Test: CacheTee3 file closed\n" );
    
    REQUIRE_RC( KDirectoryRelease( dir ) );
}                                 

FIXTURE_TEST_CASE( CacheTee3_Basic_2, CT3Fixture )
{
    KOutMsg( "Test: CacheTee3_Basic-2(no cache-file only RAM)\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );

    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
    
    const KFile * tee;
    uint32_t cluster_factor = 0;
    uint32_t ram_pages = 0;
    REQUIRE_RC( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                              cluster_factor, ram_pages, false, false, NULL ) );
    /* creating a CacheTeeFile_v3 with no file-backing and no RAM, returns the
       org-file with one additional add-ref, because of that tee is not an CacheTeeFile_v3 */
    REQUIRE( !KFileIsKCacheTeeFile_v3( tee ) );
    REQUIRE_RC( KFileRelease( tee ) );
    
    ram_pages = 100;
    REQUIRE_RC( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                              cluster_factor, ram_pages, false, false, NULL ) );
    REQUIRE( KFileIsKCacheTeeFile_v3( tee ) );
    REQUIRE_RC( KFileRelease( tee ) );

    REQUIRE_RC( KFileRelease( org ) );    
    REQUIRE_RC( KDirectoryRelease( dir ) );
}                                 

FIXTURE_TEST_CASE( CacheTee3_Read_1, CT3Fixture )
{
    KOutMsg( "Test: CacheTee3_Read-1 ( backed by file )\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );
            
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
    
    const KFile * tee;
    uint32_t cluster_factor = 0;
    uint32_t ram_pages = 0;
    REQUIRE_RC( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                                cluster_factor, ram_pages, false, false, "%s", CACHEFILE ) );

    double percent;
    uint64_t bytes_in_cache;
                                          
    uint64_t at = 0;
    size_t len = 100;

    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read at pos zero, not yet chached" ) );
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read at pos zero, now cached" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );
    
    at = 10;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read at pos 10, cached" ) );
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "again, cached" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );
    
    at = 1024L * BLOCKSIZE;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read at block boundary, not yet cached" ) );
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "again, now cached" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );
    
    at = BLOCKSIZE / 2; len = BLOCKSIZE;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "spans 2 blocks, not yet cached" ) );
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "again, now cached" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );
    
    len = BLOCKSIZE * 5 + 100;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "spans 5 blocks, partly cached" ) );
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "again, now cached" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );

    at = 100; len = 1024 * BLOCKSIZE + 500;
    REQUIRE_RC( compare_file_content_3( org, tee, at, len, "large read crossing block boundary, partly cached" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );

    at = 200; len = 2048 * BLOCKSIZE;
    REQUIRE_RC( compare_file_content_3( org, tee, at, len, "very large read, partly cached" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );

    at = 1024 * BLOCKSIZE * 2 + 10; len = 100;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read after block boundary" ) );
    at = 1024 * BLOCKSIZE * 2 - 10;
    REQUIRE_RC( compare_file_content_2( org, tee, at, len, "small read crossing block boundary" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );

    at = DATAFILESIZE - 100; len = 300;
    REQUIRE_RC( compare_file_content_1( org, tee, at, len, "small read crossing EOF" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );
    
    len = BLOCKSIZE * 10;
    REQUIRE_RC( compare_file_content_1( org, tee, at, len, "large read crossing EOF" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );

    at = DATAFILESIZE - 10000; len = 10000;
    REQUIRE_RC( compare_file_content_1( org, tee, at, len, "large read at EOF" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );

    at = DATAFILESIZE + 100; len = 100;
    REQUIRE_RC( compare_file_content_1( org, tee, at, len, "beyond EOF" ) );
    REQUIRE_RC( CacheTee3FileGetCompleteness( tee, &percent, &bytes_in_cache ) );
    KOutMsg ( "\tcache: %f%% = %lu bytes\n", percent, bytes_in_cache );

    REQUIRE_RC( KFileRelease( tee ) );    
    REQUIRE_RC( KFileRelease( org ) );
    REQUIRE_RC( KDirectoryRelease( dir ) );
}

FIXTURE_TEST_CASE( CacheTee3_Read_2, CT3Fixture )
{
    KOutMsg( "Test: CacheTee3_Read-2 ( backed by RAM )\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );
            
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
    
    const KFile * tee;
    uint32_t cluster_factor = 0;
    uint32_t ram_pages = 100;
    REQUIRE_RC( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                                cluster_factor, ram_pages, false, false, NULL ) );

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

FIXTURE_TEST_CASE( CacheTee3_Read_3, CT3Fixture )
{
    KOutMsg( "Test: CacheTee3_Read-3 ( backed by RAM and cache-file )\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );
            
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
    
    const KFile * tee;
    uint32_t cluster_factor = 0;
    uint32_t ram_pages = 100;
    REQUIRE_RC( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                                cluster_factor, ram_pages, false, false,
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

/* ------------------- CacheTee2_Multiple_Users_Multiple_Inst -------------------------------- */

static rc_t cache_access ( CT3Fixture *fixture, int tid, const KFile *origfile,
                           const KFile *cacheteefile, uint32_t min_len, uint32_t max_len )
{
    rc_t rc = 0;
    const int num_chunks = 32;
    uint64_t chunk_pos[ num_chunks ];
    size_t   chunk_len[ num_chunks ];

    for ( int i = 0; i < num_chunks; ++i )
    {
        chunk_pos[ i ] = fixture->rand_32 ( 0, DATAFILESIZE );
        chunk_len[ i ] = fixture->rand_32 ( min_len, max_len );
    }

    for ( int i = 0; i < num_chunks; ++i )
    {
        /* KOutMsg ( "THREAD #%d / CHUNK #%d (%d.%d)\n", tid, i, chunk_pos[ i ], chunk_len[ i ] ); */
        rc = fixture->compare_file_content_3 ( origfile, cacheteefile, chunk_pos[ i ], chunk_len[ i ], NULL );
        if ( rc != 0 )
        {
            KOutMsg ( "THREAD #%d / CHUNK #%d compare fail\n", tid, i );
            break;
        }
    }
    return rc;
}

struct ThreadData
{
    int tid;
    const KFile *origfile;     // optional
    const KFile *cacheteefile; // optional
    const char * cache_file;
    uint32_t ram_pages;
    CT3Fixture *fixture;
};

static rc_t CC thread_func ( const KThread *self, void *data )
{
    ThreadData *td = (ThreadData *)data;
    if ( td -> cacheteefile == NULL || td -> origfile == NULL )
    {
        KDirectory *dir;
        rc_t rc = KDirectoryNativeDir ( &dir );
        if ( rc == 0 )
        {
            const KFile *org;
            rc = KDirectoryOpenFileRead ( dir, &org, "%s", DATAFILE );
            if ( rc == 0 )
            {
                const KFile *tee;
                uint32_t cluster_factor = 2;
                if ( td -> cache_file != NULL )
                    rc = KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                            cluster_factor, td -> ram_pages, false, false, "%s", td -> cache_file );
                else
                    rc = KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
                            cluster_factor, td -> ram_pages, false, false, NULL );
                if ( rc == 0 )
                {
                    /* make random requests in the range of 100 bytes to DATAFILESIZE / 64 */
                    rc = cache_access ( td->fixture, td->tid, org, tee, 100, DATAFILESIZE / 64 );

                    /* make random requests in the range of 100 bytes to 1024 */
                    if ( rc == 0 )
                        rc = cache_access ( td->fixture, td->tid, org, tee, 10, 1024 );
                    
                    KFileRelease ( tee );
                }
                KFileRelease ( org );
            }
            KDirectoryRelease ( dir );
        }
        return rc;
    }
    return 0;
}

const int num_threads = 32;

FIXTURE_TEST_CASE ( CacheTee3_Multiple_Users_Multiple_Inst_1, CT3Fixture )
{
    KOutMsg ( "Test: CacheTee3_Multiple_Users_Multiple_Inst-1 ( cach-file backed )\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );

    KThread *t[ num_threads ];
    ThreadData td[ num_threads ];
    for ( int i = 0; i < num_threads; ++i )
    {
        td[ i ].tid = i + 1;
        td[ i ].origfile = NULL;
        td[ i ].cacheteefile = NULL;
        td[ i ].cache_file = CACHEFILE;
        td[ i ].ram_pages = 0;
        td[ i ].fixture = this;
        REQUIRE_RC ( KThreadMake ( &( t[ i ] ), thread_func, &( td[ i ] ) ) );
    }

    for ( int i = 0; i < num_threads; ++i )
    {
        rc_t rc_thread;
        REQUIRE_RC ( KThreadWait ( t[ i ], &rc_thread ) );
        REQUIRE_RC ( rc_thread );
        REQUIRE_RC ( KThreadRelease ( t[ i ] ) );
    }
}

FIXTURE_TEST_CASE ( CacheTee3_Multiple_Users_Multiple_Inst_2, CT3Fixture )
{
    KOutMsg ( "Test: CacheTee3_Multiple_Users_Multiple_Inst-2 ( RAM backed )\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );

    KThread *t[ num_threads ];
    ThreadData td[ num_threads ];
    for ( int i = 0; i < num_threads; ++i )
    {
        td[ i ].tid = i + 1;
        td[ i ].origfile = NULL;
        td[ i ].cacheteefile = NULL;
        td[ i ].cache_file = NULL;
        td[ i ].ram_pages = 1000;
        td[ i ].fixture = this;
        REQUIRE_RC ( KThreadMake ( &( t[ i ] ), thread_func, &( td[ i ] ) ) );
    }

    for ( int i = 0; i < num_threads; ++i )
    {
        rc_t rc_thread;
        REQUIRE_RC ( KThreadWait ( t[ i ], &rc_thread ) );
        REQUIRE_RC ( rc_thread );
        REQUIRE_RC ( KThreadRelease ( t[ i ] ) );
    }
}

FIXTURE_TEST_CASE ( CacheTee3_Multiple_Users_Multiple_Inst_3, CT3Fixture )
{
    KOutMsg ( "Test: CacheTee3_Multiple_Users_Multiple_Inst-3 ( cach_file and RAM backed )\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );

    KThread *t[ num_threads ];
    ThreadData td[ num_threads ];
    for ( int i = 0; i < num_threads; ++i )
    {
        td[ i ].tid = i + 1;
        td[ i ].origfile = NULL;
        td[ i ].cacheteefile = NULL;
        td[ i ].cache_file = CACHEFILE;
        td[ i ].ram_pages = 1000;
        td[ i ].fixture = this;
        REQUIRE_RC ( KThreadMake ( &( t[ i ] ), thread_func, &( td[ i ] ) ) );
    }

    for ( int i = 0; i < num_threads; ++i )
    {
        rc_t rc_thread;
        REQUIRE_RC ( KThreadWait ( t[ i ], &rc_thread ) );
        REQUIRE_RC ( rc_thread );
        REQUIRE_RC ( KThreadRelease ( t[ i ] ) );
    }
}

FIXTURE_TEST_CASE ( CacheTee3_conflict_args, CT3Fixture )
{
    KOutMsg ( "test for conflicting args: try-promote-on-close and remove-on-close \n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );
    
    KDirectory *dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &dir ) );

    KFile *org;
    REQUIRE_RC ( KDirectoryOpenFileWrite ( dir, &org, true, "%s", DATAFILE ) );

    const KFile *tee;
    uint32_t cluster_factor = 0;
    uint32_t ram_pages = 0;
    REQUIRE_RC_FAIL ( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org,
        BLOCKSIZE, cluster_factor, ram_pages, true, true, "%s", CACHEFILE ) );

    KFileRelease ( tee );
    KFileRelease ( org );
    KDirectoryRelease ( dir );
}


FIXTURE_TEST_CASE ( CacheTee3_promotion, CT3Fixture )
{
    KOutMsg ( "promotion test\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );

    KDirectory *dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &dir ) );
    KFile *org;
    REQUIRE_RC ( KDirectoryOpenFileWrite ( dir, &org, true, "%s", DATAFILE ) ); // org.data

    const KFile *tee;
    uint32_t cluster_factor = 2;
    uint32_t ram_pages = 0;
    REQUIRE_RC ( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
        cluster_factor, ram_pages, true, false, "%s", CACHEFILE ) ); // cache.dat
    REQUIRE_RC ( read_whole_file( tee, 1024 * 1024 ) );

    bool is_complete;
    REQUIRE_RC( CacheTee3FileIsComplete( tee, &is_complete ) );
    REQUIRE( is_complete );
    
    REQUIRE( file_exists( CACHEFILE1 ) );
    REQUIRE( !file_exists( CACHEFILE ) );
    KFileRelease ( tee );
    /* after releasing the tee-file there should be no .cache -file*/
    REQUIRE( !file_exists( CACHEFILE1 ) );
    REQUIRE( file_exists( CACHEFILE ) );
    
    KFileRelease ( org );
    KDirectoryRelease ( dir );
    KOutMsg ( "promotion test done\n" );    
}

FIXTURE_TEST_CASE ( CacheTee3_delete_on_exit, CT3Fixture )
{
    KOutMsg ( "delete-on-exit test\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );

    KDirectory *dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &dir ) );
    KFile *org;
    REQUIRE_RC ( KDirectoryOpenFileWrite ( dir, &org, true, "%s", DATAFILE ) ); // org.data

    const KFile *tee;
    uint32_t cluster_factor = 2;
    uint32_t ram_pages = 0;
    REQUIRE_RC ( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org, BLOCKSIZE,
        cluster_factor, ram_pages, false, true, "%s", CACHEFILE ) ); // cache.dat

    bool is_complete;
    REQUIRE_RC( CacheTee3FileIsComplete( tee, &is_complete ) );
    REQUIRE( !is_complete );

#if WINDOWS    
    REQUIRE( file_exists( CACHEFILE1 ) );
#else
    REQUIRE( !file_exists( CACHEFILE1 ) ); // this is NOT a requirement
#endif
    REQUIRE( !file_exists( CACHEFILE ) );
    KFileRelease ( tee );
    /* after releasing the tee-file there should be no .cache -file*/
    REQUIRE( !file_exists( CACHEFILE1 ) ); // this is actually the ONLY requirement
    REQUIRE( !file_exists( CACHEFILE ) );
    
    KFileRelease ( org );
    KDirectoryRelease ( dir );
    KOutMsg ( "delete-on-exit done\n" );
}

extern "C" {
/* prototype, impl is in callback_file.c */
rc_t CC MakeCallBackFile ( struct KFile **callback_file,
        struct KFile *to_wrap,
        void ( CC * cb ) ( char event, rc_t rc, uint64_t pos, size_t req_size, size_t done_size,
                           void *data1, void *data2 ),
        void * data1,
        void * data2 );
}

struct CallBackCounter
{
    uint64_t events;
    uint64_t bytes;
    uint64_t pos;
    
    CallBackCounter( void ) : events( 0 ), bytes( 0 ), pos( 0 ) {}
    void count( uint64_t a_pos, uint64_t n ) { events++; bytes += n; pos = a_pos; }
    void clear( void ) { events = 0; bytes = 0; pos = 0; }
};

void CC count_events( char event, rc_t rc, uint64_t pos, size_t req_size, size_t done_size,
                      void *data1, void *data2 )
{
    CallBackCounter * cc = ( CallBackCounter * )data1;
    if ( cc != NULL )
    {
        switch( event )
        {
            case 'R' :
            case 'B' :
            case 'D' :
            case 'F' : cc -> count( pos, done_size ); break;
        }
    }
}

FIXTURE_TEST_CASE ( CacheTee3_request_count, CT3Fixture )
{
    KOutMsg ( "counting requests\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );

    KDirectory *dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &dir ) );
    const KFile *org1;
    REQUIRE_RC ( KDirectoryOpenFileRead ( dir, &org1, "%s", DATAFILE ) ); // org.data

    const KFile *org2;
    REQUIRE_RC ( KDirectoryOpenFileRead ( dir, &org2, "%s", DATAFILE ) ); // org.data

    CallBackCounter cbc1;
    CallBackCounter cbc2;    
    KFile *org1_counted;
    REQUIRE_RC ( MakeCallBackFile ( &org1_counted, (KFile *)org1, count_events, &cbc1, NULL ) );
    KFileRelease ( org1 );
    
    const KFile *tee;
    uint32_t cluster_factor = 2;
    uint32_t ram_pages = 0;
    REQUIRE_RC ( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org1_counted, BLOCKSIZE,
        cluster_factor, ram_pages, false, false, "%s", CACHEFILE ) ); // cache.dat
    KFileRelease ( org1_counted );
    
    KFile *tee_counted;
    REQUIRE_RC ( MakeCallBackFile ( &tee_counted, (KFile *)tee, count_events, &cbc2, NULL ) );
    KFileRelease ( tee );
    
    const int num_chunks = 2048;
    for ( int i = 0; i < num_chunks; ++i )
    {
        uint64_t pos = rand_32 ( 0, DATAFILESIZE );
        size_t   len = rand_32 ( 10, 1000 );
        REQUIRE_RC( compare_file_content_3 ( org2, tee_counted, pos, len, NULL ) );
    }

    for ( int i = 0; i < num_chunks; ++i )
    {
        uint64_t pos = rand_32 ( 0, DATAFILESIZE );
        size_t   len = rand_32 ( 10, 10000 );
        REQUIRE_RC( compare_file_content_3 ( org2, tee_counted, pos, len, NULL ) );
    }
    
    KOutMsg ( "requested  = %,lu, bytes = %,lu\n", cbc2 . events, cbc2 . bytes );
    KOutMsg ( "orig. file = %,lu, bytes = %,lu\n", cbc1 . events, cbc1 . bytes );
    REQUIRE( cbc1 . events < cbc2 . events );
    
    // after reading the whole thing - no read-request should be made via orig...
    KOutMsg ( "requesting whole file\n" );
    REQUIRE_RC ( read_whole_file( tee_counted, 1024 * 1024 ) );

    /* to make shure that all outstanding requests on the background-thread are done */
    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );    

    cbc1.clear();
    cbc2.clear();
    
    for ( int i = 0; i < num_chunks; ++i )
    {
        size_t   len = rand_32 ( 10, 20000 );        
        uint64_t pos = rand_32 ( 0, DATAFILESIZE );
        REQUIRE_RC( compare_file_content_3 ( org2, tee_counted, pos, len, NULL ) );
    }
    REQUIRE_RC( compare_file_content_3 ( org2, tee_counted, DATAFILESIZE - 1000, 2000, NULL ) );
    
    KOutMsg ( "requested  = %,lu, bytes = %,lu\n", cbc2 . events, cbc2 . bytes );
    KOutMsg ( "orig. file = %,lu, bytes = %,lu\n", cbc1 . events, cbc1 . bytes );

    REQUIRE( cbc1 . events == 0 );
    REQUIRE( cbc1 . bytes == 0 );
    
    KFileRelease ( tee_counted );
    KFileRelease ( org2 );
    
    KDirectoryRelease ( dir );
    KOutMsg ( "counting requests done\n" );
}

FIXTURE_TEST_CASE ( CacheTee3_invalid_path, CT3Fixture )
{
    KOutMsg ( "invalid path\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );

#if WINDOWS
    const char * InvalidPath = "\\sra\\invalid.sra";
#else
    const char * InvalidPath = "/sra/invalid.sra";
#endif

    KDirectory *dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &dir ) );
    const KFile *org;
    REQUIRE_RC ( KDirectoryOpenFileRead ( dir, &org, "%s", DATAFILE ) ); // org.data

    uint32_t cluster_factor = 4;
    uint32_t ram_pages = 1024;
    const int num_chunks = 1024;
    
    /* open and use cacheteev3 with an invalid path for the first time */
    const KFile *tee1;
    REQUIRE_RC ( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee1, org, BLOCKSIZE,
        cluster_factor, ram_pages, false, true, "%s", InvalidPath) ); // cache.dat
    for ( int i = 0; i < num_chunks; ++i )
    {
        uint64_t pos = rand_32 ( 0, DATAFILESIZE );
        size_t   len = rand_32 ( 10, 1000 );
        REQUIRE_RC( compare_file_content_3 ( org, tee1, pos, len, NULL ) );
    }
    KFileRelease ( tee1 );

    /* open and use cacheteev3 with an invalid path for the second time */
    const KFile *tee2;
    REQUIRE_RC ( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee2, org, BLOCKSIZE,
        cluster_factor, ram_pages, false, true, "%s", InvalidPath) ); // cache.dat
    for ( int i = 0; i < num_chunks; ++i )
    {
        uint64_t pos = rand_32 ( 0, DATAFILESIZE );
        size_t   len = rand_32 ( 10, 1000 );
        REQUIRE_RC( compare_file_content_3 ( org, tee2, pos, len, NULL ) );
    }
    KFileRelease ( tee2 );
    
    KFileRelease ( org );
    
    KDirectoryRelease ( dir );
    KOutMsg ( "invalid path done\n" );
}


#if 0
struct Recorder
{
    ofstream f;
    mutex mtx;
    uint32_t counter;
    
    Recorder( const string &filename )
    {
        f.open( filename.c_str() );
        counter = 0;
    }
    
    ~Recorder() { f.close(); }
    
    void print_rc( rc_t rc )
    {
        if ( rc == 0 )
            f << "OK";
        else
        {
            char buffer[ 256 ];
            size_t num_writ;
            string_printf( buffer, sizeof buffer, &num_writ, "%R", rc );
            f << buffer;
        }
    }
    
    void record( char src, char event, rc_t rc, uint64_t pos, size_t req_size, size_t done_size )
    {
        mtx.lock();
        if ( src == 'O' ) counter++;
        f << src << " | " << event << " | rc=";
        print_rc( rc );
        f << " | pos=" << pos << " | req=" << req_size << " | done = " << done_size << endl; 
        mtx.unlock();
    }
    
    void write( const string &txt ) { f << txt << endl; }
    void clear( void ) { counter = 0; }
};

void CC record_events( char event, rc_t rc, uint64_t pos, size_t req_size, size_t done_size,
                      void *data1, void *data2 )
{
    Recorder * r = ( Recorder * )data1;
    if ( r != NULL )
    {
        char * src = ( char * )data2;
        r -> record( *src, event, rc, pos, req_size, done_size );
    }
}

FIXTURE_TEST_CASE ( CacheTee3_request_record, CT3Fixture )
{
    KOutMsg ( "recording requests\n" );
    remove_file ( CACHEFILE );
    remove_file ( CACHEFILE1 );

    KDirectory *dir;
    REQUIRE_RC ( KDirectoryNativeDir ( &dir ) );
    const KFile *org1;
    REQUIRE_RC ( KDirectoryOpenFileRead ( dir, &org1, "%s", DATAFILE ) ); // org.data

    const KFile *org2;
    REQUIRE_RC ( KDirectoryOpenFileRead ( dir, &org2, "%s", DATAFILE ) ); // org.data
    
    Recorder r( "recorded.txt" );
    char org_tag = 'O';
    KFile *org1_recorded;
    REQUIRE_RC ( MakeCallBackFile ( &org1_recorded, (KFile *)org1, record_events, &r, &org_tag ) );
    KFileRelease ( org1 );
    
    const KFile *tee;
    uint32_t cluster_factor = 2;
    uint32_t ram_pages = 0;
    REQUIRE_RC ( KDirectoryMakeKCacheTeeFile_v3 ( dir, &tee, org1_recorded, BLOCKSIZE,
                        cluster_factor, ram_pages, false, false, "%s", CACHEFILE ) ); // cache.dat
    KFileRelease ( org1_recorded );
    
    KFile *tee_recorded;
    char tee_tag = 'T';
    REQUIRE_RC ( MakeCallBackFile ( &tee_recorded, (KFile *)tee, record_events, &r, &tee_tag ) );
    KFileRelease ( tee );

    const int num_chunks = 2048;
    for ( int i = 0; i < num_chunks; ++i )
    {
        uint64_t pos = rand_32 ( 0, DATAFILESIZE );
        size_t   len = rand_32 ( 10, 1000 );
        REQUIRE_RC( compare_file_content_3 ( org2, tee_recorded, pos, len, NULL ) );
    }

    for ( int i = 0; i < num_chunks; ++i )
    {
        uint64_t pos = rand_32 ( 0, DATAFILESIZE );
        size_t   len = rand_32 ( 10, 10000 );
        REQUIRE_RC( compare_file_content_3 ( org2, tee_recorded, pos, len, NULL ) );
    }

    // after reading the whole thing - no read-request should be made via orig...
    KOutMsg ( "requesting whole file\n" );
    REQUIRE_RC ( read_whole_file( tee_recorded, 1024 * 1024 ) );

    r.write( "------ whole file read ----" );
    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
    r.clear();
    
    for ( int i = 0; i < 1024; ++i )
    {
        size_t   len = rand_32 ( 10, 20000 );        
        uint64_t pos = rand_32 ( 0, DATAFILESIZE );
        REQUIRE_RC( compare_file_content_3 ( org2, tee_recorded, pos, len, NULL ) );
    }
    
    REQUIRE_RC( compare_file_content_3 ( org2, tee_recorded, DATAFILESIZE - 1000, 2000, NULL ) );
    
    REQUIRE( r . counter == 0 );
    
    KFileRelease ( tee_recorded );
    KFileRelease ( org2 );
    
    KDirectoryRelease ( dir );
    KOutMsg ( "recording requests done\n" );
}
#endif

TEST_CASE( concurrent_reads_from_different_files )
{
    KOutMsg ( "concurrent reads from different files\n" );

    std::string urls[] = {
        "https://ftp.ncbi.nlm.nih.gov/toolbox/gbench/samples/udc_seqgraphic_rmt_testing/remote_BAM_remap_UUD-324/human/grch38_wgsim_gb_accs.bam",
        "https://ftp.ncbi.nlm.nih.gov/toolbox/gbench/samples/udc_seqgraphic_rmt_testing/remote_BAM_remap_UUD-324/human/grch38_wgsim_gb_accs.bam.bai",
        "https://ftp.ncbi.nlm.nih.gov/toolbox/gbench/samples/udc_seqgraphic_rmt_testing/remote_BAM_remap_UUD-324/yeast/yeast_wgsim_ucsc.bam",
        "https://ftp.ncbi.nlm.nih.gov/toolbox/gbench/samples/udc_seqgraphic_rmt_testing/remote_BAM_remap_UUD-324/yeast/yeast_wgsim_ucsc.bam.bai"
    };
    
    const size_t URL_COUNT = sizeof( urls ) / sizeof( urls[ 0 ] );
    uint64_t sizes[ URL_COUNT ];
    uint32_t magics[ URL_COUNT ];
    
    VFSManager* mgr = 0;
    REQUIRE_RC( VFSManagerMake( &mgr ) );
    for ( size_t i = 0; i < URL_COUNT; ++i )
    {
        VPath* path = 0;
        REQUIRE_RC( VFSManagerMakePath( mgr, &path, urls[ i ].c_str() ) );

        const KFile* file = 0;
        REQUIRE_RC( VFSManagerOpenFileRead( mgr, &file, path ) );
        
        REQUIRE_RC( KFileSize( file, &sizes[ i ] ) );
        REQUIRE_RC( KFileReadExactly( file, 0, &magics[ i ], sizeof( magics[ i ] ) ) );
        std::cout << urls[ i ] <<": "<< sizes[ i ] << " " << std::hex << magics[ i ] << std::dec << std::endl;
        REQUIRE_RC( KFileRelease( file ) );
        REQUIRE_RC( VPathRelease( path ) );
    }

    const size_t NUM_PASSES = 2;
    const size_t NUM_THREADS = 8;
    size_t thread_errors[ NUM_THREADS ];
    for ( size_t i = 0; i < NUM_THREADS; ++i )
        thread_errors[ i ] = 0;
    
    for ( size_t pass = 0; pass < NUM_PASSES; ++pass )
    {
        vector< thread > tt( NUM_THREADS );
        for ( size_t i = 0; i < NUM_THREADS; ++i )
        {
            size_t url_index = ( i % URL_COUNT );
            tt[i] = thread( [&]( const string& url, uint64_t exp_size, uint32_t exp_magic, size_t  * num_errors )
                {
                    VPath* path = 0;
                    const KFile* file = 0;
                    uint64_t size = 0;
                    char buffer[ 20 * 1024 ];

                    if ( 0 != VFSManagerMakePath( mgr, &path, url.c_str() ) )
                       (*num_errors)++;
                    else
                    {
                        if ( 0 != VFSManagerOpenFileRead( mgr, &file, path ) )
                            (*num_errors)++;
                        else
                        {
                            if ( 0 != KFileSize( file, &size ) )
                               (*num_errors)++;
                            else
                            {
                                if ( 0 != KFileReadExactly( file, 0, &buffer, sizeof( buffer ) ) )
                                    (*num_errors)++;
                                else
                                {
                                    uint32_t magic = 0;
                                    memcpy( &magic, buffer, sizeof( magic ) );

                                    if ( size != exp_size )
                                        (*num_errors)++;
                                    if ( magic != exp_magic )
                                        (*num_errors)++;
                                }
                            }

                            if ( 0 != KFileRelease( file ) )
                                (*num_errors)++;
                        }
                        if ( 0 != VPathRelease( path ) )
                            (*num_errors)++;
                   }
                }, urls[ url_index ], sizes[ url_index ], magics[ url_index ], &thread_errors[ i ] );
        }
        for ( size_t i = 0; i < NUM_THREADS; ++i )
        {
            tt[ i ].join();
        }
    }

    size_t all_errors = 0;
    for ( size_t i = 0; i < NUM_THREADS; ++i )
        all_errors += thread_errors[ i ];
    REQUIRE( all_errors == 0  );
    
    REQUIRE_RC( VFSManagerRelease( mgr ) );
 
    KOutMsg ( "concurrent reads from different files done\n" );
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
        rc = CacheTee3Tests ( argc, argv );
        KOutMsg ( "and the result is: %R\n", rc );
        ArgsWhack ( args );
    }
    return rc;
}

} /* extern "C" */
