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

#include <ext/zstd.h>

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

TEST_SUITE(ZstdTestSuite);

using namespace std;

TEST_CASE(Version)
{
    unsigned int v = ZSTD_versionNumber();
    REQUIRE_EQ( (unsigned int)ZSTD_VERSION_NUMBER, v);
}

TEST_CASE(VersionString)
{
    string v = ZSTD_versionString();
    REQUIRE_EQ( string( ZSTD_VERSION_STRING ), v);
}

TEST_CASE(CompressionLevel)
{
    // limits are for 1.4.10, may change in other versions;
    // also hard-coded in vxf/zstd.c
    REQUIRE_EQ( -131072, ZSTD_minCLevel() ); // the lower the faster, at the expense of compression
    REQUIRE_EQ( 22, ZSTD_maxCLevel() );
}

TEST_CASE(RoundTrip)
{
    srand (time(NULL));
    const size_t SrcSize = 2048;
    string src( SrcSize, 0 );
    for (size_t i = 0; i < SrcSize; ++i)
        src[ i ] = rand() % 256;

    size_t destSize = SrcSize * 2;
    auto dst = new char[ destSize ];
    size_t size = ZSTD_compress( dst, destSize, src.data(), SrcSize, ZSTD_CLEVEL_DEFAULT);

    REQUIRE_EQ( 0u, ZSTD_isError( size ) );

    auto decomp = new char[ SrcSize ];
    size_t rsize = ZSTD_decompress( decomp, SrcSize, dst, size );

    REQUIRE_EQ( SrcSize, rsize);
    REQUIRE_EQ( 0, memcmp( src.data(), decomp, SrcSize ));

    delete [] decomp;
    delete [] dst;
}

TEST_CASE(getFrameContentSize)
{
    srand (time(NULL));
    const size_t SrcSize = 2048;
    string src( SrcSize, 0 );
    for (size_t i = 0; i < SrcSize; ++i)
        src[ i ] = rand() % 256;

    size_t destSize = SrcSize * 2;
    auto dst = new char[ destSize ];
    ZSTD_compress( dst, destSize, src.data(), SrcSize, ZSTD_CLEVEL_DEFAULT);

    unsigned long long csize = ZSTD_getFrameContentSize(dst, 200); // any size >= ZSTD_FRAMEHEADERSIZE_MAX == 18
    REQUIRE_EQ( (unsigned long long )SrcSize, csize );

    delete [] dst;
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
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "test-zstd";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=ZstdTestSuite(argc, argv);
    return rc;
}

}
