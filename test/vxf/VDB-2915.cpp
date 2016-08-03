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

#include <ktst/unit_test.hpp>
#include <klib/out.h>

#include "zlib_wrapper.h"
#include <zlib.h>

/* --------------------------------------------------------------------------
This test case serves the purpose to fix an issue with the code-path in the
file 'libs/vxf/irzip.c'. Because the code in question is not directly
callable from outside of 'irzip.c., the code-section has been copied
out into this test-directory into the file 'test-irzip.c'
--------------------------------------------------------------------------*/

TEST_SUITE( VDB_2915_TEST_SUITE );


TEST_CASE( RANDOM_UINT8_BETWEEN_1_AND_32 )
{
    uint8_t * source;
    size_t size32k = 1024 * 1024 * 32;
    
    /* first make a buffer of uint8_t's with random values ( between 1 and 32 ) */
    REQUIRE_RC ( make_random_uint8_buffer( &source, size32k, 1, 32 ) );
    
    /* now compress it */
    w_buf compressed;
    REQUIRE_RC ( prepare_compress_buffer( &compressed, size32k ) );
    
    REQUIRE_RC ( wrapped_zlib_compress( &compressed, source, size32k, Z_RLE, Z_BEST_SPEED ) );
    REQUIRE_RC( KOutMsg( "after compression ( Z_RLE, Z_BEST_SPEED ) : %,ld bytes -> %,ld bytes\n", size32k, compressed.used ) );
    
    REQUIRE_RC ( wrapped_zlib_compress( &compressed, source, size32k, Z_HUFFMAN_ONLY, Z_BEST_SPEED ) );
    REQUIRE_RC( KOutMsg( "after compression ( Z_HUFFMAN_ONLY, Z_BEST_SPEED ) : %,ld bytes -> %,ld bytes\n", size32k, compressed.used ) );
    
    /* now decompress it again */
    uint8_t * decompressed;
    REQUIRE_RC( make_uint8_buffer( &decompressed, size32k ) );
    size_t decompressed_size;
    REQUIRE_RC( wrapped_zlib_decompress( decompressed, size32k, &decompressed_size, compressed.buf, compressed.used ) );
    REQUIRE_RC( KOutMsg( "after decompression : %,ld bytes -> %,ld bytes\n", compressed.used, decompressed_size ) );
    
    REQUIRE_RC( wrapped_zlib_decompress( decompressed, size32k, &decompressed_size, compressed.buf, compressed.used ) );
    REQUIRE_RC( KOutMsg( "after decompression : %,ld bytes -> %,ld bytes\n", compressed.used, decompressed_size ) );
    
    finish_compress_buffer( &compressed );
    free( decompressed );
    free( source );
}


TEST_CASE( RANDOM_UINT8_BETWEEN_0_AND_255 )
{
    uint8_t * source;
    size_t size32k = 1024 * 1024 * 32;
    
    /* first make a buffer of uint8_t's with random values ( between 0 and 255 ) */
    REQUIRE_RC ( make_random_uint8_buffer( &source, size32k, 0, 255 ) );
    
    /* now compress it */
    w_buf compressed;
    REQUIRE_RC ( prepare_compress_buffer( &compressed, size32k * 2 ) );
    
    REQUIRE_RC ( wrapped_zlib_compress( &compressed, source, size32k, Z_RLE, Z_BEST_SPEED ) );
    REQUIRE_RC( KOutMsg( "after compression ( Z_RLE, Z_BEST_SPEED ) : %,ld bytes -> %,ld bytes\n", size32k, compressed.used ) );
    
    REQUIRE_RC ( wrapped_zlib_compress( &compressed, source, size32k, Z_HUFFMAN_ONLY, Z_BEST_SPEED ) );
    REQUIRE_RC( KOutMsg( "after compression ( Z_HUFFMAN_ONLY, Z_BEST_SPEED ) : %,ld bytes -> %,ld bytes\n", size32k, compressed.used ) );
    
    /* now decompress it again */
    uint8_t * decompressed;
    REQUIRE_RC( make_uint8_buffer( &decompressed, size32k ) );
    size_t decompressed_size;
    REQUIRE_RC( wrapped_zlib_decompress( decompressed, size32k, &decompressed_size, compressed.buf, compressed.used ) );
    REQUIRE_RC( KOutMsg( "after decompression : %,ld bytes -> %,ld bytes\n", compressed.used, decompressed_size ) );
    
    REQUIRE_RC( wrapped_zlib_decompress( decompressed, size32k, &decompressed_size, compressed.buf, compressed.used ) );
    REQUIRE_RC( KOutMsg( "after decompression : %,ld bytes -> %,ld bytes\n", compressed.used, decompressed_size ) );
    
    finish_compress_buffer( &compressed );
    free( decompressed );
    free( source );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void ) { return 0x1000000; }
rc_t CC UsageSummary ( const char * progname ) { return 0; }
rc_t CC Usage ( const Args * args ){ return 0; }

const char UsageDefaultName[] = "vdb_2915_testwb-test-vxf";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc = VDB_2915_TEST_SUITE( argc, argv );
    return rc;
}

}
