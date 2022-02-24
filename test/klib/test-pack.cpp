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
    bitsz_t m_psize;
    size_t m_consumed;

    std::array<unsigned char, 8> m_src    { 0, 1, 2, 3, 3, 2, 1, 0 };
    std::array<unsigned char, 8> m_packed { 0, 0, 0, 0, 0, 0, 0, 0 };
};

FIXTURE_TEST_CASE(Pack_null_psize, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 8, 8, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, nullptr ) );
}
FIXTURE_TEST_CASE(Pack_bad_unpacked, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 7, 8, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, & m_psize ) ); // <8
    REQUIRE_RC_FAIL( Pack( 65, 8, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, & m_psize ) ); // >64
    REQUIRE_RC_FAIL( Pack( 9, 8, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, & m_psize ) ); // not a power of 2
}
FIXTURE_TEST_CASE(Pack_bad_packed, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 8, 9, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, & m_psize ) ); // m_packed > unpacked
    REQUIRE_RC_FAIL( Pack( 8, 0, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8, & m_psize ) ); // == 0
}
FIXTURE_TEST_CASE(Pack_0_ssize, PackFixture)
{
    REQUIRE_RC( Pack( 8, 8, m_src.data(), 0, & m_consumed, m_packed.data(), 0, m_packed.size() * 8, & m_psize ) ); // do nothing
    REQUIRE_EQ( size_t(0), m_consumed );
    REQUIRE_EQ( bitsz_t(0), m_psize );
}
FIXTURE_TEST_CASE(Pack_bad_ssize, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 8, 8, m_src.data(), size_t(-1), & m_consumed, m_packed.data(), 0, m_packed.size() * 8, & m_psize ) );
}
FIXTURE_TEST_CASE(Pack_small_dsize_no_consumed, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 8, 8, m_src.data(), m_src.size(), nullptr, m_packed.data(), 0, m_packed.size() * 8 - 1, & m_psize ) ); // error if consumed == nullptr
}
FIXTURE_TEST_CASE(Pack_null_dest, PackFixture)
{
    REQUIRE_RC_FAIL( Pack( 8, 8, m_src.data(), m_src.size(), nullptr, nullptr, 0, m_packed.size() * 8, & m_psize ) ); // error if consumed == nullptr
}

FIXTURE_TEST_CASE(Pack_copy, PackFixture)
{   // 8 bit to 8 bit is a copy
    REQUIRE_RC( Pack( 8, 8, m_src.data(), m_src.size(), & m_consumed, m_packed.data(), 0, m_packed.size() * 8, & m_psize ) );
    REQUIRE( m_src == m_packed );
    REQUIRE_EQ( m_src.size(), m_consumed );
    REQUIRE_EQ( m_consumed * 8, m_psize ); // consumed in bytes, psize in bits
}

FIXTURE_TEST_CASE(Pack_Unpack8, PackFixture)
{
    REQUIRE_RC( Pack( 8, 8, m_src.data(), m_src.size(), & m_consumed, m_packed.data(), 0, m_packed.size() * 8, & m_psize ) ); // error if consumed == nullptr
    REQUIRE( m_src == m_packed );
    REQUIRE_EQ( m_src.size(), m_consumed );
}

// FIXTURE_TEST_CASE(Pack_small_dsize_partially_consumed, PackFixture)
// {
//     const unsigned char src[]   = { 0, 1, 2, 3, 3, 2, 1, 0 };
//     unsigned char packed[]      = { 0, 0, 0, 0, 0, 0, 0, 0 };
//     REQUIRE_RC_FAIL( Pack( 8, 8, src.data(), src.size(), & m_consumed, packed.data(), 0, packed.size() * 8 - 1, & m_psize ) );
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
