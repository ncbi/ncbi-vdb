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
    PackUnpackSingle( const T input, uint32_t packed, bool debug = false )
    {
        const size_t srcBytes = sizeof( input );
        const size_t srcBits = srcBytes * 8;
        THROW_ON_RC( Pack( srcBits, packed, &input, srcBytes, & m_consumedBytes, m_packed.data(), 0, srcBits, & m_packedBits ) );
        THROW_ON_FALSE( srcBytes == m_consumedBytes );
        THROW_ON_FALSE( (bitsz_t)packed == m_packedBits );
        if ( debug )
        {
            int i = 0;
            bitsz_t bits = m_packedBits;
            while ( true )
            {
                if ( bits > 8 )
                {
                    cout << hex << int( m_packed[i] );
                    i++;
                    bits -= 8;
                }
                else
                {   // last byte, // 8 => bits > 0
                    // clear (8 - bits) least significant gits
                    const uint8_t mask[] = { 0, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
                    cout << hex << int( m_packed[i] & mask[i] );
                    break;
                }
            }
            cout << endl;
        }

        T unpacked = 0;
        THROW_ON_RC( Unpack( packed, srcBits, m_packed.data(), 0, m_packedBits, & m_consumedBits, & unpacked, sizeof( unpacked ), & m_unpackedBytes ) );
        THROW_ON_FALSE( (bitsz_t)packed == m_consumedBits );
        THROW_ON_FALSE( srcBytes == m_unpackedBytes );
        return unpacked;
    }

    template<typename T, size_t N>
    array<T, N>
    PackUnpackMultiple( const array<T, N> & input, uint32_t packed, bool debug = false )
    {
        const size_t srcBytes = sizeof( T ) * N;
        const size_t srcBits = sizeof( T ) * 8;
        array<uint8_t, N * srcBytes> packedBuf;
        if ( debug )
            for( size_t i = 0; i < N; ++i ) cout << hex << input[i] << endl;

        THROW_ON_RC( Pack( srcBits, packed, input.data(), srcBytes, & m_consumedBytes, packedBuf.data(), 0, sizeof( T ) * N * 8, & m_packedBits ) );
        THROW_ON_FALSE( srcBytes == m_consumedBytes );
        THROW_ON_FALSE( (bitsz_t)packed * input.size() == m_packedBits );

        if ( debug )
        {
            int i = 0;
            bitsz_t bits = m_packedBits;
            while ( true )
            {
                if ( bits > 8 )
                {
                    cout << hex << int( packedBuf[i] );
                    i++;
                    bits -= 8;
                }
                else
                {   // last byte, // 8 => bits > 0
                    // clear (8 - bits) least significant gits
                    const uint8_t mask[] = { 0, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
                    cout << hex << int( packedBuf[i] & ~mask[bits] );
                    break;
                }
            }
            cout << endl;
        }

        array<T, N> unpackedBuf;
        THROW_ON_RC( Unpack( packed, srcBits, packedBuf.data(), 0, m_packedBits, & m_consumedBits, unpackedBuf.data(), unpackedBuf.size() * N * 8, & m_unpackedBytes ) );
        if ( debug )
            for( size_t i = 0; i < N; ++i ) cout << hex << unpackedBuf[i] << endl;
        THROW_ON_FALSE( m_packedBits == m_consumedBits );
        THROW_ON_FALSE( srcBytes == m_unpackedBytes );
        return unpackedBuf;
    }

};

FIXTURE_TEST_CASE(Pack_64b_Unpack_multiple, PackFixture)
{   // into >= 32 bits
    array< uint64_t, 8 > src = { 0x1ffffffffUL, 25678, 390123, 45678, 590123, 645678, 790123, 845678 };
    for ( uint32_t packed = 40; packed < 41; ++packed )
    {
        REQUIRE( src == (PackUnpackMultiple( src, packed, true )) );
    }
}


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
