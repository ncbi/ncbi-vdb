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
* Unit tests for klib Pack function
*/

#include <klib/pack.h>

#include <ktst/unit_test.hpp>

#include <array>

using namespace std;

#define ALL

TEST_SUITE(KlibPackTestSuite);

class PackFixture
{
public:
    bitsz_t m_packedBits;
    bitsz_t m_consumedBits;

    size_t m_consumedBytes;
    size_t m_unpackedBytes;

    typedef std::array<unsigned char, 8> Buffer;
    Buffer m_src        { 0, 1, 2, 3, 3, 2, 1, 0 };
    Buffer m_packed     { 0, 0, 0, 0, 0, 0, 0, 0 };
    Buffer m_unpacked   { 0, 0, 0, 0, 0, 0, 0, 0 };

    void DumpBuffer( const Buffer & p_b )
    {
        for( size_t i = 0; i < p_b.size(); ++i ) cout << i << ": " << (unsigned int)p_b[i] << endl;
    }
};

FIXTURE_TEST_CASE(Pack_null_psize, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 8, 8, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, nullptr ) );
}
FIXTURE_TEST_CASE(Pack_bad_unpacked, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 7, 8, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) ); // <8
    REQUIRE_RC_FAIL( Pack( 65, 8, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) ); // >64
    REQUIRE_RC_FAIL( Pack( 9, 8, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) ); // not a power of 2
}
FIXTURE_TEST_CASE(Pack_bad_packed, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 8, 9, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) ); // m_packed > unpacked
    REQUIRE_RC_FAIL( Pack( 8, 0, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) ); // == 0
}
FIXTURE_TEST_CASE(Pack_0_ssize, PackFixture)
{
    REQUIRE_RC( Pack( 8, 8, m_src.data(), 0, & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) ); // do nothing
    REQUIRE_EQ( size_t(0), m_consumedBytes );
    REQUIRE_EQ( bitsz_t(0), m_packedBits );
}
FIXTURE_TEST_CASE(Pack_bad_ssize, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 8, 8, m_src.data(), size_t(-1), & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) );
}
FIXTURE_TEST_CASE(Pack_small_dsize_no_consumed, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 8, 8, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8 - 1, & m_packedBits ) );
}
FIXTURE_TEST_CASE(Pack_small_dsize, PackFixture)
{   // no error but consumed < dsize
    REQUIRE_RC( Pack( 8, 8, m_src.data(), m_src.size(), & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8 - 1, & m_packedBits ) );
    REQUIRE_EQ( size_t(7), m_consumedBytes );
    REQUIRE_EQ( m_consumedBytes * 8, m_packedBits );
}
FIXTURE_TEST_CASE(Pack_null_dest, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 8, 8, m_src.data(), m_src.size(), nullptr, nullptr, 0, m_packed.size() * 8, & m_packedBits ) );
}

FIXTURE_TEST_CASE(Pack_memcpy, PackFixture)
{   // 8 bit to 8 bit is a memcpy
    REQUIRE_RC( Pack( 8, 8, m_src.data(), m_src.size(), & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) );
    REQUIRE( m_src == m_packed );
    REQUIRE_EQ( m_src.size(), m_consumedBytes );
    REQUIRE_EQ( m_consumedBytes * 8, m_packedBits );
}

FIXTURE_TEST_CASE(Unpack_null_usize, PackFixture)
{
    REQUIRE_RC_FAIL( Unpack( 8, 8, m_packed.data(), 0, m_packed.size() * 8, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), nullptr ) );
}

