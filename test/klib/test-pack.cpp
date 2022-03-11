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

    template<typename T>
    T 
    PackUnpackSingle( const T input, uint32_t packed )
    {
        const size_t srcBytes = sizeof( input );
        const size_t srcBits = srcBytes * 8;
        THROW_ON_RC( Pack( srcBits, packed, &input, srcBytes, & m_consumedBytes, m_packed.data(), 0, m_packed.size() * 8, & m_packedBits ) );
        THROW_ON_FALSE( srcBytes == m_consumedBytes );
        THROW_ON_FALSE( (bitsz_t)packed == m_packedBits );
        T unpacked = 0;
        THROW_ON_RC( Unpack( packed, srcBits, m_packed.data(), 0, m_packedBits, & m_consumedBits, & unpacked, sizeof( unpacked ), & m_unpackedBytes ) );
        THROW_ON_FALSE( (bitsz_t)packed == m_consumedBits );
        THROW_ON_FALSE( srcBytes == m_unpackedBytes );
        return unpacked;
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
{   
    const uint8_t src = 1;
    REQUIRE_EQ( src, PackUnpackSingle( src, 2 ) );
}

FIXTURE_TEST_CASE(Pack_8_Unpack_single_lossy, PackFixture)
{   
    const uint8_t src = 5;
    REQUIRE_EQ( uint8_t(src & 0x03), PackUnpackSingle( src, 2 ) );
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

FIXTURE_TEST_CASE(Pack_16_Unpack_single, PackFixture)
{   
    const uint16_t src = 0x1234;
    REQUIRE_EQ( src, PackUnpackSingle( src, 13 ) );
}

FIXTURE_TEST_CASE(Pack_16_Unpack_single_lossy, PackFixture)
{   
    const uint16_t src = 0x1234;
    REQUIRE_EQ( uint16_t(src & 0x07ff), PackUnpackSingle( src, 11 ) );
}

FIXTURE_TEST_CASE(Pack_32_Unpack_single, PackFixture)
{   
    const uint32_t src = 0x12345678;
    REQUIRE_EQ( src, PackUnpackSingle( src, 29 ) );
}

FIXTURE_TEST_CASE(Pack_32_Unpack_single_lossy, PackFixture)
{   
    const uint32_t src = 0x1234;
    REQUIRE_EQ( uint32_t(src & 0x07ff), PackUnpackSingle( src, 11 ) );
}

//TODO: 
//FIXTURE_TEST_CASE(Pack_16_Unpack_multiple, PackFixture)
//FIXTURE_TEST_CASE(Pack_32_Unpack_multiple, PackFixture)

FIXTURE_TEST_CASE(Pack_64a_Unpack_single, PackFixture)
{   // into <= 32 bits
    const uint64_t src = 0x12345678;
    REQUIRE_EQ( src, PackUnpackSingle( src, 31 ) );
}

FIXTURE_TEST_CASE(Pack_64a_Unpack_single_lossy, PackFixture)
{   // into <= 32 bits
    const uint64_t src = 0x1234567812345678;
    REQUIRE_EQ( uint64_t(src & 0x7fffffff), PackUnpackSingle( src, 31 ) );
}

FIXTURE_TEST_CASE(Pack_64b_Unpack_single, PackFixture)
{   // into > 32 bits
    const uint64_t src = 0x12345678;
    REQUIRE_EQ( src, PackUnpackSingle( src, 33 ) );
}

FIXTURE_TEST_CASE(Pack_64b_Unpack_single_lossy, PackFixture)
{   // into > 32 bits
    const uint64_t src = 0x1234567812345678;
    REQUIRE_EQ( uint64_t(src & 0x1ffffffff), PackUnpackSingle( src, 33 ) );
}

//TODO:
//FIXTURE_TEST_CASE(Pack_64a_Unpack_multiple, PackFixture)
//FIXTURE_TEST_CASE(Pack_64b_Unpack_multiple, PackFixture)

// TEST_CASE(Pack_fail)
// {   // non-0 destination offsets are not suported
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
