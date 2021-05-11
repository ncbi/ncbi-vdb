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

#include <ext/bzlib.h>

TEST_SUITE(Bzip2TestSuite);

using namespace std;

TEST_CASE(Version)
{
    // not going to survive an update
    REQUIRE_EQ( string("1.0.6, 6-Sept-2010"), string( BZ2_bzlibVersion() ));
}

TEST_CASE(RoundTrip)
{
    srand (time(NULL));
    const size_t SrcSize = 2048;
    string src( SrcSize, 0 );
    for (size_t i = 0; i < SrcSize; ++i)
        src[ i ] = 'A'+(rand() % 26);

    uint32_t dsize = 0;
    size_t destSize = SrcSize * 2;
    auto dst = new char[ destSize ];

    {
        bz_stream s;
        memset ( & s, 0, sizeof s );

        const int blockSize100k = 5;
        const int workFactor = 0;
        REQUIRE_EQ( BZ_OK, BZ2_bzCompressInit ( & s, blockSize100k, 0, workFactor ) );

        s.next_in = const_cast<char*>( src.data() );
        s.avail_in = SrcSize;
        s.next_out = dst;
        s.avail_out = destSize;

        REQUIRE_GE( BZ_STREAM_END, BZ2_bzCompress(&s, BZ_FINISH) );
        assert ( s.total_out_hi32 == 0 );
        dsize = s.total_out_lo32;
        REQUIRE_EQ( BZ_OK, BZ2_bzCompressEnd ( & s ) );
    }

    char * decomp = new char[ SrcSize ];
    {
        bz_stream s;
        memset ( & s, 0, sizeof s );
        REQUIRE_EQ( BZ_OK, BZ2_bzDecompressInit ( & s, 0, 0 ) );

        s.next_in = dst;
        s.avail_in = dsize;
        s.next_out = decomp;
        s.avail_out = SrcSize;

        REQUIRE_GE( BZ_STREAM_END, BZ2_bzDecompress(&s) );
        REQUIRE_EQ( BZ_OK, BZ2_bzDecompressEnd ( & s ) );
        assert ( s.total_out_hi32 == 0 );
        dsize = s.total_out_lo32;
    }

    REQUIRE_EQ( SrcSize, (size_t)dsize );
    REQUIRE_EQ( 0, memcmp( src.data(), decomp, SrcSize ));


    REQUIRE_EQ( src, string(decomp) );

    delete [] decomp;
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

const char UsageDefaultName[] = "test-bzip2";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=Bzip2TestSuite(argc, argv);
    return rc;
}

}