FIXTURE_TEST_CASE(Unpack_bad_unpacked, PackFixture)
{
    REQUIRE_RC_FAIL( Unpack( 8, 9, m_packed.data(), 0, m_packed.size() * 8, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE_RC_FAIL( Unpack( 8, 7, m_packed.data(), 0, m_packed.size() * 8, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE_RC_FAIL( Unpack( 8, 65, m_packed.data(), 0, m_packed.size() * 8, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE_RC_FAIL( Unpack( 16, 8, m_packed.data(), 0, m_packed.size() * 8, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE_RC_FAIL( Unpack( 0, 8, m_packed.data(), 0, m_packed.size() * 8, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
}

FIXTURE_TEST_CASE(Unpack_ssize_0, PackFixture)
{
    REQUIRE_RC( Unpack( 8, 8, m_packed.data(), 0, 0, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE_EQ( size_t(0), m_consumedBits );
    REQUIRE_EQ( size_t(0), m_unpackedBytes );
}

FIXTURE_TEST_CASE(Unpack_ssize_low, PackFixture)
{
    REQUIRE_RC( Unpack( 8, 8, m_packed.data(), 0, 7, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE_EQ( size_t(0), m_consumedBits );
    REQUIRE_EQ( size_t(0), m_unpackedBytes );
}

FIXTURE_TEST_CASE(Unpack_null_src, PackFixture)
{
    REQUIRE_RC_FAIL( Unpack( 8, 8, nullptr, 0, m_packed.size() * 8, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
}

FIXTURE_TEST_CASE(Unack_small_dsize_no_consumed, PackFixture)
{
    REQUIRE_RC_FAIL( Unpack( 8, 8, m_packed.data(), 0, m_packed.size() * 8, nullptr, m_unpacked.data(), m_unpacked.size() - 1, & m_unpackedBytes ) );
}

FIXTURE_TEST_CASE(Unack_null_usize, PackFixture)
{
    REQUIRE_RC_FAIL( Unpack( 8, 8, m_packed.data(), 0, m_packed.size() * 8, nullptr, m_unpacked.data(), m_unpacked.size(), nullptr ) );
}

//
// round trips
//

FIXTURE_TEST_CASE(Pack_Unpack_memcpy, PackFixture)
{   // 8 bit to 8 bit is a memcpy both ways
    REQUIRE_RC( Pack( 8, 8, m_src.data(), m_src.size(), & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) );
    REQUIRE( m_src == m_packed );
    REQUIRE_EQ( m_src.size(), m_consumedBytes );
    REQUIRE_EQ( m_consumedBytes * 8, m_packedBits );
    REQUIRE_RC( Unpack( 8, 8, m_packed.data(), 0, m_packedBits, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE( m_src == m_unpacked );
    REQUIRE_EQ( m_src.size() * 8, m_consumedBits );
    REQUIRE_EQ( m_consumedBits, m_unpackedBytes  * 8);
}

FIXTURE_TEST_CASE(Pack_8_Unpack_single, PackFixture)
{   // into < 8 bits
    const uint8_t src = 1;
    REQUIRE_RC( Pack( 8, 2, &src, sizeof( src ), & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) );
    Buffer expected_packed = { 0b01000000, 0, 0, 0, 0, 0, 0, 0 };
    REQUIRE( expected_packed == m_packed );
    REQUIRE_EQ( sizeof( src ), m_consumedBytes );
    REQUIRE_EQ( (bitsz_t)2, m_packedBits );
    uint8_t unpacked = 0;
    REQUIRE_RC( Unpack( 2, 8, m_packed.data(), 0, m_packedBits, & m_consumedBits, & unpacked, sizeof( unpacked ), & m_unpackedBytes ) );
    REQUIRE( src == unpacked );
    REQUIRE_EQ( (bitsz_t)2, m_consumedBits );
    REQUIRE_EQ( (size_t)1, m_unpackedBytes );
}

FIXTURE_TEST_CASE(Pack_8_Unpack_multiple, PackFixture)
{   // into < 8 bits
    Buffer src = { 1, 2, 3, 4, 0, 0, 0, 0 };
    REQUIRE_RC( Pack( 8, 3, src.data(), 4, & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) );
    Buffer expected_packed = { 0b00101001, 0b11000000, 0, 0, 0, 0, 0, 0 };
    //DumpBuffer(m_packed);
    REQUIRE( expected_packed == m_packed );
    REQUIRE_EQ( (size_t)4, m_consumedBytes );
    REQUIRE_EQ( (bitsz_t)(3 * 4), m_packedBits );
    REQUIRE_RC( Unpack( 3, 8, m_packed.data(), 0, m_packedBits, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE( src == m_unpacked );
    REQUIRE_EQ( (bitsz_t)(3 * 4), m_consumedBits );
    REQUIRE_EQ( m_consumedBytes, m_unpackedBytes );
}

#if FAILS
FIXTURE_TEST_CASE(Pack_8_Unpack_multiple, PackFixture)
{   // into < 8 bits
    REQUIRE_RC( Pack( 8, 2, m_src.data(), m_src.size(), & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) );
    //                        { 0, 1,    2, 3,   3, 2,  1, 0 };
    Buffer expected_packed = { 0b0001, 0b1011, 0b1110, 0b0100, 0, 0, 0, 0};
    REQUIRE( expected_packed == m_packed );
    REQUIRE_EQ( m_src.size(), m_consumedBytes );
    REQUIRE_EQ( m_src.size() * 2, (size_t) m_packedBits );
    // REQUIRE_RC( Unpack( 8, 8, m_packed.data(), 0, m_packed.size() * 8, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    // REQUIRE( m_src == m_unpacked );
    // REQUIRE_EQ( m_src.size() * 8, m_consumedBits );
    // REQUIRE_EQ( m_consumedBits, m_unpackedBytes  * 8);
}
#endif

FIXTURE_TEST_CASE(Pack_64a_Unpack, PackFixture)
{   // into <= 32 bits
    const uint64_t src = 0x01020304;
    REQUIRE_RC( Pack( 64, 31, &src, sizeof( src ), & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) );
    //REQUIRE_EQ( (uint32_t)src, *(uint32_t*)m_packed.data() );
    REQUIRE_EQ( sizeof( src ), m_consumedBytes );
    REQUIRE_EQ( (bitsz_t)31, m_packedBits );
    REQUIRE_RC( Unpack( 31, 64, m_packed.data(), 0, m_packedBits, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE_EQ( src, *(uint64_t*)m_unpacked.data() );
    REQUIRE_EQ( m_packedBits, m_consumedBits );
    REQUIRE_EQ( sizeof( src ), m_unpackedBytes );
}
FIXTURE_TEST_CASE(Pack_64a_Unpack_lossy, PackFixture)
{   // into <= 32 bits
    const uint64_t src = 0x0102030405;
    REQUIRE_RC( Pack( 64, 31, &src, sizeof( src ), & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) );
    //REQUIRE_EQ( (uint32_t)src, *(uint32_t*)m_packed.data() );
    REQUIRE_EQ( sizeof( src ), m_consumedBytes );
    REQUIRE_EQ( (bitsz_t)31, m_packedBits );
    REQUIRE_RC( Unpack( 31, 64, m_packed.data(), 0, m_packedBits, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE_EQ( 0x2030405ul /*src & 0x7fffffff*/, *(uint64_t*)m_unpacked.data() );
    REQUIRE_EQ( m_packedBits, m_consumedBits );
    REQUIRE_EQ( sizeof( src ), m_unpackedBytes );
}

#if FAILS
FIXTURE_TEST_CASE(Pack_64b_Unpack, PackFixture)
{   // into > 32 bits
    const uint64_t src = 0x010203040506;
    REQUIRE_RC( Pack( 64, 50, &src, sizeof( src ), & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) );
    //REQUIRE_EQ( (uint32_t)src, *(uint32_t*)m_packed.data() );
    REQUIRE_EQ( sizeof( src ), m_consumedBytes );
    REQUIRE_EQ( (bitsz_t)50, m_packedBits );
    REQUIRE_RC( Unpack( 50, 64, m_packed.data(), 0, m_packedBits, & m_consumedBits, m_unpacked.data(), m_unpacked.size(), & m_unpackedBytes ) );
    REQUIRE_EQ( src, *(uint64_t*)m_unpacked.data() );
    REQUIRE_EQ( m_packedBits, m_consumedBits );
    REQUIRE_EQ( sizeof( src ), m_unpackedBytes );
}
#endif

// FIXTURE_TEST_CASE(Pack_small_dsize_partially_consumed, PackFixture)
// {
//     const unsigned char src[]   = { 0, 1, 2, 3, 4, 5, 6, 7 };
//     unsigned char packed[]      = { 0, 0, 0, 0, 0, 0, 0, 0 };
//     REQUIRE_RC_FAIL( Pack( 8, 8, src.data(), src.size(), & m_consumedBits, packed.data(), 0, packed.size() * 8 - 1, & m_psize ) );
//     // verify psize
// }

// TEST_CASE(Pack_success)
// {
//     const unsigned char src[] = { 0, 1, 2, 3, 3, 2, 1, 0 };
//     unsigned char packed[2] = { 0, 0 };
//     size_t consumed;
//     bitsz_t psize;
//     REQUIRE_RC( Pack( 64, 2, src.data(), src.size(), & consumed, packed.data(), 0, packed.size() * 8, & psize ) );
//     REQUIRE_EQ( sizeof( src ), consumed );
//     REQUIRE_EQ( (bitsz_t)2, psize );
// cout<<(unsigned int)packed[0]<<endl;
// cout<<(unsigned int)packed[1]<<endl;
// //    REQUIRE_EQ( (unsigned int)0xbe, (unsigned int)packed[0] );
// //    REQUIRE_EQ( (unsigned int)0xfe, (unsigned int)packed[1] );

//     unsigned char unpacked[8];
//     REQUIRE_RC( Unpack( 2, 64, packed, 0, packed.size() * 8, &consumed, unpacked, sizeof( unpacked ), & psize ) );
//     REQUIRE_EQ( packed.size() * 8, consumed );
//     REQUIRE_EQ( (bitsz_t)64, psize );
//     REQUIRE_EQ( (unsigned int)src[0], (unsigned int)unpacked[0] );
//     REQUIRE_EQ( (unsigned int)src[1], (unsigned int)unpacked[1] );
//     REQUIRE_EQ( (unsigned int)src[2], (unsigned int)unpacked[2] );
//     REQUIRE_EQ( (unsigned int)src[3], (unsigned int)unpacked[3] );
//     REQUIRE_EQ( (unsigned int)src[4], (unsigned int)unpacked[4] );
//     REQUIRE_EQ( (unsigned int)src[5], (unsigned int)unpacked[5] );
//     REQUIRE_EQ( (unsigned int)src[6], (unsigned int)unpacked[6] );
//     REQUIRE_EQ( (unsigned int)src[7], (unsigned int)unpacked[7] );
// }

// TEST_CASE(Pack_fail)
// {   // no -0 destination offsets are not suported
//     const char src[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
//     char packed[8];
//     size_t consumed;
//     bitsz_t psize;
//     const bitsz_t UnsupportedDstOff = 2; // non-0 dst_off
//     REQUIRE_RC_FAIL( Pack( 64, 2, src.data(), src.size(), & consumed, packed, UnsupportedDstOff, packed.size() * 8 - UnsupportedDstOff, & psize ) );
// }

//////////////////////////////////////////////////// Main
extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion(void) { return 0x1000000; }
rc_t CC UsageSummary(const char* progname) { return 0; }

rc_t CC Usage(const Args* args) { return 0; }

const char UsageDefaultName[] = "Test_KLIB_pack";

rc_t CC KMain(int argc, char* argv[])
{
    KConfigDisableUserSettings();
    rc_t rc = KlibPackTestSuite(argc, argv);
    return rc;
}
}
